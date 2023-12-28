#source: pcrel-lo-addend-3c.s
#as: -march=rv64i -mabi=lp64 -mno-relax
#ld: -m[riscv_choose_lp64_emul] -Tpcrel-lo-addend-3.ld
#error: .*dangerous relocation: %pcrel_lo with addend isn't allowed for R_RISCV_GOT_HI20
