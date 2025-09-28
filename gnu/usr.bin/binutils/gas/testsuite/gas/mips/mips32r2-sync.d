#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS32r2 sync instructions
#as: -32
#source: mips32r2-sync.s

# Check MIPS32r2 sync instructions assembly and disassembly

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0000000f 	sync
[0-9a-f]+ <[^>]*> 0000008f 	sync	0x2
[0-9a-f]+ <[^>]*> 0000010f 	sync_wmb
[0-9a-f]+ <[^>]*> 0000020f 	sync	0x8
[0-9a-f]+ <[^>]*> 0000040f 	sync_mb
[0-9a-f]+ <[^>]*> 0000044f 	sync_acquire
[0-9a-f]+ <[^>]*> 0000048f 	sync_release
[0-9a-f]+ <[^>]*> 000004cf 	sync_rmb
[0-9a-f]+ <[^>]*> 0000060f 	sync	0x18
[0-9a-f]+ <[^>]*> 0000000f 	sync
[0-9a-f]+ <[^>]*> 0000008f 	sync	0x2
[0-9a-f]+ <[^>]*> 0000010f 	sync_wmb
[0-9a-f]+ <[^>]*> 0000020f 	sync	0x8
[0-9a-f]+ <[^>]*> 0000040f 	sync_mb
[0-9a-f]+ <[^>]*> 0000044f 	sync_acquire
[0-9a-f]+ <[^>]*> 0000048f 	sync_release
[0-9a-f]+ <[^>]*> 000004cf 	sync_rmb
[0-9a-f]+ <[^>]*> 0000060f 	sync	0x18
	\.\.\.
