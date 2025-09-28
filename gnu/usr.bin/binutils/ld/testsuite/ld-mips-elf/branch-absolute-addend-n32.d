#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS link branch to absolute expression with addend (n32)
#source: ../../../gas/testsuite/gas/mips/branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo
#dump: branch-absolute-addend.d
