 .section .toc,"aw"
 .p2align 4
x4t:
 .quad x4
x5t:
 .quad x5
x6t:
 .quad x6

 .section .sdata,"aw"
 .p2align 4
x1:
 .quad 1
x2:
 .quad 2
x3:
 .quad 3
x4:
 .quad 4
x5:
 .quad 5
x6:
 .quad 6

 .globl _start
 .text
_start:
# no need for got entry, optimise to nop,addi
 addis 9,2,x1@got@ha
 ld 9,x1@got@l(9)
# must keep got entry, optimise to nop,addi,ld
 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 ld 6,0(5)
# can't optimise due to possibility that r28 isn't dead
 addis 28,2,x1@got@ha
 ldu 9,x1@got@l(28)
# no need for toc entry, optimise to nop,addi
 addis 9,2,x4t@toc@ha
 ld 9,x4t@toc@l(9)
# must keep toc entry, optimise to nop,addi,ld
# if we had a reloc tying the ld to x5/x5t then we could throw away
# the toc entry and optimise to nop,nop,addi
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 ld 6,0(5)
# can't optimise due to possibility that r28 isn't dead
 addis 28,2,x4t@toc@ha
 ldu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lwz 6,0(5)
 addis 9,2,x4t@toc@ha
 lwz 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lwz 6,0(5)
 addis 28,2,x4t@toc@ha
 lwzu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lwa 6,0(5)
 addis 9,2,x4t@toc@ha
 lwa 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lwa 6,0(5)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lhz 6,0(5)
 addis 9,2,x4t@toc@ha
 lhz 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lhz 6,0(5)
 addis 28,2,x4t@toc@ha
 lhzu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lha 6,0(5)
 addis 9,2,x4t@toc@ha
 lha 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lha 6,0(5)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lbz 6,0(5)
 addis 9,2,x4t@toc@ha
 lbz 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lbz 6,0(5)
 addis 28,2,x4t@toc@ha
 lbzu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lfs 6,0(5)
 addis 9,2,x4t@toc@ha
 lfs 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lfs 6,0(5)
 addis 28,2,x4t@toc@ha
 lfsu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lfd 6,0(5)
 addis 9,2,x4t@toc@ha
 lfd 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lfd 6,0(5)
 addis 28,2,x4t@toc@ha
 lfdu 9,x4t@toc@l(28)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lxv 6,0(5)
 addis 9,2,x4t@toc@ha
 lxv 9,x4t@toc@l(9)
 addis 4,2,x6t@toc@ha
 addi 5,4,x6t@toc@l
 lxv 6,0(5)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lxsd 6,0(5)
 addis 9,2,x4t@toc@ha
 lxsd 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lxsd 6,0(5)

 addis 4,2,x2@got@ha
 addi 5,4,x2@got@l
 lxssp 6,0(5)
 addis 9,2,x4t@toc@ha
 lxssp 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 lxssp 6,0(5)

 addis 9,2,x4t@toc@ha
 std 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 std 6,0(5)
 addis 28,2,x4t@toc@ha
 stdu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 stw 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stw 6,0(5)
 addis 28,2,x4t@toc@ha
 stwu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 sth 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 sth 6,0(5)
 addis 28,2,x4t@toc@ha
 sthu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 stb 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stb 6,0(5)
 addis 28,2,x4t@toc@ha
 stbu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 stfs 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stfs 6,0(5)
 addis 28,2,x4t@toc@ha
 stfsu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 stfd 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stfd 6,0(5)
 addis 28,2,x4t@toc@ha
 stfdu 9,x4t@toc@l(28)

 addis 9,2,x4t@toc@ha
 stxv 9,x4t@toc@l(9)
 addis 4,2,x6t@toc@ha
 addi 5,4,x6t@toc@l
 stxv 6,0(5)

 addis 9,2,x4t@toc@ha
 stxsd 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stxsd 6,0(5)

 addis 9,2,x4t@toc@ha
 stxssp 9,x4t@toc@l(9)
 addis 4,2,x5t@toc@ha
 addi 5,4,x5t@toc@l
 stxssp 6,0(5)
