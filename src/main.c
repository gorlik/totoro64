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

// title bar positions

// co-op
#define P1_X          0
#define P1_SCORETXT_X 5
#define P1_SCOREVAL_X 5
#define P1_BAR_X      11

#define TIME_ICON_X   13
#define TIMEVAL_X     15

#define P1_ICON_X     21
#define P1_ACORNVAL_X 23

#define P1_BONUSTXT_X 21
#define P1_BONUSVAL_X 21

#define P2_BAR_X      28
#define P2_X          29
#define P2_SCORETXT_X (P1_SCORETXT_X+P2_X)
#define P2_SCOREVAL_X (P1_SCOREVAL_X+P2_X)

// vs.
/*
#define P1_X          0
#define P1_SCORETXT_X 5
#define P1_SCOREVAL_X 5
#define P1_ICON_X     10
#define P1_ACORNVAL_X 12
#define P1_BAR_X      16
#define P1_BONUSTXT_X 10
#define P1_BONUSVAL_X 10

#define TIME_ICON_X   17
#define TIMEVAL_X     19

#define P2_BAR_X      23
#define P2_X          24
#define P2_SCORETXT_X (P1_SCORETXT_X+P2_X)
#define P2_SCOREVAL_X (P1_SCOREVAL_X+P2_X)
#define P2_ICON_X     (P1_ICON_X+P2_X)
#define P2_ACORNVAL_X (P1_ACORNVAL_X+P2_X)
#define P2_BONUSTXT_X (P1_BONUSTXT_X+P2_X)
#define P2_BONUSVAL_X (P1_BONUSVAL_X+P2_X)
*/

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
const unsigned char txt_bonus[]  = "BONUS";
const unsigned char txt_catch[]  = "CATCH    ACORNS";
const unsigned char txt_stage[]  = "STAGE";
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
const unsigned char txt_clear[]  = "STAGE CLEAR";
#define MSG_READY     txt_ready
#define MSG_SET       txt_set
#define MSG_GO        txt_go
#define MSG_GAME_OVER txt_game_over
#define MSG_STAGE_CLR txt_clear
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

#define ACC(a) ((a*50)/VFREQ)

const struct stage_t stage[] = {
#ifndef TESTING
  { STAGE_TIME, 30, 2, ACC(21), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 15, 0, ACC(10), SF_BERRIES },
  { STAGE_TIME, 25, 3, ACC(6),  SF_WIND1 },
  { STAGE_TIME, 30, 2, ACC(21), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
#endif
  { STAGE_TIME, 10, 0, ACC(4),  0 },
  { STAGE_TIME, 15, 0, ACC(6),  0 },
  { STAGE_TIME, 20, 1, ACC(8),  0 },
  { STAGE_TIME, 25, 1, ACC(10), SF_BERRIES },
  { STAGE_TIME, 30, 1, ACC(12), SF_BERRIES },
  { STAGE_TIME, 35, 1, ACC(14), SF_BERRIES },
  { STAGE_TIME, 40, 2, ACC(16), SF_BERRIES | SF_WIND1 },
  { STAGE_TIME, 45, 2, ACC(18), SF_BERRIES | SF_WIND1 },
  { STAGE_TIME, 50, 2, ACC(21), SF_BERRIES | SF_WIND1 },
  { STAGE_TIME, 55, 3, ACC(22), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 60, 3, ACC(22), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 65, 3, ACC(22), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 70, 2, ACC(23), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 80, 2, ACC(23), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
  { STAGE_TIME, 90, 1, ACC(23), SF_BERRIES | SF_WIND1 | SF_DBL_ACORN },
};

#ifdef SPRITE_MESSAGES
#define MESSAGEP(p,m) sprite_message2p(m)
#define MESSAGE(p,m)  sprite_message2(m)
#else
#define MESSAGEP(p,m) do { strcpy8f(m); convprint_big(p); delay(VFREQ/3); } while (0)
#define MESSAGE(p,m)  do { strcpy8f(m); convprint_big(p); } while (0)
#endif

#define LAST_STAGE_IDX() ((sizeof(stage)/sizeof(struct stage_t))-1)

#define PRINT_STRING_AT(p,m)    do { strcpy8f(m); convprint_big(p); } while (0)
#define PRINT_NUMBERP_AT(p,n,l) do { utoa10(n); string_pad(l); convprint_big(p); } while (0)
#define PRINT_NUMBER_AT(p,n)    do { utoa10(n); convprint_big(p); } while (0)

struct game_state_t game;

struct acorn_t acorn[MAX_ACORNS];
struct sound_t sound;

struct track_t track[2];
uint8_t vpb;


uint8_t STR_BUF[64];

uint8_t spr_mux;
uint8_t cr;

uint16_t last_rand;

int __fastcall__ utoa10 (uint16_t val);

#define totoro_update_m(p) \
  do { p_idx=p;\
  totoro_update(); \
  } while (0)

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
    _strcpy8f();		   \
  } while (0)

void __fastcall__ wait_for_input(void)
{
#ifdef TWO_PLAYER
    while(!(joy1()||joy2()));
#else
    while(!(joy2()||(PEEK(203)!=0x40)));
#endif
}

void __fastcall__ setup_sid(void)
{
  //memset(&SID,0,24);
  memset8c(0xd400,0,24); // SID address
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

void __fastcall__ stage_init()
{
  static uint8_t stage_idx;
  stage_idx=((game.stage>LAST_STAGE_IDX()) ?
	     LAST_STAGE_IDX() : game.stage-1 ) * sizeof(struct stage_t);

  __asm__("ldy #0");
  __asm__("ldx %v",stage_idx);
  __asm__("stl: lda %v,x",stage);
  __asm__("sta %v+%b,y",game,offsetof(struct game_state_t,time));
  __asm__("inx");
  __asm__("iny");
  __asm__("cpy #%b",sizeof(struct stage_t));
  __asm__("bne stl");

  game.wind_dir=0;
  game.field=0;
  game.counter=0;
}

static void __fastcall__ kiki_update()
{
  static uint16_t kiki_x;
  kiki_x+=2;
  if(game.counter&1) {
    SPR_PTR[5]=SPR_PTR[5]^1;
  } 
  if(kiki_x>400) kiki_x=0;
  VIC.spr_pos[5].y=110;
  VIC.spr_pos[5].x=kiki_x;
  if(kiki_x>255) VIC.spr_hi_x|=0x20;
  else VIC.spr_hi_x&=0xdf;
  VIC.spr_ena=0xff;
}

static void __fastcall__ spin_update()
{
  static uint16_t spin_x;
  spin_x+=2;
  
  if(spin_x>400) spin_x=0;
  VIC.spr_pos[5].y=GROUND_Y;
  VIC.spr_pos[5].x=spin_x;
  if(spin_x>255) VIC.spr_hi_x|=0x20;
  else VIC.spr_hi_x&=0xdf;
  //  VIC.spr_ena=0xff;
}

#define acorn_update_a(idx) do {	       \
  __asm__ volatile ("ldy #%b",idx*sizeof(struct acorn_t)); \
  acorn_update_asm();			       \
  } while (0)

static void __fastcall__ acorn_update_asm()
{
  //  __asm__("tay");
  __asm__("lda %v+%b,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("ora %v+%b+1,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("beq end");
  __asm__("lda %v+%b",game,offsetof(struct game_state_t,accel));
  __asm__("clc");
  __asm__("adc %v+%b,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("sta %v+%b,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("lda #$00");
  __asm__("adc %v+%b+1,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("sta %v+%b+1,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("lda %v+%b,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("clc");
  __asm__("adc %v+%b,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("sta %v+%b,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("lda %v+%b+1,y",acorn,offsetof(struct acorn_t,yv));
  __asm__("adc %v+%b+1,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("sta %v+%b+1,y",acorn,offsetof(struct acorn_t,ypos));
  //  __asm__("lda     _acorn+4,y");
  __asm__("cmp #$DD");
  __asm__("bcc active");
  __asm__("lda #$00");
  __asm__("sta %v+%b,y",acorn,offsetof(struct acorn_t,en));
  __asm__("sta %v+%b,y",acorn,offsetof(struct acorn_t,ypos));
  __asm__("sta %v+%b+1,y",acorn,offsetof(struct acorn_t,ypos));
  //  __asm__("jmp end");
  __asm__("rts");
  __asm__("active:");
  __asm__("lda %v+%b",game,offsetof(struct game_state_t,wind_cnt));
  __asm__("bne end");
  __asm__("tax");
  __asm__("lda %v+%b",game,offsetof(struct game_state_t,wind_dir));
  __asm__("cmp #$80");
  __asm__("bcc not_neg");
  __asm__("dex");
  __asm__("clc");
  __asm__("not_neg:");
  __asm__("adc %v+%b,y",acorn,offsetof(struct acorn_t,xpos));
  __asm__("sta %v+%b,y",acorn,offsetof(struct acorn_t,xpos));
  __asm__("txa");
  __asm__("adc %v+%b+1,y",acorn,offsetof(struct acorn_t,xpos));
  __asm__("sta %v+%b+1,y",acorn,offsetof(struct acorn_t,xpos));
  __asm__("end:");
}

#define acorn_update_m(a)                   \
do {	                                    \
    if(acorn[a].ypos.val) {		    \
      acorn[a].yv.val+=game.accel;        \
      acorn[a].ypos.val+=(acorn[a].yv.val); \
      if((acorn[a].ypos.hi)>GROUND_Y) {     \
      	acorn[a].en=0;			    \
	acorn[a].ypos.val=0;                \
      } else {                              \
        if(game.wind_cnt==0)              \
          acorn[a].xpos.val+=game.wind_dir;   \
      }					    \
   }   \
} while(0)

void __fastcall__ acorn_update(void)
{
#if 0
  acorn_update_a(0);
  acorn_update_a(1);
  acorn_update_a(2);
  acorn_update_a(3);
  acorn_update_a(4);
  acorn_update_a(5);
  acorn_update_a(6);
  acorn_update_a(7);
#else
  __asm__ volatile ("ldy #%b",7*sizeof(struct acorn_t));
  __asm__("loop:");
  acorn_update_asm();
  //  __asm__("jsr _acorn_update_asm");
  __asm__("tya");
  __asm__("beq end");
  __asm__("sec");
  __asm__("sbc #%b",sizeof(struct acorn_t));
  __asm__("tay");
  __asm__("jmp loop");
  __asm__("end:");
  
#endif
}

void __fastcall__ acorn_init(void)
{
  // init data
  memset8s(acorn,0,sizeof(acorn));
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

  if(STATE_PLAY_DEMO() 
     && ( ((game.counter&0x40)==0x40) || (game.flags & SF_DBL_ACORN))
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

      if((game.flags&SF_BERRIES) && (((last_rand>>8)&0x6)==4) ) {
	acorn[0].spr_ptr=SPR_BERRY;
	acorn[0].spr_color=COLOR_BLUE;
	acorn[0].en=OBJ_BERRY;
      } else if((game.apples) && (((last_rand>>8)&0x3f)==25) ) {
	game.apples--;
	acorn[0].spr_ptr=SPR_APPLE;
	acorn[0].spr_color=COLOR_LIGHTRED;
	acorn[0].en=OBJ_APPLE;
      } else {
	acorn[0].spr_ptr=(r&0x08)?SPR_ACORN_LG:SPR_ACORN_SM;
	acorn[0].spr_color=COLOR_ORANGE;
	acorn[0].en=OBJ_ACORN;
      }
    }
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

  VIC.spr_mcolor0=COLOR_BROWN;
  VIC.spr_mcolor1=COLOR_YELLOW;
}

static void __fastcall__ mode_text(void)
{
  CIA2.pra|=0x03; // selects vic page 0x0000-0x3fff

  VIC.ctrl1=0x1B; // disable bitmap, no extended color, no blank, 25 rows, ypos=3
  VIC.addr= 0x16; // screen base at 0x0400, char def at $0x1400
  VIC.bgcolor[0]=COLOR_WHITE;
}

static const uint8_t tpos[17] = {
  SPR_CENTER_X-48, 60,
  SPR_CENTER_X-24, 60,
  SPR_CENTER_X,    60,
  SPR_CENTER_X+24, 60,
  SPR_CENTER_X-96, 105,
  SPR_CENTER_X-48, 105,
  SPR_CENTER_X,    105,
  SPR_CENTER_X+48, 105,
  0, // VIC.spr_hi_x
};

static const uint8_t tcol[8] = {
  COLOR_RED,
  COLOR_RED,
  COLOR_RED,
  COLOR_RED,
  COLOR_BLACK,
  COLOR_BLACK,
  COLOR_BLACK,
  COLOR_RED,
};
  
void __fastcall__ Title_Sprite_Setup(void)
{
  VIC.spr_ena=0;       // disable all sprites
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

  // setup sprite position
  //  memcpy(0xd000,tpos,sizeof(tpos));
  __asm__("ldy #%b",sizeof(tpos));
  __asm__("loop: lda %v-1,y",tpos);
  __asm__("sta %w-1,y",0xd000);
  __asm__("dey");
  __asm__("bne loop");

  //  setup sprite colors
  //  memcpy(0xd027,tpos,sizeof(tcol));
  __asm__("ldy #%b",sizeof(tcol));
  __asm__("cloop: lda %v-1,y",tcol);
  __asm__("sta %w-1,y",0xd027);
  __asm__("dey");
  __asm__("bne cloop");
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
#ifndef SPRITE_MESSAGES
  if(STATE_PLAY_DEMO())
#endif
#if 1
    switch(game.counter&0x0F) {
    case 0:
      utoa10(game.acorns);
      string_pad(2);
      break;
    case 1:
      convert_big();
      break;
    case 2:
      wait_line(60);
      printbigat(P1_ACORNVAL_X);
      break;
    case 4:
      utoa10(game.time);
      string_pad(2);
      break;
    case 5:
      convert_big();
      break;
    case 6:
      wait_line(60);
      printbigat(TIMEVAL_X);
      break;
    case 7:
      utoa10(totoro[0].score);
      string_pad(5);
      break;
    case 8:
      wait_line(60);
      printat(P1_SCOREVAL_X,1);
      break;
    case 9:
      utoa10(totoro[1].score);
      string_pad(5);
      break;
    case 10:
      if(totoro[1].ctrl==CTRL_PLAY) {
	wait_line(60);
	printat(P2_SCOREVAL_X,1);
      }
      break;
      
#if (DEBUG&DEBUG_INFO)
    case 10:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ST:%2d IDX:%2d", game.stage, game.stage_idx);
      break;
    case 11:
      wait_line(60);
      DEBUG_BORDER_INC();
      printat(DEBUG_TXT_X,2);
      break;
    case 12:
      DEBUG_BORDER_INC();
      sprintf(STR_BUF,"ML:%2d SPD:%2d", loop1, game.accel);
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

  print_p(P1_X);
  print_p(P2_X);
  print_hourglass(TIME_ICON_X);
  print_col(P1_BAR_X|COL_R);
  print_col(P2_BAR_X|COL_L);
  
  if(flag==0) {
    print_acorn(P1_ICON_X);
  } else {
    strcpy8f(txt_bonus);
    printat(P1_BONUSTXT_X,0);
  }
  
  strcpy8f(txt_score);
  printat(P1_SCORETXT_X,0);

  if(totoro[1].ctrl==CTRL_PLAY) {
    printat(P2_SCORETXT_X,0);
  }  
  
  for(game.counter=(flag)?4:0;game.counter<9;game.counter++)
      update_top_bar();
  game.counter=0;
}

void __fastcall__ game_sprite_setup(void)
{
  VIC.spr_ena=0;

  VIC.spr_color[5]=COLOR_BLACK;
  SPR_PTR[5]=SPR_SPIN;

  VIC.spr_mcolor=0xF0; // spr 5 is  multicolor
  VIC.spr_exp_x=0x1C;
  VIC.spr_exp_y=0x1C;

  if(totoro[0].ctrl) VIC.spr_ena|=0x1c;
  if(totoro[1].ctrl) VIC.spr_ena|=0x03;
}

void __fastcall__ start_sound(void)
{
  stop_sound();
  if(STATE_PLAY_DEMO()) {
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
static void __fastcall__ sprite_message2(uint8_t msg)
{
  spr_mux=0;
  waitvsync();

  SPR_PTR[6]=msg;
  SPR_PTR[7]=msg+1;
  VIC.spr_color[6]=COLOR_BLACK;
  VIC.spr_color[7]=COLOR_BLACK;
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

static void __fastcall__ sprite_message2p(uint8_t msg)
{
  sprite_message2(msg);
  delay(VFREQ/3);
}
#endif

void __fastcall__ get_ready(void)
{
  static uint8_t offset;
  offset=(game.stage>9)?0:1;
  CLR_TOP();

  // "STAGE XX"
  PRINT_STRING_AT(12+offset,txt_stage);
  PRINT_NUMBER_AT(24+offset,game.stage);
  delay(VFREQ);

  // "CATCH XX ACORNS"
  PRINT_STRING_AT(6,txt_catch);
  PRINT_NUMBER_AT(18,game.acorns);
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
  if(totoro[0].ctrl) totoro_update_m(CHU_TOTORO);
  if(totoro[1].ctrl) totoro_update_m(CHIBI_TOTORO);
  DEBUG_BORDER(COLOR_GREEN);

  // other screen updates
  update_top_bar();

  // time
  game.counter++;
  //  game.anim_idx=(game.counter&0xF)>>2;
  game.field++;
  if(game.field==VFREQ) {
    if(STATE_PLAY_DEMO()) game.time--;
    game.field=0;
  }

  if(game.flags&SF_WIND1) {
    if(game.counter==187) {
      tr=last_rand;
      game.wind_sp=(tr&7)+1;
      if(tr&0x80) game.wind_dir=1;
      else game.wind_dir=-1;
    }
    if(game.wind_cnt==0) game.wind_cnt=game.wind_sp;
    else game.wind_cnt--;
  }

  // speed up music
  if(game.time==20) vpb=VPB-1;
  if(game.time==10) vpb=VPB-2;

  last_rand=rand();
  // black background for idle time
  DEBUG_BORDER(COLOR_BLACK);
  wait_line(GROUND_Y+23);
  DEBUG_BORDER(COLOR_YELLOW);
  // calculate new acorn positions
  acorn_update();
  acorn_add();
  //  kiki_update();
  //  spin_update();
  DEBUG_BORDER(COLOR_BLACK);
}

int main()
{
  static uint8_t flag;
  static uint16_t bonus;

  inflatemem (SPR_DATA, sprite_src_data);
#if 0
  memcpy((uint8_t *)(0x4000-64*8),VIC_BASE+SPR_GGLABS_1*64,64*8);
#else
  __asm__("ldy #0");
  __asm__("loop: lda %w-1,y",(0x4000+SPR_GGLABS_1*64));
  __asm__("sta %w-1,y",0x4000-64*8);

  __asm__("lda %w-1,y",0x4000+SPR_GGLABS_1*64+256);
  __asm__("sta %w-1,y",0x4000-64*4);

  __asm__("dey");
  __asm__("bne loop");
#endif
  
  // movie style title
#ifdef MOVIE_TITLE
  memcpy((uint8_t *)(0x4000-64*4),VIC_BASE+SPR_TITLE_MOVIE_1*64,64*3);
#endif
  setup_sid();
  
  spr_mux=0;
  CIA1.icr=0x7f; // disable all CIA1 interrupts
  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=60;
  VIC.imr=0x1; // enable raster interrupt

  Title_Sprite_Setup();
  //  mode_text();
  VIC.bgcolor[0]=COLOR_WHITE;
  VIC.spr_ena=0x0f;

  scr_strcpy8(AT(5,16),present_txt);
  inflatemem (charset, charset_data);
  VIC.spr_ena=0xff;

  scr_strcpy8(AT(11,33),version_txt);
  scr_strcpy8(AT(14,0),intro_txt);
#if (DEBUG==0)
  //  memcpy((uint8_t *)(0x400+40*16),license_txt,7*40+8);
  memcpy((uint8_t *)(0x400+40*18),license_txt,2*40);
#endif

  inflatemem (BITMAP_BASE, bitmap_data);
  // for(temp_ptr=(uint8_t *)0x6000;temp_ptr!=(uint8_t *)0x8000;temp_ptr++)
  //	*(temp_ptr)=0;
  inflatemem (SCREEN_BASE, color1_data);
  memset8s(SCREEN_BASE,((COLOR_YELLOW<<4)|COLOR_ORANGE),80); // can embed this in the color1_data array


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

  wait_for_input();
  //  cgetc();

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
    // game over
#ifndef SPRITE_MESSAGES
    CLR_TOP();
#endif
    MESSAGE(11,MSG_GAME_OVER);
    delay(VFREQ*3);

    wait_for_input();

    // game init
    totoro[0].score=0;
    totoro[1].score=0;
    totoro[0].ctrl=CTRL_PLAY;
#ifdef TWO_PLAYER
    totoro[1].ctrl=CTRL_PLAY;
#else
    totoro[1].ctrl=CTRL_AUTO;
#endif
    game.stage=1;
    game.state=GSTATE_CUT1;

    do {
      // stage init
      vpb=VPB;

      stage_init();

      totoro_init(CHU_TOTORO);
      totoro_init(CHIBI_TOTORO);
      acorn_init();
      totoro_set_pos();
      chibi_set_pos();

      game_sprite_setup();

      get_ready();

      game_sprite_setup();
      spr_mux=1;

      game.state=GSTATE_PLAY;

      for(;game.time&&game.acorns;)
	game_loop();

      game.state=GSTATE_CUT1;
      if(game.acorns==0) {
	flag=1;
	track[0].next_offset=rand()&0xe;
#ifndef SPRITE_MESSAGES
	CLR_TOP();
#endif
	MESSAGE(9,MSG_STAGE_CLR);
      } else {
	flag=0;
      }

      // make totoro walk away
      for(;(totoro[0].xpos.uval<350)||(totoro[1].ctrl&&(totoro[1].xpos.uval<350));)
	game_loop();

      delay(VFREQ/2);

      if(flag) {
	bonus=0;
	setup_top_bar(1);
	do {
	  PRINT_NUMBERP_AT(TIMEVAL_X,game.time,2);
	  utoa10(bonus);
	  string_pad(5);
	  printat(P1_BONUSVAL_X,1);
	  utoa10(totoro[0].score);
	  string_pad(5);
	  printat(P1_SCOREVAL_X,1);
	  if(totoro[1].ctrl==CTRL_PLAY) {
	    utoa10(totoro[1].score);
	    string_pad(5);
	    printat(P2_SCOREVAL_X,1);
	    totoro[1].score+=5+game.stage;
	  }
	  bonus+=5+game.stage;
	  totoro[0].score+=5+game.stage;
	  delay(1);
	} while (game.time--);
	delay(VFREQ/2);
	game.stage++;
      }

    } while(flag);

  }

  return 0;
}
