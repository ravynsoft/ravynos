#objdump: -r
#name: macro irp
#source: ../macros/irp.s

.*: +file format .*

RELOCATION RECORDS FOR .*
OFFSET[ 	]+TYPE[ 	]+VALUE.*
0+14[ 	]+[a-zA-Z0-9_]+[ 	]+bar3
0+10[ 	]+[a-zA-Z0-9_]+[ 	]+bar2
0+0c[ 	]+[a-zA-Z0-9_]+[ 	]+bar1
0+08[ 	]+[a-zA-Z0-9_]+[ 	]+foo3
0+04[ 	]+[a-zA-Z0-9_]+[ 	]+foo2
0+00[ 	]+[a-zA-Z0-9_]+[ 	]+foo1
