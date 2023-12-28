#name: MIPS16 link branch to unaligned symbol
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-branch-mips16-2.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x1002\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1008\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1008\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x100e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x100e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1014\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1014\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x101a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1020\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1020\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1026\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1026\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x102c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1032\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x104a\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1056\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1062\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1068\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1068\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x106e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x106e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1074\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1074\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x107a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1080\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1080\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1086\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1086\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x108c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x108c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1092\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10aa\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10b6\): branch to a non-instruction-aligned address\Z
