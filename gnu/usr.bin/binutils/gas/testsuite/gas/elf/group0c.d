#readelf: -sW
#name: group section name
#source: group0.s
#xfail: *-*-solaris*

#...
.*NOTYPE[ 	]+LOCAL[ 	]+DEFAULT[ 	]+[0-9]+[ 	]+\.foo_group
#pass
