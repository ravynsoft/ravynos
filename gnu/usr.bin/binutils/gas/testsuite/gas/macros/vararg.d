#objdump: -r
#name: macro vararg
# darwin (mach-o) reverses relocs.
#xfail: *-*-darwin*

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
0+0c[ 	]+[a-zA-Z0-9_]+[ 	]+foo4
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+10[ 	]+[a-zA-Z0-9_]+[ 	]+foo5
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+14[ 	]+[a-zA-Z0-9_]+[ 	]+foo6
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
#pass
