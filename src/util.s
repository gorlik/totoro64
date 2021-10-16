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

.export    _joy1
.export    _joy2
.export    _joyk
.export    _joy_any
.export    _utoa10
.importzp  sreg
.importzp  _temp_ptr
.import    _STR_BUF

.segment        "RODATA"
__dectab:
	.byte '0', '1', '2', '3', '4'
	.byte '5', '6', '7', '8', '9'

.segment        "BSS"
anytmp:
	.res 1
	
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


; int utoa (unsigned value)
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

        tay                     ; get remainder into y
        lda     __dectab,y      ; get dec character
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
