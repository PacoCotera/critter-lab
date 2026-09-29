"""Play the actual native protocol, including real timers and restart recovery."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time


class Player:
    def __init__(self, binary, save, frames):
        self.binary, self.save, self.frames = binary, save, frames
        self.process = subprocess.Popen(
            [binary, "serve"], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            env={**os.environ, "BEECHO_V1_SAVE": str(save)},
        )
        self.state = self.command("status")

    def command(self, text):
        self.process.stdin.write((text + "\n").encode())
        self.process.stdin.flush()
        self.state = json.loads(self.process.stdout.readline())
        assert "error" not in self.state, self.state
        return self.state

    def press(self, button="confirm"):
        for attempt in range(3):
            revision = self.command("status")["revision"]
            state = self.command(f"ready {revision}")
            if state["revision"] == revision:
                self.command(f"{button}-down {revision}")
                return self.command(f"{button}-up {revision}")
        raise AssertionError("Could not get a stable frame")

    def choose(self, label):
        for _ in range(10):
            if self.state["focus"] == label:
                return self.press()
            self.press("down")
        raise AssertionError((label, self.state))

    def capture(self, name):
        revision = self.command("status")["revision"]
        header = self.command(f"frame {revision}")
        content = self.process.stdout.read(header["bytes"])
        assert len(content) == header["bytes"] and content[:2] == b"BM"
        (self.frames / (name + ".bmp")).write_bytes(content)
        self.command("status")

    def wait_until(self, predicate, timeout):
        deadline = time.monotonic() + timeout
        while not predicate(self.command("status")):
            assert time.monotonic() < deadline, self.state
            time.sleep(0.25)

    def close(self):
        self.process.stdin.close()
        assert self.process.wait(timeout=5) == 0


def journey(binary, frames):
    frames.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as directory:
        save = Path(directory) / "world.save"
        player = Player(binary, save, frames)
        player.capture("01-workbench")
        player.choose("Explore")
        player.choose("Field survey")
        player.command(f"suspend {player.state['revision']}")
        before = player.state["expedition_seconds"]
        time.sleep(2.2)
        player.command(f"resume {player.state['revision']}")
        assert player.state["expedition_seconds"] == before
        player.wait_until(lambda state: state["expedition_seconds"] >= 60, 65)
        player.capture("02-expedition")
        player.choose("Cargo")
        player.choose("Discard data pack")
        player.capture("discard-review")
        player.choose("Keep this pack")
        player.choose("Return + store haul")
        assert player.state["samples"] == 1
        player.press()
        for label in ("Crown form", "Eye rings", "Body markings", "Movement", "Energy use"):
            player.choose(label)
            player.capture("review-" + label.replace(" ", "-"))
            player.choose("Start research")
            assert player.state["page"] == "finding", player.state
            player.capture("03-" + label.replace(" ", "-"))
            player.press()
        assert player.state["decoded"] == 31
        player.press("back")
        player.press("back")
        player.choose("Explore")
        player.choose("Garden forage")
        player.wait_until(lambda state: state["expedition_seconds"] >= 60, 65)
        player.choose("Cargo")
        player.choose("Return + store haul")
        player.press()
        player.choose("Prepare incubation")
        player.capture("candidate-selection")
        player.choose("Pale markings")
        assert player.state["page"] == "incubation"
        player.capture("04-incubation")
        player.close()
        player = Player(binary, save, frames)
        player.choose("Incubator")
        player.wait_until(lambda state: state["incubation_ready"], 25)
        player.choose("Open incubation")
        assert player.state["page"] == "reveal"
        player.capture("05-reveal")
        player.press()
        assert player.state["page"] == "habitat"
        player.choose("Spend time together")
        player.capture("06-habitat")
        stock = player.state["stock"]
        player.close()
        player = Player(binary, save, frames)
        assert player.state["individuals"] == 1 and player.state["stock"] == stock
        player.choose("Habitat")
        player.choose("Explore again")
        player.choose("Weather watch")
        player.wait_until(lambda state: state["expedition_seconds"] >= 2, 5)
        player.choose("Cargo")
        player.choose("Return + store haul")
        assert player.state["samples"] == 2, "Early return must not mint a sample"
        player.close()
        print("Complete native journey, real timers, repeat expedition and restart passed", flush=True)


if __name__ == "__main__":
    journey(str(Path(sys.argv[1]).resolve()), Path(sys.argv[2]))
