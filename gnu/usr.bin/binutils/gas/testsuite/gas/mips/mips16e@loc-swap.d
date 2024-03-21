#DUMPPROG: readelf
#readelf: -wl
#name: MIPS DWARF-2 location information with branch swapping
#as: -32
#source: loc-swap.s

# Verify that DWARF-2 location information for instructions reordered
# into a branch delay slot is updated to point to the branch instead.

Raw dump of debug contents of section \.debug_line:

  Offset:                      (0x)?0
  Length:                      67
  DWARF Version:               3
  Prologue Length:             33
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

 The Directory Table is empty\.

 The File Name Table \(offset 0x.*\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	loc-swap\.s

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to 0x1
  \[0x.*\]  Special opcode 11: advance Address by 0 to 0x1 and Line by 6 to 7
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0x3 and Line by 2 to 9
  \[0x.*\]  Special opcode 36: advance Address by 2 to 0x5 and Line by 3 to 12
  \[0x.*\]  Special opcode 7: advance Address by 0 to 0x5 and Line by 2 to 14 \(view 1\)
  \[0x.*\]  Special opcode 64: advance Address by 4 to 0x9 and Line by 3 to 17
  \[0x.*\]  Special opcode 7: advance Address by 0 to 0x9 and Line by 2 to 19 \(view 1\)
  \[0x.*\]  Special opcode 64: advance Address by 4 to 0xd and Line by 3 to 22
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0xf and Line by 2 to 24
  \[0x.*\]  Special opcode 36: advance Address by 2 to 0x11 and Line by 3 to 27
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0x13 and Line by 2 to 29
  \[0x.*\]  Special opcode 36: advance Address by 2 to 0x15 and Line by 3 to 32
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0x17 and Line by 2 to 34
  \[0x.*\]  Special opcode 36: advance Address by 2 to 0x19 and Line by 3 to 37
  \[0x.*\]  Special opcode 7: advance Address by 0 to 0x19 and Line by 2 to 39 \(view 1\)
  \[0x.*\]  Special opcode 92: advance Address by 6 to 0x1f and Line by 3 to 42
  \[0x.*\]  Special opcode 35: advance Address by 2 to 0x21 and Line by 2 to 44
  \[0x.*\]  Advance PC by 31 to 0x40
  \[0x.*\]  Extended opcode 1: End of Sequence
