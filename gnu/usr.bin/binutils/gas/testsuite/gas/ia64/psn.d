# as: -xnone -W 
# objdump: -d
# name: ia64 psn 

.*: +file format elf..-ia64.*

Disassembly of section \.text:

0+000 <AAAAA-0x1.*>:
       0:	08 00 04 05 60 19 	\[MMI\]       lfetch.count \[r2\],1,64
       6:	40 f8 5a c0 32 00 	            lfetch.count \[r22\],5,-64
       c:	00 00 04 00       	            nop.i 0x0
      10:	08 40 3c 2f 62 19 	\[MMI\]       lfetch.count.nt1 \[r23\],9,960
      16:	b0 80 ea c5 32 00 	            lfetch.count.nt1 \[r122\],12,-1024
      1c:	00 00 04 00       	            nop.i 0x0
      20:	08 78 08 0b 64 19 	\[MMI\]       lfetch.count.nt2 \[r5\],16,128
      26:	30 e1 3e c8 32 00 	            lfetch.count.nt2 \[r15\],20,-256
      2c:	00 00 04 00       	            nop.i 0x0
      30:	08 b8 20 fb 66 19 	\[MMI\]       lfetch.count.nta \[r125\],24,512
      36:	c0 79 22 cc 32 00 	            lfetch.count.nta \[r8\],29,960
      3c:	00 00 04 00       	            nop.i 0x0
      40:	08 08 43 25 60 19 	\[MMI\]       lfetch.count.d4 \[r18\],34,-1024
      46:	d0 77 fe c5 32 00 	            lfetch.count.d5 \[r127\],62,896
      4c:	00 00 04 00       	            nop.i 0x0
      50:	09 f0 43 15 64 19 	\[MMI\]       lfetch.count.d6 \[r10\],63,-1024
      56:	f0 07 82 cd 32 20 	            lfetch.count.d7 \[r96\],64,0
      5c:	10 04 08 50       	            tf.z p1,p2=32;;
      60:	02 00 00 00 01 00 	\[MII\]       nop.m 0x0
      66:	20 08 02 0e 28 60 	            tf.z p2,p7=32;;
      6c:	18 04 08 50       	            tf.z.unc p3,p2=32
      70:	00 00 00 00 01 00 	\[MII\]       nop.m 0x0
      76:	40 18 02 06 28 a0 	            tf.z p4,p3=33
      7c:	50 04 10 58       	            tf.z.and p5,p4=34
      80:	00 00 00 00 01 00 	\[MII\]       nop.m 0x0
      86:	50 3c 02 0c 2c c0 	            tf.nz.and p5,p6=35
      8c:	70 04 14 58       	            tf.z.and p6,p5=35
      90:	00 00 00 00 01 00 	\[MII\]       nop.m 0x0
      96:	70 f8 03 8c 28 a0 	            tf.z.or p7,p6=63
      9c:	78 04 18 51       	            tf.nz.or p5,p6=35
      a0:	00 00 00 00 01 00 	\[MII\]       nop.m 0x0
      a6:	70 18 02 8c 2c e0 	            tf.z.or.andcm p7,p6=33
      ac:	58 04 18 59       	            tf.nz.or.andcm p7,p6=34
      b0:	00 00 00 00 01 00 	\[MII\]       nop.m 0x0
      b6:	60 0c 02 8e 2c c0 	            tf.nz.or.andcm p6,p7=32
      bc:	30 04 1c 59       	            tf.z.or.andcm p6,p7=33
      c0:	11 00 00 00 01 00 	\[MIB\]       nop.m 0x0
      c6:	00 1c 02 0c 28 00 	            tf.z.unc p0,p6=33
      cc:	00 00 00 20       	            nop.b 0x0;;
      d0:	08 00 02 24 60 19 	\[MMI\]       lfetch.d4 \[r18\]
      d6:	00 00 00 02 00 00 	            nop.m 0x0
      dc:	00 00 04 00       	            nop.i 0x0
      e0:	0a 00 02 26 7e 19 	\[MMI\]       lfetch.fault.excl.d7 \[r19\];;
      e6:	10 10 3a c0 32 00 	            lfetch.count \[r14\],2,128
      ec:	01 50 58 00       	            sxt4 r8=r10
      f0:	0a f8 13 17 60 19 	\[MMI\]       lfetch.count.d4 \[r11\],64,256;;
      f6:	00 04 44 d4 32 00 	            lfetch.excl.d5 \[r17\]
      fc:	00 00 04 00       	            nop.i 0x0
     100:	0b 00 02 20 74 19 	\[MMI\]       lfetch.fault.d6 \[r16\];;
     106:	70 01 e0 03 00 60 	            mov dahr7=7
     10c:	00 48 68 73       	            clz r3=r9;;
     110:	00 b0 00 e0 01 00 	\[MII\]       mov dahr6=6
     116:	20 48 20 34 3c 40 	            mpy4 r2=r9,r8
     11c:	90 40 78 78       	            mpyshl4 r2=r9,r8
     120:	0b a8 00 d0 01 00 	\[MMI\]       mov dahr5=5;;
     126:	40 01 80 03 00 00 	            mov dahr4=4
     12c:	00 00 04 00       	            nop.i 0x0;;
     130:	11 98 00 b0 01 00 	\[MIB\]       mov dahr3=3
     136:	80 10 0c 00 40 00 	            add r8=r2,r3
     13c:	00 00 00 20       	            nop.b 0x0;;
     140:	0b 90 00 a0 01 00 	\[MMI\]       mov dahr2=2;;
     146:	10 01 20 03 00 00 	            mov dahr1=1
     14c:	00 00 04 00       	            nop.i 0x0;;
     150:	10 80 00 80 01 00 	\[MIB\]       mov dahr0=0
     156:	00 00 00 02 00 00 	            nop.i 0x0
     15c:	00 00 00 20       	            nop.b 0x0
     160:	08 60 00 0a 20 04 	\[MMI\]       mov r12=dahr\[r5\]
     166:	a0 07 dc 40 08 00 	            mov r122=dahr\[r55\]
     16c:	00 00 04 00       	            nop.i 0x0
     170:	08 00 74 83 80 11 	\[MMI\]       st1 \[r65\]=r93
     176:	00 e8 06 05 23 00 	            st1.d1 \[r65\]=r93
     17c:	00 00 04 00       	            nop.i 0x0
     180:	08 00 74 83 82 11 	\[MMI\]       st1.d1 \[r65\]=r93
     186:	00 e8 06 09 23 00 	            st1.d2 \[r65\]=r93
     18c:	00 00 04 00       	            nop.i 0x0
     190:	08 00 74 83 84 11 	\[MMI\]       st1.d2 \[r65\]=r93
     196:	00 e8 06 0d 23 00 	            st1.nta \[r65\]=r93
     19c:	00 00 04 00       	            nop.i 0x0
     1a0:	08 00 74 83 86 11 	\[MMI\]       st1.nta \[r65\]=r93
     1a6:	00 ec 06 01 23 00 	            st1.d4 \[r65\]=r93
     1ac:	00 00 04 00       	            nop.i 0x0
     1b0:	08 00 76 83 82 11 	\[MMI\]       st1.d5 \[r65\]=r93
     1b6:	00 ec 06 09 23 00 	            st1.d6 \[r65\]=r93
     1bc:	00 00 04 00       	            nop.i 0x0
     1c0:	08 00 76 83 86 11 	\[MMI\]       st1.d7 \[r65\]=r93
     1c6:	00 e8 06 11 23 00 	            st2 \[r65\]=r93
     1cc:	00 00 04 00       	            nop.i 0x0
     1d0:	08 00 74 83 8a 11 	\[MMI\]       st2.d1 \[r65\]=r93
     1d6:	00 e8 06 15 23 00 	            st2.d1 \[r65\]=r93
     1dc:	00 00 04 00       	            nop.i 0x0
     1e0:	08 00 74 83 8c 11 	\[MMI\]       st2.d2 \[r65\]=r93
     1e6:	00 e8 06 19 23 00 	            st2.d2 \[r65\]=r93
     1ec:	00 00 04 00       	            nop.i 0x0
     1f0:	08 00 74 83 8e 11 	\[MMI\]       st2.nta \[r65\]=r93
     1f6:	00 e8 06 1d 23 00 	            st2.nta \[r65\]=r93
     1fc:	00 00 04 00       	            nop.i 0x0
     200:	08 00 76 83 88 11 	\[MMI\]       st2.d4 \[r65\]=r93
     206:	00 ec 06 15 23 00 	            st2.d5 \[r65\]=r93
     20c:	00 00 04 00       	            nop.i 0x0
     210:	08 00 76 83 8c 11 	\[MMI\]       st2.d6 \[r65\]=r93
     216:	00 ec 06 1d 23 00 	            st2.d7 \[r65\]=r93
     21c:	00 00 04 00       	            nop.i 0x0
     220:	08 00 74 83 90 11 	\[MMI\]       st4 \[r65\]=r93
     226:	00 e8 06 25 23 00 	            st4.d1 \[r65\]=r93
     22c:	00 00 04 00       	            nop.i 0x0
     230:	08 00 74 83 92 11 	\[MMI\]       st4.d1 \[r65\]=r93
     236:	00 e8 06 29 23 00 	            st4.d2 \[r65\]=r93
     23c:	00 00 04 00       	            nop.i 0x0
     240:	08 00 74 83 94 11 	\[MMI\]       st4.d2 \[r65\]=r93
     246:	00 e8 06 2d 23 00 	            st4.nta \[r65\]=r93
     24c:	00 00 04 00       	            nop.i 0x0
     250:	08 00 74 83 96 11 	\[MMI\]       st4.nta \[r65\]=r93
     256:	00 ec 06 21 23 00 	            st4.d4 \[r65\]=r93
     25c:	00 00 04 00       	            nop.i 0x0
     260:	08 00 76 83 92 11 	\[MMI\]       st4.d5 \[r65\]=r93
     266:	00 ec 06 29 23 00 	            st4.d6 \[r65\]=r93
     26c:	00 00 04 00       	            nop.i 0x0
     270:	08 00 76 83 96 11 	\[MMI\]       st4.d7 \[r65\]=r93
     276:	00 e8 06 31 23 00 	            st8 \[r65\]=r93
     27c:	00 00 04 00       	            nop.i 0x0
     280:	08 00 74 83 9a 11 	\[MMI\]       st8.d1 \[r65\]=r93
     286:	00 e8 06 35 23 00 	            st8.d1 \[r65\]=r93
     28c:	00 00 04 00       	            nop.i 0x0
     290:	08 00 74 83 9c 11 	\[MMI\]       st8.d2 \[r65\]=r93
     296:	00 e8 06 39 23 00 	            st8.d2 \[r65\]=r93
     29c:	00 00 04 00       	            nop.i 0x0
     2a0:	08 00 74 83 9e 11 	\[MMI\]       st8.nta \[r65\]=r93
     2a6:	00 e8 06 3d 23 00 	            st8.nta \[r65\]=r93
     2ac:	00 00 04 00       	            nop.i 0x0
     2b0:	08 00 76 83 98 11 	\[MMI\]       st8.d4 \[r65\]=r93
     2b6:	00 ec 06 35 23 00 	            st8.d5 \[r65\]=r93
     2bc:	00 00 04 00       	            nop.i 0x0
     2c0:	08 00 76 83 9c 11 	\[MMI\]       st8.d6 \[r65\]=r93
     2c6:	00 ec 06 3d 23 00 	            st8.d7 \[r65\]=r93
     2cc:	00 00 04 00       	            nop.i 0x0
     2d0:	08 00 74 83 81 11 	\[MMI\]       st16 \[r65\]=r93,ar.csd
     2d6:	00 e8 06 03 23 00 	            st16 \[r65\]=r93,ar.csd
     2dc:	00 00 04 00       	            nop.i 0x0
     2e0:	08 00 74 83 83 11 	\[MMI\]       st16.d1 \[r65\]=r93,ar.csd
     2e6:	00 e8 06 07 23 00 	            st16.d1 \[r65\]=r93,ar.csd
     2ec:	00 00 04 00       	            nop.i 0x0
     2f0:	08 00 74 83 85 11 	\[MMI\]       st16.d2 \[r65\]=r93,ar.csd
     2f6:	00 e8 06 0b 23 00 	            st16.d2 \[r65\]=r93,ar.csd
     2fc:	00 00 04 00       	            nop.i 0x0
     300:	08 00 74 83 87 11 	\[MMI\]       st16.nta \[r65\]=r93,ar.csd
     306:	00 e8 06 0f 23 00 	            st16.nta \[r65\]=r93,ar.csd
     30c:	00 00 04 00       	            nop.i 0x0
     310:	08 00 76 83 81 11 	\[MMI\]       st16.d4 \[r65\]=r93,ar.csd
     316:	00 ec 06 07 23 00 	            st16.d5 \[r65\]=r93,ar.csd
     31c:	00 00 04 00       	            nop.i 0x0
     320:	08 00 76 83 85 11 	\[MMI\]       st16.d6 \[r65\]=r93,ar.csd
     326:	00 ec 06 0f 23 00 	            st16.d7 \[r65\]=r93,ar.csd
     32c:	00 00 04 00       	            nop.i 0x0
     330:	08 00 74 83 87 11 	\[MMI\]       st16.nta \[r65\]=r93,ar.csd
     336:	00 e8 06 0f 23 00 	            st16.nta \[r65\]=r93,ar.csd
     33c:	00 00 04 00       	            nop.i 0x0
     340:	08 00 76 83 81 11 	\[MMI\]       st16.d4 \[r65\]=r93,ar.csd
     346:	00 ec 06 07 23 00 	            st16.d5 \[r65\]=r93,ar.csd
     34c:	00 00 04 00       	            nop.i 0x0
     350:	08 00 76 83 85 11 	\[MMI\]       st16.d6 \[r65\]=r93,ar.csd
     356:	00 ec 06 0f 23 00 	            st16.d7 \[r65\]=r93,ar.csd
     35c:	00 00 04 00       	            nop.i 0x0
     360:	08 00 74 83 a0 11 	\[MMI\]       st1.rel \[r65\]=r93
     366:	00 e8 06 45 23 00 	            st1.rel.d1 \[r65\]=r93
     36c:	00 00 04 00       	            nop.i 0x0
     370:	08 00 74 83 a2 11 	\[MMI\]       st1.rel.d1 \[r65\]=r93
     376:	00 e8 06 49 23 00 	            st1.rel.d2 \[r65\]=r93
     37c:	00 00 04 00       	            nop.i 0x0
     380:	08 00 74 83 a4 11 	\[MMI\]       st1.rel.d2 \[r65\]=r93
     386:	00 e8 06 4d 23 00 	            st1.rel.nta \[r65\]=r93
     38c:	00 00 04 00       	            nop.i 0x0
     390:	08 00 74 83 a6 11 	\[MMI\]       st1.rel.nta \[r65\]=r93
     396:	00 ec 06 41 23 00 	            st1.rel.d4 \[r65\]=r93
     39c:	00 00 04 00       	            nop.i 0x0
     3a0:	08 00 76 83 a2 11 	\[MMI\]       st1.rel.d5 \[r65\]=r93
     3a6:	00 ec 06 49 23 00 	            st1.rel.d6 \[r65\]=r93
     3ac:	00 00 04 00       	            nop.i 0x0
     3b0:	08 00 76 83 a6 11 	\[MMI\]       st1.rel.d7 \[r65\]=r93
     3b6:	00 e8 06 51 23 00 	            st2.rel \[r65\]=r93
     3bc:	00 00 04 00       	            nop.i 0x0
     3c0:	08 00 74 83 aa 11 	\[MMI\]       st2.rel.d1 \[r65\]=r93
     3c6:	00 e8 06 55 23 00 	            st2.rel.d1 \[r65\]=r93
     3cc:	00 00 04 00       	            nop.i 0x0
     3d0:	08 00 74 83 ac 11 	\[MMI\]       st2.rel.d2 \[r65\]=r93
     3d6:	00 e8 06 59 23 00 	            st2.rel.d2 \[r65\]=r93
     3dc:	00 00 04 00       	            nop.i 0x0
     3e0:	08 00 74 83 ae 11 	\[MMI\]       st2.rel.nta \[r65\]=r93
     3e6:	00 e8 06 5d 23 00 	            st2.rel.nta \[r65\]=r93
     3ec:	00 00 04 00       	            nop.i 0x0
     3f0:	08 00 76 83 a8 11 	\[MMI\]       st2.rel.d4 \[r65\]=r93
     3f6:	00 ec 06 55 23 00 	            st2.rel.d5 \[r65\]=r93
     3fc:	00 00 04 00       	            nop.i 0x0
     400:	08 00 76 83 ac 11 	\[MMI\]       st2.rel.d6 \[r65\]=r93
     406:	00 ec 06 5d 23 00 	            st2.rel.d7 \[r65\]=r93
     40c:	00 00 04 00       	            nop.i 0x0
     410:	08 00 74 83 b0 11 	\[MMI\]       st4.rel \[r65\]=r93
     416:	00 e8 06 65 23 00 	            st4.rel.d1 \[r65\]=r93
     41c:	00 00 04 00       	            nop.i 0x0
     420:	08 00 74 83 b2 11 	\[MMI\]       st4.rel.d1 \[r65\]=r93
     426:	00 e8 06 69 23 00 	            st4.rel.d2 \[r65\]=r93
     42c:	00 00 04 00       	            nop.i 0x0
     430:	08 00 74 83 b4 11 	\[MMI\]       st4.rel.d2 \[r65\]=r93
     436:	00 e8 06 6d 23 00 	            st4.rel.nta \[r65\]=r93
     43c:	00 00 04 00       	            nop.i 0x0
     440:	08 00 74 83 b6 11 	\[MMI\]       st4.rel.nta \[r65\]=r93
     446:	00 ec 06 61 23 00 	            st4.rel.d4 \[r65\]=r93
     44c:	00 00 04 00       	            nop.i 0x0
     450:	08 00 76 83 b2 11 	\[MMI\]       st4.rel.d5 \[r65\]=r93
     456:	00 ec 06 69 23 00 	            st4.rel.d6 \[r65\]=r93
     45c:	00 00 04 00       	            nop.i 0x0
     460:	08 00 76 83 b6 11 	\[MMI\]       st4.rel.d7 \[r65\]=r93
     466:	00 e8 06 71 23 00 	            st8.rel \[r65\]=r93
     46c:	00 00 04 00       	            nop.i 0x0
     470:	08 00 74 83 ba 11 	\[MMI\]       st8.rel.d1 \[r65\]=r93
     476:	00 e8 06 75 23 00 	            st8.rel.d1 \[r65\]=r93
     47c:	00 00 04 00       	            nop.i 0x0
     480:	08 00 74 83 bc 11 	\[MMI\]       st8.rel.d2 \[r65\]=r93
     486:	00 e8 06 79 23 00 	            st8.rel.d2 \[r65\]=r93
     48c:	00 00 04 00       	            nop.i 0x0
     490:	08 00 74 83 be 11 	\[MMI\]       st8.rel.nta \[r65\]=r93
     496:	00 e8 06 7d 23 00 	            st8.rel.nta \[r65\]=r93
     49c:	00 00 04 00       	            nop.i 0x0
     4a0:	08 00 76 83 b8 11 	\[MMI\]       st8.rel.d4 \[r65\]=r93
     4a6:	00 ec 06 75 23 00 	            st8.rel.d5 \[r65\]=r93
     4ac:	00 00 04 00       	            nop.i 0x0
     4b0:	08 00 76 83 bc 11 	\[MMI\]       st8.rel.d6 \[r65\]=r93
     4b6:	00 ec 06 7d 23 00 	            st8.rel.d7 \[r65\]=r93
     4bc:	00 00 04 00       	            nop.i 0x0
     4c0:	08 00 74 83 a1 11 	\[MMI\]       st16.rel \[r65\]=r93,ar.csd
     4c6:	00 e8 06 43 23 00 	            st16.rel \[r65\]=r93,ar.csd
     4cc:	00 00 04 00       	            nop.i 0x0
     4d0:	08 00 74 83 a3 11 	\[MMI\]       st16.rel.d1 \[r65\]=r93,ar.csd
     4d6:	00 e8 06 47 23 00 	            st16.rel.d1 \[r65\]=r93,ar.csd
     4dc:	00 00 04 00       	            nop.i 0x0
     4e0:	08 00 74 83 a3 11 	\[MMI\]       st16.rel.d1 \[r65\]=r93,ar.csd
     4e6:	00 e8 06 47 23 00 	            st16.rel.d1 \[r65\]=r93,ar.csd
     4ec:	00 00 04 00       	            nop.i 0x0
     4f0:	08 00 74 83 a5 11 	\[MMI\]       st16.rel.d2 \[r65\]=r93,ar.csd
     4f6:	00 e8 06 4b 23 00 	            st16.rel.d2 \[r65\]=r93,ar.csd
     4fc:	00 00 04 00       	            nop.i 0x0
     500:	08 00 74 83 a5 11 	\[MMI\]       st16.rel.d2 \[r65\]=r93,ar.csd
     506:	00 e8 06 4b 23 00 	            st16.rel.d2 \[r65\]=r93,ar.csd
     50c:	00 00 04 00       	            nop.i 0x0
     510:	08 00 74 83 a7 11 	\[MMI\]       st16.rel.nta \[r65\]=r93,ar.csd
     516:	00 e8 06 4f 23 00 	            st16.rel.nta \[r65\]=r93,ar.csd
     51c:	00 00 04 00       	            nop.i 0x0
     520:	08 00 76 83 a1 11 	\[MMI\]       st16.rel.d4 \[r65\]=r93,ar.csd
     526:	00 ec 06 47 23 00 	            st16.rel.d5 \[r65\]=r93,ar.csd
     52c:	00 00 04 00       	            nop.i 0x0
     530:	08 00 76 83 a5 11 	\[MMI\]       st16.rel.d6 \[r65\]=r93,ar.csd
     536:	00 ec 06 4f 23 00 	            st16.rel.d7 \[r65\]=r93,ar.csd
     53c:	00 00 04 00       	            nop.i 0x0
     540:	08 00 74 83 a7 11 	\[MMI\]       st16.rel.nta \[r65\]=r93,ar.csd
     546:	00 e8 06 4f 23 00 	            st16.rel.nta \[r65\]=r93,ar.csd
     54c:	00 00 04 00       	            nop.i 0x0
     550:	08 00 76 83 a1 11 	\[MMI\]       st16.rel.d4 \[r65\]=r93,ar.csd
     556:	00 ec 06 47 23 00 	            st16.rel.d5 \[r65\]=r93,ar.csd
     55c:	00 00 04 00       	            nop.i 0x0
     560:	08 00 76 83 a5 11 	\[MMI\]       st16.rel.d6 \[r65\]=r93,ar.csd
     566:	00 ec 06 4f 23 00 	            st16.rel.d7 \[r65\]=r93,ar.csd
     56c:	00 00 04 00       	            nop.i 0x0
     570:	08 00 74 83 d8 11 	\[MMI\]       st8.spill \[r65\]=r93
     576:	00 e8 06 b5 23 00 	            st8.spill.d1 \[r65\]=r93
     57c:	00 00 04 00       	            nop.i 0x0
     580:	08 00 74 83 da 11 	\[MMI\]       st8.spill.d1 \[r65\]=r93
     586:	00 e8 06 b9 23 00 	            st8.spill.d2 \[r65\]=r93
     58c:	00 00 04 00       	            nop.i 0x0
     590:	08 00 74 83 dc 11 	\[MMI\]       st8.spill.d2 \[r65\]=r93
     596:	00 e8 06 bd 23 00 	            st8.spill.nta \[r65\]=r93
     59c:	00 00 04 00       	            nop.i 0x0
     5a0:	08 00 74 83 de 11 	\[MMI\]       st8.spill.nta \[r65\]=r93
     5a6:	00 ec 06 b1 23 00 	            st8.spill.d4 \[r65\]=r93
     5ac:	00 00 04 00       	            nop.i 0x0
     5b0:	08 00 76 83 da 11 	\[MMI\]       st8.spill.d5 \[r65\]=r93
     5b6:	00 ec 06 b9 23 00 	            st8.spill.d6 \[r65\]=r93
     5bc:	00 00 04 00       	            nop.i 0x0
     5c0:	08 00 76 83 de 11 	\[MMI\]       st8.spill.d7 \[r65\]=r93
     5c6:	00 00 f0 c0 32 00 	            lfetch \[r60\]
     5cc:	00 00 04 00       	            nop.i 0x0
     5d0:	08 00 00 78 62 19 	\[MMI\]       lfetch.nt1 \[r60\]
     5d6:	00 00 f0 c4 32 00 	            lfetch.nt1 \[r60\]
     5dc:	00 00 04 00       	            nop.i 0x0
     5e0:	08 00 00 78 64 19 	\[MMI\]       lfetch.nt2 \[r60\]
     5e6:	00 00 f0 c8 32 00 	            lfetch.nt2 \[r60\]
     5ec:	00 00 04 00       	            nop.i 0x0
     5f0:	08 00 00 78 66 19 	\[MMI\]       lfetch.nta \[r60\]
     5f6:	00 00 f0 cc 32 00 	            lfetch.nta \[r60\]
     5fc:	00 00 04 00       	            nop.i 0x0
     600:	08 00 02 78 60 19 	\[MMI\]       lfetch.d4 \[r60\]
     606:	00 04 f0 c4 32 00 	            lfetch.d5 \[r60\]
     60c:	00 00 04 00       	            nop.i 0x0
     610:	08 00 02 78 64 19 	\[MMI\]       lfetch.d6 \[r60\]
     616:	00 04 f0 cc 32 00 	            lfetch.d7 \[r60\]
     61c:	00 00 04 00       	            nop.i 0x0
     620:	08 00 68 79 90 19 	\[MMI\]       stfs \[r60\]=f90
     626:	00 d0 f2 24 33 00 	            stfs.d1 \[r60\]=f90
     62c:	00 00 04 00       	            nop.i 0x0
     630:	08 00 68 79 92 19 	\[MMI\]       stfs.d1 \[r60\]=f90
     636:	00 d0 f2 28 33 00 	            stfs.d2 \[r60\]=f90
     63c:	00 00 04 00       	            nop.i 0x0
     640:	08 00 68 79 94 19 	\[MMI\]       stfs.d2 \[r60\]=f90
     646:	00 d0 f2 2c 33 00 	            stfs.nta \[r60\]=f90
     64c:	00 00 04 00       	            nop.i 0x0
     650:	08 00 68 79 96 19 	\[MMI\]       stfs.nta \[r60\]=f90
     656:	00 d4 f2 20 33 00 	            stfs.d4 \[r60\]=f90
     65c:	00 00 04 00       	            nop.i 0x0
     660:	08 00 6a 79 92 19 	\[MMI\]       stfs.d5 \[r60\]=f90
     666:	00 d4 f2 28 33 00 	            stfs.d6 \[r60\]=f90
     66c:	00 00 04 00       	            nop.i 0x0
     670:	08 00 6a 79 96 19 	\[MMI\]       stfs.d7 \[r60\]=f90
     676:	00 d0 f2 30 33 00 	            stfd \[r60\]=f90
     67c:	00 00 04 00       	            nop.i 0x0
     680:	08 00 68 79 9a 19 	\[MMI\]       stfd.d1 \[r60\]=f90
     686:	00 d0 f2 34 33 00 	            stfd.d1 \[r60\]=f90
     68c:	00 00 04 00       	            nop.i 0x0
     690:	08 00 68 79 9c 19 	\[MMI\]       stfd.d2 \[r60\]=f90
     696:	00 d0 f2 38 33 00 	            stfd.d2 \[r60\]=f90
     69c:	00 00 04 00       	            nop.i 0x0
     6a0:	08 00 68 79 9e 19 	\[MMI\]       stfd.nta \[r60\]=f90
     6a6:	00 d0 f2 3c 33 00 	            stfd.nta \[r60\]=f90
     6ac:	00 00 04 00       	            nop.i 0x0
     6b0:	08 00 6a 79 98 19 	\[MMI\]       stfd.d4 \[r60\]=f90
     6b6:	00 d4 f2 34 33 00 	            stfd.d5 \[r60\]=f90
     6bc:	00 00 04 00       	            nop.i 0x0
     6c0:	08 00 6a 79 9c 19 	\[MMI\]       stfd.d6 \[r60\]=f90
     6c6:	00 d4 f2 3c 33 00 	            stfd.d7 \[r60\]=f90
     6cc:	00 00 04 00       	            nop.i 0x0
     6d0:	08 00 68 79 88 19 	\[MMI\]       stf8 \[r60\]=f90
     6d6:	00 d0 f2 14 33 00 	            stf8.d1 \[r60\]=f90
     6dc:	00 00 04 00       	            nop.i 0x0
     6e0:	08 00 68 79 8a 19 	\[MMI\]       stf8.d1 \[r60\]=f90
     6e6:	00 d0 f2 18 33 00 	            stf8.d2 \[r60\]=f90
     6ec:	00 00 04 00       	            nop.i 0x0
     6f0:	08 00 68 79 8c 19 	\[MMI\]       stf8.d2 \[r60\]=f90
     6f6:	00 d0 f2 1c 33 00 	            stf8.nta \[r60\]=f90
     6fc:	00 00 04 00       	            nop.i 0x0
     700:	08 00 68 79 8e 19 	\[MMI\]       stf8.nta \[r60\]=f90
     706:	00 d4 f2 10 33 00 	            stf8.d4 \[r60\]=f90
     70c:	00 00 04 00       	            nop.i 0x0
     710:	08 00 6a 79 8a 19 	\[MMI\]       stf8.d5 \[r60\]=f90
     716:	00 d4 f2 18 33 00 	            stf8.d6 \[r60\]=f90
     71c:	00 00 04 00       	            nop.i 0x0
     720:	08 00 6a 79 8e 19 	\[MMI\]       stf8.d7 \[r60\]=f90
     726:	00 d0 f2 00 33 00 	            stfe \[r60\]=f90
     72c:	00 00 04 00       	            nop.i 0x0
     730:	08 00 68 79 82 19 	\[MMI\]       stfe.d1 \[r60\]=f90
     736:	00 d0 f2 04 33 00 	            stfe.d1 \[r60\]=f90
     73c:	00 00 04 00       	            nop.i 0x0
     740:	08 00 68 79 84 19 	\[MMI\]       stfe.d2 \[r60\]=f90
     746:	00 d0 f2 08 33 00 	            stfe.d2 \[r60\]=f90
     74c:	00 00 04 00       	            nop.i 0x0
     750:	08 00 68 79 86 19 	\[MMI\]       stfe.nta \[r60\]=f90
     756:	00 d0 f2 0c 33 00 	            stfe.nta \[r60\]=f90
     75c:	00 00 04 00       	            nop.i 0x0
     760:	08 00 6a 79 80 19 	\[MMI\]       stfe.d4 \[r60\]=f90
     766:	00 d4 f2 04 33 00 	            stfe.d5 \[r60\]=f90
     76c:	00 00 04 00       	            nop.i 0x0
     770:	08 00 6a 79 84 19 	\[MMI\]       stfe.d6 \[r60\]=f90
     776:	00 d4 f2 0c 33 00 	            stfe.d7 \[r60\]=f90
     77c:	00 00 04 00       	            nop.i 0x0
     780:	08 00 68 79 d8 19 	\[MMI\]       stf.spill \[r60\]=f90
     786:	00 d0 f2 b4 33 00 	            stf.spill.d1 \[r60\]=f90
     78c:	00 00 04 00       	            nop.i 0x0
     790:	08 00 68 79 da 19 	\[MMI\]       stf.spill.d1 \[r60\]=f90
     796:	00 d0 f2 b8 33 00 	            stf.spill.d2 \[r60\]=f90
     79c:	00 00 04 00       	            nop.i 0x0
     7a0:	08 00 68 79 dc 19 	\[MMI\]       stf.spill.d2 \[r60\]=f90
     7a6:	00 d0 f2 bc 33 00 	            stf.spill.nta \[r60\]=f90
     7ac:	00 00 04 00       	            nop.i 0x0
     7b0:	08 00 68 79 de 19 	\[MMI\]       stf.spill.nta \[r60\]=f90
     7b6:	00 d4 f2 b0 33 00 	            stf.spill.d4 \[r60\]=f90
     7bc:	00 00 04 00       	            nop.i 0x0
     7c0:	08 00 6a 79 da 19 	\[MMI\]       stf.spill.d5 \[r60\]=f90
     7c6:	00 d4 f2 b8 33 00 	            stf.spill.d6 \[r60\]=f90
     7cc:	00 00 04 00       	            nop.i 0x0
     7d0:	08 00 6a 79 de 19 	\[MMI\]       stf.spill.d7 \[r60\]=f90
     7d6:	90 07 f4 21 30 00 	            ldfs f121=\[r125\]
     7dc:	00 00 04 00       	            nop.i 0x0
     7e0:	08 c8 03 fa 12 18 	\[MMI\]       ldfs.nt1 f121=\[r125\]
     7e6:	90 07 f4 25 30 00 	            ldfs.nt1 f121=\[r125\]
     7ec:	00 00 04 00       	            nop.i 0x0
     7f0:	08 c8 03 fa 14 18 	\[MMI\]       ldfs.d2 f121=\[r125\]
     7f6:	90 07 f4 29 30 00 	            ldfs.d2 f121=\[r125\]
     7fc:	00 00 04 00       	            nop.i 0x0
     800:	08 c8 03 fa 16 18 	\[MMI\]       ldfs.nta f121=\[r125\]
     806:	90 07 f4 2d 30 00 	            ldfs.nta f121=\[r125\]
     80c:	00 00 04 00       	            nop.i 0x0
     810:	08 c8 03 fb 10 18 	\[MMI\]       ldfs.d4 f121=\[r125\]
     816:	90 07 f6 25 30 00 	            ldfs.d5 f121=\[r125\]
     81c:	00 00 04 00       	            nop.i 0x0
     820:	08 c8 03 fb 14 18 	\[MMI\]       ldfs.d6 f121=\[r125\]
     826:	90 07 f6 2d 30 00 	            ldfs.d7 f121=\[r125\]
     82c:	00 00 04 00       	            nop.i 0x0
     830:	08 c8 03 fa 18 18 	\[MMI\]       ldfd f121=\[r125\]
     836:	90 07 f4 35 30 00 	            ldfd.nt1 f121=\[r125\]
     83c:	00 00 04 00       	            nop.i 0x0
     840:	08 c8 03 fa 1a 18 	\[MMI\]       ldfd.nt1 f121=\[r125\]
     846:	90 07 f4 39 30 00 	            ldfd.d2 f121=\[r125\]
     84c:	00 00 04 00       	            nop.i 0x0
     850:	08 c8 03 fa 1c 18 	\[MMI\]       ldfd.d2 f121=\[r125\]
     856:	90 07 f4 3d 30 00 	            ldfd.nta f121=\[r125\]
     85c:	00 00 04 00       	            nop.i 0x0
     860:	08 c8 03 fa 1e 18 	\[MMI\]       ldfd.nta f121=\[r125\]
     866:	90 07 f6 31 30 00 	            ldfd.d4 f121=\[r125\]
     86c:	00 00 04 00       	            nop.i 0x0
     870:	08 c8 03 fb 1a 18 	\[MMI\]       ldfd.d5 f121=\[r125\]
     876:	90 07 f6 39 30 00 	            ldfd.d6 f121=\[r125\]
     87c:	00 00 04 00       	            nop.i 0x0
     880:	08 c8 03 fb 1e 18 	\[MMI\]       ldfd.d7 f121=\[r125\]
     886:	90 07 f4 11 30 00 	            ldf8 f121=\[r125\]
     88c:	00 00 04 00       	            nop.i 0x0
     890:	08 c8 03 fa 0a 18 	\[MMI\]       ldf8.nt1 f121=\[r125\]
     896:	90 07 f4 15 30 00 	            ldf8.nt1 f121=\[r125\]
     89c:	00 00 04 00       	            nop.i 0x0
     8a0:	08 c8 03 fa 0c 18 	\[MMI\]       ldf8.d2 f121=\[r125\]
     8a6:	90 07 f4 19 30 00 	            ldf8.d2 f121=\[r125\]
     8ac:	00 00 04 00       	            nop.i 0x0
     8b0:	08 c8 03 fa 0e 18 	\[MMI\]       ldf8.nta f121=\[r125\]
     8b6:	90 07 f4 1d 30 00 	            ldf8.nta f121=\[r125\]
     8bc:	00 00 04 00       	            nop.i 0x0
     8c0:	08 c8 03 fb 08 18 	\[MMI\]       ldf8.d4 f121=\[r125\]
     8c6:	90 07 f6 15 30 00 	            ldf8.d5 f121=\[r125\]
     8cc:	00 00 04 00       	            nop.i 0x0
     8d0:	08 c8 03 fb 0c 18 	\[MMI\]       ldf8.d6 f121=\[r125\]
     8d6:	90 07 f6 1d 30 00 	            ldf8.d7 f121=\[r125\]
     8dc:	00 00 04 00       	            nop.i 0x0
     8e0:	08 c8 03 fa 00 18 	\[MMI\]       ldfe f121=\[r125\]
     8e6:	90 07 f4 05 30 00 	            ldfe.nt1 f121=\[r125\]
     8ec:	00 00 04 00       	            nop.i 0x0
     8f0:	08 c8 03 fa 02 18 	\[MMI\]       ldfe.nt1 f121=\[r125\]
     8f6:	90 07 f4 09 30 00 	            ldfe.d2 f121=\[r125\]
     8fc:	00 00 04 00       	            nop.i 0x0
     900:	08 c8 03 fa 04 18 	\[MMI\]       ldfe.d2 f121=\[r125\]
     906:	90 07 f4 0d 30 00 	            ldfe.nta f121=\[r125\]
     90c:	00 00 04 00       	            nop.i 0x0
     910:	08 c8 03 fa 06 18 	\[MMI\]       ldfe.nta f121=\[r125\]
     916:	90 07 f6 01 30 00 	            ldfe.d4 f121=\[r125\]
     91c:	00 00 04 00       	            nop.i 0x0
     920:	08 c8 03 fb 02 18 	\[MMI\]       ldfe.d5 f121=\[r125\]
     926:	90 07 f6 09 30 00 	            ldfe.d6 f121=\[r125\]
     92c:	00 00 04 00       	            nop.i 0x0
     930:	08 c8 03 fb 06 18 	\[MMI\]       ldfe.d7 f121=\[r125\]
     936:	90 07 f4 61 30 00 	            ldfs.s f121=\[r125\]
     93c:	00 00 04 00       	            nop.i 0x0
     940:	08 c8 03 fa 32 18 	\[MMI\]       ldfs.s.nt1 f121=\[r125\]
     946:	90 07 f4 65 30 00 	            ldfs.s.nt1 f121=\[r125\]
     94c:	00 00 04 00       	            nop.i 0x0
     950:	08 c8 03 fa 34 18 	\[MMI\]       ldfs.s.d2 f121=\[r125\]
     956:	90 07 f4 69 30 00 	            ldfs.s.d2 f121=\[r125\]
     95c:	00 00 04 00       	            nop.i 0x0
     960:	08 c8 03 fa 36 18 	\[MMI\]       ldfs.s.nta f121=\[r125\]
     966:	90 07 f4 6d 30 00 	            ldfs.s.nta f121=\[r125\]
     96c:	00 00 04 00       	            nop.i 0x0
     970:	08 c8 03 fb 30 18 	\[MMI\]       ldfs.s.d4 f121=\[r125\]
     976:	90 07 f6 65 30 00 	            ldfs.s.d5 f121=\[r125\]
     97c:	00 00 04 00       	            nop.i 0x0
     980:	08 c8 03 fb 34 18 	\[MMI\]       ldfs.s.d6 f121=\[r125\]
     986:	90 07 f6 6d 30 00 	            ldfs.s.d7 f121=\[r125\]
     98c:	00 00 04 00       	            nop.i 0x0
     990:	08 c8 03 fa 38 18 	\[MMI\]       ldfd.s f121=\[r125\]
     996:	90 07 f4 75 30 00 	            ldfd.s.nt1 f121=\[r125\]
     99c:	00 00 04 00       	            nop.i 0x0
     9a0:	08 c8 03 fa 3a 18 	\[MMI\]       ldfd.s.nt1 f121=\[r125\]
     9a6:	90 07 f4 79 30 00 	            ldfd.s.d2 f121=\[r125\]
     9ac:	00 00 04 00       	            nop.i 0x0
     9b0:	08 c8 03 fa 3c 18 	\[MMI\]       ldfd.s.d2 f121=\[r125\]
     9b6:	90 07 f4 7d 30 00 	            ldfd.s.nta f121=\[r125\]
     9bc:	00 00 04 00       	            nop.i 0x0
     9c0:	08 c8 03 fa 3e 18 	\[MMI\]       ldfd.s.nta f121=\[r125\]
     9c6:	90 07 f6 71 30 00 	            ldfd.s.d4 f121=\[r125\]
     9cc:	00 00 04 00       	            nop.i 0x0
     9d0:	08 c8 03 fb 3a 18 	\[MMI\]       ldfd.s.d5 f121=\[r125\]
     9d6:	90 07 f6 79 30 00 	            ldfd.s.d6 f121=\[r125\]
     9dc:	00 00 04 00       	            nop.i 0x0
     9e0:	08 c8 03 fb 3e 18 	\[MMI\]       ldfd.s.d7 f121=\[r125\]
     9e6:	90 07 f4 51 30 00 	            ldf8.s f121=\[r125\]
     9ec:	00 00 04 00       	            nop.i 0x0
     9f0:	08 c8 03 fa 2a 18 	\[MMI\]       ldf8.s.nt1 f121=\[r125\]
     9f6:	90 07 f4 55 30 00 	            ldf8.s.nt1 f121=\[r125\]
     9fc:	00 00 04 00       	            nop.i 0x0
     a00:	08 c8 03 fa 2c 18 	\[MMI\]       ldf8.s.d2 f121=\[r125\]
     a06:	90 07 f4 59 30 00 	            ldf8.s.d2 f121=\[r125\]
     a0c:	00 00 04 00       	            nop.i 0x0
     a10:	08 c8 03 fa 2e 18 	\[MMI\]       ldf8.s.nta f121=\[r125\]
     a16:	90 07 f4 5d 30 00 	            ldf8.s.nta f121=\[r125\]
     a1c:	00 00 04 00       	            nop.i 0x0
     a20:	08 c8 03 fb 28 18 	\[MMI\]       ldf8.s.d4 f121=\[r125\]
     a26:	90 07 f6 55 30 00 	            ldf8.s.d5 f121=\[r125\]
     a2c:	00 00 04 00       	            nop.i 0x0
     a30:	08 c8 03 fb 2c 18 	\[MMI\]       ldf8.s.d6 f121=\[r125\]
     a36:	90 07 f6 5d 30 00 	            ldf8.s.d7 f121=\[r125\]
     a3c:	00 00 04 00       	            nop.i 0x0
     a40:	08 c8 03 fa 20 18 	\[MMI\]       ldfe.s f121=\[r125\]
     a46:	90 07 f4 45 30 00 	            ldfe.s.nt1 f121=\[r125\]
     a4c:	00 00 04 00       	            nop.i 0x0
     a50:	08 c8 03 fa 22 18 	\[MMI\]       ldfe.s.nt1 f121=\[r125\]
     a56:	90 07 f4 49 30 00 	            ldfe.s.d2 f121=\[r125\]
     a5c:	00 00 04 00       	            nop.i 0x0
     a60:	08 c8 03 fa 24 18 	\[MMI\]       ldfe.s.d2 f121=\[r125\]
     a66:	90 07 f4 4d 30 00 	            ldfe.s.nta f121=\[r125\]
     a6c:	00 00 04 00       	            nop.i 0x0
     a70:	08 c8 03 fa 26 18 	\[MMI\]       ldfe.s.nta f121=\[r125\]
     a76:	90 07 f6 41 30 00 	            ldfe.s.d4 f121=\[r125\]
     a7c:	00 00 04 00       	            nop.i 0x0
     a80:	08 c8 03 fb 22 18 	\[MMI\]       ldfe.s.d5 f121=\[r125\]
     a86:	90 07 f6 49 30 00 	            ldfe.s.d6 f121=\[r125\]
     a8c:	00 00 04 00       	            nop.i 0x0
     a90:	08 c8 03 fb 26 18 	\[MMI\]       ldfe.s.d7 f121=\[r125\]
     a96:	90 07 f4 a1 30 00 	            ldfs.a f121=\[r125\]
     a9c:	00 00 04 00       	            nop.i 0x0
     aa0:	08 c8 03 fa 52 18 	\[MMI\]       ldfs.a.nt1 f121=\[r125\]
     aa6:	90 07 f4 a5 30 00 	            ldfs.a.nt1 f121=\[r125\]
     aac:	00 00 04 00       	            nop.i 0x0
     ab0:	08 c8 03 fa 54 18 	\[MMI\]       ldfs.a.d2 f121=\[r125\]
     ab6:	90 07 f4 a9 30 00 	            ldfs.a.d2 f121=\[r125\]
     abc:	00 00 04 00       	            nop.i 0x0
     ac0:	08 c8 03 fa 56 18 	\[MMI\]       ldfs.a.nta f121=\[r125\]
     ac6:	90 07 f4 ad 30 00 	            ldfs.a.nta f121=\[r125\]
     acc:	00 00 04 00       	            nop.i 0x0
     ad0:	08 c8 03 fb 50 18 	\[MMI\]       ldfs.a.d4 f121=\[r125\]
     ad6:	90 07 f6 a5 30 00 	            ldfs.a.d5 f121=\[r125\]
     adc:	00 00 04 00       	            nop.i 0x0
     ae0:	08 c8 03 fb 54 18 	\[MMI\]       ldfs.a.d6 f121=\[r125\]
     ae6:	90 07 f6 ad 30 00 	            ldfs.a.d7 f121=\[r125\]
     aec:	00 00 04 00       	            nop.i 0x0
     af0:	08 c8 03 fa 58 18 	\[MMI\]       ldfd.a f121=\[r125\]
     af6:	90 07 f4 b5 30 00 	            ldfd.a.nt1 f121=\[r125\]
     afc:	00 00 04 00       	            nop.i 0x0
     b00:	08 c8 03 fa 5a 18 	\[MMI\]       ldfd.a.nt1 f121=\[r125\]
     b06:	90 07 f4 b9 30 00 	            ldfd.a.d2 f121=\[r125\]
     b0c:	00 00 04 00       	            nop.i 0x0
     b10:	08 c8 03 fa 5c 18 	\[MMI\]       ldfd.a.d2 f121=\[r125\]
     b16:	90 07 f4 bd 30 00 	            ldfd.a.nta f121=\[r125\]
     b1c:	00 00 04 00       	            nop.i 0x0
     b20:	08 c8 03 fa 5e 18 	\[MMI\]       ldfd.a.nta f121=\[r125\]
     b26:	90 07 f6 b1 30 00 	            ldfd.a.d4 f121=\[r125\]
     b2c:	00 00 04 00       	            nop.i 0x0
     b30:	08 c8 03 fb 5a 18 	\[MMI\]       ldfd.a.d5 f121=\[r125\]
     b36:	90 07 f6 b9 30 00 	            ldfd.a.d6 f121=\[r125\]
     b3c:	00 00 04 00       	            nop.i 0x0
     b40:	08 c8 03 fb 5e 18 	\[MMI\]       ldfd.a.d7 f121=\[r125\]
     b46:	90 07 f4 91 30 00 	            ldf8.a f121=\[r125\]
     b4c:	00 00 04 00       	            nop.i 0x0
     b50:	08 c8 03 fa 4a 18 	\[MMI\]       ldf8.a.nt1 f121=\[r125\]
     b56:	90 07 f4 95 30 00 	            ldf8.a.nt1 f121=\[r125\]
     b5c:	00 00 04 00       	            nop.i 0x0
     b60:	08 c8 03 fa 4c 18 	\[MMI\]       ldf8.a.d2 f121=\[r125\]
     b66:	90 07 f4 99 30 00 	            ldf8.a.d2 f121=\[r125\]
     b6c:	00 00 04 00       	            nop.i 0x0
     b70:	08 c8 03 fa 4e 18 	\[MMI\]       ldf8.a.nta f121=\[r125\]
     b76:	90 07 f4 9d 30 00 	            ldf8.a.nta f121=\[r125\]
     b7c:	00 00 04 00       	            nop.i 0x0
     b80:	08 c8 03 fb 48 18 	\[MMI\]       ldf8.a.d4 f121=\[r125\]
     b86:	90 07 f6 95 30 00 	            ldf8.a.d5 f121=\[r125\]
     b8c:	00 00 04 00       	            nop.i 0x0
     b90:	08 c8 03 fb 4c 18 	\[MMI\]       ldf8.a.d6 f121=\[r125\]
     b96:	90 07 f6 9d 30 00 	            ldf8.a.d7 f121=\[r125\]
     b9c:	00 00 04 00       	            nop.i 0x0
     ba0:	08 c8 03 fa 40 18 	\[MMI\]       ldfe.a f121=\[r125\]
     ba6:	90 07 f4 85 30 00 	            ldfe.a.nt1 f121=\[r125\]
     bac:	00 00 04 00       	            nop.i 0x0
     bb0:	08 c8 03 fa 42 18 	\[MMI\]       ldfe.a.nt1 f121=\[r125\]
     bb6:	90 07 f4 89 30 00 	            ldfe.a.d2 f121=\[r125\]
     bbc:	00 00 04 00       	            nop.i 0x0
     bc0:	08 c8 03 fa 44 18 	\[MMI\]       ldfe.a.d2 f121=\[r125\]
     bc6:	90 07 f4 8d 30 00 	            ldfe.a.nta f121=\[r125\]
     bcc:	00 00 04 00       	            nop.i 0x0
     bd0:	08 c8 03 fa 46 18 	\[MMI\]       ldfe.a.nta f121=\[r125\]
     bd6:	90 07 f6 81 30 00 	            ldfe.a.d4 f121=\[r125\]
     bdc:	00 00 04 00       	            nop.i 0x0
     be0:	08 c8 03 fb 42 18 	\[MMI\]       ldfe.a.d5 f121=\[r125\]
     be6:	90 07 f6 89 30 00 	            ldfe.a.d6 f121=\[r125\]
     bec:	00 00 04 00       	            nop.i 0x0
     bf0:	08 c8 03 fb 46 18 	\[MMI\]       ldfe.a.d7 f121=\[r125\]
     bf6:	90 07 f4 e1 30 00 	            ldfs.sa f121=\[r125\]
     bfc:	00 00 04 00       	            nop.i 0x0
     c00:	08 c8 03 fa 72 18 	\[MMI\]       ldfs.sa.nt1 f121=\[r125\]
     c06:	90 07 f4 e5 30 00 	            ldfs.sa.nt1 f121=\[r125\]
     c0c:	00 00 04 00       	            nop.i 0x0
     c10:	08 c8 03 fa 74 18 	\[MMI\]       ldfs.sa.d2 f121=\[r125\]
     c16:	90 07 f4 e9 30 00 	            ldfs.sa.d2 f121=\[r125\]
     c1c:	00 00 04 00       	            nop.i 0x0
     c20:	08 c8 03 fa 76 18 	\[MMI\]       ldfs.sa.nta f121=\[r125\]
     c26:	90 07 f4 ed 30 00 	            ldfs.sa.nta f121=\[r125\]
     c2c:	00 00 04 00       	            nop.i 0x0
     c30:	08 c8 03 fb 70 18 	\[MMI\]       ldfs.sa.d4 f121=\[r125\]
     c36:	90 07 f6 e5 30 00 	            ldfs.sa.d5 f121=\[r125\]
     c3c:	00 00 04 00       	            nop.i 0x0
     c40:	08 c8 03 fb 74 18 	\[MMI\]       ldfs.sa.d6 f121=\[r125\]
     c46:	90 07 f6 ed 30 00 	            ldfs.sa.d7 f121=\[r125\]
     c4c:	00 00 04 00       	            nop.i 0x0
     c50:	08 c8 03 fa 78 18 	\[MMI\]       ldfd.sa f121=\[r125\]
     c56:	90 07 f4 f5 30 00 	            ldfd.sa.nt1 f121=\[r125\]
     c5c:	00 00 04 00       	            nop.i 0x0
     c60:	08 c8 03 fa 7a 18 	\[MMI\]       ldfd.sa.nt1 f121=\[r125\]
     c66:	90 07 f4 f9 30 00 	            ldfd.sa.d2 f121=\[r125\]
     c6c:	00 00 04 00       	            nop.i 0x0
     c70:	08 c8 03 fa 7c 18 	\[MMI\]       ldfd.sa.d2 f121=\[r125\]
     c76:	90 07 f4 fd 30 00 	            ldfd.sa.nta f121=\[r125\]
     c7c:	00 00 04 00       	            nop.i 0x0
     c80:	08 c8 03 fa 7e 18 	\[MMI\]       ldfd.sa.nta f121=\[r125\]
     c86:	90 07 f6 f1 30 00 	            ldfd.sa.d4 f121=\[r125\]
     c8c:	00 00 04 00       	            nop.i 0x0
     c90:	08 c8 03 fb 7a 18 	\[MMI\]       ldfd.sa.d5 f121=\[r125\]
     c96:	90 07 f6 f9 30 00 	            ldfd.sa.d6 f121=\[r125\]
     c9c:	00 00 04 00       	            nop.i 0x0
     ca0:	08 c8 03 fb 7e 18 	\[MMI\]       ldfd.sa.d7 f121=\[r125\]
     ca6:	90 07 f4 d1 30 00 	            ldf8.sa f121=\[r125\]
     cac:	00 00 04 00       	            nop.i 0x0
     cb0:	08 c8 03 fa 6a 18 	\[MMI\]       ldf8.sa.nt1 f121=\[r125\]
     cb6:	90 07 f4 d5 30 00 	            ldf8.sa.nt1 f121=\[r125\]
     cbc:	00 00 04 00       	            nop.i 0x0
     cc0:	08 c8 03 fa 6c 18 	\[MMI\]       ldf8.sa.d2 f121=\[r125\]
     cc6:	90 07 f4 d9 30 00 	            ldf8.sa.d2 f121=\[r125\]
     ccc:	00 00 04 00       	            nop.i 0x0
     cd0:	08 c8 03 fa 6e 18 	\[MMI\]       ldf8.sa.nta f121=\[r125\]
     cd6:	90 07 f4 dd 30 00 	            ldf8.sa.nta f121=\[r125\]
     cdc:	00 00 04 00       	            nop.i 0x0
     ce0:	08 c8 03 fb 68 18 	\[MMI\]       ldf8.sa.d4 f121=\[r125\]
     ce6:	90 07 f6 d5 30 00 	            ldf8.sa.d5 f121=\[r125\]
     cec:	00 00 04 00       	            nop.i 0x0
     cf0:	08 c8 03 fb 6c 18 	\[MMI\]       ldf8.sa.d6 f121=\[r125\]
     cf6:	90 07 f6 dd 30 00 	            ldf8.sa.d7 f121=\[r125\]
     cfc:	00 00 04 00       	            nop.i 0x0
     d00:	08 c8 03 fa 60 18 	\[MMI\]       ldfe.sa f121=\[r125\]
     d06:	90 07 f4 c5 30 00 	            ldfe.sa.nt1 f121=\[r125\]
     d0c:	00 00 04 00       	            nop.i 0x0
     d10:	08 c8 03 fa 62 18 	\[MMI\]       ldfe.sa.nt1 f121=\[r125\]
     d16:	90 07 f4 c9 30 00 	            ldfe.sa.d2 f121=\[r125\]
     d1c:	00 00 04 00       	            nop.i 0x0
     d20:	08 c8 03 fa 64 18 	\[MMI\]       ldfe.sa.d2 f121=\[r125\]
     d26:	90 07 f4 cd 30 00 	            ldfe.sa.nta f121=\[r125\]
     d2c:	00 00 04 00       	            nop.i 0x0
     d30:	08 c8 03 fa 66 18 	\[MMI\]       ldfe.sa.nta f121=\[r125\]
     d36:	90 07 f6 c1 30 00 	            ldfe.sa.d4 f121=\[r125\]
     d3c:	00 00 04 00       	            nop.i 0x0
     d40:	08 c8 03 fb 62 18 	\[MMI\]       ldfe.sa.d5 f121=\[r125\]
     d46:	90 07 f6 c9 30 00 	            ldfe.sa.d6 f121=\[r125\]
     d4c:	00 00 04 00       	            nop.i 0x0
     d50:	08 c8 03 fb 66 18 	\[MMI\]       ldfe.sa.d7 f121=\[r125\]
     d56:	90 07 f4 b1 31 00 	            ldf.fill f121=\[r125\]
     d5c:	00 00 04 00       	            nop.i 0x0
     d60:	08 c8 03 fa da 18 	\[MMI\]       ldf.fill.nt1 f121=\[r125\]
     d66:	90 07 f4 b5 31 00 	            ldf.fill.nt1 f121=\[r125\]
     d6c:	00 00 04 00       	            nop.i 0x0
     d70:	08 c8 03 fa dc 18 	\[MMI\]       ldf.fill.d2 f121=\[r125\]
     d76:	90 07 f4 b9 31 00 	            ldf.fill.d2 f121=\[r125\]
     d7c:	00 00 04 00       	            nop.i 0x0
     d80:	08 c8 03 fa de 18 	\[MMI\]       ldf.fill.nta f121=\[r125\]
     d86:	90 07 f4 bd 31 00 	            ldf.fill.nta f121=\[r125\]
     d8c:	00 00 04 00       	            nop.i 0x0
     d90:	08 c8 03 fb d8 18 	\[MMI\]       ldf.fill.d4 f121=\[r125\]
     d96:	90 07 f6 b5 31 00 	            ldf.fill.d5 f121=\[r125\]
     d9c:	00 00 04 00       	            nop.i 0x0
     da0:	08 c8 03 fb dc 18 	\[MMI\]       ldf.fill.d6 f121=\[r125\]
     da6:	90 07 f6 bd 31 00 	            ldf.fill.d7 f121=\[r125\]
     dac:	00 00 04 00       	            nop.i 0x0
     db0:	08 c8 03 fa 10 19 	\[MMI\]       ldfs.c.clr f121=\[r125\]
     db6:	90 07 f4 25 32 00 	            ldfs.c.clr.nt1 f121=\[r125\]
     dbc:	00 00 04 00       	            nop.i 0x0
     dc0:	08 c8 03 fa 12 19 	\[MMI\]       ldfs.c.clr.nt1 f121=\[r125\]
     dc6:	90 07 f4 29 32 00 	            ldfs.c.clr.d2 f121=\[r125\]
     dcc:	00 00 04 00       	            nop.i 0x0
     dd0:	08 c8 03 fa 14 19 	\[MMI\]       ldfs.c.clr.d2 f121=\[r125\]
     dd6:	90 07 f4 2d 32 00 	            ldfs.c.clr.nta f121=\[r125\]
     ddc:	00 00 04 00       	            nop.i 0x0
     de0:	08 c8 03 fa 16 19 	\[MMI\]       ldfs.c.clr.nta f121=\[r125\]
     de6:	90 07 f6 21 32 00 	            ldfs.c.clr.d4 f121=\[r125\]
     dec:	00 00 04 00       	            nop.i 0x0
     df0:	08 c8 03 fb 12 19 	\[MMI\]       ldfs.c.clr.d5 f121=\[r125\]
     df6:	90 07 f6 29 32 00 	            ldfs.c.clr.d6 f121=\[r125\]
     dfc:	00 00 04 00       	            nop.i 0x0
     e00:	08 c8 03 fb 16 19 	\[MMI\]       ldfs.c.clr.d7 f121=\[r125\]
     e06:	90 07 f4 31 32 00 	            ldfd.c.clr f121=\[r125\]
     e0c:	00 00 04 00       	            nop.i 0x0
     e10:	08 c8 03 fa 1a 19 	\[MMI\]       ldfd.c.clr.nt1 f121=\[r125\]
     e16:	90 07 f4 35 32 00 	            ldfd.c.clr.nt1 f121=\[r125\]
     e1c:	00 00 04 00       	            nop.i 0x0
     e20:	08 c8 03 fa 1c 19 	\[MMI\]       ldfd.c.clr.d2 f121=\[r125\]
     e26:	90 07 f4 39 32 00 	            ldfd.c.clr.d2 f121=\[r125\]
     e2c:	00 00 04 00       	            nop.i 0x0
     e30:	08 c8 03 fa 1e 19 	\[MMI\]       ldfd.c.clr.nta f121=\[r125\]
     e36:	90 07 f4 3d 32 00 	            ldfd.c.clr.nta f121=\[r125\]
     e3c:	00 00 04 00       	            nop.i 0x0
     e40:	08 c8 03 fb 18 19 	\[MMI\]       ldfd.c.clr.d4 f121=\[r125\]
     e46:	90 07 f6 35 32 00 	            ldfd.c.clr.d5 f121=\[r125\]
     e4c:	00 00 04 00       	            nop.i 0x0
     e50:	08 c8 03 fb 1c 19 	\[MMI\]       ldfd.c.clr.d6 f121=\[r125\]
     e56:	90 07 f6 3d 32 00 	            ldfd.c.clr.d7 f121=\[r125\]
     e5c:	00 00 04 00       	            nop.i 0x0
     e60:	08 c8 03 fa 08 19 	\[MMI\]       ldf8.c.clr f121=\[r125\]
     e66:	90 07 f4 15 32 00 	            ldf8.c.clr.nt1 f121=\[r125\]
     e6c:	00 00 04 00       	            nop.i 0x0
     e70:	08 c8 03 fa 0a 19 	\[MMI\]       ldf8.c.clr.nt1 f121=\[r125\]
     e76:	90 07 f4 19 32 00 	            ldf8.c.clr.d2 f121=\[r125\]
     e7c:	00 00 04 00       	            nop.i 0x0
     e80:	08 c8 03 fa 0c 19 	\[MMI\]       ldf8.c.clr.d2 f121=\[r125\]
     e86:	90 07 f4 1d 32 00 	            ldf8.c.clr.nta f121=\[r125\]
     e8c:	00 00 04 00       	            nop.i 0x0
     e90:	08 c8 03 fa 0e 19 	\[MMI\]       ldf8.c.clr.nta f121=\[r125\]
     e96:	90 07 f6 11 32 00 	            ldf8.c.clr.d4 f121=\[r125\]
     e9c:	00 00 04 00       	            nop.i 0x0
     ea0:	08 c8 03 fb 0a 19 	\[MMI\]       ldf8.c.clr.d5 f121=\[r125\]
     ea6:	90 07 f6 19 32 00 	            ldf8.c.clr.d6 f121=\[r125\]
     eac:	00 00 04 00       	            nop.i 0x0
     eb0:	08 c8 03 fb 0e 19 	\[MMI\]       ldf8.c.clr.d7 f121=\[r125\]
     eb6:	90 07 f4 01 32 00 	            ldfe.c.clr f121=\[r125\]
     ebc:	00 00 04 00       	            nop.i 0x0
     ec0:	08 c8 03 fa 02 19 	\[MMI\]       ldfe.c.clr.nt1 f121=\[r125\]
     ec6:	90 07 f4 05 32 00 	            ldfe.c.clr.nt1 f121=\[r125\]
     ecc:	00 00 04 00       	            nop.i 0x0
     ed0:	08 c8 03 fa 04 19 	\[MMI\]       ldfe.c.clr.d2 f121=\[r125\]
     ed6:	90 07 f4 09 32 00 	            ldfe.c.clr.d2 f121=\[r125\]
     edc:	00 00 04 00       	            nop.i 0x0
     ee0:	08 c8 03 fa 06 19 	\[MMI\]       ldfe.c.clr.nta f121=\[r125\]
     ee6:	90 07 f4 0d 32 00 	            ldfe.c.clr.nta f121=\[r125\]
     eec:	00 00 04 00       	            nop.i 0x0
     ef0:	08 c8 03 fb 00 19 	\[MMI\]       ldfe.c.clr.d4 f121=\[r125\]
     ef6:	90 07 f6 05 32 00 	            ldfe.c.clr.d5 f121=\[r125\]
     efc:	00 00 04 00       	            nop.i 0x0
     f00:	08 c8 03 fb 04 19 	\[MMI\]       ldfe.c.clr.d6 f121=\[r125\]
     f06:	90 07 f6 0d 32 00 	            ldfe.c.clr.d7 f121=\[r125\]
     f0c:	00 00 04 00       	            nop.i 0x0
     f10:	08 c8 03 fa 30 19 	\[MMI\]       ldfs.c.nc f121=\[r125\]
     f16:	90 07 f4 65 32 00 	            ldfs.c.nc.nt1 f121=\[r125\]
     f1c:	00 00 04 00       	            nop.i 0x0
     f20:	08 c8 03 fa 32 19 	\[MMI\]       ldfs.c.nc.nt1 f121=\[r125\]
     f26:	90 07 f4 69 32 00 	            ldfs.c.nc.d2 f121=\[r125\]
     f2c:	00 00 04 00       	            nop.i 0x0
     f30:	08 c8 03 fa 34 19 	\[MMI\]       ldfs.c.nc.d2 f121=\[r125\]
     f36:	90 07 f4 6d 32 00 	            ldfs.c.nc.nta f121=\[r125\]
     f3c:	00 00 04 00       	            nop.i 0x0
     f40:	08 c8 03 fa 36 19 	\[MMI\]       ldfs.c.nc.nta f121=\[r125\]
     f46:	90 07 f6 61 32 00 	            ldfs.c.nc.d4 f121=\[r125\]
     f4c:	00 00 04 00       	            nop.i 0x0
     f50:	08 c8 03 fb 32 19 	\[MMI\]       ldfs.c.nc.d5 f121=\[r125\]
     f56:	90 07 f6 69 32 00 	            ldfs.c.nc.d6 f121=\[r125\]
     f5c:	00 00 04 00       	            nop.i 0x0
     f60:	08 c8 03 fb 36 19 	\[MMI\]       ldfs.c.nc.d7 f121=\[r125\]
     f66:	90 07 f4 71 32 00 	            ldfd.c.nc f121=\[r125\]
     f6c:	00 00 04 00       	            nop.i 0x0
     f70:	08 c8 03 fa 3a 19 	\[MMI\]       ldfd.c.nc.nt1 f121=\[r125\]
     f76:	90 07 f4 75 32 00 	            ldfd.c.nc.nt1 f121=\[r125\]
     f7c:	00 00 04 00       	            nop.i 0x0
     f80:	08 c8 03 fa 3c 19 	\[MMI\]       ldfd.c.nc.d2 f121=\[r125\]
     f86:	90 07 f4 79 32 00 	            ldfd.c.nc.d2 f121=\[r125\]
     f8c:	00 00 04 00       	            nop.i 0x0
     f90:	08 c8 03 fa 3e 19 	\[MMI\]       ldfd.c.nc.nta f121=\[r125\]
     f96:	90 07 f4 7d 32 00 	            ldfd.c.nc.nta f121=\[r125\]
     f9c:	00 00 04 00       	            nop.i 0x0
     fa0:	08 c8 03 fb 38 19 	\[MMI\]       ldfd.c.nc.d4 f121=\[r125\]
     fa6:	90 07 f6 75 32 00 	            ldfd.c.nc.d5 f121=\[r125\]
     fac:	00 00 04 00       	            nop.i 0x0
     fb0:	08 c8 03 fb 3c 19 	\[MMI\]       ldfd.c.nc.d6 f121=\[r125\]
     fb6:	90 07 f6 7d 32 00 	            ldfd.c.nc.d7 f121=\[r125\]
     fbc:	00 00 04 00       	            nop.i 0x0
     fc0:	08 c8 03 fa 28 19 	\[MMI\]       ldf8.c.nc f121=\[r125\]
     fc6:	90 07 f4 55 32 00 	            ldf8.c.nc.nt1 f121=\[r125\]
     fcc:	00 00 04 00       	            nop.i 0x0
     fd0:	08 c8 03 fa 2a 19 	\[MMI\]       ldf8.c.nc.nt1 f121=\[r125\]
     fd6:	90 07 f4 59 32 00 	            ldf8.c.nc.d2 f121=\[r125\]
     fdc:	00 00 04 00       	            nop.i 0x0
     fe0:	08 c8 03 fa 2c 19 	\[MMI\]       ldf8.c.nc.d2 f121=\[r125\]
     fe6:	90 07 f4 5d 32 00 	            ldf8.c.nc.nta f121=\[r125\]
     fec:	00 00 04 00       	            nop.i 0x0
     ff0:	08 c8 03 fa 2e 19 	\[MMI\]       ldf8.c.nc.nta f121=\[r125\]
     ff6:	90 07 f6 51 32 00 	            ldf8.c.nc.d4 f121=\[r125\]
     ffc:	00 00 04 00       	            nop.i 0x0
    1000:	08 c8 03 fb 2a 19 	\[MMI\]       ldf8.c.nc.d5 f121=\[r125\]
    1006:	90 07 f6 59 32 00 	            ldf8.c.nc.d6 f121=\[r125\]
    100c:	00 00 04 00       	            nop.i 0x0
    1010:	08 c8 03 fb 2e 19 	\[MMI\]       ldf8.c.nc.d7 f121=\[r125\]
    1016:	90 07 f4 41 32 00 	            ldfe.c.nc f121=\[r125\]
    101c:	00 00 04 00       	            nop.i 0x0
    1020:	08 c8 03 fa 22 19 	\[MMI\]       ldfe.c.nc.nt1 f121=\[r125\]
    1026:	90 07 f4 45 32 00 	            ldfe.c.nc.nt1 f121=\[r125\]
    102c:	00 00 04 00       	            nop.i 0x0
    1030:	08 c8 03 fa 24 19 	\[MMI\]       ldfe.c.nc.d2 f121=\[r125\]
    1036:	90 07 f4 49 32 00 	            ldfe.c.nc.d2 f121=\[r125\]
    103c:	00 00 04 00       	            nop.i 0x0
    1040:	08 c8 03 fa 26 19 	\[MMI\]       ldfe.c.nc.nta f121=\[r125\]
    1046:	90 07 f4 4d 32 00 	            ldfe.c.nc.nta f121=\[r125\]
    104c:	00 00 04 00       	            nop.i 0x0
    1050:	08 c8 03 fb 20 19 	\[MMI\]       ldfe.c.nc.d4 f121=\[r125\]
    1056:	90 07 f6 45 32 00 	            ldfe.c.nc.d5 f121=\[r125\]
    105c:	00 00 04 00       	            nop.i 0x0
    1060:	08 c8 03 fb 24 19 	\[MMI\]       ldfe.c.nc.d6 f121=\[r125\]
    1066:	90 07 f6 4d 32 00 	            ldfe.c.nc.d7 f121=\[r125\]
    106c:	00 00 04 00       	            nop.i 0x0
    1070:	08 c0 03 28 00 10 	\[MMI\]       ld1 r120=\[r20\]
    1076:	80 07 50 04 20 00 	            ld1.nt1 r120=\[r20\]
    107c:	00 00 04 00       	            nop.i 0x0
    1080:	08 c0 03 28 02 10 	\[MMI\]       ld1.nt1 r120=\[r20\]
    1086:	80 07 50 08 20 00 	            ld1.d2 r120=\[r20\]
    108c:	00 00 04 00       	            nop.i 0x0
    1090:	08 c0 03 28 04 10 	\[MMI\]       ld1.d2 r120=\[r20\]
    1096:	80 07 50 0c 20 00 	            ld1.nta r120=\[r20\]
    109c:	00 00 04 00       	            nop.i 0x0
    10a0:	08 c0 03 28 06 10 	\[MMI\]       ld1.nta r120=\[r20\]
    10a6:	80 07 52 00 20 00 	            ld1.d4 r120=\[r20\]
    10ac:	00 00 04 00       	            nop.i 0x0
    10b0:	08 c0 03 29 02 10 	\[MMI\]       ld1.d5 r120=\[r20\]
    10b6:	80 07 52 08 20 00 	            ld1.d6 r120=\[r20\]
    10bc:	00 00 04 00       	            nop.i 0x0
    10c0:	08 c0 03 29 06 10 	\[MMI\]       ld1.d7 r120=\[r20\]
    10c6:	80 07 50 10 20 00 	            ld2 r120=\[r20\]
    10cc:	00 00 04 00       	            nop.i 0x0
    10d0:	08 c0 03 28 0a 10 	\[MMI\]       ld2.nt1 r120=\[r20\]
    10d6:	80 07 50 14 20 00 	            ld2.nt1 r120=\[r20\]
    10dc:	00 00 04 00       	            nop.i 0x0
    10e0:	08 c0 03 28 0c 10 	\[MMI\]       ld2.d2 r120=\[r20\]
    10e6:	80 07 50 18 20 00 	            ld2.d2 r120=\[r20\]
    10ec:	00 00 04 00       	            nop.i 0x0
    10f0:	08 c0 03 28 0e 10 	\[MMI\]       ld2.nta r120=\[r20\]
    10f6:	80 07 50 1c 20 00 	            ld2.nta r120=\[r20\]
    10fc:	00 00 04 00       	            nop.i 0x0
    1100:	08 c0 03 29 08 10 	\[MMI\]       ld2.d4 r120=\[r20\]
    1106:	80 07 52 14 20 00 	            ld2.d5 r120=\[r20\]
    110c:	00 00 04 00       	            nop.i 0x0
    1110:	08 c0 03 29 0c 10 	\[MMI\]       ld2.d6 r120=\[r20\]
    1116:	80 07 52 1c 20 00 	            ld2.d7 r120=\[r20\]
    111c:	00 00 04 00       	            nop.i 0x0
    1120:	08 c0 03 28 10 10 	\[MMI\]       ld4 r120=\[r20\]
    1126:	80 07 50 24 20 00 	            ld4.nt1 r120=\[r20\]
    112c:	00 00 04 00       	            nop.i 0x0
    1130:	08 c0 03 28 12 10 	\[MMI\]       ld4.nt1 r120=\[r20\]
    1136:	80 07 50 28 20 00 	            ld4.d2 r120=\[r20\]
    113c:	00 00 04 00       	            nop.i 0x0
    1140:	08 c0 03 28 14 10 	\[MMI\]       ld4.d2 r120=\[r20\]
    1146:	80 07 50 2c 20 00 	            ld4.nta r120=\[r20\]
    114c:	00 00 04 00       	            nop.i 0x0
    1150:	08 c0 03 28 16 10 	\[MMI\]       ld4.nta r120=\[r20\]
    1156:	80 07 52 20 20 00 	            ld4.d4 r120=\[r20\]
    115c:	00 00 04 00       	            nop.i 0x0
    1160:	08 c0 03 29 12 10 	\[MMI\]       ld4.d5 r120=\[r20\]
    1166:	80 07 52 28 20 00 	            ld4.d6 r120=\[r20\]
    116c:	00 00 04 00       	            nop.i 0x0
    1170:	08 c0 03 29 16 10 	\[MMI\]       ld4.d7 r120=\[r20\]
    1176:	80 07 50 30 20 00 	            ld8 r120=\[r20\]
    117c:	00 00 04 00       	            nop.i 0x0
    1180:	08 c0 03 28 1a 10 	\[MMI\]       ld8.nt1 r120=\[r20\]
    1186:	80 07 50 34 20 00 	            ld8.nt1 r120=\[r20\]
    118c:	00 00 04 00       	            nop.i 0x0
    1190:	08 c0 03 28 1c 10 	\[MMI\]       ld8.d2 r120=\[r20\]
    1196:	80 07 50 38 20 00 	            ld8.d2 r120=\[r20\]
    119c:	00 00 04 00       	            nop.i 0x0
    11a0:	08 c0 03 28 1e 10 	\[MMI\]       ld8.nta r120=\[r20\]
    11a6:	80 07 50 3c 20 00 	            ld8.nta r120=\[r20\]
    11ac:	00 00 04 00       	            nop.i 0x0
    11b0:	08 c0 03 29 18 10 	\[MMI\]       ld8.d4 r120=\[r20\]
    11b6:	80 07 52 34 20 00 	            ld8.d5 r120=\[r20\]
    11bc:	00 00 04 00       	            nop.i 0x0
    11c0:	08 c0 03 29 1c 10 	\[MMI\]       ld8.d6 r120=\[r20\]
    11c6:	80 07 52 3c 20 00 	            ld8.d7 r120=\[r20\]
    11cc:	00 00 04 00       	            nop.i 0x0
    11d0:	08 c0 03 28 20 10 	\[MMI\]       ld1.s r120=\[r20\]
    11d6:	80 07 50 44 20 00 	            ld1.s.nt1 r120=\[r20\]
    11dc:	00 00 04 00       	            nop.i 0x0
    11e0:	08 c0 03 28 22 10 	\[MMI\]       ld1.s.nt1 r120=\[r20\]
    11e6:	80 07 50 48 20 00 	            ld1.s.d2 r120=\[r20\]
    11ec:	00 00 04 00       	            nop.i 0x0
    11f0:	08 c0 03 28 24 10 	\[MMI\]       ld1.s.d2 r120=\[r20\]
    11f6:	80 07 50 4c 20 00 	            ld1.s.nta r120=\[r20\]
    11fc:	00 00 04 00       	            nop.i 0x0
    1200:	08 c0 03 28 26 10 	\[MMI\]       ld1.s.nta r120=\[r20\]
    1206:	80 07 52 40 20 00 	            ld1.s.d4 r120=\[r20\]
    120c:	00 00 04 00       	            nop.i 0x0
    1210:	08 c0 03 29 22 10 	\[MMI\]       ld1.s.d5 r120=\[r20\]
    1216:	80 07 52 48 20 00 	            ld1.s.d6 r120=\[r20\]
    121c:	00 00 04 00       	            nop.i 0x0
    1220:	08 c0 03 29 26 10 	\[MMI\]       ld1.s.d7 r120=\[r20\]
    1226:	80 07 50 50 20 00 	            ld2.s r120=\[r20\]
    122c:	00 00 04 00       	            nop.i 0x0
    1230:	08 c0 03 28 2a 10 	\[MMI\]       ld2.s.nt1 r120=\[r20\]
    1236:	80 07 50 54 20 00 	            ld2.s.nt1 r120=\[r20\]
    123c:	00 00 04 00       	            nop.i 0x0
    1240:	08 c0 03 28 2c 10 	\[MMI\]       ld2.s.d2 r120=\[r20\]
    1246:	80 07 50 58 20 00 	            ld2.s.d2 r120=\[r20\]
    124c:	00 00 04 00       	            nop.i 0x0
    1250:	08 c0 03 28 2e 10 	\[MMI\]       ld2.s.nta r120=\[r20\]
    1256:	80 07 50 5c 20 00 	            ld2.s.nta r120=\[r20\]
    125c:	00 00 04 00       	            nop.i 0x0
    1260:	08 c0 03 29 28 10 	\[MMI\]       ld2.s.d4 r120=\[r20\]
    1266:	80 07 52 54 20 00 	            ld2.s.d5 r120=\[r20\]
    126c:	00 00 04 00       	            nop.i 0x0
    1270:	08 c0 03 29 2c 10 	\[MMI\]       ld2.s.d6 r120=\[r20\]
    1276:	80 07 52 5c 20 00 	            ld2.s.d7 r120=\[r20\]
    127c:	00 00 04 00       	            nop.i 0x0
    1280:	08 c0 03 28 30 10 	\[MMI\]       ld4.s r120=\[r20\]
    1286:	80 07 50 64 20 00 	            ld4.s.nt1 r120=\[r20\]
    128c:	00 00 04 00       	            nop.i 0x0
    1290:	08 c0 03 28 32 10 	\[MMI\]       ld4.s.nt1 r120=\[r20\]
    1296:	80 07 50 68 20 00 	            ld4.s.d2 r120=\[r20\]
    129c:	00 00 04 00       	            nop.i 0x0
    12a0:	08 c0 03 28 34 10 	\[MMI\]       ld4.s.d2 r120=\[r20\]
    12a6:	80 07 50 6c 20 00 	            ld4.s.nta r120=\[r20\]
    12ac:	00 00 04 00       	            nop.i 0x0
    12b0:	08 c0 03 28 36 10 	\[MMI\]       ld4.s.nta r120=\[r20\]
    12b6:	80 07 52 60 20 00 	            ld4.s.d4 r120=\[r20\]
    12bc:	00 00 04 00       	            nop.i 0x0
    12c0:	08 c0 03 29 32 10 	\[MMI\]       ld4.s.d5 r120=\[r20\]
    12c6:	80 07 52 68 20 00 	            ld4.s.d6 r120=\[r20\]
    12cc:	00 00 04 00       	            nop.i 0x0
    12d0:	08 c0 03 29 36 10 	\[MMI\]       ld4.s.d7 r120=\[r20\]
    12d6:	80 07 50 70 20 00 	            ld8.s r120=\[r20\]
    12dc:	00 00 04 00       	            nop.i 0x0
    12e0:	08 c0 03 28 3a 10 	\[MMI\]       ld8.s.nt1 r120=\[r20\]
    12e6:	80 07 50 74 20 00 	            ld8.s.nt1 r120=\[r20\]
    12ec:	00 00 04 00       	            nop.i 0x0
    12f0:	08 c0 03 28 3c 10 	\[MMI\]       ld8.s.d2 r120=\[r20\]
    12f6:	80 07 50 78 20 00 	            ld8.s.d2 r120=\[r20\]
    12fc:	00 00 04 00       	            nop.i 0x0
    1300:	08 c0 03 28 3e 10 	\[MMI\]       ld8.s.nta r120=\[r20\]
    1306:	80 07 50 7c 20 00 	            ld8.s.nta r120=\[r20\]
    130c:	00 00 04 00       	            nop.i 0x0
    1310:	08 c0 03 29 38 10 	\[MMI\]       ld8.s.d4 r120=\[r20\]
    1316:	80 07 52 74 20 00 	            ld8.s.d5 r120=\[r20\]
    131c:	00 00 04 00       	            nop.i 0x0
    1320:	08 c0 03 29 3c 10 	\[MMI\]       ld8.s.d6 r120=\[r20\]
    1326:	80 07 52 7c 20 00 	            ld8.s.d7 r120=\[r20\]
    132c:	00 00 04 00       	            nop.i 0x0
    1330:	08 c0 03 28 40 10 	\[MMI\]       ld1.a r120=\[r20\]
    1336:	80 07 50 84 20 00 	            ld1.a.nt1 r120=\[r20\]
    133c:	00 00 04 00       	            nop.i 0x0
    1340:	08 c0 03 28 42 10 	\[MMI\]       ld1.a.nt1 r120=\[r20\]
    1346:	80 07 50 88 20 00 	            ld1.a.d2 r120=\[r20\]
    134c:	00 00 04 00       	            nop.i 0x0
    1350:	08 c0 03 28 44 10 	\[MMI\]       ld1.a.d2 r120=\[r20\]
    1356:	80 07 50 8c 20 00 	            ld1.a.nta r120=\[r20\]
    135c:	00 00 04 00       	            nop.i 0x0
    1360:	08 c0 03 28 46 10 	\[MMI\]       ld1.a.nta r120=\[r20\]
    1366:	80 07 52 80 20 00 	            ld1.a.d4 r120=\[r20\]
    136c:	00 00 04 00       	            nop.i 0x0
    1370:	08 c0 03 29 42 10 	\[MMI\]       ld1.a.d5 r120=\[r20\]
    1376:	80 07 52 88 20 00 	            ld1.a.d6 r120=\[r20\]
    137c:	00 00 04 00       	            nop.i 0x0
    1380:	08 c0 03 29 46 10 	\[MMI\]       ld1.a.d7 r120=\[r20\]
    1386:	80 07 50 90 20 00 	            ld2.a r120=\[r20\]
    138c:	00 00 04 00       	            nop.i 0x0
    1390:	08 c0 03 28 4a 10 	\[MMI\]       ld2.a.nt1 r120=\[r20\]
    1396:	80 07 50 94 20 00 	            ld2.a.nt1 r120=\[r20\]
    139c:	00 00 04 00       	            nop.i 0x0
    13a0:	08 c0 03 28 4c 10 	\[MMI\]       ld2.a.d2 r120=\[r20\]
    13a6:	80 07 50 9c 20 00 	            ld2.a.nta r120=\[r20\]
    13ac:	00 00 04 00       	            nop.i 0x0
    13b0:	08 c0 03 28 4e 10 	\[MMI\]       ld2.a.nta r120=\[r20\]
    13b6:	80 07 52 90 20 00 	            ld2.a.d4 r120=\[r20\]
    13bc:	00 00 04 00       	            nop.i 0x0
    13c0:	08 c0 03 29 4a 10 	\[MMI\]       ld2.a.d5 r120=\[r20\]
    13c6:	80 07 52 98 20 00 	            ld2.a.d6 r120=\[r20\]
    13cc:	00 00 04 00       	            nop.i 0x0
    13d0:	08 c0 03 29 4e 10 	\[MMI\]       ld2.a.d7 r120=\[r20\]
    13d6:	80 07 50 a0 20 00 	            ld4.a r120=\[r20\]
    13dc:	00 00 04 00       	            nop.i 0x0
    13e0:	08 c0 03 28 52 10 	\[MMI\]       ld4.a.nt1 r120=\[r20\]
    13e6:	80 07 50 a4 20 00 	            ld4.a.nt1 r120=\[r20\]
    13ec:	00 00 04 00       	            nop.i 0x0
    13f0:	08 c0 03 28 54 10 	\[MMI\]       ld4.a.d2 r120=\[r20\]
    13f6:	80 07 50 a8 20 00 	            ld4.a.d2 r120=\[r20\]
    13fc:	00 00 04 00       	            nop.i 0x0
    1400:	08 c0 03 28 56 10 	\[MMI\]       ld4.a.nta r120=\[r20\]
    1406:	80 07 50 ac 20 00 	            ld4.a.nta r120=\[r20\]
    140c:	00 00 04 00       	            nop.i 0x0
    1410:	08 c0 03 29 50 10 	\[MMI\]       ld4.a.d4 r120=\[r20\]
    1416:	80 07 52 a4 20 00 	            ld4.a.d5 r120=\[r20\]
    141c:	00 00 04 00       	            nop.i 0x0
    1420:	08 c0 03 29 54 10 	\[MMI\]       ld4.a.d6 r120=\[r20\]
    1426:	80 07 52 ac 20 00 	            ld4.a.d7 r120=\[r20\]
    142c:	00 00 04 00       	            nop.i 0x0
    1430:	08 c0 03 28 58 10 	\[MMI\]       ld8.a r120=\[r20\]
    1436:	80 07 50 b4 20 00 	            ld8.a.nt1 r120=\[r20\]
    143c:	00 00 04 00       	            nop.i 0x0
    1440:	08 c0 03 28 5a 10 	\[MMI\]       ld8.a.nt1 r120=\[r20\]
    1446:	80 07 50 b8 20 00 	            ld8.a.d2 r120=\[r20\]
    144c:	00 00 04 00       	            nop.i 0x0
    1450:	08 c0 03 28 5c 10 	\[MMI\]       ld8.a.d2 r120=\[r20\]
    1456:	80 07 50 bc 20 00 	            ld8.a.nta r120=\[r20\]
    145c:	00 00 04 00       	            nop.i 0x0
    1460:	08 c0 03 28 5e 10 	\[MMI\]       ld8.a.nta r120=\[r20\]
    1466:	80 07 52 b4 20 00 	            ld8.a.d5 r120=\[r20\]
    146c:	00 00 04 00       	            nop.i 0x0
    1470:	08 c0 03 29 5c 10 	\[MMI\]       ld8.a.d6 r120=\[r20\]
    1476:	80 07 52 bc 20 00 	            ld8.a.d7 r120=\[r20\]
    147c:	00 00 04 00       	            nop.i 0x0
    1480:	08 c0 03 28 60 10 	\[MMI\]       ld1.sa r120=\[r20\]
    1486:	80 07 50 c4 20 00 	            ld1.sa.nt1 r120=\[r20\]
    148c:	00 00 04 00       	            nop.i 0x0
    1490:	08 c0 03 28 62 10 	\[MMI\]       ld1.sa.nt1 r120=\[r20\]
    1496:	80 07 50 c8 20 00 	            ld1.sa.d2 r120=\[r20\]
    149c:	00 00 04 00       	            nop.i 0x0
    14a0:	08 c0 03 28 64 10 	\[MMI\]       ld1.sa.d2 r120=\[r20\]
    14a6:	80 07 50 cc 20 00 	            ld1.sa.nta r120=\[r20\]
    14ac:	00 00 04 00       	            nop.i 0x0
    14b0:	08 c0 03 28 66 10 	\[MMI\]       ld1.sa.nta r120=\[r20\]
    14b6:	80 07 52 c0 20 00 	            ld1.sa.d4 r120=\[r20\]
    14bc:	00 00 04 00       	            nop.i 0x0
    14c0:	08 c0 03 29 62 10 	\[MMI\]       ld1.sa.d5 r120=\[r20\]
    14c6:	80 07 52 c8 20 00 	            ld1.sa.d6 r120=\[r20\]
    14cc:	00 00 04 00       	            nop.i 0x0
    14d0:	08 c0 03 29 66 10 	\[MMI\]       ld1.sa.d7 r120=\[r20\]
    14d6:	80 07 50 d0 20 00 	            ld2.sa r120=\[r20\]
    14dc:	00 00 04 00       	            nop.i 0x0
    14e0:	08 c0 03 28 6a 10 	\[MMI\]       ld2.sa.nt1 r120=\[r20\]
    14e6:	80 07 50 d4 20 00 	            ld2.sa.nt1 r120=\[r20\]
    14ec:	00 00 04 00       	            nop.i 0x0
    14f0:	08 c0 03 28 6c 10 	\[MMI\]       ld2.sa.d2 r120=\[r20\]
    14f6:	80 07 50 d8 20 00 	            ld2.sa.d2 r120=\[r20\]
    14fc:	00 00 04 00       	            nop.i 0x0
    1500:	08 c0 03 28 6e 10 	\[MMI\]       ld2.sa.nta r120=\[r20\]
    1506:	80 07 50 dc 20 00 	            ld2.sa.nta r120=\[r20\]
    150c:	00 00 04 00       	            nop.i 0x0
    1510:	08 c0 03 29 68 10 	\[MMI\]       ld2.sa.d4 r120=\[r20\]
    1516:	80 07 52 d4 20 00 	            ld2.sa.d5 r120=\[r20\]
    151c:	00 00 04 00       	            nop.i 0x0
    1520:	08 c0 03 29 6c 10 	\[MMI\]       ld2.sa.d6 r120=\[r20\]
    1526:	80 07 52 dc 20 00 	            ld2.sa.d7 r120=\[r20\]
    152c:	00 00 04 00       	            nop.i 0x0
    1530:	08 c0 03 28 72 10 	\[MMI\]       ld4.sa.nt1 r120=\[r20\]
    1536:	80 07 50 e4 20 00 	            ld4.sa.nt1 r120=\[r20\]
    153c:	00 00 04 00       	            nop.i 0x0
    1540:	08 c0 03 28 74 10 	\[MMI\]       ld4.sa.d2 r120=\[r20\]
    1546:	80 07 50 e8 20 00 	            ld4.sa.d2 r120=\[r20\]
    154c:	00 00 04 00       	            nop.i 0x0
    1550:	08 c0 03 28 76 10 	\[MMI\]       ld4.sa.nta r120=\[r20\]
    1556:	80 07 50 ec 20 00 	            ld4.sa.nta r120=\[r20\]
    155c:	00 00 04 00       	            nop.i 0x0
    1560:	08 c0 03 29 70 10 	\[MMI\]       ld4.sa.d4 r120=\[r20\]
    1566:	80 07 52 e4 20 00 	            ld4.sa.d5 r120=\[r20\]
    156c:	00 00 04 00       	            nop.i 0x0
    1570:	08 c0 03 29 74 10 	\[MMI\]       ld4.sa.d6 r120=\[r20\]
    1576:	80 07 52 ec 20 00 	            ld4.sa.d7 r120=\[r20\]
    157c:	00 00 04 00       	            nop.i 0x0
    1580:	08 c0 03 28 78 10 	\[MMI\]       ld8.sa r120=\[r20\]
    1586:	80 07 50 f4 20 00 	            ld8.sa.nt1 r120=\[r20\]
    158c:	00 00 04 00       	            nop.i 0x0
    1590:	08 c0 03 28 7a 10 	\[MMI\]       ld8.sa.nt1 r120=\[r20\]
    1596:	80 07 50 f8 20 00 	            ld8.sa.d2 r120=\[r20\]
    159c:	00 00 04 00       	            nop.i 0x0
    15a0:	08 c0 03 28 7c 10 	\[MMI\]       ld8.sa.d2 r120=\[r20\]
    15a6:	80 07 50 fc 20 00 	            ld8.sa.nta r120=\[r20\]
    15ac:	00 00 04 00       	            nop.i 0x0
    15b0:	08 c0 03 28 7e 10 	\[MMI\]       ld8.sa.nta r120=\[r20\]
    15b6:	80 07 52 f0 20 00 	            ld8.sa.d4 r120=\[r20\]
    15bc:	00 00 04 00       	            nop.i 0x0
    15c0:	08 c0 03 29 7a 10 	\[MMI\]       ld8.sa.d5 r120=\[r20\]
    15c6:	80 07 52 f8 20 00 	            ld8.sa.d6 r120=\[r20\]
    15cc:	00 00 04 00       	            nop.i 0x0
    15d0:	08 c0 03 29 7e 10 	\[MMI\]       ld8.sa.d7 r120=\[r20\]
    15d6:	80 07 50 00 21 00 	            ld1.bias r120=\[r20\]
    15dc:	00 00 04 00       	            nop.i 0x0
    15e0:	08 c0 03 28 82 10 	\[MMI\]       ld1.bias.nt1 r120=\[r20\]
    15e6:	80 07 50 04 21 00 	            ld1.bias.nt1 r120=\[r20\]
    15ec:	00 00 04 00       	            nop.i 0x0
    15f0:	08 c0 03 28 84 10 	\[MMI\]       ld1.bias.d2 r120=\[r20\]
    15f6:	80 07 50 08 21 00 	            ld1.bias.d2 r120=\[r20\]
    15fc:	00 00 04 00       	            nop.i 0x0
    1600:	08 c0 03 28 86 10 	\[MMI\]       ld1.bias.nta r120=\[r20\]
    1606:	80 07 50 0c 21 00 	            ld1.bias.nta r120=\[r20\]
    160c:	00 00 04 00       	            nop.i 0x0
    1610:	08 c0 03 29 80 10 	\[MMI\]       ld1.bias.d4 r120=\[r20\]
    1616:	80 07 52 04 21 00 	            ld1.bias.d5 r120=\[r20\]
    161c:	00 00 04 00       	            nop.i 0x0
    1620:	08 c0 03 29 84 10 	\[MMI\]       ld1.bias.d6 r120=\[r20\]
    1626:	80 07 52 0c 21 00 	            ld1.bias.d7 r120=\[r20\]
    162c:	00 00 04 00       	            nop.i 0x0
    1630:	08 c0 03 28 88 10 	\[MMI\]       ld2.bias r120=\[r20\]
    1636:	80 07 50 14 21 00 	            ld2.bias.nt1 r120=\[r20\]
    163c:	00 00 04 00       	            nop.i 0x0
    1640:	08 c0 03 28 8a 10 	\[MMI\]       ld2.bias.nt1 r120=\[r20\]
    1646:	80 07 50 18 21 00 	            ld2.bias.d2 r120=\[r20\]
    164c:	00 00 04 00       	            nop.i 0x0
    1650:	08 c0 03 28 8c 10 	\[MMI\]       ld2.bias.d2 r120=\[r20\]
    1656:	80 07 50 1c 21 00 	            ld2.bias.nta r120=\[r20\]
    165c:	00 00 04 00       	            nop.i 0x0
    1660:	08 c0 03 28 8e 10 	\[MMI\]       ld2.bias.nta r120=\[r20\]
    1666:	80 07 52 10 21 00 	            ld2.bias.d4 r120=\[r20\]
    166c:	00 00 04 00       	            nop.i 0x0
    1670:	08 c0 03 29 8a 10 	\[MMI\]       ld2.bias.d5 r120=\[r20\]
    1676:	80 07 52 18 21 00 	            ld2.bias.d6 r120=\[r20\]
    167c:	00 00 04 00       	            nop.i 0x0
    1680:	08 c0 03 29 8e 10 	\[MMI\]       ld2.bias.d7 r120=\[r20\]
    1686:	80 07 50 20 21 00 	            ld4.bias r120=\[r20\]
    168c:	00 00 04 00       	            nop.i 0x0
    1690:	08 c0 03 28 92 10 	\[MMI\]       ld4.bias.nt1 r120=\[r20\]
    1696:	80 07 50 24 21 00 	            ld4.bias.nt1 r120=\[r20\]
    169c:	00 00 04 00       	            nop.i 0x0
    16a0:	08 c0 03 28 94 10 	\[MMI\]       ld4.bias.d2 r120=\[r20\]
    16a6:	80 07 50 28 21 00 	            ld4.bias.d2 r120=\[r20\]
    16ac:	00 00 04 00       	            nop.i 0x0
    16b0:	08 c0 03 28 96 10 	\[MMI\]       ld4.bias.nta r120=\[r20\]
    16b6:	80 07 50 2c 21 00 	            ld4.bias.nta r120=\[r20\]
    16bc:	00 00 04 00       	            nop.i 0x0
    16c0:	08 c0 03 29 90 10 	\[MMI\]       ld4.bias.d4 r120=\[r20\]
    16c6:	80 07 52 24 21 00 	            ld4.bias.d5 r120=\[r20\]
    16cc:	00 00 04 00       	            nop.i 0x0
    16d0:	08 c0 03 29 94 10 	\[MMI\]       ld4.bias.d6 r120=\[r20\]
    16d6:	80 07 52 2c 21 00 	            ld4.bias.d7 r120=\[r20\]
    16dc:	00 00 04 00       	            nop.i 0x0
    16e0:	08 c0 03 28 98 10 	\[MMI\]       ld8.bias r120=\[r20\]
    16e6:	80 07 50 34 21 00 	            ld8.bias.nt1 r120=\[r20\]
    16ec:	00 00 04 00       	            nop.i 0x0
    16f0:	08 c0 03 28 9a 10 	\[MMI\]       ld8.bias.nt1 r120=\[r20\]
    16f6:	80 07 50 38 21 00 	            ld8.bias.d2 r120=\[r20\]
    16fc:	00 00 04 00       	            nop.i 0x0
    1700:	08 c0 03 28 9c 10 	\[MMI\]       ld8.bias.d2 r120=\[r20\]
    1706:	80 07 50 3c 21 00 	            ld8.bias.nta r120=\[r20\]
    170c:	00 00 04 00       	            nop.i 0x0
    1710:	08 c0 03 28 9e 10 	\[MMI\]       ld8.bias.nta r120=\[r20\]
    1716:	80 07 52 30 21 00 	            ld8.bias.d4 r120=\[r20\]
    171c:	00 00 04 00       	            nop.i 0x0
    1720:	08 c0 03 29 9a 10 	\[MMI\]       ld8.bias.d5 r120=\[r20\]
    1726:	80 07 52 38 21 00 	            ld8.bias.d6 r120=\[r20\]
    172c:	00 00 04 00       	            nop.i 0x0
    1730:	08 c0 03 29 9e 10 	\[MMI\]       ld8.bias.d7 r120=\[r20\]
    1736:	80 07 50 40 21 00 	            ld1.acq r120=\[r20\]
    173c:	00 00 04 00       	            nop.i 0x0
    1740:	08 c0 03 28 a2 10 	\[MMI\]       ld1.acq.nt1 r120=\[r20\]
    1746:	80 07 50 44 21 00 	            ld1.acq.nt1 r120=\[r20\]
    174c:	00 00 04 00       	            nop.i 0x0
    1750:	08 c0 03 28 a4 10 	\[MMI\]       ld1.acq.d2 r120=\[r20\]
    1756:	80 07 50 48 21 00 	            ld1.acq.d2 r120=\[r20\]
    175c:	00 00 04 00       	            nop.i 0x0
    1760:	08 c0 03 28 a6 10 	\[MMI\]       ld1.acq.nta r120=\[r20\]
    1766:	80 07 50 4c 21 00 	            ld1.acq.nta r120=\[r20\]
    176c:	00 00 04 00       	            nop.i 0x0
    1770:	08 c0 03 29 a0 10 	\[MMI\]       ld1.acq.d4 r120=\[r20\]
    1776:	80 07 52 44 21 00 	            ld1.acq.d5 r120=\[r20\]
    177c:	00 00 04 00       	            nop.i 0x0
    1780:	08 c0 03 29 a4 10 	\[MMI\]       ld1.acq.d6 r120=\[r20\]
    1786:	80 07 50 50 21 00 	            ld2.acq r120=\[r20\]
    178c:	00 00 04 00       	            nop.i 0x0
    1790:	08 c0 03 28 aa 10 	\[MMI\]       ld2.acq.nt1 r120=\[r20\]
    1796:	80 07 50 54 21 00 	            ld2.acq.nt1 r120=\[r20\]
    179c:	00 00 04 00       	            nop.i 0x0
    17a0:	08 c0 03 28 ac 10 	\[MMI\]       ld2.acq.d2 r120=\[r20\]
    17a6:	80 07 50 58 21 00 	            ld2.acq.d2 r120=\[r20\]
    17ac:	00 00 04 00       	            nop.i 0x0
    17b0:	08 c0 03 28 ae 10 	\[MMI\]       ld2.acq.nta r120=\[r20\]
    17b6:	80 07 50 5c 21 00 	            ld2.acq.nta r120=\[r20\]
    17bc:	00 00 04 00       	            nop.i 0x0
    17c0:	08 c0 03 29 a8 10 	\[MMI\]       ld2.acq.d4 r120=\[r20\]
    17c6:	80 07 52 54 21 00 	            ld2.acq.d5 r120=\[r20\]
    17cc:	00 00 04 00       	            nop.i 0x0
    17d0:	08 c0 03 29 ac 10 	\[MMI\]       ld2.acq.d6 r120=\[r20\]
    17d6:	80 07 52 5c 21 00 	            ld2.acq.d7 r120=\[r20\]
    17dc:	00 00 04 00       	            nop.i 0x0
    17e0:	08 c0 03 28 b0 10 	\[MMI\]       ld4.acq r120=\[r20\]
    17e6:	80 07 50 64 21 00 	            ld4.acq.nt1 r120=\[r20\]
    17ec:	00 00 04 00       	            nop.i 0x0
    17f0:	08 c0 03 28 b2 10 	\[MMI\]       ld4.acq.nt1 r120=\[r20\]
    17f6:	80 07 50 68 21 00 	            ld4.acq.d2 r120=\[r20\]
    17fc:	00 00 04 00       	            nop.i 0x0
    1800:	08 c0 03 28 b4 10 	\[MMI\]       ld4.acq.d2 r120=\[r20\]
    1806:	80 07 50 6c 21 00 	            ld4.acq.nta r120=\[r20\]
    180c:	00 00 04 00       	            nop.i 0x0
    1810:	08 c0 03 28 b6 10 	\[MMI\]       ld4.acq.nta r120=\[r20\]
    1816:	80 07 52 60 21 00 	            ld4.acq.d4 r120=\[r20\]
    181c:	00 00 04 00       	            nop.i 0x0
    1820:	08 c0 03 29 b2 10 	\[MMI\]       ld4.acq.d5 r120=\[r20\]
    1826:	80 07 52 68 21 00 	            ld4.acq.d6 r120=\[r20\]
    182c:	00 00 04 00       	            nop.i 0x0
    1830:	08 c0 03 29 b6 10 	\[MMI\]       ld4.acq.d7 r120=\[r20\]
    1836:	80 07 50 70 21 00 	            ld8.acq r120=\[r20\]
    183c:	00 00 04 00       	            nop.i 0x0
    1840:	08 c0 03 28 ba 10 	\[MMI\]       ld8.acq.nt1 r120=\[r20\]
    1846:	80 07 50 74 21 00 	            ld8.acq.nt1 r120=\[r20\]
    184c:	00 00 04 00       	            nop.i 0x0
    1850:	08 c0 03 28 bc 10 	\[MMI\]       ld8.acq.d2 r120=\[r20\]
    1856:	80 07 50 78 21 00 	            ld8.acq.d2 r120=\[r20\]
    185c:	00 00 04 00       	            nop.i 0x0
    1860:	08 c0 03 28 be 10 	\[MMI\]       ld8.acq.nta r120=\[r20\]
    1866:	80 07 50 7c 21 00 	            ld8.acq.nta r120=\[r20\]
    186c:	00 00 04 00       	            nop.i 0x0
    1870:	08 c0 03 29 b8 10 	\[MMI\]       ld8.acq.d4 r120=\[r20\]
    1876:	80 07 52 74 21 00 	            ld8.acq.d5 r120=\[r20\]
    187c:	00 00 04 00       	            nop.i 0x0
    1880:	08 c0 03 29 bc 10 	\[MMI\]       ld8.acq.d6 r120=\[r20\]
    1886:	80 07 52 7c 21 00 	            ld8.acq.d7 r120=\[r20\]
    188c:	00 00 04 00       	            nop.i 0x0
    1890:	08 c0 03 28 d8 10 	\[MMI\]       ld8.fill r120=\[r20\]
    1896:	80 07 50 b4 21 00 	            ld8.fill.nt1 r120=\[r20\]
    189c:	00 00 04 00       	            nop.i 0x0
    18a0:	08 c0 03 28 da 10 	\[MMI\]       ld8.fill.nt1 r120=\[r20\]
    18a6:	80 07 50 b8 21 00 	            ld8.fill.d2 r120=\[r20\]
    18ac:	00 00 04 00       	            nop.i 0x0
    18b0:	08 c0 03 28 dc 10 	\[MMI\]       ld8.fill.d2 r120=\[r20\]
    18b6:	80 07 50 bc 21 00 	            ld8.fill.nta r120=\[r20\]
    18bc:	00 00 04 00       	            nop.i 0x0
    18c0:	08 c0 03 28 de 10 	\[MMI\]       ld8.fill.nta r120=\[r20\]
    18c6:	80 07 52 b0 21 00 	            ld8.fill.d4 r120=\[r20\]
    18cc:	00 00 04 00       	            nop.i 0x0
    18d0:	08 c0 03 29 da 10 	\[MMI\]       ld8.fill.d5 r120=\[r20\]
    18d6:	80 07 52 b8 21 00 	            ld8.fill.d6 r120=\[r20\]
    18dc:	00 00 04 00       	            nop.i 0x0
    18e0:	08 c0 03 29 de 10 	\[MMI\]       ld8.fill.d7 r120=\[r20\]
    18e6:	80 07 50 00 22 00 	            ld1.c.clr r120=\[r20\]
    18ec:	00 00 04 00       	            nop.i 0x0
    18f0:	08 c0 03 28 02 11 	\[MMI\]       ld1.c.clr.nt1 r120=\[r20\]
    18f6:	80 07 50 04 22 00 	            ld1.c.clr.nt1 r120=\[r20\]
    18fc:	00 00 04 00       	            nop.i 0x0
    1900:	08 c0 03 28 04 11 	\[MMI\]       ld1.c.clr.d2 r120=\[r20\]
    1906:	80 07 50 08 22 00 	            ld1.c.clr.d2 r120=\[r20\]
    190c:	00 00 04 00       	            nop.i 0x0
    1910:	08 c0 03 28 06 11 	\[MMI\]       ld1.c.clr.nta r120=\[r20\]
    1916:	80 07 50 0c 22 00 	            ld1.c.clr.nta r120=\[r20\]
    191c:	00 00 04 00       	            nop.i 0x0
    1920:	08 c0 03 29 00 11 	\[MMI\]       ld1.c.clr.d4 r120=\[r20\]
    1926:	80 07 52 04 22 00 	            ld1.c.clr.d5 r120=\[r20\]
    192c:	00 00 04 00       	            nop.i 0x0
    1930:	08 c0 03 29 04 11 	\[MMI\]       ld1.c.clr.d6 r120=\[r20\]
    1936:	80 07 52 0c 22 00 	            ld1.c.clr.d7 r120=\[r20\]
    193c:	00 00 04 00       	            nop.i 0x0
    1940:	08 c0 03 28 08 11 	\[MMI\]       ld2.c.clr r120=\[r20\]
    1946:	80 07 50 14 22 00 	            ld2.c.clr.nt1 r120=\[r20\]
    194c:	00 00 04 00       	            nop.i 0x0
    1950:	08 c0 03 28 0a 11 	\[MMI\]       ld2.c.clr.nt1 r120=\[r20\]
    1956:	80 07 50 18 22 00 	            ld2.c.clr.d2 r120=\[r20\]
    195c:	00 00 04 00       	            nop.i 0x0
    1960:	08 c0 03 28 0c 11 	\[MMI\]       ld2.c.clr.d2 r120=\[r20\]
    1966:	80 07 50 1c 22 00 	            ld2.c.clr.nta r120=\[r20\]
    196c:	00 00 04 00       	            nop.i 0x0
    1970:	08 c0 03 28 0e 11 	\[MMI\]       ld2.c.clr.nta r120=\[r20\]
    1976:	80 07 52 10 22 00 	            ld2.c.clr.d4 r120=\[r20\]
    197c:	00 00 04 00       	            nop.i 0x0
    1980:	08 c0 03 29 0a 11 	\[MMI\]       ld2.c.clr.d5 r120=\[r20\]
    1986:	80 07 52 18 22 00 	            ld2.c.clr.d6 r120=\[r20\]
    198c:	00 00 04 00       	            nop.i 0x0
    1990:	08 c0 03 29 0e 11 	\[MMI\]       ld2.c.clr.d7 r120=\[r20\]
    1996:	80 07 50 20 22 00 	            ld4.c.clr r120=\[r20\]
    199c:	00 00 04 00       	            nop.i 0x0
    19a0:	08 c0 03 28 12 11 	\[MMI\]       ld4.c.clr.nt1 r120=\[r20\]
    19a6:	80 07 50 24 22 00 	            ld4.c.clr.nt1 r120=\[r20\]
    19ac:	00 00 04 00       	            nop.i 0x0
    19b0:	08 c0 03 28 14 11 	\[MMI\]       ld4.c.clr.d2 r120=\[r20\]
    19b6:	80 07 50 28 22 00 	            ld4.c.clr.d2 r120=\[r20\]
    19bc:	00 00 04 00       	            nop.i 0x0
    19c0:	08 c0 03 28 16 11 	\[MMI\]       ld4.c.clr.nta r120=\[r20\]
    19c6:	80 07 50 2c 22 00 	            ld4.c.clr.nta r120=\[r20\]
    19cc:	00 00 04 00       	            nop.i 0x0
    19d0:	08 c0 03 29 10 11 	\[MMI\]       ld4.c.clr.d4 r120=\[r20\]
    19d6:	80 07 52 24 22 00 	            ld4.c.clr.d5 r120=\[r20\]
    19dc:	00 00 04 00       	            nop.i 0x0
    19e0:	08 c0 03 29 14 11 	\[MMI\]       ld4.c.clr.d6 r120=\[r20\]
    19e6:	80 07 52 2c 22 00 	            ld4.c.clr.d7 r120=\[r20\]
    19ec:	00 00 04 00       	            nop.i 0x0
    19f0:	08 c0 03 28 18 11 	\[MMI\]       ld8.c.clr r120=\[r20\]
    19f6:	80 07 50 34 22 00 	            ld8.c.clr.nt1 r120=\[r20\]
    19fc:	00 00 04 00       	            nop.i 0x0
    1a00:	08 c0 03 28 1a 11 	\[MMI\]       ld8.c.clr.nt1 r120=\[r20\]
    1a06:	80 07 50 38 22 00 	            ld8.c.clr.d2 r120=\[r20\]
    1a0c:	00 00 04 00       	            nop.i 0x0
    1a10:	08 c0 03 28 1c 11 	\[MMI\]       ld8.c.clr.d2 r120=\[r20\]
    1a16:	80 07 50 3c 22 00 	            ld8.c.clr.nta r120=\[r20\]
    1a1c:	00 00 04 00       	            nop.i 0x0
    1a20:	08 c0 03 28 1e 11 	\[MMI\]       ld8.c.clr.nta r120=\[r20\]
    1a26:	80 07 52 30 22 00 	            ld8.c.clr.d4 r120=\[r20\]
    1a2c:	00 00 04 00       	            nop.i 0x0
    1a30:	08 c0 03 29 1a 11 	\[MMI\]       ld8.c.clr.d5 r120=\[r20\]
    1a36:	80 07 52 38 22 00 	            ld8.c.clr.d6 r120=\[r20\]
    1a3c:	00 00 04 00       	            nop.i 0x0
    1a40:	08 c0 03 29 1e 11 	\[MMI\]       ld8.c.clr.d7 r120=\[r20\]
    1a46:	80 07 50 40 22 00 	            ld1.c.nc r120=\[r20\]
    1a4c:	00 00 04 00       	            nop.i 0x0
    1a50:	08 c0 03 28 22 11 	\[MMI\]       ld1.c.nc.nt1 r120=\[r20\]
    1a56:	80 07 50 44 22 00 	            ld1.c.nc.nt1 r120=\[r20\]
    1a5c:	00 00 04 00       	            nop.i 0x0
    1a60:	08 c0 03 28 24 11 	\[MMI\]       ld1.c.nc.d2 r120=\[r20\]
    1a66:	80 07 50 48 22 00 	            ld1.c.nc.d2 r120=\[r20\]
    1a6c:	00 00 04 00       	            nop.i 0x0
    1a70:	08 c0 03 28 26 11 	\[MMI\]       ld1.c.nc.nta r120=\[r20\]
    1a76:	80 07 50 4c 22 00 	            ld1.c.nc.nta r120=\[r20\]
    1a7c:	00 00 04 00       	            nop.i 0x0
    1a80:	08 c0 03 29 20 11 	\[MMI\]       ld1.c.nc.d4 r120=\[r20\]
    1a86:	80 07 52 44 22 00 	            ld1.c.nc.d5 r120=\[r20\]
    1a8c:	00 00 04 00       	            nop.i 0x0
    1a90:	08 c0 03 29 26 11 	\[MMI\]       ld1.c.nc.d7 r120=\[r20\]
    1a96:	80 07 50 50 22 00 	            ld2.c.nc r120=\[r20\]
    1a9c:	00 00 04 00       	            nop.i 0x0
    1aa0:	08 c0 03 28 2a 11 	\[MMI\]       ld2.c.nc.nt1 r120=\[r20\]
    1aa6:	80 07 50 54 22 00 	            ld2.c.nc.nt1 r120=\[r20\]
    1aac:	00 00 04 00       	            nop.i 0x0
    1ab0:	08 c0 03 28 2c 11 	\[MMI\]       ld2.c.nc.d2 r120=\[r20\]
    1ab6:	80 07 50 58 22 00 	            ld2.c.nc.d2 r120=\[r20\]
    1abc:	00 00 04 00       	            nop.i 0x0
    1ac0:	08 c0 03 28 2e 11 	\[MMI\]       ld2.c.nc.nta r120=\[r20\]
    1ac6:	80 07 50 5c 22 00 	            ld2.c.nc.nta r120=\[r20\]
    1acc:	00 00 04 00       	            nop.i 0x0
    1ad0:	08 c0 03 29 28 11 	\[MMI\]       ld2.c.nc.d4 r120=\[r20\]
    1ad6:	80 07 52 54 22 00 	            ld2.c.nc.d5 r120=\[r20\]
    1adc:	00 00 04 00       	            nop.i 0x0
    1ae0:	08 c0 03 29 2c 11 	\[MMI\]       ld2.c.nc.d6 r120=\[r20\]
    1ae6:	80 07 52 5c 22 00 	            ld2.c.nc.d7 r120=\[r20\]
    1aec:	00 00 04 00       	            nop.i 0x0
    1af0:	08 c0 03 28 30 11 	\[MMI\]       ld4.c.nc r120=\[r20\]
    1af6:	80 07 50 64 22 00 	            ld4.c.nc.nt1 r120=\[r20\]
    1afc:	00 00 04 00       	            nop.i 0x0
    1b00:	08 c0 03 28 32 11 	\[MMI\]       ld4.c.nc.nt1 r120=\[r20\]
    1b06:	80 07 50 68 22 00 	            ld4.c.nc.d2 r120=\[r20\]
    1b0c:	00 00 04 00       	            nop.i 0x0
    1b10:	08 c0 03 28 34 11 	\[MMI\]       ld4.c.nc.d2 r120=\[r20\]
    1b16:	80 07 50 6c 22 00 	            ld4.c.nc.nta r120=\[r20\]
    1b1c:	00 00 04 00       	            nop.i 0x0
    1b20:	08 c0 03 28 36 11 	\[MMI\]       ld4.c.nc.nta r120=\[r20\]
    1b26:	80 07 52 60 22 00 	            ld4.c.nc.d4 r120=\[r20\]
    1b2c:	00 00 04 00       	            nop.i 0x0
    1b30:	08 c0 03 29 32 11 	\[MMI\]       ld4.c.nc.d5 r120=\[r20\]
    1b36:	80 07 52 68 22 00 	            ld4.c.nc.d6 r120=\[r20\]
    1b3c:	00 00 04 00       	            nop.i 0x0
    1b40:	08 c0 03 29 36 11 	\[MMI\]       ld4.c.nc.d7 r120=\[r20\]
    1b46:	80 07 50 70 22 00 	            ld8.c.nc r120=\[r20\]
    1b4c:	00 00 04 00       	            nop.i 0x0
    1b50:	08 c0 03 28 3a 11 	\[MMI\]       ld8.c.nc.nt1 r120=\[r20\]
    1b56:	80 07 50 74 22 00 	            ld8.c.nc.nt1 r120=\[r20\]
    1b5c:	00 00 04 00       	            nop.i 0x0
    1b60:	08 c0 03 28 3c 11 	\[MMI\]       ld8.c.nc.d2 r120=\[r20\]
    1b66:	80 07 50 78 22 00 	            ld8.c.nc.d2 r120=\[r20\]
    1b6c:	00 00 04 00       	            nop.i 0x0
    1b70:	08 c0 03 28 3e 11 	\[MMI\]       ld8.c.nc.nta r120=\[r20\]
    1b76:	80 07 50 7c 22 00 	            ld8.c.nc.nta r120=\[r20\]
    1b7c:	00 00 04 00       	            nop.i 0x0
    1b80:	08 c0 03 29 38 11 	\[MMI\]       ld8.c.nc.d4 r120=\[r20\]
    1b86:	80 07 52 74 22 00 	            ld8.c.nc.d5 r120=\[r20\]
    1b8c:	00 00 04 00       	            nop.i 0x0
    1b90:	08 c0 03 29 3c 11 	\[MMI\]       ld8.c.nc.d6 r120=\[r20\]
    1b96:	80 07 52 7c 22 00 	            ld8.c.nc.d7 r120=\[r20\]
    1b9c:	00 00 04 00       	            nop.i 0x0
    1ba0:	08 c0 03 28 40 11 	\[MMI\]       ld1.c.clr.acq r120=\[r20\]
    1ba6:	80 07 50 84 22 00 	            ld1.c.clr.acq.nt1 r120=\[r20\]
    1bac:	00 00 04 00       	            nop.i 0x0
    1bb0:	08 c0 03 28 42 11 	\[MMI\]       ld1.c.clr.acq.nt1 r120=\[r20\]
    1bb6:	80 07 50 88 22 00 	            ld1.c.clr.acq.d2 r120=\[r20\]
    1bbc:	00 00 04 00       	            nop.i 0x0
    1bc0:	08 c0 03 28 44 11 	\[MMI\]       ld1.c.clr.acq.d2 r120=\[r20\]
    1bc6:	80 07 50 8c 22 00 	            ld1.c.clr.acq.nta r120=\[r20\]
    1bcc:	00 00 04 00       	            nop.i 0x0
    1bd0:	08 c0 03 28 46 11 	\[MMI\]       ld1.c.clr.acq.nta r120=\[r20\]
    1bd6:	80 07 52 80 22 00 	            ld1.c.clr.acq.d4 r120=\[r20\]
    1bdc:	00 00 04 00       	            nop.i 0x0
    1be0:	08 c0 03 29 42 11 	\[MMI\]       ld1.c.clr.acq.d5 r120=\[r20\]
    1be6:	80 07 52 88 22 00 	            ld1.c.clr.acq.d6 r120=\[r20\]
    1bec:	00 00 04 00       	            nop.i 0x0
    1bf0:	08 c0 03 29 46 11 	\[MMI\]       ld1.c.clr.acq.d7 r120=\[r20\]
    1bf6:	80 07 50 90 22 00 	            ld2.c.clr.acq r120=\[r20\]
    1bfc:	00 00 04 00       	            nop.i 0x0
    1c00:	08 c0 03 28 4a 11 	\[MMI\]       ld2.c.clr.acq.nt1 r120=\[r20\]
    1c06:	80 07 50 94 22 00 	            ld2.c.clr.acq.nt1 r120=\[r20\]
    1c0c:	00 00 04 00       	            nop.i 0x0
    1c10:	08 c0 03 28 4c 11 	\[MMI\]       ld2.c.clr.acq.d2 r120=\[r20\]
    1c16:	80 07 50 98 22 00 	            ld2.c.clr.acq.d2 r120=\[r20\]
    1c1c:	00 00 04 00       	            nop.i 0x0
    1c20:	08 c0 03 28 4e 11 	\[MMI\]       ld2.c.clr.acq.nta r120=\[r20\]
    1c26:	80 07 52 90 22 00 	            ld2.c.clr.acq.d4 r120=\[r20\]
    1c2c:	00 00 04 00       	            nop.i 0x0
    1c30:	08 c0 03 29 4a 11 	\[MMI\]       ld2.c.clr.acq.d5 r120=\[r20\]
    1c36:	80 07 52 98 22 00 	            ld2.c.clr.acq.d6 r120=\[r20\]
    1c3c:	00 00 04 00       	            nop.i 0x0
    1c40:	08 c0 03 29 4e 11 	\[MMI\]       ld2.c.clr.acq.d7 r120=\[r20\]
    1c46:	80 07 50 a0 22 00 	            ld4.c.clr.acq r120=\[r20\]
    1c4c:	00 00 04 00       	            nop.i 0x0
    1c50:	08 c0 03 28 52 11 	\[MMI\]       ld4.c.clr.acq.nt1 r120=\[r20\]
    1c56:	80 07 50 a4 22 00 	            ld4.c.clr.acq.nt1 r120=\[r20\]
    1c5c:	00 00 04 00       	            nop.i 0x0
    1c60:	08 c0 03 28 54 11 	\[MMI\]       ld4.c.clr.acq.d2 r120=\[r20\]
    1c66:	80 07 50 a8 22 00 	            ld4.c.clr.acq.d2 r120=\[r20\]
    1c6c:	00 00 04 00       	            nop.i 0x0
    1c70:	08 c0 03 28 56 11 	\[MMI\]       ld4.c.clr.acq.nta r120=\[r20\]
    1c76:	80 07 50 ac 22 00 	            ld4.c.clr.acq.nta r120=\[r20\]
    1c7c:	00 00 04 00       	            nop.i 0x0
    1c80:	08 c0 03 29 50 11 	\[MMI\]       ld4.c.clr.acq.d4 r120=\[r20\]
    1c86:	80 07 52 a4 22 00 	            ld4.c.clr.acq.d5 r120=\[r20\]
    1c8c:	00 00 04 00       	            nop.i 0x0
    1c90:	08 c0 03 29 54 11 	\[MMI\]       ld4.c.clr.acq.d6 r120=\[r20\]
    1c96:	80 07 52 ac 22 00 	            ld4.c.clr.acq.d7 r120=\[r20\]
    1c9c:	00 00 04 00       	            nop.i 0x0
    1ca0:	08 c0 03 28 58 11 	\[MMI\]       ld8.c.clr.acq r120=\[r20\]
    1ca6:	80 07 50 b4 22 00 	            ld8.c.clr.acq.nt1 r120=\[r20\]
    1cac:	00 00 04 00       	            nop.i 0x0
    1cb0:	08 c0 03 28 5a 11 	\[MMI\]       ld8.c.clr.acq.nt1 r120=\[r20\]
    1cb6:	80 07 50 b8 22 00 	            ld8.c.clr.acq.d2 r120=\[r20\]
    1cbc:	00 00 04 00       	            nop.i 0x0
    1cc0:	08 c0 03 28 5c 11 	\[MMI\]       ld8.c.clr.acq.d2 r120=\[r20\]
    1cc6:	80 07 50 bc 22 00 	            ld8.c.clr.acq.nta r120=\[r20\]
    1ccc:	00 00 04 00       	            nop.i 0x0
    1cd0:	08 c0 03 28 5e 11 	\[MMI\]       ld8.c.clr.acq.nta r120=\[r20\]
    1cd6:	80 07 52 b0 22 00 	            ld8.c.clr.acq.d4 r120=\[r20\]
    1cdc:	00 00 04 00       	            nop.i 0x0
    1ce0:	08 c0 03 29 5a 11 	\[MMI\]       ld8.c.clr.acq.d5 r120=\[r20\]
    1ce6:	80 07 52 b8 22 00 	            ld8.c.clr.acq.d6 r120=\[r20\]
    1cec:	00 00 04 00       	            nop.i 0x0
    1cf0:	08 c0 03 29 5e 11 	\[MMI\]       ld8.c.clr.acq.d7 r120=\[r20\]
    1cf6:	80 07 50 82 22 00 	            ld16 r120,ar.csd=\[r20\]
    1cfc:	00 00 04 00       	            nop.i 0x0
    1d00:	08 c0 03 28 41 11 	\[MMI\]       ld16 r120,ar.csd=\[r20\]
    1d06:	80 07 50 86 22 00 	            ld16.nt1 r120,ar.csd=\[r20\]
    1d0c:	00 00 04 00       	            nop.i 0x0
    1d10:	08 c0 03 28 43 11 	\[MMI\]       ld16.nt1 r120,ar.csd=\[r20\]
    1d16:	80 07 50 8a 22 00 	            ld16.d2 r120,ar.csd=\[r20\]
    1d1c:	00 00 04 00       	            nop.i 0x0
    1d20:	08 c0 03 28 45 11 	\[MMI\]       ld16.d2 r120,ar.csd=\[r20\]
    1d26:	80 07 50 86 22 00 	            ld16.nt1 r120,ar.csd=\[r20\]
    1d2c:	00 00 04 00       	            nop.i 0x0
    1d30:	08 c0 03 28 43 11 	\[MMI\]       ld16.nt1 r120,ar.csd=\[r20\]
    1d36:	80 07 50 8a 22 00 	            ld16.d2 r120,ar.csd=\[r20\]
    1d3c:	00 00 04 00       	            nop.i 0x0
    1d40:	08 c0 03 28 45 11 	\[MMI\]       ld16.d2 r120,ar.csd=\[r20\]
    1d46:	80 07 50 8e 22 00 	            ld16.nta r120,ar.csd=\[r20\]
    1d4c:	00 00 04 00       	            nop.i 0x0
    1d50:	08 c0 03 28 47 11 	\[MMI\]       ld16.nta r120,ar.csd=\[r20\]
    1d56:	80 07 52 82 22 00 	            ld16.d4 r120,ar.csd=\[r20\]
    1d5c:	00 00 04 00       	            nop.i 0x0
    1d60:	08 c0 03 29 43 11 	\[MMI\]       ld16.d5 r120,ar.csd=\[r20\]
    1d66:	80 07 52 8a 22 00 	            ld16.d6 r120,ar.csd=\[r20\]
    1d6c:	00 00 04 00       	            nop.i 0x0
    1d70:	08 c0 03 29 47 11 	\[MMI\]       ld16.d7 r120,ar.csd=\[r20\]
    1d76:	80 07 50 8e 22 00 	            ld16.nta r120,ar.csd=\[r20\]
    1d7c:	00 00 04 00       	            nop.i 0x0
    1d80:	08 c0 03 28 47 11 	\[MMI\]       ld16.nta r120,ar.csd=\[r20\]
    1d86:	80 07 52 82 22 00 	            ld16.d4 r120,ar.csd=\[r20\]
    1d8c:	00 00 04 00       	            nop.i 0x0
    1d90:	08 c0 03 29 43 11 	\[MMI\]       ld16.d5 r120,ar.csd=\[r20\]
    1d96:	80 07 52 8a 22 00 	            ld16.d6 r120,ar.csd=\[r20\]
    1d9c:	00 00 04 00       	            nop.i 0x0
    1da0:	08 c0 03 29 47 11 	\[MMI\]       ld16.d7 r120,ar.csd=\[r20\]
    1da6:	80 07 50 c2 22 00 	            ld16.acq r120,ar.csd=\[r20\]
    1dac:	00 00 04 00       	            nop.i 0x0
    1db0:	08 c0 03 28 61 11 	\[MMI\]       ld16.acq r120,ar.csd=\[r20\]
    1db6:	80 07 50 c6 22 00 	            ld16.acq.nt1 r120,ar.csd=\[r20\]
    1dbc:	00 00 04 00       	            nop.i 0x0
    1dc0:	08 c0 03 28 63 11 	\[MMI\]       ld16.acq.nt1 r120,ar.csd=\[r20\]
    1dc6:	80 07 50 ca 22 00 	            ld16.acq.d2 r120,ar.csd=\[r20\]
    1dcc:	00 00 04 00       	            nop.i 0x0
    1dd0:	08 c0 03 28 65 11 	\[MMI\]       ld16.acq.d2 r120,ar.csd=\[r20\]
    1dd6:	80 07 50 c6 22 00 	            ld16.acq.nt1 r120,ar.csd=\[r20\]
    1ddc:	00 00 04 00       	            nop.i 0x0
    1de0:	08 c0 03 28 63 11 	\[MMI\]       ld16.acq.nt1 r120,ar.csd=\[r20\]
    1de6:	80 07 50 ca 22 00 	            ld16.acq.d2 r120,ar.csd=\[r20\]
    1dec:	00 00 04 00       	            nop.i 0x0
    1df0:	08 c0 03 28 65 11 	\[MMI\]       ld16.acq.d2 r120,ar.csd=\[r20\]
    1df6:	80 07 50 ce 22 00 	            ld16.acq.nta r120,ar.csd=\[r20\]
    1dfc:	00 00 04 00       	            nop.i 0x0
    1e00:	08 c0 03 28 67 11 	\[MMI\]       ld16.acq.nta r120,ar.csd=\[r20\]
    1e06:	80 07 52 c2 22 00 	            ld16.acq.d4 r120,ar.csd=\[r20\]
    1e0c:	00 00 04 00       	            nop.i 0x0
    1e10:	08 c0 03 29 63 11 	\[MMI\]       ld16.acq.d5 r120,ar.csd=\[r20\]
    1e16:	80 07 52 ca 22 00 	            ld16.acq.d6 r120,ar.csd=\[r20\]
    1e1c:	00 00 04 00       	            nop.i 0x0
    1e20:	08 c0 03 29 67 11 	\[MMI\]       ld16.acq.d7 r120,ar.csd=\[r20\]
    1e26:	80 07 50 ce 22 00 	            ld16.acq.nta r120,ar.csd=\[r20\]
    1e2c:	00 00 04 00       	            nop.i 0x0
    1e30:	08 c0 03 28 67 11 	\[MMI\]       ld16.acq.nta r120,ar.csd=\[r20\]
    1e36:	80 07 52 c2 22 00 	            ld16.acq.d4 r120,ar.csd=\[r20\]
    1e3c:	00 00 04 00       	            nop.i 0x0
    1e40:	08 c0 03 29 63 11 	\[MMI\]       ld16.acq.d5 r120,ar.csd=\[r20\]
    1e46:	80 07 52 ca 22 00 	            ld16.acq.d6 r120,ar.csd=\[r20\]
    1e4c:	00 00 04 00       	            nop.i 0x0
    1e50:	09 c0 03 29 67 11 	\[MMI\]       ld16.acq.d7 r120,ar.csd=\[r20\]
    1e56:	80 07 50 30 20 00 	            ld8 r120=\[r20\]
    1e5c:	00 00 04 00       	            nop.i 0x0;;
