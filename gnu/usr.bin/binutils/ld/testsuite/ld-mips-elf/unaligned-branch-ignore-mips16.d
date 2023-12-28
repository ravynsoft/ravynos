#name: MIPS16 link branch to unaligned symbol (ignore branch ISA)
#ld: -Ttext 0x1c000000 -e 0x1c000000 --ignore-branch-isa
#source: ../../../gas/testsuite/gas/mips/unaligned-branch-mips16-2.s
#error:   [^\n]*: in function `foo':\n
#error:   \(\.text\+0x1008\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x100e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1014\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1020\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1026\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x102c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x104a\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1056\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x1068\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x106e\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1074\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1080\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x1086\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x108c\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10aa\): branch to a non-instruction-aligned address\n
#error:   \(\.text\+0x10b6\): branch to a non-instruction-aligned address\Z
