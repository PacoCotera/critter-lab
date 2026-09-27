"""Static monochrome Probe study; not firmware or interactive UI."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).parent
FONT = 'C:/Windows/Fonts/bahnschrift.ttf'
INK, PAPER = 0, 1

def label(d, x, y, value, size=13):
    d.text((x, y), value, font=ImageFont.truetype(FONT, size), fill=INK)

def page(title, status):
    im = Image.new('1', (122,250), PAPER)
    d = ImageDraw.Draw(im)
    label(d,6,5,title,15); label(d,6,27,status,12)
    d.line((6,46,116,46),fill=INK)
    return im,d

def actions(d, first, second=None):
    d.line((6,193,116,193),fill=INK)
    d.rectangle((5,201,117,222),outline=INK)
    label(d,10,203,'> '+first,13)
    if second: label(d,10,228,second,13)

def preparation(d, percent, y=56):
    label(d,6,y,f'Next pack {percent}%',13)
    d.rectangle((6,y+22,116,y+30),outline=INK)
    d.rectangle((8,y+24,8+int(106*percent/100),y+28),fill=INK)

def containers(d):
    # Separate stores, not three competing preparation bars.
    for index, (name, count) in enumerate([('D',1),('E',0),('F',0)]):
        x=9+index*38
        d.rectangle((x+6,53,x+20,57),outline=INK)
        d.rectangle((x+2,59,x+24,88),outline=INK)
        if count:
            for row in range(79,87,3):
                d.line((x+4,row,x+22,row),fill=INK)
        label(d,x+2,92,f'{name} {count}',12)

def stock(d,data,energy):
    label(d,6,103,f'Data discs   {data}',12)
    label(d,6,123,f'Energy prisms {energy}',12)
    label(d,6,143,'Filaments    0',12)

frames=[]
im,d=page('Field survey','Collecting')
containers(d)
label(d,6,111,'1 pack ready',15)
label(d,6,131,'Storage 1 / 4',12)
preparation(d,65,149)
d.line((6,190,116,190),fill=INK); d.rectangle((5,194,117,213),outline=INK)
label(d,10,196,'> Review',12); label(d,10,215,'Inspect lead',12); label(d,10,234,'Pause run',12)
frames.append(('01-gathering',im,'Gather automatically'))

im,d=page('Field survey','Next pack 65%')
label(d,6,58,'Odd pattern',15)
d.polygon([(56,91),(71,106),(56,121),(41,106)],outline=INK,width=2)
label(d,6,140,'Lead available',13); label(d,6,164,'Can wait',12)
actions(d,'Inspect','Continue'); frames.append(('02-encounter',im,'Optional encounter'))

im,d=page('Finding saved','1 pack ready')
d.polygon([(46,61),(74,69),(69,110),(39,99)],outline=INK,width=2)
label(d,6,121,'Reference',15); label(d,6,141,'shard',15)
label(d,6,169,'Study at Lab',12)
actions(d,'Continue'); frames.append(('03-reference',im,'A reference, not a genome'))

im,d=page('Sample saved','Next pack 85%')
d.rectangle((47,61,75,69),fill=INK)
d.polygon([(44,76),(78,76),(82,82),(82,112),(40,112),(40,82)],outline=INK,width=2)
label(d,6,124,'Capsule D',15); label(d,6,149,'Contents',13); label(d,6,167,'unknown',13)
actions(d,'Continue'); frames.append(('04-capsule',im,'No genetic preview'))

im,d=page('Storage','Next pack 85%')
label(d,6,57,'Packs      1 / 4',12)
label(d,6,82,'Capsules   1 / 1',12)
label(d,6,107,'Findings   1 / 1',12)
label(d,6,139,'Capsules full',13)
label(d,6,161,'Stock continues',12)
actions(d,'Return','Pause run'); frames.append(('05-capacity',im,'Full categories wait'))

im,d=page('Expedition','Paused')
preparation(d,85); label(d,6,111,'Finds retained',13)
label(d,6,137,'Continue later',13)
label(d,6,165,'Progress held',12)
actions(d,'Resume','Review'); frames.append(('06-paused',im,'Resume this expedition'))

im,d=page('Survey complete','2 packs ready')
stock(d,1,1)
label(d,6,56,'Capsule D',13); label(d,6,77,'Reference shard',12)
label(d,6,170,'Return to Lab',13)
actions(d,'Review'); frames.append(('07-complete',im,'Prepared packs retained'))

im,d=page('Lab return','Receipt pending')
label(d,6,69,'Waiting for Lab',13)
label(d,6,109,'Finds retained',13)
label(d,6,143,'Not credited yet',12)
d.line((6,193,116,193),fill=INK)
label(d,6,204,'Keys inactive',12)
label(d,6,228,'Check at Lab',12)
frames.append(('08-pending',im,'Receipt is not complete'))

im,d=page('Lab received','2 packs received')
label(d,6,62,'1 Data disc',13); label(d,6,87,'1 Energy prism',13)
label(d,6,112,'Capsule D',13); label(d,6,137,'Reference shard',12)
label(d,6,170,'Saved at Lab',13)
actions(d,'Summary'); frames.append(('09-received',im,'Credit acknowledged once'))

for name,im,_ in frames: im.save(OUT/f'{name}.png')
board=Image.new('RGB',(1044,1940),'#101c32'); d=ImageDraw.Draw(board)
def heading(x,y,value,size=20,color='#f4f7ff'):
    d.text((x,y),value,font=ImageFont.truetype(FONT,size),fill=color)
heading(24,20,'Probe expedition / static control study',30)
heading(24,65,'122 x 250 native frames, enlarged 2x / proposed layout',22,'#afbdd8')
heading(24,100,'Physical Next moves focus. Physical Confirm activates. No screen taps.',20,'#afbdd8')
for i,(name,im,caption) in enumerate(frames):
    x=24+(i%3)*344; y=160+(i//3)*590
    heading(x,y,f'{i+1:02} / {caption}',17,'#66deeb')
    board.paste(im.convert('RGB').resize((244,500),Image.Resampling.NEAREST),(x+26,y+38))
heading(24,1920,'Proposal: examples are not timing, physical readability or e-ink validation.',16,'#afbdd8')
board.save(OUT/'storyboard.png')
