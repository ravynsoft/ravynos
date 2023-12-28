#readelf: -wl
#source: binop-segref.s
#as: --em=criself --gdwarf2

# Simple instruction sequence with content-emitting pseudo-ops.
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to 0x5005a
  \[0x.*\]  Advance Line by 36 to 37
  \[0x.*\]  Copy
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5005e and Line by 1 to 38
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50062 and Line by 1 to 39
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50066 and Line by 1 to 40
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5006a and Line by 2 to 42
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5006e and Line by 1 to 43
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50072 and Line by 1 to 44
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50076 and Line by 1 to 45
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5007a and Line by 2 to 47
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5007e and Line by 1 to 48
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50082 and Line by 1 to 49
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x50086 and Line by 1 to 50
  \[0x.*\]  Special opcode .*: advance Address by 4 to 0x5008a and Line by 2 to 52
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x50090 and Line by 1 to 53
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x50096 and Line by 1 to 54
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x5009c and Line by 1 to 55
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500a2 and Line by 2 to 57
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500a8 and Line by 1 to 58
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500ae and Line by 1 to 59
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500b4 and Line by 1 to 60
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500ba and Line by 2 to 62
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500c0 and Line by 1 to 63
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500c6 and Line by 1 to 64
  \[0x.*\]  Special opcode .*: advance Address by 6 to 0x500cc and Line by 1 to 65
  \[0x.*\]  Advance PC by 327776 to 0xa012c
  \[0x.*\]  Extended opcode 1: End of Sequence
