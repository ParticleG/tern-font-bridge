import os, re
from pathlib import Path
ref = os.environ.get("GITHUB_REF", "")
tag = os.environ.get("GITHUB_REF_NAME", "")
release = ref.startswith("refs/tags/")
if release:
    m = re.fullmatch(r"(v|pre)(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)", tag)
    if not m:
        raise SystemExit("Unsupported tag: expected vX.Y.Z or preX.Y.Z")
    prefix, *parts = m.groups()
    major, minor, patch = map(int, parts)
    if major > 999 or minor > 999 or patch > 999:
        raise SystemExit("Each version component must be <= 999")
    pre = prefix == "pre"
    version = f"{major}.{minor}.{patch}" + ("-pre" if pre else "")
    code = (major * 1000000 + minor * 1000 + patch) * 2 + (1 if pre else 2)
else:
    pre = True
    version = "0.3.0-dev." + os.environ.get("GITHUB_RUN_NUMBER", "0")
    code = 1
if not 1 <= code <= 2100000000:
    raise SystemExit("versionCode out of Android range")
Path("build").mkdir(exist_ok=True)
s = Path("AndroidManifest.xml").read_text()
s = re.sub(r'android:versionCode="[^"]+"', f'android:versionCode="{code}"', s)
s = re.sub(r'android:versionName="[^"]+"', f'android:versionName="{version}"', s)
Path("build/AndroidManifest.xml").write_text(s)
with open(os.environ["GITHUB_OUTPUT"], "a") as f:
    f.write(f"version={version}\ncode={code}\nrelease={str(release).lower()}\nprerelease={str(pre).lower()}\n")
print(f"Version {version}, code {code}, release={release}, prerelease={pre}")
