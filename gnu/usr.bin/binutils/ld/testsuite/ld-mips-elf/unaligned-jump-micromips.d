#name: microMIPS link jump to unaligned symbol
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-jump-micromips-2.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x1012\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1018\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x101e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1026\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102e\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1034\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1034\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x103a\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1042\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x104a\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x104a\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1050\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1050\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1056\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x105e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1066\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1066\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x106c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x106c\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1082\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x1088\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x108e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1096\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x109e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x109e\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10a4\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10a4\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10aa\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10b2\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10ba\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10ba\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10c0\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10c0\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10c6\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10ce\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10d6\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10d6\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10dc\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10dc\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10f2\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10f8\): unsupported jump between ISA modes; consider recompiling with interlinking enabled\n
#error:   \(\.text\+0x10fe\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x111a\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1136\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1152\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1152\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x115a\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1162\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x1168\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x116e\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x118a\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x118a\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1192\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x119a\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x11a0\): jump to a non-instruction-aligned address\n
#error:   \(\.text\+0x11a6\): unsupported JALX to the same ISA mode\Z
