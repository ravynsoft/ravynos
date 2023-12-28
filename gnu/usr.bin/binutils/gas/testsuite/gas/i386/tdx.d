#as:
#objdump: -dw
#name: TDX insns
#source: tdx.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 01 cc +	tdcall
#pass
