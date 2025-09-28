 .section ".tbss","awT",@nobits
 .p2align 3
pad: .space 8
 .global a
a: .space 8
 .global b
b: .space 8
 .global c
c: .space 8
 .global d
d: .space 8
z2: .space 8
z3: .space 8

 .text
 .globl _start
_start:
#Small model OpenPower
 addi 3,2,.La@toc
 bl __tls_get_addr(.La@tlsld)
 nop
 .section .toc,"aw",@progbits
 .p2align 3
.La:
 .quad a@dtpmod
 .quad 0
 .text

#Medium mode ELF
 addis 3,2,b@got@tlsld@ha
 addi 3,3,b@got@tlsld@l
 bl __tls_get_addr(b@tlsld)
 nop

#PCrel, with dtprel access to vars
 pla 3,c@got@tlsld@pcrel
 bl __tls_get_addr@notoc(c@tlsld)
 paddi 9,3,z2@dtprel
 pld 10,z3@got@dtprel@pcrel
 add 10,10,3

#All of the above using the same symbol
 addis 3,2,.Ld@toc@ha
 addi 3,3,.Ld@toc@l
 bl __tls_get_addr(.Ld@tlsld)
 nop
 .section .toc,"aw",@progbits
 .p2align 3
.Ld:
 .quad d@dtpmod
 .quad 0
 .text
 addis 3,2,d@got@tlsld@ha
 addi 3,3,d@got@tlsld@l
 bl __tls_get_addr(d@tlsld)
 nop
 pla 3,d@got@tlsld@pcrel
 bl __tls_get_addr@notoc(d@tlsld)
