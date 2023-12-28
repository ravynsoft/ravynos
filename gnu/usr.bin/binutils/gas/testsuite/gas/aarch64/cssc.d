#name: CSSC extension
#as: -march=armv8-a+cssc
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	5ac02000 	abs	w0, w0
   4:	5ac02001 	abs	w1, w0
   8:	5ac02100 	abs	w0, w8
   c:	5ac020be 	abs	w30, w5
  10:	5ac023c4 	abs	w4, w30
  14:	dac02000 	abs	x0, x0
  18:	dac02001 	abs	x1, x0
  1c:	dac02100 	abs	x0, x8
  20:	dac020be 	abs	x30, x5
  24:	dac023c4 	abs	x4, x30
  28:	5ac01c00 	cnt	w0, w0
  2c:	5ac01c01 	cnt	w1, w0
  30:	5ac01d00 	cnt	w0, w8
  34:	5ac01cbe 	cnt	w30, w5
  38:	5ac01fc4 	cnt	w4, w30
  3c:	dac01c00 	cnt	x0, x0
  40:	dac01c01 	cnt	x1, x0
  44:	dac01d00 	cnt	x0, x8
  48:	dac01cbe 	cnt	x30, x5
  4c:	dac01fc4 	cnt	x4, x30
  50:	5ac01800 	ctz	w0, w0
  54:	5ac01801 	ctz	w1, w0
  58:	5ac01900 	ctz	w0, w8
  5c:	5ac018be 	ctz	w30, w5
  60:	5ac01bc4 	ctz	w4, w30
  64:	dac01800 	ctz	x0, x0
  68:	dac01801 	ctz	x1, x0
  6c:	dac01900 	ctz	x0, x8
  70:	dac018be 	ctz	x30, x5
  74:	dac01bc4 	ctz	x4, x30
  78:	1ac06000 	smax	w0, w0, w0
  7c:	1ac06001 	smax	w1, w0, w0
  80:	1ac06020 	smax	w0, w1, w0
  84:	1ac16000 	smax	w0, w0, w1
  88:	1ac46043 	smax	w3, w2, w4
  8c:	1ac0601e 	smax	w30, w0, w0
  90:	1ac063c0 	smax	w0, w30, w0
  94:	1ade6000 	smax	w0, w0, w30
  98:	1adc60ee 	smax	w14, w7, w28
  9c:	9ac06000 	smax	x0, x0, x0
  a0:	9ac06001 	smax	x1, x0, x0
  a4:	9ac06020 	smax	x0, x1, x0
  a8:	9ac16000 	smax	x0, x0, x1
  ac:	9ac46043 	smax	x3, x2, x4
  b0:	9ac0601e 	smax	x30, x0, x0
  b4:	9ac063c0 	smax	x0, x30, x0
  b8:	9ade6000 	smax	x0, x0, x30
  bc:	9adc60ee 	smax	x14, x7, x28
  c0:	1ac06400 	umax	w0, w0, w0
  c4:	1ac06401 	umax	w1, w0, w0
  c8:	1ac06420 	umax	w0, w1, w0
  cc:	1ac16400 	umax	w0, w0, w1
  d0:	1ac46443 	umax	w3, w2, w4
  d4:	1ac0641e 	umax	w30, w0, w0
  d8:	1ac067c0 	umax	w0, w30, w0
  dc:	1ade6400 	umax	w0, w0, w30
  e0:	1adc64ee 	umax	w14, w7, w28
  e4:	9ac06400 	umax	x0, x0, x0
  e8:	9ac06401 	umax	x1, x0, x0
  ec:	9ac06420 	umax	x0, x1, x0
  f0:	9ac16400 	umax	x0, x0, x1
  f4:	9ac46443 	umax	x3, x2, x4
  f8:	9ac0641e 	umax	x30, x0, x0
  fc:	9ac067c0 	umax	x0, x30, x0
 100:	9ade6400 	umax	x0, x0, x30
 104:	9adc64ee 	umax	x14, x7, x28
 108:	1ac06800 	smin	w0, w0, w0
 10c:	1ac06801 	smin	w1, w0, w0
 110:	1ac06820 	smin	w0, w1, w0
 114:	1ac16800 	smin	w0, w0, w1
 118:	1ac46843 	smin	w3, w2, w4
 11c:	1ac0681e 	smin	w30, w0, w0
 120:	1ac06bc0 	smin	w0, w30, w0
 124:	1ade6800 	smin	w0, w0, w30
 128:	1adc68ee 	smin	w14, w7, w28
 12c:	9ac06800 	smin	x0, x0, x0
 130:	9ac06801 	smin	x1, x0, x0
 134:	9ac06820 	smin	x0, x1, x0
 138:	9ac16800 	smin	x0, x0, x1
 13c:	9ac46843 	smin	x3, x2, x4
 140:	9ac0681e 	smin	x30, x0, x0
 144:	9ac06bc0 	smin	x0, x30, x0
 148:	9ade6800 	smin	x0, x0, x30
 14c:	9adc68ee 	smin	x14, x7, x28
 150:	1ac06c00 	umin	w0, w0, w0
 154:	1ac06c01 	umin	w1, w0, w0
 158:	1ac06c20 	umin	w0, w1, w0
 15c:	1ac16c00 	umin	w0, w0, w1
 160:	1ac46c43 	umin	w3, w2, w4
 164:	1ac06c1e 	umin	w30, w0, w0
 168:	1ac06fc0 	umin	w0, w30, w0
 16c:	1ade6c00 	umin	w0, w0, w30
 170:	1adc6cee 	umin	w14, w7, w28
 174:	9ac06c00 	umin	x0, x0, x0
 178:	9ac06c01 	umin	x1, x0, x0
 17c:	9ac06c20 	umin	x0, x1, x0
 180:	9ac16c00 	umin	x0, x0, x1
 184:	9ac46c43 	umin	x3, x2, x4
 188:	9ac06c1e 	umin	x30, x0, x0
 18c:	9ac06fc0 	umin	x0, x30, x0
 190:	9ade6c00 	umin	x0, x0, x30
 194:	9adc6cee 	umin	x14, x7, x28
 198:	11c00000 	smax	w0, w0, #0
 19c:	11c00001 	smax	w1, w0, #0
 1a0:	11c00020 	smax	w0, w1, #0
 1a4:	11c00400 	smax	w0, w0, #1
 1a8:	11c38102 	smax	w2, w8, #-32
 1ac:	11c2034d 	smax	w13, w26, #-128
 1b0:	11c1fd31 	smax	w17, w9, #127
 1b4:	91c00000 	smax	x0, x0, #0
 1b8:	91c00001 	smax	x1, x0, #0
 1bc:	91c00020 	smax	x0, x1, #0
 1c0:	91c00400 	smax	x0, x0, #1
 1c4:	91c38102 	smax	x2, x8, #-32
 1c8:	91c2034d 	smax	x13, x26, #-128
 1cc:	91c1fd31 	smax	x17, x9, #127
 1d0:	11c40000 	umax	w0, w0, #0
 1d4:	11c40001 	umax	w1, w0, #0
 1d8:	11c40020 	umax	w0, w1, #0
 1dc:	11c40400 	umax	w0, w0, #1
 1e0:	11c48902 	umax	w2, w8, #34
 1e4:	11c6034d 	umax	w13, w26, #128
 1e8:	11c7fd31 	umax	w17, w9, #255
 1ec:	91c40000 	umax	x0, x0, #0
 1f0:	91c40001 	umax	x1, x0, #0
 1f4:	91c40020 	umax	x0, x1, #0
 1f8:	91c40400 	umax	x0, x0, #1
 1fc:	91c48902 	umax	x2, x8, #34
 200:	91c6034d 	umax	x13, x26, #128
 204:	91c7fd31 	umax	x17, x9, #255
 208:	11c80000 	smin	w0, w0, #0
 20c:	11c80001 	smin	w1, w0, #0
 210:	11c80020 	smin	w0, w1, #0
 214:	11c80400 	smin	w0, w0, #1
 218:	11cb8102 	smin	w2, w8, #-32
 21c:	11ca034d 	smin	w13, w26, #-128
 220:	11c9fd31 	smin	w17, w9, #127
 224:	91c80000 	smin	x0, x0, #0
 228:	91c80001 	smin	x1, x0, #0
 22c:	91c80020 	smin	x0, x1, #0
 230:	91c80400 	smin	x0, x0, #1
 234:	91cb8102 	smin	x2, x8, #-32
 238:	91ca034d 	smin	x13, x26, #-128
 23c:	91c9fd31 	smin	x17, x9, #127
 240:	11cc0000 	umin	w0, w0, #0
 244:	11cc0001 	umin	w1, w0, #0
 248:	11cc0020 	umin	w0, w1, #0
 24c:	11cc0400 	umin	w0, w0, #1
 250:	11cc8902 	umin	w2, w8, #34
 254:	11ce034d 	umin	w13, w26, #128
 258:	11cffd31 	umin	w17, w9, #255
 25c:	91cc0000 	umin	x0, x0, #0
 260:	91cc0001 	umin	x1, x0, #0
 264:	91cc0020 	umin	x0, x1, #0
 268:	91cc0400 	umin	x0, x0, #1
 26c:	91cc8902 	umin	x2, x8, #34
 270:	91ce034d 	umin	x13, x26, #128
 274:	91cffd31 	umin	x17, x9, #255
