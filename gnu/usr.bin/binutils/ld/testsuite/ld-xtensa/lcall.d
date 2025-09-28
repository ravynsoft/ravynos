#source: lcall1.s
#source: lcall2.s
#ld: -T lcall.t
#objdump: -d
#...
00000004 <label1>:
#failif
#...
.*l32r.*
#...
