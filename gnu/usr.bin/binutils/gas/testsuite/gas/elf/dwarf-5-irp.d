#as: --gdwarf-5
#name: line number entries for section changes inside .irp
#readelf: -W -wlrR
# The am33 cr16 crx ft32 mn10* msp430 nds32* and rl78 targets do not evaluate the subtraction of symbols at assembly time.
# The bfin target does not allow .subsection with an equated symbol as operand.
# The d30v target emits sufficiently different debug info, apparently also covering padding it inserts.
# The riscv targets do not support the subtraction of symbols.
# The loongarch targets do not support the subtraction of symbols.
#xfail: am33*-* bfin-* cr16-* crx-* d30v-* ft32-* loongarch*-* mn10*-* msp430-* nds32*-* riscv*-* rl78-*

Raw dump of debug contents .*
#...
 Line Number Statements:
.*Extended opcode 2: .*
.*Special opcode .* and Line by 2 to 3
.*Set File Name to entry 2 .*
.*Advance Line by 15 to 18
.*Special opcode .* and Line by 0 to 18
.*Special opcode .* and Line by 1 to 19
.*Special opcode .* and Line by -1 to 18
.*Special opcode .* and Line by 1 to 19
.*Special opcode .* and Line by -1 to 18
.*Special opcode .* and Line by 1 to 19
.*Set File Name to entry 3 .*
.*Advance Line by 9 to 28
.*Special opcode .* and Line by 0 to 28
.*Special opcode .* and Line by 1 to 29
.*Special opcode .* and Line by -1 to 28
.*Special opcode .* and Line by 1 to 29
.*Special opcode .* and Line by -1 to 28
.*Special opcode .* and Line by 1 to 29
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Set File Name to entry 4 .*
.*Extended opcode 2: .*
.*Special opcode .* and Line by 8 to 9
.*Special opcode .* and Line by 1 to 10
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Set File Name to entry 4 .*
.*Extended opcode 2: .*
.*Special opcode .* and Line by 8 to 9
.*Special opcode .* and Line by 1 to 10
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Set File Name to entry 4 .*
.*Extended opcode 2: .*
.*Special opcode .* and Line by 8 to 9
.*Special opcode .* and Line by 1 to 10
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Set File Name to entry 4 .*
.*Extended opcode 2: .*
.*Advance Line by 35 to 36
.*Copy
.*Special opcode .* and Line by 1 to 37
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Set File Name to entry 4 .*
.*Extended opcode 2: .*
.*Advance Line by 35 to 36
.*Copy
.*Special opcode .* and Line by 1 to 37
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Extended opcode 2: .*
.*Advance Line by 41 to 42
.*Copy
.*Special opcode .* and Line by 1 to 43
.*Advance PC by .*
.*Extended opcode 1: End of Sequence

.*Extended opcode 2: .*
.*Advance Line by 41 to 42
.*Copy
.*Special opcode .* and Line by 1 to 43
.*Advance PC by .*
.*Extended opcode 1: End of Sequence


Contents of the \.debug_aranges section:

  Length: .*
  Version: +2
  Offset into \.debug_info: .*
  Pointer Size: +[248]
  Segment Size: +0

    Address +Length
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ [0-9a-f]+ ?
    0+ 0+ ?

Contents of the \.debug_rnglists section:
#...
    Offset +Begin +End
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ 0+ [0-9a-f]+ ?
    [0-9a-f]+ <End of list>

#pass
