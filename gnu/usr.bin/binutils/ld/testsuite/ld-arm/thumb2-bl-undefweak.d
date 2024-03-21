#source: thumb2-bl-undefweak.s
#as:
#target: [check_shared_lib_support]
#ld: -shared
#objdump: -dr
#...
Disassembly of section .text:

.* <foo>:
 +[0-9a-f]+:	.... .... 	bl.	[0-9a-f]+ <bar@plt>
