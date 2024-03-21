#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch-misc-5pic
#source: branch-misc-5.s
#as: -32 -call_shared

# Test branches to undefined symbols and a defined local symbol
# in another section (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	0+0000 <g6>
			0: R_MICROMIPS_PC16_S1	x1
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	0+0004 <g6\+0x4>
			4: R_MICROMIPS_PC16_S1	x2
[0-9a-f]+ <[^>]*> 40e0 fffe 	bc	0+0008 <g6\+0x8>
			8: R_MICROMIPS_PC16_S1	\.Ldata
	\.\.\.
