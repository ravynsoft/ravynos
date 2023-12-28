#as:
#objdump: -dw
#name: x86_64 TDX insns
#source: x86-64-tdx.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 01 cc +	tdcall
[ 	]*[a-f0-9]+:	66 0f 01 cd +	seamret
[ 	]*[a-f0-9]+:	66 0f 01 ce +	seamops
[ 	]*[a-f0-9]+:	66 0f 01 cf +	seamcall
#pass
