# name: MVE tail predicated low-overhead loop instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> f000 c0a9 	wlstp.8	lr, r0, 00000154 <.label>
[^>]*> f001 c0a7 	wlstp.8	lr, r1, 00000154 <.label>
[^>]*> f002 c0a5 	wlstp.8	lr, r2, 00000154 <.label>
[^>]*> f004 c0a3 	wlstp.8	lr, r4, 00000154 <.label>
[^>]*> f007 c0a1 	wlstp.8	lr, r7, 00000154 <.label>
[^>]*> f008 c09f 	wlstp.8	lr, r8, 00000154 <.label>
[^>]*> f00a c09d 	wlstp.8	lr, sl, 00000154 <.label>
[^>]*> f00c c09b 	wlstp.8	lr, ip, 00000154 <.label>
[^>]*> f00e c099 	wlstp.8	lr, lr, 00000154 <.label>
[^>]*> f010 c097 	wlstp.16	lr, r0, 00000154 <.label>
[^>]*> f011 c095 	wlstp.16	lr, r1, 00000154 <.label>
[^>]*> f012 c093 	wlstp.16	lr, r2, 00000154 <.label>
[^>]*> f014 c091 	wlstp.16	lr, r4, 00000154 <.label>
[^>]*> f017 c08f 	wlstp.16	lr, r7, 00000154 <.label>
[^>]*> f018 c08d 	wlstp.16	lr, r8, 00000154 <.label>
[^>]*> f01a c08b 	wlstp.16	lr, sl, 00000154 <.label>
[^>]*> f01c c089 	wlstp.16	lr, ip, 00000154 <.label>
[^>]*> f01e c087 	wlstp.16	lr, lr, 00000154 <.label>
[^>]*> f020 c085 	wlstp.32	lr, r0, 00000154 <.label>
[^>]*> f021 c083 	wlstp.32	lr, r1, 00000154 <.label>
[^>]*> f022 c081 	wlstp.32	lr, r2, 00000154 <.label>
[^>]*> f024 c07f 	wlstp.32	lr, r4, 00000154 <.label>
[^>]*> f027 c07d 	wlstp.32	lr, r7, 00000154 <.label>
[^>]*> f028 c07b 	wlstp.32	lr, r8, 00000154 <.label>
[^>]*> f02a c079 	wlstp.32	lr, sl, 00000154 <.label>
[^>]*> f02c c077 	wlstp.32	lr, ip, 00000154 <.label>
[^>]*> f02e c075 	wlstp.32	lr, lr, 00000154 <.label>
[^>]*> f030 c073 	wlstp.64	lr, r0, 00000154 <.label>
[^>]*> f031 c071 	wlstp.64	lr, r1, 00000154 <.label>
[^>]*> f032 c06f 	wlstp.64	lr, r2, 00000154 <.label>
[^>]*> f034 c06d 	wlstp.64	lr, r4, 00000154 <.label>
[^>]*> f037 c06b 	wlstp.64	lr, r7, 00000154 <.label>
[^>]*> f038 c069 	wlstp.64	lr, r8, 00000154 <.label>
[^>]*> f03a c067 	wlstp.64	lr, sl, 00000154 <.label>
[^>]*> f03c c065 	wlstp.64	lr, ip, 00000154 <.label>
[^>]*> f03e c063 	wlstp.64	lr, lr, 00000154 <.label>
[^>]*> f000 e001 	dlstp.8	lr, r0
[^>]*> f001 e001 	dlstp.8	lr, r1
[^>]*> f002 e001 	dlstp.8	lr, r2
[^>]*> f004 e001 	dlstp.8	lr, r4
[^>]*> f007 e001 	dlstp.8	lr, r7
[^>]*> f008 e001 	dlstp.8	lr, r8
[^>]*> f00a e001 	dlstp.8	lr, sl
[^>]*> f00c e001 	dlstp.8	lr, ip
[^>]*> f00e e001 	dlstp.8	lr, lr
[^>]*> f010 e001 	dlstp.16	lr, r0
[^>]*> f011 e001 	dlstp.16	lr, r1
[^>]*> f012 e001 	dlstp.16	lr, r2
[^>]*> f014 e001 	dlstp.16	lr, r4
[^>]*> f017 e001 	dlstp.16	lr, r7
[^>]*> f018 e001 	dlstp.16	lr, r8
[^>]*> f01a e001 	dlstp.16	lr, sl
[^>]*> f01c e001 	dlstp.16	lr, ip
[^>]*> f01e e001 	dlstp.16	lr, lr
[^>]*> f020 e001 	dlstp.32	lr, r0
[^>]*> f021 e001 	dlstp.32	lr, r1
[^>]*> f022 e001 	dlstp.32	lr, r2
[^>]*> f024 e001 	dlstp.32	lr, r4
[^>]*> f027 e001 	dlstp.32	lr, r7
[^>]*> f028 e001 	dlstp.32	lr, r8
[^>]*> f02a e001 	dlstp.32	lr, sl
[^>]*> f02c e001 	dlstp.32	lr, ip
[^>]*> f02e e001 	dlstp.32	lr, lr
[^>]*> f030 e001 	dlstp.64	lr, r0
[^>]*> f031 e001 	dlstp.64	lr, r1
[^>]*> f032 e001 	dlstp.64	lr, r2
[^>]*> f034 e001 	dlstp.64	lr, r4
[^>]*> f037 e001 	dlstp.64	lr, r7
[^>]*> f038 e001 	dlstp.64	lr, r8
[^>]*> f03a e001 	dlstp.64	lr, sl
[^>]*> f03c e001 	dlstp.64	lr, ip
[^>]*> f03e e001 	dlstp.64	lr, lr
[^>]*> f00f c093 	le	lr, 00000000 <.label_back>
[^>]*> f02f c095 	le	00000000 <.label_back>
[^>]*> f01f c097 	letp	lr, 00000000 <.label_back>
[^>]*> f00f e001 	lctp
[^>]*> bf08      	it	eq
[^>]*> f00f e001 	lctpeq
[^>]*> bf18      	it	ne
[^>]*> f00f e001 	lctpne
[^>]*> bfc8      	it	gt
[^>]*> f00f e001 	lctpgt
[^>]*> bfa8      	it	ge
[^>]*> f00f e001 	lctpge
[^>]*> bfb8      	it	lt
[^>]*> f00f e001 	lctplt
[^>]*> bfd8      	it	le
[^>]*> f00f e001 	lctple
