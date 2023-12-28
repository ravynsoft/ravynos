#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <text_label>:
   0:	2020 0f80 0000 0000 	j	0
			4: R_ARC_32_ME	.text
   8:	20e0 0f80 0000 0000 	j	0
			c: R_ARC_32_ME	.text
  10:	20e0 0f80 0000 0000 	j	0
			14: R_ARC_32_ME	.text
  18:	20e0 0f81 0000 0000 	jeq	0
			1c: R_ARC_32_ME	.text
  20:	20e0 0f81 0000 0000 	jeq	0
			24: R_ARC_32_ME	.text
  28:	20e0 0f82 0000 0000 	jne	0
			2c: R_ARC_32_ME	.text
  30:	20e0 0f82 0000 0000 	jne	0
			34: R_ARC_32_ME	.text
  38:	20e0 0f83 0000 0000 	jp	0
			3c: R_ARC_32_ME	.text
  40:	20e0 0f83 0000 0000 	jp	0
			44: R_ARC_32_ME	.text
  48:	20e0 0f84 0000 0000 	jn	0
			4c: R_ARC_32_ME	.text
  50:	20e0 0f84 0000 0000 	jn	0
			54: R_ARC_32_ME	.text
  58:	20e0 0f85 0000 0000 	jc	0
			5c: R_ARC_32_ME	.text
  60:	20e0 0f85 0000 0000 	jc	0
			64: R_ARC_32_ME	.text
  68:	20e0 0f85 0000 0000 	jc	0
			6c: R_ARC_32_ME	.text
  70:	20e0 0f86 0000 0000 	jnc	0
			74: R_ARC_32_ME	.text
  78:	20e0 0f86 0000 0000 	jnc	0
			7c: R_ARC_32_ME	.text
  80:	20e0 0f86 0000 0000 	jnc	0
			84: R_ARC_32_ME	.text
  88:	20e0 0f87 0000 0000 	jv	0
			8c: R_ARC_32_ME	.text
  90:	20e0 0f87 0000 0000 	jv	0
			94: R_ARC_32_ME	.text
  98:	20e0 0f88 0000 0000 	jnv	0
			9c: R_ARC_32_ME	.text
  a0:	20e0 0f88 0000 0000 	jnv	0
			a4: R_ARC_32_ME	.text
  a8:	20e0 0f89 0000 0000 	jgt	0
			ac: R_ARC_32_ME	.text
  b0:	20e0 0f8a 0000 0000 	jge	0
			b4: R_ARC_32_ME	.text
  b8:	20e0 0f8b 0000 0000 	jlt	0
			bc: R_ARC_32_ME	.text
  c0:	20e0 0f8c 0000 0000 	jle	0
			c4: R_ARC_32_ME	.text
  c8:	20e0 0f8d 0000 0000 	jhi	0
			cc: R_ARC_32_ME	.text
  d0:	20e0 0f8e 0000 0000 	jls	0
			d4: R_ARC_32_ME	.text
  d8:	20e0 0f8f 0000 0000 	jpnz	0
			dc: R_ARC_32_ME	.text
  e0:	2020 0f80 0000 0000 	j	0
			e4: R_ARC_32_ME	external_text_label
  e8:	20a0 0000           	j	0
