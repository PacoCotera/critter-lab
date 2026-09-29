#include "selected_lab.h"
#include <assert.h>
#include <stdio.h>
static void ready(SelectedLab *lab){selected_lab_input(lab,SELECTED_READY,0,lab->revision);}
static void press(SelectedLab *lab){selected_lab_input(lab,SELECTED_CONFIRM_DOWN,0,lab->revision);selected_lab_input(lab,SELECTED_CONFIRM_UP,0,lab->revision);}
int main(void){
 SelectedLab lab;selected_lab_init(&lab);press(&lab);assert(lab.page==V1_HOME);
 ready(&lab);unsigned old=lab.revision;selected_lab_input(&lab,SELECTED_CONFIRM_DOWN,0,old);selected_lab_input(&lab,SELECTED_ROTATE,1,old);ready(&lab);selected_lab_input(&lab,SELECTED_CONFIRM_UP,0,lab.revision);assert(lab.page==V1_HOME);
 press(&lab);assert(lab.page==V1_SAMPLES);ready(&lab);selected_lab_input(&lab,SELECTED_BACK_DOWN,0,lab.revision);selected_lab_input(&lab,SELECTED_BACK_UP,0,lab.revision);assert(lab.page==V1_HOME);
 selected_lab_input(&lab,SELECTED_CONFIRM_DOWN,0,old);selected_lab_input(&lab,SELECTED_CONFIRM_UP,0,lab.revision);assert(lab.page==V1_HOME);
 ready(&lab);selected_lab_input(&lab,SELECTED_CONFIRM_DOWN,0,lab.revision);selected_lab_input(&lab,SELECTED_SUSPEND,0,lab.revision);selected_lab_input(&lab,SELECTED_RESUME,0,lab.revision);ready(&lab);selected_lab_input(&lab,SELECTED_CONFIRM_UP,0,lab.revision);assert(lab.page==V1_HOME);
 uint8_t pixels[SELECTED_LAB_WIDTH*3];for(unsigned page=V1_HOME;page<=V1_DISCARD_REVIEW;page++){lab.page=(SelectedPage)page;for(unsigned row=0;row<SELECTED_LAB_HEIGHT;row++)selected_lab_row(&lab,row,pixels);}puts("Native V1 input and frame checks passed");return 0;
}
