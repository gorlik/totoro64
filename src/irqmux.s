.include "c64.inc"

.define IPDebug     0
.include "macros.inc"

.import   _IRQ
.import   _SPR_PTR
.import   color_save

.import   _spr_mux
.import   _acorn
ASIZE = 8

.export acorn_set_pos

;;  sprite mux setup
.define LINES_EARLY  3
.define MUX_SLOTS    2


.segment 	"BSS"
	;; temp variables for sprite muxing
mask:
	.res 1
xpos_hi:
	.res 1
en_tmp:
	.res 1

.macro setup_AXY idx
	lda #(1<<((8-MUX_SLOTS)+(idx .mod MUX_SLOTS))) ; spr mask
	ldx #(idx * ASIZE)	                       ; index in the acorn array
	ldy #((8-MUX_SLOTS)+(idx .mod MUX_SLOTS))*2    ; index in the SPR_POS register 
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

.proc acorn_set_pos: near
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
	tya
	lsr
	tay
	lda _acorn+7,x
	sta _SPR_PTR,y
	;       lda _acorn+xxx,x   ; get ready to support muxing sprite colors as well
	;	sta VIC_COL0,y
	lda _acorn+2,x 	; acorn[idx].posx.hi
	beq skip
	lda xpos_hi
	ora mask   	; set SPR_HI_X bit
	sta xpos_hi
skip:
	rts
.endproc

.proc irq_body: near
	sta mask
	lda VIC_IRR		; irq ack
	sta VIC_IRR
	lda mask 		; setup en_tmp
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

.proc set_next_raster: near
	sec
	sbc #LINES_EARLY
	sta VIC_HLINE
	lda irq_table,x
	sta IRQVec
	lda irq_table+1,x
	sta IRQVec+1
	border_restore
	jmp $EA81
.endproc

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
