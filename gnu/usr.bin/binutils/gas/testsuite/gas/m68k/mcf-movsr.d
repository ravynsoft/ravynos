#name: mcf-movsr
#objdump: -d
#as: -mcpu=5329

.*:     file format .*

Disassembly of section .text:

0+ <test_movsr>:
   0:	46c3           	movew %d3,%sr
   2:	46fc ffff      	movew #-1,%sr
   6:	40c3           	movew %sr,%d3
   8:	44c3           	movew %d3,%ccr
   a:	44fc ffff      	movew #-1,%ccr
   e:	42c3           	movew %ccr,%d3
