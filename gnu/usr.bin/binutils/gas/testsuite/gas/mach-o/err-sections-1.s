# { dg-do assemble { target x86_64-*-darwin* } }

	.section __a,__b,symbol_stubs,strip_static_syms,4	
	.section __a,__c,lazy_symbol_pointers,strip_static_syms,4
	.section __a,__d,non_lazy_symbol_pointers,strip_static_syms,4

# { dg-error "unknown or invalid section type .symbol_stubs." "" { target x86_64-*-darwin* } 3 }
# { dg-error "unknown or invalid section type .lazy_symbol_pointers." "" { target x86_64-*-darwin* } 4 }
# { dg-error "unknown or invalid section type .non_lazy_symbol_pointers." "" { target x86_64-*-darwin* } 5 }
