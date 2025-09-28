#name: MIPS LDPC from unaligned symbol 0
#source: unaligned-ldpc-0.s
#source: unaligned-data.s
#as: -mips64r6
#ld: -Ttext 0x1c000000 -Tdata 0x1c080000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> ec590004 	ldpc	v0,1c080020 <bar0>
[0-9a-f]+ <[^>]*> ec590005 	ldpc	v0,1c080028 <bar8>
	\.\.\.
