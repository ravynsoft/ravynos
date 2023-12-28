#ld: -T pr25022.t
#readelf: -SW
#xfail: [is_generic]
#xfail: fr30-*-* frv-*-elf ft32-*-* iq2000-*-* mn10200-*-* msp*-* mt-*-*
# They don't use ldelf_before_place_orphans.

#failif
#...
 +\[ *[0-9]+\] \.(bar|moo|zed) +.*
#...
