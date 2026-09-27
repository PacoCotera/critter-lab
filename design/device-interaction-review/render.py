"""Disposable paper screen composition. Run with Pillow; CRITTER_REVIEW_FONT overrides font."""
from pathlib import Path
import os
from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).parent
ROOT = next((p for p in OUT.parents if (p/'native/shared/fonts/Vera.ttf').exists()), None)
if ROOT is None:
    ROOT = next(p/'.publication/critter-lab' for p in OUT.parents if (p/'.publication/critter-lab/native/shared/fonts/Vera.ttf').exists())
FONT = os.environ.get('CRITTER_REVIEW_FONT', str(ROOT/'native/shared/fonts/Vera.ttf'))
INK, BG, PANEL = '#23323c', '#f3f3e9', '#ffffff'
CYAN, LAV, AMBER, MINT = '#74ccd4', '#c7b2e2', '#efbe62', '#a5d5b9'

def write(draw, xy, text, size=26, fill=INK):
    draw.text(xy, text, font=ImageFont.truetype(FONT,size), fill=fill)

def box(draw, bounds, fill, outline=INK, width=2):
    x,y,r,b=bounds
    draw.polygon([(x+8,y),(r-8,y),(r,y+8),(r,b-8),(r-8,b),(x+8,b),(x,b-8),(x,y+8)],fill=fill,outline=outline,width=width)

def action(draw, y, label, focused=False):
    box(draw,(376,y,976,y+56),BG)
    if focused:
        draw.rectangle((376,y,384,y+56),fill=AMBER)
        draw.rectangle((385,y+1,975,y+55),outline=AMBER,width=3)
    write(draw,(400,y+12),('> ' if focused else '  ')+label)

def lab(kind):
    im=Image.new('RGB',(1024,600),'#131e2a'); d=ImageDraw.Draw(im)
    pale, muted = '#edf0ff', '#acbdd2'
    def t(x,y,s,size=26,color=pale): write(d,(x,y),s,size,color)
    t(32,16,'CRITTER LAB',22,muted)
    t(800,16,'Sample 01',26)
    d.rectangle((936,64,976,120),outline=muted,width=2)
    for x,y in [(944,80),(952,88),(944,104),(960,104)]:
        d.rectangle((x,y,x+2,y+2),fill=LAV)
    title={'return':'Expedition return','study':'Structure','finding':'Structure'}[kind]
    subtitle={'return':'Material trail','study':'Unknown','finding':'Both possible'}[kind]
    t(32,64,title,30); t(32,112,subtitle,26,muted)
    if kind=='return':
        box(d,(80,164,944,424),'#08121d',outline='#08121d')
        t(128,208,'Sample ready for research',30)
        t(128,272,'Lab supplies',26,muted); t(128,312,'Collected 2',30)
        t(584,272,'Field encounter',26,muted); t(584,312,'Note available',26)
        t(32,456,'Choose what to investigate next.',26,muted)
        selected='Open sample question'
    else:
        stage=Image.open(OUT/('unknown-stage.png' if kind=='study' else 'reference-stage.png'))
        im.paste(stage,(80,163))
        t(32,452,'Other regions unknown',26,muted)
        if kind=='study':
            t(552,452,'Lab supplies',22,muted)
            t(552,488,'Uses 1',26); t(776,488,'In stock 2',26)
            selected='Start study'
        else:
            t(584,452,'Neither chosen',26,muted)
            t(584,488,'Supplies left 1',22,muted)
            selected='Back to sample'
    d.line((32,534,992,534),fill='#334357')
    t(48,552,'Back',26)
    x=560 if kind=='return' else 730 if kind=='study' else 704
    width=ImageDraw.Draw(im).textbbox((0,0),selected,font=ImageFont.truetype(FONT,26))[2]
    d.rectangle((x-8,544,x+width+8,589),outline=AMBER,width=3)
    t(x,552,selected,26,AMBER)
    im.save(OUT/f'lab-{kind}.png'); return im

def probe():
    im=Image.new('RGB',(122,250),'white'); d=ImageDraw.Draw(im)
    def p(y,s,size=10): write(d,(5,y),s,size,'black')
    p(5,'Material trail',12); d.line((5,25,117,25),fill='black')
    p(33,'Collecting',12)
    p(54,'Progress'); p(70,'[recorded state]')
    p(90,'Evidence retained'); p(108,'Supplies [record]')
    p(126,'Points [record]')
    d.rectangle((4,148,118,189),outline='black',width=1)
    p(154,'Optional encounter'); p(172,'> Review encounter')
    p(198,'Pause / return')
    d.line((5,220,117,220),fill='black')
    p(231,'Next    Confirm',10)
    im.save(OUT/'probe-collection.png'); return im

screens=[lab(k) for k in ('return','study','finding')]
pr=probe()
board=Image.new('RGB',(1600,1450),'#f5f1e8'); d=ImageDraw.Draw(board)
write(d,(32,24),'From expedition to finding: operate the visible focus',30)
write(d,(32,68),'Paper UI proposal / fixed simulated controls / no touch or screen-click actions',22)
for i,(im,label) in enumerate(zip(screens,['1 Return: Confirm opens the focused question','2 Study: Confirm commits the displayed cost','3 Finding: Confirm returns to the sample'])):
    y=115+i*425
    board.paste(im.resize((640,375)),(32,y))
    write(d,(704,y+8),label,22)
    notes=[['Turn moves between Back and Open sample question.','Back returns to the prior expedition context.','Receipt pending: retain record; do not offer study.'],['Turn only moves focus; it never starts a study.','Missing supplies: explain shortage; Start unavailable.','Pending: show submitted study, no second Start.','Back returns; submitted work is not cancelled.'],['Finding shows equal references; neither is chosen.','Back restores sample page and valid caller focus.','Return later preserves finding and spent supplies.','All required regions unlock before selection.']][i]
    for j,n in enumerate(notes): write(d,(704,y+66+j*44),n,22)
    if i==0:
        board.paste(pr.resize((122,250)),(1370,y+105))
        write(d,(704,y+236),'Probe 122 x 250: Next cycles visible targets.',22)
        write(d,(704,y+274),'Confirm opens encounter; Next reaches Leave / Back.',22)
        write(d,(704,y+312),'Points and progress need real domain records.',22)
    if i<2: write(d,(340,y+381),'Confirm / fresh gesture on ready frame',22)
write(d,(32,1400),'Lab originals: 1024 x 600. Probe original: 122 x 250. Review the native exports alongside this sequence.',22)
board.save(OUT/'input-storyboard.png')
