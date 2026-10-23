# Changelog

## 0.3.2-pre

- Add all five star and ten braille spinner characters found in the supported Tern APK to the UI font, using the same pinned OFL Noto donor.
- Keep Maple Regular and the guarded source-selection logic unchanged.
- Another user reports that the local signed 0.3.2-spinner-test APK renders the Thinking star animation continuously without tofu or visible jumping.
- Cloud pre0.3.2 artifact, other spinner styles/devices and long-term stability are not yet verified.
- Cloud pre0.3.1 keyboard-symbol fix has now also been confirmed by the original tester.


## 0.3.1-pre

- Add a pointer/length-guarded Geist source replacement with Tern UI Symbols (OFL derivative).
- Add Command, Control, Option, Escape, Backspace, Return and Delete glyphs from pinned Noto sources; keep Maple Regular substitution.
- Include font provenance, input/output hashes, generation tooling and both upstream licenses.
- Local signed UI test APK verified after cold start: settings now shows `⌘= / ⌘−`; logs confirm both replacements and restored memory protection.
- Cloud prerelease artifact still requires its own smoke test. Other symbols, UI layouts and long-term stability are not fully verified.


## 0.3.0

### Features
- Replace the bundled BerkeleyMono Regular source with unmodified Maple Mono Normal NF CN Regular.
- Default font automatically identifies as Maple in the validated prototype, with Chinese rendering working after a cold start.
- New package: `io.github.particleg.ternfontbridge`.
- Strict APK SHA-256, mapped-range, instruction and vtable validation for the supported Tern 0.9.5 arm64 build.

### Build and distribution
- GitHub Actions builds with Android SDK/NDK, signs with the dedicated release key and verifies its certificate fingerprint.
- Native library is uncompressed and 16 KB aligned.
- `vX.Y.Z` tags publish releases; `preX.Y.Z` tags publish prereleases, with APK and SHA256SUMS assets.
- Include the Maple OFL notice; module source is MIT licensed.

### Validation and limitations
- The Actions-built, release-key-signed dev APK was tested on-device with the old module disabled: Chinese renders after a cold start.
- This release preserves that native implementation; the exact tagged APK still needs a post-release smoke test.
- Only the supported APK hash and arm64 are supported. Medium/bold is not replaced; true italic is not bundled; some UI symbols may remain missing.
- Long-term stability and all font styles have not been verified. Disable the module and cold-start Tern to revert.

## 0.3.0-preview (source preparation)

- New identity: Tern Font Bridge / io.github.particleg.ternfontbridge.
- Preserve the working prototype 0.3 Regular-only font substitution.
- Include the unmodified Maple Regular payload and its OFL notice.
- Separate unsigned builds from private release signing.
- At the preview preparation stage, the new package had not yet been runtime-tested.
