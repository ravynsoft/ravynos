#name: MIPS link jump to unaligned symbol
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-jump-2.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x1004\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x101c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x101c\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1024\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x102c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1034\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1034\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x103c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1044\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x104c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x104c\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1054\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x105c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1064\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x107c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x107c\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1084\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x108c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1094\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1094\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x109c\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x10a4\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x10ac\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x10ac\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x10b4\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x10bc\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x10c4\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x10ec\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10f4\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10fc\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1104\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1104\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x111c\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1124\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x112c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1134\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1134\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x113c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1144\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x114c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x114c\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1154\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x115c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1164\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1164\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x117c\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\Z
