#as:
#objdump: -dr

.*:     file format .*


Disassembly of section .text:

0000000000000000 <target>:
       0:	[0-9a-f]* 	{ nop }
       8:	[0-9a-f]* 	{ nop }
      10:	[0-9a-f]* 	{ nop }
      18:	[0-9a-f]* 	{ nop }
      20:	[0-9a-f]* 	{ nop }
      28:	[0-9a-f]* 	{ nop }
      30:	[0-9a-f]* 	{ nop }
      38:	[0-9a-f]* 	{ nop }
      40:	[0-9a-f]* 	{ nop }
      48:	[0-9a-f]* 	{ nop }
      50:	[0-9a-f]* 	{ nop }
      58:	[0-9a-f]* 	{ nop }
      60:	[0-9a-f]* 	{ nop }
      68:	[0-9a-f]* 	{ nop }
      70:	[0-9a-f]* 	{ nop }
      78:	[0-9a-f]* 	{ nop }
      80:	[0-9a-f]* 	{ nop }
      88:	[0-9a-f]* 	{ nop }
      90:	[0-9a-f]* 	{ nop }
      98:	[0-9a-f]* 	{ nop }
      a0:	[0-9a-f]* 	{ nop }
      a8:	[0-9a-f]* 	{ nop }
      b0:	[0-9a-f]* 	{ nop }
      b8:	[0-9a-f]* 	{ nop }
      c0:	[0-9a-f]* 	{ nop }
      c8:	[0-9a-f]* 	{ nop }
      d0:	[0-9a-f]* 	{ nop }
      d8:	[0-9a-f]* 	{ nop }
      e0:	[0-9a-f]* 	{ nop }
      e8:	[0-9a-f]* 	{ nop }
      f0:	[0-9a-f]* 	{ nop }
      f8:	[0-9a-f]* 	{ nop }
     100:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; bnezt r15, 0 <target> }
     108:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; bnez r15, 0 <target> }
     110:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; bnez r15, 0 <target> }
     118:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; bnez r15, 0 <target> }
     120:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; bnez r15, 0 <target> }
     128:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; blez r15, 0 <target> }
     130:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; bgtzt r15, 0 <target> }
     138:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; bgtzt r15, 0 <target> }
     140:	[0-9a-f]* 	{ addli r5, r6, 4660 ; bgtzt r15, 0 <target> }
     148:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; beqzt r15, 0 <target> }
     150:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; beqzt r15, 0 <target> }
     158:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; beqzt r15, 0 <target> }
     160:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; beqz r15, 0 <target> }
     168:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; beqz r15, 0 <target> }
     170:	[0-9a-f]* 	{ addli r5, r6, 4660 ; beqz r15, 0 <target> }
     178:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; beqz r15, 0 <target> }
     180:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; blbs r15, 0 <target> }
     188:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; blbs r15, 0 <target> }
     190:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; blbst r15, 0 <target> }
     198:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; blbst r15, 0 <target> }
     1a0:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; blbst r15, 0 <target> }
     1a8:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; blbs r15, 0 <target> }
     1b0:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; blbst r15, 0 <target> }
     1b8:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; blbst r15, 0 <target> }
     1c0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; blbst r15, 0 <target> }
     1c8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; bgtz r15, 0 <target> }
     1d0:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; bgtzt r15, 0 <target> }
     1d8:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; bgtz r15, 0 <target> }
     1e0:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; bgtzt r15, 0 <target> }
     1e8:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; bgtz r15, 0 <target> }
     1f0:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; bgtzt r15, 0 <target> }
     1f8:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; bgtzt r15, 0 <target> }
     200:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; bgtz r15, 0 <target> }
     208:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; bgtzt r15, 0 <target> }
     210:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; bgtz r15, 0 <target> }
     218:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; bgtzt r15, 0 <target> }
     220:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; bgtzt r15, 0 <target> }
     228:	[0-9a-f]* 	{ cmples r5, r6, r7 ; bgtzt r15, 0 <target> }
     230:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; bgtzt r15, 0 <target> }
     238:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; j 0 <target> }
     240:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; bltzt r15, 0 <target> }
     248:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; bltz r15, 0 <target> }
     250:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; bltz r15, 0 <target> }
     258:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; bltz r15, 0 <target> }
     260:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; bltzt r15, 0 <target> }
     268:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; bltz r15, 0 <target> }
     270:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; bltzt r15, 0 <target> }
     278:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; bltzt r15, 0 <target> }
     280:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; bltz r15, 0 <target> }
     288:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; bltzt r15, 0 <target> }
     290:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; bltzt r15, 0 <target> }
     298:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; bltz r15, 0 <target> }
     2a0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; bltzt r15, 0 <target> }
     2a8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; bltz r15, 0 <target> }
     2b0:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; bltz r15, 0 <target> }
     2b8:	[0-9a-f]* 	{ moveli r5, 4660 ; bgez r15, 0 <target> }
     2c0:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; bnez r15, 0 <target> }
     2c8:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; bnez r15, 0 <target> }
     2d0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; bnez r15, 0 <target> }
     2d8:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; bnez r15, 0 <target> }
     2e0:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; bnezt r15, 0 <target> }
     2e8:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; bnez r15, 0 <target> }
     2f0:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; bnez r15, 0 <target> }
     2f8:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; bnez r15, 0 <target> }
     300:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; bnez r15, 0 <target> }
     308:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; bnez r15, 0 <target> }
     310:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; bnez r15, 0 <target> }
     318:	[0-9a-f]* 	{ revbytes r5, r6 ; blbst r15, 0 <target> }
     320:	[0-9a-f]* 	{ shrs r5, r6, r7 ; blbst r15, 0 <target> }
     328:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; blbs r15, 0 <target> }
     330:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; blbst r15, 0 <target> }
     338:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; blbs r15, 0 <target> }
     340:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; blbs r15, 0 <target> }
     348:	[0-9a-f]* 	{ v4add r5, r6, r7 ; blbs r15, 0 <target> }
     350:	[0-9a-f]* 	{ addx r5, r6, r7 ; blbs r15, 0 <target> }
     358:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; j 0 <target> }
     360:	[0-9a-f]* 	{ nor r5, r6, r7 ; blezt r15, 0 <target> }
     368:	[0-9a-f]* 	{ shl r5, r6, r7 ; blezt r15, 0 <target> }
     370:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; blez r15, 0 <target> }
     378:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; blbs r15, 0 <target> }
     380:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; blbc r15, 0 <target> }
     388:	[0-9a-f]* 	{ and r5, r6, r7 ; bgtz r15, 0 <target> }
     390:	[0-9a-f]* 	{ mz r5, r6, r7 ; blbst r15, 0 <target> }
     398:	[0-9a-f]* 	{ shl r5, r6, r7 ; blbs r15, 0 <target> }
     3a0:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; jal 0 <target> }
     3a8:	[0-9a-f]* 	{ ori r5, r6, 5 ; bgtz r15, 0 <target> }
     3b0:	[0-9a-f]* 	{ infol 4660 ; bgez r15, 0 <target> }
     3b8:	[0-9a-f]* 	{ pcnt r5, r6 ; bnezt r15, 0 <target> }
     3c0:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; j 0 <target> }
     3c8:	[0-9a-f]* 	{ movei r5, 5 ; blbs r15, 0 <target> }
     3d0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; jal 0 <target> }
     3d8:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; jal 0 <target> }
     3e0:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; j 0 <target> }
     3e8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jal 0 <target> }
     3f0:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; j 0 <target> }
     3f8:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; j 0 <target> }
     400:	[0-9a-f]* 	{ and r5, r6, r7 ; j 0 <target> }
     408:	[0-9a-f]* 	{ nop ; blbst r15, 0 <target> }
     410:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; beqzt r15, 0 <target> }
     418:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; beqzt r15, 0 <target> }
     420:	[0-9a-f]* 	{ shli r5, r6, 5 ; beqzt r15, 0 <target> }
     428:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; beqzt r15, 0 <target> }
     430:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; beqzt r15, 0 <target> }
     438:	[0-9a-f]* 	{ addli r5, r6, 4660 ; bgezt r15, 0 <target> }
     440:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; bgezt r15, 0 <target> }
     448:	[0-9a-f]* 	{ mulx r5, r6, r7 ; bgezt r15, 0 <target> }
     450:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; bgezt r15, 0 <target> }
     458:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; bgezt r15, 0 <target> }
     460:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; bgezt r15, 0 <target> }
     468:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; bgtzt r15, 0 <target> }
     470:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; bgtzt r15, 0 <target> }
     478:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; bgtzt r15, 0 <target> }
     480:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; bgtzt r15, 0 <target> }
     488:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; bgtzt r15, 0 <target> }
     490:	[0-9a-f]* 	{ addxi r5, r6, 5 ; blbct r15, 0 <target> }
     498:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; blbct r15, 0 <target> }
     4a0:	[0-9a-f]* 	{ nop ; blbct r15, 0 <target> }
     4a8:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; blbct r15, 0 <target> }
     4b0:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; blbct r15, 0 <target> }
     4b8:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; blbct r15, 0 <target> }
     4c0:	[0-9a-f]* 	{ cmula r5, r6, r7 ; blbst r15, 0 <target> }
     4c8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; blbst r15, 0 <target> }
     4d0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; blbst r15, 0 <target> }
     4d8:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; blbst r15, 0 <target> }
     4e0:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; blbst r15, 0 <target> }
     4e8:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; blezt r15, 0 <target> }
     4f0:	[0-9a-f]* 	{ blezt r15, 0 <target> }
     4f8:	[0-9a-f]* 	{ or r5, r6, r7 ; blezt r15, 0 <target> }
     500:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; blezt r15, 0 <target> }
     508:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; blezt r15, 0 <target> }
     510:	[0-9a-f]* 	{ v4add r5, r6, r7 ; blezt r15, 0 <target> }
     518:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; bltzt r15, 0 <target> }
     520:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; bltzt r15, 0 <target> }
     528:	[0-9a-f]* 	{ shrui r5, r6, 5 ; bltzt r15, 0 <target> }
     530:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; bltzt r15, 0 <target> }
     538:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; bltzt r15, 0 <target> }
     540:	[0-9a-f]* 	{ andi r5, r6, 5 ; bnezt r15, 0 <target> }
     548:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; bnezt r15, 0 <target> }
     550:	[0-9a-f]* 	{ pcnt r5, r6 ; bnezt r15, 0 <target> }
     558:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; bnezt r15, 0 <target> }
     560:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; bnezt r15, 0 <target> }
     568:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; bnezt r15, 0 <target> }
     570:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; beqz r15, 0 <target> }
     578:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; beqz r15, 0 <target> }
     580:	[0-9a-f]* 	{ shrux r5, r6, r7 ; beqz r15, 0 <target> }
     588:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; beqz r15, 0 <target> }
     590:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; beqz r15, 0 <target> }
     598:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; bgez r15, 0 <target> }
     5a0:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; bgez r15, 0 <target> }
     5a8:	[0-9a-f]* 	{ revbits r5, r6 ; bgez r15, 0 <target> }
     5b0:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; bgez r15, 0 <target> }
     5b8:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; bgez r15, 0 <target> }
     5c0:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; bgez r15, 0 <target> }
     5c8:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; bgtz r15, 0 <target> }
     5d0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; bgtz r15, 0 <target> }
     5d8:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; bgtz r15, 0 <target> }
     5e0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; bgtz r15, 0 <target> }
     5e8:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; bgtz r15, 0 <target> }
     5f0:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; blbc r15, 0 <target> }
     5f8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; blbc r15, 0 <target> }
     600:	[0-9a-f]* 	{ rotl r5, r6, r7 ; blbc r15, 0 <target> }
     608:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; blbc r15, 0 <target> }
     610:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; blbc r15, 0 <target> }
     618:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; blbc r15, 0 <target> }
     620:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; blbs r15, 0 <target> }
     628:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; blbs r15, 0 <target> }
     630:	[0-9a-f]* 	{ subx r5, r6, r7 ; blbs r15, 0 <target> }
     638:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; blbs r15, 0 <target> }
     640:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; blbs r15, 0 <target> }
     648:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; blez r15, 0 <target> }
     650:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; blez r15, 0 <target> }
     658:	[0-9a-f]* 	{ shl r5, r6, r7 ; blez r15, 0 <target> }
     660:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; blez r15, 0 <target> }
     668:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; blez r15, 0 <target> }
     670:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; blez r15, 0 <target> }
     678:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; bltz r15, 0 <target> }
     680:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; bltz r15, 0 <target> }
     688:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; bltz r15, 0 <target> }
     690:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; bltz r15, 0 <target> }
     698:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; bltz r15, 0 <target> }
     6a0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; bnez r15, 0 <target> }
     6a8:	[0-9a-f]* 	{ infol 4660 ; bnez r15, 0 <target> }
     6b0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; bnez r15, 0 <target> }
     6b8:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; bnez r15, 0 <target> }
     6c0:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; bnez r15, 0 <target> }
     6c8:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; bnez r15, 0 <target> }
     6d0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jal 0 <target> }
     6d8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jal 0 <target> }
     6e0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jal 0 <target> }
     6e8:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; jal 0 <target> }
     6f0:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; jal 0 <target> }
     6f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; jal 0 <target> }
     700:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; j 0 <target> }
     708:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; j 0 <target> }
     710:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; j 0 <target> }
     718:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; j 0 <target> }
     720:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; j 0 <target> }
     728:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 }
     730:	[0-9a-f]* 	{ fetchand r5, r6, r7 }
     738:	[0-9a-f]* 	{ ldna_add r5, r6, 5 }
     740:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 }
     748:	[0-9a-f]* 	{ shlx r5, r6, r7 }
     750:	[0-9a-f]* 	{ v1avgu r5, r6, r7 }
     758:	[0-9a-f]* 	{ v1subuc r5, r6, r7 }
     760:	[0-9a-f]* 	{ v2shru r5, r6, r7 }
     768:	[0-9a-f]* 	{ add r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
     770:	[0-9a-f]* 	{ add r15, r16, r17 ; addxi r5, r6, 5 ; ld2u r25, r26 }
     778:	[0-9a-f]* 	{ add r15, r16, r17 ; andi r5, r6, 5 ; ld2u r25, r26 }
     780:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
     788:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpeq r5, r6, r7 ; ld4s r25, r26 }
     790:	[0-9a-f]* 	{ add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch r25 }
     798:	[0-9a-f]* 	{ add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l1_fault r25 }
     7a0:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l2_fault r25 }
     7a8:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; ld2s r25, r26 }
     7b0:	[0-9a-f]* 	{ add r15, r16, r17 ; prefetch_l3 r25 }
     7b8:	[0-9a-f]* 	{ add r15, r16, r17 ; info 19 ; prefetch r25 }
     7c0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
     7c8:	[0-9a-f]* 	{ add r15, r16, r17 ; andi r5, r6, 5 ; ld1s r25, r26 }
     7d0:	[0-9a-f]* 	{ add r15, r16, r17 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
     7d8:	[0-9a-f]* 	{ add r15, r16, r17 ; move r5, r6 ; ld1u r25, r26 }
     7e0:	[0-9a-f]* 	{ add r15, r16, r17 ; ld1u r25, r26 }
     7e8:	[0-9a-f]* 	{ revbits r5, r6 ; add r15, r16, r17 ; ld2s r25, r26 }
     7f0:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
     7f8:	[0-9a-f]* 	{ add r15, r16, r17 ; subx r5, r6, r7 ; ld2u r25, r26 }
     800:	[0-9a-f]* 	{ mulx r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
     808:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
     810:	[0-9a-f]* 	{ add r15, r16, r17 ; shli r5, r6, 5 ; ld4u r25, r26 }
     818:	[0-9a-f]* 	{ add r15, r16, r17 ; move r5, r6 ; prefetch r25 }
     820:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     828:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
     830:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
     838:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
     840:	[0-9a-f]* 	{ mulax r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
     848:	[0-9a-f]* 	{ add r15, r16, r17 ; mz r5, r6, r7 ; ld4u r25, r26 }
     850:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; prefetch r25 }
     858:	[0-9a-f]* 	{ pcnt r5, r6 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
     860:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     868:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     870:	[0-9a-f]* 	{ add r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
     878:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
     880:	[0-9a-f]* 	{ add r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l2 r25 }
     888:	[0-9a-f]* 	{ add r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l2 r25 }
     890:	[0-9a-f]* 	{ add r15, r16, r17 ; prefetch_l2_fault r25 }
     898:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
     8a0:	[0-9a-f]* 	{ add r15, r16, r17 ; nop ; prefetch_l3 r25 }
     8a8:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l3_fault r25 }
     8b0:	[0-9a-f]* 	{ add r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l3_fault r25 }
     8b8:	[0-9a-f]* 	{ revbytes r5, r6 ; add r15, r16, r17 ; prefetch_l2 r25 }
     8c0:	[0-9a-f]* 	{ add r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
     8c8:	[0-9a-f]* 	{ add r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3_fault r25 }
     8d0:	[0-9a-f]* 	{ add r15, r16, r17 ; shl2add r5, r6, r7 ; st1 r25, r26 }
     8d8:	[0-9a-f]* 	{ add r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
     8e0:	[0-9a-f]* 	{ add r15, r16, r17 ; shlx r5, r6, r7 }
     8e8:	[0-9a-f]* 	{ add r15, r16, r17 ; shru r5, r6, r7 ; ld r25, r26 }
     8f0:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; add r15, r16, r17 }
     8f8:	[0-9a-f]* 	{ revbits r5, r6 ; add r15, r16, r17 ; st r25, r26 }
     900:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpne r5, r6, r7 ; st1 r25, r26 }
     908:	[0-9a-f]* 	{ add r15, r16, r17 ; subx r5, r6, r7 ; st1 r25, r26 }
     910:	[0-9a-f]* 	{ mulx r5, r6, r7 ; add r15, r16, r17 ; st2 r25, r26 }
     918:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpeqi r5, r6, 5 ; st4 r25, r26 }
     920:	[0-9a-f]* 	{ add r15, r16, r17 ; shli r5, r6, 5 ; st4 r25, r26 }
     928:	[0-9a-f]* 	{ add r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
     930:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
     938:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
     940:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; add r15, r16, r17 }
     948:	[0-9a-f]* 	{ add r15, r16, r17 ; v2packh r5, r6, r7 }
     950:	[0-9a-f]* 	{ add r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
     958:	[0-9a-f]* 	{ add r5, r6, r7 ; addi r15, r16, 5 ; st r25, r26 }
     960:	[0-9a-f]* 	{ add r5, r6, r7 ; addxi r15, r16, 5 ; st1 r25, r26 }
     968:	[0-9a-f]* 	{ add r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
     970:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
     978:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
     980:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
     988:	[0-9a-f]* 	{ add r5, r6, r7 ; dblalign4 r15, r16, r17 }
     990:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; ld2u r25, r26 }
     998:	[0-9a-f]* 	{ add r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
     9a0:	[0-9a-f]* 	{ add r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
     9a8:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpeq r15, r16, r17 ; ld r25, r26 }
     9b0:	[0-9a-f]* 	{ add r5, r6, r7 ; ld r25, r26 }
     9b8:	[0-9a-f]* 	{ add r5, r6, r7 ; shli r15, r16, 5 ; ld1s r25, r26 }
     9c0:	[0-9a-f]* 	{ add r5, r6, r7 ; rotl r15, r16, r17 ; ld1u r25, r26 }
     9c8:	[0-9a-f]* 	{ add r5, r6, r7 ; jrp r15 ; ld2s r25, r26 }
     9d0:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
     9d8:	[0-9a-f]* 	{ add r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
     9e0:	[0-9a-f]* 	{ add r5, r6, r7 ; shrui r15, r16, 5 ; ld4s r25, r26 }
     9e8:	[0-9a-f]* 	{ add r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
     9f0:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; prefetch r25 }
     9f8:	[0-9a-f]* 	{ add r5, r6, r7 ; move r15, r16 ; prefetch r25 }
     a00:	[0-9a-f]* 	{ add r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
     a08:	[0-9a-f]* 	{ add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2 r25 }
     a10:	[0-9a-f]* 	{ add r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
     a18:	[0-9a-f]* 	{ add r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
     a20:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
     a28:	[0-9a-f]* 	{ add r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
     a30:	[0-9a-f]* 	{ add r5, r6, r7 ; jrp r15 ; prefetch_l2 r25 }
     a38:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
     a40:	[0-9a-f]* 	{ add r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3 r25 }
     a48:	[0-9a-f]* 	{ add r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
     a50:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
     a58:	[0-9a-f]* 	{ add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
     a60:	[0-9a-f]* 	{ add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
     a68:	[0-9a-f]* 	{ add r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
     a70:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; st r25, r26 }
     a78:	[0-9a-f]* 	{ add r5, r6, r7 ; shli r15, r16, 5 ; st2 r25, r26 }
     a80:	[0-9a-f]* 	{ add r5, r6, r7 ; shrsi r15, r16, 5 ; st2 r25, r26 }
     a88:	[0-9a-f]* 	{ add r5, r6, r7 ; shrui r15, r16, 5 }
     a90:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; st r25, r26 }
     a98:	[0-9a-f]* 	{ add r5, r6, r7 ; or r15, r16, r17 ; st1 r25, r26 }
     aa0:	[0-9a-f]* 	{ add r5, r6, r7 ; jr r15 ; st2 r25, r26 }
     aa8:	[0-9a-f]* 	{ add r5, r6, r7 ; cmplts r15, r16, r17 ; st4 r25, r26 }
     ab0:	[0-9a-f]* 	{ add r5, r6, r7 ; stnt1 r15, r16 }
     ab8:	[0-9a-f]* 	{ add r5, r6, r7 ; subx r15, r16, r17 ; st r25, r26 }
     ac0:	[0-9a-f]* 	{ add r5, r6, r7 ; v2cmpleu r15, r16, r17 }
     ac8:	[0-9a-f]* 	{ add r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
     ad0:	[0-9a-f]* 	{ addi r15, r16, 5 ; addi r5, r6, 5 ; ld2s r25, r26 }
     ad8:	[0-9a-f]* 	{ addi r15, r16, 5 ; addxi r5, r6, 5 ; ld2u r25, r26 }
     ae0:	[0-9a-f]* 	{ addi r15, r16, 5 ; andi r5, r6, 5 ; ld2u r25, r26 }
     ae8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addi r15, r16, 5 ; ld2s r25, r26 }
     af0:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpeq r5, r6, r7 ; ld4s r25, r26 }
     af8:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch r25 }
     b00:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l1_fault r25 }
     b08:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l2_fault r25 }
     b10:	[0-9a-f]* 	{ ctz r5, r6 ; addi r15, r16, 5 ; ld2s r25, r26 }
     b18:	[0-9a-f]* 	{ addi r15, r16, 5 ; prefetch_l3 r25 }
     b20:	[0-9a-f]* 	{ addi r15, r16, 5 ; info 19 ; prefetch r25 }
     b28:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; ld r25, r26 }
     b30:	[0-9a-f]* 	{ addi r15, r16, 5 ; andi r5, r6, 5 ; ld1s r25, r26 }
     b38:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
     b40:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; ld1u r25, r26 }
     b48:	[0-9a-f]* 	{ addi r15, r16, 5 ; ld1u r25, r26 }
     b50:	[0-9a-f]* 	{ revbits r5, r6 ; addi r15, r16, 5 ; ld2s r25, r26 }
     b58:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
     b60:	[0-9a-f]* 	{ addi r15, r16, 5 ; subx r5, r6, r7 ; ld2u r25, r26 }
     b68:	[0-9a-f]* 	{ mulx r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
     b70:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
     b78:	[0-9a-f]* 	{ addi r15, r16, 5 ; shli r5, r6, 5 ; ld4u r25, r26 }
     b80:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; prefetch r25 }
     b88:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
     b90:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
     b98:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; ld4u r25, r26 }
     ba0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; ld2s r25, r26 }
     ba8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addi r15, r16, 5 ; ld2u r25, r26 }
     bb0:	[0-9a-f]* 	{ addi r15, r16, 5 ; mz r5, r6, r7 ; ld4u r25, r26 }
     bb8:	[0-9a-f]* 	{ addi r15, r16, 5 ; nor r5, r6, r7 ; prefetch r25 }
     bc0:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
     bc8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
     bd0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
     bd8:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch r25 }
     be0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
     be8:	[0-9a-f]* 	{ addi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l2 r25 }
     bf0:	[0-9a-f]* 	{ addi r15, r16, 5 ; rotl r5, r6, r7 ; prefetch_l2 r25 }
     bf8:	[0-9a-f]* 	{ addi r15, r16, 5 ; prefetch_l2_fault r25 }
     c00:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; prefetch_l2_fault r25 }
     c08:	[0-9a-f]* 	{ addi r15, r16, 5 ; nop ; prefetch_l3 r25 }
     c10:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpleu r5, r6, r7 ; prefetch_l3_fault r25 }
     c18:	[0-9a-f]* 	{ addi r15, r16, 5 ; shrsi r5, r6, 5 ; prefetch_l3_fault r25 }
     c20:	[0-9a-f]* 	{ revbytes r5, r6 ; addi r15, r16, 5 ; prefetch_l2 r25 }
     c28:	[0-9a-f]* 	{ addi r15, r16, 5 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
     c30:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch_l3_fault r25 }
     c38:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl2add r5, r6, r7 ; st1 r25, r26 }
     c40:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl3add r5, r6, r7 ; st4 r25, r26 }
     c48:	[0-9a-f]* 	{ addi r15, r16, 5 ; shlx r5, r6, r7 }
     c50:	[0-9a-f]* 	{ addi r15, r16, 5 ; shru r5, r6, r7 ; ld r25, r26 }
     c58:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; addi r15, r16, 5 }
     c60:	[0-9a-f]* 	{ revbits r5, r6 ; addi r15, r16, 5 ; st r25, r26 }
     c68:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpne r5, r6, r7 ; st1 r25, r26 }
     c70:	[0-9a-f]* 	{ addi r15, r16, 5 ; subx r5, r6, r7 ; st1 r25, r26 }
     c78:	[0-9a-f]* 	{ mulx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
     c80:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpeqi r5, r6, 5 ; st4 r25, r26 }
     c88:	[0-9a-f]* 	{ addi r15, r16, 5 ; shli r5, r6, 5 ; st4 r25, r26 }
     c90:	[0-9a-f]* 	{ addi r15, r16, 5 ; subx r5, r6, r7 ; prefetch r25 }
     c98:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
     ca0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addi r15, r16, 5 ; prefetch_l2_fault r25 }
     ca8:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; addi r15, r16, 5 }
     cb0:	[0-9a-f]* 	{ addi r15, r16, 5 ; v2packh r5, r6, r7 }
     cb8:	[0-9a-f]* 	{ addi r15, r16, 5 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
     cc0:	[0-9a-f]* 	{ addi r5, r6, 5 ; addi r15, r16, 5 ; st r25, r26 }
     cc8:	[0-9a-f]* 	{ addi r5, r6, 5 ; addxi r15, r16, 5 ; st1 r25, r26 }
     cd0:	[0-9a-f]* 	{ addi r5, r6, 5 ; andi r15, r16, 5 ; st1 r25, r26 }
     cd8:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
     ce0:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
     ce8:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld r25, r26 }
     cf0:	[0-9a-f]* 	{ addi r5, r6, 5 ; dblalign4 r15, r16, r17 }
     cf8:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; ld2u r25, r26 }
     d00:	[0-9a-f]* 	{ addi r5, r6, 5 ; jalr r15 ; ld2s r25, r26 }
     d08:	[0-9a-f]* 	{ addi r5, r6, 5 ; jr r15 ; ld4s r25, r26 }
     d10:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld r25, r26 }
     d18:	[0-9a-f]* 	{ addi r5, r6, 5 ; ld r25, r26 }
     d20:	[0-9a-f]* 	{ addi r5, r6, 5 ; shli r15, r16, 5 ; ld1s r25, r26 }
     d28:	[0-9a-f]* 	{ addi r5, r6, 5 ; rotl r15, r16, r17 ; ld1u r25, r26 }
     d30:	[0-9a-f]* 	{ addi r5, r6, 5 ; jrp r15 ; ld2s r25, r26 }
     d38:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
     d40:	[0-9a-f]* 	{ addi r5, r6, 5 ; addx r15, r16, r17 ; ld4s r25, r26 }
     d48:	[0-9a-f]* 	{ addi r5, r6, 5 ; shrui r15, r16, 5 ; ld4s r25, r26 }
     d50:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
     d58:	[0-9a-f]* 	{ addi r5, r6, 5 ; lnk r15 ; prefetch r25 }
     d60:	[0-9a-f]* 	{ addi r5, r6, 5 ; move r15, r16 ; prefetch r25 }
     d68:	[0-9a-f]* 	{ addi r5, r6, 5 ; mz r15, r16, r17 ; prefetch r25 }
     d70:	[0-9a-f]* 	{ addi r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l2 r25 }
     d78:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch r25 }
     d80:	[0-9a-f]* 	{ addi r5, r6, 5 ; prefetch_add_l2_fault r15, 5 }
     d88:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch r25 }
     d90:	[0-9a-f]* 	{ addi r5, r6, 5 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
     d98:	[0-9a-f]* 	{ addi r5, r6, 5 ; jrp r15 ; prefetch_l2 r25 }
     da0:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
     da8:	[0-9a-f]* 	{ addi r5, r6, 5 ; and r15, r16, r17 ; prefetch_l3 r25 }
     db0:	[0-9a-f]* 	{ addi r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l3 r25 }
     db8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
     dc0:	[0-9a-f]* 	{ addi r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
     dc8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
     dd0:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
     dd8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; st r25, r26 }
     de0:	[0-9a-f]* 	{ addi r5, r6, 5 ; shli r15, r16, 5 ; st2 r25, r26 }
     de8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shrsi r15, r16, 5 ; st2 r25, r26 }
     df0:	[0-9a-f]* 	{ addi r5, r6, 5 ; shrui r15, r16, 5 }
     df8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; st r25, r26 }
     e00:	[0-9a-f]* 	{ addi r5, r6, 5 ; or r15, r16, r17 ; st1 r25, r26 }
     e08:	[0-9a-f]* 	{ addi r5, r6, 5 ; jr r15 ; st2 r25, r26 }
     e10:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmplts r15, r16, r17 ; st4 r25, r26 }
     e18:	[0-9a-f]* 	{ addi r5, r6, 5 ; stnt1 r15, r16 }
     e20:	[0-9a-f]* 	{ addi r5, r6, 5 ; subx r15, r16, r17 ; st r25, r26 }
     e28:	[0-9a-f]* 	{ addi r5, r6, 5 ; v2cmpleu r15, r16, r17 }
     e30:	[0-9a-f]* 	{ addi r5, r6, 5 ; xor r15, r16, r17 ; ld1u r25, r26 }
     e38:	[0-9a-f]* 	{ addli r15, r16, 4660 ; cmpltui r5, r6, 5 }
     e40:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; addli r15, r16, 4660 }
     e48:	[0-9a-f]* 	{ addli r15, r16, 4660 ; shlx r5, r6, r7 }
     e50:	[0-9a-f]* 	{ addli r15, r16, 4660 ; v1int_h r5, r6, r7 }
     e58:	[0-9a-f]* 	{ addli r15, r16, 4660 ; v2maxsi r5, r6, 5 }
     e60:	[0-9a-f]* 	{ addli r5, r6, 4660 ; addx r15, r16, r17 }
     e68:	[0-9a-f]* 	{ addli r5, r6, 4660 ; iret }
     e70:	[0-9a-f]* 	{ addli r5, r6, 4660 ; movei r15, 5 }
     e78:	[0-9a-f]* 	{ addli r5, r6, 4660 ; shruxi r15, r16, 5 }
     e80:	[0-9a-f]* 	{ addli r5, r6, 4660 ; v1shl r15, r16, r17 }
     e88:	[0-9a-f]* 	{ addli r5, r6, 4660 ; v4add r15, r16, r17 }
     e90:	[0-9a-f]* 	{ addx r15, r16, r17 ; addi r5, r6, 5 ; prefetch r25 }
     e98:	[0-9a-f]* 	{ addx r15, r16, r17 ; addxi r5, r6, 5 ; prefetch r25 }
     ea0:	[0-9a-f]* 	{ addx r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
     ea8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
     eb0:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l1_fault r25 }
     eb8:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l2_fault r25 }
     ec0:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l3_fault r25 }
     ec8:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
     ed0:	[0-9a-f]* 	{ ctz r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
     ed8:	[0-9a-f]* 	{ addx r15, r16, r17 ; st2 r25, r26 }
     ee0:	[0-9a-f]* 	{ addx r15, r16, r17 ; info 19 ; prefetch_l3 r25 }
     ee8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; ld r25, r26 }
     ef0:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpeq r5, r6, r7 ; ld1s r25, r26 }
     ef8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1s r25, r26 }
     f00:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
     f08:	[0-9a-f]* 	{ addx r15, r16, r17 ; addxi r5, r6, 5 ; ld2s r25, r26 }
     f10:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl r5, r6, r7 ; ld2s r25, r26 }
     f18:	[0-9a-f]* 	{ addx r15, r16, r17 ; info 19 ; ld2u r25, r26 }
     f20:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; ld2u r25, r26 }
     f28:	[0-9a-f]* 	{ addx r15, r16, r17 ; or r5, r6, r7 ; ld4s r25, r26 }
     f30:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld4u r25, r26 }
     f38:	[0-9a-f]* 	{ addx r15, r16, r17 ; shrui r5, r6, 5 ; ld4u r25, r26 }
     f40:	[0-9a-f]* 	{ addx r15, r16, r17 ; move r5, r6 ; prefetch_l2_fault r25 }
     f48:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
     f50:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
     f58:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2 r25 }
     f60:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
     f68:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
     f70:	[0-9a-f]* 	{ addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l2 r25 }
     f78:	[0-9a-f]* 	{ addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l3 r25 }
     f80:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
     f88:	[0-9a-f]* 	{ addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
     f90:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch r25 }
     f98:	[0-9a-f]* 	{ addx r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
     fa0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
     fa8:	[0-9a-f]* 	{ addx r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l2 r25 }
     fb0:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l2 r25 }
     fb8:	[0-9a-f]* 	{ addx r15, r16, r17 ; move r5, r6 ; prefetch_l2_fault r25 }
     fc0:	[0-9a-f]* 	{ addx r15, r16, r17 ; prefetch_l2_fault r25 }
     fc8:	[0-9a-f]* 	{ revbits r5, r6 ; addx r15, r16, r17 ; prefetch_l3 r25 }
     fd0:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l3_fault r25 }
     fd8:	[0-9a-f]* 	{ addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l3_fault r25 }
     fe0:	[0-9a-f]* 	{ revbytes r5, r6 ; addx r15, r16, r17 ; st r25, r26 }
     fe8:	[0-9a-f]* 	{ addx r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
     ff0:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl1add r5, r6, r7 ; st4 r25, r26 }
     ff8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl2addx r5, r6, r7 ; ld r25, r26 }
    1000:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
    1008:	[0-9a-f]* 	{ addx r15, r16, r17 ; shrs r5, r6, r7 ; ld1u r25, r26 }
    1010:	[0-9a-f]* 	{ addx r15, r16, r17 ; shru r5, r6, r7 ; ld2u r25, r26 }
    1018:	[0-9a-f]* 	{ addx r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
    1020:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl r5, r6, r7 ; st r25, r26 }
    1028:	[0-9a-f]* 	{ addx r15, r16, r17 ; info 19 ; st1 r25, r26 }
    1030:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; st1 r25, r26 }
    1038:	[0-9a-f]* 	{ addx r15, r16, r17 ; or r5, r6, r7 ; st2 r25, r26 }
    1040:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; st4 r25, r26 }
    1048:	[0-9a-f]* 	{ addx r15, r16, r17 ; shrui r5, r6, 5 ; st4 r25, r26 }
    1050:	[0-9a-f]* 	{ addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l3 r25 }
    1058:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
    1060:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; st1 r25, r26 }
    1068:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; addx r15, r16, r17 }
    1070:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; addx r15, r16, r17 }
    1078:	[0-9a-f]* 	{ addx r15, r16, r17 ; xor r5, r6, r7 ; st4 r25, r26 }
    1080:	[0-9a-f]* 	{ addx r5, r6, r7 ; addi r15, r16, 5 }
    1088:	[0-9a-f]* 	{ addx r5, r6, r7 ; addxli r15, r16, 4660 }
    1090:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmpeq r15, r16, r17 ; ld r25, r26 }
    1098:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    10a0:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    10a8:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld2u r25, r26 }
    10b0:	[0-9a-f]* 	{ addx r5, r6, r7 ; exch4 r15, r16, r17 }
    10b8:	[0-9a-f]* 	{ addx r5, r6, r7 ; ill ; prefetch r25 }
    10c0:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalr r15 ; prefetch r25 }
    10c8:	[0-9a-f]* 	{ addx r5, r6, r7 ; jr r15 ; prefetch_l1_fault r25 }
    10d0:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    10d8:	[0-9a-f]* 	{ addx r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
    10e0:	[0-9a-f]* 	{ addx r5, r6, r7 ; shrui r15, r16, 5 ; ld1s r25, r26 }
    10e8:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    10f0:	[0-9a-f]* 	{ addx r5, r6, r7 ; movei r15, 5 ; ld2s r25, r26 }
    10f8:	[0-9a-f]* 	{ addx r5, r6, r7 ; ill ; ld2u r25, r26 }
    1100:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    1108:	[0-9a-f]* 	{ addx r5, r6, r7 ; ld4s r25, r26 }
    1110:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    1118:	[0-9a-f]* 	{ addx r5, r6, r7 ; lnk r15 ; prefetch_l3 r25 }
    1120:	[0-9a-f]* 	{ addx r5, r6, r7 ; move r15, r16 ; prefetch_l3 r25 }
    1128:	[0-9a-f]* 	{ addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    1130:	[0-9a-f]* 	{ addx r5, r6, r7 ; nor r15, r16, r17 ; st r25, r26 }
    1138:	[0-9a-f]* 	{ addx r5, r6, r7 ; prefetch r25 }
    1140:	[0-9a-f]* 	{ addx r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
    1148:	[0-9a-f]* 	{ addx r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch r25 }
    1150:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    1158:	[0-9a-f]* 	{ addx r5, r6, r7 ; movei r15, 5 ; prefetch_l2 r25 }
    1160:	[0-9a-f]* 	{ addx r5, r6, r7 ; info 19 ; prefetch_l2_fault r25 }
    1168:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    1170:	[0-9a-f]* 	{ addx r5, r6, r7 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
    1178:	[0-9a-f]* 	{ addx r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    1180:	[0-9a-f]* 	{ addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
    1188:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl1add r15, r16, r17 ; st r25, r26 }
    1190:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl2add r15, r16, r17 ; st2 r25, r26 }
    1198:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl3add r15, r16, r17 }
    11a0:	[0-9a-f]* 	{ addx r5, r6, r7 ; shlxi r15, r16, 5 }
    11a8:	[0-9a-f]* 	{ addx r5, r6, r7 ; shru r15, r16, r17 ; ld1s r25, r26 }
    11b0:	[0-9a-f]* 	{ addx r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
    11b8:	[0-9a-f]* 	{ addx r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
    11c0:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl1add r15, r16, r17 ; st1 r25, r26 }
    11c8:	[0-9a-f]* 	{ addx r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
    11d0:	[0-9a-f]* 	{ addx r5, r6, r7 ; st4 r25, r26 }
    11d8:	[0-9a-f]* 	{ addx r5, r6, r7 ; stnt4 r15, r16 }
    11e0:	[0-9a-f]* 	{ addx r5, r6, r7 ; subx r15, r16, r17 }
    11e8:	[0-9a-f]* 	{ addx r5, r6, r7 ; v2cmpltui r15, r16, 5 }
    11f0:	[0-9a-f]* 	{ addx r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
    11f8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; addi r5, r6, 5 ; prefetch r25 }
    1200:	[0-9a-f]* 	{ addxi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch r25 }
    1208:	[0-9a-f]* 	{ addxi r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    1210:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    1218:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l1_fault r25 }
    1220:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l2_fault r25 }
    1228:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l3_fault r25 }
    1230:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
    1238:	[0-9a-f]* 	{ ctz r5, r6 ; addxi r15, r16, 5 ; prefetch r25 }
    1240:	[0-9a-f]* 	{ addxi r15, r16, 5 ; st2 r25, r26 }
    1248:	[0-9a-f]* 	{ addxi r15, r16, 5 ; info 19 ; prefetch_l3 r25 }
    1250:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    1258:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpeq r5, r6, r7 ; ld1s r25, r26 }
    1260:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld1s r25, r26 }
    1268:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; ld1u r25, r26 }
    1270:	[0-9a-f]* 	{ addxi r15, r16, 5 ; addxi r5, r6, 5 ; ld2s r25, r26 }
    1278:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl r5, r6, r7 ; ld2s r25, r26 }
    1280:	[0-9a-f]* 	{ addxi r15, r16, 5 ; info 19 ; ld2u r25, r26 }
    1288:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addxi r15, r16, 5 ; ld2u r25, r26 }
    1290:	[0-9a-f]* 	{ addxi r15, r16, 5 ; or r5, r6, r7 ; ld4s r25, r26 }
    1298:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld4u r25, r26 }
    12a0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrui r5, r6, 5 ; ld4u r25, r26 }
    12a8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; move r5, r6 ; prefetch_l2_fault r25 }
    12b0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3 r25 }
    12b8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    12c0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    12c8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    12d0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    12d8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l2 r25 }
    12e0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l3 r25 }
    12e8:	[0-9a-f]* 	{ pcnt r5, r6 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    12f0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; mz r5, r6, r7 ; prefetch r25 }
    12f8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch r25 }
    1300:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch r25 }
    1308:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    1310:	[0-9a-f]* 	{ addxi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l2 r25 }
    1318:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l2 r25 }
    1320:	[0-9a-f]* 	{ addxi r15, r16, 5 ; move r5, r6 ; prefetch_l2_fault r25 }
    1328:	[0-9a-f]* 	{ addxi r15, r16, 5 ; prefetch_l2_fault r25 }
    1330:	[0-9a-f]* 	{ revbits r5, r6 ; addxi r15, r16, 5 ; prefetch_l3 r25 }
    1338:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpne r5, r6, r7 ; prefetch_l3_fault r25 }
    1340:	[0-9a-f]* 	{ addxi r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l3_fault r25 }
    1348:	[0-9a-f]* 	{ revbytes r5, r6 ; addxi r15, r16, 5 ; st r25, r26 }
    1350:	[0-9a-f]* 	{ addxi r15, r16, 5 ; rotli r5, r6, 5 ; st2 r25, r26 }
    1358:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl1add r5, r6, r7 ; st4 r25, r26 }
    1360:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld r25, r26 }
    1368:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
    1370:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrs r5, r6, r7 ; ld1u r25, r26 }
    1378:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shru r5, r6, r7 ; ld2u r25, r26 }
    1380:	[0-9a-f]* 	{ addxi r15, r16, 5 ; addxi r5, r6, 5 ; st r25, r26 }
    1388:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl r5, r6, r7 ; st r25, r26 }
    1390:	[0-9a-f]* 	{ addxi r15, r16, 5 ; info 19 ; st1 r25, r26 }
    1398:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addxi r15, r16, 5 ; st1 r25, r26 }
    13a0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; or r5, r6, r7 ; st2 r25, r26 }
    13a8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpltsi r5, r6, 5 ; st4 r25, r26 }
    13b0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrui r5, r6, 5 ; st4 r25, r26 }
    13b8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l3 r25 }
    13c0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    13c8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addxi r15, r16, 5 ; st1 r25, r26 }
    13d0:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; addxi r15, r16, 5 }
    13d8:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; addxi r15, r16, 5 }
    13e0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; xor r5, r6, r7 ; st4 r25, r26 }
    13e8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; addi r15, r16, 5 }
    13f0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; addxli r15, r16, 4660 }
    13f8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld r25, r26 }
    1400:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmples r15, r16, r17 ; ld r25, r26 }
    1408:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    1410:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld2u r25, r26 }
    1418:	[0-9a-f]* 	{ addxi r5, r6, 5 ; exch4 r15, r16, r17 }
    1420:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ill ; prefetch r25 }
    1428:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jalr r15 ; prefetch r25 }
    1430:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jr r15 ; prefetch_l1_fault r25 }
    1438:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmplts r15, r16, r17 ; ld r25, r26 }
    1440:	[0-9a-f]* 	{ addxi r5, r6, 5 ; addx r15, r16, r17 ; ld1s r25, r26 }
    1448:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shrui r15, r16, 5 ; ld1s r25, r26 }
    1450:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    1458:	[0-9a-f]* 	{ addxi r5, r6, 5 ; movei r15, 5 ; ld2s r25, r26 }
    1460:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ill ; ld2u r25, r26 }
    1468:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    1470:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ld4s r25, r26 }
    1478:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    1480:	[0-9a-f]* 	{ addxi r5, r6, 5 ; lnk r15 ; prefetch_l3 r25 }
    1488:	[0-9a-f]* 	{ addxi r5, r6, 5 ; move r15, r16 ; prefetch_l3 r25 }
    1490:	[0-9a-f]* 	{ addxi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    1498:	[0-9a-f]* 	{ addxi r5, r6, 5 ; nor r15, r16, r17 ; st r25, r26 }
    14a0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; prefetch r25 }
    14a8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; add r15, r16, r17 ; prefetch r25 }
    14b0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shrsi r15, r16, 5 ; prefetch r25 }
    14b8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    14c0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; movei r15, 5 ; prefetch_l2 r25 }
    14c8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; info 19 ; prefetch_l2_fault r25 }
    14d0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    14d8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
    14e0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    14e8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
    14f0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1add r15, r16, r17 ; st r25, r26 }
    14f8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl2add r15, r16, r17 ; st2 r25, r26 }
    1500:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl3add r15, r16, r17 }
    1508:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shlxi r15, r16, 5 }
    1510:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shru r15, r16, r17 ; ld1s r25, r26 }
    1518:	[0-9a-f]* 	{ addxi r5, r6, 5 ; add r15, r16, r17 ; st r25, r26 }
    1520:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shrsi r15, r16, 5 ; st r25, r26 }
    1528:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1add r15, r16, r17 ; st1 r25, r26 }
    1530:	[0-9a-f]* 	{ addxi r5, r6, 5 ; move r15, r16 ; st2 r25, r26 }
    1538:	[0-9a-f]* 	{ addxi r5, r6, 5 ; st4 r25, r26 }
    1540:	[0-9a-f]* 	{ addxi r5, r6, 5 ; stnt4 r15, r16 }
    1548:	[0-9a-f]* 	{ addxi r5, r6, 5 ; subx r15, r16, r17 }
    1550:	[0-9a-f]* 	{ addxi r5, r6, 5 ; v2cmpltui r15, r16, 5 }
    1558:	[0-9a-f]* 	{ addxi r5, r6, 5 ; xor r15, r16, r17 ; ld4u r25, r26 }
    1560:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; addxli r15, r16, 4660 }
    1568:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; addxli r15, r16, 4660 }
    1570:	[0-9a-f]* 	{ addxli r15, r16, 4660 ; shru r5, r6, r7 }
    1578:	[0-9a-f]* 	{ addxli r15, r16, 4660 ; v1minu r5, r6, r7 }
    1580:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; addxli r15, r16, 4660 }
    1588:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; and r15, r16, r17 }
    1590:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; jrp r15 }
    1598:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; nop }
    15a0:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; st2 r15, r16 }
    15a8:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; v1shru r15, r16, r17 }
    15b0:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; v4packsc r15, r16, r17 }
    15b8:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; addxsc r15, r16, r17 }
    15c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addxsc r15, r16, r17 }
    15c8:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; addxsc r15, r16, r17 }
    15d0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; addxsc r15, r16, r17 }
    15d8:	[0-9a-f]* 	{ addxsc r15, r16, r17 ; v2packh r5, r6, r7 }
    15e0:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; cmpexch r15, r16, r17 }
    15e8:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; ld1u r15, r16 }
    15f0:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; prefetch r15 }
    15f8:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; st_add r15, r16, 5 }
    1600:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; v2add r15, r16, r17 }
    1608:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; v4shru r15, r16, r17 }
    1610:	[0-9a-f]* 	{ and r15, r16, r17 ; addi r5, r6, 5 ; st1 r25, r26 }
    1618:	[0-9a-f]* 	{ and r15, r16, r17 ; addxi r5, r6, 5 ; st2 r25, r26 }
    1620:	[0-9a-f]* 	{ and r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
    1628:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
    1630:	[0-9a-f]* 	{ and r15, r16, r17 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
    1638:	[0-9a-f]* 	{ and r15, r16, r17 ; cmpleu r5, r6, r7 ; ld r25, r26 }
    1640:	[0-9a-f]* 	{ and r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1u r25, r26 }
    1648:	[0-9a-f]* 	{ and r15, r16, r17 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
    1650:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; st1 r25, r26 }
    1658:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    1660:	[0-9a-f]* 	{ and r15, r16, r17 ; add r5, r6, r7 ; ld r25, r26 }
    1668:	[0-9a-f]* 	{ revbytes r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
    1670:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    1678:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    1680:	[0-9a-f]* 	{ and r15, r16, r17 ; mz r5, r6, r7 ; ld1u r25, r26 }
    1688:	[0-9a-f]* 	{ and r15, r16, r17 ; cmples r5, r6, r7 ; ld2s r25, r26 }
    1690:	[0-9a-f]* 	{ and r15, r16, r17 ; shrs r5, r6, r7 ; ld2s r25, r26 }
    1698:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
    16a0:	[0-9a-f]* 	{ and r15, r16, r17 ; andi r5, r6, 5 ; ld4s r25, r26 }
    16a8:	[0-9a-f]* 	{ and r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    16b0:	[0-9a-f]* 	{ and r15, r16, r17 ; move r5, r6 ; ld4u r25, r26 }
    16b8:	[0-9a-f]* 	{ and r15, r16, r17 ; ld4u r25, r26 }
    16c0:	[0-9a-f]* 	{ and r15, r16, r17 ; movei r5, 5 ; ld r25, r26 }
    16c8:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; and r15, r16, r17 }
    16d0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
    16d8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 }
    16e0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
    16e8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
    16f0:	[0-9a-f]* 	{ and r15, r16, r17 ; mz r5, r6, r7 }
    16f8:	[0-9a-f]* 	{ and r15, r16, r17 ; or r5, r6, r7 ; ld1s r25, r26 }
    1700:	[0-9a-f]* 	{ and r15, r16, r17 ; addx r5, r6, r7 ; prefetch r25 }
    1708:	[0-9a-f]* 	{ and r15, r16, r17 ; rotli r5, r6, 5 ; prefetch r25 }
    1710:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
    1718:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
    1720:	[0-9a-f]* 	{ and r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
    1728:	[0-9a-f]* 	{ and r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2 r25 }
    1730:	[0-9a-f]* 	{ and r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    1738:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    1740:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3 r25 }
    1748:	[0-9a-f]* 	{ and r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l3 r25 }
    1750:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3_fault r25 }
    1758:	[0-9a-f]* 	{ revbits r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    1760:	[0-9a-f]* 	{ and r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    1768:	[0-9a-f]* 	{ and r15, r16, r17 ; shl r5, r6, r7 ; ld4s r25, r26 }
    1770:	[0-9a-f]* 	{ and r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4u r25, r26 }
    1778:	[0-9a-f]* 	{ and r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    1780:	[0-9a-f]* 	{ and r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2 r25 }
    1788:	[0-9a-f]* 	{ and r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    1790:	[0-9a-f]* 	{ and r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3 r25 }
    1798:	[0-9a-f]* 	{ and r15, r16, r17 ; cmples r5, r6, r7 ; st r25, r26 }
    17a0:	[0-9a-f]* 	{ and r15, r16, r17 ; shrs r5, r6, r7 ; st r25, r26 }
    17a8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
    17b0:	[0-9a-f]* 	{ and r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
    17b8:	[0-9a-f]* 	{ and r15, r16, r17 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
    17c0:	[0-9a-f]* 	{ and r15, r16, r17 ; move r5, r6 ; st4 r25, r26 }
    17c8:	[0-9a-f]* 	{ and r15, r16, r17 ; st4 r25, r26 }
    17d0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
    17d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; ld1u r25, r26 }
    17e0:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; and r15, r16, r17 }
    17e8:	[0-9a-f]* 	{ and r15, r16, r17 ; v1subuc r5, r6, r7 }
    17f0:	[0-9a-f]* 	{ and r15, r16, r17 ; v2shru r5, r6, r7 }
    17f8:	[0-9a-f]* 	{ and r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
    1800:	[0-9a-f]* 	{ and r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    1808:	[0-9a-f]* 	{ and r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    1810:	[0-9a-f]* 	{ and r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    1818:	[0-9a-f]* 	{ and r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    1820:	[0-9a-f]* 	{ and r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    1828:	[0-9a-f]* 	{ and r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    1830:	[0-9a-f]* 	{ and r5, r6, r7 ; fetchor4 r15, r16, r17 }
    1838:	[0-9a-f]* 	{ and r5, r6, r7 ; ill ; st2 r25, r26 }
    1840:	[0-9a-f]* 	{ and r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
    1848:	[0-9a-f]* 	{ and r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    1850:	[0-9a-f]* 	{ and r5, r6, r7 ; jalrp r15 ; ld r25, r26 }
    1858:	[0-9a-f]* 	{ and r5, r6, r7 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    1860:	[0-9a-f]* 	{ and r5, r6, r7 ; addi r15, r16, 5 ; ld1u r25, r26 }
    1868:	[0-9a-f]* 	{ and r5, r6, r7 ; shru r15, r16, r17 ; ld1u r25, r26 }
    1870:	[0-9a-f]* 	{ and r5, r6, r7 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    1878:	[0-9a-f]* 	{ and r5, r6, r7 ; move r15, r16 ; ld2u r25, r26 }
    1880:	[0-9a-f]* 	{ and r5, r6, r7 ; ld4s r25, r26 }
    1888:	[0-9a-f]* 	{ and r5, r6, r7 ; andi r15, r16, 5 ; ld4u r25, r26 }
    1890:	[0-9a-f]* 	{ and r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
    1898:	[0-9a-f]* 	{ and r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    18a0:	[0-9a-f]* 	{ and r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    18a8:	[0-9a-f]* 	{ and r5, r6, r7 ; nop ; ld1s r25, r26 }
    18b0:	[0-9a-f]* 	{ and r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    18b8:	[0-9a-f]* 	{ and r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    18c0:	[0-9a-f]* 	{ and r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    18c8:	[0-9a-f]* 	{ and r5, r6, r7 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    18d0:	[0-9a-f]* 	{ and r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    18d8:	[0-9a-f]* 	{ and r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    18e0:	[0-9a-f]* 	{ and r5, r6, r7 ; movei r15, 5 ; prefetch_l2_fault r25 }
    18e8:	[0-9a-f]* 	{ and r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    18f0:	[0-9a-f]* 	{ and r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    18f8:	[0-9a-f]* 	{ and r5, r6, r7 ; rotl r15, r16, r17 ; ld r25, r26 }
    1900:	[0-9a-f]* 	{ and r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    1908:	[0-9a-f]* 	{ and r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    1910:	[0-9a-f]* 	{ and r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    1918:	[0-9a-f]* 	{ and r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    1920:	[0-9a-f]* 	{ and r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    1928:	[0-9a-f]* 	{ and r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    1930:	[0-9a-f]* 	{ and r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
    1938:	[0-9a-f]* 	{ and r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
    1940:	[0-9a-f]* 	{ and r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    1948:	[0-9a-f]* 	{ and r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
    1950:	[0-9a-f]* 	{ and r5, r6, r7 ; mnz r15, r16, r17 ; st4 r25, r26 }
    1958:	[0-9a-f]* 	{ and r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
    1960:	[0-9a-f]* 	{ and r5, r6, r7 ; v1cmpleu r15, r16, r17 }
    1968:	[0-9a-f]* 	{ and r5, r6, r7 ; v2mnz r15, r16, r17 }
    1970:	[0-9a-f]* 	{ and r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    1978:	[0-9a-f]* 	{ andi r15, r16, 5 ; addi r5, r6, 5 ; st1 r25, r26 }
    1980:	[0-9a-f]* 	{ andi r15, r16, 5 ; addxi r5, r6, 5 ; st2 r25, r26 }
    1988:	[0-9a-f]* 	{ andi r15, r16, 5 ; andi r5, r6, 5 ; st2 r25, r26 }
    1990:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
    1998:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
    19a0:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpleu r5, r6, r7 ; ld r25, r26 }
    19a8:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld1u r25, r26 }
    19b0:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
    19b8:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; st1 r25, r26 }
    19c0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; andi r15, r16, 5 ; ld1s r25, r26 }
    19c8:	[0-9a-f]* 	{ andi r15, r16, 5 ; add r5, r6, r7 ; ld r25, r26 }
    19d0:	[0-9a-f]* 	{ revbytes r5, r6 ; andi r15, r16, 5 ; ld r25, r26 }
    19d8:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; ld1s r25, r26 }
    19e0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; ld1s r25, r26 }
    19e8:	[0-9a-f]* 	{ andi r15, r16, 5 ; mz r5, r6, r7 ; ld1u r25, r26 }
    19f0:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmples r5, r6, r7 ; ld2s r25, r26 }
    19f8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shrs r5, r6, r7 ; ld2s r25, r26 }
    1a00:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; ld2u r25, r26 }
    1a08:	[0-9a-f]* 	{ andi r15, r16, 5 ; andi r5, r6, 5 ; ld4s r25, r26 }
    1a10:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    1a18:	[0-9a-f]* 	{ andi r15, r16, 5 ; move r5, r6 ; ld4u r25, r26 }
    1a20:	[0-9a-f]* 	{ andi r15, r16, 5 ; ld4u r25, r26 }
    1a28:	[0-9a-f]* 	{ andi r15, r16, 5 ; movei r5, 5 ; ld r25, r26 }
    1a30:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; andi r15, r16, 5 }
    1a38:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    1a40:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; andi r15, r16, 5 }
    1a48:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
    1a50:	[0-9a-f]* 	{ mulax r5, r6, r7 ; andi r15, r16, 5 ; st2 r25, r26 }
    1a58:	[0-9a-f]* 	{ andi r15, r16, 5 ; mz r5, r6, r7 }
    1a60:	[0-9a-f]* 	{ andi r15, r16, 5 ; or r5, r6, r7 ; ld1s r25, r26 }
    1a68:	[0-9a-f]* 	{ andi r15, r16, 5 ; addx r5, r6, r7 ; prefetch r25 }
    1a70:	[0-9a-f]* 	{ andi r15, r16, 5 ; rotli r5, r6, 5 ; prefetch r25 }
    1a78:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    1a80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    1a88:	[0-9a-f]* 	{ andi r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
    1a90:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l2 r25 }
    1a98:	[0-9a-f]* 	{ andi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    1aa0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l2_fault r25 }
    1aa8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3 r25 }
    1ab0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l3 r25 }
    1ab8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    1ac0:	[0-9a-f]* 	{ revbits r5, r6 ; andi r15, r16, 5 ; ld1s r25, r26 }
    1ac8:	[0-9a-f]* 	{ andi r15, r16, 5 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    1ad0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl r5, r6, r7 ; ld4s r25, r26 }
    1ad8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld4u r25, r26 }
    1ae0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch r25 }
    1ae8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l2 r25 }
    1af0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    1af8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l3 r25 }
    1b00:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmples r5, r6, r7 ; st r25, r26 }
    1b08:	[0-9a-f]* 	{ andi r15, r16, 5 ; shrs r5, r6, r7 ; st r25, r26 }
    1b10:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
    1b18:	[0-9a-f]* 	{ andi r15, r16, 5 ; andi r5, r6, 5 ; st2 r25, r26 }
    1b20:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
    1b28:	[0-9a-f]* 	{ andi r15, r16, 5 ; move r5, r6 ; st4 r25, r26 }
    1b30:	[0-9a-f]* 	{ andi r15, r16, 5 ; st4 r25, r26 }
    1b38:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; ld r25, r26 }
    1b40:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; ld1u r25, r26 }
    1b48:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; andi r15, r16, 5 }
    1b50:	[0-9a-f]* 	{ andi r15, r16, 5 ; v1subuc r5, r6, r7 }
    1b58:	[0-9a-f]* 	{ andi r15, r16, 5 ; v2shru r5, r6, r7 }
    1b60:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; ld4s r25, r26 }
    1b68:	[0-9a-f]* 	{ andi r5, r6, 5 ; addx r15, r16, r17 ; ld4u r25, r26 }
    1b70:	[0-9a-f]* 	{ andi r5, r6, 5 ; and r15, r16, r17 ; ld4u r25, r26 }
    1b78:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch r25 }
    1b80:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch r25 }
    1b88:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    1b90:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    1b98:	[0-9a-f]* 	{ andi r5, r6, 5 ; fetchor4 r15, r16, r17 }
    1ba0:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; st2 r25, r26 }
    1ba8:	[0-9a-f]* 	{ andi r5, r6, 5 ; jalr r15 ; st1 r25, r26 }
    1bb0:	[0-9a-f]* 	{ andi r5, r6, 5 ; jr r15 ; st4 r25, r26 }
    1bb8:	[0-9a-f]* 	{ andi r5, r6, 5 ; jalrp r15 ; ld r25, r26 }
    1bc0:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    1bc8:	[0-9a-f]* 	{ andi r5, r6, 5 ; addi r15, r16, 5 ; ld1u r25, r26 }
    1bd0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shru r15, r16, r17 ; ld1u r25, r26 }
    1bd8:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    1be0:	[0-9a-f]* 	{ andi r5, r6, 5 ; move r15, r16 ; ld2u r25, r26 }
    1be8:	[0-9a-f]* 	{ andi r5, r6, 5 ; ld4s r25, r26 }
    1bf0:	[0-9a-f]* 	{ andi r5, r6, 5 ; andi r15, r16, 5 ; ld4u r25, r26 }
    1bf8:	[0-9a-f]* 	{ andi r5, r6, 5 ; xor r15, r16, r17 ; ld4u r25, r26 }
    1c00:	[0-9a-f]* 	{ andi r5, r6, 5 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    1c08:	[0-9a-f]* 	{ andi r5, r6, 5 ; movei r15, 5 ; ld1s r25, r26 }
    1c10:	[0-9a-f]* 	{ andi r5, r6, 5 ; nop ; ld1s r25, r26 }
    1c18:	[0-9a-f]* 	{ andi r5, r6, 5 ; or r15, r16, r17 ; ld2s r25, r26 }
    1c20:	[0-9a-f]* 	{ andi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch r25 }
    1c28:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch r25 }
    1c30:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    1c38:	[0-9a-f]* 	{ andi r5, r6, 5 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    1c40:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    1c48:	[0-9a-f]* 	{ andi r5, r6, 5 ; movei r15, 5 ; prefetch_l2_fault r25 }
    1c50:	[0-9a-f]* 	{ andi r5, r6, 5 ; info 19 ; prefetch_l3 r25 }
    1c58:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    1c60:	[0-9a-f]* 	{ andi r5, r6, 5 ; rotl r15, r16, r17 ; ld r25, r26 }
    1c68:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl r15, r16, r17 ; ld1u r25, r26 }
    1c70:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    1c78:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    1c80:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl3addx r15, r16, r17 ; prefetch r25 }
    1c88:	[0-9a-f]* 	{ andi r5, r6, 5 ; shrs r15, r16, r17 ; prefetch r25 }
    1c90:	[0-9a-f]* 	{ andi r5, r6, 5 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    1c98:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; st r25, r26 }
    1ca0:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; st1 r25, r26 }
    1ca8:	[0-9a-f]* 	{ andi r5, r6, 5 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    1cb0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl r15, r16, r17 ; st2 r25, r26 }
    1cb8:	[0-9a-f]* 	{ andi r5, r6, 5 ; mnz r15, r16, r17 ; st4 r25, r26 }
    1cc0:	[0-9a-f]* 	{ andi r5, r6, 5 ; sub r15, r16, r17 ; ld4s r25, r26 }
    1cc8:	[0-9a-f]* 	{ andi r5, r6, 5 ; v1cmpleu r15, r16, r17 }
    1cd0:	[0-9a-f]* 	{ andi r5, r6, 5 ; v2mnz r15, r16, r17 }
    1cd8:	[0-9a-f]* 	{ andi r5, r6, 5 ; xor r15, r16, r17 ; st r25, r26 }
    1ce0:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; finv r15 }
    1ce8:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; ldnt4s_add r15, r16, 5 }
    1cf0:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; shl3addx r15, r16, r17 }
    1cf8:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; v1cmpne r15, r16, r17 }
    1d00:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; v2shl r15, r16, r17 }
    1d08:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; cmpltu r15, r16, r17 }
    1d10:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; ld4s r15, r16 }
    1d18:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; prefetch_add_l3_fault r15, 5 }
    1d20:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; stnt4 r15, r16 }
    1d28:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; v2cmpleu r15, r16, r17 }
    1d30:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; add r15, r16, r17 }
    1d38:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; info 19 }
    1d40:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    1d48:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; shru r15, r16, r17 }
    1d50:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; v1minui r15, r16, 5 }
    1d58:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; v2shrui r15, r16, 5 }
    1d60:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; ld2s r25, r26 }
    1d68:	[0-9a-f]* 	{ clz r5, r6 ; addxi r15, r16, 5 ; ld2u r25, r26 }
    1d70:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; ld2u r25, r26 }
    1d78:	[0-9a-f]* 	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; ld4u r25, r26 }
    1d80:	[0-9a-f]* 	{ clz r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
    1d88:	[0-9a-f]* 	{ clz r5, r6 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    1d90:	[0-9a-f]* 	{ clz r5, r6 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
    1d98:	[0-9a-f]* 	{ clz r5, r6 ; prefetch_l3_fault r25 }
    1da0:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; st r25, r26 }
    1da8:	[0-9a-f]* 	{ clz r5, r6 ; jalrp r15 ; prefetch_l3_fault r25 }
    1db0:	[0-9a-f]* 	{ clz r5, r6 ; jrp r15 ; st1 r25, r26 }
    1db8:	[0-9a-f]* 	{ clz r5, r6 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    1dc0:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; ld1s r25, r26 }
    1dc8:	[0-9a-f]* 	{ clz r5, r6 ; jalrp r15 ; ld1u r25, r26 }
    1dd0:	[0-9a-f]* 	{ clz r5, r6 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
    1dd8:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; ld2u r25, r26 }
    1de0:	[0-9a-f]* 	{ clz r5, r6 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    1de8:	[0-9a-f]* 	{ clz r5, r6 ; shl r15, r16, r17 ; ld4s r25, r26 }
    1df0:	[0-9a-f]* 	{ clz r5, r6 ; mnz r15, r16, r17 ; ld4u r25, r26 }
    1df8:	[0-9a-f]* 	{ clz r5, r6 ; ldnt4u r15, r16 }
    1e00:	[0-9a-f]* 	{ clz r5, r6 ; mnz r15, r16, r17 ; st2 r25, r26 }
    1e08:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 }
    1e10:	[0-9a-f]* 	{ clz r5, r6 ; nop }
    1e18:	[0-9a-f]* 	{ clz r5, r6 ; prefetch r15 }
    1e20:	[0-9a-f]* 	{ clz r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
    1e28:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    1e30:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; prefetch_l1_fault r25 }
    1e38:	[0-9a-f]* 	{ clz r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
    1e40:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; prefetch_l2_fault r25 }
    1e48:	[0-9a-f]* 	{ clz r5, r6 ; shru r15, r16, r17 ; prefetch_l2_fault r25 }
    1e50:	[0-9a-f]* 	{ clz r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l3 r25 }
    1e58:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; prefetch_l3_fault r25 }
    1e60:	[0-9a-f]* 	{ clz r5, r6 ; rotl r15, r16, r17 ; st4 r25, r26 }
    1e68:	[0-9a-f]* 	{ clz r5, r6 ; shl16insli r15, r16, 4660 }
    1e70:	[0-9a-f]* 	{ clz r5, r6 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    1e78:	[0-9a-f]* 	{ clz r5, r6 ; shl3add r15, r16, r17 ; ld2s r25, r26 }
    1e80:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; ld4s r25, r26 }
    1e88:	[0-9a-f]* 	{ clz r5, r6 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
    1e90:	[0-9a-f]* 	{ clz r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    1e98:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
    1ea0:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; st1 r25, r26 }
    1ea8:	[0-9a-f]* 	{ clz r5, r6 ; cmples r15, r16, r17 ; st2 r25, r26 }
    1eb0:	[0-9a-f]* 	{ clz r5, r6 ; st4 r15, r16 }
    1eb8:	[0-9a-f]* 	{ clz r5, r6 ; shrs r15, r16, r17 ; st4 r25, r26 }
    1ec0:	[0-9a-f]* 	{ clz r5, r6 ; subx r15, r16, r17 ; ld2s r25, r26 }
    1ec8:	[0-9a-f]* 	{ clz r5, r6 ; v1shrsi r15, r16, 5 }
    1ed0:	[0-9a-f]* 	{ clz r5, r6 ; v4int_l r15, r16, r17 }
    1ed8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
    1ee0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    1ee8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3 r25 }
    1ef0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeq r15, r16, r17 ; st r25, r26 }
    1ef8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
    1f00:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmplts r15, r16, r17 ; st2 r25, r26 }
    1f08:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltu r15, r16, r17 }
    1f10:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; ld1u r25, r26 }
    1f18:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; info 19 ; ld2s r25, r26 }
    1f20:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jalrp r15 ; ld1u r25, r26 }
    1f28:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    1f30:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
    1f38:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; info 19 ; ld1s r25, r26 }
    1f40:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1u r25, r26 }
    1f48:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; ld1u_add r15, r16, 5 }
    1f50:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; ld2s r25, r26 }
    1f58:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    1f60:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    1f68:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4u r25, r26 }
    1f70:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; ldnt r15, r16 }
    1f78:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    1f80:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    1f88:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; nop ; prefetch r25 }
    1f90:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
    1f98:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    1fa0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch r25 }
    1fa8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    1fb0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch_l1_fault r25 }
    1fb8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2 r25 }
    1fc0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    1fc8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l3 r25 }
    1fd0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
    1fd8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotl r15, r16, r17 ; ld4u r25, r26 }
    1fe0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    1fe8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
    1ff0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    1ff8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    2000:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    2008:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    2010:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; st r25, r26 }
    2018:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeq r15, r16, r17 ; st1 r25, r26 }
    2020:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; st1 r25, r26 }
    2028:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; st2 r25, r26 }
    2030:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; st4 r25, r26 }
    2038:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2_fault r25 }
    2040:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; v1int_h r15, r16, r17 }
    2048:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; v2shli r15, r16, 5 }
    2050:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
    2058:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
    2060:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
    2068:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    2070:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    2078:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    2080:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    2088:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    2090:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    2098:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
    20a0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    20a8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    20b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
    20b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; xor r15, r16, r17 ; ld1s r25, r26 }
    20c0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    20c8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
    20d0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    20d8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpleu r15, r16, r17 ; ld4s r25, r26 }
    20e0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    20e8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    20f0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    20f8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
    2100:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
    2108:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
    2110:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; prefetch r25 }
    2118:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    2120:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    2128:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    2130:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    2138:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jr r15 ; prefetch_l2_fault r25 }
    2140:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    2148:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    2150:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    2158:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    2160:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
    2168:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    2170:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    2178:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    2180:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    2188:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addxi r15, r16, 5 ; st r25, r26 }
    2190:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; st r25, r26 }
    2198:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    21a0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nop ; st2 r25, r26 }
    21a8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; st4 r25, r26 }
    21b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    21b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; v1addi r15, r16, 5 }
    21c0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; v2int_l r15, r16, r17 }
    21c8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
    21d0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l2 r25 }
    21d8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2_fault r25 }
    21e0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l2_fault r25 }
    21e8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2 r25 }
    21f0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
    21f8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmples r5, r6, r7 ; st r25, r26 }
    2200:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmplts r5, r6, r7 ; st2 r25, r26 }
    2208:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmpltu r5, r6, r7 }
    2210:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l2 r25 }
    2218:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; cmpeq r15, r16, r17 }
    2220:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; info 19 ; st1 r25, r26 }
    2228:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; nop ; ld r25, r26 }
    2230:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
    2238:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shrsi r5, r6, 5 ; ld1s r25, r26 }
    2240:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    2248:	[0-9a-f]* 	{ clz r5, r6 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    2250:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl2add r5, r6, r7 ; ld2s r25, r26 }
    2258:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; movei r5, 5 ; ld2u r25, r26 }
    2260:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; add r5, r6, r7 ; ld4s r25, r26 }
    2268:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    2270:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    2278:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    2280:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; move r5, r6 ; st r25, r26 }
    2288:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; st1 r25, r26 }
    2290:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    2298:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    22a0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2 r25 }
    22a8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    22b0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l3_fault r25 }
    22b8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; nor r5, r6, r7 ; st1 r25, r26 }
    22c0:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    22c8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; or r5, r6, r7 ; prefetch r25 }
    22d0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    22d8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shrui r5, r6, 5 ; prefetch r25 }
    22e0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    22e8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2 r25 }
    22f0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l2 r25 }
    22f8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    2300:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; addx r5, r6, r7 ; prefetch_l3 r25 }
    2308:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
    2310:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    2318:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    2320:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
    2328:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl r5, r6, r7 ; ld r25, r26 }
    2330:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
    2338:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl2addx r5, r6, r7 ; ld2s r25, r26 }
    2340:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl3addx r5, r6, r7 ; ld4s r25, r26 }
    2348:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shrs r5, r6, r7 ; ld4s r25, r26 }
    2350:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shru r5, r6, r7 ; prefetch r25 }
    2358:	[0-9a-f]* 	{ clz r5, r6 ; cmpeq r15, r16, r17 ; st r25, r26 }
    2360:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl2add r5, r6, r7 ; st r25, r26 }
    2368:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; movei r5, 5 ; st1 r25, r26 }
    2370:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; add r5, r6, r7 ; st2 r25, r26 }
    2378:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    2380:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
    2388:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
    2390:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; subx r5, r6, r7 ; st1 r25, r26 }
    2398:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    23a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 }
    23a8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; v1shrs r5, r6, r7 }
    23b0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; v2shl r5, r6, r7 }
    23b8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
    23c0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
    23c8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
    23d0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    23d8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    23e0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    23e8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    23f0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    23f8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    2400:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
    2408:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    2410:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    2418:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
    2420:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; xor r15, r16, r17 ; ld1s r25, r26 }
    2428:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    2430:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
    2438:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    2440:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpleu r15, r16, r17 ; ld4s r25, r26 }
    2448:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    2450:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    2458:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    2460:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
    2468:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
    2470:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
    2478:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalr r15 ; prefetch r25 }
    2480:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    2488:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    2490:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    2498:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    24a0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jr r15 ; prefetch_l2_fault r25 }
    24a8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    24b0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    24b8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    24c0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    24c8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
    24d0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    24d8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    24e0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    24e8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    24f0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addxi r15, r16, 5 ; st r25, r26 }
    24f8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; st r25, r26 }
    2500:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    2508:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; nop ; st2 r25, r26 }
    2510:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalr r15 ; st4 r25, r26 }
    2518:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    2520:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; v1addi r15, r16, 5 }
    2528:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; v2int_l r15, r16, r17 }
    2530:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
    2538:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l2 r25 }
    2540:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l2_fault r25 }
    2548:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l2_fault r25 }
    2550:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    2558:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
    2560:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmples r5, r6, r7 ; st r25, r26 }
    2568:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmplts r5, r6, r7 ; st2 r25, r26 }
    2570:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmpltu r5, r6, r7 }
    2578:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    2580:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; cmpeqi r15, r16, 5 }
    2588:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; info 19 ; st1 r25, r26 }
    2590:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; nop ; ld r25, r26 }
    2598:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
    25a0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shrsi r5, r6, 5 ; ld1s r25, r26 }
    25a8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1u r25, r26 }
    25b0:	[0-9a-f]* 	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    25b8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 ; ld2s r25, r26 }
    25c0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; movei r5, 5 ; ld2u r25, r26 }
    25c8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; add r5, r6, r7 ; ld4s r25, r26 }
    25d0:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; ld4s r25, r26 }
    25d8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeqi r15, r16, 5 ; ld4u r25, r26 }
    25e0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeqi r15, r16, 5 ; ld4u r25, r26 }
    25e8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; move r5, r6 ; st r25, r26 }
    25f0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    25f8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3 r25 }
    2600:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    2608:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    2610:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
    2618:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l3_fault r25 }
    2620:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; nor r5, r6, r7 ; st1 r25, r26 }
    2628:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpeqi r15, r16, 5 ; st2 r25, r26 }
    2630:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; or r5, r6, r7 ; prefetch r25 }
    2638:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    2640:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shrui r5, r6, 5 ; prefetch r25 }
    2648:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l1_fault r25 }
    2650:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    2658:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl3add r5, r6, r7 ; prefetch_l2 r25 }
    2660:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
    2668:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; addx r5, r6, r7 ; prefetch_l3 r25 }
    2670:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
    2678:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    2680:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    2688:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
    2690:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl r5, r6, r7 ; ld r25, r26 }
    2698:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
    26a0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld2s r25, r26 }
    26a8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld4s r25, r26 }
    26b0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shrs r5, r6, r7 ; ld4s r25, r26 }
    26b8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shru r5, r6, r7 ; prefetch r25 }
    26c0:	[0-9a-f]* 	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; st r25, r26 }
    26c8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 ; st r25, r26 }
    26d0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; movei r5, 5 ; st1 r25, r26 }
    26d8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; add r5, r6, r7 ; st2 r25, r26 }
    26e0:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; st2 r25, r26 }
    26e8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
    26f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
    26f8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; subx r5, r6, r7 ; st1 r25, r26 }
    2700:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; st2 r25, r26 }
    2708:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeqi r15, r16, 5 }
    2710:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; v1shrs r5, r6, r7 }
    2718:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; v2shl r5, r6, r7 }
    2720:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; add r15, r16, r17 ; ld r25, r26 }
    2728:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; addx r15, r16, r17 ; ld1s r25, r26 }
    2730:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; and r15, r16, r17 ; ld1s r25, r26 }
    2738:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    2740:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    2748:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    2750:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch r25 }
    2758:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; fetchaddgez r15, r16, r17 }
    2760:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ill ; prefetch_l2_fault r25 }
    2768:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalr r15 ; prefetch_l2 r25 }
    2770:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jr r15 ; prefetch_l3 r25 }
    2778:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpne r15, r16, r17 ; ld r25, r26 }
    2780:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; andi r15, r16, 5 ; ld1s r25, r26 }
    2788:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; xor r15, r16, r17 ; ld1s r25, r26 }
    2790:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    2798:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; nor r15, r16, r17 ; ld2s r25, r26 }
    27a0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalrp r15 ; ld2u r25, r26 }
    27a8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpleu r15, r16, r17 ; ld4s r25, r26 }
    27b0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; add r15, r16, r17 ; ld4u r25, r26 }
    27b8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    27c0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; lnk r15 ; st1 r25, r26 }
    27c8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; move r15, r16 ; st1 r25, r26 }
    27d0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; mz r15, r16, r17 ; st1 r25, r26 }
    27d8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; nor r15, r16, r17 ; st4 r25, r26 }
    27e0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalr r15 ; prefetch r25 }
    27e8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; addxi r15, r16, 5 ; prefetch r25 }
    27f0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; prefetch r25 }
    27f8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    2800:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    2808:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jr r15 ; prefetch_l2_fault r25 }
    2810:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    2818:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    2820:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    2828:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; rotli r15, r16, 5 ; st2 r25, r26 }
    2830:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl1add r15, r16, r17 ; st4 r25, r26 }
    2838:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    2840:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    2848:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    2850:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shru r15, r16, r17 ; ld2u r25, r26 }
    2858:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; addxi r15, r16, 5 ; st r25, r26 }
    2860:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; st r25, r26 }
    2868:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    2870:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; nop ; st2 r25, r26 }
    2878:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalr r15 ; st4 r25, r26 }
    2880:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; ld r25, r26 }
    2888:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; v1addi r15, r16, 5 }
    2890:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; v2int_l r15, r16, r17 }
    2898:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
    28a0:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; cmpexch r15, r16, r17 }
    28a8:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; cmpexch r15, r16, r17 }
    28b0:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; cmpexch r15, r16, r17 }
    28b8:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; cmpexch r15, r16, r17 }
    28c0:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; cmpexch r15, r16, r17 }
    28c8:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; cmpexch4 r15, r16, r17 }
    28d0:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; cmpexch4 r15, r16, r17 }
    28d8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpexch4 r15, r16, r17 }
    28e0:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; cmpexch4 r15, r16, r17 }
    28e8:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; cmpexch4 r15, r16, r17 }
    28f0:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; cmpexch4 r15, r16, r17 }
    28f8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
    2900:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
    2908:	[0-9a-f]* 	{ cmples r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
    2910:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    2918:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    2920:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
    2928:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmplts r5, r6, r7 }
    2930:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    2938:	[0-9a-f]* 	{ ctz r5, r6 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    2940:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; cmples r15, r16, r17 }
    2948:	[0-9a-f]* 	{ cmples r15, r16, r17 ; info 19 ; st4 r25, r26 }
    2950:	[0-9a-f]* 	{ cmples r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    2958:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    2960:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    2968:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; ld1u r25, r26 }
    2970:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    2978:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl3add r5, r6, r7 ; ld2s r25, r26 }
    2980:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; ld2u r25, r26 }
    2988:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addx r5, r6, r7 ; ld4s r25, r26 }
    2990:	[0-9a-f]* 	{ cmples r15, r16, r17 ; rotli r5, r6, 5 ; ld4s r25, r26 }
    2998:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    29a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    29a8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
    29b0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
    29b8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
    29c0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
    29c8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    29d0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    29d8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
    29e0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
    29e8:	[0-9a-f]* 	{ pcnt r5, r6 ; cmples r15, r16, r17 }
    29f0:	[0-9a-f]* 	{ revbits r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    29f8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
    2a00:	[0-9a-f]* 	{ cmples r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
    2a08:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    2a10:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
    2a18:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l2 r25 }
    2a20:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l2_fault r25 }
    2a28:	[0-9a-f]* 	{ cmples r15, r16, r17 ; and r5, r6, r7 ; prefetch_l3 r25 }
    2a30:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    2a38:	[0-9a-f]* 	{ cmples r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    2a40:	[0-9a-f]* 	{ cmples r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    2a48:	[0-9a-f]* 	{ cmples r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    2a50:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
    2a58:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
    2a60:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
    2a68:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    2a70:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    2a78:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
    2a80:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
    2a88:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl3add r5, r6, r7 ; st r25, r26 }
    2a90:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
    2a98:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addx r5, r6, r7 ; st2 r25, r26 }
    2aa0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
    2aa8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; st4 r25, r26 }
    2ab0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmples r15, r16, r17 ; st4 r25, r26 }
    2ab8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
    2ac0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmples r15, r16, r17 }
    2ac8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; v1addi r5, r6, 5 }
    2ad0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; v1shru r5, r6, r7 }
    2ad8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; v2shlsc r5, r6, r7 }
    2ae0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    2ae8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
    2af0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
    2af8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    2b00:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    2b08:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    2b10:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    2b18:	[0-9a-f]* 	{ cmples r5, r6, r7 ; fetchand r15, r16, r17 }
    2b20:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    2b28:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    2b30:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; st r25, r26 }
    2b38:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ill ; ld r25, r26 }
    2b40:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    2b48:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ld1s_add r15, r16, 5 }
    2b50:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    2b58:	[0-9a-f]* 	{ cmples r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    2b60:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    2b68:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    2b70:	[0-9a-f]* 	{ cmples r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    2b78:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    2b80:	[0-9a-f]* 	{ cmples r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    2b88:	[0-9a-f]* 	{ cmples r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    2b90:	[0-9a-f]* 	{ cmples r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
    2b98:	[0-9a-f]* 	{ cmples r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    2ba0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; prefetch r25 }
    2ba8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    2bb0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    2bb8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    2bc0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    2bc8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    2bd0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    2bd8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    2be0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    2be8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; rotli r15, r16, 5 }
    2bf0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    2bf8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    2c00:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    2c08:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    2c10:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    2c18:	[0-9a-f]* 	{ cmples r5, r6, r7 ; andi r15, r16, 5 ; st r25, r26 }
    2c20:	[0-9a-f]* 	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    2c28:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    2c30:	[0-9a-f]* 	{ cmples r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    2c38:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    2c40:	[0-9a-f]* 	{ cmples r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
    2c48:	[0-9a-f]* 	{ cmples r5, r6, r7 ; v1cmpeq r15, r16, r17 }
    2c50:	[0-9a-f]* 	{ cmples r5, r6, r7 ; v2maxsi r15, r16, 5 }
    2c58:	[0-9a-f]* 	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    2c60:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
    2c68:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
    2c70:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
    2c78:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
    2c80:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    2c88:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
    2c90:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmplts r5, r6, r7 }
    2c98:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    2ca0:	[0-9a-f]* 	{ ctz r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
    2ca8:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; cmpleu r15, r16, r17 }
    2cb0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; info 19 ; st4 r25, r26 }
    2cb8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    2cc0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    2cc8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    2cd0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpleu r15, r16, r17 ; ld1u r25, r26 }
    2cd8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
    2ce0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl3add r5, r6, r7 ; ld2s r25, r26 }
    2ce8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    2cf0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; addx r5, r6, r7 ; ld4s r25, r26 }
    2cf8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; rotli r5, r6, 5 ; ld4s r25, r26 }
    2d00:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
    2d08:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
    2d10:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
    2d18:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
    2d20:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
    2d28:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    2d30:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
    2d38:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
    2d40:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
    2d48:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
    2d50:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpleu r15, r16, r17 }
    2d58:	[0-9a-f]* 	{ revbits r5, r6 ; cmpleu r15, r16, r17 ; prefetch r25 }
    2d60:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
    2d68:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
    2d70:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l1_fault r25 }
    2d78:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
    2d80:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l2 r25 }
    2d88:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2_fault r25 }
    2d90:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; and r5, r6, r7 ; prefetch_l3 r25 }
    2d98:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    2da0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    2da8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    2db0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    2db8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
    2dc0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
    2dc8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
    2dd0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    2dd8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    2de0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
    2de8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
    2df0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl3add r5, r6, r7 ; st r25, r26 }
    2df8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    2e00:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; addx r5, r6, r7 ; st2 r25, r26 }
    2e08:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
    2e10:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
    2e18:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
    2e20:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
    2e28:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpleu r15, r16, r17 }
    2e30:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; v1addi r5, r6, 5 }
    2e38:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; v1shru r5, r6, r7 }
    2e40:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; v2shlsc r5, r6, r7 }
    2e48:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    2e50:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
    2e58:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
    2e60:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    2e68:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    2e70:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    2e78:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    2e80:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; fetchand r15, r16, r17 }
    2e88:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    2e90:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    2e98:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jr r15 ; st r25, r26 }
    2ea0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ill ; ld r25, r26 }
    2ea8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    2eb0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ld1s_add r15, r16, 5 }
    2eb8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    2ec0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    2ec8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    2ed0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    2ed8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    2ee0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    2ee8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    2ef0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    2ef8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
    2f00:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    2f08:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jr r15 ; prefetch r25 }
    2f10:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    2f18:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    2f20:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    2f28:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    2f30:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    2f38:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    2f40:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    2f48:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    2f50:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; rotli r15, r16, 5 }
    2f58:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    2f60:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    2f68:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    2f70:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    2f78:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    2f80:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; andi r15, r16, 5 ; st r25, r26 }
    2f88:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    2f90:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    2f98:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    2fa0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    2fa8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
    2fb0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; v1cmpeq r15, r16, r17 }
    2fb8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; v2maxsi r15, r16, 5 }
    2fc0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    2fc8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
    2fd0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
    2fd8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
    2fe0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
    2fe8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    2ff0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
    2ff8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmplts r5, r6, r7 }
    3000:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    3008:	[0-9a-f]* 	{ ctz r5, r6 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
    3010:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; cmplts r15, r16, r17 }
    3018:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; info 19 ; st4 r25, r26 }
    3020:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    3028:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    3030:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    3038:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    3040:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; ld2s r25, r26 }
    3048:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl3add r5, r6, r7 ; ld2s r25, r26 }
    3050:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
    3058:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; addx r5, r6, r7 ; ld4s r25, r26 }
    3060:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; rotli r5, r6, 5 ; ld4s r25, r26 }
    3068:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
    3070:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
    3078:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
    3080:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; st4 r25, r26 }
    3088:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
    3090:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
    3098:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
    30a0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l3_fault r25 }
    30a8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
    30b0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
    30b8:	[0-9a-f]* 	{ pcnt r5, r6 ; cmplts r15, r16, r17 }
    30c0:	[0-9a-f]* 	{ revbits r5, r6 ; cmplts r15, r16, r17 ; prefetch r25 }
    30c8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
    30d0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
    30d8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
    30e0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
    30e8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l2 r25 }
    30f0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    30f8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; and r5, r6, r7 ; prefetch_l3 r25 }
    3100:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    3108:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    3110:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    3118:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    3120:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
    3128:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
    3130:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
    3138:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    3140:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    3148:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
    3150:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
    3158:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl3add r5, r6, r7 ; st r25, r26 }
    3160:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
    3168:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; addx r5, r6, r7 ; st2 r25, r26 }
    3170:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
    3178:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; st4 r25, r26 }
    3180:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmplts r15, r16, r17 ; st4 r25, r26 }
    3188:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
    3190:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmplts r15, r16, r17 }
    3198:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; v1addi r5, r6, 5 }
    31a0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; v1shru r5, r6, r7 }
    31a8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; v2shlsc r5, r6, r7 }
    31b0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    31b8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
    31c0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
    31c8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    31d0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    31d8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    31e0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    31e8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; fetchand r15, r16, r17 }
    31f0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    31f8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    3200:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jr r15 ; st r25, r26 }
    3208:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; ill ; ld r25, r26 }
    3210:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    3218:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; ld1s_add r15, r16, 5 }
    3220:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    3228:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    3230:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    3238:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    3240:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    3248:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    3250:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    3258:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    3260:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
    3268:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    3270:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jr r15 ; prefetch r25 }
    3278:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    3280:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    3288:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    3290:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    3298:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    32a0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    32a8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    32b0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    32b8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; rotli r15, r16, 5 }
    32c0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    32c8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    32d0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    32d8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    32e0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    32e8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; andi r15, r16, 5 ; st r25, r26 }
    32f0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    32f8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    3300:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    3308:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    3310:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
    3318:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; v1cmpeq r15, r16, r17 }
    3320:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; v2maxsi r15, r16, 5 }
    3328:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    3330:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3 r25 }
    3338:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
    3340:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
    3348:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    3350:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpeq r5, r6, r7 ; st r25, r26 }
    3358:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmples r5, r6, r7 ; st2 r25, r26 }
    3360:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmplts r5, r6, r7 }
    3368:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpne r5, r6, r7 ; ld r25, r26 }
    3370:	[0-9a-f]* 	{ ctz r5, r6 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    3378:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; cmpltsi r15, r16, 5 }
    3380:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; info 19 ; st4 r25, r26 }
    3388:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; or r5, r6, r7 ; ld r25, r26 }
    3390:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    3398:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    33a0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1u r25, r26 }
    33a8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld2s r25, r26 }
    33b0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl3add r5, r6, r7 ; ld2s r25, r26 }
    33b8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
    33c0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; addx r5, r6, r7 ; ld4s r25, r26 }
    33c8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; rotli r5, r6, 5 ; ld4s r25, r26 }
    33d0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltsi r15, r16, 5 ; ld4u r25, r26 }
    33d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltsi r15, r16, 5 ; ld4u r25, r26 }
    33e0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; move r5, r6 ; st2 r25, r26 }
    33e8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    33f0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; st r25, r26 }
    33f8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; st1 r25, r26 }
    3400:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    3408:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 }
    3410:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; mz r5, r6, r7 ; st1 r25, r26 }
    3418:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; nor r5, r6, r7 ; st4 r25, r26 }
    3420:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltsi r15, r16, 5 }
    3428:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    3430:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpne r5, r6, r7 ; prefetch r25 }
    3438:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; subx r5, r6, r7 ; prefetch r25 }
    3440:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l1_fault r25 }
    3448:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
    3450:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shli r5, r6, 5 ; prefetch_l2 r25 }
    3458:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l2_fault r25 }
    3460:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; and r5, r6, r7 ; prefetch_l3 r25 }
    3468:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    3470:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    3478:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    3480:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; rotl r5, r6, r7 ; ld r25, r26 }
    3488:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl r5, r6, r7 ; ld1u r25, r26 }
    3490:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
    3498:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
    34a0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch r25 }
    34a8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch r25 }
    34b0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
    34b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltsi r15, r16, 5 ; st r25, r26 }
    34c0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl3add r5, r6, r7 ; st r25, r26 }
    34c8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; st1 r25, r26 }
    34d0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; addx r5, r6, r7 ; st2 r25, r26 }
    34d8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; rotli r5, r6, 5 ; st2 r25, r26 }
    34e0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    34e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    34f0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; subx r5, r6, r7 ; st4 r25, r26 }
    34f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 }
    3500:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; v1addi r5, r6, 5 }
    3508:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; v1shru r5, r6, r7 }
    3510:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; v2shlsc r5, r6, r7 }
    3518:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; add r15, r16, r17 ; ld1u r25, r26 }
    3520:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; addx r15, r16, r17 ; ld2s r25, r26 }
    3528:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; and r15, r16, r17 ; ld2s r25, r26 }
    3530:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    3538:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    3540:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch r25 }
    3548:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    3550:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; fetchand r15, r16, r17 }
    3558:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ill ; prefetch_l3_fault r25 }
    3560:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jalr r15 ; prefetch_l3 r25 }
    3568:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jr r15 ; st r25, r26 }
    3570:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ill ; ld r25, r26 }
    3578:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    3580:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ld1s_add r15, r16, 5 }
    3588:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shli r15, r16, 5 ; ld1u r25, r26 }
    3590:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    3598:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jrp r15 ; ld2u r25, r26 }
    35a0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    35a8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; addx r15, r16, r17 ; ld4u r25, r26 }
    35b0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    35b8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; lnk r15 ; st4 r25, r26 }
    35c0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; move r15, r16 ; st4 r25, r26 }
    35c8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; mz r15, r16, r17 ; st4 r25, r26 }
    35d0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; or r15, r16, r17 ; ld r25, r26 }
    35d8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jr r15 ; prefetch r25 }
    35e0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; andi r15, r16, 5 ; prefetch r25 }
    35e8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch r25 }
    35f0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    35f8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    3600:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; lnk r15 ; prefetch_l2_fault r25 }
    3608:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    3610:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    3618:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    3620:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; rotli r15, r16, 5 }
    3628:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    3630:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    3638:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    3640:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    3648:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shru r15, r16, r17 ; ld4u r25, r26 }
    3650:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; andi r15, r16, 5 ; st r25, r26 }
    3658:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; st r25, r26 }
    3660:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    3668:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; or r15, r16, r17 ; st2 r25, r26 }
    3670:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jr r15 ; st4 r25, r26 }
    3678:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; sub r15, r16, r17 ; ld1u r25, r26 }
    3680:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; v1cmpeq r15, r16, r17 }
    3688:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; v2maxsi r15, r16, 5 }
    3690:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    3698:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
    36a0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
    36a8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
    36b0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    36b8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    36c0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
    36c8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmplts r5, r6, r7 }
    36d0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    36d8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    36e0:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; cmpltu r15, r16, r17 }
    36e8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; info 19 ; st4 r25, r26 }
    36f0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    36f8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    3700:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    3708:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    3710:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltu r15, r16, r17 ; ld2s r25, r26 }
    3718:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl3add r5, r6, r7 ; ld2s r25, r26 }
    3720:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 ; ld2u r25, r26 }
    3728:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addx r5, r6, r7 ; ld4s r25, r26 }
    3730:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; rotli r5, r6, 5 ; ld4s r25, r26 }
    3738:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
    3740:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
    3748:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
    3750:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    3758:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 ; st r25, r26 }
    3760:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; st1 r25, r26 }
    3768:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    3770:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
    3778:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
    3780:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
    3788:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltu r15, r16, r17 }
    3790:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltu r15, r16, r17 ; prefetch r25 }
    3798:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
    37a0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
    37a8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    37b0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
    37b8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l2 r25 }
    37c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
    37c8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; and r5, r6, r7 ; prefetch_l3 r25 }
    37d0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    37d8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    37e0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    37e8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    37f0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
    37f8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
    3800:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
    3808:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    3810:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    3818:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
    3820:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltu r15, r16, r17 ; st r25, r26 }
    3828:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl3add r5, r6, r7 ; st r25, r26 }
    3830:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 ; st1 r25, r26 }
    3838:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addx r5, r6, r7 ; st2 r25, r26 }
    3840:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
    3848:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    3850:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    3858:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
    3860:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpltu r15, r16, r17 }
    3868:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; v1addi r5, r6, 5 }
    3870:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; v1shru r5, r6, r7 }
    3878:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; v2shlsc r5, r6, r7 }
    3880:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    3888:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
    3890:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
    3898:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    38a0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    38a8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    38b0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    38b8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; fetchand r15, r16, r17 }
    38c0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    38c8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    38d0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jr r15 ; st r25, r26 }
    38d8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; ld r25, r26 }
    38e0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    38e8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ld1s_add r15, r16, 5 }
    38f0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    38f8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    3900:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    3908:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    3910:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    3918:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    3920:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    3928:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    3930:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
    3938:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    3940:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jr r15 ; prefetch r25 }
    3948:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    3950:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    3958:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    3960:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    3968:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    3970:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    3978:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    3980:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    3988:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; rotli r15, r16, 5 }
    3990:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    3998:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    39a0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    39a8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    39b0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    39b8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; andi r15, r16, 5 ; st r25, r26 }
    39c0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    39c8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    39d0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    39d8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    39e0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
    39e8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; v1cmpeq r15, r16, r17 }
    39f0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; v2maxsi r15, r16, 5 }
    39f8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    3a00:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; cmpltui r15, r16, 5 }
    3a08:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltui r15, r16, 5 }
    3a10:	[0-9a-f]* 	{ cmpltui r15, r16, 5 ; sub r5, r6, r7 }
    3a18:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; cmpltui r15, r16, 5 }
    3a20:	[0-9a-f]* 	{ cmpltui r15, r16, 5 ; v2packl r5, r6, r7 }
    3a28:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; cmpexch4 r15, r16, r17 }
    3a30:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; ld1u_add r15, r16, 5 }
    3a38:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; prefetch_add_l1 r15, 5 }
    3a40:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; stnt r15, r16 }
    3a48:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; v2addi r15, r16, 5 }
    3a50:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; v4sub r15, r16, r17 }
    3a58:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
    3a60:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
    3a68:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
    3a70:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpne r15, r16, r17 ; st2 r25, r26 }
    3a78:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpeq r5, r6, r7 }
    3a80:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
    3a88:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
    3a90:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
    3a98:	[0-9a-f]* 	{ ctz r5, r6 ; cmpne r15, r16, r17 ; st2 r25, r26 }
    3aa0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpne r15, r16, r17 ; ld1u r25, r26 }
    3aa8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; addi r5, r6, 5 ; ld r25, r26 }
    3ab0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    3ab8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; ld1s r25, r26 }
    3ac0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    3ac8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; nop ; ld1u r25, r26 }
    3ad0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    3ad8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
    3ae0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    3ae8:	[0-9a-f]* 	{ clz r5, r6 ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    3af0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl2add r5, r6, r7 ; ld4s r25, r26 }
    3af8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; movei r5, 5 ; ld4u r25, r26 }
    3b00:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; cmpne r15, r16, r17 }
    3b08:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
    3b10:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; cmpne r15, r16, r17 }
    3b18:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 }
    3b20:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; cmpne r15, r16, r17 }
    3b28:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 ; st2 r25, r26 }
    3b30:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpne r15, r16, r17 ; st4 r25, r26 }
    3b38:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; nop ; ld r25, r26 }
    3b40:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
    3b48:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; addxi r5, r6, 5 ; prefetch r25 }
    3b50:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
    3b58:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; info 19 ; prefetch r25 }
    3b60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpne r15, r16, r17 ; prefetch r25 }
    3b68:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    3b70:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    3b78:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l2 r25 }
    3b80:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2_fault r25 }
    3b88:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    3b90:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    3b98:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
    3ba0:	[0-9a-f]* 	{ revbits r5, r6 ; cmpne r15, r16, r17 ; ld1u r25, r26 }
    3ba8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
    3bb0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
    3bb8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
    3bc0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    3bc8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    3bd0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
    3bd8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
    3be0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
    3be8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
    3bf0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; st1 r25, r26 }
    3bf8:	[0-9a-f]* 	{ clz r5, r6 ; cmpne r15, r16, r17 ; st2 r25, r26 }
    3c00:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
    3c08:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; movei r5, 5 ; st4 r25, r26 }
    3c10:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    3c18:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    3c20:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpne r15, r16, r17 ; ld2s r25, r26 }
    3c28:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; v1cmpeq r5, r6, r7 }
    3c30:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; v2add r5, r6, r7 }
    3c38:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; v2shrui r5, r6, 5 }
    3c40:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    3c48:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    3c50:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    3c58:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    3c60:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    3c68:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    3c70:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
    3c78:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; finv r15 }
    3c80:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ill ; st4 r25, r26 }
    3c88:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    3c90:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jr r15 }
    3c98:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jr r15 ; ld r25, r26 }
    3ca0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
    3ca8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
    3cb0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    3cb8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    3cc0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; movei r15, 5 ; ld2u r25, r26 }
    3cc8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ill ; ld4s r25, r26 }
    3cd0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    3cd8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ld4u r25, r26 }
    3ce0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    3ce8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
    3cf0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; nop ; ld1u r25, r26 }
    3cf8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
    3d00:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    3d08:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch r25 }
    3d10:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
    3d18:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    3d20:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    3d28:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    3d30:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    3d38:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
    3d40:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    3d48:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    3d50:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    3d58:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    3d60:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    3d68:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    3d70:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
    3d78:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
    3d80:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
    3d88:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    3d90:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1add r15, r16, r17 ; st2 r25, r26 }
    3d98:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    3da0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    3da8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; v1cmplts r15, r16, r17 }
    3db0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; v2mz r15, r16, r17 }
    3db8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
    3dc0:	[0-9a-f]* 	{ cmul r5, r6, r7 ; flush r15 }
    3dc8:	[0-9a-f]* 	{ cmul r5, r6, r7 ; ldnt4u r15, r16 }
    3dd0:	[0-9a-f]* 	{ cmul r5, r6, r7 ; shli r15, r16, 5 }
    3dd8:	[0-9a-f]* 	{ cmul r5, r6, r7 ; v1int_h r15, r16, r17 }
    3de0:	[0-9a-f]* 	{ cmul r5, r6, r7 ; v2shli r15, r16, 5 }
    3de8:	[0-9a-f]* 	{ cmula r5, r6, r7 ; cmpltui r15, r16, 5 }
    3df0:	[0-9a-f]* 	{ cmula r5, r6, r7 ; ld4s_add r15, r16, 5 }
    3df8:	[0-9a-f]* 	{ cmula r5, r6, r7 ; prefetch r15 }
    3e00:	[0-9a-f]* 	{ cmula r5, r6, r7 ; stnt4_add r15, r16, 5 }
    3e08:	[0-9a-f]* 	{ cmula r5, r6, r7 ; v2cmplts r15, r16, r17 }
    3e10:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; addi r15, r16, 5 }
    3e18:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; infol 4660 }
    3e20:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; mnz r15, r16, r17 }
    3e28:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; shrui r15, r16, 5 }
    3e30:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; v1mnz r15, r16, r17 }
    3e38:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; v2sub r15, r16, r17 }
    3e40:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; exch r15, r16, r17 }
    3e48:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; ldnt r15, r16 }
    3e50:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; raise }
    3e58:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; v1addi r15, r16, 5 }
    3e60:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; v2int_l r15, r16, r17 }
    3e68:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; and r15, r16, r17 }
    3e70:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; jrp r15 }
    3e78:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; nop }
    3e80:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; st2 r15, r16 }
    3e88:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; v1shru r15, r16, r17 }
    3e90:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; v4packsc r15, r16, r17 }
    3e98:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; fetchand r15, r16, r17 }
    3ea0:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
    3ea8:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; shl1addx r15, r16, r17 }
    3eb0:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; v1cmplts r15, r16, r17 }
    3eb8:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; v2mz r15, r16, r17 }
    3ec0:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; cmples r15, r16, r17 }
    3ec8:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; ld2s r15, r16 }
    3ed0:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    3ed8:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; stnt1 r15, r16 }
    3ee0:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; v2addsc r15, r16, r17 }
    3ee8:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; v4subsc r15, r16, r17 }
    3ef0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; flushwb }
    3ef8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
    3f00:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; shlx r15, r16, r17 }
    3f08:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; v1int_l r15, r16, r17 }
    3f10:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; v2shlsc r15, r16, r17 }
    3f18:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; cmpne r15, r16, r17 }
    3f20:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; ld4u r15, r16 }
    3f28:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; prefetch_l1_fault r15 }
    3f30:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; stnt_add r15, r16, 5 }
    3f38:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    3f40:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
    3f48:	[0-9a-f]* 	{ ctz r5, r6 ; addx r15, r16, r17 ; ld2s r25, r26 }
    3f50:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
    3f58:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    3f60:	[0-9a-f]* 	{ ctz r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    3f68:	[0-9a-f]* 	{ ctz r5, r6 ; cmplts r15, r16, r17 ; prefetch r25 }
    3f70:	[0-9a-f]* 	{ ctz r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    3f78:	[0-9a-f]* 	{ ctz r5, r6 ; fetchand r15, r16, r17 }
    3f80:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; prefetch_l3_fault r25 }
    3f88:	[0-9a-f]* 	{ ctz r5, r6 ; jalr r15 ; prefetch_l3 r25 }
    3f90:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; st r25, r26 }
    3f98:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; ld r25, r26 }
    3fa0:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    3fa8:	[0-9a-f]* 	{ ctz r5, r6 ; ld1s_add r15, r16, 5 }
    3fb0:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 ; ld1u r25, r26 }
    3fb8:	[0-9a-f]* 	{ ctz r5, r6 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    3fc0:	[0-9a-f]* 	{ ctz r5, r6 ; jrp r15 ; ld2u r25, r26 }
    3fc8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    3fd0:	[0-9a-f]* 	{ ctz r5, r6 ; addx r15, r16, r17 ; ld4u r25, r26 }
    3fd8:	[0-9a-f]* 	{ ctz r5, r6 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    3fe0:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 ; st4 r25, r26 }
    3fe8:	[0-9a-f]* 	{ ctz r5, r6 ; move r15, r16 ; st4 r25, r26 }
    3ff0:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; st4 r25, r26 }
    3ff8:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; ld r25, r26 }
    4000:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; prefetch r25 }
    4008:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    4010:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    4018:	[0-9a-f]* 	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    4020:	[0-9a-f]* 	{ ctz r5, r6 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    4028:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 ; prefetch_l2_fault r25 }
    4030:	[0-9a-f]* 	{ ctz r5, r6 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    4038:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    4040:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    4048:	[0-9a-f]* 	{ ctz r5, r6 ; rotli r15, r16, 5 }
    4050:	[0-9a-f]* 	{ ctz r5, r6 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    4058:	[0-9a-f]* 	{ ctz r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    4060:	[0-9a-f]* 	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    4068:	[0-9a-f]* 	{ ctz r5, r6 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    4070:	[0-9a-f]* 	{ ctz r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
    4078:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; st r25, r26 }
    4080:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; st r25, r26 }
    4088:	[0-9a-f]* 	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    4090:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; st2 r25, r26 }
    4098:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; st4 r25, r26 }
    40a0:	[0-9a-f]* 	{ ctz r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
    40a8:	[0-9a-f]* 	{ ctz r5, r6 ; v1cmpeq r15, r16, r17 }
    40b0:	[0-9a-f]* 	{ ctz r5, r6 ; v2maxsi r15, r16, 5 }
    40b8:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    40c0:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; fetchand4 r15, r16, r17 }
    40c8:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; ldnt2u r15, r16 }
    40d0:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; shl2add r15, r16, r17 }
    40d8:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
    40e0:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; v2packh r15, r16, r17 }
    40e8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; dblalign2 r15, r16, r17 }
    40f0:	[0-9a-f]* 	{ dblalign2 r15, r16, r17 ; info 19 }
    40f8:	[0-9a-f]* 	{ dblalign2 r15, r16, r17 ; shl16insli r5, r6, 4660 }
    4100:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; dblalign2 r15, r16, r17 }
    4108:	[0-9a-f]* 	{ dblalign2 r15, r16, r17 ; v2cmpltu r5, r6, r7 }
    4110:	[0-9a-f]* 	{ dblalign2 r15, r16, r17 ; v4shru r5, r6, r7 }
    4118:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; flush r15 }
    4120:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; ldnt4u r15, r16 }
    4128:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; shli r15, r16, 5 }
    4130:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; v1int_h r15, r16, r17 }
    4138:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; v2shli r15, r16, 5 }
    4140:	[0-9a-f]* 	{ dblalign4 r15, r16, r17 ; cmpleu r5, r6, r7 }
    4148:	[0-9a-f]* 	{ dblalign4 r15, r16, r17 ; move r5, r6 }
    4150:	[0-9a-f]* 	{ dblalign4 r15, r16, r17 ; shl2addx r5, r6, r7 }
    4158:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; dblalign4 r15, r16, r17 }
    4160:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; dblalign4 r15, r16, r17 }
    4168:	[0-9a-f]* 	{ dblalign4 r15, r16, r17 ; xori r5, r6, 5 }
    4170:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; ill }
    4178:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; mf }
    4180:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; shrsi r15, r16, 5 }
    4188:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; v1minu r15, r16, r17 }
    4190:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; v2shru r15, r16, r17 }
    4198:	[0-9a-f]* 	{ dblalign6 r15, r16, r17 ; cmpltui r5, r6, 5 }
    41a0:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; dblalign6 r15, r16, r17 }
    41a8:	[0-9a-f]* 	{ dblalign6 r15, r16, r17 ; shlx r5, r6, r7 }
    41b0:	[0-9a-f]* 	{ dblalign6 r15, r16, r17 ; v1int_h r5, r6, r7 }
    41b8:	[0-9a-f]* 	{ dblalign6 r15, r16, r17 ; v2maxsi r5, r6, 5 }
    41c0:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; addx r15, r16, r17 }
    41c8:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; iret }
    41d0:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; movei r15, 5 }
    41d8:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; shruxi r15, r16, 5 }
    41e0:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; v1shl r15, r16, r17 }
    41e8:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; v4add r15, r16, r17 }
    41f0:	[0-9a-f]* 	{ cmula r5, r6, r7 ; dtlbpr r15 }
    41f8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; dtlbpr r15 }
    4200:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; dtlbpr r15 }
    4208:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; dtlbpr r15 }
    4210:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; dtlbpr r15 }
    4218:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; exch r15, r16, r17 }
    4220:	[0-9a-f]* 	{ exch r15, r16, r17 }
    4228:	[0-9a-f]* 	{ or r5, r6, r7 ; exch r15, r16, r17 }
    4230:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; exch r15, r16, r17 }
    4238:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; exch r15, r16, r17 }
    4240:	[0-9a-f]* 	{ v4add r5, r6, r7 ; exch r15, r16, r17 }
    4248:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; exch4 r15, r16, r17 }
    4250:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; exch4 r15, r16, r17 }
    4258:	[0-9a-f]* 	{ shrui r5, r6, 5 ; exch4 r15, r16, r17 }
    4260:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; exch4 r15, r16, r17 }
    4268:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; exch4 r15, r16, r17 }
    4270:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; andi r15, r16, 5 }
    4278:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; ld r15, r16 }
    4280:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; nor r15, r16, r17 }
    4288:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; st2_add r15, r16, 5 }
    4290:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; v1shrui r15, r16, 5 }
    4298:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; v4shl r15, r16, r17 }
    42a0:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; fetchand4 r15, r16, r17 }
    42a8:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; ldnt2u r15, r16 }
    42b0:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; shl2add r15, r16, r17 }
    42b8:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
    42c0:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v2packh r15, r16, r17 }
    42c8:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; cmpleu r15, r16, r17 }
    42d0:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; ld2s_add r15, r16, 5 }
    42d8:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; prefetch_add_l2 r15, 5 }
    42e0:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; stnt1_add r15, r16, 5 }
    42e8:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; v2cmpeq r15, r16, r17 }
    42f0:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; wh64 r15 }
    42f8:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 }
    4300:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; ldnt_add r15, r16, 5 }
    4308:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; shlxi r15, r16, 5 }
    4310:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; v1maxu r15, r16, r17 }
    4318:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; v2shrs r15, r16, r17 }
    4320:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; dblalign2 r15, r16, r17 }
    4328:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; ld4u_add r15, r16, 5 }
    4330:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; prefetch_l2 r15 }
    4338:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; sub r15, r16, r17 }
    4340:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; v2cmpltu r15, r16, r17 }
    4348:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; addx r15, r16, r17 }
    4350:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; iret }
    4358:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; movei r15, 5 }
    4360:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; shruxi r15, r16, 5 }
    4368:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v1shl r15, r16, r17 }
    4370:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v4add r15, r16, r17 }
    4378:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; fetchadd r15, r16, r17 }
    4380:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    4388:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; rotli r15, r16, 5 }
    4390:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; v1cmpeq r15, r16, r17 }
    4398:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; v2maxsi r15, r16, 5 }
    43a0:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; cmpeq r15, r16, r17 }
    43a8:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; ld1s r15, r16 }
    43b0:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; or r15, r16, r17 }
    43b8:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; st4 r15, r16 }
    43c0:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; v1sub r15, r16, r17 }
    43c8:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; v4shlsc r15, r16, r17 }
    43d0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; fetchadd r15, r16, r17 }
    43d8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; fetchadd r15, r16, r17 }
    43e0:	[0-9a-f]* 	{ subx r5, r6, r7 ; fetchadd r15, r16, r17 }
    43e8:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; fetchadd r15, r16, r17 }
    43f0:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; fetchadd r15, r16, r17 }
    43f8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; fetchadd4 r15, r16, r17 }
    4400:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; fetchadd4 r15, r16, r17 }
    4408:	[0-9a-f]* 	{ shl r5, r6, r7 ; fetchadd4 r15, r16, r17 }
    4410:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; fetchadd4 r15, r16, r17 }
    4418:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; fetchadd4 r15, r16, r17 }
    4420:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; fetchadd4 r15, r16, r17 }
    4428:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    4430:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    4438:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; fetchaddgez r15, r16, r17 }
    4440:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    4448:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    4450:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    4458:	[0-9a-f]* 	{ infol 4660 ; fetchaddgez4 r15, r16, r17 }
    4460:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    4468:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    4470:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; fetchaddgez4 r15, r16, r17 }
    4478:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    4480:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; fetchand r15, r16, r17 }
    4488:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; fetchand r15, r16, r17 }
    4490:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; fetchand r15, r16, r17 }
    4498:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; fetchand r15, r16, r17 }
    44a0:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; fetchand r15, r16, r17 }
    44a8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44b0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44b8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44c0:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44c8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44d0:	[0-9a-f]* 	{ xor r5, r6, r7 ; fetchand4 r15, r16, r17 }
    44d8:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; fetchor r15, r16, r17 }
    44e0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; fetchor r15, r16, r17 }
    44e8:	[0-9a-f]* 	{ v1add r5, r6, r7 ; fetchor r15, r16, r17 }
    44f0:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; fetchor r15, r16, r17 }
    44f8:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; fetchor r15, r16, r17 }
    4500:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; fetchor4 r15, r16, r17 }
    4508:	[0-9a-f]* 	{ movei r5, 5 ; fetchor4 r15, r16, r17 }
    4510:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; fetchor4 r15, r16, r17 }
    4518:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; fetchor4 r15, r16, r17 }
    4520:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; fetchor4 r15, r16, r17 }
    4528:	[0-9a-f]* 	{ add r5, r6, r7 ; finv r15 }
    4530:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; finv r15 }
    4538:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; finv r15 }
    4540:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; finv r15 }
    4548:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; finv r15 }
    4550:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; finv r15 }
    4558:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; flush r15 }
    4560:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; flush r15 }
    4568:	[0-9a-f]* 	{ shli r5, r6, 5 ; flush r15 }
    4570:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; flush r15 }
    4578:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; flush r15 }
    4580:	[0-9a-f]* 	{ addli r5, r6, 4660 ; flushwb }
    4588:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; flushwb }
    4590:	[0-9a-f]* 	{ mulx r5, r6, r7 ; flushwb }
    4598:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; flushwb }
    45a0:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; flushwb }
    45a8:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; flushwb }
    45b0:	[0-9a-f]* 	{ add r5, r6, r7 ; ld2u r25, r26 }
    45b8:	[0-9a-f]* 	{ addi r5, r6, 5 ; ld4u r25, r26 }
    45c0:	[0-9a-f]* 	{ addx r5, r6, r7 ; ld4u r25, r26 }
    45c8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; prefetch r25 }
    45d0:	[0-9a-f]* 	{ and r5, r6, r7 ; ld4u r25, r26 }
    45d8:	[0-9a-f]* 	{ andi r5, r6, 5 ; prefetch r25 }
    45e0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch r25 }
    45e8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    45f0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
    45f8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; prefetch_l2_fault r25 }
    4600:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
    4608:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; st1 r25, r26 }
    4610:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; st4 r25, r26 }
    4618:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ld r25, r26 }
    4620:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ld r25, r26 }
    4628:	[0-9a-f]* 	{ ctz r5, r6 ; prefetch_l3 r25 }
    4630:	[0-9a-f]* 	{ ld2u r25, r26 }
    4638:	[0-9a-f]* 	{ icoh r15 }
    4640:	[0-9a-f]* 	{ inv r15 }
    4648:	[0-9a-f]* 	{ jr r15 ; ld r25, r26 }
    4650:	[0-9a-f]* 	{ add r5, r6, r7 ; ld r25, r26 }
    4658:	[0-9a-f]* 	{ mnz r15, r16, r17 ; ld r25, r26 }
    4660:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; ld r25, r26 }
    4668:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; ld1s r25, r26 }
    4670:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; ld1s r25, r26 }
    4678:	[0-9a-f]* 	{ shrui r5, r6, 5 ; ld1s r25, r26 }
    4680:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ld1u r25, r26 }
    4688:	[0-9a-f]* 	{ revbytes r5, r6 ; ld1u r25, r26 }
    4690:	[0-9a-f]* 	{ ld1u_add r15, r16, 5 }
    4698:	[0-9a-f]* 	{ jr r15 ; ld2s r25, r26 }
    46a0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; ld2s r25, r26 }
    46a8:	[0-9a-f]* 	{ andi r15, r16, 5 ; ld2u r25, r26 }
    46b0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ld2u r25, r26 }
    46b8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ld2u r25, r26 }
    46c0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ld4s r25, r26 }
    46c8:	[0-9a-f]* 	{ or r15, r16, r17 ; ld4s r25, r26 }
    46d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ld4s r25, r26 }
    46d8:	[0-9a-f]* 	{ ill ; ld4u r25, r26 }
    46e0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ld4u r25, r26 }
    46e8:	[0-9a-f]* 	{ ldnt1u_add r15, r16, 5 }
    46f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; prefetch r25 }
    46f8:	[0-9a-f]* 	{ move r15, r16 ; prefetch_l2 r25 }
    4700:	[0-9a-f]* 	{ movei r15, 5 ; prefetch_l3 r25 }
    4708:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 }
    4710:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; prefetch r25 }
    4718:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 }
    4720:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ld4u r25, r26 }
    4728:	[0-9a-f]* 	{ mulax r5, r6, r7 ; prefetch r25 }
    4730:	[0-9a-f]* 	{ mz r15, r16, r17 ; prefetch_l1_fault r25 }
    4738:	[0-9a-f]* 	{ nop ; prefetch_l2_fault r25 }
    4740:	[0-9a-f]* 	{ nor r5, r6, r7 ; prefetch_l3_fault r25 }
    4748:	[0-9a-f]* 	{ or r5, r6, r7 ; st1 r25, r26 }
    4750:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; prefetch r25 }
    4758:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; prefetch r25 }
    4760:	[0-9a-f]* 	{ shrui r5, r6, 5 ; prefetch r25 }
    4768:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; prefetch r25 }
    4770:	[0-9a-f]* 	{ nor r5, r6, r7 ; prefetch r25 }
    4778:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; prefetch r25 }
    4780:	[0-9a-f]* 	{ ill ; prefetch_l1_fault r25 }
    4788:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch_l1_fault r25 }
    4790:	[0-9a-f]* 	{ addxi r5, r6, 5 ; prefetch_l2 r25 }
    4798:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
    47a0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; prefetch_l2 r25 }
    47a8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; prefetch_l2_fault r25 }
    47b0:	[0-9a-f]* 	{ nor r15, r16, r17 ; prefetch_l2_fault r25 }
    47b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
    47c0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; prefetch_l3 r25 }
    47c8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; prefetch_l3 r25 }
    47d0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    47d8:	[0-9a-f]* 	{ movei r5, 5 ; prefetch_l3_fault r25 }
    47e0:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch_l3_fault r25 }
    47e8:	[0-9a-f]* 	{ revbytes r5, r6 ; ld r25, r26 }
    47f0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; ld1u r25, r26 }
    47f8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; ld2u r25, r26 }
    4800:	[0-9a-f]* 	{ shl r5, r6, r7 ; ld4u r25, r26 }
    4808:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ld4u r25, r26 }
    4810:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; prefetch r25 }
    4818:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch_l2 r25 }
    4820:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; prefetch_l3 r25 }
    4828:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; st r25, r26 }
    4830:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; st2 r25, r26 }
    4838:	[0-9a-f]* 	{ shli r5, r6, 5 }
    4840:	[0-9a-f]* 	{ shrs r5, r6, r7 ; st2 r25, r26 }
    4848:	[0-9a-f]* 	{ shrsi r5, r6, 5 }
    4850:	[0-9a-f]* 	{ shrui r15, r16, 5 ; ld1s r25, r26 }
    4858:	[0-9a-f]* 	{ shruxi r5, r6, 5 }
    4860:	[0-9a-f]* 	{ jalrp r15 ; st r25, r26 }
    4868:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; st r25, r26 }
    4870:	[0-9a-f]* 	{ andi r15, r16, 5 ; st1 r25, r26 }
    4878:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; st1 r25, r26 }
    4880:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; st1 r25, r26 }
    4888:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; st2 r25, r26 }
    4890:	[0-9a-f]* 	{ or r15, r16, r17 ; st2 r25, r26 }
    4898:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; st2 r25, r26 }
    48a0:	[0-9a-f]* 	{ ill ; st4 r25, r26 }
    48a8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; st4 r25, r26 }
    48b0:	[0-9a-f]* 	{ stnt4_add r15, r16, 5 }
    48b8:	[0-9a-f]* 	{ subx r15, r16, r17 ; ld r25, r26 }
    48c0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ld r25, r26 }
    48c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ld1u r25, r26 }
    48d0:	[0-9a-f]* 	{ v1adduc r15, r16, r17 }
    48d8:	[0-9a-f]* 	{ v1minu r15, r16, r17 }
    48e0:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 }
    48e8:	[0-9a-f]* 	{ v2packuc r15, r16, r17 }
    48f0:	[0-9a-f]* 	{ v4shru r15, r16, r17 }
    48f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; st r25, r26 }
    4900:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; fetchor4 r15, r16, r17 }
    4908:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; ldnt4s r15, r16 }
    4910:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; shl3add r15, r16, r17 }
    4918:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; v1cmpltui r15, r16, 5 }
    4920:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; v2packuc r15, r16, r17 }
    4928:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; cmpltsi r15, r16, 5 }
    4930:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; ld2u_add r15, r16, 5 }
    4938:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    4940:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; stnt2_add r15, r16, 5 }
    4948:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; v2cmples r15, r16, r17 }
    4950:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; xori r15, r16, 5 }
    4958:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; ill }
    4960:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; mf }
    4968:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; shrsi r15, r16, 5 }
    4970:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; v1minu r15, r16, r17 }
    4978:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; v2shru r15, r16, r17 }
    4980:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; dblalign6 r15, r16, r17 }
    4988:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; ldna r15, r16 }
    4990:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; prefetch_l3 r15 }
    4998:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; subxsc r15, r16, r17 }
    49a0:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; v2cmpne r15, r16, r17 }
    49a8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; add r15, r16, r17 ; ld4s r25, r26 }
    49b0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addx r15, r16, r17 ; ld4u r25, r26 }
    49b8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; ld4u r25, r26 }
    49c0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 ; prefetch r25 }
    49c8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    49d0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    49d8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    49e0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; fetchor4 r15, r16, r17 }
    49e8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; ill ; st2 r25, r26 }
    49f0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jalr r15 ; st1 r25, r26 }
    49f8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jr r15 ; st4 r25, r26 }
    4a00:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jalrp r15 ; ld r25, r26 }
    4a08:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    4a10:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addi r15, r16, 5 ; ld1u r25, r26 }
    4a18:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shru r15, r16, r17 ; ld1u r25, r26 }
    4a20:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    4a28:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; move r15, r16 ; ld2u r25, r26 }
    4a30:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; ld4s r25, r26 }
    4a38:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; andi r15, r16, 5 ; ld4u r25, r26 }
    4a40:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; xor r15, r16, r17 ; ld4u r25, r26 }
    4a48:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    4a50:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; ld1s r25, r26 }
    4a58:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nop ; ld1s r25, r26 }
    4a60:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; or r15, r16, r17 ; ld2s r25, r26 }
    4a68:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
    4a70:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    4a78:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    4a80:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    4a88:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    4a90:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; prefetch_l2_fault r25 }
    4a98:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; info 19 ; prefetch_l3 r25 }
    4aa0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    4aa8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; rotl r15, r16, r17 ; ld r25, r26 }
    4ab0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl r15, r16, r17 ; ld1u r25, r26 }
    4ab8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    4ac0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    4ac8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl3addx r15, r16, r17 ; prefetch r25 }
    4ad0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
    4ad8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    4ae0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; st r25, r26 }
    4ae8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; add r15, r16, r17 ; st1 r25, r26 }
    4af0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    4af8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl r15, r16, r17 ; st2 r25, r26 }
    4b00:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; st4 r25, r26 }
    4b08:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; sub r15, r16, r17 ; ld4s r25, r26 }
    4b10:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; v1cmpleu r15, r16, r17 }
    4b18:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; v2mnz r15, r16, r17 }
    4b20:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; xor r15, r16, r17 ; st r25, r26 }
    4b28:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; finv r15 }
    4b30:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    4b38:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; shl3addx r15, r16, r17 }
    4b40:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; v1cmpne r15, r16, r17 }
    4b48:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; v2shl r15, r16, r17 }
    4b50:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; cmpltu r15, r16, r17 }
    4b58:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; ld4s r15, r16 }
    4b60:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    4b68:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; stnt4 r15, r16 }
    4b70:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; v2cmpleu r15, r16, r17 }
    4b78:	[0-9a-f]* 	{ add r5, r6, r7 ; icoh r15 }
    4b80:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; icoh r15 }
    4b88:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; icoh r15 }
    4b90:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; icoh r15 }
    4b98:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; icoh r15 }
    4ba0:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; icoh r15 }
    4ba8:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; ld1u r25, r26 }
    4bb0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ill ; ld2s r25, r26 }
    4bb8:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; ld2s r25, r26 }
    4bc0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; ill ; ld1u r25, r26 }
    4bc8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ill ; ld2u r25, r26 }
    4bd0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ill ; ld4u r25, r26 }
    4bd8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; ill ; prefetch r25 }
    4be0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; prefetch_l2 r25 }
    4be8:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; ld1u r25, r26 }
    4bf0:	[0-9a-f]* 	{ ill ; prefetch_l2_fault r25 }
    4bf8:	[0-9a-f]* 	{ info 19 ; ill ; prefetch r25 }
    4c00:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ill ; ld r25, r26 }
    4c08:	[0-9a-f]* 	{ and r5, r6, r7 ; ill ; ld1s r25, r26 }
    4c10:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ill ; ld1s r25, r26 }
    4c18:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; ld1u r25, r26 }
    4c20:	[0-9a-f]* 	{ xor r5, r6, r7 ; ill ; ld1u r25, r26 }
    4c28:	[0-9a-f]* 	{ pcnt r5, r6 ; ill ; ld2s r25, r26 }
    4c30:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; ld2u r25, r26 }
    4c38:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; ld2u r25, r26 }
    4c40:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ill ; ld4s r25, r26 }
    4c48:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ill ; ld4u r25, r26 }
    4c50:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; ill ; ld4u r25, r26 }
    4c58:	[0-9a-f]* 	{ move r5, r6 ; ill ; ld4u r25, r26 }
    4c60:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; ill ; prefetch r25 }
    4c68:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; ill ; ld2u r25, r26 }
    4c70:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ill ; ld4s r25, r26 }
    4c78:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ill ; ld1u r25, r26 }
    4c80:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ill ; ld2s r25, r26 }
    4c88:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; ld4s r25, r26 }
    4c90:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; prefetch r25 }
    4c98:	[0-9a-f]* 	{ pcnt r5, r6 ; ill ; prefetch r25 }
    4ca0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ill ; prefetch r25 }
    4ca8:	[0-9a-f]* 	{ clz r5, r6 ; ill ; prefetch r25 }
    4cb0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; ill ; prefetch r25 }
    4cb8:	[0-9a-f]* 	{ movei r5, 5 ; ill ; prefetch_l1_fault r25 }
    4cc0:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; prefetch_l2 r25 }
    4cc8:	[0-9a-f]* 	{ revbytes r5, r6 ; ill ; prefetch_l2 r25 }
    4cd0:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; prefetch_l2_fault r25 }
    4cd8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; prefetch_l2_fault r25 }
    4ce0:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; prefetch_l3 r25 }
    4ce8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    4cf0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    4cf8:	[0-9a-f]* 	{ revbytes r5, r6 ; ill ; prefetch_l1_fault r25 }
    4d00:	[0-9a-f]* 	{ rotli r5, r6, 5 ; ill ; prefetch_l2_fault r25 }
    4d08:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ill ; prefetch_l3 r25 }
    4d10:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; ill ; st r25, r26 }
    4d18:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; ill ; st2 r25, r26 }
    4d20:	[0-9a-f]* 	{ shli r5, r6, 5 ; ill }
    4d28:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ill }
    4d30:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; ill }
    4d38:	[0-9a-f]* 	{ pcnt r5, r6 ; ill ; st r25, r26 }
    4d40:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; st1 r25, r26 }
    4d48:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; st1 r25, r26 }
    4d50:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ill ; st2 r25, r26 }
    4d58:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ill ; st4 r25, r26 }
    4d60:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; ill ; st4 r25, r26 }
    4d68:	[0-9a-f]* 	{ subx r5, r6, r7 ; ill ; prefetch r25 }
    4d70:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; prefetch r25 }
    4d78:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; prefetch_l2 r25 }
    4d80:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; ill }
    4d88:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; ill }
    4d90:	[0-9a-f]* 	{ xor r5, r6, r7 ; ill ; prefetch_l3 r25 }
    4d98:	[0-9a-f]* 	{ info 19 ; add r5, r6, r7 ; prefetch_l3_fault r25 }
    4da0:	[0-9a-f]* 	{ info 19 ; addi r5, r6, 5 ; st1 r25, r26 }
    4da8:	[0-9a-f]* 	{ info 19 ; addx r5, r6, r7 ; st1 r25, r26 }
    4db0:	[0-9a-f]* 	{ info 19 ; addxi r5, r6, 5 ; st4 r25, r26 }
    4db8:	[0-9a-f]* 	{ info 19 ; and r5, r6, r7 ; st1 r25, r26 }
    4dc0:	[0-9a-f]* 	{ info 19 ; andi r5, r6, 5 ; st4 r25, r26 }
    4dc8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; info 19 ; st2 r25, r26 }
    4dd0:	[0-9a-f]* 	{ info 19 ; cmpeq r15, r16, r17 }
    4dd8:	[0-9a-f]* 	{ info 19 ; cmpeqi r5, r6, 5 ; ld1s r25, r26 }
    4de0:	[0-9a-f]* 	{ info 19 ; cmples r5, r6, r7 ; ld1s r25, r26 }
    4de8:	[0-9a-f]* 	{ info 19 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    4df0:	[0-9a-f]* 	{ info 19 ; cmplts r5, r6, r7 ; ld4s r25, r26 }
    4df8:	[0-9a-f]* 	{ info 19 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    4e00:	[0-9a-f]* 	{ info 19 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    4e08:	[0-9a-f]* 	{ info 19 ; cmpne r5, r6, r7 ; prefetch_l1_fault r25 }
    4e10:	[0-9a-f]* 	{ info 19 ; dblalign2 r5, r6, r7 }
    4e18:	[0-9a-f]* 	{ info 19 ; prefetch_l3_fault r25 }
    4e20:	[0-9a-f]* 	{ info 19 ; ill ; prefetch r25 }
    4e28:	[0-9a-f]* 	{ info 19 ; jalr r15 ; prefetch r25 }
    4e30:	[0-9a-f]* 	{ info 19 ; jr r15 ; prefetch_l1_fault r25 }
    4e38:	[0-9a-f]* 	{ info 19 ; andi r15, r16, 5 ; ld r25, r26 }
    4e40:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; info 19 ; ld r25, r26 }
    4e48:	[0-9a-f]* 	{ info 19 ; shrsi r5, r6, 5 ; ld r25, r26 }
    4e50:	[0-9a-f]* 	{ info 19 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    4e58:	[0-9a-f]* 	{ info 19 ; or r5, r6, r7 ; ld1s r25, r26 }
    4e60:	[0-9a-f]* 	{ info 19 ; xor r15, r16, r17 ; ld1s r25, r26 }
    4e68:	[0-9a-f]* 	{ info 19 ; info 19 ; ld1u r25, r26 }
    4e70:	[0-9a-f]* 	{ info 19 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    4e78:	[0-9a-f]* 	{ info 19 ; addxi r5, r6, 5 ; ld2s r25, r26 }
    4e80:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; info 19 ; ld2s r25, r26 }
    4e88:	[0-9a-f]* 	{ info 19 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    4e90:	[0-9a-f]* 	{ info 19 ; cmples r15, r16, r17 ; ld2u r25, r26 }
    4e98:	[0-9a-f]* 	{ info 19 ; nop ; ld2u r25, r26 }
    4ea0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; ld2u r25, r26 }
    4ea8:	[0-9a-f]* 	{ ctz r5, r6 ; info 19 ; ld4s r25, r26 }
    4eb0:	[0-9a-f]* 	{ info 19 ; shl r15, r16, r17 ; ld4s r25, r26 }
    4eb8:	[0-9a-f]* 	{ info 19 ; addi r5, r6, 5 ; ld4u r25, r26 }
    4ec0:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; ld4u r25, r26 }
    4ec8:	[0-9a-f]* 	{ info 19 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    4ed0:	[0-9a-f]* 	{ info 19 ; ldnt_add r15, r16, 5 }
    4ed8:	[0-9a-f]* 	{ info 19 ; mnz r15, r16, r17 ; st4 r25, r26 }
    4ee0:	[0-9a-f]* 	{ info 19 ; move r5, r6 ; ld r25, r26 }
    4ee8:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; ld1u r25, r26 }
    4ef0:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; info 19 }
    4ef8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; info 19 ; st4 r25, r26 }
    4f00:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; info 19 }
    4f08:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; info 19 ; st1 r25, r26 }
    4f10:	[0-9a-f]* 	{ mulax r5, r6, r7 ; info 19 ; st2 r25, r26 }
    4f18:	[0-9a-f]* 	{ info 19 ; mz r15, r16, r17 }
    4f20:	[0-9a-f]* 	{ info 19 ; nor r15, r16, r17 ; ld1s r25, r26 }
    4f28:	[0-9a-f]* 	{ info 19 ; or r15, r16, r17 ; ld2s r25, r26 }
    4f30:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; ld2s r25, r26 }
    4f38:	[0-9a-f]* 	{ info 19 ; cmplts r15, r16, r17 ; prefetch r25 }
    4f40:	[0-9a-f]* 	{ info 19 ; or r5, r6, r7 ; prefetch r25 }
    4f48:	[0-9a-f]* 	{ info 19 ; xor r15, r16, r17 ; prefetch r25 }
    4f50:	[0-9a-f]* 	{ info 19 ; cmpne r5, r6, r7 ; prefetch r25 }
    4f58:	[0-9a-f]* 	{ info 19 ; rotli r5, r6, 5 ; prefetch r25 }
    4f60:	[0-9a-f]* 	{ info 19 ; addi r5, r6, 5 ; prefetch_l1_fault r25 }
    4f68:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; prefetch_l1_fault r25 }
    4f70:	[0-9a-f]* 	{ info 19 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    4f78:	[0-9a-f]* 	{ info 19 ; cmpeq r5, r6, r7 ; prefetch_l2 r25 }
    4f80:	[0-9a-f]* 	{ mulx r5, r6, r7 ; info 19 ; prefetch_l2 r25 }
    4f88:	[0-9a-f]* 	{ info 19 ; sub r5, r6, r7 ; prefetch_l2 r25 }
    4f90:	[0-9a-f]* 	{ info 19 ; cmpne r15, r16, r17 ; prefetch_l2_fault r25 }
    4f98:	[0-9a-f]* 	{ info 19 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    4fa0:	[0-9a-f]* 	{ info 19 ; addi r15, r16, 5 ; prefetch_l3 r25 }
    4fa8:	[0-9a-f]* 	{ info 19 ; mnz r5, r6, r7 ; prefetch_l3 r25 }
    4fb0:	[0-9a-f]* 	{ info 19 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    4fb8:	[0-9a-f]* 	{ info 19 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    4fc0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    4fc8:	[0-9a-f]* 	{ info 19 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    4fd0:	[0-9a-f]* 	{ revbytes r5, r6 ; info 19 ; prefetch_l1_fault r25 }
    4fd8:	[0-9a-f]* 	{ info 19 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
    4fe0:	[0-9a-f]* 	{ info 19 ; rotli r5, r6, 5 ; prefetch_l3_fault r25 }
    4fe8:	[0-9a-f]* 	{ info 19 ; shl r5, r6, r7 ; st1 r25, r26 }
    4ff0:	[0-9a-f]* 	{ info 19 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    4ff8:	[0-9a-f]* 	{ info 19 ; shl1addx r5, r6, r7 ; st4 r25, r26 }
    5000:	[0-9a-f]* 	{ info 19 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    5008:	[0-9a-f]* 	{ info 19 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    5010:	[0-9a-f]* 	{ info 19 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    5018:	[0-9a-f]* 	{ info 19 ; shli r15, r16, 5 ; ld4u r25, r26 }
    5020:	[0-9a-f]* 	{ info 19 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    5028:	[0-9a-f]* 	{ info 19 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    5030:	[0-9a-f]* 	{ info 19 ; shru r15, r16, r17 ; prefetch r25 }
    5038:	[0-9a-f]* 	{ info 19 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    5040:	[0-9a-f]* 	{ info 19 ; addxi r15, r16, 5 ; st r25, r26 }
    5048:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; st r25, r26 }
    5050:	[0-9a-f]* 	{ info 19 ; shli r5, r6, 5 ; st r25, r26 }
    5058:	[0-9a-f]* 	{ info 19 ; cmples r15, r16, r17 ; st1 r25, r26 }
    5060:	[0-9a-f]* 	{ info 19 ; nop ; st1 r25, r26 }
    5068:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; st1 r25, r26 }
    5070:	[0-9a-f]* 	{ ctz r5, r6 ; info 19 ; st2 r25, r26 }
    5078:	[0-9a-f]* 	{ info 19 ; shl r15, r16, r17 ; st2 r25, r26 }
    5080:	[0-9a-f]* 	{ info 19 ; addi r5, r6, 5 ; st4 r25, r26 }
    5088:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; st4 r25, r26 }
    5090:	[0-9a-f]* 	{ info 19 ; shl3addx r15, r16, r17 ; st4 r25, r26 }
    5098:	[0-9a-f]* 	{ info 19 ; sub r15, r16, r17 ; prefetch r25 }
    50a0:	[0-9a-f]* 	{ info 19 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
    50a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; prefetch_l1_fault r25 }
    50b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; info 19 ; prefetch_l2_fault r25 }
    50b8:	[0-9a-f]* 	{ info 19 ; v1cmples r5, r6, r7 }
    50c0:	[0-9a-f]* 	{ info 19 ; v1mz r15, r16, r17 }
    50c8:	[0-9a-f]* 	{ info 19 ; v2cmpltu r15, r16, r17 }
    50d0:	[0-9a-f]* 	{ info 19 ; v2shli r5, r6, 5 }
    50d8:	[0-9a-f]* 	{ info 19 ; xor r15, r16, r17 ; ld1u r25, r26 }
    50e0:	[0-9a-f]* 	{ infol 4660 ; addi r15, r16, 5 }
    50e8:	[0-9a-f]* 	{ infol 4660 ; cmpne r15, r16, r17 }
    50f0:	[0-9a-f]* 	{ infol 4660 ; flushwb }
    50f8:	[0-9a-f]* 	{ infol 4660 ; ldnt2s r15, r16 }
    5100:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; infol 4660 }
    5108:	[0-9a-f]* 	{ infol 4660 ; shl1addx r15, r16, r17 }
    5110:	[0-9a-f]* 	{ infol 4660 ; stnt2 r15, r16 }
    5118:	[0-9a-f]* 	{ infol 4660 ; v1cmpne r5, r6, r7 }
    5120:	[0-9a-f]* 	{ infol 4660 ; v1shru r15, r16, r17 }
    5128:	[0-9a-f]* 	{ infol 4660 ; v2maxs r15, r16, r17 }
    5130:	[0-9a-f]* 	{ infol 4660 ; v2sub r5, r6, r7 }
    5138:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; inv r15 }
    5140:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; inv r15 }
    5148:	[0-9a-f]* 	{ revbytes r5, r6 ; inv r15 }
    5150:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; inv r15 }
    5158:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; inv r15 }
    5160:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; inv r15 }
    5168:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; iret }
    5170:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; iret }
    5178:	[0-9a-f]* 	{ sub r5, r6, r7 ; iret }
    5180:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; iret }
    5188:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; iret }
    5190:	[0-9a-f]* 	{ add r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    5198:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    51a0:	[0-9a-f]* 	{ and r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    51a8:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; prefetch_l3 r25 }
    51b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; st r25, r26 }
    51b8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalr r15 ; st2 r25, r26 }
    51c0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jalr r15 }
    51c8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    51d0:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; jalr r15 }
    51d8:	[0-9a-f]* 	{ jalr r15 ; ld1u r25, r26 }
    51e0:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; jalr r15 }
    51e8:	[0-9a-f]* 	{ jalr r15 ; ld r25, r26 }
    51f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jalr r15 ; ld r25, r26 }
    51f8:	[0-9a-f]* 	{ nop ; jalr r15 ; ld1s r25, r26 }
    5200:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jalr r15 ; ld1u r25, r26 }
    5208:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jalr r15 ; ld1u r25, r26 }
    5210:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    5218:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; ld2u r25, r26 }
    5220:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalr r15 ; ld2u r25, r26 }
    5228:	[0-9a-f]* 	{ movei r5, 5 ; jalr r15 ; ld4s r25, r26 }
    5230:	[0-9a-f]* 	{ add r5, r6, r7 ; jalr r15 ; ld4u r25, r26 }
    5238:	[0-9a-f]* 	{ revbytes r5, r6 ; jalr r15 ; ld4u r25, r26 }
    5240:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    5248:	[0-9a-f]* 	{ movei r5, 5 ; jalr r15 }
    5250:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    5258:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
    5260:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalr r15 ; st r25, r26 }
    5268:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    5270:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
    5278:	[0-9a-f]* 	{ nop ; jalr r15 ; st4 r25, r26 }
    5280:	[0-9a-f]* 	{ ori r5, r6, 5 ; jalr r15 }
    5288:	[0-9a-f]* 	{ info 19 ; jalr r15 ; prefetch r25 }
    5290:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 ; prefetch r25 }
    5298:	[0-9a-f]* 	{ or r5, r6, r7 ; jalr r15 ; prefetch r25 }
    52a0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jalr r15 ; prefetch_l1_fault r25 }
    52a8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jalr r15 ; prefetch_l1_fault r25 }
    52b0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
    52b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
    52c0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
    52c8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    52d0:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    52d8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalr r15 ; prefetch_l3_fault r25 }
    52e0:	[0-9a-f]* 	{ revbytes r5, r6 ; jalr r15 ; ld r25, r26 }
    52e8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalr r15 ; ld1u r25, r26 }
    52f0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    52f8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    5300:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalr r15 ; prefetch r25 }
    5308:	[0-9a-f]* 	{ shli r5, r6, 5 ; jalr r15 ; prefetch_l1_fault r25 }
    5310:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jalr r15 ; prefetch_l1_fault r25 }
    5318:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jalr r15 ; prefetch_l2_fault r25 }
    5320:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalr r15 ; st r25, r26 }
    5328:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; st1 r25, r26 }
    5330:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
    5338:	[0-9a-f]* 	{ movei r5, 5 ; jalr r15 ; st2 r25, r26 }
    5340:	[0-9a-f]* 	{ add r5, r6, r7 ; jalr r15 ; st4 r25, r26 }
    5348:	[0-9a-f]* 	{ revbytes r5, r6 ; jalr r15 ; st4 r25, r26 }
    5350:	[0-9a-f]* 	{ sub r5, r6, r7 ; jalr r15 ; st4 r25, r26 }
    5358:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalr r15 }
    5360:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 ; ld1s r25, r26 }
    5368:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; jalr r15 }
    5370:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; jalr r15 }
    5378:	[0-9a-f]* 	{ xor r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    5380:	[0-9a-f]* 	{ addi r5, r6, 5 ; jalrp r15 ; ld2u r25, r26 }
    5388:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jalrp r15 ; ld4s r25, r26 }
    5390:	[0-9a-f]* 	{ andi r5, r6, 5 ; jalrp r15 ; ld4s r25, r26 }
    5398:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    53a0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    53a8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    53b0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    53b8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalrp r15 ; prefetch_l3 r25 }
    53c0:	[0-9a-f]* 	{ ctz r5, r6 ; jalrp r15 ; ld2u r25, r26 }
    53c8:	[0-9a-f]* 	{ jalrp r15 ; prefetch_l3_fault r25 }
    53d0:	[0-9a-f]* 	{ info 19 ; jalrp r15 ; prefetch_l1_fault r25 }
    53d8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalrp r15 ; ld r25, r26 }
    53e0:	[0-9a-f]* 	{ clz r5, r6 ; jalrp r15 ; ld1s r25, r26 }
    53e8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
    53f0:	[0-9a-f]* 	{ movei r5, 5 ; jalrp r15 ; ld1u r25, r26 }
    53f8:	[0-9a-f]* 	{ add r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    5400:	[0-9a-f]* 	{ revbytes r5, r6 ; jalrp r15 ; ld2s r25, r26 }
    5408:	[0-9a-f]* 	{ ctz r5, r6 ; jalrp r15 ; ld2u r25, r26 }
    5410:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalrp r15 ; ld2u r25, r26 }
    5418:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalrp r15 ; ld4s r25, r26 }
    5420:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    5428:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    5430:	[0-9a-f]* 	{ move r5, r6 ; jalrp r15 ; prefetch r25 }
    5438:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch_l1_fault r25 }
    5440:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    5448:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    5450:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    5458:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; ld4s r25, r26 }
    5460:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    5468:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalrp r15 ; prefetch_l1_fault r25 }
    5470:	[0-9a-f]* 	{ pcnt r5, r6 ; jalrp r15 ; prefetch_l2 r25 }
    5478:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    5480:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    5488:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    5490:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalrp r15 ; prefetch_l1_fault r25 }
    5498:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    54a0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalrp r15 ; prefetch_l2 r25 }
    54a8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    54b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    54b8:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalrp r15 ; prefetch_l3 r25 }
    54c0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    54c8:	[0-9a-f]* 	{ shru r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    54d0:	[0-9a-f]* 	{ revbytes r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    54d8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalrp r15 ; prefetch_l3_fault r25 }
    54e0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jalrp r15 ; st r25, r26 }
    54e8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalrp r15 ; st2 r25, r26 }
    54f0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalrp r15 }
    54f8:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; jalrp r15 }
    5500:	[0-9a-f]* 	{ shru r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
    5508:	[0-9a-f]* 	{ add r5, r6, r7 ; jalrp r15 ; st r25, r26 }
    5510:	[0-9a-f]* 	{ revbytes r5, r6 ; jalrp r15 ; st r25, r26 }
    5518:	[0-9a-f]* 	{ ctz r5, r6 ; jalrp r15 ; st1 r25, r26 }
    5520:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalrp r15 ; st1 r25, r26 }
    5528:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalrp r15 ; st2 r25, r26 }
    5530:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
    5538:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
    5540:	[0-9a-f]* 	{ subx r5, r6, r7 ; jalrp r15 ; prefetch_l1_fault r25 }
    5548:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jalrp r15 ; prefetch_l2 r25 }
    5550:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalrp r15 ; prefetch_l3 r25 }
    5558:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; jalrp r15 }
    5560:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; jalrp r15 }
    5568:	[0-9a-f]* 	{ xor r5, r6, r7 ; jalrp r15 ; st r25, r26 }
    5570:	[0-9a-f]* 	{ addi r5, r6, 5 ; jr r15 ; st1 r25, r26 }
    5578:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jr r15 ; st2 r25, r26 }
    5580:	[0-9a-f]* 	{ andi r5, r6, 5 ; jr r15 ; st2 r25, r26 }
    5588:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jr r15 ; st1 r25, r26 }
    5590:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    5598:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jr r15 ; ld r25, r26 }
    55a0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jr r15 ; ld1u r25, r26 }
    55a8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    55b0:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; st1 r25, r26 }
    55b8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jr r15 ; ld1s r25, r26 }
    55c0:	[0-9a-f]* 	{ add r5, r6, r7 ; jr r15 ; ld r25, r26 }
    55c8:	[0-9a-f]* 	{ revbytes r5, r6 ; jr r15 ; ld r25, r26 }
    55d0:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; ld1s r25, r26 }
    55d8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jr r15 ; ld1s r25, r26 }
    55e0:	[0-9a-f]* 	{ mz r5, r6, r7 ; jr r15 ; ld1u r25, r26 }
    55e8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    55f0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    55f8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    5600:	[0-9a-f]* 	{ andi r5, r6, 5 ; jr r15 ; ld4s r25, r26 }
    5608:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
    5610:	[0-9a-f]* 	{ move r5, r6 ; jr r15 ; ld4u r25, r26 }
    5618:	[0-9a-f]* 	{ jr r15 ; ld4u r25, r26 }
    5620:	[0-9a-f]* 	{ movei r5, 5 ; jr r15 ; ld r25, r26 }
    5628:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; jr r15 }
    5630:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    5638:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 }
    5640:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jr r15 ; st1 r25, r26 }
    5648:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jr r15 ; st2 r25, r26 }
    5650:	[0-9a-f]* 	{ mz r5, r6, r7 ; jr r15 }
    5658:	[0-9a-f]* 	{ or r5, r6, r7 ; jr r15 ; ld1s r25, r26 }
    5660:	[0-9a-f]* 	{ addx r5, r6, r7 ; jr r15 ; prefetch r25 }
    5668:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jr r15 ; prefetch r25 }
    5670:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jr r15 ; prefetch r25 }
    5678:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jr r15 ; prefetch r25 }
    5680:	[0-9a-f]* 	{ nor r5, r6, r7 ; jr r15 ; prefetch_l1_fault r25 }
    5688:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jr r15 ; prefetch_l2 r25 }
    5690:	[0-9a-f]* 	{ shru r5, r6, r7 ; jr r15 ; prefetch_l2 r25 }
    5698:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jr r15 ; prefetch_l2_fault r25 }
    56a0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    56a8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    56b0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jr r15 ; prefetch_l3_fault r25 }
    56b8:	[0-9a-f]* 	{ revbits r5, r6 ; jr r15 ; ld1s r25, r26 }
    56c0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    56c8:	[0-9a-f]* 	{ shl r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
    56d0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; ld4u r25, r26 }
    56d8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jr r15 ; prefetch r25 }
    56e0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jr r15 ; prefetch_l2 r25 }
    56e8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; prefetch_l2 r25 }
    56f0:	[0-9a-f]* 	{ shru r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    56f8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; st r25, r26 }
    5700:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; st r25, r26 }
    5708:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; st1 r25, r26 }
    5710:	[0-9a-f]* 	{ andi r5, r6, 5 ; jr r15 ; st2 r25, r26 }
    5718:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; st2 r25, r26 }
    5720:	[0-9a-f]* 	{ move r5, r6 ; jr r15 ; st4 r25, r26 }
    5728:	[0-9a-f]* 	{ jr r15 ; st4 r25, r26 }
    5730:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jr r15 ; ld r25, r26 }
    5738:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jr r15 ; ld1u r25, r26 }
    5740:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; jr r15 }
    5748:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; jr r15 }
    5750:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; jr r15 }
    5758:	[0-9a-f]* 	{ add r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    5760:	[0-9a-f]* 	{ addx r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    5768:	[0-9a-f]* 	{ and r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    5770:	[0-9a-f]* 	{ clz r5, r6 ; jrp r15 ; ld4s r25, r26 }
    5778:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5780:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jrp r15 ; prefetch_l1_fault r25 }
    5788:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jrp r15 ; prefetch_l2_fault r25 }
    5790:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jrp r15 ; prefetch_l3_fault r25 }
    5798:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jrp r15 ; st r25, r26 }
    57a0:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; jrp r15 }
    57a8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jrp r15 ; prefetch_l3 r25 }
    57b0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jrp r15 ; ld r25, r26 }
    57b8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jrp r15 ; ld r25, r26 }
    57c0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jrp r15 ; ld1s r25, r26 }
    57c8:	[0-9a-f]* 	{ andi r5, r6, 5 ; jrp r15 ; ld1u r25, r26 }
    57d0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jrp r15 ; ld1u r25, r26 }
    57d8:	[0-9a-f]* 	{ move r5, r6 ; jrp r15 ; ld2s r25, r26 }
    57e0:	[0-9a-f]* 	{ jrp r15 ; ld2s r25, r26 }
    57e8:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; ld2u r25, r26 }
    57f0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    57f8:	[0-9a-f]* 	{ subx r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    5800:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    5808:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jrp r15 ; prefetch_l1_fault r25 }
    5810:	[0-9a-f]* 	{ movei r5, 5 ; jrp r15 ; prefetch_l2_fault r25 }
    5818:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jrp r15 ; prefetch_l1_fault r25 }
    5820:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5828:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5830:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    5838:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5840:	[0-9a-f]* 	{ nop ; jrp r15 ; prefetch_l2 r25 }
    5848:	[0-9a-f]* 	{ or r5, r6, r7 ; jrp r15 ; prefetch_l3 r25 }
    5850:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5858:	[0-9a-f]* 	{ shru r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5860:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jrp r15 ; prefetch r25 }
    5868:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jrp r15 ; prefetch_l1_fault r25 }
    5870:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jrp r15 ; prefetch_l1_fault r25 }
    5878:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jrp r15 ; prefetch_l2 r25 }
    5880:	[0-9a-f]* 	{ addi r5, r6, 5 ; jrp r15 ; prefetch_l2_fault r25 }
    5888:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jrp r15 ; prefetch_l2_fault r25 }
    5890:	[0-9a-f]* 	{ jrp r15 ; prefetch_l3 r25 }
    5898:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jrp r15 ; prefetch_l3 r25 }
    58a0:	[0-9a-f]* 	{ nop ; jrp r15 ; prefetch_l3_fault r25 }
    58a8:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; prefetch_l3 r25 }
    58b0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jrp r15 ; st r25, r26 }
    58b8:	[0-9a-f]* 	{ shl r5, r6, r7 ; jrp r15 ; st2 r25, r26 }
    58c0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jrp r15 ; st4 r25, r26 }
    58c8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jrp r15 ; ld r25, r26 }
    58d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; jrp r15 ; ld1u r25, r26 }
    58d8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jrp r15 ; ld1u r25, r26 }
    58e0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jrp r15 ; ld2u r25, r26 }
    58e8:	[0-9a-f]* 	{ move r5, r6 ; jrp r15 ; st r25, r26 }
    58f0:	[0-9a-f]* 	{ jrp r15 ; st r25, r26 }
    58f8:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; st1 r25, r26 }
    5900:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jrp r15 ; st2 r25, r26 }
    5908:	[0-9a-f]* 	{ subx r5, r6, r7 ; jrp r15 ; st2 r25, r26 }
    5910:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jrp r15 ; st4 r25, r26 }
    5918:	[0-9a-f]* 	{ sub r5, r6, r7 ; jrp r15 ; prefetch_l2 r25 }
    5920:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jrp r15 ; prefetch_l2_fault r25 }
    5928:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; prefetch_l3_fault r25 }
    5930:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; jrp r15 }
    5938:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; jrp r15 }
    5940:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; jrp r15 }
    5948:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ld r15, r16 }
    5950:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; ld r15, r16 }
    5958:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; ld r15, r16 }
    5960:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; ld r15, r16 }
    5968:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; ld r15, r16 }
    5970:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; ld r15, r16 }
    5978:	[0-9a-f]* 	{ add r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    5980:	[0-9a-f]* 	{ add r5, r6, r7 ; ld r25, r26 }
    5988:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addi r15, r16, 5 ; ld r25, r26 }
    5990:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld r25, r26 }
    5998:	[0-9a-f]* 	{ addi r5, r6, 5 ; movei r15, 5 ; ld r25, r26 }
    59a0:	[0-9a-f]* 	{ ctz r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
    59a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
    59b0:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl2add r15, r16, r17 ; ld r25, r26 }
    59b8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    59c0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; and r15, r16, r17 ; ld r25, r26 }
    59c8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; subx r15, r16, r17 ; ld r25, r26 }
    59d0:	[0-9a-f]* 	{ and r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    59d8:	[0-9a-f]* 	{ and r5, r6, r7 ; ld r25, r26 }
    59e0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; andi r15, r16, 5 ; ld r25, r26 }
    59e8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld r25, r26 }
    59f0:	[0-9a-f]* 	{ andi r5, r6, 5 ; movei r15, 5 ; ld r25, r26 }
    59f8:	[0-9a-f]* 	{ clz r5, r6 ; jalr r15 ; ld r25, r26 }
    5a00:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    5a08:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    5a10:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    5a18:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; nor r5, r6, r7 ; ld r25, r26 }
    5a20:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    5a28:	[0-9a-f]* 	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; ld r25, r26 }
    5a30:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 ; ld r25, r26 }
    5a38:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; move r15, r16 ; ld r25, r26 }
    5a40:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    5a48:	[0-9a-f]* 	{ cmples r15, r16, r17 ; subx r5, r6, r7 ; ld r25, r26 }
    5a50:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    5a58:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; ld r25, r26 }
    5a60:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    5a68:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    5a70:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; nor r5, r6, r7 ; ld r25, r26 }
    5a78:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    5a80:	[0-9a-f]* 	{ clz r5, r6 ; cmpltsi r15, r16, 5 ; ld r25, r26 }
    5a88:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl2add r5, r6, r7 ; ld r25, r26 }
    5a90:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; move r15, r16 ; ld r25, r26 }
    5a98:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    5aa0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; subx r5, r6, r7 ; ld r25, r26 }
    5aa8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    5ab0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    5ab8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    5ac0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    5ac8:	[0-9a-f]* 	{ ctz r5, r6 ; shl3add r15, r16, r17 ; ld r25, r26 }
    5ad0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; ld r25, r26 }
    5ad8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; ld r25, r26 }
    5ae0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addxi r15, r16, 5 ; ld r25, r26 }
    5ae8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; sub r15, r16, r17 ; ld r25, r26 }
    5af0:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; ld r25, r26 }
    5af8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; info 19 ; ld r25, r26 }
    5b00:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; info 19 ; ld r25, r26 }
    5b08:	[0-9a-f]* 	{ info 19 ; shrui r15, r16, 5 ; ld r25, r26 }
    5b10:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jalr r15 ; ld r25, r26 }
    5b18:	[0-9a-f]* 	{ and r5, r6, r7 ; jalrp r15 ; ld r25, r26 }
    5b20:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jalrp r15 ; ld r25, r26 }
    5b28:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jr r15 ; ld r25, r26 }
    5b30:	[0-9a-f]* 	{ xor r5, r6, r7 ; jr r15 ; ld r25, r26 }
    5b38:	[0-9a-f]* 	{ pcnt r5, r6 ; jrp r15 ; ld r25, r26 }
    5b40:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; lnk r15 ; ld r25, r26 }
    5b48:	[0-9a-f]* 	{ sub r5, r6, r7 ; lnk r15 ; ld r25, r26 }
    5b50:	[0-9a-f]* 	{ mulax r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    5b58:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpleu r15, r16, r17 ; ld r25, r26 }
    5b60:	[0-9a-f]* 	{ move r15, r16 ; addx r5, r6, r7 ; ld r25, r26 }
    5b68:	[0-9a-f]* 	{ move r15, r16 ; rotli r5, r6, 5 ; ld r25, r26 }
    5b70:	[0-9a-f]* 	{ move r5, r6 ; jr r15 ; ld r25, r26 }
    5b78:	[0-9a-f]* 	{ movei r15, 5 ; cmpleu r5, r6, r7 ; ld r25, r26 }
    5b80:	[0-9a-f]* 	{ movei r15, 5 ; shrsi r5, r6, 5 ; ld r25, r26 }
    5b88:	[0-9a-f]* 	{ movei r5, 5 ; rotl r15, r16, r17 ; ld r25, r26 }
    5b90:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    5b98:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ill ; ld r25, r26 }
    5ba0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    5ba8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; ld r25, r26 }
    5bb0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shru r15, r16, r17 ; ld r25, r26 }
    5bb8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; ld r25, r26 }
    5bc0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nor r15, r16, r17 ; ld r25, r26 }
    5bc8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jrp r15 ; ld r25, r26 }
    5bd0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    5bd8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpeq r15, r16, r17 ; ld r25, r26 }
    5be0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ld r25, r26 }
    5be8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrs r15, r16, r17 ; ld r25, r26 }
    5bf0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; mz r15, r16, r17 ; ld r25, r26 }
    5bf8:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpleu r15, r16, r17 ; ld r25, r26 }
    5c00:	[0-9a-f]* 	{ nop ; addi r15, r16, 5 ; ld r25, r26 }
    5c08:	[0-9a-f]* 	{ nop ; mnz r5, r6, r7 ; ld r25, r26 }
    5c10:	[0-9a-f]* 	{ nop ; shl3add r5, r6, r7 ; ld r25, r26 }
    5c18:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
    5c20:	[0-9a-f]* 	{ nor r15, r16, r17 ; subx r5, r6, r7 ; ld r25, r26 }
    5c28:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    5c30:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    5c38:	[0-9a-f]* 	{ or r5, r6, r7 ; addxi r15, r16, 5 ; ld r25, r26 }
    5c40:	[0-9a-f]* 	{ or r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    5c48:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3add r15, r16, r17 ; ld r25, r26 }
    5c50:	[0-9a-f]* 	{ revbits r5, r6 ; rotl r15, r16, r17 ; ld r25, r26 }
    5c58:	[0-9a-f]* 	{ revbytes r5, r6 ; mnz r15, r16, r17 ; ld r25, r26 }
    5c60:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpltu r5, r6, r7 ; ld r25, r26 }
    5c68:	[0-9a-f]* 	{ rotl r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    5c70:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    5c78:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; rotli r15, r16, 5 ; ld r25, r26 }
    5c80:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addx r15, r16, r17 ; ld r25, r26 }
    5c88:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shrui r15, r16, 5 ; ld r25, r26 }
    5c90:	[0-9a-f]* 	{ shl r15, r16, r17 ; nop ; ld r25, r26 }
    5c98:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
    5ca0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; andi r5, r6, 5 ; ld r25, r26 }
    5ca8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl1addx r5, r6, r7 ; ld r25, r26 }
    5cb0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    5cb8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpltu r5, r6, r7 ; ld r25, r26 }
    5cc0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    5cc8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    5cd0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl2add r15, r16, r17 ; ld r25, r26 }
    5cd8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; addx r15, r16, r17 ; ld r25, r26 }
    5ce0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shrui r15, r16, 5 ; ld r25, r26 }
    5ce8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; nop ; ld r25, r26 }
    5cf0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
    5cf8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; andi r5, r6, 5 ; ld r25, r26 }
    5d00:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl1addx r5, r6, r7 ; ld r25, r26 }
    5d08:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    5d10:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpltu r5, r6, r7 ; ld r25, r26 }
    5d18:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    5d20:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    5d28:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shli r15, r16, 5 ; ld r25, r26 }
    5d30:	[0-9a-f]* 	{ shli r5, r6, 5 ; addx r15, r16, r17 ; ld r25, r26 }
    5d38:	[0-9a-f]* 	{ shli r5, r6, 5 ; shrui r15, r16, 5 ; ld r25, r26 }
    5d40:	[0-9a-f]* 	{ shrs r15, r16, r17 ; nop ; ld r25, r26 }
    5d48:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
    5d50:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; andi r5, r6, 5 ; ld r25, r26 }
    5d58:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld r25, r26 }
    5d60:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; mnz r15, r16, r17 ; ld r25, r26 }
    5d68:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpltu r5, r6, r7 ; ld r25, r26 }
    5d70:	[0-9a-f]* 	{ shru r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    5d78:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    5d80:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrui r15, r16, 5 ; ld r25, r26 }
    5d88:	[0-9a-f]* 	{ shrui r5, r6, 5 ; addx r15, r16, r17 ; ld r25, r26 }
    5d90:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shrui r15, r16, 5 ; ld r25, r26 }
    5d98:	[0-9a-f]* 	{ sub r15, r16, r17 ; nop ; ld r25, r26 }
    5da0:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
    5da8:	[0-9a-f]* 	{ subx r15, r16, r17 ; andi r5, r6, 5 ; ld r25, r26 }
    5db0:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl1addx r5, r6, r7 ; ld r25, r26 }
    5db8:	[0-9a-f]* 	{ subx r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    5dc0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; ld r25, r26 }
    5dc8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmples r15, r16, r17 ; ld r25, r26 }
    5dd0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addi r15, r16, 5 ; ld r25, r26 }
    5dd8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shru r15, r16, r17 ; ld r25, r26 }
    5de0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2add r15, r16, r17 ; ld r25, r26 }
    5de8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
    5df0:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; ld r25, r26 }
    5df8:	[0-9a-f]* 	{ xor r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
    5e00:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; ld1s r15, r16 }
    5e08:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; ld1s r15, r16 }
    5e10:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ld1s r15, r16 }
    5e18:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; ld1s r15, r16 }
    5e20:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; ld1s r15, r16 }
    5e28:	[0-9a-f]* 	{ add r15, r16, r17 ; ld1s r25, r26 }
    5e30:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; ld1s r25, r26 }
    5e38:	[0-9a-f]* 	{ add r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1s r25, r26 }
    5e40:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; ld1s r25, r26 }
    5e48:	[0-9a-f]* 	{ addi r5, r6, 5 ; andi r15, r16, 5 ; ld1s r25, r26 }
    5e50:	[0-9a-f]* 	{ addi r5, r6, 5 ; xor r15, r16, r17 ; ld1s r25, r26 }
    5e58:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; ld1s r25, r26 }
    5e60:	[0-9a-f]* 	{ addx r5, r6, r7 ; ill ; ld1s r25, r26 }
    5e68:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    5e70:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl3add r5, r6, r7 ; ld1s r25, r26 }
    5e78:	[0-9a-f]* 	{ addxi r5, r6, 5 ; mz r15, r16, r17 ; ld1s r25, r26 }
    5e80:	[0-9a-f]* 	{ and r15, r16, r17 ; ld1s r25, r26 }
    5e88:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    5e90:	[0-9a-f]* 	{ and r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1s r25, r26 }
    5e98:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
    5ea0:	[0-9a-f]* 	{ andi r5, r6, 5 ; andi r15, r16, 5 ; ld1s r25, r26 }
    5ea8:	[0-9a-f]* 	{ andi r5, r6, 5 ; xor r15, r16, r17 ; ld1s r25, r26 }
    5eb0:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; ld1s r25, r26 }
    5eb8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    5ec0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    5ec8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; ld1s r25, r26 }
    5ed0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; ld1s r25, r26 }
    5ed8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    5ee0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    5ee8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; and r15, r16, r17 ; ld1s r25, r26 }
    5ef0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; subx r15, r16, r17 ; ld1s r25, r26 }
    5ef8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; or r5, r6, r7 ; ld1s r25, r26 }
    5f00:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ld1s r25, r26 }
    5f08:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
    5f10:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    5f18:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    5f20:	[0-9a-f]* 	{ ctz r5, r6 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    5f28:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    5f30:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    5f38:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
    5f40:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; and r15, r16, r17 ; ld1s r25, r26 }
    5f48:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; subx r15, r16, r17 ; ld1s r25, r26 }
    5f50:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; or r5, r6, r7 ; ld1s r25, r26 }
    5f58:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ld1s r25, r26 }
    5f60:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    5f68:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    5f70:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    5f78:	[0-9a-f]* 	{ ctz r5, r6 ; jalr r15 ; ld1s r25, r26 }
    5f80:	[0-9a-f]* 	{ andi r15, r16, 5 ; ld1s r25, r26 }
    5f88:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ld1s r25, r26 }
    5f90:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ld1s r25, r26 }
    5f98:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; ld1s r25, r26 }
    5fa0:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; ld1s r25, r26 }
    5fa8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; ld1s r25, r26 }
    5fb0:	[0-9a-f]* 	{ info 19 ; ill ; ld1s r25, r26 }
    5fb8:	[0-9a-f]* 	{ info 19 ; shl1add r5, r6, r7 ; ld1s r25, r26 }
    5fc0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    5fc8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    5fd0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
    5fd8:	[0-9a-f]* 	{ addx r5, r6, r7 ; jr r15 ; ld1s r25, r26 }
    5fe0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jr r15 ; ld1s r25, r26 }
    5fe8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jrp r15 ; ld1s r25, r26 }
    5ff0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; ld1s r25, r26 }
    5ff8:	[0-9a-f]* 	{ nor r5, r6, r7 ; lnk r15 ; ld1s r25, r26 }
    6000:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmplts r5, r6, r7 ; ld1s r25, r26 }
    6008:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shru r5, r6, r7 ; ld1s r25, r26 }
    6010:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    6018:	[0-9a-f]* 	{ move r15, r16 ; movei r5, 5 ; ld1s r25, r26 }
    6020:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; ld1s r25, r26 }
    6028:	[0-9a-f]* 	{ move r5, r6 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    6030:	[0-9a-f]* 	{ mulx r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    6038:	[0-9a-f]* 	{ movei r5, 5 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    6040:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    6048:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
    6050:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    6058:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    6060:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld1s r25, r26 }
    6068:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ill ; ld1s r25, r26 }
    6070:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; ld1s r25, r26 }
    6078:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; ld1s r25, r26 }
    6080:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; ld1s r25, r26 }
    6088:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    6090:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nor r15, r16, r17 ; ld1s r25, r26 }
    6098:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jrp r15 ; ld1s r25, r26 }
    60a0:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmplts r5, r6, r7 ; ld1s r25, r26 }
    60a8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shru r5, r6, r7 ; ld1s r25, r26 }
    60b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    60b8:	[0-9a-f]* 	{ nop ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    60c0:	[0-9a-f]* 	{ nop ; or r5, r6, r7 ; ld1s r25, r26 }
    60c8:	[0-9a-f]* 	{ nop ; xor r15, r16, r17 ; ld1s r25, r26 }
    60d0:	[0-9a-f]* 	{ nor r15, r16, r17 ; or r5, r6, r7 ; ld1s r25, r26 }
    60d8:	[0-9a-f]* 	{ nor r5, r6, r7 ; ld1s r25, r26 }
    60e0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; ld1s r25, r26 }
    60e8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    60f0:	[0-9a-f]* 	{ or r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    60f8:	[0-9a-f]* 	{ pcnt r5, r6 ; jalr r15 ; ld1s r25, r26 }
    6100:	[0-9a-f]* 	{ revbits r5, r6 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    6108:	[0-9a-f]* 	{ revbytes r5, r6 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    6110:	[0-9a-f]* 	{ revbytes r5, r6 ; sub r15, r16, r17 ; ld1s r25, r26 }
    6118:	[0-9a-f]* 	{ rotl r15, r16, r17 ; nor r5, r6, r7 ; ld1s r25, r26 }
    6120:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    6128:	[0-9a-f]* 	{ clz r5, r6 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    6130:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl2add r5, r6, r7 ; ld1s r25, r26 }
    6138:	[0-9a-f]* 	{ rotli r5, r6, 5 ; move r15, r16 ; ld1s r25, r26 }
    6140:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    6148:	[0-9a-f]* 	{ shl r15, r16, r17 ; subx r5, r6, r7 ; ld1s r25, r26 }
    6150:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    6158:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; ld1s r25, r26 }
    6160:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    6168:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
    6170:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; nor r5, r6, r7 ; ld1s r25, r26 }
    6178:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    6180:	[0-9a-f]* 	{ clz r5, r6 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    6188:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl2add r5, r6, r7 ; ld1s r25, r26 }
    6190:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; move r15, r16 ; ld1s r25, r26 }
    6198:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    61a0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; subx r5, r6, r7 ; ld1s r25, r26 }
    61a8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    61b0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    61b8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    61c0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
    61c8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; nor r5, r6, r7 ; ld1s r25, r26 }
    61d0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    61d8:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; ld1s r25, r26 }
    61e0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl2add r5, r6, r7 ; ld1s r25, r26 }
    61e8:	[0-9a-f]* 	{ shli r5, r6, 5 ; move r15, r16 ; ld1s r25, r26 }
    61f0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    61f8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; subx r5, r6, r7 ; ld1s r25, r26 }
    6200:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    6208:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    6210:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    6218:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; sub r15, r16, r17 ; ld1s r25, r26 }
    6220:	[0-9a-f]* 	{ shru r15, r16, r17 ; nor r5, r6, r7 ; ld1s r25, r26 }
    6228:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    6230:	[0-9a-f]* 	{ clz r5, r6 ; shrui r15, r16, 5 ; ld1s r25, r26 }
    6238:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl2add r5, r6, r7 ; ld1s r25, r26 }
    6240:	[0-9a-f]* 	{ shrui r5, r6, 5 ; move r15, r16 ; ld1s r25, r26 }
    6248:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    6250:	[0-9a-f]* 	{ sub r15, r16, r17 ; subx r5, r6, r7 ; ld1s r25, r26 }
    6258:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    6260:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; subx r15, r16, r17 ; ld1s r25, r26 }
    6268:	[0-9a-f]* 	{ subx r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    6270:	[0-9a-f]* 	{ subx r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
    6278:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    6280:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    6288:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; ld1s r25, r26 }
    6290:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; ld1s r25, r26 }
    6298:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; xor r15, r16, r17 ; ld1s r25, r26 }
    62a0:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl3add r5, r6, r7 ; ld1s r25, r26 }
    62a8:	[0-9a-f]* 	{ xor r5, r6, r7 ; mz r15, r16, r17 ; ld1s r25, r26 }
    62b0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ld1s_add r15, r16, 5 }
    62b8:	[0-9a-f]* 	{ move r5, r6 ; ld1s_add r15, r16, 5 }
    62c0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; ld1s_add r15, r16, 5 }
    62c8:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; ld1s_add r15, r16, 5 }
    62d0:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; ld1s_add r15, r16, 5 }
    62d8:	[0-9a-f]* 	{ xori r5, r6, 5 ; ld1s_add r15, r16, 5 }
    62e0:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; ld1u r15, r16 }
    62e8:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; ld1u r15, r16 }
    62f0:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; ld1u r15, r16 }
    62f8:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; ld1u r15, r16 }
    6300:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; ld1u r15, r16 }
    6308:	[0-9a-f]* 	{ add r15, r16, r17 ; info 19 ; ld1u r25, r26 }
    6310:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
    6318:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    6320:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; ld1u r25, r26 }
    6328:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpeqi r15, r16, 5 ; ld1u r25, r26 }
    6330:	[0-9a-f]* 	{ addx r15, r16, r17 ; add r5, r6, r7 ; ld1u r25, r26 }
    6338:	[0-9a-f]* 	{ revbytes r5, r6 ; addx r15, r16, r17 ; ld1u r25, r26 }
    6340:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalr r15 ; ld1u r25, r26 }
    6348:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpeqi r5, r6, 5 ; ld1u r25, r26 }
    6350:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shli r5, r6, 5 ; ld1u r25, r26 }
    6358:	[0-9a-f]* 	{ addxi r5, r6, 5 ; nor r15, r16, r17 ; ld1u r25, r26 }
    6360:	[0-9a-f]* 	{ and r15, r16, r17 ; info 19 ; ld1u r25, r26 }
    6368:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; ld1u r25, r26 }
    6370:	[0-9a-f]* 	{ and r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    6378:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; ld1u r25, r26 }
    6380:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpeqi r15, r16, 5 ; ld1u r25, r26 }
    6388:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
    6390:	[0-9a-f]* 	{ clz r5, r6 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    6398:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    63a0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nop ; ld1u r25, r26 }
    63a8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    63b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    63b8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    63c0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld1u r25, r26 }
    63c8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    63d0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ld1u r25, r26 }
    63d8:	[0-9a-f]* 	{ revbits r5, r6 ; cmples r15, r16, r17 ; ld1u r25, r26 }
    63e0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; info 19 ; ld1u r25, r26 }
    63e8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpeq r5, r6, r7 ; ld1u r25, r26 }
    63f0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
    63f8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; nop ; ld1u r25, r26 }
    6400:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    6408:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    6410:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    6418:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1u r25, r26 }
    6420:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    6428:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ld1u r25, r26 }
    6430:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    6438:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; info 19 ; ld1u r25, r26 }
    6440:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpeq r5, r6, r7 ; ld1u r25, r26 }
    6448:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
    6450:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; nop ; ld1u r25, r26 }
    6458:	[0-9a-f]* 	{ ctz r5, r6 ; jr r15 ; ld1u r25, r26 }
    6460:	[0-9a-f]* 	{ clz r5, r6 ; ld1u r25, r26 }
    6468:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ld1u r25, r26 }
    6470:	[0-9a-f]* 	{ shru r5, r6, r7 ; ld1u r25, r26 }
    6478:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nop ; ld1u r25, r26 }
    6480:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; ill ; ld1u r25, r26 }
    6488:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ill ; ld1u r25, r26 }
    6490:	[0-9a-f]* 	{ info 19 ; jalr r15 ; ld1u r25, r26 }
    6498:	[0-9a-f]* 	{ info 19 ; shl1addx r5, r6, r7 ; ld1u r25, r26 }
    64a0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jalr r15 ; ld1u r25, r26 }
    64a8:	[0-9a-f]* 	{ shli r5, r6, 5 ; jalr r15 ; ld1u r25, r26 }
    64b0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jalrp r15 ; ld1u r25, r26 }
    64b8:	[0-9a-f]* 	{ and r5, r6, r7 ; jr r15 ; ld1u r25, r26 }
    64c0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jr r15 ; ld1u r25, r26 }
    64c8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jrp r15 ; ld1u r25, r26 }
    64d0:	[0-9a-f]* 	{ xor r5, r6, r7 ; jrp r15 ; ld1u r25, r26 }
    64d8:	[0-9a-f]* 	{ pcnt r5, r6 ; lnk r15 ; ld1u r25, r26 }
    64e0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpltu r5, r6, r7 ; ld1u r25, r26 }
    64e8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; ld1u r25, r26 }
    64f0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl1add r15, r16, r17 ; ld1u r25, r26 }
    64f8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; move r15, r16 ; ld1u r25, r26 }
    6500:	[0-9a-f]* 	{ move r5, r6 ; addx r15, r16, r17 ; ld1u r25, r26 }
    6508:	[0-9a-f]* 	{ move r5, r6 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    6510:	[0-9a-f]* 	{ movei r15, 5 ; nop ; ld1u r25, r26 }
    6518:	[0-9a-f]* 	{ movei r5, 5 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    6520:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; ld1u r25, r26 }
    6528:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
    6530:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    6538:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    6540:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
    6548:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalr r15 ; ld1u r25, r26 }
    6550:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    6558:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; ld1u r25, r26 }
    6560:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
    6568:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    6570:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotl r15, r16, r17 ; ld1u r25, r26 }
    6578:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mnz r15, r16, r17 ; ld1u r25, r26 }
    6580:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpltu r5, r6, r7 ; ld1u r25, r26 }
    6588:	[0-9a-f]* 	{ mz r15, r16, r17 ; sub r5, r6, r7 ; ld1u r25, r26 }
    6590:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl1add r15, r16, r17 ; ld1u r25, r26 }
    6598:	[0-9a-f]* 	{ nop ; cmpltsi r15, r16, 5 ; ld1u r25, r26 }
    65a0:	[0-9a-f]* 	{ revbits r5, r6 ; nop ; ld1u r25, r26 }
    65a8:	[0-9a-f]* 	{ nop ; ld1u r25, r26 }
    65b0:	[0-9a-f]* 	{ revbits r5, r6 ; nor r15, r16, r17 ; ld1u r25, r26 }
    65b8:	[0-9a-f]* 	{ nor r5, r6, r7 ; info 19 ; ld1u r25, r26 }
    65c0:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpeq r5, r6, r7 ; ld1u r25, r26 }
    65c8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
    65d0:	[0-9a-f]* 	{ or r5, r6, r7 ; nop ; ld1u r25, r26 }
    65d8:	[0-9a-f]* 	{ pcnt r5, r6 ; jr r15 ; ld1u r25, r26 }
    65e0:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    65e8:	[0-9a-f]* 	{ revbytes r5, r6 ; andi r15, r16, 5 ; ld1u r25, r26 }
    65f0:	[0-9a-f]* 	{ revbytes r5, r6 ; xor r15, r16, r17 ; ld1u r25, r26 }
    65f8:	[0-9a-f]* 	{ pcnt r5, r6 ; rotl r15, r16, r17 ; ld1u r25, r26 }
    6600:	[0-9a-f]* 	{ rotl r5, r6, r7 ; ill ; ld1u r25, r26 }
    6608:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; ld1u r25, r26 }
    6610:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl3add r5, r6, r7 ; ld1u r25, r26 }
    6618:	[0-9a-f]* 	{ rotli r5, r6, 5 ; mz r15, r16, r17 ; ld1u r25, r26 }
    6620:	[0-9a-f]* 	{ shl r15, r16, r17 ; ld1u r25, r26 }
    6628:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; ld1u r25, r26 }
    6630:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    6638:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; ld1u r25, r26 }
    6640:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; andi r15, r16, 5 ; ld1u r25, r26 }
    6648:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
    6650:	[0-9a-f]* 	{ pcnt r5, r6 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    6658:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; ill ; ld1u r25, r26 }
    6660:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2add r15, r16, r17 ; ld1u r25, r26 }
    6668:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl3add r5, r6, r7 ; ld1u r25, r26 }
    6670:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mz r15, r16, r17 ; ld1u r25, r26 }
    6678:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; ld1u r25, r26 }
    6680:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    6688:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    6690:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    6698:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; andi r15, r16, 5 ; ld1u r25, r26 }
    66a0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
    66a8:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    66b0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; ill ; ld1u r25, r26 }
    66b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    66c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl3add r5, r6, r7 ; ld1u r25, r26 }
    66c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; mz r15, r16, r17 ; ld1u r25, r26 }
    66d0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; ld1u r25, r26 }
    66d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    66e0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    66e8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    66f0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; andi r15, r16, 5 ; ld1u r25, r26 }
    66f8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; xor r15, r16, r17 ; ld1u r25, r26 }
    6700:	[0-9a-f]* 	{ pcnt r5, r6 ; shru r15, r16, r17 ; ld1u r25, r26 }
    6708:	[0-9a-f]* 	{ shru r5, r6, r7 ; ill ; ld1u r25, r26 }
    6710:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    6718:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl3add r5, r6, r7 ; ld1u r25, r26 }
    6720:	[0-9a-f]* 	{ shrui r5, r6, 5 ; mz r15, r16, r17 ; ld1u r25, r26 }
    6728:	[0-9a-f]* 	{ sub r15, r16, r17 ; ld1u r25, r26 }
    6730:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
    6738:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    6740:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; ld1u r25, r26 }
    6748:	[0-9a-f]* 	{ subx r5, r6, r7 ; andi r15, r16, 5 ; ld1u r25, r26 }
    6750:	[0-9a-f]* 	{ subx r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
    6758:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shli r15, r16, 5 ; ld1u r25, r26 }
    6760:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; ld1u r25, r26 }
    6768:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; ld1u r25, r26 }
    6770:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 ; ld1u r25, r26 }
    6778:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld1u r25, r26 }
    6780:	[0-9a-f]* 	{ xor r15, r16, r17 ; shli r5, r6, 5 ; ld1u r25, r26 }
    6788:	[0-9a-f]* 	{ xor r5, r6, r7 ; nor r15, r16, r17 ; ld1u r25, r26 }
    6790:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ld1u_add r15, r16, 5 }
    6798:	[0-9a-f]* 	{ moveli r5, 4660 ; ld1u_add r15, r16, 5 }
    67a0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; ld1u_add r15, r16, 5 }
    67a8:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; ld1u_add r15, r16, 5 }
    67b0:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; ld1u_add r15, r16, 5 }
    67b8:	[0-9a-f]* 	{ addi r5, r6, 5 ; ld2s r15, r16 }
    67c0:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; ld2s r15, r16 }
    67c8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ld2s r15, r16 }
    67d0:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; ld2s r15, r16 }
    67d8:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; ld2s r15, r16 }
    67e0:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; ld2s r15, r16 }
    67e8:	[0-9a-f]* 	{ add r15, r16, r17 ; move r5, r6 ; ld2s r25, r26 }
    67f0:	[0-9a-f]* 	{ add r15, r16, r17 ; ld2s r25, r26 }
    67f8:	[0-9a-f]* 	{ add r5, r6, r7 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    6800:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addi r15, r16, 5 ; ld2s r25, r26 }
    6808:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
    6810:	[0-9a-f]* 	{ addx r15, r16, r17 ; addx r5, r6, r7 ; ld2s r25, r26 }
    6818:	[0-9a-f]* 	{ addx r15, r16, r17 ; rotli r5, r6, 5 ; ld2s r25, r26 }
    6820:	[0-9a-f]* 	{ addx r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    6828:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    6830:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
    6838:	[0-9a-f]* 	{ addxi r5, r6, 5 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    6840:	[0-9a-f]* 	{ and r15, r16, r17 ; move r5, r6 ; ld2s r25, r26 }
    6848:	[0-9a-f]* 	{ and r15, r16, r17 ; ld2s r25, r26 }
    6850:	[0-9a-f]* 	{ and r5, r6, r7 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    6858:	[0-9a-f]* 	{ mulax r5, r6, r7 ; andi r15, r16, 5 ; ld2s r25, r26 }
    6860:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
    6868:	[0-9a-f]* 	{ clz r5, r6 ; addx r15, r16, r17 ; ld2s r25, r26 }
    6870:	[0-9a-f]* 	{ clz r5, r6 ; shrui r15, r16, 5 ; ld2s r25, r26 }
    6878:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
    6880:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    6888:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
    6890:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; xor r5, r6, r7 ; ld2s r25, r26 }
    6898:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shli r15, r16, 5 ; ld2s r25, r26 }
    68a0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    68a8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    68b0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    68b8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    68c0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    68c8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmples r5, r6, r7 ; ld2s r25, r26 }
    68d0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shrs r5, r6, r7 ; ld2s r25, r26 }
    68d8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    68e0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
    68e8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; xor r5, r6, r7 ; ld2s r25, r26 }
    68f0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shli r15, r16, 5 ; ld2s r25, r26 }
    68f8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld2s r25, r26 }
    6900:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    6908:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    6910:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    6918:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    6920:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmples r5, r6, r7 ; ld2s r25, r26 }
    6928:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrs r5, r6, r7 ; ld2s r25, r26 }
    6930:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    6938:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 ; ld2s r25, r26 }
    6940:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; ld2s r25, r26 }
    6948:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; ld2s r25, r26 }
    6950:	[0-9a-f]* 	{ shrui r5, r6, 5 ; ld2s r25, r26 }
    6958:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; or r15, r16, r17 ; ld2s r25, r26 }
    6960:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; ld2s r25, r26 }
    6968:	[0-9a-f]* 	{ xor r5, r6, r7 ; ill ; ld2s r25, r26 }
    6970:	[0-9a-f]* 	{ info 19 ; jr r15 ; ld2s r25, r26 }
    6978:	[0-9a-f]* 	{ info 19 ; shl2add r5, r6, r7 ; ld2s r25, r26 }
    6980:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    6988:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jalr r15 ; ld2s r25, r26 }
    6990:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    6998:	[0-9a-f]* 	{ clz r5, r6 ; jr r15 ; ld2s r25, r26 }
    69a0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    69a8:	[0-9a-f]* 	{ movei r5, 5 ; jrp r15 ; ld2s r25, r26 }
    69b0:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; ld2s r25, r26 }
    69b8:	[0-9a-f]* 	{ revbytes r5, r6 ; lnk r15 ; ld2s r25, r26 }
    69c0:	[0-9a-f]* 	{ ctz r5, r6 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    69c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    69d0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl2add r15, r16, r17 ; ld2s r25, r26 }
    69d8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; move r15, r16 ; ld2s r25, r26 }
    69e0:	[0-9a-f]* 	{ move r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
    69e8:	[0-9a-f]* 	{ move r5, r6 ; subx r15, r16, r17 ; ld2s r25, r26 }
    69f0:	[0-9a-f]* 	{ movei r15, 5 ; or r5, r6, r7 ; ld2s r25, r26 }
    69f8:	[0-9a-f]* 	{ movei r5, 5 ; ld2s r25, r26 }
    6a00:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6a08:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
    6a10:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    6a18:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    6a20:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; nop ; ld2s r25, r26 }
    6a28:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    6a30:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 ; ld2s r25, r26 }
    6a38:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; ld2s r25, r26 }
    6a40:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; ld2s r25, r26 }
    6a48:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shli r15, r16, 5 ; ld2s r25, r26 }
    6a50:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    6a58:	[0-9a-f]* 	{ mulx r5, r6, r7 ; movei r15, 5 ; ld2s r25, r26 }
    6a60:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; ld2s r25, r26 }
    6a68:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; ld2s r25, r26 }
    6a70:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl2add r15, r16, r17 ; ld2s r25, r26 }
    6a78:	[0-9a-f]* 	{ nop ; cmpltu r15, r16, r17 ; ld2s r25, r26 }
    6a80:	[0-9a-f]* 	{ nop ; rotl r15, r16, r17 ; ld2s r25, r26 }
    6a88:	[0-9a-f]* 	{ nor r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    6a90:	[0-9a-f]* 	{ nor r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    6a98:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    6aa0:	[0-9a-f]* 	{ or r15, r16, r17 ; cmples r5, r6, r7 ; ld2s r25, r26 }
    6aa8:	[0-9a-f]* 	{ or r15, r16, r17 ; shrs r5, r6, r7 ; ld2s r25, r26 }
    6ab0:	[0-9a-f]* 	{ or r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    6ab8:	[0-9a-f]* 	{ pcnt r5, r6 ; lnk r15 ; ld2s r25, r26 }
    6ac0:	[0-9a-f]* 	{ revbits r5, r6 ; ld2s r25, r26 }
    6ac8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6ad0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; add r5, r6, r7 ; ld2s r25, r26 }
    6ad8:	[0-9a-f]* 	{ revbytes r5, r6 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    6ae0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    6ae8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
    6af0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shli r5, r6, 5 ; ld2s r25, r26 }
    6af8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nor r15, r16, r17 ; ld2s r25, r26 }
    6b00:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; ld2s r25, r26 }
    6b08:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; ld2s r25, r26 }
    6b10:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    6b18:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    6b20:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6b28:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; add r5, r6, r7 ; ld2s r25, r26 }
    6b30:	[0-9a-f]* 	{ revbytes r5, r6 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    6b38:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    6b40:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
    6b48:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shli r5, r6, 5 ; ld2s r25, r26 }
    6b50:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
    6b58:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; info 19 ; ld2s r25, r26 }
    6b60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
    6b68:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    6b70:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; ld2s r25, r26 }
    6b78:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6b80:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; add r5, r6, r7 ; ld2s r25, r26 }
    6b88:	[0-9a-f]* 	{ revbytes r5, r6 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    6b90:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    6b98:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
    6ba0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shli r5, r6, 5 ; ld2s r25, r26 }
    6ba8:	[0-9a-f]* 	{ shli r5, r6, 5 ; nor r15, r16, r17 ; ld2s r25, r26 }
    6bb0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; info 19 ; ld2s r25, r26 }
    6bb8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    6bc0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    6bc8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    6bd0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6bd8:	[0-9a-f]* 	{ shru r15, r16, r17 ; add r5, r6, r7 ; ld2s r25, r26 }
    6be0:	[0-9a-f]* 	{ revbytes r5, r6 ; shru r15, r16, r17 ; ld2s r25, r26 }
    6be8:	[0-9a-f]* 	{ shru r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
    6bf0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
    6bf8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shli r5, r6, 5 ; ld2s r25, r26 }
    6c00:	[0-9a-f]* 	{ shrui r5, r6, 5 ; nor r15, r16, r17 ; ld2s r25, r26 }
    6c08:	[0-9a-f]* 	{ sub r15, r16, r17 ; info 19 ; ld2s r25, r26 }
    6c10:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; ld2s r25, r26 }
    6c18:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    6c20:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; subx r15, r16, r17 ; ld2s r25, r26 }
    6c28:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    6c30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; add r15, r16, r17 ; ld2s r25, r26 }
    6c38:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    6c40:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    6c48:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; ld2s r25, r26 }
    6c50:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jr r15 ; ld2s r25, r26 }
    6c58:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    6c60:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
    6c68:	[0-9a-f]* 	{ xor r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    6c70:	[0-9a-f]* 	{ cmpltui r5, r6, 5 ; ld2s_add r15, r16, 5 }
    6c78:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; ld2s_add r15, r16, 5 }
    6c80:	[0-9a-f]* 	{ shlx r5, r6, r7 ; ld2s_add r15, r16, 5 }
    6c88:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; ld2s_add r15, r16, 5 }
    6c90:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; ld2s_add r15, r16, 5 }
    6c98:	[0-9a-f]* 	{ addx r5, r6, r7 ; ld2u r15, r16 }
    6ca0:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; ld2u r15, r16 }
    6ca8:	[0-9a-f]* 	{ mz r5, r6, r7 ; ld2u r15, r16 }
    6cb0:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; ld2u r15, r16 }
    6cb8:	[0-9a-f]* 	{ v2add r5, r6, r7 ; ld2u r15, r16 }
    6cc0:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; ld2u r15, r16 }
    6cc8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    6cd0:	[0-9a-f]* 	{ add r5, r6, r7 ; addi r15, r16, 5 ; ld2u r25, r26 }
    6cd8:	[0-9a-f]* 	{ add r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    6ce0:	[0-9a-f]* 	{ addi r15, r16, 5 ; mz r5, r6, r7 ; ld2u r25, r26 }
    6ce8:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
    6cf0:	[0-9a-f]* 	{ addx r15, r16, r17 ; and r5, r6, r7 ; ld2u r25, r26 }
    6cf8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl1add r5, r6, r7 ; ld2u r25, r26 }
    6d00:	[0-9a-f]* 	{ addx r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    6d08:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld2u r25, r26 }
    6d10:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrui r5, r6, 5 ; ld2u r25, r26 }
    6d18:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl r15, r16, r17 ; ld2u r25, r26 }
    6d20:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
    6d28:	[0-9a-f]* 	{ and r5, r6, r7 ; addi r15, r16, 5 ; ld2u r25, r26 }
    6d30:	[0-9a-f]* 	{ and r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    6d38:	[0-9a-f]* 	{ andi r15, r16, 5 ; mz r5, r6, r7 ; ld2u r25, r26 }
    6d40:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
    6d48:	[0-9a-f]* 	{ clz r5, r6 ; and r15, r16, r17 ; ld2u r25, r26 }
    6d50:	[0-9a-f]* 	{ clz r5, r6 ; subx r15, r16, r17 ; ld2u r25, r26 }
    6d58:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    6d60:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6d68:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; movei r5, 5 ; ld2u r25, r26 }
    6d70:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    6d78:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    6d80:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2u r25, r26 }
    6d88:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
    6d90:	[0-9a-f]* 	{ cmples r15, r16, r17 ; addxi r5, r6, 5 ; ld2u r25, r26 }
    6d98:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl r5, r6, r7 ; ld2u r25, r26 }
    6da0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    6da8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmplts r5, r6, r7 ; ld2u r25, r26 }
    6db0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shru r5, r6, r7 ; ld2u r25, r26 }
    6db8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6dc0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; movei r5, 5 ; ld2u r25, r26 }
    6dc8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    6dd0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    6dd8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld2u r25, r26 }
    6de0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
    6de8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; addxi r5, r6, 5 ; ld2u r25, r26 }
    6df0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl r5, r6, r7 ; ld2u r25, r26 }
    6df8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    6e00:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmplts r5, r6, r7 ; ld2u r25, r26 }
    6e08:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shru r5, r6, r7 ; ld2u r25, r26 }
    6e10:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6e18:	[0-9a-f]* 	{ ctz r5, r6 ; move r15, r16 ; ld2u r25, r26 }
    6e20:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ld2u r25, r26 }
    6e28:	[0-9a-f]* 	{ mulx r5, r6, r7 ; ld2u r25, r26 }
    6e30:	[0-9a-f]* 	{ sub r5, r6, r7 ; ld2u r25, r26 }
    6e38:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6e40:	[0-9a-f]* 	{ movei r5, 5 ; ill ; ld2u r25, r26 }
    6e48:	[0-9a-f]* 	{ info 19 ; add r15, r16, r17 ; ld2u r25, r26 }
    6e50:	[0-9a-f]* 	{ info 19 ; lnk r15 ; ld2u r25, r26 }
    6e58:	[0-9a-f]* 	{ info 19 ; shl2addx r5, r6, r7 ; ld2u r25, r26 }
    6e60:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; jalr r15 ; ld2u r25, r26 }
    6e68:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jalr r15 ; ld2u r25, r26 }
    6e70:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    6e78:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    6e80:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    6e88:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    6e90:	[0-9a-f]* 	{ addx r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    6e98:	[0-9a-f]* 	{ rotli r5, r6, 5 ; lnk r15 ; ld2u r25, r26 }
    6ea0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    6ea8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    6eb0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl3add r15, r16, r17 ; ld2u r25, r26 }
    6eb8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; move r15, r16 ; ld2u r25, r26 }
    6ec0:	[0-9a-f]* 	{ move r5, r6 ; cmpeq r15, r16, r17 ; ld2u r25, r26 }
    6ec8:	[0-9a-f]* 	{ move r5, r6 ; ld2u r25, r26 }
    6ed0:	[0-9a-f]* 	{ revbits r5, r6 ; movei r15, 5 ; ld2u r25, r26 }
    6ed8:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; ld2u r25, r26 }
    6ee0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    6ee8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; addx r15, r16, r17 ; ld2u r25, r26 }
    6ef0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrui r15, r16, 5 ; ld2u r25, r26 }
    6ef8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
    6f00:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
    6f08:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    6f10:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ld2u r25, r26 }
    6f18:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2u r25, r26 }
    6f20:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    6f28:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    6f30:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    6f38:	[0-9a-f]* 	{ mulx r5, r6, r7 ; nop ; ld2u r25, r26 }
    6f40:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mz r15, r16, r17 ; ld2u r25, r26 }
    6f48:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; ld2u r25, r26 }
    6f50:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl3add r15, r16, r17 ; ld2u r25, r26 }
    6f58:	[0-9a-f]* 	{ nop ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    6f60:	[0-9a-f]* 	{ nop ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6f68:	[0-9a-f]* 	{ nor r15, r16, r17 ; addxi r5, r6, 5 ; ld2u r25, r26 }
    6f70:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl r5, r6, r7 ; ld2u r25, r26 }
    6f78:	[0-9a-f]* 	{ nor r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
    6f80:	[0-9a-f]* 	{ or r15, r16, r17 ; cmplts r5, r6, r7 ; ld2u r25, r26 }
    6f88:	[0-9a-f]* 	{ or r15, r16, r17 ; shru r5, r6, r7 ; ld2u r25, r26 }
    6f90:	[0-9a-f]* 	{ or r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    6f98:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; ld2u r25, r26 }
    6fa0:	[0-9a-f]* 	{ revbits r5, r6 ; info 19 ; ld2u r25, r26 }
    6fa8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    6fb0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; addx r5, r6, r7 ; ld2u r25, r26 }
    6fb8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; rotli r5, r6, 5 ; ld2u r25, r26 }
    6fc0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    6fc8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpleu r5, r6, r7 ; ld2u r25, r26 }
    6fd0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrsi r5, r6, 5 ; ld2u r25, r26 }
    6fd8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    6fe0:	[0-9a-f]* 	{ shl r15, r16, r17 ; move r5, r6 ; ld2u r25, r26 }
    6fe8:	[0-9a-f]* 	{ shl r15, r16, r17 ; ld2u r25, r26 }
    6ff0:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    6ff8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1add r15, r16, r17 ; ld2u r25, r26 }
    7000:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    7008:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addx r5, r6, r7 ; ld2u r25, r26 }
    7010:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; rotli r5, r6, 5 ; ld2u r25, r26 }
    7018:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    7020:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpleu r5, r6, r7 ; ld2u r25, r26 }
    7028:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrsi r5, r6, 5 ; ld2u r25, r26 }
    7030:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    7038:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; move r5, r6 ; ld2u r25, r26 }
    7040:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; ld2u r25, r26 }
    7048:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    7050:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3add r15, r16, r17 ; ld2u r25, r26 }
    7058:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    7060:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addx r5, r6, r7 ; ld2u r25, r26 }
    7068:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; rotli r5, r6, 5 ; ld2u r25, r26 }
    7070:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    7078:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpleu r5, r6, r7 ; ld2u r25, r26 }
    7080:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrsi r5, r6, 5 ; ld2u r25, r26 }
    7088:	[0-9a-f]* 	{ shli r5, r6, 5 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    7090:	[0-9a-f]* 	{ shrs r15, r16, r17 ; move r5, r6 ; ld2u r25, r26 }
    7098:	[0-9a-f]* 	{ shrs r15, r16, r17 ; ld2u r25, r26 }
    70a0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    70a8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    70b0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    70b8:	[0-9a-f]* 	{ shru r15, r16, r17 ; addx r5, r6, r7 ; ld2u r25, r26 }
    70c0:	[0-9a-f]* 	{ shru r15, r16, r17 ; rotli r5, r6, 5 ; ld2u r25, r26 }
    70c8:	[0-9a-f]* 	{ shru r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    70d0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpleu r5, r6, r7 ; ld2u r25, r26 }
    70d8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shrsi r5, r6, 5 ; ld2u r25, r26 }
    70e0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    70e8:	[0-9a-f]* 	{ sub r15, r16, r17 ; move r5, r6 ; ld2u r25, r26 }
    70f0:	[0-9a-f]* 	{ sub r15, r16, r17 ; ld2u r25, r26 }
    70f8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    7100:	[0-9a-f]* 	{ mulax r5, r6, r7 ; subx r15, r16, r17 ; ld2u r25, r26 }
    7108:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    7110:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; ld2u r25, r26 }
    7118:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; ld2u r25, r26 }
    7120:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
    7128:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; ld2u r25, r26 }
    7130:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; lnk r15 ; ld2u r25, r26 }
    7138:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2u r25, r26 }
    7140:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrui r5, r6, 5 ; ld2u r25, r26 }
    7148:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; ld2u r25, r26 }
    7150:	[0-9a-f]* 	{ cmul r5, r6, r7 ; ld2u_add r15, r16, 5 }
    7158:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; ld2u_add r15, r16, 5 }
    7160:	[0-9a-f]* 	{ shrs r5, r6, r7 ; ld2u_add r15, r16, 5 }
    7168:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; ld2u_add r15, r16, 5 }
    7170:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; ld2u_add r15, r16, 5 }
    7178:	[0-9a-f]* 	{ addxli r5, r6, 4660 ; ld4s r15, r16 }
    7180:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; ld4s r15, r16 }
    7188:	[0-9a-f]* 	{ nor r5, r6, r7 ; ld4s r15, r16 }
    7190:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; ld4s r15, r16 }
    7198:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; ld4s r15, r16 }
    71a0:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; ld4s r15, r16 }
    71a8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
    71b0:	[0-9a-f]* 	{ add r5, r6, r7 ; addxi r15, r16, 5 ; ld4s r25, r26 }
    71b8:	[0-9a-f]* 	{ add r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
    71c0:	[0-9a-f]* 	{ addi r15, r16, 5 ; nor r5, r6, r7 ; ld4s r25, r26 }
    71c8:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    71d0:	[0-9a-f]* 	{ clz r5, r6 ; addx r15, r16, r17 ; ld4s r25, r26 }
    71d8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl2add r5, r6, r7 ; ld4s r25, r26 }
    71e0:	[0-9a-f]* 	{ addx r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    71e8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpne r5, r6, r7 ; ld4s r25, r26 }
    71f0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; subx r5, r6, r7 ; ld4s r25, r26 }
    71f8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld4s r25, r26 }
    7200:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    7208:	[0-9a-f]* 	{ and r5, r6, r7 ; addxi r15, r16, 5 ; ld4s r25, r26 }
    7210:	[0-9a-f]* 	{ and r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
    7218:	[0-9a-f]* 	{ andi r15, r16, 5 ; nor r5, r6, r7 ; ld4s r25, r26 }
    7220:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    7228:	[0-9a-f]* 	{ clz r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    7230:	[0-9a-f]* 	{ clz r5, r6 ; ld4s r25, r26 }
    7238:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
    7240:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    7248:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    7250:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
    7258:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shrui r15, r16, 5 ; ld4s r25, r26 }
    7260:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; nop ; ld4s r25, r26 }
    7268:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
    7270:	[0-9a-f]* 	{ cmples r15, r16, r17 ; andi r5, r6, 5 ; ld4s r25, r26 }
    7278:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    7280:	[0-9a-f]* 	{ cmples r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    7288:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpltu r5, r6, r7 ; ld4s r25, r26 }
    7290:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; sub r5, r6, r7 ; ld4s r25, r26 }
    7298:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    72a0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    72a8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
    72b0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shrui r15, r16, 5 ; ld4s r25, r26 }
    72b8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; nop ; ld4s r25, r26 }
    72c0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
    72c8:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; andi r5, r6, 5 ; ld4s r25, r26 }
    72d0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    72d8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    72e0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpltu r5, r6, r7 ; ld4s r25, r26 }
    72e8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; sub r5, r6, r7 ; ld4s r25, r26 }
    72f0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    72f8:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; ld4s r25, r26 }
    7300:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ld4s r25, r26 }
    7308:	[0-9a-f]* 	{ mz r5, r6, r7 ; ld4s r25, r26 }
    7310:	[0-9a-f]* 	{ subx r5, r6, r7 ; ld4s r25, r26 }
    7318:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    7320:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ill ; ld4s r25, r26 }
    7328:	[0-9a-f]* 	{ info 19 ; addi r15, r16, 5 ; ld4s r25, r26 }
    7330:	[0-9a-f]* 	{ info 19 ; mnz r5, r6, r7 ; ld4s r25, r26 }
    7338:	[0-9a-f]* 	{ info 19 ; shl3add r5, r6, r7 ; ld4s r25, r26 }
    7340:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    7348:	[0-9a-f]* 	{ subx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    7350:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jalrp r15 ; ld4s r25, r26 }
    7358:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jr r15 ; ld4s r25, r26 }
    7360:	[0-9a-f]* 	{ shli r5, r6, 5 ; jr r15 ; ld4s r25, r26 }
    7368:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    7370:	[0-9a-f]* 	{ and r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    7378:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    7380:	[0-9a-f]* 	{ mnz r15, r16, r17 ; mnz r5, r6, r7 ; ld4s r25, r26 }
    7388:	[0-9a-f]* 	{ mnz r15, r16, r17 ; xor r5, r6, r7 ; ld4s r25, r26 }
    7390:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shli r15, r16, 5 ; ld4s r25, r26 }
    7398:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    73a0:	[0-9a-f]* 	{ move r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    73a8:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 ; ld4s r25, r26 }
    73b0:	[0-9a-f]* 	{ movei r15, 5 ; rotl r5, r6, r7 ; ld4s r25, r26 }
    73b8:	[0-9a-f]* 	{ movei r5, 5 ; jalrp r15 ; ld4s r25, r26 }
    73c0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    73c8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    73d0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    73d8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    73e0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; ld4s r25, r26 }
    73e8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    73f0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; info 19 ; ld4s r25, r26 }
    73f8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; ld4s r25, r26 }
    7400:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
    7408:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shrui r15, r16, 5 ; ld4s r25, r26 }
    7410:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    7418:	[0-9a-f]* 	{ mulx r5, r6, r7 ; or r15, r16, r17 ; ld4s r25, r26 }
    7420:	[0-9a-f]* 	{ mz r15, r16, r17 ; mnz r5, r6, r7 ; ld4s r25, r26 }
    7428:	[0-9a-f]* 	{ mz r15, r16, r17 ; xor r5, r6, r7 ; ld4s r25, r26 }
    7430:	[0-9a-f]* 	{ mz r5, r6, r7 ; shli r15, r16, 5 ; ld4s r25, r26 }
    7438:	[0-9a-f]* 	{ ctz r5, r6 ; nop ; ld4s r25, r26 }
    7440:	[0-9a-f]* 	{ nop ; shl r15, r16, r17 ; ld4s r25, r26 }
    7448:	[0-9a-f]* 	{ nor r15, r16, r17 ; andi r5, r6, 5 ; ld4s r25, r26 }
    7450:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    7458:	[0-9a-f]* 	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    7460:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpltu r5, r6, r7 ; ld4s r25, r26 }
    7468:	[0-9a-f]* 	{ or r15, r16, r17 ; sub r5, r6, r7 ; ld4s r25, r26 }
    7470:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    7478:	[0-9a-f]* 	{ pcnt r5, r6 ; mz r15, r16, r17 ; ld4s r25, r26 }
    7480:	[0-9a-f]* 	{ revbits r5, r6 ; jalrp r15 ; ld4s r25, r26 }
    7488:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    7490:	[0-9a-f]* 	{ rotl r15, r16, r17 ; and r5, r6, r7 ; ld4s r25, r26 }
    7498:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl1add r5, r6, r7 ; ld4s r25, r26 }
    74a0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    74a8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld4s r25, r26 }
    74b0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrui r5, r6, 5 ; ld4s r25, r26 }
    74b8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl r15, r16, r17 ; ld4s r25, r26 }
    74c0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl r15, r16, r17 ; ld4s r25, r26 }
    74c8:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
    74d0:	[0-9a-f]* 	{ shl r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
    74d8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; mz r5, r6, r7 ; ld4s r25, r26 }
    74e0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    74e8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; and r5, r6, r7 ; ld4s r25, r26 }
    74f0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl1add r5, r6, r7 ; ld4s r25, r26 }
    74f8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    7500:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld4s r25, r26 }
    7508:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrui r5, r6, 5 ; ld4s r25, r26 }
    7510:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl r15, r16, r17 ; ld4s r25, r26 }
    7518:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    7520:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
    7528:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
    7530:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; mz r5, r6, r7 ; ld4s r25, r26 }
    7538:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    7540:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; and r5, r6, r7 ; ld4s r25, r26 }
    7548:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl1add r5, r6, r7 ; ld4s r25, r26 }
    7550:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    7558:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld4s r25, r26 }
    7560:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrui r5, r6, 5 ; ld4s r25, r26 }
    7568:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; ld4s r25, r26 }
    7570:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
    7578:	[0-9a-f]* 	{ shrs r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
    7580:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
    7588:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; mz r5, r6, r7 ; ld4s r25, r26 }
    7590:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    7598:	[0-9a-f]* 	{ shru r15, r16, r17 ; and r5, r6, r7 ; ld4s r25, r26 }
    75a0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl1add r5, r6, r7 ; ld4s r25, r26 }
    75a8:	[0-9a-f]* 	{ shru r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    75b0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld4s r25, r26 }
    75b8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shrui r5, r6, 5 ; ld4s r25, r26 }
    75c0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl r15, r16, r17 ; ld4s r25, r26 }
    75c8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
    75d0:	[0-9a-f]* 	{ sub r5, r6, r7 ; addi r15, r16, 5 ; ld4s r25, r26 }
    75d8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
    75e0:	[0-9a-f]* 	{ subx r15, r16, r17 ; mz r5, r6, r7 ; ld4s r25, r26 }
    75e8:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    75f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld4s r25, r26 }
    75f8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; subx r15, r16, r17 ; ld4s r25, r26 }
    7600:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    7608:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rotli r15, r16, 5 ; ld4s r25, r26 }
    7610:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; ld4s r25, r26 }
    7618:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpne r5, r6, r7 ; ld4s r25, r26 }
    7620:	[0-9a-f]* 	{ xor r15, r16, r17 ; subx r5, r6, r7 ; ld4s r25, r26 }
    7628:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4s r25, r26 }
    7630:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; ld4s_add r15, r16, 5 }
    7638:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; ld4s_add r15, r16, 5 }
    7640:	[0-9a-f]* 	{ shru r5, r6, r7 ; ld4s_add r15, r16, 5 }
    7648:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; ld4s_add r15, r16, 5 }
    7650:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; ld4s_add r15, r16, 5 }
    7658:	[0-9a-f]* 	{ and r5, r6, r7 ; ld4u r15, r16 }
    7660:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; ld4u r15, r16 }
    7668:	[0-9a-f]* 	{ ori r5, r6, 5 ; ld4u r15, r16 }
    7670:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; ld4u r15, r16 }
    7678:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; ld4u r15, r16 }
    7680:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; ld4u r15, r16 }
    7688:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    7690:	[0-9a-f]* 	{ add r5, r6, r7 ; andi r15, r16, 5 ; ld4u r25, r26 }
    7698:	[0-9a-f]* 	{ add r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
    76a0:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; ld4u r25, r26 }
    76a8:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; ld4u r25, r26 }
    76b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    76b8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl3add r5, r6, r7 ; ld4u r25, r26 }
    76c0:	[0-9a-f]* 	{ addx r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
    76c8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; ld4u r25, r26 }
    76d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    76d8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    76e0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    76e8:	[0-9a-f]* 	{ and r5, r6, r7 ; andi r15, r16, 5 ; ld4u r25, r26 }
    76f0:	[0-9a-f]* 	{ and r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
    76f8:	[0-9a-f]* 	{ pcnt r5, r6 ; andi r15, r16, 5 ; ld4u r25, r26 }
    7700:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; ld4u r25, r26 }
    7708:	[0-9a-f]* 	{ clz r5, r6 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    7710:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addi r15, r16, 5 ; ld4u r25, r26 }
    7718:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    7720:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    7728:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    7730:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    7738:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
    7740:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; or r5, r6, r7 ; ld4u r25, r26 }
    7748:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ld4u r25, r26 }
    7750:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    7758:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    7760:	[0-9a-f]* 	{ cmples r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
    7768:	[0-9a-f]* 	{ ctz r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
    7770:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
    7778:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    7780:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
    7788:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    7790:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
    7798:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; or r5, r6, r7 ; ld4u r25, r26 }
    77a0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ld4u r25, r26 }
    77a8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
    77b0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    77b8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
    77c0:	[0-9a-f]* 	{ ctz r5, r6 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    77c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    77d0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    77d8:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; ld4u r25, r26 }
    77e0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ld4u r25, r26 }
    77e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; ld4u r25, r26 }
    77f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ld4u r25, r26 }
    77f8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    7800:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ill ; ld4u r25, r26 }
    7808:	[0-9a-f]* 	{ info 19 ; addx r15, r16, r17 ; ld4u r25, r26 }
    7810:	[0-9a-f]* 	{ info 19 ; move r5, r6 ; ld4u r25, r26 }
    7818:	[0-9a-f]* 	{ info 19 ; shl3addx r5, r6, r7 ; ld4u r25, r26 }
    7820:	[0-9a-f]* 	{ jalr r15 ; ld4u r25, r26 }
    7828:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jalr r15 ; ld4u r25, r26 }
    7830:	[0-9a-f]* 	{ nop ; jalrp r15 ; ld4u r25, r26 }
    7838:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; jr r15 ; ld4u r25, r26 }
    7840:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jr r15 ; ld4u r25, r26 }
    7848:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    7850:	[0-9a-f]* 	{ clz r5, r6 ; lnk r15 ; ld4u r25, r26 }
    7858:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
    7860:	[0-9a-f]* 	{ mnz r15, r16, r17 ; movei r5, 5 ; ld4u r25, r26 }
    7868:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    7870:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    7878:	[0-9a-f]* 	{ mulx r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    7880:	[0-9a-f]* 	{ move r5, r6 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
    7888:	[0-9a-f]* 	{ movei r15, 5 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    7890:	[0-9a-f]* 	{ movei r15, 5 ; shl r5, r6, r7 ; ld4u r25, r26 }
    7898:	[0-9a-f]* 	{ movei r5, 5 ; jrp r15 ; ld4u r25, r26 }
    78a0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    78a8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    78b0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ld4u r25, r26 }
    78b8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    78c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl1add r15, r16, r17 ; ld4u r25, r26 }
    78c8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
    78d0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    78d8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4u r25, r26 }
    78e0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    78e8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
    78f0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    78f8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; rotli r15, r16, 5 ; ld4u r25, r26 }
    7900:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; ld4u r25, r26 }
    7908:	[0-9a-f]* 	{ mz r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    7910:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    7918:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nop ; ld4u r25, r26 }
    7920:	[0-9a-f]* 	{ nop ; shl1add r15, r16, r17 ; ld4u r25, r26 }
    7928:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; nor r15, r16, r17 ; ld4u r25, r26 }
    7930:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    7938:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
    7940:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; ld4u r25, r26 }
    7948:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; or r15, r16, r17 ; ld4u r25, r26 }
    7950:	[0-9a-f]* 	{ or r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    7958:	[0-9a-f]* 	{ pcnt r5, r6 ; nor r15, r16, r17 ; ld4u r25, r26 }
    7960:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; ld4u r25, r26 }
    7968:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    7970:	[0-9a-f]* 	{ clz r5, r6 ; rotl r15, r16, r17 ; ld4u r25, r26 }
    7978:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl2add r5, r6, r7 ; ld4u r25, r26 }
    7980:	[0-9a-f]* 	{ rotl r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    7988:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpne r5, r6, r7 ; ld4u r25, r26 }
    7990:	[0-9a-f]* 	{ rotli r15, r16, 5 ; subx r5, r6, r7 ; ld4u r25, r26 }
    7998:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    79a0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; ld4u r25, r26 }
    79a8:	[0-9a-f]* 	{ shl r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    79b0:	[0-9a-f]* 	{ shl r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    79b8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
    79c0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    79c8:	[0-9a-f]* 	{ clz r5, r6 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    79d0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl2add r5, r6, r7 ; ld4u r25, r26 }
    79d8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    79e0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpne r5, r6, r7 ; ld4u r25, r26 }
    79e8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; subx r5, r6, r7 ; ld4u r25, r26 }
    79f0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    79f8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    7a00:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    7a08:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    7a10:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
    7a18:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    7a20:	[0-9a-f]* 	{ clz r5, r6 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    7a28:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl2add r5, r6, r7 ; ld4u r25, r26 }
    7a30:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    7a38:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpne r5, r6, r7 ; ld4u r25, r26 }
    7a40:	[0-9a-f]* 	{ shli r15, r16, 5 ; subx r5, r6, r7 ; ld4u r25, r26 }
    7a48:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    7a50:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    7a58:	[0-9a-f]* 	{ shrs r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    7a60:	[0-9a-f]* 	{ shrs r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    7a68:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; nor r5, r6, r7 ; ld4u r25, r26 }
    7a70:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    7a78:	[0-9a-f]* 	{ clz r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
    7a80:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl2add r5, r6, r7 ; ld4u r25, r26 }
    7a88:	[0-9a-f]* 	{ shru r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    7a90:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpne r5, r6, r7 ; ld4u r25, r26 }
    7a98:	[0-9a-f]* 	{ shrui r15, r16, 5 ; subx r5, r6, r7 ; ld4u r25, r26 }
    7aa0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    7aa8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    7ab0:	[0-9a-f]* 	{ sub r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    7ab8:	[0-9a-f]* 	{ sub r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    7ac0:	[0-9a-f]* 	{ subx r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
    7ac8:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    7ad0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    7ad8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ld4u r25, r26 }
    7ae0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    7ae8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl1add r15, r16, r17 ; ld4u r25, r26 }
    7af0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; ld4u r25, r26 }
    7af8:	[0-9a-f]* 	{ xor r15, r16, r17 ; ld4u r25, r26 }
    7b00:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; ld4u r25, r26 }
    7b08:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    7b10:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; ld4u_add r15, r16, 5 }
    7b18:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; ld4u_add r15, r16, 5 }
    7b20:	[0-9a-f]* 	{ shrux r5, r6, r7 ; ld4u_add r15, r16, 5 }
    7b28:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; ld4u_add r15, r16, 5 }
    7b30:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; ld4u_add r15, r16, 5 }
    7b38:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; ld_add r15, r16, 5 }
    7b40:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; ld_add r15, r16, 5 }
    7b48:	[0-9a-f]* 	{ revbits r5, r6 ; ld_add r15, r16, 5 }
    7b50:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; ld_add r15, r16, 5 }
    7b58:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; ld_add r15, r16, 5 }
    7b60:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; ld_add r15, r16, 5 }
    7b68:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; ldna r15, r16 }
    7b70:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ldna r15, r16 }
    7b78:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; ldna r15, r16 }
    7b80:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; ldna r15, r16 }
    7b88:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; ldna r15, r16 }
    7b90:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; ldna_add r15, r16, 5 }
    7b98:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; ldna_add r15, r16, 5 }
    7ba0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; ldna_add r15, r16, 5 }
    7ba8:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; ldna_add r15, r16, 5 }
    7bb0:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; ldna_add r15, r16, 5 }
    7bb8:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; ldna_add r15, r16, 5 }
    7bc0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; ldnt r15, r16 }
    7bc8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; ldnt r15, r16 }
    7bd0:	[0-9a-f]* 	{ subx r5, r6, r7 ; ldnt r15, r16 }
    7bd8:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; ldnt r15, r16 }
    7be0:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; ldnt r15, r16 }
    7be8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; ldnt1s r15, r16 }
    7bf0:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; ldnt1s r15, r16 }
    7bf8:	[0-9a-f]* 	{ shl r5, r6, r7 ; ldnt1s r15, r16 }
    7c00:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; ldnt1s r15, r16 }
    7c08:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; ldnt1s r15, r16 }
    7c10:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; ldnt1s r15, r16 }
    7c18:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    7c20:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    7c28:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ldnt1s_add r15, r16, 5 }
    7c30:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    7c38:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    7c40:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; ldnt1u r15, r16 }
    7c48:	[0-9a-f]* 	{ infol 4660 ; ldnt1u r15, r16 }
    7c50:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ldnt1u r15, r16 }
    7c58:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; ldnt1u r15, r16 }
    7c60:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; ldnt1u r15, r16 }
    7c68:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; ldnt1u r15, r16 }
    7c70:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
    7c78:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
    7c80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ldnt1u_add r15, r16, 5 }
    7c88:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; ldnt1u_add r15, r16, 5 }
    7c90:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
    7c98:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ldnt2s r15, r16 }
    7ca0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ldnt2s r15, r16 }
    7ca8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; ldnt2s r15, r16 }
    7cb0:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; ldnt2s r15, r16 }
    7cb8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; ldnt2s r15, r16 }
    7cc0:	[0-9a-f]* 	{ xor r5, r6, r7 ; ldnt2s r15, r16 }
    7cc8:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
    7cd0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
    7cd8:	[0-9a-f]* 	{ v1add r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
    7ce0:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; ldnt2s_add r15, r16, 5 }
    7ce8:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; ldnt2s_add r15, r16, 5 }
    7cf0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; ldnt2u r15, r16 }
    7cf8:	[0-9a-f]* 	{ movei r5, 5 ; ldnt2u r15, r16 }
    7d00:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; ldnt2u r15, r16 }
    7d08:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; ldnt2u r15, r16 }
    7d10:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; ldnt2u r15, r16 }
    7d18:	[0-9a-f]* 	{ add r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    7d20:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    7d28:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    7d30:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    7d38:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; ldnt2u_add r15, r16, 5 }
    7d40:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    7d48:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ldnt4s r15, r16 }
    7d50:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; ldnt4s r15, r16 }
    7d58:	[0-9a-f]* 	{ shli r5, r6, 5 ; ldnt4s r15, r16 }
    7d60:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; ldnt4s r15, r16 }
    7d68:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; ldnt4s r15, r16 }
    7d70:	[0-9a-f]* 	{ addli r5, r6, 4660 ; ldnt4s_add r15, r16, 5 }
    7d78:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    7d80:	[0-9a-f]* 	{ mulx r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    7d88:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    7d90:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    7d98:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    7da0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ldnt4u r15, r16 }
    7da8:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; ldnt4u r15, r16 }
    7db0:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; ldnt4u r15, r16 }
    7db8:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; ldnt4u r15, r16 }
    7dc0:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; ldnt4u r15, r16 }
    7dc8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ldnt4u_add r15, r16, 5 }
    7dd0:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
    7dd8:	[0-9a-f]* 	{ nop ; ldnt4u_add r15, r16, 5 }
    7de0:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; ldnt4u_add r15, r16, 5 }
    7de8:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; ldnt4u_add r15, r16, 5 }
    7df0:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
    7df8:	[0-9a-f]* 	{ cmula r5, r6, r7 ; ldnt_add r15, r16, 5 }
    7e00:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ldnt_add r15, r16, 5 }
    7e08:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ldnt_add r15, r16, 5 }
    7e10:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; ldnt_add r15, r16, 5 }
    7e18:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; ldnt_add r15, r16, 5 }
    7e20:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
    7e28:	[0-9a-f]* 	{ addx r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7e30:	[0-9a-f]* 	{ and r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7e38:	[0-9a-f]* 	{ clz r5, r6 ; lnk r15 ; ld4u r25, r26 }
    7e40:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7e48:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; lnk r15 ; prefetch_l2 r25 }
    7e50:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; lnk r15 ; prefetch_l3 r25 }
    7e58:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; lnk r15 ; st r25, r26 }
    7e60:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    7e68:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; lnk r15 }
    7e70:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; lnk r15 ; prefetch_l3_fault r25 }
    7e78:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; lnk r15 ; ld r25, r26 }
    7e80:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; lnk r15 ; ld r25, r26 }
    7e88:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; lnk r15 ; ld1s r25, r26 }
    7e90:	[0-9a-f]* 	{ clz r5, r6 ; lnk r15 ; ld1u r25, r26 }
    7e98:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; lnk r15 ; ld1u r25, r26 }
    7ea0:	[0-9a-f]* 	{ movei r5, 5 ; lnk r15 ; ld2s r25, r26 }
    7ea8:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    7eb0:	[0-9a-f]* 	{ revbytes r5, r6 ; lnk r15 ; ld2u r25, r26 }
    7eb8:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 ; ld4s r25, r26 }
    7ec0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lnk r15 ; ld4s r25, r26 }
    7ec8:	[0-9a-f]* 	{ mz r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
    7ed0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    7ed8:	[0-9a-f]* 	{ movei r5, 5 ; lnk r15 ; prefetch_l3 r25 }
    7ee0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    7ee8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    7ef0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7ef8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7f00:	[0-9a-f]* 	{ mulx r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    7f08:	[0-9a-f]* 	{ nop ; lnk r15 ; prefetch_l2_fault r25 }
    7f10:	[0-9a-f]* 	{ or r5, r6, r7 ; lnk r15 ; prefetch_l3_fault r25 }
    7f18:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; lnk r15 ; prefetch r25 }
    7f20:	[0-9a-f]* 	{ shrui r5, r6, 5 ; lnk r15 ; prefetch r25 }
    7f28:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; prefetch r25 }
    7f30:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    7f38:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    7f40:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    7f48:	[0-9a-f]* 	{ addx r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    7f50:	[0-9a-f]* 	{ rotli r5, r6, 5 ; lnk r15 ; prefetch_l2_fault r25 }
    7f58:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; lnk r15 ; prefetch_l3 r25 }
    7f60:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; lnk r15 ; prefetch_l3 r25 }
    7f68:	[0-9a-f]* 	{ nor r5, r6, r7 ; lnk r15 ; prefetch_l3_fault r25 }
    7f70:	[0-9a-f]* 	{ revbits r5, r6 ; lnk r15 ; prefetch_l3_fault r25 }
    7f78:	[0-9a-f]* 	{ rotl r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    7f80:	[0-9a-f]* 	{ shl r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    7f88:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; lnk r15 }
    7f90:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; lnk r15 ; ld1s r25, r26 }
    7f98:	[0-9a-f]* 	{ shli r5, r6, 5 ; lnk r15 ; ld2s r25, r26 }
    7fa0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; lnk r15 ; ld2s r25, r26 }
    7fa8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; lnk r15 ; ld4s r25, r26 }
    7fb0:	[0-9a-f]* 	{ movei r5, 5 ; lnk r15 ; st r25, r26 }
    7fb8:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    7fc0:	[0-9a-f]* 	{ revbytes r5, r6 ; lnk r15 ; st1 r25, r26 }
    7fc8:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 ; st2 r25, r26 }
    7fd0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lnk r15 ; st2 r25, r26 }
    7fd8:	[0-9a-f]* 	{ mz r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    7fe0:	[0-9a-f]* 	{ sub r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    7fe8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lnk r15 ; prefetch_l3 r25 }
    7ff0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; lnk r15 ; st r25, r26 }
    7ff8:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; lnk r15 }
    8000:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; lnk r15 }
    8008:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; lnk r15 }
    8010:	[0-9a-f]* 	{ cmples r5, r6, r7 ; mf }
    8018:	[0-9a-f]* 	{ mnz r5, r6, r7 ; mf }
    8020:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mf }
    8028:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; mf }
    8030:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; mf }
    8038:	[0-9a-f]* 	{ xor r5, r6, r7 ; mf }
    8040:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8048:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8050:	[0-9a-f]* 	{ v1add r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8058:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8060:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8068:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; cmpne r15, r16, r17 }
    8070:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; ld4u r15, r16 }
    8078:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; prefetch_l1_fault r15 }
    8080:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; stnt_add r15, r16, 5 }
    8088:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; v2cmpltsi r15, r16, 5 }
    8090:	[0-9a-f]* 	{ mnz r15, r16, r17 ; add r5, r6, r7 ; ld1u r25, r26 }
    8098:	[0-9a-f]* 	{ mnz r15, r16, r17 ; addx r5, r6, r7 ; ld2s r25, r26 }
    80a0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; and r5, r6, r7 ; ld2s r25, r26 }
    80a8:	[0-9a-f]* 	{ clz r5, r6 ; mnz r15, r16, r17 ; ld1u r25, r26 }
    80b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    80b8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
    80c0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch r25 }
    80c8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    80d0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    80d8:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; mnz r15, r16, r17 }
    80e0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    80e8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    80f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl3add r5, r6, r7 ; ld r25, r26 }
    80f8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; ld1s r25, r26 }
    8100:	[0-9a-f]* 	{ mnz r15, r16, r17 ; addx r5, r6, r7 ; ld1u r25, r26 }
    8108:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rotli r5, r6, 5 ; ld1u r25, r26 }
    8110:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    8118:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    8120:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nor r5, r6, r7 ; ld2u r25, r26 }
    8128:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmplts r5, r6, r7 ; ld4s r25, r26 }
    8130:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shru r5, r6, r7 ; ld4s r25, r26 }
    8138:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mnz r15, r16, r17 ; ld4u r25, r26 }
    8140:	[0-9a-f]* 	{ mnz r15, r16, r17 ; mnz r5, r6, r7 ; ld4u r25, r26 }
    8148:	[0-9a-f]* 	{ mnz r15, r16, r17 ; movei r5, 5 ; prefetch r25 }
    8150:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; ld4u r25, r26 }
    8158:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    8160:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    8168:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    8170:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    8178:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nop ; prefetch r25 }
    8180:	[0-9a-f]* 	{ mnz r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    8188:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch r25 }
    8190:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    8198:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    81a0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; and r5, r6, r7 ; prefetch_l1_fault r25 }
    81a8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l1_fault r25 }
    81b0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l2 r25 }
    81b8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l2 r25 }
    81c0:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
    81c8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3 r25 }
    81d0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; prefetch_l3 r25 }
    81d8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l3_fault r25 }
    81e0:	[0-9a-f]* 	{ revbits r5, r6 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    81e8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
    81f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
    81f8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl1addx r5, r6, r7 ; st r25, r26 }
    8200:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
    8208:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl3addx r5, r6, r7 }
    8210:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shrs r5, r6, r7 }
    8218:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    8220:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mnz r15, r16, r17 ; st r25, r26 }
    8228:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; st r25, r26 }
    8230:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nor r5, r6, r7 ; st1 r25, r26 }
    8238:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmplts r5, r6, r7 ; st2 r25, r26 }
    8240:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shru r5, r6, r7 ; st2 r25, r26 }
    8248:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mnz r15, r16, r17 ; st4 r25, r26 }
    8250:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    8258:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
    8260:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    8268:	[0-9a-f]* 	{ mnz r15, r16, r17 ; v1cmpltui r5, r6, 5 }
    8270:	[0-9a-f]* 	{ mnz r15, r16, r17 ; v2cmples r5, r6, r7 }
    8278:	[0-9a-f]* 	{ mnz r15, r16, r17 ; v4packsc r5, r6, r7 }
    8280:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
    8288:	[0-9a-f]* 	{ mnz r5, r6, r7 ; addx r15, r16, r17 ; st r25, r26 }
    8290:	[0-9a-f]* 	{ mnz r5, r6, r7 ; and r15, r16, r17 ; st r25, r26 }
    8298:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    82a0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmples r15, r16, r17 ; st2 r25, r26 }
    82a8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmplts r15, r16, r17 }
    82b0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    82b8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ld2u r25, r26 }
    82c0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; ld4s r25, r26 }
    82c8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    82d0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
    82d8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; nop ; ld r25, r26 }
    82e0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
    82e8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpleu r15, r16, r17 ; ld1u r25, r26 }
    82f0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
    82f8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    8300:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; ld2u r25, r26 }
    8308:	[0-9a-f]* 	{ mnz r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    8310:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    8318:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
    8320:	[0-9a-f]* 	{ mnz r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    8328:	[0-9a-f]* 	{ mnz r5, r6, r7 ; movei r15, 5 ; prefetch_l1_fault r25 }
    8330:	[0-9a-f]* 	{ mnz r5, r6, r7 ; nop ; prefetch_l1_fault r25 }
    8338:	[0-9a-f]* 	{ mnz r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    8340:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    8348:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; prefetch r25 }
    8350:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    8358:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
    8360:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    8368:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    8370:	[0-9a-f]* 	{ mnz r5, r6, r7 ; movei r15, 5 ; prefetch_l3 r25 }
    8378:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    8380:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    8388:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    8390:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    8398:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    83a0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    83a8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
    83b0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shru r15, r16, r17 ; st4 r25, r26 }
    83b8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; st r25, r26 }
    83c0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
    83c8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; st2 r15, r16 }
    83d0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
    83d8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rotli r15, r16, 5 ; st4 r25, r26 }
    83e0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    83e8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; v1maxu r15, r16, r17 }
    83f0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; v2shrs r15, r16, r17 }
    83f8:	[0-9a-f]* 	{ move r15, r16 ; add r5, r6, r7 ; ld1u r25, r26 }
    8400:	[0-9a-f]* 	{ move r15, r16 ; addx r5, r6, r7 ; ld2s r25, r26 }
    8408:	[0-9a-f]* 	{ move r15, r16 ; and r5, r6, r7 ; ld2s r25, r26 }
    8410:	[0-9a-f]* 	{ clz r5, r6 ; move r15, r16 ; ld1u r25, r26 }
    8418:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; move r15, r16 ; ld2u r25, r26 }
    8420:	[0-9a-f]* 	{ move r15, r16 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
    8428:	[0-9a-f]* 	{ move r15, r16 ; cmpleu r5, r6, r7 ; prefetch r25 }
    8430:	[0-9a-f]* 	{ move r15, r16 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    8438:	[0-9a-f]* 	{ move r15, r16 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    8440:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; move r15, r16 }
    8448:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; move r15, r16 ; prefetch_l1_fault r25 }
    8450:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; move r15, r16 ; ld r25, r26 }
    8458:	[0-9a-f]* 	{ move r15, r16 ; shl3add r5, r6, r7 ; ld r25, r26 }
    8460:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; move r15, r16 ; ld1s r25, r26 }
    8468:	[0-9a-f]* 	{ move r15, r16 ; addx r5, r6, r7 ; ld1u r25, r26 }
    8470:	[0-9a-f]* 	{ move r15, r16 ; rotli r5, r6, 5 ; ld1u r25, r26 }
    8478:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; move r15, r16 ; ld2s r25, r26 }
    8480:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; ld2s r25, r26 }
    8488:	[0-9a-f]* 	{ move r15, r16 ; nor r5, r6, r7 ; ld2u r25, r26 }
    8490:	[0-9a-f]* 	{ move r15, r16 ; cmplts r5, r6, r7 ; ld4s r25, r26 }
    8498:	[0-9a-f]* 	{ move r15, r16 ; shru r5, r6, r7 ; ld4s r25, r26 }
    84a0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    84a8:	[0-9a-f]* 	{ move r15, r16 ; mnz r5, r6, r7 ; ld4u r25, r26 }
    84b0:	[0-9a-f]* 	{ move r15, r16 ; movei r5, 5 ; prefetch r25 }
    84b8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    84c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    84c8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; move r15, r16 ; ld2u r25, r26 }
    84d0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; move r15, r16 ; ld2s r25, r26 }
    84d8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    84e0:	[0-9a-f]* 	{ move r15, r16 ; nop ; prefetch r25 }
    84e8:	[0-9a-f]* 	{ move r15, r16 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    84f0:	[0-9a-f]* 	{ move r15, r16 ; cmpeqi r5, r6, 5 ; prefetch r25 }
    84f8:	[0-9a-f]* 	{ move r15, r16 ; shli r5, r6, 5 ; prefetch r25 }
    8500:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    8508:	[0-9a-f]* 	{ move r15, r16 ; and r5, r6, r7 ; prefetch_l1_fault r25 }
    8510:	[0-9a-f]* 	{ move r15, r16 ; shl1add r5, r6, r7 ; prefetch_l1_fault r25 }
    8518:	[0-9a-f]* 	{ move r15, r16 ; mnz r5, r6, r7 ; prefetch_l2 r25 }
    8520:	[0-9a-f]* 	{ move r15, r16 ; xor r5, r6, r7 ; prefetch_l2 r25 }
    8528:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; prefetch_l2_fault r25 }
    8530:	[0-9a-f]* 	{ move r15, r16 ; cmpltu r5, r6, r7 ; prefetch_l3 r25 }
    8538:	[0-9a-f]* 	{ move r15, r16 ; sub r5, r6, r7 ; prefetch_l3 r25 }
    8540:	[0-9a-f]* 	{ mulax r5, r6, r7 ; move r15, r16 ; prefetch_l3_fault r25 }
    8548:	[0-9a-f]* 	{ revbits r5, r6 ; move r15, r16 ; prefetch_l1_fault r25 }
    8550:	[0-9a-f]* 	{ move r15, r16 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
    8558:	[0-9a-f]* 	{ move r15, r16 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
    8560:	[0-9a-f]* 	{ move r15, r16 ; shl1addx r5, r6, r7 ; st r25, r26 }
    8568:	[0-9a-f]* 	{ move r15, r16 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
    8570:	[0-9a-f]* 	{ move r15, r16 ; shl3addx r5, r6, r7 }
    8578:	[0-9a-f]* 	{ move r15, r16 ; shrs r5, r6, r7 }
    8580:	[0-9a-f]* 	{ move r15, r16 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    8588:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; move r15, r16 ; st r25, r26 }
    8590:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; st r25, r26 }
    8598:	[0-9a-f]* 	{ move r15, r16 ; nor r5, r6, r7 ; st1 r25, r26 }
    85a0:	[0-9a-f]* 	{ move r15, r16 ; cmplts r5, r6, r7 ; st2 r25, r26 }
    85a8:	[0-9a-f]* 	{ move r15, r16 ; shru r5, r6, r7 ; st2 r25, r26 }
    85b0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    85b8:	[0-9a-f]* 	{ move r15, r16 ; sub r5, r6, r7 ; prefetch r25 }
    85c0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; prefetch r25 }
    85c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; prefetch_l2 r25 }
    85d0:	[0-9a-f]* 	{ move r15, r16 ; v1cmpltui r5, r6, 5 }
    85d8:	[0-9a-f]* 	{ move r15, r16 ; v2cmples r5, r6, r7 }
    85e0:	[0-9a-f]* 	{ move r15, r16 ; v4packsc r5, r6, r7 }
    85e8:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
    85f0:	[0-9a-f]* 	{ move r5, r6 ; addx r15, r16, r17 ; st r25, r26 }
    85f8:	[0-9a-f]* 	{ move r5, r6 ; and r15, r16, r17 ; st r25, r26 }
    8600:	[0-9a-f]* 	{ move r5, r6 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    8608:	[0-9a-f]* 	{ move r5, r6 ; cmples r15, r16, r17 ; st2 r25, r26 }
    8610:	[0-9a-f]* 	{ move r5, r6 ; cmplts r15, r16, r17 }
    8618:	[0-9a-f]* 	{ move r5, r6 ; cmpne r15, r16, r17 ; ld r25, r26 }
    8620:	[0-9a-f]* 	{ move r5, r6 ; ld2u r25, r26 }
    8628:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; ld4s r25, r26 }
    8630:	[0-9a-f]* 	{ move r5, r6 ; jalrp r15 ; ld2u r25, r26 }
    8638:	[0-9a-f]* 	{ move r5, r6 ; jrp r15 ; ld4u r25, r26 }
    8640:	[0-9a-f]* 	{ move r5, r6 ; nop ; ld r25, r26 }
    8648:	[0-9a-f]* 	{ move r5, r6 ; jalrp r15 ; ld1s r25, r26 }
    8650:	[0-9a-f]* 	{ move r5, r6 ; cmpleu r15, r16, r17 ; ld1u r25, r26 }
    8658:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; ld2s r25, r26 }
    8660:	[0-9a-f]* 	{ move r5, r6 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    8668:	[0-9a-f]* 	{ move r5, r6 ; shl r15, r16, r17 ; ld2u r25, r26 }
    8670:	[0-9a-f]* 	{ move r5, r6 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    8678:	[0-9a-f]* 	{ move r5, r6 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    8680:	[0-9a-f]* 	{ move r5, r6 ; ldnt1s_add r15, r16, 5 }
    8688:	[0-9a-f]* 	{ move r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
    8690:	[0-9a-f]* 	{ move r5, r6 ; movei r15, 5 ; prefetch_l1_fault r25 }
    8698:	[0-9a-f]* 	{ move r5, r6 ; nop ; prefetch_l1_fault r25 }
    86a0:	[0-9a-f]* 	{ move r5, r6 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    86a8:	[0-9a-f]* 	{ move r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
    86b0:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; prefetch r25 }
    86b8:	[0-9a-f]* 	{ move r5, r6 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    86c0:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; prefetch_l2 r25 }
    86c8:	[0-9a-f]* 	{ move r5, r6 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    86d0:	[0-9a-f]* 	{ move r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    86d8:	[0-9a-f]* 	{ move r5, r6 ; movei r15, 5 ; prefetch_l3 r25 }
    86e0:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; prefetch_l3_fault r25 }
    86e8:	[0-9a-f]* 	{ move r5, r6 ; rotl r15, r16, r17 ; prefetch r25 }
    86f0:	[0-9a-f]* 	{ move r5, r6 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    86f8:	[0-9a-f]* 	{ move r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    8700:	[0-9a-f]* 	{ move r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    8708:	[0-9a-f]* 	{ move r5, r6 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    8710:	[0-9a-f]* 	{ move r5, r6 ; shrs r15, r16, r17 ; st1 r25, r26 }
    8718:	[0-9a-f]* 	{ move r5, r6 ; shru r15, r16, r17 ; st4 r25, r26 }
    8720:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; st r25, r26 }
    8728:	[0-9a-f]* 	{ move r5, r6 ; cmples r15, r16, r17 ; st1 r25, r26 }
    8730:	[0-9a-f]* 	{ move r5, r6 ; st2 r15, r16 }
    8738:	[0-9a-f]* 	{ move r5, r6 ; shrs r15, r16, r17 ; st2 r25, r26 }
    8740:	[0-9a-f]* 	{ move r5, r6 ; rotli r15, r16, 5 ; st4 r25, r26 }
    8748:	[0-9a-f]* 	{ move r5, r6 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    8750:	[0-9a-f]* 	{ move r5, r6 ; v1maxu r15, r16, r17 }
    8758:	[0-9a-f]* 	{ move r5, r6 ; v2shrs r15, r16, r17 }
    8760:	[0-9a-f]* 	{ movei r15, 5 ; add r5, r6, r7 ; ld1u r25, r26 }
    8768:	[0-9a-f]* 	{ movei r15, 5 ; addx r5, r6, r7 ; ld2s r25, r26 }
    8770:	[0-9a-f]* 	{ movei r15, 5 ; and r5, r6, r7 ; ld2s r25, r26 }
    8778:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 ; ld1u r25, r26 }
    8780:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; movei r15, 5 ; ld2u r25, r26 }
    8788:	[0-9a-f]* 	{ movei r15, 5 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
    8790:	[0-9a-f]* 	{ movei r15, 5 ; cmpleu r5, r6, r7 ; prefetch r25 }
    8798:	[0-9a-f]* 	{ movei r15, 5 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    87a0:	[0-9a-f]* 	{ movei r15, 5 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    87a8:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; movei r15, 5 }
    87b0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; prefetch_l1_fault r25 }
    87b8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
    87c0:	[0-9a-f]* 	{ movei r15, 5 ; shl3add r5, r6, r7 ; ld r25, r26 }
    87c8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    87d0:	[0-9a-f]* 	{ movei r15, 5 ; addx r5, r6, r7 ; ld1u r25, r26 }
    87d8:	[0-9a-f]* 	{ movei r15, 5 ; rotli r5, r6, 5 ; ld1u r25, r26 }
    87e0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; ld2s r25, r26 }
    87e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; ld2s r25, r26 }
    87f0:	[0-9a-f]* 	{ movei r15, 5 ; nor r5, r6, r7 ; ld2u r25, r26 }
    87f8:	[0-9a-f]* 	{ movei r15, 5 ; cmplts r5, r6, r7 ; ld4s r25, r26 }
    8800:	[0-9a-f]* 	{ movei r15, 5 ; shru r5, r6, r7 ; ld4s r25, r26 }
    8808:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
    8810:	[0-9a-f]* 	{ movei r15, 5 ; mnz r5, r6, r7 ; ld4u r25, r26 }
    8818:	[0-9a-f]* 	{ movei r15, 5 ; movei r5, 5 ; prefetch r25 }
    8820:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
    8828:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; movei r15, 5 ; ld4s r25, r26 }
    8830:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; movei r15, 5 ; ld2u r25, r26 }
    8838:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; movei r15, 5 ; ld2s r25, r26 }
    8840:	[0-9a-f]* 	{ mulx r5, r6, r7 ; movei r15, 5 ; ld4s r25, r26 }
    8848:	[0-9a-f]* 	{ movei r15, 5 ; nop ; prefetch r25 }
    8850:	[0-9a-f]* 	{ movei r15, 5 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    8858:	[0-9a-f]* 	{ movei r15, 5 ; cmpeqi r5, r6, 5 ; prefetch r25 }
    8860:	[0-9a-f]* 	{ movei r15, 5 ; shli r5, r6, 5 ; prefetch r25 }
    8868:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    8870:	[0-9a-f]* 	{ movei r15, 5 ; and r5, r6, r7 ; prefetch_l1_fault r25 }
    8878:	[0-9a-f]* 	{ movei r15, 5 ; shl1add r5, r6, r7 ; prefetch_l1_fault r25 }
    8880:	[0-9a-f]* 	{ movei r15, 5 ; mnz r5, r6, r7 ; prefetch_l2 r25 }
    8888:	[0-9a-f]* 	{ movei r15, 5 ; xor r5, r6, r7 ; prefetch_l2 r25 }
    8890:	[0-9a-f]* 	{ pcnt r5, r6 ; movei r15, 5 ; prefetch_l2_fault r25 }
    8898:	[0-9a-f]* 	{ movei r15, 5 ; cmpltu r5, r6, r7 ; prefetch_l3 r25 }
    88a0:	[0-9a-f]* 	{ movei r15, 5 ; sub r5, r6, r7 ; prefetch_l3 r25 }
    88a8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; movei r15, 5 ; prefetch_l3_fault r25 }
    88b0:	[0-9a-f]* 	{ revbits r5, r6 ; movei r15, 5 ; prefetch_l1_fault r25 }
    88b8:	[0-9a-f]* 	{ movei r15, 5 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
    88c0:	[0-9a-f]* 	{ movei r15, 5 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
    88c8:	[0-9a-f]* 	{ movei r15, 5 ; shl1addx r5, r6, r7 ; st r25, r26 }
    88d0:	[0-9a-f]* 	{ movei r15, 5 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
    88d8:	[0-9a-f]* 	{ movei r15, 5 ; shl3addx r5, r6, r7 }
    88e0:	[0-9a-f]* 	{ movei r15, 5 ; shrs r5, r6, r7 }
    88e8:	[0-9a-f]* 	{ movei r15, 5 ; shrui r5, r6, 5 ; ld1s r25, r26 }
    88f0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; st r25, r26 }
    88f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; st r25, r26 }
    8900:	[0-9a-f]* 	{ movei r15, 5 ; nor r5, r6, r7 ; st1 r25, r26 }
    8908:	[0-9a-f]* 	{ movei r15, 5 ; cmplts r5, r6, r7 ; st2 r25, r26 }
    8910:	[0-9a-f]* 	{ movei r15, 5 ; shru r5, r6, r7 ; st2 r25, r26 }
    8918:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; movei r15, 5 ; st4 r25, r26 }
    8920:	[0-9a-f]* 	{ movei r15, 5 ; sub r5, r6, r7 ; prefetch r25 }
    8928:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; movei r15, 5 ; prefetch r25 }
    8930:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; prefetch_l2 r25 }
    8938:	[0-9a-f]* 	{ movei r15, 5 ; v1cmpltui r5, r6, 5 }
    8940:	[0-9a-f]* 	{ movei r15, 5 ; v2cmples r5, r6, r7 }
    8948:	[0-9a-f]* 	{ movei r15, 5 ; v4packsc r5, r6, r7 }
    8950:	[0-9a-f]* 	{ movei r5, 5 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
    8958:	[0-9a-f]* 	{ movei r5, 5 ; addx r15, r16, r17 ; st r25, r26 }
    8960:	[0-9a-f]* 	{ movei r5, 5 ; and r15, r16, r17 ; st r25, r26 }
    8968:	[0-9a-f]* 	{ movei r5, 5 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
    8970:	[0-9a-f]* 	{ movei r5, 5 ; cmples r15, r16, r17 ; st2 r25, r26 }
    8978:	[0-9a-f]* 	{ movei r5, 5 ; cmplts r15, r16, r17 }
    8980:	[0-9a-f]* 	{ movei r5, 5 ; cmpne r15, r16, r17 ; ld r25, r26 }
    8988:	[0-9a-f]* 	{ movei r5, 5 ; ld2u r25, r26 }
    8990:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; ld4s r25, r26 }
    8998:	[0-9a-f]* 	{ movei r5, 5 ; jalrp r15 ; ld2u r25, r26 }
    89a0:	[0-9a-f]* 	{ movei r5, 5 ; jrp r15 ; ld4u r25, r26 }
    89a8:	[0-9a-f]* 	{ movei r5, 5 ; nop ; ld r25, r26 }
    89b0:	[0-9a-f]* 	{ movei r5, 5 ; jalrp r15 ; ld1s r25, r26 }
    89b8:	[0-9a-f]* 	{ movei r5, 5 ; cmpleu r15, r16, r17 ; ld1u r25, r26 }
    89c0:	[0-9a-f]* 	{ movei r5, 5 ; add r15, r16, r17 ; ld2s r25, r26 }
    89c8:	[0-9a-f]* 	{ movei r5, 5 ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    89d0:	[0-9a-f]* 	{ movei r5, 5 ; shl r15, r16, r17 ; ld2u r25, r26 }
    89d8:	[0-9a-f]* 	{ movei r5, 5 ; mnz r15, r16, r17 ; ld4s r25, r26 }
    89e0:	[0-9a-f]* 	{ movei r5, 5 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    89e8:	[0-9a-f]* 	{ movei r5, 5 ; ldnt1s_add r15, r16, 5 }
    89f0:	[0-9a-f]* 	{ movei r5, 5 ; mnz r15, r16, r17 ; prefetch r25 }
    89f8:	[0-9a-f]* 	{ movei r5, 5 ; movei r15, 5 ; prefetch_l1_fault r25 }
    8a00:	[0-9a-f]* 	{ movei r5, 5 ; nop ; prefetch_l1_fault r25 }
    8a08:	[0-9a-f]* 	{ movei r5, 5 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    8a10:	[0-9a-f]* 	{ movei r5, 5 ; rotli r15, r16, 5 ; prefetch r25 }
    8a18:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; prefetch r25 }
    8a20:	[0-9a-f]* 	{ movei r5, 5 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    8a28:	[0-9a-f]* 	{ movei r5, 5 ; add r15, r16, r17 ; prefetch_l2 r25 }
    8a30:	[0-9a-f]* 	{ movei r5, 5 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    8a38:	[0-9a-f]* 	{ movei r5, 5 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    8a40:	[0-9a-f]* 	{ movei r5, 5 ; movei r15, 5 ; prefetch_l3 r25 }
    8a48:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; prefetch_l3_fault r25 }
    8a50:	[0-9a-f]* 	{ movei r5, 5 ; rotl r15, r16, r17 ; prefetch r25 }
    8a58:	[0-9a-f]* 	{ movei r5, 5 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    8a60:	[0-9a-f]* 	{ movei r5, 5 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    8a68:	[0-9a-f]* 	{ movei r5, 5 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    8a70:	[0-9a-f]* 	{ movei r5, 5 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    8a78:	[0-9a-f]* 	{ movei r5, 5 ; shrs r15, r16, r17 ; st1 r25, r26 }
    8a80:	[0-9a-f]* 	{ movei r5, 5 ; shru r15, r16, r17 ; st4 r25, r26 }
    8a88:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; st r25, r26 }
    8a90:	[0-9a-f]* 	{ movei r5, 5 ; cmples r15, r16, r17 ; st1 r25, r26 }
    8a98:	[0-9a-f]* 	{ movei r5, 5 ; st2 r15, r16 }
    8aa0:	[0-9a-f]* 	{ movei r5, 5 ; shrs r15, r16, r17 ; st2 r25, r26 }
    8aa8:	[0-9a-f]* 	{ movei r5, 5 ; rotli r15, r16, 5 ; st4 r25, r26 }
    8ab0:	[0-9a-f]* 	{ movei r5, 5 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    8ab8:	[0-9a-f]* 	{ movei r5, 5 ; v1maxu r15, r16, r17 }
    8ac0:	[0-9a-f]* 	{ movei r5, 5 ; v2shrs r15, r16, r17 }
    8ac8:	[0-9a-f]* 	{ moveli r15, 4660 ; addli r5, r6, 4660 }
    8ad0:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; moveli r15, 4660 }
    8ad8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; moveli r15, 4660 }
    8ae0:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; moveli r15, 4660 }
    8ae8:	[0-9a-f]* 	{ moveli r15, 4660 ; v1subuc r5, r6, r7 }
    8af0:	[0-9a-f]* 	{ moveli r15, 4660 ; v2shru r5, r6, r7 }
    8af8:	[0-9a-f]* 	{ moveli r5, 4660 ; dtlbpr r15 }
    8b00:	[0-9a-f]* 	{ moveli r5, 4660 ; ldna_add r15, r16, 5 }
    8b08:	[0-9a-f]* 	{ moveli r5, 4660 ; prefetch_l3_fault r15 }
    8b10:	[0-9a-f]* 	{ moveli r5, 4660 ; v1add r15, r16, r17 }
    8b18:	[0-9a-f]* 	{ moveli r5, 4660 ; v2int_h r15, r16, r17 }
    8b20:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b28:	[0-9a-f]* 	{ mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b30:	[0-9a-f]* 	{ or r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b38:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b40:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b48:	[0-9a-f]* 	{ v4add r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    8b50:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    8b58:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    8b60:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1_fault r25 }
    8b68:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
    8b70:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2_fault r25 }
    8b78:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 }
    8b80:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
    8b88:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 }
    8b90:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; infol 4660 }
    8b98:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jalrp r15 }
    8ba0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
    8ba8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; ld r25, r26 }
    8bb0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; ld1s r25, r26 }
    8bb8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; move r15, r16 ; ld1u r25, r26 }
    8bc0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; ld2s r25, r26 }
    8bc8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; ld2u r25, r26 }
    8bd0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    8bd8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; ld4s r25, r26 }
    8be0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; nor r15, r16, r17 ; ld4u r25, r26 }
    8be8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; lnk r15 ; ld1u r25, r26 }
    8bf0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; move r15, r16 ; ld1u r25, r26 }
    8bf8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; ld1u r25, r26 }
    8c00:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; nor r15, r16, r17 ; ld2u r25, r26 }
    8c08:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    8c10:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    8c18:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    8c20:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    8c28:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
    8c30:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    8c38:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 }
    8c40:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3 r25 }
    8c48:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
    8c50:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; ld2s r25, r26 }
    8c58:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; ld2u r25, r26 }
    8c60:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    8c68:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    8c70:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2 r25 }
    8c78:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    8c80:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    8c88:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; st r25, r26 }
    8c90:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mnz r15, r16, r17 ; st1 r25, r26 }
    8c98:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpne r15, r16, r17 ; st2 r25, r26 }
    8ca0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
    8ca8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
    8cb0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    8cb8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; v2add r15, r16, r17 }
    8cc0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; v4shru r15, r16, r17 }
    8cc8:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; cmpltsi r15, r16, 5 }
    8cd0:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; ld2u_add r15, r16, 5 }
    8cd8:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    8ce0:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; stnt2_add r15, r16, 5 }
    8ce8:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; v2cmples r15, r16, r17 }
    8cf0:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; xori r15, r16, 5 }
    8cf8:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; ill }
    8d00:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; mf }
    8d08:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; shrsi r15, r16, 5 }
    8d10:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v1minu r15, r16, r17 }
    8d18:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v2shru r15, r16, r17 }
    8d20:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; dblalign6 r15, r16, r17 }
    8d28:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; ldna r15, r16 }
    8d30:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; prefetch_l3 r15 }
    8d38:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; subxsc r15, r16, r17 }
    8d40:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; v2cmpne r15, r16, r17 }
    8d48:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
    8d50:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
    8d58:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    8d60:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    8d68:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    8d70:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    8d78:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    8d80:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; fetchor4 r15, r16, r17 }
    8d88:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ill ; st2 r25, r26 }
    8d90:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
    8d98:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jr r15 ; st4 r25, r26 }
    8da0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalrp r15 ; ld r25, r26 }
    8da8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; ld1s r25, r26 }
    8db0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; addi r15, r16, 5 ; ld1u r25, r26 }
    8db8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shru r15, r16, r17 ; ld1u r25, r26 }
    8dc0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    8dc8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; move r15, r16 ; ld2u r25, r26 }
    8dd0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; ld4s r25, r26 }
    8dd8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; ld4u r25, r26 }
    8de0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
    8de8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
    8df0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    8df8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; nop ; ld1s r25, r26 }
    8e00:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    8e08:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    8e10:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    8e18:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    8e20:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    8e28:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    8e30:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; movei r15, 5 ; prefetch_l2_fault r25 }
    8e38:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    8e40:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    8e48:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; rotl r15, r16, r17 ; ld r25, r26 }
    8e50:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    8e58:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    8e60:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    8e68:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    8e70:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    8e78:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    8e80:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
    8e88:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
    8e90:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    8e98:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
    8ea0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; st4 r25, r26 }
    8ea8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
    8eb0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
    8eb8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; v2mnz r15, r16, r17 }
    8ec0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
    8ec8:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; finv r15 }
    8ed0:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
    8ed8:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; shl3addx r15, r16, r17 }
    8ee0:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; v1cmpne r15, r16, r17 }
    8ee8:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; v2shl r15, r16, r17 }
    8ef0:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; cmpltu r15, r16, r17 }
    8ef8:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; ld4s r15, r16 }
    8f00:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    8f08:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; stnt4 r15, r16 }
    8f10:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; v2cmpleu r15, r16, r17 }
    8f18:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
    8f20:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
    8f28:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
    8f30:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    8f38:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
    8f40:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    8f48:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    8f50:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; fetchaddgez r15, r16, r17 }
    8f58:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    8f60:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
    8f68:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    8f70:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
    8f78:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
    8f80:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; ld1s r25, r26 }
    8f88:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    8f90:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
    8f98:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
    8fa0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; ld4s r25, r26 }
    8fa8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    8fb0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    8fb8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    8fc0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
    8fc8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
    8fd0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
    8fd8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalr r15 ; prefetch r25 }
    8fe0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    8fe8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    8ff0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    8ff8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    9000:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jr r15 ; prefetch_l2_fault r25 }
    9008:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    9010:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    9018:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    9020:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    9028:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
    9030:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    9038:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    9040:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    9048:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    9050:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; st r25, r26 }
    9058:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; st r25, r26 }
    9060:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    9068:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nop ; st2 r25, r26 }
    9070:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalr r15 ; st4 r25, r26 }
    9078:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
    9080:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; v1addi r15, r16, 5 }
    9088:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; v2int_l r15, r16, r17 }
    9090:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
    9098:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    90a0:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; ldnt2s r15, r16 }
    90a8:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; shl1add r15, r16, r17 }
    90b0:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
    90b8:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; v2mnz r15, r16, r17 }
    90c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; add r15, r16, r17 ; prefetch_l3 r25 }
    90c8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
    90d0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3_fault r25 }
    90d8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpeq r15, r16, r17 ; st1 r25, r26 }
    90e0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
    90e8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmplts r15, r16, r17 ; st4 r25, r26 }
    90f0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpltui r15, r16, 5 }
    90f8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ld2s r25, r26 }
    9100:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; info 19 ; ld2u r25, r26 }
    9108:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    9110:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
    9118:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; ld r25, r26 }
    9120:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    9128:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; ld1u r25, r26 }
    9130:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ld2s r15, r16 }
    9138:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    9140:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    9148:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; lnk r15 ; ld4s r25, r26 }
    9150:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
    9158:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ldnt1s r15, r16 }
    9160:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld4u r25, r26 }
    9168:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    9170:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; nop ; prefetch r25 }
    9178:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2 r25 }
    9180:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    9188:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ill ; prefetch r25 }
    9190:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l1_fault r25 }
    9198:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; prefetch_l2 r15 }
    91a0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l2 r25 }
    91a8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    91b0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; move r15, r16 ; prefetch_l3 r25 }
    91b8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
    91c0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    91c8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l1_fault r25 }
    91d0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    91d8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    91e0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 ; st r25, r26 }
    91e8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
    91f0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shru r15, r16, r17 ; st2 r25, r26 }
    91f8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; ill ; st r25, r26 }
    9200:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    9208:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; st1_add r15, r16, 5 }
    9210:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shli r15, r16, 5 ; st2 r25, r26 }
    9218:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; st4 r25, r26 }
    9220:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3 r25 }
    9228:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; v1int_l r15, r16, r17 }
    9230:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; v2shlsc r15, r16, r17 }
    9238:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    9240:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
    9248:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; ld1u r25, r26 }
    9250:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2u r25, r26 }
    9258:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmples r15, r16, r17 ; ld2u r25, r26 }
    9260:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
    9268:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    9270:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
    9278:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ill ; prefetch_l3 r25 }
    9280:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
    9288:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; prefetch_l3_fault r25 }
    9290:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ld r25, r26 }
    9298:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; ld1s r25, r26 }
    92a0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ld1s r25, r26 }
    92a8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    92b0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
    92b8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; ld2u r25, r26 }
    92c0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
    92c8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; ld4u r25, r26 }
    92d0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
    92d8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    92e0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
    92e8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; st2 r25, r26 }
    92f0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; nor r15, r16, r17 }
    92f8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    9300:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    9308:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    9310:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l1_fault r25 }
    9318:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2 r25 }
    9320:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jrp r15 ; prefetch_l2_fault r25 }
    9328:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    9330:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3_fault r25 }
    9338:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3_fault r25 }
    9340:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; st4 r25, r26 }
    9348:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 }
    9350:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1s r25, r26 }
    9358:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
    9360:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrs r15, r16, r17 ; ld2s r25, r26 }
    9368:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
    9370:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; st r25, r26 }
    9378:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; st r25, r26 }
    9380:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; st1 r25, r26 }
    9388:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; nor r15, r16, r17 ; st2 r25, r26 }
    9390:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
    9398:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
    93a0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; v1adduc r15, r16, r17 }
    93a8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; v2maxs r15, r16, r17 }
    93b0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2 r25 }
    93b8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; fetchand r15, r16, r17 }
    93c0:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
    93c8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; shl1addx r15, r16, r17 }
    93d0:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; v1cmplts r15, r16, r17 }
    93d8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; v2mz r15, r16, r17 }
    93e0:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; cmples r15, r16, r17 }
    93e8:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; ld2s r15, r16 }
    93f0:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    93f8:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; stnt1 r15, r16 }
    9400:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; v2addsc r15, r16, r17 }
    9408:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; v4subsc r15, r16, r17 }
    9410:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; flushwb }
    9418:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
    9420:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; shlx r15, r16, r17 }
    9428:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; v1int_l r15, r16, r17 }
    9430:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; v2shlsc r15, r16, r17 }
    9438:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; addi r15, r16, 5 ; ld r25, r26 }
    9440:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
    9448:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
    9450:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
    9458:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
    9460:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    9468:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
    9470:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; prefetch_l2 r25 }
    9478:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; info 19 ; prefetch_l2_fault r25 }
    9480:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    9488:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jrp r15 ; prefetch_l3 r25 }
    9490:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    9498:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
    94a0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ill ; ld1u r25, r26 }
    94a8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
    94b0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ld2s r25, r26 }
    94b8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    94c0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; ld4s r25, r26 }
    94c8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jr r15 ; ld4u r25, r26 }
    94d0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
    94d8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l3_fault r25 }
    94e0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; movei r15, 5 ; st1 r25, r26 }
    94e8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nop ; st1 r25, r26 }
    94f0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; st4 r25, r26 }
    94f8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    9500:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    9508:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; prefetch_l1_fault r25 }
    9510:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l2 r25 }
    9518:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; prefetch_l2 r25 }
    9520:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
    9528:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3 r25 }
    9530:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l3_fault r25 }
    9538:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
    9540:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
    9548:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl1addx r15, r16, r17 ; st4 r25, r26 }
    9550:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; ld r25, r26 }
    9558:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    9560:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    9568:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shrui r15, r16, 5 ; ld2u r25, r26 }
    9570:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; st r25, r26 }
    9578:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; st1 r25, r26 }
    9580:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; st2 r25, r26 }
    9588:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; xor r15, r16, r17 ; st2 r25, r26 }
    9590:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; st4 r25, r26 }
    9598:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
    95a0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; v1shl r15, r16, r17 }
    95a8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; v4add r15, r16, r17 }
    95b0:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; andi r15, r16, 5 }
    95b8:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; ld r15, r16 }
    95c0:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; nor r15, r16, r17 }
    95c8:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; st2_add r15, r16, 5 }
    95d0:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; v1shrui r15, r16, 5 }
    95d8:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; v4shl r15, r16, r17 }
    95e0:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; fetchand4 r15, r16, r17 }
    95e8:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; ldnt2u r15, r16 }
    95f0:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; shl2add r15, r16, r17 }
    95f8:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
    9600:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; v2packh r15, r16, r17 }
    9608:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
    9610:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; st1 r25, r26 }
    9618:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
    9620:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
    9628:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
    9630:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld r25, r26 }
    9638:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
    9640:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
    9648:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; info 19 ; ld4u r25, r26 }
    9650:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalrp r15 ; ld4s r25, r26 }
    9658:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jrp r15 ; prefetch r25 }
    9660:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; ld r25, r26 }
    9668:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jr r15 ; ld1s r25, r26 }
    9670:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
    9678:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; ld2s r25, r26 }
    9680:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; ld2s r25, r26 }
    9688:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; ld2u r25, r26 }
    9690:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; move r15, r16 ; ld4s r25, r26 }
    9698:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ld4u r25, r26 }
    96a0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; ldnt1u r15, r16 }
    96a8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    96b0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; movei r15, 5 ; prefetch_l2 r25 }
    96b8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nop ; prefetch_l2 r25 }
    96c0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3 r25 }
    96c8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    96d0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalr r15 ; prefetch r25 }
    96d8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l1_fault r25 }
    96e0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l2 r25 }
    96e8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
    96f0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    96f8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    9700:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    9708:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l1_fault r25 }
    9710:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    9718:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3 r25 }
    9720:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; st r25, r26 }
    9728:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; st2 r25, r26 }
    9730:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
    9738:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 }
    9740:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalr r15 ; st r25, r26 }
    9748:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    9750:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; st2 r25, r26 }
    9758:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; st2 r25, r26 }
    9760:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; st4 r25, r26 }
    9768:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; st r25, r26 }
    9770:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; v1maxui r15, r16, 5 }
    9778:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; v2shrsi r15, r16, 5 }
    9780:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; addx r15, r16, r17 }
    9788:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; iret }
    9790:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; movei r15, 5 }
    9798:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; shruxi r15, r16, 5 }
    97a0:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v1shl r15, r16, r17 }
    97a8:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v4add r15, r16, r17 }
    97b0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    97b8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    97c0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    97c8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    97d0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
    97d8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    97e0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
    97e8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; st4 r25, r26 }
    97f0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; info 19 }
    97f8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
    9800:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; ld r15, r16 }
    9808:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; ld r25, r26 }
    9810:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    9818:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld1u r25, r26 }
    9820:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; ld2s r25, r26 }
    9828:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
    9830:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; ld2u r25, r26 }
    9838:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    9840:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; nop ; ld4u r25, r26 }
    9848:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; ld1s r25, r26 }
    9850:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; move r15, r16 ; ld1s r25, r26 }
    9858:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; ld1s r25, r26 }
    9860:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
    9868:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    9870:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    9878:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    9880:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    9888:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    9890:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l2_fault r25 }
    9898:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    98a0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    98a8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l3_fault r25 }
    98b0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; ld1u r25, r26 }
    98b8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
    98c0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl2add r15, r16, r17 ; ld4s r25, r26 }
    98c8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    98d0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
    98d8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    98e0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2_fault r25 }
    98e8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
    98f0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
    98f8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
    9900:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    9908:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; st4 r25, r26 }
    9910:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    9918:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; v1subuc r15, r16, r17 }
    9920:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; v4shrs r15, r16, r17 }
    9928:	[0-9a-f]* 	{ mulax r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
    9930:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
    9938:	[0-9a-f]* 	{ mulax r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
    9940:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpeq r15, r16, r17 }
    9948:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmples r15, r16, r17 }
    9950:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
    9958:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpne r15, r16, r17 ; ld1u r25, r26 }
    9960:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ld4u r25, r26 }
    9968:	[0-9a-f]* 	{ mulax r5, r6, r7 ; info 19 ; prefetch r25 }
    9970:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
    9978:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jrp r15 ; prefetch r25 }
    9980:	[0-9a-f]* 	{ mulax r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
    9988:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jrp r15 ; ld1s r25, r26 }
    9990:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1u r25, r26 }
    9998:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
    99a0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; ld2s r25, r26 }
    99a8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    99b0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; movei r15, 5 ; ld4s r25, r26 }
    99b8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ill ; ld4u r25, r26 }
    99c0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
    99c8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    99d0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; movei r15, 5 ; prefetch_l2_fault r25 }
    99d8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nop ; prefetch_l2_fault r25 }
    99e0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3_fault r25 }
    99e8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    99f0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    99f8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
    9a00:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2 r25 }
    9a08:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    9a10:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    9a18:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nop ; prefetch_l3 r25 }
    9a20:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    9a28:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    9a30:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l3 r25 }
    9a38:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
    9a40:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    9a48:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3addx r15, r16, r17 ; st4 r25, r26 }
    9a50:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrs r15, r16, r17 ; st4 r25, r26 }
    9a58:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; ld r25, r26 }
    9a60:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; st r25, r26 }
    9a68:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
    9a70:	[0-9a-f]* 	{ mulax r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    9a78:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shru r15, r16, r17 ; st2 r25, r26 }
    9a80:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
    9a88:	[0-9a-f]* 	{ mulax r5, r6, r7 ; sub r15, r16, r17 ; st1 r25, r26 }
    9a90:	[0-9a-f]* 	{ mulax r5, r6, r7 ; v1minu r15, r16, r17 }
    9a98:	[0-9a-f]* 	{ mulax r5, r6, r7 ; v2shru r15, r16, r17 }
    9aa0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    9aa8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
    9ab0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    9ab8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    9ac0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    9ac8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
    9ad0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
    9ad8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; fetchor r15, r16, r17 }
    9ae0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; ill ; st1 r25, r26 }
    9ae8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jalr r15 ; st r25, r26 }
    9af0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jr r15 ; st2 r25, r26 }
    9af8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jalr r15 ; ld r25, r26 }
    9b00:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
    9b08:	[0-9a-f]* 	{ mulx r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    9b10:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    9b18:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    9b20:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    9b28:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    9b30:	[0-9a-f]* 	{ mulx r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    9b38:	[0-9a-f]* 	{ mulx r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
    9b40:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mf }
    9b48:	[0-9a-f]* 	{ mulx r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
    9b50:	[0-9a-f]* 	{ mulx r5, r6, r7 ; nop ; ld r25, r26 }
    9b58:	[0-9a-f]* 	{ mulx r5, r6, r7 ; or r15, r16, r17 ; ld1u r25, r26 }
    9b60:	[0-9a-f]* 	{ mulx r5, r6, r7 ; lnk r15 ; prefetch r25 }
    9b68:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch r25 }
    9b70:	[0-9a-f]* 	{ mulx r5, r6, r7 ; prefetch_l1_fault r15 }
    9b78:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l1_fault r25 }
    9b80:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    9b88:	[0-9a-f]* 	{ mulx r5, r6, r7 ; move r15, r16 ; prefetch_l2_fault r25 }
    9b90:	[0-9a-f]* 	{ mulx r5, r6, r7 ; ill ; prefetch_l3 r25 }
    9b98:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    9ba0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; raise }
    9ba8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    9bb0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    9bb8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
    9bc0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    9bc8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    9bd0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    9bd8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeqi r15, r16, 5 ; st r25, r26 }
    9be0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; st1 r15, r16 }
    9be8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
    9bf0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    9bf8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    9c00:	[0-9a-f]* 	{ mulx r5, r6, r7 ; sub r15, r16, r17 ; ld2u r25, r26 }
    9c08:	[0-9a-f]* 	{ mulx r5, r6, r7 ; v1cmples r15, r16, r17 }
    9c10:	[0-9a-f]* 	{ mulx r5, r6, r7 ; v2minsi r15, r16, 5 }
    9c18:	[0-9a-f]* 	{ mulx r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    9c20:	[0-9a-f]* 	{ mz r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    9c28:	[0-9a-f]* 	{ mz r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
    9c30:	[0-9a-f]* 	{ mz r15, r16, r17 ; andi r5, r6, 5 ; st1 r25, r26 }
    9c38:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; mz r15, r16, r17 ; st r25, r26 }
    9c40:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpeq r5, r6, r7 ; st2 r25, r26 }
    9c48:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmples r5, r6, r7 }
    9c50:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
    9c58:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpne r5, r6, r7 ; ld1u r25, r26 }
    9c60:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
    9c68:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mz r15, r16, r17 ; ld r25, r26 }
    9c70:	[0-9a-f]* 	{ mz r15, r16, r17 ; infol 4660 }
    9c78:	[0-9a-f]* 	{ revbits r5, r6 ; mz r15, r16, r17 ; ld r25, r26 }
    9c80:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    9c88:	[0-9a-f]* 	{ mz r15, r16, r17 ; subx r5, r6, r7 ; ld1s r25, r26 }
    9c90:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mz r15, r16, r17 ; ld1u r25, r26 }
    9c98:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
    9ca0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shli r5, r6, 5 ; ld2s r25, r26 }
    9ca8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; ld2u r25, r26 }
    9cb0:	[0-9a-f]* 	{ mz r15, r16, r17 ; and r5, r6, r7 ; ld4s r25, r26 }
    9cb8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl1add r5, r6, r7 ; ld4s r25, r26 }
    9cc0:	[0-9a-f]* 	{ mz r15, r16, r17 ; mnz r5, r6, r7 ; ld4u r25, r26 }
    9cc8:	[0-9a-f]* 	{ mz r15, r16, r17 ; xor r5, r6, r7 ; ld4u r25, r26 }
    9cd0:	[0-9a-f]* 	{ mz r15, r16, r17 ; move r5, r6 }
    9cd8:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; mz r15, r16, r17 }
    9ce0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; mz r15, r16, r17 ; st2 r25, r26 }
    9ce8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
    9cf0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; mz r15, r16, r17 ; st r25, r26 }
    9cf8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
    9d00:	[0-9a-f]* 	{ mz r15, r16, r17 ; mz r5, r6, r7 ; st4 r25, r26 }
    9d08:	[0-9a-f]* 	{ mz r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
    9d10:	[0-9a-f]* 	{ mz r15, r16, r17 ; addi r5, r6, 5 ; prefetch r25 }
    9d18:	[0-9a-f]* 	{ mz r15, r16, r17 ; rotl r5, r6, r7 ; prefetch r25 }
    9d20:	[0-9a-f]* 	{ mz r15, r16, r17 ; prefetch r25 }
    9d28:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    9d30:	[0-9a-f]* 	{ mz r15, r16, r17 ; nop ; prefetch_l1_fault r25 }
    9d38:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l2 r25 }
    9d40:	[0-9a-f]* 	{ mz r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l2 r25 }
    9d48:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    9d50:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    9d58:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l3 r25 }
    9d60:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; prefetch_l3_fault r25 }
    9d68:	[0-9a-f]* 	{ revbits r5, r6 ; mz r15, r16, r17 ; ld r25, r26 }
    9d70:	[0-9a-f]* 	{ mz r15, r16, r17 ; rotl r5, r6, r7 ; ld1u r25, r26 }
    9d78:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl r5, r6, r7 ; ld2u r25, r26 }
    9d80:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
    9d88:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    9d90:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l1_fault r25 }
    9d98:	[0-9a-f]* 	{ mz r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l1_fault r25 }
    9da0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2_fault r25 }
    9da8:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpeqi r5, r6, 5 ; st r25, r26 }
    9db0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shli r5, r6, 5 ; st r25, r26 }
    9db8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
    9dc0:	[0-9a-f]* 	{ mz r15, r16, r17 ; and r5, r6, r7 ; st2 r25, r26 }
    9dc8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl1add r5, r6, r7 ; st2 r25, r26 }
    9dd0:	[0-9a-f]* 	{ mz r15, r16, r17 ; mnz r5, r6, r7 ; st4 r25, r26 }
    9dd8:	[0-9a-f]* 	{ mz r15, r16, r17 ; xor r5, r6, r7 ; st4 r25, r26 }
    9de0:	[0-9a-f]* 	{ mz r15, r16, r17 ; subxsc r5, r6, r7 }
    9de8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; ld1s r25, r26 }
    9df0:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; mz r15, r16, r17 }
    9df8:	[0-9a-f]* 	{ mz r15, r16, r17 ; v1sub r5, r6, r7 }
    9e00:	[0-9a-f]* 	{ mz r15, r16, r17 ; v2shrsi r5, r6, 5 }
    9e08:	[0-9a-f]* 	{ mz r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
    9e10:	[0-9a-f]* 	{ mz r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
    9e18:	[0-9a-f]* 	{ mz r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    9e20:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    9e28:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    9e30:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
    9e38:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
    9e40:	[0-9a-f]* 	{ mz r5, r6, r7 ; fetchor r15, r16, r17 }
    9e48:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; st1 r25, r26 }
    9e50:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalr r15 ; st r25, r26 }
    9e58:	[0-9a-f]* 	{ mz r5, r6, r7 ; jr r15 ; st2 r25, r26 }
    9e60:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalr r15 ; ld r25, r26 }
    9e68:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
    9e70:	[0-9a-f]* 	{ mz r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
    9e78:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    9e80:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    9e88:	[0-9a-f]* 	{ mz r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
    9e90:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    9e98:	[0-9a-f]* 	{ mz r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
    9ea0:	[0-9a-f]* 	{ mz r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
    9ea8:	[0-9a-f]* 	{ mz r5, r6, r7 ; mf }
    9eb0:	[0-9a-f]* 	{ mz r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
    9eb8:	[0-9a-f]* 	{ mz r5, r6, r7 ; nop ; ld r25, r26 }
    9ec0:	[0-9a-f]* 	{ mz r5, r6, r7 ; or r15, r16, r17 ; ld1u r25, r26 }
    9ec8:	[0-9a-f]* 	{ mz r5, r6, r7 ; lnk r15 ; prefetch r25 }
    9ed0:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch r25 }
    9ed8:	[0-9a-f]* 	{ mz r5, r6, r7 ; prefetch_l1_fault r15 }
    9ee0:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l1_fault r25 }
    9ee8:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    9ef0:	[0-9a-f]* 	{ mz r5, r6, r7 ; move r15, r16 ; prefetch_l2_fault r25 }
    9ef8:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; prefetch_l3 r25 }
    9f00:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    9f08:	[0-9a-f]* 	{ mz r5, r6, r7 ; raise }
    9f10:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    9f18:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    9f20:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
    9f28:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    9f30:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    9f38:	[0-9a-f]* 	{ mz r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    9f40:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpeqi r15, r16, 5 ; st r25, r26 }
    9f48:	[0-9a-f]* 	{ mz r5, r6, r7 ; st1 r15, r16 }
    9f50:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
    9f58:	[0-9a-f]* 	{ mz r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    9f60:	[0-9a-f]* 	{ mz r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
    9f68:	[0-9a-f]* 	{ mz r5, r6, r7 ; sub r15, r16, r17 ; ld2u r25, r26 }
    9f70:	[0-9a-f]* 	{ mz r5, r6, r7 ; v1cmples r15, r16, r17 }
    9f78:	[0-9a-f]* 	{ mz r5, r6, r7 ; v2minsi r15, r16, 5 }
    9f80:	[0-9a-f]* 	{ mz r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    9f88:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; prefetch_l3_fault r25 }
    9f90:	[0-9a-f]* 	{ nop ; addi r5, r6, 5 ; st1 r25, r26 }
    9f98:	[0-9a-f]* 	{ nop ; addx r5, r6, r7 ; st1 r25, r26 }
    9fa0:	[0-9a-f]* 	{ nop ; addxi r5, r6, 5 ; st4 r25, r26 }
    9fa8:	[0-9a-f]* 	{ nop ; and r5, r6, r7 ; st1 r25, r26 }
    9fb0:	[0-9a-f]* 	{ nop ; andi r5, r6, 5 ; st4 r25, r26 }
    9fb8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; nop ; st1 r25, r26 }
    9fc0:	[0-9a-f]* 	{ nop ; cmpeq r15, r16, r17 ; st4 r25, r26 }
    9fc8:	[0-9a-f]* 	{ nop ; cmpeqi r5, r6, 5 ; ld r25, r26 }
    9fd0:	[0-9a-f]* 	{ nop ; cmples r5, r6, r7 ; ld r25, r26 }
    9fd8:	[0-9a-f]* 	{ nop ; cmpleu r5, r6, r7 ; ld1u r25, r26 }
    9fe0:	[0-9a-f]* 	{ nop ; cmplts r5, r6, r7 ; ld2u r25, r26 }
    9fe8:	[0-9a-f]* 	{ nop ; cmpltsi r5, r6, 5 ; ld4u r25, r26 }
    9ff0:	[0-9a-f]* 	{ nop ; cmpltu r5, r6, r7 ; prefetch r25 }
    9ff8:	[0-9a-f]* 	{ nop ; cmpne r5, r6, r7 ; prefetch r25 }
    a000:	[0-9a-f]* 	{ nop ; dblalign2 r15, r16, r17 }
    a008:	[0-9a-f]* 	{ nop ; prefetch_l2_fault r25 }
    a010:	[0-9a-f]* 	{ nop ; ill ; ld4u r25, r26 }
    a018:	[0-9a-f]* 	{ nop ; jalr r15 ; ld4s r25, r26 }
    a020:	[0-9a-f]* 	{ nop ; jr r15 ; prefetch r25 }
    a028:	[0-9a-f]* 	{ nop ; and r15, r16, r17 ; ld r25, r26 }
    a030:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; nop ; ld r25, r26 }
    a038:	[0-9a-f]* 	{ nop ; shrs r5, r6, r7 ; ld r25, r26 }
    a040:	[0-9a-f]* 	{ nop ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
    a048:	[0-9a-f]* 	{ nop ; nor r5, r6, r7 ; ld1s r25, r26 }
    a050:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; ld1s r25, r26 }
    a058:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nop ; ld1u r25, r26 }
    a060:	[0-9a-f]* 	{ nop ; shl1add r15, r16, r17 ; ld1u r25, r26 }
    a068:	[0-9a-f]* 	{ nop ; addx r5, r6, r7 ; ld2s r25, r26 }
    a070:	[0-9a-f]* 	{ nop ; movei r15, 5 ; ld2s r25, r26 }
    a078:	[0-9a-f]* 	{ nop ; shli r15, r16, 5 ; ld2s r25, r26 }
    a080:	[0-9a-f]* 	{ nop ; cmpeqi r15, r16, 5 ; ld2u r25, r26 }
    a088:	[0-9a-f]* 	{ nop ; mz r15, r16, r17 ; ld2u r25, r26 }
    a090:	[0-9a-f]* 	{ nop ; subx r15, r16, r17 ; ld2u r25, r26 }
    a098:	[0-9a-f]* 	{ nop ; cmpne r15, r16, r17 ; ld4s r25, r26 }
    a0a0:	[0-9a-f]* 	{ nop ; rotli r15, r16, 5 ; ld4s r25, r26 }
    a0a8:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; ld4u r25, r26 }
    a0b0:	[0-9a-f]* 	{ nop ; mnz r15, r16, r17 ; ld4u r25, r26 }
    a0b8:	[0-9a-f]* 	{ nop ; shl3add r15, r16, r17 ; ld4u r25, r26 }
    a0c0:	[0-9a-f]* 	{ nop ; ldnt4u r15, r16 }
    a0c8:	[0-9a-f]* 	{ nop ; mnz r15, r16, r17 ; st1 r25, r26 }
    a0d0:	[0-9a-f]* 	{ nop ; move r15, r16 ; st4 r25, r26 }
    a0d8:	[0-9a-f]* 	{ nop ; movei r5, 5 ; ld r25, r26 }
    a0e0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; nop }
    a0e8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nop ; st1 r25, r26 }
    a0f0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; nop ; st2 r25, r26 }
    a0f8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nop ; prefetch_l3_fault r25 }
    a100:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nop ; st r25, r26 }
    a108:	[0-9a-f]* 	{ nop ; mz r15, r16, r17 ; st2 r25, r26 }
    a110:	[0-9a-f]* 	{ nop ; nop ; st4 r25, r26 }
    a118:	[0-9a-f]* 	{ nop ; or r15, r16, r17 ; ld r25, r26 }
    a120:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; ld r25, r26 }
    a128:	[0-9a-f]* 	{ nop ; cmples r5, r6, r7 ; prefetch r25 }
    a130:	[0-9a-f]* 	{ nop ; nor r15, r16, r17 ; prefetch r25 }
    a138:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nop ; prefetch r25 }
    a140:	[0-9a-f]* 	{ nop ; cmpltu r15, r16, r17 ; prefetch r25 }
    a148:	[0-9a-f]* 	{ nop ; rotl r15, r16, r17 ; prefetch r25 }
    a150:	[0-9a-f]* 	{ nop ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    a158:	[0-9a-f]* 	{ nop ; lnk r15 ; prefetch_l1_fault r25 }
    a160:	[0-9a-f]* 	{ nop ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    a168:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; nop ; prefetch_l2 r25 }
    a170:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nop ; prefetch_l2 r25 }
    a178:	[0-9a-f]* 	{ nop ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    a180:	[0-9a-f]* 	{ nop ; cmpltsi r5, r6, 5 ; prefetch_l2_fault r25 }
    a188:	[0-9a-f]* 	{ revbytes r5, r6 ; nop ; prefetch_l2_fault r25 }
    a190:	[0-9a-f]* 	{ nop ; prefetch_l3 r15 }
    a198:	[0-9a-f]* 	{ nop ; jrp r15 ; prefetch_l3 r25 }
    a1a0:	[0-9a-f]* 	{ nop ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    a1a8:	[0-9a-f]* 	{ clz r5, r6 ; nop ; prefetch_l3_fault r25 }
    a1b0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nop ; prefetch_l3_fault r25 }
    a1b8:	[0-9a-f]* 	{ nop ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
    a1c0:	[0-9a-f]* 	{ revbytes r5, r6 ; nop ; ld4u r25, r26 }
    a1c8:	[0-9a-f]* 	{ nop ; rotl r5, r6, r7 ; prefetch r25 }
    a1d0:	[0-9a-f]* 	{ nop ; rotli r5, r6, 5 ; prefetch_l2 r25 }
    a1d8:	[0-9a-f]* 	{ nop ; shl r5, r6, r7 ; prefetch_l3 r25 }
    a1e0:	[0-9a-f]* 	{ nop ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
    a1e8:	[0-9a-f]* 	{ nop ; shl1addx r5, r6, r7 ; st r25, r26 }
    a1f0:	[0-9a-f]* 	{ nop ; shl2add r5, r6, r7 ; st2 r25, r26 }
    a1f8:	[0-9a-f]* 	{ nop ; shl2addx r5, r6, r7 }
    a200:	[0-9a-f]* 	{ nop ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
    a208:	[0-9a-f]* 	{ nop ; shli r15, r16, 5 ; ld2s r25, r26 }
    a210:	[0-9a-f]* 	{ nop ; shrs r15, r16, r17 ; ld1s r25, r26 }
    a218:	[0-9a-f]* 	{ nop ; shrsi r15, r16, 5 ; ld2s r25, r26 }
    a220:	[0-9a-f]* 	{ nop ; shru r15, r16, r17 ; ld4s r25, r26 }
    a228:	[0-9a-f]* 	{ nop ; shrui r15, r16, 5 ; prefetch r25 }
    a230:	[0-9a-f]* 	{ nop ; addi r5, r6, 5 ; st r25, r26 }
    a238:	[0-9a-f]* 	{ nop ; move r15, r16 ; st r25, r26 }
    a240:	[0-9a-f]* 	{ nop ; shl3addx r15, r16, r17 ; st r25, r26 }
    a248:	[0-9a-f]* 	{ nop ; cmpeq r5, r6, r7 ; st1 r25, r26 }
    a250:	[0-9a-f]* 	{ mulx r5, r6, r7 ; nop ; st1 r25, r26 }
    a258:	[0-9a-f]* 	{ nop ; sub r5, r6, r7 ; st1 r25, r26 }
    a260:	[0-9a-f]* 	{ nop ; cmpltu r5, r6, r7 ; st2 r25, r26 }
    a268:	[0-9a-f]* 	{ nop ; rotl r5, r6, r7 ; st2 r25, r26 }
    a270:	[0-9a-f]* 	{ nop ; add r15, r16, r17 ; st4 r25, r26 }
    a278:	[0-9a-f]* 	{ nop ; lnk r15 ; st4 r25, r26 }
    a280:	[0-9a-f]* 	{ nop ; shl2addx r5, r6, r7 ; st4 r25, r26 }
    a288:	[0-9a-f]* 	{ nop ; sub r15, r16, r17 ; ld2u r25, r26 }
    a290:	[0-9a-f]* 	{ nop ; subx r15, r16, r17 ; ld4u r25, r26 }
    a298:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nop ; ld1u r25, r26 }
    a2a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; ld2u r25, r26 }
    a2a8:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; nop }
    a2b0:	[0-9a-f]* 	{ nop ; v1minui r15, r16, 5 }
    a2b8:	[0-9a-f]* 	{ nop ; v2cmples r5, r6, r7 }
    a2c0:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; nop }
    a2c8:	[0-9a-f]* 	{ nop ; v4sub r15, r16, r17 }
    a2d0:	[0-9a-f]* 	{ nop ; xor r5, r6, r7 ; st2 r25, r26 }
    a2d8:	[0-9a-f]* 	{ nor r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
    a2e0:	[0-9a-f]* 	{ nor r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
    a2e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
    a2f0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; nor r15, r16, r17 ; st2 r25, r26 }
    a2f8:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpeq r5, r6, r7 }
    a300:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
    a308:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
    a310:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
    a318:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
    a320:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nor r15, r16, r17 ; ld1u r25, r26 }
    a328:	[0-9a-f]* 	{ nor r15, r16, r17 ; addi r5, r6, 5 ; ld r25, r26 }
    a330:	[0-9a-f]* 	{ nor r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    a338:	[0-9a-f]* 	{ nor r15, r16, r17 ; ld1s r25, r26 }
    a340:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; ld1s r25, r26 }
    a348:	[0-9a-f]* 	{ nor r15, r16, r17 ; nop ; ld1u r25, r26 }
    a350:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    a358:	[0-9a-f]* 	{ nor r15, r16, r17 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
    a360:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nor r15, r16, r17 ; ld2u r25, r26 }
    a368:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; ld4s r25, r26 }
    a370:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl2add r5, r6, r7 ; ld4s r25, r26 }
    a378:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; ld4u r25, r26 }
    a380:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; nor r15, r16, r17 }
    a388:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
    a390:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; nor r15, r16, r17 }
    a398:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nor r15, r16, r17 }
    a3a0:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; nor r15, r16, r17 }
    a3a8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; st2 r25, r26 }
    a3b0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
    a3b8:	[0-9a-f]* 	{ nor r15, r16, r17 ; nop ; ld r25, r26 }
    a3c0:	[0-9a-f]* 	{ nor r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
    a3c8:	[0-9a-f]* 	{ nor r15, r16, r17 ; addxi r5, r6, 5 ; prefetch r25 }
    a3d0:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
    a3d8:	[0-9a-f]* 	{ nor r15, r16, r17 ; info 19 ; prefetch r25 }
    a3e0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    a3e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    a3f0:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    a3f8:	[0-9a-f]* 	{ nor r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l2 r25 }
    a400:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2_fault r25 }
    a408:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    a410:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    a418:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    a420:	[0-9a-f]* 	{ revbits r5, r6 ; nor r15, r16, r17 ; ld1u r25, r26 }
    a428:	[0-9a-f]* 	{ nor r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
    a430:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
    a438:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
    a440:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    a448:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    a450:	[0-9a-f]* 	{ nor r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
    a458:	[0-9a-f]* 	{ nor r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
    a460:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
    a468:	[0-9a-f]* 	{ nor r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
    a470:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nor r15, r16, r17 ; st1 r25, r26 }
    a478:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
    a480:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
    a488:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; st4 r25, r26 }
    a490:	[0-9a-f]* 	{ nor r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    a498:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; ld1s r25, r26 }
    a4a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; ld2s r25, r26 }
    a4a8:	[0-9a-f]* 	{ nor r15, r16, r17 ; v1cmpeq r5, r6, r7 }
    a4b0:	[0-9a-f]* 	{ nor r15, r16, r17 ; v2add r5, r6, r7 }
    a4b8:	[0-9a-f]* 	{ nor r15, r16, r17 ; v2shrui r5, r6, 5 }
    a4c0:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    a4c8:	[0-9a-f]* 	{ nor r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    a4d0:	[0-9a-f]* 	{ nor r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    a4d8:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    a4e0:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    a4e8:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    a4f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
    a4f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; finv r15 }
    a500:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; st4 r25, r26 }
    a508:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    a510:	[0-9a-f]* 	{ nor r5, r6, r7 ; jr r15 }
    a518:	[0-9a-f]* 	{ nor r5, r6, r7 ; jr r15 ; ld r25, r26 }
    a520:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
    a528:	[0-9a-f]* 	{ nor r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
    a530:	[0-9a-f]* 	{ nor r5, r6, r7 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    a538:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    a540:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; ld2u r25, r26 }
    a548:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; ld4s r25, r26 }
    a550:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    a558:	[0-9a-f]* 	{ nor r5, r6, r7 ; ld4u r25, r26 }
    a560:	[0-9a-f]* 	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    a568:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
    a570:	[0-9a-f]* 	{ nor r5, r6, r7 ; nop ; ld1u r25, r26 }
    a578:	[0-9a-f]* 	{ nor r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
    a580:	[0-9a-f]* 	{ nor r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    a588:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch r25 }
    a590:	[0-9a-f]* 	{ nor r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
    a598:	[0-9a-f]* 	{ nor r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    a5a0:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    a5a8:	[0-9a-f]* 	{ nor r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    a5b0:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    a5b8:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
    a5c0:	[0-9a-f]* 	{ nor r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    a5c8:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    a5d0:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    a5d8:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    a5e0:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    a5e8:	[0-9a-f]* 	{ nor r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    a5f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
    a5f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
    a600:	[0-9a-f]* 	{ nor r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
    a608:	[0-9a-f]* 	{ nor r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    a610:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1add r15, r16, r17 ; st2 r25, r26 }
    a618:	[0-9a-f]* 	{ nor r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    a620:	[0-9a-f]* 	{ nor r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    a628:	[0-9a-f]* 	{ nor r5, r6, r7 ; v1cmplts r15, r16, r17 }
    a630:	[0-9a-f]* 	{ nor r5, r6, r7 ; v2mz r15, r16, r17 }
    a638:	[0-9a-f]* 	{ nor r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
    a640:	[0-9a-f]* 	{ or r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
    a648:	[0-9a-f]* 	{ or r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
    a650:	[0-9a-f]* 	{ or r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
    a658:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    a660:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpeq r5, r6, r7 }
    a668:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
    a670:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
    a678:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
    a680:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; st2 r25, r26 }
    a688:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; or r15, r16, r17 ; ld1u r25, r26 }
    a690:	[0-9a-f]* 	{ or r15, r16, r17 ; addi r5, r6, 5 ; ld r25, r26 }
    a698:	[0-9a-f]* 	{ or r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
    a6a0:	[0-9a-f]* 	{ or r15, r16, r17 ; ld1s r25, r26 }
    a6a8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; ld1s r25, r26 }
    a6b0:	[0-9a-f]* 	{ or r15, r16, r17 ; nop ; ld1u r25, r26 }
    a6b8:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
    a6c0:	[0-9a-f]* 	{ or r15, r16, r17 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
    a6c8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
    a6d0:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; ld4s r25, r26 }
    a6d8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl2add r5, r6, r7 ; ld4s r25, r26 }
    a6e0:	[0-9a-f]* 	{ or r15, r16, r17 ; movei r5, 5 ; ld4u r25, r26 }
    a6e8:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; or r15, r16, r17 }
    a6f0:	[0-9a-f]* 	{ or r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
    a6f8:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; or r15, r16, r17 }
    a700:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; or r15, r16, r17 }
    a708:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; or r15, r16, r17 }
    a710:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; or r15, r16, r17 ; st2 r25, r26 }
    a718:	[0-9a-f]* 	{ mulax r5, r6, r7 ; or r15, r16, r17 ; st4 r25, r26 }
    a720:	[0-9a-f]* 	{ or r15, r16, r17 ; nop ; ld r25, r26 }
    a728:	[0-9a-f]* 	{ or r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
    a730:	[0-9a-f]* 	{ or r15, r16, r17 ; addxi r5, r6, 5 ; prefetch r25 }
    a738:	[0-9a-f]* 	{ or r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
    a740:	[0-9a-f]* 	{ or r15, r16, r17 ; info 19 ; prefetch r25 }
    a748:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; or r15, r16, r17 ; prefetch r25 }
    a750:	[0-9a-f]* 	{ or r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    a758:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
    a760:	[0-9a-f]* 	{ or r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l2 r25 }
    a768:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    a770:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3 r25 }
    a778:	[0-9a-f]* 	{ or r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    a780:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3_fault r25 }
    a788:	[0-9a-f]* 	{ revbits r5, r6 ; or r15, r16, r17 ; ld1u r25, r26 }
    a790:	[0-9a-f]* 	{ or r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
    a798:	[0-9a-f]* 	{ or r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
    a7a0:	[0-9a-f]* 	{ or r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
    a7a8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    a7b0:	[0-9a-f]* 	{ or r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    a7b8:	[0-9a-f]* 	{ or r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
    a7c0:	[0-9a-f]* 	{ or r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
    a7c8:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
    a7d0:	[0-9a-f]* 	{ or r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
    a7d8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; st1 r25, r26 }
    a7e0:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; st2 r25, r26 }
    a7e8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
    a7f0:	[0-9a-f]* 	{ or r15, r16, r17 ; movei r5, 5 ; st4 r25, r26 }
    a7f8:	[0-9a-f]* 	{ or r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
    a800:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; or r15, r16, r17 ; ld1s r25, r26 }
    a808:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; ld2s r25, r26 }
    a810:	[0-9a-f]* 	{ or r15, r16, r17 ; v1cmpeq r5, r6, r7 }
    a818:	[0-9a-f]* 	{ or r15, r16, r17 ; v2add r5, r6, r7 }
    a820:	[0-9a-f]* 	{ or r15, r16, r17 ; v2shrui r5, r6, 5 }
    a828:	[0-9a-f]* 	{ or r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
    a830:	[0-9a-f]* 	{ or r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    a838:	[0-9a-f]* 	{ or r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    a840:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
    a848:	[0-9a-f]* 	{ or r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
    a850:	[0-9a-f]* 	{ or r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    a858:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
    a860:	[0-9a-f]* 	{ or r5, r6, r7 ; finv r15 }
    a868:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; st4 r25, r26 }
    a870:	[0-9a-f]* 	{ or r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    a878:	[0-9a-f]* 	{ or r5, r6, r7 ; jr r15 }
    a880:	[0-9a-f]* 	{ or r5, r6, r7 ; jr r15 ; ld r25, r26 }
    a888:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
    a890:	[0-9a-f]* 	{ or r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
    a898:	[0-9a-f]* 	{ or r5, r6, r7 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    a8a0:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
    a8a8:	[0-9a-f]* 	{ or r5, r6, r7 ; movei r15, 5 ; ld2u r25, r26 }
    a8b0:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; ld4s r25, r26 }
    a8b8:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    a8c0:	[0-9a-f]* 	{ or r5, r6, r7 ; ld4u r25, r26 }
    a8c8:	[0-9a-f]* 	{ or r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
    a8d0:	[0-9a-f]* 	{ or r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
    a8d8:	[0-9a-f]* 	{ or r5, r6, r7 ; nop ; ld1u r25, r26 }
    a8e0:	[0-9a-f]* 	{ or r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
    a8e8:	[0-9a-f]* 	{ or r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    a8f0:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch r25 }
    a8f8:	[0-9a-f]* 	{ or r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
    a900:	[0-9a-f]* 	{ or r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
    a908:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    a910:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    a918:	[0-9a-f]* 	{ or r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    a920:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
    a928:	[0-9a-f]* 	{ or r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    a930:	[0-9a-f]* 	{ or r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    a938:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    a940:	[0-9a-f]* 	{ or r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    a948:	[0-9a-f]* 	{ or r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    a950:	[0-9a-f]* 	{ or r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    a958:	[0-9a-f]* 	{ or r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
    a960:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
    a968:	[0-9a-f]* 	{ or r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
    a970:	[0-9a-f]* 	{ or r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    a978:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1add r15, r16, r17 ; st2 r25, r26 }
    a980:	[0-9a-f]* 	{ or r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
    a988:	[0-9a-f]* 	{ or r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    a990:	[0-9a-f]* 	{ or r5, r6, r7 ; v1cmplts r15, r16, r17 }
    a998:	[0-9a-f]* 	{ or r5, r6, r7 ; v2mz r15, r16, r17 }
    a9a0:	[0-9a-f]* 	{ or r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
    a9a8:	[0-9a-f]* 	{ ori r15, r16, 5 ; dblalign2 r5, r6, r7 }
    a9b0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ori r15, r16, 5 }
    a9b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ori r15, r16, 5 }
    a9c0:	[0-9a-f]* 	{ ori r15, r16, 5 ; v1shl r5, r6, r7 }
    a9c8:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; ori r15, r16, 5 }
    a9d0:	[0-9a-f]* 	{ ori r5, r6, 5 ; cmpltsi r15, r16, 5 }
    a9d8:	[0-9a-f]* 	{ ori r5, r6, 5 ; ld2u_add r15, r16, 5 }
    a9e0:	[0-9a-f]* 	{ ori r5, r6, 5 ; prefetch_add_l3 r15, 5 }
    a9e8:	[0-9a-f]* 	{ ori r5, r6, 5 ; stnt2_add r15, r16, 5 }
    a9f0:	[0-9a-f]* 	{ ori r5, r6, 5 ; v2cmples r15, r16, r17 }
    a9f8:	[0-9a-f]* 	{ ori r5, r6, 5 ; xori r15, r16, 5 }
    aa00:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
    aa08:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
    aa10:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
    aa18:	[0-9a-f]* 	{ pcnt r5, r6 ; cmples r15, r16, r17 ; ld1u r25, r26 }
    aa20:	[0-9a-f]* 	{ pcnt r5, r6 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
    aa28:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
    aa30:	[0-9a-f]* 	{ pcnt r5, r6 ; fetchadd4 r15, r16, r17 }
    aa38:	[0-9a-f]* 	{ pcnt r5, r6 ; ill ; prefetch_l2 r25 }
    aa40:	[0-9a-f]* 	{ pcnt r5, r6 ; jalr r15 ; prefetch_l1_fault r25 }
    aa48:	[0-9a-f]* 	{ pcnt r5, r6 ; jr r15 ; prefetch_l2_fault r25 }
    aa50:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltu r15, r16, r17 ; ld r25, r26 }
    aa58:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
    aa60:	[0-9a-f]* 	{ pcnt r5, r6 ; subx r15, r16, r17 ; ld1s r25, r26 }
    aa68:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    aa70:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; ld2s r25, r26 }
    aa78:	[0-9a-f]* 	{ pcnt r5, r6 ; jalr r15 ; ld2u r25, r26 }
    aa80:	[0-9a-f]* 	{ pcnt r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    aa88:	[0-9a-f]* 	{ pcnt r5, r6 ; ld4u r15, r16 }
    aa90:	[0-9a-f]* 	{ pcnt r5, r6 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    aa98:	[0-9a-f]* 	{ pcnt r5, r6 ; lnk r15 ; st r25, r26 }
    aaa0:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; st r25, r26 }
    aaa8:	[0-9a-f]* 	{ pcnt r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
    aab0:	[0-9a-f]* 	{ pcnt r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
    aab8:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; prefetch r25 }
    aac0:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
    aac8:	[0-9a-f]* 	{ pcnt r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    aad0:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2add r15, r16, r17 ; prefetch_l1_fault r25 }
    aad8:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; prefetch_l2 r25 }
    aae0:	[0-9a-f]* 	{ pcnt r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    aae8:	[0-9a-f]* 	{ pcnt r5, r6 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
    aaf0:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
    aaf8:	[0-9a-f]* 	{ pcnt r5, r6 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    ab00:	[0-9a-f]* 	{ pcnt r5, r6 ; rotli r15, r16, 5 ; st1 r25, r26 }
    ab08:	[0-9a-f]* 	{ pcnt r5, r6 ; shl1add r15, r16, r17 ; st2 r25, r26 }
    ab10:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2add r15, r16, r17 }
    ab18:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
    ab20:	[0-9a-f]* 	{ pcnt r5, r6 ; shrs r15, r16, r17 ; ld1s r25, r26 }
    ab28:	[0-9a-f]* 	{ pcnt r5, r6 ; shru r15, r16, r17 ; ld2s r25, r26 }
    ab30:	[0-9a-f]* 	{ pcnt r5, r6 ; addx r15, r16, r17 ; st r25, r26 }
    ab38:	[0-9a-f]* 	{ pcnt r5, r6 ; shrui r15, r16, 5 ; st r25, r26 }
    ab40:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2add r15, r16, r17 ; st1 r25, r26 }
    ab48:	[0-9a-f]* 	{ pcnt r5, r6 ; mz r15, r16, r17 ; st2 r25, r26 }
    ab50:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; st4 r25, r26 }
    ab58:	[0-9a-f]* 	{ pcnt r5, r6 ; stnt_add r15, r16, 5 }
    ab60:	[0-9a-f]* 	{ pcnt r5, r6 ; v1add r15, r16, r17 }
    ab68:	[0-9a-f]* 	{ pcnt r5, r6 ; v2int_h r15, r16, r17 }
    ab70:	[0-9a-f]* 	{ pcnt r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    ab78:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; prefetch r15 }
    ab80:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; prefetch r15 }
    ab88:	[0-9a-f]* 	{ shrux r5, r6, r7 ; prefetch r15 }
    ab90:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; prefetch r15 }
    ab98:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; prefetch r15 }
    aba0:	[0-9a-f]* 	{ add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    aba8:	[0-9a-f]* 	{ add r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    abb0:	[0-9a-f]* 	{ add r5, r6, r7 ; nop ; prefetch r25 }
    abb8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    abc0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    abc8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch r25 }
    abd0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    abd8:	[0-9a-f]* 	{ addx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    abe0:	[0-9a-f]* 	{ addx r5, r6, r7 ; prefetch r25 }
    abe8:	[0-9a-f]* 	{ revbits r5, r6 ; addxi r15, r16, 5 ; prefetch r25 }
    abf0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; info 19 ; prefetch r25 }
    abf8:	[0-9a-f]* 	{ and r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    ac00:	[0-9a-f]* 	{ and r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    ac08:	[0-9a-f]* 	{ and r5, r6, r7 ; nop ; prefetch r25 }
    ac10:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    ac18:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    ac20:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch r25 }
    ac28:	[0-9a-f]* 	{ clz r5, r6 ; rotl r15, r16, r17 ; prefetch r25 }
    ac30:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    ac38:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; ill ; prefetch r25 }
    ac40:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    ac48:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch r25 }
    ac50:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    ac58:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; prefetch r25 }
    ac60:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch r25 }
    ac68:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 ; prefetch r25 }
    ac70:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    ac78:	[0-9a-f]* 	{ cmples r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    ac80:	[0-9a-f]* 	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    ac88:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpleu r15, r16, r17 ; prefetch r25 }
    ac90:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; ill ; prefetch r25 }
    ac98:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    aca0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch r25 }
    aca8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    acb0:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; prefetch r25 }
    acb8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    acc0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; shl2addx r15, r16, r17 ; prefetch r25 }
    acc8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    acd0:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    acd8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    ace0:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpne r15, r16, r17 ; prefetch r25 }
    ace8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ill ; prefetch r25 }
    acf0:	[0-9a-f]* 	{ ctz r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    acf8:	[0-9a-f]* 	{ add r5, r6, r7 ; prefetch r25 }
    ad00:	[0-9a-f]* 	{ mnz r15, r16, r17 ; prefetch r25 }
    ad08:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; prefetch r25 }
    ad10:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; ill ; prefetch r25 }
    ad18:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; ill ; prefetch r25 }
    ad20:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; ill ; prefetch r25 }
    ad28:	[0-9a-f]* 	{ info 19 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    ad30:	[0-9a-f]* 	{ revbits r5, r6 ; info 19 ; prefetch r25 }
    ad38:	[0-9a-f]* 	{ info 19 ; prefetch r25 }
    ad40:	[0-9a-f]* 	{ revbits r5, r6 ; jalr r15 ; prefetch r25 }
    ad48:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    ad50:	[0-9a-f]* 	{ subx r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    ad58:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jr r15 ; prefetch r25 }
    ad60:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; jrp r15 ; prefetch r25 }
    ad68:	[0-9a-f]* 	{ shli r5, r6, 5 ; jrp r15 ; prefetch r25 }
    ad70:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; lnk r15 ; prefetch r25 }
    ad78:	[0-9a-f]* 	{ mnz r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    ad80:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch r25 }
    ad88:	[0-9a-f]* 	{ mnz r5, r6, r7 ; lnk r15 ; prefetch r25 }
    ad90:	[0-9a-f]* 	{ move r15, r16 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    ad98:	[0-9a-f]* 	{ move r15, r16 ; shrui r5, r6, 5 ; prefetch r25 }
    ada0:	[0-9a-f]* 	{ move r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
    ada8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    adb0:	[0-9a-f]* 	{ movei r5, 5 ; addi r15, r16, 5 ; prefetch r25 }
    adb8:	[0-9a-f]* 	{ movei r5, 5 ; shru r15, r16, r17 ; prefetch r25 }
    adc0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    adc8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    add0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jrp r15 ; prefetch r25 }
    add8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
    ade0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    ade8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; prefetch r25 }
    adf0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    adf8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    ae00:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    ae08:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    ae10:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    ae18:	[0-9a-f]* 	{ mz r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    ae20:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch r25 }
    ae28:	[0-9a-f]* 	{ mz r5, r6, r7 ; lnk r15 ; prefetch r25 }
    ae30:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nop ; prefetch r25 }
    ae38:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; nop ; prefetch r25 }
    ae40:	[0-9a-f]* 	{ nop ; shrui r5, r6, 5 ; prefetch r25 }
    ae48:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    ae50:	[0-9a-f]* 	{ nor r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    ae58:	[0-9a-f]* 	{ nor r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    ae60:	[0-9a-f]* 	{ pcnt r5, r6 ; or r15, r16, r17 ; prefetch r25 }
    ae68:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; prefetch r25 }
    ae70:	[0-9a-f]* 	{ pcnt r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    ae78:	[0-9a-f]* 	{ revbits r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    ae80:	[0-9a-f]* 	{ revbits r5, r6 ; shru r15, r16, r17 ; prefetch r25 }
    ae88:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch r25 }
    ae90:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    ae98:	[0-9a-f]* 	{ rotl r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    aea0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    aea8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; or r5, r6, r7 ; prefetch r25 }
    aeb0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; prefetch r25 }
    aeb8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    aec0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    aec8:	[0-9a-f]* 	{ shl r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    aed0:	[0-9a-f]* 	{ ctz r5, r6 ; shl1add r15, r16, r17 ; prefetch r25 }
    aed8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl1add r15, r16, r17 ; prefetch r25 }
    aee0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    aee8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    aef0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    aef8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    af00:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; or r5, r6, r7 ; prefetch r25 }
    af08:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch r25 }
    af10:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch r25 }
    af18:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    af20:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    af28:	[0-9a-f]* 	{ ctz r5, r6 ; shl3add r15, r16, r17 ; prefetch r25 }
    af30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl3add r15, r16, r17 ; prefetch r25 }
    af38:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    af40:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    af48:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    af50:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    af58:	[0-9a-f]* 	{ shli r15, r16, 5 ; or r5, r6, r7 ; prefetch r25 }
    af60:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch r25 }
    af68:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
    af70:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    af78:	[0-9a-f]* 	{ shrs r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    af80:	[0-9a-f]* 	{ ctz r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    af88:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    af90:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch r25 }
    af98:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    afa0:	[0-9a-f]* 	{ shru r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    afa8:	[0-9a-f]* 	{ shru r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
    afb0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; or r5, r6, r7 ; prefetch r25 }
    afb8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; prefetch r25 }
    afc0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    afc8:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
    afd0:	[0-9a-f]* 	{ sub r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    afd8:	[0-9a-f]* 	{ ctz r5, r6 ; subx r15, r16, r17 ; prefetch r25 }
    afe0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; subx r15, r16, r17 ; prefetch r25 }
    afe8:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    aff0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    aff8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jrp r15 ; prefetch r25 }
    b000:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpne r15, r16, r17 ; prefetch r25 }
    b008:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 ; prefetch r25 }
    b010:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; prefetch r25 }
    b018:	[0-9a-f]* 	{ revbits r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    b020:	[0-9a-f]* 	{ xor r5, r6, r7 ; info 19 ; prefetch r25 }
    b028:	[0-9a-f]* 	{ bfexts r5, r6, 5, 7 ; prefetch_add_l1 r15, 5 }
    b030:	[0-9a-f]* 	{ fsingle_mul1 r5, r6, r7 ; prefetch_add_l1 r15, 5 }
    b038:	[0-9a-f]* 	{ revbits r5, r6 ; prefetch_add_l1 r15, 5 }
    b040:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; prefetch_add_l1 r15, 5 }
    b048:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; prefetch_add_l1 r15, 5 }
    b050:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; prefetch_add_l1 r15, 5 }
    b058:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    b060:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    b068:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    b070:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    b078:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    b080:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; prefetch_add_l2 r15, 5 }
    b088:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; prefetch_add_l2 r15, 5 }
    b090:	[0-9a-f]* 	{ rotl r5, r6, r7 ; prefetch_add_l2 r15, 5 }
    b098:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; prefetch_add_l2 r15, 5 }
    b0a0:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; prefetch_add_l2 r15, 5 }
    b0a8:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; prefetch_add_l2 r15, 5 }
    b0b0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
    b0b8:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
    b0c0:	[0-9a-f]* 	{ subx r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
    b0c8:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
    b0d0:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
    b0d8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    b0e0:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    b0e8:	[0-9a-f]* 	{ shl r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    b0f0:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    b0f8:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; prefetch_add_l3 r15, 5 }
    b100:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; prefetch_add_l3 r15, 5 }
    b108:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    b110:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    b118:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; prefetch_add_l3_fault r15, 5 }
    b120:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    b128:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    b130:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; prefetch r15 }
    b138:	[0-9a-f]* 	{ infol 4660 ; prefetch r15 }
    b140:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch r15 }
    b148:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; prefetch r15 }
    b150:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; prefetch r15 }
    b158:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; prefetch r15 }
    b160:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; prefetch r25 }
    b168:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
    b170:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    b178:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl2add r5, r6, r7 ; prefetch r25 }
    b180:	[0-9a-f]* 	{ addi r5, r6, 5 ; move r15, r16 ; prefetch r25 }
    b188:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
    b190:	[0-9a-f]* 	{ addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch r25 }
    b198:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    b1a0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    b1a8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; addxi r15, r16, 5 ; prefetch r25 }
    b1b0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; sub r15, r16, r17 ; prefetch r25 }
    b1b8:	[0-9a-f]* 	{ and r15, r16, r17 ; nor r5, r6, r7 ; prefetch r25 }
    b1c0:	[0-9a-f]* 	{ and r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
    b1c8:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    b1d0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl2add r5, r6, r7 ; prefetch r25 }
    b1d8:	[0-9a-f]* 	{ andi r5, r6, 5 ; move r15, r16 ; prefetch r25 }
    b1e0:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; prefetch r25 }
    b1e8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch r25 }
    b1f0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    b1f8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shrui r15, r16, 5 ; prefetch r25 }
    b200:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; nop ; prefetch r25 }
    b208:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    b210:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    b218:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch r25 }
    b220:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch r25 }
    b228:	[0-9a-f]* 	{ cmples r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch r25 }
    b230:	[0-9a-f]* 	{ cmples r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    b238:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    b240:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch r25 }
    b248:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    b250:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shrui r15, r16, 5 ; prefetch r25 }
    b258:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; nop ; prefetch r25 }
    b260:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    b268:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    b270:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch r25 }
    b278:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch r25 }
    b280:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch r25 }
    b288:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    b290:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    b298:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
    b2a0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    b2a8:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shrui r15, r16, 5 ; prefetch r25 }
    b2b0:	[0-9a-f]* 	{ ctz r5, r6 ; shl2addx r15, r16, r17 ; prefetch r25 }
    b2b8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; prefetch r25 }
    b2c0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; prefetch r25 }
    b2c8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
    b2d0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    b2d8:	[0-9a-f]* 	{ nop ; ill ; prefetch r25 }
    b2e0:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; prefetch r25 }
    b2e8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; info 19 ; prefetch r25 }
    b2f0:	[0-9a-f]* 	{ info 19 ; shru r5, r6, r7 ; prefetch r25 }
    b2f8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalr r15 ; prefetch r25 }
    b300:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jalrp r15 ; prefetch r25 }
    b308:	[0-9a-f]* 	{ shl r5, r6, r7 ; jalrp r15 ; prefetch r25 }
    b310:	[0-9a-f]* 	{ info 19 ; jr r15 ; prefetch r25 }
    b318:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jr r15 ; prefetch r25 }
    b320:	[0-9a-f]* 	{ or r5, r6, r7 ; jrp r15 ; prefetch r25 }
    b328:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; lnk r15 ; prefetch r25 }
    b330:	[0-9a-f]* 	{ shrui r5, r6, 5 ; lnk r15 ; prefetch r25 }
    b338:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    b340:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    b348:	[0-9a-f]* 	{ move r15, r16 ; addi r5, r6, 5 ; prefetch r25 }
    b350:	[0-9a-f]* 	{ move r15, r16 ; rotl r5, r6, r7 ; prefetch r25 }
    b358:	[0-9a-f]* 	{ move r5, r6 ; jalrp r15 ; prefetch r25 }
    b360:	[0-9a-f]* 	{ movei r15, 5 ; cmples r5, r6, r7 ; prefetch r25 }
    b368:	[0-9a-f]* 	{ movei r15, 5 ; shrs r5, r6, r7 ; prefetch r25 }
    b370:	[0-9a-f]* 	{ movei r5, 5 ; or r15, r16, r17 ; prefetch r25 }
    b378:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; lnk r15 ; prefetch r25 }
    b380:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; prefetch r25 }
    b388:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch r25 }
    b390:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
    b398:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch r25 }
    b3a0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    b3a8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; nop ; prefetch r25 }
    b3b0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jr r15 ; prefetch r25 }
    b3b8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    b3c0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    b3c8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    b3d0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    b3d8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    b3e0:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    b3e8:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; prefetch r25 }
    b3f0:	[0-9a-f]* 	{ nop ; mnz r15, r16, r17 ; prefetch r25 }
    b3f8:	[0-9a-f]* 	{ nop ; shl3add r15, r16, r17 ; prefetch r25 }
    b400:	[0-9a-f]* 	{ nor r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch r25 }
    b408:	[0-9a-f]* 	{ nor r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    b410:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    b418:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    b420:	[0-9a-f]* 	{ or r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
    b428:	[0-9a-f]* 	{ or r5, r6, r7 ; shrui r15, r16, 5 ; prefetch r25 }
    b430:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2addx r15, r16, r17 ; prefetch r25 }
    b438:	[0-9a-f]* 	{ revbits r5, r6 ; or r15, r16, r17 ; prefetch r25 }
    b440:	[0-9a-f]* 	{ revbytes r5, r6 ; lnk r15 ; prefetch r25 }
    b448:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    b450:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shrui r5, r6, 5 ; prefetch r25 }
    b458:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    b460:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    b468:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    b470:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
    b478:	[0-9a-f]* 	{ shl r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    b480:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    b488:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    b490:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch r25 }
    b498:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; lnk r15 ; prefetch r25 }
    b4a0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    b4a8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shrui r5, r6, 5 ; prefetch r25 }
    b4b0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    b4b8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    b4c0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    b4c8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    b4d0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    b4d8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    b4e0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    b4e8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch r25 }
    b4f0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; lnk r15 ; prefetch r25 }
    b4f8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    b500:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shrui r5, r6, 5 ; prefetch r25 }
    b508:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    b510:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    b518:	[0-9a-f]* 	{ shli r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    b520:	[0-9a-f]* 	{ shli r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
    b528:	[0-9a-f]* 	{ shrs r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    b530:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    b538:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; and r5, r6, r7 ; prefetch r25 }
    b540:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch r25 }
    b548:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; lnk r15 ; prefetch r25 }
    b550:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
    b558:	[0-9a-f]* 	{ shru r15, r16, r17 ; shrui r5, r6, 5 ; prefetch r25 }
    b560:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    b568:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch r25 }
    b570:	[0-9a-f]* 	{ shrui r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    b578:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
    b580:	[0-9a-f]* 	{ sub r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    b588:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch r25 }
    b590:	[0-9a-f]* 	{ subx r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    b598:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch r25 }
    b5a0:	[0-9a-f]* 	{ subx r5, r6, r7 ; lnk r15 ; prefetch r25 }
    b5a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; prefetch r25 }
    b5b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch r25 }
    b5b8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; add r15, r16, r17 ; prefetch r25 }
    b5c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    b5c8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; prefetch r25 }
    b5d0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    b5d8:	[0-9a-f]* 	{ xor r5, r6, r7 ; addxi r15, r16, 5 ; prefetch r25 }
    b5e0:	[0-9a-f]* 	{ xor r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    b5e8:	[0-9a-f]* 	{ dblalign4 r5, r6, r7 ; prefetch_l1_fault r15 }
    b5f0:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; prefetch_l1_fault r15 }
    b5f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; prefetch_l1_fault r15 }
    b600:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; prefetch_l1_fault r15 }
    b608:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; prefetch_l1_fault r15 }
    b610:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    b618:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    b620:	[0-9a-f]* 	{ add r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l1_fault r25 }
    b628:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
    b630:	[0-9a-f]* 	{ addi r5, r6, 5 ; and r15, r16, r17 ; prefetch_l1_fault r25 }
    b638:	[0-9a-f]* 	{ addi r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
    b640:	[0-9a-f]* 	{ addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
    b648:	[0-9a-f]* 	{ addx r5, r6, r7 ; prefetch_l1_fault r25 }
    b650:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    b658:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    b660:	[0-9a-f]* 	{ addxi r5, r6, 5 ; movei r15, 5 ; prefetch_l1_fault r25 }
    b668:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; prefetch_l1_fault r25 }
    b670:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; prefetch_l1_fault r25 }
    b678:	[0-9a-f]* 	{ and r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l1_fault r25 }
    b680:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1_fault r25 }
    b688:	[0-9a-f]* 	{ andi r5, r6, 5 ; and r15, r16, r17 ; prefetch_l1_fault r25 }
    b690:	[0-9a-f]* 	{ andi r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
    b698:	[0-9a-f]* 	{ clz r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b6a0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    b6a8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; move r15, r16 ; prefetch_l1_fault r25 }
    b6b0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l1_fault r25 }
    b6b8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l1_fault r25 }
    b6c0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b6c8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l1_fault r25 }
    b6d0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    b6d8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l1_fault r25 }
    b6e0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
    b6e8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
    b6f0:	[0-9a-f]* 	{ clz r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l1_fault r25 }
    b6f8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l1_fault r25 }
    b700:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; move r15, r16 ; prefetch_l1_fault r25 }
    b708:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l1_fault r25 }
    b710:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l1_fault r25 }
    b718:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b720:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l1_fault r25 }
    b728:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
    b730:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l1_fault r25 }
    b738:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
    b740:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
    b748:	[0-9a-f]* 	{ clz r5, r6 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
    b750:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l1_fault r25 }
    b758:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; move r15, r16 ; prefetch_l1_fault r25 }
    b760:	[0-9a-f]* 	{ ctz r5, r6 ; info 19 ; prefetch_l1_fault r25 }
    b768:	[0-9a-f]* 	{ and r5, r6, r7 ; prefetch_l1_fault r25 }
    b770:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; prefetch_l1_fault r25 }
    b778:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    b780:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; move r15, r16 ; prefetch_l1_fault r25 }
    b788:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; ill ; prefetch_l1_fault r25 }
    b790:	[0-9a-f]* 	{ subx r5, r6, r7 ; ill ; prefetch_l1_fault r25 }
    b798:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; info 19 ; prefetch_l1_fault r25 }
    b7a0:	[0-9a-f]* 	{ info 19 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    b7a8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jalr r15 ; prefetch_l1_fault r25 }
    b7b0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jalr r15 ; prefetch_l1_fault r25 }
    b7b8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch_l1_fault r25 }
    b7c0:	[0-9a-f]* 	{ addi r5, r6, 5 ; jr r15 ; prefetch_l1_fault r25 }
    b7c8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jr r15 ; prefetch_l1_fault r25 }
    b7d0:	[0-9a-f]* 	{ jrp r15 ; prefetch_l1_fault r25 }
    b7d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jrp r15 ; prefetch_l1_fault r25 }
    b7e0:	[0-9a-f]* 	{ nop ; lnk r15 ; prefetch_l1_fault r25 }
    b7e8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l1_fault r25 }
    b7f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l1_fault r25 }
    b7f8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l1_fault r25 }
    b800:	[0-9a-f]* 	{ move r15, r16 ; move r5, r6 ; prefetch_l1_fault r25 }
    b808:	[0-9a-f]* 	{ move r15, r16 ; prefetch_l1_fault r25 }
    b810:	[0-9a-f]* 	{ move r5, r6 ; shrs r15, r16, r17 ; prefetch_l1_fault r25 }
    b818:	[0-9a-f]* 	{ mulax r5, r6, r7 ; movei r15, 5 ; prefetch_l1_fault r25 }
    b820:	[0-9a-f]* 	{ movei r5, 5 ; cmpleu r15, r16, r17 ; prefetch_l1_fault r25 }
    b828:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b830:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    b838:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b840:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
    b848:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; lnk r15 ; prefetch_l1_fault r25 }
    b850:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 }
    b858:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l1_fault r25 }
    b860:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; prefetch_l1_fault r25 }
    b868:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    b870:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b878:	[0-9a-f]* 	{ mulax r5, r6, r7 ; nop ; prefetch_l1_fault r25 }
    b880:	[0-9a-f]* 	{ mulx r5, r6, r7 ; jr r15 ; prefetch_l1_fault r25 }
    b888:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l1_fault r25 }
    b890:	[0-9a-f]* 	{ mz r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l1_fault r25 }
    b898:	[0-9a-f]* 	{ mz r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l1_fault r25 }
    b8a0:	[0-9a-f]* 	{ nop ; cmpleu r5, r6, r7 ; prefetch_l1_fault r25 }
    b8a8:	[0-9a-f]* 	{ nop ; or r15, r16, r17 ; prefetch_l1_fault r25 }
    b8b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nop ; prefetch_l1_fault r25 }
    b8b8:	[0-9a-f]* 	{ nor r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
    b8c0:	[0-9a-f]* 	{ nor r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
    b8c8:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
    b8d0:	[0-9a-f]* 	{ or r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l1_fault r25 }
    b8d8:	[0-9a-f]* 	{ or r5, r6, r7 ; move r15, r16 ; prefetch_l1_fault r25 }
    b8e0:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; prefetch_l1_fault r25 }
    b8e8:	[0-9a-f]* 	{ revbits r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l1_fault r25 }
    b8f0:	[0-9a-f]* 	{ revbytes r5, r6 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b8f8:	[0-9a-f]* 	{ revbytes r5, r6 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    b900:	[0-9a-f]* 	{ rotl r15, r16, r17 ; nop ; prefetch_l1_fault r25 }
    b908:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    b910:	[0-9a-f]* 	{ rotli r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l1_fault r25 }
    b918:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l1_fault r25 }
    b920:	[0-9a-f]* 	{ rotli r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    b928:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    b930:	[0-9a-f]* 	{ shl r15, r16, r17 ; sub r5, r6, r7 ; prefetch_l1_fault r25 }
    b938:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    b940:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    b948:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b950:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    b958:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; nop ; prefetch_l1_fault r25 }
    b960:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    b968:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l1_fault r25 }
    b970:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l1_fault r25 }
    b978:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    b980:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    b988:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; sub r5, r6, r7 ; prefetch_l1_fault r25 }
    b990:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    b998:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l1_fault r25 }
    b9a0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    b9a8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    b9b0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; nop ; prefetch_l1_fault r25 }
    b9b8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    b9c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l1_fault r25 }
    b9c8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l1_fault r25 }
    b9d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    b9d8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    b9e0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; sub r5, r6, r7 ; prefetch_l1_fault r25 }
    b9e8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    b9f0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
    b9f8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    ba00:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    ba08:	[0-9a-f]* 	{ shru r15, r16, r17 ; nop ; prefetch_l1_fault r25 }
    ba10:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    ba18:	[0-9a-f]* 	{ shrui r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l1_fault r25 }
    ba20:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l1_fault r25 }
    ba28:	[0-9a-f]* 	{ shrui r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
    ba30:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    ba38:	[0-9a-f]* 	{ sub r15, r16, r17 ; sub r5, r6, r7 ; prefetch_l1_fault r25 }
    ba40:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    ba48:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
    ba50:	[0-9a-f]* 	{ subx r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l1_fault r25 }
    ba58:	[0-9a-f]* 	{ subx r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l1_fault r25 }
    ba60:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    ba68:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
    ba70:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; lnk r15 ; prefetch_l1_fault r25 }
    ba78:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; prefetch_l1_fault r25 }
    ba80:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
    ba88:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
    ba90:	[0-9a-f]* 	{ xor r5, r6, r7 ; movei r15, 5 ; prefetch_l1_fault r25 }
    ba98:	[0-9a-f]* 	{ cmples r5, r6, r7 ; prefetch_l2 r15 }
    baa0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; prefetch_l2 r15 }
    baa8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch_l2 r15 }
    bab0:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; prefetch_l2 r15 }
    bab8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; prefetch_l2 r15 }
    bac0:	[0-9a-f]* 	{ xor r5, r6, r7 ; prefetch_l2 r15 }
    bac8:	[0-9a-f]* 	{ pcnt r5, r6 ; add r15, r16, r17 ; prefetch_l2 r25 }
    bad0:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; prefetch_l2 r25 }
    bad8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l2 r25 }
    bae0:	[0-9a-f]* 	{ addi r15, r16, 5 ; shl3add r5, r6, r7 ; prefetch_l2 r25 }
    bae8:	[0-9a-f]* 	{ addi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    baf0:	[0-9a-f]* 	{ addx r15, r16, r17 ; prefetch_l2 r25 }
    baf8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addx r15, r16, r17 ; prefetch_l2 r25 }
    bb00:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
    bb08:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    bb10:	[0-9a-f]* 	{ addxi r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l2 r25 }
    bb18:	[0-9a-f]* 	{ addxi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l2 r25 }
    bb20:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bb28:	[0-9a-f]* 	{ and r5, r6, r7 ; ill ; prefetch_l2 r25 }
    bb30:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l2 r25 }
    bb38:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl3add r5, r6, r7 ; prefetch_l2 r25 }
    bb40:	[0-9a-f]* 	{ andi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    bb48:	[0-9a-f]* 	{ clz r5, r6 ; jalrp r15 ; prefetch_l2 r25 }
    bb50:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l2 r25 }
    bb58:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bb60:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bb68:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2 r25 }
    bb70:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; prefetch_l2 r25 }
    bb78:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    bb80:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l2 r25 }
    bb88:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; movei r15, 5 ; prefetch_l2 r25 }
    bb90:	[0-9a-f]* 	{ ctz r5, r6 ; cmples r15, r16, r17 ; prefetch_l2 r25 }
    bb98:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmples r15, r16, r17 ; prefetch_l2 r25 }
    bba0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    bba8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
    bbb0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bbb8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bbc0:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2 r25 }
    bbc8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; prefetch_l2 r25 }
    bbd0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l2 r25 }
    bbd8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l2 r25 }
    bbe0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; movei r15, 5 ; prefetch_l2 r25 }
    bbe8:	[0-9a-f]* 	{ ctz r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    bbf0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    bbf8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    bc00:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    bc08:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bc10:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bc18:	[0-9a-f]* 	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
    bc20:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; prefetch_l2 r25 }
    bc28:	[0-9a-f]* 	{ rotli r5, r6, 5 ; prefetch_l2 r25 }
    bc30:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bc38:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bc40:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; prefetch_l2 r25 }
    bc48:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; info 19 ; prefetch_l2 r25 }
    bc50:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; info 19 ; prefetch_l2 r25 }
    bc58:	[0-9a-f]* 	{ info 19 ; shrui r5, r6, 5 ; prefetch_l2 r25 }
    bc60:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
    bc68:	[0-9a-f]* 	{ andi r5, r6, 5 ; jalrp r15 ; prefetch_l2 r25 }
    bc70:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    bc78:	[0-9a-f]* 	{ move r5, r6 ; jr r15 ; prefetch_l2 r25 }
    bc80:	[0-9a-f]* 	{ jr r15 ; prefetch_l2 r25 }
    bc88:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; prefetch_l2 r25 }
    bc90:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    bc98:	[0-9a-f]* 	{ subx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    bca0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    bca8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    bcb0:	[0-9a-f]* 	{ move r15, r16 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    bcb8:	[0-9a-f]* 	{ move r15, r16 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    bcc0:	[0-9a-f]* 	{ move r5, r6 ; jrp r15 ; prefetch_l2 r25 }
    bcc8:	[0-9a-f]* 	{ movei r15, 5 ; cmplts r5, r6, r7 ; prefetch_l2 r25 }
    bcd0:	[0-9a-f]* 	{ movei r15, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    bcd8:	[0-9a-f]* 	{ movei r5, 5 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    bce0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    bce8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; info 19 ; prefetch_l2 r25 }
    bcf0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
    bcf8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2 r25 }
    bd00:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    bd08:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
    bd10:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2 r25 }
    bd18:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    bd20:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; prefetch_l2 r25 }
    bd28:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
    bd30:	[0-9a-f]* 	{ mulx r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
    bd38:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    bd40:	[0-9a-f]* 	{ mulx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    bd48:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
    bd50:	[0-9a-f]* 	{ nop ; addi r5, r6, 5 ; prefetch_l2 r25 }
    bd58:	[0-9a-f]* 	{ nop ; move r15, r16 ; prefetch_l2 r25 }
    bd60:	[0-9a-f]* 	{ nop ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
    bd68:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    bd70:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; prefetch_l2 r25 }
    bd78:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    bd80:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2 r25 }
    bd88:	[0-9a-f]* 	{ or r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2 r25 }
    bd90:	[0-9a-f]* 	{ or r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bd98:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
    bda0:	[0-9a-f]* 	{ revbits r5, r6 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    bda8:	[0-9a-f]* 	{ revbytes r5, r6 ; move r15, r16 ; prefetch_l2 r25 }
    bdb0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2 r25 }
    bdb8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    bdc0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    bdc8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    bdd0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    bdd8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l2 r25 }
    bde0:	[0-9a-f]* 	{ shl r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    bde8:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    bdf0:	[0-9a-f]* 	{ clz r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    bdf8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l2 r25 }
    be00:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    be08:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2 r25 }
    be10:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    be18:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    be20:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    be28:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    be30:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2 r25 }
    be38:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    be40:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    be48:	[0-9a-f]* 	{ clz r5, r6 ; shl3add r15, r16, r17 ; prefetch_l2 r25 }
    be50:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l2 r25 }
    be58:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    be60:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2 r25 }
    be68:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    be70:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    be78:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2 r25 }
    be80:	[0-9a-f]* 	{ shli r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    be88:	[0-9a-f]* 	{ shli r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l2 r25 }
    be90:	[0-9a-f]* 	{ shrs r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    be98:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    bea0:	[0-9a-f]* 	{ clz r5, r6 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
    bea8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl2add r5, r6, r7 ; prefetch_l2 r25 }
    beb0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; move r15, r16 ; prefetch_l2 r25 }
    beb8:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2 r25 }
    bec0:	[0-9a-f]* 	{ shru r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    bec8:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    bed0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    bed8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
    bee0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; sub r15, r16, r17 ; prefetch_l2 r25 }
    bee8:	[0-9a-f]* 	{ sub r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    bef0:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2 r25 }
    bef8:	[0-9a-f]* 	{ clz r5, r6 ; subx r15, r16, r17 ; prefetch_l2 r25 }
    bf00:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l2 r25 }
    bf08:	[0-9a-f]* 	{ subx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    bf10:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; prefetch_l2 r25 }
    bf18:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
    bf20:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addx r15, r16, r17 ; prefetch_l2 r25 }
    bf28:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
    bf30:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
    bf38:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2 r25 }
    bf40:	[0-9a-f]* 	{ xor r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l2 r25 }
    bf48:	[0-9a-f]* 	{ xor r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2 r25 }
    bf50:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; prefetch_l2_fault r15 }
    bf58:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; prefetch_l2_fault r15 }
    bf60:	[0-9a-f]* 	{ v1add r5, r6, r7 ; prefetch_l2_fault r15 }
    bf68:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; prefetch_l2_fault r15 }
    bf70:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; prefetch_l2_fault r15 }
    bf78:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
    bf80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
    bf88:	[0-9a-f]* 	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    bf90:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l2_fault r25 }
    bf98:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    bfa0:	[0-9a-f]* 	{ addi r5, r6, 5 ; prefetch_l2_fault r25 }
    bfa8:	[0-9a-f]* 	{ revbits r5, r6 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
    bfb0:	[0-9a-f]* 	{ addx r5, r6, r7 ; info 19 ; prefetch_l2_fault r25 }
    bfb8:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l2_fault r25 }
    bfc0:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    bfc8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; nop ; prefetch_l2_fault r25 }
    bfd0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    bfd8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    bfe0:	[0-9a-f]* 	{ and r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    bfe8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l2_fault r25 }
    bff0:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    bff8:	[0-9a-f]* 	{ andi r5, r6, 5 ; prefetch_l2_fault r25 }
    c000:	[0-9a-f]* 	{ clz r5, r6 ; shrs r15, r16, r17 ; prefetch_l2_fault r25 }
    c008:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    c010:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    c018:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    c020:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l2_fault r25 }
    c028:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c030:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
    c038:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l2_fault r25 }
    c040:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    c048:	[0-9a-f]* 	{ pcnt r5, r6 ; cmples r15, r16, r17 ; prefetch_l2_fault r25 }
    c050:	[0-9a-f]* 	{ cmples r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    c058:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2_fault r25 }
    c060:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l2_fault r25 }
    c068:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    c070:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    c078:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    c080:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c088:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l2_fault r25 }
    c090:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l2_fault r25 }
    c098:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    c0a0:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
    c0a8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    c0b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l2_fault r25 }
    c0b8:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l2_fault r25 }
    c0c0:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    c0c8:	[0-9a-f]* 	{ ctz r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    c0d0:	[0-9a-f]* 	{ andi r5, r6, 5 ; prefetch_l2_fault r25 }
    c0d8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 }
    c0e0:	[0-9a-f]* 	{ shru r15, r16, r17 ; prefetch_l2_fault r25 }
    c0e8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    c0f0:	[0-9a-f]* 	{ ill ; prefetch_l2_fault r25 }
    c0f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; prefetch_l2_fault r25 }
    c100:	[0-9a-f]* 	{ info 19 ; info 19 ; prefetch_l2_fault r25 }
    c108:	[0-9a-f]* 	{ info 19 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c110:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
    c118:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
    c120:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jalrp r15 ; prefetch_l2_fault r25 }
    c128:	[0-9a-f]* 	{ addxi r5, r6, 5 ; jr r15 ; prefetch_l2_fault r25 }
    c130:	[0-9a-f]* 	{ shl r5, r6, r7 ; jr r15 ; prefetch_l2_fault r25 }
    c138:	[0-9a-f]* 	{ info 19 ; jrp r15 ; prefetch_l2_fault r25 }
    c140:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jrp r15 ; prefetch_l2_fault r25 }
    c148:	[0-9a-f]* 	{ or r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    c150:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2_fault r25 }
    c158:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l2_fault r25 }
    c160:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    c168:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; move r15, r16 ; prefetch_l2_fault r25 }
    c170:	[0-9a-f]* 	{ move r5, r6 ; addi r15, r16, 5 ; prefetch_l2_fault r25 }
    c178:	[0-9a-f]* 	{ move r5, r6 ; shru r15, r16, r17 ; prefetch_l2_fault r25 }
    c180:	[0-9a-f]* 	{ movei r15, 5 ; mz r5, r6, r7 ; prefetch_l2_fault r25 }
    c188:	[0-9a-f]* 	{ movei r5, 5 ; cmpltsi r15, r16, 5 ; prefetch_l2_fault r25 }
    c190:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c198:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c1a0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c1a8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    c1b0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; move r15, r16 ; prefetch_l2_fault r25 }
    c1b8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; info 19 ; prefetch_l2_fault r25 }
    c1c0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2_fault r25 }
    c1c8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c1d0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2_fault r25 }
    c1d8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c1e0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    c1e8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; lnk r15 ; prefetch_l2_fault r25 }
    c1f0:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2_fault r25 }
    c1f8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l2_fault r25 }
    c200:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    c208:	[0-9a-f]* 	{ nop ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    c210:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; prefetch_l2_fault r25 }
    c218:	[0-9a-f]* 	{ nop ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
    c220:	[0-9a-f]* 	{ pcnt r5, r6 ; nor r15, r16, r17 ; prefetch_l2_fault r25 }
    c228:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
    c230:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
    c238:	[0-9a-f]* 	{ or r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l2_fault r25 }
    c240:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
    c248:	[0-9a-f]* 	{ pcnt r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
    c250:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltsi r15, r16, 5 ; prefetch_l2_fault r25 }
    c258:	[0-9a-f]* 	{ revbytes r5, r6 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c260:	[0-9a-f]* 	{ revbytes r5, r6 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c268:	[0-9a-f]* 	{ rotl r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2_fault r25 }
    c270:	[0-9a-f]* 	{ rotl r5, r6, r7 ; prefetch_l2_fault r25 }
    c278:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    c280:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c288:	[0-9a-f]* 	{ rotli r5, r6, 5 ; movei r15, 5 ; prefetch_l2_fault r25 }
    c290:	[0-9a-f]* 	{ ctz r5, r6 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    c298:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
    c2a0:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    c2a8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    c2b0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c2b8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c2c0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2_fault r25 }
    c2c8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c2d0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    c2d8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c2e0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; movei r15, 5 ; prefetch_l2_fault r25 }
    c2e8:	[0-9a-f]* 	{ ctz r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c2f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c2f8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    c300:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    c308:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c310:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c318:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2_fault r25 }
    c320:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c328:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
    c330:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c338:	[0-9a-f]* 	{ shli r5, r6, 5 ; movei r15, 5 ; prefetch_l2_fault r25 }
    c340:	[0-9a-f]* 	{ ctz r5, r6 ; shrs r15, r16, r17 ; prefetch_l2_fault r25 }
    c348:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; prefetch_l2_fault r25 }
    c350:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    c358:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2_fault r25 }
    c360:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c368:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c370:	[0-9a-f]* 	{ shru r15, r16, r17 ; or r5, r6, r7 ; prefetch_l2_fault r25 }
    c378:	[0-9a-f]* 	{ shru r5, r6, r7 ; prefetch_l2_fault r25 }
    c380:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2_fault r25 }
    c388:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c390:	[0-9a-f]* 	{ shrui r5, r6, 5 ; movei r15, 5 ; prefetch_l2_fault r25 }
    c398:	[0-9a-f]* 	{ ctz r5, r6 ; sub r15, r16, r17 ; prefetch_l2_fault r25 }
    c3a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sub r15, r16, r17 ; prefetch_l2_fault r25 }
    c3a8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    c3b0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c3b8:	[0-9a-f]* 	{ subx r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
    c3c0:	[0-9a-f]* 	{ subx r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l2_fault r25 }
    c3c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2_fault r25 }
    c3d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    c3d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; prefetch_l2_fault r25 }
    c3e0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; info 19 ; prefetch_l2_fault r25 }
    c3e8:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l2_fault r25 }
    c3f0:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    c3f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; nop ; prefetch_l2_fault r25 }
    c400:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; prefetch_l3 r15 }
    c408:	[0-9a-f]* 	{ movei r5, 5 ; prefetch_l3 r15 }
    c410:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; prefetch_l3 r15 }
    c418:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; prefetch_l3 r15 }
    c420:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; prefetch_l3 r15 }
    c428:	[0-9a-f]* 	{ add r15, r16, r17 ; add r5, r6, r7 ; prefetch_l3 r25 }
    c430:	[0-9a-f]* 	{ revbytes r5, r6 ; add r15, r16, r17 ; prefetch_l3 r25 }
    c438:	[0-9a-f]* 	{ add r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    c440:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpeqi r5, r6, 5 ; prefetch_l3 r25 }
    c448:	[0-9a-f]* 	{ addi r15, r16, 5 ; shli r5, r6, 5 ; prefetch_l3 r25 }
    c450:	[0-9a-f]* 	{ addi r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    c458:	[0-9a-f]* 	{ addx r15, r16, r17 ; info 19 ; prefetch_l3 r25 }
    c460:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    c468:	[0-9a-f]* 	{ addx r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    c470:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3 r25 }
    c478:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmpeqi r15, r16, 5 ; prefetch_l3 r25 }
    c480:	[0-9a-f]* 	{ and r15, r16, r17 ; add r5, r6, r7 ; prefetch_l3 r25 }
    c488:	[0-9a-f]* 	{ revbytes r5, r6 ; and r15, r16, r17 ; prefetch_l3 r25 }
    c490:	[0-9a-f]* 	{ and r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    c498:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpeqi r5, r6, 5 ; prefetch_l3 r25 }
    c4a0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shli r5, r6, 5 ; prefetch_l3 r25 }
    c4a8:	[0-9a-f]* 	{ andi r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    c4b0:	[0-9a-f]* 	{ clz r5, r6 ; jrp r15 ; prefetch_l3 r25 }
    c4b8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    c4c0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c4c8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; prefetch_l3 r25 }
    c4d0:	[0-9a-f]* 	{ revbits r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c4d8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    c4e0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
    c4e8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l3 r25 }
    c4f0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; nop ; prefetch_l3 r25 }
    c4f8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    c500:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmples r15, r16, r17 ; prefetch_l3 r25 }
    c508:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    c510:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
    c518:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c520:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; prefetch_l3 r25 }
    c528:	[0-9a-f]* 	{ revbits r5, r6 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
    c530:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    c538:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
    c540:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l3 r25 }
    c548:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; nop ; prefetch_l3 r25 }
    c550:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    c558:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    c560:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    c568:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    c570:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c578:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; prefetch_l3 r25 }
    c580:	[0-9a-f]* 	{ ctz r5, r6 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
    c588:	[0-9a-f]* 	{ prefetch_l3 r25 }
    c590:	[0-9a-f]* 	{ shl r5, r6, r7 ; prefetch_l3 r25 }
    c598:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c5a0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; prefetch_l3 r25 }
    c5a8:	[0-9a-f]* 	{ revbits r5, r6 ; ill ; prefetch_l3 r25 }
    c5b0:	[0-9a-f]* 	{ info 19 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
    c5b8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    c5c0:	[0-9a-f]* 	{ info 19 ; sub r5, r6, r7 ; prefetch_l3 r25 }
    c5c8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
    c5d0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jalrp r15 ; prefetch_l3 r25 }
    c5d8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jalrp r15 ; prefetch_l3 r25 }
    c5e0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
    c5e8:	[0-9a-f]* 	{ addi r5, r6, 5 ; jrp r15 ; prefetch_l3 r25 }
    c5f0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jrp r15 ; prefetch_l3 r25 }
    c5f8:	[0-9a-f]* 	{ lnk r15 ; prefetch_l3 r25 }
    c600:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; lnk r15 ; prefetch_l3 r25 }
    c608:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nop ; prefetch_l3 r25 }
    c610:	[0-9a-f]* 	{ mnz r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    c618:	[0-9a-f]* 	{ move r15, r16 ; andi r5, r6, 5 ; prefetch_l3 r25 }
    c620:	[0-9a-f]* 	{ move r15, r16 ; shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    c628:	[0-9a-f]* 	{ move r5, r6 ; mnz r15, r16, r17 ; prefetch_l3 r25 }
    c630:	[0-9a-f]* 	{ movei r15, 5 ; cmpltu r5, r6, r7 ; prefetch_l3 r25 }
    c638:	[0-9a-f]* 	{ movei r15, 5 ; sub r5, r6, r7 ; prefetch_l3 r25 }
    c640:	[0-9a-f]* 	{ movei r5, 5 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    c648:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c650:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; jalrp r15 ; prefetch_l3 r25 }
    c658:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    c660:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3 r25 }
    c668:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    c670:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    c678:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3 r25 }
    c680:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; move r15, r16 ; prefetch_l3 r25 }
    c688:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; info 19 ; prefetch_l3 r25 }
    c690:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
    c698:	[0-9a-f]* 	{ mulx r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    c6a0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    c6a8:	[0-9a-f]* 	{ mz r15, r16, r17 ; nop ; prefetch_l3 r25 }
    c6b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
    c6b8:	[0-9a-f]* 	{ nop ; addx r5, r6, r7 ; prefetch_l3 r25 }
    c6c0:	[0-9a-f]* 	{ nop ; movei r15, 5 ; prefetch_l3 r25 }
    c6c8:	[0-9a-f]* 	{ nop ; shli r15, r16, 5 ; prefetch_l3 r25 }
    c6d0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    c6d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    c6e0:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    c6e8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3 r25 }
    c6f0:	[0-9a-f]* 	{ or r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    c6f8:	[0-9a-f]* 	{ or r5, r6, r7 ; prefetch_l3 r25 }
    c700:	[0-9a-f]* 	{ pcnt r5, r6 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
    c708:	[0-9a-f]* 	{ revbits r5, r6 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    c710:	[0-9a-f]* 	{ revbytes r5, r6 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c718:	[0-9a-f]* 	{ rotl r15, r16, r17 ; prefetch_l3 r25 }
    c720:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotl r15, r16, r17 ; prefetch_l3 r25 }
    c728:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    c730:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3 r25 }
    c738:	[0-9a-f]* 	{ rotli r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3 r25 }
    c740:	[0-9a-f]* 	{ rotli r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    c748:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; prefetch_l3 r25 }
    c750:	[0-9a-f]* 	{ shl r5, r6, r7 ; ill ; prefetch_l3 r25 }
    c758:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    c760:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    c768:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c770:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; prefetch_l3 r25 }
    c778:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l3 r25 }
    c780:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    c788:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
    c790:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3 r25 }
    c798:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    c7a0:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    c7a8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; ill ; prefetch_l3 r25 }
    c7b0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    c7b8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    c7c0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c7c8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    c7d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    c7d8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    c7e0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3 r25 }
    c7e8:	[0-9a-f]* 	{ shli r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3 r25 }
    c7f0:	[0-9a-f]* 	{ shli r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    c7f8:	[0-9a-f]* 	{ pcnt r5, r6 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
    c800:	[0-9a-f]* 	{ shrs r5, r6, r7 ; ill ; prefetch_l3 r25 }
    c808:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l3 r25 }
    c810:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    c818:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c820:	[0-9a-f]* 	{ shru r15, r16, r17 ; prefetch_l3 r25 }
    c828:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shru r15, r16, r17 ; prefetch_l3 r25 }
    c830:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    c838:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    c840:	[0-9a-f]* 	{ shrui r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3 r25 }
    c848:	[0-9a-f]* 	{ shrui r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    c850:	[0-9a-f]* 	{ pcnt r5, r6 ; sub r15, r16, r17 ; prefetch_l3 r25 }
    c858:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; prefetch_l3 r25 }
    c860:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    c868:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl3add r5, r6, r7 ; prefetch_l3 r25 }
    c870:	[0-9a-f]* 	{ subx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
    c878:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalrp r15 ; prefetch_l3 r25 }
    c880:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
    c888:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; prefetch_l3 r25 }
    c890:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    c898:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    c8a0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    c8a8:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3 r25 }
    c8b0:	[0-9a-f]* 	{ add r5, r6, r7 ; prefetch_l3_fault r15 }
    c8b8:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; prefetch_l3_fault r15 }
    c8c0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; prefetch_l3_fault r15 }
    c8c8:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; prefetch_l3_fault r15 }
    c8d0:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; prefetch_l3_fault r15 }
    c8d8:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; prefetch_l3_fault r15 }
    c8e0:	[0-9a-f]* 	{ add r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    c8e8:	[0-9a-f]* 	{ add r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    c8f0:	[0-9a-f]* 	{ add r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    c8f8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l3_fault r25 }
    c900:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    c908:	[0-9a-f]* 	{ addx r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
    c910:	[0-9a-f]* 	{ addx r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l3_fault r25 }
    c918:	[0-9a-f]* 	{ addx r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    c920:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l3_fault r25 }
    c928:	[0-9a-f]* 	{ addxi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l3_fault r25 }
    c930:	[0-9a-f]* 	{ addxi r5, r6, 5 ; or r15, r16, r17 ; prefetch_l3_fault r25 }
    c938:	[0-9a-f]* 	{ and r15, r16, r17 ; mnz r5, r6, r7 ; prefetch_l3_fault r25 }
    c940:	[0-9a-f]* 	{ and r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
    c948:	[0-9a-f]* 	{ and r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    c950:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    c958:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    c960:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; prefetch_l3_fault r25 }
    c968:	[0-9a-f]* 	{ clz r5, r6 ; shru r15, r16, r17 ; prefetch_l3_fault r25 }
    c970:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    c978:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    c980:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; info 19 ; prefetch_l3_fault r25 }
    c988:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    c990:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    c998:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    c9a0:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    c9a8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; add r5, r6, r7 ; prefetch_l3_fault r25 }
    c9b0:	[0-9a-f]* 	{ revbytes r5, r6 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
    c9b8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    c9c0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    c9c8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    c9d0:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    c9d8:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; info 19 ; prefetch_l3_fault r25 }
    c9e0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l3_fault r25 }
    c9e8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    c9f0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 }
    c9f8:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 }
    ca00:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; add r5, r6, r7 ; prefetch_l3_fault r25 }
    ca08:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
    ca10:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    ca18:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    ca20:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    ca28:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    ca30:	[0-9a-f]* 	{ ctz r5, r6 ; jrp r15 ; prefetch_l3_fault r25 }
    ca38:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
    ca40:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
    ca48:	[0-9a-f]* 	{ shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    ca50:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    ca58:	[0-9a-f]* 	{ info 19 ; ill ; prefetch_l3_fault r25 }
    ca60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; prefetch_l3_fault r25 }
    ca68:	[0-9a-f]* 	{ info 19 ; jalrp r15 ; prefetch_l3_fault r25 }
    ca70:	[0-9a-f]* 	{ info 19 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    ca78:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    ca80:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    ca88:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    ca90:	[0-9a-f]* 	{ andi r5, r6, 5 ; jr r15 ; prefetch_l3_fault r25 }
    ca98:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; prefetch_l3_fault r25 }
    caa0:	[0-9a-f]* 	{ move r5, r6 ; jrp r15 ; prefetch_l3_fault r25 }
    caa8:	[0-9a-f]* 	{ jrp r15 ; prefetch_l3_fault r25 }
    cab0:	[0-9a-f]* 	{ revbits r5, r6 ; lnk r15 ; prefetch_l3_fault r25 }
    cab8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l3_fault r25 }
    cac0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l3_fault r25 }
    cac8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cad0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; move r15, r16 ; prefetch_l3_fault r25 }
    cad8:	[0-9a-f]* 	{ move r5, r6 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    cae0:	[0-9a-f]* 	{ move r5, r6 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    cae8:	[0-9a-f]* 	{ movei r15, 5 ; nor r5, r6, r7 ; prefetch_l3_fault r25 }
    caf0:	[0-9a-f]* 	{ movei r5, 5 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
    caf8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    cb00:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 }
    cb08:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    cb10:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3_fault r25 }
    cb18:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3_fault r25 }
    cb20:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jalrp r15 ; prefetch_l3_fault r25 }
    cb28:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 }
    cb30:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3_fault r25 }
    cb38:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3_fault r25 }
    cb40:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cb48:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
    cb50:	[0-9a-f]* 	{ mulx r5, r6, r7 ; move r15, r16 ; prefetch_l3_fault r25 }
    cb58:	[0-9a-f]* 	{ mz r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l3_fault r25 }
    cb60:	[0-9a-f]* 	{ mz r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l3_fault r25 }
    cb68:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cb70:	[0-9a-f]* 	{ nop ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    cb78:	[0-9a-f]* 	{ revbytes r5, r6 ; nop ; prefetch_l3_fault r25 }
    cb80:	[0-9a-f]* 	{ nor r15, r16, r17 ; add r5, r6, r7 ; prefetch_l3_fault r25 }
    cb88:	[0-9a-f]* 	{ revbytes r5, r6 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    cb90:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    cb98:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    cba0:	[0-9a-f]* 	{ or r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    cba8:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3_fault r25 }
    cbb0:	[0-9a-f]* 	{ pcnt r5, r6 ; jrp r15 ; prefetch_l3_fault r25 }
    cbb8:	[0-9a-f]* 	{ revbits r5, r6 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
    cbc0:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    cbc8:	[0-9a-f]* 	{ revbytes r5, r6 ; prefetch_l3_fault r25 }
    cbd0:	[0-9a-f]* 	{ revbits r5, r6 ; rotl r15, r16, r17 ; prefetch_l3_fault r25 }
    cbd8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    cbe0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3_fault r25 }
    cbe8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l3_fault r25 }
    cbf0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nop ; prefetch_l3_fault r25 }
    cbf8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl r15, r16, r17 ; prefetch_l3_fault r25 }
    cc00:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl r15, r16, r17 ; prefetch_l3_fault r25 }
    cc08:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    cc10:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3_fault r25 }
    cc18:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    cc20:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch_l3_fault r25 }
    cc28:	[0-9a-f]* 	{ revbits r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cc30:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    cc38:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l3_fault r25 }
    cc40:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l3_fault r25 }
    cc48:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; nop ; prefetch_l3_fault r25 }
    cc50:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cc58:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cc60:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    cc68:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    cc70:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    cc78:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; prefetch_l3_fault r25 }
    cc80:	[0-9a-f]* 	{ revbits r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    cc88:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    cc90:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3_fault r25 }
    cc98:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l3_fault r25 }
    cca0:	[0-9a-f]* 	{ shli r5, r6, 5 ; nop ; prefetch_l3_fault r25 }
    cca8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    ccb0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    ccb8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    ccc0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    ccc8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    ccd0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; prefetch_l3_fault r25 }
    ccd8:	[0-9a-f]* 	{ revbits r5, r6 ; shru r15, r16, r17 ; prefetch_l3_fault r25 }
    cce0:	[0-9a-f]* 	{ shru r5, r6, r7 ; info 19 ; prefetch_l3_fault r25 }
    cce8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3_fault r25 }
    ccf0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l3_fault r25 }
    ccf8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; nop ; prefetch_l3_fault r25 }
    cd00:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    cd08:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
    cd10:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    cd18:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3_fault r25 }
    cd20:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    cd28:	[0-9a-f]* 	{ subx r5, r6, r7 ; prefetch_l3_fault r25 }
    cd30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    cd38:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; prefetch_l3_fault r25 }
    cd40:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; prefetch_l3_fault r25 }
    cd48:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalrp r15 ; prefetch_l3_fault r25 }
    cd50:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l3_fault r25 }
    cd58:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l3_fault r25 }
    cd60:	[0-9a-f]* 	{ xor r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3_fault r25 }
    cd68:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; raise }
    cd70:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; raise }
    cd78:	[0-9a-f]* 	{ shli r5, r6, 5 ; raise }
    cd80:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; raise }
    cd88:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; raise }
    cd90:	[0-9a-f]* 	{ revbits r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
    cd98:	[0-9a-f]* 	{ revbits r5, r6 ; addx r15, r16, r17 ; ld2s r25, r26 }
    cda0:	[0-9a-f]* 	{ revbits r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
    cda8:	[0-9a-f]* 	{ revbits r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
    cdb0:	[0-9a-f]* 	{ revbits r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
    cdb8:	[0-9a-f]* 	{ revbits r5, r6 ; cmplts r15, r16, r17 ; prefetch r25 }
    cdc0:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
    cdc8:	[0-9a-f]* 	{ revbits r5, r6 ; fetchand r15, r16, r17 }
    cdd0:	[0-9a-f]* 	{ revbits r5, r6 ; ill ; prefetch_l3_fault r25 }
    cdd8:	[0-9a-f]* 	{ revbits r5, r6 ; jalr r15 ; prefetch_l3 r25 }
    cde0:	[0-9a-f]* 	{ revbits r5, r6 ; jr r15 ; st r25, r26 }
    cde8:	[0-9a-f]* 	{ revbits r5, r6 ; ill ; ld r25, r26 }
    cdf0:	[0-9a-f]* 	{ revbits r5, r6 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
    cdf8:	[0-9a-f]* 	{ revbits r5, r6 ; ld1s_add r15, r16, 5 }
    ce00:	[0-9a-f]* 	{ revbits r5, r6 ; shli r15, r16, 5 ; ld1u r25, r26 }
    ce08:	[0-9a-f]* 	{ revbits r5, r6 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    ce10:	[0-9a-f]* 	{ revbits r5, r6 ; jrp r15 ; ld2u r25, r26 }
    ce18:	[0-9a-f]* 	{ revbits r5, r6 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
    ce20:	[0-9a-f]* 	{ revbits r5, r6 ; addx r15, r16, r17 ; ld4u r25, r26 }
    ce28:	[0-9a-f]* 	{ revbits r5, r6 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    ce30:	[0-9a-f]* 	{ revbits r5, r6 ; lnk r15 ; st4 r25, r26 }
    ce38:	[0-9a-f]* 	{ revbits r5, r6 ; move r15, r16 ; st4 r25, r26 }
    ce40:	[0-9a-f]* 	{ revbits r5, r6 ; mz r15, r16, r17 ; st4 r25, r26 }
    ce48:	[0-9a-f]* 	{ revbits r5, r6 ; or r15, r16, r17 ; ld r25, r26 }
    ce50:	[0-9a-f]* 	{ revbits r5, r6 ; jr r15 ; prefetch r25 }
    ce58:	[0-9a-f]* 	{ revbits r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    ce60:	[0-9a-f]* 	{ revbits r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    ce68:	[0-9a-f]* 	{ revbits r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    ce70:	[0-9a-f]* 	{ revbits r5, r6 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    ce78:	[0-9a-f]* 	{ revbits r5, r6 ; lnk r15 ; prefetch_l2_fault r25 }
    ce80:	[0-9a-f]* 	{ revbits r5, r6 ; cmpne r15, r16, r17 ; prefetch_l3 r25 }
    ce88:	[0-9a-f]* 	{ revbits r5, r6 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    ce90:	[0-9a-f]* 	{ revbits r5, r6 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
    ce98:	[0-9a-f]* 	{ revbits r5, r6 ; rotli r15, r16, 5 }
    cea0:	[0-9a-f]* 	{ revbits r5, r6 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    cea8:	[0-9a-f]* 	{ revbits r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    ceb0:	[0-9a-f]* 	{ revbits r5, r6 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    ceb8:	[0-9a-f]* 	{ revbits r5, r6 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    cec0:	[0-9a-f]* 	{ revbits r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
    cec8:	[0-9a-f]* 	{ revbits r5, r6 ; andi r15, r16, 5 ; st r25, r26 }
    ced0:	[0-9a-f]* 	{ revbits r5, r6 ; xor r15, r16, r17 ; st r25, r26 }
    ced8:	[0-9a-f]* 	{ revbits r5, r6 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    cee0:	[0-9a-f]* 	{ revbits r5, r6 ; or r15, r16, r17 ; st2 r25, r26 }
    cee8:	[0-9a-f]* 	{ revbits r5, r6 ; jr r15 ; st4 r25, r26 }
    cef0:	[0-9a-f]* 	{ revbits r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
    cef8:	[0-9a-f]* 	{ revbits r5, r6 ; v1cmpeq r15, r16, r17 }
    cf00:	[0-9a-f]* 	{ revbits r5, r6 ; v2maxsi r15, r16, 5 }
    cf08:	[0-9a-f]* 	{ revbits r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
    cf10:	[0-9a-f]* 	{ revbytes r5, r6 ; addi r15, r16, 5 ; prefetch_l3 r25 }
    cf18:	[0-9a-f]* 	{ revbytes r5, r6 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    cf20:	[0-9a-f]* 	{ revbytes r5, r6 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    cf28:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    cf30:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    cf38:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    cf40:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpne r15, r16, r17 }
    cf48:	[0-9a-f]* 	{ revbytes r5, r6 ; ill ; ld1u r25, r26 }
    cf50:	[0-9a-f]* 	{ revbytes r5, r6 ; jalr r15 ; ld1s r25, r26 }
    cf58:	[0-9a-f]* 	{ revbytes r5, r6 ; jr r15 ; ld2s r25, r26 }
    cf60:	[0-9a-f]* 	{ revbytes r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
    cf68:	[0-9a-f]* 	{ revbytes r5, r6 ; subx r15, r16, r17 ; ld r25, r26 }
    cf70:	[0-9a-f]* 	{ revbytes r5, r6 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    cf78:	[0-9a-f]* 	{ revbytes r5, r6 ; nor r15, r16, r17 ; ld1u r25, r26 }
    cf80:	[0-9a-f]* 	{ revbytes r5, r6 ; jalrp r15 ; ld2s r25, r26 }
    cf88:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    cf90:	[0-9a-f]* 	{ revbytes r5, r6 ; add r15, r16, r17 ; ld4s r25, r26 }
    cf98:	[0-9a-f]* 	{ revbytes r5, r6 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
    cfa0:	[0-9a-f]* 	{ revbytes r5, r6 ; shl r15, r16, r17 ; ld4u r25, r26 }
    cfa8:	[0-9a-f]* 	{ revbytes r5, r6 ; lnk r15 ; ld4u r25, r26 }
    cfb0:	[0-9a-f]* 	{ revbytes r5, r6 ; move r15, r16 ; ld4u r25, r26 }
    cfb8:	[0-9a-f]* 	{ revbytes r5, r6 ; mz r15, r16, r17 ; ld4u r25, r26 }
    cfc0:	[0-9a-f]* 	{ revbytes r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    cfc8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmples r15, r16, r17 ; prefetch r25 }
    cfd0:	[0-9a-f]* 	{ revbytes r5, r6 ; prefetch_add_l1_fault r15, 5 }
    cfd8:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch r25 }
    cfe0:	[0-9a-f]* 	{ revbytes r5, r6 ; nop ; prefetch_l1_fault r25 }
    cfe8:	[0-9a-f]* 	{ revbytes r5, r6 ; jalrp r15 ; prefetch_l2 r25 }
    cff0:	[0-9a-f]* 	{ revbytes r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    cff8:	[0-9a-f]* 	{ revbytes r5, r6 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    d000:	[0-9a-f]* 	{ revbytes r5, r6 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    d008:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    d010:	[0-9a-f]* 	{ revbytes r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
    d018:	[0-9a-f]* 	{ revbytes r5, r6 ; shl1add r15, r16, r17 ; prefetch r25 }
    d020:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    d028:	[0-9a-f]* 	{ revbytes r5, r6 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    d030:	[0-9a-f]* 	{ revbytes r5, r6 ; shli r15, r16, 5 ; st r25, r26 }
    d038:	[0-9a-f]* 	{ revbytes r5, r6 ; shrsi r15, r16, 5 ; st r25, r26 }
    d040:	[0-9a-f]* 	{ revbytes r5, r6 ; shrui r15, r16, 5 ; st2 r25, r26 }
    d048:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; st r25, r26 }
    d050:	[0-9a-f]* 	{ revbytes r5, r6 ; nop ; st1 r25, r26 }
    d058:	[0-9a-f]* 	{ revbytes r5, r6 ; jalr r15 ; st2 r25, r26 }
    d060:	[0-9a-f]* 	{ revbytes r5, r6 ; cmples r15, r16, r17 ; st4 r25, r26 }
    d068:	[0-9a-f]* 	{ revbytes r5, r6 ; st_add r15, r16, 5 }
    d070:	[0-9a-f]* 	{ revbytes r5, r6 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    d078:	[0-9a-f]* 	{ revbytes r5, r6 ; v2cmpeqi r15, r16, 5 }
    d080:	[0-9a-f]* 	{ revbytes r5, r6 ; xor r15, r16, r17 ; ld r25, r26 }
    d088:	[0-9a-f]* 	{ rotl r15, r16, r17 ; addi r5, r6, 5 ; ld1s r25, r26 }
    d090:	[0-9a-f]* 	{ rotl r15, r16, r17 ; addxi r5, r6, 5 ; ld1u r25, r26 }
    d098:	[0-9a-f]* 	{ rotl r15, r16, r17 ; andi r5, r6, 5 ; ld1u r25, r26 }
    d0a0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    d0a8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    d0b0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmples r5, r6, r7 ; ld4s r25, r26 }
    d0b8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch r25 }
    d0c0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    d0c8:	[0-9a-f]* 	{ ctz r5, r6 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    d0d0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; prefetch_l2 r25 }
    d0d8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; info 19 ; ld4u r25, r26 }
    d0e0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; ld r25, r26 }
    d0e8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; addxi r5, r6, 5 ; ld1s r25, r26 }
    d0f0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl r5, r6, r7 ; ld1s r25, r26 }
    d0f8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; info 19 ; ld1u r25, r26 }
    d100:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rotl r15, r16, r17 ; ld1u r25, r26 }
    d108:	[0-9a-f]* 	{ rotl r15, r16, r17 ; or r5, r6, r7 ; ld2s r25, r26 }
    d110:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2u r25, r26 }
    d118:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shrui r5, r6, 5 ; ld2u r25, r26 }
    d120:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; ld4s r25, r26 }
    d128:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotl r15, r16, r17 ; ld4u r25, r26 }
    d130:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl3add r5, r6, r7 ; ld4u r25, r26 }
    d138:	[0-9a-f]* 	{ rotl r15, r16, r17 ; move r5, r6 ; ld4s r25, r26 }
    d140:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotl r15, r16, r17 ; ld4u r25, r26 }
    d148:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; ld2s r25, r26 }
    d150:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotl r15, r16, r17 ; ld2u r25, r26 }
    d158:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
    d160:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotl r15, r16, r17 ; ld1u r25, r26 }
    d168:	[0-9a-f]* 	{ rotl r15, r16, r17 ; mz r5, r6, r7 ; ld2u r25, r26 }
    d170:	[0-9a-f]* 	{ rotl r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
    d178:	[0-9a-f]* 	{ pcnt r5, r6 ; rotl r15, r16, r17 ; prefetch r25 }
    d180:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
    d188:	[0-9a-f]* 	{ rotl r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    d190:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
    d198:	[0-9a-f]* 	{ rotl r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    d1a0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; prefetch_l1_fault r25 }
    d1a8:	[0-9a-f]* 	{ revbits r5, r6 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
    d1b0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    d1b8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2_fault r25 }
    d1c0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l3 r25 }
    d1c8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    d1d0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    d1d8:	[0-9a-f]* 	{ revbytes r5, r6 ; rotl r15, r16, r17 ; prefetch r25 }
    d1e0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
    d1e8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
    d1f0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
    d1f8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl3add r5, r6, r7 ; st1 r25, r26 }
    d200:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shli r5, r6, 5 ; st4 r25, r26 }
    d208:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    d210:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shrux r5, r6, r7 }
    d218:	[0-9a-f]* 	{ rotl r15, r16, r17 ; or r5, r6, r7 ; st r25, r26 }
    d220:	[0-9a-f]* 	{ rotl r15, r16, r17 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
    d228:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shrui r5, r6, 5 ; st1 r25, r26 }
    d230:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; st2 r25, r26 }
    d238:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotl r15, r16, r17 ; st4 r25, r26 }
    d240:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
    d248:	[0-9a-f]* 	{ rotl r15, r16, r17 ; subx r5, r6, r7 ; ld4u r25, r26 }
    d250:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotl r15, r16, r17 ; prefetch r25 }
    d258:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rotl r15, r16, r17 ; prefetch_l1_fault r25 }
    d260:	[0-9a-f]* 	{ rotl r15, r16, r17 ; v1mnz r5, r6, r7 }
    d268:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; rotl r15, r16, r17 }
    d270:	[0-9a-f]* 	{ rotl r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
    d278:	[0-9a-f]* 	{ rotl r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l3 r25 }
    d280:	[0-9a-f]* 	{ rotl r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    d288:	[0-9a-f]* 	{ rotl r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    d290:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    d298:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    d2a0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    d2a8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpne r15, r16, r17 }
    d2b0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; ill ; ld1u r25, r26 }
    d2b8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    d2c0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    d2c8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; and r15, r16, r17 ; ld r25, r26 }
    d2d0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
    d2d8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    d2e0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; nor r15, r16, r17 ; ld1u r25, r26 }
    d2e8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    d2f0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    d2f8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
    d300:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
    d308:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl r15, r16, r17 ; ld4u r25, r26 }
    d310:	[0-9a-f]* 	{ rotl r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
    d318:	[0-9a-f]* 	{ rotl r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    d320:	[0-9a-f]* 	{ rotl r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
    d328:	[0-9a-f]* 	{ rotl r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    d330:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    d338:	[0-9a-f]* 	{ rotl r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    d340:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    d348:	[0-9a-f]* 	{ rotl r5, r6, r7 ; nop ; prefetch_l1_fault r25 }
    d350:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    d358:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    d360:	[0-9a-f]* 	{ rotl r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    d368:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    d370:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    d378:	[0-9a-f]* 	{ rotl r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    d380:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    d388:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    d390:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    d398:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    d3a0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
    d3a8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
    d3b0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    d3b8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; nop ; st1 r25, r26 }
    d3c0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    d3c8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
    d3d0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; st_add r15, r16, 5 }
    d3d8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    d3e0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
    d3e8:	[0-9a-f]* 	{ rotl r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
    d3f0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; addi r5, r6, 5 ; ld1s r25, r26 }
    d3f8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; addxi r5, r6, 5 ; ld1u r25, r26 }
    d400:	[0-9a-f]* 	{ rotli r15, r16, 5 ; andi r5, r6, 5 ; ld1u r25, r26 }
    d408:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    d410:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    d418:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmples r5, r6, r7 ; ld4s r25, r26 }
    d420:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch r25 }
    d428:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    d430:	[0-9a-f]* 	{ ctz r5, r6 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    d438:	[0-9a-f]* 	{ rotli r15, r16, 5 ; prefetch_l2 r25 }
    d440:	[0-9a-f]* 	{ rotli r15, r16, 5 ; info 19 ; ld4u r25, r26 }
    d448:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; ld r25, r26 }
    d450:	[0-9a-f]* 	{ rotli r15, r16, 5 ; addxi r5, r6, 5 ; ld1s r25, r26 }
    d458:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl r5, r6, r7 ; ld1s r25, r26 }
    d460:	[0-9a-f]* 	{ rotli r15, r16, 5 ; info 19 ; ld1u r25, r26 }
    d468:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rotli r15, r16, 5 ; ld1u r25, r26 }
    d470:	[0-9a-f]* 	{ rotli r15, r16, 5 ; or r5, r6, r7 ; ld2s r25, r26 }
    d478:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld2u r25, r26 }
    d480:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrui r5, r6, 5 ; ld2u r25, r26 }
    d488:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; ld4s r25, r26 }
    d490:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; ld4u r25, r26 }
    d498:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl3add r5, r6, r7 ; ld4u r25, r26 }
    d4a0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; move r5, r6 ; ld4s r25, r26 }
    d4a8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; ld4u r25, r26 }
    d4b0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; ld2s r25, r26 }
    d4b8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; ld2u r25, r26 }
    d4c0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; ld1s r25, r26 }
    d4c8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotli r15, r16, 5 ; ld1u r25, r26 }
    d4d0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; mz r5, r6, r7 ; ld2u r25, r26 }
    d4d8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; nor r5, r6, r7 ; ld4u r25, r26 }
    d4e0:	[0-9a-f]* 	{ pcnt r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
    d4e8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    d4f0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    d4f8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch r25 }
    d500:	[0-9a-f]* 	{ rotli r15, r16, 5 ; move r5, r6 ; prefetch_l1_fault r25 }
    d508:	[0-9a-f]* 	{ rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    d510:	[0-9a-f]* 	{ revbits r5, r6 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    d518:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    d520:	[0-9a-f]* 	{ rotli r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l2_fault r25 }
    d528:	[0-9a-f]* 	{ mulx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3 r25 }
    d530:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    d538:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    d540:	[0-9a-f]* 	{ revbytes r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
    d548:	[0-9a-f]* 	{ rotli r15, r16, 5 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
    d550:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
    d558:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
    d560:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl3add r5, r6, r7 ; st1 r25, r26 }
    d568:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shli r5, r6, 5 ; st4 r25, r26 }
    d570:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    d578:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrux r5, r6, r7 }
    d580:	[0-9a-f]* 	{ rotli r15, r16, 5 ; or r5, r6, r7 ; st r25, r26 }
    d588:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
    d590:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shrui r5, r6, 5 ; st1 r25, r26 }
    d598:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
    d5a0:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; st4 r25, r26 }
    d5a8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shl3add r5, r6, r7 ; st4 r25, r26 }
    d5b0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; subx r5, r6, r7 ; ld4u r25, r26 }
    d5b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
    d5c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    d5c8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; v1mnz r5, r6, r7 }
    d5d0:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; rotli r15, r16, 5 }
    d5d8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
    d5e0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addi r15, r16, 5 ; prefetch_l3 r25 }
    d5e8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    d5f0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    d5f8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    d600:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    d608:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    d610:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpne r15, r16, r17 }
    d618:	[0-9a-f]* 	{ rotli r5, r6, 5 ; ill ; ld1u r25, r26 }
    d620:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalr r15 ; ld1s r25, r26 }
    d628:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jr r15 ; ld2s r25, r26 }
    d630:	[0-9a-f]* 	{ rotli r5, r6, 5 ; and r15, r16, r17 ; ld r25, r26 }
    d638:	[0-9a-f]* 	{ rotli r5, r6, 5 ; subx r15, r16, r17 ; ld r25, r26 }
    d640:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    d648:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nor r15, r16, r17 ; ld1u r25, r26 }
    d650:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalrp r15 ; ld2s r25, r26 }
    d658:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    d660:	[0-9a-f]* 	{ rotli r5, r6, 5 ; add r15, r16, r17 ; ld4s r25, r26 }
    d668:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
    d670:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl r15, r16, r17 ; ld4u r25, r26 }
    d678:	[0-9a-f]* 	{ rotli r5, r6, 5 ; lnk r15 ; ld4u r25, r26 }
    d680:	[0-9a-f]* 	{ rotli r5, r6, 5 ; move r15, r16 ; ld4u r25, r26 }
    d688:	[0-9a-f]* 	{ rotli r5, r6, 5 ; mz r15, r16, r17 ; ld4u r25, r26 }
    d690:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nor r15, r16, r17 ; prefetch r25 }
    d698:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmples r15, r16, r17 ; prefetch r25 }
    d6a0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; prefetch_add_l1_fault r15, 5 }
    d6a8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch r25 }
    d6b0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nop ; prefetch_l1_fault r25 }
    d6b8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalrp r15 ; prefetch_l2 r25 }
    d6c0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    d6c8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    d6d0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    d6d8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    d6e0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; rotli r15, r16, 5 ; prefetch r25 }
    d6e8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch r25 }
    d6f0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    d6f8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    d700:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shli r15, r16, 5 ; st r25, r26 }
    d708:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shrsi r15, r16, 5 ; st r25, r26 }
    d710:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shrui r15, r16, 5 ; st2 r25, r26 }
    d718:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; st r25, r26 }
    d720:	[0-9a-f]* 	{ rotli r5, r6, 5 ; nop ; st1 r25, r26 }
    d728:	[0-9a-f]* 	{ rotli r5, r6, 5 ; jalr r15 ; st2 r25, r26 }
    d730:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmples r15, r16, r17 ; st4 r25, r26 }
    d738:	[0-9a-f]* 	{ rotli r5, r6, 5 ; st_add r15, r16, 5 }
    d740:	[0-9a-f]* 	{ rotli r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    d748:	[0-9a-f]* 	{ rotli r5, r6, 5 ; v2cmpeqi r15, r16, 5 }
    d750:	[0-9a-f]* 	{ rotli r5, r6, 5 ; xor r15, r16, r17 ; ld r25, r26 }
    d758:	[0-9a-f]* 	{ shl r15, r16, r17 ; addi r5, r6, 5 ; ld1s r25, r26 }
    d760:	[0-9a-f]* 	{ shl r15, r16, r17 ; addxi r5, r6, 5 ; ld1u r25, r26 }
    d768:	[0-9a-f]* 	{ shl r15, r16, r17 ; andi r5, r6, 5 ; ld1u r25, r26 }
    d770:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    d778:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    d780:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmples r5, r6, r7 ; ld4s r25, r26 }
    d788:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch r25 }
    d790:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
    d798:	[0-9a-f]* 	{ ctz r5, r6 ; shl r15, r16, r17 ; ld1s r25, r26 }
    d7a0:	[0-9a-f]* 	{ shl r15, r16, r17 ; prefetch_l2 r25 }
    d7a8:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; ld4u r25, r26 }
    d7b0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; ld r25, r26 }
    d7b8:	[0-9a-f]* 	{ shl r15, r16, r17 ; addxi r5, r6, 5 ; ld1s r25, r26 }
    d7c0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl r5, r6, r7 ; ld1s r25, r26 }
    d7c8:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; ld1u r25, r26 }
    d7d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; ld1u r25, r26 }
    d7d8:	[0-9a-f]* 	{ shl r15, r16, r17 ; or r5, r6, r7 ; ld2s r25, r26 }
    d7e0:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2u r25, r26 }
    d7e8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shrui r5, r6, 5 ; ld2u r25, r26 }
    d7f0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; ld4s r25, r26 }
    d7f8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl r15, r16, r17 ; ld4u r25, r26 }
    d800:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl3add r5, r6, r7 ; ld4u r25, r26 }
    d808:	[0-9a-f]* 	{ shl r15, r16, r17 ; move r5, r6 ; ld4s r25, r26 }
    d810:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl r15, r16, r17 ; ld4u r25, r26 }
    d818:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
    d820:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl r15, r16, r17 ; ld2u r25, r26 }
    d828:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
    d830:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    d838:	[0-9a-f]* 	{ shl r15, r16, r17 ; mz r5, r6, r7 ; ld2u r25, r26 }
    d840:	[0-9a-f]* 	{ shl r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
    d848:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
    d850:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    d858:	[0-9a-f]* 	{ shl r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    d860:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
    d868:	[0-9a-f]* 	{ shl r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    d870:	[0-9a-f]* 	{ shl r15, r16, r17 ; prefetch_l1_fault r25 }
    d878:	[0-9a-f]* 	{ revbits r5, r6 ; shl r15, r16, r17 ; prefetch_l2 r25 }
    d880:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
    d888:	[0-9a-f]* 	{ shl r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2_fault r25 }
    d890:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l3 r25 }
    d898:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 }
    d8a0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; prefetch_l3_fault r25 }
    d8a8:	[0-9a-f]* 	{ revbytes r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
    d8b0:	[0-9a-f]* 	{ shl r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
    d8b8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
    d8c0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
    d8c8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl3add r5, r6, r7 ; st1 r25, r26 }
    d8d0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; st4 r25, r26 }
    d8d8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    d8e0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shrux r5, r6, r7 }
    d8e8:	[0-9a-f]* 	{ shl r15, r16, r17 ; or r5, r6, r7 ; st r25, r26 }
    d8f0:	[0-9a-f]* 	{ shl r15, r16, r17 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
    d8f8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shrui r5, r6, 5 ; st1 r25, r26 }
    d900:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
    d908:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl r15, r16, r17 ; st4 r25, r26 }
    d910:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
    d918:	[0-9a-f]* 	{ shl r15, r16, r17 ; subx r5, r6, r7 ; ld4u r25, r26 }
    d920:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
    d928:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; prefetch_l1_fault r25 }
    d930:	[0-9a-f]* 	{ shl r15, r16, r17 ; v1mnz r5, r6, r7 }
    d938:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; shl r15, r16, r17 }
    d940:	[0-9a-f]* 	{ shl r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
    d948:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l3 r25 }
    d950:	[0-9a-f]* 	{ shl r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
    d958:	[0-9a-f]* 	{ shl r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
    d960:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
    d968:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
    d970:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
    d978:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpne r15, r16, r17 }
    d980:	[0-9a-f]* 	{ shl r5, r6, r7 ; ill ; ld1u r25, r26 }
    d988:	[0-9a-f]* 	{ shl r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
    d990:	[0-9a-f]* 	{ shl r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
    d998:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 ; ld r25, r26 }
    d9a0:	[0-9a-f]* 	{ shl r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
    d9a8:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    d9b0:	[0-9a-f]* 	{ shl r5, r6, r7 ; nor r15, r16, r17 ; ld1u r25, r26 }
    d9b8:	[0-9a-f]* 	{ shl r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
    d9c0:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2u r25, r26 }
    d9c8:	[0-9a-f]* 	{ shl r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
    d9d0:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
    d9d8:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl r15, r16, r17 ; ld4u r25, r26 }
    d9e0:	[0-9a-f]* 	{ shl r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
    d9e8:	[0-9a-f]* 	{ shl r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
    d9f0:	[0-9a-f]* 	{ shl r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
    d9f8:	[0-9a-f]* 	{ shl r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    da00:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
    da08:	[0-9a-f]* 	{ shl r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
    da10:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    da18:	[0-9a-f]* 	{ shl r5, r6, r7 ; nop ; prefetch_l1_fault r25 }
    da20:	[0-9a-f]* 	{ shl r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
    da28:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
    da30:	[0-9a-f]* 	{ shl r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
    da38:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
    da40:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    da48:	[0-9a-f]* 	{ shl r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
    da50:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    da58:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    da60:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    da68:	[0-9a-f]* 	{ shl r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    da70:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
    da78:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
    da80:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    da88:	[0-9a-f]* 	{ shl r5, r6, r7 ; nop ; st1 r25, r26 }
    da90:	[0-9a-f]* 	{ shl r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
    da98:	[0-9a-f]* 	{ shl r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
    daa0:	[0-9a-f]* 	{ shl r5, r6, r7 ; st_add r15, r16, 5 }
    daa8:	[0-9a-f]* 	{ shl r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
    dab0:	[0-9a-f]* 	{ shl r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
    dab8:	[0-9a-f]* 	{ shl r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
    dac0:	[0-9a-f]* 	{ shl16insli r15, r16, 4660 ; cmpltsi r5, r6, 5 }
    dac8:	[0-9a-f]* 	{ shl16insli r15, r16, 4660 ; moveli r5, 4660 }
    dad0:	[0-9a-f]* 	{ shl16insli r15, r16, 4660 ; shl3addx r5, r6, r7 }
    dad8:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; shl16insli r15, r16, 4660 }
    dae0:	[0-9a-f]* 	{ shl16insli r15, r16, 4660 ; v2int_l r5, r6, r7 }
    dae8:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; addi r15, r16, 5 }
    daf0:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; infol 4660 }
    daf8:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; mnz r15, r16, r17 }
    db00:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; shrui r15, r16, 5 }
    db08:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; v1mnz r15, r16, r17 }
    db10:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; v2sub r15, r16, r17 }
    db18:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    db20:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    db28:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    db30:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    db38:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    db40:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    db48:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    db50:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    db58:	[0-9a-f]* 	{ ctz r5, r6 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    db60:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; st r25, r26 }
    db68:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    db70:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; ld r25, r26 }
    db78:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1add r15, r16, r17 ; ld1s r25, r26 }
    db80:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    db88:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; ld1u r25, r26 }
    db90:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    db98:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    dba0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; ld2u r25, r26 }
    dba8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; ld2u r25, r26 }
    dbb0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; nop ; ld4s r25, r26 }
    dbb8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    dbc0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    dbc8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    dbd0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
    dbd8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    dbe0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    dbe8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
    dbf0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1add r15, r16, r17 ; ld4u r25, r26 }
    dbf8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    dc00:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    dc08:	[0-9a-f]* 	{ pcnt r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    dc10:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch r25 }
    dc18:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    dc20:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    dc28:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1_fault r25 }
    dc30:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    dc38:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    dc40:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    dc48:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    dc50:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    dc58:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    dc60:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    dc68:	[0-9a-f]* 	{ revbytes r5, r6 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    dc70:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    dc78:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    dc80:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    dc88:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    dc90:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    dc98:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    dca0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    dca8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    dcb0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; st1 r25, r26 }
    dcb8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; st1 r25, r26 }
    dcc0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; nop ; st2 r25, r26 }
    dcc8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    dcd0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    dcd8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    dce0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; prefetch_l2_fault r25 }
    dce8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1add r15, r16, r17 ; prefetch_l3_fault r25 }
    dcf0:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; v1mz r5, r6, r7 }
    dcf8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; v2packuc r5, r6, r7 }
    dd00:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    dd08:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    dd10:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    dd18:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    dd20:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpexch r15, r16, r17 }
    dd28:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    dd30:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    dd38:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; dtlbpr r15 }
    dd40:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; ill ; ld4u r25, r26 }
    dd48:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    dd50:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; jr r15 ; prefetch r25 }
    dd58:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    dd60:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    dd68:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    dd70:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    dd78:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    dd80:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    dd88:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    dd90:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    dd98:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    dda0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    dda8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    ddb0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    ddb8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    ddc0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    ddc8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    ddd0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    ddd8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    dde0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    dde8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
    ddf0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    ddf8:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; prefetch_l3 r25 }
    de00:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    de08:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    de10:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    de18:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    de20:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    de28:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shli r15, r16, 5 }
    de30:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shrsi r15, r16, 5 }
    de38:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shruxi r15, r16, 5 }
    de40:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    de48:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    de50:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    de58:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    de60:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; stnt2 r15, r16 }
    de68:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    de70:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    de78:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    de80:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    de88:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    de90:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    de98:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4s r25, r26 }
    dea0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    dea8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    deb0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    deb8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    dec0:	[0-9a-f]* 	{ ctz r5, r6 ; shl1addx r15, r16, r17 ; ld4s r25, r26 }
    dec8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; st r25, r26 }
    ded0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    ded8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
    dee0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    dee8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    def0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
    def8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    df00:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    df08:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; ld2u r25, r26 }
    df10:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
    df18:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; nop ; ld4s r25, r26 }
    df20:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    df28:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    df30:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    df38:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
    df40:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    df48:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    df50:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4s r25, r26 }
    df58:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; ld4u r25, r26 }
    df60:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    df68:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    df70:	[0-9a-f]* 	{ pcnt r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    df78:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
    df80:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    df88:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    df90:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
    df98:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    dfa0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    dfa8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    dfb0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    dfb8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    dfc0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    dfc8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    dfd0:	[0-9a-f]* 	{ revbytes r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l3 r25 }
    dfd8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    dfe0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    dfe8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    dff0:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    dff8:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    e000:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    e008:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    e010:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    e018:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; st1 r25, r26 }
    e020:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; st1 r25, r26 }
    e028:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; nop ; st2 r25, r26 }
    e030:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    e038:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    e040:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    e048:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
    e050:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
    e058:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; v1mz r5, r6, r7 }
    e060:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; v2packuc r5, r6, r7 }
    e068:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    e070:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    e078:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    e080:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    e088:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpexch r15, r16, r17 }
    e090:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    e098:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    e0a0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; dtlbpr r15 }
    e0a8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; ill ; ld4u r25, r26 }
    e0b0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    e0b8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jr r15 ; prefetch r25 }
    e0c0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    e0c8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    e0d0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    e0d8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    e0e0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    e0e8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    e0f0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    e0f8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    e100:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    e108:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    e110:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    e118:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    e120:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    e128:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    e130:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    e138:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    e140:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    e148:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    e150:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
    e158:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    e160:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    e168:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    e170:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    e178:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    e180:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    e188:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    e190:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shli r15, r16, 5 }
    e198:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shrsi r15, r16, 5 }
    e1a0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shruxi r15, r16, 5 }
    e1a8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    e1b0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    e1b8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    e1c0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    e1c8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; stnt2 r15, r16 }
    e1d0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    e1d8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    e1e0:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    e1e8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    e1f0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    e1f8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    e200:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2add r15, r16, r17 ; ld4s r25, r26 }
    e208:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e210:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    e218:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    e220:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    e228:	[0-9a-f]* 	{ ctz r5, r6 ; shl2add r15, r16, r17 ; ld4s r25, r26 }
    e230:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; st r25, r26 }
    e238:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    e240:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl2add r15, r16, r17 ; ld r25, r26 }
    e248:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
    e250:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    e258:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; ld1u r25, r26 }
    e260:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    e268:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    e270:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; ld2u r25, r26 }
    e278:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2add r15, r16, r17 ; ld2u r25, r26 }
    e280:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; nop ; ld4s r25, r26 }
    e288:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    e290:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    e298:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    e2a0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
    e2a8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    e2b0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    e2b8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl2add r15, r16, r17 ; ld4s r25, r26 }
    e2c0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
    e2c8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    e2d0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    e2d8:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    e2e0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch r25 }
    e2e8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e2f0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    e2f8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l1_fault r25 }
    e300:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    e308:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    e310:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    e318:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    e320:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    e328:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    e330:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    e338:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
    e340:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    e348:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    e350:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    e358:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    e360:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    e368:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    e370:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    e378:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    e380:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; st1 r25, r26 }
    e388:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2add r15, r16, r17 ; st1 r25, r26 }
    e390:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; nop ; st2 r25, r26 }
    e398:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    e3a0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    e3a8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    e3b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2add r15, r16, r17 ; prefetch_l2_fault r25 }
    e3b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2add r15, r16, r17 ; prefetch_l3_fault r25 }
    e3c0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; v1mz r5, r6, r7 }
    e3c8:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; v2packuc r5, r6, r7 }
    e3d0:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    e3d8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    e3e0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    e3e8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    e3f0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpexch r15, r16, r17 }
    e3f8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    e400:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    e408:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; dtlbpr r15 }
    e410:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; ill ; ld4u r25, r26 }
    e418:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    e420:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; jr r15 ; prefetch r25 }
    e428:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    e430:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    e438:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    e440:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    e448:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    e450:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    e458:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    e460:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    e468:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    e470:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    e478:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    e480:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    e488:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    e490:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    e498:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    e4a0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    e4a8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    e4b0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    e4b8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch_l2_fault r25 }
    e4c0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    e4c8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; prefetch_l3 r25 }
    e4d0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    e4d8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    e4e0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    e4e8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    e4f0:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    e4f8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shli r15, r16, 5 }
    e500:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shrsi r15, r16, 5 }
    e508:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shruxi r15, r16, 5 }
    e510:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    e518:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    e520:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    e528:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    e530:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; stnt2 r15, r16 }
    e538:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    e540:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    e548:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    e550:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    e558:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    e560:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    e568:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    e570:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e578:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    e580:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    e588:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    e590:	[0-9a-f]* 	{ ctz r5, r6 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    e598:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; st r25, r26 }
    e5a0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    e5a8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
    e5b0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1s r25, r26 }
    e5b8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    e5c0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
    e5c8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    e5d0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    e5d8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; ld2u r25, r26 }
    e5e0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
    e5e8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; nop ; ld4s r25, r26 }
    e5f0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    e5f8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    e600:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    e608:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
    e610:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch r25 }
    e618:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch r25 }
    e620:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
    e628:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    e630:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    e638:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    e640:	[0-9a-f]* 	{ pcnt r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    e648:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch r25 }
    e650:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e658:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    e660:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l1_fault r25 }
    e668:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    e670:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    e678:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    e680:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    e688:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    e690:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    e698:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    e6a0:	[0-9a-f]* 	{ revbytes r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
    e6a8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    e6b0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    e6b8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    e6c0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    e6c8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    e6d0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    e6d8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    e6e0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    e6e8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; st1 r25, r26 }
    e6f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
    e6f8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; nop ; st2 r25, r26 }
    e700:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    e708:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    e710:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    e718:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
    e720:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
    e728:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; v1mz r5, r6, r7 }
    e730:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; v2packuc r5, r6, r7 }
    e738:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    e740:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    e748:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    e750:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    e758:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpexch r15, r16, r17 }
    e760:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    e768:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    e770:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; dtlbpr r15 }
    e778:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; ill ; ld4u r25, r26 }
    e780:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    e788:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jr r15 ; prefetch r25 }
    e790:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    e798:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    e7a0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    e7a8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    e7b0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    e7b8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    e7c0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    e7c8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    e7d0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    e7d8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    e7e0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    e7e8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    e7f0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    e7f8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    e800:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    e808:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    e810:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    e818:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    e820:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; prefetch_l2_fault r25 }
    e828:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    e830:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; prefetch_l3 r25 }
    e838:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    e840:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    e848:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    e850:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    e858:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    e860:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shli r15, r16, 5 }
    e868:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shrsi r15, r16, 5 }
    e870:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shruxi r15, r16, 5 }
    e878:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    e880:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    e888:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    e890:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    e898:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; stnt2 r15, r16 }
    e8a0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    e8a8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    e8b0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    e8b8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    e8c0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    e8c8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    e8d0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3add r15, r16, r17 ; ld4s r25, r26 }
    e8d8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e8e0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    e8e8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    e8f0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    e8f8:	[0-9a-f]* 	{ ctz r5, r6 ; shl3add r15, r16, r17 ; ld4s r25, r26 }
    e900:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; st r25, r26 }
    e908:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    e910:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; ld r25, r26 }
    e918:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3add r15, r16, r17 ; ld1s r25, r26 }
    e920:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    e928:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
    e930:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    e938:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    e940:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; ld2u r25, r26 }
    e948:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; ld2u r25, r26 }
    e950:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; nop ; ld4s r25, r26 }
    e958:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    e960:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    e968:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    e970:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l2 r25 }
    e978:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    e980:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    e988:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; ld4s r25, r26 }
    e990:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3add r15, r16, r17 ; ld4u r25, r26 }
    e998:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    e9a0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    e9a8:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    e9b0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
    e9b8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    e9c0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    e9c8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l1_fault r25 }
    e9d0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    e9d8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    e9e0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    e9e8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    e9f0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    e9f8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    ea00:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    ea08:	[0-9a-f]* 	{ revbytes r5, r6 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
    ea10:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    ea18:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    ea20:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    ea28:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    ea30:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    ea38:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    ea40:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    ea48:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    ea50:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; st1 r25, r26 }
    ea58:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; st1 r25, r26 }
    ea60:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; nop ; st2 r25, r26 }
    ea68:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    ea70:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    ea78:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    ea80:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
    ea88:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l3_fault r25 }
    ea90:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; v1mz r5, r6, r7 }
    ea98:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; v2packuc r5, r6, r7 }
    eaa0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    eaa8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    eab0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    eab8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    eac0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpexch r15, r16, r17 }
    eac8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    ead0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    ead8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; dtlbpr r15 }
    eae0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; ill ; ld4u r25, r26 }
    eae8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    eaf0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; jr r15 ; prefetch r25 }
    eaf8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    eb00:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    eb08:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    eb10:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    eb18:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    eb20:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    eb28:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    eb30:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    eb38:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    eb40:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    eb48:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    eb50:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    eb58:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    eb60:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    eb68:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    eb70:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    eb78:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    eb80:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    eb88:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; prefetch_l2_fault r25 }
    eb90:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    eb98:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; prefetch_l3 r25 }
    eba0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    eba8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    ebb0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    ebb8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    ebc0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    ebc8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shli r15, r16, 5 }
    ebd0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shrsi r15, r16, 5 }
    ebd8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shruxi r15, r16, 5 }
    ebe0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    ebe8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    ebf0:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    ebf8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    ec00:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; stnt2 r15, r16 }
    ec08:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    ec10:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    ec18:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    ec20:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
    ec28:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    ec30:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
    ec38:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    ec40:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    ec48:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    ec50:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    ec58:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    ec60:	[0-9a-f]* 	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    ec68:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; st r25, r26 }
    ec70:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
    ec78:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; ld r25, r26 }
    ec80:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
    ec88:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    ec90:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
    ec98:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
    eca0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    eca8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; ld2u r25, r26 }
    ecb0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
    ecb8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; nop ; ld4s r25, r26 }
    ecc0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    ecc8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    ecd0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
    ecd8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
    ece0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    ece8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    ecf0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    ecf8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
    ed00:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    ed08:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    ed10:	[0-9a-f]* 	{ pcnt r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2_fault r25 }
    ed18:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
    ed20:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
    ed28:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    ed30:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1_fault r25 }
    ed38:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    ed40:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    ed48:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
    ed50:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2_fault r25 }
    ed58:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
    ed60:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    ed68:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    ed70:	[0-9a-f]* 	{ revbytes r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
    ed78:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
    ed80:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    ed88:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    ed90:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    ed98:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
    eda0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
    eda8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
    edb0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
    edb8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; st1 r25, r26 }
    edc0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
    edc8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; nop ; st2 r25, r26 }
    edd0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    edd8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    ede0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    ede8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l2_fault r25 }
    edf0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
    edf8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; v1mz r5, r6, r7 }
    ee00:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; v2packuc r5, r6, r7 }
    ee08:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
    ee10:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
    ee18:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
    ee20:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
    ee28:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpexch r15, r16, r17 }
    ee30:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
    ee38:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    ee40:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; dtlbpr r15 }
    ee48:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; ill ; ld4u r25, r26 }
    ee50:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
    ee58:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jr r15 ; prefetch r25 }
    ee60:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
    ee68:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
    ee70:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    ee78:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
    ee80:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    ee88:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    ee90:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
    ee98:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
    eea0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    eea8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
    eeb0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
    eeb8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    eec0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    eec8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
    eed0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
    eed8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    eee0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    eee8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    eef0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
    eef8:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    ef00:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; prefetch_l3 r25 }
    ef08:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    ef10:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    ef18:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    ef20:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
    ef28:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    ef30:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shli r15, r16, 5 }
    ef38:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shrsi r15, r16, 5 }
    ef40:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shruxi r15, r16, 5 }
    ef48:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
    ef50:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
    ef58:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
    ef60:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    ef68:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; stnt2 r15, r16 }
    ef70:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
    ef78:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
    ef80:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
    ef88:	[0-9a-f]* 	{ shli r15, r16, 5 ; addi r5, r6, 5 ; ld4s r25, r26 }
    ef90:	[0-9a-f]* 	{ shli r15, r16, 5 ; addxi r5, r6, 5 ; ld4u r25, r26 }
    ef98:	[0-9a-f]* 	{ shli r15, r16, 5 ; andi r5, r6, 5 ; ld4u r25, r26 }
    efa0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; ld4s r25, r26 }
    efa8:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch r25 }
    efb0:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
    efb8:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
    efc0:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
    efc8:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 ; ld4s r25, r26 }
    efd0:	[0-9a-f]* 	{ shli r15, r16, 5 ; st r25, r26 }
    efd8:	[0-9a-f]* 	{ shli r15, r16, 5 ; info 19 ; prefetch_l2 r25 }
    efe0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; ld r25, r26 }
    efe8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; ld1s r25, r26 }
    eff0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
    eff8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
    f000:	[0-9a-f]* 	{ shli r15, r16, 5 ; addi r5, r6, 5 ; ld2s r25, r26 }
    f008:	[0-9a-f]* 	{ shli r15, r16, 5 ; rotl r5, r6, r7 ; ld2s r25, r26 }
    f010:	[0-9a-f]* 	{ shli r15, r16, 5 ; ld2u r25, r26 }
    f018:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; ld2u r25, r26 }
    f020:	[0-9a-f]* 	{ shli r15, r16, 5 ; nop ; ld4s r25, r26 }
    f028:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
    f030:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
    f038:	[0-9a-f]* 	{ shli r15, r16, 5 ; move r5, r6 ; prefetch_l1_fault r25 }
    f040:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2 r25 }
    f048:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    f050:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    f058:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; ld4s r25, r26 }
    f060:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shli r15, r16, 5 ; ld4u r25, r26 }
    f068:	[0-9a-f]* 	{ shli r15, r16, 5 ; mz r5, r6, r7 ; prefetch r25 }
    f070:	[0-9a-f]* 	{ shli r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l2 r25 }
    f078:	[0-9a-f]* 	{ pcnt r5, r6 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
    f080:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    f088:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch r25 }
    f090:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch r25 }
    f098:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
    f0a0:	[0-9a-f]* 	{ shli r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
    f0a8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl r5, r6, r7 ; prefetch_l2 r25 }
    f0b0:	[0-9a-f]* 	{ shli r15, r16, 5 ; info 19 ; prefetch_l2_fault r25 }
    f0b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
    f0c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; or r5, r6, r7 ; prefetch_l3 r25 }
    f0c8:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
    f0d0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
    f0d8:	[0-9a-f]* 	{ revbytes r5, r6 ; shli r15, r16, 5 ; prefetch_l3 r25 }
    f0e0:	[0-9a-f]* 	{ shli r15, r16, 5 ; rotli r5, r6, 5 ; st r25, r26 }
    f0e8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl1add r5, r6, r7 ; st1 r25, r26 }
    f0f0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl2add r5, r6, r7 ; st4 r25, r26 }
    f0f8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl3addx r5, r6, r7 ; ld r25, r26 }
    f100:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrs r5, r6, r7 ; ld r25, r26 }
    f108:	[0-9a-f]* 	{ shli r15, r16, 5 ; shru r5, r6, r7 ; ld1u r25, r26 }
    f110:	[0-9a-f]* 	{ shli r15, r16, 5 ; addi r5, r6, 5 ; st r25, r26 }
    f118:	[0-9a-f]* 	{ shli r15, r16, 5 ; rotl r5, r6, r7 ; st r25, r26 }
    f120:	[0-9a-f]* 	{ shli r15, r16, 5 ; st1 r25, r26 }
    f128:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; st1 r25, r26 }
    f130:	[0-9a-f]* 	{ shli r15, r16, 5 ; nop ; st2 r25, r26 }
    f138:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
    f140:	[0-9a-f]* 	{ shli r15, r16, 5 ; shrsi r5, r6, 5 ; st4 r25, r26 }
    f148:	[0-9a-f]* 	{ shli r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l2 r25 }
    f150:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
    f158:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    f160:	[0-9a-f]* 	{ shli r15, r16, 5 ; v1mz r5, r6, r7 }
    f168:	[0-9a-f]* 	{ shli r15, r16, 5 ; v2packuc r5, r6, r7 }
    f170:	[0-9a-f]* 	{ shli r15, r16, 5 ; xor r5, r6, r7 ; st1 r25, r26 }
    f178:	[0-9a-f]* 	{ shli r5, r6, 5 ; addi r15, r16, 5 ; st2 r25, r26 }
    f180:	[0-9a-f]* 	{ shli r5, r6, 5 ; addxi r15, r16, 5 ; st4 r25, r26 }
    f188:	[0-9a-f]* 	{ shli r5, r6, 5 ; andi r15, r16, 5 ; st4 r25, r26 }
    f190:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpexch r15, r16, r17 }
    f198:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmplts r15, r16, r17 ; ld r25, r26 }
    f1a0:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
    f1a8:	[0-9a-f]* 	{ shli r5, r6, 5 ; dtlbpr r15 }
    f1b0:	[0-9a-f]* 	{ shli r5, r6, 5 ; ill ; ld4u r25, r26 }
    f1b8:	[0-9a-f]* 	{ shli r5, r6, 5 ; jalr r15 ; ld4s r25, r26 }
    f1c0:	[0-9a-f]* 	{ shli r5, r6, 5 ; jr r15 ; prefetch r25 }
    f1c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmples r15, r16, r17 ; ld r25, r26 }
    f1d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; add r15, r16, r17 ; ld1s r25, r26 }
    f1d8:	[0-9a-f]* 	{ shli r5, r6, 5 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
    f1e0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; ld1u r25, r26 }
    f1e8:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 ; ld2s r25, r26 }
    f1f0:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
    f1f8:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 ; ld4s r25, r26 }
    f200:	[0-9a-f]* 	{ shli r5, r6, 5 ; subx r15, r16, r17 ; ld4s r25, r26 }
    f208:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
    f210:	[0-9a-f]* 	{ shli r5, r6, 5 ; lnk r15 ; prefetch_l2 r25 }
    f218:	[0-9a-f]* 	{ shli r5, r6, 5 ; move r15, r16 ; prefetch_l2 r25 }
    f220:	[0-9a-f]* 	{ shli r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l2 r25 }
    f228:	[0-9a-f]* 	{ shli r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l3 r25 }
    f230:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch r25 }
    f238:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch_add_l3_fault r15, 5 }
    f240:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; prefetch r25 }
    f248:	[0-9a-f]* 	{ shli r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
    f250:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
    f258:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch_l2_fault r25 }
    f260:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
    f268:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch_l3 r25 }
    f270:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
    f278:	[0-9a-f]* 	{ shli r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
    f280:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
    f288:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl2add r15, r16, r17 ; st r25, r26 }
    f290:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl3add r15, r16, r17 ; st2 r25, r26 }
    f298:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 }
    f2a0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shrsi r15, r16, 5 }
    f2a8:	[0-9a-f]* 	{ shli r5, r6, 5 ; shruxi r15, r16, 5 }
    f2b0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; st r25, r26 }
    f2b8:	[0-9a-f]* 	{ shli r5, r6, 5 ; rotli r15, r16, 5 ; st1 r25, r26 }
    f2c0:	[0-9a-f]* 	{ shli r5, r6, 5 ; lnk r15 ; st2 r25, r26 }
    f2c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
    f2d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; stnt2 r15, r16 }
    f2d8:	[0-9a-f]* 	{ shli r5, r6, 5 ; subx r15, r16, r17 ; st2 r25, r26 }
    f2e0:	[0-9a-f]* 	{ shli r5, r6, 5 ; v2cmpltsi r15, r16, 5 }
    f2e8:	[0-9a-f]* 	{ shli r5, r6, 5 ; xor r15, r16, r17 ; ld2u r25, r26 }
    f2f0:	[0-9a-f]* 	{ cmul r5, r6, r7 ; shlx r15, r16, r17 }
    f2f8:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; shlx r15, r16, r17 }
    f300:	[0-9a-f]* 	{ shlx r15, r16, r17 ; shrs r5, r6, r7 }
    f308:	[0-9a-f]* 	{ shlx r15, r16, r17 ; v1maxu r5, r6, r7 }
    f310:	[0-9a-f]* 	{ shlx r15, r16, r17 ; v2minsi r5, r6, 5 }
    f318:	[0-9a-f]* 	{ shlx r5, r6, r7 ; addxli r15, r16, 4660 }
    f320:	[0-9a-f]* 	{ shlx r5, r6, r7 ; jalrp r15 }
    f328:	[0-9a-f]* 	{ shlx r5, r6, r7 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
    f330:	[0-9a-f]* 	{ shlx r5, r6, r7 ; st1 r15, r16 }
    f338:	[0-9a-f]* 	{ shlx r5, r6, r7 ; v1shrs r15, r16, r17 }
    f340:	[0-9a-f]* 	{ shlx r5, r6, r7 ; v4int_h r15, r16, r17 }
    f348:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; shlxi r15, r16, 5 }
    f350:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shlxi r15, r16, 5 }
    f358:	[0-9a-f]* 	{ shlxi r15, r16, 5 ; shrux r5, r6, r7 }
    f360:	[0-9a-f]* 	{ shlxi r15, r16, 5 ; v1mnz r5, r6, r7 }
    f368:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; shlxi r15, r16, 5 }
    f370:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; cmpeq r15, r16, r17 }
    f378:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; ld1s r15, r16 }
    f380:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; or r15, r16, r17 }
    f388:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; st4 r15, r16 }
    f390:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; v1sub r15, r16, r17 }
    f398:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; v4shlsc r15, r16, r17 }
    f3a0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
    f3a8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
    f3b0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; andi r5, r6, 5 ; st r25, r26 }
    f3b8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    f3c0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
    f3c8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmples r5, r6, r7 ; st4 r25, r26 }
    f3d0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
    f3d8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    f3e0:	[0-9a-f]* 	{ ctz r5, r6 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    f3e8:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; shrs r15, r16, r17 }
    f3f0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; info 19 }
    f3f8:	[0-9a-f]* 	{ pcnt r5, r6 ; shrs r15, r16, r17 ; ld r25, r26 }
    f400:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpltu r5, r6, r7 ; ld1s r25, r26 }
    f408:	[0-9a-f]* 	{ shrs r15, r16, r17 ; sub r5, r6, r7 ; ld1s r25, r26 }
    f410:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    f418:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    f420:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl3addx r5, r6, r7 ; ld2s r25, r26 }
    f428:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
    f430:	[0-9a-f]* 	{ shrs r15, r16, r17 ; addxi r5, r6, 5 ; ld4s r25, r26 }
    f438:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl r5, r6, r7 ; ld4s r25, r26 }
    f440:	[0-9a-f]* 	{ shrs r15, r16, r17 ; info 19 ; ld4u r25, r26 }
    f448:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrs r15, r16, r17 ; ld4u r25, r26 }
    f450:	[0-9a-f]* 	{ shrs r15, r16, r17 ; move r5, r6 ; st4 r25, r26 }
    f458:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrs r15, r16, r17 }
    f460:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
    f468:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
    f470:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    f478:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
    f480:	[0-9a-f]* 	{ shrs r15, r16, r17 ; mz r5, r6, r7 ; st2 r25, r26 }
    f488:	[0-9a-f]* 	{ shrs r15, r16, r17 ; nor r5, r6, r7 }
    f490:	[0-9a-f]* 	{ shrs r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
    f498:	[0-9a-f]* 	{ revbytes r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
    f4a0:	[0-9a-f]* 	{ ctz r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
    f4a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
    f4b0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1_fault r25 }
    f4b8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l2 r25 }
    f4c0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    f4c8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l2_fault r25 }
    f4d0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3 r25 }
    f4d8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    f4e0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; move r5, r6 ; prefetch_l3_fault r25 }
    f4e8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; prefetch_l3_fault r25 }
    f4f0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; rotl r5, r6, r7 ; ld1s r25, r26 }
    f4f8:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl r5, r6, r7 ; ld2s r25, r26 }
    f500:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
    f508:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    f510:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    f518:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    f520:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    f528:	[0-9a-f]* 	{ shrs r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    f530:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl3addx r5, r6, r7 ; st r25, r26 }
    f538:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
    f540:	[0-9a-f]* 	{ shrs r15, r16, r17 ; addxi r5, r6, 5 ; st2 r25, r26 }
    f548:	[0-9a-f]* 	{ shrs r15, r16, r17 ; shl r5, r6, r7 ; st2 r25, r26 }
    f550:	[0-9a-f]* 	{ shrs r15, r16, r17 ; info 19 ; st4 r25, r26 }
    f558:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrs r15, r16, r17 ; st4 r25, r26 }
    f560:	[0-9a-f]* 	{ shrs r15, r16, r17 ; subx r5, r6, r7 }
    f568:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrs r15, r16, r17 ; ld r25, r26 }
    f570:	[0-9a-f]* 	{ shrs r15, r16, r17 ; v1adduc r5, r6, r7 }
    f578:	[0-9a-f]* 	{ shrs r15, r16, r17 ; v1shrui r5, r6, 5 }
    f580:	[0-9a-f]* 	{ shrs r15, r16, r17 ; v2shrs r5, r6, r7 }
    f588:	[0-9a-f]* 	{ shrs r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
    f590:	[0-9a-f]* 	{ shrs r5, r6, r7 ; addx r15, r16, r17 ; ld2u r25, r26 }
    f598:	[0-9a-f]* 	{ shrs r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
    f5a0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    f5a8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    f5b0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    f5b8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    f5c0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; fetchand4 r15, r16, r17 }
    f5c8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; ill ; st r25, r26 }
    f5d0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    f5d8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; st1 r25, r26 }
    f5e0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; info 19 ; ld r25, r26 }
    f5e8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmples r15, r16, r17 ; ld1s r25, r26 }
    f5f0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; ld1u r15, r16 }
    f5f8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    f600:	[0-9a-f]* 	{ shrs r5, r6, r7 ; rotli r15, r16, 5 ; ld2s r25, r26 }
    f608:	[0-9a-f]* 	{ shrs r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    f610:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
    f618:	[0-9a-f]* 	{ shrs r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    f620:	[0-9a-f]* 	{ shrs r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    f628:	[0-9a-f]* 	{ shrs r5, r6, r7 ; lnk r15 }
    f630:	[0-9a-f]* 	{ shrs r5, r6, r7 ; move r15, r16 }
    f638:	[0-9a-f]* 	{ shrs r5, r6, r7 ; mz r15, r16, r17 }
    f640:	[0-9a-f]* 	{ shrs r5, r6, r7 ; or r15, r16, r17 ; ld1s r25, r26 }
    f648:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jrp r15 ; prefetch r25 }
    f650:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    f658:	[0-9a-f]* 	{ shrs r5, r6, r7 ; prefetch r25 }
    f660:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
    f668:	[0-9a-f]* 	{ shrs r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    f670:	[0-9a-f]* 	{ shrs r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
    f678:	[0-9a-f]* 	{ shrs r5, r6, r7 ; prefetch_l3 r25 }
    f680:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    f688:	[0-9a-f]* 	{ shrs r5, r6, r7 ; prefetch_l3_fault r25 }
    f690:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl r15, r16, r17 ; ld r25, r26 }
    f698:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    f6a0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
    f6a8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    f6b0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
    f6b8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    f6c0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; cmpeq r15, r16, r17 ; st r25, r26 }
    f6c8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; st r25, r26 }
    f6d0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shli r15, r16, 5 ; st1 r25, r26 }
    f6d8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; rotl r15, r16, r17 ; st2 r25, r26 }
    f6e0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jrp r15 ; st4 r25, r26 }
    f6e8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; sub r15, r16, r17 ; ld2s r25, r26 }
    f6f0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
    f6f8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; v2mins r15, r16, r17 }
    f700:	[0-9a-f]* 	{ shrs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    f708:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
    f710:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; addxi r5, r6, 5 ; st r25, r26 }
    f718:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; andi r5, r6, 5 ; st r25, r26 }
    f720:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    f728:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
    f730:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmples r5, r6, r7 ; st4 r25, r26 }
    f738:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
    f740:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    f748:	[0-9a-f]* 	{ ctz r5, r6 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    f750:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; shrsi r15, r16, 5 }
    f758:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; info 19 }
    f760:	[0-9a-f]* 	{ pcnt r5, r6 ; shrsi r15, r16, 5 ; ld r25, r26 }
    f768:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpltu r5, r6, r7 ; ld1s r25, r26 }
    f770:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; sub r5, r6, r7 ; ld1s r25, r26 }
    f778:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
    f780:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    f788:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld2s r25, r26 }
    f790:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; ld2u r25, r26 }
    f798:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; addxi r5, r6, 5 ; ld4s r25, r26 }
    f7a0:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl r5, r6, r7 ; ld4s r25, r26 }
    f7a8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; info 19 ; ld4u r25, r26 }
    f7b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
    f7b8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; move r5, r6 ; st4 r25, r26 }
    f7c0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 }
    f7c8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    f7d0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; st2 r25, r26 }
    f7d8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    f7e0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
    f7e8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; mz r5, r6, r7 ; st2 r25, r26 }
    f7f0:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; nor r5, r6, r7 }
    f7f8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; add r5, r6, r7 ; prefetch r25 }
    f800:	[0-9a-f]* 	{ revbytes r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    f808:	[0-9a-f]* 	{ ctz r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    f810:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrsi r15, r16, 5 ; prefetch r25 }
    f818:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l1_fault r25 }
    f820:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l2 r25 }
    f828:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    f830:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2_fault r25 }
    f838:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l3 r25 }
    f840:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    f848:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; move r5, r6 ; prefetch_l3_fault r25 }
    f850:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
    f858:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; rotl r5, r6, r7 ; ld1s r25, r26 }
    f860:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl r5, r6, r7 ; ld2s r25, r26 }
    f868:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
    f870:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    f878:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch r25 }
    f880:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch r25 }
    f888:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    f890:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpeq r5, r6, r7 ; st r25, r26 }
    f898:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl3addx r5, r6, r7 ; st r25, r26 }
    f8a0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
    f8a8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; addxi r5, r6, 5 ; st2 r25, r26 }
    f8b0:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shl r5, r6, r7 ; st2 r25, r26 }
    f8b8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; info 19 ; st4 r25, r26 }
    f8c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrsi r15, r16, 5 ; st4 r25, r26 }
    f8c8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; subx r5, r6, r7 }
    f8d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrsi r15, r16, 5 ; ld r25, r26 }
    f8d8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; v1adduc r5, r6, r7 }
    f8e0:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; v1shrui r5, r6, 5 }
    f8e8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; v2shrs r5, r6, r7 }
    f8f0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; add r15, r16, r17 ; ld2s r25, r26 }
    f8f8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; addx r15, r16, r17 ; ld2u r25, r26 }
    f900:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; and r15, r16, r17 ; ld2u r25, r26 }
    f908:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    f910:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    f918:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch r25 }
    f920:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    f928:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; fetchand4 r15, r16, r17 }
    f930:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ill ; st r25, r26 }
    f938:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jalr r15 ; prefetch_l3_fault r25 }
    f940:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jr r15 ; st1 r25, r26 }
    f948:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; info 19 ; ld r25, r26 }
    f950:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmples r15, r16, r17 ; ld1s r25, r26 }
    f958:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; ld1u r15, r16 }
    f960:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    f968:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; rotli r15, r16, 5 ; ld2s r25, r26 }
    f970:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; lnk r15 ; ld2u r25, r26 }
    f978:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
    f980:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    f988:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; sub r15, r16, r17 ; ld4u r25, r26 }
    f990:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; lnk r15 }
    f998:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; move r15, r16 }
    f9a0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; mz r15, r16, r17 }
    f9a8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; or r15, r16, r17 ; ld1s r25, r26 }
    f9b0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jrp r15 ; prefetch r25 }
    f9b8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch r25 }
    f9c0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; prefetch r25 }
    f9c8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
    f9d0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    f9d8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
    f9e0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; prefetch_l3 r25 }
    f9e8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    f9f0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; prefetch_l3_fault r25 }
    f9f8:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shl r15, r16, r17 ; ld r25, r26 }
    fa00:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    fa08:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
    fa10:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    fa18:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shrs r15, r16, r17 ; ld4s r25, r26 }
    fa20:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
    fa28:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; st r25, r26 }
    fa30:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; st r25, r26 }
    fa38:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; shli r15, r16, 5 ; st1 r25, r26 }
    fa40:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; rotl r15, r16, r17 ; st2 r25, r26 }
    fa48:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; jrp r15 ; st4 r25, r26 }
    fa50:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; sub r15, r16, r17 ; ld2s r25, r26 }
    fa58:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
    fa60:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; v2mins r15, r16, r17 }
    fa68:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    fa70:	[0-9a-f]* 	{ shru r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
    fa78:	[0-9a-f]* 	{ shru r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
    fa80:	[0-9a-f]* 	{ shru r15, r16, r17 ; andi r5, r6, 5 ; st r25, r26 }
    fa88:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l3_fault r25 }
    fa90:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
    fa98:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmples r5, r6, r7 ; st4 r25, r26 }
    faa0:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
    faa8:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    fab0:	[0-9a-f]* 	{ ctz r5, r6 ; shru r15, r16, r17 ; prefetch_l3_fault r25 }
    fab8:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; shru r15, r16, r17 }
    fac0:	[0-9a-f]* 	{ shru r15, r16, r17 ; info 19 }
    fac8:	[0-9a-f]* 	{ pcnt r5, r6 ; shru r15, r16, r17 ; ld r25, r26 }
    fad0:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpltu r5, r6, r7 ; ld1s r25, r26 }
    fad8:	[0-9a-f]* 	{ shru r15, r16, r17 ; sub r5, r6, r7 ; ld1s r25, r26 }
    fae0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shru r15, r16, r17 ; ld1u r25, r26 }
    fae8:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    faf0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl3addx r5, r6, r7 ; ld2s r25, r26 }
    faf8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
    fb00:	[0-9a-f]* 	{ shru r15, r16, r17 ; addxi r5, r6, 5 ; ld4s r25, r26 }
    fb08:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl r5, r6, r7 ; ld4s r25, r26 }
    fb10:	[0-9a-f]* 	{ shru r15, r16, r17 ; info 19 ; ld4u r25, r26 }
    fb18:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
    fb20:	[0-9a-f]* 	{ shru r15, r16, r17 ; move r5, r6 ; st4 r25, r26 }
    fb28:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shru r15, r16, r17 }
    fb30:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    fb38:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; st2 r25, r26 }
    fb40:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l3_fault r25 }
    fb48:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
    fb50:	[0-9a-f]* 	{ shru r15, r16, r17 ; mz r5, r6, r7 ; st2 r25, r26 }
    fb58:	[0-9a-f]* 	{ shru r15, r16, r17 ; nor r5, r6, r7 }
    fb60:	[0-9a-f]* 	{ shru r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
    fb68:	[0-9a-f]* 	{ revbytes r5, r6 ; shru r15, r16, r17 ; prefetch r25 }
    fb70:	[0-9a-f]* 	{ ctz r5, r6 ; shru r15, r16, r17 ; prefetch r25 }
    fb78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shru r15, r16, r17 ; prefetch r25 }
    fb80:	[0-9a-f]* 	{ shru r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1_fault r25 }
    fb88:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l2 r25 }
    fb90:	[0-9a-f]* 	{ shru r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    fb98:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2_fault r25 }
    fba0:	[0-9a-f]* 	{ shru r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3 r25 }
    fba8:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    fbb0:	[0-9a-f]* 	{ shru r15, r16, r17 ; move r5, r6 ; prefetch_l3_fault r25 }
    fbb8:	[0-9a-f]* 	{ shru r15, r16, r17 ; prefetch_l3_fault r25 }
    fbc0:	[0-9a-f]* 	{ shru r15, r16, r17 ; rotl r5, r6, r7 ; ld1s r25, r26 }
    fbc8:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl r5, r6, r7 ; ld2s r25, r26 }
    fbd0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
    fbd8:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    fbe0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
    fbe8:	[0-9a-f]* 	{ shru r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
    fbf0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    fbf8:	[0-9a-f]* 	{ shru r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
    fc00:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl3addx r5, r6, r7 ; st r25, r26 }
    fc08:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
    fc10:	[0-9a-f]* 	{ shru r15, r16, r17 ; addxi r5, r6, 5 ; st2 r25, r26 }
    fc18:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl r5, r6, r7 ; st2 r25, r26 }
    fc20:	[0-9a-f]* 	{ shru r15, r16, r17 ; info 19 ; st4 r25, r26 }
    fc28:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; st4 r25, r26 }
    fc30:	[0-9a-f]* 	{ shru r15, r16, r17 ; subx r5, r6, r7 }
    fc38:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shru r15, r16, r17 ; ld r25, r26 }
    fc40:	[0-9a-f]* 	{ shru r15, r16, r17 ; v1adduc r5, r6, r7 }
    fc48:	[0-9a-f]* 	{ shru r15, r16, r17 ; v1shrui r5, r6, 5 }
    fc50:	[0-9a-f]* 	{ shru r15, r16, r17 ; v2shrs r5, r6, r7 }
    fc58:	[0-9a-f]* 	{ shru r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
    fc60:	[0-9a-f]* 	{ shru r5, r6, r7 ; addx r15, r16, r17 ; ld2u r25, r26 }
    fc68:	[0-9a-f]* 	{ shru r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
    fc70:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    fc78:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    fc80:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
    fc88:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    fc90:	[0-9a-f]* 	{ shru r5, r6, r7 ; fetchand4 r15, r16, r17 }
    fc98:	[0-9a-f]* 	{ shru r5, r6, r7 ; ill ; st r25, r26 }
    fca0:	[0-9a-f]* 	{ shru r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
    fca8:	[0-9a-f]* 	{ shru r5, r6, r7 ; jr r15 ; st1 r25, r26 }
    fcb0:	[0-9a-f]* 	{ shru r5, r6, r7 ; info 19 ; ld r25, r26 }
    fcb8:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmples r15, r16, r17 ; ld1s r25, r26 }
    fcc0:	[0-9a-f]* 	{ shru r5, r6, r7 ; ld1u r15, r16 }
    fcc8:	[0-9a-f]* 	{ shru r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
    fcd0:	[0-9a-f]* 	{ shru r5, r6, r7 ; rotli r15, r16, 5 ; ld2s r25, r26 }
    fcd8:	[0-9a-f]* 	{ shru r5, r6, r7 ; lnk r15 ; ld2u r25, r26 }
    fce0:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
    fce8:	[0-9a-f]* 	{ shru r5, r6, r7 ; addxi r15, r16, 5 ; ld4u r25, r26 }
    fcf0:	[0-9a-f]* 	{ shru r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
    fcf8:	[0-9a-f]* 	{ shru r5, r6, r7 ; lnk r15 }
    fd00:	[0-9a-f]* 	{ shru r5, r6, r7 ; move r15, r16 }
    fd08:	[0-9a-f]* 	{ shru r5, r6, r7 ; mz r15, r16, r17 }
    fd10:	[0-9a-f]* 	{ shru r5, r6, r7 ; or r15, r16, r17 ; ld1s r25, r26 }
    fd18:	[0-9a-f]* 	{ shru r5, r6, r7 ; jrp r15 ; prefetch r25 }
    fd20:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
    fd28:	[0-9a-f]* 	{ shru r5, r6, r7 ; prefetch r25 }
    fd30:	[0-9a-f]* 	{ shru r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
    fd38:	[0-9a-f]* 	{ shru r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
    fd40:	[0-9a-f]* 	{ shru r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
    fd48:	[0-9a-f]* 	{ shru r5, r6, r7 ; prefetch_l3 r25 }
    fd50:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
    fd58:	[0-9a-f]* 	{ shru r5, r6, r7 ; prefetch_l3_fault r25 }
    fd60:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl r15, r16, r17 ; ld r25, r26 }
    fd68:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
    fd70:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
    fd78:	[0-9a-f]* 	{ shru r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
    fd80:	[0-9a-f]* 	{ shru r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
    fd88:	[0-9a-f]* 	{ shru r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
    fd90:	[0-9a-f]* 	{ shru r5, r6, r7 ; cmpeq r15, r16, r17 ; st r25, r26 }
    fd98:	[0-9a-f]* 	{ shru r5, r6, r7 ; st r25, r26 }
    fda0:	[0-9a-f]* 	{ shru r5, r6, r7 ; shli r15, r16, 5 ; st1 r25, r26 }
    fda8:	[0-9a-f]* 	{ shru r5, r6, r7 ; rotl r15, r16, r17 ; st2 r25, r26 }
    fdb0:	[0-9a-f]* 	{ shru r5, r6, r7 ; jrp r15 ; st4 r25, r26 }
    fdb8:	[0-9a-f]* 	{ shru r5, r6, r7 ; sub r15, r16, r17 ; ld2s r25, r26 }
    fdc0:	[0-9a-f]* 	{ shru r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
    fdc8:	[0-9a-f]* 	{ shru r5, r6, r7 ; v2mins r15, r16, r17 }
    fdd0:	[0-9a-f]* 	{ shru r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
    fdd8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
    fde0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; addxi r5, r6, 5 ; st r25, r26 }
    fde8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; andi r5, r6, 5 ; st r25, r26 }
    fdf0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    fdf8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
    fe00:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmples r5, r6, r7 ; st4 r25, r26 }
    fe08:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
    fe10:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
    fe18:	[0-9a-f]* 	{ ctz r5, r6 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    fe20:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; shrui r15, r16, 5 }
    fe28:	[0-9a-f]* 	{ shrui r15, r16, 5 ; info 19 }
    fe30:	[0-9a-f]* 	{ pcnt r5, r6 ; shrui r15, r16, 5 ; ld r25, r26 }
    fe38:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpltu r5, r6, r7 ; ld1s r25, r26 }
    fe40:	[0-9a-f]* 	{ shrui r15, r16, 5 ; sub r5, r6, r7 ; ld1s r25, r26 }
    fe48:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; ld1u r25, r26 }
    fe50:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
    fe58:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl3addx r5, r6, r7 ; ld2s r25, r26 }
    fe60:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; ld2u r25, r26 }
    fe68:	[0-9a-f]* 	{ shrui r15, r16, 5 ; addxi r5, r6, 5 ; ld4s r25, r26 }
    fe70:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl r5, r6, r7 ; ld4s r25, r26 }
    fe78:	[0-9a-f]* 	{ shrui r15, r16, 5 ; info 19 ; ld4u r25, r26 }
    fe80:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrui r15, r16, 5 ; ld4u r25, r26 }
    fe88:	[0-9a-f]* 	{ shrui r15, r16, 5 ; move r5, r6 ; st4 r25, r26 }
    fe90:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 }
    fe98:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; st1 r25, r26 }
    fea0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
    fea8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    feb0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; st r25, r26 }
    feb8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; mz r5, r6, r7 ; st2 r25, r26 }
    fec0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; nor r5, r6, r7 }
    fec8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; add r5, r6, r7 ; prefetch r25 }
    fed0:	[0-9a-f]* 	{ revbytes r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    fed8:	[0-9a-f]* 	{ ctz r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    fee0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
    fee8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l1_fault r25 }
    fef0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l2 r25 }
    fef8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
    ff00:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2_fault r25 }
    ff08:	[0-9a-f]* 	{ shrui r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l3 r25 }
    ff10:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl1addx r5, r6, r7 ; prefetch_l3 r25 }
    ff18:	[0-9a-f]* 	{ shrui r15, r16, 5 ; move r5, r6 ; prefetch_l3_fault r25 }
    ff20:	[0-9a-f]* 	{ shrui r15, r16, 5 ; prefetch_l3_fault r25 }
    ff28:	[0-9a-f]* 	{ shrui r15, r16, 5 ; rotl r5, r6, r7 ; ld1s r25, r26 }
    ff30:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl r5, r6, r7 ; ld2s r25, r26 }
    ff38:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
    ff40:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
    ff48:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch r25 }
    ff50:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shrs r5, r6, r7 ; prefetch r25 }
    ff58:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
    ff60:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpeq r5, r6, r7 ; st r25, r26 }
    ff68:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl3addx r5, r6, r7 ; st r25, r26 }
    ff70:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrui r15, r16, 5 ; st1 r25, r26 }
    ff78:	[0-9a-f]* 	{ shrui r15, r16, 5 ; addxi r5, r6, 5 ; st2 r25, r26 }
    ff80:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shl r5, r6, r7 ; st2 r25, r26 }
    ff88:	[0-9a-f]* 	{ shrui r15, r16, 5 ; info 19 ; st4 r25, r26 }
    ff90:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrui r15, r16, 5 ; st4 r25, r26 }
    ff98:	[0-9a-f]* 	{ shrui r15, r16, 5 ; subx r5, r6, r7 }
    ffa0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrui r15, r16, 5 ; ld r25, r26 }
    ffa8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; v1adduc r5, r6, r7 }
    ffb0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; v1shrui r5, r6, 5 }
    ffb8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; v2shrs r5, r6, r7 }
    ffc0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; add r15, r16, r17 ; ld2s r25, r26 }
    ffc8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; addx r15, r16, r17 ; ld2u r25, r26 }
    ffd0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; and r15, r16, r17 ; ld2u r25, r26 }
    ffd8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
    ffe0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmples r15, r16, r17 ; ld4u r25, r26 }
    ffe8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch r25 }
    fff0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
    fff8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; fetchand4 r15, r16, r17 }
   10000:	[0-9a-f]* 	{ shrui r5, r6, 5 ; ill ; st r25, r26 }
   10008:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jalr r15 ; prefetch_l3_fault r25 }
   10010:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jr r15 ; st1 r25, r26 }
   10018:	[0-9a-f]* 	{ shrui r5, r6, 5 ; info 19 ; ld r25, r26 }
   10020:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmples r15, r16, r17 ; ld1s r25, r26 }
   10028:	[0-9a-f]* 	{ shrui r5, r6, 5 ; ld1u r15, r16 }
   10030:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shrs r15, r16, r17 ; ld1u r25, r26 }
   10038:	[0-9a-f]* 	{ shrui r5, r6, 5 ; rotli r15, r16, 5 ; ld2s r25, r26 }
   10040:	[0-9a-f]* 	{ shrui r5, r6, 5 ; lnk r15 ; ld2u r25, r26 }
   10048:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpltu r15, r16, r17 ; ld4s r25, r26 }
   10050:	[0-9a-f]* 	{ shrui r5, r6, 5 ; addxi r15, r16, 5 ; ld4u r25, r26 }
   10058:	[0-9a-f]* 	{ shrui r5, r6, 5 ; sub r15, r16, r17 ; ld4u r25, r26 }
   10060:	[0-9a-f]* 	{ shrui r5, r6, 5 ; lnk r15 }
   10068:	[0-9a-f]* 	{ shrui r5, r6, 5 ; move r15, r16 }
   10070:	[0-9a-f]* 	{ shrui r5, r6, 5 ; mz r15, r16, r17 }
   10078:	[0-9a-f]* 	{ shrui r5, r6, 5 ; or r15, r16, r17 ; ld1s r25, r26 }
   10080:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jrp r15 ; prefetch r25 }
   10088:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch r25 }
   10090:	[0-9a-f]* 	{ shrui r5, r6, 5 ; prefetch r25 }
   10098:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
   100a0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l2 r25 }
   100a8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
   100b0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; prefetch_l3 r25 }
   100b8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
   100c0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; prefetch_l3_fault r25 }
   100c8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl r15, r16, r17 ; ld r25, r26 }
   100d0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
   100d8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
   100e0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
   100e8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shrs r15, r16, r17 ; ld4s r25, r26 }
   100f0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
   100f8:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpeq r15, r16, r17 ; st r25, r26 }
   10100:	[0-9a-f]* 	{ shrui r5, r6, 5 ; st r25, r26 }
   10108:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shli r15, r16, 5 ; st1 r25, r26 }
   10110:	[0-9a-f]* 	{ shrui r5, r6, 5 ; rotl r15, r16, r17 ; st2 r25, r26 }
   10118:	[0-9a-f]* 	{ shrui r5, r6, 5 ; jrp r15 ; st4 r25, r26 }
   10120:	[0-9a-f]* 	{ shrui r5, r6, 5 ; sub r15, r16, r17 ; ld2s r25, r26 }
   10128:	[0-9a-f]* 	{ shrui r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
   10130:	[0-9a-f]* 	{ shrui r5, r6, 5 ; v2mins r15, r16, r17 }
   10138:	[0-9a-f]* 	{ shrui r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
   10140:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; shrux r15, r16, r17 }
   10148:	[0-9a-f]* 	{ mula_hs_hu r5, r6, r7 ; shrux r15, r16, r17 }
   10150:	[0-9a-f]* 	{ shrux r15, r16, r17 ; subx r5, r6, r7 }
   10158:	[0-9a-f]* 	{ shrux r15, r16, r17 ; v1mz r5, r6, r7 }
   10160:	[0-9a-f]* 	{ shrux r15, r16, r17 ; v2packuc r5, r6, r7 }
   10168:	[0-9a-f]* 	{ shrux r5, r6, r7 ; cmples r15, r16, r17 }
   10170:	[0-9a-f]* 	{ shrux r5, r6, r7 ; ld2s r15, r16 }
   10178:	[0-9a-f]* 	{ shrux r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
   10180:	[0-9a-f]* 	{ shrux r5, r6, r7 ; stnt1 r15, r16 }
   10188:	[0-9a-f]* 	{ shrux r5, r6, r7 ; v2addsc r15, r16, r17 }
   10190:	[0-9a-f]* 	{ shrux r5, r6, r7 ; v4subsc r15, r16, r17 }
   10198:	[0-9a-f]* 	{ shruxi r15, r16, 5 ; dblalign4 r5, r6, r7 }
   101a0:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; shruxi r15, r16, 5 }
   101a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shruxi r15, r16, 5 }
   101b0:	[0-9a-f]* 	{ shruxi r15, r16, 5 ; v1shli r5, r6, 5 }
   101b8:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; shruxi r15, r16, 5 }
   101c0:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; cmpltu r15, r16, r17 }
   101c8:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; ld4s r15, r16 }
   101d0:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; prefetch_add_l3_fault r15, 5 }
   101d8:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; stnt4 r15, r16 }
   101e0:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; v2cmpleu r15, r16, r17 }
   101e8:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; add r15, r16, r17 }
   101f0:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; info 19 }
   101f8:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
   10200:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; shru r15, r16, r17 }
   10208:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; v1minui r15, r16, 5 }
   10210:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; v2shrui r15, r16, 5 }
   10218:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; st r15, r16 }
   10220:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; st r15, r16 }
   10228:	[0-9a-f]* 	{ shlxi r5, r6, 5 ; st r15, r16 }
   10230:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; st r15, r16 }
   10238:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; st r15, r16 }
   10240:	[0-9a-f]* 	{ add r15, r16, r17 ; and r5, r6, r7 ; st r25, r26 }
   10248:	[0-9a-f]* 	{ add r15, r16, r17 ; shl1add r5, r6, r7 ; st r25, r26 }
   10250:	[0-9a-f]* 	{ add r5, r6, r7 ; lnk r15 ; st r25, r26 }
   10258:	[0-9a-f]* 	{ addi r15, r16, 5 ; cmpltsi r5, r6, 5 ; st r25, r26 }
   10260:	[0-9a-f]* 	{ addi r15, r16, 5 ; shrui r5, r6, 5 ; st r25, r26 }
   10268:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl r15, r16, r17 ; st r25, r26 }
   10270:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; st r25, r26 }
   10278:	[0-9a-f]* 	{ addx r5, r6, r7 ; addi r15, r16, 5 ; st r25, r26 }
   10280:	[0-9a-f]* 	{ addx r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
   10288:	[0-9a-f]* 	{ addxi r15, r16, 5 ; mz r5, r6, r7 ; st r25, r26 }
   10290:	[0-9a-f]* 	{ addxi r5, r6, 5 ; cmpltsi r15, r16, 5 ; st r25, r26 }
   10298:	[0-9a-f]* 	{ and r15, r16, r17 ; and r5, r6, r7 ; st r25, r26 }
   102a0:	[0-9a-f]* 	{ and r15, r16, r17 ; shl1add r5, r6, r7 ; st r25, r26 }
   102a8:	[0-9a-f]* 	{ and r5, r6, r7 ; lnk r15 ; st r25, r26 }
   102b0:	[0-9a-f]* 	{ andi r15, r16, 5 ; cmpltsi r5, r6, 5 ; st r25, r26 }
   102b8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shrui r5, r6, 5 ; st r25, r26 }
   102c0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl r15, r16, r17 ; st r25, r26 }
   102c8:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 ; st r25, r26 }
   102d0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; jalr r15 ; st r25, r26 }
   102d8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
   102e0:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
   102e8:	[0-9a-f]* 	{ cmpeq r15, r16, r17 ; shl r5, r6, r7 ; st r25, r26 }
   102f0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jrp r15 ; st r25, r26 }
   102f8:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; cmplts r5, r6, r7 ; st r25, r26 }
   10300:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; shru r5, r6, r7 ; st r25, r26 }
   10308:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; rotli r15, r16, 5 ; st r25, r26 }
   10310:	[0-9a-f]* 	{ cmples r15, r16, r17 ; movei r5, 5 ; st r25, r26 }
   10318:	[0-9a-f]* 	{ cmples r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
   10320:	[0-9a-f]* 	{ cmples r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
   10328:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
   10330:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
   10338:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
   10340:	[0-9a-f]* 	{ cmplts r15, r16, r17 ; shl r5, r6, r7 ; st r25, r26 }
   10348:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jrp r15 ; st r25, r26 }
   10350:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; cmplts r5, r6, r7 ; st r25, r26 }
   10358:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; shru r5, r6, r7 ; st r25, r26 }
   10360:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; rotli r15, r16, 5 ; st r25, r26 }
   10368:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; movei r5, 5 ; st r25, r26 }
   10370:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
   10378:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
   10380:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
   10388:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
   10390:	[0-9a-f]* 	{ ctz r5, r6 ; addxi r15, r16, 5 ; st r25, r26 }
   10398:	[0-9a-f]* 	{ ctz r5, r6 ; sub r15, r16, r17 ; st r25, r26 }
   103a0:	[0-9a-f]* 	{ jalr r15 ; st r25, r26 }
   103a8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; st r25, r26 }
   103b0:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; st r25, r26 }
   103b8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; ill ; st r25, r26 }
   103c0:	[0-9a-f]* 	{ shl r5, r6, r7 ; ill ; st r25, r26 }
   103c8:	[0-9a-f]* 	{ info 19 ; cmples r5, r6, r7 ; st r25, r26 }
   103d0:	[0-9a-f]* 	{ info 19 ; nor r15, r16, r17 ; st r25, r26 }
   103d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; st r25, r26 }
   103e0:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalr r15 ; st r25, r26 }
   103e8:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jalrp r15 ; st r25, r26 }
   103f0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jalrp r15 ; st r25, r26 }
   103f8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jr r15 ; st r25, r26 }
   10400:	[0-9a-f]* 	{ andi r5, r6, 5 ; jrp r15 ; st r25, r26 }
   10408:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jrp r15 ; st r25, r26 }
   10410:	[0-9a-f]* 	{ move r5, r6 ; lnk r15 ; st r25, r26 }
   10418:	[0-9a-f]* 	{ lnk r15 ; st r25, r26 }
   10420:	[0-9a-f]* 	{ revbits r5, r6 ; mnz r15, r16, r17 ; st r25, r26 }
   10428:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; st r25, r26 }
   10430:	[0-9a-f]* 	{ move r15, r16 ; cmpeq r5, r6, r7 ; st r25, r26 }
   10438:	[0-9a-f]* 	{ move r15, r16 ; shl3addx r5, r6, r7 ; st r25, r26 }
   10440:	[0-9a-f]* 	{ move r5, r6 ; nop ; st r25, r26 }
   10448:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; st r25, r26 }
   10450:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; st r25, r26 }
   10458:	[0-9a-f]* 	{ movei r5, 5 ; shl3add r15, r16, r17 ; st r25, r26 }
   10460:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
   10468:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; st r25, r26 }
   10470:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; ill ; st r25, r26 }
   10478:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
   10480:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; st r25, r26 }
   10488:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
   10490:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
   10498:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; st r25, r26 }
   104a0:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; jrp r15 ; st r25, r26 }
   104a8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
   104b0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; cmpeq r15, r16, r17 ; st r25, r26 }
   104b8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; st r25, r26 }
   104c0:	[0-9a-f]* 	{ revbits r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
   104c8:	[0-9a-f]* 	{ mz r5, r6, r7 ; info 19 ; st r25, r26 }
   104d0:	[0-9a-f]* 	{ nop ; and r5, r6, r7 ; st r25, r26 }
   104d8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; nop ; st r25, r26 }
   104e0:	[0-9a-f]* 	{ nop ; shrsi r15, r16, 5 ; st r25, r26 }
   104e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; st r25, r26 }
   104f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
   104f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
   10500:	[0-9a-f]* 	{ mulx r5, r6, r7 ; or r15, r16, r17 ; st r25, r26 }
   10508:	[0-9a-f]* 	{ or r5, r6, r7 ; cmplts r15, r16, r17 ; st r25, r26 }
   10510:	[0-9a-f]* 	{ pcnt r5, r6 ; addxi r15, r16, 5 ; st r25, r26 }
   10518:	[0-9a-f]* 	{ pcnt r5, r6 ; sub r15, r16, r17 ; st r25, r26 }
   10520:	[0-9a-f]* 	{ revbits r5, r6 ; shl3add r15, r16, r17 ; st r25, r26 }
   10528:	[0-9a-f]* 	{ revbytes r5, r6 ; rotl r15, r16, r17 ; st r25, r26 }
   10530:	[0-9a-f]* 	{ rotl r15, r16, r17 ; move r5, r6 ; st r25, r26 }
   10538:	[0-9a-f]* 	{ rotl r15, r16, r17 ; st r25, r26 }
   10540:	[0-9a-f]* 	{ rotl r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
   10548:	[0-9a-f]* 	{ mulax r5, r6, r7 ; rotli r15, r16, 5 ; st r25, r26 }
   10550:	[0-9a-f]* 	{ rotli r5, r6, 5 ; cmpleu r15, r16, r17 ; st r25, r26 }
   10558:	[0-9a-f]* 	{ shl r15, r16, r17 ; addx r5, r6, r7 ; st r25, r26 }
   10560:	[0-9a-f]* 	{ shl r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
   10568:	[0-9a-f]* 	{ shl r5, r6, r7 ; jr r15 ; st r25, r26 }
   10570:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
   10578:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
   10580:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
   10588:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; move r5, r6 ; st r25, r26 }
   10590:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; st r25, r26 }
   10598:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
   105a0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
   105a8:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; cmpleu r15, r16, r17 ; st r25, r26 }
   105b0:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; addx r5, r6, r7 ; st r25, r26 }
   105b8:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
   105c0:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; jr r15 ; st r25, r26 }
   105c8:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
   105d0:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
   105d8:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
   105e0:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; move r5, r6 ; st r25, r26 }
   105e8:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; st r25, r26 }
   105f0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
   105f8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
   10600:	[0-9a-f]* 	{ shli r5, r6, 5 ; cmpleu r15, r16, r17 ; st r25, r26 }
   10608:	[0-9a-f]* 	{ shrs r15, r16, r17 ; addx r5, r6, r7 ; st r25, r26 }
   10610:	[0-9a-f]* 	{ shrs r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
   10618:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; st r25, r26 }
   10620:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; cmpleu r5, r6, r7 ; st r25, r26 }
   10628:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; shrsi r5, r6, 5 ; st r25, r26 }
   10630:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; rotl r15, r16, r17 ; st r25, r26 }
   10638:	[0-9a-f]* 	{ shru r15, r16, r17 ; move r5, r6 ; st r25, r26 }
   10640:	[0-9a-f]* 	{ shru r15, r16, r17 ; st r25, r26 }
   10648:	[0-9a-f]* 	{ shru r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
   10650:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; st r25, r26 }
   10658:	[0-9a-f]* 	{ shrui r5, r6, 5 ; cmpleu r15, r16, r17 ; st r25, r26 }
   10660:	[0-9a-f]* 	{ sub r15, r16, r17 ; addx r5, r6, r7 ; st r25, r26 }
   10668:	[0-9a-f]* 	{ sub r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
   10670:	[0-9a-f]* 	{ sub r5, r6, r7 ; jr r15 ; st r25, r26 }
   10678:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpleu r5, r6, r7 ; st r25, r26 }
   10680:	[0-9a-f]* 	{ subx r15, r16, r17 ; shrsi r5, r6, 5 ; st r25, r26 }
   10688:	[0-9a-f]* 	{ subx r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
   10690:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; st r25, r26 }
   10698:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; st r25, r26 }
   106a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmples r15, r16, r17 ; st r25, r26 }
   106a8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addi r15, r16, 5 ; st r25, r26 }
   106b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; st r25, r26 }
   106b8:	[0-9a-f]* 	{ xor r15, r16, r17 ; mz r5, r6, r7 ; st r25, r26 }
   106c0:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpltsi r15, r16, 5 ; st r25, r26 }
   106c8:	[0-9a-f]* 	{ addxi r5, r6, 5 ; st1 r15, r16 }
   106d0:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; st1 r15, r16 }
   106d8:	[0-9a-f]* 	{ nop ; st1 r15, r16 }
   106e0:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; st1 r15, r16 }
   106e8:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; st1 r15, r16 }
   106f0:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; st1 r15, r16 }
   106f8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
   10700:	[0-9a-f]* 	{ add r5, r6, r7 ; addx r15, r16, r17 ; st1 r25, r26 }
   10708:	[0-9a-f]* 	{ add r5, r6, r7 ; shrui r15, r16, 5 ; st1 r25, r26 }
   10710:	[0-9a-f]* 	{ addi r15, r16, 5 ; nop ; st1 r25, r26 }
   10718:	[0-9a-f]* 	{ addi r5, r6, 5 ; cmpltu r15, r16, r17 ; st1 r25, r26 }
   10720:	[0-9a-f]* 	{ addx r15, r16, r17 ; andi r5, r6, 5 ; st1 r25, r26 }
   10728:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl1addx r5, r6, r7 ; st1 r25, r26 }
   10730:	[0-9a-f]* 	{ addx r5, r6, r7 ; mnz r15, r16, r17 ; st1 r25, r26 }
   10738:	[0-9a-f]* 	{ addxi r15, r16, 5 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
   10740:	[0-9a-f]* 	{ addxi r15, r16, 5 ; sub r5, r6, r7 ; st1 r25, r26 }
   10748:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl1add r15, r16, r17 ; st1 r25, r26 }
   10750:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
   10758:	[0-9a-f]* 	{ and r5, r6, r7 ; addx r15, r16, r17 ; st1 r25, r26 }
   10760:	[0-9a-f]* 	{ and r5, r6, r7 ; shrui r15, r16, 5 ; st1 r25, r26 }
   10768:	[0-9a-f]* 	{ andi r15, r16, 5 ; nop ; st1 r25, r26 }
   10770:	[0-9a-f]* 	{ andi r5, r6, 5 ; cmpltu r15, r16, r17 ; st1 r25, r26 }
   10778:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; st1 r25, r26 }
   10780:	[0-9a-f]* 	{ clz r5, r6 ; xor r15, r16, r17 ; st1 r25, r26 }
   10788:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shli r15, r16, 5 ; st1 r25, r26 }
   10790:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl r15, r16, r17 ; st1 r25, r26 }
   10798:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; st1 r25, r26 }
   107a0:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
   107a8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
   107b0:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; mz r5, r6, r7 ; st1 r25, r26 }
   107b8:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpltsi r15, r16, 5 ; st1 r25, r26 }
   107c0:	[0-9a-f]* 	{ cmples r15, r16, r17 ; and r5, r6, r7 ; st1 r25, r26 }
   107c8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
   107d0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
   107d8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
   107e0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; shrui r5, r6, 5 ; st1 r25, r26 }
   107e8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl r15, r16, r17 ; st1 r25, r26 }
   107f0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   107f8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
   10800:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
   10808:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; mz r5, r6, r7 ; st1 r25, r26 }
   10810:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpltsi r15, r16, 5 ; st1 r25, r26 }
   10818:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; and r5, r6, r7 ; st1 r25, r26 }
   10820:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
   10828:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
   10830:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
   10838:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; shrui r5, r6, 5 ; st1 r25, r26 }
   10840:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl r15, r16, r17 ; st1 r25, r26 }
   10848:	[0-9a-f]* 	{ ctz r5, r6 ; movei r15, 5 ; st1 r25, r26 }
   10850:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; st1 r25, r26 }
   10858:	[0-9a-f]* 	{ mz r15, r16, r17 ; st1 r25, r26 }
   10860:	[0-9a-f]* 	{ subx r15, r16, r17 ; st1 r25, r26 }
   10868:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl r15, r16, r17 ; st1 r25, r26 }
   10870:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; ill ; st1 r25, r26 }
   10878:	[0-9a-f]* 	{ info 19 ; add r5, r6, r7 ; st1 r25, r26 }
   10880:	[0-9a-f]* 	{ info 19 ; mnz r15, r16, r17 ; st1 r25, r26 }
   10888:	[0-9a-f]* 	{ info 19 ; shl3add r15, r16, r17 ; st1 r25, r26 }
   10890:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
   10898:	[0-9a-f]* 	{ sub r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
   108a0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; jalrp r15 ; st1 r25, r26 }
   108a8:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; jr r15 ; st1 r25, r26 }
   108b0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jr r15 ; st1 r25, r26 }
   108b8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; jrp r15 ; st1 r25, r26 }
   108c0:	[0-9a-f]* 	{ addxi r5, r6, 5 ; lnk r15 ; st1 r25, r26 }
   108c8:	[0-9a-f]* 	{ shl r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
   108d0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; info 19 ; st1 r25, r26 }
   108d8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; st1 r25, r26 }
   108e0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
   108e8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
   108f0:	[0-9a-f]* 	{ move r5, r6 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
   108f8:	[0-9a-f]* 	{ movei r15, 5 ; add r5, r6, r7 ; st1 r25, r26 }
   10900:	[0-9a-f]* 	{ revbytes r5, r6 ; movei r15, 5 ; st1 r25, r26 }
   10908:	[0-9a-f]* 	{ movei r5, 5 ; jalr r15 ; st1 r25, r26 }
   10910:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   10918:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; addxi r15, r16, 5 ; st1 r25, r26 }
   10920:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; st1 r25, r26 }
   10928:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 ; st1 r25, r26 }
   10930:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; st1 r25, r26 }
   10938:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; mnz r15, r16, r17 ; st1 r25, r26 }
   10940:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; ill ; st1 r25, r26 }
   10948:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
   10950:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; st1 r25, r26 }
   10958:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
   10960:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl2add r15, r16, r17 ; st1 r25, r26 }
   10968:	[0-9a-f]* 	{ mulx r5, r6, r7 ; nor r15, r16, r17 ; st1 r25, r26 }
   10970:	[0-9a-f]* 	{ mz r15, r16, r17 ; info 19 ; st1 r25, r26 }
   10978:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; st1 r25, r26 }
   10980:	[0-9a-f]* 	{ mz r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
   10988:	[0-9a-f]* 	{ nop ; cmpne r5, r6, r7 ; st1 r25, r26 }
   10990:	[0-9a-f]* 	{ nop ; rotli r5, r6, 5 ; st1 r25, r26 }
   10998:	[0-9a-f]* 	{ nor r15, r16, r17 ; and r5, r6, r7 ; st1 r25, r26 }
   109a0:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
   109a8:	[0-9a-f]* 	{ nor r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
   109b0:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpltsi r5, r6, 5 ; st1 r25, r26 }
   109b8:	[0-9a-f]* 	{ or r15, r16, r17 ; shrui r5, r6, 5 ; st1 r25, r26 }
   109c0:	[0-9a-f]* 	{ or r5, r6, r7 ; shl r15, r16, r17 ; st1 r25, r26 }
   109c8:	[0-9a-f]* 	{ pcnt r5, r6 ; movei r15, 5 ; st1 r25, r26 }
   109d0:	[0-9a-f]* 	{ revbits r5, r6 ; jalr r15 ; st1 r25, r26 }
   109d8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   109e0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
   109e8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl r5, r6, r7 ; st1 r25, r26 }
   109f0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; jrp r15 ; st1 r25, r26 }
   109f8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmplts r5, r6, r7 ; st1 r25, r26 }
   10a00:	[0-9a-f]* 	{ rotli r15, r16, 5 ; shru r5, r6, r7 ; st1 r25, r26 }
   10a08:	[0-9a-f]* 	{ rotli r5, r6, 5 ; rotli r15, r16, 5 ; st1 r25, r26 }
   10a10:	[0-9a-f]* 	{ shl r15, r16, r17 ; movei r5, 5 ; st1 r25, r26 }
   10a18:	[0-9a-f]* 	{ shl r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
   10a20:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
   10a28:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl1add r15, r16, r17 ; st1 r25, r26 }
   10a30:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   10a38:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
   10a40:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl r5, r6, r7 ; st1 r25, r26 }
   10a48:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; jrp r15 ; st1 r25, r26 }
   10a50:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmplts r5, r6, r7 ; st1 r25, r26 }
   10a58:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; shru r5, r6, r7 ; st1 r25, r26 }
   10a60:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
   10a68:	[0-9a-f]* 	{ shl2addx r15, r16, r17 ; movei r5, 5 ; st1 r25, r26 }
   10a70:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
   10a78:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
   10a80:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl3add r15, r16, r17 ; st1 r25, r26 }
   10a88:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   10a90:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
   10a98:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl r5, r6, r7 ; st1 r25, r26 }
   10aa0:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; jrp r15 ; st1 r25, r26 }
   10aa8:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmplts r5, r6, r7 ; st1 r25, r26 }
   10ab0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shru r5, r6, r7 ; st1 r25, r26 }
   10ab8:	[0-9a-f]* 	{ shli r5, r6, 5 ; rotli r15, r16, 5 ; st1 r25, r26 }
   10ac0:	[0-9a-f]* 	{ shrs r15, r16, r17 ; movei r5, 5 ; st1 r25, r26 }
   10ac8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
   10ad0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
   10ad8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
   10ae0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   10ae8:	[0-9a-f]* 	{ shru r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
   10af0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl r5, r6, r7 ; st1 r25, r26 }
   10af8:	[0-9a-f]* 	{ shru r5, r6, r7 ; jrp r15 ; st1 r25, r26 }
   10b00:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmplts r5, r6, r7 ; st1 r25, r26 }
   10b08:	[0-9a-f]* 	{ shrui r15, r16, 5 ; shru r5, r6, r7 ; st1 r25, r26 }
   10b10:	[0-9a-f]* 	{ shrui r5, r6, 5 ; rotli r15, r16, 5 ; st1 r25, r26 }
   10b18:	[0-9a-f]* 	{ sub r15, r16, r17 ; movei r5, 5 ; st1 r25, r26 }
   10b20:	[0-9a-f]* 	{ sub r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
   10b28:	[0-9a-f]* 	{ sub r5, r6, r7 ; shrsi r15, r16, 5 ; st1 r25, r26 }
   10b30:	[0-9a-f]* 	{ mulx r5, r6, r7 ; subx r15, r16, r17 ; st1 r25, r26 }
   10b38:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   10b40:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addxi r15, r16, 5 ; st1 r25, r26 }
   10b48:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sub r15, r16, r17 ; st1 r25, r26 }
   10b50:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; st1 r25, r26 }
   10b58:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rotl r15, r16, r17 ; st1 r25, r26 }
   10b60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; st1 r25, r26 }
   10b68:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
   10b70:	[0-9a-f]* 	{ xor r15, r16, r17 ; sub r5, r6, r7 ; st1 r25, r26 }
   10b78:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl1add r15, r16, r17 ; st1 r25, r26 }
   10b80:	[0-9a-f]* 	{ cmula r5, r6, r7 ; st1_add r15, r16, 5 }
   10b88:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; st1_add r15, r16, 5 }
   10b90:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; st1_add r15, r16, 5 }
   10b98:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; st1_add r15, r16, 5 }
   10ba0:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; st1_add r15, r16, 5 }
   10ba8:	[0-9a-f]* 	{ addxsc r5, r6, r7 ; st2 r15, r16 }
   10bb0:	[0-9a-f]* 	{ st2 r15, r16 }
   10bb8:	[0-9a-f]* 	{ or r5, r6, r7 ; st2 r15, r16 }
   10bc0:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; st2 r15, r16 }
   10bc8:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; st2 r15, r16 }
   10bd0:	[0-9a-f]* 	{ v4add r5, r6, r7 ; st2 r15, r16 }
   10bd8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; add r15, r16, r17 ; st2 r25, r26 }
   10be0:	[0-9a-f]* 	{ add r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
   10be8:	[0-9a-f]* 	{ add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
   10bf0:	[0-9a-f]* 	{ addi r15, r16, 5 ; or r5, r6, r7 ; st2 r25, r26 }
   10bf8:	[0-9a-f]* 	{ addi r5, r6, 5 ; st2 r25, r26 }
   10c00:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
   10c08:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
   10c10:	[0-9a-f]* 	{ addx r5, r6, r7 ; movei r15, 5 ; st2 r25, r26 }
   10c18:	[0-9a-f]* 	{ ctz r5, r6 ; addxi r15, r16, 5 ; st2 r25, r26 }
   10c20:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addxi r15, r16, 5 ; st2 r25, r26 }
   10c28:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl2add r15, r16, r17 ; st2 r25, r26 }
   10c30:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
   10c38:	[0-9a-f]* 	{ and r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
   10c40:	[0-9a-f]* 	{ and r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
   10c48:	[0-9a-f]* 	{ andi r15, r16, 5 ; or r5, r6, r7 ; st2 r25, r26 }
   10c50:	[0-9a-f]* 	{ andi r5, r6, 5 ; st2 r25, r26 }
   10c58:	[0-9a-f]* 	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; st2 r25, r26 }
   10c60:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; add r15, r16, r17 ; st2 r25, r26 }
   10c68:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrsi r15, r16, 5 ; st2 r25, r26 }
   10c70:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl1addx r15, r16, r17 ; st2 r25, r26 }
   10c78:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
   10c80:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; addxi r15, r16, 5 ; st2 r25, r26 }
   10c88:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; st2 r25, r26 }
   10c90:	[0-9a-f]* 	{ cmpeqi r15, r16, 5 ; nor r5, r6, r7 ; st2 r25, r26 }
   10c98:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; cmpne r15, r16, r17 ; st2 r25, r26 }
   10ca0:	[0-9a-f]* 	{ clz r5, r6 ; cmples r15, r16, r17 ; st2 r25, r26 }
   10ca8:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
   10cb0:	[0-9a-f]* 	{ cmples r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
   10cb8:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; cmpne r5, r6, r7 ; st2 r25, r26 }
   10cc0:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; subx r5, r6, r7 ; st2 r25, r26 }
   10cc8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl1addx r15, r16, r17 ; st2 r25, r26 }
   10cd0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; st2 r25, r26 }
   10cd8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; addxi r15, r16, 5 ; st2 r25, r26 }
   10ce0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; sub r15, r16, r17 ; st2 r25, r26 }
   10ce8:	[0-9a-f]* 	{ cmpltsi r15, r16, 5 ; nor r5, r6, r7 ; st2 r25, r26 }
   10cf0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; cmpne r15, r16, r17 ; st2 r25, r26 }
   10cf8:	[0-9a-f]* 	{ clz r5, r6 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10d00:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
   10d08:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
   10d10:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; cmpne r5, r6, r7 ; st2 r25, r26 }
   10d18:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; subx r5, r6, r7 ; st2 r25, r26 }
   10d20:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl1addx r15, r16, r17 ; st2 r25, r26 }
   10d28:	[0-9a-f]* 	{ ctz r5, r6 ; nop ; st2 r25, r26 }
   10d30:	[0-9a-f]* 	{ cmples r15, r16, r17 ; st2 r25, r26 }
   10d38:	[0-9a-f]* 	{ nop ; st2 r25, r26 }
   10d40:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; st2 r25, r26 }
   10d48:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl1addx r15, r16, r17 ; st2 r25, r26 }
   10d50:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; ill ; st2 r25, r26 }
   10d58:	[0-9a-f]* 	{ info 19 ; addi r5, r6, 5 ; st2 r25, r26 }
   10d60:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; st2 r25, r26 }
   10d68:	[0-9a-f]* 	{ info 19 ; shl3addx r15, r16, r17 ; st2 r25, r26 }
   10d70:	[0-9a-f]* 	{ ctz r5, r6 ; jalr r15 ; st2 r25, r26 }
   10d78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalr r15 ; st2 r25, r26 }
   10d80:	[0-9a-f]* 	{ mz r5, r6, r7 ; jalrp r15 ; st2 r25, r26 }
   10d88:	[0-9a-f]* 	{ cmples r5, r6, r7 ; jr r15 ; st2 r25, r26 }
   10d90:	[0-9a-f]* 	{ shrs r5, r6, r7 ; jr r15 ; st2 r25, r26 }
   10d98:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; jrp r15 ; st2 r25, r26 }
   10da0:	[0-9a-f]* 	{ andi r5, r6, 5 ; lnk r15 ; st2 r25, r26 }
   10da8:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
   10db0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
   10db8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; st2 r25, r26 }
   10dc0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
   10dc8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
   10dd0:	[0-9a-f]* 	{ move r5, r6 ; cmpleu r15, r16, r17 ; st2 r25, r26 }
   10dd8:	[0-9a-f]* 	{ movei r15, 5 ; addx r5, r6, r7 ; st2 r25, r26 }
   10de0:	[0-9a-f]* 	{ movei r15, 5 ; rotli r5, r6, 5 ; st2 r25, r26 }
   10de8:	[0-9a-f]* 	{ movei r5, 5 ; jr r15 ; st2 r25, r26 }
   10df0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10df8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; st2 r25, r26 }
   10e00:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; xor r15, r16, r17 ; st2 r25, r26 }
   10e08:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shli r15, r16, 5 ; st2 r25, r26 }
   10e10:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
   10e18:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; movei r15, 5 ; st2 r25, r26 }
   10e20:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
   10e28:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; st2 r25, r26 }
   10e30:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; st2 r25, r26 }
   10e38:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; st2 r25, r26 }
   10e40:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
   10e48:	[0-9a-f]* 	{ mulx r5, r6, r7 ; rotl r15, r16, r17 ; st2 r25, r26 }
   10e50:	[0-9a-f]* 	{ mz r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
   10e58:	[0-9a-f]* 	{ mz r15, r16, r17 ; st2 r25, r26 }
   10e60:	[0-9a-f]* 	{ mz r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
   10e68:	[0-9a-f]* 	{ nop ; st2 r25, r26 }
   10e70:	[0-9a-f]* 	{ nop ; shl r5, r6, r7 ; st2 r25, r26 }
   10e78:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
   10e80:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl2add r5, r6, r7 ; st2 r25, r26 }
   10e88:	[0-9a-f]* 	{ nor r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
   10e90:	[0-9a-f]* 	{ or r15, r16, r17 ; cmpne r5, r6, r7 ; st2 r25, r26 }
   10e98:	[0-9a-f]* 	{ or r15, r16, r17 ; subx r5, r6, r7 ; st2 r25, r26 }
   10ea0:	[0-9a-f]* 	{ or r5, r6, r7 ; shl1addx r15, r16, r17 ; st2 r25, r26 }
   10ea8:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; st2 r25, r26 }
   10eb0:	[0-9a-f]* 	{ revbits r5, r6 ; jr r15 ; st2 r25, r26 }
   10eb8:	[0-9a-f]* 	{ revbytes r5, r6 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10ec0:	[0-9a-f]* 	{ rotl r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
   10ec8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
   10ed0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; mnz r15, r16, r17 ; st2 r25, r26 }
   10ed8:	[0-9a-f]* 	{ rotli r15, r16, 5 ; cmpltu r5, r6, r7 ; st2 r25, r26 }
   10ee0:	[0-9a-f]* 	{ rotli r15, r16, 5 ; sub r5, r6, r7 ; st2 r25, r26 }
   10ee8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   10ef0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
   10ef8:	[0-9a-f]* 	{ shl r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
   10f00:	[0-9a-f]* 	{ shl r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
   10f08:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; nop ; st2 r25, r26 }
   10f10:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10f18:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
   10f20:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
   10f28:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; mnz r15, r16, r17 ; st2 r25, r26 }
   10f30:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; cmpltu r5, r6, r7 ; st2 r25, r26 }
   10f38:	[0-9a-f]* 	{ shl2add r15, r16, r17 ; sub r5, r6, r7 ; st2 r25, r26 }
   10f40:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   10f48:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shl2addx r15, r16, r17 ; st2 r25, r26 }
   10f50:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
   10f58:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
   10f60:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; nop ; st2 r25, r26 }
   10f68:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10f70:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
   10f78:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
   10f80:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; mnz r15, r16, r17 ; st2 r25, r26 }
   10f88:	[0-9a-f]* 	{ shli r15, r16, 5 ; cmpltu r5, r6, r7 ; st2 r25, r26 }
   10f90:	[0-9a-f]* 	{ shli r15, r16, 5 ; sub r5, r6, r7 ; st2 r25, r26 }
   10f98:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   10fa0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
   10fa8:	[0-9a-f]* 	{ shrs r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
   10fb0:	[0-9a-f]* 	{ shrs r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
   10fb8:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; nop ; st2 r25, r26 }
   10fc0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   10fc8:	[0-9a-f]* 	{ shru r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
   10fd0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl1addx r5, r6, r7 ; st2 r25, r26 }
   10fd8:	[0-9a-f]* 	{ shru r5, r6, r7 ; mnz r15, r16, r17 ; st2 r25, r26 }
   10fe0:	[0-9a-f]* 	{ shrui r15, r16, 5 ; cmpltu r5, r6, r7 ; st2 r25, r26 }
   10fe8:	[0-9a-f]* 	{ shrui r15, r16, 5 ; sub r5, r6, r7 ; st2 r25, r26 }
   10ff0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   10ff8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; st2 r25, r26 }
   11000:	[0-9a-f]* 	{ sub r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
   11008:	[0-9a-f]* 	{ sub r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
   11010:	[0-9a-f]* 	{ subx r15, r16, r17 ; nop ; st2 r25, r26 }
   11018:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpltu r15, r16, r17 ; st2 r25, r26 }
   11020:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; st2 r25, r26 }
   11028:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; st2 r25, r26 }
   11030:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; st2 r25, r26 }
   11038:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl r15, r16, r17 ; st2 r25, r26 }
   11040:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; movei r15, 5 ; st2 r25, r26 }
   11048:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; st2 r25, r26 }
   11050:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; st2 r25, r26 }
   11058:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl2add r15, r16, r17 ; st2 r25, r26 }
   11060:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; st2_add r15, r16, 5 }
   11068:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; st2_add r15, r16, 5 }
   11070:	[0-9a-f]* 	{ shrui r5, r6, 5 ; st2_add r15, r16, 5 }
   11078:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; st2_add r15, r16, 5 }
   11080:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; st2_add r15, r16, 5 }
   11088:	[0-9a-f]* 	{ andi r5, r6, 5 ; st4 r15, r16 }
   11090:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; st4 r15, r16 }
   11098:	[0-9a-f]* 	{ pcnt r5, r6 ; st4 r15, r16 }
   110a0:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; st4 r15, r16 }
   110a8:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; st4 r15, r16 }
   110b0:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; st4 r15, r16 }
   110b8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; add r15, r16, r17 ; st4 r25, r26 }
   110c0:	[0-9a-f]* 	{ add r5, r6, r7 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
   110c8:	[0-9a-f]* 	{ add r5, r6, r7 ; st4 r25, r26 }
   110d0:	[0-9a-f]* 	{ revbits r5, r6 ; addi r15, r16, 5 ; st4 r25, r26 }
   110d8:	[0-9a-f]* 	{ addi r5, r6, 5 ; info 19 ; st4 r25, r26 }
   110e0:	[0-9a-f]* 	{ addx r15, r16, r17 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
   110e8:	[0-9a-f]* 	{ addx r15, r16, r17 ; shl3addx r5, r6, r7 ; st4 r25, r26 }
   110f0:	[0-9a-f]* 	{ addx r5, r6, r7 ; nop ; st4 r25, r26 }
   110f8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; addxi r15, r16, 5 ; st4 r25, r26 }
   11100:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addxi r15, r16, 5 ; st4 r25, r26 }
   11108:	[0-9a-f]* 	{ addxi r5, r6, 5 ; shl3add r15, r16, r17 ; st4 r25, r26 }
   11110:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
   11118:	[0-9a-f]* 	{ and r5, r6, r7 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
   11120:	[0-9a-f]* 	{ and r5, r6, r7 ; st4 r25, r26 }
   11128:	[0-9a-f]* 	{ revbits r5, r6 ; andi r15, r16, 5 ; st4 r25, r26 }
   11130:	[0-9a-f]* 	{ andi r5, r6, 5 ; info 19 ; st4 r25, r26 }
   11138:	[0-9a-f]* 	{ clz r5, r6 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
   11140:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; addx r15, r16, r17 ; st4 r25, r26 }
   11148:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shrui r15, r16, 5 ; st4 r25, r26 }
   11150:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   11158:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
   11160:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
   11168:	[0-9a-f]* 	{ cmpeq r5, r6, r7 ; xor r15, r16, r17 ; st4 r25, r26 }
   11170:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
   11178:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; ill ; st4 r25, r26 }
   11180:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
   11188:	[0-9a-f]* 	{ cmples r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
   11190:	[0-9a-f]* 	{ cmples r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
   11198:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; st4 r25, r26 }
   111a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
   111a8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   111b0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; st4 r25, r26 }
   111b8:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
   111c0:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; st4 r25, r26 }
   111c8:	[0-9a-f]* 	{ pcnt r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
   111d0:	[0-9a-f]* 	{ cmpltsi r5, r6, 5 ; ill ; st4 r25, r26 }
   111d8:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
   111e0:	[0-9a-f]* 	{ cmpltu r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
   111e8:	[0-9a-f]* 	{ cmpltu r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
   111f0:	[0-9a-f]* 	{ cmpne r15, r16, r17 ; st4 r25, r26 }
   111f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpne r15, r16, r17 ; st4 r25, r26 }
   11200:	[0-9a-f]* 	{ cmpne r5, r6, r7 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   11208:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; st4 r25, r26 }
   11210:	[0-9a-f]* 	{ cmpleu r15, r16, r17 ; st4 r25, r26 }
   11218:	[0-9a-f]* 	{ nor r5, r6, r7 ; st4 r25, r26 }
   11220:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; st4 r25, r26 }
   11228:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   11230:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; ill ; st4 r25, r26 }
   11238:	[0-9a-f]* 	{ info 19 ; addx r5, r6, r7 ; st4 r25, r26 }
   11240:	[0-9a-f]* 	{ info 19 ; movei r15, 5 ; st4 r25, r26 }
   11248:	[0-9a-f]* 	{ info 19 ; shli r15, r16, 5 ; st4 r25, r26 }
   11250:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; jalr r15 ; st4 r25, r26 }
   11258:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jalr r15 ; st4 r25, r26 }
   11260:	[0-9a-f]* 	{ nor r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
   11268:	[0-9a-f]* 	{ cmplts r5, r6, r7 ; jr r15 ; st4 r25, r26 }
   11270:	[0-9a-f]* 	{ shru r5, r6, r7 ; jr r15 ; st4 r25, r26 }
   11278:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; jrp r15 ; st4 r25, r26 }
   11280:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
   11288:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
   11290:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mnz r15, r16, r17 ; st4 r25, r26 }
   11298:	[0-9a-f]* 	{ mnz r5, r6, r7 ; addi r15, r16, 5 ; st4 r25, r26 }
   112a0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shru r15, r16, r17 ; st4 r25, r26 }
   112a8:	[0-9a-f]* 	{ move r15, r16 ; mz r5, r6, r7 ; st4 r25, r26 }
   112b0:	[0-9a-f]* 	{ move r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
   112b8:	[0-9a-f]* 	{ movei r15, 5 ; and r5, r6, r7 ; st4 r25, r26 }
   112c0:	[0-9a-f]* 	{ movei r15, 5 ; shl1add r5, r6, r7 ; st4 r25, r26 }
   112c8:	[0-9a-f]* 	{ movei r5, 5 ; lnk r15 ; st4 r25, r26 }
   112d0:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
   112d8:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
   112e0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; st4 r25, r26 }
   112e8:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 ; st4 r25, r26 }
   112f0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 ; st4 r25, r26 }
   112f8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; nop ; st4 r25, r26 }
   11300:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; jr r15 ; st4 r25, r26 }
   11308:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
   11310:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
   11318:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; xor r15, r16, r17 ; st4 r25, r26 }
   11320:	[0-9a-f]* 	{ mulax r5, r6, r7 ; shli r15, r16, 5 ; st4 r25, r26 }
   11328:	[0-9a-f]* 	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; st4 r25, r26 }
   11330:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
   11338:	[0-9a-f]* 	{ mz r5, r6, r7 ; addi r15, r16, 5 ; st4 r25, r26 }
   11340:	[0-9a-f]* 	{ mz r5, r6, r7 ; shru r15, r16, r17 ; st4 r25, r26 }
   11348:	[0-9a-f]* 	{ nop ; ill ; st4 r25, r26 }
   11350:	[0-9a-f]* 	{ nop ; shl1add r5, r6, r7 ; st4 r25, r26 }
   11358:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
   11360:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
   11368:	[0-9a-f]* 	{ nor r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
   11370:	[0-9a-f]* 	{ or r15, r16, r17 ; st4 r25, r26 }
   11378:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; st4 r25, r26 }
   11380:	[0-9a-f]* 	{ or r5, r6, r7 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   11388:	[0-9a-f]* 	{ pcnt r5, r6 ; or r15, r16, r17 ; st4 r25, r26 }
   11390:	[0-9a-f]* 	{ revbits r5, r6 ; lnk r15 ; st4 r25, r26 }
   11398:	[0-9a-f]* 	{ revbytes r5, r6 ; st4 r25, r26 }
   113a0:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; rotl r15, r16, r17 ; st4 r25, r26 }
   113a8:	[0-9a-f]* 	{ rotl r15, r16, r17 ; shl2addx r5, r6, r7 ; st4 r25, r26 }
   113b0:	[0-9a-f]* 	{ rotl r5, r6, r7 ; movei r15, 5 ; st4 r25, r26 }
   113b8:	[0-9a-f]* 	{ ctz r5, r6 ; rotli r15, r16, 5 ; st4 r25, r26 }
   113c0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; rotli r15, r16, 5 ; st4 r25, r26 }
   113c8:	[0-9a-f]* 	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   113d0:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; st4 r25, r26 }
   113d8:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
   113e0:	[0-9a-f]* 	{ shl r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
   113e8:	[0-9a-f]* 	{ shl1add r15, r16, r17 ; or r5, r6, r7 ; st4 r25, r26 }
   113f0:	[0-9a-f]* 	{ shl1add r5, r6, r7 ; st4 r25, r26 }
   113f8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; st4 r25, r26 }
   11400:	[0-9a-f]* 	{ shl1addx r15, r16, r17 ; shl2addx r5, r6, r7 ; st4 r25, r26 }
   11408:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; movei r15, 5 ; st4 r25, r26 }
   11410:	[0-9a-f]* 	{ ctz r5, r6 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   11418:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   11420:	[0-9a-f]* 	{ shl2add r5, r6, r7 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   11428:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   11430:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
   11438:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
   11440:	[0-9a-f]* 	{ shl3add r15, r16, r17 ; or r5, r6, r7 ; st4 r25, r26 }
   11448:	[0-9a-f]* 	{ shl3add r5, r6, r7 ; st4 r25, r26 }
   11450:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; st4 r25, r26 }
   11458:	[0-9a-f]* 	{ shl3addx r15, r16, r17 ; shl2addx r5, r6, r7 ; st4 r25, r26 }
   11460:	[0-9a-f]* 	{ shl3addx r5, r6, r7 ; movei r15, 5 ; st4 r25, r26 }
   11468:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 ; st4 r25, r26 }
   11470:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shli r15, r16, 5 ; st4 r25, r26 }
   11478:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   11480:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; st4 r25, r26 }
   11488:	[0-9a-f]* 	{ shrs r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
   11490:	[0-9a-f]* 	{ shrs r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
   11498:	[0-9a-f]* 	{ shrsi r15, r16, 5 ; or r5, r6, r7 ; st4 r25, r26 }
   114a0:	[0-9a-f]* 	{ shrsi r5, r6, 5 ; st4 r25, r26 }
   114a8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; shru r15, r16, r17 ; st4 r25, r26 }
   114b0:	[0-9a-f]* 	{ shru r15, r16, r17 ; shl2addx r5, r6, r7 ; st4 r25, r26 }
   114b8:	[0-9a-f]* 	{ shru r5, r6, r7 ; movei r15, 5 ; st4 r25, r26 }
   114c0:	[0-9a-f]* 	{ ctz r5, r6 ; shrui r15, r16, 5 ; st4 r25, r26 }
   114c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; st4 r25, r26 }
   114d0:	[0-9a-f]* 	{ shrui r5, r6, 5 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   114d8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; st4 r25, r26 }
   114e0:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; st4 r25, r26 }
   114e8:	[0-9a-f]* 	{ sub r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
   114f0:	[0-9a-f]* 	{ subx r15, r16, r17 ; or r5, r6, r7 ; st4 r25, r26 }
   114f8:	[0-9a-f]* 	{ subx r5, r6, r7 ; st4 r25, r26 }
   11500:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
   11508:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; st4 r25, r26 }
   11510:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrsi r15, r16, 5 ; st4 r25, r26 }
   11518:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl1addx r15, r16, r17 ; st4 r25, r26 }
   11520:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nop ; st4 r25, r26 }
   11528:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; xor r15, r16, r17 ; st4 r25, r26 }
   11530:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; xor r15, r16, r17 ; st4 r25, r26 }
   11538:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl3add r15, r16, r17 ; st4 r25, r26 }
   11540:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; st4_add r15, r16, 5 }
   11548:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; st4_add r15, r16, 5 }
   11550:	[0-9a-f]* 	{ shruxi r5, r6, 5 ; st4_add r15, r16, 5 }
   11558:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; st4_add r15, r16, 5 }
   11560:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; st4_add r15, r16, 5 }
   11568:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; st_add r15, r16, 5 }
   11570:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; st_add r15, r16, 5 }
   11578:	[0-9a-f]* 	{ revbytes r5, r6 ; st_add r15, r16, 5 }
   11580:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; st_add r15, r16, 5 }
   11588:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; st_add r15, r16, 5 }
   11590:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; st_add r15, r16, 5 }
   11598:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; stnt r15, r16 }
   115a0:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; stnt r15, r16 }
   115a8:	[0-9a-f]* 	{ sub r5, r6, r7 ; stnt r15, r16 }
   115b0:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; stnt r15, r16 }
   115b8:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; stnt r15, r16 }
   115c0:	[0-9a-f]* 	{ clz r5, r6 ; stnt1 r15, r16 }
   115c8:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; stnt1 r15, r16 }
   115d0:	[0-9a-f]* 	{ rotli r5, r6, 5 ; stnt1 r15, r16 }
   115d8:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; stnt1 r15, r16 }
   115e0:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; stnt1 r15, r16 }
   115e8:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; stnt1 r15, r16 }
   115f0:	[0-9a-f]* 	{ ctz r5, r6 ; stnt1_add r15, r16, 5 }
   115f8:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; stnt1_add r15, r16, 5 }
   11600:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; stnt1_add r15, r16, 5 }
   11608:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; stnt1_add r15, r16, 5 }
   11610:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; stnt1_add r15, r16, 5 }
   11618:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; stnt2 r15, r16 }
   11620:	[0-9a-f]* 	{ info 19 ; stnt2 r15, r16 }
   11628:	[0-9a-f]* 	{ shl16insli r5, r6, 4660 ; stnt2 r15, r16 }
   11630:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; stnt2 r15, r16 }
   11638:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; stnt2 r15, r16 }
   11640:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; stnt2 r15, r16 }
   11648:	[0-9a-f]* 	{ dblalign2 r5, r6, r7 ; stnt2_add r15, r16, 5 }
   11650:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; stnt2_add r15, r16, 5 }
   11658:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; stnt2_add r15, r16, 5 }
   11660:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; stnt2_add r15, r16, 5 }
   11668:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; stnt2_add r15, r16, 5 }
   11670:	[0-9a-f]* 	{ cmpeqi r5, r6, 5 ; stnt4 r15, r16 }
   11678:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; stnt4 r15, r16 }
   11680:	[0-9a-f]* 	{ shl1addx r5, r6, r7 ; stnt4 r15, r16 }
   11688:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; stnt4 r15, r16 }
   11690:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; stnt4 r15, r16 }
   11698:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; stnt4 r15, r16 }
   116a0:	[0-9a-f]* 	{ dblalign6 r5, r6, r7 ; stnt4_add r15, r16, 5 }
   116a8:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; stnt4_add r15, r16, 5 }
   116b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; stnt4_add r15, r16, 5 }
   116b8:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; stnt4_add r15, r16, 5 }
   116c0:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; stnt4_add r15, r16, 5 }
   116c8:	[0-9a-f]* 	{ cmpleu r5, r6, r7 ; stnt_add r15, r16, 5 }
   116d0:	[0-9a-f]* 	{ move r5, r6 ; stnt_add r15, r16, 5 }
   116d8:	[0-9a-f]* 	{ shl2addx r5, r6, r7 ; stnt_add r15, r16, 5 }
   116e0:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; stnt_add r15, r16, 5 }
   116e8:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; stnt_add r15, r16, 5 }
   116f0:	[0-9a-f]* 	{ xori r5, r6, 5 ; stnt_add r15, r16, 5 }
   116f8:	[0-9a-f]* 	{ sub r15, r16, r17 ; addx r5, r6, r7 ; ld r25, r26 }
   11700:	[0-9a-f]* 	{ sub r15, r16, r17 ; and r5, r6, r7 ; ld r25, r26 }
   11708:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; sub r15, r16, r17 }
   11710:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
   11718:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
   11720:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4s r25, r26 }
   11728:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
   11730:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
   11738:	[0-9a-f]* 	{ sub r15, r16, r17 ; dblalign2 r5, r6, r7 }
   11740:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; sub r15, r16, r17 ; ld4u r25, r26 }
   11748:	[0-9a-f]* 	{ sub r15, r16, r17 ; andi r5, r6, 5 ; ld r25, r26 }
   11750:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl1addx r5, r6, r7 ; ld r25, r26 }
   11758:	[0-9a-f]* 	{ sub r15, r16, r17 ; move r5, r6 ; ld1s r25, r26 }
   11760:	[0-9a-f]* 	{ sub r15, r16, r17 ; ld1s r25, r26 }
   11768:	[0-9a-f]* 	{ revbits r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
   11770:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
   11778:	[0-9a-f]* 	{ sub r15, r16, r17 ; subx r5, r6, r7 ; ld2s r25, r26 }
   11780:	[0-9a-f]* 	{ mulx r5, r6, r7 ; sub r15, r16, r17 ; ld2u r25, r26 }
   11788:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld4s r25, r26 }
   11790:	[0-9a-f]* 	{ sub r15, r16, r17 ; shli r5, r6, 5 ; ld4s r25, r26 }
   11798:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
   117a0:	[0-9a-f]* 	{ sub r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
   117a8:	[0-9a-f]* 	{ sub r15, r16, r17 ; movei r5, 5 ; ld4s r25, r26 }
   117b0:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; ld2s r25, r26 }
   117b8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
   117c0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
   117c8:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
   117d0:	[0-9a-f]* 	{ mulx r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
   117d8:	[0-9a-f]* 	{ sub r15, r16, r17 ; nop ; ld2u r25, r26 }
   117e0:	[0-9a-f]* 	{ sub r15, r16, r17 ; or r5, r6, r7 ; ld4u r25, r26 }
   117e8:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
   117f0:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
   117f8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
   11800:	[0-9a-f]* 	{ sub r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l1_fault r25 }
   11808:	[0-9a-f]* 	{ sub r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l1_fault r25 }
   11810:	[0-9a-f]* 	{ sub r15, r16, r17 ; prefetch_l2 r25 }
   11818:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sub r15, r16, r17 ; prefetch_l2 r25 }
   11820:	[0-9a-f]* 	{ sub r15, r16, r17 ; nop ; prefetch_l2_fault r25 }
   11828:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l3 r25 }
   11830:	[0-9a-f]* 	{ sub r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l3 r25 }
   11838:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
   11840:	[0-9a-f]* 	{ revbits r5, r6 ; sub r15, r16, r17 ; ld4u r25, r26 }
   11848:	[0-9a-f]* 	{ sub r15, r16, r17 ; rotl r5, r6, r7 ; prefetch r25 }
   11850:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
   11858:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
   11860:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l3_fault r25 }
   11868:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl3addx r5, r6, r7 ; st1 r25, r26 }
   11870:	[0-9a-f]* 	{ sub r15, r16, r17 ; shrs r5, r6, r7 ; st1 r25, r26 }
   11878:	[0-9a-f]* 	{ sub r15, r16, r17 ; shru r5, r6, r7 ; st4 r25, r26 }
   11880:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpne r5, r6, r7 ; st r25, r26 }
   11888:	[0-9a-f]* 	{ sub r15, r16, r17 ; subx r5, r6, r7 ; st r25, r26 }
   11890:	[0-9a-f]* 	{ mulx r5, r6, r7 ; sub r15, r16, r17 ; st1 r25, r26 }
   11898:	[0-9a-f]* 	{ sub r15, r16, r17 ; cmpeqi r5, r6, 5 ; st2 r25, r26 }
   118a0:	[0-9a-f]* 	{ sub r15, r16, r17 ; shli r5, r6, 5 ; st2 r25, r26 }
   118a8:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; st4 r25, r26 }
   118b0:	[0-9a-f]* 	{ sub r15, r16, r17 ; sub r5, r6, r7 ; ld2u r25, r26 }
   118b8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sub r15, r16, r17 ; ld4s r25, r26 }
   118c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sub r15, r16, r17 ; prefetch r25 }
   118c8:	[0-9a-f]* 	{ sub r15, r16, r17 ; v1cmplts r5, r6, r7 }
   118d0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; sub r15, r16, r17 }
   118d8:	[0-9a-f]* 	{ sub r15, r16, r17 ; v4addsc r5, r6, r7 }
   118e0:	[0-9a-f]* 	{ sub r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
   118e8:	[0-9a-f]* 	{ sub r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
   118f0:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
   118f8:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
   11900:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
   11908:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   11910:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
   11918:	[0-9a-f]* 	{ sub r5, r6, r7 ; ld1s r25, r26 }
   11920:	[0-9a-f]* 	{ sub r5, r6, r7 ; info 19 ; ld1u r25, r26 }
   11928:	[0-9a-f]* 	{ sub r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
   11930:	[0-9a-f]* 	{ sub r5, r6, r7 ; jrp r15 ; ld2s r25, r26 }
   11938:	[0-9a-f]* 	{ sub r5, r6, r7 ; move r15, r16 ; ld r25, r26 }
   11940:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; ld1s r25, r26 }
   11948:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
   11950:	[0-9a-f]* 	{ sub r5, r6, r7 ; ld1u r25, r26 }
   11958:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
   11960:	[0-9a-f]* 	{ sub r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
   11968:	[0-9a-f]* 	{ sub r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
   11970:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
   11978:	[0-9a-f]* 	{ sub r5, r6, r7 ; ldna_add r15, r16, 5 }
   11980:	[0-9a-f]* 	{ sub r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
   11988:	[0-9a-f]* 	{ sub r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
   11990:	[0-9a-f]* 	{ sub r5, r6, r7 ; nop ; ld4u r25, r26 }
   11998:	[0-9a-f]* 	{ sub r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
   119a0:	[0-9a-f]* 	{ sub r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
   119a8:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
   119b0:	[0-9a-f]* 	{ sub r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1_fault r25 }
   119b8:	[0-9a-f]* 	{ sub r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
   119c0:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
   119c8:	[0-9a-f]* 	{ sub r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2_fault r25 }
   119d0:	[0-9a-f]* 	{ sub r5, r6, r7 ; lnk r15 ; prefetch_l3 r25 }
   119d8:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
   119e0:	[0-9a-f]* 	{ sub r5, r6, r7 ; rotl r15, r16, r17 ; ld4s r25, r26 }
   119e8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
   119f0:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
   119f8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
   11a00:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
   11a08:	[0-9a-f]* 	{ sub r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
   11a10:	[0-9a-f]* 	{ sub r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
   11a18:	[0-9a-f]* 	{ sub r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
   11a20:	[0-9a-f]* 	{ sub r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
   11a28:	[0-9a-f]* 	{ sub r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
   11a30:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
   11a38:	[0-9a-f]* 	{ sub r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
   11a40:	[0-9a-f]* 	{ sub r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2 r25 }
   11a48:	[0-9a-f]* 	{ sub r5, r6, r7 ; v1cmpne r15, r16, r17 }
   11a50:	[0-9a-f]* 	{ sub r5, r6, r7 ; v2shl r15, r16, r17 }
   11a58:	[0-9a-f]* 	{ sub r5, r6, r7 ; xori r15, r16, 5 }
   11a60:	[0-9a-f]* 	{ subx r15, r16, r17 ; addx r5, r6, r7 ; ld r25, r26 }
   11a68:	[0-9a-f]* 	{ subx r15, r16, r17 ; and r5, r6, r7 ; ld r25, r26 }
   11a70:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; subx r15, r16, r17 }
   11a78:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; subx r15, r16, r17 ; ld1s r25, r26 }
   11a80:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
   11a88:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4s r25, r26 }
   11a90:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
   11a98:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch r25 }
   11aa0:	[0-9a-f]* 	{ subx r15, r16, r17 ; dblalign2 r5, r6, r7 }
   11aa8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; subx r15, r16, r17 ; ld4u r25, r26 }
   11ab0:	[0-9a-f]* 	{ subx r15, r16, r17 ; andi r5, r6, 5 ; ld r25, r26 }
   11ab8:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl1addx r5, r6, r7 ; ld r25, r26 }
   11ac0:	[0-9a-f]* 	{ subx r15, r16, r17 ; move r5, r6 ; ld1s r25, r26 }
   11ac8:	[0-9a-f]* 	{ subx r15, r16, r17 ; ld1s r25, r26 }
   11ad0:	[0-9a-f]* 	{ revbits r5, r6 ; subx r15, r16, r17 ; ld1u r25, r26 }
   11ad8:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
   11ae0:	[0-9a-f]* 	{ subx r15, r16, r17 ; subx r5, r6, r7 ; ld2s r25, r26 }
   11ae8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; subx r15, r16, r17 ; ld2u r25, r26 }
   11af0:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld4s r25, r26 }
   11af8:	[0-9a-f]* 	{ subx r15, r16, r17 ; shli r5, r6, 5 ; ld4s r25, r26 }
   11b00:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; ld4u r25, r26 }
   11b08:	[0-9a-f]* 	{ subx r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
   11b10:	[0-9a-f]* 	{ subx r15, r16, r17 ; movei r5, 5 ; ld4s r25, r26 }
   11b18:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; ld2s r25, r26 }
   11b20:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; ld1u r25, r26 }
   11b28:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; ld1s r25, r26 }
   11b30:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
   11b38:	[0-9a-f]* 	{ mulx r5, r6, r7 ; subx r15, r16, r17 ; ld1u r25, r26 }
   11b40:	[0-9a-f]* 	{ subx r15, r16, r17 ; nop ; ld2u r25, r26 }
   11b48:	[0-9a-f]* 	{ subx r15, r16, r17 ; or r5, r6, r7 ; ld4u r25, r26 }
   11b50:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
   11b58:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
   11b60:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
   11b68:	[0-9a-f]* 	{ subx r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l1_fault r25 }
   11b70:	[0-9a-f]* 	{ subx r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l1_fault r25 }
   11b78:	[0-9a-f]* 	{ subx r15, r16, r17 ; prefetch_l2 r25 }
   11b80:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; subx r15, r16, r17 ; prefetch_l2 r25 }
   11b88:	[0-9a-f]* 	{ subx r15, r16, r17 ; nop ; prefetch_l2_fault r25 }
   11b90:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l3 r25 }
   11b98:	[0-9a-f]* 	{ subx r15, r16, r17 ; shrsi r5, r6, 5 ; prefetch_l3 r25 }
   11ba0:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3_fault r25 }
   11ba8:	[0-9a-f]* 	{ revbits r5, r6 ; subx r15, r16, r17 ; ld4u r25, r26 }
   11bb0:	[0-9a-f]* 	{ subx r15, r16, r17 ; rotl r5, r6, r7 ; prefetch r25 }
   11bb8:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
   11bc0:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
   11bc8:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l3_fault r25 }
   11bd0:	[0-9a-f]* 	{ subx r15, r16, r17 ; shl3addx r5, r6, r7 ; st1 r25, r26 }
   11bd8:	[0-9a-f]* 	{ subx r15, r16, r17 ; shrs r5, r6, r7 ; st1 r25, r26 }
   11be0:	[0-9a-f]* 	{ subx r15, r16, r17 ; shru r5, r6, r7 ; st4 r25, r26 }
   11be8:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpne r5, r6, r7 ; st r25, r26 }
   11bf0:	[0-9a-f]* 	{ subx r15, r16, r17 ; subx r5, r6, r7 ; st r25, r26 }
   11bf8:	[0-9a-f]* 	{ mulx r5, r6, r7 ; subx r15, r16, r17 ; st1 r25, r26 }
   11c00:	[0-9a-f]* 	{ subx r15, r16, r17 ; cmpeqi r5, r6, 5 ; st2 r25, r26 }
   11c08:	[0-9a-f]* 	{ subx r15, r16, r17 ; shli r5, r6, 5 ; st2 r25, r26 }
   11c10:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; st4 r25, r26 }
   11c18:	[0-9a-f]* 	{ subx r15, r16, r17 ; sub r5, r6, r7 ; ld2u r25, r26 }
   11c20:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; subx r15, r16, r17 ; ld4s r25, r26 }
   11c28:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; subx r15, r16, r17 ; prefetch r25 }
   11c30:	[0-9a-f]* 	{ subx r15, r16, r17 ; v1cmplts r5, r6, r7 }
   11c38:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; subx r15, r16, r17 }
   11c40:	[0-9a-f]* 	{ subx r15, r16, r17 ; v4addsc r5, r6, r7 }
   11c48:	[0-9a-f]* 	{ subx r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
   11c50:	[0-9a-f]* 	{ subx r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
   11c58:	[0-9a-f]* 	{ subx r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
   11c60:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
   11c68:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
   11c70:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
   11c78:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
   11c80:	[0-9a-f]* 	{ subx r5, r6, r7 ; ld1s r25, r26 }
   11c88:	[0-9a-f]* 	{ subx r5, r6, r7 ; info 19 ; ld1u r25, r26 }
   11c90:	[0-9a-f]* 	{ subx r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
   11c98:	[0-9a-f]* 	{ subx r5, r6, r7 ; jrp r15 ; ld2s r25, r26 }
   11ca0:	[0-9a-f]* 	{ subx r5, r6, r7 ; move r15, r16 ; ld r25, r26 }
   11ca8:	[0-9a-f]* 	{ subx r5, r6, r7 ; ill ; ld1s r25, r26 }
   11cb0:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
   11cb8:	[0-9a-f]* 	{ subx r5, r6, r7 ; ld1u r25, r26 }
   11cc0:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
   11cc8:	[0-9a-f]* 	{ subx r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
   11cd0:	[0-9a-f]* 	{ subx r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
   11cd8:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
   11ce0:	[0-9a-f]* 	{ subx r5, r6, r7 ; ldna_add r15, r16, 5 }
   11ce8:	[0-9a-f]* 	{ subx r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
   11cf0:	[0-9a-f]* 	{ subx r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
   11cf8:	[0-9a-f]* 	{ subx r5, r6, r7 ; nop ; ld4u r25, r26 }
   11d00:	[0-9a-f]* 	{ subx r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
   11d08:	[0-9a-f]* 	{ subx r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
   11d10:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch r25 }
   11d18:	[0-9a-f]* 	{ subx r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1_fault r25 }
   11d20:	[0-9a-f]* 	{ subx r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
   11d28:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l2 r25 }
   11d30:	[0-9a-f]* 	{ subx r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2_fault r25 }
   11d38:	[0-9a-f]* 	{ subx r5, r6, r7 ; lnk r15 ; prefetch_l3 r25 }
   11d40:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
   11d48:	[0-9a-f]* 	{ subx r5, r6, r7 ; rotl r15, r16, r17 ; ld4s r25, r26 }
   11d50:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
   11d58:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch r25 }
   11d60:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
   11d68:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
   11d70:	[0-9a-f]* 	{ subx r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
   11d78:	[0-9a-f]* 	{ subx r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
   11d80:	[0-9a-f]* 	{ subx r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
   11d88:	[0-9a-f]* 	{ subx r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
   11d90:	[0-9a-f]* 	{ subx r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
   11d98:	[0-9a-f]* 	{ subx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
   11da0:	[0-9a-f]* 	{ subx r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
   11da8:	[0-9a-f]* 	{ subx r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2 r25 }
   11db0:	[0-9a-f]* 	{ subx r5, r6, r7 ; v1cmpne r15, r16, r17 }
   11db8:	[0-9a-f]* 	{ subx r5, r6, r7 ; v2shl r15, r16, r17 }
   11dc0:	[0-9a-f]* 	{ subx r5, r6, r7 ; xori r15, r16, 5 }
   11dc8:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; subxsc r15, r16, r17 }
   11dd0:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; subxsc r15, r16, r17 }
   11dd8:	[0-9a-f]* 	{ subxsc r15, r16, r17 ; v1addi r5, r6, 5 }
   11de0:	[0-9a-f]* 	{ subxsc r15, r16, r17 ; v1shru r5, r6, r7 }
   11de8:	[0-9a-f]* 	{ subxsc r15, r16, r17 ; v2shlsc r5, r6, r7 }
   11df0:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; dblalign2 r15, r16, r17 }
   11df8:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; ld4u_add r15, r16, 5 }
   11e00:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; prefetch_l2 r15 }
   11e08:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; sub r15, r16, r17 }
   11e10:	[0-9a-f]* 	{ subxsc r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   11e18:	[0-9a-f]* 	{ swint3 }
   11e20:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
   11e28:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
   11e30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
   11e38:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmples r15, r16, r17 ; ld1u r25, r26 }
   11e40:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
   11e48:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
   11e50:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; fetchadd4 r15, r16, r17 }
   11e58:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; prefetch_l2 r25 }
   11e60:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalr r15 ; prefetch_l1_fault r25 }
   11e68:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jr r15 ; prefetch_l2_fault r25 }
   11e70:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmpltu r15, r16, r17 ; ld r25, r26 }
   11e78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld1s r25, r26 }
   11e80:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; subx r15, r16, r17 ; ld1s r25, r26 }
   11e88:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
   11e90:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nop ; ld2s r25, r26 }
   11e98:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalr r15 ; ld2u r25, r26 }
   11ea0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
   11ea8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ld4u r15, r16 }
   11eb0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; ld4u r25, r26 }
   11eb8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lnk r15 ; st r25, r26 }
   11ec0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; st r25, r26 }
   11ec8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
   11ed0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
   11ed8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; prefetch r25 }
   11ee0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
   11ee8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
   11ef0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2add r15, r16, r17 ; prefetch_l1_fault r25 }
   11ef8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nop ; prefetch_l2 r25 }
   11f00:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalrp r15 ; prefetch_l2_fault r25 }
   11f08:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l3 r25 }
   11f10:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
   11f18:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
   11f20:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; rotli r15, r16, 5 ; st1 r25, r26 }
   11f28:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   11f30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2add r15, r16, r17 }
   11f38:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
   11f40:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; ld1s r25, r26 }
   11f48:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shru r15, r16, r17 ; ld2s r25, r26 }
   11f50:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; st r25, r26 }
   11f58:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shrui r15, r16, 5 ; st r25, r26 }
   11f60:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl2add r15, r16, r17 ; st1 r25, r26 }
   11f68:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; st2 r25, r26 }
   11f70:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; st4 r25, r26 }
   11f78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; stnt_add r15, r16, 5 }
   11f80:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; v1add r15, r16, r17 }
   11f88:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; v2int_h r15, r16, r17 }
   11f90:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
   11f98:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
   11fa0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
   11fa8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; prefetch_l2 r25 }
   11fb0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l3 r25 }
   11fb8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
   11fc0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 ; st r25, r26 }
   11fc8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpne r15, r16, r17 ; st1 r25, r26 }
   11fd0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; icoh r15 }
   11fd8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; inv r15 }
   11fe0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jr r15 ; ld r25, r26 }
   11fe8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; ld r25, r26 }
   11ff0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shru r15, r16, r17 ; ld r25, r26 }
   11ff8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
   12000:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; movei r15, 5 ; ld1u r25, r26 }
   12008:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; ld2s r25, r26 }
   12010:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeq r15, r16, r17 ; ld2u r25, r26 }
   12018:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ld2u r25, r26 }
   12020:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
   12028:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; ld4u r25, r26 }
   12030:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; lnk r15 ; ld2s r25, r26 }
   12038:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; ld2s r25, r26 }
   12040:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; ld2s r25, r26 }
   12048:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; ld4s r25, r26 }
   12050:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
   12058:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
   12060:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
   12068:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; prefetch_l1_fault r25 }
   12070:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; prefetch_l2 r25 }
   12078:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
   12080:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; prefetch_l3 r15 }
   12088:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
   12090:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; prefetch_l3_fault r25 }
   12098:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rotli r15, r16, 5 ; ld2u r25, r26 }
   120a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
   120a8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl2add r15, r16, r17 ; prefetch r25 }
   120b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l1_fault r25 }
   120b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
   120c0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrsi r15, r16, 5 ; prefetch_l2_fault r25 }
   120c8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
   120d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; st r25, r26 }
   120d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; st1 r25, r26 }
   120e0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; st2 r25, r26 }
   120e8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; st4 r25, r26 }
   120f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; st4 r25, r26 }
   120f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
   12100:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; v2addi r15, r16, 5 }
   12108:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; v4sub r15, r16, r17 }
   12110:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; add r15, r16, r17 ; st4 r25, r26 }
   12118:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addx r15, r16, r17 }
   12120:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 }
   12128:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
   12130:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
   12138:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltsi r15, r16, 5 ; ld2s r25, r26 }
   12140:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
   12148:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; prefetch r25 }
   12150:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; info 19 ; prefetch_l1_fault r25 }
   12158:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jalrp r15 ; prefetch r25 }
   12160:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; prefetch_l2 r25 }
   12168:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rotli r15, r16, 5 ; ld r25, r26 }
   12170:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; ld1s r25, r26 }
   12178:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpne r15, r16, r17 ; ld1u r25, r26 }
   12180:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
   12188:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; subx r15, r16, r17 ; ld2s r25, r26 }
   12190:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
   12198:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; ld4s r25, r26 }
   121a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jalr r15 ; ld4u r25, r26 }
   121a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ldnt2s_add r15, r16, 5 }
   121b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
   121b8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; prefetch_l3_fault r25 }
   121c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; prefetch_l3_fault r25 }
   121c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; st1 r25, r26 }
   121d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl2add r15, r16, r17 ; prefetch r25 }
   121d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; prefetch r25 }
   121e0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
   121e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; prefetch_l2 r25 }
   121f0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; subx r15, r16, r17 ; prefetch_l2 r25 }
   121f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l2_fault r25 }
   12200:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; prefetch_l3 r25 }
   12208:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; prefetch_l3_fault r25 }
   12210:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rotl r15, r16, r17 ; prefetch_l3 r25 }
   12218:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl r15, r16, r17 ; st r25, r26 }
   12220:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl1addx r15, r16, r17 ; st1 r25, r26 }
   12228:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
   12230:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; ld r25, r26 }
   12238:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrsi r15, r16, 5 ; ld r25, r26 }
   12240:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrui r15, r16, 5 ; ld1u r25, r26 }
   12248:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; jrp r15 ; st r25, r26 }
   12250:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; cmpltu r15, r16, r17 ; st1 r25, r26 }
   12258:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addxi r15, r16, 5 ; st2 r25, r26 }
   12260:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sub r15, r16, r17 ; st2 r25, r26 }
   12268:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl2add r15, r16, r17 ; st4 r25, r26 }
   12270:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sub r15, r16, r17 ; st4 r25, r26 }
   12278:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; v1mnz r15, r16, r17 }
   12280:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; v2sub r15, r16, r17 }
   12288:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; ld4u r25, r26 }
   12290:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
   12298:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
   122a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
   122a8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
   122b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
   122b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
   122c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; finv r15 }
   122c8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; st4 r25, r26 }
   122d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 ; st2 r25, r26 }
   122d8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jr r15 }
   122e0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jr r15 ; ld r25, r26 }
   122e8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
   122f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; ld1u r25, r26 }
   122f8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrui r15, r16, 5 ; ld1u r25, r26 }
   12300:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
   12308:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; movei r15, 5 ; ld2u r25, r26 }
   12310:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; ld4s r25, r26 }
   12318:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
   12320:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ld4u r25, r26 }
   12328:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; ld r25, r26 }
   12330:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; movei r15, 5 ; ld1u r25, r26 }
   12338:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nop ; ld1u r25, r26 }
   12340:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; or r15, r16, r17 ; ld2u r25, r26 }
   12348:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; prefetch r25 }
   12350:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpleu r15, r16, r17 ; prefetch r25 }
   12358:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
   12360:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
   12368:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
   12370:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; prefetch_l2_fault r25 }
   12378:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 ; prefetch_l3 r25 }
   12380:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
   12388:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rotl r15, r16, r17 ; ld1s r25, r26 }
   12390:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; ld2s r25, r26 }
   12398:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
   123a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
   123a8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl3addx r15, r16, r17 ; prefetch r25 }
   123b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
   123b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; prefetch_l2 r25 }
   123c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; cmpleu r15, r16, r17 ; st r25, r26 }
   123c8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addi r15, r16, 5 ; st1 r25, r26 }
   123d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; st1 r25, r26 }
   123d8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl1add r15, r16, r17 ; st2 r25, r26 }
   123e0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; st4 r25, r26 }
   123e8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; ld4u r25, r26 }
   123f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; v1cmplts r15, r16, r17 }
   123f8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; v2mz r15, r16, r17 }
   12400:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; st1 r25, r26 }
   12408:	[0-9a-f]* 	{ v1add r15, r16, r17 ; dblalign2 r5, r6, r7 }
   12410:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; v1add r15, r16, r17 }
   12418:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; v1add r15, r16, r17 }
   12420:	[0-9a-f]* 	{ v1add r15, r16, r17 ; v1shl r5, r6, r7 }
   12428:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; v1add r15, r16, r17 }
   12430:	[0-9a-f]* 	{ v1add r5, r6, r7 ; cmpltsi r15, r16, 5 }
   12438:	[0-9a-f]* 	{ v1add r5, r6, r7 ; ld2u_add r15, r16, 5 }
   12440:	[0-9a-f]* 	{ v1add r5, r6, r7 ; prefetch_add_l3 r15, 5 }
   12448:	[0-9a-f]* 	{ v1add r5, r6, r7 ; stnt2_add r15, r16, 5 }
   12450:	[0-9a-f]* 	{ v1add r5, r6, r7 ; v2cmples r15, r16, r17 }
   12458:	[0-9a-f]* 	{ v1add r5, r6, r7 ; xori r15, r16, 5 }
   12460:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v1addi r15, r16, 5 }
   12468:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v1addi r15, r16, 5 }
   12470:	[0-9a-f]* 	{ v1addi r15, r16, 5 ; v1addi r5, r6, 5 }
   12478:	[0-9a-f]* 	{ v1addi r15, r16, 5 ; v1shru r5, r6, r7 }
   12480:	[0-9a-f]* 	{ v1addi r15, r16, 5 ; v2shlsc r5, r6, r7 }
   12488:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; dblalign2 r15, r16, r17 }
   12490:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; ld4u_add r15, r16, 5 }
   12498:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; prefetch_l2 r15 }
   124a0:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; sub r15, r16, r17 }
   124a8:	[0-9a-f]* 	{ v1addi r5, r6, 5 ; v2cmpltu r15, r16, r17 }
   124b0:	[0-9a-f]* 	{ v1adduc r15, r16, r17 ; addx r5, r6, r7 }
   124b8:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v1adduc r15, r16, r17 }
   124c0:	[0-9a-f]* 	{ v1adduc r15, r16, r17 ; mz r5, r6, r7 }
   124c8:	[0-9a-f]* 	{ v1adduc r15, r16, r17 ; v1cmpeq r5, r6, r7 }
   124d0:	[0-9a-f]* 	{ v1adduc r15, r16, r17 ; v2add r5, r6, r7 }
   124d8:	[0-9a-f]* 	{ v1adduc r15, r16, r17 ; v2shrui r5, r6, 5 }
   124e0:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; exch r15, r16, r17 }
   124e8:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; ldnt r15, r16 }
   124f0:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; raise }
   124f8:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; v1addi r15, r16, 5 }
   12500:	[0-9a-f]* 	{ v1adduc r5, r6, r7 ; v2int_l r15, r16, r17 }
   12508:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; and r15, r16, r17 }
   12510:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; jrp r15 }
   12518:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; nop }
   12520:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; st2 r15, r16 }
   12528:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; v1shru r15, r16, r17 }
   12530:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; v4packsc r15, r16, r17 }
   12538:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; fetchand r15, r16, r17 }
   12540:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
   12548:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; shl1addx r15, r16, r17 }
   12550:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; v1cmplts r15, r16, r17 }
   12558:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; v2mz r15, r16, r17 }
   12560:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; v1cmpeq r15, r16, r17 }
   12568:	[0-9a-f]* 	{ fsingle_sub1 r5, r6, r7 ; v1cmpeq r15, r16, r17 }
   12570:	[0-9a-f]* 	{ v1cmpeq r15, r16, r17 ; shl r5, r6, r7 }
   12578:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; v1cmpeq r15, r16, r17 }
   12580:	[0-9a-f]* 	{ v1cmpeq r15, r16, r17 ; v2cmpltsi r5, r6, 5 }
   12588:	[0-9a-f]* 	{ v1cmpeq r15, r16, r17 ; v4shrs r5, r6, r7 }
   12590:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; finv r15 }
   12598:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
   125a0:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 }
   125a8:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; v1cmpne r15, r16, r17 }
   125b0:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; v2shl r15, r16, r17 }
   125b8:	[0-9a-f]* 	{ v1cmpeqi r15, r16, 5 ; cmples r5, r6, r7 }
   125c0:	[0-9a-f]* 	{ v1cmpeqi r15, r16, 5 ; mnz r5, r6, r7 }
   125c8:	[0-9a-f]* 	{ v1cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 }
   125d0:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
   125d8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
   125e0:	[0-9a-f]* 	{ v1cmpeqi r15, r16, 5 ; xor r5, r6, r7 }
   125e8:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; icoh r15 }
   125f0:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; lnk r15 }
   125f8:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; shrs r15, r16, r17 }
   12600:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; v1maxui r15, r16, 5 }
   12608:	[0-9a-f]* 	{ v1cmpeqi r5, r6, 5 ; v2shrsi r15, r16, 5 }
   12610:	[0-9a-f]* 	{ v1cmples r15, r16, r17 ; cmpltu r5, r6, r7 }
   12618:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; v1cmples r15, r16, r17 }
   12620:	[0-9a-f]* 	{ v1cmples r15, r16, r17 ; shli r5, r6, 5 }
   12628:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; v1cmples r15, r16, r17 }
   12630:	[0-9a-f]* 	{ v1cmples r15, r16, r17 ; v2maxs r5, r6, r7 }
   12638:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; addli r15, r16, 4660 }
   12640:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; inv r15 }
   12648:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; move r15, r16 }
   12650:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; shrux r15, r16, r17 }
   12658:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; v1mz r15, r16, r17 }
   12660:	[0-9a-f]* 	{ v1cmples r5, r6, r7 ; v2subsc r15, r16, r17 }
   12668:	[0-9a-f]* 	{ cmula r5, r6, r7 ; v1cmpleu r15, r16, r17 }
   12670:	[0-9a-f]* 	{ mul_hu_hu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
   12678:	[0-9a-f]* 	{ v1cmpleu r15, r16, r17 ; shrsi r5, r6, 5 }
   12680:	[0-9a-f]* 	{ v1cmpleu r15, r16, r17 ; v1maxui r5, r6, 5 }
   12688:	[0-9a-f]* 	{ v1cmpleu r15, r16, r17 ; v2mnz r5, r6, r7 }
   12690:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; addxsc r15, r16, r17 }
   12698:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; jr r15 }
   126a0:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; mz r15, r16, r17 }
   126a8:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; st1_add r15, r16, 5 }
   126b0:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; v1shrsi r15, r16, 5 }
   126b8:	[0-9a-f]* 	{ v1cmpleu r5, r6, r7 ; v4int_l r15, r16, r17 }
   126c0:	[0-9a-f]* 	{ cmulh r5, r6, r7 ; v1cmplts r15, r16, r17 }
   126c8:	[0-9a-f]* 	{ mul_ls_lu r5, r6, r7 ; v1cmplts r15, r16, r17 }
   126d0:	[0-9a-f]* 	{ v1cmplts r15, r16, r17 ; shruxi r5, r6, 5 }
   126d8:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; v1cmplts r15, r16, r17 }
   126e0:	[0-9a-f]* 	{ v1cmplts r15, r16, r17 ; v2mz r5, r6, r7 }
   126e8:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; cmpeqi r15, r16, 5 }
   126f0:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; ld1s_add r15, r16, 5 }
   126f8:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; ori r15, r16, 5 }
   12700:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; st4_add r15, r16, 5 }
   12708:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; v1subuc r15, r16, r17 }
   12710:	[0-9a-f]* 	{ v1cmplts r5, r6, r7 ; v4shrs r15, r16, r17 }
   12718:	[0-9a-f]* 	{ ctz r5, r6 ; v1cmpltsi r15, r16, 5 }
   12720:	[0-9a-f]* 	{ mula_hs_ls r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
   12728:	[0-9a-f]* 	{ v1cmpltsi r15, r16, 5 ; subxsc r5, r6, r7 }
   12730:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
   12738:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
   12740:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; cmpleu r15, r16, r17 }
   12748:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; ld2s_add r15, r16, 5 }
   12750:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; prefetch_add_l2 r15, 5 }
   12758:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; stnt1_add r15, r16, 5 }
   12760:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; v2cmpeq r15, r16, r17 }
   12768:	[0-9a-f]* 	{ v1cmpltsi r5, r6, 5 ; wh64 r15 }
   12770:	[0-9a-f]* 	{ v1cmpltu r15, r16, r17 ; dblalign6 r5, r6, r7 }
   12778:	[0-9a-f]* 	{ mula_hu_lu r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   12780:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; v1cmpltu r15, r16, r17 }
   12788:	[0-9a-f]* 	{ v1cmpltu r15, r16, r17 ; v1shrs r5, r6, r7 }
   12790:	[0-9a-f]* 	{ v1cmpltu r15, r16, r17 ; v2shl r5, r6, r7 }
   12798:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; cmpltui r15, r16, 5 }
   127a0:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; ld4s_add r15, r16, 5 }
   127a8:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; prefetch r15 }
   127b0:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; stnt4_add r15, r16, 5 }
   127b8:	[0-9a-f]* 	{ v1cmpltu r5, r6, r7 ; v2cmplts r15, r16, r17 }
   127c0:	[0-9a-f]* 	{ v1cmpltui r15, r16, 5 ; addi r5, r6, 5 }
   127c8:	[0-9a-f]* 	{ fdouble_pack1 r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   127d0:	[0-9a-f]* 	{ mulax r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   127d8:	[0-9a-f]* 	{ v1adiffu r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   127e0:	[0-9a-f]* 	{ v1cmpltui r15, r16, 5 ; v1sub r5, r6, r7 }
   127e8:	[0-9a-f]* 	{ v1cmpltui r15, r16, 5 ; v2shrsi r5, r6, 5 }
   127f0:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; dblalign6 r15, r16, r17 }
   127f8:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; ldna r15, r16 }
   12800:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; prefetch_l3 r15 }
   12808:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; subxsc r15, r16, r17 }
   12810:	[0-9a-f]* 	{ v1cmpltui r5, r6, 5 ; v2cmpne r15, r16, r17 }
   12818:	[0-9a-f]* 	{ v1cmpne r15, r16, r17 ; addxli r5, r6, 4660 }
   12820:	[0-9a-f]* 	{ fdouble_unpack_min r5, r6, r7 ; v1cmpne r15, r16, r17 }
   12828:	[0-9a-f]* 	{ v1cmpne r15, r16, r17 ; nor r5, r6, r7 }
   12830:	[0-9a-f]* 	{ v1cmpne r15, r16, r17 ; v1cmples r5, r6, r7 }
   12838:	[0-9a-f]* 	{ v1cmpne r15, r16, r17 ; v2addsc r5, r6, r7 }
   12840:	[0-9a-f]* 	{ v1cmpne r15, r16, r17 ; v2subsc r5, r6, r7 }
   12848:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; fetchadd r15, r16, r17 }
   12850:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
   12858:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; rotli r15, r16, 5 }
   12860:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; v1cmpeq r15, r16, r17 }
   12868:	[0-9a-f]* 	{ v1cmpne r5, r6, r7 ; v2maxsi r15, r16, 5 }
   12870:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; cmpeq r15, r16, r17 }
   12878:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; ld1s r15, r16 }
   12880:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; or r15, r16, r17 }
   12888:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; st4 r15, r16 }
   12890:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; v1sub r15, r16, r17 }
   12898:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; v4shlsc r15, r16, r17 }
   128a0:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; fetchor r15, r16, r17 }
   128a8:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
   128b0:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; shl2addx r15, r16, r17 }
   128b8:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   128c0:	[0-9a-f]* 	{ v1ddotpua r5, r6, r7 ; v2packl r15, r16, r17 }
   128c8:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; cmplts r15, r16, r17 }
   128d0:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; ld2u r15, r16 }
   128d8:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
   128e0:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; stnt2 r15, r16 }
   128e8:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
   128f0:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; xor r15, r16, r17 }
   128f8:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; icoh r15 }
   12900:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; lnk r15 }
   12908:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; shrs r15, r16, r17 }
   12910:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v1maxui r15, r16, 5 }
   12918:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v2shrsi r15, r16, 5 }
   12920:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; dblalign4 r15, r16, r17 }
   12928:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; ld_add r15, r16, 5 }
   12930:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; prefetch_l2_fault r15 }
   12938:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; subx r15, r16, r17 }
   12940:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; v2cmpltui r15, r16, 5 }
   12948:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; addxi r15, r16, 5 }
   12950:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; jalr r15 }
   12958:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; moveli r15, 4660 }
   12960:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; st r15, r16 }
   12968:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; v1shli r15, r16, 5 }
   12970:	[0-9a-f]* 	{ v1dotpa r5, r6, r7 ; v4addsc r15, r16, r17 }
   12978:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; fetchadd4 r15, r16, r17 }
   12980:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; ldnt1u r15, r16 }
   12988:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; shl r15, r16, r17 }
   12990:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
   12998:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; v2mins r15, r16, r17 }
   129a0:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; cmpeqi r15, r16, 5 }
   129a8:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; ld1s_add r15, r16, 5 }
   129b0:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; ori r15, r16, 5 }
   129b8:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; st4_add r15, r16, 5 }
   129c0:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v1subuc r15, r16, r17 }
   129c8:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v4shrs r15, r16, r17 }
   129d0:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; fetchor4 r15, r16, r17 }
   129d8:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; ldnt4s r15, r16 }
   129e0:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; shl3add r15, r16, r17 }
   129e8:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   129f0:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; v2packuc r15, r16, r17 }
   129f8:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; cmpltsi r15, r16, 5 }
   12a00:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; ld2u_add r15, r16, 5 }
   12a08:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; prefetch_add_l3 r15, 5 }
   12a10:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; stnt2_add r15, r16, 5 }
   12a18:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; v2cmples r15, r16, r17 }
   12a20:	[0-9a-f]* 	{ v1dotpusa r5, r6, r7 ; xori r15, r16, 5 }
   12a28:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v1int_h r15, r16, r17 }
   12a30:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v1int_h r15, r16, r17 }
   12a38:	[0-9a-f]* 	{ v1int_h r15, r16, r17 ; v1addi r5, r6, 5 }
   12a40:	[0-9a-f]* 	{ v1int_h r15, r16, r17 ; v1shru r5, r6, r7 }
   12a48:	[0-9a-f]* 	{ v1int_h r15, r16, r17 ; v2shlsc r5, r6, r7 }
   12a50:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; dblalign2 r15, r16, r17 }
   12a58:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; ld4u_add r15, r16, 5 }
   12a60:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; prefetch_l2 r15 }
   12a68:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; sub r15, r16, r17 }
   12a70:	[0-9a-f]* 	{ v1int_h r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   12a78:	[0-9a-f]* 	{ v1int_l r15, r16, r17 ; addx r5, r6, r7 }
   12a80:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v1int_l r15, r16, r17 }
   12a88:	[0-9a-f]* 	{ v1int_l r15, r16, r17 ; mz r5, r6, r7 }
   12a90:	[0-9a-f]* 	{ v1int_l r15, r16, r17 ; v1cmpeq r5, r6, r7 }
   12a98:	[0-9a-f]* 	{ v1int_l r15, r16, r17 ; v2add r5, r6, r7 }
   12aa0:	[0-9a-f]* 	{ v1int_l r15, r16, r17 ; v2shrui r5, r6, 5 }
   12aa8:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; exch r15, r16, r17 }
   12ab0:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; ldnt r15, r16 }
   12ab8:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; raise }
   12ac0:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; v1addi r15, r16, 5 }
   12ac8:	[0-9a-f]* 	{ v1int_l r5, r6, r7 ; v2int_l r15, r16, r17 }
   12ad0:	[0-9a-f]* 	{ v1maxu r15, r16, r17 ; and r5, r6, r7 }
   12ad8:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; v1maxu r15, r16, r17 }
   12ae0:	[0-9a-f]* 	{ v1maxu r15, r16, r17 ; ori r5, r6, 5 }
   12ae8:	[0-9a-f]* 	{ v1maxu r15, r16, r17 ; v1cmplts r5, r6, r7 }
   12af0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; v1maxu r15, r16, r17 }
   12af8:	[0-9a-f]* 	{ v1maxu r15, r16, r17 ; v4addsc r5, r6, r7 }
   12b00:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; fetchaddgez r15, r16, r17 }
   12b08:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
   12b10:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; shl16insli r15, r16, 4660 }
   12b18:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; v1cmples r15, r16, r17 }
   12b20:	[0-9a-f]* 	{ v1maxu r5, r6, r7 ; v2minsi r15, r16, 5 }
   12b28:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; v1maxui r15, r16, 5 }
   12b30:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; v1maxui r15, r16, 5 }
   12b38:	[0-9a-f]* 	{ v1maxui r15, r16, 5 ; rotl r5, r6, r7 }
   12b40:	[0-9a-f]* 	{ v1maxui r15, r16, 5 ; v1cmpne r5, r6, r7 }
   12b48:	[0-9a-f]* 	{ v1maxui r15, r16, 5 ; v2cmpleu r5, r6, r7 }
   12b50:	[0-9a-f]* 	{ v1maxui r15, r16, 5 ; v4shl r5, r6, r7 }
   12b58:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; fetchor r15, r16, r17 }
   12b60:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; ldnt2u_add r15, r16, 5 }
   12b68:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; shl2addx r15, r16, r17 }
   12b70:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; v1cmpltu r15, r16, r17 }
   12b78:	[0-9a-f]* 	{ v1maxui r5, r6, 5 ; v2packl r15, r16, r17 }
   12b80:	[0-9a-f]* 	{ v1minu r15, r16, r17 ; cmpeq r5, r6, r7 }
   12b88:	[0-9a-f]* 	{ v1minu r15, r16, r17 ; infol 4660 }
   12b90:	[0-9a-f]* 	{ v1minu r15, r16, r17 ; shl1add r5, r6, r7 }
   12b98:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v1minu r15, r16, r17 }
   12ba0:	[0-9a-f]* 	{ v1minu r15, r16, r17 ; v2cmpltui r5, r6, 5 }
   12ba8:	[0-9a-f]* 	{ v1minu r15, r16, r17 ; v4sub r5, r6, r7 }
   12bb0:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; flushwb }
   12bb8:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
   12bc0:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; shlx r15, r16, r17 }
   12bc8:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; v1int_l r15, r16, r17 }
   12bd0:	[0-9a-f]* 	{ v1minu r5, r6, r7 ; v2shlsc r15, r16, r17 }
   12bd8:	[0-9a-f]* 	{ v1minui r15, r16, 5 ; cmplts r5, r6, r7 }
   12be0:	[0-9a-f]* 	{ v1minui r15, r16, 5 ; movei r5, 5 }
   12be8:	[0-9a-f]* 	{ v1minui r15, r16, 5 ; shl3add r5, r6, r7 }
   12bf0:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v1minui r15, r16, 5 }
   12bf8:	[0-9a-f]* 	{ v1minui r15, r16, 5 ; v2int_h r5, r6, r7 }
   12c00:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; add r15, r16, r17 }
   12c08:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; info 19 }
   12c10:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
   12c18:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; shru r15, r16, r17 }
   12c20:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; v1minui r15, r16, 5 }
   12c28:	[0-9a-f]* 	{ v1minui r5, r6, 5 ; v2shrui r15, r16, 5 }
   12c30:	[0-9a-f]* 	{ v1mnz r15, r16, r17 ; cmpne r5, r6, r7 }
   12c38:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v1mnz r15, r16, r17 }
   12c40:	[0-9a-f]* 	{ v1mnz r15, r16, r17 ; shlxi r5, r6, 5 }
   12c48:	[0-9a-f]* 	{ v1mnz r15, r16, r17 ; v1int_l r5, r6, r7 }
   12c50:	[0-9a-f]* 	{ v1mnz r15, r16, r17 ; v2mins r5, r6, r7 }
   12c58:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; addxi r15, r16, 5 }
   12c60:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; jalr r15 }
   12c68:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; moveli r15, 4660 }
   12c70:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; st r15, r16 }
   12c78:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; v1shli r15, r16, 5 }
   12c80:	[0-9a-f]* 	{ v1mnz r5, r6, r7 ; v4addsc r15, r16, r17 }
   12c88:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; fetchadd4 r15, r16, r17 }
   12c90:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; ldnt1u r15, r16 }
   12c98:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; shl r15, r16, r17 }
   12ca0:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
   12ca8:	[0-9a-f]* 	{ v1multu r5, r6, r7 ; v2mins r15, r16, r17 }
   12cb0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; cmpeqi r15, r16, 5 }
   12cb8:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; ld1s_add r15, r16, 5 }
   12cc0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; ori r15, r16, 5 }
   12cc8:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; st4_add r15, r16, 5 }
   12cd0:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; v1subuc r15, r16, r17 }
   12cd8:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; v4shrs r15, r16, r17 }
   12ce0:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; fetchor4 r15, r16, r17 }
   12ce8:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; ldnt4s r15, r16 }
   12cf0:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; shl3add r15, r16, r17 }
   12cf8:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   12d00:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; v2packuc r15, r16, r17 }
   12d08:	[0-9a-f]* 	{ v1mz r15, r16, r17 ; cmpeqi r5, r6, 5 }
   12d10:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; v1mz r15, r16, r17 }
   12d18:	[0-9a-f]* 	{ v1mz r15, r16, r17 ; shl1addx r5, r6, r7 }
   12d20:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; v1mz r15, r16, r17 }
   12d28:	[0-9a-f]* 	{ v1mz r15, r16, r17 ; v2cmpne r5, r6, r7 }
   12d30:	[0-9a-f]* 	{ v1mz r15, r16, r17 ; v4subsc r5, r6, r7 }
   12d38:	[0-9a-f]* 	{ v1mz r5, r6, r7 }
   12d40:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; ldnt_add r15, r16, 5 }
   12d48:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; shlxi r15, r16, 5 }
   12d50:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; v1maxu r15, r16, r17 }
   12d58:	[0-9a-f]* 	{ v1mz r5, r6, r7 ; v2shrs r15, r16, r17 }
   12d60:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; dblalign2 r15, r16, r17 }
   12d68:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; ld4u_add r15, r16, 5 }
   12d70:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; prefetch_l2 r15 }
   12d78:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; sub r15, r16, r17 }
   12d80:	[0-9a-f]* 	{ v1sadau r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   12d88:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; addx r15, r16, r17 }
   12d90:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; iret }
   12d98:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; movei r15, 5 }
   12da0:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; shruxi r15, r16, 5 }
   12da8:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; v1shl r15, r16, r17 }
   12db0:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; v4add r15, r16, r17 }
   12db8:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; v1shl r15, r16, r17 }
   12dc0:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; v1shl r15, r16, r17 }
   12dc8:	[0-9a-f]* 	{ v1shl r15, r16, r17 ; shru r5, r6, r7 }
   12dd0:	[0-9a-f]* 	{ v1shl r15, r16, r17 ; v1minu r5, r6, r7 }
   12dd8:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; v1shl r15, r16, r17 }
   12de0:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; and r15, r16, r17 }
   12de8:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; jrp r15 }
   12df0:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; nop }
   12df8:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; st2 r15, r16 }
   12e00:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; v1shru r15, r16, r17 }
   12e08:	[0-9a-f]* 	{ v1shl r5, r6, r7 ; v4packsc r15, r16, r17 }
   12e10:	[0-9a-f]* 	{ cmulhr r5, r6, r7 ; v1shli r15, r16, 5 }
   12e18:	[0-9a-f]* 	{ mul_lu_lu r5, r6, r7 ; v1shli r15, r16, 5 }
   12e20:	[0-9a-f]* 	{ shufflebytes r5, r6, r7 ; v1shli r15, r16, 5 }
   12e28:	[0-9a-f]* 	{ v1mulu r5, r6, r7 ; v1shli r15, r16, 5 }
   12e30:	[0-9a-f]* 	{ v1shli r15, r16, 5 ; v2packh r5, r6, r7 }
   12e38:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; cmpexch r15, r16, r17 }
   12e40:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; ld1u r15, r16 }
   12e48:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; prefetch r15 }
   12e50:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; st_add r15, r16, 5 }
   12e58:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; v2add r15, r16, r17 }
   12e60:	[0-9a-f]* 	{ v1shli r5, r6, 5 ; v4shru r15, r16, r17 }
   12e68:	[0-9a-f]* 	{ dblalign r5, r6, r7 ; v1shrs r15, r16, r17 }
   12e70:	[0-9a-f]* 	{ mula_hs_lu r5, r6, r7 ; v1shrs r15, r16, r17 }
   12e78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; v1shrs r15, r16, r17 }
   12e80:	[0-9a-f]* 	{ v1sadu r5, r6, r7 ; v1shrs r15, r16, r17 }
   12e88:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; v1shrs r15, r16, r17 }
   12e90:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; cmplts r15, r16, r17 }
   12e98:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; ld2u r15, r16 }
   12ea0:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
   12ea8:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; stnt2 r15, r16 }
   12eb0:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
   12eb8:	[0-9a-f]* 	{ v1shrs r5, r6, r7 ; xor r15, r16, r17 }
   12ec0:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; v1shrsi r15, r16, 5 }
   12ec8:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; v1shrsi r15, r16, 5 }
   12ed0:	[0-9a-f]* 	{ v1shrsi r15, r16, 5 ; v1add r5, r6, r7 }
   12ed8:	[0-9a-f]* 	{ v1shrsi r15, r16, 5 ; v1shrsi r5, r6, 5 }
   12ee0:	[0-9a-f]* 	{ v1shrsi r15, r16, 5 ; v2shli r5, r6, 5 }
   12ee8:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; cmpne r15, r16, r17 }
   12ef0:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; ld4u r15, r16 }
   12ef8:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; prefetch_l1_fault r15 }
   12f00:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; stnt_add r15, r16, 5 }
   12f08:	[0-9a-f]* 	{ v1shrsi r5, r6, 5 ; v2cmpltsi r15, r16, 5 }
   12f10:	[0-9a-f]* 	{ v1shru r15, r16, r17 ; addli r5, r6, 4660 }
   12f18:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; v1shru r15, r16, r17 }
   12f20:	[0-9a-f]* 	{ mulx r5, r6, r7 ; v1shru r15, r16, r17 }
   12f28:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; v1shru r15, r16, r17 }
   12f30:	[0-9a-f]* 	{ v1shru r15, r16, r17 ; v1subuc r5, r6, r7 }
   12f38:	[0-9a-f]* 	{ v1shru r15, r16, r17 ; v2shru r5, r6, r7 }
   12f40:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; dtlbpr r15 }
   12f48:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; ldna_add r15, r16, 5 }
   12f50:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; prefetch_l3_fault r15 }
   12f58:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; v1add r15, r16, r17 }
   12f60:	[0-9a-f]* 	{ v1shru r5, r6, r7 ; v2int_h r15, r16, r17 }
   12f68:	[0-9a-f]* 	{ v1shrui r15, r16, 5 ; addxsc r5, r6, r7 }
   12f70:	[0-9a-f]* 	{ v1shrui r15, r16, 5 }
   12f78:	[0-9a-f]* 	{ v1shrui r15, r16, 5 ; or r5, r6, r7 }
   12f80:	[0-9a-f]* 	{ v1shrui r15, r16, 5 ; v1cmpleu r5, r6, r7 }
   12f88:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; v1shrui r15, r16, 5 }
   12f90:	[0-9a-f]* 	{ v1shrui r15, r16, 5 ; v4add r5, r6, r7 }
   12f98:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; fetchadd4 r15, r16, r17 }
   12fa0:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; ldnt1u r15, r16 }
   12fa8:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; shl r15, r16, r17 }
   12fb0:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
   12fb8:	[0-9a-f]* 	{ v1shrui r5, r6, 5 ; v2mins r15, r16, r17 }
   12fc0:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; v1sub r15, r16, r17 }
   12fc8:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; v1sub r15, r16, r17 }
   12fd0:	[0-9a-f]* 	{ revbytes r5, r6 ; v1sub r15, r16, r17 }
   12fd8:	[0-9a-f]* 	{ v1sub r15, r16, r17 ; v1cmpltui r5, r6, 5 }
   12fe0:	[0-9a-f]* 	{ v1sub r15, r16, r17 ; v2cmples r5, r6, r7 }
   12fe8:	[0-9a-f]* 	{ v1sub r15, r16, r17 ; v4packsc r5, r6, r7 }
   12ff0:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; fetchand4 r15, r16, r17 }
   12ff8:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; ldnt2u r15, r16 }
   13000:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; shl2add r15, r16, r17 }
   13008:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
   13010:	[0-9a-f]* 	{ v1sub r5, r6, r7 ; v2packh r15, r16, r17 }
   13018:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; v1subuc r15, r16, r17 }
   13020:	[0-9a-f]* 	{ v1subuc r15, r16, r17 ; info 19 }
   13028:	[0-9a-f]* 	{ v1subuc r15, r16, r17 ; shl16insli r5, r6, 4660 }
   13030:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; v1subuc r15, r16, r17 }
   13038:	[0-9a-f]* 	{ v1subuc r15, r16, r17 ; v2cmpltu r5, r6, r7 }
   13040:	[0-9a-f]* 	{ v1subuc r15, r16, r17 ; v4shru r5, r6, r7 }
   13048:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; flush r15 }
   13050:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; ldnt4u r15, r16 }
   13058:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; shli r15, r16, 5 }
   13060:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; v1int_h r15, r16, r17 }
   13068:	[0-9a-f]* 	{ v1subuc r5, r6, r7 ; v2shli r15, r16, 5 }
   13070:	[0-9a-f]* 	{ v2add r15, r16, r17 ; cmpleu r5, r6, r7 }
   13078:	[0-9a-f]* 	{ v2add r15, r16, r17 ; move r5, r6 }
   13080:	[0-9a-f]* 	{ v2add r15, r16, r17 ; shl2addx r5, r6, r7 }
   13088:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; v2add r15, r16, r17 }
   13090:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; v2add r15, r16, r17 }
   13098:	[0-9a-f]* 	{ v2add r15, r16, r17 ; xori r5, r6, 5 }
   130a0:	[0-9a-f]* 	{ v2add r5, r6, r7 ; ill }
   130a8:	[0-9a-f]* 	{ v2add r5, r6, r7 ; mf }
   130b0:	[0-9a-f]* 	{ v2add r5, r6, r7 ; shrsi r15, r16, 5 }
   130b8:	[0-9a-f]* 	{ v2add r5, r6, r7 ; v1minu r15, r16, r17 }
   130c0:	[0-9a-f]* 	{ v2add r5, r6, r7 ; v2shru r15, r16, r17 }
   130c8:	[0-9a-f]* 	{ v2addi r15, r16, 5 ; cmpltui r5, r6, 5 }
   130d0:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; v2addi r15, r16, 5 }
   130d8:	[0-9a-f]* 	{ v2addi r15, r16, 5 ; shlx r5, r6, r7 }
   130e0:	[0-9a-f]* 	{ v2addi r15, r16, 5 ; v1int_h r5, r6, r7 }
   130e8:	[0-9a-f]* 	{ v2addi r15, r16, 5 ; v2maxsi r5, r6, 5 }
   130f0:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; addx r15, r16, r17 }
   130f8:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; iret }
   13100:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; movei r15, 5 }
   13108:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; shruxi r15, r16, 5 }
   13110:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; v1shl r15, r16, r17 }
   13118:	[0-9a-f]* 	{ v2addi r5, r6, 5 ; v4add r15, r16, r17 }
   13120:	[0-9a-f]* 	{ cmulaf r5, r6, r7 ; v2addsc r15, r16, r17 }
   13128:	[0-9a-f]* 	{ mul_hu_ls r5, r6, r7 ; v2addsc r15, r16, r17 }
   13130:	[0-9a-f]* 	{ v2addsc r15, r16, r17 ; shru r5, r6, r7 }
   13138:	[0-9a-f]* 	{ v2addsc r15, r16, r17 ; v1minu r5, r6, r7 }
   13140:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; v2addsc r15, r16, r17 }
   13148:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; and r15, r16, r17 }
   13150:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; jrp r15 }
   13158:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; nop }
   13160:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; st2 r15, r16 }
   13168:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; v1shru r15, r16, r17 }
   13170:	[0-9a-f]* 	{ v2addsc r5, r6, r7 ; v4packsc r15, r16, r17 }
   13178:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; fetchand r15, r16, r17 }
   13180:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
   13188:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; shl1addx r15, r16, r17 }
   13190:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; v1cmplts r15, r16, r17 }
   13198:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; v2mz r15, r16, r17 }
   131a0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; cmples r15, r16, r17 }
   131a8:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; ld2s r15, r16 }
   131b0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
   131b8:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; stnt1 r15, r16 }
   131c0:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; v2addsc r15, r16, r17 }
   131c8:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; v4subsc r15, r16, r17 }
   131d0:	[0-9a-f]* 	{ v2cmpeq r15, r16, r17 ; dblalign4 r5, r6, r7 }
   131d8:	[0-9a-f]* 	{ mula_hu_ls r5, r6, r7 ; v2cmpeq r15, r16, r17 }
   131e0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; v2cmpeq r15, r16, r17 }
   131e8:	[0-9a-f]* 	{ v2cmpeq r15, r16, r17 ; v1shli r5, r6, 5 }
   131f0:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; v2cmpeq r15, r16, r17 }
   131f8:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 }
   13200:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; ld4s r15, r16 }
   13208:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
   13210:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; stnt4 r15, r16 }
   13218:	[0-9a-f]* 	{ v2cmpeq r5, r6, r7 ; v2cmpleu r15, r16, r17 }
   13220:	[0-9a-f]* 	{ v2cmpeqi r15, r16, 5 ; add r5, r6, r7 }
   13228:	[0-9a-f]* 	{ fdouble_mul_flags r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
   13230:	[0-9a-f]* 	{ mula_lu_lu r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
   13238:	[0-9a-f]* 	{ v2cmpeqi r15, r16, 5 ; v1adduc r5, r6, r7 }
   13240:	[0-9a-f]* 	{ v2cmpeqi r15, r16, 5 ; v1shrui r5, r6, 5 }
   13248:	[0-9a-f]* 	{ v2cmpeqi r15, r16, 5 ; v2shrs r5, r6, r7 }
   13250:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; dblalign4 r15, r16, r17 }
   13258:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; ld_add r15, r16, 5 }
   13260:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; prefetch_l2_fault r15 }
   13268:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; subx r15, r16, r17 }
   13270:	[0-9a-f]* 	{ v2cmpeqi r5, r6, 5 ; v2cmpltui r15, r16, 5 }
   13278:	[0-9a-f]* 	{ v2cmples r15, r16, r17 ; addxi r5, r6, 5 }
   13280:	[0-9a-f]* 	{ fdouble_unpack_max r5, r6, r7 ; v2cmples r15, r16, r17 }
   13288:	[0-9a-f]* 	{ v2cmples r15, r16, r17 ; nop }
   13290:	[0-9a-f]* 	{ v2cmples r15, r16, r17 ; v1cmpeqi r5, r6, 5 }
   13298:	[0-9a-f]* 	{ v2cmples r15, r16, r17 ; v2addi r5, r6, 5 }
   132a0:	[0-9a-f]* 	{ v2cmples r15, r16, r17 ; v2sub r5, r6, r7 }
   132a8:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; exch4 r15, r16, r17 }
   132b0:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; ldnt1s r15, r16 }
   132b8:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; rotl r15, r16, r17 }
   132c0:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; v1adduc r15, r16, r17 }
   132c8:	[0-9a-f]* 	{ v2cmples r5, r6, r7 ; v2maxs r15, r16, r17 }
   132d0:	[0-9a-f]* 	{ v2cmpleu r15, r16, r17 ; andi r5, r6, 5 }
   132d8:	[0-9a-f]* 	{ fsingle_addsub2 r5, r6, r7 ; v2cmpleu r15, r16, r17 }
   132e0:	[0-9a-f]* 	{ pcnt r5, r6 ; v2cmpleu r15, r16, r17 }
   132e8:	[0-9a-f]* 	{ v2cmpleu r15, r16, r17 ; v1cmpltsi r5, r6, 5 }
   132f0:	[0-9a-f]* 	{ v2cmpleu r15, r16, r17 ; v2cmpeq r5, r6, r7 }
   132f8:	[0-9a-f]* 	{ v2cmpleu r15, r16, r17 ; v4int_h r5, r6, r7 }
   13300:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
   13308:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; ldnt2s r15, r16 }
   13310:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; shl1add r15, r16, r17 }
   13318:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
   13320:	[0-9a-f]* 	{ v2cmpleu r5, r6, r7 ; v2mnz r15, r16, r17 }
   13328:	[0-9a-f]* 	{ clz r5, r6 ; v2cmplts r15, r16, r17 }
   13330:	[0-9a-f]* 	{ fsingle_pack2 r5, r6, r7 ; v2cmplts r15, r16, r17 }
   13338:	[0-9a-f]* 	{ v2cmplts r15, r16, r17 ; rotli r5, r6, 5 }
   13340:	[0-9a-f]* 	{ v1ddotpu r5, r6, r7 ; v2cmplts r15, r16, r17 }
   13348:	[0-9a-f]* 	{ v2cmplts r15, r16, r17 ; v2cmplts r5, r6, r7 }
   13350:	[0-9a-f]* 	{ v2cmplts r15, r16, r17 ; v4shlsc r5, r6, r7 }
   13358:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; fetchor4 r15, r16, r17 }
   13360:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; ldnt4s r15, r16 }
   13368:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; shl3add r15, r16, r17 }
   13370:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; v1cmpltui r15, r16, 5 }
   13378:	[0-9a-f]* 	{ v2cmplts r5, r6, r7 ; v2packuc r15, r16, r17 }
   13380:	[0-9a-f]* 	{ v2cmpltsi r15, r16, 5 ; cmpeqi r5, r6, 5 }
   13388:	[0-9a-f]* 	{ mm r5, r6, 5, 7 ; v2cmpltsi r15, r16, 5 }
   13390:	[0-9a-f]* 	{ v2cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 }
   13398:	[0-9a-f]* 	{ v1dotp r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
   133a0:	[0-9a-f]* 	{ v2cmpltsi r15, r16, 5 ; v2cmpne r5, r6, r7 }
   133a8:	[0-9a-f]* 	{ v2cmpltsi r15, r16, 5 ; v4subsc r5, r6, r7 }
   133b0:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 }
   133b8:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; ldnt_add r15, r16, 5 }
   133c0:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; shlxi r15, r16, 5 }
   133c8:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; v1maxu r15, r16, r17 }
   133d0:	[0-9a-f]* 	{ v2cmpltsi r5, r6, 5 ; v2shrs r15, r16, r17 }
   133d8:	[0-9a-f]* 	{ v2cmpltu r15, r16, r17 ; cmpltsi r5, r6, 5 }
   133e0:	[0-9a-f]* 	{ v2cmpltu r15, r16, r17 ; moveli r5, 4660 }
   133e8:	[0-9a-f]* 	{ v2cmpltu r15, r16, r17 ; shl3addx r5, r6, r7 }
   133f0:	[0-9a-f]* 	{ v1dotpus r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   133f8:	[0-9a-f]* 	{ v2cmpltu r15, r16, r17 ; v2int_l r5, r6, r7 }
   13400:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; addi r15, r16, 5 }
   13408:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; infol 4660 }
   13410:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; mnz r15, r16, r17 }
   13418:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; shrui r15, r16, 5 }
   13420:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; v1mnz r15, r16, r17 }
   13428:	[0-9a-f]* 	{ v2cmpltu r5, r6, r7 ; v2sub r15, r16, r17 }
   13430:	[0-9a-f]* 	{ cmul r5, r6, r7 ; v2cmpltui r15, r16, 5 }
   13438:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; v2cmpltui r15, r16, 5 }
   13440:	[0-9a-f]* 	{ v2cmpltui r15, r16, 5 ; shrs r5, r6, r7 }
   13448:	[0-9a-f]* 	{ v2cmpltui r15, r16, 5 ; v1maxu r5, r6, r7 }
   13450:	[0-9a-f]* 	{ v2cmpltui r15, r16, 5 ; v2minsi r5, r6, 5 }
   13458:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; addxli r15, r16, 4660 }
   13460:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; jalrp r15 }
   13468:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
   13470:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; st1 r15, r16 }
   13478:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; v1shrs r15, r16, r17 }
   13480:	[0-9a-f]* 	{ v2cmpltui r5, r6, 5 ; v4int_h r15, r16, r17 }
   13488:	[0-9a-f]* 	{ cmulfr r5, r6, r7 ; v2cmpne r15, r16, r17 }
   13490:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; v2cmpne r15, r16, r17 }
   13498:	[0-9a-f]* 	{ v2cmpne r15, r16, r17 ; shrux r5, r6, r7 }
   134a0:	[0-9a-f]* 	{ v2cmpne r15, r16, r17 ; v1mnz r5, r6, r7 }
   134a8:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; v2cmpne r15, r16, r17 }
   134b0:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; cmpeq r15, r16, r17 }
   134b8:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; ld1s r15, r16 }
   134c0:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; or r15, r16, r17 }
   134c8:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; st4 r15, r16 }
   134d0:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; v1sub r15, r16, r17 }
   134d8:	[0-9a-f]* 	{ v2cmpne r5, r6, r7 ; v4shlsc r15, r16, r17 }
   134e0:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; fetchor r15, r16, r17 }
   134e8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
   134f0:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; shl2addx r15, r16, r17 }
   134f8:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   13500:	[0-9a-f]* 	{ v2dotp r5, r6, r7 ; v2packl r15, r16, r17 }
   13508:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; cmplts r15, r16, r17 }
   13510:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; ld2u r15, r16 }
   13518:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
   13520:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; stnt2 r15, r16 }
   13528:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
   13530:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; xor r15, r16, r17 }
   13538:	[0-9a-f]* 	{ fdouble_add_flags r5, r6, r7 ; v2int_h r15, r16, r17 }
   13540:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; v2int_h r15, r16, r17 }
   13548:	[0-9a-f]* 	{ v2int_h r15, r16, r17 ; v1add r5, r6, r7 }
   13550:	[0-9a-f]* 	{ v2int_h r15, r16, r17 ; v1shrsi r5, r6, 5 }
   13558:	[0-9a-f]* 	{ v2int_h r15, r16, r17 ; v2shli r5, r6, 5 }
   13560:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; cmpne r15, r16, r17 }
   13568:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; ld4u r15, r16 }
   13570:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; prefetch_l1_fault r15 }
   13578:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; stnt_add r15, r16, 5 }
   13580:	[0-9a-f]* 	{ v2int_h r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
   13588:	[0-9a-f]* 	{ v2int_l r15, r16, r17 ; addli r5, r6, 4660 }
   13590:	[0-9a-f]* 	{ fdouble_pack2 r5, r6, r7 ; v2int_l r15, r16, r17 }
   13598:	[0-9a-f]* 	{ mulx r5, r6, r7 ; v2int_l r15, r16, r17 }
   135a0:	[0-9a-f]* 	{ v1avgu r5, r6, r7 ; v2int_l r15, r16, r17 }
   135a8:	[0-9a-f]* 	{ v2int_l r15, r16, r17 ; v1subuc r5, r6, r7 }
   135b0:	[0-9a-f]* 	{ v2int_l r15, r16, r17 ; v2shru r5, r6, r7 }
   135b8:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; dtlbpr r15 }
   135c0:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; ldna_add r15, r16, 5 }
   135c8:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; prefetch_l3_fault r15 }
   135d0:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; v1add r15, r16, r17 }
   135d8:	[0-9a-f]* 	{ v2int_l r5, r6, r7 ; v2int_h r15, r16, r17 }
   135e0:	[0-9a-f]* 	{ v2maxs r15, r16, r17 ; addxsc r5, r6, r7 }
   135e8:	[0-9a-f]* 	{ v2maxs r15, r16, r17 }
   135f0:	[0-9a-f]* 	{ v2maxs r15, r16, r17 ; or r5, r6, r7 }
   135f8:	[0-9a-f]* 	{ v2maxs r15, r16, r17 ; v1cmpleu r5, r6, r7 }
   13600:	[0-9a-f]* 	{ v2adiffs r5, r6, r7 ; v2maxs r15, r16, r17 }
   13608:	[0-9a-f]* 	{ v2maxs r15, r16, r17 ; v4add r5, r6, r7 }
   13610:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; fetchadd4 r15, r16, r17 }
   13618:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; ldnt1u r15, r16 }
   13620:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; shl r15, r16, r17 }
   13628:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
   13630:	[0-9a-f]* 	{ v2maxs r5, r6, r7 ; v2mins r15, r16, r17 }
   13638:	[0-9a-f]* 	{ bfextu r5, r6, 5, 7 ; v2maxsi r15, r16, 5 }
   13640:	[0-9a-f]* 	{ fsingle_mul2 r5, r6, r7 ; v2maxsi r15, r16, 5 }
   13648:	[0-9a-f]* 	{ revbytes r5, r6 ; v2maxsi r15, r16, 5 }
   13650:	[0-9a-f]* 	{ v2maxsi r15, r16, 5 ; v1cmpltui r5, r6, 5 }
   13658:	[0-9a-f]* 	{ v2maxsi r15, r16, 5 ; v2cmples r5, r6, r7 }
   13660:	[0-9a-f]* 	{ v2maxsi r15, r16, 5 ; v4packsc r5, r6, r7 }
   13668:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; fetchand4 r15, r16, r17 }
   13670:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; ldnt2u r15, r16 }
   13678:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; shl2add r15, r16, r17 }
   13680:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; v1cmpltsi r15, r16, 5 }
   13688:	[0-9a-f]* 	{ v2maxsi r5, r6, 5 ; v2packh r15, r16, r17 }
   13690:	[0-9a-f]* 	{ cmovnez r5, r6, r7 ; v2mins r15, r16, r17 }
   13698:	[0-9a-f]* 	{ v2mins r15, r16, r17 ; info 19 }
   136a0:	[0-9a-f]* 	{ v2mins r15, r16, r17 ; shl16insli r5, r6, 4660 }
   136a8:	[0-9a-f]* 	{ v1ddotpus r5, r6, r7 ; v2mins r15, r16, r17 }
   136b0:	[0-9a-f]* 	{ v2mins r15, r16, r17 ; v2cmpltu r5, r6, r7 }
   136b8:	[0-9a-f]* 	{ v2mins r15, r16, r17 ; v4shru r5, r6, r7 }
   136c0:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; flush r15 }
   136c8:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; ldnt4u r15, r16 }
   136d0:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; shli r15, r16, 5 }
   136d8:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; v1int_h r15, r16, r17 }
   136e0:	[0-9a-f]* 	{ v2mins r5, r6, r7 ; v2shli r15, r16, 5 }
   136e8:	[0-9a-f]* 	{ v2minsi r15, r16, 5 ; cmpleu r5, r6, r7 }
   136f0:	[0-9a-f]* 	{ v2minsi r15, r16, 5 ; move r5, r6 }
   136f8:	[0-9a-f]* 	{ v2minsi r15, r16, 5 ; shl2addx r5, r6, r7 }
   13700:	[0-9a-f]* 	{ v1dotpu r5, r6, r7 ; v2minsi r15, r16, 5 }
   13708:	[0-9a-f]* 	{ v2dotpa r5, r6, r7 ; v2minsi r15, r16, 5 }
   13710:	[0-9a-f]* 	{ v2minsi r15, r16, 5 ; xori r5, r6, 5 }
   13718:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; ill }
   13720:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; mf }
   13728:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; shrsi r15, r16, 5 }
   13730:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; v1minu r15, r16, r17 }
   13738:	[0-9a-f]* 	{ v2minsi r5, r6, 5 ; v2shru r15, r16, r17 }
   13740:	[0-9a-f]* 	{ v2mnz r15, r16, r17 ; cmpltui r5, r6, 5 }
   13748:	[0-9a-f]* 	{ mul_hs_hu r5, r6, r7 ; v2mnz r15, r16, r17 }
   13750:	[0-9a-f]* 	{ v2mnz r15, r16, r17 ; shlx r5, r6, r7 }
   13758:	[0-9a-f]* 	{ v2mnz r15, r16, r17 ; v1int_h r5, r6, r7 }
   13760:	[0-9a-f]* 	{ v2mnz r15, r16, r17 ; v2maxsi r5, r6, 5 }
   13768:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; addx r15, r16, r17 }
   13770:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; iret }
   13778:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; movei r15, 5 }
   13780:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; shruxi r15, r16, 5 }
   13788:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; v1shl r15, r16, r17 }
   13790:	[0-9a-f]* 	{ v2mnz r5, r6, r7 ; v4add r15, r16, r17 }
   13798:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; fetchadd r15, r16, r17 }
   137a0:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
   137a8:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; rotli r15, r16, 5 }
   137b0:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; v1cmpeq r15, r16, r17 }
   137b8:	[0-9a-f]* 	{ v2mulfsc r5, r6, r7 ; v2maxsi r15, r16, 5 }
   137c0:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; cmpeq r15, r16, r17 }
   137c8:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; ld1s r15, r16 }
   137d0:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; or r15, r16, r17 }
   137d8:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; st4 r15, r16 }
   137e0:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; v1sub r15, r16, r17 }
   137e8:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; v4shlsc r15, r16, r17 }
   137f0:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; fetchor r15, r16, r17 }
   137f8:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
   13800:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; shl2addx r15, r16, r17 }
   13808:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   13810:	[0-9a-f]* 	{ v2mults r5, r6, r7 ; v2packl r15, r16, r17 }
   13818:	[0-9a-f]* 	{ v2mz r15, r16, r17 ; cmpeq r5, r6, r7 }
   13820:	[0-9a-f]* 	{ v2mz r15, r16, r17 ; infol 4660 }
   13828:	[0-9a-f]* 	{ v2mz r15, r16, r17 ; shl1add r5, r6, r7 }
   13830:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v2mz r15, r16, r17 }
   13838:	[0-9a-f]* 	{ v2mz r15, r16, r17 ; v2cmpltui r5, r6, 5 }
   13840:	[0-9a-f]* 	{ v2mz r15, r16, r17 ; v4sub r5, r6, r7 }
   13848:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; flushwb }
   13850:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
   13858:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; shlx r15, r16, r17 }
   13860:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; v1int_l r15, r16, r17 }
   13868:	[0-9a-f]* 	{ v2mz r5, r6, r7 ; v2shlsc r15, r16, r17 }
   13870:	[0-9a-f]* 	{ v2packh r15, r16, r17 ; cmplts r5, r6, r7 }
   13878:	[0-9a-f]* 	{ v2packh r15, r16, r17 ; movei r5, 5 }
   13880:	[0-9a-f]* 	{ v2packh r15, r16, r17 ; shl3add r5, r6, r7 }
   13888:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v2packh r15, r16, r17 }
   13890:	[0-9a-f]* 	{ v2packh r15, r16, r17 ; v2int_h r5, r6, r7 }
   13898:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; add r15, r16, r17 }
   138a0:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; info 19 }
   138a8:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
   138b0:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; shru r15, r16, r17 }
   138b8:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; v1minui r15, r16, 5 }
   138c0:	[0-9a-f]* 	{ v2packh r5, r6, r7 ; v2shrui r15, r16, 5 }
   138c8:	[0-9a-f]* 	{ v2packl r15, r16, r17 ; cmpne r5, r6, r7 }
   138d0:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v2packl r15, r16, r17 }
   138d8:	[0-9a-f]* 	{ v2packl r15, r16, r17 ; shlxi r5, r6, 5 }
   138e0:	[0-9a-f]* 	{ v2packl r15, r16, r17 ; v1int_l r5, r6, r7 }
   138e8:	[0-9a-f]* 	{ v2packl r15, r16, r17 ; v2mins r5, r6, r7 }
   138f0:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; addxi r15, r16, 5 }
   138f8:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; jalr r15 }
   13900:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; moveli r15, 4660 }
   13908:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; st r15, r16 }
   13910:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; v1shli r15, r16, 5 }
   13918:	[0-9a-f]* 	{ v2packl r5, r6, r7 ; v4addsc r15, r16, r17 }
   13920:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; v2packuc r15, r16, r17 }
   13928:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; v2packuc r15, r16, r17 }
   13930:	[0-9a-f]* 	{ v2packuc r15, r16, r17 ; shrui r5, r6, 5 }
   13938:	[0-9a-f]* 	{ v2packuc r15, r16, r17 ; v1minui r5, r6, 5 }
   13940:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; v2packuc r15, r16, r17 }
   13948:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; andi r15, r16, 5 }
   13950:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; ld r15, r16 }
   13958:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; nor r15, r16, r17 }
   13960:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; st2_add r15, r16, 5 }
   13968:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; v1shrui r15, r16, 5 }
   13970:	[0-9a-f]* 	{ v2packuc r5, r6, r7 ; v4shl r15, r16, r17 }
   13978:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; fetchand4 r15, r16, r17 }
   13980:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; ldnt2u r15, r16 }
   13988:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; shl2add r15, r16, r17 }
   13990:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
   13998:	[0-9a-f]* 	{ v2sadas r5, r6, r7 ; v2packh r15, r16, r17 }
   139a0:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; cmpleu r15, r16, r17 }
   139a8:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; ld2s_add r15, r16, 5 }
   139b0:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; prefetch_add_l2 r15, 5 }
   139b8:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; stnt1_add r15, r16, 5 }
   139c0:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; v2cmpeq r15, r16, r17 }
   139c8:	[0-9a-f]* 	{ v2sadau r5, r6, r7 ; wh64 r15 }
   139d0:	[0-9a-f]* 	{ v2sads r5, r6, r7 }
   139d8:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; ldnt_add r15, r16, 5 }
   139e0:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; shlxi r15, r16, 5 }
   139e8:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; v1maxu r15, r16, r17 }
   139f0:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; v2shrs r15, r16, r17 }
   139f8:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; dblalign2 r15, r16, r17 }
   13a00:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; ld4u_add r15, r16, 5 }
   13a08:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; prefetch_l2 r15 }
   13a10:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; sub r15, r16, r17 }
   13a18:	[0-9a-f]* 	{ v2sadu r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   13a20:	[0-9a-f]* 	{ v2shl r15, r16, r17 ; addx r5, r6, r7 }
   13a28:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v2shl r15, r16, r17 }
   13a30:	[0-9a-f]* 	{ v2shl r15, r16, r17 ; mz r5, r6, r7 }
   13a38:	[0-9a-f]* 	{ v2shl r15, r16, r17 ; v1cmpeq r5, r6, r7 }
   13a40:	[0-9a-f]* 	{ v2shl r15, r16, r17 ; v2add r5, r6, r7 }
   13a48:	[0-9a-f]* 	{ v2shl r15, r16, r17 ; v2shrui r5, r6, 5 }
   13a50:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; exch r15, r16, r17 }
   13a58:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; ldnt r15, r16 }
   13a60:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; raise }
   13a68:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; v1addi r15, r16, 5 }
   13a70:	[0-9a-f]* 	{ v2shl r5, r6, r7 ; v2int_l r15, r16, r17 }
   13a78:	[0-9a-f]* 	{ v2shli r15, r16, 5 ; and r5, r6, r7 }
   13a80:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; v2shli r15, r16, 5 }
   13a88:	[0-9a-f]* 	{ v2shli r15, r16, 5 ; ori r5, r6, 5 }
   13a90:	[0-9a-f]* 	{ v2shli r15, r16, 5 ; v1cmplts r5, r6, r7 }
   13a98:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; v2shli r15, r16, 5 }
   13aa0:	[0-9a-f]* 	{ v2shli r15, r16, 5 ; v4addsc r5, r6, r7 }
   13aa8:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; fetchaddgez r15, r16, r17 }
   13ab0:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; ldnt1u_add r15, r16, 5 }
   13ab8:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; shl16insli r15, r16, 4660 }
   13ac0:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; v1cmples r15, r16, r17 }
   13ac8:	[0-9a-f]* 	{ v2shli r5, r6, 5 ; v2minsi r15, r16, 5 }
   13ad0:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; v2shlsc r15, r16, r17 }
   13ad8:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; v2shlsc r15, r16, r17 }
   13ae0:	[0-9a-f]* 	{ v2shlsc r15, r16, r17 ; rotl r5, r6, r7 }
   13ae8:	[0-9a-f]* 	{ v2shlsc r15, r16, r17 ; v1cmpne r5, r6, r7 }
   13af0:	[0-9a-f]* 	{ v2shlsc r15, r16, r17 ; v2cmpleu r5, r6, r7 }
   13af8:	[0-9a-f]* 	{ v2shlsc r15, r16, r17 ; v4shl r5, r6, r7 }
   13b00:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; fetchor r15, r16, r17 }
   13b08:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
   13b10:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; shl2addx r15, r16, r17 }
   13b18:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   13b20:	[0-9a-f]* 	{ v2shlsc r5, r6, r7 ; v2packl r15, r16, r17 }
   13b28:	[0-9a-f]* 	{ v2shrs r15, r16, r17 ; cmpeq r5, r6, r7 }
   13b30:	[0-9a-f]* 	{ v2shrs r15, r16, r17 ; infol 4660 }
   13b38:	[0-9a-f]* 	{ v2shrs r15, r16, r17 ; shl1add r5, r6, r7 }
   13b40:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v2shrs r15, r16, r17 }
   13b48:	[0-9a-f]* 	{ v2shrs r15, r16, r17 ; v2cmpltui r5, r6, 5 }
   13b50:	[0-9a-f]* 	{ v2shrs r15, r16, r17 ; v4sub r5, r6, r7 }
   13b58:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; flushwb }
   13b60:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
   13b68:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; shlx r15, r16, r17 }
   13b70:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; v1int_l r15, r16, r17 }
   13b78:	[0-9a-f]* 	{ v2shrs r5, r6, r7 ; v2shlsc r15, r16, r17 }
   13b80:	[0-9a-f]* 	{ v2shrsi r15, r16, 5 ; cmplts r5, r6, r7 }
   13b88:	[0-9a-f]* 	{ v2shrsi r15, r16, 5 ; movei r5, 5 }
   13b90:	[0-9a-f]* 	{ v2shrsi r15, r16, 5 ; shl3add r5, r6, r7 }
   13b98:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v2shrsi r15, r16, 5 }
   13ba0:	[0-9a-f]* 	{ v2shrsi r15, r16, 5 ; v2int_h r5, r6, r7 }
   13ba8:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; add r15, r16, r17 }
   13bb0:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; info 19 }
   13bb8:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
   13bc0:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; shru r15, r16, r17 }
   13bc8:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; v1minui r15, r16, 5 }
   13bd0:	[0-9a-f]* 	{ v2shrsi r5, r6, 5 ; v2shrui r15, r16, 5 }
   13bd8:	[0-9a-f]* 	{ v2shru r15, r16, r17 ; cmpne r5, r6, r7 }
   13be0:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v2shru r15, r16, r17 }
   13be8:	[0-9a-f]* 	{ v2shru r15, r16, r17 ; shlxi r5, r6, 5 }
   13bf0:	[0-9a-f]* 	{ v2shru r15, r16, r17 ; v1int_l r5, r6, r7 }
   13bf8:	[0-9a-f]* 	{ v2shru r15, r16, r17 ; v2mins r5, r6, r7 }
   13c00:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; addxi r15, r16, 5 }
   13c08:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; jalr r15 }
   13c10:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; moveli r15, 4660 }
   13c18:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; st r15, r16 }
   13c20:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; v1shli r15, r16, 5 }
   13c28:	[0-9a-f]* 	{ v2shru r5, r6, r7 ; v4addsc r15, r16, r17 }
   13c30:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; v2shrui r15, r16, 5 }
   13c38:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; v2shrui r15, r16, 5 }
   13c40:	[0-9a-f]* 	{ v2shrui r15, r16, 5 ; shrui r5, r6, 5 }
   13c48:	[0-9a-f]* 	{ v2shrui r15, r16, 5 ; v1minui r5, r6, 5 }
   13c50:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; v2shrui r15, r16, 5 }
   13c58:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; andi r15, r16, 5 }
   13c60:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; ld r15, r16 }
   13c68:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; nor r15, r16, r17 }
   13c70:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; st2_add r15, r16, 5 }
   13c78:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; v1shrui r15, r16, 5 }
   13c80:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; v4shl r15, r16, r17 }
   13c88:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; v2sub r15, r16, r17 }
   13c90:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; v2sub r15, r16, r17 }
   13c98:	[0-9a-f]* 	{ v2sub r15, r16, r17 ; sub r5, r6, r7 }
   13ca0:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; v2sub r15, r16, r17 }
   13ca8:	[0-9a-f]* 	{ v2sub r15, r16, r17 ; v2packl r5, r6, r7 }
   13cb0:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; cmpexch4 r15, r16, r17 }
   13cb8:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; ld1u_add r15, r16, 5 }
   13cc0:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; prefetch_add_l1 r15, 5 }
   13cc8:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; stnt r15, r16 }
   13cd0:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; v2addi r15, r16, 5 }
   13cd8:	[0-9a-f]* 	{ v2sub r5, r6, r7 ; v4sub r15, r16, r17 }
   13ce0:	[0-9a-f]* 	{ v2subsc r15, r16, r17 ; dblalign2 r5, r6, r7 }
   13ce8:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; v2subsc r15, r16, r17 }
   13cf0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; v2subsc r15, r16, r17 }
   13cf8:	[0-9a-f]* 	{ v2subsc r15, r16, r17 ; v1shl r5, r6, r7 }
   13d00:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; v2subsc r15, r16, r17 }
   13d08:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; cmpltsi r15, r16, 5 }
   13d10:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; ld2u_add r15, r16, 5 }
   13d18:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; prefetch_add_l3 r15, 5 }
   13d20:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; stnt2_add r15, r16, 5 }
   13d28:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; v2cmples r15, r16, r17 }
   13d30:	[0-9a-f]* 	{ v2subsc r5, r6, r7 ; xori r15, r16, 5 }
   13d38:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v4add r15, r16, r17 }
   13d40:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v4add r15, r16, r17 }
   13d48:	[0-9a-f]* 	{ v4add r15, r16, r17 ; v1addi r5, r6, 5 }
   13d50:	[0-9a-f]* 	{ v4add r15, r16, r17 ; v1shru r5, r6, r7 }
   13d58:	[0-9a-f]* 	{ v4add r15, r16, r17 ; v2shlsc r5, r6, r7 }
   13d60:	[0-9a-f]* 	{ v4add r5, r6, r7 ; dblalign2 r15, r16, r17 }
   13d68:	[0-9a-f]* 	{ v4add r5, r6, r7 ; ld4u_add r15, r16, 5 }
   13d70:	[0-9a-f]* 	{ v4add r5, r6, r7 ; prefetch_l2 r15 }
   13d78:	[0-9a-f]* 	{ v4add r5, r6, r7 ; sub r15, r16, r17 }
   13d80:	[0-9a-f]* 	{ v4add r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   13d88:	[0-9a-f]* 	{ v4addsc r15, r16, r17 ; addx r5, r6, r7 }
   13d90:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; v4addsc r15, r16, r17 }
   13d98:	[0-9a-f]* 	{ v4addsc r15, r16, r17 ; mz r5, r6, r7 }
   13da0:	[0-9a-f]* 	{ v4addsc r15, r16, r17 ; v1cmpeq r5, r6, r7 }
   13da8:	[0-9a-f]* 	{ v4addsc r15, r16, r17 ; v2add r5, r6, r7 }
   13db0:	[0-9a-f]* 	{ v4addsc r15, r16, r17 ; v2shrui r5, r6, 5 }
   13db8:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; exch r15, r16, r17 }
   13dc0:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; ldnt r15, r16 }
   13dc8:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; raise }
   13dd0:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; v1addi r15, r16, 5 }
   13dd8:	[0-9a-f]* 	{ v4addsc r5, r6, r7 ; v2int_l r15, r16, r17 }
   13de0:	[0-9a-f]* 	{ v4int_h r15, r16, r17 ; and r5, r6, r7 }
   13de8:	[0-9a-f]* 	{ fsingle_add1 r5, r6, r7 ; v4int_h r15, r16, r17 }
   13df0:	[0-9a-f]* 	{ v4int_h r15, r16, r17 ; ori r5, r6, 5 }
   13df8:	[0-9a-f]* 	{ v4int_h r15, r16, r17 ; v1cmplts r5, r6, r7 }
   13e00:	[0-9a-f]* 	{ v2avgs r5, r6, r7 ; v4int_h r15, r16, r17 }
   13e08:	[0-9a-f]* 	{ v4int_h r15, r16, r17 ; v4addsc r5, r6, r7 }
   13e10:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; fetchaddgez r15, r16, r17 }
   13e18:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
   13e20:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; shl16insli r15, r16, 4660 }
   13e28:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; v1cmples r15, r16, r17 }
   13e30:	[0-9a-f]* 	{ v4int_h r5, r6, r7 ; v2minsi r15, r16, 5 }
   13e38:	[0-9a-f]* 	{ bfins r5, r6, 5, 7 ; v4int_l r15, r16, r17 }
   13e40:	[0-9a-f]* 	{ fsingle_pack1 r5, r6 ; v4int_l r15, r16, r17 }
   13e48:	[0-9a-f]* 	{ v4int_l r15, r16, r17 ; rotl r5, r6, r7 }
   13e50:	[0-9a-f]* 	{ v4int_l r15, r16, r17 ; v1cmpne r5, r6, r7 }
   13e58:	[0-9a-f]* 	{ v4int_l r15, r16, r17 ; v2cmpleu r5, r6, r7 }
   13e60:	[0-9a-f]* 	{ v4int_l r15, r16, r17 ; v4shl r5, r6, r7 }
   13e68:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; fetchor r15, r16, r17 }
   13e70:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
   13e78:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; shl2addx r15, r16, r17 }
   13e80:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; v1cmpltu r15, r16, r17 }
   13e88:	[0-9a-f]* 	{ v4int_l r5, r6, r7 ; v2packl r15, r16, r17 }
   13e90:	[0-9a-f]* 	{ v4packsc r15, r16, r17 ; cmpeq r5, r6, r7 }
   13e98:	[0-9a-f]* 	{ v4packsc r15, r16, r17 ; infol 4660 }
   13ea0:	[0-9a-f]* 	{ v4packsc r15, r16, r17 ; shl1add r5, r6, r7 }
   13ea8:	[0-9a-f]* 	{ v1ddotpusa r5, r6, r7 ; v4packsc r15, r16, r17 }
   13eb0:	[0-9a-f]* 	{ v4packsc r15, r16, r17 ; v2cmpltui r5, r6, 5 }
   13eb8:	[0-9a-f]* 	{ v4packsc r15, r16, r17 ; v4sub r5, r6, r7 }
   13ec0:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; flushwb }
   13ec8:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
   13ed0:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; shlx r15, r16, r17 }
   13ed8:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; v1int_l r15, r16, r17 }
   13ee0:	[0-9a-f]* 	{ v4packsc r5, r6, r7 ; v2shlsc r15, r16, r17 }
   13ee8:	[0-9a-f]* 	{ v4shl r15, r16, r17 ; cmplts r5, r6, r7 }
   13ef0:	[0-9a-f]* 	{ v4shl r15, r16, r17 ; movei r5, 5 }
   13ef8:	[0-9a-f]* 	{ v4shl r15, r16, r17 ; shl3add r5, r6, r7 }
   13f00:	[0-9a-f]* 	{ v1dotpua r5, r6, r7 ; v4shl r15, r16, r17 }
   13f08:	[0-9a-f]* 	{ v4shl r15, r16, r17 ; v2int_h r5, r6, r7 }
   13f10:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; add r15, r16, r17 }
   13f18:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; info 19 }
   13f20:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; mfspr r16, MEM_ERROR_CBOX_ADDR }
   13f28:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; shru r15, r16, r17 }
   13f30:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; v1minui r15, r16, 5 }
   13f38:	[0-9a-f]* 	{ v4shl r5, r6, r7 ; v2shrui r15, r16, 5 }
   13f40:	[0-9a-f]* 	{ v4shlsc r15, r16, r17 ; cmpne r5, r6, r7 }
   13f48:	[0-9a-f]* 	{ mul_hs_ls r5, r6, r7 ; v4shlsc r15, r16, r17 }
   13f50:	[0-9a-f]* 	{ v4shlsc r15, r16, r17 ; shlxi r5, r6, 5 }
   13f58:	[0-9a-f]* 	{ v4shlsc r15, r16, r17 ; v1int_l r5, r6, r7 }
   13f60:	[0-9a-f]* 	{ v4shlsc r15, r16, r17 ; v2mins r5, r6, r7 }
   13f68:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; addxi r15, r16, 5 }
   13f70:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; jalr r15 }
   13f78:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; moveli r15, 4660 }
   13f80:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; st r15, r16 }
   13f88:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; v1shli r15, r16, 5 }
   13f90:	[0-9a-f]* 	{ v4shlsc r5, r6, r7 ; v4addsc r15, r16, r17 }
   13f98:	[0-9a-f]* 	{ cmulf r5, r6, r7 ; v4shrs r15, r16, r17 }
   13fa0:	[0-9a-f]* 	{ mul_hu_lu r5, r6, r7 ; v4shrs r15, r16, r17 }
   13fa8:	[0-9a-f]* 	{ v4shrs r15, r16, r17 ; shrui r5, r6, 5 }
   13fb0:	[0-9a-f]* 	{ v4shrs r15, r16, r17 ; v1minui r5, r6, 5 }
   13fb8:	[0-9a-f]* 	{ v2muls r5, r6, r7 ; v4shrs r15, r16, r17 }
   13fc0:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; andi r15, r16, 5 }
   13fc8:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; ld r15, r16 }
   13fd0:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; nor r15, r16, r17 }
   13fd8:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; st2_add r15, r16, 5 }
   13fe0:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; v1shrui r15, r16, 5 }
   13fe8:	[0-9a-f]* 	{ v4shrs r5, r6, r7 ; v4shl r15, r16, r17 }
   13ff0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; v4shru r15, r16, r17 }
   13ff8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; v4shru r15, r16, r17 }
   14000:	[0-9a-f]* 	{ v4shru r15, r16, r17 ; sub r5, r6, r7 }
   14008:	[0-9a-f]* 	{ v1mulus r5, r6, r7 ; v4shru r15, r16, r17 }
   14010:	[0-9a-f]* 	{ v4shru r15, r16, r17 ; v2packl r5, r6, r7 }
   14018:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; cmpexch4 r15, r16, r17 }
   14020:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; ld1u_add r15, r16, 5 }
   14028:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; prefetch_add_l1 r15, 5 }
   14030:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; stnt r15, r16 }
   14038:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; v2addi r15, r16, 5 }
   14040:	[0-9a-f]* 	{ v4shru r5, r6, r7 ; v4sub r15, r16, r17 }
   14048:	[0-9a-f]* 	{ v4sub r15, r16, r17 ; dblalign2 r5, r6, r7 }
   14050:	[0-9a-f]* 	{ mula_hu_hu r5, r6, r7 ; v4sub r15, r16, r17 }
   14058:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; v4sub r15, r16, r17 }
   14060:	[0-9a-f]* 	{ v4sub r15, r16, r17 ; v1shl r5, r6, r7 }
   14068:	[0-9a-f]* 	{ v2sads r5, r6, r7 ; v4sub r15, r16, r17 }
   14070:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; cmpltsi r15, r16, 5 }
   14078:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; ld2u_add r15, r16, 5 }
   14080:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; prefetch_add_l3 r15, 5 }
   14088:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; stnt2_add r15, r16, 5 }
   14090:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; v2cmples r15, r16, r17 }
   14098:	[0-9a-f]* 	{ v4sub r5, r6, r7 ; xori r15, r16, 5 }
   140a0:	[0-9a-f]* 	{ fdouble_addsub r5, r6, r7 ; v4subsc r15, r16, r17 }
   140a8:	[0-9a-f]* 	{ mula_ls_lu r5, r6, r7 ; v4subsc r15, r16, r17 }
   140b0:	[0-9a-f]* 	{ v4subsc r15, r16, r17 ; v1addi r5, r6, 5 }
   140b8:	[0-9a-f]* 	{ v4subsc r15, r16, r17 ; v1shru r5, r6, r7 }
   140c0:	[0-9a-f]* 	{ v4subsc r15, r16, r17 ; v2shlsc r5, r6, r7 }
   140c8:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; dblalign2 r15, r16, r17 }
   140d0:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; ld4u_add r15, r16, 5 }
   140d8:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; prefetch_l2 r15 }
   140e0:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; sub r15, r16, r17 }
   140e8:	[0-9a-f]* 	{ v4subsc r5, r6, r7 ; v2cmpltu r15, r16, r17 }
   140f0:	[0-9a-f]* 	{ addx r5, r6, r7 ; wh64 r15 }
   140f8:	[0-9a-f]* 	{ fdouble_sub_flags r5, r6, r7 ; wh64 r15 }
   14100:	[0-9a-f]* 	{ mz r5, r6, r7 ; wh64 r15 }
   14108:	[0-9a-f]* 	{ v1cmpeq r5, r6, r7 ; wh64 r15 }
   14110:	[0-9a-f]* 	{ v2add r5, r6, r7 ; wh64 r15 }
   14118:	[0-9a-f]* 	{ v2shrui r5, r6, 5 ; wh64 r15 }
   14120:	[0-9a-f]* 	{ xor r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
   14128:	[0-9a-f]* 	{ xor r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
   14130:	[0-9a-f]* 	{ xor r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
   14138:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; xor r15, r16, r17 ; ld4s r25, r26 }
   14140:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
   14148:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
   14150:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
   14158:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
   14160:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; ld4s r25, r26 }
   14168:	[0-9a-f]* 	{ xor r15, r16, r17 ; st r25, r26 }
   14170:	[0-9a-f]* 	{ xor r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
   14178:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
   14180:	[0-9a-f]* 	{ cmoveqz r5, r6, r7 ; xor r15, r16, r17 ; ld1s r25, r26 }
   14188:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl2addx r5, r6, r7 ; ld1s r25, r26 }
   14190:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
   14198:	[0-9a-f]* 	{ xor r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
   141a0:	[0-9a-f]* 	{ xor r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
   141a8:	[0-9a-f]* 	{ xor r15, r16, r17 ; ld2u r25, r26 }
   141b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; ld2u r25, r26 }
   141b8:	[0-9a-f]* 	{ xor r15, r16, r17 ; nop ; ld4s r25, r26 }
   141c0:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4u r25, r26 }
   141c8:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrsi r5, r6, 5 ; ld4u r25, r26 }
   141d0:	[0-9a-f]* 	{ xor r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
   141d8:	[0-9a-f]* 	{ mul_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2 r25 }
   141e0:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
   141e8:	[0-9a-f]* 	{ mula_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
   141f0:	[0-9a-f]* 	{ mula_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; ld4s r25, r26 }
   141f8:	[0-9a-f]* 	{ mulax r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
   14200:	[0-9a-f]* 	{ xor r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
   14208:	[0-9a-f]* 	{ xor r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
   14210:	[0-9a-f]* 	{ pcnt r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
   14218:	[0-9a-f]* 	{ mulax r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
   14220:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
   14228:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
   14230:	[0-9a-f]* 	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
   14238:	[0-9a-f]* 	{ xor r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2 r25 }
   14240:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
   14248:	[0-9a-f]* 	{ xor r15, r16, r17 ; info 19 ; prefetch_l2_fault r25 }
   14250:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
   14258:	[0-9a-f]* 	{ xor r15, r16, r17 ; or r5, r6, r7 ; prefetch_l3 r25 }
   14260:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
   14268:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrui r5, r6, 5 ; prefetch_l3_fault r25 }
   14270:	[0-9a-f]* 	{ revbytes r5, r6 ; xor r15, r16, r17 ; prefetch_l3 r25 }
   14278:	[0-9a-f]* 	{ xor r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
   14280:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
   14288:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
   14290:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
   14298:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
   142a0:	[0-9a-f]* 	{ xor r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
   142a8:	[0-9a-f]* 	{ xor r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
   142b0:	[0-9a-f]* 	{ xor r15, r16, r17 ; rotl r5, r6, r7 ; st r25, r26 }
   142b8:	[0-9a-f]* 	{ xor r15, r16, r17 ; st1 r25, r26 }
   142c0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; st1 r25, r26 }
   142c8:	[0-9a-f]* 	{ xor r15, r16, r17 ; nop ; st2 r25, r26 }
   142d0:	[0-9a-f]* 	{ xor r15, r16, r17 ; cmpleu r5, r6, r7 ; st4 r25, r26 }
   142d8:	[0-9a-f]* 	{ xor r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
   142e0:	[0-9a-f]* 	{ xor r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
   142e8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
   142f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
   142f8:	[0-9a-f]* 	{ xor r15, r16, r17 ; v1mz r5, r6, r7 }
   14300:	[0-9a-f]* 	{ xor r15, r16, r17 ; v2packuc r5, r6, r7 }
   14308:	[0-9a-f]* 	{ xor r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
   14310:	[0-9a-f]* 	{ xor r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
   14318:	[0-9a-f]* 	{ xor r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
   14320:	[0-9a-f]* 	{ xor r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
   14328:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpexch r15, r16, r17 }
   14330:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
   14338:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
   14340:	[0-9a-f]* 	{ xor r5, r6, r7 ; dtlbpr r15 }
   14348:	[0-9a-f]* 	{ xor r5, r6, r7 ; ill ; ld4u r25, r26 }
   14350:	[0-9a-f]* 	{ xor r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
   14358:	[0-9a-f]* 	{ xor r5, r6, r7 ; jr r15 ; prefetch r25 }
   14360:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
   14368:	[0-9a-f]* 	{ xor r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
   14370:	[0-9a-f]* 	{ xor r5, r6, r7 ; shrsi r15, r16, 5 ; ld1s r25, r26 }
   14378:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
   14380:	[0-9a-f]* 	{ xor r5, r6, r7 ; mnz r15, r16, r17 ; ld2s r25, r26 }
   14388:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
   14390:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
   14398:	[0-9a-f]* 	{ xor r5, r6, r7 ; subx r15, r16, r17 ; ld4s r25, r26 }
   143a0:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
   143a8:	[0-9a-f]* 	{ xor r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
   143b0:	[0-9a-f]* 	{ xor r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
   143b8:	[0-9a-f]* 	{ xor r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
   143c0:	[0-9a-f]* 	{ xor r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
   143c8:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
   143d0:	[0-9a-f]* 	{ xor r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
   143d8:	[0-9a-f]* 	{ xor r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
   143e0:	[0-9a-f]* 	{ xor r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
   143e8:	[0-9a-f]* 	{ xor r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l2 r25 }
   143f0:	[0-9a-f]* 	{ xor r5, r6, r7 ; prefetch_l2_fault r25 }
   143f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3 r25 }
   14400:	[0-9a-f]* 	{ xor r5, r6, r7 ; prefetch_l3 r25 }
   14408:	[0-9a-f]* 	{ xor r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l3_fault r25 }
   14410:	[0-9a-f]* 	{ xor r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
   14418:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
   14420:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
   14428:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
   14430:	[0-9a-f]* 	{ xor r5, r6, r7 ; shli r15, r16, 5 }
   14438:	[0-9a-f]* 	{ xor r5, r6, r7 ; shrsi r15, r16, 5 }
   14440:	[0-9a-f]* 	{ xor r5, r6, r7 ; shruxi r15, r16, 5 }
   14448:	[0-9a-f]* 	{ xor r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
   14450:	[0-9a-f]* 	{ xor r5, r6, r7 ; rotli r15, r16, 5 ; st1 r25, r26 }
   14458:	[0-9a-f]* 	{ xor r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
   14460:	[0-9a-f]* 	{ xor r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
   14468:	[0-9a-f]* 	{ xor r5, r6, r7 ; stnt2 r15, r16 }
   14470:	[0-9a-f]* 	{ xor r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
   14478:	[0-9a-f]* 	{ xor r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
   14480:	[0-9a-f]* 	{ xor r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
   14488:	[0-9a-f]* 	{ cmul r5, r6, r7 ; xori r15, r16, 5 }
   14490:	[0-9a-f]* 	{ mul_hs_lu r5, r6, r7 ; xori r15, r16, 5 }
   14498:	[0-9a-f]* 	{ xori r15, r16, 5 ; shrs r5, r6, r7 }
   144a0:	[0-9a-f]* 	{ xori r15, r16, 5 ; v1maxu r5, r6, r7 }
   144a8:	[0-9a-f]* 	{ xori r15, r16, 5 ; v2minsi r5, r6, 5 }
   144b0:	[0-9a-f]* 	{ xori r5, r6, 5 ; addxli r15, r16, 4660 }
   144b8:	[0-9a-f]* 	{ xori r5, r6, 5 ; jalrp r15 }
   144c0:	[0-9a-f]* 	{ xori r5, r6, 5 ; mtspr MEM_ERROR_CBOX_ADDR, r16 }
   144c8:	[0-9a-f]* 	{ xori r5, r6, 5 ; st1 r15, r16 }
   144d0:	[0-9a-f]* 	{ xori r5, r6, 5 ; v1shrs r15, r16, r17 }
   144d8:	[0-9a-f]* 	{ xori r5, r6, 5 ; v4int_h r15, r16, r17 }
