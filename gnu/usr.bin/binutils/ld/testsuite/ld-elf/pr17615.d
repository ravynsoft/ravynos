#ld: --gc-sections -shared
#readelf: -S --wide --dyn-syms
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: [is_generic] hppa64-*-* mep-*-* mn10200-*-* ![check_shared_lib_support] 
# generic linker targets don't support --gc-sections, nor do a bunch of others

#...
  \[[ 0-9]+\] \.s?bss[ \t]+NOBITS[ \t0-9a-f]+WA.*
#...
 +[0-9]+: +[0-9a-f]+ +4 +OBJECT +GLOBAL +DEFAULT +[1-9]+ foo
#pass
