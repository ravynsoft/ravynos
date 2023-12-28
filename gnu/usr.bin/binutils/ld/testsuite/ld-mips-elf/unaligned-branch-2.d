#name: MIPS link branch to unaligned symbol 2
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-branch-2.s
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
#error:   \(\.text\+0x10e4\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10ec\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10f4\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10fc\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10fc\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1104\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1104\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1114\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x111c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1124\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x112c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x112c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1134\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1134\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x113c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1144\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1144\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x114c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x114c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1154\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x115c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x115c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1164\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1164\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1174\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x117c\): unsupported branch between ISA modes\Z
