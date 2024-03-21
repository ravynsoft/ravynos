#as: --EB
#source: atomic.s
#source: atomic-pseudoc.s
#objdump: -dr
#name: eBPF atomic instructions, big endian

.*: +file format .*bpf.*

Disassembly of section .text:

0+ <.text>:
   0:	db 12 1e ef 00 00 00 00 	xadddw \[%r1\+0x1eef\],%r2
   8:	c3 12 1e ef 00 00 00 00 	xaddw \[%r1\+0x1eef\],%r2
