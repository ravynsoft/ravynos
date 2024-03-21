#name: PCREL16 overflow (2)
#as: --32
#ld: -melf_i386
#error: .*relocation truncated to fit: R_386_PC16 .*t16.*
#error: .*relocation truncated to fit: R_386_PC16 .*_start.*
#xfail: *-*-*
