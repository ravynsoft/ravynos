#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch to absolute expression (n32)
#source: ../../../gas/testsuite/gas/mips/mips16-branch-absolute.s
#ld: -Ttext 0 -e foo
#dump: mips16-branch-absolute.d
