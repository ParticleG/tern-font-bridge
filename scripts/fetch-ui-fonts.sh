#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/ui-font-inputs
G=75d647056bcbd9eaba3a14b1b58290d0692685d6
N=ffebf8c1ee449e544955a7e813c54f9b73848eac
curl -fL --retry 2 "https://raw.githubusercontent.com/vercel/geist-font/$G/fonts/Geist/ttf/Geist-Regular.ttf" -o build/ui-font-inputs/Geist-Regular.ttf
for family in NotoSansSymbols NotoSansSymbols2; do
 curl -fL --retry 2 "https://raw.githubusercontent.com/notofonts/noto-fonts/$N/hinted/ttf/$family/$family-Regular.ttf" -o "build/ui-font-inputs/$family-Regular.ttf"
done
# make-ui-font.py validates all three hashes before processing.
