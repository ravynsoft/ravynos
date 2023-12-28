#ld: -T arm.ld
#objdump: -s

.*:     file format.*

#...
Contents of section .ARM.exidx:
 800c (f4ffff7f b0b0a880 f0ffff7f 01000000|7ffffff4 80a8b0b0 7ffffff0 00000001)  .*
 801c (ecffff7f b0b0a880 e8ffff7f 01000000|7fffffec 80a8b0b0 7fffffe8 00000001)  .*
Contents of section .far:
#...
