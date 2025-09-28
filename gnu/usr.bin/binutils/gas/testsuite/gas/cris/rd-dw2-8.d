#readelf: -wl
#source: brokw-2.s
#as: --em=criself --gdwarf2

# Simple broken word, table with two labels.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 2 to 3
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x6 and Line by 5 to 8
  \[0x.*\]  Advance Line by 9 to 17
  \[0x.*\]  Special opcode .*: advance Address by 20 to 0x1a and Line by 0 to 17
  \[0x.*\]  Advance PC by 32768 to 0x801a
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x801a and Line by 4 to 21
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x801c and Line by 1 to 22
  \[0x.*\]  Advance PC by 2 to 0x801e
  \[0x.*\]  Extended opcode 1: End of Sequence
