
.*:     file format elf32-littlearm.*


Disassembly of section \.text:

00008000 <__stm32l4xx_veneer_0>:
    8000:	e899 01fe 	ldmia\.w	r9, {r1, r2, r3, r4, r5, r6, r7, r8}
    8004:	f000 b86e 	b\.w	80e4 <__stm32l4xx_veneer_0_r>
    8008:	f7f0 a000 	udf\.w	#0
    800c:	f7f0 a000 	udf\.w	#0

00008010 <__stm32l4xx_veneer_1>:
    8010:	e8b9 01fe 	ldmia\.w	r9!, {r1, r2, r3, r4, r5, r6, r7, r8}
    8014:	f000 b868 	b\.w	80e8 <__stm32l4xx_veneer_1_r>
    8018:	f7f0 a000 	udf\.w	#0
    801c:	f7f0 a000 	udf\.w	#0

00008020 <__stm32l4xx_veneer_2>:
    8020:	e919 01fe 	ldmdb	r9, {r1, r2, r3, r4, r5, r6, r7, r8}
    8024:	f000 b862 	b\.w	80ec <__stm32l4xx_veneer_2_r>
    8028:	f7f0 a000 	udf\.w	#0
    802c:	f7f0 a000 	udf\.w	#0

00008030 <__stm32l4xx_veneer_3>:
    8030:	e939 01fe 	ldmdb	r9!, {r1, r2, r3, r4, r5, r6, r7, r8}
    8034:	f000 b85c 	b\.w	80f0 <__stm32l4xx_veneer_3_r>
    8038:	f7f0 a000 	udf\.w	#0
    803c:	f7f0 a000 	udf\.w	#0

00008040 <__stm32l4xx_veneer_4>:
    8040:	e8bd 01fe 	ldmia\.w	sp!, {r1, r2, r3, r4, r5, r6, r7, r8}
    8044:	f000 b856 	b\.w	80f4 <__stm32l4xx_veneer_4_r>
    8048:	f7f0 a000 	udf\.w	#0
    804c:	f7f0 a000 	udf\.w	#0

00008050 <__stm32l4xx_veneer_5>:
    8050:	ecd9 0a08 	vldmia	r9, {s1-s8}
    8054:	f000 b850 	b\.w	80f8 <__stm32l4xx_veneer_5_r>
    8058:	f7f0 a000 	udf\.w	#0
    805c:	f7f0 a000 	udf\.w	#0
    8060:	f7f0 a000 	udf\.w	#0
    8064:	f7f0 a000 	udf\.w	#0

00008068 <__stm32l4xx_veneer_6>:
    8068:	ecf6 4a08 	vldmia	r6!, {s9-s16}
    806c:	f000 b846 	b\.w	80fc <__stm32l4xx_veneer_6_r>
    8070:	f7f0 a000 	udf\.w	#0
    8074:	f7f0 a000 	udf\.w	#0
    8078:	f7f0 a000 	udf\.w	#0
    807c:	f7f0 a000 	udf\.w	#0

00008080 <__stm32l4xx_veneer_7>:
    8080:	ecfd 0a08 	vpop	{s1-s8}
    8084:	f000 b83c 	b\.w	8100 <__stm32l4xx_veneer_7_r>
    8088:	f7f0 a000 	udf\.w	#0
    808c:	f7f0 a000 	udf\.w	#0
    8090:	f7f0 a000 	udf\.w	#0
    8094:	f7f0 a000 	udf\.w	#0

00008098 <__stm32l4xx_veneer_8>:
    8098:	ec99 1b08 	vldmia	r9, {d1-d4}
    809c:	f000 b832 	b\.w	8104 <__stm32l4xx_veneer_8_r>
    80a0:	f7f0 a000 	udf\.w	#0
    80a4:	f7f0 a000 	udf\.w	#0
    80a8:	f7f0 a000 	udf\.w	#0
    80ac:	f7f0 a000 	udf\.w	#0

000080b0 <__stm32l4xx_veneer_9>:
    80b0:	ecb6 8b08 	vldmia	r6!, {d8-d11}
    80b4:	f000 b828 	b\.w	8108 <__stm32l4xx_veneer_9_r>
    80b8:	f7f0 a000 	udf\.w	#0
    80bc:	f7f0 a000 	udf\.w	#0
    80c0:	f7f0 a000 	udf\.w	#0
    80c4:	f7f0 a000 	udf\.w	#0

000080c8 <__stm32l4xx_veneer_a>:
    80c8:	ecbd 1b08 	vpop	{d1-d4}
    80cc:	f000 b81e 	b\.w	810c <__stm32l4xx_veneer_a_r>
    80d0:	f7f0 a000 	udf\.w	#0
    80d4:	f7f0 a000 	udf\.w	#0
    80d8:	f7f0 a000 	udf\.w	#0
    80dc:	f7f0 a000 	udf\.w	#0

000080e0 <_start>:
    80e0:	f7ff bf8e 	b\.w	8000 <__stm32l4xx_veneer_0>

000080e4 <__stm32l4xx_veneer_0_r>:
    80e4:	f7ff bf94 	b\.w	8010 <__stm32l4xx_veneer_1>

000080e8 <__stm32l4xx_veneer_1_r>:
    80e8:	f7ff bf9a 	b\.w	8020 <__stm32l4xx_veneer_2>

000080ec <__stm32l4xx_veneer_2_r>:
    80ec:	f7ff bfa0 	b\.w	8030 <__stm32l4xx_veneer_3>

000080f0 <__stm32l4xx_veneer_3_r>:
    80f0:	f7ff bfa6 	b\.w	8040 <__stm32l4xx_veneer_4>

000080f4 <__stm32l4xx_veneer_4_r>:
    80f4:	f7ff bfac 	b\.w	8050 <__stm32l4xx_veneer_5>

000080f8 <__stm32l4xx_veneer_5_r>:
    80f8:	f7ff bfb6 	b\.w	8068 <__stm32l4xx_veneer_6>

000080fc <__stm32l4xx_veneer_6_r>:
    80fc:	f7ff bfc0 	b\.w	8080 <__stm32l4xx_veneer_7>

00008100 <__stm32l4xx_veneer_7_r>:
    8100:	f7ff bfca 	b\.w	8098 <__stm32l4xx_veneer_8>

00008104 <__stm32l4xx_veneer_8_r>:
    8104:	f7ff bfd4 	b\.w	80b0 <__stm32l4xx_veneer_9>

00008108 <__stm32l4xx_veneer_9_r>:
    8108:	f7ff bfde 	b\.w	80c8 <__stm32l4xx_veneer_a>
