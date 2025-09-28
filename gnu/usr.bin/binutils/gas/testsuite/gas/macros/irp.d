#objdump: -r
#name: macro irp
#darwin (mach-o) reverses relocs.
#xfail: *-*-darwin* nds32*-*-*

.*: +file format .*

RELOCATION RECORDS FOR .*
OFFSET[ 	]+TYPE[ 	]+VALUE.*
0+00[ 	]+[a-zA-Z0-9_]+[ 	]+foo1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+04[ 	]+[a-zA-Z0-9_]+[ 	]+foo2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+08[ 	]+[a-zA-Z0-9_]+[ 	]+foo3
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+0c[ 	]+[a-zA-Z0-9_]+[ 	]+bar1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+10[ 	]+[a-zA-Z0-9_]+[ 	]+bar2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+14[ 	]+[a-zA-Z0-9_]+[ 	]+bar3
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
#pass
