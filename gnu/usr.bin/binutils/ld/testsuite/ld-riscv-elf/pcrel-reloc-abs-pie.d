#source: pcrel-reloc.s
#source: pcrel-reloc-abs.s
#as: -march=rv64i -mabi=lp64
#ld: -melf64lriscv --pie --no-relax
#error: .*relocation R_RISCV_PCREL_HI20 against absolute symbol `sym' can not be used when making a shared objec.*t
