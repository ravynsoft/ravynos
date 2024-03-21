#name: MIPS BAL to JALX conversion for local symbol (n64)
#source: ../../../gas/testsuite/gas/mips/branch-local-4.s
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: bal-jalx-local.d
