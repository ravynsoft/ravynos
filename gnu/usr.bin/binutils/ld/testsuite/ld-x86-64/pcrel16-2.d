#name: PCREL16 overflow (2)
#source: ../ld-i386/pcrel16-2.s
#ld:
#error: .*relocation truncated to fit: R_X86_64_PC16 .*t16.*
#error: .*relocation truncated to fit: R_X86_64_PC16 .*_start.*
#xfail: *-*-*
