#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  branch_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
       0:	3400      	bgtu!		0x0
       2:	35ff      	bgtu!		0x0
       4:	35fe      	bgtu!		0x0
       6:	35fd      	bgtu!		0x0
       8:	93ff 0bf8 	bgtu		0x0
       c:	93ff 0bf4 	bgtu		0x0
      10:	35f8      	bgtu!		0x0
      12:	93ff 0bee 	bgtu		0x0
      16:	93ff 0bea 	bgtu		0x0
      1a:	93ff 0be7 	bgtul		0x0
	...
     21e:	93ff 09e2 	bgtu		0x0
     222:	93ff 09de 	bgtu		0x0
     226:	93ff 09da 	bgtu		0x0
     22a:	93ff 09d6 	bgtu		0x0
     22e:	93ff 09d2 	bgtu		0x0
     232:	93ff 09ce 	bgtu		0x0
     236:	93ff 09ca 	bgtu		0x0
     23a:	93ff 09c6 	bgtu		0x0
     23e:	93ff 09c2 	bgtu		0x0
     242:	93ff 09bf 	bgtul		0x0
     246:	9000 0a46 	bgtu		0x48c
     24a:	9000 0a42 	bgtu		0x48c
     24e:	9000 0a3e 	bgtu		0x48c
     252:	9000 0a3a 	bgtu		0x48c
     256:	9000 0a36 	bgtu		0x48c
     25a:	9000 0a32 	bgtu		0x48c
     25e:	9000 0a2e 	bgtu		0x48c
     262:	9000 0a2a 	bgtu		0x48c
     266:	9000 0a26 	bgtu		0x48c
     26a:	9000 0a23 	bgtul		0x48c
	...
     46e:	340f      	bgtu!		0x48c
     470:	340e      	bgtu!		0x48c
     472:	340d      	bgtu!		0x48c
     474:	340c      	bgtu!		0x48c
     476:	9000 0816 	bgtu		0x48c
     47a:	9000 0812 	bgtu		0x48c
     47e:	3407      	bgtu!		0x48c
     480:	9000 080c 	bgtu		0x48c
     484:	9000 0808 	bgtu		0x48c
     488:	9000 0805 	bgtul		0x48c
	...
     68c:	0000      	nop!
     68e:	3500      	bgtu!		0x48e
     690:	93ff 09fe 	bgtu		0x48e
     694:	9000 0a06 	bgtu		0x89a
     698:	9000 0a02 	bgtu		0x89a
	...
     89c:	0000      	nop!
     89e:	3600      	bleu!		0x89e
     8a0:	37ff      	bleu!		0x89e
     8a2:	37fe      	bleu!		0x89e
     8a4:	37fd      	bleu!		0x89e
     8a6:	93ff 0ff8 	bleu		0x89e
     8aa:	93ff 0ff4 	bleu		0x89e
     8ae:	37f8      	bleu!		0x89e
     8b0:	93ff 0fee 	bleu		0x89e
     8b4:	93ff 0fea 	bleu		0x89e
     8b8:	93ff 0fe7 	bleul		0x89e
	...
     abc:	93ff 0de2 	bleu		0x89e
     ac0:	93ff 0dde 	bleu		0x89e
     ac4:	93ff 0dda 	bleu		0x89e
     ac8:	93ff 0dd6 	bleu		0x89e
     acc:	93ff 0dd2 	bleu		0x89e
     ad0:	93ff 0dce 	bleu		0x89e
     ad4:	93ff 0dca 	bleu		0x89e
     ad8:	93ff 0dc6 	bleu		0x89e
     adc:	93ff 0dc2 	bleu		0x89e
     ae0:	93ff 0dbf 	bleul		0x89e
     ae4:	9000 0e46 	bleu		0xd2a
     ae8:	9000 0e42 	bleu		0xd2a
     aec:	9000 0e3e 	bleu		0xd2a
     af0:	9000 0e3a 	bleu		0xd2a
     af4:	9000 0e36 	bleu		0xd2a
     af8:	9000 0e32 	bleu		0xd2a
     afc:	9000 0e2e 	bleu		0xd2a
     b00:	9000 0e2a 	bleu		0xd2a
     b04:	9000 0e26 	bleu		0xd2a
     b08:	9000 0e23 	bleul		0xd2a
	...
     d0c:	360f      	bleu!		0xd2a
     d0e:	360e      	bleu!		0xd2a
     d10:	360d      	bleu!		0xd2a
     d12:	360c      	bleu!		0xd2a
     d14:	9000 0c16 	bleu		0xd2a
     d18:	9000 0c12 	bleu		0xd2a
     d1c:	3607      	bleu!		0xd2a
     d1e:	9000 0c0c 	bleu		0xd2a
     d22:	9000 0c08 	bleu		0xd2a
     d26:	9000 0c05 	bleul		0xd2a
	...
     f2a:	0000      	nop!
     f2c:	3700      	bleu!		0xd2c
     f2e:	93ff 0dfe 	bleu		0xd2c
     f32:	9000 0e06 	bleu		0x1138
     f36:	9000 0e02 	bleu		0x1138
	...
    113a:	0000      	nop!
    113c:	3800      	beq!		0x113c
    113e:	39ff      	beq!		0x113c
    1140:	39fe      	beq!		0x113c
    1142:	39fd      	beq!		0x113c
    1144:	93ff 13f8 	beq		0x113c
    1148:	93ff 13f4 	beq		0x113c
    114c:	39f8      	beq!		0x113c
    114e:	93ff 13ee 	beq		0x113c
    1152:	93ff 13ea 	beq		0x113c
    1156:	93ff 13e7 	beql		0x113c
	...
    135a:	93ff 11e2 	beq		0x113c
    135e:	93ff 11de 	beq		0x113c
    1362:	93ff 11da 	beq		0x113c
    1366:	93ff 11d6 	beq		0x113c
    136a:	93ff 11d2 	beq		0x113c
    136e:	93ff 11ce 	beq		0x113c
    1372:	93ff 11ca 	beq		0x113c
    1376:	93ff 11c6 	beq		0x113c
    137a:	93ff 11c2 	beq		0x113c
    137e:	93ff 11bf 	beql		0x113c
    1382:	9000 1246 	beq		0x15c8
    1386:	9000 1242 	beq		0x15c8
    138a:	9000 123e 	beq		0x15c8
    138e:	9000 123a 	beq		0x15c8
    1392:	9000 1236 	beq		0x15c8
    1396:	9000 1232 	beq		0x15c8
    139a:	9000 122e 	beq		0x15c8
    139e:	9000 122a 	beq		0x15c8
    13a2:	9000 1226 	beq		0x15c8
    13a6:	9000 1223 	beql		0x15c8
	...
    15aa:	380f      	beq!		0x15c8
    15ac:	380e      	beq!		0x15c8
    15ae:	380d      	beq!		0x15c8
    15b0:	380c      	beq!		0x15c8
    15b2:	9000 1016 	beq		0x15c8
    15b6:	9000 1012 	beq		0x15c8
    15ba:	3807      	beq!		0x15c8
    15bc:	9000 100c 	beq		0x15c8
    15c0:	9000 1008 	beq		0x15c8
    15c4:	9000 1005 	beql		0x15c8
	...
    17c8:	0000      	nop!
    17ca:	3900      	beq!		0x15ca
    17cc:	93ff 11fe 	beq		0x15ca
    17d0:	9000 1206 	beq		0x19d6
    17d4:	9000 1202 	beq		0x19d6
	...
    19d8:	0000      	nop!
    19da:	3a00      	bne!		0x19da
    19dc:	3bff      	bne!		0x19da
    19de:	3bfe      	bne!		0x19da
    19e0:	3bfd      	bne!		0x19da
    19e2:	93ff 17f8 	bne		0x19da
    19e6:	93ff 17f4 	bne		0x19da
    19ea:	3bf8      	bne!		0x19da
    19ec:	93ff 17ee 	bne		0x19da
    19f0:	93ff 17ea 	bne		0x19da
    19f4:	93ff 17e7 	bnel		0x19da
	...
    1bf8:	93ff 15e2 	bne		0x19da
    1bfc:	93ff 15de 	bne		0x19da
    1c00:	93ff 15da 	bne		0x19da
    1c04:	93ff 15d6 	bne		0x19da
    1c08:	93ff 15d2 	bne		0x19da
    1c0c:	93ff 15ce 	bne		0x19da
    1c10:	93ff 15ca 	bne		0x19da
    1c14:	93ff 15c6 	bne		0x19da
    1c18:	93ff 15c2 	bne		0x19da
    1c1c:	93ff 15bf 	bnel		0x19da
    1c20:	9000 1646 	bne		0x1e66
    1c24:	9000 1642 	bne		0x1e66
    1c28:	9000 163e 	bne		0x1e66
    1c2c:	9000 163a 	bne		0x1e66
    1c30:	9000 1636 	bne		0x1e66
    1c34:	9000 1632 	bne		0x1e66
    1c38:	9000 162e 	bne		0x1e66
    1c3c:	9000 162a 	bne		0x1e66
    1c40:	9000 1626 	bne		0x1e66
    1c44:	9000 1623 	bnel		0x1e66
	...
    1e48:	3a0f      	bne!		0x1e66
    1e4a:	3a0e      	bne!		0x1e66
    1e4c:	3a0d      	bne!		0x1e66
    1e4e:	3a0c      	bne!		0x1e66
    1e50:	9000 1416 	bne		0x1e66
    1e54:	9000 1412 	bne		0x1e66
    1e58:	3a07      	bne!		0x1e66
    1e5a:	9000 140c 	bne		0x1e66
    1e5e:	9000 1408 	bne		0x1e66
    1e62:	9000 1405 	bnel		0x1e66
	...
    2066:	0000      	nop!
    2068:	3b00      	bne!		0x1e68
    206a:	93ff 15fe 	bne		0x1e68
    206e:	9000 1606 	bne		0x2274
    2072:	9000 1602 	bne		0x2274
	...
    2276:	0000      	nop!
    2278:	3c00      	bgt!		0x2278
    227a:	3dff      	bgt!		0x2278
    227c:	3dfe      	bgt!		0x2278
    227e:	3dfd      	bgt!		0x2278
    2280:	93ff 1bf8 	bgt		0x2278
    2284:	93ff 1bf4 	bgt		0x2278
    2288:	3df8      	bgt!		0x2278
    228a:	93ff 1bee 	bgt		0x2278
    228e:	93ff 1bea 	bgt		0x2278
    2292:	93ff 1be7 	bgtl		0x2278
	...
    2496:	93ff 19e2 	bgt		0x2278
    249a:	93ff 19de 	bgt		0x2278
    249e:	93ff 19da 	bgt		0x2278
    24a2:	93ff 19d6 	bgt		0x2278
    24a6:	93ff 19d2 	bgt		0x2278
    24aa:	93ff 19ce 	bgt		0x2278
    24ae:	93ff 19ca 	bgt		0x2278
    24b2:	93ff 19c6 	bgt		0x2278
    24b6:	93ff 19c2 	bgt		0x2278
    24ba:	93ff 19bf 	bgtl		0x2278
    24be:	9000 1a46 	bgt		0x2704
    24c2:	9000 1a42 	bgt		0x2704
    24c6:	9000 1a3e 	bgt		0x2704
    24ca:	9000 1a3a 	bgt		0x2704
    24ce:	9000 1a36 	bgt		0x2704
    24d2:	9000 1a32 	bgt		0x2704
    24d6:	9000 1a2e 	bgt		0x2704
    24da:	9000 1a2a 	bgt		0x2704
    24de:	9000 1a26 	bgt		0x2704
    24e2:	9000 1a23 	bgtl		0x2704
	...
    26e6:	3c0f      	bgt!		0x2704
    26e8:	3c0e      	bgt!		0x2704
    26ea:	3c0d      	bgt!		0x2704
    26ec:	3c0c      	bgt!		0x2704
    26ee:	9000 1816 	bgt		0x2704
    26f2:	9000 1812 	bgt		0x2704
    26f6:	3c07      	bgt!		0x2704
    26f8:	9000 180c 	bgt		0x2704
    26fc:	9000 1808 	bgt		0x2704
    2700:	9000 1805 	bgtl		0x2704
	...
    2904:	0000      	nop!
    2906:	3d00      	bgt!		0x2706
    2908:	93ff 19fe 	bgt		0x2706
    290c:	9000 1a06 	bgt		0x2b12
    2910:	9000 1a02 	bgt		0x2b12
	...
    2b14:	0000      	nop!
    2b16:	3e00      	ble!		0x2b16
    2b18:	3fff      	ble!		0x2b16
    2b1a:	3ffe      	ble!		0x2b16
    2b1c:	3ffd      	ble!		0x2b16
    2b1e:	93ff 1ff8 	ble		0x2b16
    2b22:	93ff 1ff4 	ble		0x2b16
    2b26:	3ff8      	ble!		0x2b16
    2b28:	93ff 1fee 	ble		0x2b16
    2b2c:	93ff 1fea 	ble		0x2b16
    2b30:	93ff 1fe7 	blel		0x2b16
	...
    2d34:	93ff 1de2 	ble		0x2b16
    2d38:	93ff 1dde 	ble		0x2b16
    2d3c:	93ff 1dda 	ble		0x2b16
    2d40:	93ff 1dd6 	ble		0x2b16
    2d44:	93ff 1dd2 	ble		0x2b16
    2d48:	93ff 1dce 	ble		0x2b16
    2d4c:	93ff 1dca 	ble		0x2b16
    2d50:	93ff 1dc6 	ble		0x2b16
    2d54:	93ff 1dc2 	ble		0x2b16
    2d58:	93ff 1dbf 	blel		0x2b16
    2d5c:	9000 1e46 	ble		0x2fa2
    2d60:	9000 1e42 	ble		0x2fa2
    2d64:	9000 1e3e 	ble		0x2fa2
    2d68:	9000 1e3a 	ble		0x2fa2
    2d6c:	9000 1e36 	ble		0x2fa2
    2d70:	9000 1e32 	ble		0x2fa2
    2d74:	9000 1e2e 	ble		0x2fa2
    2d78:	9000 1e2a 	ble		0x2fa2
    2d7c:	9000 1e26 	ble		0x2fa2
    2d80:	9000 1e23 	blel		0x2fa2
	...
    2f84:	3e0f      	ble!		0x2fa2
    2f86:	3e0e      	ble!		0x2fa2
    2f88:	3e0d      	ble!		0x2fa2
    2f8a:	3e0c      	ble!		0x2fa2
    2f8c:	9000 1c16 	ble		0x2fa2
    2f90:	9000 1c12 	ble		0x2fa2
    2f94:	3e07      	ble!		0x2fa2
    2f96:	9000 1c0c 	ble		0x2fa2
    2f9a:	9000 1c08 	ble		0x2fa2
    2f9e:	9000 1c05 	blel		0x2fa2
	...
    31a2:	0000      	nop!
    31a4:	3f00      	ble!		0x2fa4
    31a6:	93ff 1dfe 	ble		0x2fa4
    31aa:	9000 1e06 	ble		0x33b0
    31ae:	9000 1e02 	ble		0x33b0
	...
    33b2:	0000      	nop!
    33b4:	3200      	bcnz!		0x33b4
    33b6:	33ff      	bcnz!		0x33b4
    33b8:	33fe      	bcnz!		0x33b4
    33ba:	33fd      	bcnz!		0x33b4
    33bc:	93ff 3bf8 	bcnz		0x33b4
    33c0:	93ff 3bf4 	bcnz		0x33b4
    33c4:	33f8      	bcnz!		0x33b4
    33c6:	93ff 3bee 	bcnz		0x33b4
    33ca:	93ff 3bea 	bcnz		0x33b4
    33ce:	93ff 3be7 	bcnzl		0x33b4
	...
    35d2:	93ff 39e2 	bcnz		0x33b4
    35d6:	93ff 39de 	bcnz		0x33b4
    35da:	93ff 39da 	bcnz		0x33b4
    35de:	93ff 39d6 	bcnz		0x33b4
    35e2:	93ff 39d2 	bcnz		0x33b4
    35e6:	93ff 39ce 	bcnz		0x33b4
    35ea:	93ff 39ca 	bcnz		0x33b4
    35ee:	93ff 39c6 	bcnz		0x33b4
    35f2:	93ff 39c2 	bcnz		0x33b4
    35f6:	93ff 39bf 	bcnzl		0x33b4
    35fa:	9000 3a46 	bcnz		0x3840
    35fe:	9000 3a42 	bcnz		0x3840
    3602:	9000 3a3e 	bcnz		0x3840
    3606:	9000 3a3a 	bcnz		0x3840
    360a:	9000 3a36 	bcnz		0x3840
    360e:	9000 3a32 	bcnz		0x3840
    3612:	9000 3a2e 	bcnz		0x3840
    3616:	9000 3a2a 	bcnz		0x3840
    361a:	9000 3a26 	bcnz		0x3840
    361e:	9000 3a23 	bcnzl		0x3840
	...
    3822:	320f      	bcnz!		0x3840
    3824:	320e      	bcnz!		0x3840
    3826:	320d      	bcnz!		0x3840
    3828:	320c      	bcnz!		0x3840
    382a:	9000 3816 	bcnz		0x3840
    382e:	9000 3812 	bcnz		0x3840
    3832:	3207      	bcnz!		0x3840
    3834:	9000 380c 	bcnz		0x3840
    3838:	9000 3808 	bcnz		0x3840
    383c:	9000 3805 	bcnzl		0x3840
	...
    3a40:	0000      	nop!
    3a42:	3300      	bcnz!		0x3842
    3a44:	93ff 39fe 	bcnz		0x3842
    3a48:	9000 3a06 	bcnz		0x3c4e
    3a4c:	9000 3a02 	bcnz		0x3c4e
	...
    3c50:	0000      	nop!
    3c52:	3000      	b!		0x3c52
    3c54:	31ff      	b!		0x3c52
    3c56:	31fe      	b!		0x3c52
    3c58:	31fd      	b!		0x3c52
    3c5a:	93ff 3ff8 	b		0x3c52
    3c5e:	93ff 3ff4 	b		0x3c52
    3c62:	31f8      	b!		0x3c52
    3c64:	93ff 3fee 	b		0x3c52
    3c68:	93ff 3fea 	b		0x3c52
    3c6c:	93ff 3fe7 	bl		0x3c52
	...
    3e70:	93ff 3de2 	b		0x3c52
    3e74:	93ff 3dde 	b		0x3c52
    3e78:	93ff 3dda 	b		0x3c52
    3e7c:	93ff 3dd6 	b		0x3c52
    3e80:	93ff 3dd2 	b		0x3c52
    3e84:	93ff 3dce 	b		0x3c52
    3e88:	93ff 3dca 	b		0x3c52
    3e8c:	93ff 3dc6 	b		0x3c52
    3e90:	93ff 3dc2 	b		0x3c52
    3e94:	93ff 3dbf 	bl		0x3c52
    3e98:	9000 3e46 	b		0x40de
    3e9c:	9000 3e42 	b		0x40de
    3ea0:	9000 3e3e 	b		0x40de
    3ea4:	9000 3e3a 	b		0x40de
    3ea8:	9000 3e36 	b		0x40de
    3eac:	9000 3e32 	b		0x40de
    3eb0:	9000 3e2e 	b		0x40de
    3eb4:	9000 3e2a 	b		0x40de
    3eb8:	9000 3e26 	b		0x40de
    3ebc:	9000 3e23 	bl		0x40de
	...
    40c0:	300f      	b!		0x40de
    40c2:	300e      	b!		0x40de
    40c4:	300d      	b!		0x40de
    40c6:	300c      	b!		0x40de
    40c8:	9000 3c16 	b		0x40de
    40cc:	9000 3c12 	b		0x40de
    40d0:	3007      	b!		0x40de
    40d2:	9000 3c0c 	b		0x40de
    40d6:	9000 3c08 	b		0x40de
    40da:	9000 3c05 	bl		0x40de
	...
    42de:	0000      	nop!
    42e0:	3100      	b!		0x40e0
    42e2:	93ff 3dfe 	b		0x40e0
    42e6:	9000 3e06 	b		0x44ec
    42ea:	9000 3e02 	b		0x44ec
	...
    44ee:	0000      	nop!
    44f0:	0080      	br!		r0
    44f2:	008f      	br!		r15
    44f4:	0080      	br!		r0
    44f6:	0080      	br!		r0
    44f8:	0080      	br!		r0
    44fa:	0080      	br!		r0
    44fc:	0080      	br!		r0
    44fe:	0080      	br!		r0
    4500:	0080      	br!		r0
    4502:	0080      	br!		r0
    4504:	0090      	br!		r16
    4506:	009f      	br!		r31
    4508:	00a0      	brl!		r0
    450a:	00af      	brl!		r15
    450c:	00a0      	brl!		r0
    450e:	00a0      	brl!		r0
    4510:	00a0      	brl!		r0
    4512:	00a0      	brl!		r0
    4514:	00a0      	brl!		r0
    4516:	00a0      	brl!		r0
    4518:	00a0      	brl!		r0
    451a:	00a0      	brl!		r0
    451c:	00b0      	brl!		r16
    451e:	00bf      	brl!		r31
    4520:	8000 3c4c 	bcmpeq		r0,  r15 ,0x4520
    4524:	83ef 43cc 	bcmpeq		r15,  r16 ,0x4520
    4528:	83ef 7f4c 	bcmpeq		r15,  r31 ,0x4520
    452c:	83f0 7ecc 	bcmpeq		r16,  r31 ,0x4520
	...
    4730:	440f      	cmp!		r0, r15
    4732:	93ff 11ee 	beq		0x4520
    4736:	45f0      	cmp!		r15, r16
    4738:	93ff 11e8 	beq		0x4520
    473c:	45ff      	cmp!		r15, r31
    473e:	93ff 11e2 	beq		0x4520
    4742:	461f      	cmp!		r16, r31
    4744:	93ff 11dc 	beq		0x4520
    4748:	440f      	cmp!		r0, r15
    474a:	9000 1226 	beq		0x4970
    474e:	45f0      	cmp!		r15, r16
    4750:	9000 1220 	beq		0x4970
    4754:	45ff      	cmp!		r15, r31
    4756:	9000 121a 	beq		0x4970
    475a:	461f      	cmp!		r16, r31
    475c:	9000 1214 	beq		0x4970
	...
    4960:	8000 3e4c 	bcmpeq		r0,  r15 ,0x4970
    4964:	800f 41cc 	bcmpeq		r15,  r16 ,0x4970
    4968:	800f 7d4c 	bcmpeq		r15,  r31 ,0x4970
    496c:	8010 7ccc 	bcmpeq		r16,  r31 ,0x4970
	...
    4b70:	0000      	nop!
    4b72:	8200 3c4c 	bcmpeq		r0,  r15 ,0x4972
    4b76:	460f      	cmp!		r16, r15
    4b78:	93ff 11fa 	beq		0x4972
    4b7c:	440f      	cmp!		r0, r15
    4b7e:	9000 1208 	beq		0x4d86
    4b82:	460f      	cmp!		r16, r15
    4b84:	9000 1202 	beq		0x4d86
	...
    4d88:	8000 3c4e 	bcmpne		r0,  r15 ,0x4d88
    4d8c:	83ef 43ce 	bcmpne		r15,  r16 ,0x4d88
    4d90:	83ef 7f4e 	bcmpne		r15,  r31 ,0x4d88
    4d94:	83f0 7ece 	bcmpne		r16,  r31 ,0x4d88
	...
    4f98:	440f      	cmp!		r0, r15
    4f9a:	93ff 15ee 	bne		0x4d88
    4f9e:	45f0      	cmp!		r15, r16
    4fa0:	93ff 15e8 	bne		0x4d88
    4fa4:	45ff      	cmp!		r15, r31
    4fa6:	93ff 15e2 	bne		0x4d88
    4faa:	461f      	cmp!		r16, r31
    4fac:	93ff 15dc 	bne		0x4d88
    4fb0:	440f      	cmp!		r0, r15
    4fb2:	9000 1626 	bne		0x51d8
    4fb6:	45f0      	cmp!		r15, r16
    4fb8:	9000 1620 	bne		0x51d8
    4fbc:	45ff      	cmp!		r15, r31
    4fbe:	9000 161a 	bne		0x51d8
    4fc2:	461f      	cmp!		r16, r31
    4fc4:	9000 1614 	bne		0x51d8
	...
    51c8:	8000 3e4e 	bcmpne		r0,  r15 ,0x51d8
    51cc:	800f 41ce 	bcmpne		r15,  r16 ,0x51d8
    51d0:	800f 7d4e 	bcmpne		r15,  r31 ,0x51d8
    51d4:	8010 7cce 	bcmpne		r16,  r31 ,0x51d8
	...
    53d8:	0000      	nop!
    53da:	8200 3c4e 	bcmpne		r0,  r15 ,0x51da
    53de:	460f      	cmp!		r16, r15
    53e0:	93ff 15fa 	bne		0x51da
    53e4:	440f      	cmp!		r0, r15
    53e6:	9000 1608 	bne		0x55ee
    53ea:	460f      	cmp!		r16, r15
    53ec:	9000 1602 	bne		0x55ee
	...
    55f0:	8000 004c 	bcmpeqz		r0, 0x55f0
    55f4:	83ef 03cc 	bcmpeqz		r15, 0x55f0
    55f8:	83f0 034c 	bcmpeqz		r16, 0x55f0
    55fc:	83ff 02cc 	bcmpeqz		r31, 0x55f0
	...
    5800:	6000      	cmpi!		r0, 0
    5802:	93ff 11ee 	beq		0x55f0
    5806:	61e0      	cmpi!		r15, 0
    5808:	93ff 11e8 	beq		0x55f0
    580c:	6200      	cmpi!		r16, 0
    580e:	93ff 11e2 	beq		0x55f0
    5812:	63e0      	cmpi!		r31, 0
    5814:	93ff 11dc 	beq		0x55f0
    5818:	6000      	cmpi!		r0, 0
    581a:	9000 1226 	beq		0x5a40
    581e:	61e0      	cmpi!		r15, 0
    5820:	9000 1220 	beq		0x5a40
    5824:	6200      	cmpi!		r16, 0
    5826:	9000 121a 	beq		0x5a40
    582a:	63e0      	cmpi!		r31, 0
    582c:	9000 1214 	beq		0x5a40
	...
    5a30:	8000 024c 	bcmpeqz		r0, 0x5a40
    5a34:	800f 01cc 	bcmpeqz		r15, 0x5a40
    5a38:	8010 014c 	bcmpeqz		r16, 0x5a40
    5a3c:	801f 00cc 	bcmpeqz		r31, 0x5a40
	...
    5c40:	0000      	nop!
    5c42:	8200 004c 	bcmpeqz		r0, 0x5a42
    5c46:	6200      	cmpi!		r16, 0
    5c48:	93ff 11fa 	beq		0x5a42
    5c4c:	6000      	cmpi!		r0, 0
    5c4e:	9000 1208 	beq		0x5e56
    5c52:	6200      	cmpi!		r16, 0
    5c54:	9000 1202 	beq		0x5e56
	...
    5e58:	8000 004e 	bcmpnez		r0, 0x5e58
    5e5c:	83ef 03ce 	bcmpnez		r15, 0x5e58
    5e60:	83f0 034e 	bcmpnez		r16, 0x5e58
    5e64:	83ff 02ce 	bcmpnez		r31, 0x5e58
	...
    6068:	6000      	cmpi!		r0, 0
    606a:	93ff 15ee 	bne		0x5e58
    606e:	61e0      	cmpi!		r15, 0
    6070:	93ff 15e8 	bne		0x5e58
    6074:	6200      	cmpi!		r16, 0
    6076:	93ff 15e2 	bne		0x5e58
    607a:	63e0      	cmpi!		r31, 0
    607c:	93ff 15dc 	bne		0x5e58
    6080:	6000      	cmpi!		r0, 0
    6082:	9000 1626 	bne		0x62a8
    6086:	61e0      	cmpi!		r15, 0
    6088:	9000 1620 	bne		0x62a8
    608c:	6200      	cmpi!		r16, 0
    608e:	9000 161a 	bne		0x62a8
    6092:	63e0      	cmpi!		r31, 0
    6094:	9000 1614 	bne		0x62a8
	...
    6298:	8000 024e 	bcmpnez		r0, 0x62a8
    629c:	800f 01ce 	bcmpnez		r15, 0x62a8
    62a0:	8010 014e 	bcmpnez		r16, 0x62a8
    62a4:	801f 00ce 	bcmpnez		r31, 0x62a8
	...
    64a8:	0000      	nop!
    64aa:	8200 004e 	bcmpnez		r0, 0x62aa
    64ae:	6200      	cmpi!		r16, 0
    64b0:	93ff 15fa 	bne		0x62aa
    64b4:	6000      	cmpi!		r0, 0
    64b6:	9000 1608 	bne		0x66be
    64ba:	6200      	cmpi!		r16, 0
    64bc:	9000 1602 	bne		0x66be
	...
#pass
