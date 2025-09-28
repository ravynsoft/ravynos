#as: --gdwarf-5 -I${srcdir}/$subdir
#name: line number entries for expansions of .macro coming from .include
#readelf: -W -wl
# The am33 cr16 crx ft32 mn10* msp430 nds32* and rl78 targets do not evaluate the subtraction of symbols at assembly time.
# The d30v target emits sufficiently different debug info, apparently also covering padding it inserts.
# The riscv targets do not support the subtraction of symbols.
#xfail: am33*-* cr16-* crx-* d30v-* ft32-* loongarch*-* mn10*-* msp430-* nds32*-* riscv*-* rl78-*

Raw dump of debug contents .*
#...
 Line Number Statements:
.*Extended opcode 2: .*
.*Special opcode .* advance Address by 0 .* and Line by 2 to 3
.*Special opcode .* and Line by 1 to 4
.*Special opcode .* and Line by 1 to 5
.*Advance PC by .*
.*Extended opcode 1: End of Sequence
