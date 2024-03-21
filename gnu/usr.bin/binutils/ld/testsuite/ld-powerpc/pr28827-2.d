#as: -a64
#ld: -melf64ppc --plt-align=0 -T pr28827-2.lnk
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

.*:
.*:	(49 ff ff f0|f0 ff ff 49) 	b       .* <far1>
	\.\.\.

.* <.*\.plt_branch\..*>:
.*:	(e9 82 80 28|28 80 82 e9) 	ld      r12,-32728\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.* <_start>:
.*:	(49 ff ff d8|d8 ff ff 49) 	b       .* <far1>
.*:	(4b ff ff f0|f0 ff ff 4b) 	b       .*

Disassembly of section \.far1:

.*:
.*:	(4a 00 00 38|38 00 00 4a) 	b       .* <_start>

.* <.*\.long_branch\..*>:
.*:	(49 ff ff f4|f4 ff ff 49) 	b       .* <far2>
	\.\.\.

.* <far1>:
.*:	(41 82 ff f4|f4 ff 82 41) 	beq     .*
.*:	(4a 00 00 24|24 00 00 4a) 	b       .* <_start>

Disassembly of section \.far2:

.*:
.*:	(e9 82 80 20|20 80 82 e9) 	ld      r12,-32736\(r2\)
.*:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
.*:	(4e 80 04 20|20 04 80 4e) 	bctr

.*:
.*:	(4a 00 00 24|24 00 00 4a) 	b       .* <far1>
	\.\.\.

.* <far2>:
.*:	(40 82 ff f4|f4 ff 82 40) 	bne     .*
.*:	(4b ff ff e4|e4 ff ff 4b) 	b       .*
