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

 .text
 .globl _start
_start:
#Small model OpenPower
 addi 3,2,.La@toc
 bl __tls_get_addr(.La@tlsgd)
 nop
 .section .toc,"aw",@progbits
 .p2align 3
.La:
 .quad a@dtpmod
 .quad a@dtprel
 .text

#Medium mode ELF
 addis 3,2,b@got@tlsgd@ha
 addi 3,3,b@got@tlsgd@l
 bl __tls_get_addr(b@tlsgd)
 nop

#PCrel
 pla 3,c@got@tlsgd@pcrel
 bl __tls_get_addr@notoc(c@tlsgd)

#All of the above using the same symbol
 addis 3,2,.Ld@toc@ha
 addi 3,3,.Ld@toc@l
 bl __tls_get_addr(.Ld@tlsgd)
 nop
 .section .toc,"aw",@progbits
.Ld:
 .quad d@dtpmod
 .quad d@dtprel
 .text
 addis 3,2,d@got@tlsgd@ha
 addi 3,3,d@got@tlsgd@l
 bl __tls_get_addr(d@tlsgd)
 nop
 pla 3,d@got@tlsgd@pcrel
 bl __tls_get_addr@notoc(d@tlsgd)
