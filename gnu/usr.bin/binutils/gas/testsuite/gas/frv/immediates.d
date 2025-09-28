#as:
#objdump: -d
#name: Parsing immediate values

.*: +file format .*

Disassembly of section \.text:

00000000 <foo>:
   0:[ 	]+86 fc e0 00[ 	]+setlos 0xff+e000,gr3
   4:[ 	]+08 f8 3f ff[ 	]+sethi.p 0x3fff,gr4

