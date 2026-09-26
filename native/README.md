# Native first-expedition fixture

This builder entry point compiles three native programs sharing a bounded C domain and scanline renderer. The Linux Lab executable runs the saved first-expedition fixture and emits native BMP frames. Probe and Companion link the same portable code, apply and validate one selection transition and render one checksum row at boot; neither has a panel driver. Cross-compilation is not evidence of booting hardware.

| Program | Compile target | Framework |
| --- | --- | --- |
| Lab | Linux host, C17 | CMake and host GCC |
| Probe | `xiao_ble/nrf52840` (Arm Cortex-M4) | Zephyr 4.4.0, Zephyr GNU SDK 1.0.1 |
| Companion | `esp32s3` (Xtensa) | ESP-IDF 5.5.5 |

The selected development profiles are Lab 1024x600 color, Probe candidate 122x250 portrait monochrome and Companion 368x448 color. Native builds do not configure external peripherals or prove a physical boot. SDK board defaults are not a product pin map.

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
cmake -S native/lab -B native/build/lab -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build native/build/lab
native/build/lab/critter_lab --save /tmp/critter-demo-state.txt status
python3 native/tests/test_native.py native/build/lab/critter_lab
python3 native/tests/test_presenter.py native/build/lab/critter_lab
size native/build/lab/critter_lab
```

Expected initial status has revision 0 and Select available. The ELF executable and map are in `native/build/lab/`.

### Play through the browser presenter

The Python standard-library bridge permits anonymous shared staging access when `CRITTER_DEMO_PASSWORD` is absent or empty. Anyone with access uses the same saved playground. An optional nonempty password enables HTTP Basic authentication on every route, username `lab`; keep it in the process environment, never in a URL or commit. Same-origin command and body validation apply in both modes. Use HTTPS when exposing the presenter beyond a trusted local connection. Bind defaults to loopback; a local tunnel can publish that listener. `CRITTER_DEMO_BIND=0.0.0.0` is an explicit LAN option. Port defaults to 4180.

```bash
export CRITTER_DEMO_BINARY="$PWD/native/build/lab/critter_lab"
export CRITTER_DEMO_SAVE="$HOME/critter-demo-state.txt"
# Optional: set CRITTER_DEMO_PASSWORD to require HTTP Basic authentication.
python3 native/presenter/server.py
```

Open the presenter: Expedition details, Load probe, switch to Probe and Start. Simulated expedition supply two progress steps, with an optional encounter between them. Return to Lab, Bring to lab, Review study and Start study. The saved finding uses one unit of lab supplies and leaves other regions unknown. View finding revisits without spending. These are authored fixture quantities, not approved economy or complete genetics.

Reset sandbox is a simulator control outside the device. Its inline confirmation explains that resetting clears the shared playground for everyone. Confirmation returns to the initial outing and selects Lab. Reset uses the same locked, atomic saved-state command path and retry identity as other commands, while keeping revisions increasing; it is not an MCU/domain action or a physical device capability.

The browser displays native pixel images and action descriptors; it implements no collection, research or pixel rules. Frames must load before controls activate. Pending requests retain their operation ID for exact retry after uncertainty, including page reload. This is one shared, single-owner playground and one snapshot, not distributed transfer or cloud authority. Receiving atomically moves the fixture haul into the Lab snapshot; there is no radio acknowledgement to infer.

The native interface is `--save /absolute/path status`, `--save /absolute/path command NAME EXPECTED_REVISION OPERATION_ID`, or `--save /absolute/path frame lab|probe REVISION`. Each command holds a file lock, validates phase and revision, and replaces a versioned explicit-field save using fsync and rename. The most recent operation ID and payload permit an exact retry; older revisions reject. Corrupt state fails closed without resetting. To start another manual demo, stop the presenter and archive its save under a new name before restarting; no browser reset silently destroys shared progress.

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
