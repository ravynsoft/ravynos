#as:
#source: discard.s
#ld: -T discard.ld
#objdump: -hw
#name: Check that SFrame section can be discarded

#failif
#...
  [0-9] .sframe .*
#...
