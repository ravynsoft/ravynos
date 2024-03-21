#name: good .bss / .struct data allocation directives
#as: --defsym okay=1
#warning: Warning: zero assumed
#readelf: -sSW
#target: i?86-*-* x86_64-*-* ia64-*-* arm-*-* aarch64-*-*

There are [1-9][0-9]* section headers, starting at offset 0x[0-9a-f]*:

Section Headers:
#...
 *\[ [1-9]\] *\.bss        +NOBITS +0*0 +0[0-9a-f]* 0*(28|40) +0*0 +WA +0 +0 +32
 *\[ [1-9]\] *\.bss\.local +NOBITS +0*0 +0[0-9a-f]* 0*(28|40) +0*0 +WA +0 +0 +32
 *\[ [1-9]\] *\.private    +NOBITS +0*0 +0[0-9a-f]* 0*(28|40) +0*0 +WA +0 +0 +32
#...
Symbol table '\.symtab' contains [1-9][0-9]* entries:
#...
 *[0-9]*: 0*28 *0 NOTYPE *LOCAL *DEFAULT *[1-9] endof_bss
#...
 *[0-9]*: 0*28 *0 NOTYPE *LOCAL *DEFAULT *[1-9] endof_bss_local
#...
 *[0-9]*: 0*28 *0 NOTYPE *LOCAL *DEFAULT *[1-9] endof_private
#...
 *[0-9]*: 0*27 *0 NOTYPE *LOCAL *DEFAULT *ABS endof_struct
#pass
