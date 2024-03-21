#name: MIPS branch to unaligned symbol
#source: unaligned-branch.s
#source: unaligned-text.s
#as: -mips32r6
#ld: -Ttext 0x10000000 -e 0x10000000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x14\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x24\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x28\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x30\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x38\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x3c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x44\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x4c\): branch to a non-instruction-aligned address\Z
