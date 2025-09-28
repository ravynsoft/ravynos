#as:
#readelf: -x.rodata -wL
#name: DWARF2 13
# The riscv targets do not support the subtraction of symbols.
#xfail: riscv*-*

Hex dump of section '\.rodata':
  0x00000000 01 *.*

Contents of the \.debug_line section:

CU: dwarf2-13\.c:
File name +Line number +Starting address +View +Stmt
dwarf2-13\.c +1 +0x8 +x
dwarf2-13\.c +2 +0x10 +x
dwarf2-13\.c +3 +0x10 +1 +x
dwarf2-13\.c +- +0x18
