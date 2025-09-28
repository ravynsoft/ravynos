#name: MIPS BAL/JALX in PIC mode (ignore branch ISA, n64)
#source: ../../../gas/testsuite/gas/mips/branch-addend.s
#ld: -Ttext 0x1c000000 -e 0x1c000000 -shared --ignore-branch-isa
#objdump: -dr --prefix-addresses --show-raw-insn
#target: [check_shared_lib_support]
#dump: bal-jalx-pic-ignore.d
