#source: pr19162a.s
#source: pr19162b.s
#ld: -shared -z max-page-size=0x200000 -z noseparate-code
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* *-*-nacl* arm*-*-uclinuxfdpiceabi
#xfail: hppa*-*-* ![check_shared_lib_support] 
# hppa fails due to PR 12376.

#...
 +LOAD +0x0+[0-9a-f]{3} +0x[0-9a-f]+ +0x[0-9a-f]+ +0x0+[0-9a-f]{3} +0x0+[0-9a-f]{3} +R[WE]+ +0x200000
#pass
