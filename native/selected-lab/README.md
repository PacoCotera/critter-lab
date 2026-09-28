# Native selected Lab screen proof

One editable native-C1024×600 screen follows the [approved Lab baseline](../../design/game-art-proposals/35-vault-composition/18-c-refined.png). Native C owns all pixels, page/focus state, input authorization and frame revision. The browser transports the existing knob/Confirm/Back inputs and displays a native BMP; it does not compose the screen or extend game rules.

This fixture retains the known Crown, Eye-ring and pale inheritance alongside unknown adult/mild appearance. Start opens an explicitly labeled native preview, with no study submission or resource spending. Free reference inspection and Back restore caller focus. Stock remains2/1/0. Genetics, persistence, incubation and replacement of the legacy Lab domain are outside this bounded proof.

## Build with the existing project VM

Develop and run available checks locally, commit and push, then retrieve the revision through Git on the established build host. Build only a clean checkout at the exact pushed commit. Do not transfer loose source or archives to bypass version control. Record `git rev-parse HEAD` with build logs and artifact hashes. Use the installed GCC/CMake; no additional toolchain is required. The optional selected target is part of the existing Lab build:

```sh
cmake -S native/lab -B native/build/lab -DCRITTER_BUILD_SELECTED_LAB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build native/build/lab --target selected_lab selected_lab_checks
ctest --test-dir native/build/lab -R selected_lab_native_checks --output-on-failure
native/build/lab/selected-lab/selected_lab frame native/build/lab/selected-lab.bmp
```

The option defaults off; the existing `critter_lab` executable/model remain intact. The existing Native target builds workflow enables the target, runs CTest and uploads its executable, frame, source revision and SHA-256 hashes with the Lab artifacts. It does not deploy this preview. Earlier local frame experiments predate the versioned build gate and are not release evidence. This target establishes Linux host rendering, not MCU firmware or physical-panel validation.

## Assets and editable rendering

`render.c` emits one RGB888 scanline at a time and writes a24-bit BMP. Frames, text, layout and focus markers are editable C geometry. `native_font.c` and `lab_font_data.c` are reused from `../shared`; the existing licensed Vera typography is an honest approximation of the approved concept's condensed lettering, not a new font or final visual approval.

`assets.c` contains exact decoded RGB pixels from the six [37 extraction-kit crops](../../design/game-art-proposals/37-lab-extracted-kit/README.md), plus the baseline's frosted unknown motif rectangle x548,y249,w185,h119. [Asset manifest](asset-manifest.json) records source hashes, dimensions and decoded RGB hashes. No full screenshot is used as the UI, and the baked START reference is not used as a button. Opaque backgrounds and compression artifacts remain; these are source-preview crops, not native production masters. Native display scaling uses nearest source-pixel sampling.

`convert-assets.cjs` regenerates the pixel arrays using Node and an already available Sharp module (`node native/selected-lab/convert-assets.cjs`; shared installations may use `NODE_PATH`). No image generation, interpolation, alpha removal or source edit occurs.

## Input and protocol

`input.c` owns the three study targets and same-workbench preview/reference destinations. Rotation changes one focus. Confirm down captures its displayed frame identity; only a fresh matching ready-frame release activates. Blocked holds remain consumed through release, repeats cannot rearm them, and redraw invalidates an armed press. Pointer cancellation clears gestures. Suspend/blur clears gestures and readiness; resume requires a fresh displayed frame. Safe Back can request caller restoration before readiness. Stale frame-ready acknowledgements are ignored.

Several rotations during redraw update the one C-owned pending focus, without queueing intermediate targets. Rotations must identify a revision from the current page visit, within the current revision; an older page visit or pre-suspend/pre-resume revision is ignored. Only the latest pending revision can become ready. A lost input/frame response invalidates queued browser sends, clears local pointer/dial holds and attempts native cancellation. Activation remains blocked until an explicit page reload, even when cancellation cannot be confirmed; reconnect resumes with native gesture clearing and a fresh displayed frame.

The executable's `serve` mode consumes one line per command and replies with one JSON status line:

```text
status
rotate 1 REVISION
rotate -1 REVISION
confirm-down REVISION
confirm-up REVISION
back-down REVISION
back-up REVISION
cancel REVISION
suspend REVISION
resume REVISION
ready REVISION
frame REVISION
```

`frame` rejects a stale revision. A successful frame reply gives the exact byte count in a JSON line followed by that many binary BMP bytes. No game or pixel decisions are delegated to Python/JavaScript.

## Optional review transport

The small existing-presenter-style adapter owns one persistent C process and serializes its commands/frames. Run it in the same review session; it has no daemon/restart management or additional dependency:

```sh
python3 native/selected-lab/presenter.py --executable build/lab/selected-lab/selected_lab --port 4180
```

It defaults to localhost. Open its printed URL through the established VM access route. The browser's `bridge.mjs` maps physical knob drag/scroll and button down/up/cancel to native inputs, decodes native BMP frames and acknowledges the matching displayed revision. It introduces no screen touch targets, keyboard shortcuts, knob press or extra device buttons. Browser paint acknowledgement does not establish actual panel visibility.

## Focused evidence

Native CTest passed held-input/readiness, stale-frame, safe Back, cancellation/suspend and caller-focus checks, plus source-pixel/BMP interface checks. Actual browser transport exercise and independent review are the coordinator's integration boundary. No hardware performance, player comprehension or final typography claim is made.

Release test compilation explicitly undefines `NDEBUG`; a compile-time guard rejects a checks target with assertions disabled. The native checks cover rejected prior-page/future/pre-wake rotations, multiple current-page rotations and stale/latest readiness acknowledgements. Build evidence for each revision belongs to the committed-revision workflow above.

Run `node --test native/selected-lab/bridge.test.mjs` for the focused in-process transport regression. It simulates a native down whose response is lost, verifies cancellation and discarded queued release, and proves subsequent input stays blocked until reload. This local check passed; it uses a controlled DOM/fetch mock and does not replace actual native/browser evidence.
