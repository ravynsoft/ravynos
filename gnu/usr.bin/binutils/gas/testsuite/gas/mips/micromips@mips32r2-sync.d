#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS32r2 sync instructions
#as: -32
#source: mips32r2-sync.s

# Check MIPS32r2 sync instructions assembly and disassembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0000 6b7c 	sync
[0-9a-f]+ <[^>]*> 0002 6b7c 	sync	0x2
[0-9a-f]+ <[^>]*> 0004 6b7c 	sync_wmb
[0-9a-f]+ <[^>]*> 0008 6b7c 	sync	0x8
[0-9a-f]+ <[^>]*> 0010 6b7c 	sync_mb
[0-9a-f]+ <[^>]*> 0011 6b7c 	sync_acquire
[0-9a-f]+ <[^>]*> 0012 6b7c 	sync_release
[0-9a-f]+ <[^>]*> 0013 6b7c 	sync_rmb
[0-9a-f]+ <[^>]*> 0018 6b7c 	sync	0x18
[0-9a-f]+ <[^>]*> 0000 6b7c 	sync
[0-9a-f]+ <[^>]*> 0002 6b7c 	sync	0x2
[0-9a-f]+ <[^>]*> 0004 6b7c 	sync_wmb
[0-9a-f]+ <[^>]*> 0008 6b7c 	sync	0x8
[0-9a-f]+ <[^>]*> 0010 6b7c 	sync_mb
[0-9a-f]+ <[^>]*> 0011 6b7c 	sync_acquire
[0-9a-f]+ <[^>]*> 0012 6b7c 	sync_release
[0-9a-f]+ <[^>]*> 0013 6b7c 	sync_rmb
[0-9a-f]+ <[^>]*> 0018 6b7c 	sync	0x18
	\.\.\.
