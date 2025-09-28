#as: --64
#ld: -pie -melf_x86_64 --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code
#error: .*relocation R_X86_64_PC32 against undefined symbol `foo' can not be used when making a PIE object; recompile with -fPIE
