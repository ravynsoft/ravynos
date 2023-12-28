#objdump: -dr
#name: MIPS16e2 lui/addi
#as: -mips16 -mabi=32 -march=mips32r2 -mmips16e2
#source: mips16e2-hilo.s

.*: +file format .*mips.*

Disassembly of section \.text:

0+0000 <stuff>:
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+:	4c00      	addiu	a0,0
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c04 	addiu	a0,4
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+:	4c01      	addiu	a0,1
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c05 	addiu	a0,5
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 4c01 	addiu	a0,1
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 4c04 	addiu	a0,-32764
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 4c04 	addiu	a0,-32764
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c20 	lui	a0,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f010 4c00 	addiu	a0,-32768
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+:	4c00      	addiu	a0,0
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 4c04 	addiu	a0,4
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c21 	lui	a0,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 4c00 	addiu	a0,0
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f5b4 4c09 	addiu	a0,-23127
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6c22 	lui	a0,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f5b4 4c05 	addiu	a0,-23131
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+:	9d80      	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d85 	lw	a0,5\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 9d81 	lw	a0,1\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 9d84 	lw	a0,-32764\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f010 9d84 	lw	a0,-32764\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d20 	lui	a1,0x0
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f010 9d80 	lw	a0,-32768\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+:	9d80      	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f000 9d84 	lw	a0,4\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d21 	lui	a1,0x1
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f000 9d80 	lw	a0,0\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.data
[ 	]*[0-9a-f]+:	f5b4 9d89 	lw	a0,-23127\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.data
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_data_label
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_data_label
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_data_label
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_data_label
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	big_external_common
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	big_external_common
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	small_external_common
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	small_external_common
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.bss
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.bss
[ 	]*[0-9a-f]+:	f000 6d22 	lui	a1,0x2
[ 	]*[0-9a-f]+: R_MIPS16_HI16	\.sbss
[ 	]*[0-9a-f]+:	f5b4 9d85 	lw	a0,-23131\(a1\)
[ 	]*[0-9a-f]+: R_MIPS16_LO16	\.sbss
	\.\.\.
