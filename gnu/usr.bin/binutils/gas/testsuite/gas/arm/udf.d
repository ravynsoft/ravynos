#objdump: -dr --prefix-addresses --show-raw-insn
#name: UDF
#as: -march=armv8-a -mwarn-restrict-it
#warning_output: udf.l
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section \.text:
[^<]*<arm> e7f000f0 	udf	#0
[^<]*<arm\+0x4> e7fabcfd 	udf	#43981	@ 0xabcd
[^<]*<thumb> deab      	udf	#171	@ 0xab
[^<]*<thumb\+0x2> decd      	udf	#205	@ 0xcd
[^<]*<thumb\+0x4> de00      	udf	#0
[^<]*<thumb\+0x6> bf00      	nop
[^<]*<thumb\+0x8> f7f0 a000 	udf.w	#0
[^<]*<thumb\+0xc> f7f1 a234 	udf.w	#4660	@ 0x1234
[^<]*<thumb\+0x10> f7fc acdd 	udf.w	#52445	@ 0xccdd
[^<]*<thumb\+0x14> bf08      	it	eq
[^<]*<thumb\+0x16> de12      	udfeq	#18
[^<]*<thumb\+0x18> de23      	udf	#35	@ 0x23
[^<]*<thumb\+0x1a> de34      	udf	#52	@ 0x34
[^<]*<thumb\+0x1c> de56      	udf	#86	@ 0x56
[^<]*<thumb\+0x1e> bf18      	it	ne
[^<]*<thumb\+0x20> f7f1 a234 	udfne.w	#4660	@ 0x1234
[^<]*<thumb\+0x24> f7f2 a345 	udf.w	#9029	@ 0x2345
[^<]*<thumb\+0x28> f7f3 a456 	udf.w	#13398	@ 0x3456
[^<]*<thumb\+0x2c> f7f5 a678 	udf.w	#22136	@ 0x5678
