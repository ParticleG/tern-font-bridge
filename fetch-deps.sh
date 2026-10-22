#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p deps
curl -fL --retry 2 https://api.xposed.info/de/robv/android/xposed/api/82/api-82.jar -o deps/xposed-api-82.jar
curl -fL --retry 2 https://repo.maven.apache.org/maven2/com/google/android/android/4.1.1.4/android-4.1.1.4.jar -o deps/android-4.1.1.4.jar
sha256sum -c deps.sha256
