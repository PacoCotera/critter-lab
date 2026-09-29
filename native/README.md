# Native builds and Lab simulation

[CI release delivery](UPDATER.md) defines artifact provenance and the staging service boundary.

The playable Lab target is `selected_lab`: native C17 rules, durable local records
and a scanline renderer, displayed through the browser transport. See its
[current contract](selected-lab/README.md) and [playable loop](selected-lab/V1.md).
The earlier `critter_lab` fixture and portable MCU scaffolds remain separate build
targets; compiling them does not establish the complete game on those devices.

| Program | Current compiler target | Toolchain / evidence |
| --- | --- | --- |
| Lab | Linux x86-64 host; Raspberry Pi4 Model B is the selected device reference | GCC + CMake + Ninja, C17. Not ESP-IDF. ARM build, physical display/input and Pi performance unverified. |
| Legacy Probe scaffold | `xiao_ble/nrf52840` (Arm Cortex-M4) | Zephyr4.4.0 + Zephyr GNU SDK1.0.1; no panel driver or physical boot evidence. |
| Companion scaffold | `esp32s3` (Xtensa) | ESP-IDF5.5.5; separate from Lab; no panel driver or physical boot evidence. |

Current product roles and selected electronics are in [devices](../specs/devices.md).
Legacy portable build targets do not freeze the consolidated kit's final electronics.
The simulator panel represents the Lab directional cross, four workspace keys,
Back and Confirm. It does not emulate a Pi4 CPU or GPIO.

## Ubuntu build environment

Use Ubuntu 24.04 x86-64 with Python 3.12. SDK sources and release versions are recorded in [toolchains.env](toolchains.env). The workflow uses the same commands and uploads compiled binaries, linker maps and size reports. Ubuntu packages and transitive Python dependencies are not fully locked, so this is a repeatable release-pinned setup, not a byte-identical toolchain archive. CI retains the resolved Python package lists.

From the repository root, use Bash. Keep the SDK workspace outside the repository. Run Probe and Companion setup in separate shells to avoid mixing their Python environments.

```bash
sudo apt-get update
sudo apt-get install -y --no-install-recommends git build-essential cmake ninja-build \
  python3.12-venv python3.12-dev gperf device-tree-compiler xz-utils \
  libffi-dev libssl-dev dfu-util
export CRITTER_SOURCE="$PWD"
export CRITTER_TOOLS="$HOME/critter-native-tools"
mkdir -p "$CRITTER_TOOLS"
source native/toolchains.env
```

### Lab

```bash
cmake -S native/lab -B native/build/lab -G Ninja -DCMAKE_BUILD_TYPE=Release -DCRITTER_BUILD_SELECTED_LAB=ON
cmake --build native/build/lab
native/build/lab/critter_lab --save /tmp/critter-demo-state.txt status
python3 native/tests/test_native.py native/build/lab/critter_lab
python3 native/tests/test_selected_presenter.py native/build/lab/selected-lab/selected_lab
size native/build/lab/critter_lab
```

Expected initial status has revision 0 and Select available. The ELF executable and map are in `native/build/lab/`.

### Staging bundle

The Lab CI job packages the already-tested Linux x86-64 executable and the presenter
`server.py`, HTML, CSS and JavaScript from the same checkout. It also includes a
generated `release.json` and a concise `MANIFEST.txt` containing the full commit,
platform, byte sizes and SHA-256 hashes. The archive is deterministic for a given
checkout and executable: file order, ownership, permissions and timestamps are
normalized, with the commit timestamp used for its payload entries. Source metadata
records that instant as `committed_at`; the bundled `deployed_at` is always null, so
an undeployed staging input never claims an activation.

Create and independently verify the bundle from the repository root:

```bash
python3 native/package_staging.py create \
  --binary native/build/lab/selected-lab/selected_lab \
  --output native/build/lab/critter-lab-staging.tar.gz
python3 native/package_staging.py verify \
  native/build/lab/critter-lab-staging.tar.gz \
  --commit "$(git rev-parse HEAD)"
```

Creation refuses to overwrite an existing archive, including a file created during
publication, checks `GITHUB_SHA` when present, and refuses tracked presenter inputs
whose index or worktree differs from HEAD. Presenter bytes are read from HEAD's Git
objects. Verification fails closed for unexpected or unsafe
members, links, duplicate paths, non-normalized metadata, revision disagreement,
or content that does not match the manifest. The bundle is only a staging input;
it does not install, publish, run a service or provide remote access. Runtime state
is deliberately excluded: no save, lock, log, credential, environment file or
other mutable presenter data is packaged. Legacy domain callers keep `CRITTER_DEMO_SAVE` outside the
unpacked bundle when starting the presenter.

After verification and extraction, an operator may create activation metadata
before starting the presenter by atomically replacing the adjacent
`presenter/release.json`: it preserves `commit`, `subject`, and `committed_at` and
sets `deployed_at` to the actual activation instant as a timezone-aware RFC 3339
value (including the applicable `America/Mexico_City` offset). Activation is
deliberately outside this bundle creator and does not mutate the
archive, install or update software, or touch save state. Until then, the existing
footer continues to say release metadata is unavailable; once supplied, it formats
the actual activation time for Mexico City.

### Play through the browser presenter

The public presenter serves the [selected native Lab preview](selected-lab/README.md). Knob, Confirm and Back drive C-owned focus and frames. Start is a preview; resource stock remains unchanged. Research execution and persistence are not connected. The previous legacy CLI remains available for domain experiments, but is not the public screen.

The standard-library bridge retains optional HTTP Basic authentication (`CRITTER_DEMO_PASSWORD`, username `lab`), same-origin input validation and loopback binding. Use HTTPS for public access. The selected process is shared by visitors and holds only transient preview state. The existing `CRITTER_DEMO_SAVE` file is unused and untouched.

```bash
export CRITTER_DEMO_BINARY="$PWD/native/build/lab/selected-lab/selected_lab"
python3 native/presenter/server.py
```

The existing CI staging bundle packages this executable in its stable `bin/critter_lab` slot. Release metadata, service environment, package member names and deployment health endpoints remain compatible. No new deployment service is needed. Native host execution does not prove physical-panel behavior or flashed firmware.

### Legacy domain CLI

`native/build/lab/critter_lab --save /absolute/path status` retains the earlier saved expedition/research fixture. Its `command NAME EXPECTED_REVISION OPERATION_ID` and `frame lab|probe REVISION` interfaces remain available for domain tests. The selected screen does not yet connect to that saved game; do not treat the two executables as interchangeable protocols.

### Probe

One-time setup, with `CRITTER_SOURCE`, `CRITTER_TOOLS` and manifest variables from above:

```bash
python3.12 -m venv "$CRITTER_TOOLS/zephyr-venv"
source "$CRITTER_TOOLS/zephyr-venv/bin/activate"
pip install "west==$WEST_VERSION"
west init -m https://github.com/zephyrproject-rtos/zephyr \
  --mr "$ZEPHYR_TAG" "$CRITTER_TOOLS/zephyr-workspace"
cd "$CRITTER_TOOLS/zephyr-workspace"
test "$(git -C zephyr rev-parse HEAD)" = "$ZEPHYR_REVISION"
west update hal_nordic cmsis cmsis_6
west zephyr-export
pip install -r zephyr/scripts/requirements-base.txt
west sdk install --version "$ZEPHYR_SDK_VERSION" \
  --gnu-toolchains arm-zephyr-eabi --no-hosttools
cd "$CRITTER_SOURCE"
```

Build in an activated Zephyr shell:

```bash
source native/toolchains.env
source "$CRITTER_TOOLS/zephyr-venv/bin/activate"
export ZEPHYR_BASE="$CRITTER_TOOLS/zephyr-workspace/zephyr"
export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-$ZEPHYR_SDK_VERSION"
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
west build -b "$PROBE_BOARD" native/probe -d native/build/probe
"$ZEPHYR_SDK_INSTALL_DIR/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-size" \
  native/build/probe/zephyr/zephyr.elf
```

ELF, binary and map are in `native/build/probe/zephyr/`. Only the modules used by this minimal board build are fetched. Future subsystem additions must extend that set deliberately. The SDK installer verifies downloaded release archives against the publisher's checksums; it installs only the Arm compiler, without emulation/debugging host tools.

### Companion

In a fresh shell, restore the root directory, `CRITTER_SOURCE`, `CRITTER_TOOLS` and manifest variables. The SDK installer chooses tool versions from its pinned source manifest and verifies their hashes.

```bash
git clone --branch "$IDF_TAG" --depth 1 --recursive --shallow-submodules \
  https://github.com/espressif/esp-idf.git "$CRITTER_TOOLS/esp-idf"
test "$(git -C "$CRITTER_TOOLS/esp-idf" rev-parse HEAD)" = "$IDF_REVISION"
python3.12 "$CRITTER_TOOLS/esp-idf/tools/idf_tools.py" install --targets="$COMPANION_TARGET"
python3.12 "$CRITTER_TOOLS/esp-idf/tools/idf_tools.py" install-python-env
source "$CRITTER_TOOLS/esp-idf/export.sh"
idf.py -C native/companion -B "$CRITTER_SOURCE/native/build/companion" set-target "$COMPANION_TARGET"
idf.py -C native/companion -B "$CRITTER_SOURCE/native/build/companion" build
idf.py -C native/companion -B "$CRITTER_SOURCE/native/build/companion" size
```

The application ELF/bin/map, bootloader and partition binary are under `native/build/companion/`. `sdkconfig` is generated locally and ignored; no product partition or flash-capacity contract is established.

## What the evidence means

Successful target builds establish compiler/linker compatibility and the scaffold's static allocations. The Linux checks exercise the fixture's transitions, saved state, retry handling, native frames and authenticated HTTP boundary. Boot logs for the two MCU targets remain unobserved until run on physical boards. Static size reports do not measure stack/heap peaks, peripheral timing, radio behavior, display refresh, energy or thermal performance. This setup implements no simulator and makes no claim of ESP32-S3 or nRF52840 emulation.

See the [hardware-native development requirement](../docs/builders/foundation-demo.md#execution-and-module-boundaries) and [device constraints](../specs/devices.md). Framework references: [Zephyr SDK setup](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html), [XIAO BLE board](https://docs.zephyrproject.org/latest/boards/seeed/xiao_ble/doc/index.html), and [ESP-IDF ESP32-S3 setup](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/get-started/linux-macos-setup.html).


## Verified prototype

On 26 September 2026, an Ubuntu 26.04 x86-64 host with isolated Python 3.12 built all three targets. Five native checks and five HTTP checks passed. A browser completed the expedition, received the haul, ran Structure study and reloaded the retained finding through an authenticated HTTPS reverse proxy.

Probe linked with 105,228 bytes flash and 13,496 bytes RAM reported; Companion produced a 222,736-byte ESP32-S3 image. Both entry points execute a domain review transition and render a row. SDK sources match the manifest. These numbers are not application-capacity forecasts or peak-memory measurements. No physical MCU boot or panel operation was tested. Hosted CI remains separate from these host results.

## Native display contract

Profiles declare native dimensions, encoding and row size: Lab RGB888 3,072 bytes; Probe packed 1-bpp 16 bytes; Companion RGB888 1,104 bytes. The Linux BMP adapter expands monochrome only for presentation. MCU entry points use their own profiles. Row bounds, capacity and padding are checked by `native/tests/test_pixels.c`, compiled and run in CI.

The revised sample/finding views use conceptual research regions, not a production genome bitmap. Older saved page values remain usable. The browser fits each complete native raster to the available view without changing its aspect ratio. It does not require panning inside a device screen. Selected device is remembered; Companion shows an honest empty state. Physical drivers/readiness and final interaction acceptance remain separate.


Color Lab scenes use separately rasterized Bitstream Vera glyphs and a Lab-only scene/font module. Portable builds retain their own smaller atlas and profile-specific scenes. [Font provenance and regeneration](shared/fonts/README.md) records the license and source hash. These UI assets add read-only flash; row RAM remains bounded by the display profiles.

The presenter accepts an optional deployment-injected `release.json` beside `server.py`: `commit` (full lowercase SHA), `subject` (optional single-line commit title), and `deployed_at` (ISO 8601 with timezone). It snapshots validated metadata at startup and exposes only those fields through `/api/release`. The footer shows the first seven SHA characters and release time in `America/Mexico_City` as plain text. Missing metadata is shown as a development build. Never put credentials or private deployment details in this file.

The expedition's simulated-time action is always visible when available, below the device housing. Native screen action labels refer to the housing controls; they are not touchscreen hit regions. Transport requests time out after 15 seconds and retain the same pending operation for explicit retry. If browser storage is denied, retries remain available in memory while the page stays open; a warning explains that limitation. Full-frame fit preserves all pixels but does not establish comfortable phone-size readability.
