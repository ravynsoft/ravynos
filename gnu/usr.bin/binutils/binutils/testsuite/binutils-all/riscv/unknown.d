#as: -march=rv32ic
#objdump: -d
# Test the disassembly of unknown instruction encodings, specifically,
# ensure that we generate a .insn directive.

#...
Disassembly of section \.text:

[0-9a-f]+ <\.text>:
   [0-9a-f]+:	0052018b          	\.insn	4, 0x0052018b
   [0-9a-f]+:	9c45                	\.insn	2, 0x9c45
