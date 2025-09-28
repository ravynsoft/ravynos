#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS link branch to absolute expression with addend (n64)
#source: ../../../gas/testsuite/gas/mips/micromips-branch-absolute-addend.s
#ld: -Ttext 0x12340000 -e foo
#dump: micromips-branch-absolute-addend.d
