#as:
#objdump: -dr
#name: sample
.*\.o:     file format elf32-epiphany

Disassembly of section \.text:

00000000 \<beq\>:
	\.\.\.

00000002 \<bne\>:
   2:	ff10      	bne 0 \<beq\>

00000004 \<bgtu\>:
   4:	fe20      	bgtu 0 \<beq\>

00000006 \<bgteu\>:
   6:	fd30      	bgteu 0 \<beq\>

00000008 \<blteu\>:
   8:	fc40      	blteu 0 \<beq\>

0000000a \<bltu\>:
   a:	fb50      	bltu 0 \<beq\>

0000000c \<bgt\>:
   c:	fa60      	bgt 0 \<beq\>

0000000e \<bgte\>:
   e:	f970      	bgte 0 \<beq\>

00000010 \<blt\>:
  10:	f880      	blt 0 \<beq\>

00000012 \<blte\>:
  12:	f790      	blte 0 \<beq\>

00000014 \<bbeq\>:
  14:	f6a0      	bbeq 0 \<beq\>

00000016 \<bbne\>:
  16:	f5b0      	bbne 0 \<beq\>

00000018 \<bblt\>:
  18:	f4c0      	bblt 0 \<beq\>

0000001a \<b\>:
  1a:	f3e0      	b 0 \<beq\>

0000001c \<bl\>:
  1c:	f2f0      	bl 0 \<beq\>

0000001e \<jr\>:
  1e:	0542      	jr r1
  20:	1d4f 0c02 	jr r31

00000024 \<jalr\>:
  24:	0552      	jalr r1
  26:	1d5f 0c02 	jalr r31

0000002a \<add\>:
  2a:	299a      	add r1,r2,r3
  2c:	051f 920a 	add r32,r33,r34
  30:	2993      	add r1,r2,3
  32:	681b 2002 	add fp,r2,16

00000036 \<sub\>:
  36:	29ba      	sub r1,r2,r3
  38:	053f 920a 	sub r32,r33,r34
  3c:	29b3      	sub r1,r2,3
  3e:	683b 2002 	sub fp,r2,16

00000042 \<asr\>:
  42:	29ea      	asr r1,r2,r3
  44:	056f 920a 	asr r32,r33,r34
  48:	286e      	asr r1,r2,0x3
  4a:	6a0f 200e 	asr fp,r2,0x10

0000004e \<lsr\>:
  4e:	29ca      	lsr r1,r2,r3
  50:	054f 920a 	lsr r32,r33,r34
  54:	2866      	lsr r1,r2,0x3
  56:	6a0f 2006 	lsr fp,r2,0x10

0000005a \<lsl\>:
  5a:	29aa      	lsl r1,r2,r3
  5c:	052f 920a 	lsl r32,r33,r34
  60:	2876      	lsl r1,r2,0x3
  62:	6a1f 2006 	lsl fp,r2,0x10

00000066 \<orr\>:
  66:	29fa      	orr r1,r2,r3
  68:	72ff 248a 	orr fp,r12,sp

0000006c \<and\>:
  6c:	29da      	and r1,r2,r3
  6e:	72df 248a 	and fp,r12,sp

00000072 \<eor\>:
  72:	298a      	eor r1,r2,r3
  74:	728f 248a 	eor fp,r12,sp
  78:	0584      	ldrb r0,\[r1,0x3\]
  7a:	478c 201f 	ldrb r10,\[r1,\+0xff\]
  7e:	0501      	ldrb r0,\[r1,r2\]
  80:	0589 0080 	ldrb r0,\[r1,\+fp\]
  84:	0d05      	ldrb r0,\[r3\],r2
  86:	528d 2480 	ldrb r10,\[r12\],\+sp
  8a:	05a4      	ldrh r0,\[r1,0x3\]
  8c:	47ac 201f 	ldrh r10,\[r1,\+0xff\]
  90:	0521      	ldrh r0,\[r1,r2\]
  92:	05a9 0080 	ldrh r0,\[r1,\+fp\]
  96:	0d25      	ldrh r0,\[r3\],r2
  98:	52ad 2480 	ldrh r10,\[r12\],\+sp
  9c:	05c4      	ldr r0,\[r1,0x3\]
  9e:	47cc 201f 	ldr r10,\[r1,\+0xff\]
  a2:	0541      	ldr r0,\[r1,r2\]
  a4:	05c9 0080 	ldr r0,\[r1,\+fp\]
  a8:	0d45      	ldr r0,\[r3\],r2
  aa:	52cd 2480 	ldr r10,\[r12\],\+sp
  ae:	05e4      	ldrd r0,\[r1,0x3\]
  b0:	47ec 201f 	ldrd r10,\[r1,\+0xff\]
  b4:	0561      	ldrd r0,\[r1,r2\]
  b6:	05e9 0080 	ldrd r0,\[r1,\+fp\]
  ba:	0d65      	ldrd r0,\[r3\],r2
  bc:	52ed 2480 	ldrd r10,\[r12\],\+sp
  c0:	0594      	strb r0,\[r1,0x3\]
  c2:	479c 201f 	strb r10,\[r1,\+0xff\]
  c6:	0511      	strb r0,\[r1,r2\]
  c8:	0599 0080 	strb r0,\[r1,\+fp\]
  cc:	0d15      	strb r0,\[r3\],r2
  ce:	529d 2480 	strb r10,\[r12\],\+sp
  d2:	05b4      	strh r0,\[r1,0x3\]
  d4:	47bc 201f 	strh r10,\[r1,\+0xff\]
  d8:	0531      	strh r0,\[r1,r2\]
  da:	05b9 0080 	strh r0,\[r1,\+fp\]
  de:	0d35      	strh r0,\[r3\],r2
  e0:	52bd 2480 	strh r10,\[r12\],\+sp
  e4:	05d4      	str r0,\[r1,0x3\]
  e6:	47dc 201f 	str r10,\[r1,\+0xff\]
  ea:	0551      	str r0,\[r1,r2\]
  ec:	05d9 0080 	str r0,\[r1,\+fp\]
  f0:	0d55      	str r0,\[r3\],r2
  f2:	52dd 2480 	str r10,\[r12\],\+sp
  f6:	05f4      	strd r0,\[r1,0x3\]
  f8:	47fc 201f 	strd r10,\[r1,\+0xff\]
  fc:	0571      	strd r0,\[r1,r2\]
  fe:	05f9 0080 	strd r0,\[r1,\+fp\]
 102:	0d75      	strd r0,\[r3\],r2
 104:	52fd 2480 	strd r10,\[r12\],\+sp

00000108 \<mov\>:
 108:	dfe3      	mov r6,0xff
 10a:	ffeb 6ff2 	mov r31,0xffff
 10e:	004b 0102 	mov r0,0x1002
 112:	2802      	moveq r1,r2
 114:	700f 2402 	moveq fp,r12
 118:	2812      	movne r1,r2
 11a:	701f 2402 	movne fp,r12
 11e:	2822      	movgtu r1,r2
 120:	702f 2402 	movgtu fp,r12
 124:	2832      	movgteu r1,r2
 126:	703f 2402 	movgteu fp,r12
 12a:	2842      	movlteu r1,r2
 12c:	704f 2402 	movlteu fp,r12
 130:	2852      	movltu r1,r2
 132:	705f 2402 	movltu fp,r12
 136:	2862      	movgt r1,r2
 138:	706f 2402 	movgt fp,r12
 13c:	2872      	movgte r1,r2
 13e:	707f 2402 	movgte fp,r12
 142:	2882      	movlt r1,r2
 144:	708f 2402 	movlt fp,r12
 148:	2892      	movlte r1,r2
 14a:	709f 2402 	movlte fp,r12
 14e:	28a2      	movbeq r1,r2
 150:	70af 2402 	movbeq fp,r12
 154:	28b2      	movbne r1,r2
 156:	70bf 2402 	movbne fp,r12
 15a:	28c2      	movblt r1,r2
 15c:	70cf 2402 	movblt fp,r12
 160:	28d2      	movblte r1,r2
 162:	70df 2402 	movblte fp,r12
 166:	28e2      	mov r1,r2
 168:	70ef 2402 	mov fp,r12

0000016c \<nop\>:
 16c:	01a2      	nop

0000016e \<idle\>:
 16e:	01b2      	idle

00000170 \<bkpt\>:
 170:	01c2      	bkpt

00000172 \<fadd\>:
 172:	2987      	fadd r1,r2,r3
 174:	728f 2487 	fadd fp,r12,sp

00000178 \<fsub\>:
 178:	2997      	fsub r1,r2,r3
 17a:	729f 2487 	fsub fp,r12,sp

0000017e \<fmul\>:
 17e:	29a7      	fmul r1,r2,r3
 180:	72af 2487 	fmul fp,r12,sp

00000184 \<fmadd\>:
 184:	29b7      	fmadd r1,r2,r3
 186:	72bf 2487 	fmadd fp,r12,sp

0000018a \<fmsub\>:
 18a:	29c7      	fmsub r1,r2,r3
 18c:	72cf 2487 	fmsub fp,r12,sp
 190:	2102      	movts config,r1
 192:	e50f 6002 	movts status,r31
 196:	251f 0402 	movfs r1,imask
 19a:	e91f 6002 	movfs r31,pc

0000019e \<trap\>:
 19e:	03e2      	trap 0x0
 1a0:	01d2      	rti
