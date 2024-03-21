#as:
#readelf: -wlL
#name: DWARF2 6
# These targets either do not support or do not evaluate the subtraction of symbols at assembly time.
#xfail: cr16-* crx-* riscv*-*

Raw dump of debug contents of section .debug_line:

  Offset:                      (0x)?0
  Length:                      84
  DWARF Version:               2
  Prologue Length:             36
  Minimum Instruction Length:  1
  Initial value of 'is_stmt':  1
  Line Base:                   1
  Line Range:                  1
  Opcode Base:                 16

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
  Opcode 13 has 0 args
  Opcode 14 has 0 args
  Opcode 15 has 0 args

 The Directory Table is empty.

 The File Name Table \(offset 0x1f\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	dwarf2-6\.c


 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Copy
  \[0x.*\]  Copy \(view 1\)
  \[0x.*\]  Extended opcode 2: set Address to 0x1
  \[0x.*\]  Copy
  \[0x.*\]  Advance PC by 0 to 0x1
  \[0x.*\]  Copy \(view 1\)
  \[0x.*\]  Advance PC by 1 to 0x2
  \[0x.*\]  Copy
  \[0x.*\]  Advance PC by fixed size amount 1 to 0x3
  \[0x.*\]  Copy \(view 1\)
  \[0x.*\]  Special opcode 0: advance Address by 0 to 0x3 and Line by 1 to 2 \(view 2\)
  \[0x.*\]  Special opcode 1: advance Address by 1 to 0x4 and Line by 1 to 3
  \[0x.*\]  Copy \(view 1\)
  \[0x.*\]  Advance PC by constant 239 to 0xf3
  \[0x.*\]  Copy
  \[0x.*\]  Extended opcode 2: set Address to 0x100
  \[0x.*\]  Extended opcode 1: End of Sequence


Contents of the \.debug_line section:

CU: dwarf2-6\.c:
File name  *Line number  *Starting address  *View +Stmt
dwarf2-6\.c  *1  *0 +x
dwarf2-6\.c  *1  *0  *1 +x
dwarf2-6\.c  *1  *0x1 +x
dwarf2-6\.c  *1  *0x1  *1 +x
dwarf2-6\.c  *1  *0x2 +x
dwarf2-6\.c  *1  *0x3  *1 +x
dwarf2-6\.c  *2  *0x3  *2 +x
dwarf2-6\.c  *3  *0x4 +x
dwarf2-6\.c  *3  *0x4  *1 +x
dwarf2-6\.c  *3  *0xf3 +x
dwarf2-6\.c  *-  *0x100
