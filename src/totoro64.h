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

#ifndef TOTORO64_H
#define TOTORO64_H

enum t_state {
	      IDLE, JUMP, /*JUMP_UP, JUMP_DOWN,*/ RUN, BRAKE
};

enum g_mode {
	     GMODE_PLAY=0, GMODE_DEMO=1, GMODE_CUT1
};

#define MODE_PLAY_DEMO() (!(gstate.mode&0xfe))

struct game_state_t {
  unsigned char stage;
  unsigned char stage_idx;
  int time;
  unsigned int score;
  unsigned int acorns;
  unsigned char frame;
  enum g_mode mode;
};

struct acorn_t {
  unsigned char en;
  unsigned int ypos;
  unsigned int yv;
};

struct player_t {
  int xpos;
  int ypos;
  signed char xv;
  signed char yv;
  unsigned char idx;
  unsigned char blink;
  enum t_state  state;
  //  enum t_dir    dir;
};

struct stage_t {
  unsigned char time;
  unsigned char acorns;
  unsigned char speed;
  unsigned char flags;
};

// ASM function prototypes
void NMI(void);

void __fastcall__ ClrScr(void);
void __fastcall__ SetColor(unsigned char c);
void __fastcall__ ClrLine(unsigned char l);

void __fastcall__ PutLine(void);
void __fastcall__ PutCharHR(void);
void __fastcall__ printat(unsigned char x, unsigned char y);
void __fastcall__ convert_big(void);
void __fastcall__ printbigat(unsigned char x, unsigned char y);

// totoro actions
void __fastcall__ totoro_init(void);
void __fastcall__ totoro_update(void);
void __fastcall__ totoro_set_pos(void);

// acorn actions
void __fastcall__ acorn_init(void);
void __fastcall__ acorn_update(void);
void __fastcall__ acorn_set_pos(void);
unsigned char __fastcall__ acorn_find(void);
void __fastcall__ acorn_add(void);


// variables from ASM
extern unsigned int itmp;
#pragma zpsym("itmp")
extern unsigned char * line_addr;
#pragma zpsym("line_addr")
extern unsigned char get,put;
#pragma zpsym("get")
#pragma zpsym("put")
extern unsigned char c1,ctmp;
#pragma zpsym("c1")
#pragma zpsym("ctmp")


// global variables

extern const unsigned char charset[];
extern const unsigned char sprite_src_data[];
extern const unsigned char bitmap_data[];
extern const unsigned char color1_data[];
extern const unsigned char color2_data[];

extern unsigned char SCR_BASE[];   // screen base
extern unsigned char COLOR_BASE[]; // color base
extern unsigned char SPR_DATA[];   // sprite data
extern unsigned char SPR_PTR[];    // sprite pointers


extern unsigned char * const line[];      // precomputed line address

extern unsigned char STR_BUF[];




#define memset8(addr, v, c) \
  __asm__("ldx #%b",c);	    \
  __asm__("lda #%b",v);		      \
  __asm__("l%v: sta %v-1,x",addr,addr);   \
  __asm__("dex"); \
  __asm__("bne l%v",addr);

#endif
