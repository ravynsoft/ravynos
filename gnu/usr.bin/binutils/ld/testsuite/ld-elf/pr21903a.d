#source: pr21903.s
#ld: -shared --no-define-common
#readelf: --dyn-syms
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
.*: 0+0 +0 +OBJECT +GLOBAL +DEFAULT +UND foo
#pass
