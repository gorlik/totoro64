all:    make_font_table bin_to_c asm_to_bin

make_font_table: make_font_table.c
	gcc -o $@ $<

bin_to_c: bin_to_c.c
	gcc -o $@ $<

clean:
	rm -rf make_font_table bin_to_c asm_to_bin
