# Native build scaffold

This builder entry point compiles three minimal native programs. The Linux Lab executable prints its target/version and exits successfully. Probe and Companion contain one boot-log entry point each. Their firmware is cross-compiled for the preliminary targets below; compiling is not evidence of a boot on hardware.

| Program | Compile target | Framework |
| --- | --- | --- |
| Lab | Linux host, C17 | CMake and host GCC |
| Probe | `xiao_ble/nrf52840` (Arm Cortex-M4) | Zephyr 4.4.0, Zephyr GNU SDK 1.0.1 |
| Companion | `esp32s3` (Xtensa) | ESP-IDF 5.5.5 |

These build targets do not select production boards or configure external peripherals. SDK board defaults supply Probe startup, USB CDC ACM console routing and a UF2-compatible application partition; Companion uses the SDK's generic ESP32-S3 defaults. Neither is a product pin map. There is no gameplay, adapter contract, display driver, flashing procedure or device emulator here. The Lab build runs on the development host; it does not verify a Raspberry Pi image or physical Lab.

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
native/build/lab/critter_lab
size native/build/lab/critter_lab
```

Expected output is `Critter Lab native scaffold 0.1.0 | target: Linux host`, exit status 0. The ELF executable and map are in `native/build/lab/`.

### Probe

One-time setup, with `CRITTER_SOURCE`, `CRITTER_TOOLS` and manifest variables from above:

```bash
python3.12 -m venv "$CRITTER_TOOLS/zephyr-venv"
source "$CRITTER_TOOLS/zephyr-venv/bin/activate"
pip install "west==$WEST_VERSION"
west init -m https://github.com/zephyrproject-rtos/zephyr \
  --mr "$ZEPHYR_REVISION" "$CRITTER_TOOLS/zephyr-workspace"
cd "$CRITTER_TOOLS/zephyr-workspace"
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

Successful target builds establish compiler/linker compatibility and the scaffold's static allocations. Lab execution establishes only the Linux process's printed output and exit status. Boot logs for the two MCU targets remain unobserved until run on physical boards. Static size reports do not measure stack/heap peaks, peripheral timing, radio behavior, display refresh, energy or thermal performance. This setup implements no simulator and makes no claim of ESP32-S3 or nRF52840 emulation.

See the [hardware-native development requirement](../docs/builders/foundation-demo.md#hardware-native-development-requirement) and [device constraints](../specs/devices.md). Framework references: [Zephyr SDK setup](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html), [XIAO BLE board](https://docs.zephyrproject.org/latest/boards/seeed/xiao_ble/doc/index.html), and [ESP-IDF ESP32-S3 setup](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/get-started/linux-macos-setup.html).


## Verified baseline

On 26 September 2026, an Ubuntu 26.04 x86-64 development host with isolated Python 3.12 built all three targets. Linux execution returned the documented output and exit status 0. Probe linked with 44,692 bytes flash and 13,496 bytes RAM reported; Companion produced an ESP32-S3 image of 161,360 bytes using the minimal component build. These are startup scaffolds, not application capacity forecasts. SDK sources match the manifest. MCU ELF/binary outputs exist; no MCU boot or panel operation was tested. Hosted CI execution remains a separate check from these local builds.
