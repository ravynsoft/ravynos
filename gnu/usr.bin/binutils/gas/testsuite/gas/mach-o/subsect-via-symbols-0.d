#objdump: -P header
#source: empty.s
.*: +file format mach-o.*
#...
.*flags +: 00000000 \(-\)
#pass
