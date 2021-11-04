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
.import   _CHARSET
.import   _SCREEN_BASE
.import   _BITMAP_BASE
.import   _COLOR_BASE
.import   _STR_BUF

.export   _PutLine
.export   _PutBigLine
.export   _printbigat
.export   _CLR_TOP
.export   _CLR_CENTER
.export   _print_col
.export   _print_acorn
.export   _print_hourglass

.segment "BSS"
;;  can't put these in zptmp because printbig uses temp_ptr
t1:
	.res 1
line1:
	.res 1
save_ptr:
	.res 2
ccolor:
	.res 1
cpos:
	.res 1

.segment	"CODE"

save_line_ptr:
	lda _line_ptr
	sta save_ptr
	lda _line_ptr+1
	sta save_ptr+1
	rts

next_line:
	clc
	lda save_ptr
	adc #64
	sta _line_ptr
	lda save_ptr+1
	adc #1
	sta _line_ptr+1
	rts


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
	adc #<_BITMAP_BASE	; add lsb and store to _line_ptr
	sta _line_ptr 		;
	tya
	adc #>_BITMAP_BASE	; add msb and store to _line_ptr+1
	sta _line_ptr+1
	rts
.endproc

; ****************** PutChar *********************
;  Put a character
.proc _PutCharHR: near
	;; X: input char
	ldy #$00
.repeat 8,cline
	lda _CHARSET+cline*256,x	;
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
;  t1: counter for next char to write
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
;  t1: counter for next char to write
.proc _PutBigLine: near
	jsr save_line_ptr
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
	jsr next_line
	ldy #$ff
	bne loop ; branch always
ret:
 	rts
.endproc

; *********** printbigat *********************
;  print a line of large text
;  a: position
.proc _printbigat: near
	jsr     SetLinePtr
	jmp     _PutBigLine	; position is still in a
.endproc

; ****************** clr top *********************
;  Clear top 2 lines of screen
.proc _CLR_TOP: near
	ldx #160
	lda #0
loopb:  sta _BITMAP_BASE-1,x
        sta _BITMAP_BASE+160-1,x
	sta _BITMAP_BASE+320-1,x
	sta _BITMAP_BASE+480-1,x
	dex
	bne loopb
	ldx #80
loop:	lda #$FB 		; light gray and dark gray
	;	lda #$78                ; YELLOW and ORANGE
	sta _SCREEN_BASE-1,x
	lda #1 			; COLOR_WHITE
	sta _COLOR_BASE-1,x
	dex
	bne loop
	rts
.endproc

; ****************** clr center *********************
;  Clear center area of top bar
.proc _CLR_CENTER: near
	ldx #144
	lda #0
loopb:  sta _BITMAP_BASE+88-1,x
	sta _BITMAP_BASE+320+88-1,x
	dex
	bne loopb
	ldx #18
loop:	lda #$FB 		; light gray and dark gray
	sta _SCREEN_BASE+11-1,x
	sta _SCREEN_BASE+40+11-1,x
	lda #1 			; COLOR_WHITE
	sta _COLOR_BASE+11-1,x
	sta _COLOR_BASE+40+11-1,x
	dex
	bne loop
	rts
.endproc

; ---------------------------------------------------------------
; color_column
; ---------------------------------------------------------------
; position in Y
; color in A
; ---------------------------------------------------------------
color_column:
	sta _COLOR_BASE,y
	sta _COLOR_BASE+40,y
	lda #$78
	sta _SCREEN_BASE,y
	sta _SCREEN_BASE+40,y
	rts
	
; ---------------------------------------------------------------
; void __near__ __fastcall__ print_col(unsigned char)
; ---------------------------------------------------------------
.proc	_print_col: near
	ldx #146
        tay             ; forces evaluation of flags on A
	bpl skip
	dex
        dex
skip:	stx _STR_BUF
	ldx #0
	stx _STR_BUF+1
	and #$7f
	sta cpos
	jsr SetLinePtr
	jsr save_line_ptr
	jsr _PutLine
	jsr next_line
	inc _STR_BUF
	jsr _PutLine
	ldy cpos
	lda #$09		; BROWN
	jmp color_column
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ print_acorn (unsigned char)
; ---------------------------------------------------------------
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
.proc	_print_hourglass: near
	sta     cpos
	ldy     #$03		; CYAN
	sty     ccolor
	ldy     #222
	jmp     print_color_char
.endproc

; ---------------------------------------------------------------
; print_color_char
; ---------------------------------------------------------------
; char to print in Y
; position in A and cpos
; color in ccolor
; ---------------------------------------------------------------
.proc	print_color_char: near
	sty _STR_BUF
	ldy #$00
	sty _STR_BUF+1
	jsr _printbigat
	ldy cpos
	lda ccolor
	jsr color_column
	iny
	lda ccolor
	jmp color_column
.endproc

