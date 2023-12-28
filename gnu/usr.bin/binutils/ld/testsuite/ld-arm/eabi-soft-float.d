#source: eabi-soft-float.s
#as:
#ld: -r
#readelf: -h
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince
# Check that we set the soft-float ABI flag directly

ELF Header:
#...
  Flags:                             0x5000200, Version5 EABI, soft-float ABI
#...
