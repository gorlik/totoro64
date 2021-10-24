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
#include <string.h>
#include <peekpoke.h>
#include <c64.h>
#include "totoro64.h"

#if (DEBUG)
const uint8_t hexdigit[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};
#endif

unsigned char * const line[] = {
  BITMAP_BASE+0,
  BITMAP_BASE+320*1,
#if (DEBUG)	
  BITMAP_BASE+320*2,
  BITMAP_BASE+320*3,
  BITMAP_BASE+320*4,
  BITMAP_BASE+320*5,
  BITMAP_BASE+320*6,
  BITMAP_BASE+320*7,
#endif
  /*			BITMAP_BASE+320*8,
			BITMAP_BASE+320*9,
			BITMAP_BASE+320*10,
			BITMAP_BASE+320*11,
			BITMAP_BASE+320*12,
			BITMAP_BASE+320*13,
			BITMAP_BASE+320*14,
			BITMAP_BASE+320*15,
			BITMAP_BASE+320*16,
			BITMAP_BASE+320*17,
			BITMAP_BASE+320*18,
			BITMAP_BASE+320*19,
			BITMAP_BASE+320*20,
			BITMAP_BASE+320*21,
			BITMAP_BASE+320*22,
			BITMAP_BASE+320*23,
			BITMAP_BASE+320*24,*/
};


void __fastcall__ print_col(uint8_t pos)
{
  static uint8_t c;

  (void)pos;
  //      c=pos;
  //      STR_BUF[0]=(c&COL_L)?7:6;
  __asm__("ldx #147");
  __asm__("sta %v",c);
  __asm__("bpl skip"); // COL_L is the sign bit
  __asm__("dex");
  __asm__("skip: stx %v",STR_BUF);
    
  STR_BUF[1]=0;
  c&=0x7f;
  printat(c,0);
  printat(c,1);

  //  POKE(COLOR_RAM+c,COLOR_BROWN);
  //  POKE(COLOR_RAM+40+c,COLOR_BROWN)
  __asm__("ldy %v",c);
  __asm__("lda #%b",COLOR_BROWN);
  __asm__("sta %w,y",0xd800);
  __asm__("sta %w,y",0xd800+40);
}

/*
static void __fastcall__ printat(uint8_t x, uint8_t y)
{
  line_ptr=(line[y]+(x<<3));
  PutLine();
}


static void __fastcall__ printbigat(uint8_t x)
{
  line_ptr=(line[0]+(x<<3));
  PutBigLine();
}
*/
