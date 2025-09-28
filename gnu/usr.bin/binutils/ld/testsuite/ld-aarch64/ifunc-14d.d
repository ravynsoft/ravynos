#source: ifunc-14b.s
#source: ifunc-14a.s
#target: [check_shared_lib_support]
#ld: -shared -z nocombreloc
#readelf: -r --wide

#failif
#...
.* +R_AARCH64_NONE +.*
#...
