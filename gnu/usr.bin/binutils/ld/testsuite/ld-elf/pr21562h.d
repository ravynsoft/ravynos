#source: pr21562b.s
#ld: -shared -z defs -T pr21562a.t
#readelf: -s -S --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-* ![check_shared_lib_support] 

#...
  \[[ 0-9]+\] scnfoo[ \t]+PROGBITS[ \t]+[0-9a-f]+ +[0-9a-f]+ +0*10[ \t]+.*
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +NOTYPE +GLOBAL +PROTECTED +[0-9]+ +___?stop_scnfoo
#pass
