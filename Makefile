all: c64
# c128

c64: artwork
	mkdir -p build-c64n
	cd build-c64n; TV=NTSC MODE=MC make -f ../Makefile.c64
	mkdir -p build-c64
	cd build-c64; TV=PAL MODE=MC make -f ../Makefile.c64
	mkdir -p build-c64n-hr
	cd build-c64n-hr; TV=NTSC MODE=HIRES make -f ../Makefile.c64
	mkdir -p build-c64-hr
	cd build-c64-hr; TV=PAL MODE=HIRES make -f ../Makefile.c64
	grep ^ZDATA build-c64/totoro64.map
	grep ^ZDATA build-c64n/totoro64.map
	grep ^ZDATA build-c64-hr/totoro64.map
	grep ^ZDATA build-c64n-hr/totoro64.map

#c128:
#	mkdir -p build-c128
#	cd build-c128; make -f ../Makefile.c128

clean:
	rm -rf build-c64 build-c64n build-c64-hr build-c64n-hr build-c128
	rm -rf src/charset.* src/charset-hr.* src/sprites.* src/background*
	rm -rf src/bitmap.c src/color1.c src/color2.c src/*~
	rm -rf release
	cd utils; make clean

bin: all
	tar cvfz totoro64_bin.tar.gz \
		build-c64/*.bin  build-c64/*.crt build-c64n/*.bin  build-c64n/*.crt

artwork: charset sprites background

charset: tools artwork/totoro*-charset.bin
	utils/make_font_table artwork/totoro_mc-charset.bin >src/charset.bin
	cd src; zopfli --i100 --deflate charset.bin
	utils/bin_to_c src/charset.bin.deflate charset_data ZDATA >src/charset.c
	cp artwork/totoro_hires-charset.bin src/charset-hr.bin
	cd src; zopfli --i100 --deflate charset-hr.bin
	utils/bin_to_c src/charset-hr.bin.deflate charset_data ZDATA >src/charset-hr.c

sprites: prg_studio_project/sprites.bin tools
	cat prg_studio_project/game_sprites.bin prg_studio_project/title_sprites.bin >src/sprites.bin
	cd src; zopfli --i100 --deflate sprites.bin
	utils/bin_to_c src/sprites.bin.deflate sprite_src_data ZDATA >src/sprites.c

background: artwork/background.s tools
	utils/asm_to_bin artwork/background.s 4 8000 >src/background-bitmap.bin
	utils/asm_to_bin artwork/background.s 8004 1000 >src/background-color1.bin
	utils/asm_to_bin artwork/background.s 9004 1000 >src/background-color2.bin

	cd src; zopfli --i100 --deflate background-bitmap.bin
	cd src; zopfli --i100 --deflate background-color1.bin
	cd src; zopfli --i100 --deflate background-color2.bin

	utils/bin_to_c src/background-bitmap.bin.deflate bitmap_data ZDATA  >src/bitmap.c
	utils/bin_to_c src/background-color1.bin.deflate color1_data ZDATA  >>src/bitmap.c
	utils/bin_to_c src/background-color2.bin.deflate color2_data ZDATA  >>src/bitmap.c

test: all
	x64 -cart16 build-c64/totoro64.bin
#	x128 -extfrom build-c128/cterm128.bin -extfunc 1

release: all
	mkdir -p release
	cp build-c64/totoro64.crt release/totoro64-pal.crt
	cp build-c64n/totoro64.crt release/totoro64-ntsc.crt
	cp build-c64-hr/totoro64.crt release/totoro64-hires-pal.crt
	cp build-c64n-hr/totoro64.crt release/totoro64-hires-ntsc.crt
	zip totoro64.zip release/*.crt

tools: utils/*.c
	cd utils; make
