#source: elf-rel28.s
#as: -march=from-abi -n32 --defsym tlldscd=1
#objdump: -dr
#name: MIPS ELF reloc 28 (LLD/SCD, n32)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
.*:	d0840000 	lld	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL_HI16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL_LO16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_CALL16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_DISP	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_PAGE	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_OFST	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_HI16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT_LO16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GOT16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_GPREL16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_HIGHEST	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_HIGHER	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_SUB	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_GD	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_LDM	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_HI16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_DTPREL_LO16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_HI16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_TPREL_LO16	bar
.*:	f0840000 	scd	a0,0\(a0\)
			.*: R_MIPS_TLS_GOTTPREL	bar
	\.\.\.
