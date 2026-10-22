#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
umask 077
echo '9ae0c348a44a681fa8fa20a34e782e95316cec880102295a128b65760944c0f5  native/Maple-Regular.ttf' | sha256sum -c -
sha256sum -c deps.sha256
mkdir -p build/classes build/dex build/lib/arm64-v8a
clang -c -fPIC -O2 -fvisibility=hidden -mno-outline-atomics -Wall -Wextra -Werror native/probe.c -o build/probe.o
clang -c -fPIC native/entry.S -o build/entry.o
clang -c -fPIC native/font.S -o build/font.o
ld.lld -shared -z now -z relro -z noexecstack -z max-page-size=16384 --hash-style=both --no-undefined -soname libternprobe.so -o build/lib/arm64-v8a/libternprobe.so "$PREFIX/lib/crtbegin_so.o" build/probe.o build/entry.o build/font.o -L/system/lib64 -llog -ldl -lc "$PREFIX/lib/crtend_so.o"
javac --release 8 -cp deps/xposed-api-82.jar:deps/android-4.1.1.4.jar -d build/classes src/io/github/particleg/ternfontbridge/MainHook.java
jar cf build/classes.jar -C build/classes .
d8 --min-api 28 --lib "$PREFIX/share/aapt/android.jar" --classpath deps/xposed-api-82.jar --output build/dex build/classes.jar
aapt2 compile --dir res -o build/compiled-res.zip
aapt2 link -o build/resources.apk --manifest AndroidManifest.xml -I deps/android-4.1.1.4.jar -A assets build/compiled-res.zip
python - <<'PY'
import zipfile
with zipfile.ZipFile('build/resources.apk') as src, zipfile.ZipFile('build/unsigned.apk','w',zipfile.ZIP_DEFLATED) as dst:
 for i in src.infolist(): dst.writestr(i,src.read(i.filename))
 dst.write('build/dex/classes.dex','classes.dex')
 dst.write('build/lib/arm64-v8a/libternprobe.so','lib/arm64-v8a/libternprobe.so',compress_type=zipfile.ZIP_STORED)
PY
zipalign -f 16384 build/unsigned.apk build/aligned.apk
zipalign -c -v 16384 build/aligned.apk
cp build/aligned.apk build/tern-font-bridge-0.3.0-preview-unsigned.apk
printf '\nUnsigned build complete. Sign separately before installing.\n'
