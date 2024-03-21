#as: --gdwarf-5
#name: line number entries for .macro expansions
#readelf: -W -wl
# The am33 cr16 crx ft32 mn10* msp430 nds32* and rl78 targets do not evaluate the subtraction of symbols at assembly time.
# The d30v target emits sufficiently different debug info, apparently also covering padding it inserts.
# The riscv targets do not support the subtraction of symbols.
#xfail: am33*-* cr16-* crx-* d30v-* ft32-* loongarch*-* mn10*-* msp430-* nds32*-* riscv*-* rl78-*

Raw dump of debug contents .*
#...
 Line Number Statements:
.*Extended opcode 2: .*
.*Advance Line by 10017 to 10018
.*(Copy|Special opcode .* Address by 0 .* and Line by 0 to 10018)
.*Special opcode .* and Line by 1 to 10019
.*Special opcode .* and Line by 1 to 10020
.*Set File Name to entry 2 .*
.*Advance Line by -10012 to 8
.*Special opcode .* and Line by 0 to 8
.*Set File Name to entry 1 .*
.*Advance Line by 10018 to 10026
.*(Advance PC by .*|Special opcode .* and Line by 0 to 10026)
#...
.*Special opcode .* and Line by 1 to 10027
.*Special opcode .* and Line by 1 to 10028
.*Advance PC by .*
.*Extended opcode 1: End of Sequence
