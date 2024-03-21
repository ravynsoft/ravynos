#objdump: -dr
#as: -march=armv8.2-a

.*:     file format .*

Disassembly of section .*:

.* <.*>:
   [0-9a-f]+:	b34107e0 	bfxil	x0, xzr, #1, #1
   [0-9a-f]+:	b3410420 	bfxil	x0, x1, #1, #1
   [0-9a-f]+:	b341043f 	bfxil	xzr, x1, #1, #1
   [0-9a-f]+:	b34107ff 	bfxil	xzr, xzr, #1, #1
  [0-9a-f]+:	b34123e0 	bfxil	x0, xzr, #1, #8
  [0-9a-f]+:	b3412020 	bfxil	x0, x1, #1, #8
  [0-9a-f]+:	b341203f 	bfxil	xzr, x1, #1, #8
  [0-9a-f]+:	b34123ff 	bfxil	xzr, xzr, #1, #8
  [0-9a-f]+:	b3413fe0 	bfxil	x0, xzr, #1, #15
  [0-9a-f]+:	b3413c20 	bfxil	x0, x1, #1, #15
  [0-9a-f]+:	b3413c3f 	bfxil	xzr, x1, #1, #15
  [0-9a-f]+:	b3413fff 	bfxil	xzr, xzr, #1, #15
  [0-9a-f]+:	b35007e0 	bfc	x0, #48, #2
  [0-9a-f]+:	b3500420 	bfi	x0, x1, #48, #2
  [0-9a-f]+:	b350043f 	bfi	xzr, x1, #48, #2
  [0-9a-f]+:	b35007ff 	bfc	xzr, #48, #2
  [0-9a-f]+:	b35023e0 	bfc	x0, #48, #9
  [0-9a-f]+:	b3502020 	bfi	x0, x1, #48, #9
  [0-9a-f]+:	b350203f 	bfi	xzr, x1, #48, #9
  [0-9a-f]+:	b35023ff 	bfc	xzr, #48, #9
  [0-9a-f]+:	b3503fe0 	bfc	x0, #48, #16
  [0-9a-f]+:	b3503c20 	bfi	x0, x1, #48, #16
  [0-9a-f]+:	b3503c3f 	bfi	xzr, x1, #48, #16
  [0-9a-f]+:	b3503fff 	bfc	xzr, #48, #16
  [0-9a-f]+:	b35f07e0 	bfc	x0, #33, #2
  [0-9a-f]+:	b35f0420 	bfi	x0, x1, #33, #2
  [0-9a-f]+:	b35f043f 	bfi	xzr, x1, #33, #2
  [0-9a-f]+:	b35f07ff 	bfc	xzr, #33, #2
  [0-9a-f]+:	b35f23e0 	bfc	x0, #33, #9
  [0-9a-f]+:	b35f2020 	bfi	x0, x1, #33, #9
  [0-9a-f]+:	b35f203f 	bfi	xzr, x1, #33, #9
  [0-9a-f]+:	b35f23ff 	bfc	xzr, #33, #9
  [0-9a-f]+:	b35f3fe0 	bfc	x0, #33, #16
  [0-9a-f]+:	b35f3c20 	bfi	x0, x1, #33, #16
  [0-9a-f]+:	b35f3c3f 	bfi	xzr, x1, #33, #16
  [0-9a-f]+:	b35f3fff 	bfc	xzr, #33, #16
  [0-9a-f]+:	b37f03e0 	bfc	x0, #1, #1
  [0-9a-f]+:	b37f0020 	bfi	x0, x1, #1, #1
  [0-9a-f]+:	b37f003f 	bfi	xzr, x1, #1, #1
  [0-9a-f]+:	b37f03ff 	bfc	xzr, #1, #1
  [0-9a-f]+:	b37f1fe0 	bfc	x0, #1, #8
  [0-9a-f]+:	b37f1c20 	bfi	x0, x1, #1, #8
  [0-9a-f]+:	b37f1c3f 	bfi	xzr, x1, #1, #8
  [0-9a-f]+:	b37f1fff 	bfc	xzr, #1, #8
  [0-9a-f]+:	b37f3be0 	bfc	x0, #1, #15
  [0-9a-f]+:	b37f3820 	bfi	x0, x1, #1, #15
  [0-9a-f]+:	b37f383f 	bfi	xzr, x1, #1, #15
  [0-9a-f]+:	b37f3bff 	bfc	xzr, #1, #15
  [0-9a-f]+:	b37003e0 	bfc	x0, #16, #1
  [0-9a-f]+:	b3700020 	bfi	x0, x1, #16, #1
  [0-9a-f]+:	b370003f 	bfi	xzr, x1, #16, #1
  [0-9a-f]+:	b37003ff 	bfc	xzr, #16, #1
  [0-9a-f]+:	b3701fe0 	bfc	x0, #16, #8
  [0-9a-f]+:	b3701c20 	bfi	x0, x1, #16, #8
  [0-9a-f]+:	b3701c3f 	bfi	xzr, x1, #16, #8
  [0-9a-f]+:	b3701fff 	bfc	xzr, #16, #8
  [0-9a-f]+:	b3703be0 	bfc	x0, #16, #15
  [0-9a-f]+:	b3703820 	bfi	x0, x1, #16, #15
  [0-9a-f]+:	b370383f 	bfi	xzr, x1, #16, #15
  [0-9a-f]+:	b3703bff 	bfc	xzr, #16, #15
  [0-9a-f]+:	b36103e0 	bfc	x0, #31, #1
  [0-9a-f]+:	b3610020 	bfi	x0, x1, #31, #1
  [0-9a-f]+:	b361003f 	bfi	xzr, x1, #31, #1
  [0-9a-f]+:	b36103ff 	bfc	xzr, #31, #1
 [0-9a-f]+:	b3611fe0 	bfc	x0, #31, #8
 [0-9a-f]+:	b3611c20 	bfi	x0, x1, #31, #8
 [0-9a-f]+:	b3611c3f 	bfi	xzr, x1, #31, #8
 [0-9a-f]+:	b3611fff 	bfc	xzr, #31, #8
 [0-9a-f]+:	b3613be0 	bfc	x0, #31, #15
 [0-9a-f]+:	b3613820 	bfi	x0, x1, #31, #15
 [0-9a-f]+:	b361383f 	bfi	xzr, x1, #31, #15
 [0-9a-f]+:	b3613bff 	bfc	xzr, #31, #15
 [0-9a-f]+:	b37f03e0 	bfc	x0, #1, #1
 [0-9a-f]+:	b37f03ff 	bfc	xzr, #1, #1
 [0-9a-f]+:	b37f1fe0 	bfc	x0, #1, #8
 [0-9a-f]+:	b37f1fff 	bfc	xzr, #1, #8
 [0-9a-f]+:	b37f3be0 	bfc	x0, #1, #15
 [0-9a-f]+:	b37f3bff 	bfc	xzr, #1, #15
 [0-9a-f]+:	b37003e0 	bfc	x0, #16, #1
 [0-9a-f]+:	b37003ff 	bfc	xzr, #16, #1
 [0-9a-f]+:	b3701fe0 	bfc	x0, #16, #8
 [0-9a-f]+:	b3701fff 	bfc	xzr, #16, #8
 [0-9a-f]+:	b3703be0 	bfc	x0, #16, #15
 [0-9a-f]+:	b3703bff 	bfc	xzr, #16, #15
 [0-9a-f]+:	b36103e0 	bfc	x0, #31, #1
 [0-9a-f]+:	b36103ff 	bfc	xzr, #31, #1
 [0-9a-f]+:	b3611fe0 	bfc	x0, #31, #8
 [0-9a-f]+:	b3611fff 	bfc	xzr, #31, #8
 [0-9a-f]+:	b3613be0 	bfc	x0, #31, #15
 [0-9a-f]+:	b3613bff 	bfc	xzr, #31, #15
 [0-9a-f]+:	dac00fe0 	rev	x0, xzr
 [0-9a-f]+:	dac00c20 	rev	x0, x1
 [0-9a-f]+:	dac00c3f 	rev	xzr, x1
 [0-9a-f]+:	dac00fff 	rev	xzr, xzr
 [0-9a-f]+:	dac007e0 	rev16	x0, xzr
 [0-9a-f]+:	dac00420 	rev16	x0, x1
 [0-9a-f]+:	dac0043f 	rev16	xzr, x1
 [0-9a-f]+:	dac007ff 	rev16	xzr, xzr
 [0-9a-f]+:	dac00fe0 	rev	x0, xzr
 [0-9a-f]+:	dac00c20 	rev	x0, x1
 [0-9a-f]+:	dac00c3f 	rev	xzr, x1
 [0-9a-f]+:	dac00fff 	rev	xzr, xzr
