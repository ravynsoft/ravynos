#as: --gdwarf-3
#readelf: -wl
#name: DWARF2 3
#xfail: ft32*-* h8300-*-*

Raw dump of debug contents of section \.z?debug_line:

  Offset:                      (0x)?0
  Length:                      41
  DWARF Version:               3
  Prologue Length:             35
  Minimum Instruction Length:  [0-9]*
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
  1	0	0	0	/beginwarn.c

 No Line Number Statements.

