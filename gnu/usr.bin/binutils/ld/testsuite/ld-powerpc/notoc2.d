#source: notoc2.s
#as: -a64 -mpower10
#ld: -shared -z norelro
#objdump: -d -Mpower10
#target: powerpc64*-*-*

.*

Disassembly of section \.text:

.* <.*\.plt_call\.puts>:
.*:	(04 10 00 01|01 00 10 04) 	pld     r12,66200	# 10418 \[puts@plt\]
.*:	(e5 80 02 98|98 02 80 e5) 
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
#...
.*:	(04 13 ff ff|ff ff 13 04) 	pld     r12,-1	# 1bf
.*:	(e5 80 ff ff|ff ff 80 e5) 
.*:	(04 10 00 00|00 00 10 04) 	pld     r12,0	# 1c8
.*:	(e5 80 00 00|00 00 80 e5) 
.*:	(06 13 ff ff|ff ff 13 06) 	pla     r12,-1	# 1cf
.*:	(39 80 ff ff|ff ff 80 39) 
.*:	(06 10 00 00|00 00 10 06) 	pla     r12,0	# 1d8
.*:	(39 80 00 00|00 00 80 39) 
.*:	(06 10 00 00|00 00 10 06) 	pla     r3,88	# 238 <hello>
.*:	(38 60 00 58|58 00 60 38) 
.*:	(4b ff ff 99|99 ff ff 4b) 	bl      .* <.*\.plt_call\.puts>
.*:	(60 00 00 00|00 00 00 60) 	nop
#pass
