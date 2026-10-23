# Tern UI Symbols (experimental derivative)

Source: Geist Regular, vercel/geist-font commit 75d647056bcbd9eaba3a14b1b58290d0692685d6, fonts/Geist/ttf/Geist-Regular.ttf.
Donors: notofonts/noto-fonts commit ffebf8c1ee449e544955a7e813c54f9b73848eac, hinted/ttf/NotoSansSymbols/NotoSansSymbols-Regular.ttf and hinted/ttf/NotoSansSymbols2/NotoSansSymbols2-Regular.ttf.
All input files downloaded from raw.githubusercontent.com at those commits; no system or Tern-extracted font is redistributed.
Licenses: assets/licenses/Geist-OFL.txt and Noto-OFL.txt. Derivative remains SIL OFL 1.1, not MIT. Renamed Tern UI Symbols.
Build: fonttools==4.60.1; run scripts/make-ui-font.py with inputs in build/ui-font-inputs.
Added glyphs: U+2318 U+2303 U+2325 U+238B U+232B U+23CE U+2326; existing Shift/Tab retained.
All original upstream Geist outlines and horizontal metrics are checked unchanged. This upstream Geist is NOT byte-identical to Tern's embedded version, so UI differences remain possible.
Output length 128516; SHA256 a683ad8e08732f96e909dbe1aaee309a42ab9aa56faaeb53538310a07b055a94.

Input SHA256:
- NotoSansSymbols-Regular.ttf: 8f02f31959bbdf6061547a188248e13f84dc5fdd940326ec494675f453f072bb
- Geist-Regular.ttf: 85a1c6b18a6b0a06dfe9fd4f6d6a5d4979f74ec861eaef4bc7868b5492b8a117
- NotoSansSymbols2-Regular.ttf: 630846d528dbe4c4981370a4d0a9475a1fd1491a129bb411f8e157cdb5de13c6

0.3.2 experiment: additionally includes U+2722/U+2733/U+2736/U+273B/U+273D and U+280B/U+2819/U+2839/U+2838/U+283C/U+2834/U+2826/U+2827/U+2807/U+280F from the same pinned Noto Sans Symbols 2 donor. Local signed 0.3.2-spinner-test: another user reported continuous Thinking star animation without tofu/jumping. Cloud prerelease and other styles remain unverified.
