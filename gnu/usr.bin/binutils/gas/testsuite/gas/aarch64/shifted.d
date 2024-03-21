#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	aa030041 	orr	x1, x2, x3
   4:	aa030441 	orr	x1, x2, x3, lsl #1
   8:	aa030c41 	orr	x1, x2, x3, lsl #3
   c:	aa031c41 	orr	x1, x2, x3, lsl #7
  10:	aa033c41 	orr	x1, x2, x3, lsl #15
  14:	aa037c41 	orr	x1, x2, x3, lsl #31
  18:	aa03fc41 	orr	x1, x2, x3, lsl #63
  1c:	aa430041 	orr	x1, x2, x3, lsr #0
  20:	aa430441 	orr	x1, x2, x3, lsr #1
  24:	aa430c41 	orr	x1, x2, x3, lsr #3
  28:	aa431c41 	orr	x1, x2, x3, lsr #7
  2c:	aa433c41 	orr	x1, x2, x3, lsr #15
  30:	aa437c41 	orr	x1, x2, x3, lsr #31
  34:	aa43fc41 	orr	x1, x2, x3, lsr #63
  38:	aa830041 	orr	x1, x2, x3, asr #0
  3c:	aa830441 	orr	x1, x2, x3, asr #1
  40:	aa830c41 	orr	x1, x2, x3, asr #3
  44:	aa831c41 	orr	x1, x2, x3, asr #7
  48:	aa833c41 	orr	x1, x2, x3, asr #15
  4c:	aa837c41 	orr	x1, x2, x3, asr #31
  50:	aa83fc41 	orr	x1, x2, x3, asr #63
  54:	aac30041 	orr	x1, x2, x3, ror #0
  58:	aac30441 	orr	x1, x2, x3, ror #1
  5c:	aac30c41 	orr	x1, x2, x3, ror #3
  60:	aac31c41 	orr	x1, x2, x3, ror #7
  64:	aac33c41 	orr	x1, x2, x3, ror #15
  68:	aac37c41 	orr	x1, x2, x3, ror #31
  6c:	aac3fc41 	orr	x1, x2, x3, ror #63
  70:	2a030041 	orr	w1, w2, w3
  74:	2a030441 	orr	w1, w2, w3, lsl #1
  78:	2a030c41 	orr	w1, w2, w3, lsl #3
  7c:	2a031c41 	orr	w1, w2, w3, lsl #7
  80:	2a033c41 	orr	w1, w2, w3, lsl #15
  84:	2a037c41 	orr	w1, w2, w3, lsl #31
  88:	2a430041 	orr	w1, w2, w3, lsr #0
  8c:	2a430441 	orr	w1, w2, w3, lsr #1
  90:	2a430c41 	orr	w1, w2, w3, lsr #3
  94:	2a431c41 	orr	w1, w2, w3, lsr #7
  98:	2a433c41 	orr	w1, w2, w3, lsr #15
  9c:	2a437c41 	orr	w1, w2, w3, lsr #31
  a0:	2a830041 	orr	w1, w2, w3, asr #0
  a4:	2a830441 	orr	w1, w2, w3, asr #1
  a8:	2a830c41 	orr	w1, w2, w3, asr #3
  ac:	2a831c41 	orr	w1, w2, w3, asr #7
  b0:	2a833c41 	orr	w1, w2, w3, asr #15
  b4:	2a837c41 	orr	w1, w2, w3, asr #31
  b8:	2ac30041 	orr	w1, w2, w3, ror #0
  bc:	2ac30441 	orr	w1, w2, w3, ror #1
  c0:	2ac30c41 	orr	w1, w2, w3, ror #3
  c4:	2ac31c41 	orr	w1, w2, w3, ror #7
  c8:	2ac33c41 	orr	w1, w2, w3, ror #15
  cc:	2ac37c41 	orr	w1, w2, w3, ror #31
  d0:	8a030041 	and	x1, x2, x3
  d4:	8a030441 	and	x1, x2, x3, lsl #1
  d8:	8a030c41 	and	x1, x2, x3, lsl #3
  dc:	8a031c41 	and	x1, x2, x3, lsl #7
  e0:	8a033c41 	and	x1, x2, x3, lsl #15
  e4:	8a037c41 	and	x1, x2, x3, lsl #31
  e8:	8a03fc41 	and	x1, x2, x3, lsl #63
  ec:	8a430041 	and	x1, x2, x3, lsr #0
  f0:	8a430441 	and	x1, x2, x3, lsr #1
  f4:	8a430c41 	and	x1, x2, x3, lsr #3
  f8:	8a431c41 	and	x1, x2, x3, lsr #7
  fc:	8a433c41 	and	x1, x2, x3, lsr #15
 100:	8a437c41 	and	x1, x2, x3, lsr #31
 104:	8a43fc41 	and	x1, x2, x3, lsr #63
 108:	8a830041 	and	x1, x2, x3, asr #0
 10c:	8a830441 	and	x1, x2, x3, asr #1
 110:	8a830c41 	and	x1, x2, x3, asr #3
 114:	8a831c41 	and	x1, x2, x3, asr #7
 118:	8a833c41 	and	x1, x2, x3, asr #15
 11c:	8a837c41 	and	x1, x2, x3, asr #31
 120:	8a83fc41 	and	x1, x2, x3, asr #63
 124:	8ac30041 	and	x1, x2, x3, ror #0
 128:	8ac30441 	and	x1, x2, x3, ror #1
 12c:	8ac30c41 	and	x1, x2, x3, ror #3
 130:	8ac31c41 	and	x1, x2, x3, ror #7
 134:	8ac33c41 	and	x1, x2, x3, ror #15
 138:	8ac37c41 	and	x1, x2, x3, ror #31
 13c:	8ac3fc41 	and	x1, x2, x3, ror #63
 140:	0a030041 	and	w1, w2, w3
 144:	0a030441 	and	w1, w2, w3, lsl #1
 148:	0a030c41 	and	w1, w2, w3, lsl #3
 14c:	0a031c41 	and	w1, w2, w3, lsl #7
 150:	0a033c41 	and	w1, w2, w3, lsl #15
 154:	0a037c41 	and	w1, w2, w3, lsl #31
 158:	0a430041 	and	w1, w2, w3, lsr #0
 15c:	0a430441 	and	w1, w2, w3, lsr #1
 160:	0a430c41 	and	w1, w2, w3, lsr #3
 164:	0a431c41 	and	w1, w2, w3, lsr #7
 168:	0a433c41 	and	w1, w2, w3, lsr #15
 16c:	0a437c41 	and	w1, w2, w3, lsr #31
 170:	0a830041 	and	w1, w2, w3, asr #0
 174:	0a830441 	and	w1, w2, w3, asr #1
 178:	0a830c41 	and	w1, w2, w3, asr #3
 17c:	0a831c41 	and	w1, w2, w3, asr #7
 180:	0a833c41 	and	w1, w2, w3, asr #15
 184:	0a837c41 	and	w1, w2, w3, asr #31
 188:	0ac30041 	and	w1, w2, w3, ror #0
 18c:	0ac30441 	and	w1, w2, w3, ror #1
 190:	0ac30c41 	and	w1, w2, w3, ror #3
 194:	0ac31c41 	and	w1, w2, w3, ror #7
 198:	0ac33c41 	and	w1, w2, w3, ror #15
 19c:	0ac37c41 	and	w1, w2, w3, ror #31
 1a0:	ca030041 	eor	x1, x2, x3
 1a4:	ca030441 	eor	x1, x2, x3, lsl #1
 1a8:	ca030c41 	eor	x1, x2, x3, lsl #3
 1ac:	ca031c41 	eor	x1, x2, x3, lsl #7
 1b0:	ca033c41 	eor	x1, x2, x3, lsl #15
 1b4:	ca037c41 	eor	x1, x2, x3, lsl #31
 1b8:	ca03fc41 	eor	x1, x2, x3, lsl #63
 1bc:	ca430041 	eor	x1, x2, x3, lsr #0
 1c0:	ca430441 	eor	x1, x2, x3, lsr #1
 1c4:	ca430c41 	eor	x1, x2, x3, lsr #3
 1c8:	ca431c41 	eor	x1, x2, x3, lsr #7
 1cc:	ca433c41 	eor	x1, x2, x3, lsr #15
 1d0:	ca437c41 	eor	x1, x2, x3, lsr #31
 1d4:	ca43fc41 	eor	x1, x2, x3, lsr #63
 1d8:	ca830041 	eor	x1, x2, x3, asr #0
 1dc:	ca830441 	eor	x1, x2, x3, asr #1
 1e0:	ca830c41 	eor	x1, x2, x3, asr #3
 1e4:	ca831c41 	eor	x1, x2, x3, asr #7
 1e8:	ca833c41 	eor	x1, x2, x3, asr #15
 1ec:	ca837c41 	eor	x1, x2, x3, asr #31
 1f0:	ca83fc41 	eor	x1, x2, x3, asr #63
 1f4:	cac30041 	eor	x1, x2, x3, ror #0
 1f8:	cac30441 	eor	x1, x2, x3, ror #1
 1fc:	cac30c41 	eor	x1, x2, x3, ror #3
 200:	cac31c41 	eor	x1, x2, x3, ror #7
 204:	cac33c41 	eor	x1, x2, x3, ror #15
 208:	cac37c41 	eor	x1, x2, x3, ror #31
 20c:	cac3fc41 	eor	x1, x2, x3, ror #63
 210:	4a030041 	eor	w1, w2, w3
 214:	4a030441 	eor	w1, w2, w3, lsl #1
 218:	4a030c41 	eor	w1, w2, w3, lsl #3
 21c:	4a031c41 	eor	w1, w2, w3, lsl #7
 220:	4a033c41 	eor	w1, w2, w3, lsl #15
 224:	4a037c41 	eor	w1, w2, w3, lsl #31
 228:	4a430041 	eor	w1, w2, w3, lsr #0
 22c:	4a430441 	eor	w1, w2, w3, lsr #1
 230:	4a430c41 	eor	w1, w2, w3, lsr #3
 234:	4a431c41 	eor	w1, w2, w3, lsr #7
 238:	4a433c41 	eor	w1, w2, w3, lsr #15
 23c:	4a437c41 	eor	w1, w2, w3, lsr #31
 240:	4a830041 	eor	w1, w2, w3, asr #0
 244:	4a830441 	eor	w1, w2, w3, asr #1
 248:	4a830c41 	eor	w1, w2, w3, asr #3
 24c:	4a831c41 	eor	w1, w2, w3, asr #7
 250:	4a833c41 	eor	w1, w2, w3, asr #15
 254:	4a837c41 	eor	w1, w2, w3, asr #31
 258:	4ac30041 	eor	w1, w2, w3, ror #0
 25c:	4ac30441 	eor	w1, w2, w3, ror #1
 260:	4ac30c41 	eor	w1, w2, w3, ror #3
 264:	4ac31c41 	eor	w1, w2, w3, ror #7
 268:	4ac33c41 	eor	w1, w2, w3, ror #15
 26c:	4ac37c41 	eor	w1, w2, w3, ror #31
 270:	8a230041 	bic	x1, x2, x3
 274:	8a230441 	bic	x1, x2, x3, lsl #1
 278:	8a230c41 	bic	x1, x2, x3, lsl #3
 27c:	8a231c41 	bic	x1, x2, x3, lsl #7
 280:	8a233c41 	bic	x1, x2, x3, lsl #15
 284:	8a237c41 	bic	x1, x2, x3, lsl #31
 288:	8a23fc41 	bic	x1, x2, x3, lsl #63
 28c:	8a630041 	bic	x1, x2, x3, lsr #0
 290:	8a630441 	bic	x1, x2, x3, lsr #1
 294:	8a630c41 	bic	x1, x2, x3, lsr #3
 298:	8a631c41 	bic	x1, x2, x3, lsr #7
 29c:	8a633c41 	bic	x1, x2, x3, lsr #15
 2a0:	8a637c41 	bic	x1, x2, x3, lsr #31
 2a4:	8a63fc41 	bic	x1, x2, x3, lsr #63
 2a8:	8aa30041 	bic	x1, x2, x3, asr #0
 2ac:	8aa30441 	bic	x1, x2, x3, asr #1
 2b0:	8aa30c41 	bic	x1, x2, x3, asr #3
 2b4:	8aa31c41 	bic	x1, x2, x3, asr #7
 2b8:	8aa33c41 	bic	x1, x2, x3, asr #15
 2bc:	8aa37c41 	bic	x1, x2, x3, asr #31
 2c0:	8aa3fc41 	bic	x1, x2, x3, asr #63
 2c4:	8ae30041 	bic	x1, x2, x3, ror #0
 2c8:	8ae30441 	bic	x1, x2, x3, ror #1
 2cc:	8ae30c41 	bic	x1, x2, x3, ror #3
 2d0:	8ae31c41 	bic	x1, x2, x3, ror #7
 2d4:	8ae33c41 	bic	x1, x2, x3, ror #15
 2d8:	8ae37c41 	bic	x1, x2, x3, ror #31
 2dc:	8ae3fc41 	bic	x1, x2, x3, ror #63
 2e0:	0a230041 	bic	w1, w2, w3
 2e4:	0a230441 	bic	w1, w2, w3, lsl #1
 2e8:	0a230c41 	bic	w1, w2, w3, lsl #3
 2ec:	0a231c41 	bic	w1, w2, w3, lsl #7
 2f0:	0a233c41 	bic	w1, w2, w3, lsl #15
 2f4:	0a237c41 	bic	w1, w2, w3, lsl #31
 2f8:	0a630041 	bic	w1, w2, w3, lsr #0
 2fc:	0a630441 	bic	w1, w2, w3, lsr #1
 300:	0a630c41 	bic	w1, w2, w3, lsr #3
 304:	0a631c41 	bic	w1, w2, w3, lsr #7
 308:	0a633c41 	bic	w1, w2, w3, lsr #15
 30c:	0a637c41 	bic	w1, w2, w3, lsr #31
 310:	0aa30041 	bic	w1, w2, w3, asr #0
 314:	0aa30441 	bic	w1, w2, w3, asr #1
 318:	0aa30c41 	bic	w1, w2, w3, asr #3
 31c:	0aa31c41 	bic	w1, w2, w3, asr #7
 320:	0aa33c41 	bic	w1, w2, w3, asr #15
 324:	0aa37c41 	bic	w1, w2, w3, asr #31
 328:	0ae30041 	bic	w1, w2, w3, ror #0
 32c:	0ae30441 	bic	w1, w2, w3, ror #1
 330:	0ae30c41 	bic	w1, w2, w3, ror #3
 334:	0ae31c41 	bic	w1, w2, w3, ror #7
 338:	0ae33c41 	bic	w1, w2, w3, ror #15
 33c:	0ae37c41 	bic	w1, w2, w3, ror #31
 340:	aa230041 	orn	x1, x2, x3
 344:	aa230441 	orn	x1, x2, x3, lsl #1
 348:	aa230c41 	orn	x1, x2, x3, lsl #3
 34c:	aa231c41 	orn	x1, x2, x3, lsl #7
 350:	aa233c41 	orn	x1, x2, x3, lsl #15
 354:	aa237c41 	orn	x1, x2, x3, lsl #31
 358:	aa23fc41 	orn	x1, x2, x3, lsl #63
 35c:	aa630041 	orn	x1, x2, x3, lsr #0
 360:	aa630441 	orn	x1, x2, x3, lsr #1
 364:	aa630c41 	orn	x1, x2, x3, lsr #3
 368:	aa631c41 	orn	x1, x2, x3, lsr #7
 36c:	aa633c41 	orn	x1, x2, x3, lsr #15
 370:	aa637c41 	orn	x1, x2, x3, lsr #31
 374:	aa63fc41 	orn	x1, x2, x3, lsr #63
 378:	aaa30041 	orn	x1, x2, x3, asr #0
 37c:	aaa30441 	orn	x1, x2, x3, asr #1
 380:	aaa30c41 	orn	x1, x2, x3, asr #3
 384:	aaa31c41 	orn	x1, x2, x3, asr #7
 388:	aaa33c41 	orn	x1, x2, x3, asr #15
 38c:	aaa37c41 	orn	x1, x2, x3, asr #31
 390:	aaa3fc41 	orn	x1, x2, x3, asr #63
 394:	aae30041 	orn	x1, x2, x3, ror #0
 398:	aae30441 	orn	x1, x2, x3, ror #1
 39c:	aae30c41 	orn	x1, x2, x3, ror #3
 3a0:	aae31c41 	orn	x1, x2, x3, ror #7
 3a4:	aae33c41 	orn	x1, x2, x3, ror #15
 3a8:	aae37c41 	orn	x1, x2, x3, ror #31
 3ac:	aae3fc41 	orn	x1, x2, x3, ror #63
 3b0:	2a230041 	orn	w1, w2, w3
 3b4:	2a230441 	orn	w1, w2, w3, lsl #1
 3b8:	2a230c41 	orn	w1, w2, w3, lsl #3
 3bc:	2a231c41 	orn	w1, w2, w3, lsl #7
 3c0:	2a233c41 	orn	w1, w2, w3, lsl #15
 3c4:	2a237c41 	orn	w1, w2, w3, lsl #31
 3c8:	2a630041 	orn	w1, w2, w3, lsr #0
 3cc:	2a630441 	orn	w1, w2, w3, lsr #1
 3d0:	2a630c41 	orn	w1, w2, w3, lsr #3
 3d4:	2a631c41 	orn	w1, w2, w3, lsr #7
 3d8:	2a633c41 	orn	w1, w2, w3, lsr #15
 3dc:	2a637c41 	orn	w1, w2, w3, lsr #31
 3e0:	2aa30041 	orn	w1, w2, w3, asr #0
 3e4:	2aa30441 	orn	w1, w2, w3, asr #1
 3e8:	2aa30c41 	orn	w1, w2, w3, asr #3
 3ec:	2aa31c41 	orn	w1, w2, w3, asr #7
 3f0:	2aa33c41 	orn	w1, w2, w3, asr #15
 3f4:	2aa37c41 	orn	w1, w2, w3, asr #31
 3f8:	2ae30041 	orn	w1, w2, w3, ror #0
 3fc:	2ae30441 	orn	w1, w2, w3, ror #1
 400:	2ae30c41 	orn	w1, w2, w3, ror #3
 404:	2ae31c41 	orn	w1, w2, w3, ror #7
 408:	2ae33c41 	orn	w1, w2, w3, ror #15
 40c:	2ae37c41 	orn	w1, w2, w3, ror #31
 410:	ca230041 	eon	x1, x2, x3
 414:	ca230441 	eon	x1, x2, x3, lsl #1
 418:	ca230c41 	eon	x1, x2, x3, lsl #3
 41c:	ca231c41 	eon	x1, x2, x3, lsl #7
 420:	ca233c41 	eon	x1, x2, x3, lsl #15
 424:	ca237c41 	eon	x1, x2, x3, lsl #31
 428:	ca23fc41 	eon	x1, x2, x3, lsl #63
 42c:	ca630041 	eon	x1, x2, x3, lsr #0
 430:	ca630441 	eon	x1, x2, x3, lsr #1
 434:	ca630c41 	eon	x1, x2, x3, lsr #3
 438:	ca631c41 	eon	x1, x2, x3, lsr #7
 43c:	ca633c41 	eon	x1, x2, x3, lsr #15
 440:	ca637c41 	eon	x1, x2, x3, lsr #31
 444:	ca63fc41 	eon	x1, x2, x3, lsr #63
 448:	caa30041 	eon	x1, x2, x3, asr #0
 44c:	caa30441 	eon	x1, x2, x3, asr #1
 450:	caa30c41 	eon	x1, x2, x3, asr #3
 454:	caa31c41 	eon	x1, x2, x3, asr #7
 458:	caa33c41 	eon	x1, x2, x3, asr #15
 45c:	caa37c41 	eon	x1, x2, x3, asr #31
 460:	caa3fc41 	eon	x1, x2, x3, asr #63
 464:	cae30041 	eon	x1, x2, x3, ror #0
 468:	cae30441 	eon	x1, x2, x3, ror #1
 46c:	cae30c41 	eon	x1, x2, x3, ror #3
 470:	cae31c41 	eon	x1, x2, x3, ror #7
 474:	cae33c41 	eon	x1, x2, x3, ror #15
 478:	cae37c41 	eon	x1, x2, x3, ror #31
 47c:	cae3fc41 	eon	x1, x2, x3, ror #63
 480:	4a230041 	eon	w1, w2, w3
 484:	4a230441 	eon	w1, w2, w3, lsl #1
 488:	4a230c41 	eon	w1, w2, w3, lsl #3
 48c:	4a231c41 	eon	w1, w2, w3, lsl #7
 490:	4a233c41 	eon	w1, w2, w3, lsl #15
 494:	4a237c41 	eon	w1, w2, w3, lsl #31
 498:	4a630041 	eon	w1, w2, w3, lsr #0
 49c:	4a630441 	eon	w1, w2, w3, lsr #1
 4a0:	4a630c41 	eon	w1, w2, w3, lsr #3
 4a4:	4a631c41 	eon	w1, w2, w3, lsr #7
 4a8:	4a633c41 	eon	w1, w2, w3, lsr #15
 4ac:	4a637c41 	eon	w1, w2, w3, lsr #31
 4b0:	4aa30041 	eon	w1, w2, w3, asr #0
 4b4:	4aa30441 	eon	w1, w2, w3, asr #1
 4b8:	4aa30c41 	eon	w1, w2, w3, asr #3
 4bc:	4aa31c41 	eon	w1, w2, w3, asr #7
 4c0:	4aa33c41 	eon	w1, w2, w3, asr #15
 4c4:	4aa37c41 	eon	w1, w2, w3, asr #31
 4c8:	4ae30041 	eon	w1, w2, w3, ror #0
 4cc:	4ae30441 	eon	w1, w2, w3, ror #1
 4d0:	4ae30c41 	eon	w1, w2, w3, ror #3
 4d4:	4ae31c41 	eon	w1, w2, w3, ror #7
 4d8:	4ae33c41 	eon	w1, w2, w3, ror #15
 4dc:	4ae37c41 	eon	w1, w2, w3, ror #31
 4e0:	8b030041 	add	x1, x2, x3
 4e4:	8b030441 	add	x1, x2, x3, lsl #1
 4e8:	8b030c41 	add	x1, x2, x3, lsl #3
 4ec:	8b031c41 	add	x1, x2, x3, lsl #7
 4f0:	8b033c41 	add	x1, x2, x3, lsl #15
 4f4:	8b037c41 	add	x1, x2, x3, lsl #31
 4f8:	8b03fc41 	add	x1, x2, x3, lsl #63
 4fc:	8b430041 	add	x1, x2, x3, lsr #0
 500:	8b430441 	add	x1, x2, x3, lsr #1
 504:	8b430c41 	add	x1, x2, x3, lsr #3
 508:	8b431c41 	add	x1, x2, x3, lsr #7
 50c:	8b433c41 	add	x1, x2, x3, lsr #15
 510:	8b437c41 	add	x1, x2, x3, lsr #31
 514:	8b43fc41 	add	x1, x2, x3, lsr #63
 518:	8b830041 	add	x1, x2, x3, asr #0
 51c:	8b830441 	add	x1, x2, x3, asr #1
 520:	8b830c41 	add	x1, x2, x3, asr #3
 524:	8b831c41 	add	x1, x2, x3, asr #7
 528:	8b833c41 	add	x1, x2, x3, asr #15
 52c:	8b837c41 	add	x1, x2, x3, asr #31
 530:	8b83fc41 	add	x1, x2, x3, asr #63
 534:	8b230041 	add	x1, x2, w3, uxtb
 538:	8b230441 	add	x1, x2, w3, uxtb #1
 53c:	8b230841 	add	x1, x2, w3, uxtb #2
 540:	8b230c41 	add	x1, x2, w3, uxtb #3
 544:	8b231041 	add	x1, x2, w3, uxtb #4
 548:	8b232041 	add	x1, x2, w3, uxth
 54c:	8b232441 	add	x1, x2, w3, uxth #1
 550:	8b232841 	add	x1, x2, w3, uxth #2
 554:	8b232c41 	add	x1, x2, w3, uxth #3
 558:	8b233041 	add	x1, x2, w3, uxth #4
 55c:	8b234041 	add	x1, x2, w3, uxtw
 560:	8b234441 	add	x1, x2, w3, uxtw #1
 564:	8b234841 	add	x1, x2, w3, uxtw #2
 568:	8b234c41 	add	x1, x2, w3, uxtw #3
 56c:	8b235041 	add	x1, x2, w3, uxtw #4
 570:	8b236041 	add	x1, x2, x3, uxtx
 574:	8b236441 	add	x1, x2, x3, uxtx #1
 578:	8b236841 	add	x1, x2, x3, uxtx #2
 57c:	8b236c41 	add	x1, x2, x3, uxtx #3
 580:	8b237041 	add	x1, x2, x3, uxtx #4
 584:	8b238041 	add	x1, x2, w3, sxtb
 588:	8b238441 	add	x1, x2, w3, sxtb #1
 58c:	8b238841 	add	x1, x2, w3, sxtb #2
 590:	8b238c41 	add	x1, x2, w3, sxtb #3
 594:	8b239041 	add	x1, x2, w3, sxtb #4
 598:	8b23a041 	add	x1, x2, w3, sxth
 59c:	8b23a441 	add	x1, x2, w3, sxth #1
 5a0:	8b23a841 	add	x1, x2, w3, sxth #2
 5a4:	8b23ac41 	add	x1, x2, w3, sxth #3
 5a8:	8b23b041 	add	x1, x2, w3, sxth #4
 5ac:	8b23c041 	add	x1, x2, w3, sxtw
 5b0:	8b23c441 	add	x1, x2, w3, sxtw #1
 5b4:	8b23c841 	add	x1, x2, w3, sxtw #2
 5b8:	8b23cc41 	add	x1, x2, w3, sxtw #3
 5bc:	8b23d041 	add	x1, x2, w3, sxtw #4
 5c0:	8b23e041 	add	x1, x2, x3, sxtx
 5c4:	8b23e441 	add	x1, x2, x3, sxtx #1
 5c8:	8b23e841 	add	x1, x2, x3, sxtx #2
 5cc:	8b23ec41 	add	x1, x2, x3, sxtx #3
 5d0:	8b23f041 	add	x1, x2, x3, sxtx #4
 5d4:	0b030041 	add	w1, w2, w3
 5d8:	0b030441 	add	w1, w2, w3, lsl #1
 5dc:	0b030c41 	add	w1, w2, w3, lsl #3
 5e0:	0b031c41 	add	w1, w2, w3, lsl #7
 5e4:	0b033c41 	add	w1, w2, w3, lsl #15
 5e8:	0b037c41 	add	w1, w2, w3, lsl #31
 5ec:	0b430041 	add	w1, w2, w3, lsr #0
 5f0:	0b430441 	add	w1, w2, w3, lsr #1
 5f4:	0b430c41 	add	w1, w2, w3, lsr #3
 5f8:	0b431c41 	add	w1, w2, w3, lsr #7
 5fc:	0b433c41 	add	w1, w2, w3, lsr #15
 600:	0b437c41 	add	w1, w2, w3, lsr #31
 604:	0b830041 	add	w1, w2, w3, asr #0
 608:	0b830441 	add	w1, w2, w3, asr #1
 60c:	0b830c41 	add	w1, w2, w3, asr #3
 610:	0b831c41 	add	w1, w2, w3, asr #7
 614:	0b833c41 	add	w1, w2, w3, asr #15
 618:	0b837c41 	add	w1, w2, w3, asr #31
 61c:	0b230041 	add	w1, w2, w3, uxtb
 620:	0b230441 	add	w1, w2, w3, uxtb #1
 624:	0b230841 	add	w1, w2, w3, uxtb #2
 628:	0b230c41 	add	w1, w2, w3, uxtb #3
 62c:	0b231041 	add	w1, w2, w3, uxtb #4
 630:	0b232041 	add	w1, w2, w3, uxth
 634:	0b232441 	add	w1, w2, w3, uxth #1
 638:	0b232841 	add	w1, w2, w3, uxth #2
 63c:	0b232c41 	add	w1, w2, w3, uxth #3
 640:	0b233041 	add	w1, w2, w3, uxth #4
 644:	0b238041 	add	w1, w2, w3, sxtb
 648:	0b238441 	add	w1, w2, w3, sxtb #1
 64c:	0b238841 	add	w1, w2, w3, sxtb #2
 650:	0b238c41 	add	w1, w2, w3, sxtb #3
 654:	0b239041 	add	w1, w2, w3, sxtb #4
 658:	0b23a041 	add	w1, w2, w3, sxth
 65c:	0b23a441 	add	w1, w2, w3, sxth #1
 660:	0b23a841 	add	w1, w2, w3, sxth #2
 664:	0b23ac41 	add	w1, w2, w3, sxth #3
 668:	0b23b041 	add	w1, w2, w3, sxth #4
 66c:	cb030041 	sub	x1, x2, x3
 670:	cb030441 	sub	x1, x2, x3, lsl #1
 674:	cb030c41 	sub	x1, x2, x3, lsl #3
 678:	cb031c41 	sub	x1, x2, x3, lsl #7
 67c:	cb033c41 	sub	x1, x2, x3, lsl #15
 680:	cb037c41 	sub	x1, x2, x3, lsl #31
 684:	cb03fc41 	sub	x1, x2, x3, lsl #63
 688:	cb430041 	sub	x1, x2, x3, lsr #0
 68c:	cb430441 	sub	x1, x2, x3, lsr #1
 690:	cb430c41 	sub	x1, x2, x3, lsr #3
 694:	cb431c41 	sub	x1, x2, x3, lsr #7
 698:	cb433c41 	sub	x1, x2, x3, lsr #15
 69c:	cb437c41 	sub	x1, x2, x3, lsr #31
 6a0:	cb43fc41 	sub	x1, x2, x3, lsr #63
 6a4:	cb830041 	sub	x1, x2, x3, asr #0
 6a8:	cb830441 	sub	x1, x2, x3, asr #1
 6ac:	cb830c41 	sub	x1, x2, x3, asr #3
 6b0:	cb831c41 	sub	x1, x2, x3, asr #7
 6b4:	cb833c41 	sub	x1, x2, x3, asr #15
 6b8:	cb837c41 	sub	x1, x2, x3, asr #31
 6bc:	cb83fc41 	sub	x1, x2, x3, asr #63
 6c0:	cb230041 	sub	x1, x2, w3, uxtb
 6c4:	cb230441 	sub	x1, x2, w3, uxtb #1
 6c8:	cb230841 	sub	x1, x2, w3, uxtb #2
 6cc:	cb230c41 	sub	x1, x2, w3, uxtb #3
 6d0:	cb231041 	sub	x1, x2, w3, uxtb #4
 6d4:	cb232041 	sub	x1, x2, w3, uxth
 6d8:	cb232441 	sub	x1, x2, w3, uxth #1
 6dc:	cb232841 	sub	x1, x2, w3, uxth #2
 6e0:	cb232c41 	sub	x1, x2, w3, uxth #3
 6e4:	cb233041 	sub	x1, x2, w3, uxth #4
 6e8:	cb234041 	sub	x1, x2, w3, uxtw
 6ec:	cb234441 	sub	x1, x2, w3, uxtw #1
 6f0:	cb234841 	sub	x1, x2, w3, uxtw #2
 6f4:	cb234c41 	sub	x1, x2, w3, uxtw #3
 6f8:	cb235041 	sub	x1, x2, w3, uxtw #4
 6fc:	cb236041 	sub	x1, x2, x3, uxtx
 700:	cb236441 	sub	x1, x2, x3, uxtx #1
 704:	cb236841 	sub	x1, x2, x3, uxtx #2
 708:	cb236c41 	sub	x1, x2, x3, uxtx #3
 70c:	cb237041 	sub	x1, x2, x3, uxtx #4
 710:	cb238041 	sub	x1, x2, w3, sxtb
 714:	cb238441 	sub	x1, x2, w3, sxtb #1
 718:	cb238841 	sub	x1, x2, w3, sxtb #2
 71c:	cb238c41 	sub	x1, x2, w3, sxtb #3
 720:	cb239041 	sub	x1, x2, w3, sxtb #4
 724:	cb23a041 	sub	x1, x2, w3, sxth
 728:	cb23a441 	sub	x1, x2, w3, sxth #1
 72c:	cb23a841 	sub	x1, x2, w3, sxth #2
 730:	cb23ac41 	sub	x1, x2, w3, sxth #3
 734:	cb23b041 	sub	x1, x2, w3, sxth #4
 738:	cb23c041 	sub	x1, x2, w3, sxtw
 73c:	cb23c441 	sub	x1, x2, w3, sxtw #1
 740:	cb23c841 	sub	x1, x2, w3, sxtw #2
 744:	cb23cc41 	sub	x1, x2, w3, sxtw #3
 748:	cb23d041 	sub	x1, x2, w3, sxtw #4
 74c:	cb23e041 	sub	x1, x2, x3, sxtx
 750:	cb23e441 	sub	x1, x2, x3, sxtx #1
 754:	cb23e841 	sub	x1, x2, x3, sxtx #2
 758:	cb23ec41 	sub	x1, x2, x3, sxtx #3
 75c:	cb23f041 	sub	x1, x2, x3, sxtx #4
 760:	4b030041 	sub	w1, w2, w3
 764:	4b030441 	sub	w1, w2, w3, lsl #1
 768:	4b030c41 	sub	w1, w2, w3, lsl #3
 76c:	4b031c41 	sub	w1, w2, w3, lsl #7
 770:	4b033c41 	sub	w1, w2, w3, lsl #15
 774:	4b037c41 	sub	w1, w2, w3, lsl #31
 778:	4b430041 	sub	w1, w2, w3, lsr #0
 77c:	4b430441 	sub	w1, w2, w3, lsr #1
 780:	4b430c41 	sub	w1, w2, w3, lsr #3
 784:	4b431c41 	sub	w1, w2, w3, lsr #7
 788:	4b433c41 	sub	w1, w2, w3, lsr #15
 78c:	4b437c41 	sub	w1, w2, w3, lsr #31
 790:	4b830041 	sub	w1, w2, w3, asr #0
 794:	4b830441 	sub	w1, w2, w3, asr #1
 798:	4b830c41 	sub	w1, w2, w3, asr #3
 79c:	4b831c41 	sub	w1, w2, w3, asr #7
 7a0:	4b833c41 	sub	w1, w2, w3, asr #15
 7a4:	4b837c41 	sub	w1, w2, w3, asr #31
 7a8:	4b230041 	sub	w1, w2, w3, uxtb
 7ac:	4b230441 	sub	w1, w2, w3, uxtb #1
 7b0:	4b230841 	sub	w1, w2, w3, uxtb #2
 7b4:	4b230c41 	sub	w1, w2, w3, uxtb #3
 7b8:	4b231041 	sub	w1, w2, w3, uxtb #4
 7bc:	4b232041 	sub	w1, w2, w3, uxth
 7c0:	4b232441 	sub	w1, w2, w3, uxth #1
 7c4:	4b232841 	sub	w1, w2, w3, uxth #2
 7c8:	4b232c41 	sub	w1, w2, w3, uxth #3
 7cc:	4b233041 	sub	w1, w2, w3, uxth #4
 7d0:	4b238041 	sub	w1, w2, w3, sxtb
 7d4:	4b238441 	sub	w1, w2, w3, sxtb #1
 7d8:	4b238841 	sub	w1, w2, w3, sxtb #2
 7dc:	4b238c41 	sub	w1, w2, w3, sxtb #3
 7e0:	4b239041 	sub	w1, w2, w3, sxtb #4
 7e4:	4b23a041 	sub	w1, w2, w3, sxth
 7e8:	4b23a441 	sub	w1, w2, w3, sxth #1
 7ec:	4b23a841 	sub	w1, w2, w3, sxth #2
 7f0:	4b23ac41 	sub	w1, w2, w3, sxth #3
 7f4:	4b23b041 	sub	w1, w2, w3, sxth #4
 7f8:	cb0303e2 	neg	x2, x3
 7fc:	cb0307e2 	neg	x2, x3, lsl #1
 800:	cb030fe2 	neg	x2, x3, lsl #3
 804:	cb031fe2 	neg	x2, x3, lsl #7
 808:	cb033fe2 	neg	x2, x3, lsl #15
 80c:	cb037fe2 	neg	x2, x3, lsl #31
 810:	cb03ffe2 	neg	x2, x3, lsl #63
 814:	cb4303e2 	neg	x2, x3, lsr #0
 818:	cb4307e2 	neg	x2, x3, lsr #1
 81c:	cb430fe2 	neg	x2, x3, lsr #3
 820:	cb431fe2 	neg	x2, x3, lsr #7
 824:	cb433fe2 	neg	x2, x3, lsr #15
 828:	cb437fe2 	neg	x2, x3, lsr #31
 82c:	cb43ffe2 	neg	x2, x3, lsr #63
 830:	cb8303e2 	neg	x2, x3, asr #0
 834:	cb8307e2 	neg	x2, x3, asr #1
 838:	cb830fe2 	neg	x2, x3, asr #3
 83c:	cb831fe2 	neg	x2, x3, asr #7
 840:	cb833fe2 	neg	x2, x3, asr #15
 844:	cb837fe2 	neg	x2, x3, asr #31
 848:	cb83ffe2 	neg	x2, x3, asr #63
 84c:	4b0303e2 	neg	w2, w3
 850:	4b0307e2 	neg	w2, w3, lsl #1
 854:	4b030fe2 	neg	w2, w3, lsl #3
 858:	4b031fe2 	neg	w2, w3, lsl #7
 85c:	4b033fe2 	neg	w2, w3, lsl #15
 860:	4b037fe2 	neg	w2, w3, lsl #31
 864:	4b4303e2 	neg	w2, w3, lsr #0
 868:	4b4307e2 	neg	w2, w3, lsr #1
 86c:	4b430fe2 	neg	w2, w3, lsr #3
 870:	4b431fe2 	neg	w2, w3, lsr #7
 874:	4b433fe2 	neg	w2, w3, lsr #15
 878:	4b437fe2 	neg	w2, w3, lsr #31
 87c:	4b8303e2 	neg	w2, w3, asr #0
 880:	4b8307e2 	neg	w2, w3, asr #1
 884:	4b830fe2 	neg	w2, w3, asr #3
 888:	4b831fe2 	neg	w2, w3, asr #7
 88c:	4b833fe2 	neg	w2, w3, asr #15
 890:	4b837fe2 	neg	w2, w3, asr #31
 894:	eb03005f 	cmp	x2, x3
 898:	eb03045f 	cmp	x2, x3, lsl #1
 89c:	eb030c5f 	cmp	x2, x3, lsl #3
 8a0:	eb031c5f 	cmp	x2, x3, lsl #7
 8a4:	eb033c5f 	cmp	x2, x3, lsl #15
 8a8:	eb037c5f 	cmp	x2, x3, lsl #31
 8ac:	eb03fc5f 	cmp	x2, x3, lsl #63
 8b0:	eb43005f 	cmp	x2, x3, lsr #0
 8b4:	eb43045f 	cmp	x2, x3, lsr #1
 8b8:	eb430c5f 	cmp	x2, x3, lsr #3
 8bc:	eb431c5f 	cmp	x2, x3, lsr #7
 8c0:	eb433c5f 	cmp	x2, x3, lsr #15
 8c4:	eb437c5f 	cmp	x2, x3, lsr #31
 8c8:	eb43fc5f 	cmp	x2, x3, lsr #63
 8cc:	eb83005f 	cmp	x2, x3, asr #0
 8d0:	eb83045f 	cmp	x2, x3, asr #1
 8d4:	eb830c5f 	cmp	x2, x3, asr #3
 8d8:	eb831c5f 	cmp	x2, x3, asr #7
 8dc:	eb833c5f 	cmp	x2, x3, asr #15
 8e0:	eb837c5f 	cmp	x2, x3, asr #31
 8e4:	eb83fc5f 	cmp	x2, x3, asr #63
 8e8:	eb23005f 	cmp	x2, w3, uxtb
 8ec:	eb23045f 	cmp	x2, w3, uxtb #1
 8f0:	eb23085f 	cmp	x2, w3, uxtb #2
 8f4:	eb230c5f 	cmp	x2, w3, uxtb #3
 8f8:	eb23105f 	cmp	x2, w3, uxtb #4
 8fc:	eb23205f 	cmp	x2, w3, uxth
 900:	eb23245f 	cmp	x2, w3, uxth #1
 904:	eb23285f 	cmp	x2, w3, uxth #2
 908:	eb232c5f 	cmp	x2, w3, uxth #3
 90c:	eb23305f 	cmp	x2, w3, uxth #4
 910:	eb23405f 	cmp	x2, w3, uxtw
 914:	eb23445f 	cmp	x2, w3, uxtw #1
 918:	eb23485f 	cmp	x2, w3, uxtw #2
 91c:	eb234c5f 	cmp	x2, w3, uxtw #3
 920:	eb23505f 	cmp	x2, w3, uxtw #4
 924:	eb23805f 	cmp	x2, w3, sxtb
 928:	eb23845f 	cmp	x2, w3, sxtb #1
 92c:	eb23885f 	cmp	x2, w3, sxtb #2
 930:	eb238c5f 	cmp	x2, w3, sxtb #3
 934:	eb23905f 	cmp	x2, w3, sxtb #4
 938:	eb23a05f 	cmp	x2, w3, sxth
 93c:	eb23a45f 	cmp	x2, w3, sxth #1
 940:	eb23a85f 	cmp	x2, w3, sxth #2
 944:	eb23ac5f 	cmp	x2, w3, sxth #3
 948:	eb23b05f 	cmp	x2, w3, sxth #4
 94c:	eb23c05f 	cmp	x2, w3, sxtw
 950:	eb23c45f 	cmp	x2, w3, sxtw #1
 954:	eb23c85f 	cmp	x2, w3, sxtw #2
 958:	eb23cc5f 	cmp	x2, w3, sxtw #3
 95c:	eb23d05f 	cmp	x2, w3, sxtw #4
 960:	6b03005f 	cmp	w2, w3
 964:	6b03045f 	cmp	w2, w3, lsl #1
 968:	6b030c5f 	cmp	w2, w3, lsl #3
 96c:	6b031c5f 	cmp	w2, w3, lsl #7
 970:	6b033c5f 	cmp	w2, w3, lsl #15
 974:	6b037c5f 	cmp	w2, w3, lsl #31
 978:	6b43005f 	cmp	w2, w3, lsr #0
 97c:	6b43045f 	cmp	w2, w3, lsr #1
 980:	6b430c5f 	cmp	w2, w3, lsr #3
 984:	6b431c5f 	cmp	w2, w3, lsr #7
 988:	6b433c5f 	cmp	w2, w3, lsr #15
 98c:	6b437c5f 	cmp	w2, w3, lsr #31
 990:	6b83005f 	cmp	w2, w3, asr #0
 994:	6b83045f 	cmp	w2, w3, asr #1
 998:	6b830c5f 	cmp	w2, w3, asr #3
 99c:	6b831c5f 	cmp	w2, w3, asr #7
 9a0:	6b833c5f 	cmp	w2, w3, asr #15
 9a4:	6b837c5f 	cmp	w2, w3, asr #31
 9a8:	6b23005f 	cmp	w2, w3, uxtb
 9ac:	6b23045f 	cmp	w2, w3, uxtb #1
 9b0:	6b23085f 	cmp	w2, w3, uxtb #2
 9b4:	6b230c5f 	cmp	w2, w3, uxtb #3
 9b8:	6b23105f 	cmp	w2, w3, uxtb #4
 9bc:	6b23205f 	cmp	w2, w3, uxth
 9c0:	6b23245f 	cmp	w2, w3, uxth #1
 9c4:	6b23285f 	cmp	w2, w3, uxth #2
 9c8:	6b232c5f 	cmp	w2, w3, uxth #3
 9cc:	6b23305f 	cmp	w2, w3, uxth #4
 9d0:	6b23805f 	cmp	w2, w3, sxtb
 9d4:	6b23845f 	cmp	w2, w3, sxtb #1
 9d8:	6b23885f 	cmp	w2, w3, sxtb #2
 9dc:	6b238c5f 	cmp	w2, w3, sxtb #3
 9e0:	6b23905f 	cmp	w2, w3, sxtb #4
 9e4:	6b23a05f 	cmp	w2, w3, sxth
 9e8:	6b23a45f 	cmp	w2, w3, sxth #1
 9ec:	6b23a85f 	cmp	w2, w3, sxth #2
 9f0:	6b23ac5f 	cmp	w2, w3, sxth #3
 9f4:	6b23b05f 	cmp	w2, w3, sxth #4
 9f8:	ab03005f 	cmn	x2, x3
 9fc:	ab03045f 	cmn	x2, x3, lsl #1
 a00:	ab030c5f 	cmn	x2, x3, lsl #3
 a04:	ab031c5f 	cmn	x2, x3, lsl #7
 a08:	ab033c5f 	cmn	x2, x3, lsl #15
 a0c:	ab037c5f 	cmn	x2, x3, lsl #31
 a10:	ab03fc5f 	cmn	x2, x3, lsl #63
 a14:	ab43005f 	cmn	x2, x3, lsr #0
 a18:	ab43045f 	cmn	x2, x3, lsr #1
 a1c:	ab430c5f 	cmn	x2, x3, lsr #3
 a20:	ab431c5f 	cmn	x2, x3, lsr #7
 a24:	ab433c5f 	cmn	x2, x3, lsr #15
 a28:	ab437c5f 	cmn	x2, x3, lsr #31
 a2c:	ab43fc5f 	cmn	x2, x3, lsr #63
 a30:	ab83005f 	cmn	x2, x3, asr #0
 a34:	ab83045f 	cmn	x2, x3, asr #1
 a38:	ab830c5f 	cmn	x2, x3, asr #3
 a3c:	ab831c5f 	cmn	x2, x3, asr #7
 a40:	ab833c5f 	cmn	x2, x3, asr #15
 a44:	ab837c5f 	cmn	x2, x3, asr #31
 a48:	ab83fc5f 	cmn	x2, x3, asr #63
 a4c:	ab23005f 	cmn	x2, w3, uxtb
 a50:	ab23045f 	cmn	x2, w3, uxtb #1
 a54:	ab23085f 	cmn	x2, w3, uxtb #2
 a58:	ab230c5f 	cmn	x2, w3, uxtb #3
 a5c:	ab23105f 	cmn	x2, w3, uxtb #4
 a60:	ab23205f 	cmn	x2, w3, uxth
 a64:	ab23245f 	cmn	x2, w3, uxth #1
 a68:	ab23285f 	cmn	x2, w3, uxth #2
 a6c:	ab232c5f 	cmn	x2, w3, uxth #3
 a70:	ab23305f 	cmn	x2, w3, uxth #4
 a74:	ab23405f 	cmn	x2, w3, uxtw
 a78:	ab23445f 	cmn	x2, w3, uxtw #1
 a7c:	ab23485f 	cmn	x2, w3, uxtw #2
 a80:	ab234c5f 	cmn	x2, w3, uxtw #3
 a84:	ab23505f 	cmn	x2, w3, uxtw #4
 a88:	ab23805f 	cmn	x2, w3, sxtb
 a8c:	ab23845f 	cmn	x2, w3, sxtb #1
 a90:	ab23885f 	cmn	x2, w3, sxtb #2
 a94:	ab238c5f 	cmn	x2, w3, sxtb #3
 a98:	ab23905f 	cmn	x2, w3, sxtb #4
 a9c:	ab23a05f 	cmn	x2, w3, sxth
 aa0:	ab23a45f 	cmn	x2, w3, sxth #1
 aa4:	ab23a85f 	cmn	x2, w3, sxth #2
 aa8:	ab23ac5f 	cmn	x2, w3, sxth #3
 aac:	ab23b05f 	cmn	x2, w3, sxth #4
 ab0:	ab23c05f 	cmn	x2, w3, sxtw
 ab4:	ab23c45f 	cmn	x2, w3, sxtw #1
 ab8:	ab23c85f 	cmn	x2, w3, sxtw #2
 abc:	ab23cc5f 	cmn	x2, w3, sxtw #3
 ac0:	ab23d05f 	cmn	x2, w3, sxtw #4
 ac4:	2b03005f 	cmn	w2, w3
 ac8:	2b03045f 	cmn	w2, w3, lsl #1
 acc:	2b030c5f 	cmn	w2, w3, lsl #3
 ad0:	2b031c5f 	cmn	w2, w3, lsl #7
 ad4:	2b033c5f 	cmn	w2, w3, lsl #15
 ad8:	2b037c5f 	cmn	w2, w3, lsl #31
 adc:	2b43005f 	cmn	w2, w3, lsr #0
 ae0:	2b43045f 	cmn	w2, w3, lsr #1
 ae4:	2b430c5f 	cmn	w2, w3, lsr #3
 ae8:	2b431c5f 	cmn	w2, w3, lsr #7
 aec:	2b433c5f 	cmn	w2, w3, lsr #15
 af0:	2b437c5f 	cmn	w2, w3, lsr #31
 af4:	2b83005f 	cmn	w2, w3, asr #0
 af8:	2b83045f 	cmn	w2, w3, asr #1
 afc:	2b830c5f 	cmn	w2, w3, asr #3
 b00:	2b831c5f 	cmn	w2, w3, asr #7
 b04:	2b833c5f 	cmn	w2, w3, asr #15
 b08:	2b837c5f 	cmn	w2, w3, asr #31
 b0c:	2b23005f 	cmn	w2, w3, uxtb
 b10:	2b23045f 	cmn	w2, w3, uxtb #1
 b14:	2b23085f 	cmn	w2, w3, uxtb #2
 b18:	2b230c5f 	cmn	w2, w3, uxtb #3
 b1c:	2b23105f 	cmn	w2, w3, uxtb #4
 b20:	2b23205f 	cmn	w2, w3, uxth
 b24:	2b23245f 	cmn	w2, w3, uxth #1
 b28:	2b23285f 	cmn	w2, w3, uxth #2
 b2c:	2b232c5f 	cmn	w2, w3, uxth #3
 b30:	2b23305f 	cmn	w2, w3, uxth #4
 b34:	2b23805f 	cmn	w2, w3, sxtb
 b38:	2b23845f 	cmn	w2, w3, sxtb #1
 b3c:	2b23885f 	cmn	w2, w3, sxtb #2
 b40:	2b238c5f 	cmn	w2, w3, sxtb #3
 b44:	2b23905f 	cmn	w2, w3, sxtb #4
 b48:	2b23a05f 	cmn	w2, w3, sxth
 b4c:	2b23a45f 	cmn	w2, w3, sxth #1
 b50:	2b23a85f 	cmn	w2, w3, sxth #2
 b54:	2b23ac5f 	cmn	w2, w3, sxth #3
 b58:	2b23b05f 	cmn	w2, w3, sxth #4
