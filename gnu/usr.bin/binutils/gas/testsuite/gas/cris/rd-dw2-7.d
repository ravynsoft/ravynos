#readelf: -wl
#source: brokw-1.s
#as: --em=criself --gdwarf2

# Most simple broken word.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 2 to 3
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x4 and Line by 4 to 7
  \[0x.*\]  Special opcode .*: advance Address by 14 to 0x12 and Line by 8 to 15
  \[0x.*\]  Advance PC by 32768 to 0x8012
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8012 and Line by 4 to 19
  \[0x.*\]  Advance PC by 2 to 0x8014
  \[0x.*\]  Extended opcode 1: End of Sequence
