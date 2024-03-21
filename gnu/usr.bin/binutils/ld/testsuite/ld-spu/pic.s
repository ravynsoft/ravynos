 .global _end
 .global _start
 .global glob
 .weak undef

 .section .text.a,"ax"
before:
 .long 0
 .long 0

 .section .text.b,"ax"
_start:
 ila 2,.+8
 brsl 126,.+4
 sf 126,2,126
 ila 4,before+4
 ila 5,after-4
 ila 6,_start
 ila 7,end
 .reloc .,SPU_ADD_PIC,before+4
 a 4,4,126
 .reloc .,SPU_ADD_PIC,after-4
 a 5,5,126
 .reloc .,SPU_ADD_PIC,_start
 a 6,6,126
 .reloc .,SPU_ADD_PIC,end
 a 7,7,126
 ila 14,before
 .reloc .,SPU_ADD_PIC,before
 a 14,14,126

 ila 3,undef
 .reloc .,SPU_ADD_PIC,undef
 a 3,3,126
 ilhu 7,ext@h
 iohl 7,ext@l
 .reloc .,SPU_ADD_PIC,ext
 a 4,7,126
 ila 9,loc
 .reloc .,SPU_ADD_PIC,loc
 a 5,9,126
 ila 8,glob
 .reloc .,SPU_ADD_PIC,glob
 a 6,8,126
 ila 9,_end
 .reloc .,SPU_ADD_PIC,_end
 a 9,9,126

 hbrr acall,abscall
 lqr 2,undef
 stqr 2,undef
 lqr 3,ext
 lqr 4,ext+16
acall:
 brsl 0,abscall
 br abscall
end:

 .section .text.c,"ax"
 .long 0
after:
 .long 0

 .data
loc:
 .long 1,0,0,0
glob:
 .long 2,0,0,0
