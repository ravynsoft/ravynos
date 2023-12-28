#as: -L
#objdump: -P dysymtab
#target: i?86-*-darwin* powerpc-*-darwin*
#source: symbols-7.s
.*: +file format mach-o.*
#...
Load command dysymtab:
( )+local symbols: idx:( )+0  num: 4( )+\(nxtidx: 4\)
( )+external symbols: idx:( )+4  num: 1( )+\(nxtidx: 5\)
( )+undefined symbols: idx:( )+5  num: 0( )+\(nxtidx: 5\)
( )+table of content: off: 0x00000000  num: 0( )+\(endoff: 0x00000000\)
( )+module table: off: 0x00000000  num: 0( )+\(endoff: 0x00000000\)
( )+external reference table: off: 0x00000000  num: 0( )+\(endoff: 0x00000000\)
( )+indirect symbol table: off: 0x00000170  num: 4( )+\(endoff: 0x00000180\)
( )+external relocation table: off: 0x00000000  num: 0( )+\(endoff: 0x00000000\)
( )+local relocation table: off: 0x00000000  num: 0( )+\(endoff: 0x00000000\)
( )+indirect symbols:
( )+for section __DATA.__nl_symbol_ptr:
( )+0000000000000014( )+0: 0xc0000000 LOCAL ABSOLUTE
( )+0000000000000018( )+1: 0x80000000 LOCAL
( )+000000000000001c( )+2: 0xc0000000 LOCAL ABSOLUTE
( )+0000000000000020( )+3: 0x00000004 c
