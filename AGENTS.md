# Repository Guidelines

## Project Overview

Tern Font Bridge is an unofficial LSPosed/Xposed module replacing Tern's bundled BerkeleyMono Regular font source with unmodified Maple Mono Normal NF CN Regular. It targets one exact Tern 0.9.5 arm64 APK, not arbitrary builds sharing that version. It is not a general font API; Medium/bold, true italic, and other ABIs are outside current support.

## Architecture & Data Flow

- `src/io/github/particleg/ternfontbridge/MainHook.java` is the Java bootstrap: require both package and process `so.stencil.tern`, hash the installed APK, then call `System.loadLibrary("ternprobe")`. It installs no Java method hooks.
- `native/probe.c` exposes `native_init` and `JNI_OnLoad`. Both scan already-loaded libraries; `native_init` also returns a callback for later `libtern.so` loads. Keep native validation self-contained: do not assume every native entry passes through the Java hash gate.
- Before mutation, native code validates ELF ranges, getter instructions, vtable contents, and font headers/lengths. It temporarily makes a read-only page writable, atomically replaces one function-pointer slot, and restores protection. This is not inline instruction patching and does not use the framework's hook/unhook functions.
- `native/entry.S` preserves the ARM64 register frame and calls `select_source`. Only the exact original pointer/length pair is replaced; other sources pass through. `native/font.S` embeds `native/Maple-Regular.ttf` in read-only library memory, so returned font bytes require no external font file or copy.

## Key Directories

- `src/io/github/particleg/ternfontbridge/`: stateless Xposed Java bootstrap.
- `native/`: ABI-coupled C/ARM64 assembly and the checked-in font payload.
- `assets/`: loader registrations and font license, all packaged into the APK.
- `res/values/`: LSPosed scope resources, not a settings UI.
- `scripts/`: generated CI version metadata and SDK/NDK build implementation.

## Development Commands

Run from the repository root. On **ARM64 Android/Termux**:

```sh
pkg install clang python openjdk-21 d8 aapt aapt2 apksigner curl
bash fetch-deps.sh
bash build.sh
```

Output: `build/tern-font-bridge-0.3.0-preview-unsigned.apk`. `build.sh` requires Termux's `PREFIX` and Android system libraries; do not use it as a Linux-host build command.

On **Linux x86_64**, with the pinned SDK/NDK installed and `ANDROID_HOME` set:

```sh
GITHUB_OUTPUT=/dev/null python3 scripts/version.py
bash scripts/ci-build.sh
```

The first command generates `build/AndroidManifest.xml`; the second fetches dependencies and produces unsigned `build/aligned.apk`. Signing is separate; follow `README.md` or the GitHub workflow. There is no standalone app/run command: install a signed module and enable it through LSPosed.

Quick shell syntax check, not a test suite or linter:

```sh
bash -n build.sh && bash -n fetch-deps.sh && bash -n scripts/ci-build.sh
```

## Code Conventions & Common Patterns

- Preserve surrounding compact formatting; no formatter is configured. Java uses explicit imports; native helpers use `snake_case`, `static`, and hidden symbols. Keep only required native ABI entry points exported.
- Java catches and logs loading failures through `XposedBridge.log`; native validation failures log and return without patching. Preserve the legacy `TernFontProbe` tag, metadata-only messages, and bounded hot-path diagnostics.
- Native state is file-local atomics plus immutable embedded data, not a DI container or async framework. `installed` is a one-shot **attempt** guard, not proof of successful installation; do not assume failed installation retries. Keep `select_source` allocation-free.
- `native/entry.S`'s 288-byte save frame and `select_source`'s `saved[0]`, `saved[1]`, and `saved[8]` writes form one ABI contract. Preserve register restoration and the `hint #34` landing pad when changing either side.
- Supporting another Tern build requires re-deriving native offsets, instruction/table fingerprints, and font identity, not merely changing the Java APK hash. Preserve package/process and native precondition checks.
- Font changes require synchronized lengths in `native/probe.c`, SHA-256 pins in **both** build scripts and `README.md`, and the OFL notice. Preserve payload provenance; its upstream release number is unconfirmed.
- Both build scripts explicitly enumerate sources. Update both when adding/renaming source files or changing compiler/linker flags; preserve `-mno-outline-atomics`, hidden visibility, uncompressed `.so` packaging, and 16 KB ELF/APK alignment.

## Important Files

- `AndroidManifest.xml`, `res/values/arrays.xml`, `assets/xposed_init`, and `assets/native_init`: module identity, scope, and loader contracts. Keep class/library names synchronized with Java and both build scripts.
- `fetch-deps.sh` and `deps.sha256`: checksum-pinned Xposed API 82 and Android stub JARs; compile-time inputs, not APK contents.
- `scripts/version.py`: reads the root manifest template and generates the CI manifest. Tags must be `vX.Y.Z` or `preX.Y.Z`, with components 0–999 and no leading zeros. Dev builds use versionCode 1; they are not release-upgrade equivalents. Never hand-edit the generated manifest.
- `.github/workflows/build.yml`: pinned toolchain/Actions, signing, certificate verification, artifacts, and releases. Runs on matching tags or manual dispatch, **not** ordinary branch pushes or pull requests. Certificate identity comes from `ANDROID_CERT_SHA256`.
- `README.md` and `CHANGELOG.md`: compatibility, signing, release rules, and validation limits. `LICENSE` covers module source under MIT; `assets/licenses/MapleMono-OFL.txt` covers the separately licensed font and ships in the APK.

## Runtime/Tooling Preferences

- Use Bash, standard-library Python 3 (`python` is also invoked by build scripts), and Java 21 tooling compiling Java 8 bytecode with `--release 8`. There is no Gradle, Maven, CMake, Node.js/Bun, or npm/pnpm workflow.
- CI uses Ubuntu 24.04, Android SDK platform 35, Build Tools **35.0.0**, and NDK **27.2.12479018**. These paths are hardcoded in `scripts/ci-build.sh`; keep workflow installation and script paths synchronized.
- Only `arm64-v8a` is packaged. Manifest minimum API, DEX minimum API, and NDK target are 28; target SDK is 35. Declared minimum API does not establish device compatibility.
- Treat `build/`, `deps/`, and workflow-produced `dist/` as generated outputs. Keep signing keys/passwords outside the repository; never commit secrets, downloaded JARs, or generated APKs.

## Testing & QA

There is no automated test framework, test command, configured linter, coverage tooling, or coverage threshold. Existing gates are dependency/font SHA-256 checks, native `-Wall -Wextra -Werror`, strict linking, APK alignment checks, and CI signature/certificate verification. These do not prove font substitution works.

For runtime changes, smoke-test the exact signed artifact on a compatible LSPosed device:

1. Use the supported APK identified in `src/io/github/particleg/ternfontbridge/MainHook.java`; disable the old `local.tern.diagnostic` module and scope this module only to `so.stencil.tern`.
2. Capture native diagnostics with `adb logcat -s TernFontProbe`, then force-stop and reopen Tern. Check `validation=PASS`, `installed=1 protection_restored=1`, and `MAPLE_REPLACE` messages; Java bootstrap diagnostics are in LSPosed logs.
3. With font settings left at default, verify Chinese rendering and Maple identification. Medium/bold remains unchanged; some symbols may still be missing.
4. Verify rollback by disabling the module and cold-starting Tern; restore default font settings if Maple was manually selected. Do not clear app data.

Document the artifact/device actually tested. Existing prototype/dev validation does not establish that the exact tagged APK, all LSPosed variants, API 28 devices, or long-term stability have been verified.

## 0.3.1 UI symbol extension
- Also substitute the original Geist source at 0xCDBEEE, length 169056, with native/TernUI-Regular.ttf (126104 bytes). This is an OFL derivative, not MIT.
- Exact source matching and mapped-range/header checks cover both font sources. UI_REPLACE is independently bounded; the register-frame ABI is unchanged.
- Input font commits/hashes and licensing are documented in native/UI-FONT-PROVENANCE.md. scripts/fetch-ui-fonts.sh downloads pinned upstream fonts; scripts/make-ui-font.py requires fonttools==4.60.1 and checks input hashes, original outlines/metrics and symbol coverage.
- Local 0.3.1-ui-test was cold-start tested: Command symbols render correctly. Do not generalize that result to every glyph, the CI artifact or long-term stability.
- The UI derivative preserves upstream Geist outlines/metrics, not necessarily those of Tern's differently versioned embedded font.
- Current local unsigned output is build/tern-font-bridge-0.3.1-ui-test-unsigned.apk. Update documentation and both build-script hash gates when changing the UI payload.
