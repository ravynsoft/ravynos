
.*:     file format elf32-.*


Disassembly of section .text:

0[0-9a-f]+ <future>:
    [0-9a-f]+:	f2c0 e807 	bf	a, 8012 <_start>
    [0-9a-f]+:	f182 e805 	bfcsel	6, 8012 <_start>, a, eq
    [0-9a-f]+:	f080 c803 	bfl	2, 8012 <_start>
    [0-9a-f]+:	4408      	add	r0, r1

0[0-9a-f]+ <branch>:
    [0-9a-f]+:	f000 b800 	b.w	8012 <_start>

0[0-9a-f]+ <_start>:
    [0-9a-f]+:	4408      	add	r0, r1
