#name: MIPS multi-got-no-shared
#as: -KPIC -mno-shared
#source: multi-got-no-shared-1.s
#source: multi-got-no-shared-2.s
#ld: --entry func1
#objdump: -D -j .text --prefix-addresses --show-raw-insn

.*: +file format.*

Disassembly of section \.text:
004000f0 <[^>]*> 3c1c0043 	lui	gp,0x43
004000f4 <[^>]*> 279c9ff0 	addiu	gp,gp,-24592
004000f8 <[^>]*> afbc0008 	sw	gp,8\(sp\)
#...
00408da0 <[^>]*> 3c1c0043 	lui	gp,0x43
00408da4 <[^>]*> 279c2c98 	addiu	gp,gp,11416
00408da8 <[^>]*> afbc0008 	sw	gp,8\(sp\)
#pass
