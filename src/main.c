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

#define DEBUG

#define VERSION "0.12"

#define HFREQ 50

#define STAGE_TIME 60

#define EOF_LINE 0x80

// sprite max speed
#define MAX_XV 16
#define MAX_YV 10

// sprite position constants/limits
#define GROUND_Y 220
#define PGROUND_Y (GROUND_Y-21)

#define ACORN_START_Y 68

#define MIN_X 32
#define MAX_X 312
#define MAX_PX (MAX_X-24)

// title bar positions
#define TIMETXT_X 0
#define TIMEVAL_X 9

#define ACORNTXT_X 22
#define ACORNVAL_X 35

#define SCORETXT_X 15
#define SCOREVAL_X 15

// debug location
#define DEBUG_TXT_LEN 12
#define DEBUG_TXT_X   (40-DEBUG_TXT_LEN)


void __fastcall__ CLR_TOP(void);

#ifdef DEBUG
#define DEBUG_BORDER(a) VIC.bordercolor=a
#define DEBUG_BORDER_INC() VIC.bordercolor++
#else
#define DEBUG_BORDER(a)
#define DEBUG_BORDER_INC()
#endif

const unsigned char txt_score[] = "SCORE";
const unsigned char txt_time[] = "TIME";
const unsigned char txt_bonus[] = "BONUS";
const unsigned char txt_acorns[] = "ACORNS";
const unsigned char txt_ready[] = "READY";
const unsigned char txt_set[] = " SET ";
const unsigned char txt_go[] = " GO ";
const unsigned char txt_clear[] = "STAGE CLEAR";
const unsigned char txt_game_over[] = "GAME OVER";
extern const unsigned char present_txt[];
extern const unsigned char intro_txt[];
extern const unsigned char license_txt[];

#define strcpy8(dst,src)       \
  __asm__("ldx #$FF");         \
  __asm__("ls%v: inx",src);    \
  __asm__("lda %v,x",src);     \
  __asm__("sta %v,x",dst);     \
  __asm__("bne ls%v",src);

const uint8_t run_seq[] =
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

const uint16_t sound_seq[] = {
  0x22d0,
  0x1f04,
  0x2714,
  0x2e79,
  0x2714,
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

#define LAST_STAGE_IDX() ((sizeof(stage)/sizeof(struct stage_t))-1)

struct game_state_t gstate;
struct player_t totoro;
struct acorn_t acorn[4];
struct sound_t sound;

void __fastcall__ setup_sid(void)
{
  memset(&SID,0,24);
  SID.amp = 0x1f; // set volume to 15
  SID.v1.ad = 0x80;
  SID.v1.sr = 0xf6;
  loop1=0;
  t1ptr=track1;
  //  instr1=0x10; // triangular
  instr1=0;
  time1 = 0;
  vpb=8;

  //  SID.v3.ctrl= 0x20;
  SID.v3.ad = 0x00;
  SID.v3.sr = 0xa9;
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
  static uint8_t enmask;

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
}

void __fastcall__ totoro_update(void)
{
  static uint16_t r;
  
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
  totoro.blink=0;
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
  static unsigned char na;
	
  if(MODE_PLAY_DEMO() && (gstate.frame==10 || gstate.frame==27 || gstate.frame==44 ) )  {
      //  if((frame&0xf)==1)  {
      r=rand();
      
      //    if(r<RAND_MAX/2) {
      //      if(1) {

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
	//   }
    }
    
}

void __fastcall__ mode_bitmap(void)
{
  VIC.ctrl1=0x3B; // enable bitmap, no extended color, no blank, 25 rows, ypos=3
  //CIA2.pra=0x03;  // selects VIC page 0x0000-0x3FFF
  
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
  VIC.spr_exp_x=0xf0;  // totoro64 exp x
  VIC.spr_exp_y=0xf0;  // totoro64 exp y

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

void __fastcall__ wait_past_score(void)
{
  while(VIC.rasterline<=60) {};
}

void __fastcall__ setup_top_bar(uint8_t flag)
{
  CLR_TOP();

  strcpy8(STR_BUF,txt_time);
  convprint_big(TIMETXT_X);

  if(flag==0) {
    strcpy8(STR_BUF,txt_acorns);
  } else {
    strcpy8(STR_BUF,txt_bonus);
  }
  convprint_big(ACORNTXT_X);

  strcpy8(STR_BUF,txt_score);
  printat(SCORETXT_X,0);
}

void __fastcall__ update_top_bar(void)
{
  if(gstate.frame>=16*3) return;
  // interleave the updates to reduce frame time
  if(MODE_PLAY_DEMO()) {
    switch(gstate.frame&0x0F) {
    case 0:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"%2d", gstate.acorns);
      break;
    case 1:
      DEBUG_BORDER_INC();
      convert_big();
      break;
    case 2:
      wait_past_score();
      DEBUG_BORDER_INC();
      printbigat(ACORNVAL_X,0);
      break;
    case 4:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"%2d", gstate.time-1);
      break;
    case 5:
      DEBUG_BORDER_INC();
      convert_big();
      break;
    case 6:
      wait_past_score();
      DEBUG_BORDER_INC();
      printbigat(TIMEVAL_X,0);
      break;
    case 7:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"%5d",
	      gstate.score);
      break;
    case 8:
      wait_past_score();
      DEBUG_BORDER_INC();
      printat(SCOREVAL_X,1);
      break;
#ifdef DEBUG
    case 10:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ST:%2d IDX:%2d", gstate.stage, gstate.stage_idx);
      break;
    case 11:
      wait_past_score();
      DEBUG_BORDER_INC();
      printat(DEBUG_TXT_X,2);
      break;
    case 12:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ML:%2d SPD:%2d", loop1, stage[gstate.stage_idx].speed);
      break;
    case 13:
      wait_past_score();
      DEBUG_BORDER_INC();
      printat(DEBUG_TXT_X,3);
      break;
#endif
    default:
      DEBUG_BORDER_INC();
      break;
    }
  }
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

void __fastcall__ process_input(void)
{
  static uint8_t key;
  static uint8_t js;
  
  if(gstate.mode==GMODE_PLAY) {
    key=PEEK(197);
    js=joy2();
    if(js) {
      if(js&0x04) key = 10; // left
      if(js&0x08) key = 18; // right
      if(js&0x10) key = 60; // button
      if(js&0x01) key = 60; // up?
    }
  } else key=18; // simulate 'D'
  
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

void __fastcall__ process_sound(void)
{
    if(sound.timer==0) {
      SID.v3.ctrl=0x20;
      return;
    }
    if(sound.index==0) {
      SID.v3.freq=sound_seq[0];
      SID.v3.ctrl=0x21;
    }
    sound.index++;
    if(sound.index>=(sizeof(sound_seq)/2)) sound.index=1;
    SID.v3.freq=sound_seq[sound.index];
    sound.timer--;

}

#define stop_sound() \
  do { SID.v3.ctrl=0x20; } while(0)
  
void __fastcall__ start_sound(void)
{
  SID.v3.ctrl=0x20;
  if(MODE_PLAY_DEMO()) {
    sound.timer=10;
    sound.index=0;
  }
}

#define check_collision_n(a) do {\
  if(cr&(0x10<<a)) {				\
    if(acorn[a].en && (acorn[a].ypos>>8)>120) {	\
      /*VIC.spr_color[a+4]=0;*/		    \
	acorn[a].en=0;\
	start_sound(); \
	/*	  VIC.spr_ena&=~(0x10<<a) */		\
	gstate.score+=10+(PGROUND_Y-totoro.ypos);			\
	if(gstate.acorns) gstate.acorns--;			\
      }\
    } else { \
    /*	  VIC.spr_color[a+4]=COLOR_ORANGE; */	\
    }	  \
} while (0)

void __fastcall__ delay(uint8_t f)
{
  while(f--)
    waitvsync();
}

void __fastcall__ get_ready(void)
{
  CLR_TOP();
  sprintf(STR_BUF,"STAGE %d",gstate.stage);
  convprint_big(14);
  delay(50); 
  sprintf(STR_BUF,"CATCH %d ACORNS",stage[gstate.stage_idx].acorns);
  convprint_big(4);
  delay(50);
  
  CLR_TOP();
  strcpy8(STR_BUF,txt_ready);
  convprint_big(14);
  delay(20);
  strcpy8(STR_BUF,txt_set);
  convprint_big(14);
  delay(20);
  strcpy8(STR_BUF,txt_go);
  convprint_big(15);
  delay(20);
}

void __fastcall__ game_loop(void)
{
  static uint8_t cr;
  
  waitvsync();
  DEBUG_BORDER(COLOR_WHITE);
  
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

  process_sound();

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
  update_top_bar();
  
  // time
  gstate.frame++;    
  if(gstate.frame==HFREQ) {
    if(MODE_PLAY_DEMO()) gstate.time--;
    gstate.frame=0;
  }

  // speed up music
  if(gstate.time==20) vpb=7;
  if(gstate.time==10) vpb=6;

  
  // black background for idle time
  DEBUG_BORDER(COLOR_BLACK); 
}

int main()
{
  static uint8_t flag;
  static uint16_t bonus;

  inflatemem (SPR_DATA, sprite_src_data);
  memcpy((uint8_t *)(0x4000-64*8),SPR_DATA+35*64,64*8);
  setup_sid();

  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=0xb0;
  VIC.imr=0x1; // enable raster interrupt

  Title_Sprite_Setup();
  mode_text();
  VIC.spr_ena=0xff;
  

  //  memcpy((uint8_t *)(0x400+40*6+15),"presents",8);
  //  memcpy((uint8_t *)(0x400+40*16),"A Commodore 64 tribute to Studio Ghibli",39);

  memcpy((uint8_t *)(0x400+40*6+15),present_txt,8);
  memcpy((uint8_t *)(0x400+40*16),intro_txt,39);
#ifndef DEBUG
  memcpy((uint8_t *)(0x400+40*17),license_txt,7*40+8);
#endif
  
  inflatemem (SCR_BASE, bitmap_data);
  inflatemem (COLOR_BASE, color1_data);

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
    delay(15);
  }
#endif
  
  cgetc();
  
  inflatemem ((uint8_t *)0xd800, color2_data);
  //    cgetc();
  mode_bitmap();

#ifdef DEBUG
  for(ctmp=0;ctmp<DEBUG_TXT_LEN;ctmp++)
    {
      POKE(0xd800+40*2+DEBUG_TXT_X+ctmp,1);
      POKE(0xd800+40*3+DEBUG_TXT_X+ctmp,1);
      POKE(0xd800+40*4+DEBUG_TXT_X+ctmp,1);
      POKE(0xd800+40*5+DEBUG_TXT_X+ctmp,1);
      POKE(0xd800+40*6+DEBUG_TXT_X+ctmp,1);
      POKE(0xd800+40*7+DEBUG_TXT_X+ctmp,1);
      
    }

  sprintf(STR_BUF,"Sprdat $%04X",SPR_DATA);
  printat(DEBUG_TXT_X,2);
  sprintf(STR_BUF,"Color1 $%04X",COLOR_BASE);
  printat(DEBUG_TXT_X,3);
  sprintf(STR_BUF,"Bitmap $%04X",SCR_BASE);
  printat(DEBUG_TXT_X,4);
  sprintf(STR_BUF,"STRBUF $%04X",STR_BUF);
  printat(DEBUG_TXT_X,5);
  /*sprintf(STR_BUF,"Color2 $%04X",0xd800);
  printat(DEBUG_TXT_X,6);
   sprintf(STR_BUF,"T1addr $%04X",t1addr);
     printat(DEBUG_TXT_X,7);*/
#endif
  
  for(;;) { // main loop
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

      gstate.stage_idx=(gstate.stage>LAST_STAGE_IDX()) ?
	LAST_STAGE_IDX() : gstate.stage-1 ;
      
      gstate.time=stage[gstate.stage_idx].time;
      gstate.acorns=stage[gstate.stage_idx].acorns;
      
      game_sprite_setup();
      totoro_init();
      acorn_init();
      totoro_set_pos();
      
      VIC.spr_ena=0x07;
      get_ready();
      gstate.mode=GMODE_PLAY;
      setup_top_bar(0);

      for(;gstate.time&&gstate.acorns;)
	game_loop();

      //      stop_sound();
      
      gstate.mode=GMODE_CUT1;
      if(gstate.acorns==0) {
	flag=1;
	CLR_TOP();
	next_loop1=rand()&0xe;
	strcpy8(STR_BUF,txt_clear);
	convprint_big(8);
      } else {
	flag=0;
      }

      // make totoro walk away
      for(;totoro.xpos<350;)
	game_loop();

      delay(25);

      if(flag) {
	bonus=0;
	setup_top_bar(1);
	do {
	  sprintf(STR_BUF,"%2d", gstate.time);
	  convprint_big(TIMEVAL_X);
	  sprintf(STR_BUF,"%d", bonus);
	  convprint_big(ACORNVAL_X-2);
	  sprintf(STR_BUF,"%5d", gstate.score);
	  printat(SCOREVAL_X,1);
	  delay(1);
	  bonus+=5+gstate.stage;
	  gstate.score+=5+gstate.stage;
	} while (gstate.time--);
	delay(25);
	gstate.stage++;
      }

    } while(flag);

    // game over
    strcpy8(STR_BUF,txt_game_over);
    convprint_big(0);
    delay(200);
  }
  
  return 0;
}
