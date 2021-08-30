.include "c64.inc"
.export _IRQ
.export _time1, _instr1, _loop1, _next_loop1, _vpb

.importzp _t1addr
.importzp _t2addr
.import   _track1


	.segment "BSS"
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


.segment	"CODE"
FTablePalLo:
	        ;      C   C#  D   D#  E   F   F#  G   G#  A   A#  B
                .byte $17,$27,$39,$4b,$5f,$74,$8a,$a1,$ba,$d4,$f0,$0e  ; 1
                .byte $2d,$4e,$71,$96,$be,$e8,$14,$43,$74,$a9,$e1,$1c  ; 2
                .byte $5a,$9c,$e2,$2d,$7c,$cf,$28,$85,$e8,$52,$c1,$37  ; 3
                .byte $b4,$39,$c5,$5a,$f7,$9e,$4f,$0a,$d1,$a3,$82,$6e  ; 4
                .byte $68,$71,$8a,$b3,$ee,$3c,$9e,$15,$a2,$46,$04,$dc  ; 5
                .byte $d0,$e2,$14,$67,$dd,$79,$3c,$29,$44,$8d,$08,$b8  ; 6
                .byte $a1,$c5,$28,$cd,$ba,$f1,$78,$53,$87,$1a,$10,$71  ; 7
                .byte $42,$89,$4f,$9b,$74,$e2,$f0,$a6,$0e,$33,$20,$ff  ; 8

FTablePalHi:
		;      C   C#  D   D#  E   F   F#  G   G#  A   A#  B
                .byte $01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$02  ; 1
                .byte $02,$02,$02,$02,$02,$02,$03,$03,$03,$03,$03,$04  ; 2
                .byte $04,$04,$04,$05,$05,$05,$06,$06,$06,$07,$07,$08  ; 3
                .byte $08,$09,$09,$0a,$0a,$0b,$0c,$0d,$0d,$0e,$0f,$10  ; 4
                .byte $11,$12,$13,$14,$15,$17,$18,$1a,$1b,$1d,$1f,$20  ; 5
                .byte $22,$24,$27,$29,$2b,$2e,$31,$34,$37,$3a,$3e,$41  ; 6
                .byte $45,$49,$4e,$52,$57,$5c,$62,$68,$6e,$75,$7c,$83  ; 7
                .byte $8b,$93,$9c,$a5,$af,$b9,$c4,$d0,$dd,$ea,$f8,$ff  ; 8


;; ****************** IRQ Interrupt *********************
.proc _IRQ: near
	lda $d019
	bpl not_vic
	sta $d019

	lda _time1
	bne inc_end
next:
	ldy #0
	lda (_t1addr),y
	sta tmp
	and #$f0
	bne parse_cmd
	clc
	ldx _vpb
add_more:
	adc tmp
	dex
	bne add_more
	sta _time1

	iny
	clc
	lda (_t1addr),y
	bne not_pause
	lda _instr1
	sta SID_Ctl1
	jmp inc_addr

not_pause:
	adc _loop1
	tax
	lda FTablePalHi,x
	sta SID_S1Hi
	lda FTablePalLo,x
	sta SID_S1Lo
	lda _instr1
	ora #1
	sta SID_Ctl1
inc_addr:
	clc
	lda _t1addr
	adc #2
	sta _t1addr
	bcc inc_end
	; this is shorter but only works when t1addr is 16-bit aligned
	;  	inc _t1addr
	;	inc _t1addr
	; 	bne dec_end
	inc _t1addr+1

inc_end:
	dec _time1
	lda _time1
	cmp #3
	bpl end
	lda _instr1
	sta SID_Ctl1
end:
	jmp $EA81
not_vic:
	jmp $EA31


	;; cmd is in A and tmp, Y=0
parse_cmd:
	cmp #$ff
;	bne cmd_fe
	lda #<_track1
	sta _t1addr		;
	lda #>_track1
	sta _t1addr+1
	lda _next_loop1
	sta _loop1
	jmp next
cmd_fe:
	jmp next
.endproc

