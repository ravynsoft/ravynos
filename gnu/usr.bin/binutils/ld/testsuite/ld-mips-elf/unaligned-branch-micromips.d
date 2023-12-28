#name: microMIPS link branch to unaligned symbol
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-branch-micromips-2.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x100a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1012\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x101a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1032\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x103a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1062\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1072\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1082\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1088\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1088\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x108e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x108e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1094\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1094\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x109a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10a0\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10a0\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10a6\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10a6\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10ac\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10ac\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10b2\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10ca\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10d6\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10e2\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10e8\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10e8\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10ee\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10ee\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10f4\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10f4\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x10fa\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1100\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1100\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1106\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1106\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x110c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x110c\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1112\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x112a\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1136\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1142\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1146\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1146\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x114a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x114a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x114e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x114e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1152\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1156\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1156\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x115a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x115a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x115e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x115e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1162\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1172\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x117a\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1182\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1186\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1186\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x118a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x118a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x118e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x118e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1192\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x1196\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1196\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x119a\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x119a\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x119e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x119e\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x11a2\): unsupported branch between ISA modes\n
#error:   \(\.text\+0x11b2\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x11ba\): branch to a non-instruction-aligned address\Z
