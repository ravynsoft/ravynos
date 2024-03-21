#objdump: -r
#name: macro test 2
# darwin(mach-o) reverses the order of relocs.
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
#pass
