#source: eabi-soft-float.s
#as:
#ld: -r
#readelf: -h
# This test is only valid on ELF based ports.
# not-target: *-*-pe *-*-wince
# if we call "ld -r", it should *not* set either of the float ABI flags

ELF Header:
#...
  Flags:                             0x5000000, Version5 EABI
#...
