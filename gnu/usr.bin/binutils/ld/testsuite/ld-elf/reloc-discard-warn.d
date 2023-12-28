#name: Discarded dynamic relocation section
#ld: -shared -T reloc-discard.ld
#readelf: -r --use-dynamic
#target: [check_shared_lib_support]
#noskip: powerpc64*-*-*
#source: reloc-discard.s
#warning: .*discarding dynamic section.*
# More targets should be using this variant of reloc-discard.d.

There are no dynamic relocations in this file\.
