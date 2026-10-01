#include "expedition_render.h"
#include "core_art.h"
#include "native_font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Geometry and material clusters translate the reviewed expedition-map study.
 * Paths and disclosure always come from the copied domain projection. */
typedef struct { unsigned y, width; uint8_t *pixels; } RenderRow;
typedef struct {
  RenderRow *row;
  int x, y, scale, crop_x, crop_y, width, height;
} MapPainter;

static const char *site_names[] = {"Camp", "Moss bend", "Relay", "Stone shelf", "Old cache"};
static const char *resource_names[] = {"Data", "Energy", "Essence"};

static void expedition_name(const char *identity, char *value, size_t capacity) {
  const char *number = strrchr(identity, '-');
  if (number && number[1]) {
    ++number;
    const char *end = number;
    while (*end >= '0' && *end <= '9') ++end;
    if (!*end) {
      snprintf(value, capacity, "Expedition %02lu", strtoul(number, NULL, 10));
      return;
    }
  }
  snprintf(value, capacity, "Expedition");
}
enum { GRAPHITE = 0x1e282f, FIELD = 0x202b32, INK = 0xe3edef,
       MUTED = 0xa5b6bd, CYAN = 0x67cef5, WARM = 0xf1cd79,
       SAVED = 0xa3cda8, TRACK = 0x0b181f };

static void rectangle(RenderRow *row, int x, int y, int width, int height,
                      unsigned color) {
  if ((int)row->y < y || (int)row->y >= y + height) return;
  int begin = x < 0 ? 0 : x;
  int end = x + width > (int)row->width ? (int)row->width : x + width;
  for (int at = begin; at < end; ++at) {
    uint8_t *pixel = row->pixels + at * 3;
    pixel[0] = (uint8_t)(color >> 16);
    pixel[1] = (uint8_t)(color >> 8);
    pixel[2] = (uint8_t)color;
  }
}
static const NativeFont *font_for(unsigned size, int bold) {
  const NativeFont *fonts = bold ? lab_heading_fonts : lab_fonts;
  unsigned count = bold ? LAB_HEADING_FONT_COUNT : LAB_FONT_COUNT;
  const NativeFont *best = &fonts[0];
  for (unsigned i = 0; i < count; ++i)
    if (abs(fonts[i].size - (int)size) < abs(best->size - (int)size)) best = &fonts[i];
  return best;
}
static void label(RenderRow *row, int x, int y, const char *value, unsigned size,
                  int bold, unsigned color, int width) {
  const NativeFont *font = font_for(size, bold);
  char fitted[160];
  snprintf(fitted, sizeof(fitted), "%s", value);
  size_t length = strlen(fitted);
  if (native_text_width(font, fitted) > width) {
    while (length && native_text_width(font, fitted) > width - native_text_width(font, "..."))
      fitted[--length] = 0;
    if (length + 3 < sizeof(fitted)) strcat(fitted, "...");
  }
  int ink_top = font->size;
  for (const unsigned char *at = (const unsigned char *)fitted; *at; ++at) {
    unsigned index = *at >= 32 && *at <= 126 ? *at - 32 : '?' - 32;
    const NativeGlyph *glyph = &font->glyphs[index];
    if (glyph->height && glyph->top < ink_top) ink_top = glyph->top;
  }
  const uint8_t rgb[] = {(uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color};
  native_text_row(font, fitted, x, y - ink_top, row->y, row->width, row->pixels, 0, rgb);
}
static void panel(RenderRow *row, int x, int y, int width, int height) {
  core_art_panel_row(x, y, width, height, row->y, row->width, row->pixels);
}
static void message_lines(RenderRow *row, int x, int y, const char *value,
                          int width, unsigned size) {
  const NativeFont *font = font_for(size, 0);
  for (unsigned line = 0; line < 2 && *value; ++line) {
    char fitted[128] = {0};
    size_t length = 0, last_space = 0;
    while (value[length] && length + 1 < sizeof(fitted)) {
      fitted[length] = value[length];
      fitted[length + 1] = 0;
      if (native_text_width(font, fitted) > width) break;
      if (value[length] == ' ') last_space = length;
      ++length;
    }
    if (value[length] && last_space) length = last_space;
    fitted[length] = 0;
    if (line == 1) snprintf(fitted, sizeof(fitted), "%s", value);
    label(row, x, y + (int)line * 19, fitted, size, 0, MUTED, width);
    value += length;
    while (*value == ' ') ++value;
  }
}
static void focus(RenderRow *row, int x, int y, int width, int height) {
  core_art_focus_row(x, y, width, height, row->y, row->width, row->pixels);
}
static void asset(RenderRow *row, CoreArtId id, int x, int y) {
  core_art_row(id, x, y, row->y, row->width, row->pixels);
}
static void map_rectangle(MapPainter *map, int x, int y, int width, int height,
                          unsigned color) {
  int left = map->x + (x - map->crop_x) * map->scale;
  int top = map->y + (y - map->crop_y) * map->scale;
  int right = left + width * map->scale, bottom = top + height * map->scale;
  if (left < map->x) left = map->x;
  if (top < map->y) top = map->y;
  if (right > map->x + map->width) right = map->x + map->width;
  if (bottom > map->y + map->height) bottom = map->y + map->height;
  if (right > left && bottom > top) rectangle(map->row, left, top, right-left, bottom-top, color);
}
static void map_border(MapPainter *map, int x, int y, int width, int height,
                       unsigned color) {
  map_rectangle(map, x, y, width, 1, color);
  map_rectangle(map, x, y+height-1, width, 1, color);
  map_rectangle(map, x, y, 1, height, color);
  map_rectangle(map, x+width-1, y, 1, height, color);
}
static void tree(MapPainter *map, int x, int y, unsigned variant) {
  unsigned shade = variant ? 0x1b3934 : 0x203f39;
  unsigned middle = variant ? 0x32644d : 0x45734f;
  unsigned light = variant ? 0x719566 : 0x90a169;
  map_rectangle(map,x+7,y+18,4,7,0x564637);
  map_rectangle(map,x+2,y+13,15,7,shade);
  map_rectangle(map,x+4,y+7,11,9,middle);
  map_rectangle(map,x+7,y+2,6,8,light);
  map_rectangle(map,x+2,y+13,5,3,middle);
  map_rectangle(map,x+12,y+16,4,3,0x2a533f);
}
static void stone(MapPainter *map, int x, int y) {
  map_rectangle(map,x+3,y+5,15,11,0x17272c);
  map_rectangle(map,x+3,y+2,11,11,0x536d68);
  map_rectangle(map,x+5,y,7,3,0x91a49a);
  map_rectangle(map,x+3,y+8,4,3,0x354a48);
  map_rectangle(map,x+12,y+4,5,5,0x405650);
}
static void site_marker(MapPainter *map, const ExpeditionMapView *view, unsigned site) {
  int x = view->site_x[site]*20+10, y = view->site_y[site]*20+10;
  if (view->site_inspected[site]) map_border(map,x-12,y-13,24,26,CYAN);
  if (view->site_active[site]) {
    map_rectangle(map,x-12,y+13,24,2,WARM);
    map_rectangle(map,x-12,y+12,2,4,WARM);
    map_rectangle(map,x+10,y+12,2,4,WARM);
  }
  if (site == 0) {
    map_rectangle(map,x-9,y-5,18,12,0x172a30);
    for (int line=0; line<15; ++line) {
      int half=line*8/14;
      map_rectangle(map,x-half,y-9+line,half*2+1,1,0xb4aa83);
    }
    map_rectangle(map,x-2,y,4,7,0x263e3c);
  } else if (site == 1) {
    map_rectangle(map,x-8,y-6,16,13,0x325f55);
    map_rectangle(map,x-8,y-3,16,4,0x71cbd0);
    map_rectangle(map,x-4,y-8,5,4,0x9eaf74);
    map_rectangle(map,x+3,y+4,6,3,0x92b4a8);
  } else if (site == 2) {
    map_rectangle(map,x-6,y-8,12,16,0x315d65);
    map_rectangle(map,x-3,y-6,6,9,0x88b9b8);
    map_rectangle(map,x-9,y+6,18,4,0x263a3b);
    map_rectangle(map,x-1,y-12,2,5,0xb6c7be);
  } else if (site == 3) stone(map,x-10,y-8);
  else {
    map_rectangle(map,x-8,y-5,16,11,0x283b3b);
    map_rectangle(map,x-8,y-5,16,3,0x7f9a89);
    map_rectangle(map,x-3,y-5,6,11,0x9ab4a2);
  }
  if (view->site_collected[site]) {
    map_rectangle(map,x+7,y+5,3,3,SAVED);
    map_rectangle(map,x+10,y+2,3,3,SAVED);
  }
}
static void map_scene(RenderRow *row, const ExpeditionMapView *view, int x, int y,
                      int width, int height, int scale, int crop_x, int crop_y,
                      int labels, int avatar) {
  MapPainter map={row,x,y,scale,crop_x,crop_y,width,height};
  for (unsigned index=0; index<EXPEDITION_MAP_CELLS; ++index) {
    int tx=(int)(index%20)*20, ty=(int)(index/20)*20;
    unsigned terrain=view->terrain[index], variant=(index*17+index/20*13)%3;
    map_rectangle(&map,tx,ty,20,20,variant==0?0x344c40:variant==1?0x38503f:0x3c5342);
    for (unsigned cluster=0; cluster<3; ++cluster) {
      int dx=(int)((index*7+cluster*5)%8)*2;
      int dy=(int)((index*3+cluster*7)%8)*2;
      map_rectangle(&map,tx+dx,ty+dy,cluster==1?4:2,2,cluster==2?0x293f38:0x526849);
    }
    if (terrain==EXPEDITION_TERRAIN_WATER) {
      map_rectangle(&map,tx,ty,20,20,0x3a7780);
      map_rectangle(&map,tx+2,ty+4,8,2,0x83bbc0);
      map_rectangle(&map,tx+12,ty+14,8,2,0x55959c);
    }
    if (!view->paths[index]) {
      if (terrain==EXPEDITION_TERRAIN_TREE) tree(&map,tx,ty-3,index%2);
      else if (terrain==EXPEDITION_TERRAIN_STONE) stone(&map,tx,ty);
      continue;
    }
    map_rectangle(&map,tx+3,ty+3,14,14,0x8a8b65);
    map_rectangle(&map,tx+5,ty+5,10,10,0xb1ad80);
    map_rectangle(&map,tx+5,ty+12,3,2,0x777f5d);
    if (index%20<19 && view->paths[index+1]) map_rectangle(&map,tx+13,ty+4,14,12,0xa3a479);
    if (index/20<10 && view->paths[index+20]) map_rectangle(&map,tx+4,ty+13,12,14,0xa3a479);
  }
  for (unsigned site=0; site<EXPEDITION_SITE_COUNT; ++site)
    if (view->site_visible[site]) site_marker(&map,view,site);
  if (avatar && view->avatar_visible) {
    int ax=view->avatar_x*20+10, ay=view->avatar_y*20-1;
    if (ay < 7) ay = 7; /* First-row Cache still needs a complete position marker. */
    for (int line=0; line<13; ++line) {
      int half=6-abs(line-6);
      map_rectangle(&map,ax-half,ay-6+line,half*2+1,1,CYAN);
      if (half>1) map_rectangle(&map,ax-half+1,ay-6+line,half*2-1,1,0xf2fbfa);
    }
    for (unsigned site=0; site<5; ++site)
      if (view->site_visible[site] && view->avatar_x==view->site_x[site] && view->avatar_y==view->site_y[site])
        map_border(&map,ax-15,view->avatar_y ? view->avatar_y*20-5 : 1,30,31,WARM);
  }
  if (!labels) return;
  for (unsigned site=0; site<5; ++site) {
    if (!view->site_visible[site]) continue;
    int text_width=native_text_width(font_for(14,1),site_names[site]);
    int left=x+(view->site_x[site]*20-crop_x)*scale-18;
    if (left+text_width+4>x+width) left=x+width-text_width-4;
    if (left<x+3) left=x+3;
    int top=y+(view->site_y[site]*20-crop_y)*scale+23;
    if (top+16>y+height) top=y+height-17;
    rectangle(row,left-3,top-2,text_width+6,18,0x243a35);
    label(row,left,top,site_names[site],14,1,0xe2dec6,text_width+1);
  }
}
static void resource_band(RenderRow *row, const ExpeditionFieldView *view, int top) {
  static const char *states[]={"Not started","Active","Paused","Finished","Hold full"};
  char value[96];
  for (unsigned i=0; i<3; ++i) {
    int x=30+(int)i*133;
    rectangle(row,x-1,top,124,88,0x19262d);
    asset(row,(CoreArtId)(CORE_ART_DATA_COMPACT+i),x+1,top+6);
    snprintf(value,sizeof(value),"%u",view->earned[i]);
    label(row,x+61,top+7,value,27,1,INK,58);
    label(row,x+58,top+39,resource_names[i],14,0,MUTED,66);
    unsigned percent=view->preparation_ms[i]>=GAME_GATHER_ATTEMPT_MS?100:view->preparation_ms[i]*100/GAME_GATHER_ATTEMPT_MS;
    snprintf(value,sizeof(value),"%u%% prep",percent);
    label(row,x+3,top+58,value,14,0,MUTED,114);
    rectangle(row,x+2,top+73,113,9,0x09141b);
    rectangle(row,x+3,top+74,111,7,TRACK);
    rectangle(row,x+3,top+74,(int)(111*percent/100),7,i==2?SAVED:CYAN);
    unsigned state=view->preparation_status[i];
    label(row,x+3,top+82,state<5?states[state]:"Unavailable",14,0,MUTED,114);
  }
}
static void active_source(RenderRow *row,const ExpeditionFieldView *view,int y) {
  char value[128];
  unsigned active=3;
  for (unsigned i=0;i<3;++i)
    if (view->preparation_status[i]==EXPEDITION_PREP_ACTIVE || view->preparation_status[i]==EXPEDITION_PREP_CAPACITY_FULL) {active=i;break;}
  if (active<3) snprintf(value,sizeof(value),"%s / %s / %u attempts left",resource_names[active],view->source_name[active],view->remaining_chances[active]);
  else snprintf(value,sizeof(value),"No gathering active");
  label(row,30,y,value,16,1,active<3?SAVED:MUTED,390);
}
void expedition_field_row(const ExpeditionFieldView *view,unsigned y,uint8_t *pixels) {
  RenderRow row={y,450,pixels};
  rectangle(&row,0,0,450,600,GRAPHITE);
  panel(&row,12,12,426,576);
  label(&row,29,28,view->route[0]?view->route:"Expedition",24,1,INK,392);
  char value[128];
  expedition_name(view->outing_id,value,sizeof(value));
  label(&row,30,59,value,15,0,MUTED,390);
  const char *modes[]={"Probe","Cargo","Companions"};
  const int mode_x[]={28,151,275};
  for (unsigned i=0;i<3;++i) label(&row,mode_x[i],82,modes[i],18,1,i==0?CYAN:MUTED,140);
  rectangle(&row,28,98,91,2,CYAN);
  int site=view->page==EXPEDITION_PAGE_SITE;
  unsigned visible_actions=view->action_count>3?3:view->action_count;
  int shift=site && visible_actions>2?30:0;
  if (!site) {
    panel(&row,23,107,404,232);
    /* Crop the raster, not tile positions: keep both bevels and blue strokes. */
    map_scene(&row,&view->map,29,113,392,220,1,4,0,1,1);
  } else {
    panel(&row,24,110,402,220-shift);
    label(&row,42,132,view->location,25,1,INK,362);
    unsigned current=view->current_site<5?view->current_site:0;
    int crop_width=current==4?126:181;
    int crop_x=(int)view->map.site_x[current]*20-crop_width/2;
    int crop_y=(int)view->map.site_y[current]*20-28;
    if (crop_x<0) crop_x=0;
    if (crop_x+crop_width>400) crop_x=400-crop_width;
    if (crop_y<0) crop_y=0;
    if (crop_y+57>220) crop_y=163;
    map_scene(&row,&view->map,43,160,crop_width*2,114-shift,2,crop_x,crop_y,0,0);
    if (current==4) {
      asset(&row,CORE_ART_SAMPLE_NEUTRAL,332,185);
      label(&row,326,253-shift,"Sealed",17,1,INK,80);
    }
    const char *caption = view->message[0] ? view->message
      : current==4 ? "Contents unknown / collect to take it"
      : current==1 ? "Inspect markings to find the trail."
                   : "Choose supplies to gather.";
    message_lines(&row,43,279-shift,caption,362,14);
  }
  active_source(&row,view,346-shift);
  resource_band(&row,view,369-shift);
  if (site) {
    unsigned first=view->action_count>3 && view->focus>=2?view->focus-1:0;
    for (unsigned i=first;i<view->action_count && i<first+visible_actions;++i) {
      int action_y=487-shift+(int)(i-first)*30;
      if (i==view->focus) focus(&row,29,action_y-8,392,30);
      label(&row,45,action_y,view->actions[i],17,i==view->focus,i==view->focus?WARM:INK,360);
    }
    label(&row,30,540,"Up/Down: choose / Confirm: act",15,0,INK,390);
    label(&row,30,560,"Back: same map position",15,0,MUTED,390);
  } else {
    label(&row,30,474,view->location,20,1,INK,390);
    label(&row,30,504,view->message,15,0,MUTED,390);
    label(&row,30,538,"Directions: move / Confirm: inspect",15,0,INK,390);
    label(&row,30,560,"Back: modes",15,0,MUTED,390);
  }
}
static void accepted_contents(RenderRow *row,const ExpeditionReceivedView *view,int x,int y,int horizontal) {
  char value[96];
  for (unsigned i=0;i<3;++i) {
    int px=x+(horizontal?(int)i*181:0), py=y+(horizontal?0:(int)i*72);
    asset(row,(CoreArtId)(CORE_ART_DATA_COMPACT+i),px,py);
    snprintf(value,sizeof(value),"%u %s",view->accepted[i],resource_names[i]);
    label(row,px+57,py+13,value,horizontal?21:24,1,INK,horizontal?126:250);
  }
}
void expedition_received_row(const ExpeditionReceivedView *view,unsigned y,uint8_t *pixels) {
  RenderRow row={y,1024,pixels};
  rectangle(&row,0,0,1024,600,GRAPHITE);
  panel(&row,14,14,996,572);
  label(&row,36,33,"Expedition log",34,1,INK,945);
  label(&row,38,79,"Received expeditions",22,0,MUTED,945);
  if (!view->record_count) {
    panel(&row,30,116,964,418);
    label(&row,52,151,"No received expeditions",27,1,INK,912);
    label(&row,52,208,"Return with your Companion to record an outing.",22,0,MUTED,912);
    label(&row,38,551,"Back: Home",18,0,INK,945);
    return;
  }
  char value[128];
  if (!view->detail) {
    panel(&row,30,116,353,418);
    panel(&row,402,116,592,418);
    label(&row,50,138,"Received expeditions",25,1,INK,311);
    unsigned first=view->selected>=3?view->selected-2:0;
    for (unsigned i=first;i<view->record_count && i<first+4;++i) {
      int top=184+(int)(i-first)*79;
      if (i==view->selected) focus(&row,47,top,319,65);
      char record_id[64];
      snprintf(record_id,sizeof(record_id),"%s",view->record_labels[i]);
      char *contents=strstr(record_id," / ");
      if (contents) {
        *contents=0;
        contents+=3;
      }
      char record_name[64];
      expedition_name(record_id,record_name,sizeof(record_name));
      label(&row,62,top+14,record_name,20,i==view->selected,i==view->selected?WARM:INK,288);
      if (contents) label(&row,62,top+43,contents,14,0,MUTED,288);
    }
    char record_name[64];
    expedition_name(view->outing_id,record_name,sizeof(record_name));
    snprintf(value,sizeof(value),"Recorded route / %s",record_name);
    label(&row,425,140,value,27,1,INK,545);
    label(&row,426,178,view->received_label,19,0,SAVED,545);
    map_scene(&row,&view->map,426,212,400,220,1,0,0,1,0);
    if (view->sample_collected) {
      asset(&row,CORE_ART_SAMPLE_NEUTRAL,867,250);
      label(&row,850,332,"1 sealed",20,1,INK,126);
      label(&row,850,362,"sample",20,1,INK,126);
      label(&row,843,401,"Research at Lab",16,0,MUTED,142);
    } else label(&row,842,280,"Supplies only",17,0,MUTED,143);
    accepted_contents(&row,view,431,459,1);
    label(&row,38,551,"Up/Down: record / Confirm: details / Back: Home",18,0,INK,945);
  } else {
    panel(&row,30,116,582,418);
    panel(&row,632,116,362,418);
    expedition_name(view->outing_id,value,sizeof(value));
    label(&row,50,139,value,27,1,INK,540);
    label(&row,51,179,view->received_label,19,0,SAVED,540);
    map_scene(&row,&view->map,100,220,400,220,1,0,0,1,0);
    label(&row,51,464,view->trace_inspected?"Moss bend: trace inspected.":"Recorded route and visited places.",20,0,INK,540);
    label(&row,51,499,view->sample_collected?"Old cache: sealed capsule collected.":"Supplies returned / no sample collected.",20,0,INK,540);
    label(&row,652,141,"Accepted contents",25,1,INK,321);
    accepted_contents(&row,view,655,195,0);
    if (view->sample_collected) {
      asset(&row,CORE_ART_SAMPLE_NEUTRAL,654,415);
      label(&row,721,426,"1 sealed sample",20,1,INK,250);
    }
    label(&row,653,496,"Research at the Lab",18,0,MUTED,320);
    label(&row,38,551,"Back: received log",18,0,INK,945);
  }
}
