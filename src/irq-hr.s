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

.import   _irq_ctrl
.import   spr_mux_irq
.import   play_track
.import   _vspr_pos
.import   _vspr_pc
.import   _vspr_ctl
.import   _SPR_PTR
.import   restore_irq

.export   _IRQ
.export   color_save
.exportzp itmp1, itmp2

; must be equal to sizeof(struct track_t)
.define   TRACK_T_SIZE 10

;  zero page temporary storage for irq use
.segment "ZP_2" : zeropage
itmp1:
	.res 1
itmp2:
	.res 1

.segment 	"BSS"
color_save:
	.res 1
idx_save:
	.res 1

.segment	"CODE"

;; ****************** IRQ Interrupt *********************
.proc _IRQ: near
	lda VIC_IRR
	bpl end		; check if IRQ from VIC
	sta VIC_IRR	; clear VIC IRQ flag

	lda _irq_ctrl
	beq not_split
        border_set 1
	; setup text mode no mc
	lda #$1b		; char mode, no ec, no blank
	sta $d011		; ctrl1
	lda #$c8		; no MC
	sta $d016		; ctrl2
	lda #$06		; screen at base +0, charset at base +$1800
	sta $d018 		; VIC.addr

	ldy #6                  ; restore top bar sprites
	jsr restore_all_vspr

	lda #65                 ; set next irq to IRQ_SS
	sta VIC_HLINE
	lda #<IRQ_SS
	sta IRQVec
	lda #>IRQ_SS
	sta IRQVec+1
        border_restore
not_split:
	border_set 2        
	ldx #0*TRACK_T_SIZE     ; play music
	jsr play_track		; track[0]
	ldx #1*TRACK_T_SIZE
	jsr play_track		; track[1]
	border_restore
end:
	jmp $EA81		; KERNAL IRQ end
.endproc

.proc IRQ_SS: near
	lda VIC_IRR
	bpl end		; check if IRQ from VIC
	sta VIC_IRR	; clear VIC IRQ flag

	border_set 4
	ldx #66 	; wait for next scanline to reduce jitter
lp:	cpx $d012
	bne lp
	ldx #8 		; wait just enough to let VIC do all the
bw:	dex             ; fetches for the current line
	bne bw 
	lda #$08 	; restore mc bitmap
	ldx #$3b
	ldy #$d8
	; do the writes back to back
	sta $d018		; set bitmap location
	stx $d011		; set bitmap mode
	sty $d016		; set MC mode
	border_restore

	lda _irq_ctrl		; _irq_ctrl>0 acorn mux off
	bpl no_acorn_mux        ; _irq_ctrl<0 acorn mux on
	jsr restore_vspr5	; restore only sprite 5
	jsr spr_mux_irq
end:
	jmp $EA81		; KERNAL IRQ end

no_acorn_mux:			; no acorn mux
        ldy #0                  ; restore all field sprites
        jsr restore_all_vspr
        jmp restore_irq		; set vector back to _IRQ
.endproc

.proc restore_all_vspr: near
	sty idx_save
	ldx #0
pos_loop:
	lda _vspr_pos,y
	sta VIC_SPR5_X,x
	iny
	inx
	cpx #6
	bne pos_loop 		;

	ldy idx_save
	ldx #0
pcloop:
	lda _vspr_pc,y
	sta _SPR_PTR+5,x
	lda _vspr_pc+3,y
	sta VIC_SPR5_COLOR,x
	iny
	inx
	cpx #3
	bne pcloop

	ldy idx_save
        jmp restore_vspr_ctrl
.endproc

.proc restore_vspr5: near
	lda _vspr_pos
	sta VIC_SPR5_X
	lda _vspr_pos+1
	sta VIC_SPR5_Y

	lda _vspr_pc
	sta _SPR_PTR+5
	lda _vspr_pc+3
	sta VIC_SPR5_COLOR

	ldy #0
        jmp restore_vspr_ctrl
.endproc

.proc restore_vspr_ctrl: near
	lda _vspr_ctl+0,y
	sta VIC_SPR_HI_X
	lda _vspr_ctl+1,y
	sta VIC_SPR_ENA
	lda _vspr_ctl+2,y
	sta VIC_SPR_EXP_Y
	lda _vspr_ctl+3,y
	sta VIC_SPR_MCOLOR
	lda _vspr_ctl+4,y
	sta VIC_SPR_EXP_X
	rts
.endproc
