
.*:     file format elf64-nfp


Disassembly of section \.text\.i32\.me0:

0000000000000000 <\.text\.i32\.me0>:
   0:	000540f0012cd000 	  \.0  immed\[gprA_0, 0x1234\]
   8:	0002c0f0012cd280 	  \.1  immed\[n\$reg_0, 0x1234\]
  10:	000220f0012cd281 	  \.2  immed\[n\$reg_1, 0x1234\]
  18:	000660f0012cd200 	  \.3  immed\[\*l\$index0, 0x1234\]
  20:	0007c0f0012cd220 	  \.4  immed\[\*l\$index1, 0x1234\]
  28:	000fa0f0012cd230 	  \.5  immed\[\*l\$index1\+\+, 0x1234\]
  30:	000f40f0012cd231 	  \.6  immed\[\*l\$index1--, 0x1234\]
  38:	0008c8f0012cd200 	  \.7  immed\[\*l\$index2, 0x1234\]
  40:	0000a8f0012cd210 	  \.8  immed\[\*l\$index2\+\+, 0x1234\]
  48:	000048f0012cd211 	  \.9  immed\[\*l\$index2--, 0x1234\]
  50:	000968f0012cd220 	 \.10  immed\[\*l\$index3, 0x1234\]
  58:	0007e0f0012cd203 	 \.11  immed\[\*l\$index0\[3\], 0x1234\]
  60:	000540f0012cd225 	 \.12  immed\[\*l\$index1\[5\], 0x1234\]
  68:	000b28f0012cd207 	 \.13  immed\[\*l\$index2\[7\], 0x1234\]
  70:	000de8f0012cd229 	 \.14  immed\[\*l\$index3\[9\], 0x1234\]
  78:	000000f00ff003ff 	 \.15  immed\[gprB_0, 0xffff\]
  80:	000d60f220000bff 	 \.16  immed_b1\[gprB_2, 0xff\]
  88:	000f60f6200007ff 	 \.17  immed_b3\[gprB_1, 0xff\]
  90:	000080f080000f00 	 \.18  immed\[gprB_3, 0xffffffff\]
  98:	000100f086600f77 	 \.19  immed\[gprB_3, 0xffff9988\]
  a0:	000940f0012cd180 	 \.20  immed\[\$xfer_0, 0x1234\]
  a8:	000a00f0043c8581 	 \.21  immed\[\$xfer_1, 0x4321\]
  b0:	000180f0056de1ce 	 \.22  immed\[\$xfer_30, 0x5678\]
  b8:	0007c0f0400e8401 	 \.23  immed_w0\[gprA_1, 0xa1\]
  c0:	000440f4400e8802 	 \.24  immed_w1\[gprA_2, 0xa2\]
  c8:	000d00f4000e8c03 	 \.25  immed\[gprA_3, 0xa3, <<16\]
  d0:	000520f001200334 	 \.26  immed\[gprB_0, 0x1234\]
  d8:	000fa0f0400007b1 	 \.27  immed_w0\[gprB_1, 0xb1\]
  e0:	000c20f440000bb2 	 \.28  immed_w1\[gprB_2, 0xb2\]
  e8:	000560f400000fb3 	 \.29  immed\[gprB_3, 0xb3, <<16\]
  f0:	000660f200000fb3 	 \.30  immed\[gprB_3, 0xb3, <<8\]
  f8:	0001b0f200000fb3 	 \.31  immed\[gprB_3, 0xb3, <<8\], predicate_cc
 100:	0001c2f200000fb3 	 \.32  immed\[gprB_3, 0xb3, <<8\], gpr_wrboth
 108:	000ba0a0300c2f00 	 \.33  alu\[--, --, B, 0xb\]
 110:	0005a081f200da00 	 \.34  alu_shf\[--, --, B, 0x16, <<1\]
 118:	000be081d2018600 	 \.35  alu_shf\[--, --, B, 0x21, <<3\]
 120:	000240801201b200 	 \.36  alu_shf\[--, --, B, 0x2c, <<31\]
 128:	000fa081f800da00 	 \.37  alu_shf\[\$xfer_0, --, B, 0x16, <<1\]
 130:	0009e081f840da00 	 \.38  alu_shf\[\$xfer_4, --, B, 0x16, <<1\]
 138:	00006081fc80da00 	 \.39  alu_shf\[\$xfer_24, --, B, 0x16, <<1\]
 140:	000a2081fcf0da00 	 \.40  alu_shf\[\$xfer_31, --, B, 0x16, <<1\]
 148:	0004a0a0280c2f00 	 \.41  alu\[n\$reg_0, --, B, 0xb\]
 150:	0001e0a0281c2f00 	 \.42  alu\[n\$reg_1, --, B, 0xb\]
 158:	000880a0a00c2400 	 \.43  alu\[\*l\$index0, gprA_0, \+, 0x9\]
 160:	000100a0a43c2400 	 \.44  alu\[\*n\$index\+\+, gprA_0, \+, 0x9\]
 168:	000b208bc500a600 	 \.45  alu_shf\[\*l\$index0, gprA_0, OR, 0x9, <<4\]
 170:	000b00a0a20c2400 	 \.46  alu\[\*l\$index1, gprA_0, \+, 0x9\]
 178:	000740a0a30c2400 	 \.47  alu\[\*l\$index1\+\+, gprA_0, \+, 0x9\]
 180:	000200a0a31c2400 	 \.48  alu\[\*l\$index1--, gprA_0, \+, 0x9\]
 188:	000628a0a00c2400 	 \.49  alu\[\*l\$index2, gprA_0, \+, 0x9\]
 190:	000988aa210c2400 	 \.50  alu\[\*l\$index2\+\+, gprA_0, OR, 0x9\]
 198:	000f28a0a11c2400 	 \.51  alu\[\*l\$index2--, gprA_0, \+, 0x9\]
 1a0:	0005a8a0a20c2400 	 \.52  alu\[\*l\$index3, gprA_0, \+, 0x9\]
 1a8:	000480a0a03c2400 	 \.53  alu\[\*l\$index0\[3\], gprA_0, \+, 0x9\]
 1b0:	000800a0a25c2400 	 \.54  alu\[\*l\$index1\[5\], gprA_0, \+, 0x9\]
 1b8:	000c68a0a07c2400 	 \.55  alu\[\*l\$index2\[7\], gprA_0, \+, 0x9\]
 1c0:	000aa8a0a29c2400 	 \.56  alu\[\*l\$index3\[9\], gprA_0, \+, 0x9\]
 1c8:	000cc4b0c008a400 	 \.57  alu\[gprB_0, \*l\$index3\[9\], \+, gprA_0\]
 1d0:	000fe4b0c008c000 	 \.58  alu\[gprB_0, \*l\$index3\+\+, \+, gprA_0\]
 1d8:	000ac4b0c008c400 	 \.59  alu\[gprB_0, \*l\$index3--, \+, gprA_0\]
 1e0:	000bc4b080000229 	 \.60  alu\[gprB_0, \*l\$index3\[9\], \+, gprB_0\]
 1e8:	000724b080000230 	 \.61  alu\[gprB_0, \*l\$index3\+\+, \+, gprB_0\]
 1f0:	0007c4b080000231 	 \.62  alu\[gprB_0, \*l\$index3--, \+, gprB_0\]
 1f8:	000664b080000211 	 \.63  alu\[gprB_0, \*l\$index2--, \+, gprB_0\]
 200:	000a60b080000231 	 \.64  alu\[gprB_0, \*l\$index1--, \+, gprB_0\]
 208:	000bc0b080000211 	 \.65  alu\[gprB_0, \*l\$index0--, \+, gprB_0\]
 210:	000340b080000200 	 \.66  alu\[gprB_0, \*l\$index0, \+, gprB_0\]
 218:	000ee4b080000200 	 \.67  alu\[gprB_0, \*l\$index2, \+, gprB_0\]
 220:	000100b080000241 	 \.68  alu\[gprB_0, \*n\$index, \+, gprB_0\]
 228:	0004809bf0000241 	 \.69  alu_shf\[gprB_0, \*n\$index, OR, gprB_0, <<1\]
 230:	000f20a0001fff00 	 \.70  alu\[gprA_1, --, B, 0xff\]
 238:	0005c0b0002fff00 	 \.71  alu\[gprB_2, --, B, 0xff\]
 240:	000940a0000d6f00 	 \.72  alu\[gprA_0, --, B, 0x5b\]
 248:	000440a2000d6f00 	 \.73  alu\[gprA_0, --, ~B, 0x5b\]
 250:	000de081f032f200 	 \.74  alu_shf\[gprA_3, --, B, 0x5c, <<1\]
 258:	000de091d012f600 	 \.75  alu_shf\[gprB_1, --, B, 0x5d, <<3\]
 260:	000d60901022fa00 	 \.76  alu_shf\[gprB_2, --, B, 0x5e, <<31\]
 268:	000e40a0c0000402 	 \.77  alu\[gprA_0, gprB_1, \+, gprA_2\]
 270:	000340a2c0000402 	 \.78  alu\[gprA_0, gprB_1, \+16, gprA_2\]
 278:	000040a4c0000402 	 \.79  alu\[gprA_0, gprB_1, \+8, gprA_2\]
 280:	0007a0a8c0000402 	 \.80  alu\[gprA_0, gprB_1, \+carry, gprA_2\]
 288:	000d40a6c0000402 	 \.81  alu\[gprA_0, gprB_1, -carry, gprA_2\]
 290:	000aa0aac0000402 	 \.82  alu\[gprA_0, gprB_1, -, gprA_2\]
 298:	0009a0acc0000402 	 \.83  alu\[gprA_0, gprB_1, B-A, gprA_2\]
 2a0:	000da0aa40000402 	 \.84  alu\[gprA_0, gprB_1, OR, gprA_2\]
 2a8:	000740a440000402 	 \.85  alu\[gprA_0, gprB_1, AND, gprA_2\]
 2b0:	000a40a640000402 	 \.86  alu\[gprA_0, gprB_1, ~AND, gprA_2\]
 2b8:	0000a0a840000402 	 \.87  alu\[gprA_0, gprB_1, AND~, gprA_2\]
 2c0:	000ea0ac40000402 	 \.88  alu\[gprA_0, gprB_1, XOR, gprA_2\]
 2c8:	000321a0c0000402 	 \.89  alu\[gprA_0, gprB_1, \+, gprA_2\], no_cc
 2d0:	000990a0c0000402 	 \.90  alu\[gprA_0, gprB_1, \+, gprA_2\], predicate_cc
 2d8:	0009e2a0c0000402 	 \.91  alu\[gprA_0, gprB_1, \+, gprA_2\], gpr_wrboth
 2e0:	000353a0c0000402 	 \.92  alu\[gprA_0, gprB_1, \+, gprA_2\], no_cc, gpr_wrboth, predicate_cc
 2e8:	000d418b70080602 	 \.93  alu_shf\[gprA_0, gprB_1, OR, gprA_2, <<9\], no_cc
 2f0:	0006708a90080502 	 \.94  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>9\], predicate_cc
 2f8:	000ea28a90080402 	 \.95  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>rot9\], gpr_wrboth
 300:	000e138b70080402 	 \.96  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>rot23\], no_cc, gpr_wrboth, predicate_cc
 308:	000ba08a00080602 	 \.97  alu_shf\[gprA_0, gprB_1, OR, gprA_2, <<indirect\]
 310:	0000208a00080502 	 \.98  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>indirect\]
 318:	000ba0a0300c2f00 	 \.99  alu\[--, --, B, 0xb\]
 320:	000ae09d40380101 	\.100  asr\[gprB_3, gprA_1, >>20\]
 328:	000ba0a0300c2f00 	\.101  alu\[--, --, B, 0xb\]
 330:	000ea09d40310500 	\.102  asr\[gprB_3, \*n\$index, >>20\]
 338:	000ba0a0300c2f00 	\.103  alu\[--, --, B, 0xb\]
 340:	0007a09d40314100 	\.104  asr\[gprB_3, \*l\$index0, >>20\]
 348:	000ba0a0300c2f00 	\.105  alu\[--, --, B, 0xb\]
 350:	0000249d40316100 	\.106  asr\[gprB_3, \*l\$index3, >>20\]
 358:	000ba0a0300c2f00 	\.107  alu\[--, --, B, 0xb\]
 360:	000a049d40314100 	\.108  asr\[gprB_3, \*l\$index2, >>20\]
 368:	000ba0a0300c2f00 	\.109  alu\[--, --, B, 0xb\]
 370:	0004a08d45010d00 	\.110  asr\[\*l\$index0, \*n\$index\+\+, >>20\]
 378:	000ba0a0300c2f00 	\.111  alu\[--, --, B, 0xb\]
 380:	000ee08d45810d00 	\.112  asr\[\*l\$index1, \*n\$index\+\+, >>20\]
 388:	000ba0a0300c2f00 	\.113  alu\[--, --, B, 0xb\]
 390:	000a088d45010d00 	\.114  asr\[\*l\$index2, \*n\$index\+\+, >>20\]
 398:	000ba0a0300c2f00 	\.115  alu\[--, --, B, 0xb\]
 3a0:	0007819d40380101 	\.116  asr\[gprB_3, gprA_1, >>20\], no_cc
 3a8:	000ba0a0300c2f00 	\.117  alu\[--, --, B, 0xb\]
 3b0:	000d309d40380101 	\.118  asr\[gprB_3, gprA_1, >>20\], predicate_cc
 3b8:	000ba0a0300c2f00 	\.119  alu\[--, --, B, 0xb\]
 3c0:	000ba28d40380101 	\.120  asr\[gprA_3, gprA_1, >>20\], gpr_wrboth
 3c8:	0008c0d818c08120 	\.121  beq\[\.99\]
 3d0:	000d00d877c08120 	\.122  beq\[\.479\]
 3d8:	000440d877e08120 	\.123  beq\[\.479\], defer\[2\]
 3e0:	000000f0000c0300 	\.124  nop
 3e8:	000000f0000c0300 	\.125  nop
 3f0:	000540d877c08021 	\.126  bne\[\.479\]
 3f8:	0004c0d877c08022 	\.127  bmi\[\.479\]
 400:	000420d877c08023 	\.128  bpl\[\.479\]
 408:	0007c0d877c08024 	\.129  bcs\[\.479\]
 410:	0007c0d877c08024 	\.130  bcs\[\.479\]
 418:	000720d877c08025 	\.131  bcc\[\.479\]
 420:	000720d877c08025 	\.132  bcc\[\.479\]
 428:	0006a0d877c08026 	\.133  bvs\[\.479\]
 430:	000640d877c08027 	\.134  bvc\[\.479\]
 438:	0001c0d877c08028 	\.135  bge\[\.479\]
 440:	000120d877c08029 	\.136  blt\[\.479\]
 448:	000040d877c0802b 	\.137  bgt\[\.479\]
 450:	0000a0d877c0802a 	\.138  ble\[\.479\]
 458:	000c60d818c08038 	\.139  br\[\.99\]
 460:	000920d818d08038 	\.140  br\[\.99\], defer\[1\]
 468:	000000f0000c0300 	\.141  nop
 470:	000bc0d077c09000 	\.142  br_bclr\[gprA_0, 3, \.479\]
 478:	000980d077c0e004 	\.143  br_bclr\[gprA_4, 23, \.479\]
 480:	0002a0d077c0082c 	\.144  br_bclr\[gprB_2, 11, \.479\]
 488:	000300d077c02423 	\.145  br_bclr\[gprB_9, 2, \.479\]
 490:	000260d077c02421 	\.146  br_bclr\[gprB_9, 0, \.479\]
 498:	000280d077c02420 	\.147  br_bclr\[gprB_9, 31, \.479\]
 4a0:	000f00d077f02423 	\.148  br_bclr\[gprB_9, 2, \.479\], defer\[3\]
 4a8:	000000f0000c0300 	\.149  nop
 4b0:	000000f0000c0300 	\.150  nop
 4b8:	000000f0000c0300 	\.151  nop
 4c0:	000680d077c42c2b 	\.152  br_bset\[gprB_11, 10, \.479\]
 4c8:	0006e0d077c4ac0b 	\.153  br_bset\[gprA_11, 10, \.479\]
 4d0:	0002a0c877d81020 	\.154  br=byte\[gprB_4, 0, 0x0, \.479\], defer\[1\]
 4d8:	000000f0000c0300 	\.155  nop
 4e0:	000a60c877c81520 	\.156  br=byte\[gprB_5, 1, 0x0, \.479\]
 4e8:	0001e0c877c81620 	\.157  br=byte\[gprB_5, 2, 0x0, \.479\]
 4f0:	0001a4c877c94220 	\.158  br=byte\[\*l\$index2, 2, 0x0, \.479\]
 4f8:	000620c877c96220 	\.159  br=byte\[\*l\$index1, 2, 0x0, \.479\]
 500:	000540c877c81b20 	\.160  br=byte\[gprB_6, 3, 0x0, \.479\]
 508:	0000c0c877cc16ff 	\.161  br=byte\[gprB_5, 2, 0xff, \.479\]
 510:	000420c877c816a2 	\.162  br=byte\[gprB_5, 2, 0x42, \.479\]
 518:	000380c877c416ff 	\.163  br!=byte\[gprB_5, 2, 0xff, \.479\]
 520:	0002a0c877c01620 	\.164  br!=byte\[gprB_5, 2, 0x0, \.479\]
 528:	000c20d877c00236 	\.165  br_cls_state\[cls_ring0_status, \.479\]
 530:	0001a0d877e20236 	\.166  br_cls_state\[cls_ring8_status, \.479\], defer\[2\]
 538:	000000f0000c0300 	\.167  nop
 540:	000000f0000c0300 	\.168  nop
 548:	000be0d877c38236 	\.169  br_cls_state\[cls_ring14_status, \.479\]
 550:	0007c0d877c3c236 	\.170  br_cls_state\[cls_ring15_status, \.479\]
 558:	000720d877c3c237 	\.171  br_!cls_state\[cls_ring15_status, \.479\]
 560:	000cc0d877c00237 	\.172  br_!cls_state\[cls_ring0_status, \.479\]
 568:	000c00d877c00030 	\.173  br=ctx\[0, \.479\]
 570:	000dc0d877c08030 	\.174  br=ctx\[2, \.479\]
 578:	000f00d877c18030 	\.175  br=ctx\[6, \.479\]
 580:	000a40d877d18030 	\.176  br=ctx\[6, \.479\], defer\[1\]
 588:	000000f0000c0300 	\.177  nop
 590:	000d40d877c00234 	\.178  br_inp_state\[nn_empty, \.479\]
 598:	000160d877c04234 	\.179  br_inp_state\[nn_full, \.479\]
 5a0:	000c80d877c08234 	\.180  br_inp_state\[ctm_ring0_status, \.479\]
 5a8:	000100d877e28234 	\.181  br_inp_state\[ctm_ring8_status, \.479\], defer\[2\]
 5b0:	000000f0000c0300 	\.182  nop
 5b8:	000000f0000c0300 	\.183  nop
 5c0:	000a80d877c38234 	\.184  br_inp_state\[ctm_ring12_status, \.479\]
 5c8:	0006a0d877c3c234 	\.185  br_inp_state\[ctm_ring13_status, \.479\]
 5d0:	000640d877c3c235 	\.186  br_!inp_state\[ctm_ring13_status, \.479\]
 5d8:	000c60d877c08235 	\.187  br_!inp_state\[ctm_ring0_status, \.479\]
 5e0:	000260d877c04232 	\.188  br_signal\[1, \.479\]
 5e8:	000f80d877c08232 	\.189  br_signal\[2, \.479\]
 5f0:	0005a0d877c3c232 	\.190  br_signal\[15, \.479\]
 5f8:	000540d877c3c233 	\.191  br_!signal\[15, \.479\]
 600:	000b60d877f2c232 	\.192  br_signal\[11, \.479\], defer\[3\]
 608:	000000f0000c0300 	\.193  nop
 610:	000000f0000c0300 	\.194  nop
 618:	000000f0000c0300 	\.195  nop
 620:	000e40a0c0000402 	\.196  alu\[gprA_0, gprB_1, \+, gprA_2\]
 628:	0004408e02081200 	\.197  byte_align_le\[--, gprB_4\]
 630:	0008c08e00981200 	\.198  byte_align_le\[gprA_9, gprB_4\]
 638:	0004c08e00a81200 	\.199  byte_align_le\[gprA_10, gprB_4\]
 640:	0001808e00b81200 	\.200  byte_align_le\[gprA_11, gprB_4\]
 648:	000e40a0c0000402 	\.201  alu\[gprA_0, gprB_1, \+, gprA_2\]
 650:	000c808e02001100 	\.202  byte_align_be\[--, gprB_4\]
 658:	0000008e00901100 	\.203  byte_align_be\[gprA_9, gprB_4\]
 660:	000c008e00a01100 	\.204  byte_align_be\[gprA_10, gprB_4\]
 668:	0009408e00b01100 	\.205  byte_align_be\[gprA_11, gprB_4\]
 670:	000d80a0300c0300 	\.206  alu\[--, --, B, 0x0\]
 678:	000400a5b00c0000 	\.207  cam_clear
 680:	000360bb80900007 	\.208  cam_lookup\[gprB_9, gprA_7\]
 688:	0003a0bb80900200 	\.209  cam_lookup\[gprB_9, \*l\$index0\]
 690:	000e04bb80900200 	\.210  cam_lookup\[gprB_9, \*l\$index2\]
 698:	000f84bb80900203 	\.211  cam_lookup\[gprB_9, \*l\$index2\[3\]\]
 6a0:	000bc0bb80900210 	\.212  cam_lookup\[gprB_9, \*l\$index0\+\+\]
 6a8:	000280aba0000241 	\.213  cam_lookup\[\*l\$index0, \*n\$index\]
 6b0:	000ec0aba1000241 	\.214  cam_lookup\[\*l\$index0\+\+, \*n\$index\]
 6b8:	000288aba3000243 	\.215  cam_lookup\[\*l\$index3\+\+, \*n\$index\+\+\]
 6c0:	000aa0aba0200243 	\.216  cam_lookup\[\*l\$index0\[2\], \*n\$index\+\+\]
 6c8:	000060bb80901407 	\.217  cam_lookup\[gprB_9, gprA_7\], lm_addr0\[1\]
 6d0:	000060bb80902807 	\.218  cam_lookup\[gprB_9, gprA_7\], lm_addr1\[2\]
 6d8:	000660bb80907407 	\.219  cam_lookup\[gprB_9, gprA_7\], lm_addr2\[3\]
 6e0:	000660bb80904807 	\.220  cam_lookup\[gprB_9, gprA_7\], lm_addr3\[0\]
 6e8:	000222ab80900007 	\.221  cam_lookup\[gprA_9, gprA_7\], gpr_wrboth
 6f0:	0004b0bb80900007 	\.222  cam_lookup\[gprB_9, gprA_7\], predicate_cc
 6f8:	000a00a7809c0000 	\.223  cam_read_tag\[gprA_9, 0x0\]
 700:	000da2a7809c0000 	\.224  cam_read_tag\[gprA_9, 0x0\], gpr_wrboth
 708:	000dd0a7809c0000 	\.225  cam_read_tag\[gprA_9, 0x0\], predicate_cc
 710:	000900a7809c2800 	\.226  cam_read_tag\[gprA_9, 0xa\]
 718:	000a00a7809c3c00 	\.227  cam_read_tag\[gprA_9, 0xf\]
 720:	0003e0af809c0000 	\.228  cam_read_state\[gprA_9, 0x0\]
 728:	000442af809c0000 	\.229  cam_read_state\[gprA_9, 0x0\], gpr_wrboth
 730:	000392af809c0000 	\.230  cam_read_state\[gprA_9, 0x0\], gpr_wrboth, predicate_cc
 738:	0000e0af809c2800 	\.231  cam_read_state\[gprA_9, 0xa\]
 740:	0003e0af809c3c00 	\.232  cam_read_state\[gprA_9, 0xf\]
 748:	000920a9f0101700 	\.233  cam_write\[0x0, gprB_5, 1\]
 750:	000da0a9f01a0300 	\.234  cam_write\[0x0, n\$reg_0, 1\]
 758:	000e80a9f0190700 	\.235  cam_write\[0x0, \*n\$index, 1\]
 760:	0004c4a9f0180300 	\.236  cam_write\[0x0, \*l\$index2, 1\]
 768:	0008e4a9f0184300 	\.237  cam_write\[0x0, \*l\$index2\+\+, 1\]
 770:	000dc4a9f0184700 	\.238  cam_write\[0x0, \*l\$index2--, 1\]
 778:	000840a9f0b01704 	\.239  cam_write\[0x4, gprB_5, 11\]
 780:	000be0a9f0f0170f 	\.240  cam_write\[0xf, gprB_5, 15\]
 788:	0008a0adb01c0000 	\.241  cam_write_state\[0x0, 1\]
 790:	000d80adb0bc1000 	\.242  cam_write_state\[0x4, 11\]
 798:	000de0adb0fc3c00 	\.243  cam_write_state\[0xf, 15\]
 7a0:	0000c0fc142c000d 	\.244  local_csr_wr\[CRCRemainder, gprA_13\]
 7a8:	000d20a918060348 	\.245  crc_le\[crc_ccitt, \$xfer_0, \$xfer_0\]
 7b0:	000000f0000c0300 	\.246  nop
 7b8:	000d40a918160748 	\.247  crc_le\[crc_ccitt, \$xfer_1, \$xfer_1\]
 7c0:	000000f0000c0300 	\.248  nop
 7c8:	000d40a918260b48 	\.249  crc_le\[crc_ccitt, \$xfer_2, \$xfer_2\]
 7d0:	000000f0000c0300 	\.250  nop
 7d8:	000d20a918360f48 	\.251  crc_le\[crc_ccitt, \$xfer_3, \$xfer_3\]
 7e0:	000000f0000c0300 	\.252  nop
 7e8:	000000f0000c0300 	\.253  nop
 7f0:	000000f0000c0300 	\.254  nop
 7f8:	000000f0000c0300 	\.255  nop
 800:	000000f0000c0300 	\.256  nop
 808:	000f60fc140c0000 	\.257  local_csr_rd\[CRCRemainder\]
 810:	000ce0f0000c000e 	\.258  immed\[gprA_14, 0x0\]
 818:	000940a918060340 	\.259  crc_be\[crc_ccitt, \$xfer_0, \$xfer_0\]
 820:	000000f0000c0300 	\.260  nop
 828:	000920a918461340 	\.261  crc_be\[crc_ccitt, \$xfer_4, \$xfer_4\]
 830:	000000f0000c0300 	\.262  nop
 838:	000060a900061340 	\.263  crc_be\[crc_ccitt, gprA_0, \$xfer_4\]
 840:	000000f0000c0300 	\.264  nop
 848:	000c60a900001340 	\.265  crc_be\[crc_ccitt, gprA_0, gprB_4\]
 850:	000000f0000c0300 	\.266  nop
 858:	000000f0000c0300 	\.267  nop
 860:	000000f0000c0300 	\.268  nop
 868:	000000f0000c0300 	\.269  nop
 870:	000000f0000c0300 	\.270  nop
 878:	000600a918260380 	\.271  crc_be\[crc_32, \$xfer_2, \$xfer_0\]
 880:	000000f0000c0300 	\.272  nop
 888:	0004c0a9183613a0 	\.273  crc_be\[crc_iscsi, \$xfer_3, \$xfer_4\]
 890:	000000f0000c0300 	\.274  nop
 898:	0004c0a9000613c0 	\.275  crc_be\[crc_10, gprA_0, \$xfer_4\]
 8a0:	000000f0000c0300 	\.276  nop
 8a8:	000960a9000013e0 	\.277  crc_be\[crc_5, gprA_0, gprB_4\]
 8b0:	000000f0000c0300 	\.278  nop
 8b8:	000ea0a918862700 	\.279  crc_be\[--, \$xfer_8, \$xfer_9\]
 8c0:	000000f0000c0300 	\.280  nop
 8c8:	000240a918760784 	\.281  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_0_2
 8d0:	000000f0000c0300 	\.282  nop
 8d8:	0002a0a918760785 	\.283  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_0_1
 8e0:	000000f0000c0300 	\.284  nop
 8e8:	000320a918760786 	\.285  crc_be\[crc_32, \$xfer_7, \$xfer_1\], byte_0
 8f0:	000000f0000c0300 	\.286  nop
 8f8:	0000c0a918760781 	\.287  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_1_3
 900:	000000f0000c0300 	\.288  nop
 908:	000140a918760782 	\.289  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_2_3
 910:	000000f0000c0300 	\.290  nop
 918:	0001a0a918760783 	\.291  crc_be\[crc_32, \$xfer_7, \$xfer_1\], byte_3
 920:	000000f0000c0300 	\.292  nop
 928:	000782a900160780 	\.293  crc_be\[crc_32, gprA_1, \$xfer_1\], gpr_wrboth
 930:	000000f0000c0300 	\.294  nop
 938:	000ae3a900160780 	\.295  crc_be\[crc_32, gprA_1, \$xfer_1\], no_cc, gpr_wrboth
 940:	000000f0000c0300 	\.296  nop
 948:	000b73a900560780 	\.297  crc_be\[crc_32, gprA_5, \$xfer_1\], no_cc, gpr_wrboth, predicate_cc
 950:	000000f0000c0300 	\.298  nop
 958:	000122a900560781 	\.299  crc_be\[crc_32, gprA_5, \$xfer_1\], bytes_1_3, gpr_wrboth
 960:	000000f0000c0300 	\.300  nop
 968:	000000f0000c0300 	\.301  nop
 970:	000000f0000c0300 	\.302  nop
 978:	000000f0000c0300 	\.303  nop
 980:	000000f0000c0300 	\.304  nop
 988:	000000f0000c0300 	\.305  nop
 990:	0005a0e000080000 	\.306  ctx_arb\[--\]
 998:	000600e000000001 	\.307  ctx_arb\[voluntary\]
 9a0:	000220e000020000 	\.308  ctx_arb\[bpt\]
 9a8:	000460e000000220 	\.309  ctx_arb\[sig5, sig9\]
 9b0:	000d20e000200220 	\.310  ctx_arb\[sig5, sig9\], defer\[2\]
 9b8:	000180a0300c0f00 	\.311  alu\[--, --, B, 0x3\]
 9c0:	0007a0a0300c1f00 	\.312  alu\[--, --, B, 0x7\]
 9c8:	0006a0e000010220 	\.313  ctx_arb\[sig5, sig9\], any
 9d0:	000a60e077c40220 	\.314  ctx_arb\[sig5, sig9\], br\[\.479\]
 9d8:	0006409010500701 	\.315  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\]
 9e0:	000d4090a0500701 	\.316  dbl_shf\[gprB_5, gprA_1, gprB_1, >>10\]
 9e8:	000c4091f0500701 	\.317  dbl_shf\[gprB_5, gprA_1, gprB_1, >>31\]
 9f0:	000740a440000402 	\.318  alu\[gprA_0, gprB_1, AND, gprA_2\]
 9f8:	0000c09000500701 	\.319  dbl_shf\[gprB_5, gprA_1, gprB_1, >>indirect\]
 a00:	000b219010500701 	\.320  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\], no_cc
 a08:	000cf19010500701 	\.321  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\], no_cc, predicate_cc
 a10:	0000d28010500701 	\.322  dbl_shf\[gprA_5, gprA_1, gprB_1, >>1\], gpr_wrboth, predicate_cc
 a18:	000200a700f03f00 	\.323  ffs\[gprA_15, gprB_15\]
 a20:	000fe0b740fc000f 	\.324  ffs\[gprB_15, gprA_15\]
 a28:	000ec0b700f61300 	\.325  ffs\[gprB_15, \$xfer_4\]
 a30:	000660b700f88300 	\.326  ffs\[gprB_15, \*l\$index1\]
 a38:	0007e4b700f8c300 	\.327  ffs\[gprB_15, \*l\$index3\+\+\]
 a40:	0002c4b700f8c700 	\.328  ffs\[gprB_15, \*l\$index3--\]
 a48:	0004c4b700f8a700 	\.329  ffs\[gprB_15, \*l\$index3\[9\]\]
 a50:	000880a720000300 	\.330  ffs\[\*l\$index0, gprB_0\]
 a58:	000108a722090700 	\.331  ffs\[\*l\$index3, \*n\$index\]
 a60:	000128a723190f00 	\.332  ffs\[\*l\$index3--, \*n\$index\+\+\]
 a68:	0003c3a740fc000f 	\.333  ffs\[gprA_15, gprA_15\], no_cc, gpr_wrboth
 a70:	000972a740fc000f 	\.334  ffs\[gprA_15, gprA_15\], gpr_wrboth, predicate_cc
 a78:	000320f0000c0803 	\.335  immed\[gprA_3, 0x2\]
 a80:	000480e8004d4803 	\.336  jump\[gprA_3, \.338\]
 a88:	0006a0d854408038 	\.337  br\[\.337\]
 a90:	000460f000002701 	\.338  immed\[gprB_9, 0x1\]
 a98:	0006a0d854408038 	\.339  br\[\.337\]
 aa0:	0005e0f000002702 	\.340  immed\[gprB_9, 0x2\]
 aa8:	0006a0d854408038 	\.341  br\[\.337\]
 ab0:	000500f000002703 	\.342  immed\[gprB_9, 0x3\]
 ab8:	0006a0d854408038 	\.343  br\[\.337\]
 ac0:	000040c001000000 	\.344  ld_field\[gprA_0, 0001, gprB_0\]
 ac8:	0007e2c001000000 	\.345  ld_field\[gprA_0, 0001, gprB_0\], gpr_wrboth
 ad0:	000e40c401000000 	\.346  ld_field\[gprA_0, 0001, gprB_0\], load_cc
 ad8:	000790c001000000 	\.347  ld_field\[gprA_0, 0001, gprB_0\], predicate_cc
 ae0:	0005c0c005000000 	\.348  ld_field\[gprA_0, 0101, gprB_0\]
 ae8:	000080c005100000 	\.349  ld_field_w_clr\[gprA_0, 0101, gprB_0\]
 af0:	0002a2c001100000 	\.350  ld_field_w_clr\[gprA_0, 0001, gprB_0\], gpr_wrboth
 af8:	000b00c401100000 	\.351  ld_field_w_clr\[gprA_0, 0001, gprB_0\], load_cc
 b00:	0002d0c001100000 	\.352  ld_field_w_clr\[gprA_0, 0001, gprB_0\], predicate_cc
 b08:	000fc0c00f000000 	\.353  ld_field\[gprA_0, 1111, gprB_0\]
 b10:	0005e0c1fb000200 	\.354  ld_field\[gprA_0, 1011, gprB_0, <<1\]
 b18:	000460c01b000100 	\.355  ld_field\[gprA_0, 1011, gprB_0, >>1\]
 b20:	000e60c1fb000100 	\.356  ld_field\[gprA_0, 1011, gprB_0, >>31\]
 b28:	000bc0c09b000000 	\.357  ld_field\[gprA_0, 1011, gprB_0, >>rot9\]
 b30:	000e80c09b100000 	\.358  ld_field_w_clr\[gprA_0, 1011, gprB_0, >>rot9\]
 b38:	0001c0c17b000000 	\.359  ld_field\[gprA_0, 1011, gprB_0, >>rot23\]
 b40:	0002c0c41b000000 	\.360  ld_field\[gprA_0, 1011, gprB_0, >>rot1\], load_cc
 b48:	000780c41b100000 	\.361  ld_field_w_clr\[gprA_0, 1011, gprB_0, >>rot1\], load_cc
 b50:	000400f0001f7c01 	\.362  immed\[gprA_1, 0x1df\]
 b58:	000200f0001007df 	\.363  immed\[gprB_1, 0x1df\]
 b60:	0005a2f0001007df 	\.364  immed\[gprB_1, 0x1df\], gpr_wrboth
 b68:	0005d0f0001007df 	\.365  immed\[gprB_1, 0x1df\], predicate_cc
 b70:	000020fc010c0000 	\.366  local_csr_rd\[ALUOut\]
 b78:	000e60f0000c000b 	\.367  immed\[gprA_11, 0x0\]
 b80:	000ce0fc160c0000 	\.368  local_csr_rd\[MiscControl\]
 b88:	000e60f0000c000b 	\.369  immed\[gprA_11, 0x0\]
 b90:	000ae0fc076c0b02 	\.370  local_csr_wr\[XferIndex, 0x2\]
 b98:	0008a0fc076c0003 	\.371  local_csr_wr\[XferIndex, gprA_3\]
 ba0:	000520fc07600f00 	\.372  local_csr_wr\[XferIndex, gprB_3\]
 ba8:	000f20fc01a00f00 	\.373  local_csr_wr\[CtxEnables, gprB_3\]
 bb0:	000480f800000c02 	\.374  mul_step\[gprA_2, gprB_3\], start
 bb8:	000880f980000c02 	\.375  mul_step\[gprA_2, gprB_3\], 32x32_step1
 bc0:	000dc0f980100c02 	\.376  mul_step\[gprA_2, gprB_3\], 32x32_step2
 bc8:	0001c0f980200c02 	\.377  mul_step\[gprA_2, gprB_3\], 32x32_step3
 bd0:	000480f980300c02 	\.378  mul_step\[gprA_2, gprB_3\], 32x32_step4
 bd8:	000940f9804c0002 	\.379  mul_step\[gprA_2, --\], 32x32_last
 be0:	000ce0f9805c0003 	\.380  mul_step\[gprA_3, --\], 32x32_last2
 be8:	0001a0f800000802 	\.381  mul_step\[gprA_2, gprB_2\], start
 bf0:	000aa0f900000802 	\.382  mul_step\[gprA_2, gprB_2\], 16x16_step1
 bf8:	000fe0f900100802 	\.383  mul_step\[gprA_2, gprB_2\], 16x16_step2
 c00:	000f20f9004c0000 	\.384  mul_step\[gprA_0, --\], 16x16_last
 c08:	0001a0f800000802 	\.385  mul_step\[gprA_2, gprB_2\], start
 c10:	0006a0f880000802 	\.386  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c18:	000320f8804c0000 	\.387  mul_step\[gprA_0, --\], 24x8_last
 c20:	0001a0f800000802 	\.388  mul_step\[gprA_2, gprB_2\], start
 c28:	0006a0f880000802 	\.389  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c30:	0004f0f8804c0000 	\.390  mul_step\[gprA_0, --\], 24x8_last, predicate_cc
 c38:	0001a0f800000802 	\.391  mul_step\[gprA_2, gprB_2\], start
 c40:	0006a0f880000802 	\.392  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c48:	0009e3f8804c0000 	\.393  mul_step\[gprA_0, --\], 24x8_last, no_cc, gpr_wrboth
 c50:	000b80a330000000 	\.394  pop_count1\[gprB_0\]
 c58:	000c80a3b0000000 	\.395  pop_count2\[gprB_0\]
 c60:	000d80a180000000 	\.396  pop_count3\[gprA_0, gprB_0\]
 c68:	000b80a330000000 	\.397  pop_count1\[gprB_0\]
 c70:	000c80a3b0000000 	\.398  pop_count2\[gprB_0\]
 c78:	000743a180000000 	\.399  pop_count3\[gprA_0, gprB_0\], no_cc, gpr_wrboth
 c80:	0004a4a330088000 	\.400  pop_count1\[\*l\$index3\]
 c88:	0003a4a3b0088000 	\.401  pop_count2\[\*l\$index3\]
 c90:	0000e5a1a438c000 	\.402  pop_count3\[\*n\$index\+\+, \*l\$index3\+\+\], no_cc
 c98:	000b80a330000000 	\.403  pop_count1\[gprB_0\]
 ca0:	000c80a3b0000000 	\.404  pop_count2\[gprB_0\]
 ca8:	000731a180000000 	\.405  pop_count3\[gprA_0, gprB_0\], no_cc, predicate_cc
 cb0:	000480e8000c0000 	\.406  rtn\[gprA_0\]
 cb8:	000620e8000a0700 	\.407  rtn\[n\$reg_1\]
 cc0:	000600e800088300 	\.408  rtn\[\*l\$index1\]
 cc8:	000a64e800080300 	\.409  rtn\[\*l\$index2\]
 cd0:	000dc0e800200300 	\.410  rtn\[gprB_0\], defer\[2\]
 cd8:	0008a0a0300c0700 	\.411  alu\[--, --, B, 0x1\]
 ce0:	0004a0a0300c0b00 	\.412  alu\[--, --, B, 0x2\]
 ce8:	000000f0000c0300 	\.413  nop
 cf0:	000000f0000c0300 	\.414  nop
 cf8:	000000f0000c0300 	\.415  nop
 d00:	000000f0000c0300 	\.416  nop
 d08:	0003501842300c09 	\.417  arm\[read, \$xfer_3, gprA_9, gprB_3, 2\], ctx_swap\[sig4\]
 d10:	0005501842302403 	\.418  arm\[read, \$xfer_3, gprA_3, gprB_9, 2\], ctx_swap\[sig4\]
 d18:	0004801842300c09 	\.419  arm\[read, \$xfer_3, gprA_9, <<8, gprB_3, 2\], ctx_swap\[sig4\]
 d20:	000f241842302403 	\.420  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], ctx_swap\[sig4\]
 d28:	0004a0a0300c0b00 	\.421  alu\[--, --, B, 0x2\]
 d30:	0008861842302403 	\.422  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], indirect_ref, ctx_swap\[sig4\]
 d38:	0004a0a0300c0b00 	\.423  alu\[--, --, B, 0x2\]
 d40:	000e8618e2302703 	\.424  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], indirect_ref, sig_done\[sig14\]
 d48:	0007841842302503 	\.425  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], ctx_swap\[sig4\], defer\[1\]
 d50:	0008a0a0300c0700 	\.426  alu\[--, --, B, 0x1\]
 d58:	000f101843c00c09 	\.427  arm\[read, \$xfer_28, gprA_9, gprB_3, 2\], ctx_swap\[sig4\]
 d60:	000910184e800c09 	\.428  arm\[read, \$xfer_8, gprA_9, gprB_3, 8\], ctx_swap\[sig4\]
 d68:	000a106440800c09 	\.429  cls\[add, \$xfer_8, gprA_9, gprB_3, 1\], ctx_swap\[sig4\]
 d70:	0000f0664080a009 	\.430  cls\[sub, \$xfer_8, gprA_9, 0x8, 1\], ctx_swap\[sig4\]
 d78:	000160644284a009 	\.431  cls\[add64, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 d80:	000404664284a408 	\.432  cls\[sub64, \$xfer_8, 0x9, <<8, gprA_8, 2\], ctx_swap\[sig4\]
 d88:	0008a0a0300c0700 	\.433  alu\[--, --, B, 0x1\]
 d90:	00032c650340a708 	\.434  cls\[add_imm, 0x14, 0x9, <<8, gprA_8, 2\]
 d98:	0007506040880c09 	\.435  cls\[swap/test_compare_write, \$xfer_8, gprA_9, gprB_3, 1\], ctx_swap\[sig4\]
 da0:	00023c6500007f9a 	\.436  cls\[add_imm, 0x1f9a, --, 1\]
 da8:	000038653c583f14 	\.437  cls\[add_imm, 0xf14, 0xf16\]
 db0:	000b54640013c30f 	\.438  cls\[add, \$xfer_1, 0xf00f, 1\]
 db8:	0002901c10a08000 	\.439  ct\[xpb_read, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dc0:	0007501e10a48000 	\.440  ct\[reflect_read_sig_init, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dc8:	000a501c10a48000 	\.441  ct\[ring_get, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dd0:	000000f0000c0300 	\.442  nop
 dd8:	000cc0474a80a009 	\.443  mem\[add64, \$xfer_8, gprA_9, <<8, 0x8, 6\], ctx_swap\[sig4\]
 de0:	000d40404280a009 	\.444  mem\[read, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 de8:	000c405c4280a009 	\.445  mem\[read32, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 df0:	000ea0554280a009 	\.446  mem\[ctm\.pe_dma_to_memory_indirect/emem\.get/imem\.lb_bucket_read_local, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 df8:	0009204c408ca309 	\.447  mem\[lock128/lock384, \$xfer_8, gprA_9, <<8, 0x8, 1\], sig_done\[sig4\]
 e00:	000f20e000000030 	\.448  ctx_arb\[sig4, sig5\]
 e08:	0000a04c488ca309 	\.449  mem\[lock256/lock512, \$xfer_8, gprA_9, <<8, 0x8, 5\], sig_done\[sig4\]
 e10:	000f20e000000030 	\.450  ctx_arb\[sig4, sig5\]
 e18:	000ae04d4084a009 	\.451  mem\[microq128_pop, \$xfer_8, gprA_9, <<8, 0x8, 1\], ctx_swap\[sig4\]
 e20:	0002204d4080a009 	\.452  mem\[microq128_get, \$xfer_8, gprA_9, <<8, 0x8, 1\], ctx_swap\[sig4\]
 e28:	000ba04d4880a009 	\.453  mem\[microq256_get, \$xfer_8, gprA_9, <<8, 0x8, 5\], ctx_swap\[sig4\]
 e30:	0003805700028309 	\.454  mem\[ctm\.pe_dma_from_memory_buffer/emem\.fast_journal/imem\.lb_push_stats_local, \$xfer_0, gprA_9, <<8, 0x40, 1\]
 e38:	0005e04e4000a309 	\.455  mem\[queue128_lock, \$xfer_0, gprA_9, <<8, 0x8, 1\], sig_done\[sig4\]
 e40:	000f20e000000030 	\.456  ctx_arb\[sig4, sig5\]
 e48:	0001a04e0004a309 	\.457  mem\[queue128_unlock, \$xfer_0, gprA_9, <<8, 0x8, 1\]
 e50:	000c604e4800a309 	\.458  mem\[queue256_lock, \$xfer_0, gprA_9, <<8, 0x8, 5\], sig_done\[sig4\]
 e58:	000f20e000000030 	\.459  ctx_arb\[sig4, sig5\]
 e60:	0008204e0804a309 	\.460  mem\[queue256_unlock, \$xfer_0, gprA_9, <<8, 0x8, 5\]
 e68:	0008a05000001309 	\.461  mem\[ctm\.packet_wait_packet_status/emem\.rd_qdesc/imem\.stats_log, \$xfer_0, gprA_9, <<8, gprB_4, 1\]
 e70:	000b840092200c02 	\.462  ila\[read, \$xfer_2, gprB_3, <<8, gprA_2, 2\], ctx_swap\[sig9\]
 e78:	0005440182240f02 	\.463  ila\[write_check_error, \$xfer_2, gprB_3, <<8, gprA_2, 2\], sig_done\[sig8\]
 e80:	000d60e000000300 	\.464  ctx_arb\[sig8, sig9\]
 e88:	0007800410600000 	\.465  nbi\[read, \$xfer_6, gprA_0, <<8, gprB_0, 1\], ctx_swap\[sig1\]
 e90:	0002600c62000000 	\.466  pcie\[read, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 e98:	0004c40d62000000 	\.467  pcie\[write, \$xfer_0, gprB_0, <<8, gprA_0, 2\], ctx_swap\[sig6\]
 ea0:	000d601462000000 	\.468  crypto\[read, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 ea8:	0006601562000000 	\.469  crypto\[write, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 eb0:	0000601662000000 	\.470  crypto\[write_fifo, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 eb8:	000d840d60000050 	\.471  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index0, 1\], ctx_swap\[sig6\]
 ec0:	0009e40d60000058 	\.472  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index1, 1\], ctx_swap\[sig6\]
 ec8:	0009040d60000059 	\.473  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index1\[1\], 1\], ctx_swap\[sig6\]
 ed0:	000000f0000c0300 	\.474  nop
 ed8:	000000f0000c0300 	\.475  nop
 ee0:	000000f0000c0300 	\.476  nop
 ee8:	000000f0000c0300 	\.477  nop
 ef0:	000000f0000c0300 	\.478  nop
 ef8:	000060a900301340 	\.479  crc_be\[crc_ccitt, gprA_3, gprB_4\]
 f00:	000000f0000c0300 	\.480  nop
 f08:	000e20b9403d0004 	\.481  crc_be\[crc_ccitt, gprB_3, gprA_4\]
 f10:	000000f0000c0300 	\.482  nop
 f18:	000400a900301348 	\.483  crc_le\[crc_ccitt, gprA_3, gprB_4\]
 f20:	000000f0000c0300 	\.484  nop
 f28:	000400b9403d2004 	\.485  crc_le\[crc_ccitt, gprB_3, gprA_4\]
 f30:	000000f0000c0300 	\.486  nop
 f38:	0002e0b900301348 	\.487  crc_le\[crc_ccitt, gprB_3, gprB_4\]
 f40:	000000f0000c0300 	\.488  nop
 f48:	0002e0a9403d2004 	\.489  crc_le\[crc_ccitt, gprA_3, gprA_4\]
 f50:	000000f0000c0300 	\.490  nop
 f58:	000220e000020000 	\.491  ctx_arb\[bpt\]
 f60:	000420e000010000 	\.492  ctx_arb\[kill\]

Disassembly of section \.text\.i33\.me9:

0000000000000000 <\.text\.i33\.me9>:
   0:	000540f0012cd000 	  \.0  immed\[gprA_0, 0x1234\]
   8:	0002c0f0012cd280 	  \.1  immed\[n\$reg_0, 0x1234\]
  10:	000220f0012cd281 	  \.2  immed\[n\$reg_1, 0x1234\]
  18:	000660f0012cd200 	  \.3  immed\[\*l\$index0, 0x1234\]
  20:	0007c0f0012cd220 	  \.4  immed\[\*l\$index1, 0x1234\]
  28:	000fa0f0012cd230 	  \.5  immed\[\*l\$index1\+\+, 0x1234\]
  30:	000f40f0012cd231 	  \.6  immed\[\*l\$index1--, 0x1234\]
  38:	0008c8f0012cd200 	  \.7  immed\[\*l\$index2, 0x1234\]
  40:	0000a8f0012cd210 	  \.8  immed\[\*l\$index2\+\+, 0x1234\]
  48:	000048f0012cd211 	  \.9  immed\[\*l\$index2--, 0x1234\]
  50:	000968f0012cd220 	 \.10  immed\[\*l\$index3, 0x1234\]
  58:	0007e0f0012cd203 	 \.11  immed\[\*l\$index0\[3\], 0x1234\]
  60:	000540f0012cd225 	 \.12  immed\[\*l\$index1\[5\], 0x1234\]
  68:	000b28f0012cd207 	 \.13  immed\[\*l\$index2\[7\], 0x1234\]
  70:	000de8f0012cd229 	 \.14  immed\[\*l\$index3\[9\], 0x1234\]
  78:	000000f00ff003ff 	 \.15  immed\[gprB_0, 0xffff\]
  80:	000d60f220000bff 	 \.16  immed_b1\[gprB_2, 0xff\]
  88:	000f60f6200007ff 	 \.17  immed_b3\[gprB_1, 0xff\]
  90:	000080f080000f00 	 \.18  immed\[gprB_3, 0xffffffff\]
  98:	000100f086600f77 	 \.19  immed\[gprB_3, 0xffff9988\]
  a0:	000940f0012cd180 	 \.20  immed\[\$xfer_0, 0x1234\]
  a8:	000a00f0043c8581 	 \.21  immed\[\$xfer_1, 0x4321\]
  b0:	000b40f0056de19e 	 \.22  immed\[\$xfer_30, 0x5678\]
  b8:	0007c0f0400e8401 	 \.23  immed_w0\[gprA_1, 0xa1\]
  c0:	000440f4400e8802 	 \.24  immed_w1\[gprA_2, 0xa2\]
  c8:	000d00f4000e8c03 	 \.25  immed\[gprA_3, 0xa3, <<16\]
  d0:	000520f001200334 	 \.26  immed\[gprB_0, 0x1234\]
  d8:	000fa0f0400007b1 	 \.27  immed_w0\[gprB_1, 0xb1\]
  e0:	000c20f440000bb2 	 \.28  immed_w1\[gprB_2, 0xb2\]
  e8:	000560f400000fb3 	 \.29  immed\[gprB_3, 0xb3, <<16\]
  f0:	000660f200000fb3 	 \.30  immed\[gprB_3, 0xb3, <<8\]
  f8:	0001b0f200000fb3 	 \.31  immed\[gprB_3, 0xb3, <<8\], predicate_cc
 100:	0001c2f200000fb3 	 \.32  immed\[gprB_3, 0xb3, <<8\], gpr_wrboth
 108:	000ba0a0300c2f00 	 \.33  alu\[--, --, B, 0xb\]
 110:	0005a081f200da00 	 \.34  alu_shf\[--, --, B, 0x16, <<1\]
 118:	000be081d2018600 	 \.35  alu_shf\[--, --, B, 0x21, <<3\]
 120:	000240801201b200 	 \.36  alu_shf\[--, --, B, 0x2c, <<31\]
 128:	000fa081f800da00 	 \.37  alu_shf\[\$xfer_0, --, B, 0x16, <<1\]
 130:	0009e081f840da00 	 \.38  alu_shf\[\$xfer_4, --, B, 0x16, <<1\]
 138:	0009a081f980da00 	 \.39  alu_shf\[\$xfer_24, --, B, 0x16, <<1\]
 140:	0003e081f9f0da00 	 \.40  alu_shf\[\$xfer_31, --, B, 0x16, <<1\]
 148:	0004a0a0280c2f00 	 \.41  alu\[n\$reg_0, --, B, 0xb\]
 150:	0001e0a0281c2f00 	 \.42  alu\[n\$reg_1, --, B, 0xb\]
 158:	000880a0a00c2400 	 \.43  alu\[\*l\$index0, gprA_0, \+, 0x9\]
 160:	000100a0a43c2400 	 \.44  alu\[\*n\$index\+\+, gprA_0, \+, 0x9\]
 168:	000b208bc500a600 	 \.45  alu_shf\[\*l\$index0, gprA_0, OR, 0x9, <<4\]
 170:	000b00a0a20c2400 	 \.46  alu\[\*l\$index1, gprA_0, \+, 0x9\]
 178:	000740a0a30c2400 	 \.47  alu\[\*l\$index1\+\+, gprA_0, \+, 0x9\]
 180:	000200a0a31c2400 	 \.48  alu\[\*l\$index1--, gprA_0, \+, 0x9\]
 188:	000628a0a00c2400 	 \.49  alu\[\*l\$index2, gprA_0, \+, 0x9\]
 190:	000988aa210c2400 	 \.50  alu\[\*l\$index2\+\+, gprA_0, OR, 0x9\]
 198:	000f28a0a11c2400 	 \.51  alu\[\*l\$index2--, gprA_0, \+, 0x9\]
 1a0:	0005a8a0a20c2400 	 \.52  alu\[\*l\$index3, gprA_0, \+, 0x9\]
 1a8:	000480a0a03c2400 	 \.53  alu\[\*l\$index0\[3\], gprA_0, \+, 0x9\]
 1b0:	000800a0a25c2400 	 \.54  alu\[\*l\$index1\[5\], gprA_0, \+, 0x9\]
 1b8:	000c68a0a07c2400 	 \.55  alu\[\*l\$index2\[7\], gprA_0, \+, 0x9\]
 1c0:	000aa8a0a29c2400 	 \.56  alu\[\*l\$index3\[9\], gprA_0, \+, 0x9\]
 1c8:	000cc4b0c008a400 	 \.57  alu\[gprB_0, \*l\$index3\[9\], \+, gprA_0\]
 1d0:	000fe4b0c008c000 	 \.58  alu\[gprB_0, \*l\$index3\+\+, \+, gprA_0\]
 1d8:	000ac4b0c008c400 	 \.59  alu\[gprB_0, \*l\$index3--, \+, gprA_0\]
 1e0:	000bc4b080000229 	 \.60  alu\[gprB_0, \*l\$index3\[9\], \+, gprB_0\]
 1e8:	000724b080000230 	 \.61  alu\[gprB_0, \*l\$index3\+\+, \+, gprB_0\]
 1f0:	0007c4b080000231 	 \.62  alu\[gprB_0, \*l\$index3--, \+, gprB_0\]
 1f8:	000664b080000211 	 \.63  alu\[gprB_0, \*l\$index2--, \+, gprB_0\]
 200:	000a60b080000231 	 \.64  alu\[gprB_0, \*l\$index1--, \+, gprB_0\]
 208:	000bc0b080000211 	 \.65  alu\[gprB_0, \*l\$index0--, \+, gprB_0\]
 210:	000340b080000200 	 \.66  alu\[gprB_0, \*l\$index0, \+, gprB_0\]
 218:	000ee4b080000200 	 \.67  alu\[gprB_0, \*l\$index2, \+, gprB_0\]
 220:	000100b080000241 	 \.68  alu\[gprB_0, \*n\$index, \+, gprB_0\]
 228:	0004809bf0000241 	 \.69  alu_shf\[gprB_0, \*n\$index, OR, gprB_0, <<1\]
 230:	000f20a0001fff00 	 \.70  alu\[gprA_1, --, B, 0xff\]
 238:	0005c0b0002fff00 	 \.71  alu\[gprB_2, --, B, 0xff\]
 240:	000940a0000d6f00 	 \.72  alu\[gprA_0, --, B, 0x5b\]
 248:	000440a2000d6f00 	 \.73  alu\[gprA_0, --, ~B, 0x5b\]
 250:	000de081f032f200 	 \.74  alu_shf\[gprA_3, --, B, 0x5c, <<1\]
 258:	000de091d012f600 	 \.75  alu_shf\[gprB_1, --, B, 0x5d, <<3\]
 260:	000d60901022fa00 	 \.76  alu_shf\[gprB_2, --, B, 0x5e, <<31\]
 268:	000e40a0c0000402 	 \.77  alu\[gprA_0, gprB_1, \+, gprA_2\]
 270:	000340a2c0000402 	 \.78  alu\[gprA_0, gprB_1, \+16, gprA_2\]
 278:	000040a4c0000402 	 \.79  alu\[gprA_0, gprB_1, \+8, gprA_2\]
 280:	0007a0a8c0000402 	 \.80  alu\[gprA_0, gprB_1, \+carry, gprA_2\]
 288:	000d40a6c0000402 	 \.81  alu\[gprA_0, gprB_1, -carry, gprA_2\]
 290:	000aa0aac0000402 	 \.82  alu\[gprA_0, gprB_1, -, gprA_2\]
 298:	0009a0acc0000402 	 \.83  alu\[gprA_0, gprB_1, B-A, gprA_2\]
 2a0:	000da0aa40000402 	 \.84  alu\[gprA_0, gprB_1, OR, gprA_2\]
 2a8:	000740a440000402 	 \.85  alu\[gprA_0, gprB_1, AND, gprA_2\]
 2b0:	000a40a640000402 	 \.86  alu\[gprA_0, gprB_1, ~AND, gprA_2\]
 2b8:	0000a0a840000402 	 \.87  alu\[gprA_0, gprB_1, AND~, gprA_2\]
 2c0:	000ea0ac40000402 	 \.88  alu\[gprA_0, gprB_1, XOR, gprA_2\]
 2c8:	000321a0c0000402 	 \.89  alu\[gprA_0, gprB_1, \+, gprA_2\], no_cc
 2d0:	000990a0c0000402 	 \.90  alu\[gprA_0, gprB_1, \+, gprA_2\], predicate_cc
 2d8:	0009e2a0c0000402 	 \.91  alu\[gprA_0, gprB_1, \+, gprA_2\], gpr_wrboth
 2e0:	000353a0c0000402 	 \.92  alu\[gprA_0, gprB_1, \+, gprA_2\], no_cc, gpr_wrboth, predicate_cc
 2e8:	000d418b70080602 	 \.93  alu_shf\[gprA_0, gprB_1, OR, gprA_2, <<9\], no_cc
 2f0:	0006708a90080502 	 \.94  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>9\], predicate_cc
 2f8:	000ea28a90080402 	 \.95  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>rot9\], gpr_wrboth
 300:	000e138b70080402 	 \.96  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>rot23\], no_cc, gpr_wrboth, predicate_cc
 308:	000ba08a00080602 	 \.97  alu_shf\[gprA_0, gprB_1, OR, gprA_2, <<indirect\]
 310:	0000208a00080502 	 \.98  alu_shf\[gprA_0, gprB_1, OR, gprA_2, >>indirect\]
 318:	000ba0a0300c2f00 	 \.99  alu\[--, --, B, 0xb\]
 320:	000ae09d40380101 	\.100  asr\[gprB_3, gprA_1, >>20\]
 328:	000ba0a0300c2f00 	\.101  alu\[--, --, B, 0xb\]
 330:	000ea09d40310500 	\.102  asr\[gprB_3, \*n\$index, >>20\]
 338:	000ba0a0300c2f00 	\.103  alu\[--, --, B, 0xb\]
 340:	0007a09d40314100 	\.104  asr\[gprB_3, \*l\$index0, >>20\]
 348:	000ba0a0300c2f00 	\.105  alu\[--, --, B, 0xb\]
 350:	0000249d40316100 	\.106  asr\[gprB_3, \*l\$index3, >>20\]
 358:	000ba0a0300c2f00 	\.107  alu\[--, --, B, 0xb\]
 360:	000a049d40314100 	\.108  asr\[gprB_3, \*l\$index2, >>20\]
 368:	000ba0a0300c2f00 	\.109  alu\[--, --, B, 0xb\]
 370:	0004a08d45010d00 	\.110  asr\[\*l\$index0, \*n\$index\+\+, >>20\]
 378:	000ba0a0300c2f00 	\.111  alu\[--, --, B, 0xb\]
 380:	000ee08d45810d00 	\.112  asr\[\*l\$index1, \*n\$index\+\+, >>20\]
 388:	000ba0a0300c2f00 	\.113  alu\[--, --, B, 0xb\]
 390:	000a088d45010d00 	\.114  asr\[\*l\$index2, \*n\$index\+\+, >>20\]
 398:	000ba0a0300c2f00 	\.115  alu\[--, --, B, 0xb\]
 3a0:	0007819d40380101 	\.116  asr\[gprB_3, gprA_1, >>20\], no_cc
 3a8:	000ba0a0300c2f00 	\.117  alu\[--, --, B, 0xb\]
 3b0:	000d309d40380101 	\.118  asr\[gprB_3, gprA_1, >>20\], predicate_cc
 3b8:	000ba0a0300c2f00 	\.119  alu\[--, --, B, 0xb\]
 3c0:	000ba28d40380101 	\.120  asr\[gprA_3, gprA_1, >>20\], gpr_wrboth
 3c8:	0008c0d818c08120 	\.121  beq\[\.99\]
 3d0:	000d00d877c08120 	\.122  beq\[\.479\]
 3d8:	000440d877e08120 	\.123  beq\[\.479\], defer\[2\]
 3e0:	000000f0000c0300 	\.124  nop
 3e8:	000000f0000c0300 	\.125  nop
 3f0:	000540d877c08021 	\.126  bne\[\.479\]
 3f8:	0004c0d877c08022 	\.127  bmi\[\.479\]
 400:	000420d877c08023 	\.128  bpl\[\.479\]
 408:	0007c0d877c08024 	\.129  bcs\[\.479\]
 410:	0007c0d877c08024 	\.130  bcs\[\.479\]
 418:	000720d877c08025 	\.131  bcc\[\.479\]
 420:	000720d877c08025 	\.132  bcc\[\.479\]
 428:	0006a0d877c08026 	\.133  bvs\[\.479\]
 430:	000640d877c08027 	\.134  bvc\[\.479\]
 438:	0001c0d877c08028 	\.135  bge\[\.479\]
 440:	000120d877c08029 	\.136  blt\[\.479\]
 448:	000040d877c0802b 	\.137  bgt\[\.479\]
 450:	0000a0d877c0802a 	\.138  ble\[\.479\]
 458:	000c60d818c08038 	\.139  br\[\.99\]
 460:	000920d818d08038 	\.140  br\[\.99\], defer\[1\]
 468:	000000f0000c0300 	\.141  nop
 470:	000bc0d077c09000 	\.142  br_bclr\[gprA_0, 3, \.479\]
 478:	000980d077c0e004 	\.143  br_bclr\[gprA_4, 23, \.479\]
 480:	0002a0d077c0082c 	\.144  br_bclr\[gprB_2, 11, \.479\]
 488:	000300d077c02423 	\.145  br_bclr\[gprB_9, 2, \.479\]
 490:	000260d077c02421 	\.146  br_bclr\[gprB_9, 0, \.479\]
 498:	000280d077c02420 	\.147  br_bclr\[gprB_9, 31, \.479\]
 4a0:	000f00d077f02423 	\.148  br_bclr\[gprB_9, 2, \.479\], defer\[3\]
 4a8:	000000f0000c0300 	\.149  nop
 4b0:	000000f0000c0300 	\.150  nop
 4b8:	000000f0000c0300 	\.151  nop
 4c0:	000680d077c42c2b 	\.152  br_bset\[gprB_11, 10, \.479\]
 4c8:	0006e0d077c4ac0b 	\.153  br_bset\[gprA_11, 10, \.479\]
 4d0:	0002a0c877d81020 	\.154  br=byte\[gprB_4, 0, 0x0, \.479\], defer\[1\]
 4d8:	000000f0000c0300 	\.155  nop
 4e0:	000a60c877c81520 	\.156  br=byte\[gprB_5, 1, 0x0, \.479\]
 4e8:	0001e0c877c81620 	\.157  br=byte\[gprB_5, 2, 0x0, \.479\]
 4f0:	0001a4c877c94220 	\.158  br=byte\[\*l\$index2, 2, 0x0, \.479\]
 4f8:	000620c877c96220 	\.159  br=byte\[\*l\$index1, 2, 0x0, \.479\]
 500:	000540c877c81b20 	\.160  br=byte\[gprB_6, 3, 0x0, \.479\]
 508:	0000c0c877cc16ff 	\.161  br=byte\[gprB_5, 2, 0xff, \.479\]
 510:	000420c877c816a2 	\.162  br=byte\[gprB_5, 2, 0x42, \.479\]
 518:	000380c877c416ff 	\.163  br!=byte\[gprB_5, 2, 0xff, \.479\]
 520:	0002a0c877c01620 	\.164  br!=byte\[gprB_5, 2, 0x0, \.479\]
 528:	000c20d877c00236 	\.165  br_cls_state\[cls_ring0_status, \.479\]
 530:	0001a0d877e20236 	\.166  br_cls_state\[cls_ring8_status, \.479\], defer\[2\]
 538:	000000f0000c0300 	\.167  nop
 540:	000000f0000c0300 	\.168  nop
 548:	000be0d877c38236 	\.169  br_cls_state\[cls_ring14_status, \.479\]
 550:	0007c0d877c3c236 	\.170  br_cls_state\[cls_ring15_status, \.479\]
 558:	000720d877c3c237 	\.171  br_!cls_state\[cls_ring15_status, \.479\]
 560:	000cc0d877c00237 	\.172  br_!cls_state\[cls_ring0_status, \.479\]
 568:	000c00d877c00030 	\.173  br=ctx\[0, \.479\]
 570:	000dc0d877c08030 	\.174  br=ctx\[2, \.479\]
 578:	000f00d877c18030 	\.175  br=ctx\[6, \.479\]
 580:	000a40d877d18030 	\.176  br=ctx\[6, \.479\], defer\[1\]
 588:	000000f0000c0300 	\.177  nop
 590:	000d40d877c00234 	\.178  br_inp_state\[nn_empty, \.479\]
 598:	000160d877c04234 	\.179  br_inp_state\[nn_full, \.479\]
 5a0:	000c80d877c08234 	\.180  br_inp_state\[ctm_ring0_status, \.479\]
 5a8:	000100d877e28234 	\.181  br_inp_state\[ctm_ring8_status, \.479\], defer\[2\]
 5b0:	000000f0000c0300 	\.182  nop
 5b8:	000000f0000c0300 	\.183  nop
 5c0:	000a80d877c38234 	\.184  br_inp_state\[ctm_ring12_status, \.479\]
 5c8:	0006a0d877c3c234 	\.185  br_inp_state\[ctm_ring13_status, \.479\]
 5d0:	000640d877c3c235 	\.186  br_!inp_state\[ctm_ring13_status, \.479\]
 5d8:	000c60d877c08235 	\.187  br_!inp_state\[ctm_ring0_status, \.479\]
 5e0:	000260d877c04232 	\.188  br_signal\[1, \.479\]
 5e8:	000f80d877c08232 	\.189  br_signal\[2, \.479\]
 5f0:	0005a0d877c3c232 	\.190  br_signal\[15, \.479\]
 5f8:	000540d877c3c233 	\.191  br_!signal\[15, \.479\]
 600:	000b60d877f2c232 	\.192  br_signal\[11, \.479\], defer\[3\]
 608:	000000f0000c0300 	\.193  nop
 610:	000000f0000c0300 	\.194  nop
 618:	000000f0000c0300 	\.195  nop
 620:	000e40a0c0000402 	\.196  alu\[gprA_0, gprB_1, \+, gprA_2\]
 628:	0004408e02081200 	\.197  byte_align_le\[--, gprB_4\]
 630:	0008c08e00981200 	\.198  byte_align_le\[gprA_9, gprB_4\]
 638:	0004c08e00a81200 	\.199  byte_align_le\[gprA_10, gprB_4\]
 640:	0001808e00b81200 	\.200  byte_align_le\[gprA_11, gprB_4\]
 648:	000e40a0c0000402 	\.201  alu\[gprA_0, gprB_1, \+, gprA_2\]
 650:	000c808e02001100 	\.202  byte_align_be\[--, gprB_4\]
 658:	0000008e00901100 	\.203  byte_align_be\[gprA_9, gprB_4\]
 660:	000c008e00a01100 	\.204  byte_align_be\[gprA_10, gprB_4\]
 668:	0009408e00b01100 	\.205  byte_align_be\[gprA_11, gprB_4\]
 670:	000d80a0300c0300 	\.206  alu\[--, --, B, 0x0\]
 678:	000400a5b00c0000 	\.207  cam_clear
 680:	000360bb80900007 	\.208  cam_lookup\[gprB_9, gprA_7\]
 688:	0003a0bb80900200 	\.209  cam_lookup\[gprB_9, \*l\$index0\]
 690:	000e04bb80900200 	\.210  cam_lookup\[gprB_9, \*l\$index2\]
 698:	000f84bb80900203 	\.211  cam_lookup\[gprB_9, \*l\$index2\[3\]\]
 6a0:	000bc0bb80900210 	\.212  cam_lookup\[gprB_9, \*l\$index0\+\+\]
 6a8:	000280aba0000241 	\.213  cam_lookup\[\*l\$index0, \*n\$index\]
 6b0:	000ec0aba1000241 	\.214  cam_lookup\[\*l\$index0\+\+, \*n\$index\]
 6b8:	000288aba3000243 	\.215  cam_lookup\[\*l\$index3\+\+, \*n\$index\+\+\]
 6c0:	000aa0aba0200243 	\.216  cam_lookup\[\*l\$index0\[2\], \*n\$index\+\+\]
 6c8:	000060bb80901407 	\.217  cam_lookup\[gprB_9, gprA_7\], lm_addr0\[1\]
 6d0:	000060bb80902807 	\.218  cam_lookup\[gprB_9, gprA_7\], lm_addr1\[2\]
 6d8:	000660bb80907407 	\.219  cam_lookup\[gprB_9, gprA_7\], lm_addr2\[3\]
 6e0:	000660bb80904807 	\.220  cam_lookup\[gprB_9, gprA_7\], lm_addr3\[0\]
 6e8:	000222ab80900007 	\.221  cam_lookup\[gprA_9, gprA_7\], gpr_wrboth
 6f0:	0004b0bb80900007 	\.222  cam_lookup\[gprB_9, gprA_7\], predicate_cc
 6f8:	000a00a7809c0000 	\.223  cam_read_tag\[gprA_9, 0x0\]
 700:	000da2a7809c0000 	\.224  cam_read_tag\[gprA_9, 0x0\], gpr_wrboth
 708:	000dd0a7809c0000 	\.225  cam_read_tag\[gprA_9, 0x0\], predicate_cc
 710:	000900a7809c2800 	\.226  cam_read_tag\[gprA_9, 0xa\]
 718:	000a00a7809c3c00 	\.227  cam_read_tag\[gprA_9, 0xf\]
 720:	0003e0af809c0000 	\.228  cam_read_state\[gprA_9, 0x0\]
 728:	000442af809c0000 	\.229  cam_read_state\[gprA_9, 0x0\], gpr_wrboth
 730:	000392af809c0000 	\.230  cam_read_state\[gprA_9, 0x0\], gpr_wrboth, predicate_cc
 738:	0000e0af809c2800 	\.231  cam_read_state\[gprA_9, 0xa\]
 740:	0003e0af809c3c00 	\.232  cam_read_state\[gprA_9, 0xf\]
 748:	000920a9f0101700 	\.233  cam_write\[0x0, gprB_5, 1\]
 750:	000da0a9f01a0300 	\.234  cam_write\[0x0, n\$reg_0, 1\]
 758:	000e80a9f0190700 	\.235  cam_write\[0x0, \*n\$index, 1\]
 760:	0004c4a9f0180300 	\.236  cam_write\[0x0, \*l\$index2, 1\]
 768:	0008e4a9f0184300 	\.237  cam_write\[0x0, \*l\$index2\+\+, 1\]
 770:	000dc4a9f0184700 	\.238  cam_write\[0x0, \*l\$index2--, 1\]
 778:	000840a9f0b01704 	\.239  cam_write\[0x4, gprB_5, 11\]
 780:	000be0a9f0f0170f 	\.240  cam_write\[0xf, gprB_5, 15\]
 788:	0008a0adb01c0000 	\.241  cam_write_state\[0x0, 1\]
 790:	000d80adb0bc1000 	\.242  cam_write_state\[0x4, 11\]
 798:	000de0adb0fc3c00 	\.243  cam_write_state\[0xf, 15\]
 7a0:	0000c0fc142c000d 	\.244  local_csr_wr\[CRCRemainder, gprA_13\]
 7a8:	000d20a918060348 	\.245  crc_le\[crc_ccitt, \$xfer_0, \$xfer_0\]
 7b0:	000000f0000c0300 	\.246  nop
 7b8:	000d40a918160748 	\.247  crc_le\[crc_ccitt, \$xfer_1, \$xfer_1\]
 7c0:	000000f0000c0300 	\.248  nop
 7c8:	000d40a918260b48 	\.249  crc_le\[crc_ccitt, \$xfer_2, \$xfer_2\]
 7d0:	000000f0000c0300 	\.250  nop
 7d8:	000d20a918360f48 	\.251  crc_le\[crc_ccitt, \$xfer_3, \$xfer_3\]
 7e0:	000000f0000c0300 	\.252  nop
 7e8:	000000f0000c0300 	\.253  nop
 7f0:	000000f0000c0300 	\.254  nop
 7f8:	000000f0000c0300 	\.255  nop
 800:	000000f0000c0300 	\.256  nop
 808:	000f60fc140c0000 	\.257  local_csr_rd\[CRCRemainder\]
 810:	000ce0f0000c000e 	\.258  immed\[gprA_14, 0x0\]
 818:	000940a918060340 	\.259  crc_be\[crc_ccitt, \$xfer_0, \$xfer_0\]
 820:	000000f0000c0300 	\.260  nop
 828:	000920a918461340 	\.261  crc_be\[crc_ccitt, \$xfer_4, \$xfer_4\]
 830:	000000f0000c0300 	\.262  nop
 838:	000060a900061340 	\.263  crc_be\[crc_ccitt, gprA_0, \$xfer_4\]
 840:	000000f0000c0300 	\.264  nop
 848:	000c60a900001340 	\.265  crc_be\[crc_ccitt, gprA_0, gprB_4\]
 850:	000000f0000c0300 	\.266  nop
 858:	000000f0000c0300 	\.267  nop
 860:	000000f0000c0300 	\.268  nop
 868:	000000f0000c0300 	\.269  nop
 870:	000000f0000c0300 	\.270  nop
 878:	000600a918260380 	\.271  crc_be\[crc_32, \$xfer_2, \$xfer_0\]
 880:	000000f0000c0300 	\.272  nop
 888:	0004c0a9183613a0 	\.273  crc_be\[crc_iscsi, \$xfer_3, \$xfer_4\]
 890:	000000f0000c0300 	\.274  nop
 898:	0004c0a9000613c0 	\.275  crc_be\[crc_10, gprA_0, \$xfer_4\]
 8a0:	000000f0000c0300 	\.276  nop
 8a8:	000960a9000013e0 	\.277  crc_be\[crc_5, gprA_0, gprB_4\]
 8b0:	000000f0000c0300 	\.278  nop
 8b8:	000ea0a918862700 	\.279  crc_be\[--, \$xfer_8, \$xfer_9\]
 8c0:	000000f0000c0300 	\.280  nop
 8c8:	000240a918760784 	\.281  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_0_2
 8d0:	000000f0000c0300 	\.282  nop
 8d8:	0002a0a918760785 	\.283  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_0_1
 8e0:	000000f0000c0300 	\.284  nop
 8e8:	000320a918760786 	\.285  crc_be\[crc_32, \$xfer_7, \$xfer_1\], byte_0
 8f0:	000000f0000c0300 	\.286  nop
 8f8:	0000c0a918760781 	\.287  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_1_3
 900:	000000f0000c0300 	\.288  nop
 908:	000140a918760782 	\.289  crc_be\[crc_32, \$xfer_7, \$xfer_1\], bytes_2_3
 910:	000000f0000c0300 	\.290  nop
 918:	0001a0a918760783 	\.291  crc_be\[crc_32, \$xfer_7, \$xfer_1\], byte_3
 920:	000000f0000c0300 	\.292  nop
 928:	000782a900160780 	\.293  crc_be\[crc_32, gprA_1, \$xfer_1\], gpr_wrboth
 930:	000000f0000c0300 	\.294  nop
 938:	000ae3a900160780 	\.295  crc_be\[crc_32, gprA_1, \$xfer_1\], no_cc, gpr_wrboth
 940:	000000f0000c0300 	\.296  nop
 948:	000b73a900560780 	\.297  crc_be\[crc_32, gprA_5, \$xfer_1\], no_cc, gpr_wrboth, predicate_cc
 950:	000000f0000c0300 	\.298  nop
 958:	000122a900560781 	\.299  crc_be\[crc_32, gprA_5, \$xfer_1\], bytes_1_3, gpr_wrboth
 960:	000000f0000c0300 	\.300  nop
 968:	000000f0000c0300 	\.301  nop
 970:	000000f0000c0300 	\.302  nop
 978:	000000f0000c0300 	\.303  nop
 980:	000000f0000c0300 	\.304  nop
 988:	000000f0000c0300 	\.305  nop
 990:	0005a0e000080000 	\.306  ctx_arb\[--\]
 998:	000600e000000001 	\.307  ctx_arb\[voluntary\]
 9a0:	000220e000020000 	\.308  ctx_arb\[bpt\]
 9a8:	000460e000000220 	\.309  ctx_arb\[sig5, sig9\]
 9b0:	000d20e000200220 	\.310  ctx_arb\[sig5, sig9\], defer\[2\]
 9b8:	000180a0300c0f00 	\.311  alu\[--, --, B, 0x3\]
 9c0:	0007a0a0300c1f00 	\.312  alu\[--, --, B, 0x7\]
 9c8:	0006a0e000010220 	\.313  ctx_arb\[sig5, sig9\], any
 9d0:	000a60e077c40220 	\.314  ctx_arb\[sig5, sig9\], br\[\.479\]
 9d8:	0006409010500701 	\.315  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\]
 9e0:	000d4090a0500701 	\.316  dbl_shf\[gprB_5, gprA_1, gprB_1, >>10\]
 9e8:	000c4091f0500701 	\.317  dbl_shf\[gprB_5, gprA_1, gprB_1, >>31\]
 9f0:	000740a440000402 	\.318  alu\[gprA_0, gprB_1, AND, gprA_2\]
 9f8:	0000c09000500701 	\.319  dbl_shf\[gprB_5, gprA_1, gprB_1, >>indirect\]
 a00:	000b219010500701 	\.320  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\], no_cc
 a08:	000cf19010500701 	\.321  dbl_shf\[gprB_5, gprA_1, gprB_1, >>1\], no_cc, predicate_cc
 a10:	0000d28010500701 	\.322  dbl_shf\[gprA_5, gprA_1, gprB_1, >>1\], gpr_wrboth, predicate_cc
 a18:	000200a700f03f00 	\.323  ffs\[gprA_15, gprB_15\]
 a20:	000fe0b740fc000f 	\.324  ffs\[gprB_15, gprA_15\]
 a28:	000ec0b700f61300 	\.325  ffs\[gprB_15, \$xfer_4\]
 a30:	000660b700f88300 	\.326  ffs\[gprB_15, \*l\$index1\]
 a38:	0007e4b700f8c300 	\.327  ffs\[gprB_15, \*l\$index3\+\+\]
 a40:	0002c4b700f8c700 	\.328  ffs\[gprB_15, \*l\$index3--\]
 a48:	0004c4b700f8a700 	\.329  ffs\[gprB_15, \*l\$index3\[9\]\]
 a50:	000880a720000300 	\.330  ffs\[\*l\$index0, gprB_0\]
 a58:	000108a722090700 	\.331  ffs\[\*l\$index3, \*n\$index\]
 a60:	000128a723190f00 	\.332  ffs\[\*l\$index3--, \*n\$index\+\+\]
 a68:	0003c3a740fc000f 	\.333  ffs\[gprA_15, gprA_15\], no_cc, gpr_wrboth
 a70:	000972a740fc000f 	\.334  ffs\[gprA_15, gprA_15\], gpr_wrboth, predicate_cc
 a78:	000320f0000c0803 	\.335  immed\[gprA_3, 0x2\]
 a80:	000480e8004d4803 	\.336  jump\[gprA_3, \.338\]
 a88:	0006a0d854408038 	\.337  br\[\.337\]
 a90:	000460f000002701 	\.338  immed\[gprB_9, 0x1\]
 a98:	0006a0d854408038 	\.339  br\[\.337\]
 aa0:	0005e0f000002702 	\.340  immed\[gprB_9, 0x2\]
 aa8:	0006a0d854408038 	\.341  br\[\.337\]
 ab0:	000500f000002703 	\.342  immed\[gprB_9, 0x3\]
 ab8:	0006a0d854408038 	\.343  br\[\.337\]
 ac0:	000040c001000000 	\.344  ld_field\[gprA_0, 0001, gprB_0\]
 ac8:	0007e2c001000000 	\.345  ld_field\[gprA_0, 0001, gprB_0\], gpr_wrboth
 ad0:	000e40c401000000 	\.346  ld_field\[gprA_0, 0001, gprB_0\], load_cc
 ad8:	000790c001000000 	\.347  ld_field\[gprA_0, 0001, gprB_0\], predicate_cc
 ae0:	0005c0c005000000 	\.348  ld_field\[gprA_0, 0101, gprB_0\]
 ae8:	000080c005100000 	\.349  ld_field_w_clr\[gprA_0, 0101, gprB_0\]
 af0:	0002a2c001100000 	\.350  ld_field_w_clr\[gprA_0, 0001, gprB_0\], gpr_wrboth
 af8:	000b00c401100000 	\.351  ld_field_w_clr\[gprA_0, 0001, gprB_0\], load_cc
 b00:	0002d0c001100000 	\.352  ld_field_w_clr\[gprA_0, 0001, gprB_0\], predicate_cc
 b08:	000fc0c00f000000 	\.353  ld_field\[gprA_0, 1111, gprB_0\]
 b10:	0005e0c1fb000200 	\.354  ld_field\[gprA_0, 1011, gprB_0, <<1\]
 b18:	000460c01b000100 	\.355  ld_field\[gprA_0, 1011, gprB_0, >>1\]
 b20:	000e60c1fb000100 	\.356  ld_field\[gprA_0, 1011, gprB_0, >>31\]
 b28:	000bc0c09b000000 	\.357  ld_field\[gprA_0, 1011, gprB_0, >>rot9\]
 b30:	000e80c09b100000 	\.358  ld_field_w_clr\[gprA_0, 1011, gprB_0, >>rot9\]
 b38:	0001c0c17b000000 	\.359  ld_field\[gprA_0, 1011, gprB_0, >>rot23\]
 b40:	0002c0c41b000000 	\.360  ld_field\[gprA_0, 1011, gprB_0, >>rot1\], load_cc
 b48:	000780c41b100000 	\.361  ld_field_w_clr\[gprA_0, 1011, gprB_0, >>rot1\], load_cc
 b50:	000400f0001f7c01 	\.362  immed\[gprA_1, 0x1df\]
 b58:	000200f0001007df 	\.363  immed\[gprB_1, 0x1df\]
 b60:	0005a2f0001007df 	\.364  immed\[gprB_1, 0x1df\], gpr_wrboth
 b68:	0005d0f0001007df 	\.365  immed\[gprB_1, 0x1df\], predicate_cc
 b70:	000020fc010c0000 	\.366  local_csr_rd\[ALUOut\]
 b78:	000e60f0000c000b 	\.367  immed\[gprA_11, 0x0\]
 b80:	000ce0fc160c0000 	\.368  local_csr_rd\[MiscControl\]
 b88:	000e60f0000c000b 	\.369  immed\[gprA_11, 0x0\]
 b90:	000ae0fc076c0b02 	\.370  local_csr_wr\[XferIndex, 0x2\]
 b98:	0008a0fc076c0003 	\.371  local_csr_wr\[XferIndex, gprA_3\]
 ba0:	000520fc07600f00 	\.372  local_csr_wr\[XferIndex, gprB_3\]
 ba8:	000f20fc01a00f00 	\.373  local_csr_wr\[CtxEnables, gprB_3\]
 bb0:	000480f800000c02 	\.374  mul_step\[gprA_2, gprB_3\], start
 bb8:	000880f980000c02 	\.375  mul_step\[gprA_2, gprB_3\], 32x32_step1
 bc0:	000dc0f980100c02 	\.376  mul_step\[gprA_2, gprB_3\], 32x32_step2
 bc8:	0001c0f980200c02 	\.377  mul_step\[gprA_2, gprB_3\], 32x32_step3
 bd0:	000480f980300c02 	\.378  mul_step\[gprA_2, gprB_3\], 32x32_step4
 bd8:	000940f9804c0002 	\.379  mul_step\[gprA_2, --\], 32x32_last
 be0:	000ce0f9805c0003 	\.380  mul_step\[gprA_3, --\], 32x32_last2
 be8:	0001a0f800000802 	\.381  mul_step\[gprA_2, gprB_2\], start
 bf0:	000aa0f900000802 	\.382  mul_step\[gprA_2, gprB_2\], 16x16_step1
 bf8:	000fe0f900100802 	\.383  mul_step\[gprA_2, gprB_2\], 16x16_step2
 c00:	000f20f9004c0000 	\.384  mul_step\[gprA_0, --\], 16x16_last
 c08:	0001a0f800000802 	\.385  mul_step\[gprA_2, gprB_2\], start
 c10:	0006a0f880000802 	\.386  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c18:	000320f8804c0000 	\.387  mul_step\[gprA_0, --\], 24x8_last
 c20:	0001a0f800000802 	\.388  mul_step\[gprA_2, gprB_2\], start
 c28:	0006a0f880000802 	\.389  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c30:	0004f0f8804c0000 	\.390  mul_step\[gprA_0, --\], 24x8_last, predicate_cc
 c38:	0001a0f800000802 	\.391  mul_step\[gprA_2, gprB_2\], start
 c40:	0006a0f880000802 	\.392  mul_step\[gprA_2, gprB_2\], 24x8_step1
 c48:	0009e3f8804c0000 	\.393  mul_step\[gprA_0, --\], 24x8_last, no_cc, gpr_wrboth
 c50:	000b80a330000000 	\.394  pop_count1\[gprB_0\]
 c58:	000c80a3b0000000 	\.395  pop_count2\[gprB_0\]
 c60:	000d80a180000000 	\.396  pop_count3\[gprA_0, gprB_0\]
 c68:	000b80a330000000 	\.397  pop_count1\[gprB_0\]
 c70:	000c80a3b0000000 	\.398  pop_count2\[gprB_0\]
 c78:	000743a180000000 	\.399  pop_count3\[gprA_0, gprB_0\], no_cc, gpr_wrboth
 c80:	0004a4a330088000 	\.400  pop_count1\[\*l\$index3\]
 c88:	0003a4a3b0088000 	\.401  pop_count2\[\*l\$index3\]
 c90:	0000e5a1a438c000 	\.402  pop_count3\[\*n\$index\+\+, \*l\$index3\+\+\], no_cc
 c98:	000b80a330000000 	\.403  pop_count1\[gprB_0\]
 ca0:	000c80a3b0000000 	\.404  pop_count2\[gprB_0\]
 ca8:	000731a180000000 	\.405  pop_count3\[gprA_0, gprB_0\], no_cc, predicate_cc
 cb0:	000480e8000c0000 	\.406  rtn\[gprA_0\]
 cb8:	000620e8000a0700 	\.407  rtn\[n\$reg_1\]
 cc0:	000600e800088300 	\.408  rtn\[\*l\$index1\]
 cc8:	000a64e800080300 	\.409  rtn\[\*l\$index2\]
 cd0:	000dc0e800200300 	\.410  rtn\[gprB_0\], defer\[2\]
 cd8:	0008a0a0300c0700 	\.411  alu\[--, --, B, 0x1\]
 ce0:	0004a0a0300c0b00 	\.412  alu\[--, --, B, 0x2\]
 ce8:	000000f0000c0300 	\.413  nop
 cf0:	000000f0000c0300 	\.414  nop
 cf8:	000000f0000c0300 	\.415  nop
 d00:	000000f0000c0300 	\.416  nop
 d08:	0003501842300c09 	\.417  arm\[read, \$xfer_3, gprA_9, gprB_3, 2\], ctx_swap\[sig4\]
 d10:	0005501842302403 	\.418  arm\[read, \$xfer_3, gprA_3, gprB_9, 2\], ctx_swap\[sig4\]
 d18:	0004801842300c09 	\.419  arm\[read, \$xfer_3, gprA_9, <<8, gprB_3, 2\], ctx_swap\[sig4\]
 d20:	000f241842302403 	\.420  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], ctx_swap\[sig4\]
 d28:	0004a0a0300c0b00 	\.421  alu\[--, --, B, 0x2\]
 d30:	0008861842302403 	\.422  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], indirect_ref, ctx_swap\[sig4\]
 d38:	0004a0a0300c0b00 	\.423  alu\[--, --, B, 0x2\]
 d40:	000e8618e2302703 	\.424  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], indirect_ref, sig_done\[sig14\]
 d48:	0007841842302503 	\.425  arm\[read, \$xfer_3, gprB_9, <<8, gprA_3, 2\], ctx_swap\[sig4\], defer\[1\]
 d50:	0008a0a0300c0700 	\.426  alu\[--, --, B, 0x1\]
 d58:	000f101843c00c09 	\.427  arm\[read, \$xfer_28, gprA_9, gprB_3, 2\], ctx_swap\[sig4\]
 d60:	000910184e800c09 	\.428  arm\[read, \$xfer_8, gprA_9, gprB_3, 8\], ctx_swap\[sig4\]
 d68:	000a106440800c09 	\.429  cls\[add, \$xfer_8, gprA_9, gprB_3, 1\], ctx_swap\[sig4\]
 d70:	0000f0664080a009 	\.430  cls\[sub, \$xfer_8, gprA_9, 0x8, 1\], ctx_swap\[sig4\]
 d78:	000160644284a009 	\.431  cls\[add64, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 d80:	000404664284a408 	\.432  cls\[sub64, \$xfer_8, 0x9, <<8, gprA_8, 2\], ctx_swap\[sig4\]
 d88:	0008a0a0300c0700 	\.433  alu\[--, --, B, 0x1\]
 d90:	00032c650340a708 	\.434  cls\[add_imm, 0x14, 0x9, <<8, gprA_8, 2\]
 d98:	0007506040880c09 	\.435  cls\[swap/test_compare_write, \$xfer_8, gprA_9, gprB_3, 1\], ctx_swap\[sig4\]
 da0:	00023c6500007f9a 	\.436  cls\[add_imm, 0x1f9a, --, 1\]
 da8:	000038653c583f14 	\.437  cls\[add_imm, 0xf14, 0xf16\]
 db0:	000b54640013c30f 	\.438  cls\[add, \$xfer_1, 0xf00f, 1\]
 db8:	0002901c10a08000 	\.439  ct\[xpb_read, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dc0:	0007501e10a48000 	\.440  ct\[reflect_read_sig_init, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dc8:	000a501c10a48000 	\.441  ct\[ring_get, \$xfer_10, gprA_0, 0x0, 1\], ctx_swap\[sig1\]
 dd0:	000000f0000c0300 	\.442  nop
 dd8:	000cc0474a80a009 	\.443  mem\[add64, \$xfer_8, gprA_9, <<8, 0x8, 6\], ctx_swap\[sig4\]
 de0:	000d40404280a009 	\.444  mem\[read, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 de8:	000c405c4280a009 	\.445  mem\[read32, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 df0:	000ea0554280a009 	\.446  mem\[ctm\.pe_dma_to_memory_indirect/emem\.get/imem\.lb_bucket_read_local, \$xfer_8, gprA_9, <<8, 0x8, 2\], ctx_swap\[sig4\]
 df8:	0009204c408ca309 	\.447  mem\[lock128/lock384, \$xfer_8, gprA_9, <<8, 0x8, 1\], sig_done\[sig4\]
 e00:	000f20e000000030 	\.448  ctx_arb\[sig4, sig5\]
 e08:	0000a04c488ca309 	\.449  mem\[lock256/lock512, \$xfer_8, gprA_9, <<8, 0x8, 5\], sig_done\[sig4\]
 e10:	000f20e000000030 	\.450  ctx_arb\[sig4, sig5\]
 e18:	000ae04d4084a009 	\.451  mem\[microq128_pop, \$xfer_8, gprA_9, <<8, 0x8, 1\], ctx_swap\[sig4\]
 e20:	0002204d4080a009 	\.452  mem\[microq128_get, \$xfer_8, gprA_9, <<8, 0x8, 1\], ctx_swap\[sig4\]
 e28:	000ba04d4880a009 	\.453  mem\[microq256_get, \$xfer_8, gprA_9, <<8, 0x8, 5\], ctx_swap\[sig4\]
 e30:	0003805700028309 	\.454  mem\[ctm\.pe_dma_from_memory_buffer/emem\.fast_journal/imem\.lb_push_stats_local, \$xfer_0, gprA_9, <<8, 0x40, 1\]
 e38:	0005e04e4000a309 	\.455  mem\[queue128_lock, \$xfer_0, gprA_9, <<8, 0x8, 1\], sig_done\[sig4\]
 e40:	000f20e000000030 	\.456  ctx_arb\[sig4, sig5\]
 e48:	0001a04e0004a309 	\.457  mem\[queue128_unlock, \$xfer_0, gprA_9, <<8, 0x8, 1\]
 e50:	000c604e4800a309 	\.458  mem\[queue256_lock, \$xfer_0, gprA_9, <<8, 0x8, 5\], sig_done\[sig4\]
 e58:	000f20e000000030 	\.459  ctx_arb\[sig4, sig5\]
 e60:	0008204e0804a309 	\.460  mem\[queue256_unlock, \$xfer_0, gprA_9, <<8, 0x8, 5\]
 e68:	0008a05000001309 	\.461  mem\[ctm\.packet_wait_packet_status/emem\.rd_qdesc/imem\.stats_log, \$xfer_0, gprA_9, <<8, gprB_4, 1\]
 e70:	000b840092200c02 	\.462  ila\[read, \$xfer_2, gprB_3, <<8, gprA_2, 2\], ctx_swap\[sig9\]
 e78:	0005440182240f02 	\.463  ila\[write_check_error, \$xfer_2, gprB_3, <<8, gprA_2, 2\], sig_done\[sig8\]
 e80:	000d60e000000300 	\.464  ctx_arb\[sig8, sig9\]
 e88:	0007800410600000 	\.465  nbi\[read, \$xfer_6, gprA_0, <<8, gprB_0, 1\], ctx_swap\[sig1\]
 e90:	0002600c62000000 	\.466  pcie\[read, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 e98:	0004c40d62000000 	\.467  pcie\[write, \$xfer_0, gprB_0, <<8, gprA_0, 2\], ctx_swap\[sig6\]
 ea0:	000d601462000000 	\.468  crypto\[read, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 ea8:	0006601562000000 	\.469  crypto\[write, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 eb0:	0000601662000000 	\.470  crypto\[write_fifo, \$xfer_0, gprA_0, <<8, gprB_0, 2\], ctx_swap\[sig6\]
 eb8:	000d840d60000050 	\.471  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index0, 1\], ctx_swap\[sig6\]
 ec0:	0009e40d60000058 	\.472  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index1, 1\], ctx_swap\[sig6\]
 ec8:	0009040d60000059 	\.473  pcie\[write, \$xfer_0, gprB_0, <<8, \*l\$index1\[1\], 1\], ctx_swap\[sig6\]
 ed0:	000000f0000c0300 	\.474  nop
 ed8:	000000f0000c0300 	\.475  nop
 ee0:	000000f0000c0300 	\.476  nop
 ee8:	000000f0000c0300 	\.477  nop
 ef0:	000000f0000c0300 	\.478  nop
 ef8:	000060a900301340 	\.479  crc_be\[crc_ccitt, gprA_3, gprB_4\]
 f00:	000000f0000c0300 	\.480  nop
 f08:	000e20b9403d0004 	\.481  crc_be\[crc_ccitt, gprB_3, gprA_4\]
 f10:	000000f0000c0300 	\.482  nop
 f18:	000400a900301348 	\.483  crc_le\[crc_ccitt, gprA_3, gprB_4\]
 f20:	000000f0000c0300 	\.484  nop
 f28:	000400b9403d2004 	\.485  crc_le\[crc_ccitt, gprB_3, gprA_4\]
 f30:	000000f0000c0300 	\.486  nop
 f38:	0002e0b900301348 	\.487  crc_le\[crc_ccitt, gprB_3, gprB_4\]
 f40:	000000f0000c0300 	\.488  nop
 f48:	0002e0a9403d2004 	\.489  crc_le\[crc_ccitt, gprA_3, gprA_4\]
 f50:	000000f0000c0300 	\.490  nop
 f58:	000220e000020000 	\.491  ctx_arb\[bpt\]
 f60:	000420e000010000 	\.492  ctx_arb\[kill\]
