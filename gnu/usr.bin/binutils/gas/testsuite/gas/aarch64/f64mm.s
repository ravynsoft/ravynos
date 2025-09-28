/* The instructions with non-zero register numbers are there to ensure we have
   the correct argument positioning (i.e. check that the first argument is at
   the end of the word etc).
   The instructions with all-zero register numbers are to ensure the previous
   encoding didn't just "happen" to fit -- so that if we change the registers
   that changes the correct part of the word.
   Each of the numbered patterns begin and end with a 1, so we can replace
   them with all-zeros and see the entire range has changed. */

// SVE
fmmla	z17.d,  z21.d,  z27.d
fmmla	z0.d,  z0.d,  z0.d

ld1rob { z17.b }, p5/z, [sp, x27]
ld1rob { z0.b }, p0/z, [sp, x0]
ld1roh { z17.h }, p5/z, [sp, x27, lsl #1]
ld1roh { z0.h }, p0/z, [sp, x0, lsl #1]
ld1row { z17.s }, p5/z, [sp, x27, lsl #2]
ld1row { z0.s }, p0/z, [sp, x0, lsl #2]
ld1rod { z17.d }, p5/z, [sp, x27, lsl #3]
ld1rod { z0.d }, p0/z, [sp, x0, lsl #3]

ld1rob { z17.b }, p5/z, [x0, x27]
ld1rob { z0.b }, p0/z, [x0, x0]
ld1roh { z17.h }, p5/z, [x0, x27, lsl #1]
ld1roh { z0.h }, p0/z, [x0, x0, lsl #1]
ld1row { z17.s }, p5/z, [x0, x27, lsl #2]
ld1row { z0.s }, p0/z, [x0, x0, lsl #2]
ld1rod { z17.d }, p5/z, [x0, x27, lsl #3]
ld1rod { z0.d }, p0/z, [x0, x0, lsl #3]

ld1rob { z17.b }, p5/z, [sp, #0]
ld1rob { z0.b }, p0/z, [sp, #224]
ld1rob { z0.b }, p0/z, [sp, #-256]
ld1roh { z17.h }, p5/z, [sp, #0]
ld1roh { z0.h }, p0/z, [sp, #224]
ld1roh { z0.h }, p0/z, [sp, #-256]
ld1row { z17.s }, p5/z, [sp, #0]
ld1row { z0.s }, p0/z, [sp, #224]
ld1row { z0.s }, p0/z, [sp, #-256]
ld1rod { z17.d }, p5/z, [sp, #0]
ld1rod { z0.d }, p0/z, [sp, #224]
ld1rod { z0.d }, p0/z, [sp, #-256]

ld1rob { z17.b }, p5/z, [x0, #0]
ld1rob { z0.b }, p0/z, [x0, #224]
ld1rob { z0.b }, p0/z, [x0, #-256]
ld1roh { z17.h }, p5/z, [x0, #0]
ld1roh { z0.h }, p0/z, [x0, #224]
ld1roh { z0.h }, p0/z, [x0, #-256]
ld1row { z17.s }, p5/z, [x0, #0]
ld1row { z0.s }, p0/z, [x0, #224]
ld1row { z0.s }, p0/z, [x0, #-256]
ld1rod { z17.d }, p5/z, [x0, #0]
ld1rod { z0.d }, p0/z, [x0, #224]
ld1rod { z0.d }, p0/z, [x0, #-256]

zip1 z17.q, z21.q, z5.q
zip1 z0.q, z0.q, z0.q
zip2 z17.q, z21.q, z5.q
zip2 z0.q, z0.q, z0.q

uzp1 z17.q, z21.q, z5.q
uzp1 z0.q, z0.q, z0.q
uzp2 z17.q, z21.q, z5.q
uzp2 z0.q, z0.q, z0.q

trn1 z17.q, z21.q, z5.q
trn1 z0.q, z0.q, z0.q
trn2 z17.q, z21.q, z5.q
trn2 z0.q, z0.q, z0.q
