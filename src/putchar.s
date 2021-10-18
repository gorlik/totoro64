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
.include "totoro64.inc"

.import _charset

.export _PutLine
.export _CLR_TOP
.export _CLR_CENTER
.export _print_acorn
.export _print_hourglass
.export _print_p
.export _string_pad

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
@loopb: sta _BITMAP_BASE-1,x
	sta _BITMAP_BASE+160-1,x
	sta _BITMAP_BASE+320-1,x
	sta _BITMAP_BASE+480-1,x
	dex
	bne @loopb
	ldx #80
	lda #1 			; COLOR_WHITE
@loopc:	sta $d800-1,x
	dex
	bne @loopc
	rts
.endproc

.proc _CLR_CENTER: near
	ldx #144
	lda #0
@loopb: sta _BITMAP_BASE+88-1,x
	sta _BITMAP_BASE+320+88-1,x
	dex
	bne @loopb
	ldx #18
	lda #1 			; COLOR_WHITE
@loopc:	sta $d800+11-1,x
	sta $d800+40+11-1,x
	dex
	bne @loopc
	rts
.endproc


.segment	"BSS"

cpos:
	.res	1,$00
ccolor:
	.res	1,$00

; ---------------------------------------------------------------
; void __near__ __fastcall__ print_p (unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_print_p: near

.segment	"CODE"

	ldx     #'1' 		; '
	cmp     #$15		; a>20 ?
	bcc     skip
	inx			; '1'+1 = '2'
skip:	stx     _STR_BUF
	ldx     #$D0
	stx     _STR_BUF+1
	ldx     #0
	stx     _STR_BUF+2
	jsr     _convprint_big	; position is still in a
	rts
.endproc


; ---------------------------------------------------------------
; void __near__ __fastcall__ print_acorn (unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_print_acorn: near


	sta     cpos
	ldy     #$09		; BROWN
	sty     ccolor
	ldy     #$2F
	jmp     print_color_char
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ print_hourglass (unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_print_hourglass: near

	sta     cpos
	ldy     #$03		; CYAN
	sty     ccolor
	ldy     #$2E
	jmp     print_color_char
.endproc

.proc	print_color_char: near
;;  char in y
;; pos in a and cpos
;;;  color in ccolor
	sty     _STR_BUF
	ldy     #$00
	sty     _STR_BUF+1
	;; 	lda     temp_b
	jsr     _convprint_big
	ldy     cpos
	lda     ccolor
	sta     $D800,y
	sta     $D801,y
	sta     $D828,y
	sta     $D829,y
	rts
	.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ string_pad (unsigned char)
; ---------------------------------------------------------------

.segment	"BSS"
pad:
	.res	1,$00

.segment	"CODE"

.proc	_string_pad: near

	sta pad
	tay
	ldx #$FF
lloop:	inx
	lda _STR_BUF,x
	bne lloop

	cpx pad
	beq end
	;;  	ldy pad
cloop1:
	lda _STR_BUF,x
	sta _STR_BUF,y
	dey
	dex
	bpl cloop1
	lda #' '
cloop2: sta _STR_BUF,y
	dey
	bpl cloop2
end:	rts

.endproc
