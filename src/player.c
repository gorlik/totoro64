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
#include <c64.h>
#include <stdlib.h>
#include <peekpoke.h>
#include "totoro64.h"

// sprite max speed
#ifdef NTSC
#define MAX_XV 13
// V=8.33 A=0.523
#define JUMP_V 0x0855
#define JUMP_A 0x0086
#else
#define MAX_XV 16
// V=10.00 A=0.75
#define JUMP_V 0x0a00
#define JUMP_A 0x00c0
#endif

#define PGROUND_Y (GROUND_Y-23)
#define MAX_PX (MAX_X-24)

struct player_t totoro[2];

static struct player_t tcache;
uint8_t p_idx;

const uint8_t run_seq[] =
  {
    SPR_CHU_RR1, SPR_CHU_RR2, SPR_CHU_RR3, SPR_CHU_RR4, // run right
    SPR_CHU_RL1, SPR_CHU_RL2, SPR_CHU_RL3, SPR_CHU_RL4, // run left
  };

void __fastcall__ process_input(void);
void __fastcall__ check_collision(void);
void __fastcall__ totoro_move(void);

void __fastcall__ tcache_load(void)
{
  __asm__("ldx %v",p_idx);
  __asm__("ldy #0");
  __asm__("tc_load_loop:");
  __asm__("lda %v,x",totoro);
  __asm__("sta %v,y",tcache);
  __asm__("inx");
  __asm__("iny");
  __asm__("cpy #%b",sizeof(struct player_t));
  __asm__("bne tc_load_loop");
}

void __fastcall__ tcache_save(void)
{
  __asm__("ldx %v",p_idx);
  __asm__("ldy #0");
  __asm__("tc_save_loop:");
  __asm__("lda %v,y",tcache);
  __asm__("sta %v,x",totoro);
  __asm__("inx");
  __asm__("iny");
  __asm__("cpy #%b",sizeof(struct player_t));
  __asm__("bne tc_save_loop");
}

void __fastcall__ totoro_init(uint8_t p)
{
  p_idx=p;
  tcache_load();

  tcache.xv=0;
  tcache.yv.val=0;
  tcache.state=PSTATE_IDLE;
  tcache.blink=0;
  tcache.poison=0;

  if(p_idx) {
    // chibi
    tcache.ypos.val=GROUND_Y<<8;
    tcache.xpos.val=(MAX_X-MIN_X)/2+50;
    VIC.spr_color[0]=COLOR_BLACK;
    VIC.spr_color[1]=COLOR_WHITE;
  } else {
    // chu
    tcache.ypos.val=PGROUND_Y<<8;
    tcache.xpos.val=(MAX_PX-MIN_X)/2;
    VIC.spr_color[2]=COLOR_LIGHTBLUE;
    VIC.spr_color[3]=COLOR_BLACK;
    VIC.spr_color[4]=COLOR_WHITE;
  }

  tcache_save();
}

#define same_direction(a,b) (!((a^b)&0x80))

static void __fastcall__ totoro_sprite(uint8_t v)
{
  SPR_PTR[2]=v;
  SPR_PTR[3]=v+1;
  SPR_PTR[4]=v+2;
}

void __fastcall__ totoro_set_pos(void)
{
#ifdef USE_ZP
#define chu_anim_idx  zptmp1
#else
  static uint8_t chu_anim_idx;
#endif

  chu_anim_idx=(game.counter&0xF)>>2;

  if(totoro[0].poison) {
    if(totoro[0].ptype==PTYPE_SLOW) {
      chu_anim_idx=(game.counter&0x1F)>>3;
      VIC.spr_color[2] = COLOR_BLUE;
    } else {
      VIC.spr_color[2] = COLOR_VIOLET;
    }

    totoro[0].poison--;
    //__asm__("lda %v+%b", totoro, 11); // offsetof(struct player_t,poison));
    //__asm__("bne skip");
    //__asm__("dec %v+%b+1",totoro,11);  // offsetof(struct player_t,poison));
    //__asm__("skip: dec %v+%b",totoro,11); // offsetof(struct player_t,poison));
  } else {
    VIC.spr_color[2] = COLOR_LIGHTBLUE;
  }

  /*  VIC.spr_pos[2].x   = totoro[0].xpos.lo;
  VIC.spr_pos[3].x   = totoro[0].xpos.lo;
  VIC.spr_pos[4].x   = totoro[0].xpos.lo;*/
  VIC.spr_pos[2].x   = VIC.spr_pos[3].x   = VIC.spr_pos[4].x   = totoro[0].xpos.lo;

  /*  VIC.spr_pos[2].y   = totoro[0].ypos.hi;
  VIC.spr_pos[3].y   = totoro[0].ypos.hi;
  VIC.spr_pos[4].y   = totoro[0].ypos.hi;*/
  VIC.spr_pos[2].y   = VIC.spr_pos[3].y   = VIC.spr_pos[4].y   = totoro[0].ypos.hi;

  if(totoro[0].xpos.hi)   SPR_HX |=0x1C;
  else SPR_HX &= 0xE3;

  switch(totoro[0].state) {
  case PSTATE_IDLE:
    totoro_sprite(SPR_CHU_IDLE);
    if(totoro[0].blink)   SPR_PTR[2]=SPR_CHU_BLINK;
    break;
  case PSTATE_RUN:
    if(totoro[0].xv>0) {
      totoro_sprite(run_seq[chu_anim_idx]);
    } else {
      totoro_sprite(run_seq[chu_anim_idx+4]);
    }
    break;
  case PSTATE_BRAKE:
    if(totoro[0].xv>0) {
      totoro_sprite(SPR_CHU_BR);
    } else {
      totoro_sprite(SPR_CHU_BL);
    }
    break;
  case PSTATE_JUMP:
    if(totoro[0].xv>0) {
      totoro_sprite(run_seq[0]);
    } else if (totoro[0].xv<0) {
      totoro_sprite(run_seq[4]);
    } else {
      totoro_sprite(SPR_CHU_IDLE);
      // add eye movement based on up or down
    }
    break;
  }
}


void __fastcall__ chibi_set_pos(void)
{
#ifdef USE_ZP
#define chibi_anim_idx  zptmp1
#define chibi_color     zptmp2
#else
  //  static uint8_t run_offset;
  static uint8_t chibi_anim_idx;
  static uint8_t chibi_color;
#endif

  if(totoro[1].poison) {
    if(totoro[1].ptype==PTYPE_SLOW) {
      chibi_color = COLOR_BLUE;
    } else {
      chibi_color = COLOR_VIOLET;
    }
    totoro[1].poison--;
  } else {
    chibi_color = COLOR_WHITE;
  }

  VIC.spr_pos[0].x  = VIC.spr_pos[1].x   = totoro[1].xpos.lo;
  VIC.spr_pos[0].y  = VIC.spr_pos[1].y   = totoro[1].ypos.hi;

  if(totoro[1].xpos.hi)   SPR_HX |=0x03;
  else SPR_HX &= 0xFC;

  if (totoro[1].state==PSTATE_IDLE) {
    if(totoro[1].blink) {
      SPR_PTR[0]=SPR_CHIBI_IDLE+1;
      SPR_PTR[1]=SPR_CHIBI_IDLE;
      VIC.spr_color[0]=chibi_color;
      VIC.spr_color[1]=COLOR_BLACK;
    } else {
      SPR_PTR[0]=SPR_CHIBI_IDLE;
      SPR_PTR[1]=SPR_CHIBI_IDLE+1;
      VIC.spr_color[0]=COLOR_BLACK;
      VIC.spr_color[1]=chibi_color;
    }
  } else {
    switch(totoro[1].state) {
    case PSTATE_RUN:
      if(totoro[1].poison && (totoro[1].ptype==PTYPE_SLOW) ) {
	chibi_anim_idx=(game.counter>>2)&1;
      } else {
	chibi_anim_idx=(game.counter>>1)&1;
      }
      break;
    case PSTATE_JUMP:
      chibi_anim_idx=1;
      break;
    case PSTATE_BRAKE:
      chibi_anim_idx=0;
      break;
    }
    VIC.spr_color[0]=COLOR_BLACK;
    VIC.spr_color[1]=chibi_color;
    if(totoro[1].xv>0) {
      SPR_PTR[0]=SPR_CHIBI_RR+chibi_anim_idx;
      SPR_PTR[1]=SPR_CHIBI_RR+2;
    } else if (totoro[1].xv<0) {
      SPR_PTR[0]=SPR_CHIBI_RL+chibi_anim_idx;
      SPR_PTR[1]=SPR_CHIBI_RL+2;
    } else {
      SPR_PTR[0]=SPR_CHIBI_IDLE;
      SPR_PTR[1]=SPR_CHIBI_IDLE+1;
    }
  }
}

void __fastcall__ totoro_update(void)
{
  tcache_load();
  if(tcache.ctrl==0) return;

  process_input();
  //  VIC.bordercolor=COLOR_YELLOW;
  totoro_move();
  //  VIC.bordercolor=COLOR_BLUE;
  check_collision();
  // check collisions every other frame
  /*  if(gstate.counter&1) {
    if(p!=0) check_collision();
  } else {
    if(p==0) check_collision();
  }*/
  tcache_save();
}

void __fastcall__ totoro_move()
{
#ifdef USE_ZP
#define ground zptmp1
#define r      zptmp2
#else
  static uint8_t ground;
  static uint8_t r;
#endif
  static uint16_t max_x;

  tcache.xpos.val+=(tcache.xv>>2);

  if(game.state&0xfe) {
    max_x=350;
  } else {
    max_x=(p_idx)?MAX_X:MAX_PX;
  }

  if(tcache.xpos<MIN_X) {
    tcache.xpos.val=MIN_X;
    tcache.xv=-1;
  } else if(tcache.xpos>max_x) {
      tcache.xpos.val=max_x;
      tcache.xv=1;
  }

  tcache.ypos.val+=tcache.yv.val;

  if(tcache.state==PSTATE_JUMP) {
    tcache.yv.val+=JUMP_A;
  }

  ground=(p_idx)?GROUND_Y:PGROUND_Y;

  if(tcache.ypos.hi>ground) {
    tcache.ypos.val=(ground<<8);
    if(tcache.xv) tcache.state=PSTATE_RUN;
    else tcache.state=PSTATE_IDLE;
    tcache.yv.val=0;
  }

  if((tcache.xv==0) && (tcache.state!=PSTATE_JUMP))
    tcache.state=PSTATE_IDLE;

  if(tcache.blink)
    tcache.blink--;

  if(game.field==25) {
    if((tcache.state==PSTATE_IDLE) && (tcache.blink==0)) {
      r=rand();
      if((r&0x3)==0x3) {
	tcache.blink=VFREQ/10;
      }
    }
  }
}

void __fastcall__ process_input(void)
{
#ifdef USE_ZP
#define js      zptmp1
#define t_accel zptmp2
#else
  static uint8_t js;
  static uint8_t t_accel;
#endif
 
  if(game.state==GSTATE_PLAY) {
    if(p_idx==0) {
      // chu totoro
      t_accel=2;
      js=joy2k() & 0x1c; // mask joy up and down
    } else {
      // chibi totoro
      t_accel=3;
      if(tcache.ctrl==CTRL_AUTO) {
	// follow mode
	if(((totoro[0].state==PSTATE_JUMP) && (totoro[1].state!=PSTATE_JUMP))
	   && ((totoro[1].xv==0)|| same_direction(totoro[0].xv,totoro[1].xv)) ) {
	  js=0x10;
	  tcache.xv=totoro[0].xv; // must use tcache to prevent overwriting
	} else {
	  if((totoro[0].xpos.val-totoro[1].xpos.val)>50) js=0x08;
	  else if((totoro[0].xpos.val-totoro[1].xpos.val)<-74) js=0x04;
	  else js=0;
	}
      } else {
	// not auto mode
	js=joy1()& 0x1c; // mask joy up and down
      }
    }
    if(tcache.poison && (tcache.ptype==PTYPE_INVERT) && (js&0x0c)) js^=0x0c;
  } else js=0x08; // simulate 'D'
  
  if(tcache.state!=PSTATE_JUMP) {
    
    if(js&0x04) {
      if(tcache.xv>-MAX_XV) {
	tcache.xv-=t_accel;
	if(tcache.xv>0) {
	  tcache.state=PSTATE_BRAKE;
	} else {
	  tcache.state=PSTATE_RUN;
	}
      }
    }
    
    if(js&0x08) {
      if(tcache.xv<MAX_XV) {
	tcache.xv+=t_accel;
	if(tcache.xv<0) {
	  tcache.state=PSTATE_BRAKE;
	} else {
	  tcache.state=PSTATE_RUN;
	}
      }
    }

    if(js&0x10) {
      // make chibi totoro jump higher
      tcache.yv.val=(p_idx)?-(JUMP_V+JUMP_A):-JUMP_V;
      tcache.state=PSTATE_JUMP;
    }

    if(js==0) {
      if(tcache.xv==0) {
	tcache.state=PSTATE_IDLE;
      } else {
	tcache.state=PSTATE_BRAKE;
	if(tcache.xv>0) {
	  tcache.xv--;
	} else {
	  tcache.xv++;
	}
      }
    }
  
    if(tcache.poison && (tcache.ptype==PTYPE_SLOW) ) {
      if(tcache.xv<-(MAX_XV/2)) tcache.xv=-MAX_XV/2;
      if(tcache.xv>(MAX_XV/2)) tcache.xv=MAX_XV/2;
    }
  }
}

#pragma register-vars (on)
void __fastcall__ check_collision(void)
{
  register struct acorn_t *a;
  static uint8_t ty1, ty2;
  static int16_t tx1, tx2;

  ty1=tcache.ypos.hi-20;
  tx1=tcache.xpos.uval-15;
  if(p_idx) {
    ty2=tcache.ypos.hi+22;
    tx2=tcache.xpos.uval+20;
  } else {
    ty2=tcache.ypos.hi+42;
    tx2=tcache.xpos.uval+36;
  }

//  VIC.bordercolor=COLOR_WHITE;
  for(a=acorn;a!=acorn+8;a++) {
    if(a->en) {
      if( ((a->ypos.hi)>ty1)   &&
	  ((a->ypos.hi)<ty2)   &&
	  ((a->xpos.uval)>tx1) &&
	  ((a->xpos.uval)<tx2) ) {
	#if 1
	switch((a->en)&OBJ_TYPE_MASK) {
	case OBJ_ACORN:
	  start_sound();

	  if(p_idx) {
	    if(tcache.ctrl==CTRL_AUTO)
	      totoro[0].score+=10+(GROUND_Y-tcache.ypos.hi);
	    else
	      tcache.score+=10+(GROUND_Y-tcache.ypos.hi);
	  } else {
	  tcache.score+=10+(PGROUND_Y-tcache.ypos.hi);
	  }

	  if(game.acorns) game.acorns--;
	  break;
	case OBJ_BERRY:
          // FIXME: start error sound
	  tcache.poison+=(POISON_TIME*VFREQ);
	  if((a->en)&0x80) tcache.ptype=PTYPE_INVERT;
	  else tcache.ptype=PTYPE_SLOW;
	  break;
	case OBJ_APPLE:
	  // FIXME: bonus sound
	  game.time+=10;
	  break;
	}
	a->en=0;
	#endif
	//	color=1;
      }
    }
  }

  if(spin_top.en) {
    if( ((spin_top.ypos)>ty1)   &&
	((spin_top.ypos)<ty2)   &&
	((spin_top.xpos.uval)>tx1) &&
	((spin_top.xpos.uval)<tx2) ) {
      //   VIC.bordercolor=COLOR_BLUE;
      tcache.state=PSTATE_JUMP;
      tcache.yv.val=-(2*JUMP_V)/3;
      if((spin_top.xpos.uval-tx1)>25) { // this is valid for chu totoro
	tcache.xv=-MAX_XV;
      } else {
	tcache.xv=MAX_XV;
      }
      //      tcache.ypos.hi--;
    } else {
      //   VIC.bordercolor=COLOR_BLACK;
    }

  }
  //  VIC.bordercolor=COLOR_BLUE;
  //  if(p_idx) VIC.bordercolor=color;
}

