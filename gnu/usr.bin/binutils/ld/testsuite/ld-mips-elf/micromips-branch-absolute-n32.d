#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS link branch to absolute expression (n32)
#source: ../../../gas/testsuite/gas/mips/micromips-branch-absolute.s
#ld: -Ttext 0 -e foo
#dump: micromips-branch-absolute.d
