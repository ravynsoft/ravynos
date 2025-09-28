# Relocations against undefined symbols would be incorrectly let through
# by mips_elf_calculate_relocation() once the result of the
# ->undefined_symbol() callback has been interpreted in the opposite
# sense.  The link would fail anyway, but for R_MIPS_GOT_PAGE relocations
# a failure of the following assertion:
#
# BFD_ASSERT (h->dynindx >= global_got_dynindx);
#
# would additionally be reported in mips_elf_global_got_index(), because
# at this point h->dynindx for the undefined symbol would be set to -1.
# Other kinds of GOT relocations allocate a GOT index for the symbol
# referred and set its h->dynindx in _bfd_mips_elf_check_relocs(), but
# R_MIPS_GOT_PAGE relocations only allocate a GOT page at that point and
# for undefined symbols the page never gets resolved any further.

	.abicalls
	.text
	.globl	foo
	.type	foo, @function
	.ent	foo
foo:
	li	$2, %got_page(bar)
	.end	foo
	.size	foo, . - foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
