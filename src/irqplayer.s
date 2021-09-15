.include "c64.inc"
.export   _IRQ
.export   _time1, _instr1, _loop1, _next_loop1, _vpb

.importzp _t1ptr
.importzp _t2ptr
.import   _track1
.import   _FTableLo
.import   _FTableHi

.import   _spr_mux
.import   _acorn
ASIZE = 7

;;  sprite mux setup
.define LINES_EARLY  3
.define MUX_SLOTS    2


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

;; temp variables for sprite muxing
mask:
	.res 1
xpos_hi:
	.res 1
en_tmp:
	.res 1

.if IPDebug =1
.macro border_set col
	lda $d020
	sta ctmp
	lda #col
	sta $d020
.endmacro

.macro border_set_sprite col 	;
	lda $d020 ;
	sta ctmp
	lda #(col+5-MUX_SLOTS)
	sta $d020
	sta VIC_SPR0_COLOR+(8-MUX_SLOTS)+(col .mod MUX_SLOTS)
.endmacro

.macro border_restore
	lda ctmp
	sta $d020
.endmacro
.else

.macro border_set col
.endmacro

.macro border_set_sprite col
.endmacro

.macro border_restore
.endmacro

.endif

.macro setup_AXY idx
	lda #(1<<((8-MUX_SLOTS)+(idx .mod MUX_SLOTS)))
	ldx #(idx * ASIZE)	; index in the acorn array
	ldy #((8-MUX_SLOTS)+(idx .mod MUX_SLOTS))*2 ; index in the SPR_POS register 
.endmacro

.segment	"CODE"

irq_table:
	.word _IRQ
	.word IRQ2
	.word IRQ3
	.word IRQ4
	.word IRQ5
	.word IRQ6
	.word IRQ7


;; ****************** IRQ Interrupt *********************
.proc _IRQ: near
	lda $d019
	bpl not_vic	; check if IRQ from VIC
	sta $d019	; clear VIC IRQ flag
        lda _spr_mux
        beq no_mux
        jsr acorn_set_pos
no_mux:
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
not_vic:
	jmp $EA31 		; alway chain the standard kernel IRQ


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

acorn_set_pos:
        border_set 4
	lda VIC_SPR_ENA
	and #($00FF>>MUX_SLOTS)
	sta en_tmp
	lda VIC_SPR_HI_X
	and #($00FF>>MUX_SLOTS)
	sta xpos_hi


.repeat MUX_SLOTS, i
	setup_AXY i
	sta mask
        jsr spr_pos
.endrepeat

.if IPDebug =1
	lda #4
.repeat MUX_SLOTS, i
	sta VIC_SPR7_COLOR-i
.endrepeat
.endif

	lda en_tmp
	sta VIC_SPR_ENA
	lda xpos_hi
	sta VIC_SPR_HI_X

	ldx #(MUX_SLOTS-1)*2
	lda irq_table,x
	sta IRQVec
	lda irq_table+1,x
	sta IRQVec+1

	sec
	lda _acorn+(ASIZE*MUX_SLOTS)+4
	sbc #LINES_EARLY	; spr height
	sta VIC_HLINE
	border_restore
	rts
.endproc

.proc   spr_pos: near
	lda _acorn,x	; acorn[idx].enable
	beq skip
	lda en_tmp
	ora mask	; set  SPR_EN bit
	sta en_tmp
	lda _acorn+4,x 	; acorn[idx].posy.hi
	sta VIC_SPR0_Y,y
	lda _acorn+1,x 	; acorn[idx].posx.lo
	sta VIC_SPR0_X,y
	lda _acorn+2,x 	; acorn[idx].posx.hi
	beq skip
	lda xpos_hi
	ora mask   	; set SPR_HI_X bit
	sta xpos_hi
skip:
	rts
.endproc

.proc irq_body: near
	sta mask 		; setup en_tmp
	lda $d019
	sta $d019
	lda mask
	eor #$ff
	and VIC_SPR_ENA
	sta en_tmp
	lda mask		; setup xpos_hi
	eor #$ff
	and VIC_SPR_HI_X
	sta xpos_hi

	jsr spr_pos		; expects en_tmp, mask, xpos_hi and index in X and Y

	lda en_tmp		; write back en_tmp to VIC register
	sta VIC_SPR_ENA
	lda xpos_hi		; write back xpos_hi to VIC register
	sta VIC_SPR_HI_X
	lda _acorn+(4+ASIZE),x 	; next acorn.ypos.hi
	bne not_last		; check if last slot
	pla			; pull and discard return address from stack
	pla
	jmp restore_irq
not_last:
	rts
.endproc

set_next_raster:
	sec
	sbc #LINES_EARLY
	sta VIC_HLINE
	lda irq_table,x
	sta IRQVec
	lda irq_table+1,x
	sta IRQVec+1
	border_restore
	jmp $EA81

.macro IRQ_CODE idx
	border_set_sprite (idx)
	setup_AXY idx
	jsr irq_body		; expects mask in A, indexes in X and Y
	ldx #(idx*2)
	jmp set_next_raster
.endmacro

IRQ2: 	IRQ_CODE 2

IRQ3: 	IRQ_CODE 3

IRQ4: 	IRQ_CODE 4

IRQ5: 	IRQ_CODE 5

IRQ6: 	IRQ_CODE 6

IRQ7:
	border_set_sprite (7)
	setup_AXY 7
	jsr irq_body
restore_irq:
	ldx #0			; vector back to _IRQ
	lda #56			; rasterline 56-LINES_EARLY
	jmp set_next_raster
