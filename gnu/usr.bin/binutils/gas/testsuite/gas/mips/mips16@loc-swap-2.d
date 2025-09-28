#DUMPPROG: readelf
#readelf: -wl
#name: MIPS DWARF-2 location information with branch swapping (2)
#as: -32
#source: loc-swap-2.s

Raw dump of debug contents of section .debug_line:

  Offset:                      (0x)?0
  Length:                      60
  DWARF Version:               3
  Prologue Length:             35
  Minimum Instruction Length:  1
  Initial value of 'is_stmt':  1
  Line Base:                   -5
  Line Range:                  14
  Opcode Base:                 13

 Opcodes:
  Opcode 1 has 0 args
  Opcode 2 has 1 arg
  Opcode 3 has 1 arg
  Opcode 4 has 1 arg
  Opcode 5 has 1 arg
  Opcode 6 has 0 args
  Opcode 7 has 0 args
  Opcode 8 has 0 args
  Opcode 9 has 1 arg
  Opcode 10 has 0 args
  Opcode 11 has 0 args
  Opcode 12 has 1 arg

 The Directory Table is empty.

 The File Name Table \(offset 0x.*\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	loc-swap-2.s

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to 0x1
  \[0x.*\]  Special opcode 11: advance Address by 0 to 0x1 and Line by 6 to 7
  \[0x.*\]  Special opcode 7: advance Address by 0 to 0x1 and Line by 2 to 9 \(view 1\)
  \[0x.*\]  Special opcode 6: advance Address by 0 to 0x1 and Line by 1 to 10 \(view 2\)
  \[0x.*\]  Special opcode 64: advance Address by 4 to 0x5 and Line by 3 to 13
  \[0x.*\]  Special opcode 34: advance Address by 2 to 0x7 and Line by 1 to 14
  \[0x.*\]  Special opcode 6: advance Address by 0 to 0x7 and Line by 1 to 15 \(view 1\)
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0x9 and Line by 2 to 17
  \[0x.*\]  Advance PC by 1 to 0xa
  \[0x.*\]  Extended opcode 1: End of Sequence
