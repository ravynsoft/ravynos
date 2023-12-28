#readelf: -wl
#source: fragtest.s
#as: --em=criself --gdwarf2

# Highly "fragmented" code.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Special opcode .*: advance Address by 0 to (0x)?0 and Line by 4 to 5
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x2 and Line by 1 to 6
  \[0x.*\]  Advance PC by 126 to 0x80
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x80 and Line by 2 to 8
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x82 and Line by 1 to 9
  \[0x.*\]  Advance PC by 226 to 0x164
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x164 and Line by 6 to 15
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x168 and Line by 1 to 16
  \[0x.*\]  Advance PC by 126 to 0x1e6
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x1e6 and Line by 2 to 18
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x1ea and Line by 1 to 19
  \[0x.*\]  Advance PC by 1126 to 0x650
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x650 and Line by 6 to 25
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x654 and Line by 1 to 26
  \[0x.*\]  Advance PC by 126 to 0x6d2
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x6d2 and Line by 2 to 28
  \[0x.*\]  Special opcode .*: advance Address by 12 to 0x6de and Line by 1 to 29
  \[0x.*\]  Advance Line by 11 to 40
  \[0x.*\]  Advance PC by 33250 to 0x88c0
  \[0x.*\]  Copy
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x88c2 and Line by 1 to 41
  \[0x.*\]  Advance PC by 128 to 0x8942
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8942 and Line by 2 to 43
  \[0x.*\]  Special opcode .*: advance Address by 2 to 0x8944 and Line by 1 to 44
  \[0x.*\]  Advance PC by 248 to 0x8a3c
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8a3c and Line by 6 to 50
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x8a40 and Line by 1 to 51
  \[0x.*\]  Advance PC by 128 to 0x8ac0
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8ac0 and Line by 2 to 53
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x8ac4 and Line by 1 to 54
  \[0x.*\]  Advance PC by 252 to 0x8bc0
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8bc0 and Line by 6 to 60
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x8bc4 and Line by 1 to 61
  \[0x.*\]  Advance PC by 128 to 0x8c44
  \[0x.*\]  Special opcode .*: advance Address by 0 to 0x8c44 and Line by 2 to 63
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x8c48 and Line by 1 to 64
  \[0x.*\]  Advance PC by 124 to 0x8cc4
  \[0x.*\]  Extended opcode 1: End of Sequence
