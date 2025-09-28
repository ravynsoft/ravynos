#source: pr23658-1a.s
#source: pr23658-1b.s
#source: pr23658-1c.s
#source: pr23658-1d.s
#source: start.s
#ld: --build-id -shared
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
 +[0-9]+ +\.note\.4 \.note\.1 +
 +[0-9]+ +\.note.gnu.build-id \.note\.2 .note\.3 +
#pass
