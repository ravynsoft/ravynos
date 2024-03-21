#as:
#source: sframe-empty.s
#objdump: -hw
#ld: -shared
#name: Empty SFrame section

#failif
#...
  [0-9] .sframe .*
#...
