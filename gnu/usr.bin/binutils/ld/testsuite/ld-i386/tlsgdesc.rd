#source: tlsgdesc.s
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
 +\[[ 0-9]+\] \.rel.plt +.*
 +\[[ 0-9]+\] \.plt +.*
 +\[[ 0-9]+\] \.text +.*
 +\[[ 0-9]+\] \.dynamic +.*
 +\[[ 0-9]+\] \.got +.*
 +\[[ 0-9]+\] \.got.plt +.*
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

 Section to Segment mapping:
 +Segment Sections...
 +00 +.hash .dynsym .dynstr .rel.dyn .rel.plt .plt .text *
 +01 +.dynamic .got .got.plt *
 +02 +.dynamic *

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 8 entries:
 Offset +Info +Type +Sym.Value +Sym. Name
[0-9a-f ]+R_386_TLS_TPOFF32 0+ +sG3
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sG5
[0-9a-f ]+R_386_TLS_DTPMOD3 0+ +sG2
[0-9a-f ]+R_386_TLS_DTPOFF3 0+ +sG2
[0-9a-f ]+R_386_TLS_TPOFF +0+ +sG4
[0-9a-f ]+R_386_TLS_TPOFF32 0+ +sG6
[0-9a-f ]+R_386_TLS_DTPMOD3 0+ +sG1
[0-9a-f ]+R_386_TLS_DTPOFF3 0+ +sG1

Relocation section '.rel.plt' at offset 0x[0-9a-f]+ contains 3 entries:
 Offset +Info +Type +Sym.Value +Sym. Name
[0-9a-f ]+R_386_JUMP_SLOT +0+ +___tls_get_addr
[0-9a-f ]+R_386_TLS_DESC +0+ +sG1
[0-9a-f ]+R_386_TLS_DESC +0+ +sG2

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG4
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG6
 +[0-9]+: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +7 fc1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1
 +[0-9]+: 0+ +0 +NOTYPE +GLOBAL +DEFAULT +UND ___tls_get_addr

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +8 _DYNAMIC
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +10 _GLOBAL_OFFSET_TABLE_
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG4
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG6
 +[0-9]+: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +7 fc1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1
 +[0-9]+: 0+ +0 +NOTYPE +GLOBAL +DEFAULT +UND ___tls_get_addr
