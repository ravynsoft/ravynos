#name: call relaxation with alignment
#source: call-relax-0.s
#source: call-relax-1.s
#source: call-relax-2.s
#source: call-relax-3.s
#as: -march=rv32ic_zicsr -mno-arch-attr
#ld: -m[riscv_choose_ilp32_emul]
#objdump: -d
#pass
