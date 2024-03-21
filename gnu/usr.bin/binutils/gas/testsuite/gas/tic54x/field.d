#objdump: -s -t
#name: c54x field directive

.*:     file format .*

SYMBOL TABLE:
#...
.* 0x0+0 f1
.* 0x0+1 f2
.* 0x0+1 f3
.* 0x0+2 f4
.* 0x0+3 f5
.* 0x0+5 f6
.* 0x0+6 f7
.* 0x0+7 f8
#...

Contents of section \.text:
 0000 f02a0056 01000000 21430f00 00608a00 .*
