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
.import    _printbigat

.export    _joy1
.export    _joy2
.export    _joyk
.export    _joy2k	
.export    _joy_any
.export    _delay
.export    _string_pad
.export    _utoa10

.export   _print_p

.segment        "BSS"
kjoytmp:
	.res 1
anyjtmp:
	.res 1
padtmp:
	.res	1

.segment	"CODE"


; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joy_any (void)
; 
; returns any joystick or keyboard movement
.proc _joy_any: near
	jsr _joy1
	sta anyjtmp
	jsr _joy2k
	ora anyjtmp
	rts
.endproc

; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joy2k(void)
; 
; returns port 1 joystick
.proc _joy2k: near
	jsr _joy2
	bne end
	jsr _joyk
end:	rts
.endproc
	
; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joyk (void)
; 
; returns keyboard presses as a joystick
.proc _joyk: near
				;	sei
	ldx #$80
	stx CIA1_DDRB
	lda CIA1_PRA
				;	cli
	ldx #$0
	stx CIA1_DDRB
	
	eor #$ff
	tay
	and #$02
	beq not_fire
	ldx #$10 		; x contains the temp joy value
not_fire:
	stx kjoytmp
	tya
	and #$18
	lsr
	ora kjoytmp
	rts
.endproc
				;	tay
				;	txa
				;	cpy #$10
				;	bne skip1
				;	ora #$08
				;skip1:	cpy #$08
				;	bne end
				;	ora #$04
				;end:	rts
				;.endproc

; ---------------------------------------------------------------
; uint8_t __near__ __fastcall__ joy1 (void)
; 
; returns port 1 joystick
.proc _joy1: near
 	lda     CIA1_PRB
        and     #$1F
        eor     #$1F
        rts
.endproc

;  --------------------------------------------------------------- 
; uint8_t __near__ __fastcall__ joy2 (void)
; 
; returns port 2 joystick
.proc _joy2: near
	lda CIA1_PRA 		;	sty CIA1_DDRA
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
; void __near__ __fastcall__ print_p (unsigned char)
; ---------------------------------------------------------------
.proc	_print_p: near
	ldx     #231 		; '1'
	cmp     #21		; a>20 ?
	bcc     skip
	inx			; '1'+1 = '2'
skip:	stx     _STR_BUF
	ldx     #'P'
	stx     _STR_BUF+1
	ldx     #0
	stx     _STR_BUF+2
	jsr     _printbigat	; position is still in a
	rts
.endproc

; ---------------------------------------------------------------
; void __near__ __fastcall__ string_pad (unsigned char n)
; ---------------------------------------------------------------
; pad string with leading spaces to length n
.proc	_string_pad: near

	sta padtmp
	tay
	ldx #$FF
lloop:	inx
	lda _STR_BUF,x
	bne lloop

	cpx padtmp
	beq end
	;;  	ldy pad
cloop1:
	lda _STR_BUF,x
	sta _STR_BUF,y
	dey
	dex
	bpl cloop1
	lda #192               ; ' '
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
