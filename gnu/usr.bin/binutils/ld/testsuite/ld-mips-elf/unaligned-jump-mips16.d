#name: MIPS16 link jump to unaligned symbol
#ld: -Ttext 0x1c000000 -e 0x1c000000
#source: ../../../gas/testsuite/gas/mips/unaligned-jump-mips16-2.s
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x100e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1014\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x101a\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1020\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1026\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x103e\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1044\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x104a\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1050\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1056\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x105c\): cannot convert a jump to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x106e\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x107a\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x107a\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1080\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x1086\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1092\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x1092\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x1098\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x109e\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x109e\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x10a4\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x10aa\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x10aa\): unsupported JALX to the same ISA mode\n
#error:   \(\.text\+0x10b0\): jump to a non-word-aligned address\n
#error:   \(\.text\+0x10b6\): unsupported JALX to the same ISA mode\Z
