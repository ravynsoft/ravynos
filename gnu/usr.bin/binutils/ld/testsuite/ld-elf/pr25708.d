#source: pr13195.s
#ld: -shared -version-script pr13195.t
#nm: -D --with-symbol-versions
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: hppa64-*-* ![check_shared_lib_support] 
# h8300 doesn't support -shared, and hppa64 creates .foo

#..
0+ A VERS_2.0
[0-9a-f]+ T foo@@VERS_2.0
#pass
