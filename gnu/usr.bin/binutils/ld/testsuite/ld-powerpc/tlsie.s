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
 ld 9,.La@toc(2)
 add 10,9,.La@tls
 .section .toc,"aw",@progbits
.La:
 .quad a@tprel
 .text

#Medium model ELF
 addis 9,2,b@got@tprel@ha
 ld 10,b@got@tprel@l(9)
 add 3,10,b@tls

#PCrel
 pld 4,c@got@tprel@pcrel
 add 4,4,c@tls@pcrel

#All of the above using the same symbol
 addis 9,2,.Ld@toc@ha
 ld 9,.Ld@toc@l(9)
 add 10,9,.Ld@tls
 .section .toc,"aw",@progbits
.Ld:
 .quad d@tprel
 .text
 addis 9,2,d@got@tprel@ha
 ld 31,d@got@tprel@l(9)
 add 3,31,d@tls
 pld 30,d@got@tprel@pcrel
 add 3,30,d@tls@pcrel
#Note that after optimisation r31 and r30 above have a different value to
#what they would have without optimisation.  r31 may not even be written.
#Here are all the other insns that gas/ld accept as the final insn of an
#IE sequence.  The r30 value below will be different after optimisation
#for the non-update forms.
 lwzx 4,30,d@tls@pcrel
 lwzux 4,30,d@tls@pcrel
 lbzx 5,30,d@tls@pcrel
 lbzux 5,30,d@tls@pcrel
 stwx 6,30,d@tls@pcrel
 stwux 6,30,d@tls@pcrel
 stbx 7,30,d@tls@pcrel
 stbux 7,30,d@tls@pcrel
 lhzx 8,30,d@tls@pcrel
 lhzux 8,30,d@tls@pcrel
 lhax 9,30,d@tls@pcrel
 lhaux 9,30,d@tls@pcrel
 sthx 10,30,d@tls@pcrel
 sthux 10,30,d@tls@pcrel
 lfsx 11,30,d@tls@pcrel
 lfsux 11,30,d@tls@pcrel
 lfdx 12,30,d@tls@pcrel
 lfdux 12,30,d@tls@pcrel
 stfsx 13,30,d@tls@pcrel
 stfsux 13,30,d@tls@pcrel
 stfdx 14,30,d@tls@pcrel
 stfdux 14,30,d@tls@pcrel
 ldx 15,30,d@tls@pcrel
 ldux 15,30,d@tls@pcrel
 stdx 16,30,d@tls@pcrel
 stdux 16,30,d@tls@pcrel
 lwax 17,30,d@tls@pcrel
