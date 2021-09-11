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
 *  along with TOTORO64.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                            *
 ******************************************************************************/

#ifndef TOTORO64_H
#define TOTORO64_H

#include <stdint.h>

#define DEBUG_TIMING 0x01
#define DEBUG_INFO   0x02
#define DEBUG_ACORNS 0x04

//#define DEBUG (DEBUG_TIMING)

#ifndef DEBUG
#define DEBUG 0
#endif

//#define NTSC

#ifdef NTSC
#define VPB 10
#define VFREQ 60
#else
#define VPB 8
#define VFREQ 50
#endif

enum t_state {
  IDLE, JUMP, /*JUMP_UP, JUMP_DOWN,*/ RUN, BRAKE
};

enum g_mode {
  GMODE_PLAY=0, GMODE_DEMO=1, GMODE_CUT1
};

#define MODE_PLAY_DEMO() (!(gstate.mode&0xfe))


struct game_state_t {
  uint8_t  stage;     // current stage
  uint8_t  stage_idx; // index in the stage array
  uint8_t  field;     // 0 to VFREQ
  uint8_t  counter;   // free running
  uint8_t  time;      // remaining stage time in secons
  uint8_t  acorns;    // remaining acorns to catch
  uint16_t score;
  uint16_t hi_score;
  enum g_mode mode;
};

struct acorn_t {
  uint8_t  en;
  uint16_t ypos;
  uint16_t yv;   // 8.8 format
};

struct player_t {
  int16_t xpos;
  int16_t ypos;
  int8_t  xv;
  int16_t yv;    // 8.8 format
  uint8_t idx;
  uint8_t blink;
  enum t_state  state;
  //  enum t_dir    dir;
};

struct stage_t {
  uint8_t time;
  uint8_t acorns;
  uint8_t speed;
  uint8_t flags;
};

struct sound_t {
  uint8_t timer;
  uint8_t index;
};

// ASM function prototypes
void IRQ(void);

void __fastcall__ ClrScr(void);
void __fastcall__ SetColor(uint8_t c);
void __fastcall__ ClrLine(uint8_t l);

void __fastcall__ PutLine(void);
void __fastcall__ PutCharHR(void);
void __fastcall__ printat(uint8_t x, uint8_t y);
void __fastcall__ convprint_big(uint8_t x);
void __fastcall__ convert_big(void);
void __fastcall__ printbigat(uint8_t x, uint8_t y);

// totoro actions
void __fastcall__ totoro_init(void);
void __fastcall__ totoro_update(void);
void __fastcall__ totoro_set_pos(void);

// acorn actions
void __fastcall__ acorn_init(void);
void __fastcall__ acorn_update(void);
void __fastcall__ acorn_set_pos(void);
uint8_t __fastcall__ acorn_find(void);
void __fastcall__ acorn_add(void);

uint8_t __fastcall__ joy2(void);

// zp variables from ASM
extern unsigned int itmp;
#pragma zpsym("itmp")
extern unsigned char * line_ptr;
#pragma zpsym("line_ptr")
extern unsigned char c1,ctmp;
#pragma zpsym("c1")
#pragma zpsym("ctmp")

// global variables
extern const uint8_t charset[];
extern const uint8_t sprite_src_data[];
extern const uint8_t bitmap_data[];
extern const uint8_t color1_data[];
extern const uint8_t color2_data[];

// base pointers for screen data
extern uint8_t SCR_BASE[];   // screen base
extern uint8_t COLOR_BASE[]; // color base
extern uint8_t SPR_DATA[];   // sprite data
extern uint8_t SPR_PTR[];    // sprite pointers

extern uint8_t STR_BUF[];

// irq player interface
extern uint8_t track1[];
extern uint8_t instr1;
extern uint8_t time1;
extern uint8_t loop1;
extern uint8_t next_loop1;
extern uint8_t vpb;

extern const uint8_t *t1ptr;
extern const uint8_t *t2ptr;
#pragma zpsym("t1ptr")
#pragma zpsym("t2ptr")


#define memset8(addr, v, c) \
  __asm__("ldx #%b",c);	    \
  __asm__("lda #%b",v);		      \
  __asm__("l%v: sta %v-1,x",addr,addr);   \
  __asm__("dex"); \
  __asm__("bne l%v",addr);
#endif

