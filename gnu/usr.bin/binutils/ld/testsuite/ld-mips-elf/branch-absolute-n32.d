#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS link branch to absolute expression (n32)
#source: ../../../gas/testsuite/gas/mips/branch-absolute.s
#ld: -Ttext 0 -e foo
#dump: branch-absolute.d
