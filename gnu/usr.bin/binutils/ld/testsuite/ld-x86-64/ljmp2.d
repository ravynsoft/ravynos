#name: ljmp segment value overflow
#source: ../ld-i386/ljmp2.s
#source: ../ld-i386/ljmp.s
#ld: -z noseparate-code
#error: .*relocation truncated to fit: R_X86_64_16 .*
#error: .*relocation truncated to fit: R_X86_64_16 .*
