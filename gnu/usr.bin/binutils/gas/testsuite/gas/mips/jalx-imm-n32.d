#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS JAL/JALX immediate operand encoding (n32)
#as: -n32 -march=from-abi
#source: jalx-imm.s
#dump: jalx-imm.d
