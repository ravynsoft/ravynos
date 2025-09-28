#as: --gdwarf-3
#readelf: -x.rodata -wlL
#name: DWARF2 5
# The am33 cr16 crx ft32 mn10 msp430 nds32 rl78 and rx targets do not evaluate the subtraction of symbols at assembly time.
# The mep target tries to relay code sections which breaks symbolic view computations.
# The riscv targets do not support the subtraction of symbols.
# The loongarch targets do not support the subtraction of symbols.
#xfail: am3*-* cr16-* crx-* ft32*-* loongarch*-* mep-* mn10*-* msp430-* nds32*-* riscv*-* rl78-* rx-*

Hex dump of section '\.rodata':
  0x00000000 01010201 010203 *.*

Raw dump of debug contents of section \.debug_line:

  Offset:                      (0x)?0
  Length:                      [0-9]*
  DWARF Version:               3
  Prologue Length:             [0-9]*
  Minimum Instruction Length:  [0-9]*
  Initial value of 'is_stmt':  1
  Line Base:                   [-0-9]*
  Line Range:                  [0-9]*
  Opcode Base:                 [0-9]*

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

 The File Name Table \(offset 0x1c\):
  Entry	Dir	Time	Size	Name
  1	0	0	0	dwarf2-5\.c

 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Copy
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to (0x)?0 and Line by 1 to 2 \(view 1\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 8 to 0x8 and Line by 1 to 3
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x8 and Line by 1 to 4 \(view 1\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x8 and Line by 1 to 5 \(view 2\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 8 to 0x10 and Line by 1 to 6
  \[0x.*\]  Special opcode [0-9]*: advance Address by 8 to 0x18 and Line by 1 to 7
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x18 and Line by 1 to 8 \(view 1\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 8 to 0x20 and Line by 1 to 9
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x20 and Line by 1 to 10 \(view 1\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x20 and Line by 1 to 11 \(view 2\)
  \[0x.*\]  Special opcode [0-9]*: advance Address by 0 to 0x20 and Line by 1 to 12 \(view 3\)
  \[0x.*\]  Advance PC by 8 to 0x28
  \[0x.*\]  Extended opcode 1: End of Sequence


Contents of the \.debug_line section:

CU: dwarf2-5\.c:
File name +Line number +Starting address +View +Stmt
dwarf2-5\.c +1 +0 +x
dwarf2-5\.c +2 +0 +1 +x
dwarf2-5\.c +3 +0x8 +x
dwarf2-5\.c +4 +0x8 +1 +x
dwarf2-5\.c +5 +0x8 +2 +x
dwarf2-5\.c +6 +0x10 +x
dwarf2-5\.c +7 +0x18 +x
dwarf2-5\.c +8 +0x18 +1 +x
dwarf2-5\.c +9 +0x20 +x
dwarf2-5\.c +10 +0x20 +1 +x
dwarf2-5\.c +11 +0x20 +2 +x
dwarf2-5\.c +12 +0x20 +3 +x
dwarf2-5\.c +- +0x28
