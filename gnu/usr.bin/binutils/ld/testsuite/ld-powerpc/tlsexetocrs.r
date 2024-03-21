#source: tlslib.s
#source: tlstoc.s
#as: -a64
#ld:
#readelf: -WSsrl
#target: powerpc64*-*-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES Flg Lk Inf Al
 +\[[ 0-9]+\] +NULL +0+ 0+ 0+ 0+ +0 +0 +0
 +\[[ 0-9]+\] \.interp +.*
 +\[[ 0-9]+\] \.hash +.*
 +\[[ 0-9]+\] \.dynsym +.*
 +\[[ 0-9]+\] \.dynstr +.*
 +\[[ 0-9]+\] \.rela\.dyn +.*
 +\[[ 0-9]+\] \.rela\.plt +.*
 +\[[ 0-9]+\] \.text +PROGBITS .* 0+124 0+ +AX +0 +0 +32
 +\[[ 0-9]+\] \.tdata +PROGBITS .* 0+38 0+ WAT +0 +0 +8
 +\[[ 0-9]+\] \.tbss +NOBITS .* 0+38 0+ WAT +0 +0 +8
 +\[[ 0-9]+\] \.dynamic +DYNAMIC .* 0+160 10 +WA +4 +0 +8
 +\[[ 0-9]+\] \.opd .*
 +\[[ 0-9]+\] \.got +PROGBITS .* 0+58 08 +WA +0 +0 +256
 +\[[ 0-9]+\] \.plt +.*
 +\[[ 0-9]+\] \.symtab +.*
 +\[[ 0-9]+\] \.strtab +.*
 +\[[ 0-9]+\] \.shstrtab +.*
#...

Elf file type is EXEC \(Executable file\)
Entry point .*
There are [0-9]+ program headers, starting at offset [0-9]+

Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg Align
 +PHDR +0x0+40 0x0+10000040 0x0+10000040 0x0+150 0x0+150 R +0x8
 +INTERP +0x0+190 0x0+10000190 0x0+10000190 0x0+11 0x0+11 R +0x1
 +\[Requesting program interpreter: .*\]
 +LOAD .* R E 0x10000
 +LOAD .* RW +0x10000
 +DYNAMIC .* RW +0x8
 +TLS .* 0x0+38 0x0+70 R +0x8

 Section to Segment mapping:
 +Segment Sections\.\.\.
 +0+ +
 +01 +\.interp *
 +02 +\.interp \.hash \.dynsym \.dynstr \.rela\.dyn \.rela\.plt \.text *
 +03 +\.tdata \.dynamic \.opd \.got \.plt *
 +04 +\.dynamic *
 +05 +\.tdata \.tbss *

Relocation section '\.rela\.dyn' at offset .* contains 3 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_PPC64_DTPMOD64 +0+ gd \+ 0
[0-9a-f ]+R_PPC64_DTPREL64 +0+ gd \+ 0
[0-9a-f ]+R_PPC64_DTPMOD64 +0+ ld \+ 0

Relocation section '\.rela\.plt' at offset .* contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f ]+R_PPC64_JMP_SLOT +0+ __tls_get_addr_opt \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND *
.* TLS +GLOBAL +DEFAULT +UND gd
.* TLS +GLOBAL +DEFAULT +UND ld
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr_opt

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* NOTYPE +LOCAL +DEFAULT +UND *
.* SECTION +LOCAL +DEFAULT +1 \.interp
.* SECTION +LOCAL +DEFAULT +2 \.hash
.* SECTION +LOCAL +DEFAULT +3 \.dynsym
.* SECTION +LOCAL +DEFAULT +4 \.dynstr
.* SECTION +LOCAL +DEFAULT +5 \.rela\.dyn
.* SECTION +LOCAL +DEFAULT +6 \.rela\.plt
.* SECTION +LOCAL +DEFAULT +7 \.text
.* SECTION +LOCAL +DEFAULT +8 \.tdata
.* SECTION +LOCAL +DEFAULT +9 \.tbss
.* SECTION +LOCAL +DEFAULT +10 \.dynamic
.* SECTION +LOCAL +DEFAULT +11 \.opd
.* SECTION +LOCAL +DEFAULT +12 \.got
.* SECTION +LOCAL +DEFAULT +13 \.plt
.* FILE +LOCAL +DEFAULT +ABS .*
.* TLS +LOCAL +DEFAULT +8 gd4
.* TLS +LOCAL +DEFAULT +8 ld4
.* TLS +LOCAL +DEFAULT +8 ld5
.* TLS +LOCAL +DEFAULT +8 ld6
.* TLS +LOCAL +DEFAULT +8 ie4
.* TLS +LOCAL +DEFAULT +8 le4
.* TLS +LOCAL +DEFAULT +8 le5
.* NOTYPE +LOCAL +DEFAULT +12 \.Lie0
.* FILE +LOCAL +DEFAULT +ABS 
.* OBJECT +LOCAL +DEFAULT +10 _DYNAMIC
.* NOTYPE +LOCAL +DEFAULT +7 __glink_PLTresolve
.* NOTYPE +LOCAL +DEFAULT +7 .*\.plt_call\.__tls_get_addr(|_opt)
.* TLS +GLOBAL +DEFAULT +UND gd
.* TLS +GLOBAL +DEFAULT +9 le0
.* TLS +GLOBAL +DEFAULT +9 ld0
.* TLS +GLOBAL +DEFAULT +9 le1
.* TLS +GLOBAL +DEFAULT +UND ld
.* FUNC +GLOBAL +DEFAULT +11 _start
.* TLS +GLOBAL +DEFAULT +9 ld2
.* TLS +GLOBAL +DEFAULT +9 ld1
.* NOTYPE +GLOBAL +DEFAULT +13 __bss_start
.* FUNC +GLOBAL +DEFAULT +UND __tls_get_addr_opt
.* NOTYPE +GLOBAL +DEFAULT +13 _edata
.* NOTYPE +GLOBAL +DEFAULT +13 _end
.* TLS +GLOBAL +DEFAULT +9 gd0
.* TLS +GLOBAL +DEFAULT +9 ie0
