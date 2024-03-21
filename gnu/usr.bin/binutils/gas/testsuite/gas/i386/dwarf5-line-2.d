#as: -gdwarf-5
#readelf: -wl
#name: DWARF5 .debug_line 2

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
  0	\(indirect line string, offset: 0.*\): .*/gas/testsuite

 The File Name Table \(offset 0x.*, lines 1, columns 3\):
  Entry	Dir	MD5				Name
  0	0 0xbbd69fc03ce253b2dbaab2522dd519ae	\(indirect line string, offset: 0x.*\): core.c

 No Line Number Statements\.
