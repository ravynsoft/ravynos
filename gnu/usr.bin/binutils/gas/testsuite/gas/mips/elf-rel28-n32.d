#source: elf-rel28.s
#as: -march=from-abi -n32
#objdump: -dr
#name: MIPS ELF reloc 28 (n32)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
.*:	dc840000 	ld	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
.*:	fc840000 	sd	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
	\.\.\.
