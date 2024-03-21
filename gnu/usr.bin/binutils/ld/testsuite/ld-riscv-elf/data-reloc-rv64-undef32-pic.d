#source: data-reloc.s
#as: -march=rv64i -mabi=lp64 -defsym __undef__=1
#ld: -m[riscv_choose_lp64_emul] -Ttext 0x8000 --defsym _start=0x0  -shared
#error: .*relocation R_RISCV_32 against non-absolute symbol `undef' can not be used in RV64 when making a shared object.*
