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

struct player_t totoro[2];

static struct player_t tcache;
static uint8_t p_idx;

const uint8_t run_seq[] =
  {
    4,7,10,7, // run right
    16,19,22,19, // run left
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

void __fastcall__ tcache_save(uint8_t p)
{
  (void)p;
  __asm__("tax ");
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
  tcache.xv=0;
  tcache.yv.val=0;
  tcache.state=IDLE;
  tcache.blink=0;  
  
  if(p) {
    tcache.ypos.val=GROUND_Y-1;
    tcache.xpos.val=(MAX_PX-MIN_X)/2+50;
  } else {
    tcache.ypos.val=PGROUND_Y;
    tcache.xpos.val=(MAX_PX-MIN_X)/2;
  }
  
  tcache_save(p);
}

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
  VIC.spr0_x   = totoro[0].xpos.lo;
  VIC.spr1_x   = totoro[0].xpos.lo;
  VIC.spr2_x   = totoro[0].xpos.lo;

  if(totoro[0].xpos.hi)   VIC.spr_hi_x |=0x07;
  else VIC.spr_hi_x &= 0xf8;

  VIC.spr0_y   = totoro[0].ypos.lo;
  VIC.spr1_y   = totoro[0].ypos.lo;
  VIC.spr2_y   = totoro[0].ypos.lo;

  switch(totoro[0].state) {
  case IDLE:
    if(totoro[0].blink)   SPR_PTR[0]=3;
    else   SPR_PTR[0]=0;
    SPR_PTR[1]=1;
    SPR_PTR[2]=2;
    break;
  case RUN:
    if(totoro[0].xv>0) {
      SPR_PTR[0]=run_seq[gstate.anim_idx];
      SPR_PTR[1]=run_seq[gstate.anim_idx]+1;
      SPR_PTR[2]=run_seq[gstate.anim_idx]+2;
    } else {
      SPR_PTR[0]=run_seq[gstate.anim_idx+4];
      SPR_PTR[1]=run_seq[gstate.anim_idx+4]+1;
      SPR_PTR[2]=run_seq[gstate.anim_idx+4]+2;
    }
    break;
  case BRAKE:
    if(totoro[0].xv>0) {
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
    if(totoro[0].xv>0) {
      SPR_PTR[0]=run_seq[0];
      SPR_PTR[1]=run_seq[0]+1;
      SPR_PTR[2]=run_seq[0]+2;
    } else if (totoro[0].xv<0) {
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


void __fastcall__ chibi_set_pos(void)
{
  VIC.spr3_x   = totoro[1].xpos.lo;
  VIC.spr4_x   = totoro[1].xpos.lo;

  if(totoro[1].xpos.hi)   VIC.spr_hi_x |=0x18;
  else VIC.spr_hi_x &= 0xe7;

  VIC.spr3_y   = totoro[1].ypos.lo;
  VIC.spr4_y   = totoro[1].ypos.lo;

  switch(totoro[1].state) {
  case IDLE:
    if(totoro[1].blink) {
      SPR_PTR[3]=62;
      SPR_PTR[4]=61;
      VIC.spr_color[3]=COLOR_WHITE;
      VIC.spr_color[4]=COLOR_BLACK;
    } else {
      SPR_PTR[3]=61;
      SPR_PTR[4]=62;
      VIC.spr_color[3]=COLOR_BLACK;
      VIC.spr_color[4]=COLOR_WHITE;
    }
    break;
  case RUN:
  case BRAKE:
  case JUMP:
    if(totoro[1].xv>0) {
      //      SPR_PTR[3]=run_seq[gstate.anim_idx];
      //      SPR_PTR[4]=run_seq[gstate.anim_idx]+1;
      SPR_PTR[3]=65;
      SPR_PTR[4]=66;
    } else {
      SPR_PTR[3]=63;
      SPR_PTR[4]=64;
    }
    break;
    /*
  case BRAKE:
    if(totoro[1].xv>0) {
      SPR_PTR[3]=13;
      SPR_PTR[4]=14;
      //      SPR_PTR[2]=15;
    } else {
      SPR_PTR[3]=25;
      SPR_PTR[4]=26;
      //      SPR_PTR[2]=27;
    }
    break;
  case JUMP:
    if(totoro[1].xv>0) {
      SPR_PTR[3]=run_seq[0];
      SPR_PTR[4]=run_seq[0]+1;
      //      SPR_PTR[2]=run_seq[0]+2;    } else if (totoro[1].xv<0) {
      SPR_PTR[3]=run_seq[4];
      SPR_PTR[4]=run_seq[4]+1;
      //      SPR_PTR[2]=run_seq[4]+2;
    } else {
      // add eye movement based on up or down
      SPR_PTR[3]=0;
      SPR_PTR[4]=1;
      //      SPR_PTR[2]=2;
    }
    break;
    */
  }
}

void __fastcall__ totoro_update(uint8_t p)
{
  tcache_load(p);
  process_input();
  totoro_move();
  check_collision();
  tcache_save(p);
}

void __fastcall__ totoro_move()
{
  static uint16_t r;
 
  tcache.xpos.val+=(tcache.xv>>2);

  if(tcache.xpos<MIN_X) {
    tcache.xpos.val=MIN_X;
    tcache.xv=-1;
  } else if(tcache.xpos>MAX_PX) {
    if(!(gstate.mode&0xfe)) {
          tcache.xpos.val=MAX_PX;
      tcache.xv=1;
    }
  }

  tcache.ypos.val+=tcache.yv.val;

  if(tcache.state==JUMP) {
    tcache.yv++;
  }

  if(tcache.ypos>PGROUND_Y) {
    if(p_idx) tcache.ypos.val=GROUND_Y-1;
    else tcache.ypos.val=PGROUND_Y;
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
    key=PEEK(197);
    js=joy2();
    if(js) {
      if(js&0x04) key = 10; // left
      if(js&0x08) key = 18; // right
      if(js&0x10) key = 60; // button
      
      if(js&0x01) key = 60; // up?
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
      tcache.yv.val=-JUMP_V;
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

void __fastcall__ check_collision(void)
{
  //  static uint8_t color;
  
  for(ctmp=0;ctmp<MAX_ACORNS;ctmp++) {
    //    color=COLOR_ORANGE;
    if(acorn[ctmp].en) {
      if((((acorn[ctmp].ypos.hi))>(tcache.ypos.uval-20)) &&
	 (((acorn[ctmp].ypos.hi))<(tcache.ypos.uval+42)) ) {
	//	color=COLOR_YELLOW;
	if((tcache.xpos.val>(acorn[ctmp].xpos.val-36)) &&
	   (tcache.xpos.uval<(acorn[ctmp].xpos.uval+15)) ) {
	  //	  color=COLOR_RED;
	  //	    if(cr&(0x10<<ctmp)){
	  //color=COLOR_BLACK;
	  acorn[ctmp].en=0;
	  start_sound();
	  /* VIC.spr_ena&=~(0x10<<a) */
	  gstate.score+=10+(PGROUND_Y-tcache.ypos.val);
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
