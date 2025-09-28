#as: --EB
#source: exit.s
#objdump: -dr
#name: eBPF EXIT instruction, big endian

.*: +file format .*bpf.*

Disassembly of section .text:

0+ <.text>:
   0:	95 00 00 00 00 00 00 00 	exit