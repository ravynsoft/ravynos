#as: -march=armv8.2-a+fp16
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   [0-9a-f]+:	1e200400 	fccmp	s0, s0, #0x0, eq	// eq = none
   [0-9a-f]+:	1ee00400 	fccmp	h0, h0, #0x0, eq	// eq = none
   [0-9a-f]+:	1e22d420 	fccmp	s1, s2, #0x0, le
   [0-9a-f]+:	1ee2d420 	fccmp	h1, h2, #0x0, le
  [0-9a-f]+:	1e200410 	fccmpe	s0, s0, #0x0, eq	// eq = none
  [0-9a-f]+:	1ee00410 	fccmpe	h0, h0, #0x0, eq	// eq = none
  [0-9a-f]+:	1e22d430 	fccmpe	s1, s2, #0x0, le
  [0-9a-f]+:	1ee2d430 	fccmpe	h1, h2, #0x0, le
  [0-9a-f]+:	1e202000 	fcmp	s0, s0
  [0-9a-f]+:	1ee02000 	fcmp	h0, h0
  [0-9a-f]+:	1e222020 	fcmp	s1, s2
  [0-9a-f]+:	1ee22020 	fcmp	h1, h2
  [0-9a-f]+:	1e202010 	fcmpe	s0, s0
  [0-9a-f]+:	1ee02010 	fcmpe	h0, h0
  [0-9a-f]+:	1e222030 	fcmpe	s1, s2
  [0-9a-f]+:	1ee22030 	fcmpe	h1, h2
  [0-9a-f]+:	1e202008 	fcmp	s0, #0\.0
  [0-9a-f]+:	1ee02008 	fcmp	h0, #0\.0
  [0-9a-f]+:	1e202018 	fcmpe	s0, #0\.0
  [0-9a-f]+:	1ee02018 	fcmpe	h0, #0\.0
  [0-9a-f]+:	1e210c00 	fcsel	s0, s0, s1, eq	// eq = none
  [0-9a-f]+:	1ee10c00 	fcsel	h0, h0, h1, eq	// eq = none
  [0-9a-f]+:	9ee60000 	fmov	x0, h0
  [0-9a-f]+:	1ee60000 	fmov	w0, h0
  [0-9a-f]+:	9ee70001 	fmov	h1, x0
  [0-9a-f]+:	1ee70001 	fmov	h1, w0
  [0-9a-f]+:	1ee0c020 	fabs	h0, h1
  [0-9a-f]+:	1e20c020 	fabs	s0, s1
  [0-9a-f]+:	1e60c020 	fabs	d0, d1
  [0-9a-f]+:	1ee14020 	fneg	h0, h1
  [0-9a-f]+:	1e214020 	fneg	s0, s1
  [0-9a-f]+:	1e614020 	fneg	d0, d1
  [0-9a-f]+:	1ee1c020 	fsqrt	h0, h1
  [0-9a-f]+:	1e21c020 	fsqrt	s0, s1
  [0-9a-f]+:	1e61c020 	fsqrt	d0, d1
  [0-9a-f]+:	1ee44020 	frintn	h0, h1
  [0-9a-f]+:	1e244020 	frintn	s0, s1
  [0-9a-f]+:	1e644020 	frintn	d0, d1
  [0-9a-f]+:	1ee4c020 	frintp	h0, h1
  [0-9a-f]+:	1e24c020 	frintp	s0, s1
  [0-9a-f]+:	1e64c020 	frintp	d0, d1
  [0-9a-f]+:	1ee54020 	frintm	h0, h1
  [0-9a-f]+:	1e254020 	frintm	s0, s1
  [0-9a-f]+:	1e654020 	frintm	d0, d1
  [0-9a-f]+:	1ee5c020 	frintz	h0, h1
  [0-9a-f]+:	1e25c020 	frintz	s0, s1
  [0-9a-f]+:	1e65c020 	frintz	d0, d1
  [0-9a-f]+:	1ee64020 	frinta	h0, h1
  [0-9a-f]+:	1e264020 	frinta	s0, s1
  [0-9a-f]+:	1e664020 	frinta	d0, d1
  [0-9a-f]+:	1ee74020 	frintx	h0, h1
  [0-9a-f]+:	1e274020 	frintx	s0, s1
  [0-9a-f]+:	1e674020 	frintx	d0, d1
  [0-9a-f]+:	1ee7c020 	frinti	h0, h1
  [0-9a-f]+:	1e27c020 	frinti	s0, s1
  [0-9a-f]+:	1e67c020 	frinti	d0, d1
  [0-9a-f]+:	1ee20820 	fmul	h0, h1, h2
  [0-9a-f]+:	1e220820 	fmul	s0, s1, s2
  [0-9a-f]+:	1e620820 	fmul	d0, d1, d2
  [0-9a-f]+:	1ee21820 	fdiv	h0, h1, h2
  [0-9a-f]+:	1e221820 	fdiv	s0, s1, s2
  [0-9a-f]+:	1e621820 	fdiv	d0, d1, d2
  [0-9a-f]+:	1ee22820 	fadd	h0, h1, h2
  [0-9a-f]+:	1e222820 	fadd	s0, s1, s2
 [0-9a-f]+:	1e622820 	fadd	d0, d1, d2
 [0-9a-f]+:	1ee23820 	fsub	h0, h1, h2
 [0-9a-f]+:	1e223820 	fsub	s0, s1, s2
 [0-9a-f]+:	1e623820 	fsub	d0, d1, d2
 [0-9a-f]+:	1ee24820 	fmax	h0, h1, h2
 [0-9a-f]+:	1e224820 	fmax	s0, s1, s2
 [0-9a-f]+:	1e624820 	fmax	d0, d1, d2
 [0-9a-f]+:	1ee25820 	fmin	h0, h1, h2
 [0-9a-f]+:	1e225820 	fmin	s0, s1, s2
 [0-9a-f]+:	1e625820 	fmin	d0, d1, d2
 [0-9a-f]+:	1ee26820 	fmaxnm	h0, h1, h2
 [0-9a-f]+:	1e226820 	fmaxnm	s0, s1, s2
 [0-9a-f]+:	1e626820 	fmaxnm	d0, d1, d2
 [0-9a-f]+:	1ee27820 	fminnm	h0, h1, h2
 [0-9a-f]+:	1e227820 	fminnm	s0, s1, s2
 [0-9a-f]+:	1e627820 	fminnm	d0, d1, d2
 [0-9a-f]+:	1ee28820 	fnmul	h0, h1, h2
 [0-9a-f]+:	1e228820 	fnmul	s0, s1, s2
 [0-9a-f]+:	1e628820 	fnmul	d0, d1, d2
 [0-9a-f]+:	1fc20c20 	fmadd	h0, h1, h2, h3
 [0-9a-f]+:	1f020c20 	fmadd	s0, s1, s2, s3
 [0-9a-f]+:	1f420c20 	fmadd	d0, d1, d2, d3
 [0-9a-f]+:	1fc28c20 	fmsub	h0, h1, h2, h3
 [0-9a-f]+:	1f028c20 	fmsub	s0, s1, s2, s3
 [0-9a-f]+:	1f428c20 	fmsub	d0, d1, d2, d3
 [0-9a-f]+:	1fe20c20 	fnmadd	h0, h1, h2, h3
 [0-9a-f]+:	1f220c20 	fnmadd	s0, s1, s2, s3
 [0-9a-f]+:	1f620c20 	fnmadd	d0, d1, d2, d3
 [0-9a-f]+:	1fe28c20 	fnmsub	h0, h1, h2, h3
 [0-9a-f]+:	1f228c20 	fnmsub	s0, s1, s2, s3
 [0-9a-f]+:	1f628c20 	fnmsub	d0, d1, d2, d3
 [0-9a-f]+:	1e2e1000 	fmov	s0, #1\.000000000000000000e\+00
 [0-9a-f]+:	1eee1000 	fmov	h0, #1\.000000000000000000e\+00
 [0-9a-f]+:	1e02f820 	scvtf	s0, w1, #2
 [0-9a-f]+:	9e02f420 	scvtf	s0, x1, #3
 [0-9a-f]+:	1ec2f820 	scvtf	h0, w1, #2
 [0-9a-f]+:	9ec2f420 	scvtf	h0, x1, #3
 [0-9a-f]+:	1e03f820 	ucvtf	s0, w1, #2
 [0-9a-f]+:	9e03f420 	ucvtf	s0, x1, #3
 [0-9a-f]+:	1ec3f820 	ucvtf	h0, w1, #2
 [0-9a-f]+:	9ec3f420 	ucvtf	h0, x1, #3
 [0-9a-f]+:	1e58f801 	fcvtzs	w1, d0, #2
 [0-9a-f]+:	9e58f401 	fcvtzs	x1, d0, #3
 [0-9a-f]+:	1ed8f801 	fcvtzs	w1, h0, #2
 [0-9a-f]+:	9ed8f401 	fcvtzs	x1, h0, #3
 [0-9a-f]+:	1e59f801 	fcvtzu	w1, d0, #2
 [0-9a-f]+:	9e59f401 	fcvtzu	x1, d0, #3
 [0-9a-f]+:	1ed9f801 	fcvtzu	w1, h0, #2
 [0-9a-f]+:	9ed9f401 	fcvtzu	x1, h0, #3
 [0-9a-f]+:	1e200001 	fcvtns	w1, s0
 [0-9a-f]+:	9e600001 	fcvtns	x1, d0
 [0-9a-f]+:	1ee00001 	fcvtns	w1, h0
 [0-9a-f]+:	9ee00001 	fcvtns	x1, h0
 [0-9a-f]+:	1e210001 	fcvtnu	w1, s0
 [0-9a-f]+:	9e610001 	fcvtnu	x1, d0
 [0-9a-f]+:	1ee10001 	fcvtnu	w1, h0
 [0-9a-f]+:	9ee10001 	fcvtnu	x1, h0
 [0-9a-f]+:	1e250001 	fcvtau	w1, s0
 [0-9a-f]+:	9e650001 	fcvtau	x1, d0
 [0-9a-f]+:	1ee50001 	fcvtau	w1, h0
 [0-9a-f]+:	9ee50001 	fcvtau	x1, h0
 [0-9a-f]+:	1e240001 	fcvtas	w1, s0
 [0-9a-f]+:	9e640001 	fcvtas	x1, d0
 [0-9a-f]+:	1ee40001 	fcvtas	w1, h0
 [0-9a-f]+:	9ee40001 	fcvtas	x1, h0
 [0-9a-f]+:	1e280001 	fcvtps	w1, s0
 [0-9a-f]+:	9e680001 	fcvtps	x1, d0
 [0-9a-f]+:	1ee80001 	fcvtps	w1, h0
 [0-9a-f]+:	9ee80001 	fcvtps	x1, h0
 [0-9a-f]+:	1e290001 	fcvtpu	w1, s0
 [0-9a-f]+:	9e690001 	fcvtpu	x1, d0
 [0-9a-f]+:	1ee90001 	fcvtpu	w1, h0
 [0-9a-f]+:	9ee90001 	fcvtpu	x1, h0
 [0-9a-f]+:	1e300001 	fcvtms	w1, s0
 [0-9a-f]+:	9e700001 	fcvtms	x1, d0
 [0-9a-f]+:	1ef00001 	fcvtms	w1, h0
 [0-9a-f]+:	9ef00001 	fcvtms	x1, h0
 [0-9a-f]+:	1e310001 	fcvtmu	w1, s0
 [0-9a-f]+:	9e710001 	fcvtmu	x1, d0
 [0-9a-f]+:	1ef10001 	fcvtmu	w1, h0
 [0-9a-f]+:	9ef10001 	fcvtmu	x1, h0
 [0-9a-f]+:	1e220020 	scvtf	s0, w1
 [0-9a-f]+:	9e620020 	scvtf	d0, x1
 [0-9a-f]+:	1ee20020 	scvtf	h0, w1
 [0-9a-f]+:	9ee20020 	scvtf	h0, x1
 [0-9a-f]+:	1e230020 	ucvtf	s0, w1
 [0-9a-f]+:	9e630020 	ucvtf	d0, x1
 [0-9a-f]+:	1ee30020 	ucvtf	h0, w1
 [0-9a-f]+:	9ee30020 	ucvtf	h0, x1
 [0-9a-f]+:	1e604020 	fmov	d0, d1
 [0-9a-f]+:	1e204020 	fmov	s0, s1
 [0-9a-f]+:	1ee04020 	fmov	h0, h1
 [0-9a-f]+:	9ee60020 	fmov	x0, h1
 [0-9a-f]+:	1ee60020 	fmov	w0, h1
 [0-9a-f]+:	9ee70001 	fmov	h1, x0
 [0-9a-f]+:	1ee70001 	fmov	h1, w0
 [0-9a-f]+:	1e260020 	fmov	w0, s1
 [0-9a-f]+:	9e660020 	fmov	x0, d1
 [0-9a-f]+:	1e270001 	fmov	s1, w0
 [0-9a-f]+:	9e670001 	fmov	d1, x0
