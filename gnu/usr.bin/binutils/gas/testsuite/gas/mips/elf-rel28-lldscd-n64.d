#source: elf-rel28.s
#as: -march=from-abi -64 --defsym tlldscd=1
#objdump: -dr
#name: MIPS ELF reloc 28 (LLD/SCD, n64)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
	\.\.\.
