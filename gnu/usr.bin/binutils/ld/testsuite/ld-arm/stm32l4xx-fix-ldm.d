
.*:     file format elf32-littlearm.*


Disassembly of section \.text:

00008000 <__stm32l4xx_veneer_0>:
    8000:	e8b9 007e 	ldmia\.w	r9!, {r1, r2, r3, r4, r5, r6}
    8004:	e899 0380 	ldmia\.w	r9, {r7, r8, r9}
    8008:	f000 b88c 	b\.w	8124 <__stm32l4xx_veneer_0_r>
    800c:	f7f0 a000 	udf\.w	#0

00008010 <__stm32l4xx_veneer_1>:
    8010:	e8b9 007e 	ldmia\.w	r9!, {r1, r2, r3, r4, r5, r6}
    8014:	e899 8380 	ldmia\.w	r9, {r7, r8, r9, pc}
    8018:	f7f0 a000 	udf\.w	#0
    801c:	f7f0 a000 	udf\.w	#0

00008020 <__stm32l4xx_veneer_2>:
    8020:	4607      	mov	r7, r0
    8022:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    8026:	e897 0380 	ldmia\.w	r7, {r7, r8, r9}
    802a:	f000 b87f 	b\.w	812c <__stm32l4xx_veneer_2_r>
    802e:	de00      	udf	#0

00008030 <__stm32l4xx_veneer_3>:
    8030:	460f      	mov	r7, r1
    8032:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    8036:	e897 0380 	ldmia\.w	r7, {r7, r8, r9}
    803a:	f000 b879 	b\.w	8130 <__stm32l4xx_veneer_3_r>
    803e:	de00      	udf	#0

00008040 <__stm32l4xx_veneer_4>:
    8040:	4607      	mov	r7, r0
    8042:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    8046:	e897 8380 	ldmia\.w	r7, {r7, r8, r9, pc}
    804a:	de00      	udf	#0
    804c:	f7f0 a000 	udf\.w	#0

00008050 <__stm32l4xx_veneer_5>:
    8050:	460f      	mov	r7, r1
    8052:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    8056:	e897 8380 	ldmia\.w	r7, {r7, r8, r9, pc}
    805a:	de00      	udf	#0
    805c:	f7f0 a000 	udf\.w	#0

00008060 <__stm32l4xx_veneer_6>:
    8060:	e8b0 007e 	ldmia\.w	r0!, {r1, r2, r3, r4, r5, r6}
    8064:	e8b0 0380 	ldmia\.w	r0!, {r7, r8, r9}
    8068:	f000 b868 	b\.w	813c <__stm32l4xx_veneer_6_r>
    806c:	f7f0 a000 	udf\.w	#0

00008070 <__stm32l4xx_veneer_7>:
    8070:	e8b0 007e 	ldmia\.w	r0!, {r1, r2, r3, r4, r5, r6}
    8074:	e8b0 8380 	ldmia\.w	r0!, {r7, r8, r9, pc}
    8078:	f7f0 a000 	udf\.w	#0
    807c:	f7f0 a000 	udf\.w	#0

00008080 <__stm32l4xx_veneer_8>:
    8080:	e931 0380 	ldmdb	r1!, {r7, r8, r9}
    8084:	e911 007e 	ldmdb	r1, {r1, r2, r3, r4, r5, r6}
    8088:	f000 b85c 	b\.w	8144 <__stm32l4xx_veneer_8_r>
    808c:	f7f0 a000 	udf\.w	#0

00008090 <__stm32l4xx_veneer_9>:
    8090:	4651      	mov	r1, sl
    8092:	e931 0380 	ldmdb	r1!, {r7, r8, r9}
    8096:	e911 007e 	ldmdb	r1, {r1, r2, r3, r4, r5, r6}
    809a:	f000 b855 	b\.w	8148 <__stm32l4xx_veneer_9_r>
    809e:	de00      	udf	#0

000080a0 <__stm32l4xx_veneer_a>:
    80a0:	4649      	mov	r1, r9
    80a2:	e931 0380 	ldmdb	r1!, {r7, r8, r9}
    80a6:	e911 007e 	ldmdb	r1, {r1, r2, r3, r4, r5, r6}
    80aa:	f000 b84f 	b\.w	814c <__stm32l4xx_veneer_a_r>
    80ae:	de00      	udf	#0

000080b0 <__stm32l4xx_veneer_b>:
    80b0:	f1a9 0928 	sub\.w	r9, r9, #40	@ 0x28
    80b4:	e8b9 007e 	ldmia\.w	r9!, {r1, r2, r3, r4, r5, r6}
    80b8:	e899 8380 	ldmia\.w	r9, {r7, r8, r9, pc}
    80bc:	f7f0 a000 	udf\.w	#0

000080c0 <__stm32l4xx_veneer_c>:
    80c0:	f1a1 0728 	sub\.w	r7, r1, #40	@ 0x28
    80c4:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    80c8:	e897 8380 	ldmia\.w	r7, {r7, r8, r9, pc}
    80cc:	f7f0 a000 	udf\.w	#0

000080d0 <__stm32l4xx_veneer_d>:
    80d0:	f1a0 0728 	sub\.w	r7, r0, #40	@ 0x28
    80d4:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    80d8:	e897 8380 	ldmia\.w	r7, {r7, r8, r9, pc}
    80dc:	f7f0 a000 	udf\.w	#0

000080e0 <__stm32l4xx_veneer_e>:
    80e0:	e930 0380 	ldmdb	r0!, {r7, r8, r9}
    80e4:	e930 007e 	ldmdb	r0!, {r1, r2, r3, r4, r5, r6}
    80e8:	f000 b838 	b\.w	815c <__stm32l4xx_veneer_e_r>
    80ec:	f7f0 a000 	udf\.w	#0

000080f0 <__stm32l4xx_veneer_f>:
    80f0:	f1a0 0028 	sub\.w	r0, r0, #40	@ 0x28
    80f4:	4607      	mov	r7, r0
    80f6:	e8b7 007e 	ldmia\.w	r7!, {r1, r2, r3, r4, r5, r6}
    80fa:	e897 8380 	ldmia\.w	r7, {r7, r8, r9, pc}
    80fe:	de00      	udf	#0

00008100 <__stm32l4xx_veneer_10>:
    8100:	e8bd 007f 	ldmia\.w	sp!, {r0, r1, r2, r3, r4, r5, r6}
    8104:	e8bd 0380 	ldmia\.w	sp!, {r7, r8, r9}
    8108:	f000 b82c 	b\.w	8164 <__stm32l4xx_veneer_10_r>
    810c:	f7f0 a000 	udf\.w	#0

00008110 <__stm32l4xx_veneer_11>:
    8110:	e8bd 007f 	ldmia\.w	sp!, {r0, r1, r2, r3, r4, r5, r6}
    8114:	e8bd 8380 	ldmia\.w	sp!, {r7, r8, r9, pc}
    8118:	f7f0 a000 	udf\.w	#0
    811c:	f7f0 a000 	udf\.w	#0

00008120 <_start>:
    8120:	f7ff bf6e 	b\.w	8000 <__stm32l4xx_veneer_0>

00008124 <__stm32l4xx_veneer_0_r>:
    8124:	f7ff bf74 	b\.w	8010 <__stm32l4xx_veneer_1>

00008128 <__stm32l4xx_veneer_1_r>:
    8128:	f7ff bf7a 	b\.w	8020 <__stm32l4xx_veneer_2>

0000812c <__stm32l4xx_veneer_2_r>:
    812c:	f7ff bf80 	b\.w	8030 <__stm32l4xx_veneer_3>

00008130 <__stm32l4xx_veneer_3_r>:
    8130:	f7ff bf86 	b\.w	8040 <__stm32l4xx_veneer_4>

00008134 <__stm32l4xx_veneer_4_r>:
    8134:	f7ff bf8c 	b\.w	8050 <__stm32l4xx_veneer_5>

00008138 <__stm32l4xx_veneer_5_r>:
    8138:	f7ff bf92 	b\.w	8060 <__stm32l4xx_veneer_6>

0000813c <__stm32l4xx_veneer_6_r>:
    813c:	f7ff bf98 	b\.w	8070 <__stm32l4xx_veneer_7>

00008140 <__stm32l4xx_veneer_7_r>:
    8140:	f7ff bf9e 	b\.w	8080 <__stm32l4xx_veneer_8>

00008144 <__stm32l4xx_veneer_8_r>:
    8144:	f7ff bfa4 	b\.w	8090 <__stm32l4xx_veneer_9>

00008148 <__stm32l4xx_veneer_9_r>:
    8148:	f7ff bfaa 	b\.w	80a0 <__stm32l4xx_veneer_a>

0000814c <__stm32l4xx_veneer_a_r>:
    814c:	f7ff bfb0 	b\.w	80b0 <__stm32l4xx_veneer_b>

00008150 <__stm32l4xx_veneer_b_r>:
    8150:	f7ff bfb6 	b\.w	80c0 <__stm32l4xx_veneer_c>

00008154 <__stm32l4xx_veneer_c_r>:
    8154:	f7ff bfbc 	b\.w	80d0 <__stm32l4xx_veneer_d>

00008158 <__stm32l4xx_veneer_d_r>:
    8158:	f7ff bfc2 	b\.w	80e0 <__stm32l4xx_veneer_e>

0000815c <__stm32l4xx_veneer_e_r>:
    815c:	f7ff bfc8 	b\.w	80f0 <__stm32l4xx_veneer_f>

00008160 <__stm32l4xx_veneer_f_r>:
    8160:	f7ff bfce 	b\.w	8100 <__stm32l4xx_veneer_10>

00008164 <__stm32l4xx_veneer_10_r>:
    8164:	f7ff bfd4 	b\.w	8110 <__stm32l4xx_veneer_11>
