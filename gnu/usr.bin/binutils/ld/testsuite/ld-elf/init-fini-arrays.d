#source: init-fini-arrays.s
#ld: -r
#readelf: -S --wide

#...
  \[[ 0-9]+\] \.init_array\.01000[ \t]+INIT_ARRAY[ \t0-9a-f]+WA?.*
#...
  \[[ 0-9]+\] \.fini_array\.01000[ \t]+FINI_ARRAY[ \t0-9a-f]+WA?.*
#pass
