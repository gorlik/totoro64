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
   16, 20, 24, 28, 96, // 0 .. 4
   100, 104, 108, 112, 116, // 5 .. 9
   120, 124, 128, 132, 136, 140, 144, 148, 152, // A..
   156, 160, 164, 168, 172, 176, 180, 184, 188, // .. R
   224, 228, 232, 236, 240, 244, 248, 252,      // S .. Z
};

void __fastcall__ printat(unsigned char x, unsigned char y)
{
  line_ptr=(line[y]+(x<<3));
  PutLine();
}

void __fastcall__ convprint_big(unsigned char x)
{
  convert_big();
  printbigat(x,0);
}

void __fastcall__ convert_big(void)
{
   static unsigned char j,idx;

   for(j=0;STR_BUF[j];j++) {
    if(STR_BUF[j]>='0' && STR_BUF[j]<='9') {
      idx=STR_BUF[j]+1-'0';
    } else if(STR_BUF[j]>='A' && STR_BUF[j]<='Z') {
      idx=STR_BUF[j]+11-'A';
    } else idx=0;
    // printf("c: %c %d = %d\n",STR_BUF[j],STR_BUF[j],char_table[idx]);
    STR_BUF[j+41]=conv_table[idx];
  }
   STR_BUF[j+41]=0;
}

void __fastcall__ printbigat(unsigned char x, unsigned char y)
{
  static unsigned char j,k;

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
