#source: orphan-9.s
#ld: -N -T orphan-9.ld
#objdump: -h
#xfail: [uses_genelf]

#...
  . \.text         0+(08|10)  [0-9a-f]+  0+200 .*
#...
  . \.data         0+(08|10)  [0-9a-f]+  0+300 .*
#...
  . \.data\.1       0+8  [0-9a-f]+  0+3(0|1)[0-9a-f] .*
#pass
