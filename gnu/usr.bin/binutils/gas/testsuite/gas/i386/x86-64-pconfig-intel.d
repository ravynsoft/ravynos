#objdump: -dwMintel
#name: i386 PCONFIG (Intel disassembly)
#source: pconfig.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*0f 01 c5[ 	]*pconfig
#pass
