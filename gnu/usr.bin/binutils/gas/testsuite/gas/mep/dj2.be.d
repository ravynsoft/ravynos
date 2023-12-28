#as: -EB
#objdump: -dr
#source: dj2.s
#name: dj2.be

.*: +file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	07 88       	sb \$7,\(\$8\)
   2:	05 98       	sb \$5,\(\$9\)
