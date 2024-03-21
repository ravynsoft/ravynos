#name: ljmp segment value overflow
#as: --32
#source: ljmp2.s
#source: ljmp.s
#ld: -melf_i386 -z noseparate-code
#error: .*relocation truncated to fit: R_386_16 .*
#error: .*relocation truncated to fit: R_386_16 .*
