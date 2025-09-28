#name: MIPS JALX to unaligned symbol with addend 3
#source: unaligned-jalx-addend-3.s
#source: unaligned-insn.s -mips16
#ld: -Ttext 0x1c000000 -e 0x1c000000
#error: \A[^\n]*: in function `foo':\n
#error:   \(\.text\+0x0\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x8\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x10\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x18\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x20\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x28\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x30\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x38\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x40\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x48\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x50\): cannot convert a branch to JALX for a non-word-aligned address\n
#error:   \(\.text\+0x58\): cannot convert a branch to JALX for a non-word-aligned address\Z
