#include "selected_lab.h"
#include "assets.h"
#include "pip_art.h"
#include "native_font.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned y; uint8_t *pixels; } SelectedRow;
enum { BASE, PANEL, INK, MUTED, BLUE, EDGE, DEEP, WARM, SAGE, ACTION };
static const uint8_t colors[][3] = {
  {25, 36, 43}, {35, 43, 48}, {214, 222, 226}, {183, 198, 205},
  {24, 143, 234}, {70, 140, 184}, {16, 26, 33}, {237, 197, 106},
  {163, 206, 159}, {41, 41, 34}
};

static void rectangle(SelectedRow *row, int x, int y, int width, int height, unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height) return;
  for (int column = x; column < x + width; ++column)
    if (column >= 0 && column < (int)SELECTED_LAB_WIDTH) memcpy(row->pixels + column * 3, colors[color], 3);
}

static void outline(SelectedRow *row, int x, int y, int width, int height, int weight, unsigned color) {
  rectangle(row, x, y, width, weight, color);
  rectangle(row, x, y + height - weight, width, weight, color);
  rectangle(row, x, y, weight, height, color);
  rectangle(row, x + width - weight, y, weight, height, color);
}

static void stepped(SelectedRow *row, int x, int y, int width, int height, int corner, unsigned color) {
  int position = (int)row->y - y;
  if (position < 0 || position >= height) return;
  int edge = position < height / 2 ? position : height - position - 1;
  int inset = edge < corner ? ((corner - edge + 3) / 4) * 4 : 0;
  rectangle(row, x + inset, y, width - 2 * inset, height, color);
}

static void panel(SelectedRow *row, int x, int y, int width, int height) {
  stepped(row, x - 4, y - 4, width + 8, height + 8, 16, DEEP);
  stepped(row, x, y, width, height, 12, BLUE);
  stepped(row, x + 5, y + 5, width - 10, height - 10, 8, PANEL);
  rectangle(row, x + 13, y + 5, width - 26, 2, EDGE);
  outline(row, x + 15, y + 15, width - 30, height - 30, 2, EDGE);
}

static const NativeFont *font(int size) {
  for (unsigned index = 0; index < LAB_FONT_COUNT; ++index)
    if (lab_fonts[index].size == size) return &lab_fonts[index];
  assert(!"Missing existing native Lab font size");
  return NULL;
}

static void label(SelectedRow *row, int x, int y, const char *text, int size, unsigned color) {
  native_text_row(font(size), text, x, y, row->y, SELECTED_LAB_WIDTH, row->pixels, 0, colors[color]);
}

static void wrapped_label(SelectedRow *row, int x, int y, const char *text,
                          int size, unsigned color, int width) {
  char line[160] = {0};
  size_t used = 0;
  while (*text) {
    const char *end = strchr(text, ' ');
    size_t length = end ? (size_t)(end - text) : strlen(text);
    char candidate[160];
    snprintf(candidate, sizeof(candidate), "%s%.*s", line, (int)length, text);
    if (used && native_text_width(font(size), candidate) > width) {
      label(row, x, y, line, size, color);
      y += size + 5;
      used = 0;
      line[0] = 0;
    }
    if (used + length + 2 >= sizeof(line)) break;
    memcpy(line + used, text, length);
    used += length;
    line[used++] = ' ';
    line[used] = 0;
    text += length;
    if (*text == ' ') ++text;
  }
  if (used) label(row, x, y, line, size, color);
}

static void sprite(SelectedRow *row, unsigned asset, int x, int y, unsigned width, unsigned height) {
  if ((int)row->y < y || (int)row->y >= y + (int)height) return;
  const SelectedSprite *source = asset < SELECTED_SPRITE_COUNT ? &selected_sprites[asset] : &pip_sprites[asset-SELECTED_SPRITE_COUNT];
  unsigned source_y = ((unsigned)((int)row->y - y) * source->height) / height;
  for (unsigned column = 0; column < width; ++column) {
    int destination = x + (int)column;
    if (destination < 0 || destination >= (int)SELECTED_LAB_WIDTH) continue;
    unsigned source_x = column * source->width / width;
    memcpy(row->pixels + destination * 3, source->pixels + (source_y * source->width + source_x) * 3, 3);
  }
}

static void focus(SelectedRow *row, int x, int y, int width, int height) {
  const int weight = 4, length = 22;
  rectangle(row, x, y, length, weight, WARM);
  rectangle(row, x, y, weight, length, WARM);
  rectangle(row, x + width - length, y, length, weight, WARM);
  rectangle(row, x + width - weight, y, weight, length, WARM);
  rectangle(row, x, y + height - weight, length, weight, WARM);
  rectangle(row, x, y + height - length, weight, length, WARM);
  rectangle(row, x + width - length, y + height - weight, length, weight, WARM);
  rectangle(row, x + width - weight, y + height - length, weight, length, WARM);
}

static void number_label(SelectedRow *row,int x,int y,unsigned value,int size,unsigned color) { char text[32];snprintf(text,sizeof(text),"%u",value);label(row,x,y,text,size,color); }
void selected_lab_row(const SelectedLab *lab,unsigned y,uint8_t pixels[SELECTED_LAB_WIDTH*3]) {
 SelectedRow row={y,pixels};const GameState *game=&lab->game;
 rectangle(&row,0,0,1024,600,BASE);
 panel(&row,24,18,976,94);label(&row,46,32,"BEECHO LAB",34,INK);label(&row,47,77,"FIELD / DISCOVERY / LIFE",18,MUTED);
 unsigned stock[]={game->data,game->energy,game->essence};const char *names[]={"DATA","ENERGY","ESSENCE"};
 for(unsigned i=0;i<3;i++){int x=480+(int)i*167;sprite(&row,i,x,35,40,53);label(&row,x+53,34,names[i],18,MUTED);char amount[24];snprintf(amount,sizeof(amount),"%u.%02u",stock[i]/1000,(stock[i]%1000)/10);label(&row,x+54,63,amount,28,INK);}
 panel(&row,24,134,330,406);panel(&row,376,134,624,406);
 const char *titles[]={"WORKBENCH","EXPEDITION","CARGO","SAMPLES","RESEARCH","DISCOVERY","INCUBATE","INCUBATOR","HELLO, BEECHO","HABITAT"};
 label(&row,396,152,titles[lab->page],34,INK);
 unsigned count=selected_lab_options(lab);unsigned first=lab->focus>=6?lab->focus-5:0;
 for(unsigned i=first;i<count&&i<first+6;i++){int yy=159+(int)(i-first)*58; if(i==lab->focus){rectangle(&row,39,yy-2,299,46,DEEP);focus(&row,37,yy-4,303,50);}label(&row,53,yy+7,selected_lab_option(lab,i),18,i==lab->focus?WARM:INK);}
 char text[100];
 if(lab->page==V1_HOME){sprite(&row,SPRITE_SAMPLE,432,221,155,155);label(&row,613,231,"A mystery to bring home",24,INK);label(&row,613,275,"A life to discover",24,WARM);label(&row,414,430,"Explore. Research. Meet your Beecho.",24,INK);}
 else if(lab->page==V1_EXPEDITION||lab->page==V1_CARGO){
  label(&row,402,205,game->expedition_active?"PROBE / GATHERING":game->expedition_id[0]?"SURVEY COMPLETE / RETURN WITH HAUL":"CHOOSE YOUR EXPEDITION",24,WARM);
  unsigned cargo[]={game->expedition_data,game->expedition_energy,game->expedition_essence};
  for(unsigned i=0;i<3;i++){int x=416+(int)i*179;sprite(&row,i,x+25,265,65,86);snprintf(text,sizeof(text),"%u%%",(cargo[i]%GAME_PACK_SIZE)/10);label(&row,x+21,367,text,28,INK);snprintf(text,sizeof(text),"%u pack%s",cargo[i]/GAME_PACK_SIZE,cargo[i]/GAME_PACK_SIZE==1?"":"s");label(&row,x+7,410,text,22,MUTED);}
  snprintf(text,sizeof(text),"%u / 60 s  |  Sample: %s",game->expedition_elapsed,game->sample_count>=GAME_MAX_SAMPLES?"shelf full":game->expedition_elapsed>=60?"found":"scanning");label(&row,411,443,text,18,MUTED);
  unsigned total=cargo[0]+cargo[1]+cargo[2];outline(&row,411,470,550,20,2,EDGE);rectangle(&row,415,474,(int)(542*(total>4000?4000:total)/4000),12,BLUE);
 }
 else if(lab->page==V1_SAMPLES){sprite(&row,SPRITE_SAMPLE,602,235,155,155);snprintf(text,sizeof(text),"%u samples retained",game->sample_count);label(&row,452,432,text,28,INK);}
 else if(lab->page==V1_STUDIES&&lab->focus==5){
  unsigned discovered=0;for(unsigned i=0;i<5;i++)discovered+=(game->samples[lab->sample].decoded_studies>>i)&1u;
  sprite(&row,SPRITE_SAMPLE,597,240,150,150);
  snprintf(text,sizeof(text),"%u / 5 discoveries",discovered);label(&row,483,407,text,28,WARM);
  label(&row,414,465,game->samples[lab->sample].incubated?"This sample already has a Beecho.":discovered==5?"Ready to choose a complete form.":"Research every topic to prepare a form.",22,INK);
 }
 else if(lab->page==V1_STUDIES||lab->page==V1_FINDING){
  unsigned study=lab->page==V1_FINDING?lab->study:(lab->focus<5?lab->focus:0);const PipStudy *entry=pip_study(study);int known=(game->samples[lab->sample].decoded_studies&(1u<<study))!=0;
  label(&row,402,202,game->samples[lab->sample].id,22,MUTED);
  sprite(&row,known?(study==0?SPRITE_CROWN:study==1?SPRITE_EYE_RING:SPRITE_SAMPLE):SPRITE_UNKNOWN,586,242,185,130);
  label(&row,413,395,known?"DISCOVERED / FREE TO INSPECT":"UNKNOWN / RESEARCH TO DISCOVER",22,known?SAGE:WARM);
  snprintf(text,sizeof(text),"Cost: %.1f Data / %.1f Energy / %.1f Essence",known?0.0:entry->cost_data/1000.0,known?0.0:entry->cost_energy/1000.0,known?0.0:entry->cost_essence/1000.0);label(&row,413,448,text,22,INK);
  if(known)wrapped_label(&row,413,480,entry->finding,18,MUTED,548);
 }
 else if(lab->page==V1_CREATE){sprite(&row,SPRITE_SAMPLE,448,240,128,128);label(&row,605,235,"Genome decoded",28,SAGE);label(&row,605,282,"Choose a supported form",22,INK);label(&row,410,414,"1 sample + 0.5 of each resource",24,INK);label(&row,410,464,"Confirm starts one incubation.",22,WARM);}
 else if(lab->page==V1_INCUBATION){sprite(&row,SPRITE_SAMPLE,597,232,150,150);label(&row,455,409,game->incubation_ready?"READY / OPEN WHEN YOU CHOOSE":game->incubation_active?"INCUBATING / PROGRESS SAVED":"No incubation yet",24,game->incubation_ready?SAGE:INK);number_label(&row,633,461,game->incubation_elapsed,28,WARM);}
 else {int visible=game->individual_count&&game->individuals[lab->resident].revealed;label(&row,415,215,visible?game->individuals[lab->resident].id:"Your habitat awaits",24,INK);
  if(visible) sprite(&row,SELECTED_SPRITE_COUNT+(game->individuals[lab->resident].genome.loci[2][0]=='p'&&game->individuals[lab->resident].genome.loci[2][1]=='p'),552,245,157,173);
  label(&row,414,439,visible?"Your Pip is at home.":"Research your first sample to begin.",22,INK);
  if(visible){snprintf(text,sizeof(text),"Visits together: %u",game->individuals[lab->resident].care_visits);label(&row,414,480,text,22,SAGE);}
 }
 label(&row,30,557,lab->message[0]?lab->message:"Rotate to explore / Confirm to act / Back to return",22,lab->storage_error?WARM:MUTED);
}

static int word(FILE *output, unsigned value, unsigned bytes) {
  for (unsigned index = 0; index < bytes; ++index)
    if (fputc((int)((value >> (index * 8)) & 255u), output) == EOF) return 0;
  return 1;
}

int selected_lab_bmp(const SelectedLab *lab, FILE *output) {
  const unsigned stride = SELECTED_LAB_WIDTH * 3;
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  if (fwrite("BM", 1, 2, output) != 2) return 0;
  unsigned fields[][2] = {{54 + stride * SELECTED_LAB_HEIGHT, 4}, {0, 4}, {54, 4}, {40, 4}, {SELECTED_LAB_WIDTH, 4}, {SELECTED_LAB_HEIGHT, 4}, {1, 2}, {24, 2}, {0, 4}, {stride * SELECTED_LAB_HEIGHT, 4}, {2835, 4}, {2835, 4}, {0, 4}, {0, 4}};
  for (unsigned index = 0; index < sizeof(fields) / sizeof(fields[0]); ++index)
    if (!word(output, fields[index][0], fields[index][1])) return 0;
  for (unsigned y = SELECTED_LAB_HEIGHT; y > 0; --y) {
    selected_lab_row(lab, y - 1, pixels);
    for (unsigned column = 0; column < SELECTED_LAB_WIDTH; ++column) {
      uint8_t swap = pixels[column * 3];
      pixels[column * 3] = pixels[column * 3 + 2];
      pixels[column * 3 + 2] = swap;
    }
    if (fwrite(pixels, 1, stride, output) != stride) return 0;
  }
  return !ferror(output);
}
