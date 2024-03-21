#objdump: -dr --prefix-addresses --show-raw-insn
#name: Backslash-at for ARM

.*:     file format .*arm.*

Disassembly of section .text:
0+000 <.*>.*(615c|5c61).*
0+004 <foo> e3a00000 	mov	r0, #0
0+008 <foo\+0x4> e3a00000 	mov	r0, #0
0+00c <foo\+0x8> e3a00000 	mov	r0, #0
0+010 <foo\+0xc> e3a00001 	mov	r0, #1
0+014 <foo\+0x10> e3a00001 	mov	r0, #1
0+018 <foo\+0x14> e3a00001 	mov	r0, #1
0+01c <foo\+0x18> e3a00002 	mov	r0, #2
0+020 <foo\+0x1c> e3a00002 	mov	r0, #2
0+024 <foo\+0x20> e3a00002 	mov	r0, #2
#...
