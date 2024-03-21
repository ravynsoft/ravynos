.*:     file format .*

Disassembly of section .text:

00008000 <_start>:
    8000:	f050 a002 	bne.w	58008 <__bar_from_thumb>
	\.\.\.
   58004:	f040 8000 	bne.w	58008 <__bar_from_thumb>

00058008 <__bar_from_thumb>:
   58008:	4778      	bx	pc
   5800a:	e7fd      	b.n	.+ <.+>
   5800c:	ea02fffb 	b	118000 <bar>

Disassembly of section .foo:

00118000 <bar>:
  118000:	e12fff1e 	bx	lr
