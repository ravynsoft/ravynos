#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+ <pr29483>:
 +[a-f0-9]+:	65 62 62 7d 97 a0 94 ff 20 20 20 ae 	vpscatterdd %xmm26,%gs:-0x51dfdfe0\(%rdi,%xmm23,8\)\{bad\}\{%k7\}\{z\}/\(bad\)
#pass
