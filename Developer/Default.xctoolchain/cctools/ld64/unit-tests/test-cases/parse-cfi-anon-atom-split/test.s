	.section	__TEXT,__text,regular,pure_instructions
	.p2align 2

	.global foo
foo:
	.cfi_startproc
	.cfi_lsda 16, LLSDAFOO
	ret
	.cfi_endproc


bar:
	.cfi_startproc
	.cfi_lsda 16, LLSDABAR
	ret
	.cfi_endproc

	# __cstring is parsed using `ImplicitSizeSection` section class. The class has
	# a custom logic to compute atoms count in `ImplicitSizeSection::computeAtomCount`,
	# but uses `LabelAndCFIBreakIterator` when adding atoms in `::appendAtoms`.
	# rdar://93130909 (Tweak parsing and stabs generation of empty atoms) introduced
	# a bug in `LabelAndCFIBreakIterator`, where using it to parse this __cstring
	# section caused any remaining CFI starts to be ignored.
	# This lead to an inconsistent parsing, where `GCC_except_table1` and `LLSDABAR`
	# were *counted* as separate atoms, but the split at `LLSDABAR` was missing when
	# *adding* atoms.
	.section	__TEXT,__cstring,cstring_literals
l_.str:
	.asciz	"bar"

	.section	__TEXT,__gcc_except_tab
	.p2align	3
GCC_except_table0:
LLSDAFOO:
	.byte	0
	.byte	0

GCC_except_table1:
	# Reference to LLSDABAR is a CFI start address. Alignment *within* the
	# GCC_except_table1 symbol should cause LLSDABAR to be split into a separate
	# anonymous atom. Because of the bug introduced in rdar://9313090 `LLSDABAR`
	# was *counted* as a separate atom, but wasn't split when actually *adding* atoms.
	.p2align 3
LLSDABAR:
	.byte	0
	.byte	0

.subsections_via_symbols
