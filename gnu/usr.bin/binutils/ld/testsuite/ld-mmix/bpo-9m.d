#source: start3.s
#source: bpo-6.s
#source: bpo-5.s
#as: -linker-allocated-gregs
#ld: -m mmo --gc-sections
#objdump: -st

# Check that GC does not mess up things when no BPO:s are collected.
# Note that mmo doesn't support GC at the moment; it's a nop.

.*:     file format mmo

SYMBOL TABLE:
0+ g .* Main
0+14 g       \.text x
0+10 g       \.text x2

Contents of section \.text:
 0004 0000003d 00000000 0000003a 232dfe00  .*
 0014 232dfd00                             .*
Contents of section \.MMIX\.reg_contents:
 07e8 00000000 0000107c 00000000 0000a420  .*
