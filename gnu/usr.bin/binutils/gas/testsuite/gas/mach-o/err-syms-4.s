# { dg-do assemble }

	.indirect_symbol a

# { dg-error " an .indirect_symbol must be in a symbol pointer or stub section." "" { target *-*-darwin*} 3 }

