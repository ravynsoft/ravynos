#source: dsov32-1.s
#source: dsov32-2.s
#source: dsofng.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -d 

# Complement to dso-pltdis1.d; merging the other .got.plt entry.
# Depending on reloc order, one of the tests would fail.

.*:     file format elf32-cris

Disassembly of section \.plt:

0+160 <.*>:

 160:	84e2                	subq 4,\$sp
 162:	0401                	addoq 4,\$r0,\$acr
 164:	7e7a                	move \$mof,\[\$sp\]
 166:	3f7a                	move \[\$acr\],\$mof
 168:	04f2                	addq 4,\$acr
 16a:	6ffa                	move\.d \[\$acr\],\$acr
 16c:	bf09                	jump \$acr
 16e:	b005                	nop 
	\.\.\.

0000017a <dsofn4@plt>:
 17a:	6f0d ..00 0000      	addo\.d .*
 180:	6ffa                	move\.d \[\$acr\],\$acr
 182:	bf09                	jump \$acr
 184:	b005                	nop 
 186:	3f7e .... ....      	move .*,\$mof
 18c:	bf0e .... ....      	ba .*
 192:	b005                	nop 
#...
 194:	6f0d ..00 0000      	addo\.d .*
 19a:	6ffa                	move\.d \[\$acr\],\$acr
 19c:	bf09                	jump \$acr
 19e:	b005                	nop 
 1a0:	3f7e .... ....      	move .*,\$mof
 1a6:	bf0e .... ....      	ba .*
 1ac:	b005                	nop 

Disassembly of section \.text:
#...
0+1ae <dsofn3>:
 1ae:	bfbe e6ff ffff      	bsr 194 <.*>
 1b4:	b005                	nop 

0+1b6 <dsofn4>:
 1b6:	7f0d ae20 0000      	lapc 2264 <_GLOBAL_OFFSET_TABLE_>,\$r0
 1bc:	5f0d ..00           	addo\.w 0x..,\$r0,\$acr
 1c0:	bfbe baff ffff      	bsr 17a <dsofn4@plt>
#pass
