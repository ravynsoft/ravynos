#as:
#objdump: -dr

.*:     file format .*


Disassembly of section .text:

00000000 <target>:
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
     100:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; bbnst r15, 0 <target> }
     108:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; blezt r15, 0 <target> }
     110:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; bbnst r15, 0 <target> }
     118:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; bgezt r15, 0 <target> }
     120:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; bzt r15, 0 <target> }
     128:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; bbnst r15, 0 <target> }
     130:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; bgzt r15, 0 <target> }
     138:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; blezt r15, 0 <target> }
     140:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; blzt r15, 0 <target> }
     148:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; bbnst r15, 0 <target> }
     150:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; bgzt r15, 0 <target> }
     158:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; bz r15, 0 <target> }
     160:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; blzt r15, 0 <target> }
     168:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; blzt r15, 0 <target> }
     170:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; bzt r15, 0 <target> }
     178:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; bbst r15, 0 <target> }
     180:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; bbs r15, 0 <target> }
     188:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; bz r15, 0 <target> }
     190:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; blzt r15, 0 <target> }
     198:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; bgez r15, 0 <target> }
     1a0:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; bbns r15, 0 <target> }
     1a8:	[0-9a-f]* 	{ auli r5, r6, 4660 ; bzt r15, 0 <target> }
     1b0:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; bgezt r15, 0 <target> }
     1b8:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; blez r15, 0 <target> }
     1c0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; bz r15, 0 <target> }
     1c8:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; bzt r15, 0 <target> }
     1d0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; bz r15, 0 <target> }
     1d8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; bgzt r15, 0 <target> }
     1e0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; bbnst r15, 0 <target> }
     1e8:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; bbnst r15, 0 <target> }
     1f0:	[0-9a-f]* 	{ addhs r5, r6, r7 ; blezt r15, 0 <target> }
     1f8:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; blz r15, 0 <target> }
     200:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; blzt r15, 0 <target> }
     208:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; blez r15, 0 <target> }
     210:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; bz r15, 0 <target> }
     218:	[0-9a-f]* 	{ packhs r5, r6, r7 ; bnzt r15, 0 <target> }
     220:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; bzt r15, 0 <target> }
     228:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; bgez r15, 0 <target> }
     230:	[0-9a-f]* 	{ slteh r5, r6, r7 ; bbnst r15, 0 <target> }
     238:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; bgez r15, 0 <target> }
     240:	[0-9a-f]* 	{ addb r5, r6, r7 ; bbnst r15, 0 <target> }
     248:	[0-9a-f]* 	{ adds r5, r6, r7 ; bbnst r15, 0 <target> }
     250:	[0-9a-f]* 	{ inthb r5, r6, r7 ; bgez r15, 0 <target> }
     258:	[0-9a-f]* 	{ intlh r5, r6, r7 ; bbst r15, 0 <target> }
     260:	[0-9a-f]* 	{ maxih r5, r6, 5 ; bgezt r15, 0 <target> }
     268:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; blezt r15, 0 <target> }
     270:	[0-9a-f]* 	{ packhs r5, r6, r7 ; blz r15, 0 <target> }
     278:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; bnz r15, 0 <target> }
     280:	[0-9a-f]* 	{ seqih r5, r6, 5 ; bgezt r15, 0 <target> }
     288:	[0-9a-f]* 	{ shrib r5, r6, 5 ; bbnst r15, 0 <target> }
     290:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; bzt r15, 0 <target> }
     298:	[0-9a-f]* 	{ slteh r5, r6, r7 ; bgzt r15, 0 <target> }
     2a0:	[0-9a-f]* 	{ sltib r5, r6, 5 ; bbnst r15, 0 <target> }
     2a8:	[0-9a-f]* 	{ sneh r5, r6, r7 ; bgezt r15, 0 <target> }
     2b0:	[0-9a-f]* 	{ subh r5, r6, r7 ; blezt r15, 0 <target> }
     2b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; bbnst r15, 0 <target> }
     2c0:	[0-9a-f]* 	{ addhs r5, r6, r7 ; bbs r15, 0 <target> }
     2c8:	[0-9a-f]* 	{ addih r5, r6, 5 ; blzt r15, 0 <target> }
     2d0:	[0-9a-f]* 	{ avgh r5, r6, r7 ; bgez r15, 0 <target> }
     2d8:	[0-9a-f]* 	{ intlh r5, r6, r7 ; bbs r15, 0 <target> }
     2e0:	[0-9a-f]* 	{ maxih r5, r6, 5 ; bnzt r15, 0 <target> }
     2e8:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; bbns r15, 0 <target> }
     2f0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; bgez r15, 0 <target> }
     2f8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; bbnst r15, 0 <target> }
     300:	[0-9a-f]* 	{ sadh r5, r6, r7 ; blzt r15, 0 <target> }
     308:	[0-9a-f]* 	{ seqi r5, r6, 5 ; bbnst r15, 0 <target> }
     310:	[0-9a-f]* 	{ shlb r5, r6, r7 ; bbns r15, 0 <target> }
     318:	[0-9a-f]* 	{ shlib r5, r6, 5 ; bgzt r15, 0 <target> }
     320:	[0-9a-f]* 	{ shrb r5, r6, r7 ; bnzt r15, 0 <target> }
     328:	[0-9a-f]* 	{ shrih r5, r6, 5 ; bgez r15, 0 <target> }
     330:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; bz r15, 0 <target> }
     338:	[0-9a-f]* 	{ slth r5, r6, r7 ; bbst r15, 0 <target> }
     340:	[0-9a-f]* 	{ sltib r5, r6, 5 ; blzt r15, 0 <target> }
     348:	[0-9a-f]* 	{ sneb r5, r6, r7 ; bnzt r15, 0 <target> }
     350:	[0-9a-f]* 	{ srah r5, r6, r7 ; bgez r15, 0 <target> }
     358:	[0-9a-f]* 	{ sraih r5, r6, 5 ; blzt r15, 0 <target> }
     360:	[0-9a-f]* 	{ subhs r5, r6, r7 ; bgz r15, 0 <target> }
     368:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; bgez r15, 0 <target> }
     370:	[0-9a-f]* 	{ xor r5, r6, r7 ; bgezt r15, 0 <target> }
     378:	[0-9a-f]* 	{ addh r5, r6, r7 ; bnz r15, 0 <target> }
     380:	[0-9a-f]* 	{ addli r5, r6, 4660 ; jal 0 <target> }
     388:	[0-9a-f]* 	{ avgh r5, r6, r7 ; bbs r15, 0 <target> }
     390:	[0-9a-f]* 	{ minh r5, r6, r7 ; bbs r15, 0 <target> }
     398:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; bnz r15, 0 <target> }
     3a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; bnz r15, 0 <target> }
     3a8:	[0-9a-f]* 	{ mzh r5, r6, r7 ; bbst r15, 0 <target> }
     3b0:	[0-9a-f]* 	{ rl r5, r6, r7 ; bgezt r15, 0 <target> }
     3b8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; bbst r15, 0 <target> }
     3c0:	[0-9a-f]* 	{ seqb r5, r6, r7 ; bgz r15, 0 <target> }
     3c8:	[0-9a-f]* 	{ seqib r5, r6, 5 ; bzt r15, 0 <target> }
     3d0:	[0-9a-f]* 	{ shlh r5, r6, r7 ; blz r15, 0 <target> }
     3d8:	[0-9a-f]* 	{ shr r5, r6, r7 ; bbns r15, 0 <target> }
     3e0:	[0-9a-f]* 	{ shri r5, r6, 5 ; bgzt r15, 0 <target> }
     3e8:	[0-9a-f]* 	{ slt r5, r6, r7 ; bnzt r15, 0 <target> }
     3f0:	[0-9a-f]* 	{ slti r5, r6, 5 ; bbst r15, 0 <target> }
     3f8:	[0-9a-f]* 	{ sne r5, r6, r7 ; bgzt r15, 0 <target> }
     400:	[0-9a-f]* 	{ sra r5, r6, r7 ; bnzt r15, 0 <target> }
     408:	[0-9a-f]* 	{ sraib r5, r6, 5 ; blz r15, 0 <target> }
     410:	[0-9a-f]* 	{ subh r5, r6, r7 ; bbs r15, 0 <target> }
     418:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; bzt r15, 0 <target> }
     420:	[0-9a-f]* 	{ xori r5, r6, 5 ; bgez r15, 0 <target> }
     428:	[0-9a-f]* 	{ adds r5, r6, r7 ; bz r15, 0 <target> }
     430:	[0-9a-f]* 	{ infol 4660 ; blezt r15, 0 <target> }
     438:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; jal 0 <target> }
     440:	[0-9a-f]* 	{ mzb r5, r6, r7 ; bgz r15, 0 <target> }
     448:	[0-9a-f]* 	{ or r5, r6, r7 ; bnzt r15, 0 <target> }
     450:	[0-9a-f]* 	{ rli r5, r6, 5 ; blez r15, 0 <target> }
     458:	[0-9a-f]* 	{ seq r5, r6, r7 ; bgz r15, 0 <target> }
     460:	[0-9a-f]* 	{ shli r5, r6, 5 ; bbs r15, 0 <target> }
     468:	[0-9a-f]* 	{ shrih r5, r6, 5 ; bz r15, 0 <target> }
     470:	[0-9a-f]* 	{ sne r5, r6, r7 ; bzt r15, 0 <target> }
     478:	[0-9a-f]* 	{ sub r5, r6, r7 ; bnz r15, 0 <target> }
     480:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; jal 0 <target> }
     488:	[0-9a-f]* 	{ infol 4660 ; blez r15, 0 <target> }
     490:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; j 0 <target> }
     498:	[0-9a-f]* 	{ pcnt r5, r6 ; bbnst r15, 0 <target> }
     4a0:	[0-9a-f]* 	{ shl r5, r6, r7 ; bz r15, 0 <target> }
     4a8:	[0-9a-f]* 	{ bitx r5, r6 ; bbst r15, 0 <target> }
     4b0:	[0-9a-f]* 	{ infol 4660 ; blz r15, 0 <target> }
     4b8:	[0-9a-f]* 	{ movei r5, 5 ; blzt r15, 0 <target> }
     4c0:	[0-9a-f]* 	{ pcnt r5, r6 ; bbns r15, 0 <target> }
     4c8:	[0-9a-f]* 	{ bitx r5, r6 ; blz r15, 0 <target> }
     4d0:	[0-9a-f]* 	{ inthb r5, r6, r7 ; jal 0 <target> }
     4d8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; j 0 <target> }
     4e0:	[0-9a-f]* 	{ clz r5, r6 ; bbs r15, 0 <target> }
     4e8:	[0-9a-f]* 	{ move r5, r6 ; bz r15, 0 <target> }
     4f0:	[0-9a-f]* 	{ shrh r5, r6, r7 ; jal 0 <target> }
     4f8:	[0-9a-f]* 	{ subh r5, r6, r7 ; jal 0 <target> }
     500:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jal 0 <target> }
     508:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; j 0 <target> }
     510:	[0-9a-f]* 	{ info 19 ; bnzt r15, 0 <target> }
     518:	[0-9a-f]* 	{ shlib r5, r6, 5 ; j 0 <target> }
     520:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; j 0 <target> }
     528:	[0-9a-f]* 	{ s1a r5, r6, r7 ; j 0 <target> }
     530:	[0-9a-f]* 	{ blezt r15, 0 <target> }
     538:	[0-9a-f]* 	{ infol 4660 ; j 0 <target> }
     540:	[0-9a-f]* 	{ clz r5, r6 ; j 0 <target> }
     548:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; bbnst r15, 0 <target> }
     550:	[0-9a-f]* 	{ inthh r5, r6, r7 ; bbnst r15, 0 <target> }
     558:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; bbnst r15, 0 <target> }
     560:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; bbnst r15, 0 <target> }
     568:	[0-9a-f]* 	{ s3a r5, r6, r7 ; bbnst r15, 0 <target> }
     570:	[0-9a-f]* 	{ shrb r5, r6, r7 ; bbnst r15, 0 <target> }
     578:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; bbnst r15, 0 <target> }
     580:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; bbnst r15, 0 <target> }
     588:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; bgezt r15, 0 <target> }
     590:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; bgezt r15, 0 <target> }
     598:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; bgezt r15, 0 <target> }
     5a0:	[0-9a-f]* 	{ nop ; bgezt r15, 0 <target> }
     5a8:	[0-9a-f]* 	{ seq r5, r6, r7 ; bgezt r15, 0 <target> }
     5b0:	[0-9a-f]* 	{ sltb r5, r6, r7 ; bgezt r15, 0 <target> }
     5b8:	[0-9a-f]* 	{ srab r5, r6, r7 ; bgezt r15, 0 <target> }
     5c0:	[0-9a-f]* 	{ addh r5, r6, r7 ; blezt r15, 0 <target> }
     5c8:	[0-9a-f]* 	{ ctz r5, r6 ; blezt r15, 0 <target> }
     5d0:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; blezt r15, 0 <target> }
     5d8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; blezt r15, 0 <target> }
     5e0:	[0-9a-f]* 	{ packlb r5, r6, r7 ; blezt r15, 0 <target> }
     5e8:	[0-9a-f]* 	{ shlb r5, r6, r7 ; blezt r15, 0 <target> }
     5f0:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; blezt r15, 0 <target> }
     5f8:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; blezt r15, 0 <target> }
     600:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; bbns r15, 0 <target> }
     608:	[0-9a-f]* 	{ inthh r5, r6, r7 ; bbns r15, 0 <target> }
     610:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; bbns r15, 0 <target> }
     618:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; bbns r15, 0 <target> }
     620:	[0-9a-f]* 	{ s3a r5, r6, r7 ; bbns r15, 0 <target> }
     628:	[0-9a-f]* 	{ shrb r5, r6, r7 ; bbns r15, 0 <target> }
     630:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; bbns r15, 0 <target> }
     638:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; bbns r15, 0 <target> }
     640:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; bbst r15, 0 <target> }
     648:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; bbst r15, 0 <target> }
     650:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; bbst r15, 0 <target> }
     658:	[0-9a-f]* 	{ nop ; bbst r15, 0 <target> }
     660:	[0-9a-f]* 	{ seq r5, r6, r7 ; bbst r15, 0 <target> }
     668:	[0-9a-f]* 	{ sltb r5, r6, r7 ; bbst r15, 0 <target> }
     670:	[0-9a-f]* 	{ srab r5, r6, r7 ; bbst r15, 0 <target> }
     678:	[0-9a-f]* 	{ addh r5, r6, r7 ; bgez r15, 0 <target> }
     680:	[0-9a-f]* 	{ ctz r5, r6 ; bgez r15, 0 <target> }
     688:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; bgez r15, 0 <target> }
     690:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; bgez r15, 0 <target> }
     698:	[0-9a-f]* 	{ packlb r5, r6, r7 ; bgez r15, 0 <target> }
     6a0:	[0-9a-f]* 	{ shlb r5, r6, r7 ; bgez r15, 0 <target> }
     6a8:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; bgez r15, 0 <target> }
     6b0:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; bgez r15, 0 <target> }
     6b8:	[0-9a-f]* 	{ adds r5, r6, r7 ; bgzt r15, 0 <target> }
     6c0:	[0-9a-f]* 	{ intlb r5, r6, r7 ; bgzt r15, 0 <target> }
     6c8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; bgzt r15, 0 <target> }
     6d0:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; bgzt r15, 0 <target> }
     6d8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; bgzt r15, 0 <target> }
     6e0:	[0-9a-f]* 	{ shrh r5, r6, r7 ; bgzt r15, 0 <target> }
     6e8:	[0-9a-f]* 	{ sltih r5, r6, 5 ; bgzt r15, 0 <target> }
     6f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; bgzt r15, 0 <target> }
     6f8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; blez r15, 0 <target> }
     700:	[0-9a-f]* 	{ minh r5, r6, r7 ; blez r15, 0 <target> }
     708:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; blez r15, 0 <target> }
     710:	[0-9a-f]* 	{ nor r5, r6, r7 ; blez r15, 0 <target> }
     718:	[0-9a-f]* 	{ seqb r5, r6, r7 ; blez r15, 0 <target> }
     720:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; blez r15, 0 <target> }
     728:	[0-9a-f]* 	{ srah r5, r6, r7 ; blez r15, 0 <target> }
     730:	[0-9a-f]* 	{ addhs r5, r6, r7 ; blzt r15, 0 <target> }
     738:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; blzt r15, 0 <target> }
     740:	[0-9a-f]* 	{ move r5, r6 ; blzt r15, 0 <target> }
     748:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; blzt r15, 0 <target> }
     750:	[0-9a-f]* 	{ pcnt r5, r6 ; blzt r15, 0 <target> }
     758:	[0-9a-f]* 	{ shlh r5, r6, r7 ; blzt r15, 0 <target> }
     760:	[0-9a-f]* 	{ slth r5, r6, r7 ; blzt r15, 0 <target> }
     768:	[0-9a-f]* 	{ subh r5, r6, r7 ; blzt r15, 0 <target> }
     770:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; bnzt r15, 0 <target> }
     778:	[0-9a-f]* 	{ intlh r5, r6, r7 ; bnzt r15, 0 <target> }
     780:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; bnzt r15, 0 <target> }
     788:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; bnzt r15, 0 <target> }
     790:	[0-9a-f]* 	{ sadah r5, r6, r7 ; bnzt r15, 0 <target> }
     798:	[0-9a-f]* 	{ shri r5, r6, 5 ; bnzt r15, 0 <target> }
     7a0:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; bnzt r15, 0 <target> }
     7a8:	[0-9a-f]* 	{ xor r5, r6, r7 ; bnzt r15, 0 <target> }
     7b0:	[0-9a-f]* 	{ avgh r5, r6, r7 ; bbs r15, 0 <target> }
     7b8:	[0-9a-f]* 	{ minh r5, r6, r7 ; bbs r15, 0 <target> }
     7c0:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; bbs r15, 0 <target> }
     7c8:	[0-9a-f]* 	{ nor r5, r6, r7 ; bbs r15, 0 <target> }
     7d0:	[0-9a-f]* 	{ seqb r5, r6, r7 ; bbs r15, 0 <target> }
     7d8:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; bbs r15, 0 <target> }
     7e0:	[0-9a-f]* 	{ srah r5, r6, r7 ; bbs r15, 0 <target> }
     7e8:	[0-9a-f]* 	{ addhs r5, r6, r7 ; bgz r15, 0 <target> }
     7f0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; bgz r15, 0 <target> }
     7f8:	[0-9a-f]* 	{ move r5, r6 ; bgz r15, 0 <target> }
     800:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; bgz r15, 0 <target> }
     808:	[0-9a-f]* 	{ pcnt r5, r6 ; bgz r15, 0 <target> }
     810:	[0-9a-f]* 	{ shlh r5, r6, r7 ; bgz r15, 0 <target> }
     818:	[0-9a-f]* 	{ slth r5, r6, r7 ; bgz r15, 0 <target> }
     820:	[0-9a-f]* 	{ subh r5, r6, r7 ; bgz r15, 0 <target> }
     828:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; blz r15, 0 <target> }
     830:	[0-9a-f]* 	{ intlh r5, r6, r7 ; blz r15, 0 <target> }
     838:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; blz r15, 0 <target> }
     840:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; blz r15, 0 <target> }
     848:	[0-9a-f]* 	{ sadah r5, r6, r7 ; blz r15, 0 <target> }
     850:	[0-9a-f]* 	{ shri r5, r6, 5 ; blz r15, 0 <target> }
     858:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; blz r15, 0 <target> }
     860:	[0-9a-f]* 	{ xor r5, r6, r7 ; blz r15, 0 <target> }
     868:	[0-9a-f]* 	{ bitx r5, r6 ; bnz r15, 0 <target> }
     870:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; bnz r15, 0 <target> }
     878:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; bnz r15, 0 <target> }
     880:	[0-9a-f]* 	{ or r5, r6, r7 ; bnz r15, 0 <target> }
     888:	[0-9a-f]* 	{ seqh r5, r6, r7 ; bnz r15, 0 <target> }
     890:	[0-9a-f]* 	{ slte r5, r6, r7 ; bnz r15, 0 <target> }
     898:	[0-9a-f]* 	{ srai r5, r6, 5 ; bnz r15, 0 <target> }
     8a0:	[0-9a-f]* 	{ addi r5, r6, 5 ; bzt r15, 0 <target> }
     8a8:	[0-9a-f]* 	{ bzt r15, 0 <target> }
     8b0:	[0-9a-f]* 	{ movei r5, 5 ; bzt r15, 0 <target> }
     8b8:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; bzt r15, 0 <target> }
     8c0:	[0-9a-f]* 	{ rl r5, r6, r7 ; bzt r15, 0 <target> }
     8c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; bzt r15, 0 <target> }
     8d0:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; bzt r15, 0 <target> }
     8d8:	[0-9a-f]* 	{ subhs r5, r6, r7 ; bzt r15, 0 <target> }
     8e0:	[0-9a-f]* 	{ addli r5, r6, 4660 ; bz r15, 0 <target> }
     8e8:	[0-9a-f]* 	{ inthb r5, r6, r7 ; bz r15, 0 <target> }
     8f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; bz r15, 0 <target> }
     8f8:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; bz r15, 0 <target> }
     900:	[0-9a-f]* 	{ s2a r5, r6, r7 ; bz r15, 0 <target> }
     908:	[0-9a-f]* 	{ shr r5, r6, r7 ; bz r15, 0 <target> }
     910:	[0-9a-f]* 	{ sltib r5, r6, 5 ; bz r15, 0 <target> }
     918:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; bz r15, 0 <target> }
     920:	[0-9a-f]* 	{ addb r5, r6, r7 ; jal 0 <target> }
     928:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; jal 0 <target> }
     930:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jal 0 <target> }
     938:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; jal 0 <target> }
     940:	[0-9a-f]* 	{ packhb r5, r6, r7 ; jal 0 <target> }
     948:	[0-9a-f]* 	{ seqih r5, r6, 5 ; jal 0 <target> }
     950:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; jal 0 <target> }
     958:	[0-9a-f]* 	{ sub r5, r6, r7 ; jal 0 <target> }
     960:	[0-9a-f]* 	{ addih r5, r6, 5 ; j 0 <target> }
     968:	[0-9a-f]* 	{ infol 4660 ; j 0 <target> }
     970:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; j 0 <target> }
     978:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; j 0 <target> }
     980:	[0-9a-f]* 	{ s1a r5, r6, r7 ; j 0 <target> }
     988:	[0-9a-f]* 	{ shlih r5, r6, 5 ; j 0 <target> }
     990:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; j 0 <target> }
     998:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; j 0 <target> }
     9a0:	[0-9a-f]* 	{ and r5, r6, r7 }
     9a8:	[0-9a-f]* 	{ info 19 }
     9b0:	[0-9a-f]* 	{ lnk r5 }
     9b8:	[0-9a-f]* 	{ movei r5, 5 }
     9c0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 }
     9c8:	[0-9a-f]* 	{ packlb r5, r6, r7 }
     9d0:	[0-9a-f]* 	{ seqi r5, r6, 5 }
     9d8:	[0-9a-f]* 	{ sltb_u r5, r6, r7 }
     9e0:	[0-9a-f]* 	{ srah r5, r6, r7 }
     9e8:	[0-9a-f]* 	{ tns r5, r6 }
     9f0:	[0-9a-f]* 	{ add r15, r16, r17 ; addi r5, r6, 5 ; lh r25, r26 }
     9f8:	[0-9a-f]* 	{ add r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
     a00:	[0-9a-f]* 	{ bitx r5, r6 ; add r15, r16, r17 ; lh r25, r26 }
     a08:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; lh r25, r26 }
     a10:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; add r15, r16, r17 }
     a18:	[0-9a-f]* 	{ add r15, r16, r17 ; info 19 }
     a20:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; add r15, r16, r17 ; lb r25, r26 }
     a28:	[0-9a-f]* 	{ add r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
     a30:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; lb r25, r26 }
     a38:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
     a40:	[0-9a-f]* 	{ add r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
     a48:	[0-9a-f]* 	{ add r15, r16, r17 ; add r5, r6, r7 ; lh r25, r26 }
     a50:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
     a58:	[0-9a-f]* 	{ add r15, r16, r17 ; shri r5, r6, 5 ; lh r25, r26 }
     a60:	[0-9a-f]* 	{ add r15, r16, r17 ; andi r5, r6, 5 ; lh_u r25, r26 }
     a68:	[0-9a-f]* 	{ mvz r5, r6, r7 ; add r15, r16, r17 ; lh_u r25, r26 }
     a70:	[0-9a-f]* 	{ add r15, r16, r17 ; slte r5, r6, r7 ; lh_u r25, r26 }
     a78:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; lw r25, r26 }
     a80:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
     a88:	[0-9a-f]* 	{ add r15, r16, r17 ; slti_u r5, r6, 5 ; lw r25, r26 }
     a90:	[0-9a-f]* 	{ add r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
     a98:	[0-9a-f]* 	{ add r15, r16, r17 ; move r5, r6 ; sw r25, r26 }
     aa0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
     aa8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     ab0:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; add r15, r16, r17 }
     ab8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     ac0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
     ac8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
     ad0:	[0-9a-f]* 	{ add r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
     ad8:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
     ae0:	[0-9a-f]* 	{ add r15, r16, r17 ; ori r5, r6, 5 ; lb r25, r26 }
     ae8:	[0-9a-f]* 	{ pcnt r5, r6 ; add r15, r16, r17 ; sb r25, r26 }
     af0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
     af8:	[0-9a-f]* 	{ add r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
     b00:	[0-9a-f]* 	{ add r15, r16, r17 ; prefetch r25 }
     b08:	[0-9a-f]* 	{ add r15, r16, r17 ; rli r5, r6, 5 }
     b10:	[0-9a-f]* 	{ add r15, r16, r17 ; s2a r5, r6, r7 }
     b18:	[0-9a-f]* 	{ add r15, r16, r17 ; andi r5, r6, 5 ; sb r25, r26 }
     b20:	[0-9a-f]* 	{ mvz r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
     b28:	[0-9a-f]* 	{ add r15, r16, r17 ; slte r5, r6, r7 ; sb r25, r26 }
     b30:	[0-9a-f]* 	{ add r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
     b38:	[0-9a-f]* 	{ add r15, r16, r17 ; and r5, r6, r7 ; sh r25, r26 }
     b40:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
     b48:	[0-9a-f]* 	{ add r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
     b50:	[0-9a-f]* 	{ add r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
     b58:	[0-9a-f]* 	{ add r15, r16, r17 ; shr r5, r6, r7 ; lb_u r25, r26 }
     b60:	[0-9a-f]* 	{ add r15, r16, r17 ; shri r5, r6, 5 }
     b68:	[0-9a-f]* 	{ add r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
     b70:	[0-9a-f]* 	{ add r15, r16, r17 ; slte_u r5, r6, r7 ; prefetch r25 }
     b78:	[0-9a-f]* 	{ add r15, r16, r17 ; slti r5, r6, 5 }
     b80:	[0-9a-f]* 	{ add r15, r16, r17 ; sne r5, r6, r7 ; prefetch r25 }
     b88:	[0-9a-f]* 	{ add r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
     b90:	[0-9a-f]* 	{ add r15, r16, r17 ; sub r5, r6, r7 }
     b98:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; add r15, r16, r17 ; sw r25, r26 }
     ba0:	[0-9a-f]* 	{ add r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
     ba8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; sw r25, r26 }
     bb0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
     bb8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
     bc0:	[0-9a-f]* 	{ add r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
     bc8:	[0-9a-f]* 	{ add r5, r6, r7 ; addli r15, r16, 4660 }
     bd0:	[0-9a-f]* 	{ add r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
     bd8:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; lh r25, r26 }
     be0:	[0-9a-f]* 	{ add r5, r6, r7 ; inthh r15, r16, r17 }
     be8:	[0-9a-f]* 	{ add r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
     bf0:	[0-9a-f]* 	{ add r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
     bf8:	[0-9a-f]* 	{ add r5, r6, r7 ; nop ; lb_u r25, r26 }
     c00:	[0-9a-f]* 	{ add r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
     c08:	[0-9a-f]* 	{ add r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
     c10:	[0-9a-f]* 	{ add r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
     c18:	[0-9a-f]* 	{ add r5, r6, r7 ; nop ; lh_u r25, r26 }
     c20:	[0-9a-f]* 	{ add r5, r6, r7 ; slti_u r15, r16, 5 ; lh_u r25, r26 }
     c28:	[0-9a-f]* 	{ add r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
     c30:	[0-9a-f]* 	{ add r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
     c38:	[0-9a-f]* 	{ add r5, r6, r7 ; minib_u r15, r16, 5 }
     c40:	[0-9a-f]* 	{ add r5, r6, r7 ; move r15, r16 ; prefetch r25 }
     c48:	[0-9a-f]* 	{ add r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
     c50:	[0-9a-f]* 	{ add r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
     c58:	[0-9a-f]* 	{ add r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
     c60:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; prefetch r25 }
     c68:	[0-9a-f]* 	{ add r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
     c70:	[0-9a-f]* 	{ add r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
     c78:	[0-9a-f]* 	{ add r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
     c80:	[0-9a-f]* 	{ add r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
     c88:	[0-9a-f]* 	{ add r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
     c90:	[0-9a-f]* 	{ add r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
     c98:	[0-9a-f]* 	{ add r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
     ca0:	[0-9a-f]* 	{ add r5, r6, r7 ; nop ; sh r25, r26 }
     ca8:	[0-9a-f]* 	{ add r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
     cb0:	[0-9a-f]* 	{ add r5, r6, r7 ; shli r15, r16, 5 ; lb r25, r26 }
     cb8:	[0-9a-f]* 	{ add r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
     cc0:	[0-9a-f]* 	{ add r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
     cc8:	[0-9a-f]* 	{ add r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
     cd0:	[0-9a-f]* 	{ add r5, r6, r7 ; slteh r15, r16, r17 }
     cd8:	[0-9a-f]* 	{ add r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
     ce0:	[0-9a-f]* 	{ add r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
     ce8:	[0-9a-f]* 	{ add r5, r6, r7 ; srai r15, r16, 5 ; sw r25, r26 }
     cf0:	[0-9a-f]* 	{ add r5, r6, r7 ; add r15, r16, r17 ; sw r25, r26 }
     cf8:	[0-9a-f]* 	{ add r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
     d00:	[0-9a-f]* 	{ add r5, r6, r7 ; wh64 r15 }
     d08:	[0-9a-f]* 	{ addb r15, r16, r17 ; addli r5, r6, 4660 }
     d10:	[0-9a-f]* 	{ addb r15, r16, r17 ; inthb r5, r6, r7 }
     d18:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; addb r15, r16, r17 }
     d20:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; addb r15, r16, r17 }
     d28:	[0-9a-f]* 	{ addb r15, r16, r17 ; s2a r5, r6, r7 }
     d30:	[0-9a-f]* 	{ addb r15, r16, r17 ; shr r5, r6, r7 }
     d38:	[0-9a-f]* 	{ addb r15, r16, r17 ; sltib r5, r6, 5 }
     d40:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addb r15, r16, r17 }
     d48:	[0-9a-f]* 	{ addb r5, r6, r7 ; finv r15 }
     d50:	[0-9a-f]* 	{ addb r5, r6, r7 ; lbadd_u r15, r16, 5 }
     d58:	[0-9a-f]* 	{ addb r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
     d60:	[0-9a-f]* 	{ addb r5, r6, r7 ; prefetch r15 }
     d68:	[0-9a-f]* 	{ addb r5, r6, r7 ; shli r15, r16, 5 }
     d70:	[0-9a-f]* 	{ addb r5, r6, r7 ; slth_u r15, r16, r17 }
     d78:	[0-9a-f]* 	{ addb r5, r6, r7 ; subhs r15, r16, r17 }
     d80:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; addbs_u r15, r16, r17 }
     d88:	[0-9a-f]* 	{ addbs_u r15, r16, r17 ; maxb_u r5, r6, r7 }
     d90:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; addbs_u r15, r16, r17 }
     d98:	[0-9a-f]* 	{ mvz r5, r6, r7 ; addbs_u r15, r16, r17 }
     da0:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; addbs_u r15, r16, r17 }
     da8:	[0-9a-f]* 	{ addbs_u r15, r16, r17 ; shrib r5, r6, 5 }
     db0:	[0-9a-f]* 	{ addbs_u r15, r16, r17 ; sne r5, r6, r7 }
     db8:	[0-9a-f]* 	{ addbs_u r15, r16, r17 ; xori r5, r6, 5 }
     dc0:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; ill }
     dc8:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; lhadd_u r15, r16, 5 }
     dd0:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; move r15, r16 }
     dd8:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; s1a r15, r16, r17 }
     de0:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; shrb r15, r16, r17 }
     de8:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; sltib_u r15, r16, 5 }
     df0:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; tns r15, r16 }
     df8:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; addh r15, r16, r17 }
     e00:	[0-9a-f]* 	{ addh r15, r16, r17 ; minb_u r5, r6, r7 }
     e08:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; addh r15, r16, r17 }
     e10:	[0-9a-f]* 	{ addh r15, r16, r17 ; nop }
     e18:	[0-9a-f]* 	{ addh r15, r16, r17 ; seq r5, r6, r7 }
     e20:	[0-9a-f]* 	{ addh r15, r16, r17 ; sltb r5, r6, r7 }
     e28:	[0-9a-f]* 	{ addh r15, r16, r17 ; srab r5, r6, r7 }
     e30:	[0-9a-f]* 	{ addh r5, r6, r7 ; addh r15, r16, r17 }
     e38:	[0-9a-f]* 	{ addh r5, r6, r7 ; inthh r15, r16, r17 }
     e40:	[0-9a-f]* 	{ addh r5, r6, r7 ; lwadd r15, r16, 5 }
     e48:	[0-9a-f]* 	{ addh r5, r6, r7 ; mtspr 5, r16 }
     e50:	[0-9a-f]* 	{ addh r5, r6, r7 ; sbadd r15, r16, 5 }
     e58:	[0-9a-f]* 	{ addh r5, r6, r7 ; shrih r15, r16, 5 }
     e60:	[0-9a-f]* 	{ addh r5, r6, r7 ; sneb r15, r16, r17 }
     e68:	[0-9a-f]* 	{ addhs r15, r16, r17 ; add r5, r6, r7 }
     e70:	[0-9a-f]* 	{ clz r5, r6 ; addhs r15, r16, r17 }
     e78:	[0-9a-f]* 	{ addhs r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
     e80:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; addhs r15, r16, r17 }
     e88:	[0-9a-f]* 	{ addhs r15, r16, r17 ; packbs_u r5, r6, r7 }
     e90:	[0-9a-f]* 	{ addhs r15, r16, r17 ; seqib r5, r6, 5 }
     e98:	[0-9a-f]* 	{ addhs r15, r16, r17 ; slteb r5, r6, r7 }
     ea0:	[0-9a-f]* 	{ addhs r15, r16, r17 ; sraih r5, r6, 5 }
     ea8:	[0-9a-f]* 	{ addhs r5, r6, r7 ; addih r15, r16, 5 }
     eb0:	[0-9a-f]* 	{ addhs r5, r6, r7 ; iret }
     eb8:	[0-9a-f]* 	{ addhs r5, r6, r7 ; maxib_u r15, r16, 5 }
     ec0:	[0-9a-f]* 	{ addhs r5, r6, r7 ; nop }
     ec8:	[0-9a-f]* 	{ addhs r5, r6, r7 ; seqi r15, r16, 5 }
     ed0:	[0-9a-f]* 	{ addhs r5, r6, r7 ; sltb_u r15, r16, r17 }
     ed8:	[0-9a-f]* 	{ addhs r5, r6, r7 ; srah r15, r16, r17 }
     ee0:	[0-9a-f]* 	{ addi r15, r16, 5 ; add r5, r6, r7 ; lw r25, r26 }
     ee8:	[0-9a-f]* 	{ addi r15, r16, 5 ; addib r5, r6, 5 }
     ef0:	[0-9a-f]* 	{ addi r15, r16, 5 ; andi r5, r6, 5 ; lh_u r25, r26 }
     ef8:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lb r25, r26 }
     f00:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; addi r15, r16, 5 }
     f08:	[0-9a-f]* 	{ addi r15, r16, 5 ; sh r25, r26 }
     f10:	[0-9a-f]* 	{ addi r15, r16, 5 ; and r5, r6, r7 ; lb r25, r26 }
     f18:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
     f20:	[0-9a-f]* 	{ addi r15, r16, 5 ; slt_u r5, r6, r7 ; lb r25, r26 }
     f28:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
     f30:	[0-9a-f]* 	{ addi r15, r16, 5 ; nop ; lb_u r25, r26 }
     f38:	[0-9a-f]* 	{ addi r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
     f40:	[0-9a-f]* 	{ addi r15, r16, 5 ; lh r25, r26 }
     f48:	[0-9a-f]* 	{ addi r15, r16, 5 ; ori r5, r6, 5 ; lh r25, r26 }
     f50:	[0-9a-f]* 	{ addi r15, r16, 5 ; sra r5, r6, r7 ; lh r25, r26 }
     f58:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; lh_u r25, r26 }
     f60:	[0-9a-f]* 	{ addi r15, r16, 5 ; rli r5, r6, 5 ; lh_u r25, r26 }
     f68:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
     f70:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
     f78:	[0-9a-f]* 	{ addi r15, r16, 5 ; s3a r5, r6, r7 ; lw r25, r26 }
     f80:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addi r15, r16, 5 ; lw r25, r26 }
     f88:	[0-9a-f]* 	{ addi r15, r16, 5 ; mnz r5, r6, r7 ; sw r25, r26 }
     f90:	[0-9a-f]* 	{ addi r15, r16, 5 ; movei r5, 5 ; sb r25, r26 }
     f98:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; addi r15, r16, 5 ; lh_u r25, r26 }
     fa0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; addi r15, r16, 5 ; lh r25, r26 }
     fa8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; addi r15, r16, 5 ; lh_u r25, r26 }
     fb0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; addi r15, r16, 5 ; lh r25, r26 }
     fb8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; addi r15, r16, 5 ; lb_u r25, r26 }
     fc0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
     fc8:	[0-9a-f]* 	{ addi r15, r16, 5 ; mzb r5, r6, r7 }
     fd0:	[0-9a-f]* 	{ addi r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
     fd8:	[0-9a-f]* 	{ addi r15, r16, 5 ; ori r5, r6, 5 ; sw r25, r26 }
     fe0:	[0-9a-f]* 	{ bitx r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
     fe8:	[0-9a-f]* 	{ addi r15, r16, 5 ; mz r5, r6, r7 ; prefetch r25 }
     ff0:	[0-9a-f]* 	{ addi r15, r16, 5 ; slte_u r5, r6, r7 ; prefetch r25 }
     ff8:	[0-9a-f]* 	{ addi r15, r16, 5 ; rl r5, r6, r7 ; sh r25, r26 }
    1000:	[0-9a-f]* 	{ addi r15, r16, 5 ; s1a r5, r6, r7 ; sh r25, r26 }
    1008:	[0-9a-f]* 	{ addi r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
    1010:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; sb r25, r26 }
    1018:	[0-9a-f]* 	{ addi r15, r16, 5 ; rli r5, r6, 5 ; sb r25, r26 }
    1020:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addi r15, r16, 5 ; sb r25, r26 }
    1028:	[0-9a-f]* 	{ addi r15, r16, 5 ; seqi r5, r6, 5 ; lh r25, r26 }
    1030:	[0-9a-f]* 	{ addi r15, r16, 5 ; mnz r5, r6, r7 ; sh r25, r26 }
    1038:	[0-9a-f]* 	{ addi r15, r16, 5 ; rl r5, r6, r7 ; sh r25, r26 }
    1040:	[0-9a-f]* 	{ addi r15, r16, 5 ; sub r5, r6, r7 ; sh r25, r26 }
    1048:	[0-9a-f]* 	{ addi r15, r16, 5 ; shli r5, r6, 5 ; lb_u r25, r26 }
    1050:	[0-9a-f]* 	{ addi r15, r16, 5 ; shr r5, r6, r7 }
    1058:	[0-9a-f]* 	{ addi r15, r16, 5 ; slt r5, r6, r7 ; prefetch r25 }
    1060:	[0-9a-f]* 	{ addi r15, r16, 5 ; slte r5, r6, r7 ; lh_u r25, r26 }
    1068:	[0-9a-f]* 	{ addi r15, r16, 5 ; slteh_u r5, r6, r7 }
    1070:	[0-9a-f]* 	{ addi r15, r16, 5 ; slti_u r5, r6, 5 ; sh r25, r26 }
    1078:	[0-9a-f]* 	{ addi r15, r16, 5 ; sra r5, r6, r7 ; lb_u r25, r26 }
    1080:	[0-9a-f]* 	{ addi r15, r16, 5 ; srai r5, r6, 5 }
    1088:	[0-9a-f]* 	{ addi r15, r16, 5 ; and r5, r6, r7 ; sw r25, r26 }
    1090:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
    1098:	[0-9a-f]* 	{ addi r15, r16, 5 ; slt_u r5, r6, r7 ; sw r25, r26 }
    10a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    10a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    10b0:	[0-9a-f]* 	{ addi r15, r16, 5 ; xor r5, r6, r7 ; prefetch r25 }
    10b8:	[0-9a-f]* 	{ addi r5, r6, 5 ; addi r15, r16, 5 ; lb r25, r26 }
    10c0:	[0-9a-f]* 	{ addi r5, r6, 5 ; and r15, r16, r17 ; prefetch r25 }
    10c8:	[0-9a-f]* 	{ addi r5, r6, 5 ; lb_u r25, r26 }
    10d0:	[0-9a-f]* 	{ addi r5, r6, 5 ; info 19 ; lb r25, r26 }
    10d8:	[0-9a-f]* 	{ addi r5, r6, 5 ; jrp r15 }
    10e0:	[0-9a-f]* 	{ addi r5, r6, 5 ; s2a r15, r16, r17 ; lb r25, r26 }
    10e8:	[0-9a-f]* 	{ addi r5, r6, 5 ; lb_u r15, r16 }
    10f0:	[0-9a-f]* 	{ addi r5, r6, 5 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    10f8:	[0-9a-f]* 	{ addi r5, r6, 5 ; lbadd_u r15, r16, 5 }
    1100:	[0-9a-f]* 	{ addi r5, r6, 5 ; s2a r15, r16, r17 ; lh r25, r26 }
    1108:	[0-9a-f]* 	{ addi r5, r6, 5 ; lh_u r15, r16 }
    1110:	[0-9a-f]* 	{ addi r5, r6, 5 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    1118:	[0-9a-f]* 	{ addi r5, r6, 5 ; lhadd_u r15, r16, 5 }
    1120:	[0-9a-f]* 	{ addi r5, r6, 5 ; s1a r15, r16, r17 ; lw r25, r26 }
    1128:	[0-9a-f]* 	{ addi r5, r6, 5 ; lw r25, r26 }
    1130:	[0-9a-f]* 	{ addi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch r25 }
    1138:	[0-9a-f]* 	{ addi r5, r6, 5 ; movei r15, 5 ; lh_u r25, r26 }
    1140:	[0-9a-f]* 	{ addi r5, r6, 5 ; mzb r15, r16, r17 }
    1148:	[0-9a-f]* 	{ addi r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    1150:	[0-9a-f]* 	{ addi r5, r6, 5 ; ori r15, r16, 5 ; sw r25, r26 }
    1158:	[0-9a-f]* 	{ addi r5, r6, 5 ; or r15, r16, r17 ; prefetch r25 }
    1160:	[0-9a-f]* 	{ addi r5, r6, 5 ; sra r15, r16, r17 ; prefetch r25 }
    1168:	[0-9a-f]* 	{ addi r5, r6, 5 ; rli r15, r16, 5 ; lw r25, r26 }
    1170:	[0-9a-f]* 	{ addi r5, r6, 5 ; s2a r15, r16, r17 ; lw r25, r26 }
    1178:	[0-9a-f]* 	{ addi r5, r6, 5 ; andi r15, r16, 5 ; sb r25, r26 }
    1180:	[0-9a-f]* 	{ addi r5, r6, 5 ; shli r15, r16, 5 ; sb r25, r26 }
    1188:	[0-9a-f]* 	{ addi r5, r6, 5 ; seq r15, r16, r17 ; lw r25, r26 }
    1190:	[0-9a-f]* 	{ addi r5, r6, 5 ; sh r15, r16 }
    1198:	[0-9a-f]* 	{ addi r5, r6, 5 ; s3a r15, r16, r17 ; sh r25, r26 }
    11a0:	[0-9a-f]* 	{ addi r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    11a8:	[0-9a-f]* 	{ addi r5, r6, 5 ; shli r15, r16, 5 ; sw r25, r26 }
    11b0:	[0-9a-f]* 	{ addi r5, r6, 5 ; shri r15, r16, 5 ; lw r25, r26 }
    11b8:	[0-9a-f]* 	{ addi r5, r6, 5 ; slt_u r15, r16, r17 ; lh r25, r26 }
    11c0:	[0-9a-f]* 	{ addi r5, r6, 5 ; slte_u r15, r16, r17 ; lb r25, r26 }
    11c8:	[0-9a-f]* 	{ addi r5, r6, 5 ; slti r15, r16, 5 ; lw r25, r26 }
    11d0:	[0-9a-f]* 	{ addi r5, r6, 5 ; sne r15, r16, r17 ; lb r25, r26 }
    11d8:	[0-9a-f]* 	{ addi r5, r6, 5 ; sra r15, r16, r17 ; sw r25, r26 }
    11e0:	[0-9a-f]* 	{ addi r5, r6, 5 ; sub r15, r16, r17 ; lw r25, r26 }
    11e8:	[0-9a-f]* 	{ addi r5, r6, 5 ; move r15, r16 ; sw r25, r26 }
    11f0:	[0-9a-f]* 	{ addi r5, r6, 5 ; slte r15, r16, r17 ; sw r25, r26 }
    11f8:	[0-9a-f]* 	{ addi r5, r6, 5 ; xor r15, r16, r17 ; sh r25, r26 }
    1200:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; addib r15, r16, 5 }
    1208:	[0-9a-f]* 	{ addib r15, r16, 5 ; minb_u r5, r6, r7 }
    1210:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; addib r15, r16, 5 }
    1218:	[0-9a-f]* 	{ addib r15, r16, 5 ; nop }
    1220:	[0-9a-f]* 	{ addib r15, r16, 5 ; seq r5, r6, r7 }
    1228:	[0-9a-f]* 	{ addib r15, r16, 5 ; sltb r5, r6, r7 }
    1230:	[0-9a-f]* 	{ addib r15, r16, 5 ; srab r5, r6, r7 }
    1238:	[0-9a-f]* 	{ addib r5, r6, 5 ; addh r15, r16, r17 }
    1240:	[0-9a-f]* 	{ addib r5, r6, 5 ; inthh r15, r16, r17 }
    1248:	[0-9a-f]* 	{ addib r5, r6, 5 ; lwadd r15, r16, 5 }
    1250:	[0-9a-f]* 	{ addib r5, r6, 5 ; mtspr 5, r16 }
    1258:	[0-9a-f]* 	{ addib r5, r6, 5 ; sbadd r15, r16, 5 }
    1260:	[0-9a-f]* 	{ addib r5, r6, 5 ; shrih r15, r16, 5 }
    1268:	[0-9a-f]* 	{ addib r5, r6, 5 ; sneb r15, r16, r17 }
    1270:	[0-9a-f]* 	{ addih r15, r16, 5 ; add r5, r6, r7 }
    1278:	[0-9a-f]* 	{ clz r5, r6 ; addih r15, r16, 5 }
    1280:	[0-9a-f]* 	{ addih r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
    1288:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; addih r15, r16, 5 }
    1290:	[0-9a-f]* 	{ addih r15, r16, 5 ; packbs_u r5, r6, r7 }
    1298:	[0-9a-f]* 	{ addih r15, r16, 5 ; seqib r5, r6, 5 }
    12a0:	[0-9a-f]* 	{ addih r15, r16, 5 ; slteb r5, r6, r7 }
    12a8:	[0-9a-f]* 	{ addih r15, r16, 5 ; sraih r5, r6, 5 }
    12b0:	[0-9a-f]* 	{ addih r5, r6, 5 ; addih r15, r16, 5 }
    12b8:	[0-9a-f]* 	{ addih r5, r6, 5 ; iret }
    12c0:	[0-9a-f]* 	{ addih r5, r6, 5 ; maxib_u r15, r16, 5 }
    12c8:	[0-9a-f]* 	{ addih r5, r6, 5 ; nop }
    12d0:	[0-9a-f]* 	{ addih r5, r6, 5 ; seqi r15, r16, 5 }
    12d8:	[0-9a-f]* 	{ addih r5, r6, 5 ; sltb_u r15, r16, r17 }
    12e0:	[0-9a-f]* 	{ addih r5, r6, 5 ; srah r15, r16, r17 }
    12e8:	[0-9a-f]* 	{ addli r15, r16, 4660 ; addhs r5, r6, r7 }
    12f0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; addli r15, r16, 4660 }
    12f8:	[0-9a-f]* 	{ addli r15, r16, 4660 ; move r5, r6 }
    1300:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; addli r15, r16, 4660 }
    1308:	[0-9a-f]* 	{ pcnt r5, r6 ; addli r15, r16, 4660 }
    1310:	[0-9a-f]* 	{ addli r15, r16, 4660 ; shlh r5, r6, r7 }
    1318:	[0-9a-f]* 	{ addli r15, r16, 4660 ; slth r5, r6, r7 }
    1320:	[0-9a-f]* 	{ addli r15, r16, 4660 ; subh r5, r6, r7 }
    1328:	[0-9a-f]* 	{ addli r5, r6, 4660 ; and r15, r16, r17 }
    1330:	[0-9a-f]* 	{ addli r5, r6, 4660 ; jrp r15 }
    1338:	[0-9a-f]* 	{ addli r5, r6, 4660 ; minb_u r15, r16, r17 }
    1340:	[0-9a-f]* 	{ addli r5, r6, 4660 ; packbs_u r15, r16, r17 }
    1348:	[0-9a-f]* 	{ addli r5, r6, 4660 ; shadd r15, r16, 5 }
    1350:	[0-9a-f]* 	{ addli r5, r6, 4660 ; slteb_u r15, r16, r17 }
    1358:	[0-9a-f]* 	{ addli r5, r6, 4660 ; sub r15, r16, r17 }
    1360:	[0-9a-f]* 	{ addli.sn r15, r16, 4660 ; addli r5, r6, 4660 }
    1368:	[0-9a-f]* 	{ addli.sn r15, r16, 4660 ; inthh r5, r6, r7 }
    1370:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; addli.sn r15, r16, 4660 }
    1378:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; addli.sn r15, r16, 4660 }
    1380:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; addli.sn r15, r16, 4660 }
    1388:	[0-9a-f]* 	{ addli.sn r15, r16, 4660 ; shrh r5, r6, r7 }
    1390:	[0-9a-f]* 	{ addli.sn r15, r16, 4660 ; sltih r5, r6, 5 }
    1398:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addli.sn r15, r16, 4660 }
    13a0:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; icoh r15 }
    13a8:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; lhadd r15, r16, 5 }
    13b0:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; mnzh r15, r16, r17 }
    13b8:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; s1a r15, r16, r17 }
    13c0:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; shrb r15, r16, r17 }
    13c8:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; sltib_u r15, r16, 5 }
    13d0:	[0-9a-f]* 	{ addli.sn r5, r6, 4660 ; tns r15, r16 }
    13d8:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; adds r15, r16, r17 }
    13e0:	[0-9a-f]* 	{ adds r15, r16, r17 ; minb_u r5, r6, r7 }
    13e8:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; adds r15, r16, r17 }
    13f0:	[0-9a-f]* 	{ adds r15, r16, r17 ; nop }
    13f8:	[0-9a-f]* 	{ adds r15, r16, r17 ; seq r5, r6, r7 }
    1400:	[0-9a-f]* 	{ adds r15, r16, r17 ; sltb r5, r6, r7 }
    1408:	[0-9a-f]* 	{ adds r15, r16, r17 ; srab r5, r6, r7 }
    1410:	[0-9a-f]* 	{ adds r5, r6, r7 ; addh r15, r16, r17 }
    1418:	[0-9a-f]* 	{ adds r5, r6, r7 ; inthh r15, r16, r17 }
    1420:	[0-9a-f]* 	{ adds r5, r6, r7 ; lwadd r15, r16, 5 }
    1428:	[0-9a-f]* 	{ adds r5, r6, r7 ; mtspr 5, r16 }
    1430:	[0-9a-f]* 	{ adds r5, r6, r7 ; sbadd r15, r16, 5 }
    1438:	[0-9a-f]* 	{ adds r5, r6, r7 ; shrih r15, r16, 5 }
    1440:	[0-9a-f]* 	{ adds r5, r6, r7 ; sneb r15, r16, r17 }
    1448:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; add r15, r16, r17 }
    1450:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; info 19 }
    1458:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; lnk r15 }
    1460:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; movei r15, 5 }
    1468:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; s2a r15, r16, r17 }
    1470:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; shrh r15, r16, r17 }
    1478:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; sltih r15, r16, 5 }
    1480:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; wh64 r15 }
    1488:	[0-9a-f]* 	{ adiffh r5, r6, r7 }
    1490:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; lh_u r15, r16 }
    1498:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; mnzb r15, r16, r17 }
    14a0:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; rl r15, r16, r17 }
    14a8:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; shlih r15, r16, 5 }
    14b0:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; slti_u r15, r16, 5 }
    14b8:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; sw r15, r16 }
    14c0:	[0-9a-f]* 	{ and r15, r16, r17 ; addi r5, r6, 5 ; lb r25, r26 }
    14c8:	[0-9a-f]* 	{ and r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
    14d0:	[0-9a-f]* 	{ bitx r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    14d8:	[0-9a-f]* 	{ clz r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    14e0:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
    14e8:	[0-9a-f]* 	{ and r15, r16, r17 ; info 19 ; sh r25, r26 }
    14f0:	[0-9a-f]* 	{ and r15, r16, r17 ; movei r5, 5 ; lb r25, r26 }
    14f8:	[0-9a-f]* 	{ and r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
    1500:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    1508:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    1510:	[0-9a-f]* 	{ and r15, r16, r17 ; seq r5, r6, r7 ; lb_u r25, r26 }
    1518:	[0-9a-f]* 	{ and r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
    1520:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    1528:	[0-9a-f]* 	{ and r15, r16, r17 ; shli r5, r6, 5 ; lh r25, r26 }
    1530:	[0-9a-f]* 	{ and r15, r16, r17 ; addi r5, r6, 5 ; lh_u r25, r26 }
    1538:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    1540:	[0-9a-f]* 	{ and r15, r16, r17 ; slt r5, r6, r7 ; lh_u r25, r26 }
    1548:	[0-9a-f]* 	{ bitx r5, r6 ; and r15, r16, r17 ; lw r25, r26 }
    1550:	[0-9a-f]* 	{ and r15, r16, r17 ; mz r5, r6, r7 ; lw r25, r26 }
    1558:	[0-9a-f]* 	{ and r15, r16, r17 ; slte_u r5, r6, r7 ; lw r25, r26 }
    1560:	[0-9a-f]* 	{ and r15, r16, r17 ; minih r5, r6, 5 }
    1568:	[0-9a-f]* 	{ and r15, r16, r17 ; move r5, r6 ; sb r25, r26 }
    1570:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; and r15, r16, r17 ; lw r25, r26 }
    1578:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    1580:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; and r15, r16, r17 }
    1588:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    1590:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    1598:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    15a0:	[0-9a-f]* 	{ and r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    15a8:	[0-9a-f]* 	{ and r15, r16, r17 ; nop ; sw r25, r26 }
    15b0:	[0-9a-f]* 	{ and r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
    15b8:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; lw r25, r26 }
    15c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    15c8:	[0-9a-f]* 	{ and r15, r16, r17 ; s3a r5, r6, r7 ; prefetch r25 }
    15d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
    15d8:	[0-9a-f]* 	{ and r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
    15e0:	[0-9a-f]* 	{ and r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    15e8:	[0-9a-f]* 	{ and r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
    15f0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    15f8:	[0-9a-f]* 	{ and r15, r16, r17 ; slt r5, r6, r7 ; sb r25, r26 }
    1600:	[0-9a-f]* 	{ and r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
    1608:	[0-9a-f]* 	{ and r15, r16, r17 ; add r5, r6, r7 ; sh r25, r26 }
    1610:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; and r15, r16, r17 ; sh r25, r26 }
    1618:	[0-9a-f]* 	{ and r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
    1620:	[0-9a-f]* 	{ and r15, r16, r17 ; shl r5, r6, r7 ; lh_u r25, r26 }
    1628:	[0-9a-f]* 	{ and r15, r16, r17 ; shlih r5, r6, 5 }
    1630:	[0-9a-f]* 	{ and r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
    1638:	[0-9a-f]* 	{ and r15, r16, r17 ; slt_u r5, r6, r7 ; prefetch r25 }
    1640:	[0-9a-f]* 	{ and r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    1648:	[0-9a-f]* 	{ and r15, r16, r17 ; slti r5, r6, 5 ; sh r25, r26 }
    1650:	[0-9a-f]* 	{ and r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
    1658:	[0-9a-f]* 	{ and r15, r16, r17 ; srah r5, r6, r7 }
    1660:	[0-9a-f]* 	{ and r15, r16, r17 ; sub r5, r6, r7 ; sh r25, r26 }
    1668:	[0-9a-f]* 	{ and r15, r16, r17 ; movei r5, 5 ; sw r25, r26 }
    1670:	[0-9a-f]* 	{ and r15, r16, r17 ; s1a r5, r6, r7 ; sw r25, r26 }
    1678:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
    1680:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
    1688:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
    1690:	[0-9a-f]* 	{ and r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    1698:	[0-9a-f]* 	{ and r5, r6, r7 ; addib r15, r16, 5 }
    16a0:	[0-9a-f]* 	{ and r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    16a8:	[0-9a-f]* 	{ and r5, r6, r7 ; ill ; lb r25, r26 }
    16b0:	[0-9a-f]* 	{ and r5, r6, r7 ; infol 4660 }
    16b8:	[0-9a-f]* 	{ and r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    16c0:	[0-9a-f]* 	{ and r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    16c8:	[0-9a-f]* 	{ and r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    16d0:	[0-9a-f]* 	{ and r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    16d8:	[0-9a-f]* 	{ and r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    16e0:	[0-9a-f]* 	{ and r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    16e8:	[0-9a-f]* 	{ and r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    16f0:	[0-9a-f]* 	{ and r5, r6, r7 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    16f8:	[0-9a-f]* 	{ and r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    1700:	[0-9a-f]* 	{ and r5, r6, r7 ; slt_u r15, r16, r17 ; lw r25, r26 }
    1708:	[0-9a-f]* 	{ and r5, r6, r7 ; minb_u r15, r16, r17 }
    1710:	[0-9a-f]* 	{ and r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
    1718:	[0-9a-f]* 	{ and r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    1720:	[0-9a-f]* 	{ and r5, r6, r7 ; nop ; sw r25, r26 }
    1728:	[0-9a-f]* 	{ and r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
    1730:	[0-9a-f]* 	{ and r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    1738:	[0-9a-f]* 	{ and r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    1740:	[0-9a-f]* 	{ and r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    1748:	[0-9a-f]* 	{ and r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    1750:	[0-9a-f]* 	{ and r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
    1758:	[0-9a-f]* 	{ and r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    1760:	[0-9a-f]* 	{ and r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    1768:	[0-9a-f]* 	{ and r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
    1770:	[0-9a-f]* 	{ and r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    1778:	[0-9a-f]* 	{ and r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    1780:	[0-9a-f]* 	{ and r5, r6, r7 ; shlb r15, r16, r17 }
    1788:	[0-9a-f]* 	{ and r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    1790:	[0-9a-f]* 	{ and r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
    1798:	[0-9a-f]* 	{ and r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    17a0:	[0-9a-f]* 	{ and r5, r6, r7 ; slteb r15, r16, r17 }
    17a8:	[0-9a-f]* 	{ and r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
    17b0:	[0-9a-f]* 	{ and r5, r6, r7 ; sneb r15, r16, r17 }
    17b8:	[0-9a-f]* 	{ and r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
    17c0:	[0-9a-f]* 	{ and r5, r6, r7 ; subs r15, r16, r17 }
    17c8:	[0-9a-f]* 	{ and r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    17d0:	[0-9a-f]* 	{ and r5, r6, r7 ; swadd r15, r16, 5 }
    17d8:	[0-9a-f]* 	{ andi r15, r16, 5 ; add r5, r6, r7 ; sb r25, r26 }
    17e0:	[0-9a-f]* 	{ andi r15, r16, 5 ; addli r5, r6, 4660 }
    17e8:	[0-9a-f]* 	{ andi r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    17f0:	[0-9a-f]* 	{ bytex r5, r6 ; andi r15, r16, 5 ; lh r25, r26 }
    17f8:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; lb r25, r26 }
    1800:	[0-9a-f]* 	{ andi r15, r16, 5 }
    1808:	[0-9a-f]* 	{ bitx r5, r6 ; andi r15, r16, 5 ; lb r25, r26 }
    1810:	[0-9a-f]* 	{ andi r15, r16, 5 ; mz r5, r6, r7 ; lb r25, r26 }
    1818:	[0-9a-f]* 	{ andi r15, r16, 5 ; slte_u r5, r6, r7 ; lb r25, r26 }
    1820:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; lb_u r25, r26 }
    1828:	[0-9a-f]* 	{ andi r15, r16, 5 ; or r5, r6, r7 ; lb_u r25, r26 }
    1830:	[0-9a-f]* 	{ andi r15, r16, 5 ; sne r5, r6, r7 ; lb_u r25, r26 }
    1838:	[0-9a-f]* 	{ andi r15, r16, 5 ; mnz r5, r6, r7 ; lh r25, r26 }
    1840:	[0-9a-f]* 	{ andi r15, r16, 5 ; rl r5, r6, r7 ; lh r25, r26 }
    1848:	[0-9a-f]* 	{ andi r15, r16, 5 ; sub r5, r6, r7 ; lh r25, r26 }
    1850:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    1858:	[0-9a-f]* 	{ andi r15, r16, 5 ; s2a r5, r6, r7 ; lh_u r25, r26 }
    1860:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; lh_u r25, r26 }
    1868:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; andi r15, r16, 5 ; lw r25, r26 }
    1870:	[0-9a-f]* 	{ andi r15, r16, 5 ; seqi r5, r6, 5 ; lw r25, r26 }
    1878:	[0-9a-f]* 	{ andi r15, r16, 5 ; lw r25, r26 }
    1880:	[0-9a-f]* 	{ andi r15, r16, 5 ; mnzb r5, r6, r7 }
    1888:	[0-9a-f]* 	{ andi r15, r16, 5 ; movei r5, 5 ; sw r25, r26 }
    1890:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    1898:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; andi r15, r16, 5 ; lw r25, r26 }
    18a0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    18a8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; andi r15, r16, 5 ; lw r25, r26 }
    18b0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    18b8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
    18c0:	[0-9a-f]* 	{ andi r15, r16, 5 ; nop ; lb r25, r26 }
    18c8:	[0-9a-f]* 	{ andi r15, r16, 5 ; or r5, r6, r7 ; lb r25, r26 }
    18d0:	[0-9a-f]* 	{ andi r15, r16, 5 ; packbs_u r5, r6, r7 }
    18d8:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    18e0:	[0-9a-f]* 	{ andi r15, r16, 5 ; nor r5, r6, r7 ; prefetch r25 }
    18e8:	[0-9a-f]* 	{ andi r15, r16, 5 ; slti_u r5, r6, 5 ; prefetch r25 }
    18f0:	[0-9a-f]* 	{ andi r15, r16, 5 ; rl r5, r6, r7 }
    18f8:	[0-9a-f]* 	{ andi r15, r16, 5 ; s1a r5, r6, r7 }
    1900:	[0-9a-f]* 	{ andi r15, r16, 5 ; s3a r5, r6, r7 }
    1908:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; andi r15, r16, 5 ; sb r25, r26 }
    1910:	[0-9a-f]* 	{ andi r15, r16, 5 ; s2a r5, r6, r7 ; sb r25, r26 }
    1918:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
    1920:	[0-9a-f]* 	{ andi r15, r16, 5 ; seqi r5, r6, 5 ; lw r25, r26 }
    1928:	[0-9a-f]* 	{ andi r15, r16, 5 ; movei r5, 5 ; sh r25, r26 }
    1930:	[0-9a-f]* 	{ andi r15, r16, 5 ; s1a r5, r6, r7 ; sh r25, r26 }
    1938:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; sh r25, r26 }
    1940:	[0-9a-f]* 	{ andi r15, r16, 5 ; shli r5, r6, 5 ; lh_u r25, r26 }
    1948:	[0-9a-f]* 	{ andi r15, r16, 5 ; shrh r5, r6, r7 }
    1950:	[0-9a-f]* 	{ andi r15, r16, 5 ; slt r5, r6, r7 ; sh r25, r26 }
    1958:	[0-9a-f]* 	{ andi r15, r16, 5 ; slte r5, r6, r7 ; prefetch r25 }
    1960:	[0-9a-f]* 	{ andi r15, r16, 5 ; slth_u r5, r6, r7 }
    1968:	[0-9a-f]* 	{ andi r15, r16, 5 ; slti_u r5, r6, 5 }
    1970:	[0-9a-f]* 	{ andi r15, r16, 5 ; sra r5, r6, r7 ; lh_u r25, r26 }
    1978:	[0-9a-f]* 	{ andi r15, r16, 5 ; sraih r5, r6, 5 }
    1980:	[0-9a-f]* 	{ bitx r5, r6 ; andi r15, r16, 5 ; sw r25, r26 }
    1988:	[0-9a-f]* 	{ andi r15, r16, 5 ; mz r5, r6, r7 ; sw r25, r26 }
    1990:	[0-9a-f]* 	{ andi r15, r16, 5 ; slte_u r5, r6, r7 ; sw r25, r26 }
    1998:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; sh r25, r26 }
    19a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; sh r25, r26 }
    19a8:	[0-9a-f]* 	{ andi r15, r16, 5 ; xor r5, r6, r7 ; sh r25, r26 }
    19b0:	[0-9a-f]* 	{ andi r5, r6, 5 ; addi r15, r16, 5 ; lh r25, r26 }
    19b8:	[0-9a-f]* 	{ andi r5, r6, 5 ; and r15, r16, r17 ; sh r25, r26 }
    19c0:	[0-9a-f]* 	{ andi r5, r6, 5 ; lh_u r25, r26 }
    19c8:	[0-9a-f]* 	{ andi r5, r6, 5 ; info 19 ; lh r25, r26 }
    19d0:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; lb r25, r26 }
    19d8:	[0-9a-f]* 	{ andi r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
    19e0:	[0-9a-f]* 	{ andi r5, r6, 5 ; addi r15, r16, 5 ; lb_u r25, r26 }
    19e8:	[0-9a-f]* 	{ andi r5, r6, 5 ; seqi r15, r16, 5 ; lb_u r25, r26 }
    19f0:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; lh r25, r26 }
    19f8:	[0-9a-f]* 	{ andi r5, r6, 5 ; seq r15, r16, r17 ; lh r25, r26 }
    1a00:	[0-9a-f]* 	{ andi r5, r6, 5 ; addi r15, r16, 5 ; lh_u r25, r26 }
    1a08:	[0-9a-f]* 	{ andi r5, r6, 5 ; seqi r15, r16, 5 ; lh_u r25, r26 }
    1a10:	[0-9a-f]* 	{ andi r5, r6, 5 ; lw r15, r16 }
    1a18:	[0-9a-f]* 	{ andi r5, r6, 5 ; s3a r15, r16, r17 ; lw r25, r26 }
    1a20:	[0-9a-f]* 	{ andi r5, r6, 5 ; lwadd r15, r16, 5 }
    1a28:	[0-9a-f]* 	{ andi r5, r6, 5 ; mnz r15, r16, r17 ; sh r25, r26 }
    1a30:	[0-9a-f]* 	{ andi r5, r6, 5 ; movei r15, 5 ; prefetch r25 }
    1a38:	[0-9a-f]* 	{ andi r5, r6, 5 ; nop ; lb r25, r26 }
    1a40:	[0-9a-f]* 	{ andi r5, r6, 5 ; or r15, r16, r17 ; lb r25, r26 }
    1a48:	[0-9a-f]* 	{ andi r5, r6, 5 ; packbs_u r15, r16, r17 }
    1a50:	[0-9a-f]* 	{ andi r5, r6, 5 ; rl r15, r16, r17 ; prefetch r25 }
    1a58:	[0-9a-f]* 	{ andi r5, r6, 5 ; sub r15, r16, r17 ; prefetch r25 }
    1a60:	[0-9a-f]* 	{ andi r5, r6, 5 ; rli r15, r16, 5 ; sb r25, r26 }
    1a68:	[0-9a-f]* 	{ andi r5, r6, 5 ; s2a r15, r16, r17 ; sb r25, r26 }
    1a70:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; sb r25, r26 }
    1a78:	[0-9a-f]* 	{ andi r5, r6, 5 ; shri r15, r16, 5 ; sb r25, r26 }
    1a80:	[0-9a-f]* 	{ andi r5, r6, 5 ; seq r15, r16, r17 ; sb r25, r26 }
    1a88:	[0-9a-f]* 	{ andi r5, r6, 5 ; addi r15, r16, 5 ; sh r25, r26 }
    1a90:	[0-9a-f]* 	{ andi r5, r6, 5 ; seqi r15, r16, 5 ; sh r25, r26 }
    1a98:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    1aa0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shlib r15, r16, 5 }
    1aa8:	[0-9a-f]* 	{ andi r5, r6, 5 ; shri r15, r16, 5 ; sb r25, r26 }
    1ab0:	[0-9a-f]* 	{ andi r5, r6, 5 ; slt_u r15, r16, r17 ; lw r25, r26 }
    1ab8:	[0-9a-f]* 	{ andi r5, r6, 5 ; slte_u r15, r16, r17 ; lh r25, r26 }
    1ac0:	[0-9a-f]* 	{ andi r5, r6, 5 ; slti r15, r16, 5 ; sb r25, r26 }
    1ac8:	[0-9a-f]* 	{ andi r5, r6, 5 ; sne r15, r16, r17 ; lh r25, r26 }
    1ad0:	[0-9a-f]* 	{ andi r5, r6, 5 ; srab r15, r16, r17 }
    1ad8:	[0-9a-f]* 	{ andi r5, r6, 5 ; sub r15, r16, r17 ; sb r25, r26 }
    1ae0:	[0-9a-f]* 	{ andi r5, r6, 5 ; mz r15, r16, r17 ; sw r25, r26 }
    1ae8:	[0-9a-f]* 	{ andi r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
    1af0:	[0-9a-f]* 	{ andi r5, r6, 5 ; xor r15, r16, r17 }
    1af8:	[0-9a-f]* 	{ bitx r5, r6 ; auli r15, r16, 4660 }
    1b00:	[0-9a-f]* 	{ auli r15, r16, 4660 ; minib_u r5, r6, 5 }
    1b08:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; auli r15, r16, 4660 }
    1b10:	[0-9a-f]* 	{ auli r15, r16, 4660 ; or r5, r6, r7 }
    1b18:	[0-9a-f]* 	{ auli r15, r16, 4660 ; seqh r5, r6, r7 }
    1b20:	[0-9a-f]* 	{ auli r15, r16, 4660 ; slte r5, r6, r7 }
    1b28:	[0-9a-f]* 	{ auli r15, r16, 4660 ; srai r5, r6, 5 }
    1b30:	[0-9a-f]* 	{ auli r5, r6, 4660 ; addi r15, r16, 5 }
    1b38:	[0-9a-f]* 	{ auli r5, r6, 4660 ; intlh r15, r16, r17 }
    1b40:	[0-9a-f]* 	{ auli r5, r6, 4660 ; maxb_u r15, r16, r17 }
    1b48:	[0-9a-f]* 	{ auli r5, r6, 4660 ; mzb r15, r16, r17 }
    1b50:	[0-9a-f]* 	{ auli r5, r6, 4660 ; seqb r15, r16, r17 }
    1b58:	[0-9a-f]* 	{ auli r5, r6, 4660 ; slt_u r15, r16, r17 }
    1b60:	[0-9a-f]* 	{ auli r5, r6, 4660 ; sra r15, r16, r17 }
    1b68:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; addbs_u r15, r16, r17 }
    1b70:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; inthb r15, r16, r17 }
    1b78:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; lw_na r15, r16 }
    1b80:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; moveli.sn r15, 4660 }
    1b88:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; sb r15, r16 }
    1b90:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; shrib r15, r16, 5 }
    1b98:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; sne r15, r16, r17 }
    1ba0:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; xori r15, r16, 5 }
    1ba8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; ill }
    1bb0:	[0-9a-f]* 	{ avgh r5, r6, r7 ; lhadd_u r15, r16, 5 }
    1bb8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; move r15, r16 }
    1bc0:	[0-9a-f]* 	{ avgh r5, r6, r7 ; s1a r15, r16, r17 }
    1bc8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; shrb r15, r16, r17 }
    1bd0:	[0-9a-f]* 	{ avgh r5, r6, r7 ; sltib_u r15, r16, 5 }
    1bd8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; tns r15, r16 }
    1be0:	[0-9a-f]* 	{ bitx r5, r6 ; addi r15, r16, 5 ; lh r25, r26 }
    1be8:	[0-9a-f]* 	{ bitx r5, r6 ; and r15, r16, r17 ; sh r25, r26 }
    1bf0:	[0-9a-f]* 	{ bitx r5, r6 ; lh_u r25, r26 }
    1bf8:	[0-9a-f]* 	{ bitx r5, r6 ; info 19 ; lh r25, r26 }
    1c00:	[0-9a-f]* 	{ bitx r5, r6 ; add r15, r16, r17 ; lb r25, r26 }
    1c08:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; lb r25, r26 }
    1c10:	[0-9a-f]* 	{ bitx r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
    1c18:	[0-9a-f]* 	{ bitx r5, r6 ; seqi r15, r16, 5 ; lb_u r25, r26 }
    1c20:	[0-9a-f]* 	{ bitx r5, r6 ; add r15, r16, r17 ; lh r25, r26 }
    1c28:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; lh r25, r26 }
    1c30:	[0-9a-f]* 	{ bitx r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
    1c38:	[0-9a-f]* 	{ bitx r5, r6 ; seqi r15, r16, 5 ; lh_u r25, r26 }
    1c40:	[0-9a-f]* 	{ bitx r5, r6 ; lw r15, r16 }
    1c48:	[0-9a-f]* 	{ bitx r5, r6 ; s3a r15, r16, r17 ; lw r25, r26 }
    1c50:	[0-9a-f]* 	{ bitx r5, r6 ; lwadd r15, r16, 5 }
    1c58:	[0-9a-f]* 	{ bitx r5, r6 ; mnz r15, r16, r17 ; sh r25, r26 }
    1c60:	[0-9a-f]* 	{ bitx r5, r6 ; movei r15, 5 ; prefetch r25 }
    1c68:	[0-9a-f]* 	{ bitx r5, r6 ; nop ; lb r25, r26 }
    1c70:	[0-9a-f]* 	{ bitx r5, r6 ; or r15, r16, r17 ; lb r25, r26 }
    1c78:	[0-9a-f]* 	{ bitx r5, r6 ; packbs_u r15, r16, r17 }
    1c80:	[0-9a-f]* 	{ bitx r5, r6 ; rl r15, r16, r17 ; prefetch r25 }
    1c88:	[0-9a-f]* 	{ bitx r5, r6 ; sub r15, r16, r17 ; prefetch r25 }
    1c90:	[0-9a-f]* 	{ bitx r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
    1c98:	[0-9a-f]* 	{ bitx r5, r6 ; s2a r15, r16, r17 ; sb r25, r26 }
    1ca0:	[0-9a-f]* 	{ bitx r5, r6 ; ill ; sb r25, r26 }
    1ca8:	[0-9a-f]* 	{ bitx r5, r6 ; shri r15, r16, 5 ; sb r25, r26 }
    1cb0:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; sb r25, r26 }
    1cb8:	[0-9a-f]* 	{ bitx r5, r6 ; addi r15, r16, 5 ; sh r25, r26 }
    1cc0:	[0-9a-f]* 	{ bitx r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
    1cc8:	[0-9a-f]* 	{ bitx r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    1cd0:	[0-9a-f]* 	{ bitx r5, r6 ; shlib r15, r16, 5 }
    1cd8:	[0-9a-f]* 	{ bitx r5, r6 ; shri r15, r16, 5 ; sb r25, r26 }
    1ce0:	[0-9a-f]* 	{ bitx r5, r6 ; slt_u r15, r16, r17 ; lw r25, r26 }
    1ce8:	[0-9a-f]* 	{ bitx r5, r6 ; slte_u r15, r16, r17 ; lh r25, r26 }
    1cf0:	[0-9a-f]* 	{ bitx r5, r6 ; slti r15, r16, 5 ; sb r25, r26 }
    1cf8:	[0-9a-f]* 	{ bitx r5, r6 ; sne r15, r16, r17 ; lh r25, r26 }
    1d00:	[0-9a-f]* 	{ bitx r5, r6 ; srab r15, r16, r17 }
    1d08:	[0-9a-f]* 	{ bitx r5, r6 ; sub r15, r16, r17 ; sb r25, r26 }
    1d10:	[0-9a-f]* 	{ bitx r5, r6 ; mz r15, r16, r17 ; sw r25, r26 }
    1d18:	[0-9a-f]* 	{ bitx r5, r6 ; slti r15, r16, 5 ; sw r25, r26 }
    1d20:	[0-9a-f]* 	{ bitx r5, r6 ; xor r15, r16, r17 }
    1d28:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
    1d30:	[0-9a-f]* 	{ bytex r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
    1d38:	[0-9a-f]* 	{ bytex r5, r6 ; lw r25, r26 }
    1d40:	[0-9a-f]* 	{ bytex r5, r6 ; info 19 ; lh_u r25, r26 }
    1d48:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lb r25, r26 }
    1d50:	[0-9a-f]* 	{ bytex r5, r6 ; seqi r15, r16, 5 ; lb r25, r26 }
    1d58:	[0-9a-f]* 	{ bytex r5, r6 ; and r15, r16, r17 ; lb_u r25, r26 }
    1d60:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; lb_u r25, r26 }
    1d68:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lh r25, r26 }
    1d70:	[0-9a-f]* 	{ bytex r5, r6 ; seqi r15, r16, 5 ; lh r25, r26 }
    1d78:	[0-9a-f]* 	{ bytex r5, r6 ; and r15, r16, r17 ; lh_u r25, r26 }
    1d80:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    1d88:	[0-9a-f]* 	{ bytex r5, r6 ; add r15, r16, r17 ; lw r25, r26 }
    1d90:	[0-9a-f]* 	{ bytex r5, r6 ; seq r15, r16, r17 ; lw r25, r26 }
    1d98:	[0-9a-f]* 	{ bytex r5, r6 ; lwadd_na r15, r16, 5 }
    1da0:	[0-9a-f]* 	{ bytex r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
    1da8:	[0-9a-f]* 	{ bytex r5, r6 ; movei r15, 5 ; sb r25, r26 }
    1db0:	[0-9a-f]* 	{ bytex r5, r6 ; nop ; lb_u r25, r26 }
    1db8:	[0-9a-f]* 	{ bytex r5, r6 ; or r15, r16, r17 ; lb_u r25, r26 }
    1dc0:	[0-9a-f]* 	{ bytex r5, r6 ; packhb r15, r16, r17 }
    1dc8:	[0-9a-f]* 	{ bytex r5, r6 ; rli r15, r16, 5 ; prefetch r25 }
    1dd0:	[0-9a-f]* 	{ bytex r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    1dd8:	[0-9a-f]* 	{ bytex r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
    1de0:	[0-9a-f]* 	{ bytex r5, r6 ; s2a r15, r16, r17 ; sh r25, r26 }
    1de8:	[0-9a-f]* 	{ bytex r5, r6 ; info 19 ; sb r25, r26 }
    1df0:	[0-9a-f]* 	{ bytex r5, r6 ; slt r15, r16, r17 ; sb r25, r26 }
    1df8:	[0-9a-f]* 	{ bytex r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
    1e00:	[0-9a-f]* 	{ bytex r5, r6 ; and r15, r16, r17 ; sh r25, r26 }
    1e08:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; sh r25, r26 }
    1e10:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    1e18:	[0-9a-f]* 	{ bytex r5, r6 ; shlih r15, r16, 5 }
    1e20:	[0-9a-f]* 	{ bytex r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
    1e28:	[0-9a-f]* 	{ bytex r5, r6 ; slt_u r15, r16, r17 ; prefetch r25 }
    1e30:	[0-9a-f]* 	{ bytex r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    1e38:	[0-9a-f]* 	{ bytex r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    1e40:	[0-9a-f]* 	{ bytex r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
    1e48:	[0-9a-f]* 	{ bytex r5, r6 ; srah r15, r16, r17 }
    1e50:	[0-9a-f]* 	{ bytex r5, r6 ; sub r15, r16, r17 ; sh r25, r26 }
    1e58:	[0-9a-f]* 	{ bytex r5, r6 ; nop ; sw r25, r26 }
    1e60:	[0-9a-f]* 	{ bytex r5, r6 ; slti_u r15, r16, 5 ; sw r25, r26 }
    1e68:	[0-9a-f]* 	{ bytex r5, r6 ; xori r15, r16, 5 }
    1e70:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    1e78:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; lb r25, r26 }
    1e80:	[0-9a-f]* 	{ clz r5, r6 ; sb r25, r26 }
    1e88:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; prefetch r25 }
    1e90:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; lb r25, r26 }
    1e98:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; lb r25, r26 }
    1ea0:	[0-9a-f]* 	{ clz r5, r6 ; lb_u r25, r26 }
    1ea8:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
    1eb0:	[0-9a-f]* 	{ clz r5, r6 ; andi r15, r16, 5 ; lh r25, r26 }
    1eb8:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; lh r25, r26 }
    1ec0:	[0-9a-f]* 	{ clz r5, r6 ; lh_u r25, r26 }
    1ec8:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
    1ed0:	[0-9a-f]* 	{ clz r5, r6 ; and r15, r16, r17 ; lw r25, r26 }
    1ed8:	[0-9a-f]* 	{ clz r5, r6 ; shl r15, r16, r17 ; lw r25, r26 }
    1ee0:	[0-9a-f]* 	{ clz r5, r6 ; maxh r15, r16, r17 }
    1ee8:	[0-9a-f]* 	{ clz r5, r6 ; mnzb r15, r16, r17 }
    1ef0:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 ; sw r25, r26 }
    1ef8:	[0-9a-f]* 	{ clz r5, r6 ; nop ; lh_u r25, r26 }
    1f00:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
    1f08:	[0-9a-f]* 	{ clz r5, r6 ; packlb r15, r16, r17 }
    1f10:	[0-9a-f]* 	{ clz r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
    1f18:	[0-9a-f]* 	{ clz r5, r6 ; raise }
    1f20:	[0-9a-f]* 	{ clz r5, r6 ; rli r15, r16, 5 }
    1f28:	[0-9a-f]* 	{ clz r5, r6 ; s2a r15, r16, r17 }
    1f30:	[0-9a-f]* 	{ clz r5, r6 ; move r15, r16 ; sb r25, r26 }
    1f38:	[0-9a-f]* 	{ clz r5, r6 ; slte r15, r16, r17 ; sb r25, r26 }
    1f40:	[0-9a-f]* 	{ clz r5, r6 ; seq r15, r16, r17 }
    1f48:	[0-9a-f]* 	{ clz r5, r6 ; sh r25, r26 }
    1f50:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; sh r25, r26 }
    1f58:	[0-9a-f]* 	{ clz r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
    1f60:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
    1f68:	[0-9a-f]* 	{ clz r5, r6 ; shri r15, r16, 5 }
    1f70:	[0-9a-f]* 	{ clz r5, r6 ; slt_u r15, r16, r17 ; sh r25, r26 }
    1f78:	[0-9a-f]* 	{ clz r5, r6 ; slte_u r15, r16, r17 ; prefetch r25 }
    1f80:	[0-9a-f]* 	{ clz r5, r6 ; slti r15, r16, 5 }
    1f88:	[0-9a-f]* 	{ clz r5, r6 ; sne r15, r16, r17 ; prefetch r25 }
    1f90:	[0-9a-f]* 	{ clz r5, r6 ; srai r15, r16, 5 ; lb_u r25, r26 }
    1f98:	[0-9a-f]* 	{ clz r5, r6 ; sub r15, r16, r17 }
    1fa0:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    1fa8:	[0-9a-f]* 	{ clz r5, r6 ; sra r15, r16, r17 ; sw r25, r26 }
    1fb0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; addb r15, r16, r17 }
    1fb8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; infol 4660 }
    1fc0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; lw r15, r16 }
    1fc8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; moveli r15, 4660 }
    1fd0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; s3a r15, r16, r17 }
    1fd8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; shri r15, r16, 5 }
    1fe0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; sltih_u r15, r16, 5 }
    1fe8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; xor r15, r16, r17 }
    1ff0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; icoh r15 }
    1ff8:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; lhadd r15, r16, 5 }
    2000:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; mnzh r15, r16, r17 }
    2008:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; rli r15, r16, 5 }
    2010:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; shr r15, r16, r17 }
    2018:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; sltib r15, r16, 5 }
    2020:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; swadd r15, r16, 5 }
    2028:	[0-9a-f]* 	{ ctz r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
    2030:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; sb r25, r26 }
    2038:	[0-9a-f]* 	{ ctz r5, r6 ; lh r25, r26 }
    2040:	[0-9a-f]* 	{ ctz r5, r6 ; info 19 ; lb_u r25, r26 }
    2048:	[0-9a-f]* 	{ ctz r5, r6 ; lb r15, r16 }
    2050:	[0-9a-f]* 	{ ctz r5, r6 ; s3a r15, r16, r17 ; lb r25, r26 }
    2058:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; lb_u r25, r26 }
    2060:	[0-9a-f]* 	{ ctz r5, r6 ; seq r15, r16, r17 ; lb_u r25, r26 }
    2068:	[0-9a-f]* 	{ ctz r5, r6 ; lh r15, r16 }
    2070:	[0-9a-f]* 	{ ctz r5, r6 ; s3a r15, r16, r17 ; lh r25, r26 }
    2078:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; lh_u r25, r26 }
    2080:	[0-9a-f]* 	{ ctz r5, r6 ; seq r15, r16, r17 ; lh_u r25, r26 }
    2088:	[0-9a-f]* 	{ ctz r5, r6 ; lnk r15 }
    2090:	[0-9a-f]* 	{ ctz r5, r6 ; s2a r15, r16, r17 ; lw r25, r26 }
    2098:	[0-9a-f]* 	{ ctz r5, r6 ; lw_na r15, r16 }
    20a0:	[0-9a-f]* 	{ ctz r5, r6 ; mnz r15, r16, r17 ; sb r25, r26 }
    20a8:	[0-9a-f]* 	{ ctz r5, r6 ; movei r15, 5 ; lw r25, r26 }
    20b0:	[0-9a-f]* 	{ ctz r5, r6 ; mzh r15, r16, r17 }
    20b8:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 }
    20c0:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 }
    20c8:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
    20d0:	[0-9a-f]* 	{ ctz r5, r6 ; srai r15, r16, 5 ; prefetch r25 }
    20d8:	[0-9a-f]* 	{ ctz r5, r6 ; rli r15, r16, 5 ; prefetch r25 }
    20e0:	[0-9a-f]* 	{ ctz r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
    20e8:	[0-9a-f]* 	{ ctz r5, r6 ; sb r25, r26 }
    20f0:	[0-9a-f]* 	{ ctz r5, r6 ; shr r15, r16, r17 ; sb r25, r26 }
    20f8:	[0-9a-f]* 	{ ctz r5, r6 ; seq r15, r16, r17 ; prefetch r25 }
    2100:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
    2108:	[0-9a-f]* 	{ ctz r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
    2110:	[0-9a-f]* 	{ ctz r5, r6 ; shl r15, r16, r17 ; lb_u r25, r26 }
    2118:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 }
    2120:	[0-9a-f]* 	{ ctz r5, r6 ; shri r15, r16, 5 ; prefetch r25 }
    2128:	[0-9a-f]* 	{ ctz r5, r6 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    2130:	[0-9a-f]* 	{ ctz r5, r6 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    2138:	[0-9a-f]* 	{ ctz r5, r6 ; slti r15, r16, 5 ; prefetch r25 }
    2140:	[0-9a-f]* 	{ ctz r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
    2148:	[0-9a-f]* 	{ ctz r5, r6 ; sra r15, r16, r17 }
    2150:	[0-9a-f]* 	{ ctz r5, r6 ; sub r15, r16, r17 ; prefetch r25 }
    2158:	[0-9a-f]* 	{ ctz r5, r6 ; movei r15, 5 ; sw r25, r26 }
    2160:	[0-9a-f]* 	{ ctz r5, r6 ; slte_u r15, r16, r17 ; sw r25, r26 }
    2168:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; sw r25, r26 }
    2170:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; dtlbpr r15 }
    2178:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; dtlbpr r15 }
    2180:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; dtlbpr r15 }
    2188:	[0-9a-f]* 	{ nop ; dtlbpr r15 }
    2190:	[0-9a-f]* 	{ seq r5, r6, r7 ; dtlbpr r15 }
    2198:	[0-9a-f]* 	{ sltb r5, r6, r7 ; dtlbpr r15 }
    21a0:	[0-9a-f]* 	{ srab r5, r6, r7 ; dtlbpr r15 }
    21a8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; addh r15, r16, r17 }
    21b0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; inthh r15, r16, r17 }
    21b8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; lwadd r15, r16, 5 }
    21c0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; mtspr 5, r16 }
    21c8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; sbadd r15, r16, 5 }
    21d0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; shrih r15, r16, 5 }
    21d8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; sneb r15, r16, r17 }
    21e0:	[0-9a-f]* 	{ add r5, r6, r7 ; finv r15 }
    21e8:	[0-9a-f]* 	{ clz r5, r6 ; finv r15 }
    21f0:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; finv r15 }
    21f8:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; finv r15 }
    2200:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; finv r15 }
    2208:	[0-9a-f]* 	{ seqib r5, r6, 5 ; finv r15 }
    2210:	[0-9a-f]* 	{ slteb r5, r6, r7 ; finv r15 }
    2218:	[0-9a-f]* 	{ sraih r5, r6, 5 ; finv r15 }
    2220:	[0-9a-f]* 	{ addih r5, r6, 5 ; flush r15 }
    2228:	[0-9a-f]* 	{ infol 4660 ; flush r15 }
    2230:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; flush r15 }
    2238:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; flush r15 }
    2240:	[0-9a-f]* 	{ s1a r5, r6, r7 ; flush r15 }
    2248:	[0-9a-f]* 	{ shlih r5, r6, 5 ; flush r15 }
    2250:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; flush r15 }
    2258:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; flush r15 }
    2260:	[0-9a-f]* 	{ add r5, r6, r7 ; lw r25, r26 }
    2268:	[0-9a-f]* 	{ addi r15, r16, 5 ; sb r25, r26 }
    2270:	[0-9a-f]* 	{ addli.sn r15, r16, 4660 }
    2278:	[0-9a-f]* 	{ and r5, r6, r7 ; lw r25, r26 }
    2280:	[0-9a-f]* 	{ andi r5, r6, 5 ; lw r25, r26 }
    2288:	[0-9a-f]* 	{ bytex r5, r6 ; lb r25, r26 }
    2290:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 }
    2298:	[0-9a-f]* 	{ lw r25, r26 }
    22a0:	[0-9a-f]* 	{ info 19 ; lh_u r25, r26 }
    22a8:	[0-9a-f]* 	{ jr r15 }
    22b0:	[0-9a-f]* 	{ move r15, r16 ; lb r25, r26 }
    22b8:	[0-9a-f]* 	{ or r15, r16, r17 ; lb r25, r26 }
    22c0:	[0-9a-f]* 	{ shl r5, r6, r7 ; lb r25, r26 }
    22c8:	[0-9a-f]* 	{ sne r5, r6, r7 ; lb r25, r26 }
    22d0:	[0-9a-f]* 	{ and r5, r6, r7 ; lb_u r25, r26 }
    22d8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; lb_u r25, r26 }
    22e0:	[0-9a-f]* 	{ rli r5, r6, 5 ; lb_u r25, r26 }
    22e8:	[0-9a-f]* 	{ slt r5, r6, r7 ; lb_u r25, r26 }
    22f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; lb_u r25, r26 }
    22f8:	[0-9a-f]* 	{ ctz r5, r6 ; lh r25, r26 }
    2300:	[0-9a-f]* 	{ mvz r5, r6, r7 ; lh r25, r26 }
    2308:	[0-9a-f]* 	{ s3a r5, r6, r7 ; lh r25, r26 }
    2310:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; lh r25, r26 }
    2318:	[0-9a-f]* 	{ lh_u r15, r16 }
    2320:	[0-9a-f]* 	{ movei r15, 5 ; lh_u r25, r26 }
    2328:	[0-9a-f]* 	{ ori r15, r16, 5 ; lh_u r25, r26 }
    2330:	[0-9a-f]* 	{ shli r5, r6, 5 ; lh_u r25, r26 }
    2338:	[0-9a-f]* 	{ sra r5, r6, r7 ; lh_u r25, r26 }
    2340:	[0-9a-f]* 	{ and r15, r16, r17 ; lw r25, r26 }
    2348:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lw r25, r26 }
    2350:	[0-9a-f]* 	{ rli r15, r16, 5 ; lw r25, r26 }
    2358:	[0-9a-f]* 	{ slt r15, r16, r17 ; lw r25, r26 }
    2360:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lw r25, r26 }
    2368:	[0-9a-f]* 	{ minb_u r15, r16, r17 }
    2370:	[0-9a-f]* 	{ mnz r5, r6, r7 ; lb r25, r26 }
    2378:	[0-9a-f]* 	{ move r15, r16 ; sb r25, r26 }
    2380:	[0-9a-f]* 	{ movei r15, 5 ; sb r25, r26 }
    2388:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lb_u r25, r26 }
    2390:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lb r25, r26 }
    2398:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 }
    23a0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; lb r25, r26 }
    23a8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 }
    23b0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sw r25, r26 }
    23b8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sh r25, r26 }
    23c0:	[0-9a-f]* 	{ mz r5, r6, r7 ; sh r25, r26 }
    23c8:	[0-9a-f]* 	{ nor r15, r16, r17 ; lh_u r25, r26 }
    23d0:	[0-9a-f]* 	{ or r15, r16, r17 ; lh_u r25, r26 }
    23d8:	[0-9a-f]* 	{ ori r15, r16, 5 ; lh_u r25, r26 }
    23e0:	[0-9a-f]* 	{ packhb r5, r6, r7 }
    23e8:	[0-9a-f]* 	{ and r15, r16, r17 ; prefetch r25 }
    23f0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; prefetch r25 }
    23f8:	[0-9a-f]* 	{ rli r15, r16, 5 ; prefetch r25 }
    2400:	[0-9a-f]* 	{ slt r15, r16, r17 ; prefetch r25 }
    2408:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; prefetch r25 }
    2410:	[0-9a-f]* 	{ rl r5, r6, r7 ; lh r25, r26 }
    2418:	[0-9a-f]* 	{ rli r5, r6, 5 ; lh r25, r26 }
    2420:	[0-9a-f]* 	{ s1a r5, r6, r7 ; lh r25, r26 }
    2428:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lh r25, r26 }
    2430:	[0-9a-f]* 	{ s3a r5, r6, r7 ; lh r25, r26 }
    2438:	[0-9a-f]* 	{ and r5, r6, r7 ; sb r25, r26 }
    2440:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sb r25, r26 }
    2448:	[0-9a-f]* 	{ rli r5, r6, 5 ; sb r25, r26 }
    2450:	[0-9a-f]* 	{ slt r5, r6, r7 ; sb r25, r26 }
    2458:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sb r25, r26 }
    2460:	[0-9a-f]* 	{ seq r5, r6, r7 ; lh_u r25, r26 }
    2468:	[0-9a-f]* 	{ seqi r15, r16, 5 }
    2470:	[0-9a-f]* 	{ and r15, r16, r17 ; sh r25, r26 }
    2478:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sh r25, r26 }
    2480:	[0-9a-f]* 	{ rli r15, r16, 5 ; sh r25, r26 }
    2488:	[0-9a-f]* 	{ slt r15, r16, r17 ; sh r25, r26 }
    2490:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sh r25, r26 }
    2498:	[0-9a-f]* 	{ shl r5, r6, r7 ; lh r25, r26 }
    24a0:	[0-9a-f]* 	{ shli r15, r16, 5 ; sw r25, r26 }
    24a8:	[0-9a-f]* 	{ shr r15, r16, r17 ; lw r25, r26 }
    24b0:	[0-9a-f]* 	{ shri r15, r16, 5 ; lb r25, r26 }
    24b8:	[0-9a-f]* 	{ shrib r15, r16, 5 }
    24c0:	[0-9a-f]* 	{ slt r5, r6, r7 ; sb r25, r26 }
    24c8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sb r25, r26 }
    24d0:	[0-9a-f]* 	{ slte r5, r6, r7 ; lh r25, r26 }
    24d8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; lh r25, r26 }
    24e0:	[0-9a-f]* 	{ slti r15, r16, 5 ; lb r25, r26 }
    24e8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; lb r25, r26 }
    24f0:	[0-9a-f]* 	{ sltib r15, r16, 5 }
    24f8:	[0-9a-f]* 	{ sne r5, r6, r7 ; lh r25, r26 }
    2500:	[0-9a-f]* 	{ sra r15, r16, r17 ; sw r25, r26 }
    2508:	[0-9a-f]* 	{ srai r15, r16, 5 ; lw r25, r26 }
    2510:	[0-9a-f]* 	{ sub r15, r16, r17 ; lb r25, r26 }
    2518:	[0-9a-f]* 	{ subb r15, r16, r17 }
    2520:	[0-9a-f]* 	{ bytex r5, r6 ; sw r25, r26 }
    2528:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sw r25, r26 }
    2530:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sw r25, r26 }
    2538:	[0-9a-f]* 	{ slte r5, r6, r7 ; sw r25, r26 }
    2540:	[0-9a-f]* 	{ xor r5, r6, r7 ; sw r25, r26 }
    2548:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sh r25, r26 }
    2550:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sh r25, r26 }
    2558:	[0-9a-f]* 	{ xor r5, r6, r7 ; prefetch r25 }
    2560:	[0-9a-f]* 	{ and r5, r6, r7 ; icoh r15 }
    2568:	[0-9a-f]* 	{ maxh r5, r6, r7 ; icoh r15 }
    2570:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; icoh r15 }
    2578:	[0-9a-f]* 	{ mz r5, r6, r7 ; icoh r15 }
    2580:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; icoh r15 }
    2588:	[0-9a-f]* 	{ shrih r5, r6, 5 ; icoh r15 }
    2590:	[0-9a-f]* 	{ sneb r5, r6, r7 ; icoh r15 }
    2598:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; lb r25, r26 }
    25a0:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; sb r25, r26 }
    25a8:	[0-9a-f]* 	{ and r5, r6, r7 ; ill }
    25b0:	[0-9a-f]* 	{ bitx r5, r6 ; ill ; sb r25, r26 }
    25b8:	[0-9a-f]* 	{ clz r5, r6 ; ill ; sb r25, r26 }
    25c0:	[0-9a-f]* 	{ ill ; lh_u r25, r26 }
    25c8:	[0-9a-f]* 	{ intlb r5, r6, r7 ; ill }
    25d0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ill ; lb r25, r26 }
    25d8:	[0-9a-f]* 	{ shli r5, r6, 5 ; ill ; lb r25, r26 }
    25e0:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; lb_u r25, r26 }
    25e8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ill ; lb_u r25, r26 }
    25f0:	[0-9a-f]* 	{ slt r5, r6, r7 ; ill ; lb_u r25, r26 }
    25f8:	[0-9a-f]* 	{ bitx r5, r6 ; ill ; lh r25, r26 }
    2600:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; lh r25, r26 }
    2608:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; ill ; lh r25, r26 }
    2610:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; lh_u r25, r26 }
    2618:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; lh_u r25, r26 }
    2620:	[0-9a-f]* 	{ sne r5, r6, r7 ; ill ; lh_u r25, r26 }
    2628:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; lw r25, r26 }
    2630:	[0-9a-f]* 	{ rl r5, r6, r7 ; ill ; lw r25, r26 }
    2638:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; lw r25, r26 }
    2640:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; lw r25, r26 }
    2648:	[0-9a-f]* 	{ movei r5, 5 ; ill ; lh r25, r26 }
    2650:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; ill }
    2658:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ill }
    2660:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; ill }
    2668:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ill }
    2670:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; ill ; sw r25, r26 }
    2678:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ill ; sb r25, r26 }
    2680:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; sb r25, r26 }
    2688:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; lw r25, r26 }
    2690:	[0-9a-f]* 	{ ori r5, r6, 5 ; ill ; lw r25, r26 }
    2698:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; prefetch r25 }
    26a0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; ill ; prefetch r25 }
    26a8:	[0-9a-f]* 	{ shri r5, r6, 5 ; ill ; prefetch r25 }
    26b0:	[0-9a-f]* 	{ rl r5, r6, r7 ; ill ; lh_u r25, r26 }
    26b8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; ill ; lh_u r25, r26 }
    26c0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ill ; lh_u r25, r26 }
    26c8:	[0-9a-f]* 	{ ctz r5, r6 ; ill ; sb r25, r26 }
    26d0:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; sb r25, r26 }
    26d8:	[0-9a-f]* 	{ sne r5, r6, r7 ; ill ; sb r25, r26 }
    26e0:	[0-9a-f]* 	{ seqb r5, r6, r7 ; ill }
    26e8:	[0-9a-f]* 	{ clz r5, r6 ; ill ; sh r25, r26 }
    26f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; sh r25, r26 }
    26f8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ill ; sh r25, r26 }
    2700:	[0-9a-f]* 	{ shl r5, r6, r7 ; ill }
    2708:	[0-9a-f]* 	{ shr r5, r6, r7 ; ill ; prefetch r25 }
    2710:	[0-9a-f]* 	{ slt r5, r6, r7 ; ill ; lb_u r25, r26 }
    2718:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; ill }
    2720:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; ill }
    2728:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ill ; lh_u r25, r26 }
    2730:	[0-9a-f]* 	{ sne r5, r6, r7 ; ill }
    2738:	[0-9a-f]* 	{ srai r5, r6, 5 ; ill ; prefetch r25 }
    2740:	[0-9a-f]* 	{ subhs r5, r6, r7 ; ill }
    2748:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ill ; sw r25, r26 }
    2750:	[0-9a-f]* 	{ shli r5, r6, 5 ; ill ; sw r25, r26 }
    2758:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; lb_u r25, r26 }
    2760:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ill ; lb_u r25, r26 }
    2768:	[0-9a-f]* 	{ xor r5, r6, r7 ; ill ; lb_u r25, r26 }
    2770:	[0-9a-f]* 	{ info 19 ; add r5, r6, r7 ; lb r25, r26 }
    2778:	[0-9a-f]* 	{ info 19 ; addi r15, r16, 5 ; lh r25, r26 }
    2780:	[0-9a-f]* 	{ info 19 ; addih r15, r16, 5 }
    2788:	[0-9a-f]* 	{ info 19 ; and r5, r6, r7 ; lb r25, r26 }
    2790:	[0-9a-f]* 	{ info 19 ; andi r5, r6, 5 ; lb r25, r26 }
    2798:	[0-9a-f]* 	{ bitx r5, r6 ; info 19 ; sb r25, r26 }
    27a0:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; sb r25, r26 }
    27a8:	[0-9a-f]* 	{ info 19 ; lb r25, r26 }
    27b0:	[0-9a-f]* 	{ info 19 ; ill }
    27b8:	[0-9a-f]* 	{ info 19 ; inv r15 }
    27c0:	[0-9a-f]* 	{ info 19 ; ill ; lb r25, r26 }
    27c8:	[0-9a-f]* 	{ info 19 ; mz r5, r6, r7 ; lb r25, r26 }
    27d0:	[0-9a-f]* 	{ info 19 ; seq r5, r6, r7 ; lb r25, r26 }
    27d8:	[0-9a-f]* 	{ info 19 ; slti r5, r6, 5 ; lb r25, r26 }
    27e0:	[0-9a-f]* 	{ info 19 ; add r5, r6, r7 ; lb_u r25, r26 }
    27e8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    27f0:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; lb_u r25, r26 }
    27f8:	[0-9a-f]* 	{ info 19 ; shr r5, r6, r7 ; lb_u r25, r26 }
    2800:	[0-9a-f]* 	{ info 19 ; srai r5, r6, 5 ; lb_u r25, r26 }
    2808:	[0-9a-f]* 	{ info 19 ; andi r5, r6, 5 ; lh r25, r26 }
    2810:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; info 19 ; lh r25, r26 }
    2818:	[0-9a-f]* 	{ info 19 ; s1a r5, r6, r7 ; lh r25, r26 }
    2820:	[0-9a-f]* 	{ info 19 ; slt_u r5, r6, r7 ; lh r25, r26 }
    2828:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; info 19 ; lh r25, r26 }
    2830:	[0-9a-f]* 	{ info 19 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    2838:	[0-9a-f]* 	{ info 19 ; nor r15, r16, r17 ; lh_u r25, r26 }
    2840:	[0-9a-f]* 	{ info 19 ; seqi r5, r6, 5 ; lh_u r25, r26 }
    2848:	[0-9a-f]* 	{ info 19 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    2850:	[0-9a-f]* 	{ info 19 ; add r15, r16, r17 ; lw r25, r26 }
    2858:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; lw r25, r26 }
    2860:	[0-9a-f]* 	{ info 19 ; ori r5, r6, 5 ; lw r25, r26 }
    2868:	[0-9a-f]* 	{ info 19 ; shr r15, r16, r17 ; lw r25, r26 }
    2870:	[0-9a-f]* 	{ info 19 ; srai r15, r16, 5 ; lw r25, r26 }
    2878:	[0-9a-f]* 	{ info 19 ; maxih r15, r16, 5 }
    2880:	[0-9a-f]* 	{ info 19 ; mnz r15, r16, r17 ; sb r25, r26 }
    2888:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; lh r25, r26 }
    2890:	[0-9a-f]* 	{ info 19 ; movei r15, 5 ; lh r25, r26 }
    2898:	[0-9a-f]* 	{ info 19 ; moveli.sn r15, 4660 }
    28a0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; info 19 ; sb r25, r26 }
    28a8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; info 19 ; prefetch r25 }
    28b0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; info 19 ; sb r25, r26 }
    28b8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; info 19 ; prefetch r25 }
    28c0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; info 19 ; lw r25, r26 }
    28c8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    28d0:	[0-9a-f]* 	{ info 19 ; mz r5, r6, r7 ; lh_u r25, r26 }
    28d8:	[0-9a-f]* 	{ info 19 ; nop }
    28e0:	[0-9a-f]* 	{ info 19 ; nor r5, r6, r7 }
    28e8:	[0-9a-f]* 	{ info 19 ; or r5, r6, r7 }
    28f0:	[0-9a-f]* 	{ info 19 ; ori r5, r6, 5 }
    28f8:	[0-9a-f]* 	{ info 19 ; add r15, r16, r17 ; prefetch r25 }
    2900:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; prefetch r25 }
    2908:	[0-9a-f]* 	{ info 19 ; ori r5, r6, 5 ; prefetch r25 }
    2910:	[0-9a-f]* 	{ info 19 ; shr r15, r16, r17 ; prefetch r25 }
    2918:	[0-9a-f]* 	{ info 19 ; srai r15, r16, 5 ; prefetch r25 }
    2920:	[0-9a-f]* 	{ info 19 ; rl r15, r16, r17 ; sw r25, r26 }
    2928:	[0-9a-f]* 	{ info 19 ; rli r15, r16, 5 ; sw r25, r26 }
    2930:	[0-9a-f]* 	{ info 19 ; s1a r15, r16, r17 ; sw r25, r26 }
    2938:	[0-9a-f]* 	{ info 19 ; s2a r15, r16, r17 ; sw r25, r26 }
    2940:	[0-9a-f]* 	{ info 19 ; s3a r15, r16, r17 ; sw r25, r26 }
    2948:	[0-9a-f]* 	{ info 19 ; add r5, r6, r7 ; sb r25, r26 }
    2950:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; info 19 ; sb r25, r26 }
    2958:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; sb r25, r26 }
    2960:	[0-9a-f]* 	{ info 19 ; shr r5, r6, r7 ; sb r25, r26 }
    2968:	[0-9a-f]* 	{ info 19 ; srai r5, r6, 5 ; sb r25, r26 }
    2970:	[0-9a-f]* 	{ info 19 ; seq r15, r16, r17 }
    2978:	[0-9a-f]* 	{ info 19 ; seqi r15, r16, 5 ; prefetch r25 }
    2980:	[0-9a-f]* 	{ info 19 ; add r15, r16, r17 ; sh r25, r26 }
    2988:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; sh r25, r26 }
    2990:	[0-9a-f]* 	{ info 19 ; ori r5, r6, 5 ; sh r25, r26 }
    2998:	[0-9a-f]* 	{ info 19 ; shr r15, r16, r17 ; sh r25, r26 }
    29a0:	[0-9a-f]* 	{ info 19 ; srai r15, r16, 5 ; sh r25, r26 }
    29a8:	[0-9a-f]* 	{ info 19 ; shl r15, r16, r17 ; sw r25, r26 }
    29b0:	[0-9a-f]* 	{ info 19 ; shli r15, r16, 5 ; lw r25, r26 }
    29b8:	[0-9a-f]* 	{ info 19 ; shr r15, r16, r17 ; lb r25, r26 }
    29c0:	[0-9a-f]* 	{ info 19 ; shrb r15, r16, r17 }
    29c8:	[0-9a-f]* 	{ info 19 ; shri r5, r6, 5 ; sb r25, r26 }
    29d0:	[0-9a-f]* 	{ info 19 ; slt r5, r6, r7 ; lh r25, r26 }
    29d8:	[0-9a-f]* 	{ info 19 ; slt_u r5, r6, r7 ; lh r25, r26 }
    29e0:	[0-9a-f]* 	{ info 19 ; slte r15, r16, r17 ; sw r25, r26 }
    29e8:	[0-9a-f]* 	{ info 19 ; slte_u r15, r16, r17 ; sw r25, r26 }
    29f0:	[0-9a-f]* 	{ info 19 ; slth r15, r16, r17 }
    29f8:	[0-9a-f]* 	{ info 19 ; slti r5, r6, 5 ; sb r25, r26 }
    2a00:	[0-9a-f]* 	{ info 19 ; slti_u r5, r6, 5 ; sb r25, r26 }
    2a08:	[0-9a-f]* 	{ info 19 ; sne r15, r16, r17 ; sw r25, r26 }
    2a10:	[0-9a-f]* 	{ info 19 ; sra r15, r16, r17 ; lw r25, r26 }
    2a18:	[0-9a-f]* 	{ info 19 ; srai r15, r16, 5 ; lb r25, r26 }
    2a20:	[0-9a-f]* 	{ info 19 ; sraib r15, r16, 5 }
    2a28:	[0-9a-f]* 	{ info 19 ; sub r5, r6, r7 ; sb r25, r26 }
    2a30:	[0-9a-f]* 	{ info 19 ; and r5, r6, r7 ; sw r25, r26 }
    2a38:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; info 19 ; sw r25, r26 }
    2a40:	[0-9a-f]* 	{ info 19 ; rli r5, r6, 5 ; sw r25, r26 }
    2a48:	[0-9a-f]* 	{ info 19 ; slt r5, r6, r7 ; sw r25, r26 }
    2a50:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; sw r25, r26 }
    2a58:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; lh_u r25, r26 }
    2a60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; info 19 ; lh_u r25, r26 }
    2a68:	[0-9a-f]* 	{ info 19 ; xor r5, r6, r7 ; lb_u r25, r26 }
    2a70:	[0-9a-f]* 	{ infol 4660 ; addhs r5, r6, r7 }
    2a78:	[0-9a-f]* 	{ infol 4660 ; auli r5, r6, 4660 }
    2a80:	[0-9a-f]* 	{ infol 4660 ; inthh r15, r16, r17 }
    2a88:	[0-9a-f]* 	{ infol 4660 ; lnk r15 }
    2a90:	[0-9a-f]* 	{ infol 4660 ; minib_u r5, r6, 5 }
    2a98:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; infol 4660 }
    2aa0:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; infol 4660 }
    2aa8:	[0-9a-f]* 	{ infol 4660 ; packhb r15, r16, r17 }
    2ab0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; infol 4660 }
    2ab8:	[0-9a-f]* 	{ infol 4660 ; shadd r15, r16, 5 }
    2ac0:	[0-9a-f]* 	{ infol 4660 ; shri r5, r6, 5 }
    2ac8:	[0-9a-f]* 	{ infol 4660 ; slteb_u r5, r6, r7 }
    2ad0:	[0-9a-f]* 	{ infol 4660 ; sltih_u r5, r6, 5 }
    2ad8:	[0-9a-f]* 	{ infol 4660 ; sub r5, r6, r7 }
    2ae0:	[0-9a-f]* 	{ infol 4660 ; xor r5, r6, r7 }
    2ae8:	[0-9a-f]* 	{ avgh r5, r6, r7 ; inthb r15, r16, r17 }
    2af0:	[0-9a-f]* 	{ inthb r15, r16, r17 ; minh r5, r6, r7 }
    2af8:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; inthb r15, r16, r17 }
    2b00:	[0-9a-f]* 	{ inthb r15, r16, r17 ; nor r5, r6, r7 }
    2b08:	[0-9a-f]* 	{ inthb r15, r16, r17 ; seqb r5, r6, r7 }
    2b10:	[0-9a-f]* 	{ inthb r15, r16, r17 ; sltb_u r5, r6, r7 }
    2b18:	[0-9a-f]* 	{ inthb r15, r16, r17 ; srah r5, r6, r7 }
    2b20:	[0-9a-f]* 	{ inthb r5, r6, r7 ; addhs r15, r16, r17 }
    2b28:	[0-9a-f]* 	{ inthb r5, r6, r7 ; intlb r15, r16, r17 }
    2b30:	[0-9a-f]* 	{ inthb r5, r6, r7 ; lwadd_na r15, r16, 5 }
    2b38:	[0-9a-f]* 	{ inthb r5, r6, r7 ; mz r15, r16, r17 }
    2b40:	[0-9a-f]* 	{ inthb r5, r6, r7 ; seq r15, r16, r17 }
    2b48:	[0-9a-f]* 	{ inthb r5, r6, r7 ; slt r15, r16, r17 }
    2b50:	[0-9a-f]* 	{ inthb r5, r6, r7 ; sneh r15, r16, r17 }
    2b58:	[0-9a-f]* 	{ inthh r15, r16, r17 ; addb r5, r6, r7 }
    2b60:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; inthh r15, r16, r17 }
    2b68:	[0-9a-f]* 	{ inthh r15, r16, r17 ; mnz r5, r6, r7 }
    2b70:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; inthh r15, r16, r17 }
    2b78:	[0-9a-f]* 	{ inthh r15, r16, r17 ; packhb r5, r6, r7 }
    2b80:	[0-9a-f]* 	{ inthh r15, r16, r17 ; seqih r5, r6, 5 }
    2b88:	[0-9a-f]* 	{ inthh r15, r16, r17 ; slteb_u r5, r6, r7 }
    2b90:	[0-9a-f]* 	{ inthh r15, r16, r17 ; sub r5, r6, r7 }
    2b98:	[0-9a-f]* 	{ inthh r5, r6, r7 ; addli r15, r16, 4660 }
    2ba0:	[0-9a-f]* 	{ inthh r5, r6, r7 ; jalr r15 }
    2ba8:	[0-9a-f]* 	{ inthh r5, r6, r7 ; maxih r15, r16, 5 }
    2bb0:	[0-9a-f]* 	{ inthh r5, r6, r7 ; nor r15, r16, r17 }
    2bb8:	[0-9a-f]* 	{ inthh r5, r6, r7 ; seqib r15, r16, 5 }
    2bc0:	[0-9a-f]* 	{ inthh r5, r6, r7 ; slte r15, r16, r17 }
    2bc8:	[0-9a-f]* 	{ inthh r5, r6, r7 ; srai r15, r16, 5 }
    2bd0:	[0-9a-f]* 	{ intlb r15, r16, r17 ; addi r5, r6, 5 }
    2bd8:	[0-9a-f]* 	{ intlb r15, r16, r17 }
    2be0:	[0-9a-f]* 	{ intlb r15, r16, r17 ; movei r5, 5 }
    2be8:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; intlb r15, r16, r17 }
    2bf0:	[0-9a-f]* 	{ intlb r15, r16, r17 ; rl r5, r6, r7 }
    2bf8:	[0-9a-f]* 	{ intlb r15, r16, r17 ; shli r5, r6, 5 }
    2c00:	[0-9a-f]* 	{ intlb r15, r16, r17 ; slth_u r5, r6, r7 }
    2c08:	[0-9a-f]* 	{ intlb r15, r16, r17 ; subhs r5, r6, r7 }
    2c10:	[0-9a-f]* 	{ intlb r5, r6, r7 ; andi r15, r16, 5 }
    2c18:	[0-9a-f]* 	{ intlb r5, r6, r7 ; lb r15, r16 }
    2c20:	[0-9a-f]* 	{ intlb r5, r6, r7 ; minh r15, r16, r17 }
    2c28:	[0-9a-f]* 	{ intlb r5, r6, r7 ; packhb r15, r16, r17 }
    2c30:	[0-9a-f]* 	{ intlb r5, r6, r7 ; shl r15, r16, r17 }
    2c38:	[0-9a-f]* 	{ intlb r5, r6, r7 ; slteh r15, r16, r17 }
    2c40:	[0-9a-f]* 	{ intlb r5, r6, r7 ; subb r15, r16, r17 }
    2c48:	[0-9a-f]* 	{ intlh r15, r16, r17 ; addli.sn r5, r6, 4660 }
    2c50:	[0-9a-f]* 	{ intlh r15, r16, r17 ; inthh r5, r6, r7 }
    2c58:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; intlh r15, r16, r17 }
    2c60:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; intlh r15, r16, r17 }
    2c68:	[0-9a-f]* 	{ intlh r15, r16, r17 ; s3a r5, r6, r7 }
    2c70:	[0-9a-f]* 	{ intlh r15, r16, r17 ; shrb r5, r6, r7 }
    2c78:	[0-9a-f]* 	{ intlh r15, r16, r17 ; sltib_u r5, r6, 5 }
    2c80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; intlh r15, r16, r17 }
    2c88:	[0-9a-f]* 	{ intlh r5, r6, r7 ; flush r15 }
    2c90:	[0-9a-f]* 	{ intlh r5, r6, r7 ; lh r15, r16 }
    2c98:	[0-9a-f]* 	{ intlh r5, r6, r7 ; mnz r15, r16, r17 }
    2ca0:	[0-9a-f]* 	{ intlh r5, r6, r7 ; raise }
    2ca8:	[0-9a-f]* 	{ intlh r5, r6, r7 ; shlib r15, r16, 5 }
    2cb0:	[0-9a-f]* 	{ intlh r5, r6, r7 ; slti r15, r16, 5 }
    2cb8:	[0-9a-f]* 	{ intlh r5, r6, r7 ; subs r15, r16, r17 }
    2cc0:	[0-9a-f]* 	{ and r5, r6, r7 ; inv r15 }
    2cc8:	[0-9a-f]* 	{ maxh r5, r6, r7 ; inv r15 }
    2cd0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; inv r15 }
    2cd8:	[0-9a-f]* 	{ mz r5, r6, r7 ; inv r15 }
    2ce0:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; inv r15 }
    2ce8:	[0-9a-f]* 	{ shrih r5, r6, 5 ; inv r15 }
    2cf0:	[0-9a-f]* 	{ sneb r5, r6, r7 ; inv r15 }
    2cf8:	[0-9a-f]* 	{ add r5, r6, r7 ; iret }
    2d00:	[0-9a-f]* 	{ clz r5, r6 ; iret }
    2d08:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; iret }
    2d10:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; iret }
    2d18:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; iret }
    2d20:	[0-9a-f]* 	{ seqib r5, r6, 5 ; iret }
    2d28:	[0-9a-f]* 	{ slteb r5, r6, r7 ; iret }
    2d30:	[0-9a-f]* 	{ sraih r5, r6, 5 ; iret }
    2d38:	[0-9a-f]* 	{ addih r5, r6, 5 ; jalr r15 }
    2d40:	[0-9a-f]* 	{ infol 4660 ; jalr r15 }
    2d48:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; jalr r15 }
    2d50:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; jalr r15 }
    2d58:	[0-9a-f]* 	{ s1a r5, r6, r7 ; jalr r15 }
    2d60:	[0-9a-f]* 	{ shlih r5, r6, 5 ; jalr r15 }
    2d68:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; jalr r15 }
    2d70:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; jalr r15 }
    2d78:	[0-9a-f]* 	{ andi r5, r6, 5 ; jalrp r15 }
    2d80:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; jalrp r15 }
    2d88:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; jalrp r15 }
    2d90:	[0-9a-f]* 	{ mzb r5, r6, r7 ; jalrp r15 }
    2d98:	[0-9a-f]* 	{ sadh r5, r6, r7 ; jalrp r15 }
    2da0:	[0-9a-f]* 	{ slt r5, r6, r7 ; jalrp r15 }
    2da8:	[0-9a-f]* 	{ sneh r5, r6, r7 ; jalrp r15 }
    2db0:	[0-9a-f]* 	{ addb r5, r6, r7 ; jr r15 }
    2db8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; jr r15 }
    2dc0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; jr r15 }
    2dc8:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; jr r15 }
    2dd0:	[0-9a-f]* 	{ packhb r5, r6, r7 ; jr r15 }
    2dd8:	[0-9a-f]* 	{ seqih r5, r6, 5 ; jr r15 }
    2de0:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; jr r15 }
    2de8:	[0-9a-f]* 	{ sub r5, r6, r7 ; jr r15 }
    2df0:	[0-9a-f]* 	{ addli r5, r6, 4660 ; jrp r15 }
    2df8:	[0-9a-f]* 	{ inthb r5, r6, r7 ; jrp r15 }
    2e00:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; jrp r15 }
    2e08:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; jrp r15 }
    2e10:	[0-9a-f]* 	{ s2a r5, r6, r7 ; jrp r15 }
    2e18:	[0-9a-f]* 	{ shr r5, r6, r7 ; jrp r15 }
    2e20:	[0-9a-f]* 	{ sltib r5, r6, 5 ; jrp r15 }
    2e28:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; jrp r15 }
    2e30:	[0-9a-f]* 	{ auli r5, r6, 4660 ; lb r15, r16 }
    2e38:	[0-9a-f]* 	{ maxih r5, r6, 5 ; lb r15, r16 }
    2e40:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; lb r15, r16 }
    2e48:	[0-9a-f]* 	{ mzh r5, r6, r7 ; lb r15, r16 }
    2e50:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; lb r15, r16 }
    2e58:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; lb r15, r16 }
    2e60:	[0-9a-f]* 	{ sra r5, r6, r7 ; lb r15, r16 }
    2e68:	[0-9a-f]* 	{ add r15, r16, r17 ; and r5, r6, r7 ; lb r25, r26 }
    2e70:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; add r15, r16, r17 ; lb r25, r26 }
    2e78:	[0-9a-f]* 	{ add r15, r16, r17 ; slt_u r5, r6, r7 ; lb r25, r26 }
    2e80:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; lb r25, r26 }
    2e88:	[0-9a-f]* 	{ add r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    2e90:	[0-9a-f]* 	{ ctz r5, r6 ; addi r15, r16, 5 ; lb r25, r26 }
    2e98:	[0-9a-f]* 	{ addi r15, r16, 5 ; or r5, r6, r7 ; lb r25, r26 }
    2ea0:	[0-9a-f]* 	{ addi r15, r16, 5 ; sne r5, r6, r7 ; lb r25, r26 }
    2ea8:	[0-9a-f]* 	{ addi r5, r6, 5 ; mz r15, r16, r17 ; lb r25, r26 }
    2eb0:	[0-9a-f]* 	{ addi r5, r6, 5 ; slti r15, r16, 5 ; lb r25, r26 }
    2eb8:	[0-9a-f]* 	{ and r15, r16, r17 ; movei r5, 5 ; lb r25, r26 }
    2ec0:	[0-9a-f]* 	{ and r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
    2ec8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    2ed0:	[0-9a-f]* 	{ and r5, r6, r7 ; rl r15, r16, r17 ; lb r25, r26 }
    2ed8:	[0-9a-f]* 	{ and r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    2ee0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; andi r15, r16, 5 ; lb r25, r26 }
    2ee8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl r5, r6, r7 ; lb r25, r26 }
    2ef0:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; lb r25, r26 }
    2ef8:	[0-9a-f]* 	{ andi r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
    2f00:	[0-9a-f]* 	{ bitx r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    2f08:	[0-9a-f]* 	{ bitx r5, r6 ; shl r15, r16, r17 ; lb r25, r26 }
    2f10:	[0-9a-f]* 	{ bytex r5, r6 ; lb r25, r26 }
    2f18:	[0-9a-f]* 	{ bytex r5, r6 ; shr r15, r16, r17 ; lb r25, r26 }
    2f20:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; lb r25, r26 }
    2f28:	[0-9a-f]* 	{ clz r5, r6 ; slt r15, r16, r17 ; lb r25, r26 }
    2f30:	[0-9a-f]* 	{ ctz r5, r6 ; move r15, r16 ; lb r25, r26 }
    2f38:	[0-9a-f]* 	{ ctz r5, r6 ; slte r15, r16, r17 ; lb r25, r26 }
    2f40:	[0-9a-f]* 	{ clz r5, r6 ; lb r25, r26 }
    2f48:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; lb r25, r26 }
    2f50:	[0-9a-f]* 	{ s3a r15, r16, r17 ; lb r25, r26 }
    2f58:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; lb r25, r26 }
    2f60:	[0-9a-f]* 	{ lb r25, r26 }
    2f68:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ill ; lb r25, r26 }
    2f70:	[0-9a-f]* 	{ shr r5, r6, r7 ; ill ; lb r25, r26 }
    2f78:	[0-9a-f]* 	{ info 19 ; addi r15, r16, 5 ; lb r25, r26 }
    2f80:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; info 19 ; lb r25, r26 }
    2f88:	[0-9a-f]* 	{ info 19 ; rl r15, r16, r17 ; lb r25, r26 }
    2f90:	[0-9a-f]* 	{ info 19 ; shri r15, r16, 5 ; lb r25, r26 }
    2f98:	[0-9a-f]* 	{ info 19 ; sub r15, r16, r17 ; lb r25, r26 }
    2fa0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    2fa8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rli r5, r6, 5 ; lb r25, r26 }
    2fb0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; lb r25, r26 }
    2fb8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    2fc0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    2fc8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    2fd0:	[0-9a-f]* 	{ move r15, r16 ; seqi r5, r6, 5 ; lb r25, r26 }
    2fd8:	[0-9a-f]* 	{ move r15, r16 ; lb r25, r26 }
    2fe0:	[0-9a-f]* 	{ move r5, r6 ; s3a r15, r16, r17 ; lb r25, r26 }
    2fe8:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 ; lb r25, r26 }
    2ff0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
    2ff8:	[0-9a-f]* 	{ movei r15, 5 ; slt r5, r6, r7 ; lb r25, r26 }
    3000:	[0-9a-f]* 	{ movei r5, 5 ; lb r25, r26 }
    3008:	[0-9a-f]* 	{ movei r5, 5 ; shr r15, r16, r17 ; lb r25, r26 }
    3010:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; info 19 ; lb r25, r26 }
    3018:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    3020:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    3028:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    3030:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    3038:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    3040:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    3048:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    3050:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    3058:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    3060:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    3068:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lb r25, r26 }
    3070:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    3078:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; add r15, r16, r17 ; lb r25, r26 }
    3080:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    3088:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    3090:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    3098:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; lb r25, r26 }
    30a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    30a8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; info 19 ; lb r25, r26 }
    30b0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    30b8:	[0-9a-f]* 	{ mz r15, r16, r17 ; lb r25, r26 }
    30c0:	[0-9a-f]* 	{ mz r15, r16, r17 ; ori r5, r6, 5 ; lb r25, r26 }
    30c8:	[0-9a-f]* 	{ mz r15, r16, r17 ; sra r5, r6, r7 ; lb r25, r26 }
    30d0:	[0-9a-f]* 	{ mz r5, r6, r7 ; nop ; lb r25, r26 }
    30d8:	[0-9a-f]* 	{ mz r5, r6, r7 ; slti_u r15, r16, 5 ; lb r25, r26 }
    30e0:	[0-9a-f]* 	{ nop ; ill ; lb r25, r26 }
    30e8:	[0-9a-f]* 	{ nop ; mz r5, r6, r7 ; lb r25, r26 }
    30f0:	[0-9a-f]* 	{ nop ; seq r5, r6, r7 ; lb r25, r26 }
    30f8:	[0-9a-f]* 	{ nop ; slti r5, r6, 5 ; lb r25, r26 }
    3100:	[0-9a-f]* 	{ nor r15, r16, r17 ; and r5, r6, r7 ; lb r25, r26 }
    3108:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    3110:	[0-9a-f]* 	{ nor r15, r16, r17 ; slt_u r5, r6, r7 ; lb r25, r26 }
    3118:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; lb r25, r26 }
    3120:	[0-9a-f]* 	{ nor r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    3128:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; lb r25, r26 }
    3130:	[0-9a-f]* 	{ or r15, r16, r17 ; or r5, r6, r7 ; lb r25, r26 }
    3138:	[0-9a-f]* 	{ or r15, r16, r17 ; sne r5, r6, r7 ; lb r25, r26 }
    3140:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    3148:	[0-9a-f]* 	{ or r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    3150:	[0-9a-f]* 	{ ori r15, r16, 5 ; movei r5, 5 ; lb r25, r26 }
    3158:	[0-9a-f]* 	{ ori r15, r16, 5 ; s1a r5, r6, r7 ; lb r25, r26 }
    3160:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ori r15, r16, 5 ; lb r25, r26 }
    3168:	[0-9a-f]* 	{ ori r5, r6, 5 ; rl r15, r16, r17 ; lb r25, r26 }
    3170:	[0-9a-f]* 	{ ori r5, r6, 5 ; sub r15, r16, r17 ; lb r25, r26 }
    3178:	[0-9a-f]* 	{ pcnt r5, r6 ; s1a r15, r16, r17 ; lb r25, r26 }
    3180:	[0-9a-f]* 	{ pcnt r5, r6 ; lb r25, r26 }
    3188:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; lb r25, r26 }
    3190:	[0-9a-f]* 	{ rl r15, r16, r17 ; shr r5, r6, r7 ; lb r25, r26 }
    3198:	[0-9a-f]* 	{ rl r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    31a0:	[0-9a-f]* 	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    31a8:	[0-9a-f]* 	{ bitx r5, r6 ; rli r15, r16, 5 ; lb r25, r26 }
    31b0:	[0-9a-f]* 	{ rli r15, r16, 5 ; mz r5, r6, r7 ; lb r25, r26 }
    31b8:	[0-9a-f]* 	{ rli r15, r16, 5 ; slte_u r5, r6, r7 ; lb r25, r26 }
    31c0:	[0-9a-f]* 	{ rli r5, r6, 5 ; mnz r15, r16, r17 ; lb r25, r26 }
    31c8:	[0-9a-f]* 	{ rli r5, r6, 5 ; slt_u r15, r16, r17 ; lb r25, r26 }
    31d0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; info 19 ; lb r25, r26 }
    31d8:	[0-9a-f]* 	{ pcnt r5, r6 ; s1a r15, r16, r17 ; lb r25, r26 }
    31e0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; srai r5, r6, 5 ; lb r25, r26 }
    31e8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    31f0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    31f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    3200:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
    3208:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s2a r15, r16, r17 ; lb r25, r26 }
    3210:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s1a r15, r16, r17 ; lb r25, r26 }
    3218:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lb r25, r26 }
    3220:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s3a r15, r16, r17 ; lb r25, r26 }
    3228:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shr r5, r6, r7 ; lb r25, r26 }
    3230:	[0-9a-f]* 	{ s3a r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    3238:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    3240:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; lb r25, r26 }
    3248:	[0-9a-f]* 	{ seq r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    3250:	[0-9a-f]* 	{ seq r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
    3258:	[0-9a-f]* 	{ seq r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    3260:	[0-9a-f]* 	{ seq r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    3268:	[0-9a-f]* 	{ seqi r15, r16, 5 ; info 19 ; lb r25, r26 }
    3270:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; lb r25, r26 }
    3278:	[0-9a-f]* 	{ seqi r15, r16, 5 ; srai r5, r6, 5 ; lb r25, r26 }
    3280:	[0-9a-f]* 	{ seqi r5, r6, 5 ; nor r15, r16, r17 ; lb r25, r26 }
    3288:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sne r15, r16, r17 ; lb r25, r26 }
    3290:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    3298:	[0-9a-f]* 	{ shl r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
    32a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; lb r25, r26 }
    32a8:	[0-9a-f]* 	{ shl r5, r6, r7 ; s1a r15, r16, r17 ; lb r25, r26 }
    32b0:	[0-9a-f]* 	{ shl r5, r6, r7 ; lb r25, r26 }
    32b8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; lb r25, r26 }
    32c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shr r5, r6, r7 ; lb r25, r26 }
    32c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    32d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    32d8:	[0-9a-f]* 	{ bitx r5, r6 ; shr r15, r16, r17 ; lb r25, r26 }
    32e0:	[0-9a-f]* 	{ shr r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    32e8:	[0-9a-f]* 	{ shr r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
    32f0:	[0-9a-f]* 	{ shr r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    32f8:	[0-9a-f]* 	{ shr r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    3300:	[0-9a-f]* 	{ shri r15, r16, 5 ; info 19 ; lb r25, r26 }
    3308:	[0-9a-f]* 	{ pcnt r5, r6 ; shri r15, r16, 5 ; lb r25, r26 }
    3310:	[0-9a-f]* 	{ shri r15, r16, 5 ; srai r5, r6, 5 ; lb r25, r26 }
    3318:	[0-9a-f]* 	{ shri r5, r6, 5 ; nor r15, r16, r17 ; lb r25, r26 }
    3320:	[0-9a-f]* 	{ shri r5, r6, 5 ; sne r15, r16, r17 ; lb r25, r26 }
    3328:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    3330:	[0-9a-f]* 	{ slt r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
    3338:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slt r15, r16, r17 ; lb r25, r26 }
    3340:	[0-9a-f]* 	{ slt r5, r6, r7 ; s1a r15, r16, r17 ; lb r25, r26 }
    3348:	[0-9a-f]* 	{ slt r5, r6, r7 ; lb r25, r26 }
    3350:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    3358:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shr r5, r6, r7 ; lb r25, r26 }
    3360:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    3368:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    3370:	[0-9a-f]* 	{ bitx r5, r6 ; slte r15, r16, r17 ; lb r25, r26 }
    3378:	[0-9a-f]* 	{ slte r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    3380:	[0-9a-f]* 	{ slte r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
    3388:	[0-9a-f]* 	{ slte r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    3390:	[0-9a-f]* 	{ slte r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    3398:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; info 19 ; lb r25, r26 }
    33a0:	[0-9a-f]* 	{ pcnt r5, r6 ; slte_u r15, r16, r17 ; lb r25, r26 }
    33a8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; srai r5, r6, 5 ; lb r25, r26 }
    33b0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    33b8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    33c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    33c8:	[0-9a-f]* 	{ slti r15, r16, 5 ; s3a r5, r6, r7 ; lb r25, r26 }
    33d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lb r25, r26 }
    33d8:	[0-9a-f]* 	{ slti r5, r6, 5 ; s1a r15, r16, r17 ; lb r25, r26 }
    33e0:	[0-9a-f]* 	{ slti r5, r6, 5 ; lb r25, r26 }
    33e8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lb r25, r26 }
    33f0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shr r5, r6, r7 ; lb r25, r26 }
    33f8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    3400:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    3408:	[0-9a-f]* 	{ bitx r5, r6 ; sne r15, r16, r17 ; lb r25, r26 }
    3410:	[0-9a-f]* 	{ sne r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    3418:	[0-9a-f]* 	{ sne r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
    3420:	[0-9a-f]* 	{ sne r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    3428:	[0-9a-f]* 	{ sne r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    3430:	[0-9a-f]* 	{ sra r15, r16, r17 ; info 19 ; lb r25, r26 }
    3438:	[0-9a-f]* 	{ pcnt r5, r6 ; sra r15, r16, r17 ; lb r25, r26 }
    3440:	[0-9a-f]* 	{ sra r15, r16, r17 ; srai r5, r6, 5 ; lb r25, r26 }
    3448:	[0-9a-f]* 	{ sra r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    3450:	[0-9a-f]* 	{ sra r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    3458:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    3460:	[0-9a-f]* 	{ srai r15, r16, 5 ; s3a r5, r6, r7 ; lb r25, r26 }
    3468:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; lb r25, r26 }
    3470:	[0-9a-f]* 	{ srai r5, r6, 5 ; s1a r15, r16, r17 ; lb r25, r26 }
    3478:	[0-9a-f]* 	{ srai r5, r6, 5 ; lb r25, r26 }
    3480:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    3488:	[0-9a-f]* 	{ sub r15, r16, r17 ; shr r5, r6, r7 ; lb r25, r26 }
    3490:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    3498:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    34a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lb r25, r26 }
    34a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shr r15, r16, r17 ; lb r25, r26 }
    34b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; lb r25, r26 }
    34b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; lb r25, r26 }
    34c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; lb r25, r26 }
    34c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; lb r25, r26 }
    34d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; lb r25, r26 }
    34d8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lb r25, r26 }
    34e0:	[0-9a-f]* 	{ xor r15, r16, r17 ; movei r5, 5 ; lb r25, r26 }
    34e8:	[0-9a-f]* 	{ xor r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
    34f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; lb r25, r26 }
    34f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; rl r15, r16, r17 ; lb r25, r26 }
    3500:	[0-9a-f]* 	{ xor r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    3508:	[0-9a-f]* 	{ avgh r5, r6, r7 ; lb_u r15, r16 }
    3510:	[0-9a-f]* 	{ minh r5, r6, r7 ; lb_u r15, r16 }
    3518:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; lb_u r15, r16 }
    3520:	[0-9a-f]* 	{ nor r5, r6, r7 ; lb_u r15, r16 }
    3528:	[0-9a-f]* 	{ seqb r5, r6, r7 ; lb_u r15, r16 }
    3530:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; lb_u r15, r16 }
    3538:	[0-9a-f]* 	{ srah r5, r6, r7 ; lb_u r15, r16 }
    3540:	[0-9a-f]* 	{ bitx r5, r6 ; add r15, r16, r17 ; lb_u r25, r26 }
    3548:	[0-9a-f]* 	{ add r15, r16, r17 ; mz r5, r6, r7 ; lb_u r25, r26 }
    3550:	[0-9a-f]* 	{ add r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
    3558:	[0-9a-f]* 	{ add r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    3560:	[0-9a-f]* 	{ add r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    3568:	[0-9a-f]* 	{ addi r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    3570:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
    3578:	[0-9a-f]* 	{ addi r15, r16, 5 ; srai r5, r6, 5 ; lb_u r25, r26 }
    3580:	[0-9a-f]* 	{ addi r5, r6, 5 ; nor r15, r16, r17 ; lb_u r25, r26 }
    3588:	[0-9a-f]* 	{ addi r5, r6, 5 ; sne r15, r16, r17 ; lb_u r25, r26 }
    3590:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    3598:	[0-9a-f]* 	{ and r15, r16, r17 ; s3a r5, r6, r7 ; lb_u r25, r26 }
    35a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; lb_u r25, r26 }
    35a8:	[0-9a-f]* 	{ and r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    35b0:	[0-9a-f]* 	{ and r5, r6, r7 ; lb_u r25, r26 }
    35b8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    35c0:	[0-9a-f]* 	{ andi r15, r16, 5 ; shr r5, r6, r7 ; lb_u r25, r26 }
    35c8:	[0-9a-f]* 	{ andi r5, r6, 5 ; and r15, r16, r17 ; lb_u r25, r26 }
    35d0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shl r15, r16, r17 ; lb_u r25, r26 }
    35d8:	[0-9a-f]* 	{ bitx r5, r6 ; lb_u r25, r26 }
    35e0:	[0-9a-f]* 	{ bitx r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
    35e8:	[0-9a-f]* 	{ bytex r5, r6 ; info 19 ; lb_u r25, r26 }
    35f0:	[0-9a-f]* 	{ bytex r5, r6 ; slt r15, r16, r17 ; lb_u r25, r26 }
    35f8:	[0-9a-f]* 	{ clz r5, r6 ; move r15, r16 ; lb_u r25, r26 }
    3600:	[0-9a-f]* 	{ clz r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    3608:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; lb_u r25, r26 }
    3610:	[0-9a-f]* 	{ ctz r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
    3618:	[0-9a-f]* 	{ lb_u r25, r26 }
    3620:	[0-9a-f]* 	{ mz r15, r16, r17 ; lb_u r25, r26 }
    3628:	[0-9a-f]* 	{ seq r15, r16, r17 ; lb_u r25, r26 }
    3630:	[0-9a-f]* 	{ slti r15, r16, 5 ; lb_u r25, r26 }
    3638:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; lb_u r25, r26 }
    3640:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ill ; lb_u r25, r26 }
    3648:	[0-9a-f]* 	{ slt r5, r6, r7 ; ill ; lb_u r25, r26 }
    3650:	[0-9a-f]* 	{ info 19 ; and r15, r16, r17 ; lb_u r25, r26 }
    3658:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    3660:	[0-9a-f]* 	{ info 19 ; rli r15, r16, 5 ; lb_u r25, r26 }
    3668:	[0-9a-f]* 	{ info 19 ; slt r15, r16, r17 ; lb_u r25, r26 }
    3670:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; lb_u r25, r26 }
    3678:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    3680:	[0-9a-f]* 	{ mnz r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    3688:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    3690:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    3698:	[0-9a-f]* 	{ mnz r5, r6, r7 ; xor r15, r16, r17 ; lb_u r25, r26 }
    36a0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    36a8:	[0-9a-f]* 	{ move r15, r16 ; shli r5, r6, 5 ; lb_u r25, r26 }
    36b0:	[0-9a-f]* 	{ move r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
    36b8:	[0-9a-f]* 	{ move r5, r6 ; seqi r15, r16, 5 ; lb_u r25, r26 }
    36c0:	[0-9a-f]* 	{ movei r15, 5 ; andi r5, r6, 5 ; lb_u r25, r26 }
    36c8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    36d0:	[0-9a-f]* 	{ movei r15, 5 ; slte r5, r6, r7 ; lb_u r25, r26 }
    36d8:	[0-9a-f]* 	{ movei r5, 5 ; info 19 ; lb_u r25, r26 }
    36e0:	[0-9a-f]* 	{ movei r5, 5 ; slt r15, r16, r17 ; lb_u r25, r26 }
    36e8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    36f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte r15, r16, r17 ; lb_u r25, r26 }
    36f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mz r15, r16, r17 ; lb_u r25, r26 }
    3700:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
    3708:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
    3710:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
    3718:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3720:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3728:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    3730:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; lb_u r25, r26 }
    3738:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    3740:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
    3748:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    3750:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    3758:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
    3760:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; lb_u r25, r26 }
    3768:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3770:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    3778:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slt r15, r16, r17 ; lb_u r25, r26 }
    3780:	[0-9a-f]* 	{ mvz r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    3788:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slte r15, r16, r17 ; lb_u r25, r26 }
    3790:	[0-9a-f]* 	{ mz r15, r16, r17 ; mnz r5, r6, r7 ; lb_u r25, r26 }
    3798:	[0-9a-f]* 	{ mz r15, r16, r17 ; rl r5, r6, r7 ; lb_u r25, r26 }
    37a0:	[0-9a-f]* 	{ mz r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    37a8:	[0-9a-f]* 	{ mz r5, r6, r7 ; or r15, r16, r17 ; lb_u r25, r26 }
    37b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; sra r15, r16, r17 ; lb_u r25, r26 }
    37b8:	[0-9a-f]* 	{ nop ; mnz r15, r16, r17 ; lb_u r25, r26 }
    37c0:	[0-9a-f]* 	{ nop ; nor r15, r16, r17 ; lb_u r25, r26 }
    37c8:	[0-9a-f]* 	{ nop ; seqi r5, r6, 5 ; lb_u r25, r26 }
    37d0:	[0-9a-f]* 	{ nop ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    37d8:	[0-9a-f]* 	{ bitx r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    37e0:	[0-9a-f]* 	{ nor r15, r16, r17 ; mz r5, r6, r7 ; lb_u r25, r26 }
    37e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
    37f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    37f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    3800:	[0-9a-f]* 	{ or r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    3808:	[0-9a-f]* 	{ pcnt r5, r6 ; or r15, r16, r17 ; lb_u r25, r26 }
    3810:	[0-9a-f]* 	{ or r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    3818:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
    3820:	[0-9a-f]* 	{ or r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
    3828:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3830:	[0-9a-f]* 	{ ori r15, r16, 5 ; s3a r5, r6, r7 ; lb_u r25, r26 }
    3838:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3840:	[0-9a-f]* 	{ ori r5, r6, 5 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    3848:	[0-9a-f]* 	{ ori r5, r6, 5 ; lb_u r25, r26 }
    3850:	[0-9a-f]* 	{ pcnt r5, r6 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3858:	[0-9a-f]* 	{ rl r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    3860:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    3868:	[0-9a-f]* 	{ rl r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    3870:	[0-9a-f]* 	{ rl r5, r6, r7 ; lb_u r25, r26 }
    3878:	[0-9a-f]* 	{ rl r5, r6, r7 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3880:	[0-9a-f]* 	{ clz r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
    3888:	[0-9a-f]* 	{ rli r15, r16, 5 ; nor r5, r6, r7 ; lb_u r25, r26 }
    3890:	[0-9a-f]* 	{ rli r15, r16, 5 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    3898:	[0-9a-f]* 	{ rli r5, r6, 5 ; movei r15, 5 ; lb_u r25, r26 }
    38a0:	[0-9a-f]* 	{ rli r5, r6, 5 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    38a8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; move r5, r6 ; lb_u r25, r26 }
    38b0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    38b8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    38c0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    38c8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    38d0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    38d8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    38e0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; lb_u r25, r26 }
    38e8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    38f0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    38f8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3900:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    3908:	[0-9a-f]* 	{ s3a r5, r6, r7 ; lb_u r25, r26 }
    3910:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3918:	[0-9a-f]* 	{ clz r5, r6 ; seq r15, r16, r17 ; lb_u r25, r26 }
    3920:	[0-9a-f]* 	{ seq r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    3928:	[0-9a-f]* 	{ seq r15, r16, r17 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    3930:	[0-9a-f]* 	{ seq r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    3938:	[0-9a-f]* 	{ seq r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    3940:	[0-9a-f]* 	{ seqi r15, r16, 5 ; move r5, r6 ; lb_u r25, r26 }
    3948:	[0-9a-f]* 	{ seqi r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    3950:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; seqi r15, r16, 5 ; lb_u r25, r26 }
    3958:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3960:	[0-9a-f]* 	{ seqi r5, r6, 5 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3968:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
    3970:	[0-9a-f]* 	{ shl r15, r16, r17 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    3978:	[0-9a-f]* 	{ shl r15, r16, r17 ; lb_u r25, r26 }
    3980:	[0-9a-f]* 	{ shl r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3988:	[0-9a-f]* 	{ shli r15, r16, 5 ; addi r5, r6, 5 ; lb_u r25, r26 }
    3990:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    3998:	[0-9a-f]* 	{ shli r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    39a0:	[0-9a-f]* 	{ shli r5, r6, 5 ; lb_u r25, r26 }
    39a8:	[0-9a-f]* 	{ shli r5, r6, 5 ; shr r15, r16, r17 ; lb_u r25, r26 }
    39b0:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
    39b8:	[0-9a-f]* 	{ shr r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    39c0:	[0-9a-f]* 	{ shr r15, r16, r17 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    39c8:	[0-9a-f]* 	{ shr r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    39d0:	[0-9a-f]* 	{ shr r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    39d8:	[0-9a-f]* 	{ shri r15, r16, 5 ; move r5, r6 ; lb_u r25, r26 }
    39e0:	[0-9a-f]* 	{ shri r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    39e8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shri r15, r16, 5 ; lb_u r25, r26 }
    39f0:	[0-9a-f]* 	{ shri r5, r6, 5 ; ori r15, r16, 5 ; lb_u r25, r26 }
    39f8:	[0-9a-f]* 	{ shri r5, r6, 5 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3a00:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slt r15, r16, r17 ; lb_u r25, r26 }
    3a08:	[0-9a-f]* 	{ slt r15, r16, r17 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    3a10:	[0-9a-f]* 	{ slt r15, r16, r17 ; lb_u r25, r26 }
    3a18:	[0-9a-f]* 	{ slt r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3a20:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    3a28:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    3a30:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    3a38:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; lb_u r25, r26 }
    3a40:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3a48:	[0-9a-f]* 	{ clz r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    3a50:	[0-9a-f]* 	{ slte r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    3a58:	[0-9a-f]* 	{ slte r15, r16, r17 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    3a60:	[0-9a-f]* 	{ slte r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    3a68:	[0-9a-f]* 	{ slte r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    3a70:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; move r5, r6 ; lb_u r25, r26 }
    3a78:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    3a80:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    3a88:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3a90:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3a98:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
    3aa0:	[0-9a-f]* 	{ slti r15, r16, 5 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    3aa8:	[0-9a-f]* 	{ slti r15, r16, 5 ; lb_u r25, r26 }
    3ab0:	[0-9a-f]* 	{ slti r5, r6, 5 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3ab8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; addi r5, r6, 5 ; lb_u r25, r26 }
    3ac0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    3ac8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    3ad0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; lb_u r25, r26 }
    3ad8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3ae0:	[0-9a-f]* 	{ clz r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
    3ae8:	[0-9a-f]* 	{ sne r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    3af0:	[0-9a-f]* 	{ sne r15, r16, r17 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    3af8:	[0-9a-f]* 	{ sne r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    3b00:	[0-9a-f]* 	{ sne r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    3b08:	[0-9a-f]* 	{ sra r15, r16, r17 ; move r5, r6 ; lb_u r25, r26 }
    3b10:	[0-9a-f]* 	{ sra r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    3b18:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sra r15, r16, r17 ; lb_u r25, r26 }
    3b20:	[0-9a-f]* 	{ sra r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    3b28:	[0-9a-f]* 	{ sra r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3b30:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    3b38:	[0-9a-f]* 	{ srai r15, r16, 5 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    3b40:	[0-9a-f]* 	{ srai r15, r16, 5 ; lb_u r25, r26 }
    3b48:	[0-9a-f]* 	{ srai r5, r6, 5 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    3b50:	[0-9a-f]* 	{ sub r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    3b58:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    3b60:	[0-9a-f]* 	{ sub r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    3b68:	[0-9a-f]* 	{ sub r5, r6, r7 ; lb_u r25, r26 }
    3b70:	[0-9a-f]* 	{ sub r5, r6, r7 ; shr r15, r16, r17 ; lb_u r25, r26 }
    3b78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; info 19 ; lb_u r25, r26 }
    3b80:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slt r15, r16, r17 ; lb_u r25, r26 }
    3b88:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; lb_u r25, r26 }
    3b90:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    3b98:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; lb_u r25, r26 }
    3ba0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
    3ba8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    3bb0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
    3bb8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; lb_u r25, r26 }
    3bc0:	[0-9a-f]* 	{ xor r15, r16, r17 ; s3a r5, r6, r7 ; lb_u r25, r26 }
    3bc8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lb_u r25, r26 }
    3bd0:	[0-9a-f]* 	{ xor r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    3bd8:	[0-9a-f]* 	{ xor r5, r6, r7 ; lb_u r25, r26 }
    3be0:	[0-9a-f]* 	{ bytex r5, r6 ; lbadd r15, r16, 5 }
    3be8:	[0-9a-f]* 	{ minih r5, r6, 5 ; lbadd r15, r16, 5 }
    3bf0:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; lbadd r15, r16, 5 }
    3bf8:	[0-9a-f]* 	{ ori r5, r6, 5 ; lbadd r15, r16, 5 }
    3c00:	[0-9a-f]* 	{ seqi r5, r6, 5 ; lbadd r15, r16, 5 }
    3c08:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; lbadd r15, r16, 5 }
    3c10:	[0-9a-f]* 	{ sraib r5, r6, 5 ; lbadd r15, r16, 5 }
    3c18:	[0-9a-f]* 	{ addib r5, r6, 5 ; lbadd_u r15, r16, 5 }
    3c20:	[0-9a-f]* 	{ info 19 ; lbadd_u r15, r16, 5 }
    3c28:	[0-9a-f]* 	{ moveli r5, 4660 ; lbadd_u r15, r16, 5 }
    3c30:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; lbadd_u r15, r16, 5 }
    3c38:	[0-9a-f]* 	{ rli r5, r6, 5 ; lbadd_u r15, r16, 5 }
    3c40:	[0-9a-f]* 	{ shlib r5, r6, 5 ; lbadd_u r15, r16, 5 }
    3c48:	[0-9a-f]* 	{ slti r5, r6, 5 ; lbadd_u r15, r16, 5 }
    3c50:	[0-9a-f]* 	{ subs r5, r6, r7 ; lbadd_u r15, r16, 5 }
    3c58:	[0-9a-f]* 	{ and r5, r6, r7 ; lh r15, r16 }
    3c60:	[0-9a-f]* 	{ maxh r5, r6, r7 ; lh r15, r16 }
    3c68:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lh r15, r16 }
    3c70:	[0-9a-f]* 	{ mz r5, r6, r7 ; lh r15, r16 }
    3c78:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; lh r15, r16 }
    3c80:	[0-9a-f]* 	{ shrih r5, r6, 5 ; lh r15, r16 }
    3c88:	[0-9a-f]* 	{ sneb r5, r6, r7 ; lh r15, r16 }
    3c90:	[0-9a-f]* 	{ add r15, r16, r17 ; add r5, r6, r7 ; lh r25, r26 }
    3c98:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    3ca0:	[0-9a-f]* 	{ add r15, r16, r17 ; shri r5, r6, 5 ; lh r25, r26 }
    3ca8:	[0-9a-f]* 	{ add r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
    3cb0:	[0-9a-f]* 	{ add r5, r6, r7 ; shli r15, r16, 5 ; lh r25, r26 }
    3cb8:	[0-9a-f]* 	{ bytex r5, r6 ; addi r15, r16, 5 ; lh r25, r26 }
    3cc0:	[0-9a-f]* 	{ addi r15, r16, 5 ; nop ; lh r25, r26 }
    3cc8:	[0-9a-f]* 	{ addi r15, r16, 5 ; slti r5, r6, 5 ; lh r25, r26 }
    3cd0:	[0-9a-f]* 	{ addi r5, r6, 5 ; move r15, r16 ; lh r25, r26 }
    3cd8:	[0-9a-f]* 	{ addi r5, r6, 5 ; slte r15, r16, r17 ; lh r25, r26 }
    3ce0:	[0-9a-f]* 	{ and r15, r16, r17 ; mnz r5, r6, r7 ; lh r25, r26 }
    3ce8:	[0-9a-f]* 	{ and r15, r16, r17 ; rl r5, r6, r7 ; lh r25, r26 }
    3cf0:	[0-9a-f]* 	{ and r15, r16, r17 ; sub r5, r6, r7 ; lh r25, r26 }
    3cf8:	[0-9a-f]* 	{ and r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    3d00:	[0-9a-f]* 	{ and r5, r6, r7 ; sra r15, r16, r17 ; lh r25, r26 }
    3d08:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
    3d10:	[0-9a-f]* 	{ andi r15, r16, 5 ; seq r5, r6, r7 ; lh r25, r26 }
    3d18:	[0-9a-f]* 	{ andi r15, r16, 5 ; xor r5, r6, r7 ; lh r25, r26 }
    3d20:	[0-9a-f]* 	{ andi r5, r6, 5 ; s2a r15, r16, r17 ; lh r25, r26 }
    3d28:	[0-9a-f]* 	{ bitx r5, r6 ; add r15, r16, r17 ; lh r25, r26 }
    3d30:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; lh r25, r26 }
    3d38:	[0-9a-f]* 	{ bytex r5, r6 ; and r15, r16, r17 ; lh r25, r26 }
    3d40:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    3d48:	[0-9a-f]* 	{ clz r5, r6 ; lh r25, r26 }
    3d50:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lh r25, r26 }
    3d58:	[0-9a-f]* 	{ ctz r5, r6 ; info 19 ; lh r25, r26 }
    3d60:	[0-9a-f]* 	{ ctz r5, r6 ; slt r15, r16, r17 ; lh r25, r26 }
    3d68:	[0-9a-f]* 	{ bitx r5, r6 ; lh r25, r26 }
    3d70:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; lh r25, r26 }
    3d78:	[0-9a-f]* 	{ s2a r15, r16, r17 ; lh r25, r26 }
    3d80:	[0-9a-f]* 	{ slte r15, r16, r17 ; lh r25, r26 }
    3d88:	[0-9a-f]* 	{ xor r15, r16, r17 ; lh r25, r26 }
    3d90:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ill ; lh r25, r26 }
    3d98:	[0-9a-f]* 	{ shl r5, r6, r7 ; ill ; lh r25, r26 }
    3da0:	[0-9a-f]* 	{ info 19 ; add r15, r16, r17 ; lh r25, r26 }
    3da8:	[0-9a-f]* 	{ info 19 ; movei r5, 5 ; lh r25, r26 }
    3db0:	[0-9a-f]* 	{ info 19 ; ori r5, r6, 5 ; lh r25, r26 }
    3db8:	[0-9a-f]* 	{ info 19 ; shr r15, r16, r17 ; lh r25, r26 }
    3dc0:	[0-9a-f]* 	{ info 19 ; srai r15, r16, 5 ; lh r25, r26 }
    3dc8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; info 19 ; lh r25, r26 }
    3dd0:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; lh r25, r26 }
    3dd8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; srai r5, r6, 5 ; lh r25, r26 }
    3de0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; nor r15, r16, r17 ; lh r25, r26 }
    3de8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; sne r15, r16, r17 ; lh r25, r26 }
    3df0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    3df8:	[0-9a-f]* 	{ move r15, r16 ; s3a r5, r6, r7 ; lh r25, r26 }
    3e00:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; lh r25, r26 }
    3e08:	[0-9a-f]* 	{ move r5, r6 ; s1a r15, r16, r17 ; lh r25, r26 }
    3e10:	[0-9a-f]* 	{ move r5, r6 ; lh r25, r26 }
    3e18:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
    3e20:	[0-9a-f]* 	{ movei r15, 5 ; shr r5, r6, r7 ; lh r25, r26 }
    3e28:	[0-9a-f]* 	{ movei r5, 5 ; and r15, r16, r17 ; lh r25, r26 }
    3e30:	[0-9a-f]* 	{ movei r5, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    3e38:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lh r25, r26 }
    3e40:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shr r15, r16, r17 ; lh r25, r26 }
    3e48:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; info 19 ; lh r25, r26 }
    3e50:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
    3e58:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    3e60:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    3e68:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    3e70:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    3e78:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; lh r25, r26 }
    3e80:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sne r15, r16, r17 ; lh r25, r26 }
    3e88:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    3e90:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srai r15, r16, 5 ; lh r25, r26 }
    3e98:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    3ea0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
    3ea8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    3eb0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    3eb8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    3ec0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    3ec8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    3ed0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; lh r25, r26 }
    3ed8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shr r15, r16, r17 ; lh r25, r26 }
    3ee0:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; lh r25, r26 }
    3ee8:	[0-9a-f]* 	{ mz r15, r16, r17 ; nor r5, r6, r7 ; lh r25, r26 }
    3ef0:	[0-9a-f]* 	{ mz r15, r16, r17 ; slti_u r5, r6, 5 ; lh r25, r26 }
    3ef8:	[0-9a-f]* 	{ mz r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
    3f00:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte_u r15, r16, r17 ; lh r25, r26 }
    3f08:	[0-9a-f]* 	{ ctz r5, r6 ; nop ; lh r25, r26 }
    3f10:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nop ; lh r25, r26 }
    3f18:	[0-9a-f]* 	{ nop ; s3a r5, r6, r7 ; lh r25, r26 }
    3f20:	[0-9a-f]* 	{ nop ; slte_u r5, r6, r7 ; lh r25, r26 }
    3f28:	[0-9a-f]* 	{ nor r15, r16, r17 ; add r5, r6, r7 ; lh r25, r26 }
    3f30:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; nor r15, r16, r17 ; lh r25, r26 }
    3f38:	[0-9a-f]* 	{ nor r15, r16, r17 ; shri r5, r6, 5 ; lh r25, r26 }
    3f40:	[0-9a-f]* 	{ nor r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
    3f48:	[0-9a-f]* 	{ nor r5, r6, r7 ; shli r15, r16, 5 ; lh r25, r26 }
    3f50:	[0-9a-f]* 	{ bytex r5, r6 ; or r15, r16, r17 ; lh r25, r26 }
    3f58:	[0-9a-f]* 	{ or r15, r16, r17 ; nop ; lh r25, r26 }
    3f60:	[0-9a-f]* 	{ or r15, r16, r17 ; slti r5, r6, 5 ; lh r25, r26 }
    3f68:	[0-9a-f]* 	{ or r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    3f70:	[0-9a-f]* 	{ or r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    3f78:	[0-9a-f]* 	{ ori r15, r16, 5 ; mnz r5, r6, r7 ; lh r25, r26 }
    3f80:	[0-9a-f]* 	{ ori r15, r16, 5 ; rl r5, r6, r7 ; lh r25, r26 }
    3f88:	[0-9a-f]* 	{ ori r15, r16, 5 ; sub r5, r6, r7 ; lh r25, r26 }
    3f90:	[0-9a-f]* 	{ ori r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    3f98:	[0-9a-f]* 	{ ori r5, r6, 5 ; sra r15, r16, r17 ; lh r25, r26 }
    3fa0:	[0-9a-f]* 	{ pcnt r5, r6 ; rl r15, r16, r17 ; lh r25, r26 }
    3fa8:	[0-9a-f]* 	{ pcnt r5, r6 ; sub r15, r16, r17 ; lh r25, r26 }
    3fb0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    3fb8:	[0-9a-f]* 	{ rl r15, r16, r17 ; shl r5, r6, r7 ; lh r25, r26 }
    3fc0:	[0-9a-f]* 	{ rl r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    3fc8:	[0-9a-f]* 	{ rl r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    3fd0:	[0-9a-f]* 	{ rli r15, r16, 5 ; and r5, r6, r7 ; lh r25, r26 }
    3fd8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    3fe0:	[0-9a-f]* 	{ rli r15, r16, 5 ; slt_u r5, r6, r7 ; lh r25, r26 }
    3fe8:	[0-9a-f]* 	{ rli r5, r6, 5 ; ill ; lh r25, r26 }
    3ff0:	[0-9a-f]* 	{ rli r5, r6, 5 ; shri r15, r16, 5 ; lh r25, r26 }
    3ff8:	[0-9a-f]* 	{ ctz r5, r6 ; s1a r15, r16, r17 ; lh r25, r26 }
    4000:	[0-9a-f]* 	{ s1a r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    4008:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sne r5, r6, r7 ; lh r25, r26 }
    4010:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    4018:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    4020:	[0-9a-f]* 	{ s2a r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    4028:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    4030:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s2a r15, r16, r17 ; lh r25, r26 }
    4038:	[0-9a-f]* 	{ s2a r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    4040:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    4048:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    4050:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shl r5, r6, r7 ; lh r25, r26 }
    4058:	[0-9a-f]* 	{ s3a r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    4060:	[0-9a-f]* 	{ s3a r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    4068:	[0-9a-f]* 	{ seq r15, r16, r17 ; and r5, r6, r7 ; lh r25, r26 }
    4070:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    4078:	[0-9a-f]* 	{ seq r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
    4080:	[0-9a-f]* 	{ seq r5, r6, r7 ; ill ; lh r25, r26 }
    4088:	[0-9a-f]* 	{ seq r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    4090:	[0-9a-f]* 	{ ctz r5, r6 ; seqi r15, r16, 5 ; lh r25, r26 }
    4098:	[0-9a-f]* 	{ seqi r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    40a0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; sne r5, r6, r7 ; lh r25, r26 }
    40a8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; mz r15, r16, r17 ; lh r25, r26 }
    40b0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slti r15, r16, 5 ; lh r25, r26 }
    40b8:	[0-9a-f]* 	{ shl r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    40c0:	[0-9a-f]* 	{ shl r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    40c8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    40d0:	[0-9a-f]* 	{ shl r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    40d8:	[0-9a-f]* 	{ shl r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    40e0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shli r15, r16, 5 ; lh r25, r26 }
    40e8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shl r5, r6, r7 ; lh r25, r26 }
    40f0:	[0-9a-f]* 	{ shli r5, r6, 5 ; add r15, r16, r17 ; lh r25, r26 }
    40f8:	[0-9a-f]* 	{ shli r5, r6, 5 ; seq r15, r16, r17 ; lh r25, r26 }
    4100:	[0-9a-f]* 	{ shr r15, r16, r17 ; and r5, r6, r7 ; lh r25, r26 }
    4108:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shr r15, r16, r17 ; lh r25, r26 }
    4110:	[0-9a-f]* 	{ shr r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
    4118:	[0-9a-f]* 	{ shr r5, r6, r7 ; ill ; lh r25, r26 }
    4120:	[0-9a-f]* 	{ shr r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    4128:	[0-9a-f]* 	{ ctz r5, r6 ; shri r15, r16, 5 ; lh r25, r26 }
    4130:	[0-9a-f]* 	{ shri r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    4138:	[0-9a-f]* 	{ shri r15, r16, 5 ; sne r5, r6, r7 ; lh r25, r26 }
    4140:	[0-9a-f]* 	{ shri r5, r6, 5 ; mz r15, r16, r17 ; lh r25, r26 }
    4148:	[0-9a-f]* 	{ shri r5, r6, 5 ; slti r15, r16, 5 ; lh r25, r26 }
    4150:	[0-9a-f]* 	{ slt r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    4158:	[0-9a-f]* 	{ slt r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    4160:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; lh r25, r26 }
    4168:	[0-9a-f]* 	{ slt r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    4170:	[0-9a-f]* 	{ slt r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    4178:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
    4180:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shl r5, r6, r7 ; lh r25, r26 }
    4188:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    4190:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    4198:	[0-9a-f]* 	{ slte r15, r16, r17 ; and r5, r6, r7 ; lh r25, r26 }
    41a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    41a8:	[0-9a-f]* 	{ slte r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
    41b0:	[0-9a-f]* 	{ slte r5, r6, r7 ; ill ; lh r25, r26 }
    41b8:	[0-9a-f]* 	{ slte r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    41c0:	[0-9a-f]* 	{ ctz r5, r6 ; slte_u r15, r16, r17 ; lh r25, r26 }
    41c8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    41d0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sne r5, r6, r7 ; lh r25, r26 }
    41d8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    41e0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    41e8:	[0-9a-f]* 	{ slti r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    41f0:	[0-9a-f]* 	{ slti r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    41f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti r15, r16, 5 ; lh r25, r26 }
    4200:	[0-9a-f]* 	{ slti r5, r6, 5 ; rl r15, r16, r17 ; lh r25, r26 }
    4208:	[0-9a-f]* 	{ slti r5, r6, 5 ; sub r15, r16, r17 ; lh r25, r26 }
    4210:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
    4218:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shl r5, r6, r7 ; lh r25, r26 }
    4220:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; add r15, r16, r17 ; lh r25, r26 }
    4228:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; seq r15, r16, r17 ; lh r25, r26 }
    4230:	[0-9a-f]* 	{ sne r15, r16, r17 ; and r5, r6, r7 ; lh r25, r26 }
    4238:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sne r15, r16, r17 ; lh r25, r26 }
    4240:	[0-9a-f]* 	{ sne r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
    4248:	[0-9a-f]* 	{ sne r5, r6, r7 ; ill ; lh r25, r26 }
    4250:	[0-9a-f]* 	{ sne r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    4258:	[0-9a-f]* 	{ ctz r5, r6 ; sra r15, r16, r17 ; lh r25, r26 }
    4260:	[0-9a-f]* 	{ sra r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    4268:	[0-9a-f]* 	{ sra r15, r16, r17 ; sne r5, r6, r7 ; lh r25, r26 }
    4270:	[0-9a-f]* 	{ sra r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    4278:	[0-9a-f]* 	{ sra r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    4280:	[0-9a-f]* 	{ srai r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    4288:	[0-9a-f]* 	{ srai r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    4290:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; srai r15, r16, 5 ; lh r25, r26 }
    4298:	[0-9a-f]* 	{ srai r5, r6, 5 ; rl r15, r16, r17 ; lh r25, r26 }
    42a0:	[0-9a-f]* 	{ srai r5, r6, 5 ; sub r15, r16, r17 ; lh r25, r26 }
    42a8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    42b0:	[0-9a-f]* 	{ sub r15, r16, r17 ; shl r5, r6, r7 ; lh r25, r26 }
    42b8:	[0-9a-f]* 	{ sub r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    42c0:	[0-9a-f]* 	{ sub r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    42c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; lh r25, r26 }
    42d0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    42d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; lh r25, r26 }
    42e0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shr r15, r16, r17 ; lh r25, r26 }
    42e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; info 19 ; lh r25, r26 }
    42f0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slt r15, r16, r17 ; lh r25, r26 }
    42f8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; lh r25, r26 }
    4300:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slte r15, r16, r17 ; lh r25, r26 }
    4308:	[0-9a-f]* 	{ xor r15, r16, r17 ; mnz r5, r6, r7 ; lh r25, r26 }
    4310:	[0-9a-f]* 	{ xor r15, r16, r17 ; rl r5, r6, r7 ; lh r25, r26 }
    4318:	[0-9a-f]* 	{ xor r15, r16, r17 ; sub r5, r6, r7 ; lh r25, r26 }
    4320:	[0-9a-f]* 	{ xor r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    4328:	[0-9a-f]* 	{ xor r5, r6, r7 ; sra r15, r16, r17 ; lh r25, r26 }
    4330:	[0-9a-f]* 	{ auli r5, r6, 4660 ; lh_u r15, r16 }
    4338:	[0-9a-f]* 	{ maxih r5, r6, 5 ; lh_u r15, r16 }
    4340:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; lh_u r15, r16 }
    4348:	[0-9a-f]* 	{ mzh r5, r6, r7 ; lh_u r15, r16 }
    4350:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; lh_u r15, r16 }
    4358:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; lh_u r15, r16 }
    4360:	[0-9a-f]* 	{ sra r5, r6, r7 ; lh_u r15, r16 }
    4368:	[0-9a-f]* 	{ add r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
    4370:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; add r15, r16, r17 ; lh_u r25, r26 }
    4378:	[0-9a-f]* 	{ add r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
    4380:	[0-9a-f]* 	{ add r5, r6, r7 ; ill ; lh_u r25, r26 }
    4388:	[0-9a-f]* 	{ add r5, r6, r7 ; shri r15, r16, 5 ; lh_u r25, r26 }
    4390:	[0-9a-f]* 	{ ctz r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
    4398:	[0-9a-f]* 	{ addi r15, r16, 5 ; or r5, r6, r7 ; lh_u r25, r26 }
    43a0:	[0-9a-f]* 	{ addi r15, r16, 5 ; sne r5, r6, r7 ; lh_u r25, r26 }
    43a8:	[0-9a-f]* 	{ addi r5, r6, 5 ; mz r15, r16, r17 ; lh_u r25, r26 }
    43b0:	[0-9a-f]* 	{ addi r5, r6, 5 ; slti r15, r16, 5 ; lh_u r25, r26 }
    43b8:	[0-9a-f]* 	{ and r15, r16, r17 ; movei r5, 5 ; lh_u r25, r26 }
    43c0:	[0-9a-f]* 	{ and r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    43c8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; and r15, r16, r17 ; lh_u r25, r26 }
    43d0:	[0-9a-f]* 	{ and r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    43d8:	[0-9a-f]* 	{ and r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    43e0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    43e8:	[0-9a-f]* 	{ andi r15, r16, 5 ; shl r5, r6, r7 ; lh_u r25, r26 }
    43f0:	[0-9a-f]* 	{ andi r5, r6, 5 ; add r15, r16, r17 ; lh_u r25, r26 }
    43f8:	[0-9a-f]* 	{ andi r5, r6, 5 ; seq r15, r16, r17 ; lh_u r25, r26 }
    4400:	[0-9a-f]* 	{ bitx r5, r6 ; and r15, r16, r17 ; lh_u r25, r26 }
    4408:	[0-9a-f]* 	{ bitx r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4410:	[0-9a-f]* 	{ bytex r5, r6 ; lh_u r25, r26 }
    4418:	[0-9a-f]* 	{ bytex r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
    4420:	[0-9a-f]* 	{ clz r5, r6 ; info 19 ; lh_u r25, r26 }
    4428:	[0-9a-f]* 	{ clz r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
    4430:	[0-9a-f]* 	{ ctz r5, r6 ; move r15, r16 ; lh_u r25, r26 }
    4438:	[0-9a-f]* 	{ ctz r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
    4440:	[0-9a-f]* 	{ clz r5, r6 ; lh_u r25, r26 }
    4448:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; lh_u r25, r26 }
    4450:	[0-9a-f]* 	{ s3a r15, r16, r17 ; lh_u r25, r26 }
    4458:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; lh_u r25, r26 }
    4460:	[0-9a-f]* 	{ lh_u r25, r26 }
    4468:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ill ; lh_u r25, r26 }
    4470:	[0-9a-f]* 	{ shr r5, r6, r7 ; ill ; lh_u r25, r26 }
    4478:	[0-9a-f]* 	{ info 19 ; addi r15, r16, 5 ; lh_u r25, r26 }
    4480:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    4488:	[0-9a-f]* 	{ info 19 ; rl r15, r16, r17 ; lh_u r25, r26 }
    4490:	[0-9a-f]* 	{ info 19 ; shri r15, r16, 5 ; lh_u r25, r26 }
    4498:	[0-9a-f]* 	{ info 19 ; sub r15, r16, r17 ; lh_u r25, r26 }
    44a0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; move r5, r6 ; lh_u r25, r26 }
    44a8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rli r5, r6, 5 ; lh_u r25, r26 }
    44b0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    44b8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    44c0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    44c8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
    44d0:	[0-9a-f]* 	{ move r15, r16 ; seqi r5, r6, 5 ; lh_u r25, r26 }
    44d8:	[0-9a-f]* 	{ move r15, r16 ; lh_u r25, r26 }
    44e0:	[0-9a-f]* 	{ move r5, r6 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    44e8:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 ; lh_u r25, r26 }
    44f0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    44f8:	[0-9a-f]* 	{ movei r15, 5 ; slt r5, r6, r7 ; lh_u r25, r26 }
    4500:	[0-9a-f]* 	{ movei r5, 5 ; lh_u r25, r26 }
    4508:	[0-9a-f]* 	{ movei r5, 5 ; shr r15, r16, r17 ; lh_u r25, r26 }
    4510:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    4518:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slt r15, r16, r17 ; lh_u r25, r26 }
    4520:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
    4528:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slte r15, r16, r17 ; lh_u r25, r26 }
    4530:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    4538:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
    4540:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    4548:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    4550:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    4558:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    4560:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lh_u r25, r26 }
    4568:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    4570:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    4578:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; add r15, r16, r17 ; lh_u r25, r26 }
    4580:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; lh_u r25, r26 }
    4588:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    4590:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4598:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; lh_u r25, r26 }
    45a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shr r15, r16, r17 ; lh_u r25, r26 }
    45a8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    45b0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt r15, r16, r17 ; lh_u r25, r26 }
    45b8:	[0-9a-f]* 	{ mz r15, r16, r17 ; lh_u r25, r26 }
    45c0:	[0-9a-f]* 	{ mz r15, r16, r17 ; ori r5, r6, 5 ; lh_u r25, r26 }
    45c8:	[0-9a-f]* 	{ mz r15, r16, r17 ; sra r5, r6, r7 ; lh_u r25, r26 }
    45d0:	[0-9a-f]* 	{ mz r5, r6, r7 ; nop ; lh_u r25, r26 }
    45d8:	[0-9a-f]* 	{ mz r5, r6, r7 ; slti_u r15, r16, 5 ; lh_u r25, r26 }
    45e0:	[0-9a-f]* 	{ nop ; ill ; lh_u r25, r26 }
    45e8:	[0-9a-f]* 	{ nop ; mz r5, r6, r7 ; lh_u r25, r26 }
    45f0:	[0-9a-f]* 	{ nop ; seq r5, r6, r7 ; lh_u r25, r26 }
    45f8:	[0-9a-f]* 	{ nop ; slti r5, r6, 5 ; lh_u r25, r26 }
    4600:	[0-9a-f]* 	{ nor r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
    4608:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    4610:	[0-9a-f]* 	{ nor r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
    4618:	[0-9a-f]* 	{ nor r5, r6, r7 ; ill ; lh_u r25, r26 }
    4620:	[0-9a-f]* 	{ nor r5, r6, r7 ; shri r15, r16, 5 ; lh_u r25, r26 }
    4628:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
    4630:	[0-9a-f]* 	{ or r15, r16, r17 ; or r5, r6, r7 ; lh_u r25, r26 }
    4638:	[0-9a-f]* 	{ or r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
    4640:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    4648:	[0-9a-f]* 	{ or r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
    4650:	[0-9a-f]* 	{ ori r15, r16, 5 ; movei r5, 5 ; lh_u r25, r26 }
    4658:	[0-9a-f]* 	{ ori r15, r16, 5 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    4660:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ori r15, r16, 5 ; lh_u r25, r26 }
    4668:	[0-9a-f]* 	{ ori r5, r6, 5 ; rl r15, r16, r17 ; lh_u r25, r26 }
    4670:	[0-9a-f]* 	{ ori r5, r6, 5 ; sub r15, r16, r17 ; lh_u r25, r26 }
    4678:	[0-9a-f]* 	{ pcnt r5, r6 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    4680:	[0-9a-f]* 	{ pcnt r5, r6 ; lh_u r25, r26 }
    4688:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    4690:	[0-9a-f]* 	{ rl r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    4698:	[0-9a-f]* 	{ rl r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    46a0:	[0-9a-f]* 	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    46a8:	[0-9a-f]* 	{ bitx r5, r6 ; rli r15, r16, 5 ; lh_u r25, r26 }
    46b0:	[0-9a-f]* 	{ rli r15, r16, 5 ; mz r5, r6, r7 ; lh_u r25, r26 }
    46b8:	[0-9a-f]* 	{ rli r15, r16, 5 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    46c0:	[0-9a-f]* 	{ rli r5, r6, 5 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    46c8:	[0-9a-f]* 	{ rli r5, r6, 5 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    46d0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; info 19 ; lh_u r25, r26 }
    46d8:	[0-9a-f]* 	{ pcnt r5, r6 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    46e0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; srai r5, r6, 5 ; lh_u r25, r26 }
    46e8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    46f0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    46f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    4700:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    4708:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    4710:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    4718:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lh_u r25, r26 }
    4720:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    4728:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    4730:	[0-9a-f]* 	{ s3a r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    4738:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4740:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; lh_u r25, r26 }
    4748:	[0-9a-f]* 	{ seq r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    4750:	[0-9a-f]* 	{ seq r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    4758:	[0-9a-f]* 	{ seq r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    4760:	[0-9a-f]* 	{ seq r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    4768:	[0-9a-f]* 	{ seqi r15, r16, 5 ; info 19 ; lh_u r25, r26 }
    4770:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; lh_u r25, r26 }
    4778:	[0-9a-f]* 	{ seqi r15, r16, 5 ; srai r5, r6, 5 ; lh_u r25, r26 }
    4780:	[0-9a-f]* 	{ seqi r5, r6, 5 ; nor r15, r16, r17 ; lh_u r25, r26 }
    4788:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sne r15, r16, r17 ; lh_u r25, r26 }
    4790:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4798:	[0-9a-f]* 	{ shl r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    47a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    47a8:	[0-9a-f]* 	{ shl r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    47b0:	[0-9a-f]* 	{ shl r5, r6, r7 ; lh_u r25, r26 }
    47b8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    47c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shr r5, r6, r7 ; lh_u r25, r26 }
    47c8:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 ; lh_u r25, r26 }
    47d0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lh_u r25, r26 }
    47d8:	[0-9a-f]* 	{ bitx r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
    47e0:	[0-9a-f]* 	{ shr r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    47e8:	[0-9a-f]* 	{ shr r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    47f0:	[0-9a-f]* 	{ shr r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    47f8:	[0-9a-f]* 	{ shr r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    4800:	[0-9a-f]* 	{ shri r15, r16, 5 ; info 19 ; lh_u r25, r26 }
    4808:	[0-9a-f]* 	{ pcnt r5, r6 ; shri r15, r16, 5 ; lh_u r25, r26 }
    4810:	[0-9a-f]* 	{ shri r15, r16, 5 ; srai r5, r6, 5 ; lh_u r25, r26 }
    4818:	[0-9a-f]* 	{ shri r5, r6, 5 ; nor r15, r16, r17 ; lh_u r25, r26 }
    4820:	[0-9a-f]* 	{ shri r5, r6, 5 ; sne r15, r16, r17 ; lh_u r25, r26 }
    4828:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt r15, r16, r17 ; lh_u r25, r26 }
    4830:	[0-9a-f]* 	{ slt r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    4838:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
    4840:	[0-9a-f]* 	{ slt r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    4848:	[0-9a-f]* 	{ slt r5, r6, r7 ; lh_u r25, r26 }
    4850:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    4858:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    4860:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    4868:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4870:	[0-9a-f]* 	{ bitx r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
    4878:	[0-9a-f]* 	{ slte r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    4880:	[0-9a-f]* 	{ slte r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    4888:	[0-9a-f]* 	{ slte r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    4890:	[0-9a-f]* 	{ slte r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    4898:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; info 19 ; lh_u r25, r26 }
    48a0:	[0-9a-f]* 	{ pcnt r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    48a8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; srai r5, r6, 5 ; lh_u r25, r26 }
    48b0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    48b8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    48c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
    48c8:	[0-9a-f]* 	{ slti r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    48d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lh_u r25, r26 }
    48d8:	[0-9a-f]* 	{ slti r5, r6, 5 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    48e0:	[0-9a-f]* 	{ slti r5, r6, 5 ; lh_u r25, r26 }
    48e8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lh_u r25, r26 }
    48f0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shr r5, r6, r7 ; lh_u r25, r26 }
    48f8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; and r15, r16, r17 ; lh_u r25, r26 }
    4900:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shl r15, r16, r17 ; lh_u r25, r26 }
    4908:	[0-9a-f]* 	{ bitx r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
    4910:	[0-9a-f]* 	{ sne r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    4918:	[0-9a-f]* 	{ sne r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    4920:	[0-9a-f]* 	{ sne r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    4928:	[0-9a-f]* 	{ sne r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    4930:	[0-9a-f]* 	{ sra r15, r16, r17 ; info 19 ; lh_u r25, r26 }
    4938:	[0-9a-f]* 	{ pcnt r5, r6 ; sra r15, r16, r17 ; lh_u r25, r26 }
    4940:	[0-9a-f]* 	{ sra r15, r16, r17 ; srai r5, r6, 5 ; lh_u r25, r26 }
    4948:	[0-9a-f]* 	{ sra r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    4950:	[0-9a-f]* 	{ sra r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    4958:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    4960:	[0-9a-f]* 	{ srai r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    4968:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; lh_u r25, r26 }
    4970:	[0-9a-f]* 	{ srai r5, r6, 5 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    4978:	[0-9a-f]* 	{ srai r5, r6, 5 ; lh_u r25, r26 }
    4980:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    4988:	[0-9a-f]* 	{ sub r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    4990:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    4998:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    49a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lh_u r25, r26 }
    49a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
    49b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; lh_u r25, r26 }
    49b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
    49c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; lh_u r25, r26 }
    49c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
    49d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; lh_u r25, r26 }
    49d8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lh_u r25, r26 }
    49e0:	[0-9a-f]* 	{ xor r15, r16, r17 ; movei r5, 5 ; lh_u r25, r26 }
    49e8:	[0-9a-f]* 	{ xor r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    49f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; lh_u r25, r26 }
    49f8:	[0-9a-f]* 	{ xor r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    4a00:	[0-9a-f]* 	{ xor r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    4a08:	[0-9a-f]* 	{ avgh r5, r6, r7 ; lhadd r15, r16, 5 }
    4a10:	[0-9a-f]* 	{ minh r5, r6, r7 ; lhadd r15, r16, 5 }
    4a18:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; lhadd r15, r16, 5 }
    4a20:	[0-9a-f]* 	{ nor r5, r6, r7 ; lhadd r15, r16, 5 }
    4a28:	[0-9a-f]* 	{ seqb r5, r6, r7 ; lhadd r15, r16, 5 }
    4a30:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; lhadd r15, r16, 5 }
    4a38:	[0-9a-f]* 	{ srah r5, r6, r7 ; lhadd r15, r16, 5 }
    4a40:	[0-9a-f]* 	{ addhs r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a48:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a50:	[0-9a-f]* 	{ move r5, r6 ; lhadd_u r15, r16, 5 }
    4a58:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a60:	[0-9a-f]* 	{ pcnt r5, r6 ; lhadd_u r15, r16, 5 }
    4a68:	[0-9a-f]* 	{ shlh r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a70:	[0-9a-f]* 	{ slth r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a78:	[0-9a-f]* 	{ subh r5, r6, r7 ; lhadd_u r15, r16, 5 }
    4a80:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; lnk r15 }
    4a88:	[0-9a-f]* 	{ intlh r5, r6, r7 ; lnk r15 }
    4a90:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lnk r15 }
    4a98:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; lnk r15 }
    4aa0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; lnk r15 }
    4aa8:	[0-9a-f]* 	{ shri r5, r6, 5 ; lnk r15 }
    4ab0:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; lnk r15 }
    4ab8:	[0-9a-f]* 	{ xor r5, r6, r7 ; lnk r15 }
    4ac0:	[0-9a-f]* 	{ bitx r5, r6 ; lw r15, r16 }
    4ac8:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; lw r15, r16 }
    4ad0:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; lw r15, r16 }
    4ad8:	[0-9a-f]* 	{ or r5, r6, r7 ; lw r15, r16 }
    4ae0:	[0-9a-f]* 	{ seqh r5, r6, r7 ; lw r15, r16 }
    4ae8:	[0-9a-f]* 	{ slte r5, r6, r7 ; lw r15, r16 }
    4af0:	[0-9a-f]* 	{ srai r5, r6, 5 ; lw r15, r16 }
    4af8:	[0-9a-f]* 	{ bytex r5, r6 ; add r15, r16, r17 ; lw r25, r26 }
    4b00:	[0-9a-f]* 	{ add r15, r16, r17 ; nop ; lw r25, r26 }
    4b08:	[0-9a-f]* 	{ add r15, r16, r17 ; slti r5, r6, 5 ; lw r25, r26 }
    4b10:	[0-9a-f]* 	{ add r5, r6, r7 ; move r15, r16 ; lw r25, r26 }
    4b18:	[0-9a-f]* 	{ add r5, r6, r7 ; slte r15, r16, r17 ; lw r25, r26 }
    4b20:	[0-9a-f]* 	{ addi r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
    4b28:	[0-9a-f]* 	{ addi r15, r16, 5 ; rl r5, r6, r7 ; lw r25, r26 }
    4b30:	[0-9a-f]* 	{ addi r15, r16, 5 ; sub r5, r6, r7 ; lw r25, r26 }
    4b38:	[0-9a-f]* 	{ addi r5, r6, 5 ; or r15, r16, r17 ; lw r25, r26 }
    4b40:	[0-9a-f]* 	{ addi r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
    4b48:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; and r15, r16, r17 ; lw r25, r26 }
    4b50:	[0-9a-f]* 	{ and r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
    4b58:	[0-9a-f]* 	{ and r15, r16, r17 ; xor r5, r6, r7 ; lw r25, r26 }
    4b60:	[0-9a-f]* 	{ and r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    4b68:	[0-9a-f]* 	{ andi r15, r16, 5 ; add r5, r6, r7 ; lw r25, r26 }
    4b70:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; andi r15, r16, 5 ; lw r25, r26 }
    4b78:	[0-9a-f]* 	{ andi r15, r16, 5 ; shri r5, r6, 5 ; lw r25, r26 }
    4b80:	[0-9a-f]* 	{ andi r5, r6, 5 ; andi r15, r16, 5 ; lw r25, r26 }
    4b88:	[0-9a-f]* 	{ andi r5, r6, 5 ; shli r15, r16, 5 ; lw r25, r26 }
    4b90:	[0-9a-f]* 	{ bitx r5, r6 ; ill ; lw r25, r26 }
    4b98:	[0-9a-f]* 	{ bitx r5, r6 ; shri r15, r16, 5 ; lw r25, r26 }
    4ba0:	[0-9a-f]* 	{ bytex r5, r6 ; mnz r15, r16, r17 ; lw r25, r26 }
    4ba8:	[0-9a-f]* 	{ bytex r5, r6 ; slt_u r15, r16, r17 ; lw r25, r26 }
    4bb0:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 ; lw r25, r26 }
    4bb8:	[0-9a-f]* 	{ clz r5, r6 ; slte_u r15, r16, r17 ; lw r25, r26 }
    4bc0:	[0-9a-f]* 	{ ctz r5, r6 ; nop ; lw r25, r26 }
    4bc8:	[0-9a-f]* 	{ ctz r5, r6 ; slti_u r15, r16, 5 ; lw r25, r26 }
    4bd0:	[0-9a-f]* 	{ ill ; lw r25, r26 }
    4bd8:	[0-9a-f]* 	{ mz r5, r6, r7 ; lw r25, r26 }
    4be0:	[0-9a-f]* 	{ seq r5, r6, r7 ; lw r25, r26 }
    4be8:	[0-9a-f]* 	{ slti r5, r6, 5 ; lw r25, r26 }
    4bf0:	[0-9a-f]* 	{ and r5, r6, r7 ; ill ; lw r25, r26 }
    4bf8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ill ; lw r25, r26 }
    4c00:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ill ; lw r25, r26 }
    4c08:	[0-9a-f]* 	{ info 19 ; and r5, r6, r7 ; lw r25, r26 }
    4c10:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; info 19 ; lw r25, r26 }
    4c18:	[0-9a-f]* 	{ info 19 ; rli r5, r6, 5 ; lw r25, r26 }
    4c20:	[0-9a-f]* 	{ info 19 ; slt r5, r6, r7 ; lw r25, r26 }
    4c28:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; lw r25, r26 }
    4c30:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    4c38:	[0-9a-f]* 	{ mnz r15, r16, r17 ; s3a r5, r6, r7 ; lw r25, r26 }
    4c40:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; lw r25, r26 }
    4c48:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    4c50:	[0-9a-f]* 	{ mnz r5, r6, r7 ; lw r25, r26 }
    4c58:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; move r15, r16 ; lw r25, r26 }
    4c60:	[0-9a-f]* 	{ move r15, r16 ; shr r5, r6, r7 ; lw r25, r26 }
    4c68:	[0-9a-f]* 	{ move r5, r6 ; and r15, r16, r17 ; lw r25, r26 }
    4c70:	[0-9a-f]* 	{ move r5, r6 ; shl r15, r16, r17 ; lw r25, r26 }
    4c78:	[0-9a-f]* 	{ bitx r5, r6 ; movei r15, 5 ; lw r25, r26 }
    4c80:	[0-9a-f]* 	{ movei r15, 5 ; mz r5, r6, r7 ; lw r25, r26 }
    4c88:	[0-9a-f]* 	{ movei r15, 5 ; slte_u r5, r6, r7 ; lw r25, r26 }
    4c90:	[0-9a-f]* 	{ movei r5, 5 ; mnz r15, r16, r17 ; lw r25, r26 }
    4c98:	[0-9a-f]* 	{ movei r5, 5 ; slt_u r15, r16, r17 ; lw r25, r26 }
    4ca0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    4ca8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    4cb0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nop ; lw r25, r26 }
    4cb8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
    4cc0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    4cc8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    4cd0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    4cd8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    4ce0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    4ce8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; lw r25, r26 }
    4cf0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
    4cf8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    4d00:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    4d08:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; andi r15, r16, 5 ; lw r25, r26 }
    4d10:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    4d18:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ill ; lw r25, r26 }
    4d20:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    4d28:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    4d30:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slt_u r15, r16, r17 ; lw r25, r26 }
    4d38:	[0-9a-f]* 	{ mvz r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    4d40:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    4d48:	[0-9a-f]* 	{ mz r15, r16, r17 ; move r5, r6 ; lw r25, r26 }
    4d50:	[0-9a-f]* 	{ mz r15, r16, r17 ; rli r5, r6, 5 ; lw r25, r26 }
    4d58:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; lw r25, r26 }
    4d60:	[0-9a-f]* 	{ mz r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    4d68:	[0-9a-f]* 	{ mz r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    4d70:	[0-9a-f]* 	{ nop ; mnz r5, r6, r7 ; lw r25, r26 }
    4d78:	[0-9a-f]* 	{ nop ; nor r5, r6, r7 ; lw r25, r26 }
    4d80:	[0-9a-f]* 	{ nop ; shl r15, r16, r17 ; lw r25, r26 }
    4d88:	[0-9a-f]* 	{ nop ; sne r15, r16, r17 ; lw r25, r26 }
    4d90:	[0-9a-f]* 	{ bytex r5, r6 ; nor r15, r16, r17 ; lw r25, r26 }
    4d98:	[0-9a-f]* 	{ nor r15, r16, r17 ; nop ; lw r25, r26 }
    4da0:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti r5, r6, 5 ; lw r25, r26 }
    4da8:	[0-9a-f]* 	{ nor r5, r6, r7 ; move r15, r16 ; lw r25, r26 }
    4db0:	[0-9a-f]* 	{ nor r5, r6, r7 ; slte r15, r16, r17 ; lw r25, r26 }
    4db8:	[0-9a-f]* 	{ or r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
    4dc0:	[0-9a-f]* 	{ or r15, r16, r17 ; rl r5, r6, r7 ; lw r25, r26 }
    4dc8:	[0-9a-f]* 	{ or r15, r16, r17 ; sub r5, r6, r7 ; lw r25, r26 }
    4dd0:	[0-9a-f]* 	{ or r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    4dd8:	[0-9a-f]* 	{ or r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    4de0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    4de8:	[0-9a-f]* 	{ ori r15, r16, 5 ; seq r5, r6, r7 ; lw r25, r26 }
    4df0:	[0-9a-f]* 	{ ori r15, r16, 5 ; xor r5, r6, r7 ; lw r25, r26 }
    4df8:	[0-9a-f]* 	{ ori r5, r6, 5 ; s2a r15, r16, r17 ; lw r25, r26 }
    4e00:	[0-9a-f]* 	{ pcnt r5, r6 ; add r15, r16, r17 ; lw r25, r26 }
    4e08:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 ; lw r25, r26 }
    4e10:	[0-9a-f]* 	{ rl r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    4e18:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    4e20:	[0-9a-f]* 	{ rl r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    4e28:	[0-9a-f]* 	{ rl r5, r6, r7 ; ill ; lw r25, r26 }
    4e30:	[0-9a-f]* 	{ rl r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    4e38:	[0-9a-f]* 	{ ctz r5, r6 ; rli r15, r16, 5 ; lw r25, r26 }
    4e40:	[0-9a-f]* 	{ rli r15, r16, 5 ; or r5, r6, r7 ; lw r25, r26 }
    4e48:	[0-9a-f]* 	{ rli r15, r16, 5 ; sne r5, r6, r7 ; lw r25, r26 }
    4e50:	[0-9a-f]* 	{ rli r5, r6, 5 ; mz r15, r16, r17 ; lw r25, r26 }
    4e58:	[0-9a-f]* 	{ rli r5, r6, 5 ; slti r15, r16, 5 ; lw r25, r26 }
    4e60:	[0-9a-f]* 	{ s1a r15, r16, r17 ; movei r5, 5 ; lw r25, r26 }
    4e68:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s1a r5, r6, r7 ; lw r25, r26 }
    4e70:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s1a r15, r16, r17 ; lw r25, r26 }
    4e78:	[0-9a-f]* 	{ s1a r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    4e80:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    4e88:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    4e90:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shl r5, r6, r7 ; lw r25, r26 }
    4e98:	[0-9a-f]* 	{ s2a r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    4ea0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    4ea8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    4eb0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
    4eb8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    4ec0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ill ; lw r25, r26 }
    4ec8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    4ed0:	[0-9a-f]* 	{ ctz r5, r6 ; seq r15, r16, r17 ; lw r25, r26 }
    4ed8:	[0-9a-f]* 	{ seq r15, r16, r17 ; or r5, r6, r7 ; lw r25, r26 }
    4ee0:	[0-9a-f]* 	{ seq r15, r16, r17 ; sne r5, r6, r7 ; lw r25, r26 }
    4ee8:	[0-9a-f]* 	{ seq r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    4ef0:	[0-9a-f]* 	{ seq r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    4ef8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; movei r5, 5 ; lw r25, r26 }
    4f00:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s1a r5, r6, r7 ; lw r25, r26 }
    4f08:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; seqi r15, r16, 5 ; lw r25, r26 }
    4f10:	[0-9a-f]* 	{ seqi r5, r6, 5 ; rl r15, r16, r17 ; lw r25, r26 }
    4f18:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sub r15, r16, r17 ; lw r25, r26 }
    4f20:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    4f28:	[0-9a-f]* 	{ shl r15, r16, r17 ; shl r5, r6, r7 ; lw r25, r26 }
    4f30:	[0-9a-f]* 	{ shl r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    4f38:	[0-9a-f]* 	{ shl r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    4f40:	[0-9a-f]* 	{ shli r15, r16, 5 ; and r5, r6, r7 ; lw r25, r26 }
    4f48:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    4f50:	[0-9a-f]* 	{ shli r15, r16, 5 ; slt_u r5, r6, r7 ; lw r25, r26 }
    4f58:	[0-9a-f]* 	{ shli r5, r6, 5 ; ill ; lw r25, r26 }
    4f60:	[0-9a-f]* 	{ shli r5, r6, 5 ; shri r15, r16, 5 ; lw r25, r26 }
    4f68:	[0-9a-f]* 	{ ctz r5, r6 ; shr r15, r16, r17 ; lw r25, r26 }
    4f70:	[0-9a-f]* 	{ shr r15, r16, r17 ; or r5, r6, r7 ; lw r25, r26 }
    4f78:	[0-9a-f]* 	{ shr r15, r16, r17 ; sne r5, r6, r7 ; lw r25, r26 }
    4f80:	[0-9a-f]* 	{ shr r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    4f88:	[0-9a-f]* 	{ shr r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    4f90:	[0-9a-f]* 	{ shri r15, r16, 5 ; movei r5, 5 ; lw r25, r26 }
    4f98:	[0-9a-f]* 	{ shri r15, r16, 5 ; s1a r5, r6, r7 ; lw r25, r26 }
    4fa0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shri r15, r16, 5 ; lw r25, r26 }
    4fa8:	[0-9a-f]* 	{ shri r5, r6, 5 ; rl r15, r16, r17 ; lw r25, r26 }
    4fb0:	[0-9a-f]* 	{ shri r5, r6, 5 ; sub r15, r16, r17 ; lw r25, r26 }
    4fb8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
    4fc0:	[0-9a-f]* 	{ slt r15, r16, r17 ; shl r5, r6, r7 ; lw r25, r26 }
    4fc8:	[0-9a-f]* 	{ slt r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    4fd0:	[0-9a-f]* 	{ slt r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    4fd8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    4fe0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slt_u r15, r16, r17 ; lw r25, r26 }
    4fe8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    4ff0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ill ; lw r25, r26 }
    4ff8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    5000:	[0-9a-f]* 	{ ctz r5, r6 ; slte r15, r16, r17 ; lw r25, r26 }
    5008:	[0-9a-f]* 	{ slte r15, r16, r17 ; or r5, r6, r7 ; lw r25, r26 }
    5010:	[0-9a-f]* 	{ slte r15, r16, r17 ; sne r5, r6, r7 ; lw r25, r26 }
    5018:	[0-9a-f]* 	{ slte r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    5020:	[0-9a-f]* 	{ slte r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    5028:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; movei r5, 5 ; lw r25, r26 }
    5030:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s1a r5, r6, r7 ; lw r25, r26 }
    5038:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte_u r15, r16, r17 ; lw r25, r26 }
    5040:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    5048:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    5050:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    5058:	[0-9a-f]* 	{ slti r15, r16, 5 ; shl r5, r6, r7 ; lw r25, r26 }
    5060:	[0-9a-f]* 	{ slti r5, r6, 5 ; add r15, r16, r17 ; lw r25, r26 }
    5068:	[0-9a-f]* 	{ slti r5, r6, 5 ; seq r15, r16, r17 ; lw r25, r26 }
    5070:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; and r5, r6, r7 ; lw r25, r26 }
    5078:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
    5080:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slt_u r5, r6, r7 ; lw r25, r26 }
    5088:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ill ; lw r25, r26 }
    5090:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shri r15, r16, 5 ; lw r25, r26 }
    5098:	[0-9a-f]* 	{ ctz r5, r6 ; sne r15, r16, r17 ; lw r25, r26 }
    50a0:	[0-9a-f]* 	{ sne r15, r16, r17 ; or r5, r6, r7 ; lw r25, r26 }
    50a8:	[0-9a-f]* 	{ sne r15, r16, r17 ; sne r5, r6, r7 ; lw r25, r26 }
    50b0:	[0-9a-f]* 	{ sne r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    50b8:	[0-9a-f]* 	{ sne r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    50c0:	[0-9a-f]* 	{ sra r15, r16, r17 ; movei r5, 5 ; lw r25, r26 }
    50c8:	[0-9a-f]* 	{ sra r15, r16, r17 ; s1a r5, r6, r7 ; lw r25, r26 }
    50d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sra r15, r16, r17 ; lw r25, r26 }
    50d8:	[0-9a-f]* 	{ sra r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    50e0:	[0-9a-f]* 	{ sra r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    50e8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    50f0:	[0-9a-f]* 	{ srai r15, r16, 5 ; shl r5, r6, r7 ; lw r25, r26 }
    50f8:	[0-9a-f]* 	{ srai r5, r6, 5 ; add r15, r16, r17 ; lw r25, r26 }
    5100:	[0-9a-f]* 	{ srai r5, r6, 5 ; seq r15, r16, r17 ; lw r25, r26 }
    5108:	[0-9a-f]* 	{ sub r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    5110:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    5118:	[0-9a-f]* 	{ sub r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    5120:	[0-9a-f]* 	{ sub r5, r6, r7 ; ill ; lw r25, r26 }
    5128:	[0-9a-f]* 	{ sub r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    5130:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; lw r25, r26 }
    5138:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slt_u r15, r16, r17 ; lw r25, r26 }
    5140:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; movei r15, 5 ; lw r25, r26 }
    5148:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte_u r15, r16, r17 ; lw r25, r26 }
    5150:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop ; lw r25, r26 }
    5158:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slti_u r15, r16, 5 ; lw r25, r26 }
    5160:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; or r15, r16, r17 ; lw r25, r26 }
    5168:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sra r15, r16, r17 ; lw r25, r26 }
    5170:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    5178:	[0-9a-f]* 	{ xor r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
    5180:	[0-9a-f]* 	{ xor r15, r16, r17 ; xor r5, r6, r7 ; lw r25, r26 }
    5188:	[0-9a-f]* 	{ xor r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    5190:	[0-9a-f]* 	{ add r5, r6, r7 ; lw_na r15, r16 }
    5198:	[0-9a-f]* 	{ clz r5, r6 ; lw_na r15, r16 }
    51a0:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; lw_na r15, r16 }
    51a8:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; lw_na r15, r16 }
    51b0:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; lw_na r15, r16 }
    51b8:	[0-9a-f]* 	{ seqib r5, r6, 5 ; lw_na r15, r16 }
    51c0:	[0-9a-f]* 	{ slteb r5, r6, r7 ; lw_na r15, r16 }
    51c8:	[0-9a-f]* 	{ sraih r5, r6, 5 ; lw_na r15, r16 }
    51d0:	[0-9a-f]* 	{ addih r5, r6, 5 ; lwadd r15, r16, 5 }
    51d8:	[0-9a-f]* 	{ infol 4660 ; lwadd r15, r16, 5 }
    51e0:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; lwadd r15, r16, 5 }
    51e8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; lwadd r15, r16, 5 }
    51f0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; lwadd r15, r16, 5 }
    51f8:	[0-9a-f]* 	{ shlih r5, r6, 5 ; lwadd r15, r16, 5 }
    5200:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; lwadd r15, r16, 5 }
    5208:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; lwadd r15, r16, 5 }
    5210:	[0-9a-f]* 	{ andi r5, r6, 5 ; lwadd_na r15, r16, 5 }
    5218:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; lwadd_na r15, r16, 5 }
    5220:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; lwadd_na r15, r16, 5 }
    5228:	[0-9a-f]* 	{ mzb r5, r6, r7 ; lwadd_na r15, r16, 5 }
    5230:	[0-9a-f]* 	{ sadh r5, r6, r7 ; lwadd_na r15, r16, 5 }
    5238:	[0-9a-f]* 	{ slt r5, r6, r7 ; lwadd_na r15, r16, 5 }
    5240:	[0-9a-f]* 	{ sneh r5, r6, r7 ; lwadd_na r15, r16, 5 }
    5248:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; addb r5, r6, r7 }
    5250:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; maxb_u r15, r16, r17 }
    5258:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; mnz r5, r6, r7 }
    5260:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; maxb_u r15, r16, r17 }
    5268:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; packhb r5, r6, r7 }
    5270:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; seqih r5, r6, 5 }
    5278:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; slteb_u r5, r6, r7 }
    5280:	[0-9a-f]* 	{ maxb_u r15, r16, r17 ; sub r5, r6, r7 }
    5288:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; addli r15, r16, 4660 }
    5290:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; jalr r15 }
    5298:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; maxih r15, r16, 5 }
    52a0:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; nor r15, r16, r17 }
    52a8:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; seqib r15, r16, 5 }
    52b0:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; slte r15, r16, r17 }
    52b8:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; srai r15, r16, 5 }
    52c0:	[0-9a-f]* 	{ maxh r15, r16, r17 ; addi r5, r6, 5 }
    52c8:	[0-9a-f]* 	{ maxh r15, r16, r17 }
    52d0:	[0-9a-f]* 	{ maxh r15, r16, r17 ; movei r5, 5 }
    52d8:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; maxh r15, r16, r17 }
    52e0:	[0-9a-f]* 	{ maxh r15, r16, r17 ; rl r5, r6, r7 }
    52e8:	[0-9a-f]* 	{ maxh r15, r16, r17 ; shli r5, r6, 5 }
    52f0:	[0-9a-f]* 	{ maxh r15, r16, r17 ; slth_u r5, r6, r7 }
    52f8:	[0-9a-f]* 	{ maxh r15, r16, r17 ; subhs r5, r6, r7 }
    5300:	[0-9a-f]* 	{ maxh r5, r6, r7 ; andi r15, r16, 5 }
    5308:	[0-9a-f]* 	{ maxh r5, r6, r7 ; lb r15, r16 }
    5310:	[0-9a-f]* 	{ maxh r5, r6, r7 ; minh r15, r16, r17 }
    5318:	[0-9a-f]* 	{ maxh r5, r6, r7 ; packhb r15, r16, r17 }
    5320:	[0-9a-f]* 	{ maxh r5, r6, r7 ; shl r15, r16, r17 }
    5328:	[0-9a-f]* 	{ maxh r5, r6, r7 ; slteh r15, r16, r17 }
    5330:	[0-9a-f]* 	{ maxh r5, r6, r7 ; subb r15, r16, r17 }
    5338:	[0-9a-f]* 	{ maxib_u r15, r16, 5 ; addli.sn r5, r6, 4660 }
    5340:	[0-9a-f]* 	{ maxib_u r15, r16, 5 ; inthh r5, r6, r7 }
    5348:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; maxib_u r15, r16, 5 }
    5350:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; maxib_u r15, r16, 5 }
    5358:	[0-9a-f]* 	{ maxib_u r15, r16, 5 ; s3a r5, r6, r7 }
    5360:	[0-9a-f]* 	{ maxib_u r15, r16, 5 ; shrb r5, r6, r7 }
    5368:	[0-9a-f]* 	{ maxib_u r15, r16, 5 ; sltib_u r5, r6, 5 }
    5370:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; maxib_u r15, r16, 5 }
    5378:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; flush r15 }
    5380:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; lh r15, r16 }
    5388:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; mnz r15, r16, r17 }
    5390:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; raise }
    5398:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; shlib r15, r16, 5 }
    53a0:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; slti r15, r16, 5 }
    53a8:	[0-9a-f]* 	{ maxib_u r5, r6, 5 ; subs r15, r16, r17 }
    53b0:	[0-9a-f]* 	{ maxih r15, r16, 5 ; and r5, r6, r7 }
    53b8:	[0-9a-f]* 	{ maxih r15, r16, 5 ; maxh r5, r6, r7 }
    53c0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; maxih r15, r16, 5 }
    53c8:	[0-9a-f]* 	{ maxih r15, r16, 5 ; mz r5, r6, r7 }
    53d0:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; maxih r15, r16, 5 }
    53d8:	[0-9a-f]* 	{ maxih r15, r16, 5 ; shrih r5, r6, 5 }
    53e0:	[0-9a-f]* 	{ maxih r15, r16, 5 ; sneb r5, r6, r7 }
    53e8:	[0-9a-f]* 	{ maxih r5, r6, 5 ; add r15, r16, r17 }
    53f0:	[0-9a-f]* 	{ maxih r5, r6, 5 ; info 19 }
    53f8:	[0-9a-f]* 	{ maxih r5, r6, 5 ; lnk r15 }
    5400:	[0-9a-f]* 	{ maxih r5, r6, 5 ; movei r15, 5 }
    5408:	[0-9a-f]* 	{ maxih r5, r6, 5 ; s2a r15, r16, r17 }
    5410:	[0-9a-f]* 	{ maxih r5, r6, 5 ; shrh r15, r16, r17 }
    5418:	[0-9a-f]* 	{ maxih r5, r6, 5 ; sltih r15, r16, 5 }
    5420:	[0-9a-f]* 	{ maxih r5, r6, 5 ; wh64 r15 }
    5428:	[0-9a-f]* 	{ avgh r5, r6, r7 ; mf }
    5430:	[0-9a-f]* 	{ minh r5, r6, r7 ; mf }
    5438:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; mf }
    5440:	[0-9a-f]* 	{ nor r5, r6, r7 ; mf }
    5448:	[0-9a-f]* 	{ seqb r5, r6, r7 ; mf }
    5450:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; mf }
    5458:	[0-9a-f]* 	{ srah r5, r6, r7 ; mf }
    5460:	[0-9a-f]* 	{ addhs r5, r6, r7 ; mfspr r16, 5 }
    5468:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; mfspr r16, 5 }
    5470:	[0-9a-f]* 	{ move r5, r6 ; mfspr r16, 5 }
    5478:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mfspr r16, 5 }
    5480:	[0-9a-f]* 	{ pcnt r5, r6 ; mfspr r16, 5 }
    5488:	[0-9a-f]* 	{ shlh r5, r6, r7 ; mfspr r16, 5 }
    5490:	[0-9a-f]* 	{ slth r5, r6, r7 ; mfspr r16, 5 }
    5498:	[0-9a-f]* 	{ subh r5, r6, r7 ; mfspr r16, 5 }
    54a0:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; minb_u r15, r16, r17 }
    54a8:	[0-9a-f]* 	{ minb_u r15, r16, r17 ; intlh r5, r6, r7 }
    54b0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; minb_u r15, r16, r17 }
    54b8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; minb_u r15, r16, r17 }
    54c0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; minb_u r15, r16, r17 }
    54c8:	[0-9a-f]* 	{ minb_u r15, r16, r17 ; shri r5, r6, 5 }
    54d0:	[0-9a-f]* 	{ minb_u r15, r16, r17 ; sltih_u r5, r6, 5 }
    54d8:	[0-9a-f]* 	{ minb_u r15, r16, r17 ; xor r5, r6, r7 }
    54e0:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; icoh r15 }
    54e8:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; lhadd r15, r16, 5 }
    54f0:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; mnzh r15, r16, r17 }
    54f8:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; rli r15, r16, 5 }
    5500:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; shr r15, r16, r17 }
    5508:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; sltib r15, r16, 5 }
    5510:	[0-9a-f]* 	{ minb_u r5, r6, r7 ; swadd r15, r16, 5 }
    5518:	[0-9a-f]* 	{ minh r15, r16, r17 ; auli r5, r6, 4660 }
    5520:	[0-9a-f]* 	{ minh r15, r16, r17 ; maxih r5, r6, 5 }
    5528:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; minh r15, r16, r17 }
    5530:	[0-9a-f]* 	{ minh r15, r16, r17 ; mzh r5, r6, r7 }
    5538:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; minh r15, r16, r17 }
    5540:	[0-9a-f]* 	{ minh r15, r16, r17 ; slt_u r5, r6, r7 }
    5548:	[0-9a-f]* 	{ minh r15, r16, r17 ; sra r5, r6, r7 }
    5550:	[0-9a-f]* 	{ minh r5, r6, r7 ; addbs_u r15, r16, r17 }
    5558:	[0-9a-f]* 	{ minh r5, r6, r7 ; inthb r15, r16, r17 }
    5560:	[0-9a-f]* 	{ minh r5, r6, r7 ; lw_na r15, r16 }
    5568:	[0-9a-f]* 	{ minh r5, r6, r7 ; moveli.sn r15, 4660 }
    5570:	[0-9a-f]* 	{ minh r5, r6, r7 ; sb r15, r16 }
    5578:	[0-9a-f]* 	{ minh r5, r6, r7 ; shrib r15, r16, 5 }
    5580:	[0-9a-f]* 	{ minh r5, r6, r7 ; sne r15, r16, r17 }
    5588:	[0-9a-f]* 	{ minh r5, r6, r7 ; xori r15, r16, 5 }
    5590:	[0-9a-f]* 	{ bytex r5, r6 ; minib_u r15, r16, 5 }
    5598:	[0-9a-f]* 	{ minib_u r15, r16, 5 ; minih r5, r6, 5 }
    55a0:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; minib_u r15, r16, 5 }
    55a8:	[0-9a-f]* 	{ minib_u r15, r16, 5 ; ori r5, r6, 5 }
    55b0:	[0-9a-f]* 	{ minib_u r15, r16, 5 ; seqi r5, r6, 5 }
    55b8:	[0-9a-f]* 	{ minib_u r15, r16, 5 ; slte_u r5, r6, r7 }
    55c0:	[0-9a-f]* 	{ minib_u r15, r16, 5 ; sraib r5, r6, 5 }
    55c8:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; addib r15, r16, 5 }
    55d0:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; inv r15 }
    55d8:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; maxh r15, r16, r17 }
    55e0:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; mzh r15, r16, r17 }
    55e8:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; seqh r15, r16, r17 }
    55f0:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; sltb r15, r16, r17 }
    55f8:	[0-9a-f]* 	{ minib_u r5, r6, 5 ; srab r15, r16, r17 }
    5600:	[0-9a-f]* 	{ minih r15, r16, 5 ; addh r5, r6, r7 }
    5608:	[0-9a-f]* 	{ ctz r5, r6 ; minih r15, r16, 5 }
    5610:	[0-9a-f]* 	{ minih r15, r16, 5 ; mnzh r5, r6, r7 }
    5618:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; minih r15, r16, 5 }
    5620:	[0-9a-f]* 	{ minih r15, r16, 5 ; packlb r5, r6, r7 }
    5628:	[0-9a-f]* 	{ minih r15, r16, 5 ; shlb r5, r6, r7 }
    5630:	[0-9a-f]* 	{ minih r15, r16, 5 ; slteh_u r5, r6, r7 }
    5638:	[0-9a-f]* 	{ minih r15, r16, 5 ; subbs_u r5, r6, r7 }
    5640:	[0-9a-f]* 	{ minih r5, r6, 5 ; adds r15, r16, r17 }
    5648:	[0-9a-f]* 	{ minih r5, r6, 5 ; jr r15 }
    5650:	[0-9a-f]* 	{ minih r5, r6, 5 ; mfspr r16, 5 }
    5658:	[0-9a-f]* 	{ minih r5, r6, 5 ; ori r15, r16, 5 }
    5660:	[0-9a-f]* 	{ minih r5, r6, 5 ; sh r15, r16 }
    5668:	[0-9a-f]* 	{ minih r5, r6, 5 ; slteb r15, r16, r17 }
    5670:	[0-9a-f]* 	{ minih r5, r6, 5 ; sraih r15, r16, 5 }
    5678:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; addih r5, r6, 5 }
    5680:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; infol 4660 }
    5688:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; moveli.sn r5, 4660 }
    5690:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
    5698:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; s1a r5, r6, r7 }
    56a0:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; shlih r5, r6, 5 }
    56a8:	[0-9a-f]* 	{ mm r15, r16, r17, 5, 7 ; slti_u r5, r6, 5 }
    56b0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mm r15, r16, r17, 5, 7 }
    56b8:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; dtlbpr r15 }
    56c0:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; lbadd r15, r16, 5 }
    56c8:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; minih r15, r16, 5 }
    56d0:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; packlb r15, r16, r17 }
    56d8:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; shlh r15, r16, r17 }
    56e0:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; slth r15, r16, r17 }
    56e8:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; subh r15, r16, r17 }
    56f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; addbs_u r5, r6, r7 }
    56f8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; and r5, r6, r7 ; lb r25, r26 }
    5700:	[0-9a-f]* 	{ mnz r15, r16, r17 ; auli r5, r6, 4660 }
    5708:	[0-9a-f]* 	{ bytex r5, r6 ; mnz r15, r16, r17 ; sh r25, r26 }
    5710:	[0-9a-f]* 	{ ctz r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
    5718:	[0-9a-f]* 	{ mnz r15, r16, r17 ; info 19 ; lw r25, r26 }
    5720:	[0-9a-f]* 	{ mnz r15, r16, r17 ; info 19 ; lb r25, r26 }
    5728:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; lb r25, r26 }
    5730:	[0-9a-f]* 	{ mnz r15, r16, r17 ; srai r5, r6, 5 ; lb r25, r26 }
    5738:	[0-9a-f]* 	{ mnz r15, r16, r17 ; movei r5, 5 ; lb_u r25, r26 }
    5740:	[0-9a-f]* 	{ mnz r15, r16, r17 ; s1a r5, r6, r7 ; lb_u r25, r26 }
    5748:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    5750:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh r25, r26 }
    5758:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seq r5, r6, r7 ; lh r25, r26 }
    5760:	[0-9a-f]* 	{ mnz r15, r16, r17 ; xor r5, r6, r7 ; lh r25, r26 }
    5768:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    5770:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shli r5, r6, 5 ; lh_u r25, r26 }
    5778:	[0-9a-f]* 	{ mnz r15, r16, r17 ; addi r5, r6, 5 ; lw r25, r26 }
    5780:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    5788:	[0-9a-f]* 	{ mnz r15, r16, r17 ; slt r5, r6, r7 ; lw r25, r26 }
    5790:	[0-9a-f]* 	{ mnz r15, r16, r17 ; minb_u r5, r6, r7 }
    5798:	[0-9a-f]* 	{ mnz r15, r16, r17 ; move r5, r6 ; lh_u r25, r26 }
    57a0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    57a8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    57b0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 }
    57b8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    57c0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; mnz r15, r16, r17 }
    57c8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    57d0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mnz r15, r16, r17 ; sh r25, r26 }
    57d8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nop ; prefetch r25 }
    57e0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; or r5, r6, r7 ; prefetch r25 }
    57e8:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    57f0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; move r5, r6 ; prefetch r25 }
    57f8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
    5800:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
    5808:	[0-9a-f]* 	{ mnz r15, r16, r17 ; rli r5, r6, 5 ; lw r25, r26 }
    5810:	[0-9a-f]* 	{ mnz r15, r16, r17 ; s2a r5, r6, r7 ; lw r25, r26 }
    5818:	[0-9a-f]* 	{ sadh r5, r6, r7 ; mnz r15, r16, r17 }
    5820:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    5828:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shli r5, r6, 5 ; sb r25, r26 }
    5830:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seq r5, r6, r7 ; lb_u r25, r26 }
    5838:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seqi r5, r6, 5 }
    5840:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; mnz r15, r16, r17 ; sh r25, r26 }
    5848:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl r5, r6, r7 ; sh r25, r26 }
    5850:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shl r5, r6, r7 ; lb r25, r26 }
    5858:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shli r5, r6, 5 ; sw r25, r26 }
    5860:	[0-9a-f]* 	{ mnz r15, r16, r17 ; shri r5, r6, 5 ; lw r25, r26 }
    5868:	[0-9a-f]* 	{ mnz r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
    5870:	[0-9a-f]* 	{ mnz r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
    5878:	[0-9a-f]* 	{ mnz r15, r16, r17 ; slti r5, r6, 5 ; lw r25, r26 }
    5880:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sne r5, r6, r7 ; lb r25, r26 }
    5888:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sra r5, r6, r7 ; sw r25, r26 }
    5890:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; lw r25, r26 }
    5898:	[0-9a-f]* 	{ mnz r15, r16, r17 ; info 19 ; sw r25, r26 }
    58a0:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
    58a8:	[0-9a-f]* 	{ mnz r15, r16, r17 ; srai r5, r6, 5 ; sw r25, r26 }
    58b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mnz r15, r16, r17 ; lh r25, r26 }
    58b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; lh r25, r26 }
    58c0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
    58c8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; addi r15, r16, 5 ; sh r25, r26 }
    58d0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
    58d8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; sw r25, r26 }
    58e0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; sh r25, r26 }
    58e8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; lb r25, r26 }
    58f0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    58f8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    5900:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slt r15, r16, r17 ; lb_u r25, r26 }
    5908:	[0-9a-f]* 	{ mnz r5, r6, r7 ; ill ; lh r25, r26 }
    5910:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    5918:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    5920:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slt r15, r16, r17 ; lh_u r25, r26 }
    5928:	[0-9a-f]* 	{ mnz r5, r6, r7 ; lw r25, r26 }
    5930:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
    5938:	[0-9a-f]* 	{ mnz r5, r6, r7 ; maxih r15, r16, 5 }
    5940:	[0-9a-f]* 	{ mnz r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    5948:	[0-9a-f]* 	{ mnz r5, r6, r7 ; moveli r15, 4660 }
    5950:	[0-9a-f]* 	{ mnz r5, r6, r7 ; nop ; prefetch r25 }
    5958:	[0-9a-f]* 	{ mnz r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    5960:	[0-9a-f]* 	{ mnz r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
    5968:	[0-9a-f]* 	{ mnz r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    5970:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    5978:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    5980:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    5988:	[0-9a-f]* 	{ mnz r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    5990:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    5998:	[0-9a-f]* 	{ mnz r5, r6, r7 ; seqh r15, r16, r17 }
    59a0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; info 19 ; sh r25, r26 }
    59a8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    59b0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    59b8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shr r15, r16, r17 ; lh_u r25, r26 }
    59c0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shrih r15, r16, 5 }
    59c8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slt_u r15, r16, r17 }
    59d0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    59d8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    59e0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    59e8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    59f0:	[0-9a-f]* 	{ mnz r5, r6, r7 ; subbs_u r15, r16, r17 }
    59f8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; rl r15, r16, r17 ; sw r25, r26 }
    5a00:	[0-9a-f]* 	{ mnz r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    5a08:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; addh r5, r6, r7 }
    5a10:	[0-9a-f]* 	{ ctz r5, r6 ; mnzb r15, r16, r17 }
    5a18:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; mnzh r5, r6, r7 }
    5a20:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; mnzb r15, r16, r17 }
    5a28:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; packlb r5, r6, r7 }
    5a30:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; shlb r5, r6, r7 }
    5a38:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; slteh_u r5, r6, r7 }
    5a40:	[0-9a-f]* 	{ mnzb r15, r16, r17 ; subbs_u r5, r6, r7 }
    5a48:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; adds r15, r16, r17 }
    5a50:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; jr r15 }
    5a58:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; mfspr r16, 5 }
    5a60:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; ori r15, r16, 5 }
    5a68:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; sh r15, r16 }
    5a70:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; slteb r15, r16, r17 }
    5a78:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; sraih r15, r16, 5 }
    5a80:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; addih r5, r6, 5 }
    5a88:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; infol 4660 }
    5a90:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; moveli.sn r5, 4660 }
    5a98:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mnzh r15, r16, r17 }
    5aa0:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; s1a r5, r6, r7 }
    5aa8:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; shlih r5, r6, 5 }
    5ab0:	[0-9a-f]* 	{ mnzh r15, r16, r17 ; slti_u r5, r6, 5 }
    5ab8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mnzh r15, r16, r17 }
    5ac0:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; dtlbpr r15 }
    5ac8:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; lbadd r15, r16, 5 }
    5ad0:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; minih r15, r16, 5 }
    5ad8:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; packlb r15, r16, r17 }
    5ae0:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; shlh r15, r16, r17 }
    5ae8:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; slth r15, r16, r17 }
    5af0:	[0-9a-f]* 	{ mnzh r5, r6, r7 ; subh r15, r16, r17 }
    5af8:	[0-9a-f]* 	{ move r15, r16 ; addbs_u r5, r6, r7 }
    5b00:	[0-9a-f]* 	{ move r15, r16 ; and r5, r6, r7 ; lb r25, r26 }
    5b08:	[0-9a-f]* 	{ move r15, r16 ; auli r5, r6, 4660 }
    5b10:	[0-9a-f]* 	{ bytex r5, r6 ; move r15, r16 ; sh r25, r26 }
    5b18:	[0-9a-f]* 	{ ctz r5, r6 ; move r15, r16 ; prefetch r25 }
    5b20:	[0-9a-f]* 	{ move r15, r16 ; info 19 ; lw r25, r26 }
    5b28:	[0-9a-f]* 	{ move r15, r16 ; info 19 ; lb r25, r26 }
    5b30:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; lb r25, r26 }
    5b38:	[0-9a-f]* 	{ move r15, r16 ; srai r5, r6, 5 ; lb r25, r26 }
    5b40:	[0-9a-f]* 	{ move r15, r16 ; movei r5, 5 ; lb_u r25, r26 }
    5b48:	[0-9a-f]* 	{ move r15, r16 ; s1a r5, r6, r7 ; lb_u r25, r26 }
    5b50:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; lb_u r25, r26 }
    5b58:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    5b60:	[0-9a-f]* 	{ move r15, r16 ; seq r5, r6, r7 ; lh r25, r26 }
    5b68:	[0-9a-f]* 	{ move r15, r16 ; xor r5, r6, r7 ; lh r25, r26 }
    5b70:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
    5b78:	[0-9a-f]* 	{ move r15, r16 ; shli r5, r6, 5 ; lh_u r25, r26 }
    5b80:	[0-9a-f]* 	{ move r15, r16 ; addi r5, r6, 5 ; lw r25, r26 }
    5b88:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; move r15, r16 ; lw r25, r26 }
    5b90:	[0-9a-f]* 	{ move r15, r16 ; slt r5, r6, r7 ; lw r25, r26 }
    5b98:	[0-9a-f]* 	{ move r15, r16 ; minb_u r5, r6, r7 }
    5ba0:	[0-9a-f]* 	{ move r15, r16 ; move r5, r6 ; lh_u r25, r26 }
    5ba8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    5bb0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    5bb8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; move r15, r16 }
    5bc0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    5bc8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; move r15, r16 }
    5bd0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    5bd8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; move r15, r16 ; sh r25, r26 }
    5be0:	[0-9a-f]* 	{ move r15, r16 ; nop ; prefetch r25 }
    5be8:	[0-9a-f]* 	{ move r15, r16 ; or r5, r6, r7 ; prefetch r25 }
    5bf0:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; lb_u r25, r26 }
    5bf8:	[0-9a-f]* 	{ move r15, r16 ; move r5, r6 ; prefetch r25 }
    5c00:	[0-9a-f]* 	{ move r15, r16 ; rli r5, r6, 5 ; prefetch r25 }
    5c08:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; prefetch r25 }
    5c10:	[0-9a-f]* 	{ move r15, r16 ; rli r5, r6, 5 ; lw r25, r26 }
    5c18:	[0-9a-f]* 	{ move r15, r16 ; s2a r5, r6, r7 ; lw r25, r26 }
    5c20:	[0-9a-f]* 	{ sadh r5, r6, r7 ; move r15, r16 }
    5c28:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; move r15, r16 ; sb r25, r26 }
    5c30:	[0-9a-f]* 	{ move r15, r16 ; shli r5, r6, 5 ; sb r25, r26 }
    5c38:	[0-9a-f]* 	{ move r15, r16 ; seq r5, r6, r7 ; lb_u r25, r26 }
    5c40:	[0-9a-f]* 	{ move r15, r16 ; seqi r5, r6, 5 }
    5c48:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; move r15, r16 ; sh r25, r26 }
    5c50:	[0-9a-f]* 	{ move r15, r16 ; shl r5, r6, r7 ; sh r25, r26 }
    5c58:	[0-9a-f]* 	{ move r15, r16 ; shl r5, r6, r7 ; lb r25, r26 }
    5c60:	[0-9a-f]* 	{ move r15, r16 ; shli r5, r6, 5 ; sw r25, r26 }
    5c68:	[0-9a-f]* 	{ move r15, r16 ; shri r5, r6, 5 ; lw r25, r26 }
    5c70:	[0-9a-f]* 	{ move r15, r16 ; slt_u r5, r6, r7 ; lh r25, r26 }
    5c78:	[0-9a-f]* 	{ move r15, r16 ; slte_u r5, r6, r7 ; lb r25, r26 }
    5c80:	[0-9a-f]* 	{ move r15, r16 ; slti r5, r6, 5 ; lw r25, r26 }
    5c88:	[0-9a-f]* 	{ move r15, r16 ; sne r5, r6, r7 ; lb r25, r26 }
    5c90:	[0-9a-f]* 	{ move r15, r16 ; sra r5, r6, r7 ; sw r25, r26 }
    5c98:	[0-9a-f]* 	{ move r15, r16 ; sub r5, r6, r7 ; lw r25, r26 }
    5ca0:	[0-9a-f]* 	{ move r15, r16 ; info 19 ; sw r25, r26 }
    5ca8:	[0-9a-f]* 	{ pcnt r5, r6 ; move r15, r16 ; sw r25, r26 }
    5cb0:	[0-9a-f]* 	{ move r15, r16 ; srai r5, r6, 5 ; sw r25, r26 }
    5cb8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; lh r25, r26 }
    5cc0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; move r15, r16 ; lh r25, r26 }
    5cc8:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; lb_u r25, r26 }
    5cd0:	[0-9a-f]* 	{ move r5, r6 ; addi r15, r16, 5 ; sh r25, r26 }
    5cd8:	[0-9a-f]* 	{ move r5, r6 ; andi r15, r16, 5 ; lh r25, r26 }
    5ce0:	[0-9a-f]* 	{ move r5, r6 ; sw r25, r26 }
    5ce8:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; sh r25, r26 }
    5cf0:	[0-9a-f]* 	{ move r5, r6 ; ill ; lb r25, r26 }
    5cf8:	[0-9a-f]* 	{ move r5, r6 ; shri r15, r16, 5 ; lb r25, r26 }
    5d00:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; lb_u r25, r26 }
    5d08:	[0-9a-f]* 	{ move r5, r6 ; slt r15, r16, r17 ; lb_u r25, r26 }
    5d10:	[0-9a-f]* 	{ move r5, r6 ; ill ; lh r25, r26 }
    5d18:	[0-9a-f]* 	{ move r5, r6 ; shri r15, r16, 5 ; lh r25, r26 }
    5d20:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; lh_u r25, r26 }
    5d28:	[0-9a-f]* 	{ move r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
    5d30:	[0-9a-f]* 	{ move r5, r6 ; lw r25, r26 }
    5d38:	[0-9a-f]* 	{ move r5, r6 ; shr r15, r16, r17 ; lw r25, r26 }
    5d40:	[0-9a-f]* 	{ move r5, r6 ; maxih r15, r16, 5 }
    5d48:	[0-9a-f]* 	{ move r5, r6 ; move r15, r16 ; lb r25, r26 }
    5d50:	[0-9a-f]* 	{ move r5, r6 ; moveli r15, 4660 }
    5d58:	[0-9a-f]* 	{ move r5, r6 ; nop ; prefetch r25 }
    5d60:	[0-9a-f]* 	{ move r5, r6 ; or r15, r16, r17 ; prefetch r25 }
    5d68:	[0-9a-f]* 	{ move r5, r6 ; add r15, r16, r17 ; prefetch r25 }
    5d70:	[0-9a-f]* 	{ move r5, r6 ; seq r15, r16, r17 ; prefetch r25 }
    5d78:	[0-9a-f]* 	{ move r5, r6 ; rl r15, r16, r17 ; lb_u r25, r26 }
    5d80:	[0-9a-f]* 	{ move r5, r6 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    5d88:	[0-9a-f]* 	{ move r5, r6 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    5d90:	[0-9a-f]* 	{ move r5, r6 ; mz r15, r16, r17 ; sb r25, r26 }
    5d98:	[0-9a-f]* 	{ move r5, r6 ; slti r15, r16, 5 ; sb r25, r26 }
    5da0:	[0-9a-f]* 	{ move r5, r6 ; seqh r15, r16, r17 }
    5da8:	[0-9a-f]* 	{ move r5, r6 ; info 19 ; sh r25, r26 }
    5db0:	[0-9a-f]* 	{ move r5, r6 ; slt r15, r16, r17 ; sh r25, r26 }
    5db8:	[0-9a-f]* 	{ move r5, r6 ; shl r15, r16, r17 ; sh r25, r26 }
    5dc0:	[0-9a-f]* 	{ move r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
    5dc8:	[0-9a-f]* 	{ move r5, r6 ; shrih r15, r16, 5 }
    5dd0:	[0-9a-f]* 	{ move r5, r6 ; slt_u r15, r16, r17 }
    5dd8:	[0-9a-f]* 	{ move r5, r6 ; slte_u r15, r16, r17 ; sh r25, r26 }
    5de0:	[0-9a-f]* 	{ move r5, r6 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    5de8:	[0-9a-f]* 	{ move r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
    5df0:	[0-9a-f]* 	{ move r5, r6 ; srai r15, r16, 5 ; lh_u r25, r26 }
    5df8:	[0-9a-f]* 	{ move r5, r6 ; subbs_u r15, r16, r17 }
    5e00:	[0-9a-f]* 	{ move r5, r6 ; rl r15, r16, r17 ; sw r25, r26 }
    5e08:	[0-9a-f]* 	{ move r5, r6 ; sub r15, r16, r17 ; sw r25, r26 }
    5e10:	[0-9a-f]* 	{ movei r15, 5 ; add r5, r6, r7 ; lh_u r25, r26 }
    5e18:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 }
    5e20:	[0-9a-f]* 	{ movei r15, 5 ; andi r5, r6, 5 ; lh r25, r26 }
    5e28:	[0-9a-f]* 	{ bitx r5, r6 ; movei r15, 5 }
    5e30:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 }
    5e38:	[0-9a-f]* 	{ movei r15, 5 ; sb r25, r26 }
    5e40:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 ; lb r25, r26 }
    5e48:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
    5e50:	[0-9a-f]* 	{ movei r15, 5 ; slt r5, r6, r7 ; lb r25, r26 }
    5e58:	[0-9a-f]* 	{ bitx r5, r6 ; movei r15, 5 ; lb_u r25, r26 }
    5e60:	[0-9a-f]* 	{ movei r15, 5 ; mz r5, r6, r7 ; lb_u r25, r26 }
    5e68:	[0-9a-f]* 	{ movei r15, 5 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
    5e70:	[0-9a-f]* 	{ ctz r5, r6 ; movei r15, 5 ; lh r25, r26 }
    5e78:	[0-9a-f]* 	{ movei r15, 5 ; or r5, r6, r7 ; lh r25, r26 }
    5e80:	[0-9a-f]* 	{ movei r15, 5 ; sne r5, r6, r7 ; lh r25, r26 }
    5e88:	[0-9a-f]* 	{ movei r15, 5 ; mnz r5, r6, r7 ; lh_u r25, r26 }
    5e90:	[0-9a-f]* 	{ movei r15, 5 ; rl r5, r6, r7 ; lh_u r25, r26 }
    5e98:	[0-9a-f]* 	{ movei r15, 5 ; sub r5, r6, r7 ; lh_u r25, r26 }
    5ea0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    5ea8:	[0-9a-f]* 	{ movei r15, 5 ; s2a r5, r6, r7 ; lw r25, r26 }
    5eb0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; lw r25, r26 }
    5eb8:	[0-9a-f]* 	{ movei r15, 5 ; mnz r5, r6, r7 ; sh r25, r26 }
    5ec0:	[0-9a-f]* 	{ movei r15, 5 ; movei r5, 5 ; prefetch r25 }
    5ec8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
    5ed0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    5ed8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
    5ee0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    5ee8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
    5ef0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; movei r15, 5 }
    5ef8:	[0-9a-f]* 	{ movei r15, 5 ; mz r5, r6, r7 }
    5f00:	[0-9a-f]* 	{ movei r15, 5 ; nor r5, r6, r7 ; sh r25, r26 }
    5f08:	[0-9a-f]* 	{ movei r15, 5 ; ori r5, r6, 5 ; sh r25, r26 }
    5f10:	[0-9a-f]* 	{ movei r15, 5 ; andi r5, r6, 5 ; prefetch r25 }
    5f18:	[0-9a-f]* 	{ mvz r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    5f20:	[0-9a-f]* 	{ movei r15, 5 ; slte r5, r6, r7 ; prefetch r25 }
    5f28:	[0-9a-f]* 	{ movei r15, 5 ; rl r5, r6, r7 ; sb r25, r26 }
    5f30:	[0-9a-f]* 	{ movei r15, 5 ; s1a r5, r6, r7 ; sb r25, r26 }
    5f38:	[0-9a-f]* 	{ movei r15, 5 ; s3a r5, r6, r7 ; sb r25, r26 }
    5f40:	[0-9a-f]* 	{ movei r15, 5 ; mnz r5, r6, r7 ; sb r25, r26 }
    5f48:	[0-9a-f]* 	{ movei r15, 5 ; rl r5, r6, r7 ; sb r25, r26 }
    5f50:	[0-9a-f]* 	{ movei r15, 5 ; sub r5, r6, r7 ; sb r25, r26 }
    5f58:	[0-9a-f]* 	{ movei r15, 5 ; seqi r5, r6, 5 ; lb_u r25, r26 }
    5f60:	[0-9a-f]* 	{ movei r15, 5 ; info 19 ; sh r25, r26 }
    5f68:	[0-9a-f]* 	{ pcnt r5, r6 ; movei r15, 5 ; sh r25, r26 }
    5f70:	[0-9a-f]* 	{ movei r15, 5 ; srai r5, r6, 5 ; sh r25, r26 }
    5f78:	[0-9a-f]* 	{ movei r15, 5 ; shli r5, r6, 5 ; lb r25, r26 }
    5f80:	[0-9a-f]* 	{ movei r15, 5 ; shr r5, r6, r7 ; sw r25, r26 }
    5f88:	[0-9a-f]* 	{ movei r15, 5 ; slt r5, r6, r7 ; lw r25, r26 }
    5f90:	[0-9a-f]* 	{ movei r15, 5 ; slte r5, r6, r7 ; lh r25, r26 }
    5f98:	[0-9a-f]* 	{ movei r15, 5 ; slteh r5, r6, r7 }
    5fa0:	[0-9a-f]* 	{ movei r15, 5 ; slti_u r5, r6, 5 ; sb r25, r26 }
    5fa8:	[0-9a-f]* 	{ movei r15, 5 ; sra r5, r6, r7 ; lb r25, r26 }
    5fb0:	[0-9a-f]* 	{ movei r15, 5 ; srai r5, r6, 5 ; sw r25, r26 }
    5fb8:	[0-9a-f]* 	{ movei r15, 5 ; addi r5, r6, 5 ; sw r25, r26 }
    5fc0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; sw r25, r26 }
    5fc8:	[0-9a-f]* 	{ movei r15, 5 ; slt r5, r6, r7 ; sw r25, r26 }
    5fd0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; movei r15, 5 ; lw r25, r26 }
    5fd8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; movei r15, 5 ; lw r25, r26 }
    5fe0:	[0-9a-f]* 	{ movei r15, 5 ; xor r5, r6, r7 ; lw r25, r26 }
    5fe8:	[0-9a-f]* 	{ movei r5, 5 ; addhs r15, r16, r17 }
    5ff0:	[0-9a-f]* 	{ movei r5, 5 ; and r15, r16, r17 ; lw r25, r26 }
    5ff8:	[0-9a-f]* 	{ movei r5, 5 ; lb r25, r26 }
    6000:	[0-9a-f]* 	{ movei r5, 5 ; ill }
    6008:	[0-9a-f]* 	{ movei r5, 5 ; jr r15 }
    6010:	[0-9a-f]* 	{ movei r5, 5 ; s1a r15, r16, r17 ; lb r25, r26 }
    6018:	[0-9a-f]* 	{ movei r5, 5 ; lb r25, r26 }
    6020:	[0-9a-f]* 	{ movei r5, 5 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    6028:	[0-9a-f]* 	{ movei r5, 5 ; lbadd r15, r16, 5 }
    6030:	[0-9a-f]* 	{ movei r5, 5 ; s1a r15, r16, r17 ; lh r25, r26 }
    6038:	[0-9a-f]* 	{ movei r5, 5 ; lh r25, r26 }
    6040:	[0-9a-f]* 	{ movei r5, 5 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    6048:	[0-9a-f]* 	{ movei r5, 5 ; lhadd r15, r16, 5 }
    6050:	[0-9a-f]* 	{ movei r5, 5 ; rli r15, r16, 5 ; lw r25, r26 }
    6058:	[0-9a-f]* 	{ movei r5, 5 ; xor r15, r16, r17 ; lw r25, r26 }
    6060:	[0-9a-f]* 	{ movei r5, 5 ; mnz r15, r16, r17 ; lw r25, r26 }
    6068:	[0-9a-f]* 	{ movei r5, 5 ; movei r15, 5 ; lh r25, r26 }
    6070:	[0-9a-f]* 	{ movei r5, 5 ; mz r15, r16, r17 }
    6078:	[0-9a-f]* 	{ movei r5, 5 ; nor r15, r16, r17 ; sh r25, r26 }
    6080:	[0-9a-f]* 	{ movei r5, 5 ; ori r15, r16, 5 ; sh r25, r26 }
    6088:	[0-9a-f]* 	{ movei r5, 5 ; nor r15, r16, r17 ; prefetch r25 }
    6090:	[0-9a-f]* 	{ movei r5, 5 ; sne r15, r16, r17 ; prefetch r25 }
    6098:	[0-9a-f]* 	{ movei r5, 5 ; rli r15, r16, 5 ; lh_u r25, r26 }
    60a0:	[0-9a-f]* 	{ movei r5, 5 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    60a8:	[0-9a-f]* 	{ movei r5, 5 ; and r15, r16, r17 ; sb r25, r26 }
    60b0:	[0-9a-f]* 	{ movei r5, 5 ; shl r15, r16, r17 ; sb r25, r26 }
    60b8:	[0-9a-f]* 	{ movei r5, 5 ; seq r15, r16, r17 ; lh_u r25, r26 }
    60c0:	[0-9a-f]* 	{ movei r5, 5 ; seqih r15, r16, 5 }
    60c8:	[0-9a-f]* 	{ movei r5, 5 ; s2a r15, r16, r17 ; sh r25, r26 }
    60d0:	[0-9a-f]* 	{ movei r5, 5 ; shadd r15, r16, 5 }
    60d8:	[0-9a-f]* 	{ movei r5, 5 ; shli r15, r16, 5 ; sh r25, r26 }
    60e0:	[0-9a-f]* 	{ movei r5, 5 ; shri r15, r16, 5 ; lh_u r25, r26 }
    60e8:	[0-9a-f]* 	{ movei r5, 5 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    60f0:	[0-9a-f]* 	{ movei r5, 5 ; slte r15, r16, r17 }
    60f8:	[0-9a-f]* 	{ movei r5, 5 ; slti r15, r16, 5 ; lh_u r25, r26 }
    6100:	[0-9a-f]* 	{ movei r5, 5 ; sltih_u r15, r16, 5 }
    6108:	[0-9a-f]* 	{ movei r5, 5 ; sra r15, r16, r17 ; sh r25, r26 }
    6110:	[0-9a-f]* 	{ movei r5, 5 ; sub r15, r16, r17 ; lh_u r25, r26 }
    6118:	[0-9a-f]* 	{ movei r5, 5 ; mnz r15, r16, r17 ; sw r25, r26 }
    6120:	[0-9a-f]* 	{ movei r5, 5 ; slt_u r15, r16, r17 ; sw r25, r26 }
    6128:	[0-9a-f]* 	{ movei r5, 5 ; xor r15, r16, r17 ; sb r25, r26 }
    6130:	[0-9a-f]* 	{ moveli r15, 4660 ; auli r5, r6, 4660 }
    6138:	[0-9a-f]* 	{ moveli r15, 4660 ; maxih r5, r6, 5 }
    6140:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; moveli r15, 4660 }
    6148:	[0-9a-f]* 	{ moveli r15, 4660 ; mzh r5, r6, r7 }
    6150:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; moveli r15, 4660 }
    6158:	[0-9a-f]* 	{ moveli r15, 4660 ; slt_u r5, r6, r7 }
    6160:	[0-9a-f]* 	{ moveli r15, 4660 ; sra r5, r6, r7 }
    6168:	[0-9a-f]* 	{ moveli r5, 4660 ; addbs_u r15, r16, r17 }
    6170:	[0-9a-f]* 	{ moveli r5, 4660 ; inthb r15, r16, r17 }
    6178:	[0-9a-f]* 	{ moveli r5, 4660 ; lw_na r15, r16 }
    6180:	[0-9a-f]* 	{ moveli r5, 4660 ; moveli.sn r15, 4660 }
    6188:	[0-9a-f]* 	{ moveli r5, 4660 ; sb r15, r16 }
    6190:	[0-9a-f]* 	{ moveli r5, 4660 ; shrib r15, r16, 5 }
    6198:	[0-9a-f]* 	{ moveli r5, 4660 ; sne r15, r16, r17 }
    61a0:	[0-9a-f]* 	{ moveli r5, 4660 ; xori r15, r16, 5 }
    61a8:	[0-9a-f]* 	{ clz r5, r6 ; moveli.sn r15, 4660 }
    61b0:	[0-9a-f]* 	{ moveli.sn r15, 4660 ; mm r5, r6, r7, 5, 7 }
    61b8:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; moveli.sn r15, 4660 }
    61c0:	[0-9a-f]* 	{ moveli.sn r15, 4660 ; packhb r5, r6, r7 }
    61c8:	[0-9a-f]* 	{ moveli.sn r15, 4660 ; seqih r5, r6, 5 }
    61d0:	[0-9a-f]* 	{ moveli.sn r15, 4660 ; slteb_u r5, r6, r7 }
    61d8:	[0-9a-f]* 	{ moveli.sn r15, 4660 ; sub r5, r6, r7 }
    61e0:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; addli r15, r16, 4660 }
    61e8:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; jalrp r15 }
    61f0:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; mf }
    61f8:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; ori r15, r16, 5 }
    6200:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; sh r15, r16 }
    6208:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; slteb r15, r16, r17 }
    6210:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; sraih r15, r16, 5 }
    6218:	[0-9a-f]* 	{ addih r5, r6, 5 ; mtspr 5, r16 }
    6220:	[0-9a-f]* 	{ infol 4660 ; mtspr 5, r16 }
    6228:	[0-9a-f]* 	{ moveli.sn r5, 4660 ; mtspr 5, r16 }
    6230:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mtspr 5, r16 }
    6238:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mtspr 5, r16 }
    6240:	[0-9a-f]* 	{ shlih r5, r6, 5 ; mtspr 5, r16 }
    6248:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; mtspr 5, r16 }
    6250:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mtspr 5, r16 }
    6258:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
    6260:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    6268:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lb_u r25, r26 }
    6270:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; info 19 ; lb r25, r26 }
    6278:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; jrp r15 }
    6280:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    6288:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lb_u r15, r16 }
    6290:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    6298:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lbadd_u r15, r16, 5 }
    62a0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    62a8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lh_u r15, r16 }
    62b0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    62b8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lhadd_u r15, r16, 5 }
    62c0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    62c8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; lw r25, r26 }
    62d0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    62d8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    62e0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mzb r15, r16, r17 }
    62e8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    62f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    62f8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    6300:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    6308:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
    6310:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    6318:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; andi r15, r16, 5 ; sb r25, r26 }
    6320:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    6328:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    6330:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sh r15, r16 }
    6338:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    6340:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    6348:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    6350:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    6358:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
    6360:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
    6368:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    6370:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    6378:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
    6380:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    6388:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    6390:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    6398:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    63a0:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; flush r15 }
    63a8:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; lh r15, r16 }
    63b0:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; mnz r15, r16, r17 }
    63b8:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; raise }
    63c0:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; shlib r15, r16, 5 }
    63c8:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; slti r15, r16, 5 }
    63d0:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; subs r15, r16, r17 }
    63d8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; addhs r15, r16, r17 }
    63e0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; and r15, r16, r17 ; lw r25, r26 }
    63e8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; lb r25, r26 }
    63f0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; ill }
    63f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; jr r15 }
    6400:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s1a r15, r16, r17 ; lb r25, r26 }
    6408:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; lb r25, r26 }
    6410:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    6418:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; lbadd r15, r16, 5 }
    6420:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
    6428:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; lh r25, r26 }
    6430:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    6438:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; lhadd r15, r16, 5 }
    6440:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
    6448:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    6450:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    6458:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
    6460:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mz r15, r16, r17 }
    6468:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; sh r25, r26 }
    6470:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    6478:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    6480:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sne r15, r16, r17 ; prefetch r25 }
    6488:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; rli r15, r16, 5 ; lh_u r25, r26 }
    6490:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    6498:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    64a0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    64a8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; seq r15, r16, r17 ; lh_u r25, r26 }
    64b0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; seqih r15, r16, 5 }
    64b8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
    64c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shadd r15, r16, 5 }
    64c8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    64d0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shri r15, r16, 5 ; lh_u r25, r26 }
    64d8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    64e0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slte r15, r16, r17 }
    64e8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
    64f0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sltih_u r15, r16, 5 }
    64f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
    6500:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    6508:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    6510:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    6518:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
    6520:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; addi r15, r16, 5 ; lb_u r25, r26 }
    6528:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    6530:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lh r25, r26 }
    6538:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    6540:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lb r15, r16 }
    6548:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s3a r15, r16, r17 ; lb r25, r26 }
    6550:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
    6558:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    6560:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lh r15, r16 }
    6568:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    6570:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; add r15, r16, r17 ; lh_u r25, r26 }
    6578:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; lh_u r25, r26 }
    6580:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lnk r15 }
    6588:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    6590:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; lw_na r15, r16 }
    6598:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    65a0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    65a8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mzh r15, r16, r17 }
    65b0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; nor r15, r16, r17 }
    65b8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 }
    65c0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    65c8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    65d0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    65d8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
    65e0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sb r25, r26 }
    65e8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    65f0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    65f8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
    6600:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
    6608:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
    6610:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shli r15, r16, 5 }
    6618:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
    6620:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    6628:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    6630:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    6638:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
    6640:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sra r15, r16, r17 }
    6648:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    6650:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; movei r15, 5 ; sw r25, r26 }
    6658:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    6660:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    6668:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 }
    6670:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; lh_u r15, r16 }
    6678:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; mnzb r15, r16, r17 }
    6680:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; rl r15, r16, r17 }
    6688:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; shlih r15, r16, 5 }
    6690:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; slti_u r15, r16, 5 }
    6698:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; sw r15, r16 }
    66a0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
    66a8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    66b0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lb_u r25, r26 }
    66b8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; info 19 ; lb r25, r26 }
    66c0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; jrp r15 }
    66c8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    66d0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lb_u r15, r16 }
    66d8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    66e0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lbadd_u r15, r16, 5 }
    66e8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    66f0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lh_u r15, r16 }
    66f8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    6700:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lhadd_u r15, r16, 5 }
    6708:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    6710:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; lw r25, r26 }
    6718:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    6720:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    6728:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mzb r15, r16, r17 }
    6730:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    6738:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    6740:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    6748:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    6750:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
    6758:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    6760:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; andi r15, r16, 5 ; sb r25, r26 }
    6768:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    6770:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    6778:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sh r15, r16 }
    6780:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    6788:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    6790:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    6798:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    67a0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
    67a8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
    67b0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    67b8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    67c0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
    67c8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    67d0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    67d8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    67e0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    67e8:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; flush r15 }
    67f0:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; lh r15, r16 }
    67f8:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; mnz r15, r16, r17 }
    6800:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; raise }
    6808:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; shlib r15, r16, 5 }
    6810:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; slti r15, r16, 5 }
    6818:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; subs r15, r16, r17 }
    6820:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; auli r15, r16, 4660 }
    6828:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; lb_u r15, r16 }
    6830:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; minib_u r15, r16, 5 }
    6838:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; packhs r15, r16, r17 }
    6840:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; shlb r15, r16, r17 }
    6848:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; slteh_u r15, r16, r17 }
    6850:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; subbs_u r15, r16, r17 }
    6858:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; adds r15, r16, r17 }
    6860:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; jr r15 }
    6868:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; mfspr r16, 5 }
    6870:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; ori r15, r16, 5 }
    6878:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; sh r15, r16 }
    6880:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; slteb r15, r16, r17 }
    6888:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; sraih r15, r16, 5 }
    6890:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; addih r15, r16, 5 }
    6898:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; iret }
    68a0:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; maxib_u r15, r16, 5 }
    68a8:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; nop }
    68b0:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; seqi r15, r16, 5 }
    68b8:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; sltb_u r15, r16, r17 }
    68c0:	[0-9a-f]* 	{ mulhl_us r5, r6, r7 ; srah r15, r16, r17 }
    68c8:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; addhs r15, r16, r17 }
    68d0:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; intlb r15, r16, r17 }
    68d8:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; lwadd_na r15, r16, 5 }
    68e0:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; mz r15, r16, r17 }
    68e8:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; seq r15, r16, r17 }
    68f0:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; slt r15, r16, r17 }
    68f8:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; sneh r15, r16, r17 }
    6900:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; addb r15, r16, r17 }
    6908:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; infol 4660 }
    6910:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; lw r15, r16 }
    6918:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; moveli r15, 4660 }
    6920:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; s3a r15, r16, r17 }
    6928:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; shri r15, r16, 5 }
    6930:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; sltih_u r15, r16, 5 }
    6938:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; xor r15, r16, r17 }
    6940:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; icoh r15 }
    6948:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; lhadd r15, r16, 5 }
    6950:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; mnzh r15, r16, r17 }
    6958:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; rli r15, r16, 5 }
    6960:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; shr r15, r16, r17 }
    6968:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; sltib r15, r16, 5 }
    6970:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; swadd r15, r16, 5 }
    6978:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; finv r15 }
    6980:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; lbadd_u r15, r16, 5 }
    6988:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
    6990:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; prefetch r15 }
    6998:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; shli r15, r16, 5 }
    69a0:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; slth_u r15, r16, r17 }
    69a8:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; subhs r15, r16, r17 }
    69b0:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; andi r15, r16, 5 }
    69b8:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; lb r15, r16 }
    69c0:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; minh r15, r16, r17 }
    69c8:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; packhb r15, r16, r17 }
    69d0:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; shl r15, r16, r17 }
    69d8:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; slteh r15, r16, r17 }
    69e0:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; subb r15, r16, r17 }
    69e8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; add r15, r16, r17 }
    69f0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    69f8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; auli r15, r16, 4660 }
    6a00:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ill ; prefetch r25 }
    6a08:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; inv r15 }
    6a10:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; or r15, r16, r17 ; lb r25, r26 }
    6a18:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
    6a20:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    6a28:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    6a30:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    6a38:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sra r15, r16, r17 ; lh r25, r26 }
    6a40:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    6a48:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    6a50:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    6a58:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    6a60:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    6a68:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    6a70:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    6a78:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    6a80:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    6a88:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    6a90:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    6a98:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; rl r15, r16, r17 }
    6aa0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s1a r15, r16, r17 }
    6aa8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 }
    6ab0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 ; sb r25, r26 }
    6ab8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sbadd r15, r16, 5 }
    6ac0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    6ac8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    6ad0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    6ad8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    6ae0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shrh r15, r16, r17 }
    6ae8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    6af0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    6af8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slth_u r15, r16, r17 }
    6b00:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slti_u r15, r16, 5 }
    6b08:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sra r15, r16, r17 ; lh_u r25, r26 }
    6b10:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sraih r15, r16, 5 }
    6b18:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
    6b20:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    6b28:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
    6b30:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; addbs_u r15, r16, r17 }
    6b38:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    6b40:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; finv r15 }
    6b48:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ill ; sh r25, r26 }
    6b50:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; jalr r15 }
    6b58:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rl r15, r16, r17 ; lb r25, r26 }
    6b60:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    6b68:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    6b70:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lb_u r25, r26 }
    6b78:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    6b80:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    6b88:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lh_u r25, r26 }
    6b90:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    6b98:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    6ba0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    6ba8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh r25, r26 }
    6bb0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
    6bb8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
    6bc0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    6bc8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    6bd0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    6bd8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    6be0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    6be8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    6bf0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    6bf8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; sb r25, r26 }
    6c00:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    6c08:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seqi r15, r16, 5 }
    6c10:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    6c18:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    6c20:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    6c28:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shri r15, r16, 5 ; lb_u r25, r26 }
    6c30:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt r15, r16, r17 }
    6c38:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte r15, r16, r17 ; sh r25, r26 }
    6c40:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
    6c48:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sltib_u r15, r16, 5 }
    6c50:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    6c58:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    6c60:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; ill ; sw r25, r26 }
    6c68:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    6c70:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    6c78:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; auli r15, r16, 4660 }
    6c80:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; lb_u r15, r16 }
    6c88:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; minib_u r15, r16, 5 }
    6c90:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; packhs r15, r16, r17 }
    6c98:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; shlb r15, r16, r17 }
    6ca0:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; slteh_u r15, r16, r17 }
    6ca8:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; subbs_u r15, r16, r17 }
    6cb0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; addb r15, r16, r17 }
    6cb8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    6cc0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; dtlbpr r15 }
    6cc8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ill ; sb r25, r26 }
    6cd0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; iret }
    6cd8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    6ce0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    6ce8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    6cf0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    6cf8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    6d00:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; srai r15, r16, 5 ; lh r25, r26 }
    6d08:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    6d10:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    6d18:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    6d20:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    6d28:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    6d30:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; move r15, r16 }
    6d38:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    6d40:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    6d48:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    6d50:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    6d58:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    6d60:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    6d68:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    6d70:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sb r15, r16 }
    6d78:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    6d80:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    6d88:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    6d90:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    6d98:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    6da0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    6da8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    6db0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    6db8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    6dc0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    6dc8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sltib r15, r16, 5 }
    6dd0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    6dd8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    6de0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sw r25, r26 }
    6de8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
    6df0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    6df8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; addh r15, r16, r17 }
    6e00:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    6e08:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; flush r15 }
    6e10:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; ill ; sw r25, r26 }
    6e18:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; jalrp r15 }
    6e20:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    6e28:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; xor r15, r16, r17 ; lb r25, r26 }
    6e30:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    6e38:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; lb_u r25, r26 }
    6e40:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    6e48:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
    6e50:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    6e58:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; lh_u r25, r26 }
    6e60:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    6e68:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    6e70:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    6e78:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    6e80:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
    6e88:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    6e90:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
    6e98:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; nop ; prefetch r25 }
    6ea0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    6ea8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    6eb0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    6eb8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; addi r15, r16, 5 ; sb r25, r26 }
    6ec0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seqi r15, r16, 5 ; sb r25, r26 }
    6ec8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    6ed0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seqib r15, r16, 5 }
    6ed8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
    6ee0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sh r25, r26 }
    6ee8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    6ef0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    6ef8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    6f00:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    6f08:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    6f10:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sltih r15, r16, 5 }
    6f18:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    6f20:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    6f28:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; info 19 ; sw r25, r26 }
    6f30:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    6f38:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    6f40:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; dtlbpr r15 }
    6f48:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; lbadd r15, r16, 5 }
    6f50:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; minih r15, r16, 5 }
    6f58:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; packlb r15, r16, r17 }
    6f60:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; shlh r15, r16, r17 }
    6f68:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; slth r15, r16, r17 }
    6f70:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; subh r15, r16, r17 }
    6f78:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; addbs_u r15, r16, r17 }
    6f80:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    6f88:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; finv r15 }
    6f90:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ill ; sh r25, r26 }
    6f98:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; jalr r15 }
    6fa0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rl r15, r16, r17 ; lb r25, r26 }
    6fa8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    6fb0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    6fb8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; lb_u r25, r26 }
    6fc0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    6fc8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    6fd0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; lh_u r25, r26 }
    6fd8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    6fe0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    6fe8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    6ff0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mnz r15, r16, r17 ; lh r25, r26 }
    6ff8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
    7000:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
    7008:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    7010:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    7018:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    7020:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    7028:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    7030:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    7038:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    7040:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; seq r15, r16, r17 ; sb r25, r26 }
    7048:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    7050:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; seqi r15, r16, 5 }
    7058:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    7060:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    7068:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    7070:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shri r15, r16, 5 ; lb_u r25, r26 }
    7078:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt r15, r16, r17 }
    7080:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slte r15, r16, r17 ; sh r25, r26 }
    7088:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
    7090:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sltib_u r15, r16, 5 }
    7098:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    70a0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    70a8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ill ; sw r25, r26 }
    70b0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    70b8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    70c0:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; auli r15, r16, 4660 }
    70c8:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; lb_u r15, r16 }
    70d0:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; minib_u r15, r16, 5 }
    70d8:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; packhs r15, r16, r17 }
    70e0:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; shlb r15, r16, r17 }
    70e8:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; slteh_u r15, r16, r17 }
    70f0:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; subbs_u r15, r16, r17 }
    70f8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; addb r15, r16, r17 }
    7100:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    7108:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; dtlbpr r15 }
    7110:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ill ; sb r25, r26 }
    7118:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; iret }
    7120:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    7128:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    7130:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    7138:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    7140:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    7148:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; srai r15, r16, 5 ; lh r25, r26 }
    7150:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    7158:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    7160:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    7168:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    7170:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    7178:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; move r15, r16 }
    7180:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    7188:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    7190:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    7198:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    71a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    71a8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    71b0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    71b8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sb r15, r16 }
    71c0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    71c8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    71d0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    71d8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    71e0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    71e8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    71f0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    71f8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    7200:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    7208:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    7210:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sltib r15, r16, 5 }
    7218:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    7220:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    7228:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; sw r25, r26 }
    7230:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
    7238:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    7240:	[0-9a-f]* 	{ mvz r5, r6, r7 ; addh r15, r16, r17 }
    7248:	[0-9a-f]* 	{ mvz r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    7250:	[0-9a-f]* 	{ mvz r5, r6, r7 ; flush r15 }
    7258:	[0-9a-f]* 	{ mvz r5, r6, r7 ; ill ; sw r25, r26 }
    7260:	[0-9a-f]* 	{ mvz r5, r6, r7 ; jalrp r15 }
    7268:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    7270:	[0-9a-f]* 	{ mvz r5, r6, r7 ; xor r15, r16, r17 ; lb r25, r26 }
    7278:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    7280:	[0-9a-f]* 	{ mvz r5, r6, r7 ; lb_u r25, r26 }
    7288:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    7290:	[0-9a-f]* 	{ mvz r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
    7298:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    72a0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; lh_u r25, r26 }
    72a8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    72b0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    72b8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    72c0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    72c8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
    72d0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    72d8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
    72e0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nop ; prefetch r25 }
    72e8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    72f0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
    72f8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    7300:	[0-9a-f]* 	{ mvz r5, r6, r7 ; addi r15, r16, 5 ; sb r25, r26 }
    7308:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seqi r15, r16, 5 ; sb r25, r26 }
    7310:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    7318:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seqib r15, r16, 5 }
    7320:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
    7328:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sh r25, r26 }
    7330:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    7338:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
    7340:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    7348:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    7350:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    7358:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sltih r15, r16, 5 }
    7360:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    7368:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
    7370:	[0-9a-f]* 	{ mvz r5, r6, r7 ; info 19 ; sw r25, r26 }
    7378:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    7380:	[0-9a-f]* 	{ mvz r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    7388:	[0-9a-f]* 	{ mz r15, r16, r17 ; addi r5, r6, 5 ; lb r25, r26 }
    7390:	[0-9a-f]* 	{ mz r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
    7398:	[0-9a-f]* 	{ bitx r5, r6 ; mz r15, r16, r17 ; lb r25, r26 }
    73a0:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; lb r25, r26 }
    73a8:	[0-9a-f]* 	{ ctz r5, r6 ; mz r15, r16, r17 ; sw r25, r26 }
    73b0:	[0-9a-f]* 	{ mz r15, r16, r17 ; info 19 ; sh r25, r26 }
    73b8:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; lb r25, r26 }
    73c0:	[0-9a-f]* 	{ mz r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
    73c8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; lb r25, r26 }
    73d0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mz r15, r16, r17 ; lb_u r25, r26 }
    73d8:	[0-9a-f]* 	{ mz r15, r16, r17 ; seq r5, r6, r7 ; lb_u r25, r26 }
    73e0:	[0-9a-f]* 	{ mz r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
    73e8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    73f0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shli r5, r6, 5 ; lh r25, r26 }
    73f8:	[0-9a-f]* 	{ mz r15, r16, r17 ; addi r5, r6, 5 ; lh_u r25, r26 }
    7400:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    7408:	[0-9a-f]* 	{ mz r15, r16, r17 ; slt r5, r6, r7 ; lh_u r25, r26 }
    7410:	[0-9a-f]* 	{ bitx r5, r6 ; mz r15, r16, r17 ; lw r25, r26 }
    7418:	[0-9a-f]* 	{ mz r15, r16, r17 ; mz r5, r6, r7 ; lw r25, r26 }
    7420:	[0-9a-f]* 	{ mz r15, r16, r17 ; slte_u r5, r6, r7 ; lw r25, r26 }
    7428:	[0-9a-f]* 	{ mz r15, r16, r17 ; minih r5, r6, 5 }
    7430:	[0-9a-f]* 	{ mz r15, r16, r17 ; move r5, r6 ; sb r25, r26 }
    7438:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    7440:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    7448:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; mz r15, r16, r17 }
    7450:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    7458:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    7460:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    7468:	[0-9a-f]* 	{ mz r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
    7470:	[0-9a-f]* 	{ mz r15, r16, r17 ; nop ; sw r25, r26 }
    7478:	[0-9a-f]* 	{ mz r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
    7480:	[0-9a-f]* 	{ pcnt r5, r6 ; mz r15, r16, r17 ; lw r25, r26 }
    7488:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    7490:	[0-9a-f]* 	{ mz r15, r16, r17 ; s3a r5, r6, r7 ; prefetch r25 }
    7498:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    74a0:	[0-9a-f]* 	{ mz r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
    74a8:	[0-9a-f]* 	{ mz r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    74b0:	[0-9a-f]* 	{ mz r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
    74b8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    74c0:	[0-9a-f]* 	{ mz r15, r16, r17 ; slt r5, r6, r7 ; sb r25, r26 }
    74c8:	[0-9a-f]* 	{ mz r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
    74d0:	[0-9a-f]* 	{ mz r15, r16, r17 ; add r5, r6, r7 ; sh r25, r26 }
    74d8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
    74e0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
    74e8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shl r5, r6, r7 ; lh_u r25, r26 }
    74f0:	[0-9a-f]* 	{ mz r15, r16, r17 ; shlih r5, r6, 5 }
    74f8:	[0-9a-f]* 	{ mz r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
    7500:	[0-9a-f]* 	{ mz r15, r16, r17 ; slt_u r5, r6, r7 ; prefetch r25 }
    7508:	[0-9a-f]* 	{ mz r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    7510:	[0-9a-f]* 	{ mz r15, r16, r17 ; slti r5, r6, 5 ; sh r25, r26 }
    7518:	[0-9a-f]* 	{ mz r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
    7520:	[0-9a-f]* 	{ mz r15, r16, r17 ; srah r5, r6, r7 }
    7528:	[0-9a-f]* 	{ mz r15, r16, r17 ; sub r5, r6, r7 ; sh r25, r26 }
    7530:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; sw r25, r26 }
    7538:	[0-9a-f]* 	{ mz r15, r16, r17 ; s1a r5, r6, r7 ; sw r25, r26 }
    7540:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; sw r25, r26 }
    7548:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    7550:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    7558:	[0-9a-f]* 	{ mz r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    7560:	[0-9a-f]* 	{ mz r5, r6, r7 ; addib r15, r16, 5 }
    7568:	[0-9a-f]* 	{ mz r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    7570:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; lb r25, r26 }
    7578:	[0-9a-f]* 	{ mz r5, r6, r7 ; infol 4660 }
    7580:	[0-9a-f]* 	{ mz r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
    7588:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    7590:	[0-9a-f]* 	{ mz r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
    7598:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    75a0:	[0-9a-f]* 	{ mz r5, r6, r7 ; move r15, r16 ; lh r25, r26 }
    75a8:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    75b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    75b8:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    75c0:	[0-9a-f]* 	{ mz r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
    75c8:	[0-9a-f]* 	{ mz r5, r6, r7 ; slt_u r15, r16, r17 ; lw r25, r26 }
    75d0:	[0-9a-f]* 	{ mz r5, r6, r7 ; minb_u r15, r16, r17 }
    75d8:	[0-9a-f]* 	{ mz r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
    75e0:	[0-9a-f]* 	{ mz r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    75e8:	[0-9a-f]* 	{ mz r5, r6, r7 ; nop ; sw r25, r26 }
    75f0:	[0-9a-f]* 	{ mz r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
    75f8:	[0-9a-f]* 	{ mz r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    7600:	[0-9a-f]* 	{ mz r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    7608:	[0-9a-f]* 	{ mz r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    7610:	[0-9a-f]* 	{ mz r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    7618:	[0-9a-f]* 	{ mz r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
    7620:	[0-9a-f]* 	{ mz r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    7628:	[0-9a-f]* 	{ mz r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    7630:	[0-9a-f]* 	{ mz r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
    7638:	[0-9a-f]* 	{ mz r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    7640:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    7648:	[0-9a-f]* 	{ mz r5, r6, r7 ; shlb r15, r16, r17 }
    7650:	[0-9a-f]* 	{ mz r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    7658:	[0-9a-f]* 	{ mz r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
    7660:	[0-9a-f]* 	{ mz r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    7668:	[0-9a-f]* 	{ mz r5, r6, r7 ; slteb r15, r16, r17 }
    7670:	[0-9a-f]* 	{ mz r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
    7678:	[0-9a-f]* 	{ mz r5, r6, r7 ; sneb r15, r16, r17 }
    7680:	[0-9a-f]* 	{ mz r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
    7688:	[0-9a-f]* 	{ mz r5, r6, r7 ; subs r15, r16, r17 }
    7690:	[0-9a-f]* 	{ mz r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    7698:	[0-9a-f]* 	{ mz r5, r6, r7 ; swadd r15, r16, 5 }
    76a0:	[0-9a-f]* 	{ mzb r15, r16, r17 ; addib r5, r6, 5 }
    76a8:	[0-9a-f]* 	{ mzb r15, r16, r17 ; info 19 }
    76b0:	[0-9a-f]* 	{ mzb r15, r16, r17 ; moveli r5, 4660 }
    76b8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; mzb r15, r16, r17 }
    76c0:	[0-9a-f]* 	{ mzb r15, r16, r17 ; rli r5, r6, 5 }
    76c8:	[0-9a-f]* 	{ mzb r15, r16, r17 ; shlib r5, r6, 5 }
    76d0:	[0-9a-f]* 	{ mzb r15, r16, r17 ; slti r5, r6, 5 }
    76d8:	[0-9a-f]* 	{ mzb r15, r16, r17 ; subs r5, r6, r7 }
    76e0:	[0-9a-f]* 	{ mzb r5, r6, r7 ; auli r15, r16, 4660 }
    76e8:	[0-9a-f]* 	{ mzb r5, r6, r7 ; lb_u r15, r16 }
    76f0:	[0-9a-f]* 	{ mzb r5, r6, r7 ; minib_u r15, r16, 5 }
    76f8:	[0-9a-f]* 	{ mzb r5, r6, r7 ; packhs r15, r16, r17 }
    7700:	[0-9a-f]* 	{ mzb r5, r6, r7 ; shlb r15, r16, r17 }
    7708:	[0-9a-f]* 	{ mzb r5, r6, r7 ; slteh_u r15, r16, r17 }
    7710:	[0-9a-f]* 	{ mzb r5, r6, r7 ; subbs_u r15, r16, r17 }
    7718:	[0-9a-f]* 	{ mzh r15, r16, r17 ; adds r5, r6, r7 }
    7720:	[0-9a-f]* 	{ mzh r15, r16, r17 ; intlb r5, r6, r7 }
    7728:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; mzh r15, r16, r17 }
    7730:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; mzh r15, r16, r17 }
    7738:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; mzh r15, r16, r17 }
    7740:	[0-9a-f]* 	{ mzh r15, r16, r17 ; shrh r5, r6, r7 }
    7748:	[0-9a-f]* 	{ mzh r15, r16, r17 ; sltih r5, r6, 5 }
    7750:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mzh r15, r16, r17 }
    7758:	[0-9a-f]* 	{ mzh r5, r6, r7 }
    7760:	[0-9a-f]* 	{ mzh r5, r6, r7 ; lh_u r15, r16 }
    7768:	[0-9a-f]* 	{ mzh r5, r6, r7 ; mnzb r15, r16, r17 }
    7770:	[0-9a-f]* 	{ mzh r5, r6, r7 ; rl r15, r16, r17 }
    7778:	[0-9a-f]* 	{ mzh r5, r6, r7 ; shlih r15, r16, 5 }
    7780:	[0-9a-f]* 	{ mzh r5, r6, r7 ; slti_u r15, r16, 5 }
    7788:	[0-9a-f]* 	{ mzh r5, r6, r7 ; sw r15, r16 }
    7790:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; lh_u r25, r26 }
    7798:	[0-9a-f]* 	{ nop ; addi r15, r16, 5 ; prefetch r25 }
    77a0:	[0-9a-f]* 	{ nop ; addli r5, r6, 4660 }
    77a8:	[0-9a-f]* 	{ nop ; and r5, r6, r7 ; lh_u r25, r26 }
    77b0:	[0-9a-f]* 	{ nop ; andi r5, r6, 5 ; lh_u r25, r26 }
    77b8:	[0-9a-f]* 	{ bitx r5, r6 ; nop }
    77c0:	[0-9a-f]* 	{ clz r5, r6 ; nop ; sw r25, r26 }
    77c8:	[0-9a-f]* 	{ nop ; lb_u r25, r26 }
    77d0:	[0-9a-f]* 	{ nop ; info 19 ; lb r25, r26 }
    77d8:	[0-9a-f]* 	{ nop ; iret }
    77e0:	[0-9a-f]* 	{ nop ; info 19 ; lb r25, r26 }
    77e8:	[0-9a-f]* 	{ nop ; nop ; lb r25, r26 }
    77f0:	[0-9a-f]* 	{ nop ; seqi r15, r16, 5 ; lb r25, r26 }
    77f8:	[0-9a-f]* 	{ nop ; slti_u r15, r16, 5 ; lb r25, r26 }
    7800:	[0-9a-f]* 	{ nop ; addi r15, r16, 5 ; lb_u r25, r26 }
    7808:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nop ; lb_u r25, r26 }
    7810:	[0-9a-f]* 	{ nop ; rl r15, r16, r17 ; lb_u r25, r26 }
    7818:	[0-9a-f]* 	{ nop ; shri r15, r16, 5 ; lb_u r25, r26 }
    7820:	[0-9a-f]* 	{ nop ; sub r15, r16, r17 ; lb_u r25, r26 }
    7828:	[0-9a-f]* 	{ bitx r5, r6 ; nop ; lh r25, r26 }
    7830:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; nop ; lh r25, r26 }
    7838:	[0-9a-f]* 	{ nop ; s2a r15, r16, r17 ; lh r25, r26 }
    7840:	[0-9a-f]* 	{ nop ; slte r15, r16, r17 ; lh r25, r26 }
    7848:	[0-9a-f]* 	{ nop ; xor r15, r16, r17 ; lh r25, r26 }
    7850:	[0-9a-f]* 	{ nop ; mnz r5, r6, r7 ; lh_u r25, r26 }
    7858:	[0-9a-f]* 	{ nop ; nor r5, r6, r7 ; lh_u r25, r26 }
    7860:	[0-9a-f]* 	{ nop ; shl r15, r16, r17 ; lh_u r25, r26 }
    7868:	[0-9a-f]* 	{ nop ; sne r15, r16, r17 ; lh_u r25, r26 }
    7870:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; lw r25, r26 }
    7878:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; nop ; lw r25, r26 }
    7880:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; lw r25, r26 }
    7888:	[0-9a-f]* 	{ nop ; shr r5, r6, r7 ; lw r25, r26 }
    7890:	[0-9a-f]* 	{ nop ; srai r5, r6, 5 ; lw r25, r26 }
    7898:	[0-9a-f]* 	{ nop ; maxih r5, r6, 5 }
    78a0:	[0-9a-f]* 	{ nop ; mnz r15, r16, r17 ; sh r25, r26 }
    78a8:	[0-9a-f]* 	{ nop ; move r15, r16 ; lh_u r25, r26 }
    78b0:	[0-9a-f]* 	{ nop ; movei r15, 5 ; lh_u r25, r26 }
    78b8:	[0-9a-f]* 	{ nop ; moveli.sn r5, 4660 }
    78c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nop ; sh r25, r26 }
    78c8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; nop ; sb r25, r26 }
    78d0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nop ; sh r25, r26 }
    78d8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; nop ; sb r25, r26 }
    78e0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; nop ; prefetch r25 }
    78e8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nop ; lw r25, r26 }
    78f0:	[0-9a-f]* 	{ nop ; mz r5, r6, r7 ; lw r25, r26 }
    78f8:	[0-9a-f]* 	{ nop ; nop }
    7900:	[0-9a-f]* 	{ nop ; nor r5, r6, r7 }
    7908:	[0-9a-f]* 	{ nop ; or r5, r6, r7 }
    7910:	[0-9a-f]* 	{ nop ; ori r5, r6, 5 }
    7918:	[0-9a-f]* 	{ nop ; add r15, r16, r17 ; prefetch r25 }
    7920:	[0-9a-f]* 	{ nop ; movei r5, 5 ; prefetch r25 }
    7928:	[0-9a-f]* 	{ nop ; ori r5, r6, 5 ; prefetch r25 }
    7930:	[0-9a-f]* 	{ nop ; shr r15, r16, r17 ; prefetch r25 }
    7938:	[0-9a-f]* 	{ nop ; srai r15, r16, 5 ; prefetch r25 }
    7940:	[0-9a-f]* 	{ nop ; rl r15, r16, r17 ; sw r25, r26 }
    7948:	[0-9a-f]* 	{ nop ; rli r15, r16, 5 ; sw r25, r26 }
    7950:	[0-9a-f]* 	{ nop ; s1a r15, r16, r17 ; sw r25, r26 }
    7958:	[0-9a-f]* 	{ nop ; s2a r15, r16, r17 ; sw r25, r26 }
    7960:	[0-9a-f]* 	{ nop ; s3a r15, r16, r17 ; sw r25, r26 }
    7968:	[0-9a-f]* 	{ nop ; add r5, r6, r7 ; sb r25, r26 }
    7970:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; nop ; sb r25, r26 }
    7978:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; sb r25, r26 }
    7980:	[0-9a-f]* 	{ nop ; shr r5, r6, r7 ; sb r25, r26 }
    7988:	[0-9a-f]* 	{ nop ; srai r5, r6, 5 ; sb r25, r26 }
    7990:	[0-9a-f]* 	{ nop ; seq r15, r16, r17 }
    7998:	[0-9a-f]* 	{ nop ; seqi r15, r16, 5 ; prefetch r25 }
    79a0:	[0-9a-f]* 	{ nop ; add r15, r16, r17 ; sh r25, r26 }
    79a8:	[0-9a-f]* 	{ nop ; movei r5, 5 ; sh r25, r26 }
    79b0:	[0-9a-f]* 	{ nop ; ori r5, r6, 5 ; sh r25, r26 }
    79b8:	[0-9a-f]* 	{ nop ; shr r15, r16, r17 ; sh r25, r26 }
    79c0:	[0-9a-f]* 	{ nop ; srai r15, r16, 5 ; sh r25, r26 }
    79c8:	[0-9a-f]* 	{ nop ; shl r15, r16, r17 ; sw r25, r26 }
    79d0:	[0-9a-f]* 	{ nop ; shli r15, r16, 5 ; lw r25, r26 }
    79d8:	[0-9a-f]* 	{ nop ; shr r15, r16, r17 ; lb r25, r26 }
    79e0:	[0-9a-f]* 	{ nop ; shrb r15, r16, r17 }
    79e8:	[0-9a-f]* 	{ nop ; shri r5, r6, 5 ; sb r25, r26 }
    79f0:	[0-9a-f]* 	{ nop ; slt r5, r6, r7 ; lh r25, r26 }
    79f8:	[0-9a-f]* 	{ nop ; slt_u r5, r6, r7 ; lh r25, r26 }
    7a00:	[0-9a-f]* 	{ nop ; slte r15, r16, r17 ; sw r25, r26 }
    7a08:	[0-9a-f]* 	{ nop ; slte_u r15, r16, r17 ; sw r25, r26 }
    7a10:	[0-9a-f]* 	{ nop ; slth r15, r16, r17 }
    7a18:	[0-9a-f]* 	{ nop ; slti r5, r6, 5 ; sb r25, r26 }
    7a20:	[0-9a-f]* 	{ nop ; slti_u r5, r6, 5 ; sb r25, r26 }
    7a28:	[0-9a-f]* 	{ nop ; sne r15, r16, r17 ; sw r25, r26 }
    7a30:	[0-9a-f]* 	{ nop ; sra r15, r16, r17 ; lw r25, r26 }
    7a38:	[0-9a-f]* 	{ nop ; srai r15, r16, 5 ; lb r25, r26 }
    7a40:	[0-9a-f]* 	{ nop ; sraib r15, r16, 5 }
    7a48:	[0-9a-f]* 	{ nop ; sub r5, r6, r7 ; sb r25, r26 }
    7a50:	[0-9a-f]* 	{ nop ; and r5, r6, r7 ; sw r25, r26 }
    7a58:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nop ; sw r25, r26 }
    7a60:	[0-9a-f]* 	{ nop ; rli r5, r6, 5 ; sw r25, r26 }
    7a68:	[0-9a-f]* 	{ nop ; slt r5, r6, r7 ; sw r25, r26 }
    7a70:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nop ; sw r25, r26 }
    7a78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nop }
    7a80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nop }
    7a88:	[0-9a-f]* 	{ nop ; xor r15, r16, r17 ; sh r25, r26 }
    7a90:	[0-9a-f]* 	{ nor r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
    7a98:	[0-9a-f]* 	{ nor r15, r16, r17 ; addih r5, r6, 5 }
    7aa0:	[0-9a-f]* 	{ nor r15, r16, r17 ; andi r5, r6, 5 ; lw r25, r26 }
    7aa8:	[0-9a-f]* 	{ bytex r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    7ab0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; nor r15, r16, r17 }
    7ab8:	[0-9a-f]* 	{ nor r15, r16, r17 ; sw r25, r26 }
    7ac0:	[0-9a-f]* 	{ nor r15, r16, r17 ; andi r5, r6, 5 ; lb r25, r26 }
    7ac8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    7ad0:	[0-9a-f]* 	{ nor r15, r16, r17 ; slte r5, r6, r7 ; lb r25, r26 }
    7ad8:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    7ae0:	[0-9a-f]* 	{ nor r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    7ae8:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti_u r5, r6, 5 ; lb_u r25, r26 }
    7af0:	[0-9a-f]* 	{ nor r15, r16, r17 ; info 19 ; lh r25, r26 }
    7af8:	[0-9a-f]* 	{ pcnt r5, r6 ; nor r15, r16, r17 ; lh r25, r26 }
    7b00:	[0-9a-f]* 	{ nor r15, r16, r17 ; srai r5, r6, 5 ; lh r25, r26 }
    7b08:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; lh_u r25, r26 }
    7b10:	[0-9a-f]* 	{ nor r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    7b18:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; lh_u r25, r26 }
    7b20:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    7b28:	[0-9a-f]* 	{ nor r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
    7b30:	[0-9a-f]* 	{ nor r15, r16, r17 ; xor r5, r6, r7 ; lw r25, r26 }
    7b38:	[0-9a-f]* 	{ nor r15, r16, r17 ; mnz r5, r6, r7 }
    7b40:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; sh r25, r26 }
    7b48:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    7b50:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    7b58:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    7b60:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    7b68:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; nor r15, r16, r17 ; lh r25, r26 }
    7b70:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
    7b78:	[0-9a-f]* 	{ nor r15, r16, r17 ; mzh r5, r6, r7 }
    7b80:	[0-9a-f]* 	{ nor r15, r16, r17 ; nor r5, r6, r7 }
    7b88:	[0-9a-f]* 	{ nor r15, r16, r17 ; ori r5, r6, 5 }
    7b90:	[0-9a-f]* 	{ bytex r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    7b98:	[0-9a-f]* 	{ nor r15, r16, r17 ; nop ; prefetch r25 }
    7ba0:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti r5, r6, 5 ; prefetch r25 }
    7ba8:	[0-9a-f]* 	{ nor r15, r16, r17 ; rl r5, r6, r7 ; sw r25, r26 }
    7bb0:	[0-9a-f]* 	{ nor r15, r16, r17 ; s1a r5, r6, r7 ; sw r25, r26 }
    7bb8:	[0-9a-f]* 	{ nor r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
    7bc0:	[0-9a-f]* 	{ nor r15, r16, r17 ; movei r5, 5 ; sb r25, r26 }
    7bc8:	[0-9a-f]* 	{ nor r15, r16, r17 ; s1a r5, r6, r7 ; sb r25, r26 }
    7bd0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; sb r25, r26 }
    7bd8:	[0-9a-f]* 	{ nor r15, r16, r17 ; seqi r5, r6, 5 ; lh_u r25, r26 }
    7be0:	[0-9a-f]* 	{ nor r15, r16, r17 ; move r5, r6 ; sh r25, r26 }
    7be8:	[0-9a-f]* 	{ nor r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
    7bf0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; sh r25, r26 }
    7bf8:	[0-9a-f]* 	{ nor r15, r16, r17 ; shli r5, r6, 5 ; lh r25, r26 }
    7c00:	[0-9a-f]* 	{ nor r15, r16, r17 ; shrb r5, r6, r7 }
    7c08:	[0-9a-f]* 	{ nor r15, r16, r17 ; slt r5, r6, r7 ; sb r25, r26 }
    7c10:	[0-9a-f]* 	{ nor r15, r16, r17 ; slte r5, r6, r7 ; lw r25, r26 }
    7c18:	[0-9a-f]* 	{ nor r15, r16, r17 ; slth r5, r6, r7 }
    7c20:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    7c28:	[0-9a-f]* 	{ nor r15, r16, r17 ; sra r5, r6, r7 ; lh r25, r26 }
    7c30:	[0-9a-f]* 	{ nor r15, r16, r17 ; sraib r5, r6, 5 }
    7c38:	[0-9a-f]* 	{ nor r15, r16, r17 ; andi r5, r6, 5 ; sw r25, r26 }
    7c40:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    7c48:	[0-9a-f]* 	{ nor r15, r16, r17 ; slte r5, r6, r7 ; sw r25, r26 }
    7c50:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; sb r25, r26 }
    7c58:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; sb r25, r26 }
    7c60:	[0-9a-f]* 	{ nor r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    7c68:	[0-9a-f]* 	{ nor r5, r6, r7 ; addi r15, r16, 5 ; lb_u r25, r26 }
    7c70:	[0-9a-f]* 	{ nor r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    7c78:	[0-9a-f]* 	{ nor r5, r6, r7 ; lh r25, r26 }
    7c80:	[0-9a-f]* 	{ nor r5, r6, r7 ; info 19 ; lb_u r25, r26 }
    7c88:	[0-9a-f]* 	{ nor r5, r6, r7 ; lb r15, r16 }
    7c90:	[0-9a-f]* 	{ nor r5, r6, r7 ; s3a r15, r16, r17 ; lb r25, r26 }
    7c98:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
    7ca0:	[0-9a-f]* 	{ nor r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    7ca8:	[0-9a-f]* 	{ nor r5, r6, r7 ; lh r15, r16 }
    7cb0:	[0-9a-f]* 	{ nor r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    7cb8:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; lh_u r25, r26 }
    7cc0:	[0-9a-f]* 	{ nor r5, r6, r7 ; seq r15, r16, r17 ; lh_u r25, r26 }
    7cc8:	[0-9a-f]* 	{ nor r5, r6, r7 ; lnk r15 }
    7cd0:	[0-9a-f]* 	{ nor r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    7cd8:	[0-9a-f]* 	{ nor r5, r6, r7 ; lw_na r15, r16 }
    7ce0:	[0-9a-f]* 	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    7ce8:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    7cf0:	[0-9a-f]* 	{ nor r5, r6, r7 ; mzh r15, r16, r17 }
    7cf8:	[0-9a-f]* 	{ nor r5, r6, r7 ; nor r15, r16, r17 }
    7d00:	[0-9a-f]* 	{ nor r5, r6, r7 ; ori r15, r16, 5 }
    7d08:	[0-9a-f]* 	{ nor r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    7d10:	[0-9a-f]* 	{ nor r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    7d18:	[0-9a-f]* 	{ nor r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    7d20:	[0-9a-f]* 	{ nor r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
    7d28:	[0-9a-f]* 	{ nor r5, r6, r7 ; sb r25, r26 }
    7d30:	[0-9a-f]* 	{ nor r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    7d38:	[0-9a-f]* 	{ nor r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    7d40:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
    7d48:	[0-9a-f]* 	{ nor r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
    7d50:	[0-9a-f]* 	{ nor r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
    7d58:	[0-9a-f]* 	{ nor r5, r6, r7 ; shli r15, r16, 5 }
    7d60:	[0-9a-f]* 	{ nor r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
    7d68:	[0-9a-f]* 	{ nor r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    7d70:	[0-9a-f]* 	{ nor r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    7d78:	[0-9a-f]* 	{ nor r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    7d80:	[0-9a-f]* 	{ nor r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
    7d88:	[0-9a-f]* 	{ nor r5, r6, r7 ; sra r15, r16, r17 }
    7d90:	[0-9a-f]* 	{ nor r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    7d98:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; sw r25, r26 }
    7da0:	[0-9a-f]* 	{ nor r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    7da8:	[0-9a-f]* 	{ nor r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    7db0:	[0-9a-f]* 	{ or r15, r16, r17 ; addi r5, r6, 5 ; lh_u r25, r26 }
    7db8:	[0-9a-f]* 	{ or r15, r16, r17 ; and r5, r6, r7 ; sb r25, r26 }
    7dc0:	[0-9a-f]* 	{ bitx r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
    7dc8:	[0-9a-f]* 	{ clz r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
    7dd0:	[0-9a-f]* 	{ or r15, r16, r17 ; lb r25, r26 }
    7dd8:	[0-9a-f]* 	{ or r15, r16, r17 ; infol 4660 }
    7de0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; or r15, r16, r17 ; lb r25, r26 }
    7de8:	[0-9a-f]* 	{ or r15, r16, r17 ; seq r5, r6, r7 ; lb r25, r26 }
    7df0:	[0-9a-f]* 	{ or r15, r16, r17 ; xor r5, r6, r7 ; lb r25, r26 }
    7df8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; or r15, r16, r17 ; lb_u r25, r26 }
    7e00:	[0-9a-f]* 	{ or r15, r16, r17 ; shli r5, r6, 5 ; lb_u r25, r26 }
    7e08:	[0-9a-f]* 	{ or r15, r16, r17 ; addi r5, r6, 5 ; lh r25, r26 }
    7e10:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    7e18:	[0-9a-f]* 	{ or r15, r16, r17 ; slt r5, r6, r7 ; lh r25, r26 }
    7e20:	[0-9a-f]* 	{ bitx r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
    7e28:	[0-9a-f]* 	{ or r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    7e30:	[0-9a-f]* 	{ or r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
    7e38:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; lw r25, r26 }
    7e40:	[0-9a-f]* 	{ or r15, r16, r17 ; or r5, r6, r7 ; lw r25, r26 }
    7e48:	[0-9a-f]* 	{ or r15, r16, r17 ; sne r5, r6, r7 ; lw r25, r26 }
    7e50:	[0-9a-f]* 	{ or r15, r16, r17 ; mnz r5, r6, r7 ; lb_u r25, r26 }
    7e58:	[0-9a-f]* 	{ or r15, r16, r17 ; move r5, r6 }
    7e60:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; or r15, r16, r17 ; sh r25, r26 }
    7e68:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    7e70:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; or r15, r16, r17 }
    7e78:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    7e80:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    7e88:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; or r15, r16, r17 ; lh_u r25, r26 }
    7e90:	[0-9a-f]* 	{ or r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
    7e98:	[0-9a-f]* 	{ or r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
    7ea0:	[0-9a-f]* 	{ or r15, r16, r17 ; ori r5, r6, 5 ; lb_u r25, r26 }
    7ea8:	[0-9a-f]* 	{ pcnt r5, r6 ; or r15, r16, r17 ; sh r25, r26 }
    7eb0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    7eb8:	[0-9a-f]* 	{ or r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
    7ec0:	[0-9a-f]* 	{ or r15, r16, r17 ; rl r5, r6, r7 ; lb r25, r26 }
    7ec8:	[0-9a-f]* 	{ or r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
    7ed0:	[0-9a-f]* 	{ or r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
    7ed8:	[0-9a-f]* 	{ bitx r5, r6 ; or r15, r16, r17 ; sb r25, r26 }
    7ee0:	[0-9a-f]* 	{ or r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    7ee8:	[0-9a-f]* 	{ or r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
    7ef0:	[0-9a-f]* 	{ or r15, r16, r17 ; seq r5, r6, r7 ; sh r25, r26 }
    7ef8:	[0-9a-f]* 	{ or r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    7f00:	[0-9a-f]* 	{ mvz r5, r6, r7 ; or r15, r16, r17 ; sh r25, r26 }
    7f08:	[0-9a-f]* 	{ or r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    7f10:	[0-9a-f]* 	{ or r15, r16, r17 ; shl r5, r6, r7 ; sb r25, r26 }
    7f18:	[0-9a-f]* 	{ or r15, r16, r17 ; shr r5, r6, r7 ; lh r25, r26 }
    7f20:	[0-9a-f]* 	{ or r15, r16, r17 ; shrib r5, r6, 5 }
    7f28:	[0-9a-f]* 	{ or r15, r16, r17 ; slt_u r5, r6, r7 ; sw r25, r26 }
    7f30:	[0-9a-f]* 	{ or r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
    7f38:	[0-9a-f]* 	{ or r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    7f40:	[0-9a-f]* 	{ or r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
    7f48:	[0-9a-f]* 	{ or r15, r16, r17 ; srai r5, r6, 5 ; lh r25, r26 }
    7f50:	[0-9a-f]* 	{ or r15, r16, r17 ; subb r5, r6, r7 }
    7f58:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
    7f60:	[0-9a-f]* 	{ or r15, r16, r17 ; seq r5, r6, r7 ; sw r25, r26 }
    7f68:	[0-9a-f]* 	{ or r15, r16, r17 ; xor r5, r6, r7 ; sw r25, r26 }
    7f70:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    7f78:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    7f80:	[0-9a-f]* 	{ or r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
    7f88:	[0-9a-f]* 	{ or r5, r6, r7 ; addli.sn r15, r16, 4660 }
    7f90:	[0-9a-f]* 	{ or r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
    7f98:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; lh_u r25, r26 }
    7fa0:	[0-9a-f]* 	{ or r5, r6, r7 ; intlb r15, r16, r17 }
    7fa8:	[0-9a-f]* 	{ or r5, r6, r7 ; nop ; lb r25, r26 }
    7fb0:	[0-9a-f]* 	{ or r5, r6, r7 ; slti_u r15, r16, 5 ; lb r25, r26 }
    7fb8:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
    7fc0:	[0-9a-f]* 	{ or r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
    7fc8:	[0-9a-f]* 	{ or r5, r6, r7 ; nop ; lh r25, r26 }
    7fd0:	[0-9a-f]* 	{ or r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
    7fd8:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    7fe0:	[0-9a-f]* 	{ or r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    7fe8:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; lw r25, r26 }
    7ff0:	[0-9a-f]* 	{ or r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    7ff8:	[0-9a-f]* 	{ or r5, r6, r7 ; minih r15, r16, 5 }
    8000:	[0-9a-f]* 	{ or r5, r6, r7 ; move r15, r16 ; sb r25, r26 }
    8008:	[0-9a-f]* 	{ or r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
    8010:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
    8018:	[0-9a-f]* 	{ or r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    8020:	[0-9a-f]* 	{ or r5, r6, r7 ; info 19 ; prefetch r25 }
    8028:	[0-9a-f]* 	{ or r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8030:	[0-9a-f]* 	{ or r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    8038:	[0-9a-f]* 	{ or r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
    8040:	[0-9a-f]* 	{ or r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    8048:	[0-9a-f]* 	{ or r5, r6, r7 ; rli r15, r16, 5 ; sb r25, r26 }
    8050:	[0-9a-f]* 	{ or r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
    8058:	[0-9a-f]* 	{ or r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    8060:	[0-9a-f]* 	{ or r5, r6, r7 ; nor r15, r16, r17 ; sh r25, r26 }
    8068:	[0-9a-f]* 	{ or r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    8070:	[0-9a-f]* 	{ or r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    8078:	[0-9a-f]* 	{ or r5, r6, r7 ; shr r15, r16, r17 }
    8080:	[0-9a-f]* 	{ or r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8088:	[0-9a-f]* 	{ or r5, r6, r7 ; slte r15, r16, r17 ; lh_u r25, r26 }
    8090:	[0-9a-f]* 	{ or r5, r6, r7 ; slteh_u r15, r16, r17 }
    8098:	[0-9a-f]* 	{ or r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    80a0:	[0-9a-f]* 	{ or r5, r6, r7 ; sra r15, r16, r17 ; lb_u r25, r26 }
    80a8:	[0-9a-f]* 	{ or r5, r6, r7 ; srai r15, r16, 5 }
    80b0:	[0-9a-f]* 	{ or r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
    80b8:	[0-9a-f]* 	{ or r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    80c0:	[0-9a-f]* 	{ or r5, r6, r7 ; xor r15, r16, r17 ; lb r25, r26 }
    80c8:	[0-9a-f]* 	{ ori r15, r16, 5 ; add r5, r6, r7 }
    80d0:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; ori r15, r16, 5 }
    80d8:	[0-9a-f]* 	{ ori r15, r16, 5 ; andi r5, r6, 5 ; sw r25, r26 }
    80e0:	[0-9a-f]* 	{ bytex r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
    80e8:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 ; lh_u r25, r26 }
    80f0:	[0-9a-f]* 	{ ori r15, r16, 5 ; info 19 ; lh r25, r26 }
    80f8:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 ; lb r25, r26 }
    8100:	[0-9a-f]* 	{ ori r15, r16, 5 ; or r5, r6, r7 ; lb r25, r26 }
    8108:	[0-9a-f]* 	{ ori r15, r16, 5 ; sne r5, r6, r7 ; lb r25, r26 }
    8110:	[0-9a-f]* 	{ ori r15, r16, 5 ; mnz r5, r6, r7 ; lb_u r25, r26 }
    8118:	[0-9a-f]* 	{ ori r15, r16, 5 ; rl r5, r6, r7 ; lb_u r25, r26 }
    8120:	[0-9a-f]* 	{ ori r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
    8128:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    8130:	[0-9a-f]* 	{ ori r15, r16, 5 ; s2a r5, r6, r7 ; lh r25, r26 }
    8138:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; lh r25, r26 }
    8140:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    8148:	[0-9a-f]* 	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; lh_u r25, r26 }
    8150:	[0-9a-f]* 	{ ori r15, r16, 5 ; lh_u r25, r26 }
    8158:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    8160:	[0-9a-f]* 	{ ori r15, r16, 5 ; shr r5, r6, r7 ; lw r25, r26 }
    8168:	[0-9a-f]* 	{ ori r15, r16, 5 ; maxib_u r5, r6, 5 }
    8170:	[0-9a-f]* 	{ ori r15, r16, 5 ; move r5, r6 ; lb_u r25, r26 }
    8178:	[0-9a-f]* 	{ ori r15, r16, 5 ; moveli.sn r5, 4660 }
    8180:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    8188:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    8190:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    8198:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    81a0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
    81a8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    81b0:	[0-9a-f]* 	{ ori r15, r16, 5 ; nop ; lh_u r25, r26 }
    81b8:	[0-9a-f]* 	{ ori r15, r16, 5 ; or r5, r6, r7 ; lh_u r25, r26 }
    81c0:	[0-9a-f]* 	{ ori r15, r16, 5 ; packlb r5, r6, r7 }
    81c8:	[0-9a-f]* 	{ ori r15, r16, 5 ; info 19 ; prefetch r25 }
    81d0:	[0-9a-f]* 	{ pcnt r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
    81d8:	[0-9a-f]* 	{ ori r15, r16, 5 ; srai r5, r6, 5 ; prefetch r25 }
    81e0:	[0-9a-f]* 	{ ori r15, r16, 5 ; rli r5, r6, 5 ; lh r25, r26 }
    81e8:	[0-9a-f]* 	{ ori r15, r16, 5 ; s2a r5, r6, r7 ; lh r25, r26 }
    81f0:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; ori r15, r16, 5 }
    81f8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
    8200:	[0-9a-f]* 	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    8208:	[0-9a-f]* 	{ ori r15, r16, 5 ; sb r25, r26 }
    8210:	[0-9a-f]* 	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; sh r25, r26 }
    8218:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    8220:	[0-9a-f]* 	{ ori r15, r16, 5 ; seq r5, r6, r7 ; sh r25, r26 }
    8228:	[0-9a-f]* 	{ ori r15, r16, 5 ; xor r5, r6, r7 ; sh r25, r26 }
    8230:	[0-9a-f]* 	{ ori r15, r16, 5 ; shli r5, r6, 5 ; sb r25, r26 }
    8238:	[0-9a-f]* 	{ ori r15, r16, 5 ; shri r5, r6, 5 ; lh r25, r26 }
    8240:	[0-9a-f]* 	{ ori r15, r16, 5 ; slt_u r5, r6, r7 ; lb r25, r26 }
    8248:	[0-9a-f]* 	{ ori r15, r16, 5 ; slte r5, r6, r7 ; sw r25, r26 }
    8250:	[0-9a-f]* 	{ ori r15, r16, 5 ; slti r5, r6, 5 ; lh r25, r26 }
    8258:	[0-9a-f]* 	{ ori r15, r16, 5 ; sltih r5, r6, 5 }
    8260:	[0-9a-f]* 	{ ori r15, r16, 5 ; sra r5, r6, r7 ; sb r25, r26 }
    8268:	[0-9a-f]* 	{ ori r15, r16, 5 ; sub r5, r6, r7 ; lh r25, r26 }
    8270:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 ; sw r25, r26 }
    8278:	[0-9a-f]* 	{ ori r15, r16, 5 ; or r5, r6, r7 ; sw r25, r26 }
    8280:	[0-9a-f]* 	{ ori r15, r16, 5 ; sne r5, r6, r7 ; sw r25, r26 }
    8288:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ori r15, r16, 5 ; lb r25, r26 }
    8290:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; lb r25, r26 }
    8298:	[0-9a-f]* 	{ ori r15, r16, 5 ; xori r5, r6, 5 }
    82a0:	[0-9a-f]* 	{ ori r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    82a8:	[0-9a-f]* 	{ ori r5, r6, 5 ; andi r15, r16, 5 ; lb r25, r26 }
    82b0:	[0-9a-f]* 	{ ori r5, r6, 5 ; sb r25, r26 }
    82b8:	[0-9a-f]* 	{ ori r5, r6, 5 ; info 19 ; prefetch r25 }
    82c0:	[0-9a-f]* 	{ ori r5, r6, 5 ; andi r15, r16, 5 ; lb r25, r26 }
    82c8:	[0-9a-f]* 	{ ori r5, r6, 5 ; shli r15, r16, 5 ; lb r25, r26 }
    82d0:	[0-9a-f]* 	{ ori r5, r6, 5 ; lb_u r25, r26 }
    82d8:	[0-9a-f]* 	{ ori r5, r6, 5 ; shr r15, r16, r17 ; lb_u r25, r26 }
    82e0:	[0-9a-f]* 	{ ori r5, r6, 5 ; andi r15, r16, 5 ; lh r25, r26 }
    82e8:	[0-9a-f]* 	{ ori r5, r6, 5 ; shli r15, r16, 5 ; lh r25, r26 }
    82f0:	[0-9a-f]* 	{ ori r5, r6, 5 ; lh_u r25, r26 }
    82f8:	[0-9a-f]* 	{ ori r5, r6, 5 ; shr r15, r16, r17 ; lh_u r25, r26 }
    8300:	[0-9a-f]* 	{ ori r5, r6, 5 ; and r15, r16, r17 ; lw r25, r26 }
    8308:	[0-9a-f]* 	{ ori r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
    8310:	[0-9a-f]* 	{ ori r5, r6, 5 ; maxh r15, r16, r17 }
    8318:	[0-9a-f]* 	{ ori r5, r6, 5 ; mnzb r15, r16, r17 }
    8320:	[0-9a-f]* 	{ ori r5, r6, 5 ; movei r15, 5 ; sw r25, r26 }
    8328:	[0-9a-f]* 	{ ori r5, r6, 5 ; nop ; lh_u r25, r26 }
    8330:	[0-9a-f]* 	{ ori r5, r6, 5 ; or r15, r16, r17 ; lh_u r25, r26 }
    8338:	[0-9a-f]* 	{ ori r5, r6, 5 ; packlb r15, r16, r17 }
    8340:	[0-9a-f]* 	{ ori r5, r6, 5 ; s2a r15, r16, r17 ; prefetch r25 }
    8348:	[0-9a-f]* 	{ ori r5, r6, 5 ; raise }
    8350:	[0-9a-f]* 	{ ori r5, r6, 5 ; rli r15, r16, 5 }
    8358:	[0-9a-f]* 	{ ori r5, r6, 5 ; s2a r15, r16, r17 }
    8360:	[0-9a-f]* 	{ ori r5, r6, 5 ; move r15, r16 ; sb r25, r26 }
    8368:	[0-9a-f]* 	{ ori r5, r6, 5 ; slte r15, r16, r17 ; sb r25, r26 }
    8370:	[0-9a-f]* 	{ ori r5, r6, 5 ; seq r15, r16, r17 }
    8378:	[0-9a-f]* 	{ ori r5, r6, 5 ; sh r25, r26 }
    8380:	[0-9a-f]* 	{ ori r5, r6, 5 ; shr r15, r16, r17 ; sh r25, r26 }
    8388:	[0-9a-f]* 	{ ori r5, r6, 5 ; shl r15, r16, r17 ; prefetch r25 }
    8390:	[0-9a-f]* 	{ ori r5, r6, 5 ; shr r15, r16, r17 ; lb_u r25, r26 }
    8398:	[0-9a-f]* 	{ ori r5, r6, 5 ; shri r15, r16, 5 }
    83a0:	[0-9a-f]* 	{ ori r5, r6, 5 ; slt_u r15, r16, r17 ; sh r25, r26 }
    83a8:	[0-9a-f]* 	{ ori r5, r6, 5 ; slte_u r15, r16, r17 ; prefetch r25 }
    83b0:	[0-9a-f]* 	{ ori r5, r6, 5 ; slti r15, r16, 5 }
    83b8:	[0-9a-f]* 	{ ori r5, r6, 5 ; sne r15, r16, r17 ; prefetch r25 }
    83c0:	[0-9a-f]* 	{ ori r5, r6, 5 ; srai r15, r16, 5 ; lb_u r25, r26 }
    83c8:	[0-9a-f]* 	{ ori r5, r6, 5 ; sub r15, r16, r17 }
    83d0:	[0-9a-f]* 	{ ori r5, r6, 5 ; or r15, r16, r17 ; sw r25, r26 }
    83d8:	[0-9a-f]* 	{ ori r5, r6, 5 ; sra r15, r16, r17 ; sw r25, r26 }
    83e0:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; addb r5, r6, r7 }
    83e8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; packbs_u r15, r16, r17 }
    83f0:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; mnz r5, r6, r7 }
    83f8:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; packbs_u r15, r16, r17 }
    8400:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; packhb r5, r6, r7 }
    8408:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; seqih r5, r6, 5 }
    8410:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; slteb_u r5, r6, r7 }
    8418:	[0-9a-f]* 	{ packbs_u r15, r16, r17 ; sub r5, r6, r7 }
    8420:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; addli r15, r16, 4660 }
    8428:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; jalr r15 }
    8430:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; maxih r15, r16, 5 }
    8438:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; nor r15, r16, r17 }
    8440:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; seqib r15, r16, 5 }
    8448:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; slte r15, r16, r17 }
    8450:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; srai r15, r16, 5 }
    8458:	[0-9a-f]* 	{ packhb r15, r16, r17 ; addi r5, r6, 5 }
    8460:	[0-9a-f]* 	{ packhb r15, r16, r17 }
    8468:	[0-9a-f]* 	{ packhb r15, r16, r17 ; movei r5, 5 }
    8470:	[0-9a-f]* 	{ mulll_su r5, r6, r7 ; packhb r15, r16, r17 }
    8478:	[0-9a-f]* 	{ packhb r15, r16, r17 ; rl r5, r6, r7 }
    8480:	[0-9a-f]* 	{ packhb r15, r16, r17 ; shli r5, r6, 5 }
    8488:	[0-9a-f]* 	{ packhb r15, r16, r17 ; slth_u r5, r6, r7 }
    8490:	[0-9a-f]* 	{ packhb r15, r16, r17 ; subhs r5, r6, r7 }
    8498:	[0-9a-f]* 	{ packhb r5, r6, r7 ; andi r15, r16, 5 }
    84a0:	[0-9a-f]* 	{ packhb r5, r6, r7 ; lb r15, r16 }
    84a8:	[0-9a-f]* 	{ packhb r5, r6, r7 ; minh r15, r16, r17 }
    84b0:	[0-9a-f]* 	{ packhb r5, r6, r7 ; packhb r15, r16, r17 }
    84b8:	[0-9a-f]* 	{ packhb r5, r6, r7 ; shl r15, r16, r17 }
    84c0:	[0-9a-f]* 	{ packhb r5, r6, r7 ; slteh r15, r16, r17 }
    84c8:	[0-9a-f]* 	{ packhb r5, r6, r7 ; subb r15, r16, r17 }
    84d0:	[0-9a-f]* 	{ packhs r15, r16, r17 ; addli.sn r5, r6, 4660 }
    84d8:	[0-9a-f]* 	{ packhs r15, r16, r17 ; inthh r5, r6, r7 }
    84e0:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; packhs r15, r16, r17 }
    84e8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; packhs r15, r16, r17 }
    84f0:	[0-9a-f]* 	{ packhs r15, r16, r17 ; s3a r5, r6, r7 }
    84f8:	[0-9a-f]* 	{ packhs r15, r16, r17 ; shrb r5, r6, r7 }
    8500:	[0-9a-f]* 	{ packhs r15, r16, r17 ; sltib_u r5, r6, 5 }
    8508:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; packhs r15, r16, r17 }
    8510:	[0-9a-f]* 	{ packhs r5, r6, r7 ; flush r15 }
    8518:	[0-9a-f]* 	{ packhs r5, r6, r7 ; lh r15, r16 }
    8520:	[0-9a-f]* 	{ packhs r5, r6, r7 ; mnz r15, r16, r17 }
    8528:	[0-9a-f]* 	{ packhs r5, r6, r7 ; raise }
    8530:	[0-9a-f]* 	{ packhs r5, r6, r7 ; shlib r15, r16, 5 }
    8538:	[0-9a-f]* 	{ packhs r5, r6, r7 ; slti r15, r16, 5 }
    8540:	[0-9a-f]* 	{ packhs r5, r6, r7 ; subs r15, r16, r17 }
    8548:	[0-9a-f]* 	{ packlb r15, r16, r17 ; and r5, r6, r7 }
    8550:	[0-9a-f]* 	{ packlb r15, r16, r17 ; maxh r5, r6, r7 }
    8558:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; packlb r15, r16, r17 }
    8560:	[0-9a-f]* 	{ packlb r15, r16, r17 ; mz r5, r6, r7 }
    8568:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; packlb r15, r16, r17 }
    8570:	[0-9a-f]* 	{ packlb r15, r16, r17 ; shrih r5, r6, 5 }
    8578:	[0-9a-f]* 	{ packlb r15, r16, r17 ; sneb r5, r6, r7 }
    8580:	[0-9a-f]* 	{ packlb r5, r6, r7 ; add r15, r16, r17 }
    8588:	[0-9a-f]* 	{ packlb r5, r6, r7 ; info 19 }
    8590:	[0-9a-f]* 	{ packlb r5, r6, r7 ; lnk r15 }
    8598:	[0-9a-f]* 	{ packlb r5, r6, r7 ; movei r15, 5 }
    85a0:	[0-9a-f]* 	{ packlb r5, r6, r7 ; s2a r15, r16, r17 }
    85a8:	[0-9a-f]* 	{ packlb r5, r6, r7 ; shrh r15, r16, r17 }
    85b0:	[0-9a-f]* 	{ packlb r5, r6, r7 ; sltih r15, r16, 5 }
    85b8:	[0-9a-f]* 	{ packlb r5, r6, r7 ; wh64 r15 }
    85c0:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
    85c8:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
    85d0:	[0-9a-f]* 	{ pcnt r5, r6 ; lw r25, r26 }
    85d8:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; lh_u r25, r26 }
    85e0:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; lb r25, r26 }
    85e8:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; lb r25, r26 }
    85f0:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; lb_u r25, r26 }
    85f8:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; lb_u r25, r26 }
    8600:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; lh r25, r26 }
    8608:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; lh r25, r26 }
    8610:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; lh_u r25, r26 }
    8618:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    8620:	[0-9a-f]* 	{ pcnt r5, r6 ; add r15, r16, r17 ; lw r25, r26 }
    8628:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 ; lw r25, r26 }
    8630:	[0-9a-f]* 	{ pcnt r5, r6 ; lwadd_na r15, r16, 5 }
    8638:	[0-9a-f]* 	{ pcnt r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
    8640:	[0-9a-f]* 	{ pcnt r5, r6 ; movei r15, 5 ; sb r25, r26 }
    8648:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; lb_u r25, r26 }
    8650:	[0-9a-f]* 	{ pcnt r5, r6 ; or r15, r16, r17 ; lb_u r25, r26 }
    8658:	[0-9a-f]* 	{ pcnt r5, r6 ; packhb r15, r16, r17 }
    8660:	[0-9a-f]* 	{ pcnt r5, r6 ; rli r15, r16, 5 ; prefetch r25 }
    8668:	[0-9a-f]* 	{ pcnt r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    8670:	[0-9a-f]* 	{ pcnt r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
    8678:	[0-9a-f]* 	{ pcnt r5, r6 ; s2a r15, r16, r17 ; sh r25, r26 }
    8680:	[0-9a-f]* 	{ pcnt r5, r6 ; info 19 ; sb r25, r26 }
    8688:	[0-9a-f]* 	{ pcnt r5, r6 ; slt r15, r16, r17 ; sb r25, r26 }
    8690:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
    8698:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; sh r25, r26 }
    86a0:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; sh r25, r26 }
    86a8:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
    86b0:	[0-9a-f]* 	{ pcnt r5, r6 ; shlih r15, r16, 5 }
    86b8:	[0-9a-f]* 	{ pcnt r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
    86c0:	[0-9a-f]* 	{ pcnt r5, r6 ; slt_u r15, r16, r17 ; prefetch r25 }
    86c8:	[0-9a-f]* 	{ pcnt r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    86d0:	[0-9a-f]* 	{ pcnt r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    86d8:	[0-9a-f]* 	{ pcnt r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
    86e0:	[0-9a-f]* 	{ pcnt r5, r6 ; srah r15, r16, r17 }
    86e8:	[0-9a-f]* 	{ pcnt r5, r6 ; sub r15, r16, r17 ; sh r25, r26 }
    86f0:	[0-9a-f]* 	{ pcnt r5, r6 ; nop ; sw r25, r26 }
    86f8:	[0-9a-f]* 	{ pcnt r5, r6 ; slti_u r15, r16, 5 ; sw r25, r26 }
    8700:	[0-9a-f]* 	{ pcnt r5, r6 ; xori r15, r16, 5 }
    8708:	[0-9a-f]* 	{ bytex r5, r6 ; prefetch r15 }
    8710:	[0-9a-f]* 	{ minih r5, r6, 5 ; prefetch r15 }
    8718:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; prefetch r15 }
    8720:	[0-9a-f]* 	{ ori r5, r6, 5 ; prefetch r15 }
    8728:	[0-9a-f]* 	{ seqi r5, r6, 5 ; prefetch r15 }
    8730:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; prefetch r15 }
    8738:	[0-9a-f]* 	{ sraib r5, r6, 5 ; prefetch r15 }
    8740:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; prefetch r25 }
    8748:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; prefetch r25 }
    8750:	[0-9a-f]* 	{ add r15, r16, r17 ; slti_u r5, r6, 5 ; prefetch r25 }
    8758:	[0-9a-f]* 	{ add r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    8760:	[0-9a-f]* 	{ add r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    8768:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; prefetch r25 }
    8770:	[0-9a-f]* 	{ addi r15, r16, 5 ; rli r5, r6, 5 ; prefetch r25 }
    8778:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    8780:	[0-9a-f]* 	{ addi r5, r6, 5 ; ori r15, r16, 5 ; prefetch r25 }
    8788:	[0-9a-f]* 	{ addi r5, r6, 5 ; srai r15, r16, 5 ; prefetch r25 }
    8790:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    8798:	[0-9a-f]* 	{ and r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
    87a0:	[0-9a-f]* 	{ and r15, r16, r17 ; prefetch r25 }
    87a8:	[0-9a-f]* 	{ and r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    87b0:	[0-9a-f]* 	{ andi r15, r16, 5 ; addi r5, r6, 5 ; prefetch r25 }
    87b8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
    87c0:	[0-9a-f]* 	{ andi r15, r16, 5 ; slt r5, r6, r7 ; prefetch r25 }
    87c8:	[0-9a-f]* 	{ andi r5, r6, 5 ; prefetch r25 }
    87d0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shr r15, r16, r17 ; prefetch r25 }
    87d8:	[0-9a-f]* 	{ bitx r5, r6 ; info 19 ; prefetch r25 }
    87e0:	[0-9a-f]* 	{ bitx r5, r6 ; slt r15, r16, r17 ; prefetch r25 }
    87e8:	[0-9a-f]* 	{ bytex r5, r6 ; move r15, r16 ; prefetch r25 }
    87f0:	[0-9a-f]* 	{ bytex r5, r6 ; slte r15, r16, r17 ; prefetch r25 }
    87f8:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    8800:	[0-9a-f]* 	{ clz r5, r6 ; slti r15, r16, 5 ; prefetch r25 }
    8808:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    8810:	[0-9a-f]* 	{ ctz r5, r6 ; sne r15, r16, r17 ; prefetch r25 }
    8818:	[0-9a-f]* 	{ info 19 ; prefetch r25 }
    8820:	[0-9a-f]* 	{ nop ; prefetch r25 }
    8828:	[0-9a-f]* 	{ seqi r15, r16, 5 ; prefetch r25 }
    8830:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; prefetch r25 }
    8838:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; prefetch r25 }
    8840:	[0-9a-f]* 	{ mvz r5, r6, r7 ; ill ; prefetch r25 }
    8848:	[0-9a-f]* 	{ slte r5, r6, r7 ; ill ; prefetch r25 }
    8850:	[0-9a-f]* 	{ info 19 ; andi r15, r16, 5 ; prefetch r25 }
    8858:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; info 19 ; prefetch r25 }
    8860:	[0-9a-f]* 	{ info 19 ; s1a r15, r16, r17 ; prefetch r25 }
    8868:	[0-9a-f]* 	{ info 19 ; slt_u r15, r16, r17 ; prefetch r25 }
    8870:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; info 19 ; prefetch r25 }
    8878:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    8880:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seq r5, r6, r7 ; prefetch r25 }
    8888:	[0-9a-f]* 	{ mnz r15, r16, r17 ; xor r5, r6, r7 ; prefetch r25 }
    8890:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
    8898:	[0-9a-f]* 	{ move r15, r16 ; add r5, r6, r7 ; prefetch r25 }
    88a0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    88a8:	[0-9a-f]* 	{ move r15, r16 ; shri r5, r6, 5 ; prefetch r25 }
    88b0:	[0-9a-f]* 	{ move r5, r6 ; andi r15, r16, 5 ; prefetch r25 }
    88b8:	[0-9a-f]* 	{ move r5, r6 ; shli r15, r16, 5 ; prefetch r25 }
    88c0:	[0-9a-f]* 	{ bytex r5, r6 ; movei r15, 5 ; prefetch r25 }
    88c8:	[0-9a-f]* 	{ movei r15, 5 ; nop ; prefetch r25 }
    88d0:	[0-9a-f]* 	{ movei r15, 5 ; slti r5, r6, 5 ; prefetch r25 }
    88d8:	[0-9a-f]* 	{ movei r5, 5 ; move r15, r16 ; prefetch r25 }
    88e0:	[0-9a-f]* 	{ movei r5, 5 ; slte r15, r16, r17 ; prefetch r25 }
    88e8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    88f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    88f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
    8900:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sne r15, r16, r17 ; prefetch r25 }
    8908:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    8910:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    8918:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    8920:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    8928:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
    8930:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; add r15, r16, r17 ; prefetch r25 }
    8938:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    8940:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    8948:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    8950:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; prefetch r25 }
    8958:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shr r15, r16, r17 ; prefetch r25 }
    8960:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; info 19 ; prefetch r25 }
    8968:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8970:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    8978:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    8980:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    8988:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    8990:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; prefetch r25 }
    8998:	[0-9a-f]* 	{ mz r15, r16, r17 ; s1a r5, r6, r7 ; prefetch r25 }
    89a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    89a8:	[0-9a-f]* 	{ mz r5, r6, r7 ; rl r15, r16, r17 ; prefetch r25 }
    89b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    89b8:	[0-9a-f]* 	{ nop ; move r15, r16 ; prefetch r25 }
    89c0:	[0-9a-f]* 	{ nop ; or r15, r16, r17 ; prefetch r25 }
    89c8:	[0-9a-f]* 	{ nop ; shl r5, r6, r7 ; prefetch r25 }
    89d0:	[0-9a-f]* 	{ nop ; sne r5, r6, r7 ; prefetch r25 }
    89d8:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    89e0:	[0-9a-f]* 	{ nor r15, r16, r17 ; nor r5, r6, r7 ; prefetch r25 }
    89e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti_u r5, r6, 5 ; prefetch r25 }
    89f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    89f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    8a00:	[0-9a-f]* 	{ or r15, r16, r17 ; move r5, r6 ; prefetch r25 }
    8a08:	[0-9a-f]* 	{ or r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
    8a10:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; or r15, r16, r17 ; prefetch r25 }
    8a18:	[0-9a-f]* 	{ or r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    8a20:	[0-9a-f]* 	{ or r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    8a28:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
    8a30:	[0-9a-f]* 	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; prefetch r25 }
    8a38:	[0-9a-f]* 	{ ori r15, r16, 5 ; prefetch r25 }
    8a40:	[0-9a-f]* 	{ ori r5, r6, 5 ; s3a r15, r16, r17 ; prefetch r25 }
    8a48:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
    8a50:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
    8a58:	[0-9a-f]* 	{ rl r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    8a60:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rl r15, r16, r17 ; prefetch r25 }
    8a68:	[0-9a-f]* 	{ rl r15, r16, r17 ; slte r5, r6, r7 ; prefetch r25 }
    8a70:	[0-9a-f]* 	{ rl r5, r6, r7 ; info 19 ; prefetch r25 }
    8a78:	[0-9a-f]* 	{ rl r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8a80:	[0-9a-f]* 	{ rli r15, r16, 5 ; prefetch r25 }
    8a88:	[0-9a-f]* 	{ rli r15, r16, 5 ; ori r5, r6, 5 ; prefetch r25 }
    8a90:	[0-9a-f]* 	{ rli r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    8a98:	[0-9a-f]* 	{ rli r5, r6, 5 ; nop ; prefetch r25 }
    8aa0:	[0-9a-f]* 	{ rli r5, r6, 5 ; slti_u r15, r16, 5 ; prefetch r25 }
    8aa8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    8ab0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
    8ab8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s1a r15, r16, r17 ; prefetch r25 }
    8ac0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    8ac8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    8ad0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
    8ad8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    8ae0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    8ae8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    8af0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    8af8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    8b00:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte r5, r6, r7 ; prefetch r25 }
    8b08:	[0-9a-f]* 	{ s3a r5, r6, r7 ; info 19 ; prefetch r25 }
    8b10:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8b18:	[0-9a-f]* 	{ seq r15, r16, r17 ; prefetch r25 }
    8b20:	[0-9a-f]* 	{ seq r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    8b28:	[0-9a-f]* 	{ seq r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8b30:	[0-9a-f]* 	{ seq r5, r6, r7 ; nop ; prefetch r25 }
    8b38:	[0-9a-f]* 	{ seq r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    8b40:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    8b48:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s2a r5, r6, r7 ; prefetch r25 }
    8b50:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
    8b58:	[0-9a-f]* 	{ seqi r5, r6, 5 ; rli r15, r16, 5 ; prefetch r25 }
    8b60:	[0-9a-f]* 	{ seqi r5, r6, 5 ; xor r15, r16, r17 ; prefetch r25 }
    8b68:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    8b70:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    8b78:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    8b80:	[0-9a-f]* 	{ shl r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    8b88:	[0-9a-f]* 	{ shli r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    8b90:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    8b98:	[0-9a-f]* 	{ shli r15, r16, 5 ; slte r5, r6, r7 ; prefetch r25 }
    8ba0:	[0-9a-f]* 	{ shli r5, r6, 5 ; info 19 ; prefetch r25 }
    8ba8:	[0-9a-f]* 	{ shli r5, r6, 5 ; slt r15, r16, r17 ; prefetch r25 }
    8bb0:	[0-9a-f]* 	{ shr r15, r16, r17 ; prefetch r25 }
    8bb8:	[0-9a-f]* 	{ shr r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    8bc0:	[0-9a-f]* 	{ shr r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8bc8:	[0-9a-f]* 	{ shr r5, r6, r7 ; nop ; prefetch r25 }
    8bd0:	[0-9a-f]* 	{ shr r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    8bd8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
    8be0:	[0-9a-f]* 	{ shri r15, r16, 5 ; s2a r5, r6, r7 ; prefetch r25 }
    8be8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shri r15, r16, 5 ; prefetch r25 }
    8bf0:	[0-9a-f]* 	{ shri r5, r6, 5 ; rli r15, r16, 5 ; prefetch r25 }
    8bf8:	[0-9a-f]* 	{ shri r5, r6, 5 ; xor r15, r16, r17 ; prefetch r25 }
    8c00:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8c08:	[0-9a-f]* 	{ slt r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    8c10:	[0-9a-f]* 	{ slt r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    8c18:	[0-9a-f]* 	{ slt r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    8c20:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    8c28:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt_u r15, r16, r17 ; prefetch r25 }
    8c30:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slte r5, r6, r7 ; prefetch r25 }
    8c38:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; info 19 ; prefetch r25 }
    8c40:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8c48:	[0-9a-f]* 	{ slte r15, r16, r17 ; prefetch r25 }
    8c50:	[0-9a-f]* 	{ slte r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    8c58:	[0-9a-f]* 	{ slte r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8c60:	[0-9a-f]* 	{ slte r5, r6, r7 ; nop ; prefetch r25 }
    8c68:	[0-9a-f]* 	{ slte r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    8c70:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    8c78:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
    8c80:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte_u r15, r16, r17 ; prefetch r25 }
    8c88:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    8c90:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    8c98:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    8ca0:	[0-9a-f]* 	{ slti r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    8ca8:	[0-9a-f]* 	{ slti r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    8cb0:	[0-9a-f]* 	{ slti r5, r6, 5 ; seqi r15, r16, 5 ; prefetch r25 }
    8cb8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
    8cc0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    8cc8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slte r5, r6, r7 ; prefetch r25 }
    8cd0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; info 19 ; prefetch r25 }
    8cd8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slt r15, r16, r17 ; prefetch r25 }
    8ce0:	[0-9a-f]* 	{ sne r15, r16, r17 ; prefetch r25 }
    8ce8:	[0-9a-f]* 	{ sne r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    8cf0:	[0-9a-f]* 	{ sne r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8cf8:	[0-9a-f]* 	{ sne r5, r6, r7 ; nop ; prefetch r25 }
    8d00:	[0-9a-f]* 	{ sne r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    8d08:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    8d10:	[0-9a-f]* 	{ sra r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
    8d18:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; prefetch r25 }
    8d20:	[0-9a-f]* 	{ sra r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    8d28:	[0-9a-f]* 	{ sra r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    8d30:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    8d38:	[0-9a-f]* 	{ srai r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    8d40:	[0-9a-f]* 	{ srai r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
    8d48:	[0-9a-f]* 	{ srai r5, r6, 5 ; seqi r15, r16, 5 ; prefetch r25 }
    8d50:	[0-9a-f]* 	{ sub r15, r16, r17 ; andi r5, r6, 5 ; prefetch r25 }
    8d58:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    8d60:	[0-9a-f]* 	{ sub r15, r16, r17 ; slte r5, r6, r7 ; prefetch r25 }
    8d68:	[0-9a-f]* 	{ sub r5, r6, r7 ; info 19 ; prefetch r25 }
    8d70:	[0-9a-f]* 	{ sub r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    8d78:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; prefetch r25 }
    8d80:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; prefetch r25 }
    8d88:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    8d90:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti r15, r16, 5 ; prefetch r25 }
    8d98:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    8da0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sne r15, r16, r17 ; prefetch r25 }
    8da8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
    8db0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; prefetch r25 }
    8db8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    8dc0:	[0-9a-f]* 	{ xor r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
    8dc8:	[0-9a-f]* 	{ xor r15, r16, r17 ; prefetch r25 }
    8dd0:	[0-9a-f]* 	{ xor r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    8dd8:	[0-9a-f]* 	{ addb r5, r6, r7 ; raise }
    8de0:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; raise }
    8de8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; raise }
    8df0:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; raise }
    8df8:	[0-9a-f]* 	{ packhb r5, r6, r7 ; raise }
    8e00:	[0-9a-f]* 	{ seqih r5, r6, 5 ; raise }
    8e08:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; raise }
    8e10:	[0-9a-f]* 	{ sub r5, r6, r7 ; raise }
    8e18:	[0-9a-f]* 	{ rl r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    8e20:	[0-9a-f]* 	{ rl r15, r16, r17 ; adds r5, r6, r7 }
    8e28:	[0-9a-f]* 	{ rl r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    8e30:	[0-9a-f]* 	{ bytex r5, r6 ; rl r15, r16, r17 ; lw r25, r26 }
    8e38:	[0-9a-f]* 	{ ctz r5, r6 ; rl r15, r16, r17 ; lh r25, r26 }
    8e40:	[0-9a-f]* 	{ rl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    8e48:	[0-9a-f]* 	{ clz r5, r6 ; rl r15, r16, r17 ; lb r25, r26 }
    8e50:	[0-9a-f]* 	{ rl r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    8e58:	[0-9a-f]* 	{ rl r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    8e60:	[0-9a-f]* 	{ rl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    8e68:	[0-9a-f]* 	{ pcnt r5, r6 ; rl r15, r16, r17 ; lb_u r25, r26 }
    8e70:	[0-9a-f]* 	{ rl r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    8e78:	[0-9a-f]* 	{ rl r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    8e80:	[0-9a-f]* 	{ rl r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    8e88:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rl r15, r16, r17 ; lh r25, r26 }
    8e90:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    8e98:	[0-9a-f]* 	{ rl r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    8ea0:	[0-9a-f]* 	{ rl r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    8ea8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    8eb0:	[0-9a-f]* 	{ rl r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    8eb8:	[0-9a-f]* 	{ rl r15, r16, r17 ; maxh r5, r6, r7 }
    8ec0:	[0-9a-f]* 	{ rl r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    8ec8:	[0-9a-f]* 	{ rl r15, r16, r17 ; moveli r5, 4660 }
    8ed0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    8ed8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    8ee0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    8ee8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    8ef0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rl r15, r16, r17 ; prefetch r25 }
    8ef8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
    8f00:	[0-9a-f]* 	{ rl r15, r16, r17 ; nop ; lh r25, r26 }
    8f08:	[0-9a-f]* 	{ rl r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    8f10:	[0-9a-f]* 	{ rl r15, r16, r17 ; packhs r5, r6, r7 }
    8f18:	[0-9a-f]* 	{ rl r15, r16, r17 ; prefetch r25 }
    8f20:	[0-9a-f]* 	{ rl r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    8f28:	[0-9a-f]* 	{ rl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8f30:	[0-9a-f]* 	{ rl r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    8f38:	[0-9a-f]* 	{ rl r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    8f40:	[0-9a-f]* 	{ sadah r5, r6, r7 ; rl r15, r16, r17 }
    8f48:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    8f50:	[0-9a-f]* 	{ rl r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    8f58:	[0-9a-f]* 	{ rl r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    8f60:	[0-9a-f]* 	{ rl r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    8f68:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    8f70:	[0-9a-f]* 	{ rl r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    8f78:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rl r15, r16, r17 ; sh r25, r26 }
    8f80:	[0-9a-f]* 	{ rl r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    8f88:	[0-9a-f]* 	{ rl r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    8f90:	[0-9a-f]* 	{ rl r15, r16, r17 ; slt r5, r6, r7 }
    8f98:	[0-9a-f]* 	{ rl r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    8fa0:	[0-9a-f]* 	{ rl r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    8fa8:	[0-9a-f]* 	{ rl r15, r16, r17 ; sltib_u r5, r6, 5 }
    8fb0:	[0-9a-f]* 	{ rl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    8fb8:	[0-9a-f]* 	{ rl r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    8fc0:	[0-9a-f]* 	{ clz r5, r6 ; rl r15, r16, r17 ; sw r25, r26 }
    8fc8:	[0-9a-f]* 	{ rl r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    8fd0:	[0-9a-f]* 	{ rl r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    8fd8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; rl r15, r16, r17 }
    8fe0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rl r15, r16, r17 }
    8fe8:	[0-9a-f]* 	{ rl r15, r16, r17 ; xor r5, r6, r7 }
    8ff0:	[0-9a-f]* 	{ rl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    8ff8:	[0-9a-f]* 	{ rl r5, r6, r7 ; and r15, r16, r17 }
    9000:	[0-9a-f]* 	{ rl r5, r6, r7 ; prefetch r25 }
    9008:	[0-9a-f]* 	{ rl r5, r6, r7 ; info 19 ; lw r25, r26 }
    9010:	[0-9a-f]* 	{ rl r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    9018:	[0-9a-f]* 	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    9020:	[0-9a-f]* 	{ rl r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    9028:	[0-9a-f]* 	{ rl r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    9030:	[0-9a-f]* 	{ rl r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    9038:	[0-9a-f]* 	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    9040:	[0-9a-f]* 	{ rl r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    9048:	[0-9a-f]* 	{ rl r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    9050:	[0-9a-f]* 	{ rl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    9058:	[0-9a-f]* 	{ rl r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    9060:	[0-9a-f]* 	{ rl r5, r6, r7 ; maxb_u r15, r16, r17 }
    9068:	[0-9a-f]* 	{ rl r5, r6, r7 ; mnz r15, r16, r17 }
    9070:	[0-9a-f]* 	{ rl r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    9078:	[0-9a-f]* 	{ rl r5, r6, r7 ; nop ; lh r25, r26 }
    9080:	[0-9a-f]* 	{ rl r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    9088:	[0-9a-f]* 	{ rl r5, r6, r7 ; packhs r15, r16, r17 }
    9090:	[0-9a-f]* 	{ rl r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    9098:	[0-9a-f]* 	{ rl r5, r6, r7 ; prefetch r25 }
    90a0:	[0-9a-f]* 	{ rl r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    90a8:	[0-9a-f]* 	{ rl r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    90b0:	[0-9a-f]* 	{ rl r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    90b8:	[0-9a-f]* 	{ rl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    90c0:	[0-9a-f]* 	{ rl r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    90c8:	[0-9a-f]* 	{ rl r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    90d0:	[0-9a-f]* 	{ rl r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    90d8:	[0-9a-f]* 	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    90e0:	[0-9a-f]* 	{ rl r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    90e8:	[0-9a-f]* 	{ rl r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    90f0:	[0-9a-f]* 	{ rl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    90f8:	[0-9a-f]* 	{ rl r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    9100:	[0-9a-f]* 	{ rl r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    9108:	[0-9a-f]* 	{ rl r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    9110:	[0-9a-f]* 	{ rl r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    9118:	[0-9a-f]* 	{ rl r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    9120:	[0-9a-f]* 	{ rl r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    9128:	[0-9a-f]* 	{ rl r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    9130:	[0-9a-f]* 	{ rli r15, r16, 5 ; add r5, r6, r7 ; lb r25, r26 }
    9138:	[0-9a-f]* 	{ rli r15, r16, 5 ; addi r5, r6, 5 ; sb r25, r26 }
    9140:	[0-9a-f]* 	{ rli r15, r16, 5 ; and r5, r6, r7 }
    9148:	[0-9a-f]* 	{ bitx r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
    9150:	[0-9a-f]* 	{ clz r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
    9158:	[0-9a-f]* 	{ rli r15, r16, 5 ; lh_u r25, r26 }
    9160:	[0-9a-f]* 	{ rli r15, r16, 5 ; intlb r5, r6, r7 }
    9168:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    9170:	[0-9a-f]* 	{ rli r15, r16, 5 ; shli r5, r6, 5 ; lb r25, r26 }
    9178:	[0-9a-f]* 	{ rli r15, r16, 5 ; addi r5, r6, 5 ; lb_u r25, r26 }
    9180:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
    9188:	[0-9a-f]* 	{ rli r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    9190:	[0-9a-f]* 	{ bitx r5, r6 ; rli r15, r16, 5 ; lh r25, r26 }
    9198:	[0-9a-f]* 	{ rli r15, r16, 5 ; mz r5, r6, r7 ; lh r25, r26 }
    91a0:	[0-9a-f]* 	{ rli r15, r16, 5 ; slte_u r5, r6, r7 ; lh r25, r26 }
    91a8:	[0-9a-f]* 	{ ctz r5, r6 ; rli r15, r16, 5 ; lh_u r25, r26 }
    91b0:	[0-9a-f]* 	{ rli r15, r16, 5 ; or r5, r6, r7 ; lh_u r25, r26 }
    91b8:	[0-9a-f]* 	{ rli r15, r16, 5 ; sne r5, r6, r7 ; lh_u r25, r26 }
    91c0:	[0-9a-f]* 	{ rli r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
    91c8:	[0-9a-f]* 	{ rli r15, r16, 5 ; rl r5, r6, r7 ; lw r25, r26 }
    91d0:	[0-9a-f]* 	{ rli r15, r16, 5 ; sub r5, r6, r7 ; lw r25, r26 }
    91d8:	[0-9a-f]* 	{ rli r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
    91e0:	[0-9a-f]* 	{ rli r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    91e8:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; rli r15, r16, 5 }
    91f0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; rli r15, r16, 5 }
    91f8:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; rli r15, r16, 5 }
    9200:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 }
    9208:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    9210:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; rli r15, r16, 5 ; sb r25, r26 }
    9218:	[0-9a-f]* 	{ rli r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
    9220:	[0-9a-f]* 	{ rli r15, r16, 5 ; nor r5, r6, r7 ; lw r25, r26 }
    9228:	[0-9a-f]* 	{ rli r15, r16, 5 ; ori r5, r6, 5 ; lw r25, r26 }
    9230:	[0-9a-f]* 	{ rli r15, r16, 5 ; add r5, r6, r7 ; prefetch r25 }
    9238:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    9240:	[0-9a-f]* 	{ rli r15, r16, 5 ; shri r5, r6, 5 ; prefetch r25 }
    9248:	[0-9a-f]* 	{ rli r15, r16, 5 ; rl r5, r6, r7 ; lh_u r25, r26 }
    9250:	[0-9a-f]* 	{ rli r15, r16, 5 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    9258:	[0-9a-f]* 	{ rli r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    9260:	[0-9a-f]* 	{ ctz r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
    9268:	[0-9a-f]* 	{ rli r15, r16, 5 ; or r5, r6, r7 ; sb r25, r26 }
    9270:	[0-9a-f]* 	{ rli r15, r16, 5 ; sne r5, r6, r7 ; sb r25, r26 }
    9278:	[0-9a-f]* 	{ rli r15, r16, 5 ; seqb r5, r6, r7 }
    9280:	[0-9a-f]* 	{ clz r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
    9288:	[0-9a-f]* 	{ rli r15, r16, 5 ; nor r5, r6, r7 ; sh r25, r26 }
    9290:	[0-9a-f]* 	{ rli r15, r16, 5 ; slti_u r5, r6, 5 ; sh r25, r26 }
    9298:	[0-9a-f]* 	{ rli r15, r16, 5 ; shl r5, r6, r7 }
    92a0:	[0-9a-f]* 	{ rli r15, r16, 5 ; shr r5, r6, r7 ; prefetch r25 }
    92a8:	[0-9a-f]* 	{ rli r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    92b0:	[0-9a-f]* 	{ rli r15, r16, 5 ; sltb_u r5, r6, r7 }
    92b8:	[0-9a-f]* 	{ rli r15, r16, 5 ; slte_u r5, r6, r7 }
    92c0:	[0-9a-f]* 	{ rli r15, r16, 5 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    92c8:	[0-9a-f]* 	{ rli r15, r16, 5 ; sne r5, r6, r7 }
    92d0:	[0-9a-f]* 	{ rli r15, r16, 5 ; srai r5, r6, 5 ; prefetch r25 }
    92d8:	[0-9a-f]* 	{ rli r15, r16, 5 ; subhs r5, r6, r7 }
    92e0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    92e8:	[0-9a-f]* 	{ rli r15, r16, 5 ; shli r5, r6, 5 ; sw r25, r26 }
    92f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
    92f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
    9300:	[0-9a-f]* 	{ rli r15, r16, 5 ; xor r5, r6, r7 ; lb_u r25, r26 }
    9308:	[0-9a-f]* 	{ rli r5, r6, 5 ; addb r15, r16, r17 }
    9310:	[0-9a-f]* 	{ rli r5, r6, 5 ; and r15, r16, r17 ; lb_u r25, r26 }
    9318:	[0-9a-f]* 	{ rli r5, r6, 5 ; dtlbpr r15 }
    9320:	[0-9a-f]* 	{ rli r5, r6, 5 ; ill ; sb r25, r26 }
    9328:	[0-9a-f]* 	{ rli r5, r6, 5 ; iret }
    9330:	[0-9a-f]* 	{ rli r5, r6, 5 ; ori r15, r16, 5 ; lb r25, r26 }
    9338:	[0-9a-f]* 	{ rli r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    9340:	[0-9a-f]* 	{ rli r5, r6, 5 ; rl r15, r16, r17 ; lb_u r25, r26 }
    9348:	[0-9a-f]* 	{ rli r5, r6, 5 ; sub r15, r16, r17 ; lb_u r25, r26 }
    9350:	[0-9a-f]* 	{ rli r5, r6, 5 ; ori r15, r16, 5 ; lh r25, r26 }
    9358:	[0-9a-f]* 	{ rli r5, r6, 5 ; srai r15, r16, 5 ; lh r25, r26 }
    9360:	[0-9a-f]* 	{ rli r5, r6, 5 ; rl r15, r16, r17 ; lh_u r25, r26 }
    9368:	[0-9a-f]* 	{ rli r5, r6, 5 ; sub r15, r16, r17 ; lh_u r25, r26 }
    9370:	[0-9a-f]* 	{ rli r5, r6, 5 ; or r15, r16, r17 ; lw r25, r26 }
    9378:	[0-9a-f]* 	{ rli r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
    9380:	[0-9a-f]* 	{ rli r5, r6, 5 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    9388:	[0-9a-f]* 	{ rli r5, r6, 5 ; move r15, r16 }
    9390:	[0-9a-f]* 	{ rli r5, r6, 5 ; mz r15, r16, r17 ; sb r25, r26 }
    9398:	[0-9a-f]* 	{ rli r5, r6, 5 ; nor r15, r16, r17 ; lw r25, r26 }
    93a0:	[0-9a-f]* 	{ rli r5, r6, 5 ; ori r15, r16, 5 ; lw r25, r26 }
    93a8:	[0-9a-f]* 	{ rli r5, r6, 5 ; movei r15, 5 ; prefetch r25 }
    93b0:	[0-9a-f]* 	{ rli r5, r6, 5 ; slte_u r15, r16, r17 ; prefetch r25 }
    93b8:	[0-9a-f]* 	{ rli r5, r6, 5 ; rli r15, r16, 5 ; lb r25, r26 }
    93c0:	[0-9a-f]* 	{ rli r5, r6, 5 ; s2a r15, r16, r17 ; lb r25, r26 }
    93c8:	[0-9a-f]* 	{ rli r5, r6, 5 ; sb r15, r16 }
    93d0:	[0-9a-f]* 	{ rli r5, r6, 5 ; s3a r15, r16, r17 ; sb r25, r26 }
    93d8:	[0-9a-f]* 	{ rli r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
    93e0:	[0-9a-f]* 	{ rli r5, r6, 5 ; seqi r15, r16, 5 ; sw r25, r26 }
    93e8:	[0-9a-f]* 	{ rli r5, r6, 5 ; rl r15, r16, r17 ; sh r25, r26 }
    93f0:	[0-9a-f]* 	{ rli r5, r6, 5 ; sub r15, r16, r17 ; sh r25, r26 }
    93f8:	[0-9a-f]* 	{ rli r5, r6, 5 ; shli r15, r16, 5 ; lw r25, r26 }
    9400:	[0-9a-f]* 	{ rli r5, r6, 5 ; shri r15, r16, 5 ; lb r25, r26 }
    9408:	[0-9a-f]* 	{ rli r5, r6, 5 ; slt r15, r16, r17 ; sw r25, r26 }
    9410:	[0-9a-f]* 	{ rli r5, r6, 5 ; slte r15, r16, r17 ; sb r25, r26 }
    9418:	[0-9a-f]* 	{ rli r5, r6, 5 ; slti r15, r16, 5 ; lb r25, r26 }
    9420:	[0-9a-f]* 	{ rli r5, r6, 5 ; sltib r15, r16, 5 }
    9428:	[0-9a-f]* 	{ rli r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
    9430:	[0-9a-f]* 	{ rli r5, r6, 5 ; sub r15, r16, r17 ; lb r25, r26 }
    9438:	[0-9a-f]* 	{ rli r5, r6, 5 ; sw r25, r26 }
    9440:	[0-9a-f]* 	{ rli r5, r6, 5 ; shr r15, r16, r17 ; sw r25, r26 }
    9448:	[0-9a-f]* 	{ rli r5, r6, 5 ; xor r15, r16, r17 ; lh_u r25, r26 }
    9450:	[0-9a-f]* 	{ s1a r15, r16, r17 ; addh r5, r6, r7 }
    9458:	[0-9a-f]* 	{ s1a r15, r16, r17 ; and r5, r6, r7 ; lb_u r25, r26 }
    9460:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; s1a r15, r16, r17 }
    9468:	[0-9a-f]* 	{ bytex r5, r6 ; s1a r15, r16, r17 ; sw r25, r26 }
    9470:	[0-9a-f]* 	{ ctz r5, r6 ; s1a r15, r16, r17 ; sb r25, r26 }
    9478:	[0-9a-f]* 	{ s1a r15, r16, r17 ; info 19 ; prefetch r25 }
    9480:	[0-9a-f]* 	{ s1a r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
    9488:	[0-9a-f]* 	{ s1a r15, r16, r17 ; rl r5, r6, r7 ; lb r25, r26 }
    9490:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sub r5, r6, r7 ; lb r25, r26 }
    9498:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    94a0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    94a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    94b0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
    94b8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; seqi r5, r6, 5 ; lh r25, r26 }
    94c0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; lh r25, r26 }
    94c8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    94d0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    94d8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    94e0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    94e8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    94f0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; minh r5, r6, r7 }
    94f8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; move r5, r6 ; lw r25, r26 }
    9500:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
    9508:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    9510:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; s1a r15, r16, r17 }
    9518:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
    9520:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s1a r15, r16, r17 ; lb r25, r26 }
    9528:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; s1a r15, r16, r17 }
    9530:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    9538:	[0-9a-f]* 	{ s1a r15, r16, r17 ; nop ; sb r25, r26 }
    9540:	[0-9a-f]* 	{ s1a r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    9548:	[0-9a-f]* 	{ pcnt r5, r6 ; s1a r15, r16, r17 ; lh r25, r26 }
    9550:	[0-9a-f]* 	{ s1a r15, r16, r17 ; movei r5, 5 ; prefetch r25 }
    9558:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s1a r5, r6, r7 ; prefetch r25 }
    9560:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s1a r15, r16, r17 ; prefetch r25 }
    9568:	[0-9a-f]* 	{ s1a r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
    9570:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
    9578:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; s1a r15, r16, r17 }
    9580:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    9588:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    9590:	[0-9a-f]* 	{ s1a r15, r16, r17 ; seq r5, r6, r7 ; lh r25, r26 }
    9598:	[0-9a-f]* 	{ s1a r15, r16, r17 ; seqib r5, r6, 5 }
    95a0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
    95a8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shli r5, r6, 5 ; sh r25, r26 }
    95b0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
    95b8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shli r5, r6, 5 }
    95c0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
    95c8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
    95d0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
    95d8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; slti r5, r6, 5 ; prefetch r25 }
    95e0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sne r5, r6, r7 ; lb_u r25, r26 }
    95e8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sra r5, r6, r7 }
    95f0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    95f8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; mnz r5, r6, r7 ; sw r25, r26 }
    9600:	[0-9a-f]* 	{ s1a r15, r16, r17 ; rl r5, r6, r7 ; sw r25, r26 }
    9608:	[0-9a-f]* 	{ s1a r15, r16, r17 ; sub r5, r6, r7 ; sw r25, r26 }
    9610:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    9618:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s1a r15, r16, r17 ; lh_u r25, r26 }
    9620:	[0-9a-f]* 	{ s1a r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    9628:	[0-9a-f]* 	{ s1a r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
    9630:	[0-9a-f]* 	{ s1a r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    9638:	[0-9a-f]* 	{ s1a r5, r6, r7 }
    9640:	[0-9a-f]* 	{ s1a r5, r6, r7 ; info 19 ; sw r25, r26 }
    9648:	[0-9a-f]* 	{ s1a r5, r6, r7 ; info 19 ; lb r25, r26 }
    9650:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    9658:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    9660:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    9668:	[0-9a-f]* 	{ s1a r5, r6, r7 ; info 19 ; lh r25, r26 }
    9670:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
    9678:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    9680:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    9688:	[0-9a-f]* 	{ s1a r5, r6, r7 ; ill ; lw r25, r26 }
    9690:	[0-9a-f]* 	{ s1a r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    9698:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mf }
    96a0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    96a8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; moveli.sn r15, 4660 }
    96b0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; nop ; sb r25, r26 }
    96b8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    96c0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    96c8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    96d0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    96d8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
    96e0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    96e8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; nop ; sb r25, r26 }
    96f0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    96f8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
    9700:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mnz r15, r16, r17 ; sh r25, r26 }
    9708:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt_u r15, r16, r17 ; sh r25, r26 }
    9710:	[0-9a-f]* 	{ s1a r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    9718:	[0-9a-f]* 	{ s1a r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
    9720:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    9728:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sltb r15, r16, r17 }
    9730:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    9738:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
    9740:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    9748:	[0-9a-f]* 	{ s1a r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    9750:	[0-9a-f]* 	{ s1a r5, r6, r7 ; subh r15, r16, r17 }
    9758:	[0-9a-f]* 	{ s1a r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    9760:	[0-9a-f]* 	{ s1a r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    9768:	[0-9a-f]* 	{ s2a r15, r16, r17 ; add r5, r6, r7 ; lw r25, r26 }
    9770:	[0-9a-f]* 	{ s2a r15, r16, r17 ; addib r5, r6, 5 }
    9778:	[0-9a-f]* 	{ s2a r15, r16, r17 ; andi r5, r6, 5 ; lh_u r25, r26 }
    9780:	[0-9a-f]* 	{ bytex r5, r6 ; s2a r15, r16, r17 ; lb r25, r26 }
    9788:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; s2a r15, r16, r17 }
    9790:	[0-9a-f]* 	{ s2a r15, r16, r17 ; sh r25, r26 }
    9798:	[0-9a-f]* 	{ s2a r15, r16, r17 ; and r5, r6, r7 ; lb r25, r26 }
    97a0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    97a8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slt_u r5, r6, r7 ; lb r25, r26 }
    97b0:	[0-9a-f]* 	{ bytex r5, r6 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    97b8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; nop ; lb_u r25, r26 }
    97c0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    97c8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; lh r25, r26 }
    97d0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; ori r5, r6, 5 ; lh r25, r26 }
    97d8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; sra r5, r6, r7 ; lh r25, r26 }
    97e0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; move r5, r6 ; lh_u r25, r26 }
    97e8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; rli r5, r6, 5 ; lh_u r25, r26 }
    97f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    97f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    9800:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s3a r5, r6, r7 ; lw r25, r26 }
    9808:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s2a r15, r16, r17 ; lw r25, r26 }
    9810:	[0-9a-f]* 	{ s2a r15, r16, r17 ; mnz r5, r6, r7 ; sw r25, r26 }
    9818:	[0-9a-f]* 	{ s2a r15, r16, r17 ; movei r5, 5 ; sb r25, r26 }
    9820:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    9828:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    9830:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
    9838:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    9840:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    9848:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    9850:	[0-9a-f]* 	{ s2a r15, r16, r17 ; mzb r5, r6, r7 }
    9858:	[0-9a-f]* 	{ s2a r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    9860:	[0-9a-f]* 	{ s2a r15, r16, r17 ; ori r5, r6, 5 ; sw r25, r26 }
    9868:	[0-9a-f]* 	{ bitx r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
    9870:	[0-9a-f]* 	{ s2a r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    9878:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slte_u r5, r6, r7 ; prefetch r25 }
    9880:	[0-9a-f]* 	{ s2a r15, r16, r17 ; rl r5, r6, r7 ; sh r25, r26 }
    9888:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s1a r5, r6, r7 ; sh r25, r26 }
    9890:	[0-9a-f]* 	{ s2a r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    9898:	[0-9a-f]* 	{ s2a r15, r16, r17 ; move r5, r6 ; sb r25, r26 }
    98a0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; rli r5, r6, 5 ; sb r25, r26 }
    98a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s2a r15, r16, r17 ; sb r25, r26 }
    98b0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; seqi r5, r6, 5 ; lh r25, r26 }
    98b8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; mnz r5, r6, r7 ; sh r25, r26 }
    98c0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; rl r5, r6, r7 ; sh r25, r26 }
    98c8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; sub r5, r6, r7 ; sh r25, r26 }
    98d0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shli r5, r6, 5 ; lb_u r25, r26 }
    98d8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shr r5, r6, r7 }
    98e0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slt r5, r6, r7 ; prefetch r25 }
    98e8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slte r5, r6, r7 ; lh_u r25, r26 }
    98f0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slteh_u r5, r6, r7 }
    98f8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
    9900:	[0-9a-f]* 	{ s2a r15, r16, r17 ; sra r5, r6, r7 ; lb_u r25, r26 }
    9908:	[0-9a-f]* 	{ s2a r15, r16, r17 ; srai r5, r6, 5 }
    9910:	[0-9a-f]* 	{ s2a r15, r16, r17 ; and r5, r6, r7 ; sw r25, r26 }
    9918:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    9920:	[0-9a-f]* 	{ s2a r15, r16, r17 ; slt_u r5, r6, r7 ; sw r25, r26 }
    9928:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
    9930:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
    9938:	[0-9a-f]* 	{ s2a r15, r16, r17 ; xor r5, r6, r7 ; prefetch r25 }
    9940:	[0-9a-f]* 	{ s2a r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
    9948:	[0-9a-f]* 	{ s2a r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
    9950:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lb_u r25, r26 }
    9958:	[0-9a-f]* 	{ s2a r5, r6, r7 ; info 19 ; lb r25, r26 }
    9960:	[0-9a-f]* 	{ s2a r5, r6, r7 ; jrp r15 }
    9968:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    9970:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lb_u r15, r16 }
    9978:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    9980:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lbadd_u r15, r16, 5 }
    9988:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
    9990:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lh_u r15, r16 }
    9998:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    99a0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lhadd_u r15, r16, 5 }
    99a8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
    99b0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; lw r25, r26 }
    99b8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
    99c0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
    99c8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; mzb r15, r16, r17 }
    99d0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    99d8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    99e0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
    99e8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    99f0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
    99f8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
    9a00:	[0-9a-f]* 	{ s2a r5, r6, r7 ; andi r15, r16, 5 ; sb r25, r26 }
    9a08:	[0-9a-f]* 	{ s2a r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    9a10:	[0-9a-f]* 	{ s2a r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    9a18:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sh r15, r16 }
    9a20:	[0-9a-f]* 	{ s2a r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    9a28:	[0-9a-f]* 	{ s2a r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    9a30:	[0-9a-f]* 	{ s2a r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    9a38:	[0-9a-f]* 	{ s2a r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    9a40:	[0-9a-f]* 	{ s2a r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
    9a48:	[0-9a-f]* 	{ s2a r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
    9a50:	[0-9a-f]* 	{ s2a r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    9a58:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
    9a60:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
    9a68:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    9a70:	[0-9a-f]* 	{ s2a r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    9a78:	[0-9a-f]* 	{ s2a r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    9a80:	[0-9a-f]* 	{ s2a r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    9a88:	[0-9a-f]* 	{ s3a r15, r16, r17 ; addi r5, r6, 5 ; lh r25, r26 }
    9a90:	[0-9a-f]* 	{ s3a r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
    9a98:	[0-9a-f]* 	{ bitx r5, r6 ; s3a r15, r16, r17 ; lh r25, r26 }
    9aa0:	[0-9a-f]* 	{ clz r5, r6 ; s3a r15, r16, r17 ; lh r25, r26 }
    9aa8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; s3a r15, r16, r17 }
    9ab0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; info 19 }
    9ab8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s3a r15, r16, r17 ; lb r25, r26 }
    9ac0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
    9ac8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s3a r15, r16, r17 ; lb r25, r26 }
    9ad0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
    9ad8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
    9ae0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; add r5, r6, r7 ; lh r25, r26 }
    9ae8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    9af0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shri r5, r6, 5 ; lh r25, r26 }
    9af8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; andi r5, r6, 5 ; lh_u r25, r26 }
    9b00:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s3a r15, r16, r17 ; lh_u r25, r26 }
    9b08:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte r5, r6, r7 ; lh_u r25, r26 }
    9b10:	[0-9a-f]* 	{ clz r5, r6 ; s3a r15, r16, r17 ; lw r25, r26 }
    9b18:	[0-9a-f]* 	{ s3a r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
    9b20:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slti_u r5, r6, 5 ; lw r25, r26 }
    9b28:	[0-9a-f]* 	{ s3a r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
    9b30:	[0-9a-f]* 	{ s3a r15, r16, r17 ; move r5, r6 ; sw r25, r26 }
    9b38:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    9b40:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    9b48:	[0-9a-f]* 	{ mulhl_uu r5, r6, r7 ; s3a r15, r16, r17 }
    9b50:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    9b58:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
    9b60:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    9b68:	[0-9a-f]* 	{ s3a r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
    9b70:	[0-9a-f]* 	{ s3a r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    9b78:	[0-9a-f]* 	{ s3a r15, r16, r17 ; ori r5, r6, 5 ; lb r25, r26 }
    9b80:	[0-9a-f]* 	{ pcnt r5, r6 ; s3a r15, r16, r17 ; sb r25, r26 }
    9b88:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s3a r15, r16, r17 ; prefetch r25 }
    9b90:	[0-9a-f]* 	{ s3a r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
    9b98:	[0-9a-f]* 	{ s3a r15, r16, r17 ; prefetch r25 }
    9ba0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; rli r5, r6, 5 }
    9ba8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; s2a r5, r6, r7 }
    9bb0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; andi r5, r6, 5 ; sb r25, r26 }
    9bb8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    9bc0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte r5, r6, r7 ; sb r25, r26 }
    9bc8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    9bd0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; and r5, r6, r7 ; sh r25, r26 }
    9bd8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    9be0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
    9be8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
    9bf0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shr r5, r6, r7 ; lb_u r25, r26 }
    9bf8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; shri r5, r6, 5 }
    9c00:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
    9c08:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte_u r5, r6, r7 ; prefetch r25 }
    9c10:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slti r5, r6, 5 }
    9c18:	[0-9a-f]* 	{ s3a r15, r16, r17 ; sne r5, r6, r7 ; prefetch r25 }
    9c20:	[0-9a-f]* 	{ s3a r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    9c28:	[0-9a-f]* 	{ s3a r15, r16, r17 ; sub r5, r6, r7 }
    9c30:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s3a r15, r16, r17 ; sw r25, r26 }
    9c38:	[0-9a-f]* 	{ s3a r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
    9c40:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s3a r15, r16, r17 ; sw r25, r26 }
    9c48:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s3a r15, r16, r17 ; sh r25, r26 }
    9c50:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s3a r15, r16, r17 ; sh r25, r26 }
    9c58:	[0-9a-f]* 	{ s3a r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    9c60:	[0-9a-f]* 	{ s3a r5, r6, r7 ; addli r15, r16, 4660 }
    9c68:	[0-9a-f]* 	{ s3a r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    9c70:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ill ; lh r25, r26 }
    9c78:	[0-9a-f]* 	{ s3a r5, r6, r7 ; inthh r15, r16, r17 }
    9c80:	[0-9a-f]* 	{ s3a r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
    9c88:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    9c90:	[0-9a-f]* 	{ s3a r5, r6, r7 ; nop ; lb_u r25, r26 }
    9c98:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    9ca0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    9ca8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
    9cb0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; nop ; lh_u r25, r26 }
    9cb8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti_u r15, r16, 5 ; lh_u r25, r26 }
    9cc0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
    9cc8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    9cd0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; minib_u r15, r16, 5 }
    9cd8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    9ce0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
    9ce8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
    9cf0:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    9cf8:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ill ; prefetch r25 }
    9d00:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
    9d08:	[0-9a-f]* 	{ s3a r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    9d10:	[0-9a-f]* 	{ s3a r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    9d18:	[0-9a-f]* 	{ s3a r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    9d20:	[0-9a-f]* 	{ s3a r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    9d28:	[0-9a-f]* 	{ s3a r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    9d30:	[0-9a-f]* 	{ s3a r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    9d38:	[0-9a-f]* 	{ s3a r5, r6, r7 ; nop ; sh r25, r26 }
    9d40:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    9d48:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shli r15, r16, 5 ; lb r25, r26 }
    9d50:	[0-9a-f]* 	{ s3a r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
    9d58:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
    9d60:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
    9d68:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slteh r15, r16, r17 }
    9d70:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    9d78:	[0-9a-f]* 	{ s3a r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
    9d80:	[0-9a-f]* 	{ s3a r5, r6, r7 ; srai r15, r16, 5 ; sw r25, r26 }
    9d88:	[0-9a-f]* 	{ s3a r5, r6, r7 ; add r15, r16, r17 ; sw r25, r26 }
    9d90:	[0-9a-f]* 	{ s3a r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    9d98:	[0-9a-f]* 	{ s3a r5, r6, r7 ; wh64 r15 }
    9da0:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; addli r15, r16, 4660 }
    9da8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; jalr r15 }
    9db0:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; maxih r15, r16, 5 }
    9db8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; nor r15, r16, r17 }
    9dc0:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; seqib r15, r16, 5 }
    9dc8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; slte r15, r16, r17 }
    9dd0:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; srai r15, r16, 5 }
    9dd8:	[0-9a-f]* 	{ sadah r5, r6, r7 ; addi r15, r16, 5 }
    9de0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; intlh r15, r16, r17 }
    9de8:	[0-9a-f]* 	{ sadah r5, r6, r7 ; maxb_u r15, r16, r17 }
    9df0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; mzb r15, r16, r17 }
    9df8:	[0-9a-f]* 	{ sadah r5, r6, r7 ; seqb r15, r16, r17 }
    9e00:	[0-9a-f]* 	{ sadah r5, r6, r7 ; slt_u r15, r16, r17 }
    9e08:	[0-9a-f]* 	{ sadah r5, r6, r7 ; sra r15, r16, r17 }
    9e10:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; addbs_u r15, r16, r17 }
    9e18:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; inthb r15, r16, r17 }
    9e20:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; lw_na r15, r16 }
    9e28:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; moveli.sn r15, 4660 }
    9e30:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; sb r15, r16 }
    9e38:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; shrib r15, r16, 5 }
    9e40:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; sne r15, r16, r17 }
    9e48:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; xori r15, r16, 5 }
    9e50:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; ill }
    9e58:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; lhadd_u r15, r16, 5 }
    9e60:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; move r15, r16 }
    9e68:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; s1a r15, r16, r17 }
    9e70:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; shrb r15, r16, r17 }
    9e78:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; sltib_u r15, r16, 5 }
    9e80:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; tns r15, r16 }
    9e88:	[0-9a-f]* 	{ sadh r5, r6, r7 ; flush r15 }
    9e90:	[0-9a-f]* 	{ sadh r5, r6, r7 ; lh r15, r16 }
    9e98:	[0-9a-f]* 	{ sadh r5, r6, r7 ; mnz r15, r16, r17 }
    9ea0:	[0-9a-f]* 	{ sadh r5, r6, r7 ; raise }
    9ea8:	[0-9a-f]* 	{ sadh r5, r6, r7 ; shlib r15, r16, 5 }
    9eb0:	[0-9a-f]* 	{ sadh r5, r6, r7 ; slti r15, r16, 5 }
    9eb8:	[0-9a-f]* 	{ sadh r5, r6, r7 ; subs r15, r16, r17 }
    9ec0:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; auli r15, r16, 4660 }
    9ec8:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; lb_u r15, r16 }
    9ed0:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; minib_u r15, r16, 5 }
    9ed8:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; packhs r15, r16, r17 }
    9ee0:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; shlb r15, r16, r17 }
    9ee8:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; slteh_u r15, r16, r17 }
    9ef0:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; subbs_u r15, r16, r17 }
    9ef8:	[0-9a-f]* 	{ adds r5, r6, r7 ; sb r15, r16 }
    9f00:	[0-9a-f]* 	{ intlb r5, r6, r7 ; sb r15, r16 }
    9f08:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sb r15, r16 }
    9f10:	[0-9a-f]* 	{ mulllsa_uu r5, r6, r7 ; sb r15, r16 }
    9f18:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; sb r15, r16 }
    9f20:	[0-9a-f]* 	{ shrh r5, r6, r7 ; sb r15, r16 }
    9f28:	[0-9a-f]* 	{ sltih r5, r6, 5 ; sb r15, r16 }
    9f30:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sb r15, r16 }
    9f38:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    9f40:	[0-9a-f]* 	{ add r15, r16, r17 ; shl r5, r6, r7 ; sb r25, r26 }
    9f48:	[0-9a-f]* 	{ add r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    9f50:	[0-9a-f]* 	{ add r5, r6, r7 ; seq r15, r16, r17 ; sb r25, r26 }
    9f58:	[0-9a-f]* 	{ addi r15, r16, 5 ; and r5, r6, r7 ; sb r25, r26 }
    9f60:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; addi r15, r16, 5 ; sb r25, r26 }
    9f68:	[0-9a-f]* 	{ addi r15, r16, 5 ; slt_u r5, r6, r7 ; sb r25, r26 }
    9f70:	[0-9a-f]* 	{ addi r5, r6, 5 ; ill ; sb r25, r26 }
    9f78:	[0-9a-f]* 	{ addi r5, r6, 5 ; shri r15, r16, 5 ; sb r25, r26 }
    9f80:	[0-9a-f]* 	{ ctz r5, r6 ; and r15, r16, r17 ; sb r25, r26 }
    9f88:	[0-9a-f]* 	{ and r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    9f90:	[0-9a-f]* 	{ and r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
    9f98:	[0-9a-f]* 	{ and r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    9fa0:	[0-9a-f]* 	{ and r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    9fa8:	[0-9a-f]* 	{ andi r15, r16, 5 ; movei r5, 5 ; sb r25, r26 }
    9fb0:	[0-9a-f]* 	{ andi r15, r16, 5 ; s1a r5, r6, r7 ; sb r25, r26 }
    9fb8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
    9fc0:	[0-9a-f]* 	{ andi r5, r6, 5 ; rl r15, r16, r17 ; sb r25, r26 }
    9fc8:	[0-9a-f]* 	{ andi r5, r6, 5 ; sub r15, r16, r17 ; sb r25, r26 }
    9fd0:	[0-9a-f]* 	{ bitx r5, r6 ; s1a r15, r16, r17 ; sb r25, r26 }
    9fd8:	[0-9a-f]* 	{ bitx r5, r6 ; sb r25, r26 }
    9fe0:	[0-9a-f]* 	{ bytex r5, r6 ; s3a r15, r16, r17 ; sb r25, r26 }
    9fe8:	[0-9a-f]* 	{ clz r5, r6 ; addi r15, r16, 5 ; sb r25, r26 }
    9ff0:	[0-9a-f]* 	{ clz r5, r6 ; seqi r15, r16, 5 ; sb r25, r26 }
    9ff8:	[0-9a-f]* 	{ ctz r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
    a000:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 ; sb r25, r26 }
    a008:	[0-9a-f]* 	{ and r5, r6, r7 ; sb r25, r26 }
    a010:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sb r25, r26 }
    a018:	[0-9a-f]* 	{ rli r5, r6, 5 ; sb r25, r26 }
    a020:	[0-9a-f]* 	{ slt r5, r6, r7 ; sb r25, r26 }
    a028:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sb r25, r26 }
    a030:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; ill ; sb r25, r26 }
    a038:	[0-9a-f]* 	{ s3a r5, r6, r7 ; ill ; sb r25, r26 }
    a040:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; sb r25, r26 }
    a048:	[0-9a-f]* 	{ info 19 ; move r15, r16 ; sb r25, r26 }
    a050:	[0-9a-f]* 	{ info 19 ; or r15, r16, r17 ; sb r25, r26 }
    a058:	[0-9a-f]* 	{ info 19 ; shl r5, r6, r7 ; sb r25, r26 }
    a060:	[0-9a-f]* 	{ info 19 ; sne r5, r6, r7 ; sb r25, r26 }
    a068:	[0-9a-f]* 	{ clz r5, r6 ; mnz r15, r16, r17 ; sb r25, r26 }
    a070:	[0-9a-f]* 	{ mnz r15, r16, r17 ; nor r5, r6, r7 ; sb r25, r26 }
    a078:	[0-9a-f]* 	{ mnz r15, r16, r17 ; slti_u r5, r6, 5 ; sb r25, r26 }
    a080:	[0-9a-f]* 	{ mnz r5, r6, r7 ; movei r15, 5 ; sb r25, r26 }
    a088:	[0-9a-f]* 	{ mnz r5, r6, r7 ; slte_u r15, r16, r17 ; sb r25, r26 }
    a090:	[0-9a-f]* 	{ move r15, r16 ; move r5, r6 ; sb r25, r26 }
    a098:	[0-9a-f]* 	{ move r15, r16 ; rli r5, r6, 5 ; sb r25, r26 }
    a0a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; sb r25, r26 }
    a0a8:	[0-9a-f]* 	{ move r5, r6 ; ori r15, r16, 5 ; sb r25, r26 }
    a0b0:	[0-9a-f]* 	{ move r5, r6 ; srai r15, r16, 5 ; sb r25, r26 }
    a0b8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; movei r15, 5 ; sb r25, r26 }
    a0c0:	[0-9a-f]* 	{ movei r15, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    a0c8:	[0-9a-f]* 	{ movei r15, 5 ; sb r25, r26 }
    a0d0:	[0-9a-f]* 	{ movei r5, 5 ; s3a r15, r16, r17 ; sb r25, r26 }
    a0d8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; addi r15, r16, 5 ; sb r25, r26 }
    a0e0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seqi r15, r16, 5 ; sb r25, r26 }
    a0e8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; andi r15, r16, 5 ; sb r25, r26 }
    a0f0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    a0f8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ill ; sb r25, r26 }
    a100:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shri r15, r16, 5 ; sb r25, r26 }
    a108:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    a110:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a118:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; movei r15, 5 ; sb r25, r26 }
    a120:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slte_u r15, r16, r17 ; sb r25, r26 }
    a128:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; nop ; sb r25, r26 }
    a130:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    a138:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    a140:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    a148:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    a150:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    a158:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    a160:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sb r25, r26 }
    a168:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    a170:	[0-9a-f]* 	{ mvz r5, r6, r7 ; addi r15, r16, 5 ; sb r25, r26 }
    a178:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seqi r15, r16, 5 ; sb r25, r26 }
    a180:	[0-9a-f]* 	{ mz r15, r16, r17 ; andi r5, r6, 5 ; sb r25, r26 }
    a188:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    a190:	[0-9a-f]* 	{ mz r15, r16, r17 ; slte r5, r6, r7 ; sb r25, r26 }
    a198:	[0-9a-f]* 	{ mz r5, r6, r7 ; info 19 ; sb r25, r26 }
    a1a0:	[0-9a-f]* 	{ mz r5, r6, r7 ; slt r15, r16, r17 ; sb r25, r26 }
    a1a8:	[0-9a-f]* 	{ bitx r5, r6 ; nop ; sb r25, r26 }
    a1b0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; nop ; sb r25, r26 }
    a1b8:	[0-9a-f]* 	{ nop ; s2a r15, r16, r17 ; sb r25, r26 }
    a1c0:	[0-9a-f]* 	{ nop ; slte r15, r16, r17 ; sb r25, r26 }
    a1c8:	[0-9a-f]* 	{ nop ; xor r15, r16, r17 ; sb r25, r26 }
    a1d0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    a1d8:	[0-9a-f]* 	{ nor r15, r16, r17 ; shl r5, r6, r7 ; sb r25, r26 }
    a1e0:	[0-9a-f]* 	{ nor r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
    a1e8:	[0-9a-f]* 	{ nor r5, r6, r7 ; seq r15, r16, r17 ; sb r25, r26 }
    a1f0:	[0-9a-f]* 	{ or r15, r16, r17 ; and r5, r6, r7 ; sb r25, r26 }
    a1f8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    a200:	[0-9a-f]* 	{ or r15, r16, r17 ; slt_u r5, r6, r7 ; sb r25, r26 }
    a208:	[0-9a-f]* 	{ or r5, r6, r7 ; ill ; sb r25, r26 }
    a210:	[0-9a-f]* 	{ or r5, r6, r7 ; shri r15, r16, 5 ; sb r25, r26 }
    a218:	[0-9a-f]* 	{ ctz r5, r6 ; ori r15, r16, 5 ; sb r25, r26 }
    a220:	[0-9a-f]* 	{ ori r15, r16, 5 ; or r5, r6, r7 ; sb r25, r26 }
    a228:	[0-9a-f]* 	{ ori r15, r16, 5 ; sne r5, r6, r7 ; sb r25, r26 }
    a230:	[0-9a-f]* 	{ ori r5, r6, 5 ; mz r15, r16, r17 ; sb r25, r26 }
    a238:	[0-9a-f]* 	{ ori r5, r6, 5 ; slti r15, r16, 5 ; sb r25, r26 }
    a240:	[0-9a-f]* 	{ pcnt r5, r6 ; nor r15, r16, r17 ; sb r25, r26 }
    a248:	[0-9a-f]* 	{ pcnt r5, r6 ; sne r15, r16, r17 ; sb r25, r26 }
    a250:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
    a258:	[0-9a-f]* 	{ rl r15, r16, r17 ; s3a r5, r6, r7 ; sb r25, r26 }
    a260:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rl r15, r16, r17 ; sb r25, r26 }
    a268:	[0-9a-f]* 	{ rl r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    a270:	[0-9a-f]* 	{ rl r5, r6, r7 ; sb r25, r26 }
    a278:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; rli r15, r16, 5 ; sb r25, r26 }
    a280:	[0-9a-f]* 	{ rli r15, r16, 5 ; shr r5, r6, r7 ; sb r25, r26 }
    a288:	[0-9a-f]* 	{ rli r5, r6, 5 ; and r15, r16, r17 ; sb r25, r26 }
    a290:	[0-9a-f]* 	{ rli r5, r6, 5 ; shl r15, r16, r17 ; sb r25, r26 }
    a298:	[0-9a-f]* 	{ bitx r5, r6 ; s1a r15, r16, r17 ; sb r25, r26 }
    a2a0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    a2a8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
    a2b0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    a2b8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a2c0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; info 19 ; sb r25, r26 }
    a2c8:	[0-9a-f]* 	{ pcnt r5, r6 ; s2a r15, r16, r17 ; sb r25, r26 }
    a2d0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; srai r5, r6, 5 ; sb r25, r26 }
    a2d8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    a2e0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    a2e8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    a2f0:	[0-9a-f]* 	{ s3a r15, r16, r17 ; s3a r5, r6, r7 ; sb r25, r26 }
    a2f8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s3a r15, r16, r17 ; sb r25, r26 }
    a300:	[0-9a-f]* 	{ s3a r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    a308:	[0-9a-f]* 	{ s3a r5, r6, r7 ; sb r25, r26 }
    a310:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seq r15, r16, r17 ; sb r25, r26 }
    a318:	[0-9a-f]* 	{ seq r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    a320:	[0-9a-f]* 	{ seq r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    a328:	[0-9a-f]* 	{ seq r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    a330:	[0-9a-f]* 	{ bitx r5, r6 ; seqi r15, r16, 5 ; sb r25, r26 }
    a338:	[0-9a-f]* 	{ seqi r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
    a340:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slte_u r5, r6, r7 ; sb r25, r26 }
    a348:	[0-9a-f]* 	{ seqi r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    a350:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a358:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; sb r25, r26 }
    a360:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; sb r25, r26 }
    a368:	[0-9a-f]* 	{ shl r15, r16, r17 ; srai r5, r6, 5 ; sb r25, r26 }
    a370:	[0-9a-f]* 	{ shl r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    a378:	[0-9a-f]* 	{ shl r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    a380:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    a388:	[0-9a-f]* 	{ shli r15, r16, 5 ; s3a r5, r6, r7 ; sb r25, r26 }
    a390:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; sb r25, r26 }
    a398:	[0-9a-f]* 	{ shli r5, r6, 5 ; s1a r15, r16, r17 ; sb r25, r26 }
    a3a0:	[0-9a-f]* 	{ shli r5, r6, 5 ; sb r25, r26 }
    a3a8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    a3b0:	[0-9a-f]* 	{ shr r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    a3b8:	[0-9a-f]* 	{ shr r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    a3c0:	[0-9a-f]* 	{ shr r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    a3c8:	[0-9a-f]* 	{ bitx r5, r6 ; shri r15, r16, 5 ; sb r25, r26 }
    a3d0:	[0-9a-f]* 	{ shri r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
    a3d8:	[0-9a-f]* 	{ shri r15, r16, 5 ; slte_u r5, r6, r7 ; sb r25, r26 }
    a3e0:	[0-9a-f]* 	{ shri r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    a3e8:	[0-9a-f]* 	{ shri r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a3f0:	[0-9a-f]* 	{ slt r15, r16, r17 ; info 19 ; sb r25, r26 }
    a3f8:	[0-9a-f]* 	{ pcnt r5, r6 ; slt r15, r16, r17 ; sb r25, r26 }
    a400:	[0-9a-f]* 	{ slt r15, r16, r17 ; srai r5, r6, 5 ; sb r25, r26 }
    a408:	[0-9a-f]* 	{ slt r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
    a410:	[0-9a-f]* 	{ slt r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    a418:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a420:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; s3a r5, r6, r7 ; sb r25, r26 }
    a428:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a430:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    a438:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sb r25, r26 }
    a440:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    a448:	[0-9a-f]* 	{ slte r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    a450:	[0-9a-f]* 	{ slte r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    a458:	[0-9a-f]* 	{ slte r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    a460:	[0-9a-f]* 	{ bitx r5, r6 ; slte_u r15, r16, r17 ; sb r25, r26 }
    a468:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    a470:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
    a478:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    a480:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a488:	[0-9a-f]* 	{ slti r15, r16, 5 ; info 19 ; sb r25, r26 }
    a490:	[0-9a-f]* 	{ pcnt r5, r6 ; slti r15, r16, 5 ; sb r25, r26 }
    a498:	[0-9a-f]* 	{ slti r15, r16, 5 ; srai r5, r6, 5 ; sb r25, r26 }
    a4a0:	[0-9a-f]* 	{ slti r5, r6, 5 ; nor r15, r16, r17 ; sb r25, r26 }
    a4a8:	[0-9a-f]* 	{ slti r5, r6, 5 ; sne r15, r16, r17 ; sb r25, r26 }
    a4b0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    a4b8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; s3a r5, r6, r7 ; sb r25, r26 }
    a4c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti_u r15, r16, 5 ; sb r25, r26 }
    a4c8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; s1a r15, r16, r17 ; sb r25, r26 }
    a4d0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sb r25, r26 }
    a4d8:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    a4e0:	[0-9a-f]* 	{ sne r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    a4e8:	[0-9a-f]* 	{ sne r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
    a4f0:	[0-9a-f]* 	{ sne r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    a4f8:	[0-9a-f]* 	{ bitx r5, r6 ; sra r15, r16, r17 ; sb r25, r26 }
    a500:	[0-9a-f]* 	{ sra r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    a508:	[0-9a-f]* 	{ sra r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
    a510:	[0-9a-f]* 	{ sra r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    a518:	[0-9a-f]* 	{ sra r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    a520:	[0-9a-f]* 	{ srai r15, r16, 5 ; info 19 ; sb r25, r26 }
    a528:	[0-9a-f]* 	{ pcnt r5, r6 ; srai r15, r16, 5 ; sb r25, r26 }
    a530:	[0-9a-f]* 	{ srai r15, r16, 5 ; srai r5, r6, 5 ; sb r25, r26 }
    a538:	[0-9a-f]* 	{ srai r5, r6, 5 ; nor r15, r16, r17 ; sb r25, r26 }
    a540:	[0-9a-f]* 	{ srai r5, r6, 5 ; sne r15, r16, r17 ; sb r25, r26 }
    a548:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    a550:	[0-9a-f]* 	{ sub r15, r16, r17 ; s3a r5, r6, r7 ; sb r25, r26 }
    a558:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; sb r25, r26 }
    a560:	[0-9a-f]* 	{ sub r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
    a568:	[0-9a-f]* 	{ sub r5, r6, r7 ; sb r25, r26 }
    a570:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s3a r15, r16, r17 ; sb r25, r26 }
    a578:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; sb r25, r26 }
    a580:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; seqi r15, r16, 5 ; sb r25, r26 }
    a588:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
    a590:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; sb r25, r26 }
    a598:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; sb r25, r26 }
    a5a0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; sb r25, r26 }
    a5a8:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; sb r25, r26 }
    a5b0:	[0-9a-f]* 	{ xor r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    a5b8:	[0-9a-f]* 	{ xor r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
    a5c0:	[0-9a-f]* 	{ xor r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    a5c8:	[0-9a-f]* 	{ xor r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    a5d0:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; sbadd r15, r16, 5 }
    a5d8:	[0-9a-f]* 	{ maxb_u r5, r6, r7 ; sbadd r15, r16, 5 }
    a5e0:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; sbadd r15, r16, 5 }
    a5e8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sbadd r15, r16, 5 }
    a5f0:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; sbadd r15, r16, 5 }
    a5f8:	[0-9a-f]* 	{ shrib r5, r6, 5 ; sbadd r15, r16, 5 }
    a600:	[0-9a-f]* 	{ sne r5, r6, r7 ; sbadd r15, r16, 5 }
    a608:	[0-9a-f]* 	{ xori r5, r6, 5 ; sbadd r15, r16, 5 }
    a610:	[0-9a-f]* 	{ seq r15, r16, r17 ; addi r5, r6, 5 ; prefetch r25 }
    a618:	[0-9a-f]* 	{ seq r15, r16, r17 ; and r5, r6, r7 ; sw r25, r26 }
    a620:	[0-9a-f]* 	{ bitx r5, r6 ; seq r15, r16, r17 ; prefetch r25 }
    a628:	[0-9a-f]* 	{ clz r5, r6 ; seq r15, r16, r17 ; prefetch r25 }
    a630:	[0-9a-f]* 	{ seq r15, r16, r17 ; lh r25, r26 }
    a638:	[0-9a-f]* 	{ seq r15, r16, r17 ; inthh r5, r6, r7 }
    a640:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    a648:	[0-9a-f]* 	{ seq r15, r16, r17 ; shl r5, r6, r7 ; lb r25, r26 }
    a650:	[0-9a-f]* 	{ seq r15, r16, r17 ; add r5, r6, r7 ; lb_u r25, r26 }
    a658:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
    a660:	[0-9a-f]* 	{ seq r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    a668:	[0-9a-f]* 	{ seq r15, r16, r17 ; andi r5, r6, 5 ; lh r25, r26 }
    a670:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
    a678:	[0-9a-f]* 	{ seq r15, r16, r17 ; slte r5, r6, r7 ; lh r25, r26 }
    a680:	[0-9a-f]* 	{ clz r5, r6 ; seq r15, r16, r17 ; lh_u r25, r26 }
    a688:	[0-9a-f]* 	{ seq r15, r16, r17 ; nor r5, r6, r7 ; lh_u r25, r26 }
    a690:	[0-9a-f]* 	{ seq r15, r16, r17 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    a698:	[0-9a-f]* 	{ seq r15, r16, r17 ; info 19 ; lw r25, r26 }
    a6a0:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 ; lw r25, r26 }
    a6a8:	[0-9a-f]* 	{ seq r15, r16, r17 ; srai r5, r6, 5 ; lw r25, r26 }
    a6b0:	[0-9a-f]* 	{ seq r15, r16, r17 ; mnz r5, r6, r7 ; lh_u r25, r26 }
    a6b8:	[0-9a-f]* 	{ seq r15, r16, r17 ; movei r5, 5 ; lb_u r25, r26 }
    a6c0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seq r15, r16, r17 }
    a6c8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    a6d0:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; seq r15, r16, r17 }
    a6d8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    a6e0:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
    a6e8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    a6f0:	[0-9a-f]* 	{ seq r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
    a6f8:	[0-9a-f]* 	{ seq r15, r16, r17 ; nor r5, r6, r7 ; lh_u r25, r26 }
    a700:	[0-9a-f]* 	{ seq r15, r16, r17 ; ori r5, r6, 5 ; lh_u r25, r26 }
    a708:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 }
    a710:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
    a718:	[0-9a-f]* 	{ seq r15, r16, r17 ; shr r5, r6, r7 ; prefetch r25 }
    a720:	[0-9a-f]* 	{ seq r15, r16, r17 ; rl r5, r6, r7 ; lh r25, r26 }
    a728:	[0-9a-f]* 	{ seq r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    a730:	[0-9a-f]* 	{ seq r15, r16, r17 ; s3a r5, r6, r7 ; lh r25, r26 }
    a738:	[0-9a-f]* 	{ clz r5, r6 ; seq r15, r16, r17 ; sb r25, r26 }
    a740:	[0-9a-f]* 	{ seq r15, r16, r17 ; nor r5, r6, r7 ; sb r25, r26 }
    a748:	[0-9a-f]* 	{ seq r15, r16, r17 ; slti_u r5, r6, 5 ; sb r25, r26 }
    a750:	[0-9a-f]* 	{ seq r15, r16, r17 ; seq r5, r6, r7 }
    a758:	[0-9a-f]* 	{ bytex r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
    a760:	[0-9a-f]* 	{ seq r15, r16, r17 ; nop ; sh r25, r26 }
    a768:	[0-9a-f]* 	{ seq r15, r16, r17 ; slti r5, r6, 5 ; sh r25, r26 }
    a770:	[0-9a-f]* 	{ seq r15, r16, r17 ; shl r5, r6, r7 ; sw r25, r26 }
    a778:	[0-9a-f]* 	{ seq r15, r16, r17 ; shr r5, r6, r7 ; lw r25, r26 }
    a780:	[0-9a-f]* 	{ seq r15, r16, r17 ; slt r5, r6, r7 ; lb r25, r26 }
    a788:	[0-9a-f]* 	{ seq r15, r16, r17 ; sltb r5, r6, r7 }
    a790:	[0-9a-f]* 	{ seq r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
    a798:	[0-9a-f]* 	{ seq r15, r16, r17 ; slti_u r5, r6, 5 ; lh r25, r26 }
    a7a0:	[0-9a-f]* 	{ seq r15, r16, r17 ; sne r5, r6, r7 ; sw r25, r26 }
    a7a8:	[0-9a-f]* 	{ seq r15, r16, r17 ; srai r5, r6, 5 ; lw r25, r26 }
    a7b0:	[0-9a-f]* 	{ seq r15, r16, r17 ; subh r5, r6, r7 }
    a7b8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    a7c0:	[0-9a-f]* 	{ seq r15, r16, r17 ; shl r5, r6, r7 ; sw r25, r26 }
    a7c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; seq r15, r16, r17 ; lb r25, r26 }
    a7d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; seq r15, r16, r17 ; lb r25, r26 }
    a7d8:	[0-9a-f]* 	{ seq r15, r16, r17 ; xor r5, r6, r7 ; lb r25, r26 }
    a7e0:	[0-9a-f]* 	{ seq r5, r6, r7 ; add r15, r16, r17 }
    a7e8:	[0-9a-f]* 	{ seq r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    a7f0:	[0-9a-f]* 	{ seq r5, r6, r7 ; auli r15, r16, 4660 }
    a7f8:	[0-9a-f]* 	{ seq r5, r6, r7 ; ill ; prefetch r25 }
    a800:	[0-9a-f]* 	{ seq r5, r6, r7 ; inv r15 }
    a808:	[0-9a-f]* 	{ seq r5, r6, r7 ; or r15, r16, r17 ; lb r25, r26 }
    a810:	[0-9a-f]* 	{ seq r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
    a818:	[0-9a-f]* 	{ seq r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
    a820:	[0-9a-f]* 	{ seq r5, r6, r7 ; srai r15, r16, 5 ; lb_u r25, r26 }
    a828:	[0-9a-f]* 	{ seq r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    a830:	[0-9a-f]* 	{ seq r5, r6, r7 ; sra r15, r16, r17 ; lh r25, r26 }
    a838:	[0-9a-f]* 	{ seq r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    a840:	[0-9a-f]* 	{ seq r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    a848:	[0-9a-f]* 	{ seq r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    a850:	[0-9a-f]* 	{ seq r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    a858:	[0-9a-f]* 	{ seq r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
    a860:	[0-9a-f]* 	{ seq r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    a868:	[0-9a-f]* 	{ seq r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
    a870:	[0-9a-f]* 	{ seq r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
    a878:	[0-9a-f]* 	{ seq r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
    a880:	[0-9a-f]* 	{ seq r5, r6, r7 ; move r15, r16 ; prefetch r25 }
    a888:	[0-9a-f]* 	{ seq r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    a890:	[0-9a-f]* 	{ seq r5, r6, r7 ; rl r15, r16, r17 }
    a898:	[0-9a-f]* 	{ seq r5, r6, r7 ; s1a r15, r16, r17 }
    a8a0:	[0-9a-f]* 	{ seq r5, r6, r7 ; s3a r15, r16, r17 }
    a8a8:	[0-9a-f]* 	{ seq r5, r6, r7 ; s2a r15, r16, r17 ; sb r25, r26 }
    a8b0:	[0-9a-f]* 	{ seq r5, r6, r7 ; sbadd r15, r16, 5 }
    a8b8:	[0-9a-f]* 	{ seq r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    a8c0:	[0-9a-f]* 	{ seq r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    a8c8:	[0-9a-f]* 	{ seq r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    a8d0:	[0-9a-f]* 	{ seq r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    a8d8:	[0-9a-f]* 	{ seq r5, r6, r7 ; shrh r15, r16, r17 }
    a8e0:	[0-9a-f]* 	{ seq r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    a8e8:	[0-9a-f]* 	{ seq r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    a8f0:	[0-9a-f]* 	{ seq r5, r6, r7 ; slth_u r15, r16, r17 }
    a8f8:	[0-9a-f]* 	{ seq r5, r6, r7 ; slti_u r15, r16, 5 }
    a900:	[0-9a-f]* 	{ seq r5, r6, r7 ; sra r15, r16, r17 ; lh_u r25, r26 }
    a908:	[0-9a-f]* 	{ seq r5, r6, r7 ; sraih r15, r16, 5 }
    a910:	[0-9a-f]* 	{ seq r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
    a918:	[0-9a-f]* 	{ seq r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    a920:	[0-9a-f]* 	{ seq r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
    a928:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; seqb r15, r16, r17 }
    a930:	[0-9a-f]* 	{ seqb r15, r16, r17 ; intlh r5, r6, r7 }
    a938:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seqb r15, r16, r17 }
    a940:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seqb r15, r16, r17 }
    a948:	[0-9a-f]* 	{ sadah r5, r6, r7 ; seqb r15, r16, r17 }
    a950:	[0-9a-f]* 	{ seqb r15, r16, r17 ; shri r5, r6, 5 }
    a958:	[0-9a-f]* 	{ seqb r15, r16, r17 ; sltih_u r5, r6, 5 }
    a960:	[0-9a-f]* 	{ seqb r15, r16, r17 ; xor r5, r6, r7 }
    a968:	[0-9a-f]* 	{ seqb r5, r6, r7 ; icoh r15 }
    a970:	[0-9a-f]* 	{ seqb r5, r6, r7 ; lhadd r15, r16, 5 }
    a978:	[0-9a-f]* 	{ seqb r5, r6, r7 ; mnzh r15, r16, r17 }
    a980:	[0-9a-f]* 	{ seqb r5, r6, r7 ; rli r15, r16, 5 }
    a988:	[0-9a-f]* 	{ seqb r5, r6, r7 ; shr r15, r16, r17 }
    a990:	[0-9a-f]* 	{ seqb r5, r6, r7 ; sltib r15, r16, 5 }
    a998:	[0-9a-f]* 	{ seqb r5, r6, r7 ; swadd r15, r16, 5 }
    a9a0:	[0-9a-f]* 	{ seqh r15, r16, r17 ; auli r5, r6, 4660 }
    a9a8:	[0-9a-f]* 	{ seqh r15, r16, r17 ; maxih r5, r6, 5 }
    a9b0:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; seqh r15, r16, r17 }
    a9b8:	[0-9a-f]* 	{ seqh r15, r16, r17 ; mzh r5, r6, r7 }
    a9c0:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; seqh r15, r16, r17 }
    a9c8:	[0-9a-f]* 	{ seqh r15, r16, r17 ; slt_u r5, r6, r7 }
    a9d0:	[0-9a-f]* 	{ seqh r15, r16, r17 ; sra r5, r6, r7 }
    a9d8:	[0-9a-f]* 	{ seqh r5, r6, r7 ; addbs_u r15, r16, r17 }
    a9e0:	[0-9a-f]* 	{ seqh r5, r6, r7 ; inthb r15, r16, r17 }
    a9e8:	[0-9a-f]* 	{ seqh r5, r6, r7 ; lw_na r15, r16 }
    a9f0:	[0-9a-f]* 	{ seqh r5, r6, r7 ; moveli.sn r15, 4660 }
    a9f8:	[0-9a-f]* 	{ seqh r5, r6, r7 ; sb r15, r16 }
    aa00:	[0-9a-f]* 	{ seqh r5, r6, r7 ; shrib r15, r16, 5 }
    aa08:	[0-9a-f]* 	{ seqh r5, r6, r7 ; sne r15, r16, r17 }
    aa10:	[0-9a-f]* 	{ seqh r5, r6, r7 ; xori r15, r16, 5 }
    aa18:	[0-9a-f]* 	{ seqi r15, r16, 5 ; addi r5, r6, 5 ; prefetch r25 }
    aa20:	[0-9a-f]* 	{ seqi r15, r16, 5 ; and r5, r6, r7 ; sw r25, r26 }
    aa28:	[0-9a-f]* 	{ bitx r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
    aa30:	[0-9a-f]* 	{ clz r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
    aa38:	[0-9a-f]* 	{ seqi r15, r16, 5 ; lh r25, r26 }
    aa40:	[0-9a-f]* 	{ seqi r15, r16, 5 ; inthh r5, r6, r7 }
    aa48:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
    aa50:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shl r5, r6, r7 ; lb r25, r26 }
    aa58:	[0-9a-f]* 	{ seqi r15, r16, 5 ; add r5, r6, r7 ; lb_u r25, r26 }
    aa60:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seqi r15, r16, 5 ; lb_u r25, r26 }
    aa68:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
    aa70:	[0-9a-f]* 	{ seqi r15, r16, 5 ; andi r5, r6, 5 ; lh r25, r26 }
    aa78:	[0-9a-f]* 	{ mvz r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
    aa80:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slte r5, r6, r7 ; lh r25, r26 }
    aa88:	[0-9a-f]* 	{ clz r5, r6 ; seqi r15, r16, 5 ; lh_u r25, r26 }
    aa90:	[0-9a-f]* 	{ seqi r15, r16, 5 ; nor r5, r6, r7 ; lh_u r25, r26 }
    aa98:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    aaa0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; info 19 ; lw r25, r26 }
    aaa8:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; lw r25, r26 }
    aab0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; srai r5, r6, 5 ; lw r25, r26 }
    aab8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; mnz r5, r6, r7 ; lh_u r25, r26 }
    aac0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; movei r5, 5 ; lb_u r25, r26 }
    aac8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seqi r15, r16, 5 }
    aad0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    aad8:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; seqi r15, r16, 5 }
    aae0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    aae8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    aaf0:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    aaf8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; mz r5, r6, r7 ; prefetch r25 }
    ab00:	[0-9a-f]* 	{ seqi r15, r16, 5 ; nor r5, r6, r7 ; lh_u r25, r26 }
    ab08:	[0-9a-f]* 	{ seqi r15, r16, 5 ; ori r5, r6, 5 ; lh_u r25, r26 }
    ab10:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 }
    ab18:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    ab20:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shr r5, r6, r7 ; prefetch r25 }
    ab28:	[0-9a-f]* 	{ seqi r15, r16, 5 ; rl r5, r6, r7 ; lh r25, r26 }
    ab30:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    ab38:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s3a r5, r6, r7 ; lh r25, r26 }
    ab40:	[0-9a-f]* 	{ clz r5, r6 ; seqi r15, r16, 5 ; sb r25, r26 }
    ab48:	[0-9a-f]* 	{ seqi r15, r16, 5 ; nor r5, r6, r7 ; sb r25, r26 }
    ab50:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slti_u r5, r6, 5 ; sb r25, r26 }
    ab58:	[0-9a-f]* 	{ seqi r15, r16, 5 ; seq r5, r6, r7 }
    ab60:	[0-9a-f]* 	{ bytex r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
    ab68:	[0-9a-f]* 	{ seqi r15, r16, 5 ; nop ; sh r25, r26 }
    ab70:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slti r5, r6, 5 ; sh r25, r26 }
    ab78:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shl r5, r6, r7 ; sw r25, r26 }
    ab80:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shr r5, r6, r7 ; lw r25, r26 }
    ab88:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slt r5, r6, r7 ; lb r25, r26 }
    ab90:	[0-9a-f]* 	{ seqi r15, r16, 5 ; sltb r5, r6, r7 }
    ab98:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slte_u r5, r6, r7 ; sw r25, r26 }
    aba0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; slti_u r5, r6, 5 ; lh r25, r26 }
    aba8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; sne r5, r6, r7 ; sw r25, r26 }
    abb0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; srai r5, r6, 5 ; lw r25, r26 }
    abb8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; subh r5, r6, r7 }
    abc0:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    abc8:	[0-9a-f]* 	{ seqi r15, r16, 5 ; shl r5, r6, r7 ; sw r25, r26 }
    abd0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; seqi r15, r16, 5 ; lb r25, r26 }
    abd8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; seqi r15, r16, 5 ; lb r25, r26 }
    abe0:	[0-9a-f]* 	{ seqi r15, r16, 5 ; xor r5, r6, r7 ; lb r25, r26 }
    abe8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; add r15, r16, r17 }
    abf0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    abf8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; auli r15, r16, 4660 }
    ac00:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ill ; prefetch r25 }
    ac08:	[0-9a-f]* 	{ seqi r5, r6, 5 ; inv r15 }
    ac10:	[0-9a-f]* 	{ seqi r5, r6, 5 ; or r15, r16, r17 ; lb r25, r26 }
    ac18:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sra r15, r16, r17 ; lb r25, r26 }
    ac20:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; lb_u r25, r26 }
    ac28:	[0-9a-f]* 	{ seqi r5, r6, 5 ; srai r15, r16, 5 ; lb_u r25, r26 }
    ac30:	[0-9a-f]* 	{ seqi r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    ac38:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sra r15, r16, r17 ; lh r25, r26 }
    ac40:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; lh_u r25, r26 }
    ac48:	[0-9a-f]* 	{ seqi r5, r6, 5 ; srai r15, r16, 5 ; lh_u r25, r26 }
    ac50:	[0-9a-f]* 	{ seqi r5, r6, 5 ; nor r15, r16, r17 ; lw r25, r26 }
    ac58:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
    ac60:	[0-9a-f]* 	{ seqi r5, r6, 5 ; mnz r15, r16, r17 ; lb r25, r26 }
    ac68:	[0-9a-f]* 	{ seqi r5, r6, 5 ; move r15, r16 ; sw r25, r26 }
    ac70:	[0-9a-f]* 	{ seqi r5, r6, 5 ; mz r15, r16, r17 ; prefetch r25 }
    ac78:	[0-9a-f]* 	{ seqi r5, r6, 5 ; nor r15, r16, r17 ; lh_u r25, r26 }
    ac80:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; lh_u r25, r26 }
    ac88:	[0-9a-f]* 	{ seqi r5, r6, 5 ; move r15, r16 ; prefetch r25 }
    ac90:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slte r15, r16, r17 ; prefetch r25 }
    ac98:	[0-9a-f]* 	{ seqi r5, r6, 5 ; rl r15, r16, r17 }
    aca0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; s1a r15, r16, r17 }
    aca8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; s3a r15, r16, r17 }
    acb0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; s2a r15, r16, r17 ; sb r25, r26 }
    acb8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sbadd r15, r16, 5 }
    acc0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; seqi r15, r16, 5 ; sh r25, r26 }
    acc8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; sh r25, r26 }
    acd0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; srai r15, r16, 5 ; sh r25, r26 }
    acd8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
    ace0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; shrh r15, r16, r17 }
    ace8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slt r15, r16, r17 ; sh r25, r26 }
    acf0:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slte r15, r16, r17 ; prefetch r25 }
    acf8:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slth_u r15, r16, r17 }
    ad00:	[0-9a-f]* 	{ seqi r5, r6, 5 ; slti_u r15, r16, 5 }
    ad08:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sra r15, r16, r17 ; lh_u r25, r26 }
    ad10:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sraih r15, r16, 5 }
    ad18:	[0-9a-f]* 	{ seqi r5, r6, 5 ; andi r15, r16, 5 ; sw r25, r26 }
    ad20:	[0-9a-f]* 	{ seqi r5, r6, 5 ; shli r15, r16, 5 ; sw r25, r26 }
    ad28:	[0-9a-f]* 	{ seqi r5, r6, 5 ; xor r15, r16, r17 ; lh r25, r26 }
    ad30:	[0-9a-f]* 	{ adiffb_u r5, r6, r7 ; seqib r15, r16, 5 }
    ad38:	[0-9a-f]* 	{ seqib r15, r16, 5 ; intlh r5, r6, r7 }
    ad40:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; seqib r15, r16, 5 }
    ad48:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; seqib r15, r16, 5 }
    ad50:	[0-9a-f]* 	{ sadah r5, r6, r7 ; seqib r15, r16, 5 }
    ad58:	[0-9a-f]* 	{ seqib r15, r16, 5 ; shri r5, r6, 5 }
    ad60:	[0-9a-f]* 	{ seqib r15, r16, 5 ; sltih_u r5, r6, 5 }
    ad68:	[0-9a-f]* 	{ seqib r15, r16, 5 ; xor r5, r6, r7 }
    ad70:	[0-9a-f]* 	{ seqib r5, r6, 5 ; icoh r15 }
    ad78:	[0-9a-f]* 	{ seqib r5, r6, 5 ; lhadd r15, r16, 5 }
    ad80:	[0-9a-f]* 	{ seqib r5, r6, 5 ; mnzh r15, r16, r17 }
    ad88:	[0-9a-f]* 	{ seqib r5, r6, 5 ; rli r15, r16, 5 }
    ad90:	[0-9a-f]* 	{ seqib r5, r6, 5 ; shr r15, r16, r17 }
    ad98:	[0-9a-f]* 	{ seqib r5, r6, 5 ; sltib r15, r16, 5 }
    ada0:	[0-9a-f]* 	{ seqib r5, r6, 5 ; swadd r15, r16, 5 }
    ada8:	[0-9a-f]* 	{ seqih r15, r16, 5 ; auli r5, r6, 4660 }
    adb0:	[0-9a-f]* 	{ seqih r15, r16, 5 ; maxih r5, r6, 5 }
    adb8:	[0-9a-f]* 	{ mulhl_ss r5, r6, r7 ; seqih r15, r16, 5 }
    adc0:	[0-9a-f]* 	{ seqih r15, r16, 5 ; mzh r5, r6, r7 }
    adc8:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; seqih r15, r16, 5 }
    add0:	[0-9a-f]* 	{ seqih r15, r16, 5 ; slt_u r5, r6, r7 }
    add8:	[0-9a-f]* 	{ seqih r15, r16, 5 ; sra r5, r6, r7 }
    ade0:	[0-9a-f]* 	{ seqih r5, r6, 5 ; addbs_u r15, r16, r17 }
    ade8:	[0-9a-f]* 	{ seqih r5, r6, 5 ; inthb r15, r16, r17 }
    adf0:	[0-9a-f]* 	{ seqih r5, r6, 5 ; lw_na r15, r16 }
    adf8:	[0-9a-f]* 	{ seqih r5, r6, 5 ; moveli.sn r15, 4660 }
    ae00:	[0-9a-f]* 	{ seqih r5, r6, 5 ; sb r15, r16 }
    ae08:	[0-9a-f]* 	{ seqih r5, r6, 5 ; shrib r15, r16, 5 }
    ae10:	[0-9a-f]* 	{ seqih r5, r6, 5 ; sne r15, r16, r17 }
    ae18:	[0-9a-f]* 	{ seqih r5, r6, 5 ; xori r15, r16, 5 }
    ae20:	[0-9a-f]* 	{ bytex r5, r6 ; sh r15, r16 }
    ae28:	[0-9a-f]* 	{ minih r5, r6, 5 ; sh r15, r16 }
    ae30:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; sh r15, r16 }
    ae38:	[0-9a-f]* 	{ ori r5, r6, 5 ; sh r15, r16 }
    ae40:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sh r15, r16 }
    ae48:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sh r15, r16 }
    ae50:	[0-9a-f]* 	{ sraib r5, r6, 5 ; sh r15, r16 }
    ae58:	[0-9a-f]* 	{ clz r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
    ae60:	[0-9a-f]* 	{ add r15, r16, r17 ; nor r5, r6, r7 ; sh r25, r26 }
    ae68:	[0-9a-f]* 	{ add r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
    ae70:	[0-9a-f]* 	{ add r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    ae78:	[0-9a-f]* 	{ add r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    ae80:	[0-9a-f]* 	{ addi r15, r16, 5 ; move r5, r6 ; sh r25, r26 }
    ae88:	[0-9a-f]* 	{ addi r15, r16, 5 ; rli r5, r6, 5 ; sh r25, r26 }
    ae90:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addi r15, r16, 5 ; sh r25, r26 }
    ae98:	[0-9a-f]* 	{ addi r5, r6, 5 ; ori r15, r16, 5 ; sh r25, r26 }
    aea0:	[0-9a-f]* 	{ addi r5, r6, 5 ; srai r15, r16, 5 ; sh r25, r26 }
    aea8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; and r15, r16, r17 ; sh r25, r26 }
    aeb0:	[0-9a-f]* 	{ and r15, r16, r17 ; seqi r5, r6, 5 ; sh r25, r26 }
    aeb8:	[0-9a-f]* 	{ and r15, r16, r17 ; sh r25, r26 }
    aec0:	[0-9a-f]* 	{ and r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    aec8:	[0-9a-f]* 	{ andi r15, r16, 5 ; addi r5, r6, 5 ; sh r25, r26 }
    aed0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    aed8:	[0-9a-f]* 	{ andi r15, r16, 5 ; slt r5, r6, r7 ; sh r25, r26 }
    aee0:	[0-9a-f]* 	{ andi r5, r6, 5 ; sh r25, r26 }
    aee8:	[0-9a-f]* 	{ andi r5, r6, 5 ; shr r15, r16, r17 ; sh r25, r26 }
    aef0:	[0-9a-f]* 	{ bitx r5, r6 ; info 19 ; sh r25, r26 }
    aef8:	[0-9a-f]* 	{ bitx r5, r6 ; slt r15, r16, r17 ; sh r25, r26 }
    af00:	[0-9a-f]* 	{ bytex r5, r6 ; move r15, r16 ; sh r25, r26 }
    af08:	[0-9a-f]* 	{ bytex r5, r6 ; slte r15, r16, r17 ; sh r25, r26 }
    af10:	[0-9a-f]* 	{ clz r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
    af18:	[0-9a-f]* 	{ clz r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    af20:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; sh r25, r26 }
    af28:	[0-9a-f]* 	{ ctz r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
    af30:	[0-9a-f]* 	{ info 19 ; sh r25, r26 }
    af38:	[0-9a-f]* 	{ nop ; sh r25, r26 }
    af40:	[0-9a-f]* 	{ seqi r15, r16, 5 ; sh r25, r26 }
    af48:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sh r25, r26 }
    af50:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; sh r25, r26 }
    af58:	[0-9a-f]* 	{ mvz r5, r6, r7 ; ill ; sh r25, r26 }
    af60:	[0-9a-f]* 	{ slte r5, r6, r7 ; ill ; sh r25, r26 }
    af68:	[0-9a-f]* 	{ info 19 ; andi r15, r16, 5 ; sh r25, r26 }
    af70:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; info 19 ; sh r25, r26 }
    af78:	[0-9a-f]* 	{ info 19 ; s1a r15, r16, r17 ; sh r25, r26 }
    af80:	[0-9a-f]* 	{ info 19 ; slt_u r15, r16, r17 ; sh r25, r26 }
    af88:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; info 19 ; sh r25, r26 }
    af90:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; sh r25, r26 }
    af98:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seq r5, r6, r7 ; sh r25, r26 }
    afa0:	[0-9a-f]* 	{ mnz r15, r16, r17 ; xor r5, r6, r7 ; sh r25, r26 }
    afa8:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
    afb0:	[0-9a-f]* 	{ move r15, r16 ; add r5, r6, r7 ; sh r25, r26 }
    afb8:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; move r15, r16 ; sh r25, r26 }
    afc0:	[0-9a-f]* 	{ move r15, r16 ; shri r5, r6, 5 ; sh r25, r26 }
    afc8:	[0-9a-f]* 	{ move r5, r6 ; andi r15, r16, 5 ; sh r25, r26 }
    afd0:	[0-9a-f]* 	{ move r5, r6 ; shli r15, r16, 5 ; sh r25, r26 }
    afd8:	[0-9a-f]* 	{ bytex r5, r6 ; movei r15, 5 ; sh r25, r26 }
    afe0:	[0-9a-f]* 	{ movei r15, 5 ; nop ; sh r25, r26 }
    afe8:	[0-9a-f]* 	{ movei r15, 5 ; slti r5, r6, 5 ; sh r25, r26 }
    aff0:	[0-9a-f]* 	{ movei r5, 5 ; move r15, r16 ; sh r25, r26 }
    aff8:	[0-9a-f]* 	{ movei r5, 5 ; slte r15, r16, r17 ; sh r25, r26 }
    b000:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
    b008:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    b010:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; sh r25, r26 }
    b018:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    b020:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    b028:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    b030:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    b038:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    b040:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
    b048:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
    b050:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
    b058:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; and r15, r16, r17 ; sh r25, r26 }
    b060:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    b068:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; sh r25, r26 }
    b070:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shr r15, r16, r17 ; sh r25, r26 }
    b078:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; info 19 ; sh r25, r26 }
    b080:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b088:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; move r15, r16 ; sh r25, r26 }
    b090:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; sh r25, r26 }
    b098:	[0-9a-f]* 	{ mvz r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
    b0a0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    b0a8:	[0-9a-f]* 	{ mz r15, r16, r17 ; movei r5, 5 ; sh r25, r26 }
    b0b0:	[0-9a-f]* 	{ mz r15, r16, r17 ; s1a r5, r6, r7 ; sh r25, r26 }
    b0b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
    b0c0:	[0-9a-f]* 	{ mz r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    b0c8:	[0-9a-f]* 	{ mz r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    b0d0:	[0-9a-f]* 	{ nop ; move r15, r16 ; sh r25, r26 }
    b0d8:	[0-9a-f]* 	{ nop ; or r15, r16, r17 ; sh r25, r26 }
    b0e0:	[0-9a-f]* 	{ nop ; shl r5, r6, r7 ; sh r25, r26 }
    b0e8:	[0-9a-f]* 	{ nop ; sne r5, r6, r7 ; sh r25, r26 }
    b0f0:	[0-9a-f]* 	{ clz r5, r6 ; nor r15, r16, r17 ; sh r25, r26 }
    b0f8:	[0-9a-f]* 	{ nor r15, r16, r17 ; nor r5, r6, r7 ; sh r25, r26 }
    b100:	[0-9a-f]* 	{ nor r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
    b108:	[0-9a-f]* 	{ nor r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    b110:	[0-9a-f]* 	{ nor r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    b118:	[0-9a-f]* 	{ or r15, r16, r17 ; move r5, r6 ; sh r25, r26 }
    b120:	[0-9a-f]* 	{ or r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
    b128:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; or r15, r16, r17 ; sh r25, r26 }
    b130:	[0-9a-f]* 	{ or r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    b138:	[0-9a-f]* 	{ or r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    b140:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
    b148:	[0-9a-f]* 	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; sh r25, r26 }
    b150:	[0-9a-f]* 	{ ori r15, r16, 5 ; sh r25, r26 }
    b158:	[0-9a-f]* 	{ ori r5, r6, 5 ; s3a r15, r16, r17 ; sh r25, r26 }
    b160:	[0-9a-f]* 	{ pcnt r5, r6 ; addi r15, r16, 5 ; sh r25, r26 }
    b168:	[0-9a-f]* 	{ pcnt r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
    b170:	[0-9a-f]* 	{ rl r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    b178:	[0-9a-f]* 	{ mvz r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    b180:	[0-9a-f]* 	{ rl r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    b188:	[0-9a-f]* 	{ rl r5, r6, r7 ; info 19 ; sh r25, r26 }
    b190:	[0-9a-f]* 	{ rl r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b198:	[0-9a-f]* 	{ rli r15, r16, 5 ; sh r25, r26 }
    b1a0:	[0-9a-f]* 	{ rli r15, r16, 5 ; ori r5, r6, 5 ; sh r25, r26 }
    b1a8:	[0-9a-f]* 	{ rli r15, r16, 5 ; sra r5, r6, r7 ; sh r25, r26 }
    b1b0:	[0-9a-f]* 	{ rli r5, r6, 5 ; nop ; sh r25, r26 }
    b1b8:	[0-9a-f]* 	{ rli r5, r6, 5 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b1c0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
    b1c8:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    b1d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s1a r15, r16, r17 ; sh r25, r26 }
    b1d8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    b1e0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    b1e8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
    b1f0:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shli r5, r6, 5 ; sh r25, r26 }
    b1f8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; addi r15, r16, 5 ; sh r25, r26 }
    b200:	[0-9a-f]* 	{ s2a r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    b208:	[0-9a-f]* 	{ s3a r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    b210:	[0-9a-f]* 	{ mvz r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    b218:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    b220:	[0-9a-f]* 	{ s3a r5, r6, r7 ; info 19 ; sh r25, r26 }
    b228:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b230:	[0-9a-f]* 	{ seq r15, r16, r17 ; sh r25, r26 }
    b238:	[0-9a-f]* 	{ seq r15, r16, r17 ; ori r5, r6, 5 ; sh r25, r26 }
    b240:	[0-9a-f]* 	{ seq r15, r16, r17 ; sra r5, r6, r7 ; sh r25, r26 }
    b248:	[0-9a-f]* 	{ seq r5, r6, r7 ; nop ; sh r25, r26 }
    b250:	[0-9a-f]* 	{ seq r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b258:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    b260:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s2a r5, r6, r7 ; sh r25, r26 }
    b268:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
    b270:	[0-9a-f]* 	{ seqi r5, r6, 5 ; rli r15, r16, 5 ; sh r25, r26 }
    b278:	[0-9a-f]* 	{ seqi r5, r6, 5 ; xor r15, r16, r17 ; sh r25, r26 }
    b280:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    b288:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; sh r25, r26 }
    b290:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; sh r25, r26 }
    b298:	[0-9a-f]* 	{ shl r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    b2a0:	[0-9a-f]* 	{ shli r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    b2a8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    b2b0:	[0-9a-f]* 	{ shli r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    b2b8:	[0-9a-f]* 	{ shli r5, r6, 5 ; info 19 ; sh r25, r26 }
    b2c0:	[0-9a-f]* 	{ shli r5, r6, 5 ; slt r15, r16, r17 ; sh r25, r26 }
    b2c8:	[0-9a-f]* 	{ shr r15, r16, r17 ; sh r25, r26 }
    b2d0:	[0-9a-f]* 	{ shr r15, r16, r17 ; ori r5, r6, 5 ; sh r25, r26 }
    b2d8:	[0-9a-f]* 	{ shr r15, r16, r17 ; sra r5, r6, r7 ; sh r25, r26 }
    b2e0:	[0-9a-f]* 	{ shr r5, r6, r7 ; nop ; sh r25, r26 }
    b2e8:	[0-9a-f]* 	{ shr r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b2f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
    b2f8:	[0-9a-f]* 	{ shri r15, r16, 5 ; s2a r5, r6, r7 ; sh r25, r26 }
    b300:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
    b308:	[0-9a-f]* 	{ shri r5, r6, 5 ; rli r15, r16, 5 ; sh r25, r26 }
    b310:	[0-9a-f]* 	{ shri r5, r6, 5 ; xor r15, r16, r17 ; sh r25, r26 }
    b318:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b320:	[0-9a-f]* 	{ slt r15, r16, r17 ; shli r5, r6, 5 ; sh r25, r26 }
    b328:	[0-9a-f]* 	{ slt r5, r6, r7 ; addi r15, r16, 5 ; sh r25, r26 }
    b330:	[0-9a-f]* 	{ slt r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
    b338:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    b340:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt_u r15, r16, r17 ; sh r25, r26 }
    b348:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    b350:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; info 19 ; sh r25, r26 }
    b358:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b360:	[0-9a-f]* 	{ slte r15, r16, r17 ; sh r25, r26 }
    b368:	[0-9a-f]* 	{ slte r15, r16, r17 ; ori r5, r6, 5 ; sh r25, r26 }
    b370:	[0-9a-f]* 	{ slte r15, r16, r17 ; sra r5, r6, r7 ; sh r25, r26 }
    b378:	[0-9a-f]* 	{ slte r5, r6, r7 ; nop ; sh r25, r26 }
    b380:	[0-9a-f]* 	{ slte r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b388:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    b390:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    b398:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte_u r15, r16, r17 ; sh r25, r26 }
    b3a0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    b3a8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    b3b0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    b3b8:	[0-9a-f]* 	{ slti r15, r16, 5 ; shli r5, r6, 5 ; sh r25, r26 }
    b3c0:	[0-9a-f]* 	{ slti r5, r6, 5 ; addi r15, r16, 5 ; sh r25, r26 }
    b3c8:	[0-9a-f]* 	{ slti r5, r6, 5 ; seqi r15, r16, 5 ; sh r25, r26 }
    b3d0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    b3d8:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b3e0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    b3e8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; info 19 ; sh r25, r26 }
    b3f0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slt r15, r16, r17 ; sh r25, r26 }
    b3f8:	[0-9a-f]* 	{ sne r15, r16, r17 ; sh r25, r26 }
    b400:	[0-9a-f]* 	{ sne r15, r16, r17 ; ori r5, r6, 5 ; sh r25, r26 }
    b408:	[0-9a-f]* 	{ sne r15, r16, r17 ; sra r5, r6, r7 ; sh r25, r26 }
    b410:	[0-9a-f]* 	{ sne r5, r6, r7 ; nop ; sh r25, r26 }
    b418:	[0-9a-f]* 	{ sne r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
    b420:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
    b428:	[0-9a-f]* 	{ sra r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    b430:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; sh r25, r26 }
    b438:	[0-9a-f]* 	{ sra r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    b440:	[0-9a-f]* 	{ sra r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    b448:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    b450:	[0-9a-f]* 	{ srai r15, r16, 5 ; shli r5, r6, 5 ; sh r25, r26 }
    b458:	[0-9a-f]* 	{ srai r5, r6, 5 ; addi r15, r16, 5 ; sh r25, r26 }
    b460:	[0-9a-f]* 	{ srai r5, r6, 5 ; seqi r15, r16, 5 ; sh r25, r26 }
    b468:	[0-9a-f]* 	{ sub r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    b470:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    b478:	[0-9a-f]* 	{ sub r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    b480:	[0-9a-f]* 	{ sub r5, r6, r7 ; info 19 ; sh r25, r26 }
    b488:	[0-9a-f]* 	{ sub r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    b490:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; sh r25, r26 }
    b498:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; sh r25, r26 }
    b4a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
    b4a8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    b4b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; sh r25, r26 }
    b4b8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
    b4c0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; sh r25, r26 }
    b4c8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; sh r25, r26 }
    b4d0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    b4d8:	[0-9a-f]* 	{ xor r15, r16, r17 ; seqi r5, r6, 5 ; sh r25, r26 }
    b4e0:	[0-9a-f]* 	{ xor r15, r16, r17 ; sh r25, r26 }
    b4e8:	[0-9a-f]* 	{ xor r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
    b4f0:	[0-9a-f]* 	{ addb r5, r6, r7 ; shadd r15, r16, 5 }
    b4f8:	[0-9a-f]* 	{ crc32_32 r5, r6, r7 ; shadd r15, r16, 5 }
    b500:	[0-9a-f]* 	{ mnz r5, r6, r7 ; shadd r15, r16, 5 }
    b508:	[0-9a-f]* 	{ mulhla_us r5, r6, r7 ; shadd r15, r16, 5 }
    b510:	[0-9a-f]* 	{ packhb r5, r6, r7 ; shadd r15, r16, 5 }
    b518:	[0-9a-f]* 	{ seqih r5, r6, 5 ; shadd r15, r16, 5 }
    b520:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; shadd r15, r16, 5 }
    b528:	[0-9a-f]* 	{ sub r5, r6, r7 ; shadd r15, r16, 5 }
    b530:	[0-9a-f]* 	{ shl r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    b538:	[0-9a-f]* 	{ shl r15, r16, r17 ; adds r5, r6, r7 }
    b540:	[0-9a-f]* 	{ shl r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    b548:	[0-9a-f]* 	{ bytex r5, r6 ; shl r15, r16, r17 ; lw r25, r26 }
    b550:	[0-9a-f]* 	{ ctz r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    b558:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    b560:	[0-9a-f]* 	{ clz r5, r6 ; shl r15, r16, r17 ; lb r25, r26 }
    b568:	[0-9a-f]* 	{ shl r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    b570:	[0-9a-f]* 	{ shl r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    b578:	[0-9a-f]* 	{ shl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    b580:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; lb_u r25, r26 }
    b588:	[0-9a-f]* 	{ shl r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    b590:	[0-9a-f]* 	{ shl r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    b598:	[0-9a-f]* 	{ shl r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    b5a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
    b5a8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    b5b0:	[0-9a-f]* 	{ shl r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    b5b8:	[0-9a-f]* 	{ shl r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    b5c0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    b5c8:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    b5d0:	[0-9a-f]* 	{ shl r15, r16, r17 ; maxh r5, r6, r7 }
    b5d8:	[0-9a-f]* 	{ shl r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    b5e0:	[0-9a-f]* 	{ shl r15, r16, r17 ; moveli r5, 4660 }
    b5e8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    b5f0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    b5f8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    b600:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    b608:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
    b610:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    b618:	[0-9a-f]* 	{ shl r15, r16, r17 ; nop ; lh r25, r26 }
    b620:	[0-9a-f]* 	{ shl r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    b628:	[0-9a-f]* 	{ shl r15, r16, r17 ; packhs r5, r6, r7 }
    b630:	[0-9a-f]* 	{ shl r15, r16, r17 ; prefetch r25 }
    b638:	[0-9a-f]* 	{ shl r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    b640:	[0-9a-f]* 	{ shl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    b648:	[0-9a-f]* 	{ shl r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    b650:	[0-9a-f]* 	{ shl r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    b658:	[0-9a-f]* 	{ sadah r5, r6, r7 ; shl r15, r16, r17 }
    b660:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shl r15, r16, r17 ; sb r25, r26 }
    b668:	[0-9a-f]* 	{ shl r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    b670:	[0-9a-f]* 	{ shl r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    b678:	[0-9a-f]* 	{ shl r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    b680:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    b688:	[0-9a-f]* 	{ shl r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    b690:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; sh r25, r26 }
    b698:	[0-9a-f]* 	{ shl r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    b6a0:	[0-9a-f]* 	{ shl r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    b6a8:	[0-9a-f]* 	{ shl r15, r16, r17 ; slt r5, r6, r7 }
    b6b0:	[0-9a-f]* 	{ shl r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    b6b8:	[0-9a-f]* 	{ shl r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    b6c0:	[0-9a-f]* 	{ shl r15, r16, r17 ; sltib_u r5, r6, 5 }
    b6c8:	[0-9a-f]* 	{ shl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    b6d0:	[0-9a-f]* 	{ shl r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    b6d8:	[0-9a-f]* 	{ clz r5, r6 ; shl r15, r16, r17 ; sw r25, r26 }
    b6e0:	[0-9a-f]* 	{ shl r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    b6e8:	[0-9a-f]* 	{ shl r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    b6f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shl r15, r16, r17 }
    b6f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shl r15, r16, r17 }
    b700:	[0-9a-f]* 	{ shl r15, r16, r17 ; xor r5, r6, r7 }
    b708:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    b710:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 }
    b718:	[0-9a-f]* 	{ shl r5, r6, r7 ; prefetch r25 }
    b720:	[0-9a-f]* 	{ shl r5, r6, r7 ; info 19 ; lw r25, r26 }
    b728:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    b730:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    b738:	[0-9a-f]* 	{ shl r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    b740:	[0-9a-f]* 	{ shl r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    b748:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    b750:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    b758:	[0-9a-f]* 	{ shl r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    b760:	[0-9a-f]* 	{ shl r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    b768:	[0-9a-f]* 	{ shl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    b770:	[0-9a-f]* 	{ shl r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    b778:	[0-9a-f]* 	{ shl r5, r6, r7 ; maxb_u r15, r16, r17 }
    b780:	[0-9a-f]* 	{ shl r5, r6, r7 ; mnz r15, r16, r17 }
    b788:	[0-9a-f]* 	{ shl r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    b790:	[0-9a-f]* 	{ shl r5, r6, r7 ; nop ; lh r25, r26 }
    b798:	[0-9a-f]* 	{ shl r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    b7a0:	[0-9a-f]* 	{ shl r5, r6, r7 ; packhs r15, r16, r17 }
    b7a8:	[0-9a-f]* 	{ shl r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    b7b0:	[0-9a-f]* 	{ shl r5, r6, r7 ; prefetch r25 }
    b7b8:	[0-9a-f]* 	{ shl r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    b7c0:	[0-9a-f]* 	{ shl r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    b7c8:	[0-9a-f]* 	{ shl r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    b7d0:	[0-9a-f]* 	{ shl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    b7d8:	[0-9a-f]* 	{ shl r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    b7e0:	[0-9a-f]* 	{ shl r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    b7e8:	[0-9a-f]* 	{ shl r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    b7f0:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    b7f8:	[0-9a-f]* 	{ shl r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    b800:	[0-9a-f]* 	{ shl r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    b808:	[0-9a-f]* 	{ shl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    b810:	[0-9a-f]* 	{ shl r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    b818:	[0-9a-f]* 	{ shl r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    b820:	[0-9a-f]* 	{ shl r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    b828:	[0-9a-f]* 	{ shl r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    b830:	[0-9a-f]* 	{ shl r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    b838:	[0-9a-f]* 	{ shl r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    b840:	[0-9a-f]* 	{ shl r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    b848:	[0-9a-f]* 	{ shlb r15, r16, r17 ; add r5, r6, r7 }
    b850:	[0-9a-f]* 	{ clz r5, r6 ; shlb r15, r16, r17 }
    b858:	[0-9a-f]* 	{ shlb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    b860:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; shlb r15, r16, r17 }
    b868:	[0-9a-f]* 	{ shlb r15, r16, r17 ; packbs_u r5, r6, r7 }
    b870:	[0-9a-f]* 	{ shlb r15, r16, r17 ; seqib r5, r6, 5 }
    b878:	[0-9a-f]* 	{ shlb r15, r16, r17 ; slteb r5, r6, r7 }
    b880:	[0-9a-f]* 	{ shlb r15, r16, r17 ; sraih r5, r6, 5 }
    b888:	[0-9a-f]* 	{ shlb r5, r6, r7 ; addih r15, r16, 5 }
    b890:	[0-9a-f]* 	{ shlb r5, r6, r7 ; iret }
    b898:	[0-9a-f]* 	{ shlb r5, r6, r7 ; maxib_u r15, r16, 5 }
    b8a0:	[0-9a-f]* 	{ shlb r5, r6, r7 ; nop }
    b8a8:	[0-9a-f]* 	{ shlb r5, r6, r7 ; seqi r15, r16, 5 }
    b8b0:	[0-9a-f]* 	{ shlb r5, r6, r7 ; sltb_u r15, r16, r17 }
    b8b8:	[0-9a-f]* 	{ shlb r5, r6, r7 ; srah r15, r16, r17 }
    b8c0:	[0-9a-f]* 	{ shlh r15, r16, r17 ; addhs r5, r6, r7 }
    b8c8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; shlh r15, r16, r17 }
    b8d0:	[0-9a-f]* 	{ shlh r15, r16, r17 ; move r5, r6 }
    b8d8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shlh r15, r16, r17 }
    b8e0:	[0-9a-f]* 	{ pcnt r5, r6 ; shlh r15, r16, r17 }
    b8e8:	[0-9a-f]* 	{ shlh r15, r16, r17 ; shlh r5, r6, r7 }
    b8f0:	[0-9a-f]* 	{ shlh r15, r16, r17 ; slth r5, r6, r7 }
    b8f8:	[0-9a-f]* 	{ shlh r15, r16, r17 ; subh r5, r6, r7 }
    b900:	[0-9a-f]* 	{ shlh r5, r6, r7 ; and r15, r16, r17 }
    b908:	[0-9a-f]* 	{ shlh r5, r6, r7 ; jrp r15 }
    b910:	[0-9a-f]* 	{ shlh r5, r6, r7 ; minb_u r15, r16, r17 }
    b918:	[0-9a-f]* 	{ shlh r5, r6, r7 ; packbs_u r15, r16, r17 }
    b920:	[0-9a-f]* 	{ shlh r5, r6, r7 ; shadd r15, r16, 5 }
    b928:	[0-9a-f]* 	{ shlh r5, r6, r7 ; slteb_u r15, r16, r17 }
    b930:	[0-9a-f]* 	{ shlh r5, r6, r7 ; sub r15, r16, r17 }
    b938:	[0-9a-f]* 	{ shli r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
    b940:	[0-9a-f]* 	{ shli r15, r16, 5 ; adds r5, r6, r7 }
    b948:	[0-9a-f]* 	{ shli r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    b950:	[0-9a-f]* 	{ bytex r5, r6 ; shli r15, r16, 5 ; lw r25, r26 }
    b958:	[0-9a-f]* 	{ ctz r5, r6 ; shli r15, r16, 5 ; lh r25, r26 }
    b960:	[0-9a-f]* 	{ shli r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    b968:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; lb r25, r26 }
    b970:	[0-9a-f]* 	{ shli r15, r16, 5 ; nor r5, r6, r7 ; lb r25, r26 }
    b978:	[0-9a-f]* 	{ shli r15, r16, 5 ; slti_u r5, r6, 5 ; lb r25, r26 }
    b980:	[0-9a-f]* 	{ shli r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    b988:	[0-9a-f]* 	{ pcnt r5, r6 ; shli r15, r16, 5 ; lb_u r25, r26 }
    b990:	[0-9a-f]* 	{ shli r15, r16, 5 ; srai r5, r6, 5 ; lb_u r25, r26 }
    b998:	[0-9a-f]* 	{ shli r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    b9a0:	[0-9a-f]* 	{ shli r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    b9a8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; lh r25, r26 }
    b9b0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    b9b8:	[0-9a-f]* 	{ shli r15, r16, 5 ; seq r5, r6, r7 ; lh_u r25, r26 }
    b9c0:	[0-9a-f]* 	{ shli r15, r16, 5 ; xor r5, r6, r7 ; lh_u r25, r26 }
    b9c8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    b9d0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shli r5, r6, 5 ; lw r25, r26 }
    b9d8:	[0-9a-f]* 	{ shli r15, r16, 5 ; maxh r5, r6, r7 }
    b9e0:	[0-9a-f]* 	{ shli r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
    b9e8:	[0-9a-f]* 	{ shli r15, r16, 5 ; moveli r5, 4660 }
    b9f0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    b9f8:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    ba00:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    ba08:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    ba10:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
    ba18:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    ba20:	[0-9a-f]* 	{ shli r15, r16, 5 ; nop ; lh r25, r26 }
    ba28:	[0-9a-f]* 	{ shli r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    ba30:	[0-9a-f]* 	{ shli r15, r16, 5 ; packhs r5, r6, r7 }
    ba38:	[0-9a-f]* 	{ shli r15, r16, 5 ; prefetch r25 }
    ba40:	[0-9a-f]* 	{ shli r15, r16, 5 ; ori r5, r6, 5 ; prefetch r25 }
    ba48:	[0-9a-f]* 	{ shli r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    ba50:	[0-9a-f]* 	{ shli r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    ba58:	[0-9a-f]* 	{ shli r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    ba60:	[0-9a-f]* 	{ sadah r5, r6, r7 ; shli r15, r16, 5 }
    ba68:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
    ba70:	[0-9a-f]* 	{ shli r15, r16, 5 ; seq r5, r6, r7 ; sb r25, r26 }
    ba78:	[0-9a-f]* 	{ shli r15, r16, 5 ; xor r5, r6, r7 ; sb r25, r26 }
    ba80:	[0-9a-f]* 	{ shli r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    ba88:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    ba90:	[0-9a-f]* 	{ shli r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
    ba98:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; sh r25, r26 }
    baa0:	[0-9a-f]* 	{ shli r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    baa8:	[0-9a-f]* 	{ shli r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
    bab0:	[0-9a-f]* 	{ shli r15, r16, 5 ; slt r5, r6, r7 }
    bab8:	[0-9a-f]* 	{ shli r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    bac0:	[0-9a-f]* 	{ shli r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
    bac8:	[0-9a-f]* 	{ shli r15, r16, 5 ; sltib_u r5, r6, 5 }
    bad0:	[0-9a-f]* 	{ shli r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    bad8:	[0-9a-f]* 	{ shli r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
    bae0:	[0-9a-f]* 	{ clz r5, r6 ; shli r15, r16, 5 ; sw r25, r26 }
    bae8:	[0-9a-f]* 	{ shli r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
    baf0:	[0-9a-f]* 	{ shli r15, r16, 5 ; slti_u r5, r6, 5 ; sw r25, r26 }
    baf8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shli r15, r16, 5 }
    bb00:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shli r15, r16, 5 }
    bb08:	[0-9a-f]* 	{ shli r15, r16, 5 ; xor r5, r6, r7 }
    bb10:	[0-9a-f]* 	{ shli r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    bb18:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 }
    bb20:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch r25 }
    bb28:	[0-9a-f]* 	{ shli r5, r6, 5 ; info 19 ; lw r25, r26 }
    bb30:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    bb38:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    bb40:	[0-9a-f]* 	{ shli r5, r6, 5 ; andi r15, r16, 5 ; lb_u r25, r26 }
    bb48:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; lb_u r25, r26 }
    bb50:	[0-9a-f]* 	{ shli r5, r6, 5 ; and r15, r16, r17 ; lh r25, r26 }
    bb58:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    bb60:	[0-9a-f]* 	{ shli r5, r6, 5 ; andi r15, r16, 5 ; lh_u r25, r26 }
    bb68:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
    bb70:	[0-9a-f]* 	{ shli r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    bb78:	[0-9a-f]* 	{ shli r5, r6, 5 ; seqi r15, r16, 5 ; lw r25, r26 }
    bb80:	[0-9a-f]* 	{ shli r5, r6, 5 ; maxb_u r15, r16, r17 }
    bb88:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 }
    bb90:	[0-9a-f]* 	{ shli r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
    bb98:	[0-9a-f]* 	{ shli r5, r6, 5 ; nop ; lh r25, r26 }
    bba0:	[0-9a-f]* 	{ shli r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    bba8:	[0-9a-f]* 	{ shli r5, r6, 5 ; packhs r15, r16, r17 }
    bbb0:	[0-9a-f]* 	{ shli r5, r6, 5 ; s1a r15, r16, r17 ; prefetch r25 }
    bbb8:	[0-9a-f]* 	{ shli r5, r6, 5 ; prefetch r25 }
    bbc0:	[0-9a-f]* 	{ shli r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
    bbc8:	[0-9a-f]* 	{ shli r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
    bbd0:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    bbd8:	[0-9a-f]* 	{ shli r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    bbe0:	[0-9a-f]* 	{ shli r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
    bbe8:	[0-9a-f]* 	{ shli r5, r6, 5 ; andi r15, r16, 5 ; sh r25, r26 }
    bbf0:	[0-9a-f]* 	{ shli r5, r6, 5 ; shli r15, r16, 5 ; sh r25, r26 }
    bbf8:	[0-9a-f]* 	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
    bc00:	[0-9a-f]* 	{ shli r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
    bc08:	[0-9a-f]* 	{ shli r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
    bc10:	[0-9a-f]* 	{ shli r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    bc18:	[0-9a-f]* 	{ shli r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
    bc20:	[0-9a-f]* 	{ shli r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
    bc28:	[0-9a-f]* 	{ shli r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
    bc30:	[0-9a-f]* 	{ shli r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    bc38:	[0-9a-f]* 	{ shli r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
    bc40:	[0-9a-f]* 	{ shli r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    bc48:	[0-9a-f]* 	{ shli r5, r6, 5 ; sne r15, r16, r17 ; sw r25, r26 }
    bc50:	[0-9a-f]* 	{ shlib r15, r16, 5 ; add r5, r6, r7 }
    bc58:	[0-9a-f]* 	{ clz r5, r6 ; shlib r15, r16, 5 }
    bc60:	[0-9a-f]* 	{ shlib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
    bc68:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; shlib r15, r16, 5 }
    bc70:	[0-9a-f]* 	{ shlib r15, r16, 5 ; packbs_u r5, r6, r7 }
    bc78:	[0-9a-f]* 	{ shlib r15, r16, 5 ; seqib r5, r6, 5 }
    bc80:	[0-9a-f]* 	{ shlib r15, r16, 5 ; slteb r5, r6, r7 }
    bc88:	[0-9a-f]* 	{ shlib r15, r16, 5 ; sraih r5, r6, 5 }
    bc90:	[0-9a-f]* 	{ shlib r5, r6, 5 ; addih r15, r16, 5 }
    bc98:	[0-9a-f]* 	{ shlib r5, r6, 5 ; iret }
    bca0:	[0-9a-f]* 	{ shlib r5, r6, 5 ; maxib_u r15, r16, 5 }
    bca8:	[0-9a-f]* 	{ shlib r5, r6, 5 ; nop }
    bcb0:	[0-9a-f]* 	{ shlib r5, r6, 5 ; seqi r15, r16, 5 }
    bcb8:	[0-9a-f]* 	{ shlib r5, r6, 5 ; sltb_u r15, r16, r17 }
    bcc0:	[0-9a-f]* 	{ shlib r5, r6, 5 ; srah r15, r16, r17 }
    bcc8:	[0-9a-f]* 	{ shlih r15, r16, 5 ; addhs r5, r6, r7 }
    bcd0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; shlih r15, r16, 5 }
    bcd8:	[0-9a-f]* 	{ shlih r15, r16, 5 ; move r5, r6 }
    bce0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shlih r15, r16, 5 }
    bce8:	[0-9a-f]* 	{ pcnt r5, r6 ; shlih r15, r16, 5 }
    bcf0:	[0-9a-f]* 	{ shlih r15, r16, 5 ; shlh r5, r6, r7 }
    bcf8:	[0-9a-f]* 	{ shlih r15, r16, 5 ; slth r5, r6, r7 }
    bd00:	[0-9a-f]* 	{ shlih r15, r16, 5 ; subh r5, r6, r7 }
    bd08:	[0-9a-f]* 	{ shlih r5, r6, 5 ; and r15, r16, r17 }
    bd10:	[0-9a-f]* 	{ shlih r5, r6, 5 ; jrp r15 }
    bd18:	[0-9a-f]* 	{ shlih r5, r6, 5 ; minb_u r15, r16, r17 }
    bd20:	[0-9a-f]* 	{ shlih r5, r6, 5 ; packbs_u r15, r16, r17 }
    bd28:	[0-9a-f]* 	{ shlih r5, r6, 5 ; shadd r15, r16, 5 }
    bd30:	[0-9a-f]* 	{ shlih r5, r6, 5 ; slteb_u r15, r16, r17 }
    bd38:	[0-9a-f]* 	{ shlih r5, r6, 5 ; sub r15, r16, r17 }
    bd40:	[0-9a-f]* 	{ shr r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    bd48:	[0-9a-f]* 	{ shr r15, r16, r17 ; adds r5, r6, r7 }
    bd50:	[0-9a-f]* 	{ shr r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    bd58:	[0-9a-f]* 	{ bytex r5, r6 ; shr r15, r16, r17 ; lw r25, r26 }
    bd60:	[0-9a-f]* 	{ ctz r5, r6 ; shr r15, r16, r17 ; lh r25, r26 }
    bd68:	[0-9a-f]* 	{ shr r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    bd70:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; lb r25, r26 }
    bd78:	[0-9a-f]* 	{ shr r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    bd80:	[0-9a-f]* 	{ shr r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    bd88:	[0-9a-f]* 	{ shr r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    bd90:	[0-9a-f]* 	{ pcnt r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
    bd98:	[0-9a-f]* 	{ shr r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    bda0:	[0-9a-f]* 	{ shr r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    bda8:	[0-9a-f]* 	{ shr r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    bdb0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shr r15, r16, r17 ; lh r25, r26 }
    bdb8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shr r15, r16, r17 ; lh_u r25, r26 }
    bdc0:	[0-9a-f]* 	{ shr r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    bdc8:	[0-9a-f]* 	{ shr r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    bdd0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
    bdd8:	[0-9a-f]* 	{ shr r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    bde0:	[0-9a-f]* 	{ shr r15, r16, r17 ; maxh r5, r6, r7 }
    bde8:	[0-9a-f]* 	{ shr r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    bdf0:	[0-9a-f]* 	{ shr r15, r16, r17 ; moveli r5, 4660 }
    bdf8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shr r15, r16, r17 ; sh r25, r26 }
    be00:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    be08:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shr r15, r16, r17 ; sh r25, r26 }
    be10:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    be18:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shr r15, r16, r17 ; prefetch r25 }
    be20:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
    be28:	[0-9a-f]* 	{ shr r15, r16, r17 ; nop ; lh r25, r26 }
    be30:	[0-9a-f]* 	{ shr r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    be38:	[0-9a-f]* 	{ shr r15, r16, r17 ; packhs r5, r6, r7 }
    be40:	[0-9a-f]* 	{ shr r15, r16, r17 ; prefetch r25 }
    be48:	[0-9a-f]* 	{ shr r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    be50:	[0-9a-f]* 	{ shr r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    be58:	[0-9a-f]* 	{ shr r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    be60:	[0-9a-f]* 	{ shr r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    be68:	[0-9a-f]* 	{ sadah r5, r6, r7 ; shr r15, r16, r17 }
    be70:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
    be78:	[0-9a-f]* 	{ shr r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    be80:	[0-9a-f]* 	{ shr r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    be88:	[0-9a-f]* 	{ shr r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    be90:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shr r15, r16, r17 ; sh r25, r26 }
    be98:	[0-9a-f]* 	{ shr r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    bea0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shr r15, r16, r17 ; sh r25, r26 }
    bea8:	[0-9a-f]* 	{ shr r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    beb0:	[0-9a-f]* 	{ shr r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    beb8:	[0-9a-f]* 	{ shr r15, r16, r17 ; slt r5, r6, r7 }
    bec0:	[0-9a-f]* 	{ shr r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    bec8:	[0-9a-f]* 	{ shr r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    bed0:	[0-9a-f]* 	{ shr r15, r16, r17 ; sltib_u r5, r6, 5 }
    bed8:	[0-9a-f]* 	{ shr r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    bee0:	[0-9a-f]* 	{ shr r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    bee8:	[0-9a-f]* 	{ clz r5, r6 ; shr r15, r16, r17 ; sw r25, r26 }
    bef0:	[0-9a-f]* 	{ shr r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    bef8:	[0-9a-f]* 	{ shr r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    bf00:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shr r15, r16, r17 }
    bf08:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shr r15, r16, r17 }
    bf10:	[0-9a-f]* 	{ shr r15, r16, r17 ; xor r5, r6, r7 }
    bf18:	[0-9a-f]* 	{ shr r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    bf20:	[0-9a-f]* 	{ shr r5, r6, r7 ; and r15, r16, r17 }
    bf28:	[0-9a-f]* 	{ shr r5, r6, r7 ; prefetch r25 }
    bf30:	[0-9a-f]* 	{ shr r5, r6, r7 ; info 19 ; lw r25, r26 }
    bf38:	[0-9a-f]* 	{ shr r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    bf40:	[0-9a-f]* 	{ shr r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    bf48:	[0-9a-f]* 	{ shr r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    bf50:	[0-9a-f]* 	{ shr r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    bf58:	[0-9a-f]* 	{ shr r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    bf60:	[0-9a-f]* 	{ shr r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    bf68:	[0-9a-f]* 	{ shr r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    bf70:	[0-9a-f]* 	{ shr r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    bf78:	[0-9a-f]* 	{ shr r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    bf80:	[0-9a-f]* 	{ shr r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    bf88:	[0-9a-f]* 	{ shr r5, r6, r7 ; maxb_u r15, r16, r17 }
    bf90:	[0-9a-f]* 	{ shr r5, r6, r7 ; mnz r15, r16, r17 }
    bf98:	[0-9a-f]* 	{ shr r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    bfa0:	[0-9a-f]* 	{ shr r5, r6, r7 ; nop ; lh r25, r26 }
    bfa8:	[0-9a-f]* 	{ shr r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    bfb0:	[0-9a-f]* 	{ shr r5, r6, r7 ; packhs r15, r16, r17 }
    bfb8:	[0-9a-f]* 	{ shr r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    bfc0:	[0-9a-f]* 	{ shr r5, r6, r7 ; prefetch r25 }
    bfc8:	[0-9a-f]* 	{ shr r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    bfd0:	[0-9a-f]* 	{ shr r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    bfd8:	[0-9a-f]* 	{ shr r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    bfe0:	[0-9a-f]* 	{ shr r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    bfe8:	[0-9a-f]* 	{ shr r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    bff0:	[0-9a-f]* 	{ shr r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    bff8:	[0-9a-f]* 	{ shr r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    c000:	[0-9a-f]* 	{ shr r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    c008:	[0-9a-f]* 	{ shr r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    c010:	[0-9a-f]* 	{ shr r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    c018:	[0-9a-f]* 	{ shr r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c020:	[0-9a-f]* 	{ shr r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    c028:	[0-9a-f]* 	{ shr r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    c030:	[0-9a-f]* 	{ shr r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    c038:	[0-9a-f]* 	{ shr r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    c040:	[0-9a-f]* 	{ shr r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    c048:	[0-9a-f]* 	{ shr r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    c050:	[0-9a-f]* 	{ shr r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    c058:	[0-9a-f]* 	{ shrb r15, r16, r17 ; add r5, r6, r7 }
    c060:	[0-9a-f]* 	{ clz r5, r6 ; shrb r15, r16, r17 }
    c068:	[0-9a-f]* 	{ shrb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    c070:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; shrb r15, r16, r17 }
    c078:	[0-9a-f]* 	{ shrb r15, r16, r17 ; packbs_u r5, r6, r7 }
    c080:	[0-9a-f]* 	{ shrb r15, r16, r17 ; seqib r5, r6, 5 }
    c088:	[0-9a-f]* 	{ shrb r15, r16, r17 ; slteb r5, r6, r7 }
    c090:	[0-9a-f]* 	{ shrb r15, r16, r17 ; sraih r5, r6, 5 }
    c098:	[0-9a-f]* 	{ shrb r5, r6, r7 ; addih r15, r16, 5 }
    c0a0:	[0-9a-f]* 	{ shrb r5, r6, r7 ; iret }
    c0a8:	[0-9a-f]* 	{ shrb r5, r6, r7 ; maxib_u r15, r16, 5 }
    c0b0:	[0-9a-f]* 	{ shrb r5, r6, r7 ; nop }
    c0b8:	[0-9a-f]* 	{ shrb r5, r6, r7 ; seqi r15, r16, 5 }
    c0c0:	[0-9a-f]* 	{ shrb r5, r6, r7 ; sltb_u r15, r16, r17 }
    c0c8:	[0-9a-f]* 	{ shrb r5, r6, r7 ; srah r15, r16, r17 }
    c0d0:	[0-9a-f]* 	{ shrh r15, r16, r17 ; addhs r5, r6, r7 }
    c0d8:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; shrh r15, r16, r17 }
    c0e0:	[0-9a-f]* 	{ shrh r15, r16, r17 ; move r5, r6 }
    c0e8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shrh r15, r16, r17 }
    c0f0:	[0-9a-f]* 	{ pcnt r5, r6 ; shrh r15, r16, r17 }
    c0f8:	[0-9a-f]* 	{ shrh r15, r16, r17 ; shlh r5, r6, r7 }
    c100:	[0-9a-f]* 	{ shrh r15, r16, r17 ; slth r5, r6, r7 }
    c108:	[0-9a-f]* 	{ shrh r15, r16, r17 ; subh r5, r6, r7 }
    c110:	[0-9a-f]* 	{ shrh r5, r6, r7 ; and r15, r16, r17 }
    c118:	[0-9a-f]* 	{ shrh r5, r6, r7 ; jrp r15 }
    c120:	[0-9a-f]* 	{ shrh r5, r6, r7 ; minb_u r15, r16, r17 }
    c128:	[0-9a-f]* 	{ shrh r5, r6, r7 ; packbs_u r15, r16, r17 }
    c130:	[0-9a-f]* 	{ shrh r5, r6, r7 ; shadd r15, r16, 5 }
    c138:	[0-9a-f]* 	{ shrh r5, r6, r7 ; slteb_u r15, r16, r17 }
    c140:	[0-9a-f]* 	{ shrh r5, r6, r7 ; sub r15, r16, r17 }
    c148:	[0-9a-f]* 	{ shri r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
    c150:	[0-9a-f]* 	{ shri r15, r16, 5 ; adds r5, r6, r7 }
    c158:	[0-9a-f]* 	{ shri r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    c160:	[0-9a-f]* 	{ bytex r5, r6 ; shri r15, r16, 5 ; lw r25, r26 }
    c168:	[0-9a-f]* 	{ ctz r5, r6 ; shri r15, r16, 5 ; lh r25, r26 }
    c170:	[0-9a-f]* 	{ shri r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    c178:	[0-9a-f]* 	{ clz r5, r6 ; shri r15, r16, 5 ; lb r25, r26 }
    c180:	[0-9a-f]* 	{ shri r15, r16, 5 ; nor r5, r6, r7 ; lb r25, r26 }
    c188:	[0-9a-f]* 	{ shri r15, r16, 5 ; slti_u r5, r6, 5 ; lb r25, r26 }
    c190:	[0-9a-f]* 	{ shri r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    c198:	[0-9a-f]* 	{ pcnt r5, r6 ; shri r15, r16, 5 ; lb_u r25, r26 }
    c1a0:	[0-9a-f]* 	{ shri r15, r16, 5 ; srai r5, r6, 5 ; lb_u r25, r26 }
    c1a8:	[0-9a-f]* 	{ shri r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    c1b0:	[0-9a-f]* 	{ shri r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    c1b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shri r15, r16, 5 ; lh r25, r26 }
    c1c0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shri r15, r16, 5 ; lh_u r25, r26 }
    c1c8:	[0-9a-f]* 	{ shri r15, r16, 5 ; seq r5, r6, r7 ; lh_u r25, r26 }
    c1d0:	[0-9a-f]* 	{ shri r15, r16, 5 ; xor r5, r6, r7 ; lh_u r25, r26 }
    c1d8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    c1e0:	[0-9a-f]* 	{ shri r15, r16, 5 ; shli r5, r6, 5 ; lw r25, r26 }
    c1e8:	[0-9a-f]* 	{ shri r15, r16, 5 ; maxh r5, r6, r7 }
    c1f0:	[0-9a-f]* 	{ shri r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
    c1f8:	[0-9a-f]* 	{ shri r15, r16, 5 ; moveli r5, 4660 }
    c200:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
    c208:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; shri r15, r16, 5 ; sb r25, r26 }
    c210:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
    c218:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shri r15, r16, 5 ; sb r25, r26 }
    c220:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
    c228:	[0-9a-f]* 	{ mvz r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    c230:	[0-9a-f]* 	{ shri r15, r16, 5 ; nop ; lh r25, r26 }
    c238:	[0-9a-f]* 	{ shri r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    c240:	[0-9a-f]* 	{ shri r15, r16, 5 ; packhs r5, r6, r7 }
    c248:	[0-9a-f]* 	{ shri r15, r16, 5 ; prefetch r25 }
    c250:	[0-9a-f]* 	{ shri r15, r16, 5 ; ori r5, r6, 5 ; prefetch r25 }
    c258:	[0-9a-f]* 	{ shri r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    c260:	[0-9a-f]* 	{ shri r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    c268:	[0-9a-f]* 	{ shri r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    c270:	[0-9a-f]* 	{ sadah r5, r6, r7 ; shri r15, r16, 5 }
    c278:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; shri r15, r16, 5 ; sb r25, r26 }
    c280:	[0-9a-f]* 	{ shri r15, r16, 5 ; seq r5, r6, r7 ; sb r25, r26 }
    c288:	[0-9a-f]* 	{ shri r15, r16, 5 ; xor r5, r6, r7 ; sb r25, r26 }
    c290:	[0-9a-f]* 	{ shri r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    c298:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
    c2a0:	[0-9a-f]* 	{ shri r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
    c2a8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
    c2b0:	[0-9a-f]* 	{ shri r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    c2b8:	[0-9a-f]* 	{ shri r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
    c2c0:	[0-9a-f]* 	{ shri r15, r16, 5 ; slt r5, r6, r7 }
    c2c8:	[0-9a-f]* 	{ shri r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    c2d0:	[0-9a-f]* 	{ shri r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
    c2d8:	[0-9a-f]* 	{ shri r15, r16, 5 ; sltib_u r5, r6, 5 }
    c2e0:	[0-9a-f]* 	{ shri r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    c2e8:	[0-9a-f]* 	{ shri r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
    c2f0:	[0-9a-f]* 	{ clz r5, r6 ; shri r15, r16, 5 ; sw r25, r26 }
    c2f8:	[0-9a-f]* 	{ shri r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
    c300:	[0-9a-f]* 	{ shri r15, r16, 5 ; slti_u r5, r6, 5 ; sw r25, r26 }
    c308:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shri r15, r16, 5 }
    c310:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shri r15, r16, 5 }
    c318:	[0-9a-f]* 	{ shri r15, r16, 5 ; xor r5, r6, r7 }
    c320:	[0-9a-f]* 	{ shri r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    c328:	[0-9a-f]* 	{ shri r5, r6, 5 ; and r15, r16, r17 }
    c330:	[0-9a-f]* 	{ shri r5, r6, 5 ; prefetch r25 }
    c338:	[0-9a-f]* 	{ shri r5, r6, 5 ; info 19 ; lw r25, r26 }
    c340:	[0-9a-f]* 	{ shri r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    c348:	[0-9a-f]* 	{ shri r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    c350:	[0-9a-f]* 	{ shri r5, r6, 5 ; andi r15, r16, 5 ; lb_u r25, r26 }
    c358:	[0-9a-f]* 	{ shri r5, r6, 5 ; shli r15, r16, 5 ; lb_u r25, r26 }
    c360:	[0-9a-f]* 	{ shri r5, r6, 5 ; and r15, r16, r17 ; lh r25, r26 }
    c368:	[0-9a-f]* 	{ shri r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    c370:	[0-9a-f]* 	{ shri r5, r6, 5 ; andi r15, r16, 5 ; lh_u r25, r26 }
    c378:	[0-9a-f]* 	{ shri r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
    c380:	[0-9a-f]* 	{ shri r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    c388:	[0-9a-f]* 	{ shri r5, r6, 5 ; seqi r15, r16, 5 ; lw r25, r26 }
    c390:	[0-9a-f]* 	{ shri r5, r6, 5 ; maxb_u r15, r16, r17 }
    c398:	[0-9a-f]* 	{ shri r5, r6, 5 ; mnz r15, r16, r17 }
    c3a0:	[0-9a-f]* 	{ shri r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
    c3a8:	[0-9a-f]* 	{ shri r5, r6, 5 ; nop ; lh r25, r26 }
    c3b0:	[0-9a-f]* 	{ shri r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    c3b8:	[0-9a-f]* 	{ shri r5, r6, 5 ; packhs r15, r16, r17 }
    c3c0:	[0-9a-f]* 	{ shri r5, r6, 5 ; s1a r15, r16, r17 ; prefetch r25 }
    c3c8:	[0-9a-f]* 	{ shri r5, r6, 5 ; prefetch r25 }
    c3d0:	[0-9a-f]* 	{ shri r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
    c3d8:	[0-9a-f]* 	{ shri r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
    c3e0:	[0-9a-f]* 	{ shri r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    c3e8:	[0-9a-f]* 	{ shri r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c3f0:	[0-9a-f]* 	{ shri r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
    c3f8:	[0-9a-f]* 	{ shri r5, r6, 5 ; andi r15, r16, 5 ; sh r25, r26 }
    c400:	[0-9a-f]* 	{ shri r5, r6, 5 ; shli r15, r16, 5 ; sh r25, r26 }
    c408:	[0-9a-f]* 	{ shri r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
    c410:	[0-9a-f]* 	{ shri r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
    c418:	[0-9a-f]* 	{ shri r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
    c420:	[0-9a-f]* 	{ shri r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c428:	[0-9a-f]* 	{ shri r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
    c430:	[0-9a-f]* 	{ shri r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
    c438:	[0-9a-f]* 	{ shri r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
    c440:	[0-9a-f]* 	{ shri r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    c448:	[0-9a-f]* 	{ shri r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
    c450:	[0-9a-f]* 	{ shri r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    c458:	[0-9a-f]* 	{ shri r5, r6, 5 ; sne r15, r16, r17 ; sw r25, r26 }
    c460:	[0-9a-f]* 	{ shrib r15, r16, 5 ; add r5, r6, r7 }
    c468:	[0-9a-f]* 	{ clz r5, r6 ; shrib r15, r16, 5 }
    c470:	[0-9a-f]* 	{ shrib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
    c478:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; shrib r15, r16, 5 }
    c480:	[0-9a-f]* 	{ shrib r15, r16, 5 ; packbs_u r5, r6, r7 }
    c488:	[0-9a-f]* 	{ shrib r15, r16, 5 ; seqib r5, r6, 5 }
    c490:	[0-9a-f]* 	{ shrib r15, r16, 5 ; slteb r5, r6, r7 }
    c498:	[0-9a-f]* 	{ shrib r15, r16, 5 ; sraih r5, r6, 5 }
    c4a0:	[0-9a-f]* 	{ shrib r5, r6, 5 ; addih r15, r16, 5 }
    c4a8:	[0-9a-f]* 	{ shrib r5, r6, 5 ; iret }
    c4b0:	[0-9a-f]* 	{ shrib r5, r6, 5 ; maxib_u r15, r16, 5 }
    c4b8:	[0-9a-f]* 	{ shrib r5, r6, 5 ; nop }
    c4c0:	[0-9a-f]* 	{ shrib r5, r6, 5 ; seqi r15, r16, 5 }
    c4c8:	[0-9a-f]* 	{ shrib r5, r6, 5 ; sltb_u r15, r16, r17 }
    c4d0:	[0-9a-f]* 	{ shrib r5, r6, 5 ; srah r15, r16, r17 }
    c4d8:	[0-9a-f]* 	{ shrih r15, r16, 5 ; addhs r5, r6, r7 }
    c4e0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; shrih r15, r16, 5 }
    c4e8:	[0-9a-f]* 	{ shrih r15, r16, 5 ; move r5, r6 }
    c4f0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; shrih r15, r16, 5 }
    c4f8:	[0-9a-f]* 	{ pcnt r5, r6 ; shrih r15, r16, 5 }
    c500:	[0-9a-f]* 	{ shrih r15, r16, 5 ; shlh r5, r6, r7 }
    c508:	[0-9a-f]* 	{ shrih r15, r16, 5 ; slth r5, r6, r7 }
    c510:	[0-9a-f]* 	{ shrih r15, r16, 5 ; subh r5, r6, r7 }
    c518:	[0-9a-f]* 	{ shrih r5, r6, 5 ; and r15, r16, r17 }
    c520:	[0-9a-f]* 	{ shrih r5, r6, 5 ; jrp r15 }
    c528:	[0-9a-f]* 	{ shrih r5, r6, 5 ; minb_u r15, r16, r17 }
    c530:	[0-9a-f]* 	{ shrih r5, r6, 5 ; packbs_u r15, r16, r17 }
    c538:	[0-9a-f]* 	{ shrih r5, r6, 5 ; shadd r15, r16, 5 }
    c540:	[0-9a-f]* 	{ shrih r5, r6, 5 ; slteb_u r15, r16, r17 }
    c548:	[0-9a-f]* 	{ shrih r5, r6, 5 ; sub r15, r16, r17 }
    c550:	[0-9a-f]* 	{ slt r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    c558:	[0-9a-f]* 	{ slt r15, r16, r17 ; adds r5, r6, r7 }
    c560:	[0-9a-f]* 	{ slt r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    c568:	[0-9a-f]* 	{ bytex r5, r6 ; slt r15, r16, r17 ; lw r25, r26 }
    c570:	[0-9a-f]* 	{ ctz r5, r6 ; slt r15, r16, r17 ; lh r25, r26 }
    c578:	[0-9a-f]* 	{ slt r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    c580:	[0-9a-f]* 	{ clz r5, r6 ; slt r15, r16, r17 ; lb r25, r26 }
    c588:	[0-9a-f]* 	{ slt r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    c590:	[0-9a-f]* 	{ slt r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    c598:	[0-9a-f]* 	{ slt r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    c5a0:	[0-9a-f]* 	{ pcnt r5, r6 ; slt r15, r16, r17 ; lb_u r25, r26 }
    c5a8:	[0-9a-f]* 	{ slt r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    c5b0:	[0-9a-f]* 	{ slt r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    c5b8:	[0-9a-f]* 	{ slt r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    c5c0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; lh r25, r26 }
    c5c8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slt r15, r16, r17 ; lh_u r25, r26 }
    c5d0:	[0-9a-f]* 	{ slt r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    c5d8:	[0-9a-f]* 	{ slt r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    c5e0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
    c5e8:	[0-9a-f]* 	{ slt r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    c5f0:	[0-9a-f]* 	{ slt r15, r16, r17 ; maxh r5, r6, r7 }
    c5f8:	[0-9a-f]* 	{ slt r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    c600:	[0-9a-f]* 	{ slt r15, r16, r17 ; moveli r5, 4660 }
    c608:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    c610:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slt r15, r16, r17 ; sb r25, r26 }
    c618:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    c620:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slt r15, r16, r17 ; sb r25, r26 }
    c628:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
    c630:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
    c638:	[0-9a-f]* 	{ slt r15, r16, r17 ; nop ; lh r25, r26 }
    c640:	[0-9a-f]* 	{ slt r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    c648:	[0-9a-f]* 	{ slt r15, r16, r17 ; packhs r5, r6, r7 }
    c650:	[0-9a-f]* 	{ slt r15, r16, r17 ; prefetch r25 }
    c658:	[0-9a-f]* 	{ slt r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    c660:	[0-9a-f]* 	{ slt r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    c668:	[0-9a-f]* 	{ slt r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    c670:	[0-9a-f]* 	{ slt r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    c678:	[0-9a-f]* 	{ sadah r5, r6, r7 ; slt r15, r16, r17 }
    c680:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slt r15, r16, r17 ; sb r25, r26 }
    c688:	[0-9a-f]* 	{ slt r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    c690:	[0-9a-f]* 	{ slt r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    c698:	[0-9a-f]* 	{ slt r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    c6a0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
    c6a8:	[0-9a-f]* 	{ slt r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    c6b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slt r15, r16, r17 ; sh r25, r26 }
    c6b8:	[0-9a-f]* 	{ slt r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    c6c0:	[0-9a-f]* 	{ slt r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    c6c8:	[0-9a-f]* 	{ slt r15, r16, r17 ; slt r5, r6, r7 }
    c6d0:	[0-9a-f]* 	{ slt r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    c6d8:	[0-9a-f]* 	{ slt r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    c6e0:	[0-9a-f]* 	{ slt r15, r16, r17 ; sltib_u r5, r6, 5 }
    c6e8:	[0-9a-f]* 	{ slt r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    c6f0:	[0-9a-f]* 	{ slt r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    c6f8:	[0-9a-f]* 	{ clz r5, r6 ; slt r15, r16, r17 ; sw r25, r26 }
    c700:	[0-9a-f]* 	{ slt r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    c708:	[0-9a-f]* 	{ slt r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    c710:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slt r15, r16, r17 }
    c718:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slt r15, r16, r17 }
    c720:	[0-9a-f]* 	{ slt r15, r16, r17 ; xor r5, r6, r7 }
    c728:	[0-9a-f]* 	{ slt r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    c730:	[0-9a-f]* 	{ slt r5, r6, r7 ; and r15, r16, r17 }
    c738:	[0-9a-f]* 	{ slt r5, r6, r7 ; prefetch r25 }
    c740:	[0-9a-f]* 	{ slt r5, r6, r7 ; info 19 ; lw r25, r26 }
    c748:	[0-9a-f]* 	{ slt r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    c750:	[0-9a-f]* 	{ slt r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    c758:	[0-9a-f]* 	{ slt r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    c760:	[0-9a-f]* 	{ slt r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    c768:	[0-9a-f]* 	{ slt r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    c770:	[0-9a-f]* 	{ slt r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    c778:	[0-9a-f]* 	{ slt r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    c780:	[0-9a-f]* 	{ slt r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    c788:	[0-9a-f]* 	{ slt r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    c790:	[0-9a-f]* 	{ slt r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    c798:	[0-9a-f]* 	{ slt r5, r6, r7 ; maxb_u r15, r16, r17 }
    c7a0:	[0-9a-f]* 	{ slt r5, r6, r7 ; mnz r15, r16, r17 }
    c7a8:	[0-9a-f]* 	{ slt r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    c7b0:	[0-9a-f]* 	{ slt r5, r6, r7 ; nop ; lh r25, r26 }
    c7b8:	[0-9a-f]* 	{ slt r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    c7c0:	[0-9a-f]* 	{ slt r5, r6, r7 ; packhs r15, r16, r17 }
    c7c8:	[0-9a-f]* 	{ slt r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    c7d0:	[0-9a-f]* 	{ slt r5, r6, r7 ; prefetch r25 }
    c7d8:	[0-9a-f]* 	{ slt r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    c7e0:	[0-9a-f]* 	{ slt r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    c7e8:	[0-9a-f]* 	{ slt r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    c7f0:	[0-9a-f]* 	{ slt r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c7f8:	[0-9a-f]* 	{ slt r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    c800:	[0-9a-f]* 	{ slt r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    c808:	[0-9a-f]* 	{ slt r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    c810:	[0-9a-f]* 	{ slt r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    c818:	[0-9a-f]* 	{ slt r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    c820:	[0-9a-f]* 	{ slt r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    c828:	[0-9a-f]* 	{ slt r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c830:	[0-9a-f]* 	{ slt r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    c838:	[0-9a-f]* 	{ slt r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    c840:	[0-9a-f]* 	{ slt r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    c848:	[0-9a-f]* 	{ slt r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    c850:	[0-9a-f]* 	{ slt r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    c858:	[0-9a-f]* 	{ slt r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    c860:	[0-9a-f]* 	{ slt r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    c868:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; add r5, r6, r7 ; lb r25, r26 }
    c870:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
    c878:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; and r5, r6, r7 }
    c880:	[0-9a-f]* 	{ bitx r5, r6 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c888:	[0-9a-f]* 	{ clz r5, r6 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c890:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; lh_u r25, r26 }
    c898:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; intlb r5, r6, r7 }
    c8a0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
    c8a8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shli r5, r6, 5 ; lb r25, r26 }
    c8b0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    c8b8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    c8c0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    c8c8:	[0-9a-f]* 	{ bitx r5, r6 ; slt_u r15, r16, r17 ; lh r25, r26 }
    c8d0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
    c8d8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slte_u r5, r6, r7 ; lh r25, r26 }
    c8e0:	[0-9a-f]* 	{ ctz r5, r6 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    c8e8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; or r5, r6, r7 ; lh_u r25, r26 }
    c8f0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
    c8f8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
    c900:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; rl r5, r6, r7 ; lw r25, r26 }
    c908:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; sub r5, r6, r7 ; lw r25, r26 }
    c910:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
    c918:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    c920:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; slt_u r15, r16, r17 }
    c928:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slt_u r15, r16, r17 }
    c930:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; slt_u r15, r16, r17 }
    c938:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt_u r15, r16, r17 }
    c940:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    c948:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c950:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    c958:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
    c960:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; ori r5, r6, 5 ; lw r25, r26 }
    c968:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
    c970:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slt_u r15, r16, r17 ; prefetch r25 }
    c978:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
    c980:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; rl r5, r6, r7 ; lh_u r25, r26 }
    c988:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    c990:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    c998:	[0-9a-f]* 	{ ctz r5, r6 ; slt_u r15, r16, r17 ; sb r25, r26 }
    c9a0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    c9a8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
    c9b0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; seqb r5, r6, r7 }
    c9b8:	[0-9a-f]* 	{ clz r5, r6 ; slt_u r15, r16, r17 ; sh r25, r26 }
    c9c0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; nor r5, r6, r7 ; sh r25, r26 }
    c9c8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
    c9d0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shl r5, r6, r7 }
    c9d8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shr r5, r6, r7 ; prefetch r25 }
    c9e0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    c9e8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; sltb_u r5, r6, r7 }
    c9f0:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slte_u r5, r6, r7 }
    c9f8:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    ca00:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; sne r5, r6, r7 }
    ca08:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; srai r5, r6, 5 ; prefetch r25 }
    ca10:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; subhs r5, r6, r7 }
    ca18:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    ca20:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; shli r5, r6, 5 ; sw r25, r26 }
    ca28:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    ca30:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    ca38:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
    ca40:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; addb r15, r16, r17 }
    ca48:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    ca50:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; dtlbpr r15 }
    ca58:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ill ; sb r25, r26 }
    ca60:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; iret }
    ca68:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    ca70:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    ca78:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    ca80:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    ca88:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    ca90:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; srai r15, r16, 5 ; lh r25, r26 }
    ca98:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    caa0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    caa8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    cab0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    cab8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    cac0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; move r15, r16 }
    cac8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    cad0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    cad8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    cae0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    cae8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    caf0:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    caf8:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    cb00:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sb r15, r16 }
    cb08:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    cb10:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    cb18:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    cb20:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    cb28:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    cb30:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    cb38:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    cb40:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    cb48:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    cb50:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    cb58:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sltib r15, r16, 5 }
    cb60:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    cb68:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    cb70:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; sw r25, r26 }
    cb78:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
    cb80:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    cb88:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; sltb r15, r16, r17 }
    cb90:	[0-9a-f]* 	{ sltb r15, r16, r17 ; maxb_u r5, r6, r7 }
    cb98:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; sltb r15, r16, r17 }
    cba0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sltb r15, r16, r17 }
    cba8:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; sltb r15, r16, r17 }
    cbb0:	[0-9a-f]* 	{ sltb r15, r16, r17 ; shrib r5, r6, 5 }
    cbb8:	[0-9a-f]* 	{ sltb r15, r16, r17 ; sne r5, r6, r7 }
    cbc0:	[0-9a-f]* 	{ sltb r15, r16, r17 ; xori r5, r6, 5 }
    cbc8:	[0-9a-f]* 	{ sltb r5, r6, r7 ; ill }
    cbd0:	[0-9a-f]* 	{ sltb r5, r6, r7 ; lhadd_u r15, r16, 5 }
    cbd8:	[0-9a-f]* 	{ sltb r5, r6, r7 ; move r15, r16 }
    cbe0:	[0-9a-f]* 	{ sltb r5, r6, r7 ; s1a r15, r16, r17 }
    cbe8:	[0-9a-f]* 	{ sltb r5, r6, r7 ; shrb r15, r16, r17 }
    cbf0:	[0-9a-f]* 	{ sltb r5, r6, r7 ; sltib_u r15, r16, 5 }
    cbf8:	[0-9a-f]* 	{ sltb r5, r6, r7 ; tns r15, r16 }
    cc00:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; sltb_u r15, r16, r17 }
    cc08:	[0-9a-f]* 	{ sltb_u r15, r16, r17 ; minb_u r5, r6, r7 }
    cc10:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; sltb_u r15, r16, r17 }
    cc18:	[0-9a-f]* 	{ sltb_u r15, r16, r17 ; nop }
    cc20:	[0-9a-f]* 	{ sltb_u r15, r16, r17 ; seq r5, r6, r7 }
    cc28:	[0-9a-f]* 	{ sltb_u r15, r16, r17 ; sltb r5, r6, r7 }
    cc30:	[0-9a-f]* 	{ sltb_u r15, r16, r17 ; srab r5, r6, r7 }
    cc38:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; addh r15, r16, r17 }
    cc40:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; inthh r15, r16, r17 }
    cc48:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; lwadd r15, r16, 5 }
    cc50:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; mtspr 5, r16 }
    cc58:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; sbadd r15, r16, 5 }
    cc60:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; shrih r15, r16, 5 }
    cc68:	[0-9a-f]* 	{ sltb_u r5, r6, r7 ; sneb r15, r16, r17 }
    cc70:	[0-9a-f]* 	{ slte r15, r16, r17 ; add r5, r6, r7 ; lb r25, r26 }
    cc78:	[0-9a-f]* 	{ slte r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
    cc80:	[0-9a-f]* 	{ slte r15, r16, r17 ; and r5, r6, r7 }
    cc88:	[0-9a-f]* 	{ bitx r5, r6 ; slte r15, r16, r17 ; sb r25, r26 }
    cc90:	[0-9a-f]* 	{ clz r5, r6 ; slte r15, r16, r17 ; sb r25, r26 }
    cc98:	[0-9a-f]* 	{ slte r15, r16, r17 ; lh_u r25, r26 }
    cca0:	[0-9a-f]* 	{ slte r15, r16, r17 ; intlb r5, r6, r7 }
    cca8:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
    ccb0:	[0-9a-f]* 	{ slte r15, r16, r17 ; shli r5, r6, 5 ; lb r25, r26 }
    ccb8:	[0-9a-f]* 	{ slte r15, r16, r17 ; addi r5, r6, 5 ; lb_u r25, r26 }
    ccc0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slte r15, r16, r17 ; lb_u r25, r26 }
    ccc8:	[0-9a-f]* 	{ slte r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    ccd0:	[0-9a-f]* 	{ bitx r5, r6 ; slte r15, r16, r17 ; lh r25, r26 }
    ccd8:	[0-9a-f]* 	{ slte r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
    cce0:	[0-9a-f]* 	{ slte r15, r16, r17 ; slte_u r5, r6, r7 ; lh r25, r26 }
    cce8:	[0-9a-f]* 	{ ctz r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
    ccf0:	[0-9a-f]* 	{ slte r15, r16, r17 ; or r5, r6, r7 ; lh_u r25, r26 }
    ccf8:	[0-9a-f]* 	{ slte r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
    cd00:	[0-9a-f]* 	{ slte r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
    cd08:	[0-9a-f]* 	{ slte r15, r16, r17 ; rl r5, r6, r7 ; lw r25, r26 }
    cd10:	[0-9a-f]* 	{ slte r15, r16, r17 ; sub r5, r6, r7 ; lw r25, r26 }
    cd18:	[0-9a-f]* 	{ slte r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
    cd20:	[0-9a-f]* 	{ slte r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    cd28:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; slte r15, r16, r17 }
    cd30:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slte r15, r16, r17 }
    cd38:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; slte r15, r16, r17 }
    cd40:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte r15, r16, r17 }
    cd48:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    cd50:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    cd58:	[0-9a-f]* 	{ slte r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
    cd60:	[0-9a-f]* 	{ slte r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
    cd68:	[0-9a-f]* 	{ slte r15, r16, r17 ; ori r5, r6, 5 ; lw r25, r26 }
    cd70:	[0-9a-f]* 	{ slte r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
    cd78:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
    cd80:	[0-9a-f]* 	{ slte r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
    cd88:	[0-9a-f]* 	{ slte r15, r16, r17 ; rl r5, r6, r7 ; lh_u r25, r26 }
    cd90:	[0-9a-f]* 	{ slte r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    cd98:	[0-9a-f]* 	{ slte r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    cda0:	[0-9a-f]* 	{ ctz r5, r6 ; slte r15, r16, r17 ; sb r25, r26 }
    cda8:	[0-9a-f]* 	{ slte r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    cdb0:	[0-9a-f]* 	{ slte r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
    cdb8:	[0-9a-f]* 	{ slte r15, r16, r17 ; seqb r5, r6, r7 }
    cdc0:	[0-9a-f]* 	{ clz r5, r6 ; slte r15, r16, r17 ; sh r25, r26 }
    cdc8:	[0-9a-f]* 	{ slte r15, r16, r17 ; nor r5, r6, r7 ; sh r25, r26 }
    cdd0:	[0-9a-f]* 	{ slte r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
    cdd8:	[0-9a-f]* 	{ slte r15, r16, r17 ; shl r5, r6, r7 }
    cde0:	[0-9a-f]* 	{ slte r15, r16, r17 ; shr r5, r6, r7 ; prefetch r25 }
    cde8:	[0-9a-f]* 	{ slte r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
    cdf0:	[0-9a-f]* 	{ slte r15, r16, r17 ; sltb_u r5, r6, r7 }
    cdf8:	[0-9a-f]* 	{ slte r15, r16, r17 ; slte_u r5, r6, r7 }
    ce00:	[0-9a-f]* 	{ slte r15, r16, r17 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    ce08:	[0-9a-f]* 	{ slte r15, r16, r17 ; sne r5, r6, r7 }
    ce10:	[0-9a-f]* 	{ slte r15, r16, r17 ; srai r5, r6, 5 ; prefetch r25 }
    ce18:	[0-9a-f]* 	{ slte r15, r16, r17 ; subhs r5, r6, r7 }
    ce20:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
    ce28:	[0-9a-f]* 	{ slte r15, r16, r17 ; shli r5, r6, 5 ; sw r25, r26 }
    ce30:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    ce38:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    ce40:	[0-9a-f]* 	{ slte r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
    ce48:	[0-9a-f]* 	{ slte r5, r6, r7 ; addb r15, r16, r17 }
    ce50:	[0-9a-f]* 	{ slte r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    ce58:	[0-9a-f]* 	{ slte r5, r6, r7 ; dtlbpr r15 }
    ce60:	[0-9a-f]* 	{ slte r5, r6, r7 ; ill ; sb r25, r26 }
    ce68:	[0-9a-f]* 	{ slte r5, r6, r7 ; iret }
    ce70:	[0-9a-f]* 	{ slte r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
    ce78:	[0-9a-f]* 	{ slte r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    ce80:	[0-9a-f]* 	{ slte r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
    ce88:	[0-9a-f]* 	{ slte r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
    ce90:	[0-9a-f]* 	{ slte r5, r6, r7 ; ori r15, r16, 5 ; lh r25, r26 }
    ce98:	[0-9a-f]* 	{ slte r5, r6, r7 ; srai r15, r16, 5 ; lh r25, r26 }
    cea0:	[0-9a-f]* 	{ slte r5, r6, r7 ; rl r15, r16, r17 ; lh_u r25, r26 }
    cea8:	[0-9a-f]* 	{ slte r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    ceb0:	[0-9a-f]* 	{ slte r5, r6, r7 ; or r15, r16, r17 ; lw r25, r26 }
    ceb8:	[0-9a-f]* 	{ slte r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    cec0:	[0-9a-f]* 	{ slte r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    cec8:	[0-9a-f]* 	{ slte r5, r6, r7 ; move r15, r16 }
    ced0:	[0-9a-f]* 	{ slte r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
    ced8:	[0-9a-f]* 	{ slte r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
    cee0:	[0-9a-f]* 	{ slte r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
    cee8:	[0-9a-f]* 	{ slte r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
    cef0:	[0-9a-f]* 	{ slte r5, r6, r7 ; slte_u r15, r16, r17 ; prefetch r25 }
    cef8:	[0-9a-f]* 	{ slte r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
    cf00:	[0-9a-f]* 	{ slte r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
    cf08:	[0-9a-f]* 	{ slte r5, r6, r7 ; sb r15, r16 }
    cf10:	[0-9a-f]* 	{ slte r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
    cf18:	[0-9a-f]* 	{ slte r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
    cf20:	[0-9a-f]* 	{ slte r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    cf28:	[0-9a-f]* 	{ slte r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
    cf30:	[0-9a-f]* 	{ slte r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    cf38:	[0-9a-f]* 	{ slte r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
    cf40:	[0-9a-f]* 	{ slte r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
    cf48:	[0-9a-f]* 	{ slte r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    cf50:	[0-9a-f]* 	{ slte r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
    cf58:	[0-9a-f]* 	{ slte r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
    cf60:	[0-9a-f]* 	{ slte r5, r6, r7 ; sltib r15, r16, 5 }
    cf68:	[0-9a-f]* 	{ slte r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    cf70:	[0-9a-f]* 	{ slte r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
    cf78:	[0-9a-f]* 	{ slte r5, r6, r7 ; sw r25, r26 }
    cf80:	[0-9a-f]* 	{ slte r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
    cf88:	[0-9a-f]* 	{ slte r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    cf90:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; addh r5, r6, r7 }
    cf98:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; and r5, r6, r7 ; lb_u r25, r26 }
    cfa0:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; slte_u r15, r16, r17 }
    cfa8:	[0-9a-f]* 	{ bytex r5, r6 ; slte_u r15, r16, r17 ; sw r25, r26 }
    cfb0:	[0-9a-f]* 	{ ctz r5, r6 ; slte_u r15, r16, r17 ; sb r25, r26 }
    cfb8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; info 19 ; prefetch r25 }
    cfc0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
    cfc8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; rl r5, r6, r7 ; lb r25, r26 }
    cfd0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sub r5, r6, r7 ; lb r25, r26 }
    cfd8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    cfe0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    cfe8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    cff0:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slte_u r15, r16, r17 ; lh r25, r26 }
    cff8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; seqi r5, r6, 5 ; lh r25, r26 }
    d000:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; lh r25, r26 }
    d008:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    d010:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shr r5, r6, r7 ; lh_u r25, r26 }
    d018:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; and r5, r6, r7 ; lw r25, r26 }
    d020:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    d028:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; slt_u r5, r6, r7 ; lw r25, r26 }
    d030:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; minh r5, r6, r7 }
    d038:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; move r5, r6 ; lw r25, r26 }
    d040:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lh r25, r26 }
    d048:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    d050:	[0-9a-f]* 	{ mulhhsa_uu r5, r6, r7 ; slte_u r15, r16, r17 }
    d058:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
    d060:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
    d068:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slte_u r15, r16, r17 }
    d070:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    d078:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; nop ; sb r25, r26 }
    d080:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
    d088:	[0-9a-f]* 	{ pcnt r5, r6 ; slte_u r15, r16, r17 ; lh r25, r26 }
    d090:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; movei r5, 5 ; prefetch r25 }
    d098:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s1a r5, r6, r7 ; prefetch r25 }
    d0a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte_u r15, r16, r17 ; prefetch r25 }
    d0a8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
    d0b0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
    d0b8:	[0-9a-f]* 	{ sadh_u r5, r6, r7 ; slte_u r15, r16, r17 }
    d0c0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slte_u r15, r16, r17 ; sb r25, r26 }
    d0c8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shr r5, r6, r7 ; sb r25, r26 }
    d0d0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; seq r5, r6, r7 ; lh r25, r26 }
    d0d8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; seqib r5, r6, 5 }
    d0e0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
    d0e8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shli r5, r6, 5 ; sh r25, r26 }
    d0f0:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
    d0f8:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shli r5, r6, 5 }
    d100:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
    d108:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
    d110:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
    d118:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; slti r5, r6, 5 ; prefetch r25 }
    d120:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sne r5, r6, r7 ; lb_u r25, r26 }
    d128:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sra r5, r6, r7 }
    d130:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
    d138:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; mnz r5, r6, r7 ; sw r25, r26 }
    d140:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; rl r5, r6, r7 ; sw r25, r26 }
    d148:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; sub r5, r6, r7 ; sw r25, r26 }
    d150:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    d158:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    d160:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
    d168:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
    d170:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    d178:	[0-9a-f]* 	{ slte_u r5, r6, r7 }
    d180:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; info 19 ; sw r25, r26 }
    d188:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; info 19 ; lb r25, r26 }
    d190:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    d198:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    d1a0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
    d1a8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; info 19 ; lh r25, r26 }
    d1b0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
    d1b8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
    d1c0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
    d1c8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; ill ; lw r25, r26 }
    d1d0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
    d1d8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mf }
    d1e0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
    d1e8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; moveli.sn r15, 4660 }
    d1f0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; nop ; sb r25, r26 }
    d1f8:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
    d200:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
    d208:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
    d210:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
    d218:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
    d220:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
    d228:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; nop ; sb r25, r26 }
    d230:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    d238:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
    d240:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; mnz r15, r16, r17 ; sh r25, r26 }
    d248:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt_u r15, r16, r17 ; sh r25, r26 }
    d250:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    d258:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
    d260:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
    d268:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sltb r15, r16, r17 }
    d270:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    d278:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
    d280:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    d288:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    d290:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; subh r15, r16, r17 }
    d298:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    d2a0:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    d2a8:	[0-9a-f]* 	{ slteb r15, r16, r17 ; addhs r5, r6, r7 }
    d2b0:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; slteb r15, r16, r17 }
    d2b8:	[0-9a-f]* 	{ slteb r15, r16, r17 ; move r5, r6 }
    d2c0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slteb r15, r16, r17 }
    d2c8:	[0-9a-f]* 	{ pcnt r5, r6 ; slteb r15, r16, r17 }
    d2d0:	[0-9a-f]* 	{ slteb r15, r16, r17 ; shlh r5, r6, r7 }
    d2d8:	[0-9a-f]* 	{ slteb r15, r16, r17 ; slth r5, r6, r7 }
    d2e0:	[0-9a-f]* 	{ slteb r15, r16, r17 ; subh r5, r6, r7 }
    d2e8:	[0-9a-f]* 	{ slteb r5, r6, r7 ; and r15, r16, r17 }
    d2f0:	[0-9a-f]* 	{ slteb r5, r6, r7 ; jrp r15 }
    d2f8:	[0-9a-f]* 	{ slteb r5, r6, r7 ; minb_u r15, r16, r17 }
    d300:	[0-9a-f]* 	{ slteb r5, r6, r7 ; packbs_u r15, r16, r17 }
    d308:	[0-9a-f]* 	{ slteb r5, r6, r7 ; shadd r15, r16, 5 }
    d310:	[0-9a-f]* 	{ slteb r5, r6, r7 ; slteb_u r15, r16, r17 }
    d318:	[0-9a-f]* 	{ slteb r5, r6, r7 ; sub r15, r16, r17 }
    d320:	[0-9a-f]* 	{ slteb_u r15, r16, r17 ; addli r5, r6, 4660 }
    d328:	[0-9a-f]* 	{ slteb_u r15, r16, r17 ; inthb r5, r6, r7 }
    d330:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slteb_u r15, r16, r17 }
    d338:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; slteb_u r15, r16, r17 }
    d340:	[0-9a-f]* 	{ slteb_u r15, r16, r17 ; s2a r5, r6, r7 }
    d348:	[0-9a-f]* 	{ slteb_u r15, r16, r17 ; shr r5, r6, r7 }
    d350:	[0-9a-f]* 	{ slteb_u r15, r16, r17 ; sltib r5, r6, 5 }
    d358:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slteb_u r15, r16, r17 }
    d360:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; finv r15 }
    d368:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; lbadd_u r15, r16, 5 }
    d370:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
    d378:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; prefetch r15 }
    d380:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; shli r15, r16, 5 }
    d388:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; slth_u r15, r16, r17 }
    d390:	[0-9a-f]* 	{ slteb_u r5, r6, r7 ; subhs r15, r16, r17 }
    d398:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; slteh r15, r16, r17 }
    d3a0:	[0-9a-f]* 	{ slteh r15, r16, r17 ; maxb_u r5, r6, r7 }
    d3a8:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; slteh r15, r16, r17 }
    d3b0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slteh r15, r16, r17 }
    d3b8:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; slteh r15, r16, r17 }
    d3c0:	[0-9a-f]* 	{ slteh r15, r16, r17 ; shrib r5, r6, 5 }
    d3c8:	[0-9a-f]* 	{ slteh r15, r16, r17 ; sne r5, r6, r7 }
    d3d0:	[0-9a-f]* 	{ slteh r15, r16, r17 ; xori r5, r6, 5 }
    d3d8:	[0-9a-f]* 	{ slteh r5, r6, r7 ; ill }
    d3e0:	[0-9a-f]* 	{ slteh r5, r6, r7 ; lhadd_u r15, r16, 5 }
    d3e8:	[0-9a-f]* 	{ slteh r5, r6, r7 ; move r15, r16 }
    d3f0:	[0-9a-f]* 	{ slteh r5, r6, r7 ; s1a r15, r16, r17 }
    d3f8:	[0-9a-f]* 	{ slteh r5, r6, r7 ; shrb r15, r16, r17 }
    d400:	[0-9a-f]* 	{ slteh r5, r6, r7 ; sltib_u r15, r16, 5 }
    d408:	[0-9a-f]* 	{ slteh r5, r6, r7 ; tns r15, r16 }
    d410:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; slteh_u r15, r16, r17 }
    d418:	[0-9a-f]* 	{ slteh_u r15, r16, r17 ; minb_u r5, r6, r7 }
    d420:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; slteh_u r15, r16, r17 }
    d428:	[0-9a-f]* 	{ slteh_u r15, r16, r17 ; nop }
    d430:	[0-9a-f]* 	{ slteh_u r15, r16, r17 ; seq r5, r6, r7 }
    d438:	[0-9a-f]* 	{ slteh_u r15, r16, r17 ; sltb r5, r6, r7 }
    d440:	[0-9a-f]* 	{ slteh_u r15, r16, r17 ; srab r5, r6, r7 }
    d448:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; addh r15, r16, r17 }
    d450:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; inthh r15, r16, r17 }
    d458:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; lwadd r15, r16, 5 }
    d460:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; mtspr 5, r16 }
    d468:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; sbadd r15, r16, 5 }
    d470:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; shrih r15, r16, 5 }
    d478:	[0-9a-f]* 	{ slteh_u r5, r6, r7 ; sneb r15, r16, r17 }
    d480:	[0-9a-f]* 	{ slth r15, r16, r17 ; add r5, r6, r7 }
    d488:	[0-9a-f]* 	{ clz r5, r6 ; slth r15, r16, r17 }
    d490:	[0-9a-f]* 	{ slth r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    d498:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; slth r15, r16, r17 }
    d4a0:	[0-9a-f]* 	{ slth r15, r16, r17 ; packbs_u r5, r6, r7 }
    d4a8:	[0-9a-f]* 	{ slth r15, r16, r17 ; seqib r5, r6, 5 }
    d4b0:	[0-9a-f]* 	{ slth r15, r16, r17 ; slteb r5, r6, r7 }
    d4b8:	[0-9a-f]* 	{ slth r15, r16, r17 ; sraih r5, r6, 5 }
    d4c0:	[0-9a-f]* 	{ slth r5, r6, r7 ; addih r15, r16, 5 }
    d4c8:	[0-9a-f]* 	{ slth r5, r6, r7 ; iret }
    d4d0:	[0-9a-f]* 	{ slth r5, r6, r7 ; maxib_u r15, r16, 5 }
    d4d8:	[0-9a-f]* 	{ slth r5, r6, r7 ; nop }
    d4e0:	[0-9a-f]* 	{ slth r5, r6, r7 ; seqi r15, r16, 5 }
    d4e8:	[0-9a-f]* 	{ slth r5, r6, r7 ; sltb_u r15, r16, r17 }
    d4f0:	[0-9a-f]* 	{ slth r5, r6, r7 ; srah r15, r16, r17 }
    d4f8:	[0-9a-f]* 	{ slth_u r15, r16, r17 ; addhs r5, r6, r7 }
    d500:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; slth_u r15, r16, r17 }
    d508:	[0-9a-f]* 	{ slth_u r15, r16, r17 ; move r5, r6 }
    d510:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slth_u r15, r16, r17 }
    d518:	[0-9a-f]* 	{ pcnt r5, r6 ; slth_u r15, r16, r17 }
    d520:	[0-9a-f]* 	{ slth_u r15, r16, r17 ; shlh r5, r6, r7 }
    d528:	[0-9a-f]* 	{ slth_u r15, r16, r17 ; slth r5, r6, r7 }
    d530:	[0-9a-f]* 	{ slth_u r15, r16, r17 ; subh r5, r6, r7 }
    d538:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; and r15, r16, r17 }
    d540:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; jrp r15 }
    d548:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; minb_u r15, r16, r17 }
    d550:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; packbs_u r15, r16, r17 }
    d558:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; shadd r15, r16, 5 }
    d560:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; slteb_u r15, r16, r17 }
    d568:	[0-9a-f]* 	{ slth_u r5, r6, r7 ; sub r15, r16, r17 }
    d570:	[0-9a-f]* 	{ slti r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
    d578:	[0-9a-f]* 	{ slti r15, r16, 5 ; adds r5, r6, r7 }
    d580:	[0-9a-f]* 	{ slti r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    d588:	[0-9a-f]* 	{ bytex r5, r6 ; slti r15, r16, 5 ; lw r25, r26 }
    d590:	[0-9a-f]* 	{ ctz r5, r6 ; slti r15, r16, 5 ; lh r25, r26 }
    d598:	[0-9a-f]* 	{ slti r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    d5a0:	[0-9a-f]* 	{ clz r5, r6 ; slti r15, r16, 5 ; lb r25, r26 }
    d5a8:	[0-9a-f]* 	{ slti r15, r16, 5 ; nor r5, r6, r7 ; lb r25, r26 }
    d5b0:	[0-9a-f]* 	{ slti r15, r16, 5 ; slti_u r5, r6, 5 ; lb r25, r26 }
    d5b8:	[0-9a-f]* 	{ slti r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    d5c0:	[0-9a-f]* 	{ pcnt r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
    d5c8:	[0-9a-f]* 	{ slti r15, r16, 5 ; srai r5, r6, 5 ; lb_u r25, r26 }
    d5d0:	[0-9a-f]* 	{ slti r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    d5d8:	[0-9a-f]* 	{ slti r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    d5e0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti r15, r16, 5 ; lh r25, r26 }
    d5e8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
    d5f0:	[0-9a-f]* 	{ slti r15, r16, 5 ; seq r5, r6, r7 ; lh_u r25, r26 }
    d5f8:	[0-9a-f]* 	{ slti r15, r16, 5 ; xor r5, r6, r7 ; lh_u r25, r26 }
    d600:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    d608:	[0-9a-f]* 	{ slti r15, r16, 5 ; shli r5, r6, 5 ; lw r25, r26 }
    d610:	[0-9a-f]* 	{ slti r15, r16, 5 ; maxh r5, r6, r7 }
    d618:	[0-9a-f]* 	{ slti r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
    d620:	[0-9a-f]* 	{ slti r15, r16, 5 ; moveli r5, 4660 }
    d628:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    d630:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    d638:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    d640:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    d648:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
    d650:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
    d658:	[0-9a-f]* 	{ slti r15, r16, 5 ; nop ; lh r25, r26 }
    d660:	[0-9a-f]* 	{ slti r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    d668:	[0-9a-f]* 	{ slti r15, r16, 5 ; packhs r5, r6, r7 }
    d670:	[0-9a-f]* 	{ slti r15, r16, 5 ; prefetch r25 }
    d678:	[0-9a-f]* 	{ slti r15, r16, 5 ; ori r5, r6, 5 ; prefetch r25 }
    d680:	[0-9a-f]* 	{ slti r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    d688:	[0-9a-f]* 	{ slti r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    d690:	[0-9a-f]* 	{ slti r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    d698:	[0-9a-f]* 	{ sadah r5, r6, r7 ; slti r15, r16, 5 }
    d6a0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; sb r25, r26 }
    d6a8:	[0-9a-f]* 	{ slti r15, r16, 5 ; seq r5, r6, r7 ; sb r25, r26 }
    d6b0:	[0-9a-f]* 	{ slti r15, r16, 5 ; xor r5, r6, r7 ; sb r25, r26 }
    d6b8:	[0-9a-f]* 	{ slti r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    d6c0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    d6c8:	[0-9a-f]* 	{ slti r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
    d6d0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    d6d8:	[0-9a-f]* 	{ slti r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    d6e0:	[0-9a-f]* 	{ slti r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
    d6e8:	[0-9a-f]* 	{ slti r15, r16, 5 ; slt r5, r6, r7 }
    d6f0:	[0-9a-f]* 	{ slti r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    d6f8:	[0-9a-f]* 	{ slti r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
    d700:	[0-9a-f]* 	{ slti r15, r16, 5 ; sltib_u r5, r6, 5 }
    d708:	[0-9a-f]* 	{ slti r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    d710:	[0-9a-f]* 	{ slti r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
    d718:	[0-9a-f]* 	{ clz r5, r6 ; slti r15, r16, 5 ; sw r25, r26 }
    d720:	[0-9a-f]* 	{ slti r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
    d728:	[0-9a-f]* 	{ slti r15, r16, 5 ; slti_u r5, r6, 5 ; sw r25, r26 }
    d730:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti r15, r16, 5 }
    d738:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slti r15, r16, 5 }
    d740:	[0-9a-f]* 	{ slti r15, r16, 5 ; xor r5, r6, r7 }
    d748:	[0-9a-f]* 	{ slti r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    d750:	[0-9a-f]* 	{ slti r5, r6, 5 ; and r15, r16, r17 }
    d758:	[0-9a-f]* 	{ slti r5, r6, 5 ; prefetch r25 }
    d760:	[0-9a-f]* 	{ slti r5, r6, 5 ; info 19 ; lw r25, r26 }
    d768:	[0-9a-f]* 	{ slti r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    d770:	[0-9a-f]* 	{ slti r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    d778:	[0-9a-f]* 	{ slti r5, r6, 5 ; andi r15, r16, 5 ; lb_u r25, r26 }
    d780:	[0-9a-f]* 	{ slti r5, r6, 5 ; shli r15, r16, 5 ; lb_u r25, r26 }
    d788:	[0-9a-f]* 	{ slti r5, r6, 5 ; and r15, r16, r17 ; lh r25, r26 }
    d790:	[0-9a-f]* 	{ slti r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    d798:	[0-9a-f]* 	{ slti r5, r6, 5 ; andi r15, r16, 5 ; lh_u r25, r26 }
    d7a0:	[0-9a-f]* 	{ slti r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
    d7a8:	[0-9a-f]* 	{ slti r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    d7b0:	[0-9a-f]* 	{ slti r5, r6, 5 ; seqi r15, r16, 5 ; lw r25, r26 }
    d7b8:	[0-9a-f]* 	{ slti r5, r6, 5 ; maxb_u r15, r16, r17 }
    d7c0:	[0-9a-f]* 	{ slti r5, r6, 5 ; mnz r15, r16, r17 }
    d7c8:	[0-9a-f]* 	{ slti r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
    d7d0:	[0-9a-f]* 	{ slti r5, r6, 5 ; nop ; lh r25, r26 }
    d7d8:	[0-9a-f]* 	{ slti r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    d7e0:	[0-9a-f]* 	{ slti r5, r6, 5 ; packhs r15, r16, r17 }
    d7e8:	[0-9a-f]* 	{ slti r5, r6, 5 ; s1a r15, r16, r17 ; prefetch r25 }
    d7f0:	[0-9a-f]* 	{ slti r5, r6, 5 ; prefetch r25 }
    d7f8:	[0-9a-f]* 	{ slti r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
    d800:	[0-9a-f]* 	{ slti r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
    d808:	[0-9a-f]* 	{ slti r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    d810:	[0-9a-f]* 	{ slti r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    d818:	[0-9a-f]* 	{ slti r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
    d820:	[0-9a-f]* 	{ slti r5, r6, 5 ; andi r15, r16, 5 ; sh r25, r26 }
    d828:	[0-9a-f]* 	{ slti r5, r6, 5 ; shli r15, r16, 5 ; sh r25, r26 }
    d830:	[0-9a-f]* 	{ slti r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
    d838:	[0-9a-f]* 	{ slti r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
    d840:	[0-9a-f]* 	{ slti r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
    d848:	[0-9a-f]* 	{ slti r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    d850:	[0-9a-f]* 	{ slti r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
    d858:	[0-9a-f]* 	{ slti r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
    d860:	[0-9a-f]* 	{ slti r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
    d868:	[0-9a-f]* 	{ slti r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    d870:	[0-9a-f]* 	{ slti r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
    d878:	[0-9a-f]* 	{ slti r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    d880:	[0-9a-f]* 	{ slti r5, r6, 5 ; sne r15, r16, r17 ; sw r25, r26 }
    d888:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; add r5, r6, r7 ; lb r25, r26 }
    d890:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; addi r5, r6, 5 ; sb r25, r26 }
    d898:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; and r5, r6, r7 }
    d8a0:	[0-9a-f]* 	{ bitx r5, r6 ; slti_u r15, r16, 5 ; sb r25, r26 }
    d8a8:	[0-9a-f]* 	{ clz r5, r6 ; slti_u r15, r16, 5 ; sb r25, r26 }
    d8b0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; lh_u r25, r26 }
    d8b8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; intlb r5, r6, r7 }
    d8c0:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti_u r15, r16, 5 ; lb r25, r26 }
    d8c8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shli r5, r6, 5 ; lb r25, r26 }
    d8d0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; addi r5, r6, 5 ; lb_u r25, r26 }
    d8d8:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    d8e0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    d8e8:	[0-9a-f]* 	{ bitx r5, r6 ; slti_u r15, r16, 5 ; lh r25, r26 }
    d8f0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; mz r5, r6, r7 ; lh r25, r26 }
    d8f8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slte_u r5, r6, r7 ; lh r25, r26 }
    d900:	[0-9a-f]* 	{ ctz r5, r6 ; slti_u r15, r16, 5 ; lh_u r25, r26 }
    d908:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; or r5, r6, r7 ; lh_u r25, r26 }
    d910:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sne r5, r6, r7 ; lh_u r25, r26 }
    d918:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
    d920:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; rl r5, r6, r7 ; lw r25, r26 }
    d928:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sub r5, r6, r7 ; lw r25, r26 }
    d930:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
    d938:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    d940:	[0-9a-f]* 	{ mulhh_su r5, r6, r7 ; slti_u r15, r16, 5 }
    d948:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; slti_u r15, r16, 5 }
    d950:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; slti_u r15, r16, 5 }
    d958:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti_u r15, r16, 5 }
    d960:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slti_u r15, r16, 5 ; sw r25, r26 }
    d968:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
    d970:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
    d978:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; nor r5, r6, r7 ; lw r25, r26 }
    d980:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; ori r5, r6, 5 ; lw r25, r26 }
    d988:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; add r5, r6, r7 ; prefetch r25 }
    d990:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; slti_u r15, r16, 5 ; prefetch r25 }
    d998:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shri r5, r6, 5 ; prefetch r25 }
    d9a0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; rl r5, r6, r7 ; lh_u r25, r26 }
    d9a8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; s1a r5, r6, r7 ; lh_u r25, r26 }
    d9b0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    d9b8:	[0-9a-f]* 	{ ctz r5, r6 ; slti_u r15, r16, 5 ; sb r25, r26 }
    d9c0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; or r5, r6, r7 ; sb r25, r26 }
    d9c8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sne r5, r6, r7 ; sb r25, r26 }
    d9d0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; seqb r5, r6, r7 }
    d9d8:	[0-9a-f]* 	{ clz r5, r6 ; slti_u r15, r16, 5 ; sh r25, r26 }
    d9e0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; nor r5, r6, r7 ; sh r25, r26 }
    d9e8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slti_u r5, r6, 5 ; sh r25, r26 }
    d9f0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shl r5, r6, r7 }
    d9f8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shr r5, r6, r7 ; prefetch r25 }
    da00:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
    da08:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sltb_u r5, r6, r7 }
    da10:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slte_u r5, r6, r7 }
    da18:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
    da20:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; sne r5, r6, r7 }
    da28:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; srai r5, r6, 5 ; prefetch r25 }
    da30:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; subhs r5, r6, r7 }
    da38:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; slti_u r15, r16, 5 ; sw r25, r26 }
    da40:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; shli r5, r6, 5 ; sw r25, r26 }
    da48:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    da50:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
    da58:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; xor r5, r6, r7 ; lb_u r25, r26 }
    da60:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; addb r15, r16, r17 }
    da68:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; and r15, r16, r17 ; lb_u r25, r26 }
    da70:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; dtlbpr r15 }
    da78:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ill ; sb r25, r26 }
    da80:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; iret }
    da88:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ori r15, r16, 5 ; lb r25, r26 }
    da90:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    da98:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; rl r15, r16, r17 ; lb_u r25, r26 }
    daa0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sub r15, r16, r17 ; lb_u r25, r26 }
    daa8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ori r15, r16, 5 ; lh r25, r26 }
    dab0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; srai r15, r16, 5 ; lh r25, r26 }
    dab8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; rl r15, r16, r17 ; lh_u r25, r26 }
    dac0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sub r15, r16, r17 ; lh_u r25, r26 }
    dac8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; or r15, r16, r17 ; lw r25, r26 }
    dad0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
    dad8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; mnz r15, r16, r17 ; lb_u r25, r26 }
    dae0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; move r15, r16 }
    dae8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; mz r15, r16, r17 ; sb r25, r26 }
    daf0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; nor r15, r16, r17 ; lw r25, r26 }
    daf8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; ori r15, r16, 5 ; lw r25, r26 }
    db00:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; movei r15, 5 ; prefetch r25 }
    db08:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slte_u r15, r16, r17 ; prefetch r25 }
    db10:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; rli r15, r16, 5 ; lb r25, r26 }
    db18:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; s2a r15, r16, r17 ; lb r25, r26 }
    db20:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sb r15, r16 }
    db28:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; s3a r15, r16, r17 ; sb r25, r26 }
    db30:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
    db38:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; seqi r15, r16, 5 ; sw r25, r26 }
    db40:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; rl r15, r16, r17 ; sh r25, r26 }
    db48:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sub r15, r16, r17 ; sh r25, r26 }
    db50:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shli r15, r16, 5 ; lw r25, r26 }
    db58:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shri r15, r16, 5 ; lb r25, r26 }
    db60:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slt r15, r16, r17 ; sw r25, r26 }
    db68:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slte r15, r16, r17 ; sb r25, r26 }
    db70:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slti r15, r16, 5 ; lb r25, r26 }
    db78:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sltib r15, r16, 5 }
    db80:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
    db88:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sub r15, r16, r17 ; lb r25, r26 }
    db90:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sw r25, r26 }
    db98:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; shr r15, r16, r17 ; sw r25, r26 }
    dba0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; xor r15, r16, r17 ; lh_u r25, r26 }
    dba8:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; sltib r15, r16, 5 }
    dbb0:	[0-9a-f]* 	{ sltib r15, r16, 5 ; maxb_u r5, r6, r7 }
    dbb8:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; sltib r15, r16, 5 }
    dbc0:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sltib r15, r16, 5 }
    dbc8:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; sltib r15, r16, 5 }
    dbd0:	[0-9a-f]* 	{ sltib r15, r16, 5 ; shrib r5, r6, 5 }
    dbd8:	[0-9a-f]* 	{ sltib r15, r16, 5 ; sne r5, r6, r7 }
    dbe0:	[0-9a-f]* 	{ sltib r15, r16, 5 ; xori r5, r6, 5 }
    dbe8:	[0-9a-f]* 	{ sltib r5, r6, 5 ; ill }
    dbf0:	[0-9a-f]* 	{ sltib r5, r6, 5 ; lhadd_u r15, r16, 5 }
    dbf8:	[0-9a-f]* 	{ sltib r5, r6, 5 ; move r15, r16 }
    dc00:	[0-9a-f]* 	{ sltib r5, r6, 5 ; s1a r15, r16, r17 }
    dc08:	[0-9a-f]* 	{ sltib r5, r6, 5 ; shrb r15, r16, r17 }
    dc10:	[0-9a-f]* 	{ sltib r5, r6, 5 ; sltib_u r15, r16, 5 }
    dc18:	[0-9a-f]* 	{ sltib r5, r6, 5 ; tns r15, r16 }
    dc20:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; sltib_u r15, r16, 5 }
    dc28:	[0-9a-f]* 	{ sltib_u r15, r16, 5 ; minb_u r5, r6, r7 }
    dc30:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; sltib_u r15, r16, 5 }
    dc38:	[0-9a-f]* 	{ sltib_u r15, r16, 5 ; nop }
    dc40:	[0-9a-f]* 	{ sltib_u r15, r16, 5 ; seq r5, r6, r7 }
    dc48:	[0-9a-f]* 	{ sltib_u r15, r16, 5 ; sltb r5, r6, r7 }
    dc50:	[0-9a-f]* 	{ sltib_u r15, r16, 5 ; srab r5, r6, r7 }
    dc58:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; addh r15, r16, r17 }
    dc60:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; inthh r15, r16, r17 }
    dc68:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; lwadd r15, r16, 5 }
    dc70:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; mtspr 5, r16 }
    dc78:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; sbadd r15, r16, 5 }
    dc80:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; shrih r15, r16, 5 }
    dc88:	[0-9a-f]* 	{ sltib_u r5, r6, 5 ; sneb r15, r16, r17 }
    dc90:	[0-9a-f]* 	{ sltih r15, r16, 5 ; add r5, r6, r7 }
    dc98:	[0-9a-f]* 	{ clz r5, r6 ; sltih r15, r16, 5 }
    dca0:	[0-9a-f]* 	{ sltih r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
    dca8:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; sltih r15, r16, 5 }
    dcb0:	[0-9a-f]* 	{ sltih r15, r16, 5 ; packbs_u r5, r6, r7 }
    dcb8:	[0-9a-f]* 	{ sltih r15, r16, 5 ; seqib r5, r6, 5 }
    dcc0:	[0-9a-f]* 	{ sltih r15, r16, 5 ; slteb r5, r6, r7 }
    dcc8:	[0-9a-f]* 	{ sltih r15, r16, 5 ; sraih r5, r6, 5 }
    dcd0:	[0-9a-f]* 	{ sltih r5, r6, 5 ; addih r15, r16, 5 }
    dcd8:	[0-9a-f]* 	{ sltih r5, r6, 5 ; iret }
    dce0:	[0-9a-f]* 	{ sltih r5, r6, 5 ; maxib_u r15, r16, 5 }
    dce8:	[0-9a-f]* 	{ sltih r5, r6, 5 ; nop }
    dcf0:	[0-9a-f]* 	{ sltih r5, r6, 5 ; seqi r15, r16, 5 }
    dcf8:	[0-9a-f]* 	{ sltih r5, r6, 5 ; sltb_u r15, r16, r17 }
    dd00:	[0-9a-f]* 	{ sltih r5, r6, 5 ; srah r15, r16, r17 }
    dd08:	[0-9a-f]* 	{ sltih_u r15, r16, 5 ; addhs r5, r6, r7 }
    dd10:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; sltih_u r15, r16, 5 }
    dd18:	[0-9a-f]* 	{ sltih_u r15, r16, 5 ; move r5, r6 }
    dd20:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sltih_u r15, r16, 5 }
    dd28:	[0-9a-f]* 	{ pcnt r5, r6 ; sltih_u r15, r16, 5 }
    dd30:	[0-9a-f]* 	{ sltih_u r15, r16, 5 ; shlh r5, r6, r7 }
    dd38:	[0-9a-f]* 	{ sltih_u r15, r16, 5 ; slth r5, r6, r7 }
    dd40:	[0-9a-f]* 	{ sltih_u r15, r16, 5 ; subh r5, r6, r7 }
    dd48:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; and r15, r16, r17 }
    dd50:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; jrp r15 }
    dd58:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; minb_u r15, r16, r17 }
    dd60:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; packbs_u r15, r16, r17 }
    dd68:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; shadd r15, r16, 5 }
    dd70:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; slteb_u r15, r16, r17 }
    dd78:	[0-9a-f]* 	{ sltih_u r5, r6, 5 ; sub r15, r16, r17 }
    dd80:	[0-9a-f]* 	{ sne r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    dd88:	[0-9a-f]* 	{ sne r15, r16, r17 ; adds r5, r6, r7 }
    dd90:	[0-9a-f]* 	{ sne r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    dd98:	[0-9a-f]* 	{ bytex r5, r6 ; sne r15, r16, r17 ; lw r25, r26 }
    dda0:	[0-9a-f]* 	{ ctz r5, r6 ; sne r15, r16, r17 ; lh r25, r26 }
    dda8:	[0-9a-f]* 	{ sne r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    ddb0:	[0-9a-f]* 	{ clz r5, r6 ; sne r15, r16, r17 ; lb r25, r26 }
    ddb8:	[0-9a-f]* 	{ sne r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    ddc0:	[0-9a-f]* 	{ sne r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    ddc8:	[0-9a-f]* 	{ sne r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    ddd0:	[0-9a-f]* 	{ pcnt r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
    ddd8:	[0-9a-f]* 	{ sne r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    dde0:	[0-9a-f]* 	{ sne r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    dde8:	[0-9a-f]* 	{ sne r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    ddf0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sne r15, r16, r17 ; lh r25, r26 }
    ddf8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    de00:	[0-9a-f]* 	{ sne r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    de08:	[0-9a-f]* 	{ sne r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    de10:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    de18:	[0-9a-f]* 	{ sne r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    de20:	[0-9a-f]* 	{ sne r15, r16, r17 ; maxh r5, r6, r7 }
    de28:	[0-9a-f]* 	{ sne r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    de30:	[0-9a-f]* 	{ sne r15, r16, r17 ; moveli r5, 4660 }
    de38:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    de40:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    de48:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    de50:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    de58:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sne r15, r16, r17 ; prefetch r25 }
    de60:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    de68:	[0-9a-f]* 	{ sne r15, r16, r17 ; nop ; lh r25, r26 }
    de70:	[0-9a-f]* 	{ sne r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    de78:	[0-9a-f]* 	{ sne r15, r16, r17 ; packhs r5, r6, r7 }
    de80:	[0-9a-f]* 	{ sne r15, r16, r17 ; prefetch r25 }
    de88:	[0-9a-f]* 	{ sne r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    de90:	[0-9a-f]* 	{ sne r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    de98:	[0-9a-f]* 	{ sne r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    dea0:	[0-9a-f]* 	{ sne r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    dea8:	[0-9a-f]* 	{ sadah r5, r6, r7 ; sne r15, r16, r17 }
    deb0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sne r15, r16, r17 ; sb r25, r26 }
    deb8:	[0-9a-f]* 	{ sne r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    dec0:	[0-9a-f]* 	{ sne r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    dec8:	[0-9a-f]* 	{ sne r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    ded0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
    ded8:	[0-9a-f]* 	{ sne r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    dee0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
    dee8:	[0-9a-f]* 	{ sne r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    def0:	[0-9a-f]* 	{ sne r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    def8:	[0-9a-f]* 	{ sne r15, r16, r17 ; slt r5, r6, r7 }
    df00:	[0-9a-f]* 	{ sne r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    df08:	[0-9a-f]* 	{ sne r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    df10:	[0-9a-f]* 	{ sne r15, r16, r17 ; sltib_u r5, r6, 5 }
    df18:	[0-9a-f]* 	{ sne r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    df20:	[0-9a-f]* 	{ sne r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    df28:	[0-9a-f]* 	{ clz r5, r6 ; sne r15, r16, r17 ; sw r25, r26 }
    df30:	[0-9a-f]* 	{ sne r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    df38:	[0-9a-f]* 	{ sne r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    df40:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sne r15, r16, r17 }
    df48:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sne r15, r16, r17 }
    df50:	[0-9a-f]* 	{ sne r15, r16, r17 ; xor r5, r6, r7 }
    df58:	[0-9a-f]* 	{ sne r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    df60:	[0-9a-f]* 	{ sne r5, r6, r7 ; and r15, r16, r17 }
    df68:	[0-9a-f]* 	{ sne r5, r6, r7 ; prefetch r25 }
    df70:	[0-9a-f]* 	{ sne r5, r6, r7 ; info 19 ; lw r25, r26 }
    df78:	[0-9a-f]* 	{ sne r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    df80:	[0-9a-f]* 	{ sne r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    df88:	[0-9a-f]* 	{ sne r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    df90:	[0-9a-f]* 	{ sne r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    df98:	[0-9a-f]* 	{ sne r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    dfa0:	[0-9a-f]* 	{ sne r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    dfa8:	[0-9a-f]* 	{ sne r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    dfb0:	[0-9a-f]* 	{ sne r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    dfb8:	[0-9a-f]* 	{ sne r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    dfc0:	[0-9a-f]* 	{ sne r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    dfc8:	[0-9a-f]* 	{ sne r5, r6, r7 ; maxb_u r15, r16, r17 }
    dfd0:	[0-9a-f]* 	{ sne r5, r6, r7 ; mnz r15, r16, r17 }
    dfd8:	[0-9a-f]* 	{ sne r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    dfe0:	[0-9a-f]* 	{ sne r5, r6, r7 ; nop ; lh r25, r26 }
    dfe8:	[0-9a-f]* 	{ sne r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    dff0:	[0-9a-f]* 	{ sne r5, r6, r7 ; packhs r15, r16, r17 }
    dff8:	[0-9a-f]* 	{ sne r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    e000:	[0-9a-f]* 	{ sne r5, r6, r7 ; prefetch r25 }
    e008:	[0-9a-f]* 	{ sne r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    e010:	[0-9a-f]* 	{ sne r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    e018:	[0-9a-f]* 	{ sne r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    e020:	[0-9a-f]* 	{ sne r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e028:	[0-9a-f]* 	{ sne r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    e030:	[0-9a-f]* 	{ sne r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    e038:	[0-9a-f]* 	{ sne r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    e040:	[0-9a-f]* 	{ sne r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    e048:	[0-9a-f]* 	{ sne r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    e050:	[0-9a-f]* 	{ sne r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    e058:	[0-9a-f]* 	{ sne r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e060:	[0-9a-f]* 	{ sne r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    e068:	[0-9a-f]* 	{ sne r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    e070:	[0-9a-f]* 	{ sne r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    e078:	[0-9a-f]* 	{ sne r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    e080:	[0-9a-f]* 	{ sne r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    e088:	[0-9a-f]* 	{ sne r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    e090:	[0-9a-f]* 	{ sne r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    e098:	[0-9a-f]* 	{ sneb r15, r16, r17 ; add r5, r6, r7 }
    e0a0:	[0-9a-f]* 	{ clz r5, r6 ; sneb r15, r16, r17 }
    e0a8:	[0-9a-f]* 	{ sneb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    e0b0:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; sneb r15, r16, r17 }
    e0b8:	[0-9a-f]* 	{ sneb r15, r16, r17 ; packbs_u r5, r6, r7 }
    e0c0:	[0-9a-f]* 	{ sneb r15, r16, r17 ; seqib r5, r6, 5 }
    e0c8:	[0-9a-f]* 	{ sneb r15, r16, r17 ; slteb r5, r6, r7 }
    e0d0:	[0-9a-f]* 	{ sneb r15, r16, r17 ; sraih r5, r6, 5 }
    e0d8:	[0-9a-f]* 	{ sneb r5, r6, r7 ; addih r15, r16, 5 }
    e0e0:	[0-9a-f]* 	{ sneb r5, r6, r7 ; iret }
    e0e8:	[0-9a-f]* 	{ sneb r5, r6, r7 ; maxib_u r15, r16, 5 }
    e0f0:	[0-9a-f]* 	{ sneb r5, r6, r7 ; nop }
    e0f8:	[0-9a-f]* 	{ sneb r5, r6, r7 ; seqi r15, r16, 5 }
    e100:	[0-9a-f]* 	{ sneb r5, r6, r7 ; sltb_u r15, r16, r17 }
    e108:	[0-9a-f]* 	{ sneb r5, r6, r7 ; srah r15, r16, r17 }
    e110:	[0-9a-f]* 	{ sneh r15, r16, r17 ; addhs r5, r6, r7 }
    e118:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; sneh r15, r16, r17 }
    e120:	[0-9a-f]* 	{ sneh r15, r16, r17 ; move r5, r6 }
    e128:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sneh r15, r16, r17 }
    e130:	[0-9a-f]* 	{ pcnt r5, r6 ; sneh r15, r16, r17 }
    e138:	[0-9a-f]* 	{ sneh r15, r16, r17 ; shlh r5, r6, r7 }
    e140:	[0-9a-f]* 	{ sneh r15, r16, r17 ; slth r5, r6, r7 }
    e148:	[0-9a-f]* 	{ sneh r15, r16, r17 ; subh r5, r6, r7 }
    e150:	[0-9a-f]* 	{ sneh r5, r6, r7 ; and r15, r16, r17 }
    e158:	[0-9a-f]* 	{ sneh r5, r6, r7 ; jrp r15 }
    e160:	[0-9a-f]* 	{ sneh r5, r6, r7 ; minb_u r15, r16, r17 }
    e168:	[0-9a-f]* 	{ sneh r5, r6, r7 ; packbs_u r15, r16, r17 }
    e170:	[0-9a-f]* 	{ sneh r5, r6, r7 ; shadd r15, r16, 5 }
    e178:	[0-9a-f]* 	{ sneh r5, r6, r7 ; slteb_u r15, r16, r17 }
    e180:	[0-9a-f]* 	{ sneh r5, r6, r7 ; sub r15, r16, r17 }
    e188:	[0-9a-f]* 	{ sra r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    e190:	[0-9a-f]* 	{ sra r15, r16, r17 ; adds r5, r6, r7 }
    e198:	[0-9a-f]* 	{ sra r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    e1a0:	[0-9a-f]* 	{ bytex r5, r6 ; sra r15, r16, r17 ; lw r25, r26 }
    e1a8:	[0-9a-f]* 	{ ctz r5, r6 ; sra r15, r16, r17 ; lh r25, r26 }
    e1b0:	[0-9a-f]* 	{ sra r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    e1b8:	[0-9a-f]* 	{ clz r5, r6 ; sra r15, r16, r17 ; lb r25, r26 }
    e1c0:	[0-9a-f]* 	{ sra r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    e1c8:	[0-9a-f]* 	{ sra r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    e1d0:	[0-9a-f]* 	{ sra r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    e1d8:	[0-9a-f]* 	{ pcnt r5, r6 ; sra r15, r16, r17 ; lb_u r25, r26 }
    e1e0:	[0-9a-f]* 	{ sra r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    e1e8:	[0-9a-f]* 	{ sra r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    e1f0:	[0-9a-f]* 	{ sra r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    e1f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sra r15, r16, r17 ; lh r25, r26 }
    e200:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sra r15, r16, r17 ; lh_u r25, r26 }
    e208:	[0-9a-f]* 	{ sra r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    e210:	[0-9a-f]* 	{ sra r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    e218:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    e220:	[0-9a-f]* 	{ sra r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    e228:	[0-9a-f]* 	{ sra r15, r16, r17 ; maxh r5, r6, r7 }
    e230:	[0-9a-f]* 	{ sra r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    e238:	[0-9a-f]* 	{ sra r15, r16, r17 ; moveli r5, 4660 }
    e240:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
    e248:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    e250:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
    e258:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    e260:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
    e268:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
    e270:	[0-9a-f]* 	{ sra r15, r16, r17 ; nop ; lh r25, r26 }
    e278:	[0-9a-f]* 	{ sra r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    e280:	[0-9a-f]* 	{ sra r15, r16, r17 ; packhs r5, r6, r7 }
    e288:	[0-9a-f]* 	{ sra r15, r16, r17 ; prefetch r25 }
    e290:	[0-9a-f]* 	{ sra r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    e298:	[0-9a-f]* 	{ sra r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    e2a0:	[0-9a-f]* 	{ sra r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    e2a8:	[0-9a-f]* 	{ sra r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    e2b0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; sra r15, r16, r17 }
    e2b8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
    e2c0:	[0-9a-f]* 	{ sra r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    e2c8:	[0-9a-f]* 	{ sra r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    e2d0:	[0-9a-f]* 	{ sra r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    e2d8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
    e2e0:	[0-9a-f]* 	{ sra r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    e2e8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sra r15, r16, r17 ; sh r25, r26 }
    e2f0:	[0-9a-f]* 	{ sra r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    e2f8:	[0-9a-f]* 	{ sra r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    e300:	[0-9a-f]* 	{ sra r15, r16, r17 ; slt r5, r6, r7 }
    e308:	[0-9a-f]* 	{ sra r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    e310:	[0-9a-f]* 	{ sra r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    e318:	[0-9a-f]* 	{ sra r15, r16, r17 ; sltib_u r5, r6, 5 }
    e320:	[0-9a-f]* 	{ sra r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    e328:	[0-9a-f]* 	{ sra r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    e330:	[0-9a-f]* 	{ clz r5, r6 ; sra r15, r16, r17 ; sw r25, r26 }
    e338:	[0-9a-f]* 	{ sra r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    e340:	[0-9a-f]* 	{ sra r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    e348:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sra r15, r16, r17 }
    e350:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 }
    e358:	[0-9a-f]* 	{ sra r15, r16, r17 ; xor r5, r6, r7 }
    e360:	[0-9a-f]* 	{ sra r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    e368:	[0-9a-f]* 	{ sra r5, r6, r7 ; and r15, r16, r17 }
    e370:	[0-9a-f]* 	{ sra r5, r6, r7 ; prefetch r25 }
    e378:	[0-9a-f]* 	{ sra r5, r6, r7 ; info 19 ; lw r25, r26 }
    e380:	[0-9a-f]* 	{ sra r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    e388:	[0-9a-f]* 	{ sra r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    e390:	[0-9a-f]* 	{ sra r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    e398:	[0-9a-f]* 	{ sra r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    e3a0:	[0-9a-f]* 	{ sra r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    e3a8:	[0-9a-f]* 	{ sra r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    e3b0:	[0-9a-f]* 	{ sra r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    e3b8:	[0-9a-f]* 	{ sra r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    e3c0:	[0-9a-f]* 	{ sra r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    e3c8:	[0-9a-f]* 	{ sra r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    e3d0:	[0-9a-f]* 	{ sra r5, r6, r7 ; maxb_u r15, r16, r17 }
    e3d8:	[0-9a-f]* 	{ sra r5, r6, r7 ; mnz r15, r16, r17 }
    e3e0:	[0-9a-f]* 	{ sra r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    e3e8:	[0-9a-f]* 	{ sra r5, r6, r7 ; nop ; lh r25, r26 }
    e3f0:	[0-9a-f]* 	{ sra r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    e3f8:	[0-9a-f]* 	{ sra r5, r6, r7 ; packhs r15, r16, r17 }
    e400:	[0-9a-f]* 	{ sra r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    e408:	[0-9a-f]* 	{ sra r5, r6, r7 ; prefetch r25 }
    e410:	[0-9a-f]* 	{ sra r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    e418:	[0-9a-f]* 	{ sra r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    e420:	[0-9a-f]* 	{ sra r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    e428:	[0-9a-f]* 	{ sra r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e430:	[0-9a-f]* 	{ sra r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    e438:	[0-9a-f]* 	{ sra r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    e440:	[0-9a-f]* 	{ sra r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    e448:	[0-9a-f]* 	{ sra r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    e450:	[0-9a-f]* 	{ sra r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    e458:	[0-9a-f]* 	{ sra r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    e460:	[0-9a-f]* 	{ sra r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e468:	[0-9a-f]* 	{ sra r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    e470:	[0-9a-f]* 	{ sra r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    e478:	[0-9a-f]* 	{ sra r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    e480:	[0-9a-f]* 	{ sra r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    e488:	[0-9a-f]* 	{ sra r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    e490:	[0-9a-f]* 	{ sra r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    e498:	[0-9a-f]* 	{ sra r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    e4a0:	[0-9a-f]* 	{ srab r15, r16, r17 ; add r5, r6, r7 }
    e4a8:	[0-9a-f]* 	{ clz r5, r6 ; srab r15, r16, r17 }
    e4b0:	[0-9a-f]* 	{ srab r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    e4b8:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; srab r15, r16, r17 }
    e4c0:	[0-9a-f]* 	{ srab r15, r16, r17 ; packbs_u r5, r6, r7 }
    e4c8:	[0-9a-f]* 	{ srab r15, r16, r17 ; seqib r5, r6, 5 }
    e4d0:	[0-9a-f]* 	{ srab r15, r16, r17 ; slteb r5, r6, r7 }
    e4d8:	[0-9a-f]* 	{ srab r15, r16, r17 ; sraih r5, r6, 5 }
    e4e0:	[0-9a-f]* 	{ srab r5, r6, r7 ; addih r15, r16, 5 }
    e4e8:	[0-9a-f]* 	{ srab r5, r6, r7 ; iret }
    e4f0:	[0-9a-f]* 	{ srab r5, r6, r7 ; maxib_u r15, r16, 5 }
    e4f8:	[0-9a-f]* 	{ srab r5, r6, r7 ; nop }
    e500:	[0-9a-f]* 	{ srab r5, r6, r7 ; seqi r15, r16, 5 }
    e508:	[0-9a-f]* 	{ srab r5, r6, r7 ; sltb_u r15, r16, r17 }
    e510:	[0-9a-f]* 	{ srab r5, r6, r7 ; srah r15, r16, r17 }
    e518:	[0-9a-f]* 	{ srah r15, r16, r17 ; addhs r5, r6, r7 }
    e520:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; srah r15, r16, r17 }
    e528:	[0-9a-f]* 	{ srah r15, r16, r17 ; move r5, r6 }
    e530:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srah r15, r16, r17 }
    e538:	[0-9a-f]* 	{ pcnt r5, r6 ; srah r15, r16, r17 }
    e540:	[0-9a-f]* 	{ srah r15, r16, r17 ; shlh r5, r6, r7 }
    e548:	[0-9a-f]* 	{ srah r15, r16, r17 ; slth r5, r6, r7 }
    e550:	[0-9a-f]* 	{ srah r15, r16, r17 ; subh r5, r6, r7 }
    e558:	[0-9a-f]* 	{ srah r5, r6, r7 ; and r15, r16, r17 }
    e560:	[0-9a-f]* 	{ srah r5, r6, r7 ; jrp r15 }
    e568:	[0-9a-f]* 	{ srah r5, r6, r7 ; minb_u r15, r16, r17 }
    e570:	[0-9a-f]* 	{ srah r5, r6, r7 ; packbs_u r15, r16, r17 }
    e578:	[0-9a-f]* 	{ srah r5, r6, r7 ; shadd r15, r16, 5 }
    e580:	[0-9a-f]* 	{ srah r5, r6, r7 ; slteb_u r15, r16, r17 }
    e588:	[0-9a-f]* 	{ srah r5, r6, r7 ; sub r15, r16, r17 }
    e590:	[0-9a-f]* 	{ srai r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
    e598:	[0-9a-f]* 	{ srai r15, r16, 5 ; adds r5, r6, r7 }
    e5a0:	[0-9a-f]* 	{ srai r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
    e5a8:	[0-9a-f]* 	{ bytex r5, r6 ; srai r15, r16, 5 ; lw r25, r26 }
    e5b0:	[0-9a-f]* 	{ ctz r5, r6 ; srai r15, r16, 5 ; lh r25, r26 }
    e5b8:	[0-9a-f]* 	{ srai r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    e5c0:	[0-9a-f]* 	{ clz r5, r6 ; srai r15, r16, 5 ; lb r25, r26 }
    e5c8:	[0-9a-f]* 	{ srai r15, r16, 5 ; nor r5, r6, r7 ; lb r25, r26 }
    e5d0:	[0-9a-f]* 	{ srai r15, r16, 5 ; slti_u r5, r6, 5 ; lb r25, r26 }
    e5d8:	[0-9a-f]* 	{ srai r15, r16, 5 ; info 19 ; lb_u r25, r26 }
    e5e0:	[0-9a-f]* 	{ pcnt r5, r6 ; srai r15, r16, 5 ; lb_u r25, r26 }
    e5e8:	[0-9a-f]* 	{ srai r15, r16, 5 ; srai r5, r6, 5 ; lb_u r25, r26 }
    e5f0:	[0-9a-f]* 	{ srai r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
    e5f8:	[0-9a-f]* 	{ srai r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
    e600:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; srai r15, r16, 5 ; lh r25, r26 }
    e608:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
    e610:	[0-9a-f]* 	{ srai r15, r16, 5 ; seq r5, r6, r7 ; lh_u r25, r26 }
    e618:	[0-9a-f]* 	{ srai r15, r16, 5 ; xor r5, r6, r7 ; lh_u r25, r26 }
    e620:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    e628:	[0-9a-f]* 	{ srai r15, r16, 5 ; shli r5, r6, 5 ; lw r25, r26 }
    e630:	[0-9a-f]* 	{ srai r15, r16, 5 ; maxh r5, r6, r7 }
    e638:	[0-9a-f]* 	{ srai r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
    e640:	[0-9a-f]* 	{ srai r15, r16, 5 ; moveli r5, 4660 }
    e648:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    e650:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
    e658:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    e660:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
    e668:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; srai r15, r16, 5 ; prefetch r25 }
    e670:	[0-9a-f]* 	{ mvz r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
    e678:	[0-9a-f]* 	{ srai r15, r16, 5 ; nop ; lh r25, r26 }
    e680:	[0-9a-f]* 	{ srai r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
    e688:	[0-9a-f]* 	{ srai r15, r16, 5 ; packhs r5, r6, r7 }
    e690:	[0-9a-f]* 	{ srai r15, r16, 5 ; prefetch r25 }
    e698:	[0-9a-f]* 	{ srai r15, r16, 5 ; ori r5, r6, 5 ; prefetch r25 }
    e6a0:	[0-9a-f]* 	{ srai r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    e6a8:	[0-9a-f]* 	{ srai r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
    e6b0:	[0-9a-f]* 	{ srai r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    e6b8:	[0-9a-f]* 	{ sadah r5, r6, r7 ; srai r15, r16, 5 }
    e6c0:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
    e6c8:	[0-9a-f]* 	{ srai r15, r16, 5 ; seq r5, r6, r7 ; sb r25, r26 }
    e6d0:	[0-9a-f]* 	{ srai r15, r16, 5 ; xor r5, r6, r7 ; sb r25, r26 }
    e6d8:	[0-9a-f]* 	{ srai r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
    e6e0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; srai r15, r16, 5 ; sh r25, r26 }
    e6e8:	[0-9a-f]* 	{ srai r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
    e6f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; sh r25, r26 }
    e6f8:	[0-9a-f]* 	{ srai r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
    e700:	[0-9a-f]* 	{ srai r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
    e708:	[0-9a-f]* 	{ srai r15, r16, 5 ; slt r5, r6, r7 }
    e710:	[0-9a-f]* 	{ srai r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
    e718:	[0-9a-f]* 	{ srai r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
    e720:	[0-9a-f]* 	{ srai r15, r16, 5 ; sltib_u r5, r6, 5 }
    e728:	[0-9a-f]* 	{ srai r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
    e730:	[0-9a-f]* 	{ srai r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
    e738:	[0-9a-f]* 	{ clz r5, r6 ; srai r15, r16, 5 ; sw r25, r26 }
    e740:	[0-9a-f]* 	{ srai r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
    e748:	[0-9a-f]* 	{ srai r15, r16, 5 ; slti_u r5, r6, 5 ; sw r25, r26 }
    e750:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; srai r15, r16, 5 }
    e758:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; srai r15, r16, 5 }
    e760:	[0-9a-f]* 	{ srai r15, r16, 5 ; xor r5, r6, r7 }
    e768:	[0-9a-f]* 	{ srai r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    e770:	[0-9a-f]* 	{ srai r5, r6, 5 ; and r15, r16, r17 }
    e778:	[0-9a-f]* 	{ srai r5, r6, 5 ; prefetch r25 }
    e780:	[0-9a-f]* 	{ srai r5, r6, 5 ; info 19 ; lw r25, r26 }
    e788:	[0-9a-f]* 	{ srai r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
    e790:	[0-9a-f]* 	{ srai r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
    e798:	[0-9a-f]* 	{ srai r5, r6, 5 ; andi r15, r16, 5 ; lb_u r25, r26 }
    e7a0:	[0-9a-f]* 	{ srai r5, r6, 5 ; shli r15, r16, 5 ; lb_u r25, r26 }
    e7a8:	[0-9a-f]* 	{ srai r5, r6, 5 ; and r15, r16, r17 ; lh r25, r26 }
    e7b0:	[0-9a-f]* 	{ srai r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
    e7b8:	[0-9a-f]* 	{ srai r5, r6, 5 ; andi r15, r16, 5 ; lh_u r25, r26 }
    e7c0:	[0-9a-f]* 	{ srai r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
    e7c8:	[0-9a-f]* 	{ srai r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
    e7d0:	[0-9a-f]* 	{ srai r5, r6, 5 ; seqi r15, r16, 5 ; lw r25, r26 }
    e7d8:	[0-9a-f]* 	{ srai r5, r6, 5 ; maxb_u r15, r16, r17 }
    e7e0:	[0-9a-f]* 	{ srai r5, r6, 5 ; mnz r15, r16, r17 }
    e7e8:	[0-9a-f]* 	{ srai r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
    e7f0:	[0-9a-f]* 	{ srai r5, r6, 5 ; nop ; lh r25, r26 }
    e7f8:	[0-9a-f]* 	{ srai r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
    e800:	[0-9a-f]* 	{ srai r5, r6, 5 ; packhs r15, r16, r17 }
    e808:	[0-9a-f]* 	{ srai r5, r6, 5 ; s1a r15, r16, r17 ; prefetch r25 }
    e810:	[0-9a-f]* 	{ srai r5, r6, 5 ; prefetch r25 }
    e818:	[0-9a-f]* 	{ srai r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
    e820:	[0-9a-f]* 	{ srai r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
    e828:	[0-9a-f]* 	{ srai r5, r6, 5 ; mnz r15, r16, r17 ; sb r25, r26 }
    e830:	[0-9a-f]* 	{ srai r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e838:	[0-9a-f]* 	{ srai r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
    e840:	[0-9a-f]* 	{ srai r5, r6, 5 ; andi r15, r16, 5 ; sh r25, r26 }
    e848:	[0-9a-f]* 	{ srai r5, r6, 5 ; shli r15, r16, 5 ; sh r25, r26 }
    e850:	[0-9a-f]* 	{ srai r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
    e858:	[0-9a-f]* 	{ srai r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
    e860:	[0-9a-f]* 	{ srai r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
    e868:	[0-9a-f]* 	{ srai r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
    e870:	[0-9a-f]* 	{ srai r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
    e878:	[0-9a-f]* 	{ srai r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
    e880:	[0-9a-f]* 	{ srai r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
    e888:	[0-9a-f]* 	{ srai r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
    e890:	[0-9a-f]* 	{ srai r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
    e898:	[0-9a-f]* 	{ srai r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    e8a0:	[0-9a-f]* 	{ srai r5, r6, 5 ; sne r15, r16, r17 ; sw r25, r26 }
    e8a8:	[0-9a-f]* 	{ sraib r15, r16, 5 ; add r5, r6, r7 }
    e8b0:	[0-9a-f]* 	{ clz r5, r6 ; sraib r15, r16, 5 }
    e8b8:	[0-9a-f]* 	{ sraib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
    e8c0:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; sraib r15, r16, 5 }
    e8c8:	[0-9a-f]* 	{ sraib r15, r16, 5 ; packbs_u r5, r6, r7 }
    e8d0:	[0-9a-f]* 	{ sraib r15, r16, 5 ; seqib r5, r6, 5 }
    e8d8:	[0-9a-f]* 	{ sraib r15, r16, 5 ; slteb r5, r6, r7 }
    e8e0:	[0-9a-f]* 	{ sraib r15, r16, 5 ; sraih r5, r6, 5 }
    e8e8:	[0-9a-f]* 	{ sraib r5, r6, 5 ; addih r15, r16, 5 }
    e8f0:	[0-9a-f]* 	{ sraib r5, r6, 5 ; iret }
    e8f8:	[0-9a-f]* 	{ sraib r5, r6, 5 ; maxib_u r15, r16, 5 }
    e900:	[0-9a-f]* 	{ sraib r5, r6, 5 ; nop }
    e908:	[0-9a-f]* 	{ sraib r5, r6, 5 ; seqi r15, r16, 5 }
    e910:	[0-9a-f]* 	{ sraib r5, r6, 5 ; sltb_u r15, r16, r17 }
    e918:	[0-9a-f]* 	{ sraib r5, r6, 5 ; srah r15, r16, r17 }
    e920:	[0-9a-f]* 	{ sraih r15, r16, 5 ; addhs r5, r6, r7 }
    e928:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; sraih r15, r16, 5 }
    e930:	[0-9a-f]* 	{ sraih r15, r16, 5 ; move r5, r6 }
    e938:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sraih r15, r16, 5 }
    e940:	[0-9a-f]* 	{ pcnt r5, r6 ; sraih r15, r16, 5 }
    e948:	[0-9a-f]* 	{ sraih r15, r16, 5 ; shlh r5, r6, r7 }
    e950:	[0-9a-f]* 	{ sraih r15, r16, 5 ; slth r5, r6, r7 }
    e958:	[0-9a-f]* 	{ sraih r15, r16, 5 ; subh r5, r6, r7 }
    e960:	[0-9a-f]* 	{ sraih r5, r6, 5 ; and r15, r16, r17 }
    e968:	[0-9a-f]* 	{ sraih r5, r6, 5 ; jrp r15 }
    e970:	[0-9a-f]* 	{ sraih r5, r6, 5 ; minb_u r15, r16, r17 }
    e978:	[0-9a-f]* 	{ sraih r5, r6, 5 ; packbs_u r15, r16, r17 }
    e980:	[0-9a-f]* 	{ sraih r5, r6, 5 ; shadd r15, r16, 5 }
    e988:	[0-9a-f]* 	{ sraih r5, r6, 5 ; slteb_u r15, r16, r17 }
    e990:	[0-9a-f]* 	{ sraih r5, r6, 5 ; sub r15, r16, r17 }
    e998:	[0-9a-f]* 	{ sub r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
    e9a0:	[0-9a-f]* 	{ sub r15, r16, r17 ; adds r5, r6, r7 }
    e9a8:	[0-9a-f]* 	{ sub r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
    e9b0:	[0-9a-f]* 	{ bytex r5, r6 ; sub r15, r16, r17 ; lw r25, r26 }
    e9b8:	[0-9a-f]* 	{ ctz r5, r6 ; sub r15, r16, r17 ; lh r25, r26 }
    e9c0:	[0-9a-f]* 	{ sub r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    e9c8:	[0-9a-f]* 	{ clz r5, r6 ; sub r15, r16, r17 ; lb r25, r26 }
    e9d0:	[0-9a-f]* 	{ sub r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
    e9d8:	[0-9a-f]* 	{ sub r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
    e9e0:	[0-9a-f]* 	{ sub r15, r16, r17 ; info 19 ; lb_u r25, r26 }
    e9e8:	[0-9a-f]* 	{ pcnt r5, r6 ; sub r15, r16, r17 ; lb_u r25, r26 }
    e9f0:	[0-9a-f]* 	{ sub r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
    e9f8:	[0-9a-f]* 	{ sub r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
    ea00:	[0-9a-f]* 	{ sub r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
    ea08:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sub r15, r16, r17 ; lh r25, r26 }
    ea10:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
    ea18:	[0-9a-f]* 	{ sub r15, r16, r17 ; seq r5, r6, r7 ; lh_u r25, r26 }
    ea20:	[0-9a-f]* 	{ sub r15, r16, r17 ; xor r5, r6, r7 ; lh_u r25, r26 }
    ea28:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    ea30:	[0-9a-f]* 	{ sub r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    ea38:	[0-9a-f]* 	{ sub r15, r16, r17 ; maxh r5, r6, r7 }
    ea40:	[0-9a-f]* 	{ sub r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
    ea48:	[0-9a-f]* 	{ sub r15, r16, r17 ; moveli r5, 4660 }
    ea50:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    ea58:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    ea60:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    ea68:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    ea70:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
    ea78:	[0-9a-f]* 	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
    ea80:	[0-9a-f]* 	{ sub r15, r16, r17 ; nop ; lh r25, r26 }
    ea88:	[0-9a-f]* 	{ sub r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
    ea90:	[0-9a-f]* 	{ sub r15, r16, r17 ; packhs r5, r6, r7 }
    ea98:	[0-9a-f]* 	{ sub r15, r16, r17 ; prefetch r25 }
    eaa0:	[0-9a-f]* 	{ sub r15, r16, r17 ; ori r5, r6, 5 ; prefetch r25 }
    eaa8:	[0-9a-f]* 	{ sub r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    eab0:	[0-9a-f]* 	{ sub r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
    eab8:	[0-9a-f]* 	{ sub r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
    eac0:	[0-9a-f]* 	{ sadah r5, r6, r7 ; sub r15, r16, r17 }
    eac8:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sub r15, r16, r17 ; sb r25, r26 }
    ead0:	[0-9a-f]* 	{ sub r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
    ead8:	[0-9a-f]* 	{ sub r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
    eae0:	[0-9a-f]* 	{ sub r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
    eae8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    eaf0:	[0-9a-f]* 	{ sub r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
    eaf8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; sh r25, r26 }
    eb00:	[0-9a-f]* 	{ sub r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
    eb08:	[0-9a-f]* 	{ sub r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
    eb10:	[0-9a-f]* 	{ sub r15, r16, r17 ; slt r5, r6, r7 }
    eb18:	[0-9a-f]* 	{ sub r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
    eb20:	[0-9a-f]* 	{ sub r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
    eb28:	[0-9a-f]* 	{ sub r15, r16, r17 ; sltib_u r5, r6, 5 }
    eb30:	[0-9a-f]* 	{ sub r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
    eb38:	[0-9a-f]* 	{ sub r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
    eb40:	[0-9a-f]* 	{ clz r5, r6 ; sub r15, r16, r17 ; sw r25, r26 }
    eb48:	[0-9a-f]* 	{ sub r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
    eb50:	[0-9a-f]* 	{ sub r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
    eb58:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sub r15, r16, r17 }
    eb60:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sub r15, r16, r17 }
    eb68:	[0-9a-f]* 	{ sub r15, r16, r17 ; xor r5, r6, r7 }
    eb70:	[0-9a-f]* 	{ sub r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    eb78:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 }
    eb80:	[0-9a-f]* 	{ sub r5, r6, r7 ; prefetch r25 }
    eb88:	[0-9a-f]* 	{ sub r5, r6, r7 ; info 19 ; lw r25, r26 }
    eb90:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
    eb98:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
    eba0:	[0-9a-f]* 	{ sub r5, r6, r7 ; andi r15, r16, 5 ; lb_u r25, r26 }
    eba8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
    ebb0:	[0-9a-f]* 	{ sub r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
    ebb8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lh r25, r26 }
    ebc0:	[0-9a-f]* 	{ sub r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
    ebc8:	[0-9a-f]* 	{ sub r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
    ebd0:	[0-9a-f]* 	{ sub r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
    ebd8:	[0-9a-f]* 	{ sub r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
    ebe0:	[0-9a-f]* 	{ sub r5, r6, r7 ; maxb_u r15, r16, r17 }
    ebe8:	[0-9a-f]* 	{ sub r5, r6, r7 ; mnz r15, r16, r17 }
    ebf0:	[0-9a-f]* 	{ sub r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
    ebf8:	[0-9a-f]* 	{ sub r5, r6, r7 ; nop ; lh r25, r26 }
    ec00:	[0-9a-f]* 	{ sub r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
    ec08:	[0-9a-f]* 	{ sub r5, r6, r7 ; packhs r15, r16, r17 }
    ec10:	[0-9a-f]* 	{ sub r5, r6, r7 ; s1a r15, r16, r17 ; prefetch r25 }
    ec18:	[0-9a-f]* 	{ sub r5, r6, r7 ; prefetch r25 }
    ec20:	[0-9a-f]* 	{ sub r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    ec28:	[0-9a-f]* 	{ sub r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    ec30:	[0-9a-f]* 	{ sub r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
    ec38:	[0-9a-f]* 	{ sub r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    ec40:	[0-9a-f]* 	{ sub r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    ec48:	[0-9a-f]* 	{ sub r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
    ec50:	[0-9a-f]* 	{ sub r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
    ec58:	[0-9a-f]* 	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
    ec60:	[0-9a-f]* 	{ sub r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
    ec68:	[0-9a-f]* 	{ sub r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    ec70:	[0-9a-f]* 	{ sub r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
    ec78:	[0-9a-f]* 	{ sub r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
    ec80:	[0-9a-f]* 	{ sub r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    ec88:	[0-9a-f]* 	{ sub r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
    ec90:	[0-9a-f]* 	{ sub r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
    ec98:	[0-9a-f]* 	{ sub r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    eca0:	[0-9a-f]* 	{ sub r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    eca8:	[0-9a-f]* 	{ sub r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    ecb0:	[0-9a-f]* 	{ subb r15, r16, r17 ; add r5, r6, r7 }
    ecb8:	[0-9a-f]* 	{ clz r5, r6 ; subb r15, r16, r17 }
    ecc0:	[0-9a-f]* 	{ subb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
    ecc8:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; subb r15, r16, r17 }
    ecd0:	[0-9a-f]* 	{ subb r15, r16, r17 ; packbs_u r5, r6, r7 }
    ecd8:	[0-9a-f]* 	{ subb r15, r16, r17 ; seqib r5, r6, 5 }
    ece0:	[0-9a-f]* 	{ subb r15, r16, r17 ; slteb r5, r6, r7 }
    ece8:	[0-9a-f]* 	{ subb r15, r16, r17 ; sraih r5, r6, 5 }
    ecf0:	[0-9a-f]* 	{ subb r5, r6, r7 ; addih r15, r16, 5 }
    ecf8:	[0-9a-f]* 	{ subb r5, r6, r7 ; iret }
    ed00:	[0-9a-f]* 	{ subb r5, r6, r7 ; maxib_u r15, r16, 5 }
    ed08:	[0-9a-f]* 	{ subb r5, r6, r7 ; nop }
    ed10:	[0-9a-f]* 	{ subb r5, r6, r7 ; seqi r15, r16, 5 }
    ed18:	[0-9a-f]* 	{ subb r5, r6, r7 ; sltb_u r15, r16, r17 }
    ed20:	[0-9a-f]* 	{ subb r5, r6, r7 ; srah r15, r16, r17 }
    ed28:	[0-9a-f]* 	{ subbs_u r15, r16, r17 ; addhs r5, r6, r7 }
    ed30:	[0-9a-f]* 	{ dword_align r5, r6, r7 ; subbs_u r15, r16, r17 }
    ed38:	[0-9a-f]* 	{ subbs_u r15, r16, r17 ; move r5, r6 }
    ed40:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; subbs_u r15, r16, r17 }
    ed48:	[0-9a-f]* 	{ pcnt r5, r6 ; subbs_u r15, r16, r17 }
    ed50:	[0-9a-f]* 	{ subbs_u r15, r16, r17 ; shlh r5, r6, r7 }
    ed58:	[0-9a-f]* 	{ subbs_u r15, r16, r17 ; slth r5, r6, r7 }
    ed60:	[0-9a-f]* 	{ subbs_u r15, r16, r17 ; subh r5, r6, r7 }
    ed68:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; and r15, r16, r17 }
    ed70:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; jrp r15 }
    ed78:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; minb_u r15, r16, r17 }
    ed80:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; packbs_u r15, r16, r17 }
    ed88:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; shadd r15, r16, 5 }
    ed90:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; slteb_u r15, r16, r17 }
    ed98:	[0-9a-f]* 	{ subbs_u r5, r6, r7 ; sub r15, r16, r17 }
    eda0:	[0-9a-f]* 	{ subh r15, r16, r17 ; addli r5, r6, 4660 }
    eda8:	[0-9a-f]* 	{ subh r15, r16, r17 ; inthb r5, r6, r7 }
    edb0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; subh r15, r16, r17 }
    edb8:	[0-9a-f]* 	{ mullla_su r5, r6, r7 ; subh r15, r16, r17 }
    edc0:	[0-9a-f]* 	{ subh r15, r16, r17 ; s2a r5, r6, r7 }
    edc8:	[0-9a-f]* 	{ subh r15, r16, r17 ; shr r5, r6, r7 }
    edd0:	[0-9a-f]* 	{ subh r15, r16, r17 ; sltib r5, r6, 5 }
    edd8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; subh r15, r16, r17 }
    ede0:	[0-9a-f]* 	{ subh r5, r6, r7 ; finv r15 }
    ede8:	[0-9a-f]* 	{ subh r5, r6, r7 ; lbadd_u r15, r16, 5 }
    edf0:	[0-9a-f]* 	{ subh r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
    edf8:	[0-9a-f]* 	{ subh r5, r6, r7 ; prefetch r15 }
    ee00:	[0-9a-f]* 	{ subh r5, r6, r7 ; shli r15, r16, 5 }
    ee08:	[0-9a-f]* 	{ subh r5, r6, r7 ; slth_u r15, r16, r17 }
    ee10:	[0-9a-f]* 	{ subh r5, r6, r7 ; subhs r15, r16, r17 }
    ee18:	[0-9a-f]* 	{ adiffh r5, r6, r7 ; subhs r15, r16, r17 }
    ee20:	[0-9a-f]* 	{ subhs r15, r16, r17 ; maxb_u r5, r6, r7 }
    ee28:	[0-9a-f]* 	{ mulhha_su r5, r6, r7 ; subhs r15, r16, r17 }
    ee30:	[0-9a-f]* 	{ mvz r5, r6, r7 ; subhs r15, r16, r17 }
    ee38:	[0-9a-f]* 	{ sadah_u r5, r6, r7 ; subhs r15, r16, r17 }
    ee40:	[0-9a-f]* 	{ subhs r15, r16, r17 ; shrib r5, r6, 5 }
    ee48:	[0-9a-f]* 	{ subhs r15, r16, r17 ; sne r5, r6, r7 }
    ee50:	[0-9a-f]* 	{ subhs r15, r16, r17 ; xori r5, r6, 5 }
    ee58:	[0-9a-f]* 	{ subhs r5, r6, r7 ; ill }
    ee60:	[0-9a-f]* 	{ subhs r5, r6, r7 ; lhadd_u r15, r16, 5 }
    ee68:	[0-9a-f]* 	{ subhs r5, r6, r7 ; move r15, r16 }
    ee70:	[0-9a-f]* 	{ subhs r5, r6, r7 ; s1a r15, r16, r17 }
    ee78:	[0-9a-f]* 	{ subhs r5, r6, r7 ; shrb r15, r16, r17 }
    ee80:	[0-9a-f]* 	{ subhs r5, r6, r7 ; sltib_u r15, r16, 5 }
    ee88:	[0-9a-f]* 	{ subhs r5, r6, r7 ; tns r15, r16 }
    ee90:	[0-9a-f]* 	{ avgb_u r5, r6, r7 ; subs r15, r16, r17 }
    ee98:	[0-9a-f]* 	{ subs r15, r16, r17 ; minb_u r5, r6, r7 }
    eea0:	[0-9a-f]* 	{ mulhl_su r5, r6, r7 ; subs r15, r16, r17 }
    eea8:	[0-9a-f]* 	{ subs r15, r16, r17 ; nop }
    eeb0:	[0-9a-f]* 	{ subs r15, r16, r17 ; seq r5, r6, r7 }
    eeb8:	[0-9a-f]* 	{ subs r15, r16, r17 ; sltb r5, r6, r7 }
    eec0:	[0-9a-f]* 	{ subs r15, r16, r17 ; srab r5, r6, r7 }
    eec8:	[0-9a-f]* 	{ subs r5, r6, r7 ; addh r15, r16, r17 }
    eed0:	[0-9a-f]* 	{ subs r5, r6, r7 ; inthh r15, r16, r17 }
    eed8:	[0-9a-f]* 	{ subs r5, r6, r7 ; lwadd r15, r16, 5 }
    eee0:	[0-9a-f]* 	{ subs r5, r6, r7 ; mtspr 5, r16 }
    eee8:	[0-9a-f]* 	{ subs r5, r6, r7 ; sbadd r15, r16, 5 }
    eef0:	[0-9a-f]* 	{ subs r5, r6, r7 ; shrih r15, r16, 5 }
    eef8:	[0-9a-f]* 	{ subs r5, r6, r7 ; sneb r15, r16, r17 }
    ef00:	[0-9a-f]* 	{ add r5, r6, r7 ; sw r15, r16 }
    ef08:	[0-9a-f]* 	{ clz r5, r6 ; sw r15, r16 }
    ef10:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; sw r15, r16 }
    ef18:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; sw r15, r16 }
    ef20:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; sw r15, r16 }
    ef28:	[0-9a-f]* 	{ seqib r5, r6, 5 ; sw r15, r16 }
    ef30:	[0-9a-f]* 	{ slteb r5, r6, r7 ; sw r15, r16 }
    ef38:	[0-9a-f]* 	{ sraih r5, r6, 5 ; sw r15, r16 }
    ef40:	[0-9a-f]* 	{ ctz r5, r6 ; add r15, r16, r17 ; sw r25, r26 }
    ef48:	[0-9a-f]* 	{ add r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
    ef50:	[0-9a-f]* 	{ add r15, r16, r17 ; sne r5, r6, r7 ; sw r25, r26 }
    ef58:	[0-9a-f]* 	{ add r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
    ef60:	[0-9a-f]* 	{ add r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    ef68:	[0-9a-f]* 	{ addi r15, r16, 5 ; movei r5, 5 ; sw r25, r26 }
    ef70:	[0-9a-f]* 	{ addi r15, r16, 5 ; s1a r5, r6, r7 ; sw r25, r26 }
    ef78:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; sw r25, r26 }
    ef80:	[0-9a-f]* 	{ addi r5, r6, 5 ; rl r15, r16, r17 ; sw r25, r26 }
    ef88:	[0-9a-f]* 	{ addi r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
    ef90:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
    ef98:	[0-9a-f]* 	{ and r15, r16, r17 ; shl r5, r6, r7 ; sw r25, r26 }
    efa0:	[0-9a-f]* 	{ and r5, r6, r7 ; add r15, r16, r17 ; sw r25, r26 }
    efa8:	[0-9a-f]* 	{ and r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    efb0:	[0-9a-f]* 	{ andi r15, r16, 5 ; and r5, r6, r7 ; sw r25, r26 }
    efb8:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
    efc0:	[0-9a-f]* 	{ andi r15, r16, 5 ; slt_u r5, r6, r7 ; sw r25, r26 }
    efc8:	[0-9a-f]* 	{ andi r5, r6, 5 ; ill ; sw r25, r26 }
    efd0:	[0-9a-f]* 	{ andi r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
    efd8:	[0-9a-f]* 	{ bitx r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
    efe0:	[0-9a-f]* 	{ bitx r5, r6 ; slt_u r15, r16, r17 ; sw r25, r26 }
    efe8:	[0-9a-f]* 	{ bytex r5, r6 ; movei r15, 5 ; sw r25, r26 }
    eff0:	[0-9a-f]* 	{ bytex r5, r6 ; slte_u r15, r16, r17 ; sw r25, r26 }
    eff8:	[0-9a-f]* 	{ clz r5, r6 ; nop ; sw r25, r26 }
    f000:	[0-9a-f]* 	{ clz r5, r6 ; slti_u r15, r16, 5 ; sw r25, r26 }
    f008:	[0-9a-f]* 	{ ctz r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    f010:	[0-9a-f]* 	{ ctz r5, r6 ; sra r15, r16, r17 ; sw r25, r26 }
    f018:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sw r25, r26 }
    f020:	[0-9a-f]* 	{ nor r15, r16, r17 ; sw r25, r26 }
    f028:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sw r25, r26 }
    f030:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; sw r25, r26 }
    f038:	[0-9a-f]* 	{ bitx r5, r6 ; ill ; sw r25, r26 }
    f040:	[0-9a-f]* 	{ mz r5, r6, r7 ; ill ; sw r25, r26 }
    f048:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; ill ; sw r25, r26 }
    f050:	[0-9a-f]* 	{ info 19 ; andi r5, r6, 5 ; sw r25, r26 }
    f058:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; info 19 ; sw r25, r26 }
    f060:	[0-9a-f]* 	{ info 19 ; s1a r5, r6, r7 ; sw r25, r26 }
    f068:	[0-9a-f]* 	{ info 19 ; slt_u r5, r6, r7 ; sw r25, r26 }
    f070:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; info 19 ; sw r25, r26 }
    f078:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f080:	[0-9a-f]* 	{ mnz r15, r16, r17 ; seqi r5, r6, 5 ; sw r25, r26 }
    f088:	[0-9a-f]* 	{ mnz r15, r16, r17 ; sw r25, r26 }
    f090:	[0-9a-f]* 	{ mnz r5, r6, r7 ; s3a r15, r16, r17 ; sw r25, r26 }
    f098:	[0-9a-f]* 	{ move r15, r16 ; addi r5, r6, 5 ; sw r25, r26 }
    f0a0:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
    f0a8:	[0-9a-f]* 	{ move r15, r16 ; slt r5, r6, r7 ; sw r25, r26 }
    f0b0:	[0-9a-f]* 	{ move r5, r6 ; sw r25, r26 }
    f0b8:	[0-9a-f]* 	{ move r5, r6 ; shr r15, r16, r17 ; sw r25, r26 }
    f0c0:	[0-9a-f]* 	{ clz r5, r6 ; movei r15, 5 ; sw r25, r26 }
    f0c8:	[0-9a-f]* 	{ movei r15, 5 ; nor r5, r6, r7 ; sw r25, r26 }
    f0d0:	[0-9a-f]* 	{ movei r15, 5 ; slti_u r5, r6, 5 ; sw r25, r26 }
    f0d8:	[0-9a-f]* 	{ movei r5, 5 ; movei r15, 5 ; sw r25, r26 }
    f0e0:	[0-9a-f]* 	{ movei r5, 5 ; slte_u r15, r16, r17 ; sw r25, r26 }
    f0e8:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; nop ; sw r25, r26 }
    f0f0:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; slti_u r15, r16, 5 ; sw r25, r26 }
    f0f8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
    f100:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
    f108:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; rl r15, r16, r17 ; sw r25, r26 }
    f110:	[0-9a-f]* 	{ mulhha_ss r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    f118:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    f120:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; sw r25, r26 }
    f128:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 ; sw r25, r26 }
    f130:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
    f138:	[0-9a-f]* 	{ mulll_ss r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    f140:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
    f148:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
    f150:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; ill ; sw r25, r26 }
    f158:	[0-9a-f]* 	{ mullla_ss r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    f160:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f168:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f170:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; movei r15, 5 ; sw r25, r26 }
    f178:	[0-9a-f]* 	{ mvnz r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    f180:	[0-9a-f]* 	{ mvz r5, r6, r7 ; nop ; sw r25, r26 }
    f188:	[0-9a-f]* 	{ mvz r5, r6, r7 ; slti_u r15, r16, 5 ; sw r25, r26 }
    f190:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
    f198:	[0-9a-f]* 	{ mz r15, r16, r17 ; s2a r5, r6, r7 ; sw r25, r26 }
    f1a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; sw r25, r26 }
    f1a8:	[0-9a-f]* 	{ mz r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
    f1b0:	[0-9a-f]* 	{ mz r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    f1b8:	[0-9a-f]* 	{ nop ; move r5, r6 ; sw r25, r26 }
    f1c0:	[0-9a-f]* 	{ nop ; or r5, r6, r7 ; sw r25, r26 }
    f1c8:	[0-9a-f]* 	{ nop ; shli r15, r16, 5 ; sw r25, r26 }
    f1d0:	[0-9a-f]* 	{ nop ; sra r15, r16, r17 ; sw r25, r26 }
    f1d8:	[0-9a-f]* 	{ ctz r5, r6 ; nor r15, r16, r17 ; sw r25, r26 }
    f1e0:	[0-9a-f]* 	{ nor r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
    f1e8:	[0-9a-f]* 	{ nor r15, r16, r17 ; sne r5, r6, r7 ; sw r25, r26 }
    f1f0:	[0-9a-f]* 	{ nor r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
    f1f8:	[0-9a-f]* 	{ nor r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    f200:	[0-9a-f]* 	{ or r15, r16, r17 ; movei r5, 5 ; sw r25, r26 }
    f208:	[0-9a-f]* 	{ or r15, r16, r17 ; s1a r5, r6, r7 ; sw r25, r26 }
    f210:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    f218:	[0-9a-f]* 	{ or r5, r6, r7 ; rl r15, r16, r17 ; sw r25, r26 }
    f220:	[0-9a-f]* 	{ or r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
    f228:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
    f230:	[0-9a-f]* 	{ ori r15, r16, 5 ; shl r5, r6, r7 ; sw r25, r26 }
    f238:	[0-9a-f]* 	{ ori r5, r6, 5 ; add r15, r16, r17 ; sw r25, r26 }
    f240:	[0-9a-f]* 	{ ori r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
    f248:	[0-9a-f]* 	{ pcnt r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
    f250:	[0-9a-f]* 	{ pcnt r5, r6 ; shl r15, r16, r17 ; sw r25, r26 }
    f258:	[0-9a-f]* 	{ bitx r5, r6 ; rl r15, r16, r17 ; sw r25, r26 }
    f260:	[0-9a-f]* 	{ rl r15, r16, r17 ; mz r5, r6, r7 ; sw r25, r26 }
    f268:	[0-9a-f]* 	{ rl r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f270:	[0-9a-f]* 	{ rl r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f278:	[0-9a-f]* 	{ rl r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f280:	[0-9a-f]* 	{ rli r15, r16, 5 ; info 19 ; sw r25, r26 }
    f288:	[0-9a-f]* 	{ pcnt r5, r6 ; rli r15, r16, 5 ; sw r25, r26 }
    f290:	[0-9a-f]* 	{ rli r15, r16, 5 ; srai r5, r6, 5 ; sw r25, r26 }
    f298:	[0-9a-f]* 	{ rli r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
    f2a0:	[0-9a-f]* 	{ rli r5, r6, 5 ; sne r15, r16, r17 ; sw r25, r26 }
    f2a8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    f2b0:	[0-9a-f]* 	{ s1a r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
    f2b8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s1a r15, r16, r17 ; sw r25, r26 }
    f2c0:	[0-9a-f]* 	{ s1a r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    f2c8:	[0-9a-f]* 	{ s1a r5, r6, r7 ; sw r25, r26 }
    f2d0:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
    f2d8:	[0-9a-f]* 	{ s2a r15, r16, r17 ; shr r5, r6, r7 ; sw r25, r26 }
    f2e0:	[0-9a-f]* 	{ s2a r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
    f2e8:	[0-9a-f]* 	{ s2a r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    f2f0:	[0-9a-f]* 	{ bitx r5, r6 ; s3a r15, r16, r17 ; sw r25, r26 }
    f2f8:	[0-9a-f]* 	{ s3a r15, r16, r17 ; mz r5, r6, r7 ; sw r25, r26 }
    f300:	[0-9a-f]* 	{ s3a r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f308:	[0-9a-f]* 	{ s3a r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f310:	[0-9a-f]* 	{ s3a r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f318:	[0-9a-f]* 	{ seq r15, r16, r17 ; info 19 ; sw r25, r26 }
    f320:	[0-9a-f]* 	{ pcnt r5, r6 ; seq r15, r16, r17 ; sw r25, r26 }
    f328:	[0-9a-f]* 	{ seq r15, r16, r17 ; srai r5, r6, 5 ; sw r25, r26 }
    f330:	[0-9a-f]* 	{ seq r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    f338:	[0-9a-f]* 	{ seq r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    f340:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
    f348:	[0-9a-f]* 	{ seqi r15, r16, 5 ; s3a r5, r6, r7 ; sw r25, r26 }
    f350:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; seqi r15, r16, 5 ; sw r25, r26 }
    f358:	[0-9a-f]* 	{ seqi r5, r6, 5 ; s1a r15, r16, r17 ; sw r25, r26 }
    f360:	[0-9a-f]* 	{ seqi r5, r6, 5 ; sw r25, r26 }
    f368:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    f370:	[0-9a-f]* 	{ shl r15, r16, r17 ; shr r5, r6, r7 ; sw r25, r26 }
    f378:	[0-9a-f]* 	{ shl r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
    f380:	[0-9a-f]* 	{ shl r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    f388:	[0-9a-f]* 	{ bitx r5, r6 ; shli r15, r16, 5 ; sw r25, r26 }
    f390:	[0-9a-f]* 	{ shli r15, r16, 5 ; mz r5, r6, r7 ; sw r25, r26 }
    f398:	[0-9a-f]* 	{ shli r15, r16, 5 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f3a0:	[0-9a-f]* 	{ shli r5, r6, 5 ; mnz r15, r16, r17 ; sw r25, r26 }
    f3a8:	[0-9a-f]* 	{ shli r5, r6, 5 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f3b0:	[0-9a-f]* 	{ shr r15, r16, r17 ; info 19 ; sw r25, r26 }
    f3b8:	[0-9a-f]* 	{ pcnt r5, r6 ; shr r15, r16, r17 ; sw r25, r26 }
    f3c0:	[0-9a-f]* 	{ shr r15, r16, r17 ; srai r5, r6, 5 ; sw r25, r26 }
    f3c8:	[0-9a-f]* 	{ shr r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    f3d0:	[0-9a-f]* 	{ shr r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    f3d8:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
    f3e0:	[0-9a-f]* 	{ shri r15, r16, 5 ; s3a r5, r6, r7 ; sw r25, r26 }
    f3e8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; sw r25, r26 }
    f3f0:	[0-9a-f]* 	{ shri r5, r6, 5 ; s1a r15, r16, r17 ; sw r25, r26 }
    f3f8:	[0-9a-f]* 	{ shri r5, r6, 5 ; sw r25, r26 }
    f400:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
    f408:	[0-9a-f]* 	{ slt r15, r16, r17 ; shr r5, r6, r7 ; sw r25, r26 }
    f410:	[0-9a-f]* 	{ slt r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
    f418:	[0-9a-f]* 	{ slt r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
    f420:	[0-9a-f]* 	{ bitx r5, r6 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f428:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; mz r5, r6, r7 ; sw r25, r26 }
    f430:	[0-9a-f]* 	{ slt_u r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f438:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f440:	[0-9a-f]* 	{ slt_u r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f448:	[0-9a-f]* 	{ slte r15, r16, r17 ; info 19 ; sw r25, r26 }
    f450:	[0-9a-f]* 	{ pcnt r5, r6 ; slte r15, r16, r17 ; sw r25, r26 }
    f458:	[0-9a-f]* 	{ slte r15, r16, r17 ; srai r5, r6, 5 ; sw r25, r26 }
    f460:	[0-9a-f]* 	{ slte r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    f468:	[0-9a-f]* 	{ slte r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    f470:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
    f478:	[0-9a-f]* 	{ slte_u r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
    f480:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slte_u r15, r16, r17 ; sw r25, r26 }
    f488:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    f490:	[0-9a-f]* 	{ slte_u r5, r6, r7 ; sw r25, r26 }
    f498:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
    f4a0:	[0-9a-f]* 	{ slti r15, r16, 5 ; shr r5, r6, r7 ; sw r25, r26 }
    f4a8:	[0-9a-f]* 	{ slti r5, r6, 5 ; and r15, r16, r17 ; sw r25, r26 }
    f4b0:	[0-9a-f]* 	{ slti r5, r6, 5 ; shl r15, r16, r17 ; sw r25, r26 }
    f4b8:	[0-9a-f]* 	{ bitx r5, r6 ; slti_u r15, r16, 5 ; sw r25, r26 }
    f4c0:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; mz r5, r6, r7 ; sw r25, r26 }
    f4c8:	[0-9a-f]* 	{ slti_u r15, r16, 5 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f4d0:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; mnz r15, r16, r17 ; sw r25, r26 }
    f4d8:	[0-9a-f]* 	{ slti_u r5, r6, 5 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f4e0:	[0-9a-f]* 	{ sne r15, r16, r17 ; info 19 ; sw r25, r26 }
    f4e8:	[0-9a-f]* 	{ pcnt r5, r6 ; sne r15, r16, r17 ; sw r25, r26 }
    f4f0:	[0-9a-f]* 	{ sne r15, r16, r17 ; srai r5, r6, 5 ; sw r25, r26 }
    f4f8:	[0-9a-f]* 	{ sne r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
    f500:	[0-9a-f]* 	{ sne r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
    f508:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
    f510:	[0-9a-f]* 	{ sra r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
    f518:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sra r15, r16, r17 ; sw r25, r26 }
    f520:	[0-9a-f]* 	{ sra r5, r6, r7 ; s1a r15, r16, r17 ; sw r25, r26 }
    f528:	[0-9a-f]* 	{ sra r5, r6, r7 ; sw r25, r26 }
    f530:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; srai r15, r16, 5 ; sw r25, r26 }
    f538:	[0-9a-f]* 	{ srai r15, r16, 5 ; shr r5, r6, r7 ; sw r25, r26 }
    f540:	[0-9a-f]* 	{ srai r5, r6, 5 ; and r15, r16, r17 ; sw r25, r26 }
    f548:	[0-9a-f]* 	{ srai r5, r6, 5 ; shl r15, r16, r17 ; sw r25, r26 }
    f550:	[0-9a-f]* 	{ bitx r5, r6 ; sub r15, r16, r17 ; sw r25, r26 }
    f558:	[0-9a-f]* 	{ sub r15, r16, r17 ; mz r5, r6, r7 ; sw r25, r26 }
    f560:	[0-9a-f]* 	{ sub r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
    f568:	[0-9a-f]* 	{ sub r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    f570:	[0-9a-f]* 	{ sub r5, r6, r7 ; slt_u r15, r16, r17 ; sw r25, r26 }
    f578:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; movei r15, 5 ; sw r25, r26 }
    f580:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte_u r15, r16, r17 ; sw r25, r26 }
    f588:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nop ; sw r25, r26 }
    f590:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti_u r15, r16, 5 ; sw r25, r26 }
    f598:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; sw r25, r26 }
    f5a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; sw r25, r26 }
    f5a8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rl r15, r16, r17 ; sw r25, r26 }
    f5b0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; sw r25, r26 }
    f5b8:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
    f5c0:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl r5, r6, r7 ; sw r25, r26 }
    f5c8:	[0-9a-f]* 	{ xor r5, r6, r7 ; add r15, r16, r17 ; sw r25, r26 }
    f5d0:	[0-9a-f]* 	{ xor r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
    f5d8:	[0-9a-f]* 	{ addbs_u r5, r6, r7 ; swadd r15, r16, 5 }
    f5e0:	[0-9a-f]* 	{ crc32_8 r5, r6, r7 ; swadd r15, r16, 5 }
    f5e8:	[0-9a-f]* 	{ mnzb r5, r6, r7 ; swadd r15, r16, 5 }
    f5f0:	[0-9a-f]* 	{ mulhla_uu r5, r6, r7 ; swadd r15, r16, 5 }
    f5f8:	[0-9a-f]* 	{ packhs r5, r6, r7 ; swadd r15, r16, 5 }
    f600:	[0-9a-f]* 	{ shl r5, r6, r7 ; swadd r15, r16, 5 }
    f608:	[0-9a-f]* 	{ slteh r5, r6, r7 ; swadd r15, r16, 5 }
    f610:	[0-9a-f]* 	{ subb r5, r6, r7 ; swadd r15, r16, 5 }
    f618:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; add r15, r16, r17 ; prefetch r25 }
    f620:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; addih r15, r16, 5 }
    f628:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
    f630:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ill ; lb_u r25, r26 }
    f638:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; inthb r15, r16, r17 }
    f640:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; movei r15, 5 ; lb r25, r26 }
    f648:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte_u r15, r16, r17 ; lb r25, r26 }
    f650:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; lb_u r25, r26 }
    f658:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
    f660:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; movei r15, 5 ; lh r25, r26 }
    f668:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte_u r15, r16, r17 ; lh r25, r26 }
    f670:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; lh_u r25, r26 }
    f678:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti r15, r16, 5 ; lh_u r25, r26 }
    f680:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; lw r25, r26 }
    f688:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; lw r25, r26 }
    f690:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; minh r15, r16, r17 }
    f698:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; move r15, r16 ; lw r25, r26 }
    f6a0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; lb_u r25, r26 }
    f6a8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; nop }
    f6b0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; or r15, r16, r17 }
    f6b8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; prefetch r25 }
    f6c0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shr r15, r16, r17 ; prefetch r25 }
    f6c8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; rl r15, r16, r17 ; prefetch r25 }
    f6d0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s1a r15, r16, r17 ; prefetch r25 }
    f6d8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s3a r15, r16, r17 ; prefetch r25 }
    f6e0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; ori r15, r16, 5 ; sb r25, r26 }
    f6e8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; srai r15, r16, 5 ; sb r25, r26 }
    f6f0:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; seqi r15, r16, 5 ; lh_u r25, r26 }
    f6f8:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
    f700:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
    f708:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shlh r15, r16, r17 }
    f710:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; shr r15, r16, r17 ; sh r25, r26 }
    f718:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
    f720:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
    f728:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slteb_u r15, r16, r17 }
    f730:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; slti_u r15, r16, 5 ; prefetch r25 }
    f738:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sneh r15, r16, r17 }
    f740:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; srai r15, r16, 5 ; sh r25, r26 }
    f748:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; sw r15, r16 }
    f750:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; s3a r15, r16, r17 ; sw r25, r26 }
    f758:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; tns r15, r16 }
    f760:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
    f768:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addli.sn r15, r16, 4660 }
    f770:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; sw r25, r26 }
    f778:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ill ; lh_u r25, r26 }
    f780:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; intlb r15, r16, r17 }
    f788:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nop ; lb r25, r26 }
    f790:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti_u r15, r16, 5 ; lb r25, r26 }
    f798:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    f7a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
    f7a8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nop ; lh r25, r26 }
    f7b0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti_u r15, r16, 5 ; lh r25, r26 }
    f7b8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; lh_u r25, r26 }
    f7c0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
    f7c8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; lw r25, r26 }
    f7d0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti r15, r16, 5 ; lw r25, r26 }
    f7d8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; minih r15, r16, 5 }
    f7e0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; move r15, r16 ; sb r25, r26 }
    f7e8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; lh_u r25, r26 }
    f7f0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
    f7f8:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; ori r15, r16, 5 ; lb_u r25, r26 }
    f800:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; info 19 ; prefetch r25 }
    f808:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; prefetch r25 }
    f810:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rl r15, r16, r17 ; sh r25, r26 }
    f818:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s1a r15, r16, r17 ; sh r25, r26 }
    f820:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; s3a r15, r16, r17 ; sh r25, r26 }
    f828:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
    f830:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; sb r25, r26 }
    f838:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
    f840:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; sh r25, r26 }
    f848:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
    f850:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; lb_u r25, r26 }
    f858:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; shr r15, r16, r17 }
    f860:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; prefetch r25 }
    f868:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
    f870:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slteh_u r15, r16, r17 }
    f878:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; slti_u r15, r16, 5 ; sh r25, r26 }
    f880:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; sra r15, r16, r17 ; lb_u r25, r26 }
    f888:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; srai r15, r16, 5 }
    f890:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; sw r25, r26 }
    f898:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; seqi r15, r16, 5 ; sw r25, r26 }
    f8a0:	[0-9a-f]* 	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; lb r25, r26 }
    f8a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; add r15, r16, r17 }
    f8b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
    f8b8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; auli r15, r16, 4660 }
    f8c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ill ; prefetch r25 }
    f8c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; inv r15 }
    f8d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; lb r25, r26 }
    f8d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; lb r25, r26 }
    f8e0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; lb_u r25, r26 }
    f8e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; srai r15, r16, 5 ; lb_u r25, r26 }
    f8f0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; lh r25, r26 }
    f8f8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; lh r25, r26 }
    f900:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; lh_u r25, r26 }
    f908:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; srai r15, r16, 5 ; lh_u r25, r26 }
    f910:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; lw r25, r26 }
    f918:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sne r15, r16, r17 ; lw r25, r26 }
    f920:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; lb r25, r26 }
    f928:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; sw r25, r26 }
    f930:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    f938:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; lh_u r25, r26 }
    f940:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; lh_u r25, r26 }
    f948:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; move r15, r16 ; prefetch r25 }
    f950:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; prefetch r25 }
    f958:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; rl r15, r16, r17 }
    f960:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s1a r15, r16, r17 }
    f968:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s3a r15, r16, r17 }
    f970:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; s2a r15, r16, r17 ; sb r25, r26 }
    f978:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sbadd r15, r16, 5 }
    f980:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
    f988:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; sh r25, r26 }
    f990:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; srai r15, r16, 5 ; sh r25, r26 }
    f998:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; lh_u r25, r26 }
    f9a0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shrh r15, r16, r17 }
    f9a8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slt r15, r16, r17 ; sh r25, r26 }
    f9b0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; prefetch r25 }
    f9b8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slth_u r15, r16, r17 }
    f9c0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; slti_u r15, r16, 5 }
    f9c8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; lh_u r25, r26 }
    f9d0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; sraih r15, r16, 5 }
    f9d8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; andi r15, r16, 5 ; sw r25, r26 }
    f9e0:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; sw r25, r26 }
    f9e8:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; xor r15, r16, r17 ; lh r25, r26 }
    f9f0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; addbs_u r15, r16, r17 }
    f9f8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; lh r25, r26 }
    fa00:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; finv r15 }
    fa08:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; sh r25, r26 }
    fa10:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; jalr r15 }
    fa18:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rl r15, r16, r17 ; lb r25, r26 }
    fa20:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; lb r25, r26 }
    fa28:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
    fa30:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lb_u r25, r26 }
    fa38:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rl r15, r16, r17 ; lh r25, r26 }
    fa40:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; lh r25, r26 }
    fa48:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rli r15, r16, 5 ; lh_u r25, r26 }
    fa50:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lh_u r25, r26 }
    fa58:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; lw r25, r26 }
    fa60:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; srai r15, r16, 5 ; lw r25, r26 }
    fa68:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; lh r25, r26 }
    fa70:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; movei r15, 5 ; lb r25, r26 }
    fa78:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
    fa80:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
    fa88:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
    fa90:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
    fa98:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; prefetch r25 }
    faa0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
    faa8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; s2a r15, r16, r17 ; lb_u r25, r26 }
    fab0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; sb r25, r26 }
    fab8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; seq r15, r16, r17 ; sb r25, r26 }
    fac0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; seq r15, r16, r17 ; lb_u r25, r26 }
    fac8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; seqi r15, r16, 5 }
    fad0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
    fad8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; sh r25, r26 }
    fae0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; prefetch r25 }
    fae8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; lb_u r25, r26 }
    faf0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slt r15, r16, r17 }
    faf8:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slte r15, r16, r17 ; sh r25, r26 }
    fb00:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
    fb08:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sltib_u r15, r16, 5 }
    fb10:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sra r15, r16, r17 ; prefetch r25 }
    fb18:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; lb_u r25, r26 }
    fb20:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; ill ; sw r25, r26 }
    fb28:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; sw r25, r26 }
    fb30:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lw r25, r26 }
    fb38:	[0-9a-f]* 	{ and r5, r6, r7 ; tns r15, r16 }
    fb40:	[0-9a-f]* 	{ maxh r5, r6, r7 ; tns r15, r16 }
    fb48:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; tns r15, r16 }
    fb50:	[0-9a-f]* 	{ mz r5, r6, r7 ; tns r15, r16 }
    fb58:	[0-9a-f]* 	{ sadb_u r5, r6, r7 ; tns r15, r16 }
    fb60:	[0-9a-f]* 	{ shrih r5, r6, 5 ; tns r15, r16 }
    fb68:	[0-9a-f]* 	{ sneb r5, r6, r7 ; tns r15, r16 }
    fb70:	[0-9a-f]* 	{ add r5, r6, r7 ; wh64 r15 }
    fb78:	[0-9a-f]* 	{ clz r5, r6 ; wh64 r15 }
    fb80:	[0-9a-f]* 	{ mm r5, r6, r7, 5, 7 ; wh64 r15 }
    fb88:	[0-9a-f]* 	{ mulhla_su r5, r6, r7 ; wh64 r15 }
    fb90:	[0-9a-f]* 	{ packbs_u r5, r6, r7 ; wh64 r15 }
    fb98:	[0-9a-f]* 	{ seqib r5, r6, 5 ; wh64 r15 }
    fba0:	[0-9a-f]* 	{ slteb r5, r6, r7 ; wh64 r15 }
    fba8:	[0-9a-f]* 	{ sraih r5, r6, 5 ; wh64 r15 }
    fbb0:	[0-9a-f]* 	{ xor r15, r16, r17 ; add r5, r6, r7 ; sh r25, r26 }
    fbb8:	[0-9a-f]* 	{ xor r15, r16, r17 ; addli.sn r5, r6, 4660 }
    fbc0:	[0-9a-f]* 	{ xor r15, r16, r17 ; andi r5, r6, 5 ; sb r25, r26 }
    fbc8:	[0-9a-f]* 	{ bytex r5, r6 ; xor r15, r16, r17 ; lh_u r25, r26 }
    fbd0:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; lb_u r25, r26 }
    fbd8:	[0-9a-f]* 	{ xor r15, r16, r17 ; info 19 ; lb r25, r26 }
    fbe0:	[0-9a-f]* 	{ bytex r5, r6 ; xor r15, r16, r17 ; lb r25, r26 }
    fbe8:	[0-9a-f]* 	{ xor r15, r16, r17 ; nop ; lb r25, r26 }
    fbf0:	[0-9a-f]* 	{ xor r15, r16, r17 ; slti r5, r6, 5 ; lb r25, r26 }
    fbf8:	[0-9a-f]* 	{ xor r15, r16, r17 ; lb_u r25, r26 }
    fc00:	[0-9a-f]* 	{ xor r15, r16, r17 ; ori r5, r6, 5 ; lb_u r25, r26 }
    fc08:	[0-9a-f]* 	{ xor r15, r16, r17 ; sra r5, r6, r7 ; lb_u r25, r26 }
    fc10:	[0-9a-f]* 	{ xor r15, r16, r17 ; move r5, r6 ; lh r25, r26 }
    fc18:	[0-9a-f]* 	{ xor r15, r16, r17 ; rli r5, r6, 5 ; lh r25, r26 }
    fc20:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; lh r25, r26 }
    fc28:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    fc30:	[0-9a-f]* 	{ xor r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
    fc38:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lh_u r25, r26 }
    fc40:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    fc48:	[0-9a-f]* 	{ xor r15, r16, r17 ; shl r5, r6, r7 ; lw r25, r26 }
    fc50:	[0-9a-f]* 	{ xor r15, r16, r17 ; maxb_u r5, r6, r7 }
    fc58:	[0-9a-f]* 	{ xor r15, r16, r17 ; mnzh r5, r6, r7 }
    fc60:	[0-9a-f]* 	{ xor r15, r16, r17 ; movei r5, 5 }
    fc68:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
    fc70:	[0-9a-f]* 	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    fc78:	[0-9a-f]* 	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
    fc80:	[0-9a-f]* 	{ mulll_uu r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    fc88:	[0-9a-f]* 	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
    fc90:	[0-9a-f]* 	{ mvz r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
    fc98:	[0-9a-f]* 	{ xor r15, r16, r17 ; nop ; lb_u r25, r26 }
    fca0:	[0-9a-f]* 	{ xor r15, r16, r17 ; or r5, r6, r7 ; lb_u r25, r26 }
    fca8:	[0-9a-f]* 	{ xor r15, r16, r17 ; packhb r5, r6, r7 }
    fcb0:	[0-9a-f]* 	{ ctz r5, r6 ; xor r15, r16, r17 ; prefetch r25 }
    fcb8:	[0-9a-f]* 	{ xor r15, r16, r17 ; or r5, r6, r7 ; prefetch r25 }
    fcc0:	[0-9a-f]* 	{ xor r15, r16, r17 ; sne r5, r6, r7 ; prefetch r25 }
    fcc8:	[0-9a-f]* 	{ xor r15, r16, r17 ; rli r5, r6, 5 ; lb r25, r26 }
    fcd0:	[0-9a-f]* 	{ xor r15, r16, r17 ; s2a r5, r6, r7 ; lb r25, r26 }
    fcd8:	[0-9a-f]* 	{ sadab_u r5, r6, r7 ; xor r15, r16, r17 }
    fce0:	[0-9a-f]* 	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
    fce8:	[0-9a-f]* 	{ xor r15, r16, r17 ; s3a r5, r6, r7 ; sb r25, r26 }
    fcf0:	[0-9a-f]* 	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; sb r25, r26 }
    fcf8:	[0-9a-f]* 	{ xor r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
    fd00:	[0-9a-f]* 	{ mulhh_ss r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
    fd08:	[0-9a-f]* 	{ xor r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
    fd10:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; xor r15, r16, r17 ; sh r25, r26 }
    fd18:	[0-9a-f]* 	{ xor r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
    fd20:	[0-9a-f]* 	{ xor r15, r16, r17 ; shri r5, r6, 5 ; lb r25, r26 }
    fd28:	[0-9a-f]* 	{ xor r15, r16, r17 ; slt r5, r6, r7 ; sw r25, r26 }
    fd30:	[0-9a-f]* 	{ xor r15, r16, r17 ; slte r5, r6, r7 ; sb r25, r26 }
    fd38:	[0-9a-f]* 	{ xor r15, r16, r17 ; slti r5, r6, 5 ; lb r25, r26 }
    fd40:	[0-9a-f]* 	{ xor r15, r16, r17 ; sltib r5, r6, 5 }
    fd48:	[0-9a-f]* 	{ xor r15, r16, r17 ; sra r5, r6, r7 ; lw r25, r26 }
    fd50:	[0-9a-f]* 	{ xor r15, r16, r17 ; sub r5, r6, r7 ; lb r25, r26 }
    fd58:	[0-9a-f]* 	{ bytex r5, r6 ; xor r15, r16, r17 ; sw r25, r26 }
    fd60:	[0-9a-f]* 	{ xor r15, r16, r17 ; nop ; sw r25, r26 }
    fd68:	[0-9a-f]* 	{ xor r15, r16, r17 ; slti r5, r6, 5 ; sw r25, r26 }
    fd70:	[0-9a-f]* 	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; sw r25, r26 }
    fd78:	[0-9a-f]* 	{ tblidxb2 r5, r6 ; xor r15, r16, r17 ; sw r25, r26 }
    fd80:	[0-9a-f]* 	{ xor r15, r16, r17 ; xor r5, r6, r7 ; sw r25, r26 }
    fd88:	[0-9a-f]* 	{ xor r5, r6, r7 ; addi r15, r16, 5 ; lh_u r25, r26 }
    fd90:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
    fd98:	[0-9a-f]* 	{ xor r5, r6, r7 ; lw r25, r26 }
    fda0:	[0-9a-f]* 	{ xor r5, r6, r7 ; info 19 ; lh_u r25, r26 }
    fda8:	[0-9a-f]* 	{ xor r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
    fdb0:	[0-9a-f]* 	{ xor r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
    fdb8:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
    fdc0:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
    fdc8:	[0-9a-f]* 	{ xor r5, r6, r7 ; addi r15, r16, 5 ; lh r25, r26 }
    fdd0:	[0-9a-f]* 	{ xor r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
    fdd8:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
    fde0:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    fde8:	[0-9a-f]* 	{ xor r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
    fdf0:	[0-9a-f]* 	{ xor r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
    fdf8:	[0-9a-f]* 	{ xor r5, r6, r7 ; lwadd_na r15, r16, 5 }
    fe00:	[0-9a-f]* 	{ xor r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
    fe08:	[0-9a-f]* 	{ xor r5, r6, r7 ; movei r15, 5 ; sb r25, r26 }
    fe10:	[0-9a-f]* 	{ xor r5, r6, r7 ; nop ; lb_u r25, r26 }
    fe18:	[0-9a-f]* 	{ xor r5, r6, r7 ; or r15, r16, r17 ; lb_u r25, r26 }
    fe20:	[0-9a-f]* 	{ xor r5, r6, r7 ; packhb r15, r16, r17 }
    fe28:	[0-9a-f]* 	{ xor r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
    fe30:	[0-9a-f]* 	{ xor r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
    fe38:	[0-9a-f]* 	{ xor r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
    fe40:	[0-9a-f]* 	{ xor r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
    fe48:	[0-9a-f]* 	{ xor r5, r6, r7 ; info 19 ; sb r25, r26 }
    fe50:	[0-9a-f]* 	{ xor r5, r6, r7 ; slt r15, r16, r17 ; sb r25, r26 }
    fe58:	[0-9a-f]* 	{ xor r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
    fe60:	[0-9a-f]* 	{ xor r5, r6, r7 ; and r15, r16, r17 ; sh r25, r26 }
    fe68:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
    fe70:	[0-9a-f]* 	{ xor r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
    fe78:	[0-9a-f]* 	{ xor r5, r6, r7 ; shlih r15, r16, 5 }
    fe80:	[0-9a-f]* 	{ xor r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
    fe88:	[0-9a-f]* 	{ xor r5, r6, r7 ; slt_u r15, r16, r17 ; prefetch r25 }
    fe90:	[0-9a-f]* 	{ xor r5, r6, r7 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
    fe98:	[0-9a-f]* 	{ xor r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
    fea0:	[0-9a-f]* 	{ xor r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
    fea8:	[0-9a-f]* 	{ xor r5, r6, r7 ; srah r15, r16, r17 }
    feb0:	[0-9a-f]* 	{ xor r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
    feb8:	[0-9a-f]* 	{ xor r5, r6, r7 ; nop ; sw r25, r26 }
    fec0:	[0-9a-f]* 	{ xor r5, r6, r7 ; slti_u r15, r16, 5 ; sw r25, r26 }
    fec8:	[0-9a-f]* 	{ xor r5, r6, r7 ; xori r15, r16, 5 }
    fed0:	[0-9a-f]* 	{ bytex r5, r6 ; xori r15, r16, 5 }
    fed8:	[0-9a-f]* 	{ xori r15, r16, 5 ; minih r5, r6, 5 }
    fee0:	[0-9a-f]* 	{ mulhla_ss r5, r6, r7 ; xori r15, r16, 5 }
    fee8:	[0-9a-f]* 	{ xori r15, r16, 5 ; ori r5, r6, 5 }
    fef0:	[0-9a-f]* 	{ xori r15, r16, 5 ; seqi r5, r6, 5 }
    fef8:	[0-9a-f]* 	{ xori r15, r16, 5 ; slte_u r5, r6, r7 }
    ff00:	[0-9a-f]* 	{ xori r15, r16, 5 ; sraib r5, r6, 5 }
    ff08:	[0-9a-f]* 	{ xori r5, r6, 5 ; addib r15, r16, 5 }
    ff10:	[0-9a-f]* 	{ xori r5, r6, 5 ; inv r15 }
    ff18:	[0-9a-f]* 	{ xori r5, r6, 5 ; maxh r15, r16, r17 }
    ff20:	[0-9a-f]* 	{ xori r5, r6, 5 ; mzh r15, r16, r17 }
    ff28:	[0-9a-f]* 	{ xori r5, r6, 5 ; seqh r15, r16, r17 }
    ff30:	[0-9a-f]* 	{ xori r5, r6, 5 ; sltb r15, r16, r17 }
    ff38:	[0-9a-f]* 	{ xori r5, r6, 5 ; srab r15, r16, r17 }
