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

.export   play_track
.export   _time1, _instr1, _loop1, _next_loop1, _vpb

.importzp _t1ptr
.importzp _t2ptr
.import   _track1
.import   _FTableLo
.import   _FTableHi

.import   acorn_set_pos
	
.import   _spr_mux

.define SIDRestart  0


.segment 	"BSS"
tmp:
	.res 1
_next_loop1:
	.res 1
_time1:
	.res 1
_instr1:
	.res 1
_loop1:
	.res 1
_vpb:
	.res 1
ctmp:
	.res 1
_l_offset:
	.res 1

.segment	"CODE"

;; ****************** IRQ Interrupt *********************
.proc play_track: near
        border_set 1
	lda _time1
	bne time_dec		; timer not expired
next:
	ldy #0
	lda (_t1ptr),y
	sta tmp
	jsr inc_t1ptr 		; _t1ptr point to the next byte
	and #$f0
	bne parse_cmd
	;; new note
.if SIDRestart = 1
	lda #$80
	sta SID_AD1
	lda #$F6
	sta SID_SUR1
	lda #$00
.endif
	clc
	ldx _vpb
add_more:
	adc tmp
	dex
	bne add_more
	sta _time1

	lda (_t1ptr),y
	bne not_pause
	lda _instr1
	sta SID_Ctl1
	jmp inc_ptr

not_pause:
	adc _loop1
	adc _l_offset
	tax
	lda _FTableHi,x
	sta SID_S1Hi
	lda _FTableLo,x
	sta SID_S1Lo
	lda _instr1
	ora #1
	sta SID_Ctl1
inc_ptr:
	jsr inc_t1ptr
time_dec:
	dec _time1
	lda _time1
	cmp #2
	bne end
.if SIDRestart = 1
	lda #0
	sta SID_AD1
	sta SID_SUR1
.endif
	lda _instr1
	sta SID_Ctl1
end:
	border_restore
	rts

	;; cmd is in tmp, Y=0
parse_cmd:
	lda tmp
	cmp #$ff
	beq cmd_ff
	cmp #$fe
	beq cmd_fe
	cmp #$fd
	beq cmd_fd
	jmp next

cmd_ff:	 			; end of stream
	lda #<_track1
	sta _t1ptr
	lda #>_track1
	sta _t1ptr+1		; point to the beginning of the stream
	lda _next_loop1
	sta _loop1		; set the new offset
	jmp next

cmd_fe:	 			; change instrument
	lda (_t1ptr),y
	sta _instr1
	jsr inc_t1ptr
	jmp next
cmd_fd:	 			; change offset
	lda (_t1ptr),y
	sta _l_offset
	jsr inc_t1ptr
	jmp next

inc_t1ptr:
	inc _t1ptr
	bne t1inc_end
	inc _t1ptr+1
t1inc_end:
	rts
.endproc
