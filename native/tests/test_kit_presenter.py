"""Actual three-device HTTP/native journey; no parallel browser game rules."""
import importlib.util
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import threading
import time
from urllib.error import HTTPError
from urllib.request import Request, urlopen

spec = importlib.util.spec_from_file_location("presenter", Path(__file__).parents[1] / "presenter/server.py")
presenter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(presenter)


def run(binary, proof=None):
    with tempfile.TemporaryDirectory(prefix="beecho-kit-http-") as directory:
        os.environ["BEECHO_V1_SAVE"] = str(Path(directory) / "world")
        server = presenter.ThreadingHTTPServer(("127.0.0.1", 0), presenter.Handler)
        server.password = ""
        server.release = {"commit": None}
        server.native = presenter.NativeProcess(binary, kit=True)
        threading.Thread(target=server.serve_forever, daemon=True).start()
        origin = f"http://127.0.0.1:{server.server_port}"

        def request(path, data=None, same_origin=True):
            headers = {"Content-Type": "application/json", "X-Requested-With": "CritterLab"}
            if same_origin:
                headers["Origin"] = origin
            body = None if data is None else json.dumps(data).encode()
            try:
                response = urlopen(Request(origin + path, data=body, headers=headers), timeout=10)
            except HTTPError as error:
                response = error
            with response:
                return response.status, response.read()

        def state(device):
            code, body = request(f"/api/devices/{device}/status")
            assert code == 200, (code, body)
            return json.loads(body)

        def frame(device, name=None):
            for _ in range(5):
                current = state(device)
                code, pixels = request(f"/api/devices/{device}/frame?revision={current['revision']}")
                if code == 409:
                    continue
                assert code == 200 and pixels[:2] == b"BM"
                assert struct.unpack_from("<ii", pixels, 18) == (current["width"], current["height"])
                if proof and name:
                    Path(proof).mkdir(parents=True, exist_ok=True)
                    (Path(proof) / f"{name}-{device}.bmp").write_bytes(pixels)
                return current
            raise AssertionError("Frame never became ready")

        def event(device, name, revision):
            code, body = request("/api/device-input", {"device": device, "event": name, "revision": revision})
            assert code == 200, (code, body)
            return json.loads(body)

        def press(device, button):
            current = frame(device)
            revision = current["revision"]
            event(device, "ready", revision)
            event(device, button + "-down", revision)
            return event(device, button + "-up", revision)

        def link(device, online):
            assert request("/api/link", {"device": device, "online": online})[0] == 200

        try:
            for device in ("lab", "companion", "dock"):
                frame(device, "initial")
            assert press("companion", "right")["mode"] == 1
            frame("companion", "mode-cargo")
            assert press("companion", "right")["mode"] == 2
            frame("companion", "mode-companions")
            press("companion", "left")
            press("companion", "left")
            assert request("/api/link", {"device": "companion", "online": False}, False)[0] == 403
            assert request("/api/device-input", {"device": "companion", "event": "research-down", "revision": 1})[0] == 400
            assert request("/api/devices/probe/status")[0] == 400
            assert request("/api/input", {"event": "confirm-down", "revision": 1})[0] == 400
            assert press("companion", "confirm")["page"] == "probe"
            assert state("companion")["cargo"] == [0, 0, 0]
            press("companion", "confirm")
            time.sleep(5.1)
            assert sum(state("companion")["cargo"]) > 0
            frame("companion", "gathering")
            assert state("lab")["stock"] == [0, 0, 0]
            press("companion", "confirm")  # Cargo
            frame("companion", "cargo")
            press("companion", "confirm")  # Review
            frame("companion", "send-review")
            reviewed = state("companion")["cargo"]
            preparation = state("companion")["gather_progress_ms"]
            time.sleep(2.1)
            assert state("companion")["cargo"] == reviewed
            assert state("companion")["gather_progress_ms"] == preparation
            link("companion", False)
            sealed = press("companion", "confirm")
            assert sealed["phase"] == 1
            frame("companion", "sending-offline")
            time.sleep(2.1)
            assert state("companion")["phase"] == 1
            cargo = sealed["cargo"]
            request("/api/status")  # Legacy health must not bypass sealed-cargo ownership.
            assert state("companion")["cargo"] == cargo
            link("companion", True)
            time.sleep(2.1)
            assert state("companion")["phase"] == 2
            assert state("lab")["page"] == "cargo"  # Immediate reception.
            frame("lab", "incoming")
            link("dock", False)
            link("companion", False)
            accepted = press("lab", "confirm")
            credited = [amount // 100 * 100 for amount in cargo]
            retained = [amount % 100 for amount in cargo]
            assert accepted["phase"] == 4 and accepted["stock"] == credited
            for device in ("lab", "companion", "dock"):
                frame(device, "accepted-offline")
            assert state("dock")["dock_stock"] == [0, 0, 0]
            assert press("lab", "confirm")["stock"] == credited
            link("companion", True)
            link("dock", True)
            time.sleep(2.1)
            assert state("companion")["phase"] == 5
            assert state("companion")["cargo"] == retained
            assert state("companion")["gather_progress_ms"] == preparation
            frame("companion", "receipt")
            assert state("companion")["focus"] == "Return to Probe"
            assert state("dock")["dock_stock"] == credited
            press("lab", "back")
            for device in ("lab", "companion", "dock"):
                frame(device, "complete")
            assert press("companion", "confirm")["page"] == "probe"
            assert state("companion")["focus"] == "Continue expedition"
            press("companion", "confirm")
            frame("companion", "continued")
            print("Three-device HTTP/native frame, handoff, link recovery and endpoint guards passed", flush=True)
        finally:
            server.shutdown()
            server.server_close()
            server.native.close()


if __name__ == "__main__":
    run(str(Path(sys.argv[1]).resolve()), sys.argv[2] if len(sys.argv) > 2 else None)
