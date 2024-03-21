#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JALR relocation against local symbol
#as: -32
#source: jal-svr4pic-local.s
#dump: mips1@jal-svr4pic-local.d
