#name: microMIPS BAL to JALX conversion for local symbol (n32)
#source: ../../../gas/testsuite/gas/mips/branch-local-7.s
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: bal-jalx-local-micromips.d
