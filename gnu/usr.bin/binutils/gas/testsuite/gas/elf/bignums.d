#readelf: -x .data
#name: bignum byte values
#xfail: rx-*
# The RX target sometimes calls its data section D_1.
# 
# Test that 8-bit and 16-bit constants can be specified via bignums.
# 
# Note - we should really apply this test to all targets, not just
# ELF based ones, but we need a tool that can dump the data section
# in a fixed format and readelf fits the bill.

Hex dump of section .*:
  0x00000000 9800(7698|9876) 9800(7698|9876) 9800.*
#pass
