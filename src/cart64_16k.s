;
; Startup code for cc65 (C64 version)
;

.export         __STARTUP__ : absolute = 1      ; Mark as startup
.import         _main
.import         BSOUT
.import         __RAM_START__, __RAM_SIZE__	; Linker generated

.import		__DATA_LOAD__, __DATA_RUN__, __DATA_SIZE__

.include        "zeropage.inc"
.include        "c64.inc"

.segment	"HEADERDATA"

HeaderB:
@magic:
	;.byt	"C64  CARTRIDGE  "
	.byt	$43,$36,$34,$20, $43,$41,$52,$54
	.byt	$52,$49,$44,$47, $45,$20,$20,$20
@headelen:
	.byt	$00,$00,$00,$40
@ver:
	.byt	$01,$00
@carttye:
	.byt	$00,$00
@EXROM:
	.byt	$00
@GAME:
	.byt	$00
@reserved1:
	.byt	$00,$00,$00,$00,$00,$00
@Name:	;You put the name of the cartridge here.
	.byt	"GMT"

.segment	"CHIP0"
ChipB:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$40,$10	;Use for 16k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0000
@start:
	.byt	$80,$00
@size2:
	.byt	$40,$00

; ------------------------------------------------------------------------
; Startup code

.segment        "STARTUP"

	.word	Start           ;Cold start
	.word	$FEBC		;Warm start, default calls NMI exit.
	.byt	$C3,$C2,$CD,$38,$30 ;magic to identify cartridge

Start:

	jsr	$FF84		;Init. I/O
	jsr	$FF87		;Init. RAM
	jsr	$FF8A		;Restore vectors
	jsr	$FF81		;Init. screen

; Set up the stack.
	;	lda    	#<(__HIRAM_START__ + __HIRAM_SIZE__)
	lda    	#<($D000)
	sta	sp
	;	lda	#>(__HIRAM_START__ + __HIRAM_SIZE__)
	lda	#>($D000)
       	sta	sp+1   		; Set argument stack ptr


	ldy #<__DATA_SIZE__
dloop:	
	lda __DATA_LOAD__-1,y
	sta __DATA_RUN__-1,y
	dey
	bne dloop

; Switch to second charset
	lda	#14
	jsr	BSOUT

; Clear the BSS data.
;        jsr     zerobss

; Jump to main
        jmp     _main
