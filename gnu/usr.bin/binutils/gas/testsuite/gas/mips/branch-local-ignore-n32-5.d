#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branch local symbol relocation 5 (ignore branch ISA, n32)
#as: -n32 -march=from-abi -mignore-branch-isa
#source: branch-local-5.s
#dump: branch-local-ignore-5.d
