#name: PR ld/12627
#as: --32
#ld: -melf_i386 -T pr12627.t
#nm: -n

#...
0+100 A __bss16_dwords
#pass
