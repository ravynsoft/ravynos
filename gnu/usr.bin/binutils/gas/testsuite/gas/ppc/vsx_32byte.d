#as: -mpower10
#objdump: -dr -Mpower10
#name: VSX 32-byte loads and stores

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(18 5f 00 00|00 00 5f 18) 	lxvp    vs2,0\(r31\)
.*:	(1b e0 ff f0|f0 ff e0 1b) 	lxvp    vs62,-16\(0\)
.*:	(04 00 00 00|00 00 00 04) 	plxvp   vs4,1\(r30\)
.*:	(e8 9e 00 01|01 00 9e e8) 
.*:	(04 03 ff ff|ff ff 03 04) 	plxvp   vs60,-1\(r9\)
.*:	(eb a9 ff ff|ff ff a9 eb) 
.*:	(04 10 12 34|34 12 10 04) 	plxvp   vs6,305419896	# 12345690
.*:	(e8 c0 56 78|78 56 c0 e8) 
.*:	(04 13 ff ff|ff ff 13 04) 	plxvp   vs58,-32	# 0 <_start>
.*:	(eb 60 ff e0|e0 ff 60 eb) 
.*:	(7f 20 0a 9a|9a 0a 20 7f) 	lxvpx   vs56,0,r1
.*:	(19 1d 00 01|01 00 1d 19) 	stxvp   vs8,0\(r29\)
.*:	(1a e0 ff f1|f1 ff e0 1a) 	stxvp   vs54,-16\(0\)
.*:	(04 00 00 00|00 00 00 04) 	pstxvp  vs10,1\(r28\)
.*:	(f9 5c 00 01|01 00 5c f9) 
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(04 03 ff ff|ff ff 03 04) 	pstxvp  vs52,-1\(r8\)
.*:	(fa a8 ff ff|ff ff a8 fa) 
.*:	(04 10 12 34|34 12 10 04) 	pstxvp  vs12,305419896	# 123456c0
.*:	(f9 80 56 78|78 56 80 f9) 
.*:	(04 13 ff ff|ff ff 13 04) 	pstxvp  vs50,-80	# 0 <_start>
.*:	(fa 60 ff b0|b0 ff 60 fa) 
.*:	(7e 20 0b 9a|9a 0b 20 7e) 	stxvpx  vs48,0,r1
