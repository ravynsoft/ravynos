#as: 
#objdump: -d -j .text.1 -j .text.2
#name: long jump relaxation

.*: +file format .*xtensa.*

Disassembly of section \.text\.1:

00000000 <\.text\.1>:
# Skip instructions to load a8 since they will vary depending on whether
# the Xtensa configuration uses L32R or Const16.
#...
  .*:	.*	jx	a8
Disassembly of section \.text\.2:

00000000 <\.text\.2>:
   0:	.*	j	.*
#...
