/******************************************************************************
 *  TOTORO64                                                                  *
 *  A Studio Ghibli inspired game for the Commodore 64                        *
 *  Copyright 2022 Gabriele Gorla                                             *
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

#include "totoro64.h"
#include <c64.h>

struct sound_t sound;

const uint16_t sound_seq[] = {
  0x22d0,
  0x1f04,
  0x2714,
  0x2e79,
  0x2714,
};

const uint16_t ftable2[] = {
  26397,
  8809,
  11099
};

void __fastcall__ start_sound(uint8_t t)
{
  stop_sound();
  if(STATE_PLAY_DEMO()) {
    sound.type=t;
    sound.timer=10;
    sound.index=0;
  }
}

void __fastcall__ process_sound(void)
{
  if(!sound.type) return;
  
  if(sound.timer==0) {
    stop_sound();
    return;
  }

  switch(sound.type) {
  case SFX_ACORN:
    // use index 0 only the first time
    if(sound.index>=(sizeof(sound_seq)/2)) sound.index=1;
    SID.v3.freq=sound_seq[sound.index];
    if(sound.index==0) {
      SID.v3.ad = 0x00;
      SID.v3.sr = 0xa9;
      SID.v3.ctrl=0x21;
      sound.last=0x21;
    }
    sound.index++;
    break;
    /*
  case SFX_BONUS:
    SID.v3.freq=ftable2[sound.index];
    
    if(sound.index==0) {
      SID.v3.ad = 0x01;
      SID.v3.sr = 0xf9;
    
      SID.v3.ctrl=0x11;
      sound.last=0x11;
    }
    if((sound.timer&0x3)==0x3) sound.index++;
    break;
    */
    //  case SFX_SLOW:
    //  case SFX_INVERT:
    //    break;
  }

  sound.timer--;
}

void __fastcall__ stop_sound(void)
{
  SID.v3.ctrl=sound.last&0xf0;
  sound.type=SFX_NONE;
}
