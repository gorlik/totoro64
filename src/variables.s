;******************************************************************************
;*  TOTORO64                                                                  *
;*  A Studio Ghibli inspired game for the Commodore 64                        *
;*  Copyright 2021 Gabriele Gorla                                             *
;*                                                                            *
;*  This program is free software: you can redistribute it and/or modify      *
;*  it under the terms of the GNU General Public License as published by      *
;*  the Free Software Foundation, either version 3 of the License, or         *
;*  (at your option) any later version.                                       *
;*                                                                            *
;*  TOTORO64 is distributed in the hope that it will be useful,               *
;*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
;*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
;*  GNU General Public License for more details.                              *
;*                                                                            *
;*  You should have received a copy of the GNU General Public License         *
;*  along with TOTORO64.  If not, see <http://www.gnu.org/licenses/>.         *
;*                                                                            *
;******************************************************************************

.exportzp       _line_ptr
.exportzp       _temp_ptr
.exportzp       _zptmp1
.exportzp       _zptmp2


.export _VIC_BASE
.export _SCREEN_BASE
.export _SPR_PTR
.export _SPR_DATA
.export _VIC_CHARSET
.export _BITMAP_BASE
.export _CHARSET
.export _VIC_REG
.export _COLOR_BASE

.segment "ZP_2" : zeropage
;;  ZP_2 range 0xF7-0xFE
;; points to netx location to write in the bitmap
_line_ptr:
	.res 2

;; temporary zero page storage
_temp_ptr:
_zptmp1:
	.res 1
_zptmp2:
	.res 1

_VIC_BASE    = $4000
_SCREEN_BASE = _VIC_BASE
_SPR_PTR     = _SCREEN_BASE + $3f8
_SPR_DATA    = _VIC_BASE + $400
_VIC_CHARSET = _VIC_BASE + $1800
_BITMAP_BASE = _VIC_BASE   + $2000
_CHARSET     = $c000
_VIC_REG     = $d000
_COLOR_BASE  = $d800

