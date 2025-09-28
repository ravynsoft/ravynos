#as:
#objdump: -dr
#name: regression

.*\.o:     file format elf32-epiphany


Disassembly of section \.text:

00000000 \<START\>:
   0:	000b 0802 	mov r0,0x8000
   4:	0056      	lsl r0,r0,0x2
   6:	c0c3      	mov r6,0x6
   8:	6063      	mov r3,0x3
   a:	ff1f fc0a 	add r63,r63,r6
   e:	fd1f fc0a 	add r63,r63,r2
  12:	dc0b e072 	mov r62,0x7e0
  16:	ff3f ff8a 	sub r63,r63,r62
  1a:	0300      	beq 20 \<BRANCH1\>
  1c:	0023      	mov r0,0x1
  1e:	0fe2      	trap 0x3

00000020 \<BRANCH1\>:
  20:	0900      	beq 32 \<BRANCH2\>
  22:	0023      	mov r0,0x1
  24:	0fe2      	trap 0x3
  26:	0023      	mov r0,0x1
  28:	0fe2      	trap 0x3
  2a:	0023      	mov r0,0x1
  2c:	0fe2      	trap 0x3
  2e:	0023      	mov r0,0x1
  30:	0fe2      	trap 0x3

00000032 \<BRANCH2\>:
  32:	1c10      	bne 6a \<FAIL_BRANCH\>

00000034 \<BRANCH3\>:
  34:	1b60      	bgt 6a \<FAIL_BRANCH\>

00000036 \<BRANCH4\>:
  36:	0370      	bgte 3c \<BRANCH5\>
  38:	0023      	mov r0,0x1
  3a:	0fe2      	trap 0x3

0000003c \<BRANCH5\>:
  3c:	0390      	blte 42 \<BRANCH6\>
  3e:	0023      	mov r0,0x1
  40:	0fe2      	trap 0x3

00000042 \<BRANCH6\>:
  42:	1480      	blt 6a \<FAIL_BRANCH\>

00000044 \<BRANCH8\>:
  44:	efe8 0000 	b 222 \<LONGJUMP\>
  48:	0023      	mov r0,0x1
  4a:	0fe2      	trap 0x3

0000004c \<RETURN\>:
  4c:	edf8 0000 	bl 226 \<FUNCTION\>
  50:	e00b e002 	mov r63,0x0
			50: R_EPIPHANY_LOW	\.text\+0x5c
  54:	1d4f 1c02 	jr r63
  58:	0023      	mov r0,0x1
  5a:	0fe2      	trap 0x3

0000005c \<JARLAB\>:
  5c:	e00b e002 	mov r63,0x0
			5c: R_EPIPHANY_LOW	\.text\+0x226
  60:	1d5f 1c02 	jalr r63
  64:	05e0      	b 6e \<NEXT\>
  66:	0023      	mov r0,0x1
  68:	0fe2      	trap 0x3

0000006a \<FAIL_BRANCH\>:
  6a:	0023      	mov r0,0x1
  6c:	0fe2      	trap 0x3

0000006e \<NEXT\>:
  6e:	8014      	strb r4,\[r0\]
  70:	e00c e000 	ldrb r63,\[r0,\+0x0\]
  74:	fe3f fc0a 	sub r63,r63,r4
  78:	0300      	beq 7e \<STOREB\>
  7a:	0023      	mov r0,0x1
  7c:	0fe2      	trap 0x3

0000007e \<STOREB\>:
  7e:	a39c 0001 	strb r5,\[r0,\+0xf\]
  82:	e38c e001 	ldrb r63,\[r0,\+0xf\]
  86:	febf fc0a 	sub r63,r63,r5
  8a:	0300      	beq 90 \<STORES\>
  8c:	0023      	mov r0,0x1
  8e:	0fe2      	trap 0x3

00000090 \<STORES\>:
  90:	8034      	strh r4,\[r0\]
  92:	e02c e000 	ldrh r63,\[r0,\+0x0\]
  96:	fe3f fc0a 	sub r63,r63,r4
  9a:	0300      	beq a0 \<STORES2\>
  9c:	0023      	mov r0,0x1
  9e:	0fe2      	trap 0x3

000000a0 \<STORES2\>:
  a0:	a33c 0001 	strh r5,\[r0,\+0xe\]
  a4:	e32c e001 	ldrh r63,\[r0,\+0xe\]
  a8:	febf fc0a 	sub r63,r63,r5
  ac:	0300      	beq b2 \<STORE\>
  ae:	0023      	mov r0,0x1
  b0:	0fe2      	trap 0x3

000000b2 \<STORE\>:
  b2:	8054      	str r4,\[r0\]
  b4:	e04c e000 	ldr r63,\[r0,\+0x0\]
  b8:	fe3f fc0a 	sub r63,r63,r4
  bc:	0300      	beq c2 \<STORE2\>
  be:	0023      	mov r0,0x1
  c0:	0fe2      	trap 0x3

000000c2 \<STORE2\>:
  c2:	a25c 0001 	str r5,\[r0,\+0xc\]
  c6:	e24c e001 	ldr r63,\[r0,\+0xc\]
  ca:	febf fc0a 	sub r63,r63,r5
  ce:	0300      	beq d4 \<STOREBI\>
  d0:	0023      	mov r0,0x1
  d2:	0fe2      	trap 0x3

000000d4 \<STOREBI\>:
  d4:	8211      	strb r4,\[r0,r4\]
  d6:	e209 e000 	ldrb r63,\[r0,\+r4\]
  da:	fe3f fc0a 	sub r63,r63,r4
  de:	0300      	beq e4 \<STORESI\>
  e0:	0023      	mov r0,0x1
  e2:	0fe2      	trap 0x3

000000e4 \<STORESI\>:
  e4:	a231      	strh r5,\[r0,r4\]
  e6:	e229 e000 	ldrh r63,\[r0,\+r4\]
  ea:	febf fc0a 	sub r63,r63,r5
  ee:	0300      	beq f4 \<STOREI\>
  f0:	0023      	mov r0,0x1
  f2:	0fe2      	trap 0x3

000000f4 \<STOREI\>:
  f4:	c251      	str r6,\[r0,r4\]
  f6:	e249 e000 	ldr r63,\[r0,\+r4\]
  fa:	ff3f fc0a 	sub r63,r63,r6
  fe:	0300      	beq 104 \<PMB\>
 100:	0023      	mov r0,0x1
 102:	0fe2      	trap 0x3

00000104 \<PMB\>:
 104:	8215      	strb r4,\[r0\],r4
 106:	023b 0000 	sub r0,r0,4
 10a:	e20d e000 	ldrb r63,\[r0\],\+r4
 10e:	023b 0000 	sub r0,r0,4
 112:	fe3f fc0a 	sub r63,r63,r4
 116:	0300      	beq 11c \<PMS\>
 118:	0023      	mov r0,0x1
 11a:	0fe2      	trap 0x3

0000011c \<PMS\>:
 11c:	a235      	strh r5,\[r0\],r4
 11e:	023b 0000 	sub r0,r0,4
 122:	e22d e000 	ldrh r63,\[r0\],\+r4
 126:	febf fc0a 	sub r63,r63,r5
 12a:	0300      	beq 130 \<PM\>
 12c:	0023      	mov r0,0x1
 12e:	0fe2      	trap 0x3

00000130 \<PM\>:
 130:	023b 0000 	sub r0,r0,4
 134:	c255      	str r6,\[r0\],r4
 136:	023b 0000 	sub r0,r0,4
 13a:	e24d e000 	ldr r63,\[r0\],\+r4
 13e:	023b 0000 	sub r0,r0,4
 142:	ff3f fc0a 	sub r63,r63,r6
 146:	0300      	beq 14c \<MOVLAB\>
 148:	0023      	mov r0,0x1
 14a:	0fe2      	trap 0x3

0000014c \<MOVLAB\>:
 14c:	ffeb e002 	mov r63,0xff
 150:	3fe3      	mov r1,0xff
 152:	fcbf fc0a 	sub r63,r63,r1
 156:	0300      	beq 15c \<ADDLAB\>
 158:	0023      	mov r0,0x1
 15a:	0fe2      	trap 0x3

0000015c \<ADDLAB\>:
 15c:	e99b e000 	add r63,r2,3
 160:	febb fc00 	sub r63,r63,5
 164:	0300      	beq 16a \<SUBLAB\>
 166:	0023      	mov r0,0x1
 168:	0fe2      	trap 0x3

0000016a \<SUBLAB\>:
 16a:	e8bb e000 	sub r63,r2,1
 16e:	fcbb fc00 	sub r63,r63,1
 172:	0300      	beq 178 \<LSRLAB\>
 174:	0023      	mov r0,0x1
 176:	0fe2      	trap 0x3

00000178 \<LSRLAB\>:
 178:	f84f e006 	lsr r63,r6,0x2
 17c:	fcbb fc00 	sub r63,r63,1
 180:	0300      	beq 186 \<LSLLAB\>
 182:	0023      	mov r0,0x1
 184:	0fe2      	trap 0x3

00000186 \<LSLLAB\>:
 186:	ec5f e006 	lsl r63,r3,0x2
 18a:	fe3b fc01 	sub r63,r63,12
 18e:	0300      	beq 194 \<LSRILAB\>
 190:	0023      	mov r0,0x1
 192:	0fe2      	trap 0x3

00000194 \<LSRILAB\>:
 194:	f94f e00a 	lsr r63,r6,r2
 198:	fcbb fc00 	sub r63,r63,1
 19c:	0300      	beq 1a2 \<LSLILAB\>
 19e:	0023      	mov r0,0x1
 1a0:	0fe2      	trap 0x3

000001a2 \<LSLILAB\>:
 1a2:	ed2f e00a 	lsl r63,r3,r2
 1a6:	fe3b fc01 	sub r63,r63,12
 1aa:	0300      	beq 1b0 \<ORRLAB\>
 1ac:	0023      	mov r0,0x1
 1ae:	0fe2      	trap 0x3

000001b0 \<ORRLAB\>:
 1b0:	ae7a      	orr r5,r3,r4
 1b2:	f7bb e000 	sub r63,r5,7
 1b6:	0300      	beq 1bc \<ANDLAB\>
 1b8:	0023      	mov r0,0x1
 1ba:	0fe2      	trap 0x3

000001bc \<ANDLAB\>:
 1bc:	ae5a      	and r5,r3,r4
 1be:	f43b e000 	sub r63,r5,0
 1c2:	0300      	beq 1c8 \<EORLAB\>
 1c4:	0023      	mov r0,0x1
 1c6:	0fe2      	trap 0x3

000001c8 \<EORLAB\>:
 1c8:	ad0a      	eor r5,r3,r2
 1ca:	f4bb e000 	sub r63,r5,1
 1ce:	0300      	beq 1d4 \<ADD3LAB\>
 1d0:	0023      	mov r0,0x1
 1d2:	0fe2      	trap 0x3

000001d4 \<ADD3LAB\>:
 1d4:	e99f e00a 	add r63,r2,r3
 1d8:	febb fc00 	sub r63,r63,5
 1dc:	0300      	beq 1e2 \<SUB3LAB\>
 1de:	0023      	mov r0,0x1
 1e0:	0fe2      	trap 0x3

000001e2 \<SUB3LAB\>:
 1e2:	fa3f e00a 	sub r63,r6,r4
 1e6:	fd3b fc00 	sub r63,r63,2
 1ea:	0300      	beq 1f0 \<MOVRLAB\>
 1ec:	0023      	mov r0,0x1
 1ee:	0fe2      	trap 0x3

000001f0 \<MOVRLAB\>:
 1f0:	e8ef e002 	mov r63,r2
 1f4:	fd3b fc00 	sub r63,r63,2
 1f8:	0b00      	beq 20e \<NOPLAB\>
 1fa:	0023      	mov r0,0x1
 1fc:	0fe2      	trap 0x3

000001fe \<MOVTFLAB\>:
 1fe:	0502      	movts status,r0
 200:	e51f e002 	movfs r63,status
 204:	fc3f fc0a 	sub r63,r63,r0
 208:	fb00      	beq 1fe \<MOVTFLAB\>
 20a:	0023      	mov r0,0x1
 20c:	0fe2      	trap 0x3

0000020e \<NOPLAB\>:
 20e:	01a2      	nop
 210:	01a2      	nop
 212:	01a2      	nop
 214:	01a2      	nop

00000216 \<PASSED\>:
 216:	0003      	mov r0,0x0
 218:	0fe2      	trap 0x3
 21a:	01b2      	idle

0000021c \<FAILED\>:
 21c:	0023      	mov r0,0x1
 21e:	0fe2      	trap 0x3
 220:	01b2      	idle

00000222 \<LONGJUMP\>:
 222:	15e8 ffff 	b 4c \<RETURN\>

00000226 \<FUNCTION\>:
 226:	194f 0402 	rts
