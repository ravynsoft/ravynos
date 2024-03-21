#source: orphan-12.s
#ld: -T orphan-11.ld --strip-debug --orphan-handling=error
#objdump: -wh

#...
  . \.text .*
  . \.data .*
#pass
