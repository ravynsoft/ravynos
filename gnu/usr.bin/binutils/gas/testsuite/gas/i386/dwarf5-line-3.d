#as: -g -gdwarf-5
#readelf: -wl
#name: DWARF5 .debug_line 3

Raw dump of debug contents of section \.z?debug_line:

  Offset:                      (0x)?0
  Length:                      .*
  DWARF Version:               5
  Address size \(bytes\):        .*
  Segment selector \(bytes\):    0
  Prologue Length:             .*
  Minimum Instruction Length:  1
  Maximum Ops per Instruction: 1
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

 The Directory Table \(offset 0x.*, lines 1, columns 1\):
  Entry	Name
  0	\(indirect line string, offset: 0.*\): .*

 The File Name Table \(offset 0x.*, lines 2, columns 2\):
  Entry	Dir	Name
  0	0	\(indirect line string, offset: 0x.*\): dwarf5-line-3.S
  1	0	\(indirect line string, offset: 0x.*\): dwarf5-line-3.S

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode 7: advance Address by 0 to (0x)?0 and Line by 2 to 3
  \[0x.*\]  Advance PC by 1 to 0x1
  \[0x.*\]  Extended opcode 1: End of Sequence


