.include "c64.inc"
.export   _IRQ
.export   _time1, _instr1, _loop1, _next_loop1, _vpb

.importzp _t1ptr
.importzp _t2addr
.import   _track1
.import   _FTableLo
.import   _FTableHi

.define SIDRestart  0
.define IPDebug     0

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
.proc _IRQ: near
	lda $d019
	bpl not_vic	; check if IRQ from VIC
	sta $d019	; clear VIC IRQ flag
.if IPDebug =1
	lda $d020
	sta ctmp
	lda _instr1
	lsr a
	lsr a
	lsr a
	lsr a
	sta $d020
.endif
	lda _time1
	bne time_inc_end		; timer not expired
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
time_inc_end:
	dec _time1
	lda _time1
	cmp #2
	bne end
.IF SIDRestart = 1
	lda #0
	sta SID_AD1
	sta SID_SUR1
.ENDIF
	lda _instr1
	sta SID_Ctl1
end:
.if IPDebug =1
	lda ctmp
	sta $d020
.endif
        jmp $EA81
not_vic:
	jmp $EA31 		; chain the standard kernel IRQ


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
