#objdump: -d
#name:    
#source:  brset-clr-reg-imm-rel.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	03 10 05    	brset d2, #2, \*\+5
   3:	03 19 17    	brset d3, #3, \*\+23
   6:	03 11 71    	brset d3, #2, \*-15
   9:	03 40 43    	brset d2, #8, \*-61
   c:	03 40 fd 01 	brset d2, #8, \*-767
  10:	03 6e fd 01 	brset d6, #13, \*-767

00000014 <L2>:
  14:	02 10 05    	brclr d2, #2, \*\+5
  17:	02 19 17    	brclr d3, #3, \*\+23
  1a:	02 11 fc c0 	brclr d3, #2, \*-832
  1e:	02 40 43    	brclr d2, #8, \*-61
  21:	02 40 fd 01 	brclr d2, #8, \*-767
