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
.export _CLR_TOP

.segment "BSS"
t1:
	.res 1


.segment	"CODE"

; ****************** PutChar *********************
;  Put a character
.proc _PutCharHR: near
	;; X: input char
	ldy #$00
.repeat 8,cline
	lda _charset+cline*256,x	;
	sta (_line_ptr),y
	iny
.endrepeat

	lda _line_ptr
	clc
	adc #$08
	bcc no_carry
	inc _line_ptr+1
no_carry:
	sta _line_ptr
	rts
.endproc


; ****************** PutLine *********************
;  Put a line of text
	;; t1: counter for next char to write
.proc _PutLine: near
	ldy #$FF
start:	iny
	sty t1
	ldx _STR_BUF,y		; next char to print
	beq end			; test for null terminator
	jsr _PutCharHR		; char to print is in X
        lda t1
	cmp #40
	beq end			; max 40 characters
	tay
	jmp start
end:
 	rts
	.endproc

; ****************** clr top *********************
;  Clear top 2 lines of screen
.proc _CLR_TOP: near
	ldx #160
	lda #0
@loop:  sta _SCR_BASE-1,x
	sta _SCR_BASE+160-1,x
	sta _SCR_BASE+320-1,x
	sta _SCR_BASE+480-1,x
	dex
	bne @loop
	rts
.endproc
