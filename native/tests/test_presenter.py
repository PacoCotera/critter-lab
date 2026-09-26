"""Check authentication and command transport using the real native executable."""
import base64
import importlib.util
import json
from pathlib import Path
import sys
import subprocess
import tempfile
import threading
import unittest
from unittest.mock import patch
from urllib.error import HTTPError
from urllib.request import Request, urlopen

BINARY = str(Path(sys.argv.pop(1)).resolve())
SPEC = importlib.util.spec_from_file_location("presenter", Path(__file__).parents[1] / "presenter/server.py")
PRESENTER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PRESENTER)


class Presenter(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.server = PRESENTER.ThreadingHTTPServer(("127.0.0.1", 0), PRESENTER.Handler)
        self.server.password = "test-only-password"
        self.server.binary = BINARY
        self.server.save = str(Path(self.directory.name) / "state.txt")
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.addCleanup(self.server.server_close)
        self.addCleanup(self.server.shutdown)
        self.origin = f"http://127.0.0.1:{self.server.server_port}"

    def request(self, path, data=None, authenticated=True, headers=None):
        values = dict(headers or {})
        if authenticated:
            values["Authorization"] = "Basic " + base64.b64encode(b"lab:test-only-password").decode()
        request = Request(self.origin + path, data=data, headers=values)
        try:
            response = urlopen(request, timeout=5)
        except HTTPError as error:
            response = error
        with response:
            return response.status, response.read(), response.headers

    def test_all_routes_require_authentication(self):
        for route in ("/", "/app.js", "/style.css", "/api/status", "/api/frame?device=lab&revision=0"):
            self.assertEqual(self.request(route, authenticated=False)[0], 401)
        self.assertEqual(self.request("/api/command", b"{}", authenticated=False)[0], 401)
        self.assertEqual(self.request("/", authenticated=True)[0], 200)

    def test_origin_body_and_native_revision_guards(self):
        body = json.dumps({"name": "select", "revision": 0, "operation_id": "first"}).encode()
        headers = {"Content-Type": "application/json", "X-Requested-With": "CritterLab", "Origin": self.origin}
        self.assertEqual(self.request("/api/command", body)[0], 403)
        wrong = dict(headers, Origin="https://another.example")
        self.assertEqual(self.request("/api/command", body, headers=wrong)[0], 403)
        code, output, _ = self.request("/api/command", body, headers=headers)
        self.assertEqual(code, 200)
        self.assertEqual(json.loads(output)["revision"], 1)
        self.assertEqual(self.request("/api/command", body, headers=headers)[1], output)
        code, frame, _ = self.request("/api/frame?device=lab&revision=1")
        self.assertEqual(code, 200)
        self.assertEqual(frame[:2], b"BM")
        self.assertEqual(self.request("/api/frame?device=lab&revision=0")[0], 409)
        self.assertEqual(self.request("/../../etc/passwd")[0], 404)

    def test_uncertain_native_exit_remains_retryable(self):
        body = json.dumps({"name": "select", "revision": 0, "operation_id": "uncertain"}).encode()
        headers = {"Content-Type": "application/json", "X-Requested-With": "CritterLab", "Origin": self.origin}
        for native_exit, http_status in ((2, 409), (3, 503), (-9, 503)):
            failure = subprocess.CompletedProcess([], native_exit, b'{"error":"test failure"}', b"")
            with patch.object(PRESENTER.subprocess, "run", return_value=failure):
                self.assertEqual(self.request("/api/command", body, headers=headers)[0], http_status)


if __name__ == "__main__":
    unittest.main()
