#DUMPPROG: readelf
#readelf: -wl
#as: -32
#name: MIPS DWARF-2 location information with branch swapping (3)
#...
 Line Number Statements:
.*  Set prologue_end to true
.*  Extended opcode 2: set Address to (0x)?[01]
.*  Copy
#------------------------------------------------------------------------
# There used to be a bogus:
#   Set prologue_end to true
# here
#------------------------------------------------------------------------
.*  Special opcode 6: advance Address by 0 to (0x)?[01] and Line by 1 to 2 \(view 1\)
.*  Advance PC by .*
.*  Extended opcode 1: End of Sequence
