#source: orphan4.s
#ld: -T orphan4.ld
#objdump: -h

#...
  . \.foo          0+1  0+1000  0+1000  .*  2\*\*0
#pass
