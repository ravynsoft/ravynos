# name: MVE vctp instructions
# as: -march=armv8.1-m.main+mve
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> f000 e801 	vctp.8	r0
[^>]*> f010 e801 	vctp.16	r0
[^>]*> f020 e801 	vctp.32	r0
[^>]*> f030 e801 	vctp.64	r0
[^>]*> f001 e801 	vctp.8	r1
[^>]*> f011 e801 	vctp.16	r1
[^>]*> f021 e801 	vctp.32	r1
[^>]*> f031 e801 	vctp.64	r1
[^>]*> f002 e801 	vctp.8	r2
[^>]*> f012 e801 	vctp.16	r2
[^>]*> f022 e801 	vctp.32	r2
[^>]*> f032 e801 	vctp.64	r2
[^>]*> f004 e801 	vctp.8	r4
[^>]*> f014 e801 	vctp.16	r4
[^>]*> f024 e801 	vctp.32	r4
[^>]*> f034 e801 	vctp.64	r4
[^>]*> f008 e801 	vctp.8	r8
[^>]*> f018 e801 	vctp.16	r8
[^>]*> f028 e801 	vctp.32	r8
[^>]*> f038 e801 	vctp.64	r8
[^>]*> fe71 0f4d 	vpst
[^>]*> f000 e801 	vctpt.8	r0
[^>]*> fe71 0f4d 	vpst
[^>]*> f010 e801 	vctpt.16	r0
[^>]*> fe71 0f4d 	vpst
[^>]*> f020 e801 	vctpt.32	r0
[^>]*> fe71 0f4d 	vpst
[^>]*> f030 e801 	vctpt.64	r0
[^>]*> fe71 0f4d 	vpst
[^>]*> f001 e801 	vctpt.8	r1
[^>]*> fe71 0f4d 	vpst
[^>]*> f011 e801 	vctpt.16	r1
[^>]*> fe71 0f4d 	vpst
[^>]*> f021 e801 	vctpt.32	r1
[^>]*> fe71 0f4d 	vpst
[^>]*> f031 e801 	vctpt.64	r1
[^>]*> fe71 0f4d 	vpst
[^>]*> f002 e801 	vctpt.8	r2
[^>]*> fe71 0f4d 	vpst
[^>]*> f012 e801 	vctpt.16	r2
[^>]*> fe71 0f4d 	vpst
[^>]*> f022 e801 	vctpt.32	r2
[^>]*> fe71 0f4d 	vpst
[^>]*> f032 e801 	vctpt.64	r2
[^>]*> fe71 0f4d 	vpst
[^>]*> f004 e801 	vctpt.8	r4
[^>]*> fe71 0f4d 	vpst
[^>]*> f014 e801 	vctpt.16	r4
[^>]*> fe71 0f4d 	vpst
[^>]*> f024 e801 	vctpt.32	r4
[^>]*> fe71 0f4d 	vpst
[^>]*> f034 e801 	vctpt.64	r4
[^>]*> fe71 0f4d 	vpst
[^>]*> f008 e801 	vctpt.8	r8
[^>]*> fe71 0f4d 	vpst
[^>]*> f018 e801 	vctpt.16	r8
[^>]*> fe71 0f4d 	vpst
[^>]*> f028 e801 	vctpt.32	r8
[^>]*> fe71 0f4d 	vpst
[^>]*> f038 e801 	vctpt.64	r8
