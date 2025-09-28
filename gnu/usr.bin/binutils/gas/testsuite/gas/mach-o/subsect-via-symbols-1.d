#objdump: -P header
#source: subsect-via-symbols.s
.*: +file format mach-o.*
#...
.*flags +: 00002000 \(subsections_via_symbols\)
#pass
