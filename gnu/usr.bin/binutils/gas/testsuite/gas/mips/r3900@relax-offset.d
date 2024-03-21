#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS PIC branch relaxation with offset
#as: -32 -relax-branch
#warning_output: relax-offset.l
#source: relax-offset.s
#dump: mips1@relax-offset.d
