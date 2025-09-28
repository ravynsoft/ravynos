#readelf: -wl
#source: branch-warn-1.s
#as: --em=criself --gdwarf2

# Simple branch-expansion, type 1.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 7 to 8
  \[0x.*\]  Special opcode .*: advance Address by 12 to 0xc and Line by 1 to 9
  \[0x.*\]  Advance PC by 2 to 0xe
  \[0x.*\]  Extended opcode 1: End of Sequence
