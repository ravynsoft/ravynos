#source: tlsbin.s
#as:
#ld: -melf32xtensa
#readelf: -WSsrl
#target: xtensa*-*-linux*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Addr +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 00 +0 +0 +0
 +\[[ 0-9]+\] .interp +.*
 +\[[ 0-9]+\] .hash +.*
 +\[[ 0-9]+\] .dynsym +.*
 +\[[ 0-9]+\] .dynstr +.*
 +\[[ 0-9]+\] .rela.dyn +.*
 +\[[ 0-9]+\] .text +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +AX +0 +0 +4
 +\[[ 0-9]+\] .got.loc +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +A +0 +0 +4
 +\[[ 0-9]+\] .tdata +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WAT +0 +0 +4
 +\[[ 0-9]+\] .dynamic +DYNAMIC +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 08 +WA +4 +0 +4
 +\[[ 0-9]+\] .got +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WA +0 +0 +4
 +\[[ 0-9]+\] .xtensa.info +NOTE +0+ .*
 +\[[ 0-9]+\] .xt.lit +PROGBITS +0+ .*
 +\[[ 0-9]+\] .xt.prop +PROGBITS +0+ .*
 +\[[ 0-9]+\] .symtab +.*
 +\[[ 0-9]+\] .strtab +.*
 +\[[ 0-9]+\] .shstrtab +.*
Key to Flags:
#...

Elf file type is DYN \(Position-Independent Executable file\)
Entry point 0x[0-9a-f]+
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +PHDR.*
 +INTERP.*
.*Requesting program interpreter.*
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R E 0x1000
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW +0x1000
 +DYNAMIC +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW +0x4
 +TLS +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R +0x4

 Section to Segment mapping:
 +Segment Sections...
 +00 *
 +01 +.interp *
 +02 +.interp .hash .dynsym .dynstr .rela.dyn .text .got.loc *
 +03 +.tdata .dynamic .got *
 +04 +.dynamic *
 +05 +.tdata *

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 3 entries:
 +Offset +Info +Type +Sym\. Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +0+ +sG1 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +0+ +sG2 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +0+ +sG2 \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: 0+[0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 __bss_start
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1
 +[0-9]+: 0+[0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _edata
 +[0-9]+: 0+[0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _end

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +1 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +2 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +3 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +4 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +5 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +6 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +7 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +8 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +9 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +10 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +11 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +12 .*
 +[0-9]+: [0-9a-f]+ +0 +SECTION +LOCAL +DEFAULT +13 .*
 +[0-9]+: [0-9a-f]+ +0 +FILE +LOCAL +DEFAULT +ABS .*tlsbin.o
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl1
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl2
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl3
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl4
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl5
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl6
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl7
 +[0-9]+: [0-9a-f]+ +0 +TLS +LOCAL +DEFAULT +8 sl8
 +[0-9]+: 0+ +0 +FILE +LOCAL +DEFAULT +ABS *
 +[0-9]+: 0+ +0 +TLS +LOCAL +DEFAULT +8 _TLS_MODULE_BASE_
 +[0-9]+: [0-9a-f]+ +0 +OBJECT +LOCAL +DEFAULT +ABS _DYNAMIC
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg8
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg3
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh3
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG2
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg4
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg5
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh7
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh8
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg1
 +[0-9]+: [0-9a-f]+ +0 +FUNC +GLOBAL +DEFAULT +6 _start
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh4
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh5
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 __bss_start
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg2
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +UND sG1
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh1
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg6
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +DEFAULT +8 sg7
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _edata
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _end
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh2
 +[0-9]+: [0-9a-f]+ +0 +TLS +GLOBAL +HIDDEN +8 sh6
