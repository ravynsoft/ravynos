#source: start.s
#source: sec-10.s
#ld: -m mmo
#objdump: -s

# There was yet another bug in the strip-zeros-at-beginning-and-end-of-data
# code: it requires outputting the location when data is stripped, and that
# location is only valid for tetra alignments as the low bits are ignored.

.*:     file format mmo

Contents of section \.text:
 0*0 e3fd0001 2a000000 00000000 00000000  .*
 0*10 00000000 00000000 00000000 00000000  .*
#...
 0*7ff0 00000000 00000000 00000000 00000000  .*
 0*8000 00000000 00000000 00000000 2b2c0000  .*

