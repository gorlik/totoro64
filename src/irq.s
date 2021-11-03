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
.include "c64.inc"

.define IPDebug     0
.include "macros.inc"

.import   _spr_mux
.import   spr_mux_irq
.import   play_track

.export   _IRQ
.export   color_save
.exportzp itmp1, itmp2

; must be equal to sizeof(struct track_t)
.define   TRACK_T_SIZE 10

;  zero page temporary storage for irq use
.segment "ZP_2" : zeropage
itmp1:
	.res 1
itmp2:
	.res 1

.segment 	"BSS"
color_save:
	.res 1

.segment	"CODE"

;; ****************** IRQ Interrupt *********************
.proc _IRQ: near
	lda VIC_IRR
	bpl not_vic	; check if IRQ from VIC
	sta VIC_IRR	; clear VIC IRQ flag
        lda _spr_mux
        beq no_mux
        jsr spr_mux_irq
no_mux:
	ldx #0*TRACK_T_SIZE
	jsr play_track		; track[0]
	ldx #1*TRACK_T_SIZE
	jsr play_track		; track[1]
not_vic:
	jmp $EA81		; KERNAL IRQ end
.endproc
