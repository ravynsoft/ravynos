#objdump: -dr --prefix-addresses --show-raw-insn -M cp0-names=mips32r2
#name: XPA instructions
#as: -32 -mxpa -mvirt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40420800 	mfhc0	v0,c0_random
[0-9a-f]+ <[^>]*> 40428000 	mfhc0	v0,c0_config
[0-9a-f]+ <[^>]*> 40420002 	mfhc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 40420007 	mfhc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 40c20800 	mthc0	v0,c0_random
[0-9a-f]+ <[^>]*> 40c28000 	mthc0	v0,c0_config
[0-9a-f]+ <[^>]*> 40c20002 	mthc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 40c20007 	mthc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 40620c00 	mfhgc0	v0,c0_random
[0-9a-f]+ <[^>]*> 40628400 	mfhgc0	v0,c0_config
[0-9a-f]+ <[^>]*> 40620402 	mfhgc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 40620407 	mfhgc0	v0,\$0,7
[0-9a-f]+ <[^>]*> 40620e00 	mthgc0	v0,c0_random
[0-9a-f]+ <[^>]*> 40628600 	mthgc0	v0,c0_config
[0-9a-f]+ <[^>]*> 40620602 	mthgc0	v0,c0_mvpconf0
[0-9a-f]+ <[^>]*> 40620607 	mthgc0	v0,\$0,7
	...
