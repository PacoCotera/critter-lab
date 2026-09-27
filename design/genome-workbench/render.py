"""Static owner-review study. No executable UI or generated biological artwork."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).parent
FONT = Path('C:/Windows/Fonts/bahnschrift.ttf')
C = dict(bg='#101c32', panel='#192b47', line='#344c70', text='#f4f7ff',
         muted='#afbdd8', cyan='#66deeb', purple='#bba6ff', mint='#96e5ba', amber='#ffd477')

def text(d, x, y, value, size=26, color='text'):
    d.text((x,y), value, font=ImageFont.truetype(str(FONT),size), fill=C.get(color,color))

def panel(d, box, color='line'):
    x,y,r,b=box
    d.polygon([(x+8,y),(r-8,y),(r,y+8),(r,b-8),(r-8,b),(x+8,b),(x,b-8),(x,y+8)],
              fill=C['panel'],outline=C[color],width=2)

def focus(d, box):
    x,y,r,b=box
    for px,py,sx,sy in [(x,y,1,1),(r,y,-1,1),(x,b,1,-1),(r,b,-1,-1)]:
        d.line([(px+16*sx,py),(px,py),(px,py+16*sy)], fill=C['amber'],width=4)

def frame(title, identity, stock, hint):
    im=Image.new('RGB',(1024,600),C['bg']); d=ImageDraw.Draw(im)
    text(d,24,18,title,30); text(d,720,23,identity,22,'cyan')
    d.line((24,64,1000,64),fill=C['line'],width=2)
    text(d,24,78,stock,22,'muted')
    d.line((24,548,1000,548),fill=C['line'],width=2)
    text(d,24,566,hint,22)
    return im,d

def pair(d,x,y,second='unknown',selected=False):
    # Paired inherited copies: hatched tile means unresolved, never a lock.
    for offset,value in [(0,'p'),(116,second)]:
        panel(d,(x+offset,y,x+offset+96,y+88),'purple')
        if value=='unknown':
            for i in range(12,76,12):
                d.line((x+offset+i,y+16,x+offset+i-8,y+64),fill=C['line'],width=3)
        else: text(d,x+offset+35,y+23,value,30,'purple')
    d.line((x+96,y+44,x+116,y+44),fill=C['purple'],width=3)
    if selected: focus(d,(x-8,y-8,x+220,y+96))

def zone(d,second='unknown',selected=True):
    # Research neighborhood, not a chromosome or biological spatial claim.
    d.line([(140,272),(260,272),(312,316),(416,316)],fill=C['line'],width=3)
    d.line([(628,316),(716,316),(756,236),(888,236)],fill=C['line'],width=3)
    d.line([(716,316),(756,414),(888,414)],fill=C['line'],width=3)
    text(d,48,196,'Structure',22,'muted'); text(d,48,228,'Known',22,'mint')
    text(d,756,166,'Sensing',22,'muted'); text(d,756,198,'Known',22,'mint')
    text(d,756,358,'Movement',22,'muted'); text(d,756,390,'Known',22,'mint')
    text(d,416,192,'Markings',30,'purple')
    pair(d,416,272,second,selected)
    text(d,416,386,'1 of 2 copies known' if second=='unknown' else 'Both copies known',22,'muted')

frames=[]
im,d=frame('Genome collection','Lab / 3 genomes','Stock   Mineral 3     Lumen 3     Catalyst 0','Turn: choose record   Confirm: inspect   Back: Lab')
for y,name,detail,cost in [(132,'A / Markings','One copy still unknown','Compare: 2 Mineral'),(260,'B / Movement','Drive, then its energy cost','Study: 2 Lumen'),(388,'C / Crown','Second crown copy unknown','Needs 1 Catalyst')]:
    panel(d,(24,y,1000,y+108)); text(d,48,y+12,name,30,'cyan')
    text(d,48,y+57,detail,26,'muted'); text(d,688,y+24,cost,22,'mint' if name[0]!='C' else 'muted')
focus(d,(24,132,1000,240)); text(d,32,510,'Capsule D awaits preparation',22,'muted')
frames.append(('01-collection',im))

im,d=frame('Research workbench','Sample A / partial','Stock   Mineral 3     Lumen 3     Catalyst 0','Turn: explore zones   Confirm: inspect study   Back: collection')
zone(d); text(d,32,488,'Markings comparison',26); text(d,630,490,'3 Mineral available / need 2',22,'mint')
frames.append(('02-zone',im))

im,d=frame('Markings comparison','Sample A / partial','Stock   Mineral 3     Lumen 3     Catalyst 0','Turn: choose   Confirm: activate focused action   Back: workbench')
pair(d,416,150); text(d,32,274,'Compare the unresolved copy',30)
text(d,32,324,'Mineral grains',26); text(d,544,324,'Have 3    Use 2    Remain 1',26,'purple')
text(d,32,374,'The result is still unknown.',22,'muted')
panel(d,(24,440,632,520)); text(d,48,462,'Begin comparison',26); focus(d,(24,440,632,520))
text(d,736,462,'Return to zone',26)
frames.append(('03-commit',im))

im,d=frame('Comparison in progress','Sample A / partial','Comparison submitted   /   stock update awaits confirmation','Turn / Confirm: inactive   Back: collection')
zone(d,selected=False); text(d,32,478,'Resolving the second copy',30,'purple')
text(d,32,518,'Result not yet saved',22,'muted')
frames.append(('04-pending',im))

im,d=frame('Discovery saved','Sample A / decoded','Stock   Mineral 1     Lumen 3     Catalyst 0','Confirm: return to workbench   Back: collection')
pair(d,144,178,'P'); text(d,440,148,'Pale variant carried',30,'mint')
text(d,440,200,'No pale body markings',26)
text(d,440,244,'May pass the variant to offspring',22,'muted')
d.line((24,344,1000,344),fill=C['line'],width=2)
text(d,32,372,'Markings zone decoded',26,'mint'); text(d,32,420,'All required findings established',22,'muted')
text(d,32,474,'Incubation still needs its own preparation.',22,'muted')
frames.append(('05-discovery',im))

for name,im in frames: im.save(HERE/f'{name}.png')
board=Image.new('RGB',(1088,5*696+148),C['bg']); d=ImageDraw.Draw(board)
text(d,32,20,'Collection → research → discovery',30)
text(d,32,66,'Lab 1024 × 600 / static proposal / console controls only',22,'muted')
captions=['1 / Choose among research opportunities','2 / Inspect an unknown part of the genome','3 / Review the cost before spending','4 / Keep the same study while awaiting its result','5 / See what was discovered, and what it means']
for i,((name,im),caption) in enumerate(zip(frames,captions)):
    y=148+i*696; text(d,32,y,caption,26,'cyan'); board.paste(im,(32,y+48))
board.save(HERE/'storyboard.png')
