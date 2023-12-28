#source: eabi-soft-float.s
#as:
#ld: -r
#readelf: -h
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince
# if we compile for EABI ver4, ld should *not* set either of the float ABI flags

ELF Header:
#...
  Flags:                             0x4000000, Version4 EABI
#...
