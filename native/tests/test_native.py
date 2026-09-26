"""Host integration checks. Pass the built executable as the only argument."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

BINARY = str(Path(sys.argv.pop(1)).resolve())


class NativeLoop(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.save = Path(self.directory.name) / "state.txt"
        self.operation = 0

    def invoke(self, *arguments, success=True):
        result = subprocess.run([BINARY, "--save", str(self.save), *map(str, arguments)],
                                capture_output=True, check=False)
        self.assertEqual(result.returncode == 0, success, result.stdout)
        return result.stdout

    def state(self):
        return json.loads(self.invoke("status"))

    def command(self, name):
        revision = self.state()["revision"]
        self.operation += 1
        return json.loads(self.invoke("command", name, revision, f"op-{self.operation}"))

    def complete(self, inspect=True):
        for action in ("select", "review", "load", "start", "advance"):
            self.command(action)
        self.command("inspect" if inspect else "leave")
        for action in ("advance", "receive", "research", "study_review", "run"):
            self.command(action)

    def test_loop_persists_and_does_not_regrant(self):
        self.complete()
        saved = self.state()
        self.assertTrue(saved["finding"])
        self.assertEqual(saved["reagent"], 1)
        self.assertEqual(saved["unknown_regions"], 2)
        self.assertEqual(saved["event"], 2)
        for action in ("run", "receive", "advance", "start"):
            self.invoke("command", action, saved["revision"], "invalid", success=False)
        self.assertEqual(saved, self.state())
        for action in ("back", "research", "study_review", "finding"):
            self.command(action)
        self.assertTrue(self.state()["finding"])
        self.assertEqual(self.state()["reagent"], 1)

    def test_exact_retry_and_stale_are_distinct(self):
        first = self.invoke("command", "select", 0, "same-id")
        self.assertEqual(first, self.invoke("command", "select", 0, "same-id"))
        self.invoke("command", "review", 0, "same-id", success=False)
        self.invoke("command", "select", 0, "old-id", success=False)
        self.command("review")
        self.invoke("command", "select", 0, "same-id", success=False)
        self.assertEqual(self.state()["revision"], 2)

    def test_leave_is_optional_and_corruption_fails_closed(self):
        self.complete(inspect=False)
        self.assertEqual(self.state()["event"], 3)
        data = self.save.read_bytes() + b"unexpected tail\n"
        self.save.write_bytes(data)
        self.invoke("status", success=False)
        self.invoke("command", "select", 0, "new", success=False)
        self.assertEqual(self.save.read_bytes(), data)

    def test_native_bmp_dimensions_pixels_and_revision(self):
        for device, dimensions in (("lab", (400, 240)), ("probe", (250, 122))):
            frame = self.invoke("frame", device, 0)
            self.assertEqual(frame[:2], b"BM")
            self.assertEqual(struct.unpack_from("<II", frame, 18), dimensions)
            self.assertEqual(struct.unpack_from("<I", frame, 2)[0], len(frame))
            self.assertGreater(len(set(frame[54:])), 1)
        self.command("select")
        self.invoke("frame", "lab", 0, success=False)

    def test_save_failure_is_uncertain_and_same_identity_can_retry(self):
        temporary = Path(str(self.save) + ".tmp")
        temporary.mkdir()
        arguments = [BINARY, "--save", str(self.save), "command", "select", "0", "retry-save"]
        result = subprocess.run(arguments, capture_output=True, check=False)
        self.assertEqual(result.returncode, 3, result.stdout)
        temporary.rmdir()
        result = subprocess.run(arguments, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(json.loads(result.stdout)["revision"], 1)


if __name__ == "__main__":
    unittest.main()
