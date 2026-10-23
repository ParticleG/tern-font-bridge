"""Build OFL derivative from pinned upstream inputs in build/ui-font-inputs.
Requires fonttools==4.60.1. Does not use fonts extracted from Tern.
"""
from pathlib import Path
import hashlib
from fontTools.ttLib import TTFont
from fontTools.pens.ttGlyphPen import TTGlyphPen
p=Path('build/ui-font-inputs')
pins={
 'Geist-Regular.ttf':'85a1c6b18a6b0a06dfe9fd4f6d6a5d4979f74ec861eaef4bc7868b5492b8a117',
 'NotoSansSymbols-Regular.ttf':'8f02f31959bbdf6061547a188248e13f84dc5fdd940326ec494675f453f072bb',
 'NotoSansSymbols2-Regular.ttf':'630846d528dbe4c4981370a4d0a9475a1fd1491a129bb411f8e157cdb5de13c6'}
for name,digest in pins.items():
 if hashlib.sha256((p/name).read_bytes()).hexdigest()!=digest:raise ValueError('Input hash mismatch: '+name)
f=TTFont(p/'Geist-Regular.ttf',recalcTimestamp=False)
original=TTFont(p/'Geist-Regular.ttf',recalcTimestamp=False)
donors=[TTFont(p/n) for n in ['NotoSansSymbols-Regular.ttf','NotoSansSymbols2-Regular.ttf']]
chars='⌘⌃⌥⇧⎋⌫⏎⌦⇥'
order=list(f.getGlyphOrder());added=[]
for ch in chars:
 cp=ord(ch)
 if cp in f.getBestCmap():continue
 donor=next(d for d in donors if cp in d.getBestCmap())
 assert donor['head'].unitsPerEm==f['head'].unitsPerEm
 name=donor.getBestCmap()[cp];new=f'tfb{cp:04X}'
 # Decompose outlines, without copying donor hint programs or composite references.
 from fontTools.pens.recordingPen import DecomposingRecordingPen
 gs=donor.getGlyphSet();rec=DecomposingRecordingPen(gs);gs[name].draw(rec)
 pen=TTGlyphPen(None);rec.replay(pen);f['glyf'][new]=pen.glyph()
 f['hmtx'][new]=donor['hmtx'][name];order.append(new)
 for t in f['cmap'].tables:
  if t.isUnicode() and t.format in (4,12):t.cmap[cp]=new
 added.append(ch)
f.setGlyphOrder(order)
for nid,text in [(1,'Tern UI Symbols'),(2,'Regular'),(3,'Tern UI Symbols 0.3.1'),(4,'Tern UI Symbols Regular'),(6,'TernUISymbols-Regular'),(16,'Tern UI Symbols'),(17,'Regular')]:
 f['name'].removeNames(nameID=nid)
 f['name'].setName(text,nid,3,1,0x409)
f['name'].setName('Derived from Geist and Noto Sans Symbols / Symbols 2; SIL OFL 1.1.',10,3,1,0x409)
f.save('native/TernUI-Regular.ttf')
g=TTFont('native/TernUI-Regular.ttf')
assert all(ord(c) in g.getBestCmap() for c in chars)
for cp,name in original.getBestCmap().items():
 assert g.getBestCmap()[cp]==name
 assert g['hmtx'][name]==original['hmtx'][name]
 assert g['glyf'][name].compile(g['glyf'])==original['glyf'][name].compile(original['glyf'])
print('Added:',added,'Original upstream outlines and horizontal metrics preserved.')
