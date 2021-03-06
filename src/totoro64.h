/******************************************************************************
 *  TOTORO64                                                                  *
 *  A Studio Ghibli inspired game for the Commodore 64                        *
 *  Copyright 2021-2022 Gabriele Gorla                                        *
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

#ifndef TOTORO64_H
#define TOTORO64_H

#include <stdint.h>

// user modifiable compile time options
#define SPRITE_MESSAGES
//#define MOVIE_TITLE
#define STAGE_TIME 60
#define POISON_TIME 5

// these are set in the makefile
//#define NTSC
//#define HIRES

// end of user modifiable configuration
#define VERSION "v0.39"

#define USE_ZP

#define DEBUG_TIMING 0x01
#define DEBUG_INFO   0x02
#define DEBUG_ACORNS 0x04

//#define DEBUG (DEBUG_TIMING)

#ifndef DEBUG
#define DEBUG 0
#endif

#ifdef NTSC
#define VPB 10
#define VFREQ 60
#else
#define VPB 8
#define VFREQ 50
#endif


// indexes in the sprite data
// (SPR_DATA-VIC_BASE)/64
#define SPR_DATA_OFFSET   16

#define SPR_CHU_IDLE      (0+SPR_DATA_OFFSET)
#define SPR_CHU_BLINK     (3+SPR_DATA_OFFSET)
#define SPR_CHU_RR1       (4+SPR_DATA_OFFSET)
#define SPR_CHU_RR1       (4+SPR_DATA_OFFSET)
#define SPR_CHU_RR2       (7+SPR_DATA_OFFSET)
#define SPR_CHU_RR3       (10+SPR_DATA_OFFSET)
#define SPR_CHU_RR4       (7+SPR_DATA_OFFSET)
#define SPR_CHU_RL1       (16+SPR_DATA_OFFSET)
#define SPR_CHU_RL2       (19+SPR_DATA_OFFSET)
#define SPR_CHU_RL3       (22+SPR_DATA_OFFSET)
#define SPR_CHU_RL4       (19+SPR_DATA_OFFSET)
#define SPR_CHU_BR        (13+SPR_DATA_OFFSET)
#define SPR_CHU_BL        (25+SPR_DATA_OFFSET)
#define SPR_CHIBI_IDLE    (28+SPR_DATA_OFFSET)
#define SPR_CHIBI_RR      (30+SPR_DATA_OFFSET)
#define SPR_CHIBI_RL      (33+SPR_DATA_OFFSET)

#define SPR_ACORN_LG      (36+SPR_DATA_OFFSET)
#define SPR_ACORN_SM      (37+SPR_DATA_OFFSET)
#define SPR_BERRY         (38+SPR_DATA_OFFSET)
#define SPR_APPLE         (39+SPR_DATA_OFFSET)
#define SPR_CHERRY        (40+SPR_DATA_OFFSET)

#define SPR_SPIN          (44+SPR_DATA_OFFSET)

#define SPR_TXT_GAME_OVER (49+SPR_DATA_OFFSET+2)
#define SPR_TXT_STAGE_CLR (52+SPR_DATA_OFFSET+2)
#define SPR_TXT_READY     (55+SPR_DATA_OFFSET+2)
#define SPR_TXT_SET       (58+SPR_DATA_OFFSET+2)
#define SPR_TXT_GO        (61+SPR_DATA_OFFSET+2)


#define SPR_GGLABS_1      (64+SPR_DATA_OFFSET)
#define SPR_TITLE_BOLD_1  (68+SPR_DATA_OFFSET)
#define SPR_64            (71+SPR_DATA_OFFSET)
#define SPR_TITLE_MOVIE_1 (72+SPR_DATA_OFFSET)

#ifdef HIRES
#define VSPR_FIELD   0
#define VSPR_BAR     1
#define VZONES 2
#define VNUM 3
#define VTSIZE (VNUM*2)
#define VIDX(a) ((a-(8-VNUM)))

#define CHARSET_PTR VIC_CHARSET

#define SPR_EN vspr_ctl[VSPR_FIELD].en
#define SPR_MC vspr_ctl[VSPR_FIELD].mc
#define SPR_EX vspr_ctl[VSPR_FIELD].exp_x
#define SPR_EY vspr_ctl[VSPR_FIELD].exp_y
#define SPR_HX vspr_ctl[VSPR_FIELD].hi_x

#define VSPR_XPOS(x)  vspr_pos[VIDX(x)*2]
#define VSPR_YPOS(x)  vspr_pos[VIDX(x)*2+1]
#define VSPR_PTR(x)   vspr_pc[VIDX(x)]
#define VSPR_COLOR(x) vspr_pc[VIDX(x)+VNUM]

#else
#define CHARSET_PTR CHARSET

#define SPR_EN VIC.spr_ena
#define SPR_MC VIC.spr_mcolor
#define SPR_EX VIC.spr_exp_x
#define SPR_EY VIC.spr_exp_y
#define SPR_HX VIC.spr_hi_x

#define VSPR_XPOS(s)  VIC.spr_pos[s].x
#define VSPR_YPOS(s)  VIC.spr_pos[s].y
#define VSPR_PTR(s)   SPR_PTR[s]
#define VSPR_COLOR(s) VIC.spr_color[s]

#endif

#define SFX_NONE   0
#define SFX_ACORN  1
#define SFX_BONUS  2
#define SFX_SLOW   3
#define SFX_INVERT 4

// the following must be kept in sync with the assembly code
#define MAX_ACORNS 8
#define MUX_SLOTS  2

#define SCR_SPLIT_EN  0x01
#define SPR_MUX_EN    0x80

// sprite position constants/limits
#define GROUND_Y 220
#define ACORN_START_Y 74

#define MIN_X 32
#define MAX_X 312

#define SPR_CENTER_X 184

#define CHU_TOTORO   0
#define CHIBI_TOTORO sizeof(struct player_t)

// stage flags
#define SF_BERRIES    0x01
#define SF_BERRIES_ALT 0x02
#define SF_WIND1       0x04
#define SF_WIND2       0x08
//#define SF_DBL_ACORN   0x10
#define SF_SPIN       0x20

#define COL_R 0x00
#define COL_L 0x80

// player control modes
#define CTRL_OFF  0x00
#define CTRL_AUTO 0x01
#define CTRL_PLAY 0x02

// falling object types
#define OBJ_TYPE_MASK 0x0F
#define OBJ_ACORN  0x1
#define OBJ_BERRY  0x2
#define OBJ_APPLE  0x3

// player states
#define PSTATE_IDLE  0
#define PSTATE_JUMP  1
#define PSTATE_RUN   2
#define PSTATE_BRAKE 3

// poison types
#define PTYPE_SLOW   0x01
#define PTYPE_INVERT 0x02

// game states
#define GSTATE_PLAY 0
#define GSTATE_DEMO 1
#define GSTATE_CUT1 2

// game modes
#define GMODE_1P_SOLO  0
#define GMODE_1P_STD   1
#define GMODE_2P_COOP  2
#define GMODE_2P_IND   3
#define GMODE_2P_VS    4
#define GMODE_2P_MASK  0xfe


union word {
  uint16_t uval;
  int16_t val;
  struct {
    uint8_t lo;
    uint8_t hi;
  };
};

typedef union word word_t;

#define STATE_PLAY_DEMO() (!(game.state&0xfe))

struct stage_t {
  uint8_t time;
  uint8_t acorns;
  uint8_t apples;
  uint8_t accel;
  uint8_t flags;
};

struct game_state_t {
  uint8_t  mode;
  uint8_t  state;
  uint8_t  stage;     // current stage
  uint8_t  field;     // 0 to VFREQ - used to decrement the game time
  uint8_t  counter;   // free running
  //  struct stage_t st;
  uint8_t  time;      // remaining stage time in secons
  uint8_t  acorns;    // remaining acorns to catch
  uint8_t  apples;
  uint8_t  accel;
  uint8_t  flags;
  // wind parameters
  uint8_t  wind_sp;   // vblanks between updates
  uint8_t  wind_cnt;  // update counter
  int8_t   wind_dir;  // 0, 1, -1
  // acorn firing counter
  uint8_t  acorn_cnt;
  // misc
  uint16_t hi_score;
  uint16_t random;
};

struct acorn_t {
  uint8_t en;
  word_t  xpos;
  word_t  ypos; // 8.8 format
  word_t  yv;   // 8.8 format
  uint8_t spr_ptr;
  uint8_t spr_color;
};

struct spin_top_t {
  uint8_t en;
  word_t  xpos;
  uint8_t ypos;
  int8_t  xv;
  uint8_t idx;
};

struct player_t {
  uint8_t  ctrl;
  uint16_t score;
  word_t   xpos;
  word_t   ypos;
  int8_t   xv;
  word_t   yv;       // 8.8 format
  uint8_t  blink;    // blink counter
  uint16_t poison;   // poison counter
  uint8_t  ptype;    // poison type
  //  enum t_state  state;
  uint8_t  state;    // player state
};

struct sound_t {
  uint8_t type;
  uint8_t last;
  uint8_t timer;
  uint8_t index;
};

struct track_t {
  uint16_t ptr;           // ptr to next byte in track
  uint8_t  timer;         // note timer
  uint8_t  instr;         // instrument
  uint8_t  track_offset;  // semitone offset from music data
  uint8_t  global_offset; // semitone offset from main code
  uint8_t  voice_offset;  // SID register offset for the voice
  uint8_t  next_offset;   // next semitone offset
  uint16_t restart_ptr;
};

struct instr_t {
  uint8_t wf;
  uint8_t ad;
  uint8_t sr;
};

struct vspr_ctl_t {
  uint8_t hi_x;
  uint8_t en;
  uint8_t exp_y;
  uint8_t mc;
  uint8_t exp_x;
  uint8_t pad;
  //  uint8_t mc0;
  //  uint8_t mc1;
};

// ASM function prototypes
void IRQ(void);

void __fastcall__ ClrScr(void);
void __fastcall__ SetColor(uint8_t c);
void __fastcall__ ClrLine(uint8_t l);

void __fastcall__ PutLine(void);
void __fastcall__ PutBigLine(void);
void __fastcall__ PutCharHR(void);
void __fastcall__ printat_f(uint8_t pos);
void __fastcall__ printbigat(uint8_t x);
void __fastcall__ print_acorn(uint8_t c);
void __fastcall__ print_hourglass(uint8_t c);
void __fastcall__ print_col(uint8_t c);
void __fastcall__ print_p(uint8_t c);

// totoro actions
void __fastcall__ totoro_init(uint8_t p);
void __fastcall__ totoro_update(void);
void __fastcall__ totoro_set_pos(void);
void __fastcall__ chibi_set_pos(void);

// acorn actions
void __fastcall__ acorn_init(void);
void __fastcall__ acorn_update(void);
void __fastcall__ acorn_set_pos(void);
uint8_t __fastcall__ acorn_find(void);
void __fastcall__ acorn_add(void);

// misc
void __fastcall__ start_sound(uint8_t t);
void __fastcall__ process_sound(void);
void __fastcall__ stop_sound(void);
uint8_t __fastcall__ joy1(void);
uint8_t __fastcall__ joy2(void);
uint8_t __fastcall__ joyk(void);
uint8_t __fastcall__ joy2k(void);
uint8_t __fastcall__ joy_any(void);
void __fastcall__ string_pad(int8_t pad);
int __fastcall__ utoa10 (uint16_t val);
void __fastcall__ delay  (uint8_t t);
void __fastcall__ setup(void);
void __fastcall__ CLR_TOP(void);
void __fastcall__ CLR_CENTER(void);

// zp variables from ASM
extern uint8_t * line_ptr;
#pragma zpsym("line_ptr")
// temp_ptr and zptmpx share the same location
extern uint8_t * temp_ptr;
#pragma zpsym("temp_ptr")
extern uint8_t zptmp1,zptmp2;
#pragma zpsym("zptmp1")
#pragma zpsym("zptmp2")

// compressed data
extern const uint8_t charset_data[];
extern const uint8_t sprite_src_data[];
extern const uint8_t bitmap_data[];
extern const uint8_t color1_data[];
extern const uint8_t color2_data[];

// base pointers for graphics data
extern uint8_t VIC_BASE[];     // base pointer of VIC memory
extern uint8_t SCREEN_BASE[];  // character screen base
extern uint8_t SPR_PTR[];      // sprite pointers
extern uint8_t SPR_DATA[];     // sprite data
extern uint8_t VIC_CHARSET[];  // uncompressed charset (in vic memory)
extern uint8_t BITMAP_BASE[];  // bitmap base

extern uint8_t CHARSET[];      // uncompressed charset
extern uint8_t VIC_REG[];      // VIC register base
extern uint8_t COLOR_BASE[];   // color ram

// global state
extern struct  game_state_t game;
extern struct  player_t totoro[2];
extern struct  acorn_t acorn[MAX_ACORNS];
extern struct  spin_top_t spin_top;

// other global variables
extern uint8_t p_idx;
extern uint8_t irq_ctrl;
extern uint8_t STR_BUF[64];

#ifdef HIRES
extern struct vspr_ctl_t  vspr_ctl[VZONES];
extern uint8_t vspr_pos[VTSIZE*VZONES];
extern uint8_t vspr_pc[VTSIZE*VZONES];
#endif

// irq player interface
extern const   uint8_t track0_data[];
extern const   uint8_t track1_data[];
extern struct  track_t track[2];
extern uint8_t vpb;

#define memset8s(addr,v,c) do {			 \
  __asm__("ldx #%b",c);				 \
  __asm__("lda #%b",v);				 \
  __asm__("ms8s%s: sta %v-1,x",__LINE__,addr);   \
  __asm__("dex");				 \
  __asm__("bne ms8s%s",__LINE__);		 \
  } while (0)

#define memset8c(addr,v,c) do {			    \
  __asm__("ldx #%b",c);				    \
  __asm__("lda #%b",v);				    \
  __asm__("ms8c%s: sta %w-1,x",__LINE__,addr);	    \
  __asm__("dex");				    \
  __asm__("bne ms8c%s",__LINE__);		    \
  } while (0)

#define memcpy8c(dst,src,len) do {		\
    __asm__("ldy #%b",len);			\
    __asm__("mc8c%s: lda %v-1,y",__LINE__,src); \
    __asm__("sta %w-1,y",dst);			\
    __asm__("dey");				\
    __asm__("bne mc8c%s",__LINE__);		\
  } while (0)

#define memcpy8s(dst,src,len) do {		\
    __asm__("ldy #%b",len);			\
    __asm__("mc8s%s: lda %v-1,y",__LINE__,src); \
    __asm__("sta %v-1,y",dst);			\
    __asm__("dey");				\
    __asm__("bne mc8s%s",__LINE__);		\
  } while (0)

#ifndef HIRES
#define printat(x,y) do {				\
    __asm__("lda #<(_BITMAP_BASE+%w)", (y*320+x*8) );	\
    __asm__("sta %v",line_ptr);				\
    __asm__("lda #>(_BITMAP_BASE+%w)", (y*320+x*8) );	\
    __asm__("sta %v+1",line_ptr);			\
    PutLine();						\
  } while (0)
#else
#define printat(x,y) printat_f(y*40+x)
#endif
#endif
