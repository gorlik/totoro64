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

.import   _vpb
.import   _track

.import   _FTableLo
.import   _FTableHi

.import   _itable

.import color_save

.define SIDRestart  0

.define TRACK_T_SIZE 10

.segment "ZP_2" : zeropage
track_ptr:
	.res 2

.segment 	"BSS"
tmp:
	.res 1
trk_offset:
	.res 1

;;  offsets for the track_t structure
.define TRACK_PTR0 0
.define TRACK_PTR1 1
.define TIMER      2
.define INSTR      3
.define TRACK_OFF  4
.define GLB_OFF    5
.define SID_OFF    6
.define NEXT_OFF   7
.define START_PTR0 8
.define START_PTR1 9


;; temporary storage that must be saved
timer:
	.res 1
instr:
	.res 1
track_offset:
	.res 1
	;; additional temporary storage (not saved)
global_offset:
	.res 1			;
SID_offset:
	.res 1


.macro load_track_state
	stx trk_offset
	lda _track+TRACK_PTR0,x
	sta track_ptr
	lda _track+TRACK_PTR1,x
	sta track_ptr+1

	lda _track+TIMER,x
	sta timer
	lda _track+INSTR,x
	sta instr
	lda _track+TRACK_OFF,x
	sta track_offset
	lda _track+GLB_OFF,x
	sta global_offset
	lda _track+SID_OFF,x
	sta SID_offset
	tax
.endmacro

.macro save_track_state
				;.local loop
	ldx trk_offset
	lda track_ptr
	sta _track+TRACK_PTR0,x
	lda track_ptr+1
	sta _track+TRACK_PTR1,x

	lda timer
	sta _track+TIMER,x

	lda instr
	sta _track+INSTR,x
.endmacro

.segment	"CODE"

;; ****************** Music Player Interrupt *********************
.proc play_track: near
        border_set 1
	load_track_state
	lda timer  		;
	bne timer_dec		; timer not expired
next:
	ldy #0
	lda (track_ptr),y
	sta tmp
	jsr inc_track_ptr 	; _track_ptr point to the next byte
	and #$f0
	bne parse_cmd
	;; new note
.if SIDRestart = 1
	lda #$80
	sta SID_AD1,x
	lda #$F6
	sta SID_SUR1,x
	lda #$00
.endif
	clc
	ldx _vpb
add_more:
	adc tmp			; tmp contains the note lenght in beats
	dex
	bne add_more
	sta timer    	; store timer

	ldx SID_offset
	ldy #0
	lda (track_ptr),y	; get the note
	bne not_pause
	lda instr      ; get the instrument
	sta SID_Ctl1,x
	jmp inc_ptr

not_pause:			;note in A
	clc
	adc track_offset
	adc global_offset
	tay
	lda _FTableHi,y
	sta SID_S1Hi,x
	lda _FTableLo,y
	sta SID_S1Lo,x
	lda instr
	ora #1
	sta SID_Ctl1,x
inc_ptr:
	jsr inc_track_ptr
timer_dec:
	dec timer
	lda timer
	cmp #2
	bne end
.if SIDRestart = 1
	lda #0
	sta SID_AD1,x
	sta SID_SUR1,x
.endif
	lda instr	; stop sound
	sta SID_Ctl1,x
end:
	save_track_state
	border_restore
	rts

	;; cmd is in tmp, Y=0
parse_cmd:
	ldx trk_offset
	lda tmp
	cmp #$ff
	beq cmd_ff
	cmp #$fe
	beq cmd_fe
	cmp #$fd
	beq cmd_fd
	and #$f0
	cmp #$e0
	beq cmd_ex
	jmp next

cmd_ff:	 				; end of stream
	lda _track+START_PTR0,x		; restart_ptr
	sta track_ptr
	lda _track+START_PTR1,x
	sta track_ptr+1			; point to the beginning of the stream
	lda _track+NEXT_OFF,x		; next offset
	sta _track+GLB_OFF,x	        ; set the new global offset
	sta global_offset
	jmp next

cmd_fe:				; change instrument from table	
	lda (track_ptr),y	; instrument index
	tay                     ; intrument indexd in Y
	lda _itable,y
	sta instr
	ldx SID_offset
	lda _itable+1,y
	sta SID_AD1,x
	lda _itable+2,y
	sta SID_SUR1,x
	jmp inc_track_next

cmd_fd:	 			; change track offset
	lda (track_ptr),y
	sta _track+TRACK_OFF,x
	sta track_offset
	jmp inc_track_next

	
cmd_ex:	 			; change sid register
	lda tmp
	and #$0f
	sta tmp
	lda _track+SID_OFF,x
	clc
	adc tmp
	tax
	lda (track_ptr),y
	sta SID,x
	jmp inc_track_next


	
inc_track_next:
	jsr inc_track_ptr
;	inc track_ptr
; 	bne inc_end
;	inc track_ptr+1
	jmp next
.endproc
	
.proc inc_track_ptr: near
	inc track_ptr
	bne inc_end
	inc track_ptr+1
inc_end:
	rts
.endproc
