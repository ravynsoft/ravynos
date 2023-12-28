#source: align.s
#source: tlspic1.s
#source: tlspic2.s
#as:
#ld: -shared -melf64alpha
#readelf: -WSsrl
#target: alpha*-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 0+ +0 +0 +0
 +\[[ 0-9]+\] .hash +.*
 +\[[ 0-9]+\] .dynsym +.*
 +\[[ 0-9]+\] .dynstr +.*
 +\[[ 0-9]+\] .rela.dyn +.*
 +\[[ 0-9]+\] .rela.plt +.*
 +\[[ 0-9]+\] .plt +.*
 +\[[ 0-9]+\] .text +PROGBITS +0+1000 0+1000 0+ac 0+ +AX +0 +0 4096
 +\[[ 0-9]+\] .eh_frame +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +A +0 +0 +8
 +\[[ 0-9]+\] .tdata +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 0+ WAT +0 +0 +4
 +\[[ 0-9]+\] .tbss +NOBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 0+ WAT +0 +0 +1
 +\[[ 0-9]+\] .dynamic +DYNAMIC +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 10 +WA +3 +0 +8
 +\[[ 0-9]+\] .got +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 0+ +WA +0 +0 +8
 +\[[ 0-9]+\] .symtab +.*
 +\[[ 0-9]+\] .strtab +.*
 +\[[ 0-9]+\] .shstrtab +.*
#...

Elf file type is DYN \(Shared object file\)
Entry point 0x[0-9a-f]+
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +LOAD +0x0+ 0x0+ 0x0+ 0x[0-9a-f]+ 0x[0-9a-f]+ R E 0x10000
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW  0x10000
 +DYNAMIC +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW +0x8
 +TLS +0x0+10d8 0x0+110d8 0x0+110d8 0x0+60 0x0+80 R +0x4
#...

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 7 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_DTPMOD64 +0+ sg1 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_DTPREL64 +0+ sg1 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_TPREL64 +0+4 sg2 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_TPREL64 +44
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_DTPMOD64 +0+
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_DTPMOD64 +0+
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_TPREL64 +24

Relocation section '.rela.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ R_ALPHA_JMP_SLOT +0+ __tls_get_addr \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* [0-9a-f]+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg8
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg3
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg4
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg5
.* [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +UND __tls_get_addr
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg1
.* [0-9a-f]+ +172 +FUNC +GLOBAL +DEFAULT +\[STD GPLOAD\] +7 fn1
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg2
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg6
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg7

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* [0-9a-f]+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +1.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +2.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +3.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +4.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +5.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +6.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +7.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +8.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +9.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +10.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +11.*
.* [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +12.*
.* FILE +LOCAL +DEFAULT +ABS .*
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl1
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl2
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl3
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl4
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl5
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl6
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl7
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sl8
.* FILE +LOCAL +DEFAULT +ABS 
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH1
.* [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +ABS _DYNAMIC
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh3
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH2
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH7
.* [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +ABS _PROCEDURE_LINKAGE_TABLE_
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh7
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh8
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH4
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh4
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH3
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh5
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH5
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH6
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +10 sH8
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh1
.* [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +ABS _GLOBAL_OFFSET_TABLE_
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh2
.* [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +9 sh6
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg8
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg3
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg4
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg5
.* [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +UND __tls_get_addr
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg1
.* [0-9a-f]+ +172 +FUNC +GLOBAL +DEFAULT +\[STD GPLOAD\] +7 fn1
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg2
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg6
.* [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +9 sg7
