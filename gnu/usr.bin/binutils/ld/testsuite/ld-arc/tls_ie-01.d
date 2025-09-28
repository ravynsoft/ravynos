#source: tls_ie-01.s
#as: -mcpu=arc700
#ld: -static
#objdump: -D -j .got
#
# sample outputs:
# 1) baremetal toolchain (little endian)
# ,-------------------------------------------------------.
# |tmpdir/dump:     file format elf32-littlearc           |
# |                                                       |
# |                                                       |
# |Disassembly of section .got:                           |
# |                                                       |
# |00002110 <_GLOBAL_OFFSET_TABLE_>:                      |
# |	...                                               |
# |    211c:	08 00 00 00     	.word	0x00000008|
# |    2120:	0c 00 00 00     	.word	0x0000000c|
# `-------------------------------------------------------'
#
# 2) linux toolchain (little endian)
# ,-------------------------------------------------------.
# |tmpdir/dump:     file format elf32-littlearc           |
# |                                                       |
# |                                                       |
# |Disassembly of section .got:                           |
# |                                                       |
# |000120a4 <.got>:                                       |
# |   120a4:	08 00 00 00     	.word	0x00000008|
# |   120a8:	0c 00 00 00     	.word	0x0000000c|
# `-------------------------------------------------------'
#
# 3) baremetal toolchain (big endian)
# ,-------------------------------------------------------.
# |tmpdir/dump:     file format elf32-bigarc              |
# |                                                       |
# |                                                       |
# |Disassembly of section .got:                           |
# |                                                       |
# |00002110 <_GLOBAL_OFFSET_TABLE_>:                      |
# |	...                                               |
# |    211c:	00 00 00 08        	.word	0x00000008|
# |    2120:	00 00 00 0c        	.word	0x0000000c|
# `-------------------------------------------------------'
#
# 4) linux toolchain (big endian)
# ,-------------------------------------------------------.
# |tmpdir/dump:     file format elf32-bigarc              |
# |                                                       |
# |                                                       |
# |Disassembly of section .got:                           |
# |                                                       |
# |00013ff4 <.got>:                                       |
# |   13ff4:	00 00 00 08        	.word	0x00000008|
# |   13ff8:	00 00 00 0c        	.word	0x0000000c|
# `-------------------------------------------------------'

#...
Disassembly of section \.got:
#...
\s+[0-9a-f]+:\s+[0-9a-f\s]+\.word\s+0x0+8
\s+[0-9a-f]+:\s+[0-9a-f\s]+\.word\s+0x0+c
#pass
