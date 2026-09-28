"""Bounded review transport. Native C owns every game state and screen pixel."""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import subprocess
import threading
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parent
EVENTS = {"rotate", "confirm-down", "confirm-up", "back-down", "back-up", "cancel", "suspend", "resume", "ready"}
FILES = {"/": ("index.html", "text/html; charset=utf-8"), "/bridge.mjs": ("bridge.mjs", "text/javascript; charset=utf-8"), "/style.css": ("style.css", "text/css; charset=utf-8")}


class NativeProcess:
    def __init__(self, executable):
        self.process = subprocess.Popen([str(executable), "serve"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.lock = threading.Lock()

    def command(self, line, frame=False):
        with self.lock:
            self.process.stdin.write((line + "\n").encode("ascii"))
            self.process.stdin.flush()
            header = self.process.stdout.readline()
            if not header:
                raise RuntimeError("Native process unavailable")
            result = json.loads(header)
            if not frame or "error" in result:
                return result, None
            if result.get("bytes") != 54 + 1024 * 600 * 3:
                raise RuntimeError("Unexpected native frame length")
            pixels = bytearray()
            while len(pixels) < result["bytes"]:
                chunk = self.process.stdout.read(result["bytes"] - len(pixels))
                if not chunk:
                    raise RuntimeError("Native frame interrupted")
                pixels.extend(chunk)
            return result, bytes(pixels)

    def close(self):
        self.process.stdin.close()
        try:
            self.process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            self.process.terminate()
            self.process.wait()


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *arguments):
        pass

    def reply(self, status, body, content_type="application/json"):
        if isinstance(body, dict):
            body = json.dumps(body).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Security-Policy", "default-src 'self'; img-src 'self' blob:; script-src 'self'; style-src 'self'; frame-ancestors 'none'; base-uri 'none'")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        try:
            parsed = urlsplit(self.path)
            if parsed.path in FILES:
                filename, content_type = FILES[parsed.path]
                self.reply(200, (ROOT / filename).read_bytes(), content_type)
            elif parsed.path == "/api/status":
                result, _ = self.server.native.command("status")
                self.reply(200, result)
            elif parsed.path == "/api/frame":
                revisions = parse_qs(parsed.query).get("revision", [])
                if len(revisions) != 1 or not revisions[0].isdigit() or not 0 < int(revisions[0]) <= 4294967295:
                    self.reply(400, {"error": "Invalid frame revision"})
                    return
                result, pixels = self.server.native.command("frame " + revisions[0], frame=True)
                self.reply(409 if pixels is None else 200, result if pixels is None else pixels, "application/json" if pixels is None else "image/bmp")
            else:
                self.reply(404, {"error": "Not found"})
        except (OSError, ValueError, RuntimeError):
            self.reply(503, {"error": "Native transport unavailable"})

    def do_POST(self):
        try:
            if self.path != "/api/input" or not self.headers.get("Content-Type", "").startswith("application/json"):
                self.reply(400, {"error": "Unsupported request"})
                return
            origin = self.headers.get("Origin")
            if origin != "http://" + self.headers.get("Host", ""):
                self.reply(403, {"error": "Same-origin console input required"})
                return
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 1024:
                self.reply(400, {"error": "Invalid input length"})
                return
            event = json.loads(self.rfile.read(length))
            if not isinstance(event, dict):
                self.reply(400, {"error": "Invalid native input"})
                return
            name, revision = event.get("event"), event.get("revision")
            if not isinstance(name, str) or name not in EVENTS or type(revision) is not int or not 0 <= revision <= 4294967295:
                self.reply(400, {"error": "Invalid native input"})
                return
            if name == "rotate":
                delta = event.get("delta")
                if type(delta) is not int or delta not in (-1, 1):
                    self.reply(400, {"error": "Invalid knob rotation"})
                    return
                line = f"rotate {delta} {revision}"
            else:
                line = f"{name} {revision}"
            result, _ = self.server.native.command(line)
            self.reply(400 if "error" in result else 200, result)
        except (OSError, ValueError, RuntimeError):
            self.reply(503, {"error": "Native transport unavailable"})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=4180)
    options = parser.parse_args()
    native = NativeProcess(options.executable.resolve())
    server = ThreadingHTTPServer((options.host, options.port), Handler)
    server.native = native
    print(f"Native selected Lab: http://{options.host}:{options.port}", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        native.close()


if __name__ == "__main__":
    main()
