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
.setcpu		"6502"
;.autoimport 	on

.importzp _line_ptr
.importzp _temp_ptr
.import   _charset
.import   _SCREEN_BASE
.import   _BITMAP_BASE	
.import   _STR_BUF
.import   _line

.export   _PutLine
.export   _PutBigLine
.export   _CLR_TOP
.export   _CLR_CENTER
.export   _print_acorn
.export   _print_hourglass
.export   _print_p

.segment "BSS"
t1:
	.res 1
line1:
	.res 1

.segment	"CODE"

; ****************** SetLinePtr *********************
;  Put a character
.proc SetLinePtr: near
	;; a: char position
	rol
	rol
	rol
	tay			; save the partial rorate in y
	and #$f8		; mask lsb
	tax			; save lsb in x
	tya
	rol			; rotate one more time (carry bit)
	and #$03 		; mask msb
	tay			; save msb in y
	clc
	txa 			;
	adc _line		; add lsb and store to _line_ptr
	sta _line_ptr 		;
	tya
	adc _line+1		; add msb and store to _line_ptr+1
	sta _line_ptr+1
	rts
.endproc

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
loop:	iny
	sty t1
	ldx _STR_BUF,y		; next char to print
	beq end			; test for null terminator
	jsr _PutCharHR		; char to print is in X
	ldy t1
	cpy #40
	bne loop			; max 40 characters
end: 	rts
.endproc

; ****************** PutBigLine *********************
;  Put a line of big text
	;; t1: counter for next char to write
.proc _PutBigLine: near
	clc
	lda _line_ptr
	adc #64
	sta _temp_ptr
	lda _line_ptr+1
	adc #1
	sta _temp_ptr+1
	ldy #$0
	sty line1
	dey ; loop needs $ff to start
loop:	iny
	sty t1
	lda _STR_BUF,y		; next char to print
	beq end			; test for null terminator
	sec
	sbc #192		; subtract the char offset
	asl			; multiply by 4
	asl
	tax
	lda line1
	beq skip	; branch if first line
	inx		; else increment the char twice
	inx
skip:
	jsr _PutCharHR		; char to print is in X
	inx
	jsr _PutCharHR		; char to print is in X	
	ldy t1
	cpy #20
	bne loop			; max 20 characters
end:
	lda line1
	bne ret
	inc line1
	lda _temp_ptr
	sta _line_ptr
	lda _temp_ptr+1
	sta _line_ptr+1
	ldy #$ff
	bne loop ; branch always
ret:
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
@loopc:	lda #1 			; COLOR_WHITE
	sta $d800-1,x
	;	lda #$78                ; YELLOW and ORANGE
	lda #$FB 		; light gray and dark gray
	sta _SCREEN_BASE-1,x
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
@loopc:	lda #1 			; COLOR_WHITE
	sta $d800+11-1,x
	sta $d800+40+11-1,x
	lda #$FB
	sta _SCREEN_BASE+11-1,x
	sta _SCREEN_BASE+40+11-1,x
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

	ldx     #231 		; '1'
	cmp     #$15		; a>20 ?
	bcc     skip
	inx			; '1'+1 = '2'
skip:	stx     _STR_BUF
	ldx     #'P'
	stx     _STR_BUF+1
	ldx     #0
	stx     _STR_BUF+2
	jsr     SetLinePtr
	jsr     _PutBigLine	; position is still in a
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
	ldy     #223
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
	ldy     #222
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
	jsr     SetLinePtr
	jsr     _PutBigLine
	ldy     cpos
	lda     ccolor
	sta     $D800,y
	sta     $D801,y
	sta     $D828,y
	sta     $D829,y
	lda #$78
	sta _SCREEN_BASE,y
	sta _SCREEN_BASE+1,y
	sta _SCREEN_BASE+40,y
	sta _SCREEN_BASE+41,y
	rts
.endproc
