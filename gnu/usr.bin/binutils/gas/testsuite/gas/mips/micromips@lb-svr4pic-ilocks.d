#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS lb-svr4pic-ilocks
#source: lb-pic.s
#as: -32 -KPIC

# Test the lb macro with -KPIC (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1c80 0000 	lb	a0,0\(zero\)
[0-9a-f]+ <[^>]*> 1c80 0001 	lb	a0,1\(zero\)
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 1c84 8000 	lb	a0,-32768\(a0\)
[0-9a-f]+ <[^>]*> 1c80 8000 	lb	a0,-32768\(zero\)
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> 41a4 0002 	lui	a0,0x2
[0-9a-f]+ <[^>]*> 1c84 a5a5 	lb	a0,-23131\(a0\)
[0-9a-f]+ <[^>]*> 1c85 0000 	lb	a0,0\(a1\)
[0-9a-f]+ <[^>]*> 1c85 0001 	lb	a0,1\(a1\)
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 8000 	lb	a0,-32768\(a0\)
[0-9a-f]+ <[^>]*> 1c85 8000 	lb	a0,-32768\(a1\)
[0-9a-f]+ <[^>]*> 41a4 0001 	lui	a0,0x1
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> 41a4 0002 	lui	a0,0x2
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 a5a5 	lb	a0,-23131\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.data
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.data
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_data_label
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_data_label
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_common
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_common
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 03e8 	addiu	a0,a0,1000
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.data
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.data
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_data_label
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_data_label
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_common
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_common
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 03e8 	addiu	a0,a0,1000
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.data
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.data
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_data_label
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_data_label
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_common
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_common
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 03e8 	addiu	a0,a0,1000
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0000 	lb	a0,0\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.data
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.data
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_data_label
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_data_label
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	big_external_common
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	small_external_common
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 0000 	addiu	a0,a0,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> fc9c 0000 	lw	a0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.bss
[0-9a-f]+ <[^>]*> 3084 03e8 	addiu	a0,a0,1000
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.bss
[0-9a-f]+ <[^>]*> 00a4 2150 	addu	a0,a0,a1
[0-9a-f]+ <[^>]*> 1c84 0001 	lb	a0,1\(a0\)
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
