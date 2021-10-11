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
// should be 8 with .66 acceleration
// approximate to 9 and .75 acceleration
#define JUMP_V 0x900
#define JUMP_A 200
#else
#define MAX_XV 16
#define JUMP_V 0xa00
#define JUMP_A 200
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
  tcache.state=IDLE;
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

void __fastcall__ totoro_set_pos(void)
{
  static uint8_t anim_idx;

  if(totoro[0].poison) {
    anim_idx=(gstate.counter&0xF)>>2;
    VIC.spr_color[2] = COLOR_BLUE;
    totoro[0].poison--;
  } else {
    anim_idx=(gstate.counter&0x1F)>>3;
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
  
  if(totoro[0].xpos.hi)   VIC.spr_hi_x |=0x1C;
  else VIC.spr_hi_x &= 0xE3;

  switch(totoro[0].state) {
  case IDLE:
    if(totoro[0].blink)   SPR_PTR[2]=SPR_CHU_BLINK;
    else   SPR_PTR[2]=SPR_CHU_IDLE;
    SPR_PTR[3]=SPR_CHU_IDLE+1;
    SPR_PTR[4]=SPR_CHU_IDLE+2;
    break;
  case RUN:
    if(totoro[0].xv>0) {
      SPR_PTR[2]=run_seq[anim_idx];
      SPR_PTR[3]=run_seq[anim_idx]+1;
      SPR_PTR[4]=run_seq[anim_idx]+2;
    } else {
      SPR_PTR[2]=run_seq[anim_idx+4];
      SPR_PTR[3]=run_seq[anim_idx+4]+1;
      SPR_PTR[4]=run_seq[anim_idx+4]+2;
    }
    break;
  case BRAKE:
    if(totoro[0].xv>0) {
      SPR_PTR[2]=SPR_CHU_BR;
      SPR_PTR[3]=SPR_CHU_BR+1;
      SPR_PTR[4]=SPR_CHU_BR+2;
    } else {
      SPR_PTR[2]=SPR_CHU_BL;
      SPR_PTR[3]=SPR_CHU_BL+1;
      SPR_PTR[4]=SPR_CHU_BL+2;
    }
    break;
  case JUMP:
    if(totoro[0].xv>0) {
      SPR_PTR[2]=run_seq[0];
      SPR_PTR[3]=run_seq[0]+1;
      SPR_PTR[4]=run_seq[0]+2;
    } else if (totoro[0].xv<0) {
      SPR_PTR[2]=run_seq[4];
      SPR_PTR[3]=run_seq[4]+1;
      SPR_PTR[4]=run_seq[4]+2;
    } else {
      // add eye movement based on up or down
      SPR_PTR[2]=SPR_CHU_IDLE;
      SPR_PTR[3]=SPR_CHU_IDLE+1;
      SPR_PTR[4]=SPR_CHU_IDLE+2;
    }
    break;
  }
}


void __fastcall__ chibi_set_pos(void)
{
  //  static uint8_t run_offset;
  static uint8_t anim_idx;
  static uint8_t tcolor;

  if(totoro[1].poison) {
    tcolor = COLOR_BLUE;
    totoro[1].poison--;
  } else {
    tcolor = COLOR_WHITE;
  }
  
  VIC.spr_pos[0].x  = VIC.spr_pos[1].x   = totoro[1].xpos.lo;
  VIC.spr_pos[0].y  = VIC.spr_pos[1].y   = totoro[1].ypos.hi;
  
  if(totoro[1].xpos.hi)   VIC.spr_hi_x |=0x03;
  else VIC.spr_hi_x &= 0xFC;
  
  
  if (totoro[1].state==IDLE) {
    if(totoro[1].blink) {
      SPR_PTR[0]=SPR_CHIBI_IDLE+1;
      SPR_PTR[1]=SPR_CHIBI_IDLE;
      VIC.spr_color[0]=tcolor;
      VIC.spr_color[1]=COLOR_BLACK;
    } else {
      SPR_PTR[0]=SPR_CHIBI_IDLE;
      SPR_PTR[1]=SPR_CHIBI_IDLE+1;
      VIC.spr_color[0]=COLOR_BLACK;
      VIC.spr_color[1]=tcolor;
    }
  } else {
    switch(totoro[1].state) {
    case RUN:
      if(totoro[1].poison) anim_idx=(gstate.counter>>2)&1;
      else anim_idx=(gstate.counter>>1)&1;
      break;
    case JUMP:
      anim_idx=1;
      break;
    case BRAKE:
      anim_idx=0;
      break;
    }
    VIC.spr_color[0]=COLOR_BLACK;
    VIC.spr_color[1]=tcolor;
    if(totoro[1].xv>0) {
      SPR_PTR[0]=SPR_CHIBI_RR+anim_idx;
      SPR_PTR[1]=SPR_CHIBI_RR+2;
    } else if (totoro[1].xv<0) {
      SPR_PTR[0]=SPR_CHIBI_RL+anim_idx;
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
  static uint8_t r;
  static uint8_t ground;
  static uint16_t max_x; 

  tcache.xpos.val+=(tcache.xv>>2);

  if(gstate.mode&0xfe) {
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

  if(tcache.state==JUMP) {
    tcache.yv.val+=JUMP_A;
  }

  ground=(p_idx)?GROUND_Y:PGROUND_Y;

  if(tcache.ypos.hi>ground) {
    tcache.ypos.val=(ground<<8);
    if(tcache.xv) tcache.state=RUN;
    else tcache.state=IDLE;
    tcache.yv.val=0;
  }

  if((tcache.xv==0) && (tcache.state!=JUMP))
    tcache.state=IDLE;

  if(tcache.blink)
    tcache.blink--;

  if(gstate.field==25) {
    if((tcache.state==IDLE) && (tcache.blink==0)) {
      r=rand();
      if((r&0x3)==0x3) {
	tcache.blink=VFREQ/10;
      }
    }
  }
}

void __fastcall__ process_input(void)
{
  static uint8_t key;
  static uint8_t js;
  static uint8_t acc;
  
  if(gstate.mode==GMODE_PLAY) {
    if(p_idx==0) {
      // chu totoro
      acc=2;
      js=joy2()&0x1c; // mask joy up and down
#ifndef TWO_PLAYER
      if(js==0) {
	key=PEEK(203);
	if(key==10) js=0x04;
	else if (key==18) js=0x08;
	if(PEEK(653)&1) js|=0x10;
      }
#endif
    } else {
#if 1
      // chibi totoro
      acc=3;
      if(tcache.ctrl==CTRL_AUTO) {
	// follow mode
	if(((totoro[0].state==JUMP) && (totoro[1].state!=JUMP))
	   && ((totoro[1].xv==0)|| same_direction(totoro[0].xv,totoro[1].xv)) ) {
	  js=0x10;
	  tcache.xv=totoro[0].xv; // must use tcache to prevent overwriting
	} else {
	  if((totoro[0].xpos.val-totoro[1].xpos.val)>50) js=0x08;
	  else if((totoro[0].xpos.val-totoro[1].xpos.val)<-74) js=0x04;
	  else js=0;
	}
      } else if(tcache.ctrl==CTRL_PLAY) {
	// 2 player mode
	js=joy1() & 0x1c; // mask joy up and down
      } else key=0;
#endif
    }
  } else js=0x08; // simulate 'D'
  
  if(tcache.state!=JUMP) {
    
    if(js&0x04) {
      if(tcache.xv>-MAX_XV) {
	tcache.xv-=acc;
	if(tcache.xv>0) {
	  tcache.state=BRAKE;
	} else {
	  tcache.state=RUN;
	}
      }
    }
    
    if(js&0x08) {
      if(tcache.xv<MAX_XV) {
	tcache.xv+=acc;
	if(tcache.xv<0) {
	  tcache.state=BRAKE;
	} else {
	  tcache.state=RUN;
	}
      }
    }

    if(js&0x10) {
      // make chibi totoro jump higher
      tcache.yv.val=(p_idx)?-(JUMP_V+JUMP_A):-JUMP_V;
      tcache.state=JUMP;
    }

    if(js==0) {
      if(tcache.xv==0) {
	tcache.state=IDLE;
      } else {
	tcache.state=BRAKE;
	if(tcache.xv>0) {
	  tcache.xv--;
	} else {
	  tcache.xv++;
	}
      }
    }
  
    if(tcache.poison) {
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
	switch(a->en) {
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
	  
	  if(gstate.acorns) gstate.acorns--;
	  break;
	case OBJ_BERRY:
          // FIXME: start error sound
	  tcache.poison+=250;
	  break;
	case OBJ_APPLE:
	  // FIXME: bonus sound
	  gstate.time+=10;
	  break;
	}
	a->en=0;
	#endif
	//	color=1;
      }
    }
  }
  //  VIC.bordercolor=COLOR_BLUE;
  //  if(p_idx) VIC.bordercolor=color;
}

