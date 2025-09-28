#source: call1.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -z call-nop=prefix-nop
#error: invalid number for -z call-nop=prefix-: nop
