#source: ./r-bcc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <beq>:
       0:	20 32                         	beq\.b	32 <dsp8>
       2:	3a 86 13                      	beq\.w	1388 <dsp16>
       5:	1d                            	bne\.s	a <bne>
       6:	04 1a a1 07                   	bra\.a	7a120 <dsp24>

0000000a <bne>:
       a:	21 28                         	bne\.b	32 <dsp8>
       c:	3b 7c 13                      	bne\.w	1388 <dsp16>
       f:	15                            	beq\.s	14 <bgt>
      10:	04 10 a1 07                   	bra\.a	7a120 <dsp24>

00000014 <bgt>:
      14:	2a 1e                         	bgt\.b	32 <dsp8>
      16:	2b 05                         	ble\.b	1b <bgt\+0x7>
      18:	38 70 13                      	bra\.w	1388 <dsp16>
      1b:	2b 06                         	ble\.b	21 <bltu>
      1d:	04 03 a1 07                   	bra\.a	7a120 <dsp24>

00000021 <bltu>:
      21:	23 11                         	bnc\.b	32 <dsp8>
      23:	22 05                         	bc\.b	28 <bltu\+0x7>
      25:	38 63 13                      	bra\.w	1388 <dsp16>
      28:	22 06                         	bc\.b	2e <done>
      2a:	04 f6 a0 07                   	bra\.a	7a120 <dsp24>

0000002e <done>:
      2e:	03                            	nop
      2f:	00                            	brk
	\.\.\.

00000032 <dsp8>:
	\.\.\.

00001388 <dsp16>:
	\.\.\.
