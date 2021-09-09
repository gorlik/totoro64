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

const unsigned char present_txt[] = "Presents";
const unsigned char intro_txt[] = "A Commodore 64 tribute to Studio Ghibli";

#ifndef DEBUG
const unsigned char license_txt[] =
  "Copyright (c) 2021 Gabriele Gorla       "
  "This program is free software: you can  "
  "redistribute it and/or modify it under  "
  "the terms of the GNU General Public     "
  "License as published by the Free        "
  "Software Foundation either License      "
  "version 3 or (at your option) any later "
  "version.";
#endif
