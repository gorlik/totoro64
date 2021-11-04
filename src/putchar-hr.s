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
.import   _SCREEN_BASE
.import   _BITMAP_BASE
.import   _COLOR_BASE
.import   _STR_BUF
.import   _vspr_pos
.import   _vspr_pc
.import   _vspr_ctl

.export   _printat_f
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

; ****************** printat *********************
;  Print a line of text
;  a: position
.proc _printat_f: near
	tax
	ldy #$FF
loop:	iny
	lda _STR_BUF,y		; next char to print
	beq end			; test for null terminator
	sta _SCREEN_BASE,x
	inx
        cpy #40
	bne loop		; max 40 characters
end:	rts
.endproc

; *********** printbigat *********************
;  print a line of large text
;  a: position
.proc _printbigat: near
	tax
	ldy #$FF
loop:	iny
	lda _STR_BUF,y		; next char to print
	beq end			; test for null terminator
	sec

	sbc #192		; offset in char table
	asl
	asl			; multiply by 4
	clc
	sta _SCREEN_BASE,x
	adc #1
	sta _SCREEN_BASE+1,x
	adc #1
	sta _SCREEN_BASE+40,x
	adc #1
	sta _SCREEN_BASE+41,x	
	inx
	inx
        cpy #20
	bne loop		; max 20 characters
end:
 	rts
.endproc

; ****************** clr top *********************
;  Clear top 2 lines of screen
.proc _CLR_TOP: near
	lda #0
	sta _vspr_ctl+6+1 	; disable all sprites
	ldx #80
loop:	lda #0
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
	lda #0
	sta _vspr_ctl+6+1 	; disable all sprites
	ldx #18
loop:	lda #0
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
	rts
	
; ---------------------------------------------------------------
; void __near__ __fastcall__ print_col(unsigned char)
; ---------------------------------------------------------------
.proc	_print_col: near
	ldx #146
        tay                     ; forces evaluation of flags on A
	bpl skip
	dex
        dex
skip:	and #$7f
	tay                     ; position is now in Y
	txa
	sta _SCREEN_BASE,y
        inx
	txa
	sta _SCREEN_BASE+40,y
	lda #$09		; BROWN
	jmp color_column
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ print_acorn (unsigned char)
; ---------------------------------------------------------------
.proc	_print_acorn: near
	sta	cpos
	ldy     #42+16
	ldx     #6-5
	cmp #20			;use spr 5 if pos >20
	bmi skip
	dex
skip:ldx #0
	lda     #$08		; ORANGE
	jmp     print_color_char
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ print_hourglass (unsigned char)
; ---------------------------------------------------------------
.proc	_print_hourglass: near
	sta	cpos
	ldy     #43+16
	ldx     #7-5
	lda	#$03 		; CYAN
	jmp print_color_char
.endproc

; ---------------------------------------------------------------
; print_color_char
; ---------------------------------------------------------------
; char to print in Y
; position in cpos
; sprite idx x 
; color in A 
; ---------------------------------------------------------------
smask:
	.byte $20, $40, $80
	
.proc	print_color_char: near
	sta     _vspr_pc+6+3,x 	; spr color
	tya
	sta     _vspr_pc+6,x	; spr ptr
	lda     smask,x
	ora     _vspr_ctl+6+1	; enable the sprite
	sta     _vspr_ctl+6+1	
	
	txa
	asl
	tax			; index *= 2
	lda 	#44
	sta	_vspr_pos+6+1,x	; pos Y

	lda cpos
	asl
	asl
	asl
	clc
	adc #24
	
	sta 	_vspr_pos+6,x
	;;  need to handle hi_x and the other attributes
	rts
.endproc
