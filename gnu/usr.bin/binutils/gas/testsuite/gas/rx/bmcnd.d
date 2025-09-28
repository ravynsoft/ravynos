#source: ./bmcnd.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fc e0 02                      	bmc	#0, \[r0\]\.b
   3:	fc e0 f2                      	bmc	#0, \[r15\]\.b
   6:	fc e1 02 fc                   	bmc	#0, 252\[r0\]\.b
   a:	fc e1 f2 fc                   	bmc	#0, 252\[r15\]\.b
   e:	fc e2 02 fc ff                	bmc	#0, 65532\[r0\]\.b
  13:	fc e2 f2 fc ff                	bmc	#0, 65532\[r15\]\.b
  18:	fc fc 02                      	bmc	#7, \[r0\]\.b
  1b:	fc fc f2                      	bmc	#7, \[r15\]\.b
  1e:	fc fd 02 fc                   	bmc	#7, 252\[r0\]\.b
  22:	fc fd f2 fc                   	bmc	#7, 252\[r15\]\.b
  26:	fc fe 02 fc ff                	bmc	#7, 65532\[r0\]\.b
  2b:	fc fe f2 fc ff                	bmc	#7, 65532\[r15\]\.b
  30:	fc e0 02                      	bmc	#0, \[r0\]\.b
  33:	fc e0 f2                      	bmc	#0, \[r15\]\.b
  36:	fc e1 02 fc                   	bmc	#0, 252\[r0\]\.b
  3a:	fc e1 f2 fc                   	bmc	#0, 252\[r15\]\.b
  3e:	fc e2 02 fc ff                	bmc	#0, 65532\[r0\]\.b
  43:	fc e2 f2 fc ff                	bmc	#0, 65532\[r15\]\.b
  48:	fc fc 02                      	bmc	#7, \[r0\]\.b
  4b:	fc fc f2                      	bmc	#7, \[r15\]\.b
  4e:	fc fd 02 fc                   	bmc	#7, 252\[r0\]\.b
  52:	fc fd f2 fc                   	bmc	#7, 252\[r15\]\.b
  56:	fc fe 02 fc ff                	bmc	#7, 65532\[r0\]\.b
  5b:	fc fe f2 fc ff                	bmc	#7, 65532\[r15\]\.b
  60:	fc e0 00                      	bmeq	#0, \[r0\]\.b
  63:	fc e0 f0                      	bmeq	#0, \[r15\]\.b
  66:	fc e1 00 fc                   	bmeq	#0, 252\[r0\]\.b
  6a:	fc e1 f0 fc                   	bmeq	#0, 252\[r15\]\.b
  6e:	fc e2 00 fc ff                	bmeq	#0, 65532\[r0\]\.b
  73:	fc e2 f0 fc ff                	bmeq	#0, 65532\[r15\]\.b
  78:	fc fc 00                      	bmeq	#7, \[r0\]\.b
  7b:	fc fc f0                      	bmeq	#7, \[r15\]\.b
  7e:	fc fd 00 fc                   	bmeq	#7, 252\[r0\]\.b
  82:	fc fd f0 fc                   	bmeq	#7, 252\[r15\]\.b
  86:	fc fe 00 fc ff                	bmeq	#7, 65532\[r0\]\.b
  8b:	fc fe f0 fc ff                	bmeq	#7, 65532\[r15\]\.b
  90:	fc e0 00                      	bmeq	#0, \[r0\]\.b
  93:	fc e0 f0                      	bmeq	#0, \[r15\]\.b
  96:	fc e1 00 fc                   	bmeq	#0, 252\[r0\]\.b
  9a:	fc e1 f0 fc                   	bmeq	#0, 252\[r15\]\.b
  9e:	fc e2 00 fc ff                	bmeq	#0, 65532\[r0\]\.b
  a3:	fc e2 f0 fc ff                	bmeq	#0, 65532\[r15\]\.b
  a8:	fc fc 00                      	bmeq	#7, \[r0\]\.b
  ab:	fc fc f0                      	bmeq	#7, \[r15\]\.b
  ae:	fc fd 00 fc                   	bmeq	#7, 252\[r0\]\.b
  b2:	fc fd f0 fc                   	bmeq	#7, 252\[r15\]\.b
  b6:	fc fe 00 fc ff                	bmeq	#7, 65532\[r0\]\.b
  bb:	fc fe f0 fc ff                	bmeq	#7, 65532\[r15\]\.b
  c0:	fc e0 04                      	bmgtu	#0, \[r0\]\.b
  c3:	fc e0 f4                      	bmgtu	#0, \[r15\]\.b
  c6:	fc e1 04 fc                   	bmgtu	#0, 252\[r0\]\.b
  ca:	fc e1 f4 fc                   	bmgtu	#0, 252\[r15\]\.b
  ce:	fc e2 04 fc ff                	bmgtu	#0, 65532\[r0\]\.b
  d3:	fc e2 f4 fc ff                	bmgtu	#0, 65532\[r15\]\.b
  d8:	fc fc 04                      	bmgtu	#7, \[r0\]\.b
  db:	fc fc f4                      	bmgtu	#7, \[r15\]\.b
  de:	fc fd 04 fc                   	bmgtu	#7, 252\[r0\]\.b
  e2:	fc fd f4 fc                   	bmgtu	#7, 252\[r15\]\.b
  e6:	fc fe 04 fc ff                	bmgtu	#7, 65532\[r0\]\.b
  eb:	fc fe f4 fc ff                	bmgtu	#7, 65532\[r15\]\.b
  f0:	fc e0 06                      	bmpz	#0, \[r0\]\.b
  f3:	fc e0 f6                      	bmpz	#0, \[r15\]\.b
  f6:	fc e1 06 fc                   	bmpz	#0, 252\[r0\]\.b
  fa:	fc e1 f6 fc                   	bmpz	#0, 252\[r15\]\.b
  fe:	fc e2 06 fc ff                	bmpz	#0, 65532\[r0\]\.b
 103:	fc e2 f6 fc ff                	bmpz	#0, 65532\[r15\]\.b
 108:	fc fc 06                      	bmpz	#7, \[r0\]\.b
 10b:	fc fc f6                      	bmpz	#7, \[r15\]\.b
 10e:	fc fd 06 fc                   	bmpz	#7, 252\[r0\]\.b
 112:	fc fd f6 fc                   	bmpz	#7, 252\[r15\]\.b
 116:	fc fe 06 fc ff                	bmpz	#7, 65532\[r0\]\.b
 11b:	fc fe f6 fc ff                	bmpz	#7, 65532\[r15\]\.b
 120:	fc e0 08                      	bmge	#0, \[r0\]\.b
 123:	fc e0 f8                      	bmge	#0, \[r15\]\.b
 126:	fc e1 08 fc                   	bmge	#0, 252\[r0\]\.b
 12a:	fc e1 f8 fc                   	bmge	#0, 252\[r15\]\.b
 12e:	fc e2 08 fc ff                	bmge	#0, 65532\[r0\]\.b
 133:	fc e2 f8 fc ff                	bmge	#0, 65532\[r15\]\.b
 138:	fc fc 08                      	bmge	#7, \[r0\]\.b
 13b:	fc fc f8                      	bmge	#7, \[r15\]\.b
 13e:	fc fd 08 fc                   	bmge	#7, 252\[r0\]\.b
 142:	fc fd f8 fc                   	bmge	#7, 252\[r15\]\.b
 146:	fc fe 08 fc ff                	bmge	#7, 65532\[r0\]\.b
 14b:	fc fe f8 fc ff                	bmge	#7, 65532\[r15\]\.b
 150:	fc e0 0a                      	bmgt	#0, \[r0\]\.b
 153:	fc e0 fa                      	bmgt	#0, \[r15\]\.b
 156:	fc e1 0a fc                   	bmgt	#0, 252\[r0\]\.b
 15a:	fc e1 fa fc                   	bmgt	#0, 252\[r15\]\.b
 15e:	fc e2 0a fc ff                	bmgt	#0, 65532\[r0\]\.b
 163:	fc e2 fa fc ff                	bmgt	#0, 65532\[r15\]\.b
 168:	fc fc 0a                      	bmgt	#7, \[r0\]\.b
 16b:	fc fc fa                      	bmgt	#7, \[r15\]\.b
 16e:	fc fd 0a fc                   	bmgt	#7, 252\[r0\]\.b
 172:	fc fd fa fc                   	bmgt	#7, 252\[r15\]\.b
 176:	fc fe 0a fc ff                	bmgt	#7, 65532\[r0\]\.b
 17b:	fc fe fa fc ff                	bmgt	#7, 65532\[r15\]\.b
 180:	fc e0 0c                      	bmo	#0, \[r0\]\.b
 183:	fc e0 fc                      	bmo	#0, \[r15\]\.b
 186:	fc e1 0c fc                   	bmo	#0, 252\[r0\]\.b
 18a:	fc e1 fc fc                   	bmo	#0, 252\[r15\]\.b
 18e:	fc e2 0c fc ff                	bmo	#0, 65532\[r0\]\.b
 193:	fc e2 fc fc ff                	bmo	#0, 65532\[r15\]\.b
 198:	fc fc 0c                      	bmo	#7, \[r0\]\.b
 19b:	fc fc fc                      	bmo	#7, \[r15\]\.b
 19e:	fc fd 0c fc                   	bmo	#7, 252\[r0\]\.b
 1a2:	fc fd fc fc                   	bmo	#7, 252\[r15\]\.b
 1a6:	fc fe 0c fc ff                	bmo	#7, 65532\[r0\]\.b
 1ab:	fc fe fc fc ff                	bmo	#7, 65532\[r15\]\.b
 1b0:	fc e0 03                      	bmnc	#0, \[r0\]\.b
 1b3:	fc e0 f3                      	bmnc	#0, \[r15\]\.b
 1b6:	fc e1 03 fc                   	bmnc	#0, 252\[r0\]\.b
 1ba:	fc e1 f3 fc                   	bmnc	#0, 252\[r15\]\.b
 1be:	fc e2 03 fc ff                	bmnc	#0, 65532\[r0\]\.b
 1c3:	fc e2 f3 fc ff                	bmnc	#0, 65532\[r15\]\.b
 1c8:	fc fc 03                      	bmnc	#7, \[r0\]\.b
 1cb:	fc fc f3                      	bmnc	#7, \[r15\]\.b
 1ce:	fc fd 03 fc                   	bmnc	#7, 252\[r0\]\.b
 1d2:	fc fd f3 fc                   	bmnc	#7, 252\[r15\]\.b
 1d6:	fc fe 03 fc ff                	bmnc	#7, 65532\[r0\]\.b
 1db:	fc fe f3 fc ff                	bmnc	#7, 65532\[r15\]\.b
 1e0:	fc e0 03                      	bmnc	#0, \[r0\]\.b
 1e3:	fc e0 f3                      	bmnc	#0, \[r15\]\.b
 1e6:	fc e1 03 fc                   	bmnc	#0, 252\[r0\]\.b
 1ea:	fc e1 f3 fc                   	bmnc	#0, 252\[r15\]\.b
 1ee:	fc e2 03 fc ff                	bmnc	#0, 65532\[r0\]\.b
 1f3:	fc e2 f3 fc ff                	bmnc	#0, 65532\[r15\]\.b
 1f8:	fc fc 03                      	bmnc	#7, \[r0\]\.b
 1fb:	fc fc f3                      	bmnc	#7, \[r15\]\.b
 1fe:	fc fd 03 fc                   	bmnc	#7, 252\[r0\]\.b
 202:	fc fd f3 fc                   	bmnc	#7, 252\[r15\]\.b
 206:	fc fe 03 fc ff                	bmnc	#7, 65532\[r0\]\.b
 20b:	fc fe f3 fc ff                	bmnc	#7, 65532\[r15\]\.b
 210:	fc e0 01                      	bmne	#0, \[r0\]\.b
 213:	fc e0 f1                      	bmne	#0, \[r15\]\.b
 216:	fc e1 01 fc                   	bmne	#0, 252\[r0\]\.b
 21a:	fc e1 f1 fc                   	bmne	#0, 252\[r15\]\.b
 21e:	fc e2 01 fc ff                	bmne	#0, 65532\[r0\]\.b
 223:	fc e2 f1 fc ff                	bmne	#0, 65532\[r15\]\.b
 228:	fc fc 01                      	bmne	#7, \[r0\]\.b
 22b:	fc fc f1                      	bmne	#7, \[r15\]\.b
 22e:	fc fd 01 fc                   	bmne	#7, 252\[r0\]\.b
 232:	fc fd f1 fc                   	bmne	#7, 252\[r15\]\.b
 236:	fc fe 01 fc ff                	bmne	#7, 65532\[r0\]\.b
 23b:	fc fe f1 fc ff                	bmne	#7, 65532\[r15\]\.b
 240:	fc e0 01                      	bmne	#0, \[r0\]\.b
 243:	fc e0 f1                      	bmne	#0, \[r15\]\.b
 246:	fc e1 01 fc                   	bmne	#0, 252\[r0\]\.b
 24a:	fc e1 f1 fc                   	bmne	#0, 252\[r15\]\.b
 24e:	fc e2 01 fc ff                	bmne	#0, 65532\[r0\]\.b
 253:	fc e2 f1 fc ff                	bmne	#0, 65532\[r15\]\.b
 258:	fc fc 01                      	bmne	#7, \[r0\]\.b
 25b:	fc fc f1                      	bmne	#7, \[r15\]\.b
 25e:	fc fd 01 fc                   	bmne	#7, 252\[r0\]\.b
 262:	fc fd f1 fc                   	bmne	#7, 252\[r15\]\.b
 266:	fc fe 01 fc ff                	bmne	#7, 65532\[r0\]\.b
 26b:	fc fe f1 fc ff                	bmne	#7, 65532\[r15\]\.b
 270:	fc e0 05                      	bmleu	#0, \[r0\]\.b
 273:	fc e0 f5                      	bmleu	#0, \[r15\]\.b
 276:	fc e1 05 fc                   	bmleu	#0, 252\[r0\]\.b
 27a:	fc e1 f5 fc                   	bmleu	#0, 252\[r15\]\.b
 27e:	fc e2 05 fc ff                	bmleu	#0, 65532\[r0\]\.b
 283:	fc e2 f5 fc ff                	bmleu	#0, 65532\[r15\]\.b
 288:	fc fc 05                      	bmleu	#7, \[r0\]\.b
 28b:	fc fc f5                      	bmleu	#7, \[r15\]\.b
 28e:	fc fd 05 fc                   	bmleu	#7, 252\[r0\]\.b
 292:	fc fd f5 fc                   	bmleu	#7, 252\[r15\]\.b
 296:	fc fe 05 fc ff                	bmleu	#7, 65532\[r0\]\.b
 29b:	fc fe f5 fc ff                	bmleu	#7, 65532\[r15\]\.b
 2a0:	fc e0 07                      	bmn	#0, \[r0\]\.b
 2a3:	fc e0 f7                      	bmn	#0, \[r15\]\.b
 2a6:	fc e1 07 fc                   	bmn	#0, 252\[r0\]\.b
 2aa:	fc e1 f7 fc                   	bmn	#0, 252\[r15\]\.b
 2ae:	fc e2 07 fc ff                	bmn	#0, 65532\[r0\]\.b
 2b3:	fc e2 f7 fc ff                	bmn	#0, 65532\[r15\]\.b
 2b8:	fc fc 07                      	bmn	#7, \[r0\]\.b
 2bb:	fc fc f7                      	bmn	#7, \[r15\]\.b
 2be:	fc fd 07 fc                   	bmn	#7, 252\[r0\]\.b
 2c2:	fc fd f7 fc                   	bmn	#7, 252\[r15\]\.b
 2c6:	fc fe 07 fc ff                	bmn	#7, 65532\[r0\]\.b
 2cb:	fc fe f7 fc ff                	bmn	#7, 65532\[r15\]\.b
 2d0:	fc e0 09                      	bmlt	#0, \[r0\]\.b
 2d3:	fc e0 f9                      	bmlt	#0, \[r15\]\.b
 2d6:	fc e1 09 fc                   	bmlt	#0, 252\[r0\]\.b
 2da:	fc e1 f9 fc                   	bmlt	#0, 252\[r15\]\.b
 2de:	fc e2 09 fc ff                	bmlt	#0, 65532\[r0\]\.b
 2e3:	fc e2 f9 fc ff                	bmlt	#0, 65532\[r15\]\.b
 2e8:	fc fc 09                      	bmlt	#7, \[r0\]\.b
 2eb:	fc fc f9                      	bmlt	#7, \[r15\]\.b
 2ee:	fc fd 09 fc                   	bmlt	#7, 252\[r0\]\.b
 2f2:	fc fd f9 fc                   	bmlt	#7, 252\[r15\]\.b
 2f6:	fc fe 09 fc ff                	bmlt	#7, 65532\[r0\]\.b
 2fb:	fc fe f9 fc ff                	bmlt	#7, 65532\[r15\]\.b
 300:	fc e0 0b                      	bmle	#0, \[r0\]\.b
 303:	fc e0 fb                      	bmle	#0, \[r15\]\.b
 306:	fc e1 0b fc                   	bmle	#0, 252\[r0\]\.b
 30a:	fc e1 fb fc                   	bmle	#0, 252\[r15\]\.b
 30e:	fc e2 0b fc ff                	bmle	#0, 65532\[r0\]\.b
 313:	fc e2 fb fc ff                	bmle	#0, 65532\[r15\]\.b
 318:	fc fc 0b                      	bmle	#7, \[r0\]\.b
 31b:	fc fc fb                      	bmle	#7, \[r15\]\.b
 31e:	fc fd 0b fc                   	bmle	#7, 252\[r0\]\.b
 322:	fc fd fb fc                   	bmle	#7, 252\[r15\]\.b
 326:	fc fe 0b fc ff                	bmle	#7, 65532\[r0\]\.b
 32b:	fc fe fb fc ff                	bmle	#7, 65532\[r15\]\.b
 330:	fc e0 0d                      	bmno	#0, \[r0\]\.b
 333:	fc e0 fd                      	bmno	#0, \[r15\]\.b
 336:	fc e1 0d fc                   	bmno	#0, 252\[r0\]\.b
 33a:	fc e1 fd fc                   	bmno	#0, 252\[r15\]\.b
 33e:	fc e2 0d fc ff                	bmno	#0, 65532\[r0\]\.b
 343:	fc e2 fd fc ff                	bmno	#0, 65532\[r15\]\.b
 348:	fc fc 0d                      	bmno	#7, \[r0\]\.b
 34b:	fc fc fd                      	bmno	#7, \[r15\]\.b
 34e:	fc fd 0d fc                   	bmno	#7, 252\[r0\]\.b
 352:	fc fd fd fc                   	bmno	#7, 252\[r15\]\.b
 356:	fc fe 0d fc ff                	bmno	#7, 65532\[r0\]\.b
 35b:	fc fe fd fc ff                	bmno	#7, 65532\[r15\]\.b
 360:	fd e0 20                      	bmc	#0, r0
 363:	fd e0 2f                      	bmc	#0, r15
 366:	fd ff 20                      	bmc	#31, r0
 369:	fd ff 2f                      	bmc	#31, r15
 36c:	fd e0 20                      	bmc	#0, r0
 36f:	fd e0 2f                      	bmc	#0, r15
 372:	fd ff 20                      	bmc	#31, r0
 375:	fd ff 2f                      	bmc	#31, r15
 378:	fd e0 00                      	bmeq	#0, r0
 37b:	fd e0 0f                      	bmeq	#0, r15
 37e:	fd ff 00                      	bmeq	#31, r0
 381:	fd ff 0f                      	bmeq	#31, r15
 384:	fd e0 00                      	bmeq	#0, r0
 387:	fd e0 0f                      	bmeq	#0, r15
 38a:	fd ff 00                      	bmeq	#31, r0
 38d:	fd ff 0f                      	bmeq	#31, r15
 390:	fd e0 40                      	bmgtu	#0, r0
 393:	fd e0 4f                      	bmgtu	#0, r15
 396:	fd ff 40                      	bmgtu	#31, r0
 399:	fd ff 4f                      	bmgtu	#31, r15
 39c:	fd e0 60                      	bmpz	#0, r0
 39f:	fd e0 6f                      	bmpz	#0, r15
 3a2:	fd ff 60                      	bmpz	#31, r0
 3a5:	fd ff 6f                      	bmpz	#31, r15
 3a8:	fd e0 80                      	bmge	#0, r0
 3ab:	fd e0 8f                      	bmge	#0, r15
 3ae:	fd ff 80                      	bmge	#31, r0
 3b1:	fd ff 8f                      	bmge	#31, r15
 3b4:	fd e0 a0                      	bmgt	#0, r0
 3b7:	fd e0 af                      	bmgt	#0, r15
 3ba:	fd ff a0                      	bmgt	#31, r0
 3bd:	fd ff af                      	bmgt	#31, r15
 3c0:	fd e0 c0                      	bmo	#0, r0
 3c3:	fd e0 cf                      	bmo	#0, r15
 3c6:	fd ff c0                      	bmo	#31, r0
 3c9:	fd ff cf                      	bmo	#31, r15
 3cc:	fd e0 30                      	bmnc	#0, r0
 3cf:	fd e0 3f                      	bmnc	#0, r15
 3d2:	fd ff 30                      	bmnc	#31, r0
 3d5:	fd ff 3f                      	bmnc	#31, r15
 3d8:	fd e0 30                      	bmnc	#0, r0
 3db:	fd e0 3f                      	bmnc	#0, r15
 3de:	fd ff 30                      	bmnc	#31, r0
 3e1:	fd ff 3f                      	bmnc	#31, r15
 3e4:	fd e0 10                      	bmne	#0, r0
 3e7:	fd e0 1f                      	bmne	#0, r15
 3ea:	fd ff 10                      	bmne	#31, r0
 3ed:	fd ff 1f                      	bmne	#31, r15
 3f0:	fd e0 10                      	bmne	#0, r0
 3f3:	fd e0 1f                      	bmne	#0, r15
 3f6:	fd ff 10                      	bmne	#31, r0
 3f9:	fd ff 1f                      	bmne	#31, r15
 3fc:	fd e0 50                      	bmleu	#0, r0
 3ff:	fd e0 5f                      	bmleu	#0, r15
 402:	fd ff 50                      	bmleu	#31, r0
 405:	fd ff 5f                      	bmleu	#31, r15
 408:	fd e0 70                      	bmn	#0, r0
 40b:	fd e0 7f                      	bmn	#0, r15
 40e:	fd ff 70                      	bmn	#31, r0
 411:	fd ff 7f                      	bmn	#31, r15
 414:	fd e0 90                      	bmlt	#0, r0
 417:	fd e0 9f                      	bmlt	#0, r15
 41a:	fd ff 90                      	bmlt	#31, r0
 41d:	fd ff 9f                      	bmlt	#31, r15
 420:	fd e0 b0                      	bmle	#0, r0
 423:	fd e0 bf                      	bmle	#0, r15
 426:	fd ff b0                      	bmle	#31, r0
 429:	fd ff bf                      	bmle	#31, r15
 42c:	fd e0 d0                      	bmno	#0, r0
 42f:	fd e0 df                      	bmno	#0, r15
 432:	fd ff d0                      	bmno	#31, r0
 435:	fd ff df                      	bmno	#31, r15
