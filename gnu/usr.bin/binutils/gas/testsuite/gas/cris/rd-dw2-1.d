#readelf: -wl
#source: addi.s
#as: --em=criself --gdwarf2
# A most simple instruction sequence.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 4 to 5
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x2 and Line by 1 to 6
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x4 and Line by 1 to 7
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x6 and Line by 1 to 8
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x8 and Line by 1 to 9
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0xa and Line by 1 to 10
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0xc and Line by 1 to 11
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0xe and Line by 1 to 12
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x10 and Line by 1 to 13
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x12 and Line by 1 to 14
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x14 and Line by 1 to 15
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x16 and Line by 1 to 16
  \[0x.*\]  Advance PC by 2 to 0x18
  \[0x.*\]  Extended opcode 1: End of Sequence
