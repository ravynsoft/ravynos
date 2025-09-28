
.*:     file format elf32-littlearm.*


Disassembly of section \.text:

00008000 <__stm32l4xx_veneer_0>:
    8000:	ecba 1b08 	vldmia	sl!, {d1-d4}
    8004:	ecba 5b08 	vldmia	sl!, {d5-d8}
    8008:	ecba 9b08 	vldmia	sl!, {d9-d12}
    800c:	ecba db06 	vldmia	sl!, {d13-d15}
    8010:	f1aa 0a78 	sub\.w	sl, sl, #120	@ 0x78
    8014:	f000 b826 	b\.w	8064 <__stm32l4xx_veneer_0_r>

00008018 <__stm32l4xx_veneer_1>:
    8018:	ecb7 5b08 	vldmia	r7!, {d5-d8}
    801c:	ecb7 9b08 	vldmia	r7!, {d9-d12}
    8020:	ecb7 db06 	vldmia	r7!, {d13-d15}
    8024:	f000 b820 	b\.w	8068 <__stm32l4xx_veneer_1_r>
    8028:	f7f0 a000 	udf\.w	#0
    802c:	f7f0 a000 	udf\.w	#0

00008030 <__stm32l4xx_veneer_2>:
    8030:	ecbd 1b08 	vpop	{d1-d4}
    8034:	ecbd 5b02 	vpop	{d5}
    8038:	f000 b818 	b\.w	806c <__stm32l4xx_veneer_2_r>
    803c:	f7f0 a000 	udf\.w	#0
    8040:	f7f0 a000 	udf\.w	#0
    8044:	f7f0 a000 	udf\.w	#0

00008048 <__stm32l4xx_veneer_3>:
    8048:	ed3c 1b08 	vldmdb	ip!, {d1-d4}
    804c:	ed3c 5b08 	vldmdb	ip!, {d5-d8}
    8050:	ed3c 9b08 	vldmdb	ip!, {d9-d12}
    8054:	ed3c db06 	vldmdb	ip!, {d13-d15}
    8058:	f000 b80a 	b\.w	8070 <__stm32l4xx_veneer_3_r>
    805c:	f7f0 a000 	udf\.w	#0

00008060 <_start>:
    8060:	f7ff bfce 	b\.w	8000 <__stm32l4xx_veneer_0>

00008064 <__stm32l4xx_veneer_0_r>:
    8064:	f7ff bfd8 	b\.w	8018 <__stm32l4xx_veneer_1>

00008068 <__stm32l4xx_veneer_1_r>:
    8068:	f7ff bfe2 	b\.w	8030 <__stm32l4xx_veneer_2>

0000806c <__stm32l4xx_veneer_2_r>:
    806c:	f7ff bfec 	b\.w	8048 <__stm32l4xx_veneer_3>
