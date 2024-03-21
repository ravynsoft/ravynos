#name: Valid Armv8.1-M Mainline FPCXT_NS and FPCXT_S register usage
#source: armv8_1-m-fpcxt-reg.s
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^*]+> ed6d cf81 	vstr	FPCXTNS, \[sp, #-4\]!
[^*]+> ed6d ef81 	vstr	FPCXTS, \[sp, #-4\]!
[^*]+> ed6d cf81 	vstr	FPCXTNS, \[sp, #-4\]!
[^*]+> ed6d ef81 	vstr	FPCXTS, \[sp, #-4\]!
[^*]+> ed6d cf81 	vstr	FPCXTNS, \[sp, #-4\]!
[^*]+> ed6d ef81 	vstr	FPCXTS, \[sp, #-4\]!
[^*]+> ed6d cf81 	vstr	FPCXTNS, \[sp, #-4\]!
[^*]+> ed6d ef81 	vstr	FPCXTS, \[sp, #-4\]!
0+.* <[^>]*> edd3 cf80 	vldr	FPCXTNS, \[r3\]
0+.* <[^>]*> edd3 cf80 	vldr	FPCXTNS, \[r3\]
0+.* <[^>]*> edd3 cf80 	vldr	FPCXTNS, \[r3\]
0+.* <[^>]*> edd3 cf80 	vldr	FPCXTNS, \[r3\]
0+.* <[^>]*> edd3 ef80 	vldr	FPCXTS, \[r3\]
0+.* <[^>]*> edd3 ef80 	vldr	FPCXTS, \[r3\]
0+.* <[^>]*> edd3 ef80 	vldr	FPCXTS, \[r3\]
0+.* <[^>]*> edd3 ef80 	vldr	FPCXTS, \[r3\]
[^*]+> eefe 4a10 	vmrs	r4, fpcxt_ns
[^*]+> eefe 4a10 	vmrs	r4, fpcxt_ns
[^*]+> eeff 5a10 	vmrs	r5, fpcxt_s
[^*]+> eeff 5a10 	vmrs	r5, fpcxt_s
[^*]+> eefe 4a10 	vmrs	r4, fpcxt_ns
[^*]+> eefe 4a10 	vmrs	r4, fpcxt_ns
[^*]+> eeff 5a10 	vmrs	r5, fpcxt_s
[^*]+> eeff 5a10 	vmrs	r5, fpcxt_s
[^*]+> eeee 4a10 	vmsr	fpcxt_ns, r4
[^*]+> eeee 4a10 	vmsr	fpcxt_ns, r4
[^*]+> eeef 5a10 	vmsr	fpcxt_s, r5
[^*]+> eeef 5a10 	vmsr	fpcxt_s, r5
[^*]+> eeee 4a10 	vmsr	fpcxt_ns, r4
[^*]+> eeee 4a10 	vmsr	fpcxt_ns, r4
[^*]+> eeef 5a10 	vmsr	fpcxt_s, r5
[^*]+> eeef 5a10 	vmsr	fpcxt_s, r5
