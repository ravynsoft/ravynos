#as: -m68030
#objdump: --architecture=m68k:68030 -d
#name: pmove

# Test handling of the 68030/68851 pmove instructions.

.*: +file format .*

Disassembly of section .text:

0+ <.*>:
[ 0-9a-f]+:	f010 6200      	pmove %psr,%a0@
[ 0-9a-f]+:	f011 6000      	pmove %a1@,%psr
[ 0-9a-f]+:	f012 6600      	pmove %pcsr,%a2@
[ 0-9a-f]+:	f013 7200      	pmove %bad0,%a3@
[ 0-9a-f]+:	f014 7004      	pmove %a4@,%bad1
