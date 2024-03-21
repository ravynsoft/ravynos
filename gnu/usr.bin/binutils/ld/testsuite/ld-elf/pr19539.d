#source: start.s
#source: pr19539.s
#ld: -pie -T pr19539.t --warn-textrel
#readelf : --dyn-syms --wide
#warning: .*: creating DT_TEXTREL in a PIE
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi
# The BFIN target always generates a relocation.
#xfail: ![check_pie_support] || bfin-*-*

Symbol table '\.dynsym' contains [0-9]+ entr(y|ies):
#pass
