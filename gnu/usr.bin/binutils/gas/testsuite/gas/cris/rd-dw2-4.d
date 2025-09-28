#readelf: -wl
#source: branch-warn-2.s
#as: --em=criself --gdwarf2

# Simple branch-expansion, type 2.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 7 to 8
  \[0x.*\]  Advance PC by 32780 to 0x800c
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x800c and Line by 3 to 11
  \[0x.*\]  Advance PC by 2 to 0x800e
  \[0x.*\]  Extended opcode 1: End of Sequence
