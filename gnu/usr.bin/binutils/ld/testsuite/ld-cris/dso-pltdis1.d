#source: dsov32-1.s
#source: dsov32-2.s
#source: dsofn4g.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -d -R

# Check dissassembly of the .plt section, specifically the synthetic
# symbols, in a DSO in which a .got.plt entry has been merged into a
# regular .got entry.  There was a bug in which some (i.e. subsequent
# with regards to reloc order) synthetic X@plt entries were wrong if
# there were merged .got entries present; dsofn4@plt below.  The
# alternatives in the matching regexps are placeholders for a future
# improvement: synthetic symbols for .plt entries with merged .got
# entries (lost as a consequence of the relocs no longer accounted for
# in .rela.plt and the default synthetic-symbol implementation just
# iterating over .rela.plt).

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
#...
 17a:	6f0d ..00 0000      	addo\.d .*
 180:	6ffa                	move\.d \[\$acr\],\$acr
 182:	bf09                	jump \$acr
 184:	b005                	nop 
 186:	3f7e .... ....      	move .*,\$mof
 18c:	bf0e .... ....      	ba .*
 192:	b005                	nop 

0+194 <dsofn@plt>:
 194:	6f0d ..00 0000      	addo\.d .*
 19a:	6ffa                	move\.d \[\$acr\],\$acr
 19c:	bf09                	jump \$acr
 19e:	b005                	nop 
 1a0:	3f7e .... ....      	move .*,\$mof
 1a6:	bf0e baff ffff      	ba 160 <.*>
 1ac:	b005                	nop 

Disassembly of section \.text:
#...
0+1ae <dsofn3>:
 1ae:	bfbe e6ff ffff      	bsr 194 <dsofn@plt>
 1b4:	b005                	nop 

0+1b6 <dsofn4>:
 1b6:	7f0d ae20 0000      	lapc 2264 <_GLOBAL_OFFSET_TABLE_>,\$r0
 1bc:	5f0d 1400           	addo\.w 0x14,\$r0,\$acr
 1c0:	bfbe baff ffff      	bsr 17a <.*>
#pass
