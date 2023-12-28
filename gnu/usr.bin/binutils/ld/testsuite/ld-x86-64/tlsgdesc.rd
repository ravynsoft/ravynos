#source: tlsgdesc.s
#as: --64
#ld: -shared -melf64_x86_64 --no-ld-generated-unwind-info
#readelf: -WSsrl
#target: x86_64-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 0+ +0 +0 +0
 +\[[ 0-9]+\] \.hash +.*
 +\[[ 0-9]+\] \.dynsym +.*
 +\[[ 0-9]+\] \.dynstr +.*
 +\[[ 0-9]+\] \.rela.dyn +.*
 +\[[ 0-9]+\] \.rela.plt +.*
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
 +00 +.hash .dynsym .dynstr .rela.dyn .rela.plt .plt .text *
 +01 +.dynamic .got .got.plt *
 +02 +.dynamic *

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 8 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +0+100000012 R_X86_64_TPOFF64 +0+ sG3 \+ 0
[0-9a-f]+ +0+200000012 R_X86_64_TPOFF64 +0+ sG5 \+ 0
[0-9a-f]+ +0+300000010 R_X86_64_DTPMOD64 +0+ sG2 \+ 0
[0-9a-f]+ +0+300000011 R_X86_64_DTPOFF64 +0+ sG2 \+ 0
[0-9a-f]+ +0+400000012 R_X86_64_TPOFF64 +0+ sG4 \+ 0
[0-9a-f]+ +0+600000012 R_X86_64_TPOFF64 +0+ sG6 \+ 0
[0-9a-f]+ +0+800000010 R_X86_64_DTPMOD64 +0+ sG1 \+ 0
[0-9a-f]+ +0+800000011 R_X86_64_DTPOFF64 +0+ sG1 \+ 0

Relocation section '.rela.plt' at offset 0x[0-9a-f]+ contains 3 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +0+500000007 R_X86_64_JUMP_SLOT +0+ __tls_get_addr \+ 0
[0-9a-f]+ +0+800000024 R_X86_64_TLSDESC +0+ sG1 \+ 0
[0-9a-f]+ +0+300000024 R_X86_64_TLSDESC +0+ sG2 \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG4
 +[0-9]+: 0+ +0 +NOTYPE +GLOBAL +DEFAULT +UND __tls_get_addr
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG6
 +[0-9]+: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +7 fc1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +8 _DYNAMIC
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +10 _GLOBAL_OFFSET_TABLE_
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG4
 +[0-9]+: 0+ +0 +NOTYPE +GLOBAL +DEFAULT +UND __tls_get_addr
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG6
 +[0-9]+: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +7 fc1
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1
