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

static unsigned char * const line[] = {
	        BITMAP_BASE+0,
			BITMAP_BASE+320*1,
			BITMAP_BASE+320*2,
			BITMAP_BASE+320*3,
			BITMAP_BASE+320*4,
			BITMAP_BASE+320*5,
			BITMAP_BASE+320*6,
			BITMAP_BASE+320*7,
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

static const unsigned char conv_table[] = {
   2,
   8, 12, 16, 20, 24, 28, 96, // '/' 0 .. 4
   100, 104, 108, 112, 116, // 5 .. 9
   120, 124, 128, 132, 136, 140, 144, 148, 152, // A..
   156, 160, 164, 168, 172, 176, 180, 184, 188, // .. R
   224, 228, 232, 236, 240, 244, 248, 252,      // S .. Z
};

void __fastcall__ print_col(uint8_t c)
{
  STR_BUF[0]=(c&COL_L)?7:6;
  STR_BUF[1]=0;
  c&=0x7f;
  printat(c,0);
  printat(c,1);
 
  POKE(COLOR_RAM+c,COLOR_BROWN);
  POKE(COLOR_RAM+40+c,COLOR_BROWN);
}


void __fastcall__ print_p(uint8_t c)
{
  STR_BUF[0]=(c>20)?'2':'1';
  STR_BUF[1]='P';
  STR_BUF[2]=0;
  convprint_big(c);
}

void __fastcall__ print_hourglass(uint8_t c)
{
  STR_BUF[0]='.';
  STR_BUF[1]=0;
  convprint_big(c);
  POKE(COLOR_RAM+c,COLOR_CYAN);
  POKE(COLOR_RAM+1+c,COLOR_CYAN);
  POKE(COLOR_RAM+40+c,COLOR_CYAN);
  POKE(COLOR_RAM+41+c,COLOR_CYAN);
}
void __fastcall__ print_acorn(uint8_t c)
{
  STR_BUF[0]='/';
  STR_BUF[1]=0;
  convprint_big(c);
  POKE(COLOR_RAM+c,COLOR_BROWN);
  POKE(COLOR_RAM+1+c,COLOR_BROWN);
}

void __fastcall__ printat(uint8_t x, uint8_t y)
{
  line_ptr=(line[y]+(x<<3));
  PutLine();
}

void __fastcall__ convprint_big(uint8_t x)
{
  convert_big();
  printbigat(x,0);
}

void __fastcall__ convert_big(void)
{
   static uint8_t j,idx;

   for(j=0;STR_BUF[j];j++) {
    if(STR_BUF[j]>='.' && STR_BUF[j]<='9') {
      idx=STR_BUF[j]+1-'.';
    } else if(STR_BUF[j]>='A' && STR_BUF[j]<='Z') {
      idx=STR_BUF[j]+13-'A';
    } else idx=0;
    // printf("c: %c %d = %d\n",STR_BUF[j],STR_BUF[j],char_table[idx]);
    STR_BUF[j+41]=conv_table[idx];
  }
   STR_BUF[j+41]=0;
}

void __fastcall__ printbigat(uint8_t x, uint8_t y)
{
  static uint8_t j,k;

  /*   for(j=0;STR_BUF[j];j++) {
    if(STR_BUF[j]>='0' && STR_BUF[j]<='9') {
      idx=STR_BUF[j]+1-'0';
    } else if(STR_BUF[j]>='A' && STR_BUF[j]<='Z') {
      idx=STR_BUF[j]+11-'A';
    } else idx=0;
    // printf("c: %c %d = %d\n",STR_BUF[j],STR_BUF[j],char_table[idx]);
    STR_BUF[j+40]=char_table[idx];
  }
   STR_BUF[j+40]=0;
  */

  for(k=0,j=41;STR_BUF[j];j++) {
    STR_BUF[k]=(STR_BUF[j])+0;
    k++;
    STR_BUF[k]=((STR_BUF[j])+1);
    k++;
  }
  STR_BUF[k]=0;
  printat(x,y);

  for(k=0,j=41;STR_BUF[j];j++) {
    STR_BUF[k]=(STR_BUF[j])+2;
    k++;
    STR_BUF[k]=((STR_BUF[j])+3);
    k++;
  }
  STR_BUF[k]=0;
  printat(x,y+1);

}

void __fastcall__ string_pad(int8_t pad)
{
  static int8_t delta, i;
  delta=pad-strlen(STR_BUF);

  if(delta==0) return;

  for(i=pad;i>=0;i--)
    if((i-delta)>=0) STR_BUF[i]=STR_BUF[i-delta];
    else STR_BUF[i]=' ';
}
