#source: tlsbinpic2.s
#source: tlsbin.s
#as: --64
#ld: -shared -melf_x86_64 --no-ld-generated-unwind-info
#readelf: -WSsrl
#target: x86_64-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 00 +0 +0 +0
 +\[[ 0-9]+\] .interp +.*
 +\[[ 0-9]+\] .hash +.*
 +\[[ 0-9]+\] .dynsym +.*
 +\[[ 0-9]+\] .dynstr +.*
 +\[[ 0-9]+\] .rela.dyn +.*
 +\[[ 0-9]+\] .text +PROGBITS +0+401000 0+1000 0+233 00 +AX +0 +0 +4096
 +\[[ 0-9]+\] .tdata +PROGBITS +0+601233 0+1233 0+60 00 WAT +0 +0 +1
 +\[[ 0-9]+\] .tbss +NOBITS +0+601293 0+1293 0+40 00 WAT +0 +0 +1
 +\[[ 0-9]+\] .dynamic +DYNAMIC +0+601298 0+1298 0+100 10 +WA +4 +0 +8
 +\[[ 0-9]+\] .got +PROGBITS +0+601398 0+1398 0+28 08 +WA +0 +0 +8
 +\[[ 0-9]+\] .got.plt +PROGBITS +0+6013c0 0+13c0 0+18 08 +WA +0 +0 +8
 +\[[ 0-9]+\] .symtab +.*
 +\[[ 0-9]+\] .strtab +.*
 +\[[ 0-9]+\] .shstrtab +.*
Key to Flags:
#...

Elf file type is EXEC \(Executable file\)
Entry point 0x40113b
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +PHDR.*
 +INTERP.*
.*Requesting program interpreter.*
 +LOAD +0x0+ 0x0+400000 0x0+400000 0x0+1233 0x0+1233 R E 0x200000
 +LOAD +0x0+1233 0x0+601233 0x0+601233 0x0+1a5 0x0+1a5 RW +0x200000
 +DYNAMIC +0x0+1298 0x0+601298 0x0+601298 0x0+100 0x0+100 RW +0x8
 +TLS +0x0+1233 0x0+601233 0x0+601233 0x0+60 0x0+a0 R +0x1

 Section to Segment mapping:
 +Segment Sections...
 +00 *
 +01 +.interp *
 +02 +.interp .hash .dynsym .dynstr .rela.dyn .text *
 +03 +.tdata .dynamic .got .got.plt *
 +04 +.dynamic *
 +05 +.tdata .tbss *

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 5 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_X86_64_TPOFF64 +0+ sG5 \+ 0
[0-9a-f ]+R_X86_64_TPOFF64 +0+ sG2 \+ 0
[0-9a-f ]+R_X86_64_GLOB_DAT +0+ __tls_get_addr \+ 0
[0-9a-f ]+R_X86_64_TPOFF64 +0+ sG6 \+ 0
[0-9a-f ]+R_X86_64_TPOFF64 +0+ sG1 \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND *
.* TLS +GLOBAL +DEFAULT +UND sG5
.* TLS +GLOBAL +DEFAULT +UND sG2
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr
.* TLS +GLOBAL +DEFAULT +UND sG6
.* TLS +GLOBAL +DEFAULT +UND sG1

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND *
.* FILE +LOCAL +DEFAULT +ABS .*tlsbinpic2.o
.* TLS +LOCAL +DEFAULT +7 sl1
.* TLS +LOCAL +DEFAULT +7 sl2
.* TLS +LOCAL +DEFAULT +7 sl3
.* TLS +LOCAL +DEFAULT +7 sl4
.* TLS +LOCAL +DEFAULT +7 sl5
.* TLS +LOCAL +DEFAULT +7 sl6
.* TLS +LOCAL +DEFAULT +7 sl7
.* TLS +LOCAL +DEFAULT +7 sl8
.* FILE +LOCAL +DEFAULT +ABS .*tlsbin.o
.* TLS +LOCAL +DEFAULT +8 bl1
.* TLS +LOCAL +DEFAULT +8 bl2
.* TLS +LOCAL +DEFAULT +8 bl3
.* TLS +LOCAL +DEFAULT +8 bl4
.* TLS +LOCAL +DEFAULT +8 bl5
.* TLS +LOCAL +DEFAULT +8 bl6
.* TLS +LOCAL +DEFAULT +8 bl7
.* TLS +LOCAL +DEFAULT +8 bl8
.* FILE +LOCAL +DEFAULT +ABS 
.* OBJECT +LOCAL +DEFAULT +9 _DYNAMIC
.* OBJECT +LOCAL +DEFAULT +11 _GLOBAL_OFFSET_TABLE_
.* TLS +GLOBAL +DEFAULT +7 sg8
.* TLS +GLOBAL +DEFAULT +8 bg8
.* TLS +GLOBAL +DEFAULT +8 bg6
.* TLS +GLOBAL +DEFAULT +UND sG5
.* TLS +GLOBAL +DEFAULT +8 bg3
.* TLS +GLOBAL +DEFAULT +7 sg3
.* TLS +GLOBAL +HIDDEN +7 sh3
.* TLS +GLOBAL +DEFAULT +UND sG2
.* TLS +GLOBAL +DEFAULT +7 sg4
.* TLS +GLOBAL +DEFAULT +7 sg5
.* TLS +GLOBAL +DEFAULT +8 bg5
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr
.* TLS +GLOBAL +HIDDEN +7 sh7
.* TLS +GLOBAL +HIDDEN +7 sh8
.* TLS +GLOBAL +DEFAULT +7 sg1
.* FUNC +GLOBAL +DEFAULT +6 _start
.* TLS +GLOBAL +HIDDEN +7 sh4
.* TLS +GLOBAL +DEFAULT +8 bg7
.* TLS +GLOBAL +HIDDEN +7 sh5
.* NOTYPE +GLOBAL +DEFAULT +11 __bss_start
.* TLS +GLOBAL +DEFAULT +UND sG6
.* FUNC +GLOBAL +DEFAULT +6 fn2
.* TLS +GLOBAL +DEFAULT +7 sg2
.* TLS +GLOBAL +DEFAULT +UND sG1
.* TLS +GLOBAL +HIDDEN +7 sh1
.* TLS +GLOBAL +DEFAULT +7 sg6
.* TLS +GLOBAL +DEFAULT +7 sg7
.* NOTYPE +GLOBAL +DEFAULT +11 _edata
.* NOTYPE +GLOBAL +DEFAULT +11 _end
.* TLS +GLOBAL +HIDDEN +7 sh2
.* TLS +GLOBAL +HIDDEN +7 sh6
.* TLS +GLOBAL +DEFAULT +8 bg2
.* TLS +GLOBAL +DEFAULT +8 bg1
.* TLS +GLOBAL +DEFAULT +8 bg4
