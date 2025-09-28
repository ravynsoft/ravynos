#source: elf-rel28.s
#as: -march=from-abi -64
#objdump: -dr
#name: MIPS ELF reloc 28 (n64)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
	\.\.\.
