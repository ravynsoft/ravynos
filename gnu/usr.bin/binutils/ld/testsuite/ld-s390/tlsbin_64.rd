#source: tlsbinpic.s
#source: tlsbin.s
#as: -m64 -Aesame
#ld: -shared -melf64_s390
#readelf: -Ssrl
#target: s390x-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 00 +0 +0 +0
 +\[[ 0-9]+\] .interp .*
 +\[[ 0-9]+\] .hash .*
 +\[[ 0-9]+\] .dynsym .*
 +\[[ 0-9]+\] .dynstr .*
 +\[[ 0-9]+\] .rela.dyn .*
 +\[[ 0-9]+\] .rela.plt .*
 +\[[ 0-9]+\] .plt .*
 +\[[ 0-9]+\] .text +PROGBITS .*
 +\[[ 0-9]+\] .tdata +PROGBITS .* 0+60 00 WAT +0 +0 +32
 +\[[ 0-9]+\] .tbss +NOBITS .* 0+40 00 WAT +0 +0 +1
 +\[[ 0-9]+\] .dynamic +DYNAMIC .*
 +\[[ 0-9]+\] .got +PROGBITS .*
 +\[[ 0-9]+\] .symtab .*
 +\[[ 0-9]+\] .strtab .*
 +\[[ 0-9]+\] .shstrtab .*
Key to Flags:
#...

Elf file type is EXEC \(Executable file\)
Entry point 0x[0-9a-f]+
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +PHDR +0x0+40 0x0+1000040 0x0+1000040 0x0+150 0x0+150 R +0x8
 +INTERP +0x0+190 0x0+1000190 0x0+1000190 0x0+f 0x0+f R +0x1
.*Requesting program interpreter.*
 +LOAD .* R E 0x1000
 +LOAD .* RW +0x1000
 +DYNAMIC .* RW +0x8
 +TLS .* 0x0+60 0x0+a0 R +0x20

 Section to Segment mapping:
 +Segment Sections...
 +00 *
 +01 +.interp *
 +02 +.interp .hash .dynsym .dynstr .rela.dyn .rela.plt .plt .text *
 +03 +.tdata .dynamic .got *
 +04 +.dynamic *
 +05 +.tdata .tbss *

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 4 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_390_TLS_TPOFF +0+ sG3 \+ 0
[0-9a-f ]+R_390_TLS_TPOFF +0+ sG2 \+ 0
[0-9a-f ]+R_390_TLS_TPOFF +0+ sG6 \+ 0
[0-9a-f ]+R_390_TLS_TPOFF +0+ sG1 \+ 0

Relocation section '.rela.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_390_JMP_SLOT[0-9a-f ]+__tls_get_offset \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND 
.* TLS +GLOBAL +DEFAULT +UND sG3
.* TLS +GLOBAL +DEFAULT +UND sG2
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_offset
.* TLS +GLOBAL +DEFAULT +UND sG6
.* TLS +GLOBAL +DEFAULT +UND sG1

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND 
.* SECTION +LOCAL +DEFAULT +1.*
.* SECTION +LOCAL +DEFAULT +2.*
.* SECTION +LOCAL +DEFAULT +3.*
.* SECTION +LOCAL +DEFAULT +4.*
.* SECTION +LOCAL +DEFAULT +5.*
.* SECTION +LOCAL +DEFAULT +6.*
.* SECTION +LOCAL +DEFAULT +7.*
.* SECTION +LOCAL +DEFAULT +8.*
.* SECTION +LOCAL +DEFAULT +9.*
.* SECTION +LOCAL +DEFAULT +10.*
.* SECTION +LOCAL +DEFAULT +11.*
.* SECTION +LOCAL +DEFAULT +12.*
.* FILE +LOCAL +DEFAULT +ABS .*
.* TLS +LOCAL +DEFAULT +9 sl1
.* TLS +LOCAL +DEFAULT +9 sl2
.* TLS +LOCAL +DEFAULT +9 sl3
.* TLS +LOCAL +DEFAULT +9 sl4
.* TLS +LOCAL +DEFAULT +9 sl5
.* TLS +LOCAL +DEFAULT +9 sl6
.* TLS +LOCAL +DEFAULT +9 sl7
.* TLS +LOCAL +DEFAULT +9 sl8
.* FILE +LOCAL +DEFAULT +ABS .*
.* TLS +LOCAL +DEFAULT +10 bl1
.* TLS +LOCAL +DEFAULT +10 bl2
.* TLS +LOCAL +DEFAULT +10 bl3
.* TLS +LOCAL +DEFAULT +10 bl4
.* TLS +LOCAL +DEFAULT +10 bl5
.* TLS +LOCAL +DEFAULT +10 bl6
.* TLS +LOCAL +DEFAULT +10 bl7
.* TLS +LOCAL +DEFAULT +10 bl8
.* FILE +LOCAL +DEFAULT +ABS .*
.* OBJECT +LOCAL +DEFAULT +11 _DYNAMIC
.* OBJECT +LOCAL +DEFAULT +12 _GLOBAL_OFFSET_TABLE_
.* TLS +GLOBAL +DEFAULT +UND sG3
.* TLS +GLOBAL +DEFAULT +9 sg8
.* TLS +GLOBAL +DEFAULT +10 bg8
.* TLS +GLOBAL +DEFAULT +10 bg6
.* TLS +GLOBAL +DEFAULT +10 bg3
.* TLS +GLOBAL +DEFAULT +9 sg3
.* TLS +GLOBAL +HIDDEN +9 sh3
.* TLS +GLOBAL +DEFAULT +UND sG2
.* TLS +GLOBAL +DEFAULT +9 sg4
.* TLS +GLOBAL +DEFAULT +9 sg5
.* TLS +GLOBAL +DEFAULT +10 bg5
.* TLS +GLOBAL +HIDDEN +9 sh7
.* TLS +GLOBAL +HIDDEN +9 sh8
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_offset
.* TLS +GLOBAL +DEFAULT +9 sg1
.* FUNC +GLOBAL +DEFAULT +8 _start
.* TLS +GLOBAL +HIDDEN +9 sh4
.* TLS +GLOBAL +DEFAULT +10 bg7
.* TLS +GLOBAL +HIDDEN +9 sh5
.* NOTYPE +GLOBAL +DEFAULT +12 __bss_start
.* TLS +GLOBAL +DEFAULT +UND sG6
.* FUNC +GLOBAL +DEFAULT +8 fn2
.* TLS +GLOBAL +DEFAULT +9 sg2
.* TLS +GLOBAL +DEFAULT +UND sG1
.* TLS +GLOBAL +HIDDEN +9 sh1
.* TLS +GLOBAL +DEFAULT +9 sg6
.* TLS +GLOBAL +DEFAULT +9 sg7
.* NOTYPE +GLOBAL +DEFAULT +12 _edata
.* NOTYPE +GLOBAL +DEFAULT +12 _end
.* TLS +GLOBAL +HIDDEN +9 sh2
.* TLS +GLOBAL +HIDDEN +9 sh6
.* TLS +GLOBAL +DEFAULT +10 bg2
.* TLS +GLOBAL +DEFAULT +10 bg1
.* TLS +GLOBAL +DEFAULT +10 bg4
