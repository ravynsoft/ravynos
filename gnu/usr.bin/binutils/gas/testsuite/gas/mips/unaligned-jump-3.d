#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS jump to unaligned symbol 3
#as: -n32 -march=from-abi
#source: unaligned-jump-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar0
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar0
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar0
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar4\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar16
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar16
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar16
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar17
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar17
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar17
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x1
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x2
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x3
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 74000000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 0c000000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 08000000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MIPS_26	bar18\+0x4
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000027 	nor	zero,zero,zero
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
