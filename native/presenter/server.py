"""Presentation transport with optional authentication. Game decisions and pixels come from C."""
from datetime import datetime
import base64
import binascii
import hmac
import json
import os
from pathlib import Path
import re
import subprocess
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parent
BUTTONS = {'up', 'down', 'left', 'right', 'research', 'critters', 'library', 'habitat', 'confirm', 'back'}
EVENTS = {f'{button}-{edge}' for button in BUTTONS for edge in ('down', 'up')} | {'cancel', 'suspend', 'resume', 'ready'}


class NativeProcess:
    """One selected native process; serialized status/input and binary frame reads."""
    def __init__(self, executable, timeout=10, kit=False):
        self.process = subprocess.Popen([str(executable), "kit-serve" if kit else "serve"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.lock = threading.Lock()
        self.timeout = timeout
        self.unavailable = False

    def command(self, line, frame=False):
        if not self.lock.acquire(timeout=self.timeout):
            raise RuntimeError("Native transport busy or unavailable")
        finished = threading.Event()

        def expire():
            if not finished.is_set():
                self.unavailable = True
                self.process.kill()

        deadline = threading.Timer(self.timeout, expire)
        try:
            if self.unavailable:
                raise RuntimeError("Native process unavailable")
            deadline.start()
            self.process.stdin.write((line + "\n").encode("ascii"))
            self.process.stdin.flush()
            header = self.process.stdout.readline()
            if not header:
                raise RuntimeError("Native process unavailable")
            try:
                result = json.loads(header)
            except (ValueError, UnicodeError) as error:
                raise RuntimeError("Invalid native response") from error
            if not isinstance(result, dict):
                raise RuntimeError("Invalid native response")
            if not frame or "error" in result:
                return result, None
            parts = line.split()
            dimensions = {"0": (1024, 600), "1": (450, 600), "2": (792, 272)}
            width, height = dimensions.get(parts[1], (1024, 600)) if parts[0] == "device" else (1024, 600)
            expected = 54 + ((width * 3 + 3) & ~3) * height
            if result.get("bytes") != expected:
                raise RuntimeError("Unexpected native frame length")
            pixels = bytearray()
            while len(pixels) < result["bytes"]:
                chunk = self.process.stdout.read(result["bytes"] - len(pixels))
                if not chunk:
                    raise RuntimeError("Native frame interrupted")
                pixels.extend(chunk)
            return result, bytes(pixels)
        except (OSError, RuntimeError):
            self.unavailable = True
            self.process.kill()
            raise
        finally:
            finished.set()
            deadline.cancel()
            if deadline.ident is not None:
                deadline.join()
            self.lock.release()

    def close(self):
        self.process.stdin.close()
        try:
            self.process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            self.process.terminate()
            self.process.wait()


def load_release(path):
    """Read deployment metadata once; no runtime environment or game state."""
    try:
        metadata = json.loads(path.read_text(encoding="utf-8"))
        commit = metadata["commit"]
        committed_at = metadata["committed_at"]
        deployed_at = metadata["deployed_at"]
        if not isinstance(commit, str) or not re.fullmatch(r"[0-9a-f]{40}", commit):
            raise ValueError()
        if not isinstance(committed_at, str):
            raise ValueError()
        committed = datetime.fromisoformat(committed_at.replace("Z", "+00:00"))
        if committed.tzinfo is None:
            raise ValueError()
        if deployed_at is not None:
            if not isinstance(deployed_at, str):
                raise ValueError()
            deployed = datetime.fromisoformat(deployed_at.replace("Z", "+00:00"))
            if deployed.tzinfo is None:
                raise ValueError()
            deployed_at = deployed.isoformat()
        result = {"commit": commit, "committed_at": committed.isoformat(),
                  "deployed_at": deployed_at}
        if "subject" in metadata:
            subject = metadata["subject"]
            if (not isinstance(subject, str) or not 1 <= len(subject) <= 200
                    or not subject.strip() or any(ord(character) < 32 or ord(character) == 127
                                                 or character in "\u0085\u2028\u2029" for character in subject)):
                raise ValueError()
            result["subject"] = subject
        return result
    except (OSError, ValueError, KeyError, TypeError, UnicodeError):
        return {"commit": None, "committed_at": None, "deployed_at": None}


class Handler(BaseHTTPRequestHandler):
    server_version = "CritterPresentation/1"

    def log_message(self, *args):
        pass  # Never record authorization, query strings or request bodies.

    def reply(self, code, body, content_type="application/json"):
        if isinstance(body, dict):
            body = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Referrer-Policy", "no-referrer")
        self.send_header("Content-Security-Policy", "default-src 'self'; img-src 'self' blob:; style-src 'self'; script-src 'self'; frame-ancestors 'none'; base-uri 'none'")
        if code == 401:
            self.send_header("WWW-Authenticate", 'Basic realm="Critter Lab", charset="UTF-8"')
        self.end_headers()
        self.wfile.write(body)

    def authorized(self):
        if not self.server.password:
            return True
        try:
            scheme, value = self.headers.get("Authorization", "").split(" ", 1)
            supplied = base64.b64decode(value, validate=True)
            expected = ("lab:" + self.server.password).encode()
            valid = scheme.lower() == "basic" and hmac.compare_digest(supplied, expected)
        except (ValueError, binascii.Error):
            valid = False
        if not valid:
            self.reply(401, {"error": "Authentication required"})
        return valid

    def do_GET(self):
        if not self.authorized():
            return
        url = urlsplit(self.path)
        try:
            if url.path in {"/", "/index.html", "/style.css", "/app.js"} and not url.query:
                name = "index.html" if url.path == "/" else url.path[1:]
                mime = {"index.html": "text/html; charset=utf-8", "style.css": "text/css", "app.js": "text/javascript"}[name]
                self.reply(200, (ROOT / name).read_bytes(), mime)
            elif url.path == "/api/release" and not url.query:
                self.reply(200, self.server.release)
            elif url.path.startswith("/api/devices/"):
                parts = url.path.split("/")
                devices = {"lab": "0", "companion": "1", "dock": "2"}
                if len(parts) != 5 or parts[3] not in devices or parts[4] not in {"status", "frame"}:
                    raise ValueError()
                prefix = "device " + devices[parts[3]]
                if parts[4] == "status" and url.query:
                    raise ValueError()
                if parts[4] == "status":
                    result, _ = self.server.native.command(prefix + " status")
                    self.reply(200, result)
                else:
                    query = parse_qs(url.query, strict_parsing=True)
                    if set(query) != {"revision"} or len(query["revision"]) != 1:
                        raise ValueError()
                    revision = query["revision"][0]
                    if not re.fullmatch(r"[0-9]{1,10}", revision) or not 0 < int(revision) <= 4294967295:
                        raise ValueError()
                    result, pixels = self.server.native.command(prefix + " frame " + revision, frame=True)
                    self.reply(409 if pixels is None else 200, result if pixels is None else pixels, "application/json" if pixels is None else "image/bmp")
            elif url.path == "/api/status" and not url.query:
                result, _ = self.server.native.command("status")
                self.reply(200, result)
            elif url.path == "/api/frame":
                query = parse_qs(url.query, strict_parsing=True)
                if set(query) != {"revision"} or any(len(v) != 1 for v in query.values()):
                    raise ValueError()
                revision = query["revision"][0]
                if not re.fullmatch(r"[0-9]{1,10}", revision) or not 0 < int(revision) <= 4294967295:
                    raise ValueError()
                result, pixels = self.server.native.command("frame " + revision, frame=True)
                self.reply(409 if pixels is None else 200, result if pixels is None else pixels, "application/json" if pixels is None else "image/bmp")
            else:
                self.reply(404, {"error": "Unknown route"})
        except ValueError:
            self.reply(400, {"error": "Invalid frame query"})
        except (OSError, RuntimeError):
            self.reply(503, {"error": "Unable to load native screen"})

    def do_POST(self):
        if not self.authorized():
            return
        # The local TLS tunnel supplies X-Forwarded-Proto. Never use a forwarded
        # host to broaden the origin allowlist, and never enable CORS.
        try:
            origin = urlsplit(self.headers.get("Origin", ""))
        except ValueError:
            self.reply(403, {"error": "Invalid origin"})
            return
        scheme = self.headers.get("X-Forwarded-Proto", "http")
        if (self.path not in {"/api/input", "/api/device-input", "/api/link"} or scheme not in {"http", "https"}
                or origin.scheme != scheme
                or origin.netloc != self.headers.get("Host") or origin.path
                or origin.query or origin.fragment
                or self.headers.get("X-Requested-With") != "CritterLab"
                or self.headers.get_content_type() != "application/json"):
            self.reply(403, {"error": "Same-origin command required"})
            return
        try:
            size = int(self.headers.get("Content-Length", "0"))
            if not 1 <= size <= 1024:
                raise ValueError()
            command = json.loads(self.rfile.read(size))
            if not isinstance(command, dict):
                raise ValueError()
            devices = {"lab": "0", "companion": "1", "dock": "2"}
            if self.path == "/api/link":
                device, online = command.get("device"), command.get("online")
                if set(command) != {"device", "online"} or device not in {"companion", "dock"} or type(online) is not bool:
                    raise ValueError()
                result, _ = self.server.native.command(f"device {devices[device]} link {int(online)}")
                self.reply(400 if "error" in result else 200, result)
                return
            name, revision = command.get("event"), command.get("revision")
            expected_keys = {"event", "revision"}
            device = command.get("device")
            if self.path == "/api/device-input":
                expected_keys.add("device")
                if device not in devices:
                    raise ValueError()
                allowed_buttons = BUTTONS if device == "lab" else ({"up", "down", "left", "right", "back", "confirm"} if device == "companion" else {"up", "down", "confirm", "research", "critters"})
                allowed = {f"{button}-{edge}" for button in allowed_buttons for edge in ("down", "up")} | {"cancel", "suspend", "resume", "ready"}
                if name not in allowed:
                    raise ValueError()
            if set(command) != expected_keys or not isinstance(name, str) or name not in EVENTS:
                raise ValueError()
            if type(revision) is not int or not 0 <= revision <= 4294967295:
                raise ValueError()
            line = (f"device {devices[device]} " if self.path == "/api/device-input" else "") + f"{name} {revision}"
            result, _ = self.server.native.command(line)
            self.reply(400 if "error" in result else 200, result)
        except (ValueError, UnicodeError):
            self.reply(400, {"error": "Invalid command body"})
        except (OSError, RuntimeError):
            self.reply(503, {"error": "Native transport interrupted; activation stopped"})


def main():
    password = os.environ.get("CRITTER_DEMO_PASSWORD", "")
    server = ThreadingHTTPServer((os.environ.get("CRITTER_DEMO_BIND", "127.0.0.1"),
                                  int(os.environ.get("CRITTER_DEMO_PORT", "4180"))), Handler)
    server.release = load_release(ROOT / "release.json")
    server.password = password
    server.binary = str(Path(os.environ["CRITTER_DEMO_BINARY"]).resolve(strict=True))
    server.native = NativeProcess(server.binary, kit=True)
    access = "authentication required" if password else "anonymous shared staging access"
    print(f"Critter native presenter listening; {access}", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        server.native.close()


if __name__ == "__main__":
    main()



