"""Generate immutable native-size ASCII coverage; requires Pillow 12.3.0.
Run from this directory: python generate.py. Original unmodified Bitstream Vera
from ReportLab's font distribution; exact redistribution license in LICENSE.txt.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
root = Path(__file__).resolve().parent
for name, sizes in [('portable', [9, 11, 20, 24]), ('lab', [18, 19, 21, 22, 26, 34, 24, 28, 32, 36, 40, 44, 48])]:
    data=[]; glyphs=[]; records=[]
    for size in sizes:
        font=ImageFont.truetype(str(root/'Vera.ttf'),size)
        first=len(glyphs)
        for code in range(32,127):
            char=chr(code); left,top,right,bottom=font.getbbox(char)
            width=right-left; height=bottom-top
            mask=Image.new('L',(max(1,width),max(1,height)))
            ImageDraw.Draw(mask).text((-left,-top),char,font=font,fill=255)
            glyphs.append((len(data),width,height,left,top,round(font.getlength(char))))
            data.extend(mask.tobytes() if width and height else [])
        records.append((size,font.getmetrics()[0],first))
    text='#include "native_font.h"\n'
    text+=f'static const uint8_t {name}_coverage[] = {{\n'
    text+='\n'.join('  '+','.join(map(str,data[i:i+32]))+',' for i in range(0,len(data),32))+'\n};\n'
    text+=f'static const NativeGlyph {name}_glyphs[] = {{\n'
    text+='\n'.join('  {'+','.join(map(str,g))+'},' for g in glyphs)+'\n};\n'
    text+=f'const NativeFont {name}_fonts[] = {{\n'
    text+='\n'.join(f'  {{{size}, {baseline}, {name}_coverage, {name}_glyphs + {first}}},' for size,baseline,first in records)+'\n};\n'
    (root.parent/f'{name}_font_data.c').write_text(text)
    print(name,len(data),'coverage bytes')

