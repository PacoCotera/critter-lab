#include "selected_lab.h"
#include <stdio.h>
#include <string.h>
static void changed(SelectedLab *lab) { ++lab->revision; lab->ready=0; }
static void enter(SelectedLab *lab, SelectedPage page) { lab->page=page;lab->focus=0;changed(lab);lab->page_revision=lab->revision; }
static unsigned visible_residents(const SelectedLab *lab) {unsigned count=0;for(unsigned i=0;i<lab->game.individual_count;i++)count+=lab->game.individuals[i].revealed;return count;}
void selected_lab_init(SelectedLab *lab) { memset(lab,0,sizeof(*lab)); game_state_init(&lab->game);lab->revision=lab->page_revision=1; }
int selected_lab_load(SelectedLab *lab,const char *path,uint32_t clock) {
 if(strlen(path)>=sizeof(lab->save_path)) return 0;
 strcpy(lab->save_path,path);lab->clock=clock;
 int loaded=game_state_load(path,&lab->game);
 if(loaded<0) { lab->storage_error=1;strcpy(lab->message,"Save unavailable. Existing data preserved.");return 0; }
 if(loaded>0&&game_state_save(path,&lab->game)!=0){lab->storage_error=1;strcpy(lab->message,"Storage unavailable. Reload before continuing.");return 0;}
 game_rules_resume_runtime(&lab->game,clock);return 1;
}
const char *selected_lab_page(const SelectedLab *lab) {
 static const char *names[]={"home","expedition","cargo","samples","research","finding","creation","incubation","reveal","habitat"};return names[lab->page];
}
unsigned selected_lab_options(const SelectedLab *lab) {
 switch(lab->page) {
 case V1_HOME:return 4;case V1_EXPEDITION:return lab->game.expedition_id[0]?2:3;
 case V1_CARGO:return 4;case V1_SAMPLES:return lab->game.sample_count?lab->game.sample_count:1;
 case V1_STUDIES:return 6;case V1_FINDING:return 1;case V1_CREATE:return 2;
 case V1_INCUBATION:return 1;case V1_REVEAL:return 1;case V1_HABITAT:return visible_residents(lab)?3:1;
 } return 1;
}
const char *selected_lab_option(const SelectedLab *lab,unsigned option) {
 static const char *home[]={"Explore","Research","Incubator","Habitat"};
 static const char *routes[]={"Field survey","Garden forage","Weather watch"};
 static const char *cargo[]={"Return + store haul","Discard data pack","Discard energy pack","Discard essence pack"};
 static const char *care[]={"Spend time together","Next resident","Explore again"};
 switch(lab->page) {
 case V1_HOME:return home[option%4];
 case V1_EXPEDITION:return lab->game.expedition_id[0]?(option?"Cargo":"Keep exploring"):routes[option%3];
 case V1_CARGO:return cargo[option%4];
 case V1_SAMPLES:return lab->game.sample_count?lab->game.samples[option%lab->game.sample_count].id:"Find a sample outdoors";
 case V1_STUDIES:return option<5?pip_study(option)->title:"Prepare incubation";
 case V1_FINDING:return "Back to research";
 case V1_CREATE:return option?"Pale markings":"Plain coat / carries pale";
 case V1_INCUBATION:return lab->game.incubation_ready?"Open incubation":"Return to workbench";
 case V1_REVEAL:return "Meet in the habitat";
 case V1_HABITAT:return visible_residents(lab)?care[option%3]:"Explore for your first sample";
 } return "Return";
}
const char *selected_lab_focus(const SelectedLab *lab) {return selected_lab_option(lab,lab->focus);}
static GameResult commit(SelectedLab *lab,GameCommand command) {
 if(lab->storage_error)return GAME_STORAGE;
 char id[64];snprintf(id,sizeof(id),"local-%llu",(unsigned long long)lab->game.last_operation_sequence+1);
 command.sequence=lab->game.last_operation_sequence+1;command.operation_id=id;
 GameResult result=lab->storage_error?GAME_STORAGE:game_apply(lab->save_path,&lab->game,&command);
 if(result==GAME_COMMITTED_UNCERTAIN) {strcpy(lab->message,"Recorded; storage durability uncertain. Reload before continuing.");lab->storage_error=1;}
 else if(result==GAME_OK||result==GAME_DUPLICATE) strcpy(lab->message,"Saved");
 else if(result==GAME_UNAVAILABLE) strcpy(lab->message,"More supplies or discoveries needed.");
 else if(result==GAME_STORAGE) {strcpy(lab->message,"Could not save. No change accepted.");lab->storage_error=1;}
 else strcpy(lab->message,"Action unavailable. Your progress is safe.");
 changed(lab);return result;
}
void selected_lab_tick(SelectedLab *lab,uint32_t clock) {
 if(clock<=lab->clock) return;
 lab->clock=clock;
 if(lab->suspended||lab->confirm.held||lab->back.held) { game_rules_resume_runtime(&lab->game,clock);return; }
 GameCommand command={0};command.data.monotonic_seconds=clock;
 if(lab->game.expedition_active) {command.type=GAME_COMMAND_EXPEDITION_TICK;commit(lab,command);}
 if(lab->game.incubation_active&&!lab->game.incubation_ready) {command.type=GAME_COMMAND_INCUBATION_TICK;commit(lab,command);}
}
static void activate(SelectedLab *lab) {
 GameCommand command={0}; unsigned focus=lab->focus;
 switch(lab->page) {
 case V1_HOME: {static const SelectedPage pages[]={V1_EXPEDITION,V1_SAMPLES,V1_INCUBATION,V1_HABITAT};enter(lab,pages[focus]);if(lab->page==V1_HABITAT&&visible_residents(lab)){for(unsigned i=0;i<lab->game.individual_count;i++)if(lab->game.individuals[i].revealed){lab->resident=i;break;}}break;}
 case V1_EXPEDITION:
  if(lab->game.expedition_id[0]) {if(focus)enter(lab,V1_CARGO);else {strcpy(lab->message,"Gathering while you explore. Cargo keeps your haul.");changed(lab);} }
  else {command.type=GAME_COMMAND_EXPEDITION_START;command.data.expedition.kind=(GameExpeditionKind)focus;command.data.expedition.monotonic_seconds=lab->clock;commit(lab,command);lab->focus=0;game_rules_resume_runtime(&lab->game,lab->clock);} break;
 case V1_CARGO:
  if(!focus) {command.type=GAME_COMMAND_EXPEDITION_OFFLOAD;if(commit(lab,command)==GAME_OK)enter(lab,V1_SAMPLES);}
  else {command.type=GAME_COMMAND_EXPEDITION_DISCARD;command.data.discard.resource=(GameResource)(focus-1);command.data.discard.quantity=GAME_PACK_SIZE;command.data.discard.confirm=1;commit(lab,command);}break;
 case V1_SAMPLES:if(lab->game.sample_count){lab->sample=focus;enter(lab,V1_STUDIES);}else enter(lab,V1_EXPEDITION);break;
 case V1_STUDIES:
  if(focus==5) {if(lab->game.samples[lab->sample].decoded_studies==31)enter(lab,V1_CREATE);else {strcpy(lab->message,"Discover every region before incubation.");changed(lab);} }
  else {lab->study=focus; if(lab->game.samples[lab->sample].decoded_studies&(1u<<focus))enter(lab,V1_FINDING);
   else {command.type=GAME_COMMAND_STUDY;command.data.study.sample=lab->sample;command.data.study.study=focus;if(commit(lab,command)==GAME_OK)enter(lab,V1_FINDING);} }break;
 case V1_FINDING:enter(lab,V1_STUDIES);lab->focus=lab->study;break;
 case V1_CREATE:if(lab->game.samples[lab->sample].incubated){strcpy(lab->message,"This sample already has a Beecho. Choose another sample.");changed(lab);break;}if(lab->game.individual_count>=GAME_MAX_INDIVIDUALS){strcpy(lab->message,"Your 8 resident spaces are occupied. Explore or visit your habitat.");changed(lab);break;}command.type=GAME_COMMAND_INCUBATION_START;command.data.creation.sample=lab->sample;command.data.creation.preference=focus;command.data.creation.monotonic_seconds=lab->clock;if(commit(lab,command)==GAME_OK){enter(lab,V1_INCUBATION);game_rules_resume_runtime(&lab->game,lab->clock);}break;
 case V1_INCUBATION:if(lab->game.incubation_ready){command.type=GAME_COMMAND_INCUBATION_OPEN;if(commit(lab,command)==GAME_OK){lab->resident=lab->game.individual_count-1;enter(lab,V1_REVEAL);}}else enter(lab,V1_HOME);break;
 case V1_REVEAL:command.type=GAME_COMMAND_HABITAT_VISIT;command.data.habitat.individual=lab->resident;command.data.habitat.habitat=1;if(commit(lab,command)>=GAME_OK)enter(lab,V1_HABITAT);break;
 case V1_HABITAT:
  if(!visible_residents(lab)||focus==2)enter(lab,V1_EXPEDITION);
  else if(focus==1){do {lab->resident=(lab->resident+1)%lab->game.individual_count;} while(!lab->game.individuals[lab->resident].revealed);changed(lab);}
  else {command.type=GAME_COMMAND_CARE_VISIT;command.data.individual=lab->resident;if(commit(lab,command)==GAME_OK)strcpy(lab->message,"Pip perks up and settles beside you.");}break;
 }
}
void selected_lab_input(SelectedLab *lab,SelectedInput input,int delta,unsigned frame) {
 if(input==SELECTED_READY){if(frame==lab->revision&&!lab->suspended)lab->ready=1;return;}
 if(input==SELECTED_CANCEL||input==SELECTED_SUSPEND||input==SELECTED_RESUME){memset(&lab->confirm,0,sizeof(lab->confirm));memset(&lab->back,0,sizeof(lab->back));if(input!=SELECTED_CANCEL){lab->suspended=input==SELECTED_SUSPEND;changed(lab);game_rules_resume_runtime(&lab->game,lab->clock);}return;}
 if(input==SELECTED_ROTATE){if(!lab->suspended&&delta&&frame>=lab->page_revision&&frame<=lab->revision){unsigned count=selected_lab_options(lab);lab->focus=(lab->focus+(delta>0?1:count-1))%count;changed(lab);}return;}
 int back=input==SELECTED_BACK_DOWN||input==SELECTED_BACK_UP;SelectedGesture *gesture=back?&lab->back:&lab->confirm;
 if(input==SELECTED_CONFIRM_DOWN||input==SELECTED_BACK_DOWN){if(!gesture->held){gesture->held=1;gesture->revision=lab->revision;gesture->allowed=!lab->suspended&&lab->ready&&frame==lab->revision;}return;}
 if(!gesture->held)return;
 int allowed=gesture->allowed&&gesture->revision==lab->revision&&frame==lab->revision;memset(gesture,0,sizeof(*gesture));if(!allowed||lab->suspended)return;
 if(back){if(lab->page==V1_FINDING||lab->page==V1_CREATE){enter(lab,V1_STUDIES);lab->focus=lab->study;}else if(lab->page==V1_STUDIES)enter(lab,V1_SAMPLES);else enter(lab,V1_HOME);}else activate(lab);
}
