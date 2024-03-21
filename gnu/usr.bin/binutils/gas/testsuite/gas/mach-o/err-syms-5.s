# { dg-do assemble { target i?86-*-darwin* powerpc*-*-darwin* } }

# Show that we can check that there are enough syms for the section.

# too few.

	.section __dummy, __dummy, symbol_stubs,strip_static_syms,4

	.indirect_symbol a

	.section __dummy, __dummy1,lazy_symbol_pointers

	.indirect_symbol b

	.section __dummy, __dummy2,non_lazy_symbol_pointers

	.indirect_symbol c

# OK.
	.section __dummy, __dummy3,non_lazy_symbol_pointers

	.indirect_symbol d
	.space 4
	
	.section __dummy, __dummy4,symbol_stubs,strip_static_syms,17

	.indirect_symbol e
	.space 17

# too many

	.section __dummy, __dummy5,lazy_symbol_pointers

	.indirect_symbol f
	.space 8

# { dg-error "the number of .indirect_symbols defined in section __dummy.__dummy does not match the number expected .1 defined, 0 expected." "" { target *-*-darwin*} 0 }
# { dg-error "the number of .indirect_symbols defined in section __dummy.__dummy1 does not match the number expected .1 defined, 0 expected." "" { target *-*-darwin*} 0 }
# { dg-error "the number of .indirect_symbols defined in section __dummy.__dummy2 does not match the number expected .1 defined, 0 expected." "" { target *-*-darwin*} 0 }
# { dg-error "the number of .indirect_symbols defined in section __dummy.__dummy5 does not match the number expected .1 defined, 2 expected." "" { target *-*-darwin*} 0 }
