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
 *  TOTORO64 is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *  GNU General Public License for more details.                              *
 *                                                                            *
 *  You should have received a copy of the GNU General Public License         *
 *  along with TOTORO64.  If not, see <http://www.gnu.org/licenses/>.         *
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

#define VERSION "0.16"

#define STAGE_TIME 60

#define EOF_LINE 0x80

// sprite max speed
#ifdef NTSC
#define MAX_XV 13
// should be 8 with .66 acceleration
// approximate to 9 and .75 acceleration
#define JUMP_V 9
#else
#define MAX_XV 16
#define JUMP_V 10
#endif

// sprite position constants/limits
#define GROUND_Y 220
#define PGROUND_Y (GROUND_Y-21)

#define ACORN_START_Y 68

#define MIN_X 32
#define MAX_X 312
#define MAX_PX (MAX_X-24)

#define SPR_CENTER_X 184

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

#if (DEBUG & DEBUG_TIMING)
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
const unsigned char txt_catch[] = "CATCH    ";
const unsigned char txt_stage[] = "STAGE ";
const unsigned char txt_ready[] = "READY";
const unsigned char txt_set[] = " SET ";
const unsigned char txt_go[] = " GO ";
const unsigned char txt_clear[] = "CLEAR";
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

#define strcpy8v(dst,src)      \
  __asm__("ldx #$FF");         \
  __asm__("ls%v: inx",src);    \
  __asm__("lda %v,x",src);     \
  __asm__("sta %w,x",dst);     \
  __asm__("bne ls%v",src);

const uint8_t run_seq[] =
  {
    4,7,10,7, // run right
    16,19,22,19, // run left
  };

const uint16_t sound_seq[] = {
  0x22d0,
  0x1f04,
  0x2714,
  0x2e79,
  0x2714,
};

#ifdef NTSC
const struct stage_t stage[] =
  {
   { STAGE_TIME, 10, 3, 0 },
   { STAGE_TIME, 15, 4, 0 },
   { STAGE_TIME, 20, 6, 0 },
   { STAGE_TIME, 25, 7, 0 },
   { STAGE_TIME, 30, 8, 0 },
   { STAGE_TIME, 35, 10, 0 },
   { STAGE_TIME, 40, 11, 0 },
   { STAGE_TIME, 50, 13, 0 },
   { STAGE_TIME, 60, 15, 0 },
};
#else
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
   { STAGE_TIME, 60, 21, 0 },
};
#endif

#define LAST_STAGE_IDX() ((sizeof(stage)/sizeof(struct stage_t))-1)

struct game_state_t gstate;
struct player_t totoro;
struct acorn_t acorn[MAX_ACORNS];
struct sound_t sound;

struct track_t track[2];
uint8_t vpb;


uint8_t STR_BUF[64];

uint8_t spr_mux;
uint8_t cr;


int __fastcall__ utoa10 (uint16_t val);

void __fastcall__ _strcpy8f (void)
{
  __asm__("ldy #$FF");
  __asm__("strloop: iny");
  __asm__("lda (_temp_ptr),y");
  __asm__("sta _STR_BUF,y");
  __asm__("bne strloop");
}

#define strcpy8f(a) do { \
    temp_ptr=(unsigned char *)a;		\
  _strcpy8f();\
  } while (0)

void __fastcall__ setup_sid(void)
{
  memset(&SID,0,24);
  SID.amp = 0x1f; // set volume to 15
  SID.v1.ad = 0x80;
  SID.v1.sr = 0xf6;
  track[0].ptr=(uint16_t)track0_data;
  track[0].restart_ptr=(uint16_t)track0_data;
  track[0].track_offset=0;
  track[0].global_offset=4;
  track[0].next_offset=4;
  track[0].timer=0;
  track[0].voice_offset=0;

  SID.v2.ad = 0x40;
  SID.v2.sr = 0x56; 
  track[1].ptr=(uint16_t)track1_data;
  track[1].restart_ptr=(uint16_t)track1_data;
  track[1].track_offset=0;
  track[1].global_offset=0;
  track[1].next_offset=0;
  track[1].timer=0;
  track[1].voice_offset=7;

  vpb = VPB;

  //  SID.v3.ctrl= 0x20;
  SID.v3.ad = 0x00;
  SID.v3.sr = 0xa9;
}

void __fastcall__ totoro_set_pos(void)
{
  VIC.spr0_x   = totoro.xpos.lo;
  VIC.spr1_x   = totoro.xpos.lo;
  VIC.spr2_x   = totoro.xpos.lo;

  if(totoro.xpos.hi)   VIC.spr_hi_x |=0x07;
  else VIC.spr_hi_x &= 0xf8;

  VIC.spr0_y   = totoro.ypos.lo;
  VIC.spr1_y   = totoro.ypos.lo;
  VIC.spr2_y   = totoro.ypos.lo;

  switch(totoro.state) {
  case IDLE:
    if(totoro.blink)   SPR_PTR[0]=3;
    else   SPR_PTR[0]=0;
    SPR_PTR[1]=1;
    SPR_PTR[2]=2;
    break;
  case RUN:
    if(totoro.xv>0) {
      SPR_PTR[0]=run_seq[totoro.idx];
      SPR_PTR[1]=run_seq[totoro.idx]+1;
      SPR_PTR[2]=run_seq[totoro.idx]+2;
    } else {
      SPR_PTR[0]=run_seq[totoro.idx+4];
      SPR_PTR[1]=run_seq[totoro.idx+4]+1;
      SPR_PTR[2]=run_seq[totoro.idx+4]+2;
    }
    break;
  case BRAKE:
    if(totoro.xv>0) {
      SPR_PTR[0]=13;
      SPR_PTR[1]=14;
      SPR_PTR[2]=15;
    } else {
      SPR_PTR[0]=25;
      SPR_PTR[1]=26;
      SPR_PTR[2]=27;
    }
    break;
  case JUMP:
    if(totoro.xv>0) {
      SPR_PTR[0]=run_seq[0];
      SPR_PTR[1]=run_seq[0]+1;
      SPR_PTR[2]=run_seq[0]+2;
    } else if (totoro.xv<0) {
      SPR_PTR[0]=run_seq[4];
      SPR_PTR[1]=run_seq[4]+1;
      SPR_PTR[2]=run_seq[4]+2;
    } else {
      // add eye movement based on up or down
      SPR_PTR[0]=0;
      SPR_PTR[1]=1;
      SPR_PTR[2]=2;
    }
    break;
  }
}

void __fastcall__ totoro_update(void)
{
  static uint16_t r;
 
  totoro.xpos.val+=(totoro.xv>>2);

  if(totoro.xpos<MIN_X) {
    totoro.xpos.val=MIN_X;
    totoro.xv=-1;
  } else if(totoro.xpos>MAX_PX) {
    if(!(gstate.mode&0xfe)) {
      totoro.xpos.val=MAX_PX;
      totoro.xv=1;
    }
  }

  totoro.ypos.val+=totoro.yv.val;

  if(totoro.state==JUMP) {
    totoro.yv++;
  }

  if(totoro.ypos>PGROUND_Y) {
    totoro.ypos.val=PGROUND_Y;
    if(totoro.xv) totoro.state=RUN;
    else totoro.state=IDLE;
    totoro.yv.val=0;
  }

  if((totoro.xv==0) && (totoro.state!=JUMP))
    totoro.state=IDLE;

  if(totoro.blink)
    totoro.blink--;

  totoro.idx=(gstate.counter&0xF)>>2;

  if(gstate.field==25) {
    if((totoro.state==IDLE) && (totoro.blink==0)) {
      r=rand();
      if((r&0x3)==0x3) {
	totoro.blink=VFREQ/10;
      }
    }
  }
}


void __fastcall__ totoro_init(void)
{
  totoro.xpos.val=(MAX_PX-MIN_X)/2;
  totoro.ypos.val=PGROUND_Y;
  totoro.xv=0;
  totoro.yv.val=0;
  totoro.idx=0;
  totoro.state=IDLE;
  totoro.blink=0;
}

#define acorn_update_m(a) do {		  	 \
    if(acorn[a].ypos.val) {				 \
      acorn[a].yv.val+=stage[gstate.stage_idx].speed;\
      acorn[a].ypos.val+=(acorn[a].yv.val);\
      if((acorn[a].ypos.hi)>GROUND_Y) {\
      	acorn[a].en=0;				 \
	acorn[a].ypos.val=0; \
	} \
    }						 \
} while(0)

void __fastcall__ acorn_update(void)
{
  //  static uint8_t t;
  //  t=PEEK(0xd020);
  //  POKE(0xd020,9);
  acorn_update_m(0);
  acorn_update_m(1);
  acorn_update_m(2);
  acorn_update_m(3);
  acorn_update_m(4);
  acorn_update_m(5);
  acorn_update_m(6);
  acorn_update_m(7);
/*
  for(ctmp=0;ctmp<MAX_ACORNS;ctmp++) {
    if(acorn[ctmp].ypos.val) {
      acorn[ctmp].yv.val+=stage[gstate.stage_idx].speed;
      acorn[ctmp].ypos.val+=(acorn[ctmp].yv.val);
      if((acorn[ctmp].ypos.hi)>GROUND_Y) {
      	acorn[ctmp].en=0;
	acorn[ctmp].ypos.val=0;
	}
    } */

/*    if(acorn[ctmp].en) {
      acorn[ctmp].yv.val+=stage[gstate.stage_idx].speed;
      acorn[ctmp].ypos.val+=(acorn[ctmp].yv.val);
      if((acorn[ctmp].ypos.hi)>GROUND_Y)
	acorn[ctmp].en=0;
    }
  }*/
//  POKE(0xd020,t);
}

void __fastcall__ acorn_init(void)
{
  // init data
  memset8(acorn,0,sizeof(acorn));
}

unsigned char __fastcall__ acorn_free_slot(void)
{
  if( (acorn[MAX_ACORNS-1].en==0) &&
      (acorn[0].ypos.hi)>(ACORN_START_Y+2) &&
     ( (acorn[MUX_SLOTS-1].ypos.hi>ACORN_START_Y+23) || (acorn[MUX_SLOTS-1].ypos.val==0) /*|| (acorn[2].ypos.val==0)*/) ) return 1;
  if(acorn[0].en==0) return 1;
  return 0;
}

void __fastcall__ acorn_add(void)
{
  static unsigned int oldr=0;
  static unsigned int r;

  // maybe change to counter
  if(MODE_PLAY_DEMO() && 
      (gstate.field==10 || gstate.field==27 || gstate.field==44 ) )  {
      //  if((frame&0xf)==1)  {
    //    r=rand();
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
      if(acorn_free_slot()) {
	// shift acorn data 
	__asm__("ldx #(%b*%b)",MAX_ACORNS-1,sizeof(struct acorn_t));
	__asm__("loop1: lda %v-1,x",acorn);
	__asm__("sta %v+%b-1,x",acorn,sizeof(struct acorn_t));
	__asm__("dex");
	__asm__("bne loop1");
//	__asm__("lda $d01e");

	acorn[0].en=1;
	acorn[0].xpos.val=r;
	acorn[0].ypos.val=ACORN_START_Y<<8;
	acorn[0].yv.val=4;
	acorn[0].spr_ptr=(r&0x08)?SPR_ACORN_LG:SPR_ACORN_SM;
	    //	VIC.spr_color[na+4]=COLOR_ORANGE;
	  }
	}
	//   }
    }
}

void __fastcall__ mode_bitmap(void)
{
  CIA2.pra=(CIA2.pra&0xfc)|0x2;  // selects VIC page 0x4000-0x7FFF

  VIC.ctrl1=0x3B; // enable bitmap, no extended color, no blank, 25 rows, ypos=3
  VIC.addr =0x78; // color ram at base +0x1c00, bitmap at base + 0x2000
  VIC.ctrl2=0xD8; // multicolor, 40 cols, xpos=0
  VIC.bgcolor[0]=COLOR_BLACK;
  VIC.spr_ena=0;
}

void __fastcall__ mode_text(void)
{
  CIA2.pra|=0x03; // selects vic page 0x0000-0x3fff

  VIC.ctrl1=0x1B; // disable bitmap, no extended color, no blank, 25 rows, ypos=3
  VIC.addr= 0x16; // screen base at 0x0400, char def at $0x1400
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

  __asm__("ldx #248");
  __asm__("loopt: txa");
  __asm__("sta $7f8-248,x");
  __asm__("inx");
  __asm__("bne loopt");
  
  /*
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
  */
  
  VIC.spr_pos[0].x=SPR_CENTER_X-48;
  VIC.spr_pos[1].x=SPR_CENTER_X-24;
  VIC.spr_pos[2].x=SPR_CENTER_X;
  VIC.spr_pos[3].x=SPR_CENTER_X+24;

  VIC.spr_pos[4].x=SPR_CENTER_X-96;
  VIC.spr_pos[5].x=SPR_CENTER_X-48;
  VIC.spr_pos[6].x=SPR_CENTER_X;
  VIC.spr_pos[7].x=SPR_CENTER_X+48;

  VIC.spr_pos[0].y=60;
  VIC.spr_pos[1].y=60;
  VIC.spr_pos[2].y=60;
  VIC.spr_pos[3].y=60;

  VIC.spr_pos[4].y=105;
  VIC.spr_pos[5].y=105;
  VIC.spr_pos[6].y=105;
  VIC.spr_pos[7].y=105;
}

void __fastcall__ wait_past_score(void)
{
  //  VIC.bordercolor=COLOR_WHITE;
  while(VIC.rasterline<=60) {};
  //  VIC.bordercolor=COLOR_BLACK;
}

void __fastcall__ setup_top_bar(uint8_t flag)
{
  CLR_TOP();

  //  strcpy8(STR_BUF,txt_time);
  strcpy8f(txt_time);
  convprint_big(TIMETXT_X);

  if(flag==0) {
    //    strcpy8(STR_BUF,txt_acorns);
    temp_ptr=(unsigned char *)txt_acorns;
  } else {
    //    strcpy8(STR_BUF,txt_bonus);
    temp_ptr=(unsigned char *)txt_bonus;
  }
  _strcpy8f();
  convprint_big(ACORNTXT_X);

  //strcpy8(STR_BUF,txt_score);
  strcpy8f(txt_score);
  printat(SCORETXT_X,0);
}

void __fastcall__ update_top_bar(void)
{
  // interleave the updates to reduce frame time
  if(MODE_PLAY_DEMO()) {
#if 1
    switch(gstate.counter&0x0F) {
    case 0:
      DEBUG_BORDER_INC();
      //      sprintf(STR_BUF,"%2d", gstate.acorns);
      utoa10(gstate.acorns);
      string_pad(2);
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
      //      sprintf(STR_BUF,"%2d", gstate.time-1);
      utoa10(gstate.time-1);
      string_pad(2);
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
      //      sprintf(STR_BUF,"%5d", gstate.score);
      utoa10(gstate.score);
      string_pad(5);
      break;
    case 8:
      wait_past_score();
      DEBUG_BORDER_INC();
      printat(SCOREVAL_X,1);
      break;
#if (DEBUG&DEBUG_INFO)
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
#else
    sprintf(STR_BUF,"%04x %02x %02x ", track[0].ptr, *(uint8_t *)(track[0].ptr),
	    *(uint8_t *)(track[0].ptr+1));
    printat(DEBUG_TXT_X,2);
    //    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[4].ypos.hi, acorn[5].ypos.hi,
    //	    acorn[6].ypos.hi, acorn[7].ypos.hi);
    //    printat(DEBUG_TXT_X,3);
    /*
    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[0].ypos.hi, acorn[1].ypos.hi,
	    acorn[2].ypos.hi, acorn[3].ypos.hi);
    printat(DEBUG_TXT_X,2);
    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[4].ypos.hi, acorn[5].ypos.hi,
	    acorn[6].ypos.hi, acorn[7].ypos.hi);
    printat(DEBUG_TXT_X,3);
    */
    /*    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[0].en, acorn[0].spr_ptr,
	    acorn[1].en, acorn[1].spr_ptr);
    printat(DEBUG_TXT_X,2);
    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[2].en, acorn[2].spr_ptr,
	    acorn[3].en, acorn[3].spr_ptr);
	    printat(DEBUG_TXT_X,3);*/

    /*    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[0].spr_ptr, acorn[1].spr_ptr,
	    acorn[2].spr_ptr, acorn[3].spr_ptr);
    printat(DEBUG_TXT_X,4);
    sprintf(STR_BUF,"%02x %02x %02x %02x ", acorn[4].spr_ptr, acorn[5].spr_ptr,
	    acorn[6].spr_ptr, acorn[7].spr_ptr);
    printat(DEBUG_TXT_X,5);
    */
#endif
      /*
    STR_BUF[1]=hexdigit[(gstate.counter&0x0f)];
    STR_BUF[0]=hexdigit[(gstate.counter>>4)&0x0f];
    STR_BUF[2]=' ';
    STR_BUF[3]=' ';
    STR_BUF[5]=hexdigit[(gstate.field&0x0f)];
    STR_BUF[4]=hexdigit[(gstate.field>>4)&0x0f];
    printat(DEBUG_TXT_X,6);
    */
  }
}

void __fastcall__ game_sprite_setup(void)
{
  VIC.spr_ena=0;

  VIC.spr_color[0]=COLOR_LIGHTBLUE;
  VIC.spr_color[1]=COLOR_BLACK;
  VIC.spr_color[2]=COLOR_WHITE;
  VIC.spr_color[3]=COLOR_BLACK;
#if (DEBUG&DEBUG_ACORNS)
  VIC.spr_color[4]=COLOR_BLACK;
  VIC.spr_color[5]=COLOR_YELLOW;
  VIC.spr_color[6]=COLOR_BLUE;
  VIC.spr_color[7]=COLOR_GREEN;
#else
  VIC.spr_color[4]=COLOR_ORANGE;
  VIC.spr_color[5]=COLOR_ORANGE;
  VIC.spr_color[6]=COLOR_ORANGE;
  VIC.spr_color[7]=COLOR_ORANGE;
#endif

  VIC.spr_mcolor=0xF4; // leaf is undecided if mcolor
  VIC.spr_exp_x=0x07;
  VIC.spr_exp_y=0x07;

  VIC.spr_mcolor0=COLOR_BROWN;
  VIC.spr_mcolor1=COLOR_YELLOW;

  SPR_PTR[0]=0;
  SPR_PTR[1]=2;
  SPR_PTR[2]=3;
  //SPR_PTR[3]=2;
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
      totoro.yv.val=-JUMP_V;
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

#define stop_sound() \
  do { SID.v3.ctrl=0x20; } while(0)

void __fastcall__ start_sound(void)
{
  stop_sound();
  if(MODE_PLAY_DEMO()) {
    sound.timer=10;
    sound.index=0;
  }
}

void __fastcall__ process_sound(void)
{
    if(sound.timer==0) {
      stop_sound();
      return;
    }
    SID.v3.freq=sound_seq[sound.index];
    
    if(sound.index==0) {
      SID.v3.ctrl=0x21;
    }
    //    VIC.bordercolor=sound.index+1;
    // use index 0 only the first time
    if(sound.index>=(sizeof(sound_seq)/2)) sound.index=0;
    sound.index++;
    sound.timer--;
}

void __fastcall__ check_collision(void)
{
  //  static uint8_t color;

  for(ctmp=0;ctmp<MAX_ACORNS;ctmp++) {
    //    color=COLOR_ORANGE;
    if(acorn[ctmp].en) {
      if((((acorn[ctmp].ypos.hi))>(totoro.ypos.uval-20)) &&
	 (((acorn[ctmp].ypos.hi))<(totoro.ypos.uval+42)) ) {
	//	color=COLOR_YELLOW;
	if((totoro.xpos.val>(acorn[ctmp].xpos.val-36)) &&
	   (totoro.xpos.uval<(acorn[ctmp].xpos.uval+15)) ) {
	  //	  color=COLOR_RED;
//	    if(cr&(0x10<<ctmp)){
	      //color=COLOR_BLACK;
	      acorn[ctmp].en=0;
	      start_sound();
	      /* VIC.spr_ena&=~(0x10<<a) */
	      gstate.score+=10+(PGROUND_Y-totoro.ypos.val);
	      if(gstate.acorns) gstate.acorns--;
//	    }
	}
      }
    }
    //    VIC.spr_color[(ctmp&3)+4]=color;
  }
  //  for(ctmp=0;ctmp<4;ctmp++) {
  //    VIC.spr_color[(ctmp&3)+4]=(cr&(0x10<<ctmp))?COLOR_BLACK:COLOR_ORANGE;
  //  }
}

void __fastcall__ delay(uint8_t f)
{
  while(f--)
    waitvsync();
}

void __fastcall__ get_ready(void)
{
  CLR_TOP();
  //  sprintf(STR_BUF,"STAGE %d",gstate.stage);
  //  convprint_big(14);
  //  strcpy8(STR_BUF,txt_stage);
  strcpy8f(txt_stage);
  convprint_big(14);
  utoa10(gstate.stage);
  //  string_pad(2);
  convprint_big(26);
  
  delay(VFREQ);
  //  sprintf(STR_BUF,"CATCH %d ACORNS",stage[gstate.stage_idx].acorns);
  //  convprint_big(4);
  strcpy8(STR_BUF,txt_catch);
  convprint_big(6);
  
  utoa10(stage[gstate.stage_idx].acorns);
  string_pad(2);
  convprint_big(18);
  strcpy8(STR_BUF,txt_acorns);
  convprint_big(24);
  
  delay(VFREQ);

  CLR_TOP();
  strcpy8(STR_BUF,txt_ready);
  convprint_big(15);
  delay(VFREQ/3);
  strcpy8(STR_BUF,txt_set);
  convprint_big(15);
  delay(VFREQ/3);
  strcpy8(STR_BUF,txt_go);
  convprint_big(16);
  delay(VFREQ/3);
}

void __fastcall__ game_loop(void)
{
  //  VIC.bordercolor=COLOR_RED;
  waitvsync();
  //  VIC.bordercolor=COLOR_BLACK;
  DEBUG_BORDER(COLOR_WHITE);

  // screen updates
  totoro_set_pos();

  // calculate new acorn positions
  acorn_update();
  acorn_add();
  DEBUG_BORDER_INC();

  // input processing
  process_input();
  DEBUG_BORDER_INC();

  // calculate new totoro position
  totoro_update();
  DEBUG_BORDER_INC();

  process_sound();

  // collision
  cr=VIC.spr_coll;
  check_collision();
  cr=VIC.spr_coll;

  // other screen updates
  DEBUG_BORDER_INC();
  update_top_bar();

  // time
  gstate.counter++;
  gstate.field++;
  if(gstate.field==VFREQ) {
    if(MODE_PLAY_DEMO()) gstate.time--;
    gstate.field=0;
  }

  // speed up music
  if(gstate.time==20) vpb=VPB-1;
  if(gstate.time==10) vpb=VPB-2;

  // black background for idle time
  DEBUG_BORDER(COLOR_BLACK);
}

int main()
{
  static uint8_t flag;
  static uint16_t bonus;

  inflatemem (SPR_DATA, sprite_src_data);
  memcpy((uint8_t *)(0x4000-64*8),SPR_DATA+35*64,64*8);
// movie style title
//  memcpy((uint8_t *)(0x4000-64*4),SPR_DATA+58*64,64*3);
  setup_sid();

  spr_mux=0;
  CIA1.icr=0x7f; // disable all CIA1 interrupts
  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=60;
  VIC.imr=0x1; // enable raster interrupt

  Title_Sprite_Setup();
  mode_text();
  VIC.spr_ena=0xff;

  memcpy((uint8_t *)(0x400+40*5+16),present_txt,8);
  memcpy((uint8_t *)(0x400+40*14),intro_txt,39);
  //strcpy8v((0x400+40*6+15),present_txt);
  //strcpy8v((0x400+40*16),intro_txt);
#if (DEBUG==0)
  memcpy((uint8_t *)(0x400+40*16),license_txt,7*40+8);
#endif

  inflatemem (charset, charset_data);
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

#if (DEBUG&DEBUG_INFO)
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
      vpb=VPB;
      gstate.time=0;
      gstate.field=0;
      gstate.counter=0;

      gstate.stage_idx=(gstate.stage>LAST_STAGE_IDX()) ?
	LAST_STAGE_IDX() : gstate.stage-1 ;

      gstate.time=stage[gstate.stage_idx].time;
      gstate.acorns=stage[gstate.stage_idx].acorns;

      game_sprite_setup();
      totoro_init();
      acorn_init();
      spr_mux=1;
      totoro_set_pos();

      VIC.spr_ena=0x07;
      get_ready();
      gstate.mode=GMODE_PLAY;
      setup_top_bar(0);

      for(;gstate.time&&gstate.acorns;)
	game_loop();

      gstate.mode=GMODE_CUT1;
      if(gstate.acorns==0) {
	flag=1;
	track[0].next_offset=rand()&0xe;
	CLR_TOP();
	strcpy8(STR_BUF,txt_stage);
	convprint_big(8);
	strcpy8(STR_BUF,txt_clear);
	convprint_big(20);
      } else {
	flag=0;
      }

      // make totoro walk away
      for(;totoro.xpos.uval<350;)
	game_loop();

      delay(VFREQ/2);

      if(flag) {
	bonus=0;
	setup_top_bar(1);
	do {
	  //	  sprintf(STR_BUF,"%2d", gstate.time);
	  utoa10(gstate.time);
	  string_pad(2);
	  convprint_big(TIMEVAL_X);
	  //	  sprintf(STR_BUF,"%d", bonus);
	  utoa10(bonus);
	  //	  string_pad(4);
	  convprint_big(ACORNVAL_X-2);
	  //	  sprintf(STR_BUF,"%5d", gstate.score);
	  utoa10(gstate.score);
	  string_pad(5);
	  printat(SCOREVAL_X,1);
	  delay(1);
	  bonus+=5+gstate.stage;
	  gstate.score+=5+gstate.stage;
	} while (gstate.time--);
	delay(VFREQ/2);
	gstate.stage++;
      }

    } while(flag);

    // game over
    
    //  strcpy8(STR_BUF,txt_game_over);
    //  convprint_big(0);
    spr_mux=0;
    SPR_PTR[6]=43;
    SPR_PTR[7]=44;
    VIC.spr_pos[6].x=136;
    VIC.spr_pos[7].x=184;
    VIC.spr_pos[6].y=120;
    VIC.spr_pos[7].y=120;
    VIC.spr_exp_x |= 0xc0;
    VIC.spr_exp_y |= 0xc0;
    VIC.spr_hi_x &= 0x3f;
    VIC.spr_mcolor |= 0xc0;
    VIC.spr_ena |= 0xc0;
    delay(VFREQ*3);
  }

  return 0;
}
