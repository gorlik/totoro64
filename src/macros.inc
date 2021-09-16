
.if IPDebug = 1
.macro border_set col
	lda $d020
	sta ctmp
	lda #col
	sta $d020
.endmacro

.macro border_set_sprite col	;
	lda $d020		;
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
