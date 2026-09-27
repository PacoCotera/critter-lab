from pathlib import Path
import textwrap
from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).parent
BG = '#f5f1e8'
INK = '#24313a'
CYAN = '#7ed6db'
PURPLE = '#d0bce8'
MINT = '#a5d8bd'
AMBER = '#f1c36a'
import os
FONT = os.environ.get('CRITTER_REVIEW_FONT', str(OUT.parent.parent/'native/shared/fonts/Vera.ttf'))

def font(size):
    return ImageFont.truetype(FONT, size)

im = Image.new('RGB', (1800, 1500), BG)
d = ImageDraw.Draw(im)

def text(x, y, s, size=23, color=INK, width=45):
    s = s.replace('→', '>')
    for line in textwrap.wrap(s, width=width):
        d.text((x,y), line, font=font(size), fill=color)
        y += size + 7
    return y

d.text((55, 35), 'An expedition becomes a question worth returning to', font=font(43), fill=INK)
text(55, 95, 'Connected paper storyboard / PROPOSED fixture / owner review / no working device or save behavior claimed', 23, width=125)
text(55, 145, 'Lab plans and interprets → Probe accompanies ordinary activity → Lab keeps the investigation → Companion continues the relationship', 24, width=120)

cards = [
('Lab · choose and prepare', 'I want material for an investigation.', 'Material trail', ['Medium length · material focus', 'Possible encounters · difficulty explained', 'Select expedition → prepare → ready'], 'The same named expedition is ready on the Probe. Collection has not started.', CYAN),
('Probe · collect and check', 'I carry it normally and check when convenient.', 'Collecting', ['Evidence / supplies / collection points*', 'Optional encounter waiting: Inspect / Leave', 'Earlier valid progress stays.'], '*Points required; meaning is open. Display actual collection state. Neutral fragment note; no sample contents.', AMBER),
('Lab · return and receive', 'I bring home something I can investigate.', 'Review collected results', ['Sample / expedition origin', 'Lab supplies / separate resource record', 'Pending receipt → accepted receipt'], 'The same sample and resource record remain identifiable. A retry must not add duplicate supplies.', CYAN),
('Lab · choose a question', 'I decide what is worth learning next.', 'Material study', ['Question: What structures are supported?', 'Actual cost and available Lab supplies', 'Review → Start study'], 'Starting commits the displayed cost once. Missing supplies preserves the question and earlier findings.', PURPLE),
('Lab · saved finding', 'I understand what changed and what is still open.', 'Finding retained', ['Structural region: known alternatives', 'Other required regions: unresolved', 'Next supported question / return later'], 'Leaving and returning keeps the sample, finding and spent supplies. One study does not unlock creation.', MINT),
('Future · creation and Companion', 'I choose knowingly, then meet the individual.', 'Continuation / not implemented', ['Resolve every required genomic region', 'Select a complete supported configuration', 'Explicit creation → READY → OPEN → Companion'], 'Creation uses the sample once; knowledge remains. The same saved individual continues into bonding and development.', '#dedbd5'),
]

for i, (title, intent, screen, lines, consequence, accent) in enumerate(cards):
    col, row = i % 3, i // 3
    x, y = 55 + col*575, 230 + row*565
    fill = '#faf9f4' if i < 5 else '#ece9e2'
    d.rounded_rectangle((x,y,x+540,y+520), radius=12, fill=fill, outline=INK, width=2)
    d.rectangle((x,y,x+540,y+9), fill=accent)
    text(x+22,y+25,f'{i+1}. {title}',28,width=35)
    text(x+22,y+73,intent,22,width=43)
    # A paper panel, deliberately free of invented controls or enclosure geometry.
    d.rectangle((x+22,y+140,x+518,y+325), fill=BG, outline=INK, width=2)
    d.rectangle((x+35,y+155,x+47,y+180), fill=accent, outline=INK)
    text(x+61,y+153,screen,26,width=31)
    yy = y+200
    for line in lines:
        yy = text(x+39,yy,line,21,width=41) + 5
    text(x+22,y+348,'Retained consequence',22,width=40)
    text(x+22,y+386,consequence,22,width=43)
    if col < 2:
        ax, ay = x+545, y+255
        d.line((ax,ay,ax+24,ay),fill=INK,width=3)
        d.polygon([(ax+24,ay),(ax+16,ay-7),(ax+16,ay+7)],fill=INK)

# Connect the rows explicitly without implying spatial/device layout.
d.line((1680,758,1680,777,30,777,30,1020,48,1020), fill=INK,width=3)
d.polygon([(48,1020),(40,1013),(40,1027)],fill=INK)
text(55,1390,'Review focus: interruption without loss · optional attention · clear progression from evidence to knowledge',24,width=125)
text(55,1430,'Paper composition only. Colors borrow refinement 02 vocabulary; screen layouts, hardware, content and persistence remain unvalidated.',21,width=135)
im.save(OUT/'storyboard.png')
