#as: -march=armv8-a
#source: sp-usage-thumb2-relax.s
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

.* <.*>:
.*:	f107 0d01 	add.w	sp, r7, #1
.*:	f1a7 0d01 	sub.w	sp, r7, #1
.*:	f207 0d01 	addw	sp, r7, #1
.*:	f2a7 0d01 	subw	sp, r7, #1
.*:	ea2d 0702 	bic.w	r7, sp, r2
.*:	eb7d 0702 	sbcs.w	r7, sp, r2
.*:	ea0d 0702 	and.w	r7, sp, r2
.*:	ea8d 0702 	eor.w	r7, sp, r2
.*:	fb1d dd0d 	smlabb	sp, sp, sp, sp
.*:	fb1d b003 	smlabb	r0, sp, r3, fp
.*:	fb1d dd2d 	smlatb	sp, sp, sp, sp
.*:	fb1d b023 	smlatb	r0, sp, r3, fp
.*:	fb1d dd1d 	smlabt	sp, sp, sp, sp
.*:	fb1d b013 	smlabt	r0, sp, r3, fp
.*:	fb1d dd3d 	smlatt	sp, sp, sp, sp
.*:	fb1d b033 	smlatt	r0, sp, r3, fp
