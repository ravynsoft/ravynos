#PROG: objcopy
#objdump: -dr --prefix-addresses --show-raw-insn
#name: Mixed MIPS and microMIPS disassembly
#as: -mips2

# Test mixed-mode disassembly in overlapping sections.

.*: +file format .*mips.*

Disassembly of section \.text\.foo:
[0-9a-f]+ <[^>]*> 27bdffe0 	addiu	sp,sp,-32
[0-9a-f]+ <[^>]*> afbf001c 	sw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <.*>
[ 	]*[0-9a-f]+: R_MIPS_26	baz
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 8fbf001c 	lw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 03e00008 	jr	ra
[0-9a-f]+ <[^>]*> 27bd0020 	addiu	sp,sp,32
	\.\.\.

Disassembly of section \.text\.bar:
[0-9a-f]+ <[^>]*> 4ff1      	addiu	sp,sp,-32
[0-9a-f]+ <[^>]*> cbe7      	sw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	baz
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 4be7      	lw	ra,28\(sp\)
[0-9a-f]+ <[^>]*> 4708      	jraddiusp	32
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
