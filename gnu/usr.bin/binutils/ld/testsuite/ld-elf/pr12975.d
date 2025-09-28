#ld: --gc-sections -shared -version-script pr12975.t
#readelf: -s --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: [is_generic] hppa64-*-* mep-*-* mn10200-*-* ![check_shared_lib_support] 
# generic linker targets don't support --gc-sections, nor do a bunch of others

#failif
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +FUNC +LOCAL +DEFAULT +[1-9]+ bar
#...
