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
static uint8_t p_idx;

const uint8_t run_seq[] =
  {
    SPR_CHU_RR1, SPR_CHU_RR2, SPR_CHU_RR3, SPR_CHU_RR4, // run right
    SPR_CHU_RL1, SPR_CHU_RL2, SPR_CHU_RL3, SPR_CHU_RL4, // run left
  };

void __fastcall__ process_input(void);
void __fastcall__ check_collision(void);
void __fastcall__ totoro_move(void);

void __fastcall__ tcache_load(uint8_t p)
{
  (void)p;
  __asm__("sta %v",p_idx);
  __asm__("tax");
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
  tcache.xv=0;
  tcache.yv.val=0;
  tcache.state=IDLE;
  tcache.blink=0;  

  if(p) {
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

//#define ttotoro ((struct player_t *)temp_ptr)
/*
#pragma register-vars (on)

void __fastcall__ totoro_init(register struct player_t *ttotoro)
{
  ttotoro->ypos.val=PGROUND_Y;
  ttotoro->xv=0;
  ttotoro->yv.val=0;
  ttotoro->idx=0;
  ttotoro->state=IDLE;
  ttotoro->blink=0;  
  
  if(1) tcache.xpos.val=(MAX_PX-MIN_X)/2+50;
  else tcache.xpos.val=(MAX_PX-MIN_X)/2;

  //  tcache_save(p);
  }*/

/*
void __fastcall__ totoro_update(uint8_t p)
{
  static uint16_t r;
 
  totoro[p].xpos.val+=(totoro[p].xv>>2);

  if(totoro[p].xpos<MIN_X) {
    totoro[p].xpos.val=MIN_X;
    totoro[p].xv=-1;
  } else if(totoro[p].xpos>MAX_PX) {
    if(!(gstate.mode&0xfe)) {
          totoro[p].xpos.val=MAX_PX;
      totoro[p].xv=1;
    }
  }

  totoro[p].ypos.val+=totoro[p].yv.val;

  if(totoro[p].state==JUMP) {
    totoro[p].yv++;
  }

  if(totoro[p].ypos>PGROUND_Y) {
    totoro[p].ypos.val=PGROUND_Y;
    if(totoro[p].xv) totoro[p].state=RUN;
    else totoro[p].state=IDLE;
    totoro[p].yv.val=0;
  }

  if((totoro[p].xv==0) && (totoro[p].state!=JUMP))
    totoro[p].state=IDLE;

  if(totoro[p].blink)
    totoro[p].blink--;

  totoro[p].idx=(gstate.counter&0xF)>>2;

  if(gstate.field==25) {
    if((totoro[p].state==IDLE) && (totoro[p].blink==0)) {
      r=rand();
      if((r&0x3)==0x3) {
	totoro[p].blink=VFREQ/10;
      }
    }
  }
}
*/

void __fastcall__ totoro_set_pos(void)
{
  VIC.spr_pos[2].x   = totoro[0].xpos.lo;
  VIC.spr_pos[3].x   = totoro[0].xpos.lo;
  VIC.spr_pos[4].x   = totoro[0].xpos.lo;

  if(totoro[0].xpos.hi)   VIC.spr_hi_x |=0x1C;
  else VIC.spr_hi_x &= 0xE3;

  VIC.spr_pos[2].y   = totoro[0].ypos.hi;
  VIC.spr_pos[3].y   = totoro[0].ypos.hi;
  VIC.spr_pos[4].y   = totoro[0].ypos.hi;

  switch(totoro[0].state) {
  case IDLE:
    if(totoro[0].blink)   SPR_PTR[2]=SPR_CHU_BLINK;
    else   SPR_PTR[2]=SPR_CHU_IDLE;
    SPR_PTR[3]=SPR_CHU_IDLE+1;
    SPR_PTR[4]=SPR_CHU_IDLE+2;
    break;
  case RUN:
    if(totoro[0].xv>0) {
      SPR_PTR[2]=run_seq[gstate.anim_idx];
      SPR_PTR[3]=run_seq[gstate.anim_idx]+1;
      SPR_PTR[4]=run_seq[gstate.anim_idx]+2;
    } else {
      SPR_PTR[2]=run_seq[gstate.anim_idx+4];
      SPR_PTR[3]=run_seq[gstate.anim_idx+4]+1;
      SPR_PTR[4]=run_seq[gstate.anim_idx+4]+2;
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
  static uint8_t run_offset;

  //  run_offset=0;
  VIC.spr_pos[0].x   = totoro[1].xpos.lo;
  VIC.spr_pos[1].x   = totoro[1].xpos.lo;

  if(totoro[1].xpos.hi)   VIC.spr_hi_x |=0x03;
  else VIC.spr_hi_x &= 0xFC;

  VIC.spr_pos[0].y   = totoro[1].ypos.hi;
  VIC.spr_pos[1].y   = totoro[1].ypos.hi;

  if (totoro[1].state==IDLE) {
    if(totoro[1].blink) {
      SPR_PTR[0]=SPR_CHIBI_IDLE+1;
      SPR_PTR[1]=SPR_CHIBI_IDLE;
      VIC.spr_color[0]=COLOR_WHITE;
      VIC.spr_color[1]=COLOR_BLACK;
    } else {
      SPR_PTR[0]=SPR_CHIBI_IDLE;
      SPR_PTR[1]=SPR_CHIBI_IDLE+1;
      VIC.spr_color[0]=COLOR_BLACK;
      VIC.spr_color[1]=COLOR_WHITE;
    }
  } else {
    switch(totoro[1].state) {
    case RUN:
      run_offset=gstate.anim_idx&1;
      break;
    case JUMP:
      run_offset=1;
      break;
    case BRAKE:
      run_offset=0;
      break;
    }
    VIC.spr_color[0]=COLOR_BLACK;
    VIC.spr_color[1]=COLOR_WHITE;
    if(totoro[1].xv>0) {
      SPR_PTR[0]=SPR_CHIBI_RR+run_offset;
      SPR_PTR[1]=SPR_CHIBI_RR+2;
    } else if (totoro[1].xv<0) {
      SPR_PTR[0]=SPR_CHIBI_RL+run_offset;
      SPR_PTR[1]=SPR_CHIBI_RL+2;
    } else {
      SPR_PTR[0]=SPR_CHIBI_IDLE;
      SPR_PTR[1]=SPR_CHIBI_IDLE+1;
    }
  }
}

void __fastcall__ totoro_update(uint8_t p)
{
  tcache_load(p);
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

  max_x=(p_idx)?MAX_X:MAX_PX;

  if(tcache.xpos<MIN_X) {
    tcache.xpos.val=MIN_X;
    tcache.xv=-1;
  } else if(tcache.xpos>max_x) {
    if(!(gstate.mode&0xfe)) {
          tcache.xpos.val=max_x;
      tcache.xv=1;
    }
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
  
  if(gstate.mode==GMODE_PLAY) {
    if(p_idx==0) {
      // chu totoro
      key=PEEK(197);
      js=joy2();
      if(js) {
	if(js&0x04) key = 10; // left
	if(js&0x08) key = 18; // right
	if(js&0x10) key = 60; // button
      
	if(js&0x01) key = 60; // up?
      }
    } else {
      // chibi totoro
      if(((totoro[0].state==JUMP) && (totoro[1].state!=JUMP))
	 && ((totoro[1].xv==0)|| same_direction(totoro[0].xv,totoro[1].xv)) ) {
	key=60;
        tcache.xv=totoro[0].xv; // must use tcache to prevent overwriting
      } else {
	if((totoro[0].xpos.val-totoro[1].xpos.val)>50) key=18;
	else if((totoro[0].xpos.val-totoro[1].xpos.val)<-74) key=10;
	else key=0;
      }
    }
  } else key=18; // simulate 'D'
  
  if(tcache.state!=JUMP)
    switch(key) {
    case 10: // A
      if(tcache.xv>-MAX_XV) {
	tcache.xv-=2;
	if(tcache.xv>0) {
	  tcache.state=BRAKE;
	} else {
	  tcache.state=RUN;
	}
      }
      break;
    case 18: // D
      if(tcache.xv<MAX_XV) {
	tcache.xv+=2;
	if(tcache.xv<0) {
	  tcache.state=BRAKE;
	} else {
	  tcache.state=RUN;
	}
      }
      break;
    case 60: // space
      // make chibi totoro jump higher
      tcache.yv.val=(p_idx)?-(JUMP_V+JUMP_A):-JUMP_V;
      tcache.state=JUMP;
      break;
    default:
      if(tcache.xv>0) {
	tcache.xv--;
	tcache.state=BRAKE;
      } else if(tcache.xv<0) {
	tcache.xv++;
	tcache.state=BRAKE;
      } else {
	tcache.state=IDLE;
      }
      break;
    }
}

#if 1
#pragma register-vars (on)
void __fastcall__ check_collision(void)
{
  register struct acorn_t *a;
  static uint8_t ty1, ty2;
  static int16_t tx1, tx2;

  ty1=tcache.ypos.hi-20;
  ty2=tcache.ypos.hi+42;
  tx1=tcache.xpos.uval-15;
  tx2=tcache.xpos.uval+36;

//  VIC.bordercolor=COLOR_WHITE;
  for(a=acorn;a!=acorn+8;a++) {
    if(a->en) {
      if( ((a->ypos.hi)>ty1)   &&
	  ((a->ypos.hi)<ty2)   &&
	  ((a->xpos.uval)>tx1) &&
	  ((a->xpos.uval)<tx2) ) {
	switch(a->en) {
	case 1:
	  start_sound();
	  /* VIC.spr_ena&=~(0x10<<a) */
	  gstate.score+=10+(PGROUND_Y-tcache.ypos.hi);
	  if(gstate.acorns) gstate.acorns--;
	  break;
	case 2:
	  break;
	}
	a->en=0;
      }
    }
  }
  //  VIC.bordercolor=COLOR_BLUE;
}

#else

void __fastcall__ check_collision(void)
{
  static uint8_t ty1, ty2;
  static int16_t tx1, tx2;
  static uint8_t i;

  ty1=tcache.ypos.hi-20;
  ty2=tcache.ypos.hi+42;
  tx1=tcache.xpos.uval-15;
  tx2=tcache.xpos.uval+36;

  //  static uint8_t color;
  VIC.bordercolor=COLOR_CYAN;
  for(i=0;i<MAX_ACORNS;i++) {
    if(acorn[i].en) {
      if(((acorn[i].ypos.hi)>ty1)  &&
	 ((acorn[i].ypos.hi)<ty2)  &&
	 ((acorn[i].xpos.val)>tx1) &&
	 ((acorn[i].xpos.uval)<tx2) ) {
	  acorn[i].en=0;
	  start_sound();
//	   VIC.spr_ena&=~(0x10<<a) 
	  gstate.score+=10+(PGROUND_Y-tcache.ypos.hi);
	  if(gstate.acorns) gstate.acorns--;
	}
      }
    }
  VIC.bordercolor=COLOR_GREEN;
}

#endif
