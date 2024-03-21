#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS jump to unaligned symbol 3
#as: -n32 -march=from-abi
#source: unaligned-jump-micromips-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar0
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar0
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar0
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar0
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar4\+0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar16
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar16
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar16
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar16
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar17
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar17
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar17
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar17
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 0000 	jalx	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> f400 0000 	jal	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 7400 0000 	jals	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> d400 0000 	j	00000000 <foo-0x1000>
[ 	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar18\+0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 001f 0f3c 	jr	ra
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
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
