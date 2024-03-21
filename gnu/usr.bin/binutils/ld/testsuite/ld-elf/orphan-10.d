#source: orphan-10.s
#ld: -N -T orphan-9.ld
#objdump: -h
#xfail: [uses_genelf]

#...
  . \.text         0+(08|10)  [0-9a-f]+  0+200 .*
#...
  . \.data\.1       0+8  [0-9a-f]+  0+300 .*
#pass
