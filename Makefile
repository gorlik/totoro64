all: c64
# c128

c64: artwork
	mkdir -p build-c64
	cd build-c64; make -f ../Makefile.c64
	grep ^RODATA build-c64/game.map

#c128:
#	mkdir -p build-c128
#	cd build-c128; make -f ../Makefile.c128

clean:
	rm -rf build-c64 build-c128 src/charset.* src/sprites.* src/background*
	rm -rf src/bitmap.c src/color1.c src/color2.c src/*~
	cd utils; make clean

bin: all
	tar cvfz gterm_bin.tar.gz \
		build-c64/*.bin  build-c64/*.crt

artwork: charset sprites background

charset: tools artwork/totoro-charset.bin
	utils/make_font_table artwork/totoro-charset.bin >src/charset.bin
	utils/bin_to_c src/charset.bin charset TABLES >src/charset.c

sprites: tools prg_studio_project/sprites.bin
	cp prg_studio_project/sprites.bin src
	cd src; zopfli --i100 --deflate sprites.bin
	utils/bin_to_c src/sprites.bin.deflate sprite_src_data  >src/sprites.c

background: tools artwork/background.s
	utils/asm_to_bin artwork/background.s 4 8000 >src/background-bitmap.bin
	utils/asm_to_bin artwork/background.s 8004 1000 >src/background-color1.bin
	utils/asm_to_bin artwork/background.s 9004 1000 >src/background-color2.bin

	cd src; zopfli --i100 --deflate background-bitmap.bin
	cd src; zopfli --i100 --deflate background-color1.bin
	cd src; zopfli --i100 --deflate background-color2.bin

	utils/bin_to_c src/background-bitmap.bin.deflate bitmap_data   >src/bitmap.c
	utils/bin_to_c src/background-color1.bin.deflate color1_data   >src/color1.c
	utils/bin_to_c src/background-color2.bin.deflate color2_data   >src/color2.c

test: all
	x64 -cart16 build-c64/game64.bin

#	x128 -extfrom build-c128/cterm128.bin -extfunc 1

tools: utils/*.c
	cd utils; make
