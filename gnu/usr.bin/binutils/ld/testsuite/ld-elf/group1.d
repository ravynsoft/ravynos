#source: group1a.s
#source: group1b.s
#ld: -T group.ld
#readelf: -s
#xfail: [is_generic] i?86-*-solaris* x86_64-*-solaris*
# generic linker targets don't comply with all symbol merging rules

Symbol table '.symtab' contains .* entries:
#...
.*: 0+1000 +0 +(NOTYPE|OBJECT) +WEAK +DEFAULT +. foo
