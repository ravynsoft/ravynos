#source: eabi-hard-float.s
#as:
#ld: -r
#readelf: -h
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince
# Check that we set the hard-float ABI flag directly

ELF Header:
#...
  Flags:                             0x5000400, Version5 EABI, hard-float ABI
#...
