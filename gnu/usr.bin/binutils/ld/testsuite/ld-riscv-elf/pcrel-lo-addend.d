#name: %pcrel_lo section symbol with an addend
#source: pcrel-lo-addend.s
#as: -march=rv32ic
#ld: -m[riscv_choose_ilp32_emul]
#error: .*dangerous relocation: %pcrel_lo section symbol with an addend
