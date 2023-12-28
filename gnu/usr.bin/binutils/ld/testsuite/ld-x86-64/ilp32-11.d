#as: --x32
#ld: -shared -melf32_x86_64
#error: .*addend 0x7fffffff in relocation R_X86_64_64 against symbol `func' at 0 in section `.data.rel.local' is out of range
