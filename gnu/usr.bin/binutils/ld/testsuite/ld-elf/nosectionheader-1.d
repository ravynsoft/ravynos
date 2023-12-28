#source: start.s
#ld: -z nosectionheader
#readelf: -h -S
#xfail: [uses_genelf]
# These targets don't support -z.

#...
  Start of section headers:[ \t]+0 \(bytes into file\)
#...
  Size of section headers:[ \t]+0 \(bytes\)
  Number of section headers:[ \t]+0
  Section header string table index:[ \t]+0

There are no sections in this file.
