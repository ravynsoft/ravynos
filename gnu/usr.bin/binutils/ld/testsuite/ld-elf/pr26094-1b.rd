#ld:
#readelf: --dyn-syms --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support]

#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +OBJECT +GLOBAL +PROTECTED +[0-9]+ +___?start_FOO@@SOME_VERSION_NAME
#pass
