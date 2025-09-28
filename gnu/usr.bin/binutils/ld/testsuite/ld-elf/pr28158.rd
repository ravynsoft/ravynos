#ld: -shared -version-script pr27128.t
#readelf: --dyn-syms --wide
#target: x86_64-*-linux* i?86-*-linux-gnu

#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +OBJECT +GLOBAL +DEFAULT +[1-9]+ foo@VERS_2.0 \([0-9]+\)
#pass
