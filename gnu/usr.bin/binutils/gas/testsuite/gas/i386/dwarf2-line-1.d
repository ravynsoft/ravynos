#as: -gdwarf-2
#readelf: -wl
#name: DWARF .debug_line 1

Raw dump of debug contents of section \.z?debug_line:

  Offset:                      (0x)?0
  Length:                      .*
  DWARF Version:               2
  Prologue Length:             .*
  Minimum Instruction Length:  1
  Initial value of 'is_stmt':  1
  Line Base:                   -5
  Line Range:                  14
  Opcode Base:                 10

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

 The Directory Table \(offset 0x.*\):
  .*

 The File Name Table \(offset 0x.*\):
  Entry	Dir	Time	Size	Name
  1	1	0	0	dwarf2-line-1.s

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode 13: advance Address by 0 to (0x)?0 and Line by 8 to 9
  \[0x.*\]  Special opcode 20: advance Address by 1 to 0x1 and Line by 1 to 10
  \[0x.*\]  Advance PC by 1 to 0x2
  \[0x.*\]  Extended opcode 1: End of Sequence


