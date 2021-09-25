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

#define STAGE_TIME 60

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

#define TXT_PTR 0x400
#define AT(r,c) (TXT_PTR+40*r+c)

void __fastcall__ CLR_TOP(void);

#if (DEBUG & DEBUG_TIMING)
#define DEBUG_BORDER(a) VIC.bordercolor=a
#define DEBUG_BORDER_INC() VIC.bordercolor++
#else
#define DEBUG_BORDER(a)
#define DEBUG_BORDER_INC()
#endif

extern const unsigned char present_txt[];
extern const unsigned char intro_txt[];
extern const unsigned char version_txt[];
extern const unsigned char license_txt[];

const unsigned char txt_score[]  = "SCORE";
const unsigned char txt_time[]   = "TIME";
const unsigned char txt_bonus[]  = "BONUS";
const unsigned char txt_acorns[] = "ACORNS";
const unsigned char txt_catch[]  = "CATCH    ";
const unsigned char txt_stage[]  = "STAGE ";
const unsigned char txt_clear[]  = "CLEAR";
#ifdef SPRITE_MESSAGES
#define MSG_READY     SPR_TXT_READY
#define MSG_SET       SPR_TXT_SET
#define MSG_GO        SPR_TXT_GO
#define MSG_GAME_OVER SPR_TXT_GAME_OVER
#define MSG_STAGE_CLR SPR_TXT_STAGE_CLR
#else
const unsigned char txt_ready[] = "READY";
const unsigned char txt_set[]   = " SET ";
const unsigned char txt_go[]    = " GO ";
const unsigned char txt_game_over[] = "GAME OVER";
#define MSG_READY     txt_ready
#define MSG_SET       txt_set
#define MSG_GO        txt_go
#define MSG_GAME_OVER txt_game_over
#endif

#define scr_strcpy8(dst,src)    \
  __asm__("ldx #$FF");          \
  __asm__("ls%v: inx",src);			\
  __asm__("lda %v,x",src);			\
  __asm__("beq fs%v",src);			\
  __asm__("sta %w,x",dst);			\
  __asm__("bne ls%v",src);			\
  __asm__("fs%v:",src);     

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
   { STAGE_TIME, 10, 3, SF_PINECONES },
   { STAGE_TIME, 15, 4, SF_WIND1 },
   { STAGE_TIME, 20, 6, SF_ACORN1 },
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
   { STAGE_TIME, 10, 4, SF_PINECONES },
   { STAGE_TIME, 15, 6, SF_WIND1 },
   { STAGE_TIME, 20, 8, SF_ACORN1 },
   { STAGE_TIME, 25, 10, 0 },
   { STAGE_TIME, 30, 12, 0 },
   { STAGE_TIME, 35, 14, 0 },
   { STAGE_TIME, 40, 16, 0 },
   { STAGE_TIME, 50, 18, 0 },
   { STAGE_TIME, 60, 21, 0 },
};
#endif

#ifdef SPRITE_MESSAGES
#define MESSAGEP(p,m) sprite_message2p(m)
#define MESSAGE(p,m) sprite_message2(m)
#else
#define MESSAGEP(p,m) do { strcpy8f(m); convprint_big(p); delay(VFREQ/3); } while (0)
#define MESSAGE(p,m) do { strcpy8f(m); convprint_big(p); } while (0)
#endif

#define LAST_STAGE_IDX() ((sizeof(stage)/sizeof(struct stage_t))-1)

#define PRINT_STRING_AT(p,m)   do { strcpy8f(m); convprint_big(p); } while (0)
#define PRINT_NUMBERP_AT(p,n,l) do { utoa10(n); string_pad(l); convprint_big(p); } while (0)
#define PRINT_NUMBER_AT(p,n)   do { utoa10(n); convprint_big(p); } while (0)

struct game_state_t gstate;

struct acorn_t acorn[MAX_ACORNS];
struct sound_t sound;

struct track_t track[2];
uint8_t vpb;


uint8_t STR_BUF[64];

uint8_t spr_mux;
uint8_t cr;

uint16_t last_rand;

int __fastcall__ utoa10 (uint16_t val);

void __fastcall__ _strcpy8f (void)
{
  __asm__("ldy #$FF");
  __asm__("strloop: iny");
  __asm__("lda (_temp_ptr),y");
  __asm__("sta _STR_BUF,y");
  __asm__("bne strloop");
}

#define strcpy8f(a) do {           \
    temp_ptr=(unsigned char *)a;   \
  _strcpy8f();                     \
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

#define acorn_update_m(a)                   \
do {	                                    \
    if(acorn[a].ypos.val) {		    \
      acorn[a].yv.val+=gstate.accel;        \
      acorn[a].ypos.val+=(acorn[a].yv.val); \
      if((acorn[a].ypos.hi)>GROUND_Y) {     \
      	acorn[a].en=0;			    \
	acorn[a].ypos.val=0;                \
      } else {                              \
        if(gstate.wind_cnt==0)              \
          acorn[a].xpos.val+=gstate.wind_dir;   \
      }					    \
   }   \
} while(0)

#pragma register-vars (on)
void acorn_update_f(uint8_t idx)
{
  register struct acorn_t *a=&acorn[idx];

  if(a->ypos.val) {
    a->yv.val+=gstate.accel;
    a->ypos.val+=(a->yv.val);
    if((a->ypos.hi)>GROUND_Y) {
      a->en=0;
      a->ypos.val=0;
    } else {
   if(gstate.wind_cnt==0)
      a->xpos.val+=gstate.wind_dir;
  }
}
}

void __fastcall__ acorn_update(void)
{
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
      acorn[ctmp].yv.val+=gstate.accel;
      acorn[ctmp].ypos.val+=(acorn[ctmp].yv.val);
      if((acorn[ctmp].ypos.hi)>GROUND_Y) {
      	acorn[ctmp].en=0;
	acorn[ctmp].ypos.val=0;
	}
    } */

/*    if(acorn[ctmp].en) {
      acorn[ctmp].yv.val+=gstate.accel;
      acorn[ctmp].ypos.val+=(acorn[ctmp].yv.val);
      if((acorn[ctmp].ypos.hi)>GROUND_Y)
	acorn[ctmp].en=0;
    }
  }*/
}

void __fastcall__ acorn_init(void)
{
  // init data
  memset8(acorn,0,sizeof(acorn));
}

unsigned char __fastcall__ acorn_free_slot(void)
{
  if( (acorn[MAX_ACORNS-1].en==0) &&
      (acorn[0].ypos.hi)>(ACORN_START_Y+6) &&
     ( (acorn[MUX_SLOTS-1].ypos.hi>ACORN_START_Y+23) || (acorn[MUX_SLOTS-1].ypos.val==0) /*|| (acorn[2].ypos.val==0)*/) ) return 1;
  if(acorn[0].en==0) return 1;
  return 0;
}

void __fastcall__ acorn_add(void)
{
  static unsigned int oldr=0;
  static unsigned int r;

  // maybe change to counter
  if(MODE_PLAY_DEMO() 
     && ( ((gstate.counter&0x40)==0x40) || (gstate.flags & SF_ACORN1))
    // && (gstate.field==10 || gstate.field==27 || gstate.field==44 ) 
    )  {
	// new acorn
        r=last_rand&0x3f;
	if((r>=36) || (abs(r-oldr)<8) ) return;
	oldr=r;

	  r<<=3;
	  r+=MIN_X;
      if(acorn_free_slot()) {
 // VIC.bordercolor=COLOR_YELLOW;
	// shift acorn data 
	__asm__("sei");
	__asm__("ldx #(%b*%b)",MAX_ACORNS-1,sizeof(struct acorn_t));
	__asm__("loop1: lda %v-1,x",acorn);
	__asm__("sta %v+%b-1,x",acorn,sizeof(struct acorn_t));
	__asm__("dex");
	__asm__("bne loop1");
	__asm__("cli");

	acorn[0].xpos.val=r;
	acorn[0].ypos.val=ACORN_START_Y<<8;
	acorn[0].yv.val=4;
	acorn[0].spr_ptr=(r&0x08)?SPR_ACORN_LG:SPR_ACORN_SM;
//	acorn[0].spr_ptr=(r&0x08)?SPR_ACORN_LG:SPR_ACORN_LG;
        if((gstate.flags&SF_PINECONES)&& (((last_rand>>8)&0x7)==3) ) {
	  acorn[0].spr_color=COLOR_BLACK;
	} else 	acorn[0].spr_color=COLOR_ORANGE;
	acorn[0].en=1;
	  }
	//   }
    }
}

void __fastcall__ mode_bitmap(void)
{
  CIA2.pra=(CIA2.pra&0xfc)|0x2;  // selects VIC page 0x4000-0x7FFF

  VIC.ctrl1=0x3B; // enable bitmap, no extended color, no blank, 25 rows, ypos=3
  //  VIC.addr =0x78; // screen at base +0x1c00, bitmap at base + 0x2000
  VIC.addr =0x08; // screen at base +0x0000, bitmap at base + 0x2000
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

  // setup sprite pointers for the title screen
  //  for(i=0;i<8;i++) POKE(0x7f8+i,248+i);
  __asm__("ldx #248");
  __asm__("loopt: txa");
  __asm__("sta $7f8-248,x");
  __asm__("inx");
  __asm__("bne loopt");
  
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

void __fastcall__ wait_line(uint8_t l)
{
  //  VIC.bordercolor=COLOR_WHITE;
  while(VIC.rasterline<=l) {};
  //  VIC.bordercolor=COLOR_BLACK;
}

void __fastcall__ update_top_bar(void)
{
  // interleave the updates to reduce frame time
  //  if(MODE_PLAY_DEMO()) {
#if 1
    switch(gstate.counter&0x0F) {
    case 0:
      utoa10(gstate.acorns);
      string_pad(2);
      break;
    case 1:
      convert_big();
      break;
    case 2:
      wait_line(60);
      printbigat(ACORNVAL_X,0);
      break;
    case 4:
      utoa10(gstate.time-1);
      string_pad(2);
      break;
    case 5:
      convert_big();
      break;
    case 6:
      wait_line(60);
      printbigat(TIMEVAL_X,0);
      break;
    case 7:
      utoa10(gstate.score);
      string_pad(5);
      break;
    case 8:
      wait_line(60);
      printat(SCOREVAL_X,1);
      break;
#if (DEBUG&DEBUG_INFO)
    case 10:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ST:%2d IDX:%2d", gstate.stage, gstate.stage_idx);
      break;
    case 11:
      wait_line(60);
      DEBUG_BORDER_INC();
      printat(DEBUG_TXT_X,2);
      break;
    case 12:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ML:%2d SPD:%2d", loop1, gstate.accel);
      break;
    case 13:
      wait_line(60);
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
    //  }
}

void __fastcall__ setup_top_bar(uint8_t flag)
{
  CLR_TOP();

  PRINT_STRING_AT(TIMETXT_X,txt_time);

  if(flag==0) {
    //    strcpy8(STR_BUF,txt_acorns);
    temp_ptr=(unsigned char *)txt_acorns;
  } else {
    //    strcpy8(STR_BUF,txt_bonus);
    temp_ptr=(unsigned char *)txt_bonus;
  }
  _strcpy8f();
  convprint_big(ACORNTXT_X);

  strcpy8f(txt_score);
  printat(SCORETXT_X,0);

  for(gstate.counter=0;gstate.counter<9;gstate.counter++)
    update_top_bar();
  gstate.counter=0;
}

void __fastcall__ game_sprite_setup(void)
{
  VIC.spr_ena=0;

#if (DEBUG&DEBUG_ACORNS)
  VIC.spr_color[6]=COLOR_BLUE;
  VIC.spr_color[7]=COLOR_GREEN;
#else
  VIC.spr_color[6]=COLOR_ORANGE;
  VIC.spr_color[7]=COLOR_ORANGE;
#endif
  VIC.spr_color[5]=COLOR_BLACK;
  SPR_PTR[5]=70;

  VIC.spr_mcolor=0xF0; // spr 5 is  multicolor
  VIC.spr_exp_x=0x3C;
  VIC.spr_exp_y=0x3C;

  VIC.spr_mcolor0=COLOR_BROWN;
  VIC.spr_mcolor1=COLOR_YELLOW;
}

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
    // VIC.bordercolor=sound.index+1;
    // use index 0 only the first time
    if(sound.index>=(sizeof(sound_seq)/2)) sound.index=0;
    sound.index++;
    sound.timer--;
}

void __fastcall__ delay(uint8_t f)
{
  while(f--)
    waitvsync();
}

#ifdef SPRITE_MESSAGES
void __fastcall__ sprite_message2(uint8_t msg)
{
  spr_mux=0;
  waitvsync();
  
  SPR_PTR[6]=msg;
  SPR_PTR[7]=msg+1;
  VIC.spr_color[6]=COLOR_ORANGE;
  VIC.spr_color[7]=COLOR_ORANGE;
  VIC.spr_pos[6].x=SPR_CENTER_X-48;
  VIC.spr_pos[7].x=SPR_CENTER_X;
  VIC.spr_pos[6].y=120;
  VIC.spr_pos[7].y=120;
  VIC.spr_exp_x |= 0xc0;
  VIC.spr_exp_y |= 0xc0;
  VIC.spr_hi_x  &= 0x3f;
  VIC.spr_mcolor |= 0xc0;
  VIC.spr_ena |= 0xc0;
}

void __fastcall__ sprite_message2p(uint8_t msg)
{
  sprite_message2(msg);
  delay(VFREQ/3);
}
#endif

void __fastcall__ get_ready(void)
{
  CLR_TOP();
  
  PRINT_STRING_AT(14,txt_stage);
  PRINT_NUMBERP_AT(26,gstate.stage,2);
  delay(VFREQ);

  //  sprintf(STR_BUF,"CATCH %d ACORNS",stage[gstate.stage_idx].acorns);
  PRINT_STRING_AT(6,txt_catch);
  PRINT_NUMBER_AT(18,gstate.acorns);
  PRINT_STRING_AT(24,txt_acorns);
  delay(VFREQ);

  CLR_TOP();
#ifdef SPRITE_MESSAGES
  setup_top_bar(0);
#endif
  MESSAGEP(15,MSG_READY);
  MESSAGEP(15,MSG_SET);
  MESSAGEP(16,MSG_GO);
#ifndef SPRITE_MESSAGES
  setup_top_bar(0);
#endif
}

void __fastcall__ game_loop(void)
{
  static uint8_t tr;
  
  waitvsync();
//  cgetc();
  DEBUG_BORDER(COLOR_WHITE);

  // screen updates
  totoro_set_pos();
  chibi_set_pos();

  process_sound();
  DEBUG_BORDER(COLOR_RED);
  //  VIC.bordercolor=COLOR_BLACK;

  // process input, move player and perform collision detection
  totoro_update(CHU_TOTORO);
  totoro_update(CHIBI_TOTORO);
  DEBUG_BORDER(COLOR_GREEN);

  // other screen updates
  update_top_bar();

  // time
  gstate.counter++;
  gstate.anim_idx=(gstate.counter&0xF)>>2;
  gstate.field++;
  if(gstate.field==VFREQ) {
    if(MODE_PLAY_DEMO()) gstate.time--;
    gstate.field=0;
  }

  if(gstate.flags&SF_WIND1) {
    if(gstate.counter==187) {
      tr=last_rand;
      gstate.wind_sp=(tr&7)+1;
      if(tr&0x80) gstate.wind_dir=1;
      else gstate.wind_dir=-1;
    }
    if(gstate.wind_cnt==0) gstate.wind_cnt=gstate.wind_sp;
    else gstate.wind_cnt--;
  }

  // speed up music
  if(gstate.time==20) vpb=VPB-1;
  if(gstate.time==10) vpb=VPB-2;

  last_rand=rand();
  // black background for idle time
  DEBUG_BORDER(COLOR_BLACK);
  wait_line(GROUND_Y+23);
  DEBUG_BORDER(COLOR_YELLOW);
  // calculate new acorn positions
  acorn_update();
  acorn_add();
  DEBUG_BORDER(COLOR_BLACK);
}

int main()
{
  static uint8_t flag;
  static uint16_t bonus;

  inflatemem (SPR_DATA, sprite_src_data);
  memcpy((uint8_t *)(0x4000-64*8),SPR_DATA+35*64,64*8);
  // movie style title
  // memcpy((uint8_t *)(0x4000-64*4),SPR_DATA+58*64,64*3);
  setup_sid();

  spr_mux=0;
  CIA1.icr=0x7f; // disable all CIA1 interrupts
  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=60;
  VIC.imr=0x1; // enable raster interrupt

  Title_Sprite_Setup();
  mode_text();
  VIC.spr_ena=0x0f;

  scr_strcpy8(AT(5,16),present_txt);
  inflatemem (charset, charset_data);
  VIC.spr_ena=0xff;

  scr_strcpy8(AT(11,33),version_txt);
  scr_strcpy8(AT(14,0),intro_txt);
#if (DEBUG==0)
  memcpy((uint8_t *)(0x400+40*16),license_txt,7*40+8);
#endif

  inflatemem (BITMAP_BASE, bitmap_data);
 // for(temp_ptr=(uint8_t *)0x6000;temp_ptr!=(uint8_t *)0x8000;temp_ptr++)
//	*(temp_ptr)=0;
  inflatemem (SCREEN_BASE, color1_data);

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

  inflatemem (COLOR_RAM, color2_data);
  //    cgetc();
  mode_bitmap();

#if (DEBUG&DEBUG_INFO)
  for(ctmp=0;ctmp<DEBUG_TXT_LEN;ctmp++)
    {
      POKE(COLOR_RAM+40*2+DEBUG_TXT_X+ctmp,1);
      POKE(COLOR_RAM*3+DEBUG_TXT_X+ctmp,1);
      POKE(COLOR_RAM*4+DEBUG_TXT_X+ctmp,1);
      POKE(COLOR_RAM*5+DEBUG_TXT_X+ctmp,1);
      POKE(COLOR_RAM*6+DEBUG_TXT_X+ctmp,1);
      POKE(COLOR_RAM*7+DEBUG_TXT_X+ctmp,1);
    }

  sprintf(STR_BUF,"Sprdat $%04X",SPR_DATA);
  printat(DEBUG_TXT_X,2);
  sprintf(STR_BUF,"Color1 $%04X",SCREEN_BASE);
  printat(DEBUG_TXT_X,3);
  sprintf(STR_BUF,"Bitmap $%04X",BITMAP_BASE);
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
      gstate.anim_idx=0;

      gstate.stage_idx=(gstate.stage>LAST_STAGE_IDX()) ?
	LAST_STAGE_IDX() : gstate.stage-1 ;

      gstate.time=stage[gstate.stage_idx].time;
      gstate.acorns=stage[gstate.stage_idx].acorns;
      gstate.accel=stage[gstate.stage_idx].accel;
      gstate.flags=stage[gstate.stage_idx].flags;
      gstate.wind_sp=0;

      game_sprite_setup();
      totoro_init(CHU_TOTORO);
      totoro_init(CHIBI_TOTORO);
      acorn_init();
      totoro_set_pos();
      chibi_set_pos();

      VIC.spr_ena=0x1F;
      //VIC.spr_ena=0x1C;
      
      get_ready();
      
      VIC.spr_ena=0x1F;
      VIC.spr_exp_x=0x3C;
      VIC.spr_exp_y=0x3C;
      spr_mux=1;
  
      gstate.mode=GMODE_PLAY;

      for(;gstate.time&&gstate.acorns;)
	game_loop();

      gstate.mode=GMODE_CUT1;
      if(gstate.acorns==0) {
	flag=1;
	track[0].next_offset=rand()&0xe;
#ifdef SPRITE_MESSAGES
	MESSAGE(0,MSG_STAGE_CLR);
#else
	CLR_TOP();
	PRINT_STRING_AT(8,txt_stage);
	PRINT_STRING_AT(20,txt_clear);
#endif
      } else {
	flag=0;
      }

      // make totoro walk away
      for(;totoro[0].xpos.uval<400;)
	game_loop();

      delay(VFREQ/2);

      if(flag) {
	bonus=0;
	setup_top_bar(1);
	do {
	  PRINT_NUMBERP_AT(TIMEVAL_X,gstate.time,2);
	  PRINT_NUMBER_AT(ACORNVAL_X-2,bonus);
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
    MESSAGE(0,MSG_GAME_OVER);
    delay(VFREQ*3);
  }

  return 0;
}
