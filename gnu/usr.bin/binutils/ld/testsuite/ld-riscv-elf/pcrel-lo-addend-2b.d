#name: %pcrel_lo overflow with an addend (2b)
#source: pcrel-lo-addend-2b.s
#as: -march=rv32ic
#ld: -m[riscv_choose_ilp32_emul] --no-relax
#error: .*dangerous relocation: %pcrel_lo overflow with an addend, the value of %pcrel_hi is 0x1000 without any addend, but may be 0x2000 after adding the %pcrel_lo addend
