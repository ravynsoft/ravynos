#as: -L -I $srcdir/$subdir
#objdump: -P dysymtab
#target: i?86-*-darwin* powerpc-*-darwin*
#source: symbols-6.s
.*: +file format mach-o.*
#...
Load command dysymtab:
( )+local symbols: idx:( )+0( )+num: 55( )+\(nxtidx: 55\)
( )+external symbols: idx:( )+55( )+num: 24( )+\(nxtidx: 79\)
( )+undefined symbols: idx:( )+79( )+num: 30( )+\(nxtidx: 109\)
( )+table of content: off: 0x00000000( )+num: 0( )+\(endoff: 0x00000000\)
( )+module table: off: 0x00000000( )+num: 0( )+\(endoff: 0x00000000\)
( )+external reference table: off: 0x00000000( )+num: 0( )+\(endoff: 0x00000000\)
( )+indirect symbol table: off: 0x00000428( )+num: 25( )+\(endoff: 0x0000048c\)
( )+external relocation table: off: 0x00000000( )+num: 0( )+\(endoff: 0x00000000\)
( )+local relocation table: off: 0x00000000( )+num: 0( )+\(endoff: 0x00000000\)
( )+indirect symbols:
( )+for section __dummy.__dummy:
( )+0000000000000096( )+0: 0x0000005e a
( )+000000000000009e( )+1: 0x00000063 b
( )+00000000000000a6( )+2: 0x0000003d c
( )+00000000000000ae( )+3: 0x0000001b d
( )+00000000000000b6( )+4: 0x00000018 e
( )+00000000000000be( )+5: 0x00000040 f
( )+00000000000000c6( )+6: 0x00000066 g
( )+for section __DATA.__la_symbol_ptr:
( )+00000000000000d0( )+7: 0x0000005f a1
( )+00000000000000d4( )+8: 0x00000064 b1
( )+00000000000000d8( )+9: 0x0000003e c1
( )+00000000000000dc( )+10: 0x0000001c d1
( )+00000000000000e0( )+11: 0x00000019 e1
( )+00000000000000e4( )+12: 0x00000041 f1
( )+00000000000000e8( )+13: 0x00000067 g1
( )+for section __DATA.__nl_symbol_ptr:
( )+00000000000000ec( )+14: 0x00000060 a2
( )+00000000000000f0( )+15: 0x00000065 b2
( )+00000000000000f4( )+16: 0x0000003f c2
( )+00000000000000f8( )+17: 0x80000000 LOCAL
( )+00000000000000fc( )+18: 0x80000000 LOCAL
( )+0000000000000100( )+19: 0x00000042 f2
( )+0000000000000104( )+20: 0x00000068 g2
( )+0000000000000108( )+21: 0x00000041 f1
( )+000000000000010c( )+22: 0x00000067 g1
( )+0000000000000110( )+23: 0x00000060 a2
( )+0000000000000114( )+24: 0x00000065 b2
