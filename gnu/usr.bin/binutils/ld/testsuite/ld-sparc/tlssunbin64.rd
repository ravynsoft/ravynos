#source: tlssunbin64.s
#as: --64
#ld: -shared -melf64_sparc tmpdir/libtlslib64.so tmpdir/tlssunbinpic64.o
#readelf: -WSsrl
#target: sparc*-*-*

.*

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 00 +0 +0 +0
 +\[[ 0-9]+\] .interp +.*
 +\[[ 0-9]+\] .hash +.*
 +\[[ 0-9]+\] .dynsym +.*
 +\[[ 0-9]+\] .dynstr +.*
 +\[[ 0-9]+\] .gnu.version +.*
 +\[[ 0-9]+\] .gnu.version_r +.*
 +\[[ 0-9]+\] .rela.dyn +.*
 +\[[ 0-9]+\] .text +PROGBITS +0+101000 0+1000 0+11a4 00 +AX +0 +0 4096
 +\[[ 0-9]+\] .tdata +PROGBITS +0+2021a4 0+21a4 0+0060 00 WAT +0 +0 +4
 +\[[ 0-9]+\] .tbss +NOBITS +0+202204 0+2204 0+40 00 WAT +0 +0 +4
 +\[[ 0-9]+\] .dynamic +DYNAMIC +0+202208 0+2208 0+130 10 +WA +4 +0 +8
 +\[[ 0-9]+\] .got +PROGBITS +0+202338 0+2338 0+28 08 +WA +0 +0 +8
 +\[[ 0-9]+\] .symtab +.*
 +\[[ 0-9]+\] .strtab +.*
 +\[[ 0-9]+\] .shstrtab +.*
#...

Elf file type is EXEC \(Executable file\)
Entry point 0x102000
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +PHDR +0x0+40 0x0+100040 0x0+100040 0x0+150 0x0+150 R +0x8
 +INTERP +0x0+190 0x0+100190 0x0+100190 0x0+19 0x0+19 R +0x1
.*Requesting program interpreter.*
 +LOAD .* R E 0x100000
 +LOAD .* RW +0x100000
 +DYNAMIC .* RW +0x8
 +TLS .* 0x0+60 0x0+a0 R +0x4
#...

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 4 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_SPARC_TLS_TPOFF64 +0+ +sG5 \+ 0
[0-9a-f ]+R_SPARC_TLS_TPOFF64 +0+ +sG2 \+ 0
[0-9a-f ]+R_SPARC_TLS_TPOFF64 +0+ +sG6 \+ 0
[0-9a-f ]+R_SPARC_TLS_TPOFF64 +0+ +sG1 \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND *
.* TLS +GLOBAL +DEFAULT +UND sG5
.* TLS +GLOBAL +DEFAULT +UND sG2
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr@SUNWprivate_1.1 \(2\)
.* TLS +GLOBAL +DEFAULT +UND sG6
.* TLS +GLOBAL +DEFAULT +UND sG1

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
.* NOTYPE +LOCAL +DEFAULT +UND *
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
.* OBJECT +LOCAL +DEFAULT +12 _PROCEDURE_LINKAGE_TABLE_
.* OBJECT +LOCAL +DEFAULT +12 _GLOBAL_OFFSET_TABLE_
.* TLS +GLOBAL +DEFAULT +9 sg8
.* TLS +GLOBAL +DEFAULT +10 bg8
.* TLS +GLOBAL +DEFAULT +10 bg6
.* TLS +GLOBAL +DEFAULT +UND sG5
.* TLS +GLOBAL +DEFAULT +10 bg3
.* TLS +GLOBAL +DEFAULT +9 sg3
.* TLS +GLOBAL +HIDDEN +9 sh3
.* TLS +GLOBAL +DEFAULT +UND sG2
.* TLS +GLOBAL +DEFAULT +9 sg4
.* TLS +GLOBAL +DEFAULT +9 sg5
.* TLS +GLOBAL +DEFAULT +10 bg5
.* TLS +GLOBAL +HIDDEN +9 sh7
.* TLS +GLOBAL +HIDDEN +9 sh8
.* TLS +GLOBAL +DEFAULT +9 sg1
.* FUNC +GLOBAL +DEFAULT +8 _start
.* TLS +GLOBAL +HIDDEN +9 sh4
.* TLS +GLOBAL +DEFAULT +10 bg7
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr@SUNWprivate_1.1
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
