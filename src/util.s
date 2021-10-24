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

.importzp  sreg
.importzp  _temp_ptr
.import    _STR_BUF
.import    _waitvsync

.export    _joy1
.export    _joy2
.export    _joyk
.export    _joy_any
.export    _delay
.export    _string_pad
.export    _utoa10


.segment        "BSS"
anytmp:
	.res 1

; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joy_any (void)
; 
; returns any joystick or keyboard movement

.segment        "CODE"
.proc _joy_any: near
	jsr _joy1
	sta anytmp
	jsr _joy2
	ora anytmp
	sta anytmp
	jsr _joyk
	ora anytmp
	rts
.endproc

; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joyk (void)
; 
; returns keyboard presses as a joystick
.proc _joyk: near
	lda #0
	ldx 203
	cpx #10
	bne skip1
	ora #$04
skip1:	cpx #18
	bne skip2
	ora #$08
skip2:	tay
	lda 653
	and #$01
	tax
	tya
	cpx #1
	bne end
	ora #$10
end:	rts
.endproc

; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joy1 (void)
; 
; returns port 1 joystick
.proc _joy1: near
	lda     #$7F
        sei
        sta     CIA1_PRA
        lda     CIA1_PRB
        cli
        and     #$1F
        eor     #$1F
        rts
.endproc

;  --------------------------------------------------------------- 
; uint8_t __near__ __fastcall__ joy2 (void)
; 
; returns port 2 joystick
.proc _joy2: near
	ldx #0
	lda #$E0
	ldy #$FF
	sei
	sta CIA1_DDRA
	lda CIA1_PRA
	sty CIA1_DDRA
	cli
	and #$1F
	eor #$1F
	rts
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ delay (uint8_t b)
; 
; delays b fields
.proc _delay: near
	tax
loop:	jsr _waitvsync
	dex
	bne loop
	rts
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ string_pad (unsigned char n)
; ---------------------------------------------------------------
; pad string with leading spaces to length n
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
	;; lda #' '
	lda #192
cloop2: sta _STR_BUF,y
	dey
	bpl cloop2
end:	rts

.endproc

; ---------------------------------------------------------------
; int __near__ __fastcall__ utoa10 (unsigned int x)
;
; convert x into a string in _STR_BUF and returns the lenght
; adapted from cc65 library code
_utoa10:
	sta sreg
	stx sreg+1

; Convert to string by dividing and push the result onto the stack

utoa10:   lda     #$00
          pha                     ; sentinel

; Divide sreg/tmp1 -> sreg, remainder in a

L5:     ldy     #16             ; 16 bit
        lda     #0              ; remainder
L6:     asl     sreg
        rol     sreg+1
        rol     a
        cmp     #10
        bcc     L7
        sbc     #10
        inc     sreg
L7:     dey
        bne     L6

	clc
	adc #230		; add offset into char table
        pha                     ; save char value on stack

        lda     sreg
        ora     sreg+1
        bne     L5

; Get the characters from the stack into the string

        ldy     #0
L9:     pla
        sta     _STR_BUF,y
        beq     L10             ; jump if sentinel
        iny
        bne     L9              ; jump always

; Done! Return the target string

L10:    ldx #0
	tya
	rts
