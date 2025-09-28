#as: -gdwarf-5
#readelf: -W -wL
#name: DWARF5 .loc 0
# The am33 cr16 crx ft32 mn10 msp430 nds32 and rl78 targets do not evaluate the subtraction of symbols at assembly time.
# The riscv targets do not support the subtraction of symbols.
# The loongarch targets do not support the subtraction of symbols.
#xfail: am3*-* cr16-* crx-* ft32*-* loongarch*-* mn10*-* msp430-* nds32*-* riscv*-* rl78-*

Contents of the \.debug_line section:

CU: .*/gas/testsuite/foo\.c:
File name +Line number +Starting address +View +Stmt

.*/gas/testsuite/foo.c:
foo\.c +1 +0x8 +x
foo\.c +2 +0x10 +x
foo\.c +- +0x10
