#name: MIPS JALX to unaligned symbol 3
#source: unaligned-jalx-3.s
#source: unaligned-insn.s -mips16
#as: -EB
#ld: -EB -Ttext 0x1c000000 -e 0x1c000000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x0\): cannot convert a branch to JALX for a non-word-aligned address\Z
