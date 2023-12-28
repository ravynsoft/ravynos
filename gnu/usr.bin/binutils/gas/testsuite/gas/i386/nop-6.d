#as: -march=i686+nop --generate-missing-build-notes=no
#objdump: -drw
#name: i386 .nops 6

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	0f 1f 40 00          	nopl   0x0\(%eax\)

Disassembly of section .altinstr_replacement:

0+ <.altinstr_replacement>:
 +[a-f0-9]+:	75 fe                	jne    0 <_start>
 +[a-f0-9]+:	89 c4                	mov    %eax,%esp
#pass
