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
#include <c64.h>
#include <zlib.h>
#include "totoro64.h"

// lowercase
#pragma charmap(97,1)
#pragma charmap(98,2)
#pragma charmap(99,3)
#pragma charmap(100,4)
#pragma charmap(101,5)
#pragma charmap(102,6)
#pragma charmap(103,7)
#pragma charmap(104,8)
#pragma charmap(105,9)
#pragma charmap(106,10)
#pragma charmap(107,11)
#pragma charmap(108,12)
#pragma charmap(109,13)
#pragma charmap(110,14)
#pragma charmap(111,15)
#pragma charmap(112,16)
#pragma charmap(113,17)
#pragma charmap(114,18)
#pragma charmap(115,19)
#pragma charmap(116,20)
#pragma charmap(117,21)
#pragma charmap(118,22)
#pragma charmap(119,23)
#pragma charmap(120,24)
#pragma charmap(121,25)
#pragma charmap(122,26)

// uppercase
#pragma charmap(65,65)
#pragma charmap(66,66)
#pragma charmap(67,67)
#pragma charmap(68,68)
#pragma charmap(69,69)
#pragma charmap(70,70)
#pragma charmap(71,71)
#pragma charmap(72,72)
#pragma charmap(73,73)
#pragma charmap(74,74)
#pragma charmap(75,75)
#pragma charmap(76,76)
#pragma charmap(77,77)
#pragma charmap(78,78)
#pragma charmap(79,79)
#pragma charmap(80,80)
#pragma charmap(81,81)
#pragma charmap(82,82)
#pragma charmap(83,83)
#pragma charmap(84,84)
#pragma charmap(85,85)
#pragma charmap(86,86)
#pragma charmap(87,87)
#pragma charmap(88,88)
#pragma charmap(89,89)
#pragma charmap(90,90)

#define TXT_PTR 0x400
#define AT(r,c) (TXT_PTR+40*r+c)

#define scr_cpy8(dst,src,color)  do {	\
    __asm__("ldx #$FF");		\
    __asm__("clv");			\
    __asm__("lc%s: inx",__LINE__);	\
    __asm__("lda #%b",color);		\
    __asm__("lda %v,x",src);		\
    __asm__("beq fs%v",src);		\
    __asm__("sta %w,x",dst);		\
    __asm__("lda #%b",color);		\
    __asm__("sta $d400+%w,x",dst);	\
    __asm__("bvc lc%s",__LINE__);	\
    __asm__("fs%v:",src);		\
  } while (0)


const unsigned char version_txt[] = VERSION;
const unsigned char present_txt[] = "Presents";
const unsigned char intro_txt[] = "A Commodore 64 tribute to Studio Ghibli";

#if (DEBUG==0)
//#if 0
const unsigned char license_txt[] =
  "   Copyright (c) 2021 Gabriele Gorla    "
  " Licensed under the GNU GPL v3 or later ";
//  "This program is free software: you can  "
//  "redistribute it and/or modify it under  "
//  "the terms of the GNU General Public     "
//  "License version 3 or any later version. ";
//  "License as published by the Free        "
//  "Software Foundation either License      "
//  "version 3 or (at your option) any later "
//  "version.";
#endif

const unsigned char url[] =
  "http://gglabs.us";


static const uint8_t tpos[17] = {
  SPR_CENTER_X-48, 60,
  SPR_CENTER_X-24, 60,
  SPR_CENTER_X,    60,
  SPR_CENTER_X+24, 60,
  SPR_CENTER_X-96, 112,
  SPR_CENTER_X-48, 112,
  SPR_CENTER_X,    112,
  SPR_CENTER_X+48, 112,
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


static void __fastcall__ setup_sid(void);
static void __fastcall__ Title_Sprite_Setup(void);

void __fastcall__ setup(void)
{
  inflatemem (SPR_DATA, sprite_src_data);
  
  __asm__("ldy #0");
  __asm__("loop:");
  __asm__("lda %v+%w-1,y",VIC_BASE,SPR_GGLABS_1*64);
  __asm__("sta %v-%w-1,y",VIC_BASE,64*8);
  
  __asm__("lda %v+%w-1,y",VIC_BASE,SPR_TITLE_BOLD_1*64);
#ifdef MOVIE_TITLE
  __asm__("cpy #192");
  __asm__("bcs skip");
  __asm__("lda %v+%w-1,y",VIC_BASE,SPR_TITLE_MOVIE_1*64);
  __asm__("skip:");
#endif
  __asm__("sta %v-%w-1,y",VIC_BASE,64*4);
  
  __asm__("dey");
  __asm__("bne loop");
  
  setup_sid();
  
  irq_ctrl=0;
  CIA1.icr=0x7f; // disable all CIA1 interrupts
  *((unsigned int *)0x0314)=(unsigned int)IRQ;
  VIC.rasterline=1;
  VIC.imr=0x1; // enable raster interrupt
  VIC.bgcolor[0]=COLOR_WHITE;
  CIA1.pra=0;
  CIA1.prb=0;
  CIA1.ddra=0;
  CIA1.ddrb=0;
  
  Title_Sprite_Setup();
  //  mode_text();
  VIC.spr_ena=0x0f;


  scr_cpy8(AT(6,16),present_txt,COLOR_LIGHTBLUE);
  inflatemem (CHARSET_PTR, charset_data);
  VIC.spr_ena=0xff;
  
  scr_cpy8(AT(12,33),version_txt,COLOR_BLACK);
  scr_cpy8(AT(15,0),intro_txt,COLOR_LIGHTBLUE);
#if (DEBUG==0)
  scr_cpy8(AT(19,0),license_txt,COLOR_LIGHTBLUE);
#endif

  scr_cpy8(AT(23,12),url,COLOR_BLACK);
  
  inflatemem (BITMAP_BASE, bitmap_data);
  inflatemem (SCREEN_BASE, color1_data);
  CLR_TOP();
  
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
}


static void __fastcall__ setup_sid(void)
{
  memset8c(0xd400,0,24); // SID address
  SID.amp = 0x1f;        // set volume to 15

  memset8s(track,0,sizeof(struct track_t)*2);

  track[0].ptr = track[0].restart_ptr = (uint16_t)track0_data;
  //  track[0].voice_offset=0;

  track[1].ptr = track[1].restart_ptr = (uint16_t)track1_data;
  track[1].voice_offset=7;

  vpb = VPB;

  //  sound effects
  SID.v3.ad = 0x00;
  SID.v3.sr = 0xa9;
}

static void __fastcall__ Title_Sprite_Setup(void)
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
  memcpy8c(0xd000,tpos,sizeof(tpos));

  //  setup sprite colors
  memcpy8c(0xd027,tcol,sizeof(tcol));
}
