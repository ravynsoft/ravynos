#objdump: -r
#name: macro rept
#darwin (mach-o) reverses relocs.
#xfail: *-*-darwin* nds32*-*-*

.*: +file format .*

RELOCATION RECORDS FOR .*
OFFSET[ 	]+TYPE[ 	]+VALUE.*
0+00[ 	]+[a-zA-Z0-9_]+[ 	]+foo1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+04[ 	]+[a-zA-Z0-9_]+[ 	]+foo1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+08[ 	]+[a-zA-Z0-9_]+[ 	]+foo1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
#pass
