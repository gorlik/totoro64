/******************************************************************************
 *  TOTORO64                                                                  *
 *  A Studio Ghibli inspired game for the Commodore 64                        *
 *  Copyright 2021 Gabriele Gorla                                             *
 *                                                                            *
 *  This program is free software: you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by      *
 *  the Free Software Foundation, either version 3 of the License, or         *
 *  (at your option) any later version.                                       *
 *                                                                            *
 *  GTERM is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *  GNU General Public License for more details.                              *
 *                                                                            *
 *  You should have received a copy of the GNU General Public License         *
 *  along with GTERM.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <peekpoke.h>
#include <c64.h>
#include <zlib.h>

#include "totoro64.h"

//#define DEBUG

#define VERSION "0.10"

#define HFREQ 50

#define STAGE_TIME 60

#define EOF_LINE 0x80

#define MAX_XV 16
#define MAX_YV 10

#define GROUND_Y 220
#define PGROUND_Y (GROUND_Y-21)

#define ACORN_START_Y 68

#define MIN_X 32
#define MAX_X 312
#define MAX_PX (MAX_X-24)

#define CLR_TOP() memset(SCR_BASE,0x0,640)

#ifdef DEBUG
#define DEBUG_BORDER(a) VIC.bordercolor=a
#define DEBUG_BORDER_INC() VIC.bordercolor++
#else
#define DEBUG_BORDER(a)
#define DEBUG_BORDER_INC()
#endif

void IRQ(void);

const unsigned char run_seq[] =
  {
   8,4,9,
   10,5,11,
   12,6,13,
   10,5,11,
   20,16,21,
   22,17,23,
   24,18,25,
   22,17,23
  };

const struct stage_t stage[] =
  {
   { STAGE_TIME, 10, 4, 0 },
   { STAGE_TIME, 15, 6, 0 },
   { STAGE_TIME, 20, 8, 0 },
   { STAGE_TIME, 25, 10, 0 },
   { STAGE_TIME, 30, 12, 0 },
   { STAGE_TIME, 35, 14, 0 },
   { STAGE_TIME, 40, 16, 0 },
   { STAGE_TIME, 50, 18, 0 },
   { STAGE_TIME, 60, 20, 0 },
};

struct game_state_t gstate;
struct player_t totoro;
struct acorn_t acorn[4];


extern unsigned char track1[];
extern unsigned char instr1;
extern unsigned char time1;
extern unsigned char loop1;
extern unsigned char next_loop1;
extern unsigned char vpb;

extern const unsigned char *t1addr;
extern const unsigned char *t2addr;
#pragma zpsym("t1addr")
#pragma zpsym("t2addr")

void __fastcall__ setup_sid(void)
{
  memset(&SID,0,24);
  SID.amp = 0x1f; // set volume to 15
  SID.v1.ad = 0x80;
  SID.v1.sr = 0xf6;
  loop1=0;
  t1addr=track1;
  instr1=0x10; // triangular
  time1 = 0;
  vpb=8;
}

void __fastcall__ totoro_set_pos(void)
{
  VIC.spr0_x   = totoro.xpos;
  VIC.spr1_x   = totoro.xpos;
  VIC.spr2_x   = totoro.xpos;
  
  if(totoro.xpos>255)   VIC.spr_hi_x |=0x07;
  else VIC.spr_hi_x &= 0xf8;

  VIC.spr0_y   = totoro.ypos;
  VIC.spr1_y   = totoro.ypos;
  VIC.spr2_y   = totoro.ypos;

  switch(totoro.state) {
  case IDLE:
    if(totoro.blink)   SPR_PTR[0]=1;
    else   SPR_PTR[0]=0;
    SPR_PTR[1]=2;
    SPR_PTR[2]=3;
    break;
  case RUN:
    if(totoro.xv>0) {
      SPR_PTR[0]=run_seq[0+totoro.idx*3];
      SPR_PTR[1]=run_seq[1+totoro.idx*3];
      SPR_PTR[2]=run_seq[2+totoro.idx*3];
    } else {
      SPR_PTR[0]=run_seq[12+totoro.idx*3];
      SPR_PTR[1]=run_seq[13+totoro.idx*3];
      SPR_PTR[2]=run_seq[14+totoro.idx*3];
    }
    break;
  case BRAKE:
    if(totoro.xv>0) {
      SPR_PTR[0]=14;
      SPR_PTR[1]=7;
      SPR_PTR[2]=15;
    } else {
      SPR_PTR[0]=26;
      SPR_PTR[1]=19;
      SPR_PTR[2]=27;
    }
    break;
  case JUMP:
    if(totoro.xv>0) {
      SPR_PTR[0]=run_seq[0];
      SPR_PTR[1]=run_seq[1];
      SPR_PTR[2]=run_seq[2];
    } else if (totoro.xv<0) {
      SPR_PTR[0]=run_seq[12];
      SPR_PTR[1]=run_seq[13];
      SPR_PTR[2]=run_seq[14];
    } else {
      // add eye movement based on up or down
      SPR_PTR[0]=0;
      SPR_PTR[1]=2;
      SPR_PTR[2]=3;
    }
    break;
  }
}

void __fastcall__ acorn_set_pos(void)
{
  static unsigned char enmask;

  enmask=0;

  if(acorn[0].en) {
      VIC.spr_pos[4].y=acorn[0].ypos>>8;
      enmask|=0x10;
  }
  if(acorn[1].en) {
      VIC.spr_pos[5].y=acorn[1].ypos>>8;
      enmask|=0x20;
  }
  if(acorn[2].en) {
      VIC.spr_pos[6].y=acorn[2].ypos>>8;
      enmask|=0x40;
  }
  if(acorn[3].en) {
      VIC.spr_pos[7].y=acorn[3].ypos>>8;
      enmask|=0x80;
  }
  
  VIC.spr_ena=(VIC.spr_ena&0x0f)|enmask;
  //enmask=VIC.spr_coll; //reset sprite collision 
}


void __fastcall__ totoro_update(void)
{
  static unsigned int r;
  
  totoro.xpos+=(totoro.xv>>2);


  if(totoro.xpos<MIN_X) {
    totoro.xpos=MIN_X;
    totoro.xv=-1;
  } else if(totoro.xpos>MAX_PX) {
    if(!(gstate.mode&0xfe)) {
      totoro.xpos=MAX_PX;
      totoro.xv=1;
    }
  }
  
  totoro.ypos+=totoro.yv;
  
  if(totoro.state==JUMP) {
    totoro.yv++;
  }
  
  if(totoro.ypos>PGROUND_Y) {
    totoro.ypos=PGROUND_Y;
    if(totoro.xv) totoro.state=RUN;
    else totoro.state=IDLE;
    totoro.yv=0;
  }
  
  if((totoro.xv==0) && (totoro.state!=JUMP))
    totoro.state=IDLE;
  
  if(totoro.blink)
    totoro.blink--;
  
  totoro.idx=(gstate.frame&0xF)>>2;
  
  if(gstate.frame==25) {
    if((totoro.state==IDLE) && (totoro.blink==0)) {
      r=rand();
      if((r&0x3)==0x3) {
	totoro.blink=5;
      }
    } 
  }
}


void __fastcall__ totoro_init(void)
{
  totoro.xpos=(MAX_PX-MIN_X)/2;
  totoro.ypos=PGROUND_Y;
  totoro.xv=0;
  totoro.yv=0;
  totoro.idx=0;
  totoro.state=IDLE;
} 

/*
void __fastcall__ update_acorn_f(unsigned char a)
{						
  if(acorn[a].en) {
    acorn[a].yv+=3;
    acorn[a].ypos+=(acorn[a].yv);
    if((acorn[a].ypos>>8)>GROUND_Y)
      acorn[a].en=0;
  }
}
*/

#define acorn_update_m(a) do {\
    if(acorn[a].en) {\
      acorn[a].yv+=stage[gstate.stage_idx].speed;\
      acorn[a].ypos+=(acorn[a].yv);\
      if((acorn[a].ypos>>8)>GROUND_Y)\
	acorn[a].en=0;\
    }\
} while(0)

void __fastcall__ acorn_update(void)
{
  acorn_update_m(0);
  acorn_update_m(1);
  acorn_update_m(2);
  acorn_update_m(3);
}

void __fastcall__ acorn_init(void)
{
  // init data
  for(ctmp=0;ctmp<4;ctmp++)
    acorn[ctmp].en=0;
}

unsigned char __fastcall__ acorn_find(void)
{
  //  unsigned char c;
  for(ctmp=0;ctmp<4;ctmp++)
    if(acorn[ctmp].en==0) return ctmp;
  return 0x80;
}

void __fastcall__ acorn_add(void)
{
  static unsigned int oldr=0;
  static unsigned int r;
  
  if(MODE_PLAY_DEMO() && (gstate.frame==10 || gstate.frame==27 || gstate.frame==44 ) )  {
      //  if((frame&0xf)==1)  {
      r=rand();
      
      //    if(r<RAND_MAX/2) {
      if(1) {
	unsigned char na;
	// new acorn
	do {
	  r=rand();
	  r&=0x3f;
	} while (abs(r-oldr)<8);
	oldr=r;
	
	if(r<36) {
	  r<<=3;
	  r+=MIN_X;
	  na=acorn_find();
	  if(na!=0x80) {
	    acorn[na].en=1;
	    acorn[na].ypos=ACORN_START_Y<<8;
	    acorn[na].yv=0;
	    //	VIC.spr_color[na+4]=COLOR_ORANGE;
	    //r=(na<<5)+MIN_X;
	    VIC.spr_pos[na+4].x  = r;
	    if(r>255) VIC.spr_hi_x|=(1<<(na+4));
	    else VIC.spr_hi_x&=~(1<<(na+4));
	  }
	}
      }
    }
    
}

void __fastcall__ mode_bitmap(void)
{
  VIC.ctrl1=0x3B; // enable bitmap, no extended color, no blank, 25 rows, ypos=3
  //CIA2.pra=0x03;  // selects VIC page 0xC000-0xFFFF
  
  CIA2.pra=(CIA2.pra&0xfc)|0x2;  // selects VIC page 0x4000-0x7FFF
  //CIA2.pra=(CIA2.pra&0xfc)|0x1;  // selects VIC page 0x8000-0xBFFF
  //CIA2.pra=(CIA2.pra&0xfc)|0x0; // selects VIC page 0xC000-0xFFFF
  
  VIC.addr=0x78;  
  //  VIC.addr=0x78;  // color ram at base +0x1c00, bitmap at base + 0x2000
  VIC.ctrl2=0xD8; // multicolor, 40 cols, xpos=0
  VIC.bgcolor[0]=COLOR_BLACK;
  VIC.spr_ena=0;
}

void __fastcall__ mode_text(void)
{
  VIC.ctrl1=0x1B; // disable bitmap, no extended color, no blank, 25 rows, ypos=3
  CIA2.pra|=0x03;  // selects vic page 0x0000-0x3fff
  VIC.addr=0x16;  // screen base at 0x0400, char def at $0x1400

  VIC.bgcolor[0]=COLOR_WHITE;
}

void __fastcall__ Title_Sprite_Setup(void)
{
  VIC.spr_ena=0;
  
  VIC.spr_color[0]=COLOR_RED; 
  VIC.spr_color[1]=COLOR_RED;
  VIC.spr_color[2]=COLOR_RED;
  VIC.spr_color[3]=COLOR_RED;
  VIC.spr_color[4]=COLOR_BLACK;
  VIC.spr_color[5]=COLOR_BLACK;
  VIC.spr_color[6]=COLOR_BLACK;
  VIC.spr_color[7]=COLOR_RED;

  VIC.spr_mcolor=0x00; // all no multicolor
  VIC.spr_exp_x=0xf0;  // all exp x
  VIC.spr_exp_y=0xf0;  // all exp y

  // GGLABS
  POKE(0x7f8+0,248);
  POKE(0x7f8+1,249);
  POKE(0x7f8+2,250);
  POKE(0x7f8+3,251);
  // totoro64 logo
  POKE(0x7f8+4,252);
  POKE(0x7f8+5,253);
  POKE(0x7f8+6,254);
  POKE(0x7f8+7,255);
  /*
  VIC.spr_pos[0].x=80;
  VIC.spr_pos[1].x=80+48;
  VIC.spr_pos[2].x=80+48*2;
  VIC.spr_pos[3].x=80+48*3;
  */
  VIC.spr_pos[0].x=128;
  VIC.spr_pos[1].x=128+24;
  VIC.spr_pos[2].x=128+24*2;
  VIC.spr_pos[3].x=128+24*3;
  
  VIC.spr_pos[4].x=80;
  VIC.spr_pos[5].x=80+48;
  VIC.spr_pos[6].x=80+48*2;
  VIC.spr_pos[7].x=80+48*3;
  
  VIC.spr_pos[0].y=60;
  VIC.spr_pos[1].y=60;
  VIC.spr_pos[2].y=60;
  VIC.spr_pos[3].y=60;
  
  VIC.spr_pos[4].y=120;
  VIC.spr_pos[5].y=120;
  VIC.spr_pos[6].y=120;
  VIC.spr_pos[7].y=120;
}

void __fastcall__ update_top_bar(void)
{
  // interleave the updates to reduce frame time
#if 0
  switch(gstate.frame&0x07) {
  case 0:
    sprintf(STR_BUF,"SCORE %d",gstate.score);
    break;
  case 1:
      convert_big();
      break;
  case 2:
      printbigat(20,0);
      break;
  case 3:
    sprintf(STR_BUF,"TIME %d  ",gstate.time);
    break;
  case 4:
    convert_big();
    break;
  case 5:
    printbigat(0,0);
    break;
  }
#else
  if(MODE_PLAY_DEMO()) {
    switch(gstate.frame&0x07) {
    case 0:
      sprintf(STR_BUF,"ACORN: %2d  SCORE %d",
	      gstate.acorns, gstate.score);
      break;
    case 1:
      printat(18,0);
      break;
    case 2:
      sprintf(STR_BUF,"ST:%d  ML:%d  SP:%d", gstate.stage,
	      loop1, stage[gstate.stage_idx].speed);
      break;
    case 3:
      printat(18,1);
      break;
      
    case 5:
      sprintf(STR_BUF,"TIME %d  ",gstate.time);
      break;
    case 6:
      convert_big();
      break;
    case 7:
      printbigat(0,0);
      break;
    }
  }
#endif  
}
  
void __fastcall__ game_sprite_setup(void)
{
  VIC.spr_ena=0;

  VIC.spr_color[0]=COLOR_LIGHTBLUE; 
  VIC.spr_color[1]=COLOR_BLACK;
  VIC.spr_color[2]=COLOR_WHITE;
  VIC.spr_color[3]=COLOR_BLACK;
#ifndef DEBUG
  VIC.spr_color[4]=COLOR_ORANGE;
  VIC.spr_color[5]=COLOR_ORANGE;
  VIC.spr_color[6]=COLOR_ORANGE;
  VIC.spr_color[7]=COLOR_ORANGE;
#else
  VIC.spr_color[4]=COLOR_BLACK;
  VIC.spr_color[5]=COLOR_YELLOW;
  VIC.spr_color[6]=COLOR_BLUE;
  VIC.spr_color[7]=COLOR_GREEN;
#endif
  
  VIC.spr_mcolor=0xF4; // leaf is undecided if mcolor
  VIC.spr_exp_x=0x07;
  VIC.spr_exp_y=0x07;

  VIC.spr_mcolor0=COLOR_BROWN;
  VIC.spr_mcolor1=COLOR_YELLOW;
    
  SPR_PTR[0]=0;
  SPR_PTR[1]=2;
  SPR_PTR[2]=3;
  SPR_PTR[3]=2;
  SPR_PTR[4]=28;
  SPR_PTR[5]=28;
  SPR_PTR[6]=28;
  SPR_PTR[7]=28;
}



void __fastcall__ wait_past_score(void)
{
  //  while(VIC.rasterline==EOF_LINE) {};
  //  while(VIC.rasterline!=EOF_LINE && VIC.rasterline!=(EOF_LINE+1) && VIC.rasterline!=(EOF_LINE+2) ) {};
  while(VIC.rasterline<=60) {};
}


void __fastcall__ process_input(void)
{
  static unsigned char key;
  
  if(gstate.mode==GMODE_PLAY) key=PEEK(197);
  else key=18; // simulate 'D'
  
  if(totoro.state!=JUMP)
    switch(key) {
    case 10: // A
      if(totoro.xv>-MAX_XV) {
	totoro.xv-=2;
	if(totoro.xv>0) {
	  totoro.state=BRAKE;
	} else {
	  totoro.state=RUN;
	}
      }
      break;
    case 18: // D
      if(totoro.xv<MAX_XV) {
	totoro.xv+=2;
	if(totoro.xv<0) {
	  totoro.state=BRAKE;
	} else {
	  totoro.state=RUN;
	}
      }
      break;
    case 60: // space
      totoro.yv=-MAX_YV;
      totoro.state=JUMP;
      break;
    default:
      if(totoro.xv>0) {
	totoro.xv--;
	totoro.state=BRAKE;
      } else if(totoro.xv<0) {
	totoro.xv++;
	totoro.state=BRAKE;
      } else {
	totoro.state=IDLE;
      }
      break;
    }  
}

#define check_collision_n(a) do {\
  if(cr&(0x10<<a)) {				\
    if(acorn[a].en && (acorn[a].ypos>>8)>120) {	\
      /*VIC.spr_color[a+4]=0;*/		    \
	acorn[a].en=0;\
	/*	  VIC.spr_ena&=~(0x10<<a) */		\
	gstate.score+=10+(PGROUND_Y-totoro.ypos);			\
	if(gstate.acorns) gstate.acorns--;			\
      }\
    } else { \
    /*	  VIC.spr_color[a+4]=COLOR_ORANGE; */	\
    }	  \
} while (0)

void __fastcall__ delay(unsigned char f)
{
  while(f--)
    waitvsync();
}

void __fastcall__ get_ready(void)
{
  CLR_TOP();
  sprintf(STR_BUF,"STAGE %d",gstate.stage);
  convert_big();
  printbigat(14,0);
  delay(50); 
  //  CLR_TOP();
  sprintf(STR_BUF,"CATCH %d ACORNS",stage[gstate.stage_idx].acorns);
  convert_big();
  printbigat(4,0);
  delay(50);
  
  CLR_TOP();
  sprintf(STR_BUF,"READY");
  convert_big();
  printbigat(14,0);
  delay(20);
  sprintf(STR_BUF," SET ");
  convert_big();
  printbigat(14,0);
  delay(20);
  sprintf(STR_BUF," GO  ");
  convert_big();
  printbigat(14,0);
  delay(20);
  CLR_TOP();
}

void __fastcall__ game_loop(void)
{
  static unsigned char cr;
  
  waitvsync();
  DEBUG_BORDER(COLOR_WHITE);
  //  VIC.bordercolor=COLOR_WHITE;
  
  // screen updates
  totoro_set_pos();
  acorn_set_pos();
  DEBUG_BORDER_INC();
  
  // input processing
  process_input();
  DEBUG_BORDER_INC();
  
  // calculate new totoro position
  totoro_update();
  DEBUG_BORDER_INC();

  // calculate new acorn positions
  acorn_update();
  DEBUG_BORDER_INC();

  acorn_add();
  
  // collision
  cr=VIC.spr_coll;
  check_collision_n(0);
  check_collision_n(1);
  check_collision_n(2);
  check_collision_n(3);
  cr=VIC.spr_coll;
  
  // other screen updates
  DEBUG_BORDER_INC();
  wait_past_score();
  DEBUG_BORDER_INC();
  update_top_bar();
  
    // time
  gstate.frame++;    
  if(gstate.frame==50) {
    if(MODE_PLAY_DEMO()) gstate.time--;
    gstate.frame=0;
  }

  if(gstate.time==20) vpb=7;
  if(gstate.time==10) vpb=6;
  
  // black background for idle time
  DEBUG_BORDER(COLOR_BLACK); 
}

int main()
{
  static unsigned char flag;
  static unsigned int bonus;

  inflatemem (SPR_DATA, sprite_src_data);
  memcpy((unsigned char *)(0x4000-64*8),SPR_DATA+35*64,64*8);
  setup_sid();

  //    printf("v1addr %04x\n",v1addr);
  
  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=0xb0;
  VIC.imr=0x1; // enable raster interrupt

  Title_Sprite_Setup();
  mode_text();
  VIC.spr_ena=0xff;
  

  memcpy((unsigned char *)(0x400+40*6+15),"presents",8);
  memcpy((unsigned char *)(0x400+40*16),"A Commodore 64 tribute to Studio Ghibli",39);
  
  //  printf("\n\n\n\n\n\n\n               presents\n\n\n\n\n\n\n\n");
  //  printf("A Commodore 64 tribute to Studio Ghibli\n");
  // printf("Copyright (c) 2021 Gabriele Gorla\n\n");
  /*  printf("This program is free software: you can\n");
  printf("redistribute it and/or modify it under\n");
  printf("the terms of the GNU General Public\n");
  printf("License as published by the Free\n");
  printf("Software Foundation either License\n");
  printf("version 3 or (at your option) any later\nversion.\n\n");
  */

  //#ifdef DEBUG
  // printf("STR_BUF at 0x%04X\n",STR_BUF);
  // printf("sprite data at 0x%04X\n",SPR_DATA);
#ifdef DEBUG
  printf("bitmap at 0x%04X\n",SCR_BASE);
#endif
  inflatemem (SCR_BASE, bitmap_data);

#ifdef DEBUG
  printf("color1 at 0x%04X\n",COLOR_BASE);
  printf("color2 at 0x%04X\n",0xd800);
#endif
  inflatemem (COLOR_BASE, color1_data);
  //     printf("v1addr %04x\n",v1addr);
  //  printf("\n\n\nv1addr %04x\n",v1addr);
  //    printf("v1addr %04x\n",v1addr);

  #if 0

  for(flag=0;flag<32;flag++) {
    switch(flag&3) {
    case 0:
      VIC.spr_pos[4].y=120;
      VIC.spr_pos[5].y=120+21;
      VIC.spr_pos[6].y=120+21;
      VIC.spr_exp_y=0x10;
      break;
    case 1:
      VIC.spr_pos[4].y=120+21;
      VIC.spr_pos[5].y=120;
      VIC.spr_pos[6].y=120+21;
      VIC.spr_exp_y=0x20;
      break;
    case 2:
      VIC.spr_pos[4].y=120+21;
      VIC.spr_pos[5].y=120+21;
      VIC.spr_pos[6].y=120;
      VIC.spr_exp_y=0x40;
      break;
    case 3:
      VIC.spr_pos[4].y=120+21;
      VIC.spr_pos[5].y=120+21;
      VIC.spr_pos[6].y=120+21;
      VIC.spr_exp_y=0x00;
      break;
    }
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
    waitvsync();
  }
#endif
  
  cgetc();

  inflatemem ((unsigned char *)0xd800, color2_data);
  //#endif
  //    cgetc();
  // setup the NMI vector
  //  *((unsigned int *)0x0318)=(unsigned int)NMI;
  
  mode_bitmap();
    
  for(;;) { // main loop
    //    cgetc();
    while(PEEK(197)!=60);

    // game init
    gstate.score=0;
    gstate.stage=1;
    gstate.mode=GMODE_CUT1;

    do {

    // stage init
      vpb=8;
    gstate.time=0;
    gstate.frame=0;
    gstate.stage_idx=(gstate.stage<(sizeof(stage)/sizeof(struct stage_t)))?(gstate.stage-1):((sizeof(stage)/sizeof(struct stage_t)))-1;

    gstate.time=stage[gstate.stage_idx].time;
    gstate.acorns=stage[gstate.stage_idx].acorns;
      
      game_sprite_setup();
      totoro_init();
      acorn_init();
      totoro_set_pos();
      
      VIC.spr_ena=0x07;
      get_ready();
      gstate.mode=GMODE_PLAY;
      

      for(;gstate.time&&gstate.acorns;)
	game_loop();
      
      gstate.mode=GMODE_CUT1;
      if(gstate.acorns==0) {
	flag=1;
	CLR_TOP();
	next_loop1=rand()&0xe;
	//	vpb=7+(rand()&1);
	sprintf(STR_BUF,"STAGE CLEAR");
	convert_big();
	printbigat(8,0);
      } else {
	flag=0;
      }

      // make totoro walk away
      for(;totoro.xpos<350;)
	game_loop();

      delay(25);

      if(flag) {
	CLR_TOP();
	bonus=0;
	do {
	  sprintf(STR_BUF,"TIME %d ",gstate.time);
	  convert_big();
	  printbigat(0,0);
	  sprintf(STR_BUF,"BONUS %d",bonus);
	  convert_big();
	  printbigat(20,0);
	  delay(2);
	  bonus+=5+gstate.stage;
	} while (gstate.time--);
	delay(25);
	gstate.score+=bonus;
	gstate.stage++;
      }

    } while(flag);

    // game over
    sprintf(STR_BUF,"GAME OVER");
    convert_big();
    printbigat(0,0);
    delay(200);
  }
  
  return 0;
}
