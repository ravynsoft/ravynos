#name: MIPS JALX to local symbol (n32)
#source: ../../../gas/testsuite/gas/mips/jalx-local.s
#ld: -Ttext 0x1c000000 -e 0x1c000000
#objdump: -dr --prefix-addresses --show-raw-insn
#dump: jalx-local.d
