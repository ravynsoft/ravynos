
.*:     file format elf32-littlearm.*


Disassembly of section \.text:

00008000 <__stm32l4xx_veneer_0>:
    8000:	ecf9 0a08 	vldmia	r9!, {s1-s8}
    8004:	ecf9 4a08 	vldmia	r9!, {s9-s16}
    8008:	ecf9 8a08 	vldmia	r9!, {s17-s24}
    800c:	ecf9 ca07 	vldmia	r9!, {s25-s31}
    8010:	f1a9 097c 	sub\.w	r9, r9, #124	@ 0x7c
    8014:	f000 b826 	b\.w	8064 <__stm32l4xx_veneer_0_r>

00008018 <__stm32l4xx_veneer_1>:
    8018:	ecf6 4a08 	vldmia	r6!, {s9-s16}
    801c:	ecf6 8a08 	vldmia	r6!, {s17-s24}
    8020:	ecf6 ca05 	vldmia	r6!, {s25-s29}
    8024:	f000 b820 	b\.w	8068 <__stm32l4xx_veneer_1_r>
    8028:	f7f0 a000 	udf\.w	#0
    802c:	f7f0 a000 	udf\.w	#0

00008030 <__stm32l4xx_veneer_2>:
    8030:	ecfd 0a08 	vpop	{s1-s8}
    8034:	ecfd 4a01 	vpop	{s9}
    8038:	f000 b818 	b\.w	806c <__stm32l4xx_veneer_2_r>
    803c:	f7f0 a000 	udf\.w	#0
    8040:	f7f0 a000 	udf\.w	#0
    8044:	f7f0 a000 	udf\.w	#0

00008048 <__stm32l4xx_veneer_3>:
    8048:	ed7b 0a08 	vldmdb	fp!, {s1-s8}
    804c:	ed7b 4a08 	vldmdb	fp!, {s9-s16}
    8050:	ed7b 8a08 	vldmdb	fp!, {s17-s24}
    8054:	ed7b ca07 	vldmdb	fp!, {s25-s31}
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
