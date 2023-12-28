#source: tlspic1.s
#source: tlspic2.s
#as:
#ld: -shared -melf32xtensa
#readelf: -WSsrl
#target: xtensa*-*-linux*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Addr +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 00 +0 +0 +0
 +\[[ 0-9]+\] .hash +.*
 +\[[ 0-9]+\] .dynsym +.*
 +\[[ 0-9]+\] .dynstr +.*
 +\[[ 0-9]+\] .rela.dyn +.*
 +\[[ 0-9]+\] .text +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +AX +0 +0 +4
 +\[[ 0-9]+\] .got.loc +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +A +0 +0 +4
 +\[[ 0-9]+\] .tdata +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WAT +0 +0 +4
 +\[[ 0-9]+\] .tbss +NOBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WAT +0 +0 +4
 +\[[ 0-9]+\] .dynamic +DYNAMIC +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 08 +WA +3 +0 +4
 +\[[ 0-9]+\] .got +PROGBITS +[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 +WA +0 +0 +4
 +\[[ 0-9]+\] .xtensa.info +NOTE +0+ .*
 +\[[ 0-9]+\] .xt.lit +PROGBITS +0+ .*
 +\[[ 0-9]+\] .xt.prop +PROGBITS +0+ .*
 +\[[ 0-9]+\] .symtab +.*
 +\[[ 0-9]+\] .strtab +.*
 +\[[ 0-9]+\] .shstrtab +.*
Key to Flags:
#...

Elf file type is DYN \(Shared object file\)
Entry point 0x[0-9a-f]+
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R E 0x1000
 +LOAD +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW +0x1000
 +DYNAMIC +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW +0x4
 +TLS +0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R +0x4

 Section to Segment mapping:
 +Segment Sections...
 +00 +.hash .dynsym .dynstr .rela.dyn .text .got.loc *
 +01 +.tdata .dynamic .got *
 +02 +.dynamic *
 +03 +.tdata .tbss *

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 18 entries:
 +Offset +Info +Type +Sym\. Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_FN +0+ +sg1 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_ARG +0+ +sg1 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +0+4 +sg2 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +0+4 +sg2 \+ 0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_FN +20
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_ARG +20
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +24
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_FN +40
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_ARG +40
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +44
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_FN +60
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_ARG +60
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +64
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_FN +0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLSDESC_ARG +0
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +24
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +44
[0-9a-f]+ +[0-9a-f]+ R_XTENSA_TLS_TPOFF +64

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +[0-9]+: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND *
 +[0-9]+: 0+1c +0 +TLS +GLOBAL +DEFAULT +7 sg8
 +[0-9]+: 0+8 +0 +TLS +GLOBAL +DEFAULT +7 sg3
 +[0-9]+: 0+c +0 +TLS +GLOBAL +DEFAULT +7 sg4
 +[0-9]+: 0+10 +0 +TLS +GLOBAL +DEFAULT +7 sg5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +7 sg1
 +[0-9]+: 0+350 +0 +FUNC +GLOBAL +DEFAULT +5 _start
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 __bss_start
 +[0-9]+: 0+4 +0 +TLS +GLOBAL +DEFAULT +7 sg2
 +[0-9]+: 0+14 +0 +TLS +GLOBAL +DEFAULT +7 sg6
 +[0-9]+: 0+18 +0 +TLS +GLOBAL +DEFAULT +7 sg7
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _edata
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _end

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
 +[0-9]+: [0-9a-f]+ +0 +FILE +LOCAL +DEFAULT +ABS .*tlspic1.o
 +[0-9]+: 0+20 +0 +TLS +LOCAL +DEFAULT +7 sl1
 +[0-9]+: 0+24 +0 +TLS +LOCAL +DEFAULT +7 sl2
 +[0-9]+: 0+28 +0 +TLS +LOCAL +DEFAULT +7 sl3
 +[0-9]+: 0+2c +0 +TLS +LOCAL +DEFAULT +7 sl4
 +[0-9]+: 0+30 +0 +TLS +LOCAL +DEFAULT +7 sl5
 +[0-9]+: 0+34 +0 +TLS +LOCAL +DEFAULT +7 sl6
 +[0-9]+: 0+38 +0 +TLS +LOCAL +DEFAULT +7 sl7
 +[0-9]+: 0+3c +0 +TLS +LOCAL +DEFAULT +7 sl8
 +[0-9]+: 0+ +0 +FILE +LOCAL +DEFAULT +ABS *
 +[0-9]+: 0+60 +0 +TLS +LOCAL +DEFAULT +8 sH1
 +[0-9]+: 0+ +0 +TLS +LOCAL +DEFAULT +7 _TLS_MODULE_BASE_
 +[0-9]+: 0+144c +0 +OBJECT +LOCAL +DEFAULT +ABS _DYNAMIC
 +[0-9]+: 0+48 +0 +TLS +LOCAL +DEFAULT +7 sh3
 +[0-9]+: 0+64 +0 +TLS +LOCAL +DEFAULT +8 sH2
 +[0-9]+: 0+78 +0 +TLS +LOCAL +DEFAULT +8 sH7
 +[0-9]+: 0+58 +0 +TLS +LOCAL +DEFAULT +7 sh7
 +[0-9]+: 0+5c +0 +TLS +LOCAL +DEFAULT +7 sh8
 +[0-9]+: 0+6c +0 +TLS +LOCAL +DEFAULT +8 sH4
 +[0-9]+: 0+4c +0 +TLS +LOCAL +DEFAULT +7 sh4
 +[0-9]+: 0+68 +0 +TLS +LOCAL +DEFAULT +8 sH3
 +[0-9]+: 0+50 +0 +TLS +LOCAL +DEFAULT +7 sh5
 +[0-9]+: 0+70 +0 +TLS +LOCAL +DEFAULT +8 sH5
 +[0-9]+: 0+74 +0 +TLS +LOCAL +DEFAULT +8 sH6
 +[0-9]+: 0+7c +0 +TLS +LOCAL +DEFAULT +8 sH8
 +[0-9]+: 0+40 +0 +TLS +LOCAL +DEFAULT +7 sh1
 +[0-9]+: 0+44 +0 +TLS +LOCAL +DEFAULT +7 sh2
 +[0-9]+: 0+54 +0 +TLS +LOCAL +DEFAULT +7 sh6
 +[0-9]+: 0+1c +0 +TLS +GLOBAL +DEFAULT +7 sg8
 +[0-9]+: 0+8 +0 +TLS +GLOBAL +DEFAULT +7 sg3
 +[0-9]+: 0+c +0 +TLS +GLOBAL +DEFAULT +7 sg4
 +[0-9]+: 0+10 +0 +TLS +GLOBAL +DEFAULT +7 sg5
 +[0-9]+: 0+ +0 +TLS +GLOBAL +DEFAULT +7 sg1
 +[0-9]+: 0+350 +0 +FUNC +GLOBAL +DEFAULT +5 _start
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 __bss_start
 +[0-9]+: 0+4 +0 +TLS +GLOBAL +DEFAULT +7 sg2
 +[0-9]+: 0+14 +0 +TLS +GLOBAL +DEFAULT +7 sg6
 +[0-9]+: 0+18 +0 +TLS +GLOBAL +DEFAULT +7 sg7
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _edata
 +[0-9]+: [0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +10 _end
