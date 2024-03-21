#source: elf-rel28.s
#as: -march=from-abi -mmicromips -n32 --defsym tlldscd=1
#objdump: -dr
#name: MIPS ELF reloc 28 (LLD/SCD, microMIPS, n32)

.*:     file format .*


Disassembly of section \.text:

.* <foo>:
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL_HI16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL_LO16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_DISP	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_PAGE	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_OFST	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_HI16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_LO16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GPREL16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MIPS_16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_HIGHEST	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_HIGHER	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_SUB	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_GD	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_LDM	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
.*:	6081 7000 	lld	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL_HI16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL_LO16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_CALL16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_DISP	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_PAGE	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_OFST	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_HI16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT_LO16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GOT16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_GPREL16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MIPS_16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_HIGHEST	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_HIGHER	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_SUB	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_GD	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_LDM	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_DTPREL_HI16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_DTPREL_LO16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_TPREL_HI16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_TPREL_LO16	bar
.*:	6081 f000 	scd	a0,0\(at\)
.*:	3024 0000 	addiu	at,a0,0
			.*: R_MICROMIPS_TLS_GOTTPREL	bar
.*:	6081 f000 	scd	a0,0\(at\)
	\.\.\.
