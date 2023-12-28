 .weak x1, x2, x3, x4
 .protected x2
 .hidden x3
 .internal x4
 .global _start
_start:
 pld 3,x1@got@pcrel
 pld 3,x2@got@pcrel
 pld 3,x3@got@pcrel
 pld 3,x4@got@pcrel
 ld 3,x1@got(2)
 ld 3,x2@got(2)
 ld 3,x3@got(2)
 ld 3,x4@got(2)
 addis 9,2,x1@got@ha
 ld 3,x1@got@l(9)
 addis 9,2,x2@got@ha
 ld 3,x2@got@l(9)
 addis 9,2,x3@got@ha
 ld 3,x3@got@l(9)
 addis 9,2,x4@got@ha
 ld 3,x4@got@l(9)
