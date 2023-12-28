;;; Test 16bit relocate with --xgate-ramoffset
;;; 
	.sect .text
	.globl _start
_start:

  ldw r1,#var
  ldw r2,#var+0x106 ; check for correct carry too
