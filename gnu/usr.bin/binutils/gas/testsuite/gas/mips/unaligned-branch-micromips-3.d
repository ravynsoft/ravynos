#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS branch to unaligned symbol 3
#as: -n32 -march=from-abi
#source: unaligned-branch-micromips-2.s

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001006 <foo\+0x6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar0-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000100e <foo\+0xe>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar1-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001016 <foo\+0x16>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar2-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000101e <foo\+0x1e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar3-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001026 <foo\+0x26>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000102e <foo\+0x2e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001036 <foo\+0x36>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000103e <foo\+0x3e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001046 <foo\+0x46>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000104e <foo\+0x4e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar16-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001056 <foo\+0x56>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar17-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000105e <foo\+0x5e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x4
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001066 <foo\+0x66>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x3
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000106e <foo\+0x6e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x2
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	00001076 <foo\+0x76>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x1
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4060 0000 	bal	0000107e <foo\+0x7e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18
[0-9a-f]+ <[^>]*> 0003 12d0 	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	00001086 <foo\+0x86>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar0-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	0000108c <foo\+0x8c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar1-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	00001092 <foo\+0x92>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar2-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	00001098 <foo\+0x98>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar3-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	0000109e <foo\+0x9e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010a4 <foo\+0xa4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010aa <foo\+0xaa>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010b0 <foo\+0xb0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010b6 <foo\+0xb6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010bc <foo\+0xbc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar16-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010c2 <foo\+0xc2>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar17-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010c8 <foo\+0xc8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010ce <foo\+0xce>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010d4 <foo\+0xd4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010da <foo\+0xda>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> 4260 0000 	bals	000010e0 <foo\+0xe0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,000010e6 <foo\+0xe6>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar0-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,000010ec <foo\+0xec>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar1-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,000010f2 <foo\+0xf2>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar2-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,000010f8 <foo\+0xf8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar3-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,000010fe <foo\+0xfe>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001104 <foo\+0x104>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,0000110a <foo\+0x10a>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001110 <foo\+0x110>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001116 <foo\+0x116>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,0000111c <foo\+0x11c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar16-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001122 <foo\+0x122>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar17-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001128 <foo\+0x128>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,0000112e <foo\+0x12e>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x3
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001134 <foo\+0x134>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,0000113a <foo\+0x13a>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> b462 0000 	bne	v0,v1,00001140 <foo\+0x140>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001144 <foo\+0x144>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar0-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001148 <foo\+0x148>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar1-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	0000114c <foo\+0x14c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar2-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001150 <foo\+0x150>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar3-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001154 <foo\+0x154>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar4-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001158 <foo\+0x158>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar4-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	0000115c <foo\+0x15c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001160 <foo\+0x160>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001164 <foo\+0x164>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001168 <foo\+0x168>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar16-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	0000116c <foo\+0x16c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar17-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001170 <foo\+0x170>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar18-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001174 <foo\+0x174>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar18-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001178 <foo\+0x178>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	0000117c <foo\+0x17c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> cc00      	b	00001180 <foo\+0x180>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	bar18\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,00001184 <foo\+0x184>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar0-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,00001188 <foo\+0x188>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar1-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,0000118c <foo\+0x18c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar2-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,00001190 <foo\+0x190>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar3-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,00001194 <foo\+0x194>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar4-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,00001198 <foo\+0x198>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar4-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,0000119c <foo\+0x19c>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar4
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011a0 <foo\+0x1a0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar4\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011a4 <foo\+0x1a4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar4\+0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011a8 <foo\+0x1a8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar16-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011ac <foo\+0x1ac>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar17-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011b0 <foo\+0x1b0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar18-0x2
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011b4 <foo\+0x1b4>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar18-0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011b8 <foo\+0x1b8>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar18
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011bc <foo\+0x1bc>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar18\+0x1
[0-9a-f]+ <[^>]*> 4413      	not	v0,v1
[0-9a-f]+ <[^>]*> ad00      	bnez	v0,000011c0 <foo\+0x1c0>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC7_S1	bar18\+0x2
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
