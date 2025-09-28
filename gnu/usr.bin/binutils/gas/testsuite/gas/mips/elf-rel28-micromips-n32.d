#source: elf-rel28.s
#as: -march=from-abi -mmicromips -n32
#objdump: -dr
#name: MIPS ELF reloc 28 (microMIPS, n32)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_HI16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_LO16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_CALL16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_DISP	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_PAGE	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_OFST	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_HI16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_LO16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GOT16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_GPREL16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHEST	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHER	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_SUB	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GD	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_LDM	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
.*:	dc84 0000 	ld	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_HI16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL_LO16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_CALL16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_DISP	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_PAGE	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_OFST	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_HI16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT_LO16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GOT16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_GPREL16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHEST	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_HIGHER	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_SUB	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GD	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_LDM	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
.*:	d884 0000 	sd	a0,0\(a0\)
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
	\.\.\.
