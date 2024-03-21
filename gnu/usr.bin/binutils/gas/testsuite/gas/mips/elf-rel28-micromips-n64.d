#source: elf-rel28.s
#as: -march=from-abi -mmicromips -64
#objdump: -dr
#name: MIPS ELF reloc 28 (microMIPS, n64)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_DISP	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_PAGE	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_OFST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GPREL16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MIPS_16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHEST	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHER	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_SUB	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GD	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_LDM	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
			.*: R_MIPS_NONE	\*ABS\*
			.*: R_MIPS_NONE	\*ABS\*
	\.\.\.
