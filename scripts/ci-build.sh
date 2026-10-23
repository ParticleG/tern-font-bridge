#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${ANDROID_HOME:?}"
BT="$ANDROID_HOME/build-tools/35.0.0"
NDK="$ANDROID_HOME/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64"
CC="$NDK/bin/aarch64-linux-android28-clang"
ANDROID_JAR="$ANDROID_HOME/platforms/android-35/android.jar"
bash fetch-deps.sh
echo '9ae0c348a44a681fa8fa20a34e782e95316cec880102295a128b65760944c0f5  native/Maple-Regular.ttf' | sha256sum -c -
echo 'a683ad8e08732f96e909dbe1aaee309a42ab9aa56faaeb53538310a07b055a94  native/TernUI-Regular.ttf' | sha256sum -c -
mkdir -p build/classes build/dex build/lib/arm64-v8a
"$CC" -shared -fPIC -O2 -fvisibility=hidden -mno-outline-atomics -Wall -Wextra -Werror -I"$NDK/sysroot/usr/include" -Wl,-z,max-page-size=16384 -Wl,-z,now -Wl,-z,relro -Wl,--no-undefined -Wl,-soname,libternprobe.so native/probe.c native/entry.S native/font.S -llog -ldl -o build/lib/arm64-v8a/libternprobe.so
javac --release 8 -cp "deps/xposed-api-82.jar:$ANDROID_JAR" -d build/classes src/io/github/particleg/ternfontbridge/MainHook.java
jar cf build/classes.jar -C build/classes .
"$BT/d8" --min-api 28 --lib "$ANDROID_JAR" --classpath deps/xposed-api-82.jar --output build/dex build/classes.jar
"$BT/aapt2" compile --dir res -o build/compiled-res.zip
"$BT/aapt2" link -o build/resources.apk --manifest build/AndroidManifest.xml -I "$ANDROID_JAR" -A assets build/compiled-res.zip
python - <<'PACK'
import zipfile
with zipfile.ZipFile('build/resources.apk') as src, zipfile.ZipFile('build/unsigned.apk','w',zipfile.ZIP_DEFLATED) as dst:
 for i in src.infolist(): dst.writestr(i,src.read(i.filename))
 dst.write('build/dex/classes.dex','classes.dex')
 dst.write('build/lib/arm64-v8a/libternprobe.so','lib/arm64-v8a/libternprobe.so',compress_type=zipfile.ZIP_STORED)
PACK
"$BT/zipalign" -f -P 16 4 build/unsigned.apk build/aligned.apk
"$BT/zipalign" -c -P 16 -v 4 build/aligned.apk
