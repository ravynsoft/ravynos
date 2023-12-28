#source: lns-big-delta.s
#readelf: -wl
#name: lns-big-delta
Raw dump of debug contents of section \.debug_line:
#...
 Line Number Statements:
  \[0x.*\]  Extended opcode 2: set Address to (0x)?0
  \[0x.*\]  Copy
  \[0x.*\]  Advance Line by 1 to 2
  \[0x.*\]  Advance PC by fixed size amount 0 to (0x)?0
  \[0x.*\]  Copy \(view 1\)
  \[0x.*\]  Advance Line by 1 to 3
  \[0x.*\]  Extended opcode 2: set Address to 0x.*
  \[0x.*\]  Copy
  \[0x.*\]  Advance PC by fixed size amount .* to 0x.*
  \[0x.*\]  Extended opcode 1: End of Sequence
#pass
