 .text
 .global _start
_start:
 addpcis 3,(ext1-0f)@ha
0: addi 3,3,(ext1-0b)@l
 addpcis 4,(ext2-0f)@ha
0: addi 4,4,(ext2-0b)@l
 addpcis 5,(forw-0f)@ha
0: addi 5,5,(forw-0b)@l
 .space 32764
forw:
 nop
