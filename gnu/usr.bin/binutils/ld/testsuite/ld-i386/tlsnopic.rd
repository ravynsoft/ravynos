#source: tlsnopic1.s
#source: tlsnopic2.s
#as: --32
#ld: -shared -melf_i386 --no-ld-generated-unwind-info
#readelf: -Ssrl
#target: i?86-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Addr +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 0+ +0 +0 +0
 +\[[ 0-9]+\] \.hash +.*
 +\[[ 0-9]+\] \.dynsym +.*
 +\[[ 0-9]+\] \.dynstr +.*
 +\[[ 0-9]+\] \.rel.dyn +.*
 +\[[ 0-9]+\] \.text +PROGBITS +0+1000 .*
 +\[[ 0-9]+\] \.tbss +NOBITS +[0-9a-f]+ [0-9a-f]+ 000024 00 WAT +0 +0 +1
 +\[[ 0-9]+\] \.dynamic +DYNAMIC +0+20f4 .*
 +\[[ 0-9]+\] \.got +PROGBITS +0+2174 .*
 +\[[ 0-9]+\] \.got.plt +PROGBITS +0+218c .*
 +\[[ 0-9]+\] \.symtab +.*
 +\[[ 0-9]+\] \.strtab +.*
 +\[[ 0-9]+\] \.shstrtab +.*
Key to Flags:
#...

Elf file type is DYN \(Shared object file\)
Entry point 0x[0-9a-f]+
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +LOAD.*
 +LOAD.*
 +DYNAMIC.*
 +TLS +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x0+ 0x0+24 R +0x1

 Section to Segment mapping:
 +Segment Sections...
 +00 +.hash .dynsym .dynstr .rel.dyn .text *
 +01 +.dynamic .got .got.plt *
 +02 +.dynamic *
 +03 +.tbss *

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 20 entries:
 Offset +Info +Type +Sym.Value +Sym. Name
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_RELATIVE +
[0-9a-f ]+R_386_TLS_TPOFF32 0+ +sg3
[0-9a-f ]+R_386_TLS_TPOFF32
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sg4
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sg5
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sg1
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sg2


Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg4
 +[0-9]+: 0+1000 +0 +FUNC +GLOBAL +DEFAULT +5 fn3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg2

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
.* FILE +LOCAL +DEFAULT +ABS .*tlsnopic1.o
 +[0-9]+: 0+00 +0 +TLS +LOCAL +DEFAULT +6 bl1
 +[0-9]+: 0+04 +0 +TLS +LOCAL +DEFAULT +6 bl2
 +[0-9]+: 0+08 +0 +TLS +LOCAL +DEFAULT +6 bl3
 +[0-9]+: 0+0c +0 +TLS +LOCAL +DEFAULT +6 bl4
 +[0-9]+: 0+10 +0 +TLS +LOCAL +DEFAULT +6 bl5
.* FILE +LOCAL +DEFAULT +ABS 
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +7 _DYNAMIC
 +[0-9]+: 0+1c +0 +TLS +LOCAL +DEFAULT +6 sh3
 +[0-9]+: 0+20 +0 +TLS +LOCAL +DEFAULT +6 sh4
 +[0-9]+: 0+14 +0 +TLS +LOCAL +DEFAULT +6 sh1
 +[0-9]+: 0+218c +0 +OBJECT +LOCAL +DEFAULT +9 _GLOBAL_OFFSET_TABLE_
 +[0-9]+: 0+18 +0 +TLS +LOCAL +DEFAULT +6 sh2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg4
 +[0-9]+: 0+1000 +0 +FUNC +GLOBAL +DEFAULT +5 fn3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sg2
