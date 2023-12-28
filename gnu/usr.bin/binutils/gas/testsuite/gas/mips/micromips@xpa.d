#objdump: -dr --prefix-addresses --show-raw-insn -M cp0-names=mips32r2
#name: XPA instructions
#source: xpa.s
#as: -32 -mxpa -mvirt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0041 00f4 	mfhc0	v0,c0_random
[0-9a-f]+ <[^>]*> 0050 00f4 	mfhc0	v0,c0_config
[0-9a-f]+ <[^>]*> 0040 10f4 	mfhc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 0040 38f4 	mfhc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 0041 02f4 	mthc0	v0,c0_random
[0-9a-f]+ <[^>]*> 0050 02f4 	mthc0	v0,c0_config
[0-9a-f]+ <[^>]*> 0040 12f4 	mthc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 0040 3af4 	mthc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 0041 04f4 	mfhgc0	v0,c0_random
[0-9a-f]+ <[^>]*> 0050 04f4 	mfhgc0	v0,c0_config
[0-9a-f]+ <[^>]*> 0040 14f4 	mfhgc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 0040 3cf4 	mfhgc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 0041 06f4 	mthgc0	v0,c0_random
[0-9a-f]+ <[^>]*> 0050 06f4 	mthgc0	v0,c0_config
[0-9a-f]+ <[^>]*> 0040 16f4 	mthgc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 0040 3ef4 	mthgc0	v0,\$0,7
	\.\.\.
