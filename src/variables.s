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

.segment "ZP_2" : zeropage
;;  ZP_2 range 0xF7-0xFE
_line_ptr:
	.res 2
_track_ptr:
	.res 2
_t2ptr:
	.res 2
_c1:
	.res 1
_ctmp:
	.res 1


.exportzp       _line_ptr
.exportzp       _track_ptr
.exportzp       _t2ptr
.exportzp       _c1
.exportzp       _ctmp

_VIC_BASE   = $4000
_SCR_BASE   = _VIC_BASE   + $2000
_COLOR_BASE = _SCR_BASE   - $400
_SPR_PTR    = _COLOR_BASE + $3f8
_SPR_DATA   = _VIC_BASE

_charset    = $c000

.export _SCR_BASE
.export _COLOR_BASE
.export _SPR_DATA
.export _SPR_PTR
.export _charset
