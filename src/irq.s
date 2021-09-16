.include "c64.inc"

.define IPDebug     0
.include "macros.inc"


.export   _IRQ
.export   color_save

.import   _spr_mux
.import   acorn_set_pos
.import   play_track

.segment 	"BSS"
color_save:
	.res 1

.segment	"CODE"

;; ****************** IRQ Interrupt *********************
.proc _IRQ: near
	lda VIC_IRR
	bpl not_vic	; check if IRQ from VIC
	sta VIC_IRR	; clear VIC IRQ flag
        lda _spr_mux
        beq no_mux
        jsr acorn_set_pos
no_mux:
	jsr play_track
not_vic:
	jmp $EA31 		; alway chain the standard kernel IRQ
.endproc

