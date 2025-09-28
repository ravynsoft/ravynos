#ld: -shared --version-script local1.map
#readelf: -s --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
    .*: [0-9a-f]* +[0-9a-f]+ +OBJECT +LOCAL +DEFAULT +[0-9] +foo
#...
