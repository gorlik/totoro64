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
;*  GTERM is distributed in the hope that it will be useful,                  *
;*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
;*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
;*  GNU General Public License for more details.                              *
;*                                                                            *
;*  You should have received a copy of the GNU General Public License         *
;*  along with GTERM.  If not, see <http://www.gnu.org/licenses/>.            *
;*                                                                            *
;******************************************************************************
.include "totoro64.inc"

.import _charset

.export _PutLine
.export _PutCharHR

.segment "BSS"
t1:
	.res 1


.segment	"CODE"

; ****************** PutChar *********************
;  Put a character
.proc _PutCharHR: near
	;; c1: input char

	lda _c1
	bpl skip
	sec
	sbc #96
skip:
        tax
	ldx _c1
	ldy #$00
.repeat 8,cline
	lda _charset+cline*256,x	;
	sta (_line_addr),y
	iny
.endrepeat

	lda _line_addr
	clc
	adc #$08
	bne no_carry
	inc _line_addr+1
no_carry:
	sta _line_addr
	rts
.endproc


; ****************** PutLine *********************
;  Put a line of text
	;; c1: temp char to print
	;; t1: counter for next char to write
.proc _PutLine: near
	ldy #$00
start:
	lda _STR_BUF,y
	beq end
skip:	sta _c1
	iny
	sty t1
	jsr _PutCharHR
        lda t1
	cmp #40
	beq end
	tay
	jmp start
end:
 	rts
.endproc
