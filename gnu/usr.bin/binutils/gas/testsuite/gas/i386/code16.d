#objdump: -drw -mi8086
#name: i386 with .code16
#warning_output: code16.e

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
 +[a-f0-9]+:	f3 66 a5             	rep movsl %ds:\(%si\),%es:\(%di\)
 +[a-f0-9]+:	f3 66 a7             	repz cmpsl %es:\(%di\),%ds:\(%si\)
 +[a-f0-9]+:	66 f3 a5             	rep movsl %ds:\(%si\),%es:\(%di\)
 +[a-f0-9]+:	66 f3 a7             	repz cmpsl %es:\(%di\),%ds:\(%si\)
 +[a-f0-9]+:	0f 20 d1             	mov    %cr2,%ecx
 +[a-f0-9]+:	0f 22 d1             	mov    %ecx,%cr2
 +[a-f0-9]+:	0f 21 d1             	mov    %d[br]2,%ecx
 +[a-f0-9]+:	0f 23 d1             	mov    %ecx,%d[br]2
 +[a-f0-9]+:	0f 24 d1             	mov    %tr2,%ecx
 +[a-f0-9]+:	0f 26 d1             	mov    %ecx,%tr2
 +[a-f0-9]+:	66 0f c9             	bswap  %ecx
 +[a-f0-9]+:	66 f3 a5             	rep movsl %ds:\(%si\),%es:\(%di\)
 +[a-f0-9]+:	66 f3 a7             	repz cmpsl %es:\(%di\),%ds:\(%si\)
#pass
