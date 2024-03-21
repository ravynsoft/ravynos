#name: MIPS LWPC from unaligned symbol 0
#source: unaligned-lwpc-0.s
#source: unaligned-data.s
#as: -mips32r6
#ld: -Ttext 0x1c000000 -Tdata 0x1c080000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> ec4a0008 	lwpc	v0,1c080020 <bar0>
[0-9a-f]+ <[^>]*> ec4a0008 	lwpc	v0,1c080024 <bar4>
	\.\.\.
