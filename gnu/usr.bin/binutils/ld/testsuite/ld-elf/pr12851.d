#source: pr12851.s
#source: start.s
#ld: --gc-sections
#readelf: -s --wide
#xfail: [is_generic] hppa64-*-* mep-*-* mn10200-*-*
# generic linker targets don't support --gc-sections, nor do a bunch of others

#...
 +.* _.stapsdt.base
#pass
