# { dg-do assemble { target x86_64-*-darwin* } }

	.section __dummy, __dummy, symbol_stubs,strip_static_syms,4

	.indirect_symbol a

	.section __dummy, __dummy1,lazy_symbol_pointers

	.indirect_symbol b

	.section __dummy, __dummy2,non_lazy_symbol_pointers

	.indirect_symbol c

# { dg-error "unknown or invalid section type .symbol_stubs." "" { target x86_64-*-darwin* } 3 }
# { dg-error "an .indirect_symbol must be in a symbol pointer or stub section" "" { target x86_64-*-darwin* } 5 }
# { dg-error "unknown or invalid section type .lazy_symbol_pointers." "" { target x86_64-*-darwin* } 7 }
# { dg-error "an .indirect_symbol must be in a symbol pointer or stub section" "" { target x86_64-*-darwin* } 9 }
# { dg-error "unknown or invalid section type .non_lazy_symbol_pointers." "" { target x86_64-*-darwin* } 11 }
# { dg-error "an .indirect_symbol must be in a symbol pointer or stub section" "" { target x86_64-*-darwin* } 13 }
