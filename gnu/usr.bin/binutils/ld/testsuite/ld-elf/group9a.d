#source: group9.s
#ld: -r --gc-sections --entry foo
#readelf: -g --wide
# generic linker targets don't support --gc-sections, nor do a bunch of others
#xfail: [is_generic] hppa64-*-* mep-*-* mn10200-*-*

COMDAT group section \[[ 0-9]+\] `.group' \[foo\] contains . sections:
   \[Index\]    Name
   \[[ 0-9]+\]   .text.foo
#...
   \[[ 0-9]+\]   .data.foo
#pass
