#name: ARM Integer division instructions
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> e735f819 	udiv	r5, r9, r8
0+004 <[^>]*> e739f715 	udiv	r9, r5, r7
0+008 <[^>]*> e710f010 	sdiv	r0, r0, r0
