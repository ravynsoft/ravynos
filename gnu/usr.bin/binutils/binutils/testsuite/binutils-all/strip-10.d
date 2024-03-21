#PROG: strip
#source: unique.s
#strip: -g
#readelf: -sh
#name: strip on STB_GNU_UNIQUE

#...
  OS/ABI:[ \t]+UNIX - GNU
#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +OBJECT +(UNIQUE|<OS specific>: 10) +DEFAULT +[1-9] foo
