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
#include "totoro64.h"

#ifdef NTSC
const uint8_t FTableLo[] = {
  // C   C#    D     D#    E      F     F#    G    G#     A    A#   B
  0x0c, 0x1c, 0x2d, 0x3f, 0x52, 0x66, 0x7b, 0x92, 0xaa, 0xc3, 0xde, 0xfa,  // 1
  0x18, 0x38, 0x5a, 0x7e, 0xa4, 0xcc, 0xf7, 0x24, 0x54, 0x86, 0xbc, 0xf5,  // 2
  0x31, 0x71, 0xb5, 0xfc, 0x48, 0x98, 0xee, 0x48, 0xa9, 0x0d, 0x79, 0xea,  // 3
  0x62, 0xe2, 0x6a, 0xf8, 0x90, 0x30, 0xdc, 0x90, 0x52, 0x1a, 0xf2, 0xd4,  // 4
  0xc4, 0xc4, 0xd4, 0xf0, 0x20, 0x60, 0xb8, 0x20, 0xa4, 0x34, 0xe4, 0xa8,  // 5
  0x88, 0x88, 0xa8, 0xe0, 0x40, 0xc0, 0x70, 0x40, 0x48, 0x68, 0xc8, 0x50,  // 6
  0x10, 0x10, 0x50, 0xc0, 0x80, 0x80, 0xe0, 0x80, 0x90, 0xd0, 0x90, 0xa0,  // 7
  0x20, 0x20, 0xa0, 0x80, 0x00, 0x00, 0xc0, 0x00, 0x20, 0xa0, 0x20, 0x40,  // 8
};

const uint8_t FTableHi[] = {
  // C   C#    D     D#    E      F     F#    G    G#     A    A#   B
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  // 1
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03,  // 2
  0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x07,  // 3
  0x08, 0x08, 0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f,  // 4
  0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1f,  // 5
  0x21, 0x23, 0x25, 0x27, 0x2a, 0x2c, 0x2f, 0x32, 0x35, 0x38, 0x3b, 0x3f,  // 6
  0x43, 0x47, 0x4b, 0x4f, 0x54, 0x59, 0x5e, 0x64, 0x6a, 0x70, 0x77, 0x7e,  // 7
  0x86, 0x8e, 0x96, 0x9f, 0xa9, 0xb3, 0xbd, 0xc9, 0xd5, 0xe1, 0xef, 0xfd,  // 8
};
#else
const uint8_t FTableLo[] = {
  // C   C#    D     D#    E      F     F#    G    G#     A    A#   B
  0x17, 0x27, 0x39, 0x4b, 0x5f, 0x74, 0x8a, 0xa1, 0xba, 0xd4, 0xf0, 0x0e,  // 1
  0x2d, 0x4e, 0x71, 0x96, 0xbe, 0xe8, 0x14, 0x43, 0x74, 0xa9, 0xe1, 0x1c,  // 2
  0x5a, 0x9c, 0xe2, 0x2d, 0x7c, 0xcf, 0x28, 0x85, 0xe8, 0x52, 0xc1, 0x37,  // 3
  0xb4, 0x39, 0xc5, 0x5a, 0xf7, 0x9e, 0x4f, 0x0a, 0xd1, 0xa3, 0x82, 0x6e,  // 4
  0x68, 0x71, 0x8a, 0xb3, 0xee, 0x3c, 0x9e, 0x15, 0xa2, 0x46, 0x04, 0xdc,  // 5
  0xd0, 0xe2, 0x14, 0x67, 0xdd, 0x79, 0x3c, 0x29, 0x44, 0x8d, 0x08, 0xb8,  // 6
  0xa1, 0xc5, 0x28, 0xcd, 0xba, 0xf1, 0x78, 0x53, 0x87, 0x1a, 0x10, 0x71,  // 7
  0x42, 0x89, 0x4f, 0x9b, 0x74, 0xe2, 0xf0, 0xa6, 0x0e, 0x33, 0x20, 0xff,  // 8
};

const uint8_t FTableHi[] = {
  // C   C#    D     D#    E      F     F#    G    G#     A    A#   B
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,  // 1
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04,  // 2
  0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x08,  // 3
  0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x10,  // 4
  0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18, 0x1a, 0x1b, 0x1d, 0x1f, 0x20,  // 5
  0x22, 0x24, 0x27, 0x29, 0x2b, 0x2e, 0x31, 0x34, 0x37, 0x3a, 0x3e, 0x41,  // 6
  0x45, 0x49, 0x4e, 0x52, 0x57, 0x5c, 0x62, 0x68, 0x6e, 0x75, 0x7c, 0x83,  // 7
  0x8b, 0x93, 0x9c, 0xa5, 0xaf, 0xb9, 0xc4, 0xd0, 0xdd, 0xea, 0xf8, 0xff,  // 8
};
#endif


const uint8_t track1[] =
  {
   0xFE,0x10, 	// set instrument
   0xFD,0x10,

   0x02,0x2b, 	// 1
   0x01,0x28,
   0x02,0x24,
   0x02,0x2b,

   0x02,0x29,
   0x07,0x26,

   0x02,0x29,
   0x01,0x26,
   0x02,0x23,
   0x02,0x29,
   0x02,0x28,
   0x07,0x24,

   0x05,0x2d, 	// 5
   0x01,0x2f,
   0x01,0x2b,
   0x01,0x29,
   0x04,0x2b,
   0x01,0x00, 	// pause
   0x01,0x2d,
   0x01,0x29,
   0x01,0x28,

   0x04,0x29,
   0x01,0x00, 	// pause
   0x01,0x21,
   0x01,0x24,
   0x02,0x29,

   0x01,0x21,
   0x06,0x21,

	        // repeat
   0x02,0x00, 	// 9 pause
   0x02,0x24,
   0x02,0x23,
   0x01,0x24,

   0x09,0x1f,

   0x02,0x00, 	// pause
   0x02,0x24,
   0x02,0x23,
   0x01,0x24,

   0x09,0x28,

   0x02,0x29, 	// 13
   0x02,0x28,
   0x02,0x26,
   0x01,0x24,
   0x03,0x29,
   0x02,0x28,
   0x02,0x26,
   0x02,0x24,

   0x03,0x24,
   0x01,0x26,
   0x04,0x26,
   0x03,0x30,
   0x01,0x32,
   0x04,0x32,

   0x02,0x00, 	// 17
   0x02,0x24,
   0x02,0x23,
   0x01,0x24,
   0x09,0x1f,

   0x02,0x00,
   0x02,0x24,
   0x02,0x23,
   0x01,0x24,
   0x09,0x2b,

   0x01,0x29, 	// 21
   0x01,0x29,
   0x01,0x29,
   0x01,0x29,
   0x01,0x29,
   0x01,0x28,
   0x01,0x26,
   0x05,0x29,

   0x01,0x00,
   0x01,0x26,
   0x01,0x28,
   0x01,0x29,

   0x02,0x28,
   0x02,0x28,
   0x01,0x28,
   0x01,0x26,
   0x01,0x24,
   0x09,0x28,

   0x02,0x21,	// 25
   0x02,0x23,
   0x02,0x24,
   0x01,0x26,
   0x03,0x21,
   0x02,0x23,
   0x01,0x24,
   0x01,0x26,
   0x01,0x24,
   0x09,0x2b,
   0x04,0x00,
   0x01,0x24,
   0x01,0x26,
   0x01,0x28,
   0x01,0x29,

   0x02,0x2b,  	// 29
   0x01,0x28,
   0x02,0x24,
   0x02,0x2b,
   0x02,0x29,
   0x07,0x26,

   0x02,0x29,
   0x01,0x26,
   0x02,0x23,
   0x02,0x29,
   0x02,0x28,
   0x07,0x24,

   0x02,0x00, 	// 33
   0x02,0x21,
   0x02,0x24,
   0x02,0x29,

   0x02,0x28,
   0x01,0x2b,
   0x04,0x24,
   0x01,0x28,

   0x01,0x29,
   0x01,0x28,
   0x01,0x29,
   0x01,0x28,
   0x01,0x29,
   0x01,0x28,
   0x01,0x24,
   0x05,0x26,

   0x01,0x24,
   0x01,0x26,
   0x01,0x28,
   0x01,0x29,

   0x02,0x2b, 	// 37
   0x01,0x28,
   0x02,0x24,
   0x02,0x2b,
   0x02,0x29,
   0x07,0x26,

   0x02,0x29,
   0x01,0x26,
   0x02,0x23,
   0x02,0x29,
   0x02,0x28,
   0x07,0x24,

   0x02,0x21, 	// 41
   0x02,0x2d,
   0x01,0x2b,
   0x01,0x29,
   0x01,0x28,
   0x01,0x29,
   0x03,0x2b,
   0x01,0x24,
   0x03,0x24,
   0x01,0x28,

   0x01,0x29,
   0x01,0x28,
   0x01,0x24,
   0x01,0x29,
   0x01,0x28,
   0x01,0x24,
   0x01,0x2d,
   0x09,0x2b,

   0x02,0x00, 	// 45
   0x01,0x1f,
   0x01,0x1f,

   0x01,0x29,
   0x01,0x28,
   0x01,0x26,
   0x01,0x28,

   0x08,0x24,
   0x04,0x00,
   0xff,0xff,
  };
