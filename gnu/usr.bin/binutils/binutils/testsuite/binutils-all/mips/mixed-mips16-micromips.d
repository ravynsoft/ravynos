#PROG: objcopy
#objdump: -drt
#name: Mixed MIPS16 and microMIPS disassembly

# Test mixed-mode compressed disassembly.

.*: +file format .*mips.*

SYMBOL TABLE:
#...
[0-9a-f]+ g +F +\.text	0+00000c 0xf0 foo
#...
[0-9a-f]+ g +F +\.text	0+00000c 0x80 bar

Disassembly of section \.text:
[0-9a-f]+ <foo>:
 +[0-9a-f]+:	b202      	lw	v0,8 <\.foo\.data>
 +[0-9a-f]+:	9a60      	lw	v1,0\(v0\)
 +[0-9a-f]+:	eb00      	jr	v1
 +[0-9a-f]+:	653b      	move	t9,v1

[0-9a-f]+ <\.foo\.data>:
 +[0-9a-f]+:	4040 4040 0000 0000                         @@@@\.\.\.\.

[0-9a-f]+ <bar>:
 +[0-9a-f]+:	41a3 0000 	lui	v1,0x0
 +[0-9a-f]+:	ff23 0000 	lw	t9,0\(v1\)
 +[0-9a-f]+:	45b9      	jrc	t9
 +[0-9a-f]+:	0c00      	nop
	\.\.\.
