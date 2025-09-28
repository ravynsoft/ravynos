#ld: -r  --gc-sections -u foo
#readelf: -S --wide
# generic linker targets don't support --gc-sections, nor do a bunch of
# others.
#xfail: [is_generic] hppa64-*-* mep-*-* mn10200-*-*

#...
  \[[ 0-9]+\] \.preinit_array\.01000[ \t]+PREINIT_ARRAY[ \t0-9a-f]+WA?.*
#...
  \[[ 0-9]+\] \.init_array\.01000[ \t]+INIT_ARRAY[ \t0-9a-f]+WA?.*
#...
  \[[ 0-9]+\] \.fini_array\.01000[ \t]+FINI_ARRAY[ \t0-9a-f]+WA?.*
#pass
