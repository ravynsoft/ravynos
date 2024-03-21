#nm: --extern-only
#name: ELF struct
#
# Test the .struct pseudo-op.
# The #... is there to match extra symbols inserted by
# some toolchains, eg msp430-elf will add _crt0_movedata.

#...
0+0 D l1
0+4 D l2
0+2 A w1
0+4 A w2
0+6 A w3
