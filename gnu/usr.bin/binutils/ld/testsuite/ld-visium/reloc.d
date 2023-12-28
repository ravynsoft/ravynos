#source: reloc1.s
#source: reloc2.s
#ld: -T ld1.ld
#objdump: -s

.*: +file format .*

Contents of section .text:
 10000 00000000 7800000a 84c20034 8482003c  ....x......4...<
 10010 84a20001 84c20002 0442002c 84020002  .........B.,....
 10020 0482002c 84a20002 84c21000           ...,........    
Contents of section .data:
 30000 00140014 00000014 0001002c 0002002c  ...........,...,
 30010 10001010                             ....            
