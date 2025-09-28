#source: pr19807-2.s
#as: --64
#ld: -pie -melf_x86_64
#error: .*relocation R_X86_64_32 against `.data' can not be used when making a PIE object; recompile with -fPIE
