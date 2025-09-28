#source: call3.s
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386 -z call-nop=prefix-nop
#error: invalid number for -z call-nop=prefix-: nop
