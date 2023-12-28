#source: relocs-1027-symbolic-func.s
#target: [check_shared_lib_support]
#ld: -shared -Bsymbolic-functions
#readelf: -r --wide
#...
.* +R_AARCH64_RELATIVE +.*
