#as: --EL
#objdump: -dr
#name: eBPF EXIT instruction

.*: +file format .*bpf.*

Disassembly of section .text:

0+ <.text>:
   0:	95 00 00 00 00 00 00 00 	exit