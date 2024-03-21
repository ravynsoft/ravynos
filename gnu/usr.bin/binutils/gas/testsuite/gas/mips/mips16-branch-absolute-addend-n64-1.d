#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch to absolute expression with addend 1 (n64)
#as: -64 -march=from-abi
#source: mips16-branch-absolute-addend-1.s
#dump: mips16-branch-absolute-addend-n64.d
