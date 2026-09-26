"""Authenticated presentation transport. All game decisions and pixels come from C."""
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
LOCK = threading.Lock()
TOKEN = re.compile(r"[A-Za-z0-9_-]{1,64}\Z")


def load_release(path):
    """Read deployment metadata once; no runtime environment or game state."""
    try:
        metadata = json.loads(path.read_text(encoding="utf-8"))
        commit = metadata["commit"]
        deployed_at = metadata["deployed_at"]
        if not isinstance(commit, str) or not re.fullmatch(r"[0-9a-f]{40}", commit):
            raise ValueError()
        if not isinstance(deployed_at, str):
            raise ValueError()
        timestamp = datetime.fromisoformat(deployed_at.replace("Z", "+00:00"))
        if timestamp.tzinfo is None:
            raise ValueError()
        result = {"commit": commit, "deployed_at": timestamp.isoformat()}
        if "subject" in metadata:
            subject = metadata["subject"]
            if (not isinstance(subject, str) or not 1 <= len(subject) <= 200
                    or not subject.strip() or any(ord(character) < 32 or ord(character) == 127
                                                 or character in "\u0085\u2028\u2029" for character in subject)):
                raise ValueError()
            result["subject"] = subject
        return result
    except (OSError, ValueError, KeyError, TypeError, UnicodeError):
        return {"commit": None, "deployed_at": None}


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

    def native(self, arguments, image=False):
        with LOCK:
            result = subprocess.run([self.server.binary, "--save", self.server.save, *arguments],
                                    capture_output=True, timeout=10, check=False)
        if result.returncode:
            try:
                failure = json.loads(result.stdout)
            except (ValueError, UnicodeError):
                failure = {"error": "Native process failed; saved state preserved"}
            # Only exit 2 proves a deterministic rejection. A failed save may
            # already have renamed its snapshot before directory fsync failed.
            # Retain the browser's operation identity for every uncertain exit.
            self.reply(409 if result.returncode == 2 else 503, failure)
        else:
            self.reply(200, result.stdout, "image/bmp" if image else "application/json")

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
            elif url.path == "/api/status" and not url.query:
                self.native(["status"])
            elif url.path == "/api/frame":
                query = parse_qs(url.query, strict_parsing=True)
                if set(query) != {"device", "revision"} or any(len(v) != 1 for v in query.values()):
                    raise ValueError()
                device, revision = query["device"][0], query["revision"][0]
                if device not in {"lab", "probe", "companion"} or not re.fullmatch(r"[0-9]{1,10}", revision):
                    raise ValueError()
                self.native(["frame", device, revision], image=True)
            else:
                self.reply(404, {"error": "Unknown route"})
        except (ValueError, OSError, subprocess.TimeoutExpired):
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
        if (self.path != "/api/command" or scheme not in {"http", "https"}
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
            if not isinstance(command, dict) or set(command) != {"name", "revision", "operation_id"}:
                raise ValueError()
            if not all(isinstance(command[k], str) and TOKEN.fullmatch(command[k]) for k in ("name", "operation_id")):
                raise ValueError()
            if type(command["revision"]) is not int or not 0 <= command["revision"] <= 4294967295:
                raise ValueError()
            self.native(["command", command["name"], str(command["revision"]), command["operation_id"]])
        except (ValueError, UnicodeError):
            self.reply(400, {"error": "Invalid command body"})
        except (OSError, subprocess.TimeoutExpired):
            self.reply(503, {"error": "Command result uncertain; retry the same action"})


def main():
    password = os.environ.get("CRITTER_DEMO_PASSWORD", "")
    if not password:
        raise SystemExit("CRITTER_DEMO_PASSWORD is required")
    server = ThreadingHTTPServer((os.environ.get("CRITTER_DEMO_BIND", "127.0.0.1"),
                                  int(os.environ.get("CRITTER_DEMO_PORT", "4180"))), Handler)
    server.release = load_release(ROOT / "release.json")
    server.password = password
    server.binary = str(Path(os.environ["CRITTER_DEMO_BINARY"]).resolve(strict=True))
    server.save = str(Path(os.environ["CRITTER_DEMO_SAVE"]).resolve())
    print("Critter native presenter listening; authentication required", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
