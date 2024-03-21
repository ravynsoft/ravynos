#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch to unaligned symbol 3
#as: -n32 -march=from-abi
#source: unaligned-branch-mips16-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001006 <foo\+0x6>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar0-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000100c <foo\+0xc>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar1-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001012 <foo\+0x12>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar2-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001018 <foo\+0x18>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar3-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000101e <foo\+0x1e>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001024 <foo\+0x24>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x3
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000102a <foo\+0x2a>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x2
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001030 <foo\+0x30>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x1
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001036 <foo\+0x36>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000103c <foo\+0x3c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar16-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001042 <foo\+0x42>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar17-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001048 <foo\+0x48>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000104e <foo\+0x4e>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x3
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001054 <foo\+0x54>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x2
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	0000105a <foo\+0x5a>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x1
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 1000 	b	00001060 <foo\+0x60>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001066 <foo\+0x66>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar0-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,0000106c <foo\+0x6c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar1-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001072 <foo\+0x72>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar2-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001078 <foo\+0x78>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar3-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,0000107e <foo\+0x7e>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001084 <foo\+0x84>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x3
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,0000108a <foo\+0x8a>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x2
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001090 <foo\+0x90>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4-0x1
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,00001096 <foo\+0x96>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,0000109c <foo\+0x9c>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar16-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010a2 <foo\+0xa2>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar17-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010a8 <foo\+0xa8>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x4
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010ae <foo\+0xae>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x3
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010b4 <foo\+0xb4>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x2
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010ba <foo\+0xba>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18-0x1
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> f000 2a00 	bnez	v0,000010c0 <foo\+0xc0>
[ 	]*[0-9a-f]+: R_MIPS16_PC16_S1	bar18
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> ea6f      	not	v0,v1
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
