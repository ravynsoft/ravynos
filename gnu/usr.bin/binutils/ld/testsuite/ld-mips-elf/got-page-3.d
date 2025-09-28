#name: GOT page test 3
#source: got-page-3a.s
#source: got-page-3b.s
#source: got-page-3c.s
#as: -mips3
#ld: -T got-page-1.ld -shared
#objdump: -dr
#
# got-page-3a.s and got-page-3b.s should get assigned the same GOT,
# with a page estimate of 10.  Thus the first page entry has offset
# -32744 (-32768 + 0x8000 - ELF_MIPS_GP_OFFSET + MIPS_RESERVED_GOTNO)
# and the first global entry has an offset -32744 + 40 == -32704.
#
# got-page-3c.s should get its own GOT, and needs no page entries.
# The first global symbol should therefore be at offset -32744.
#
#...
.*	lw	a0,-32744\(gp\)
.*	addiu	a0,a0,.*
#...
.*	lw	a1,-32704\(gp\)
#...
.* <f3>:
#...
.*	lw	a1,-32744\(gp\)
#pass
