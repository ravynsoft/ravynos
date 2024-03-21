#source: orphan3a.s
#source: orphan3b.s
#source: orphan3c.s
#source: orphan3d.s
#source: orphan3e.s
#source: orphan3f.s
#ld:
#readelf: -S --wide
#xfail: [uses_genelf]
#xfail: xstormy16-*-* pru-*-*

#...
  \[[ 0-9]+\] \.foo +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +0+20 +0+ +A +0 +0 +[0-9]+
#...
  \[[ 0-9]+\] \.foo +NOBITS +[0-9a-f]+ +[0-9a-f]+ +0+20 +0+ +A +0 +0 +[0-9]+
#...
  \[[ 0-9]+\] \.foo +PROGBITS +0+ +[0-9a-f]+ +0+20 +0+ +0 +0 +[0-9]+
  \[[ 0-9]+\] [._][^f].*
#pass
