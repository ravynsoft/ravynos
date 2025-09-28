#source: start.s
#readelf: -d -W
#ld: -pie
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi
#xfail: ![check_pie_support] 

#...
 +0x[0-9a-f]+ +\(FLAGS_1\) +Flags: +PIE
#pass
