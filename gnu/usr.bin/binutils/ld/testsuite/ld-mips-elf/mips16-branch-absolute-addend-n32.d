#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch to absolute expression with addend (n32)
#source: ../../../gas/testsuite/gas/mips/mips16-branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo
#dump: mips16-branch-absolute-addend.d
