#as: --EB
#source: reloc-data.s
#source: reloc-insn-external.s
#objdump: -dr
#ld: -Tdata=0x20 -EB
#name: reloc insn external BE

.*: +file format .*bpfbe

Disassembly of section .text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	18 10 00 00 00 00 00 28[ 	]*lddw %r1,0x28
 *[0-9a-f]+:	00 00 00 00 00 00 00 00[ 	]*
