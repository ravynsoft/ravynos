# { dg-do assemble { target x86_64-*-darwin* } }

	.symbol_stub	
	.lazy_symbol_pointer	
	.non_lazy_symbol_pointer

# { dg-error ".symbol_stub is not used for the selected target" "" { target x86_64-*-darwin* } 3 }
# { dg-error ".lazy_symbol_pointer is not used for the selected target" "" { target x86_64-*-darwin* } 4 }
# { dg-error ".non_lazy_symbol_pointer is not used for the selected target" "" { target x86_64-*-darwin* } 5 }
