#source: expdyn1.s
#source: dsov32-1.s
#source: dsov32-2.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux -z nocombreloc --hash-style=sysv
#objdump: -d 

# Check dissassembly of .plt section.

.*:     file format elf32-cris

Disassembly of section \.plt:

0+198 <.plt>:

 198:	84e2                	subq 4,\$sp
 19a:	0401                	addoq 4,\$r0,\$acr
 19c:	7e7a                	move \$mof,\[\$sp\]
 19e:	3f7a                	move \[\$acr\],\$mof
 1a0:	04f2                	addq 4,\$acr
 1a2:	6ffa                	move\.d \[\$acr\],\$acr
 1a4:	bf09                	jump \$acr
 1a6:	b005                	nop 
	\.\.\.

0+1b2 <dsofn4@plt>:
 1b2:	6f0d 0c00 0000      	addo\.d c <.*>,\$r0,\$acr
 1b8:	6ffa                	move\.d \[\$acr\],\$acr
 1ba:	bf09                	jump \$acr
 1bc:	b005                	nop 
 1be:	3f7e 0000 0000      	move 0 <.*>,\$mof
 1c4:	bf0e d4ff ffff      	ba 198 <.*>
 1ca:	b005                	nop 

0+1cc <dsofn@plt>:
 1cc:	6f0d 1000 0000      	addo\.d 10 <.*>,\$r0,\$acr
 1d2:	6ffa                	move\.d \[\$acr\],\$acr
 1d4:	bf09                	jump \$acr
 1d6:	b005                	nop 
 1d8:	3f7e 0c00 0000      	move c <.*>,\$mof
 1de:	bf0e baff ffff      	ba 198 <.*>
 1e4:	b005                	nop 

Disassembly of section \.text:
#...
0+1ea <dsofn3>:
 1ea:	bfbe e2ff ffff      	bsr 1cc <dsofn@plt>
 1f0:	b005                	nop 

0+1f2 <dsofn4>:
 1f2:	7f0d a620 0000      	lapc 2298 <_GLOBAL_OFFSET_TABLE_>,\$r0
 1f8:	5f0d 1400           	addo\.w 0x14,\$r0,\$acr
 1fc:	bfbe b6ff ffff      	bsr 1b2 <dsofn4@plt>
#pass
