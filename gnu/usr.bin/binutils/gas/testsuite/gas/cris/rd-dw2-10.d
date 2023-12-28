#readelf: -wl
#source: continue.s
#as: --em=criself --gdwarf2

# Continued line.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 6 to 7
  \[0x.*\]  Advance PC by 4 to 0x4
  \[0x.*\]  Extended opcode 1: End of Sequence
