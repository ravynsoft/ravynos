#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-2
#source: branch-misc-2.s
#as: -32 -non_shared

# Test branches to global symbols in current file (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
	\.\.\.
	\.\.\.
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+003c <x>
			3c: R_MICROMIPS_PC16_S1	g1
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0044 <x\+0x8>
			44: R_MICROMIPS_PC16_S1	g2
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+004c <x\+0x10>
			4c: R_MICROMIPS_PC16_S1	g3
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0054 <x\+0x18>
			54: R_MICROMIPS_PC16_S1	g4
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+005c <x\+0x20>
			5c: R_MICROMIPS_PC16_S1	g5
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 4060 fffe 	bal	0+0064 <x\+0x28>
			64: R_MICROMIPS_PC16_S1	g6
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
	\.\.\.
	\.\.\.
	\.\.\.
