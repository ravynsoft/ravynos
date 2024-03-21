#source: hash1.s
#readelf: -d -I
#ld: -nostdlib -shared --hash-style=gnu
#target: [check_shared_lib_support]
#xfail: mips*-*-irix*

#...
 +0x[0-9a-z]+ +\(MIPS_XHASH\) +0x[0-9a-z]+
#...
 +1 +1 +\( 50.0%\) +100.0%
#...
