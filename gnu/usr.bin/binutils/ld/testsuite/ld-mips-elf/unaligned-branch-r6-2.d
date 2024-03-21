#name: MIPSr6 link branch to unaligned symbol 2
#as: -mips64r6
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-branch-r6-4.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x101c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1024\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x102c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1034\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x103c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1044\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x104c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1054\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x105c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x107c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1084\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x108c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1094\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x109c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10a4\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10ac\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10b4\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10bc\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10f4\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10fc\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1104\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1124\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x112c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1134\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x113c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1144\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x114c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1154\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x115c\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1164\): branch to a non-instruction-aligned address\Z
