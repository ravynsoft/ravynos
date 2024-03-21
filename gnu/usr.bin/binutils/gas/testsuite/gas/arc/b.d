#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

00000000 <text_label>:
   0:	0001 0000           	b	0	;0 <text_label>
   4:	07fc ffc0           	b	-4	;0 <text_label>
   8:	07f8 ffc0           	b	-8	;0 <text_label>
   c:	07f4 ffc1           	beq	-12	;0 <text_label>
  10:	07f0 ffc1           	beq	-16	;0 <text_label>
  14:	07ec ffc2           	bne	-20	;0 <text_label>
  18:	07e8 ffc2           	bne	-24	;0 <text_label>
  1c:	07e4 ffc3           	bp	-28	;0 <text_label>
  20:	07e0 ffc3           	bp	-32	;0 <text_label>
  24:	07dc ffc4           	bn	-36	;0 <text_label>
  28:	07d8 ffc4           	bn	-40	;0 <text_label>
  2c:	07d4 ffc5           	bc	-44	;0 <text_label>
  30:	07d0 ffc5           	bc	-48	;0 <text_label>
  34:	07cc ffc5           	bc	-52	;0 <text_label>
  38:	07c8 ffc6           	bnc	-56	;0 <text_label>
  3c:	07c4 ffc6           	bnc	-60	;0 <text_label>
  40:	07c0 ffc6           	bnc	-64	;0 <text_label>
  44:	07bc ffc7           	bv	-68	;0 <text_label>
  48:	07b8 ffc7           	bv	-72	;0 <text_label>
  4c:	07b4 ffc8           	bnv	-76	;0 <text_label>
  50:	07b0 ffc8           	bnv	-80	;0 <text_label>
  54:	07ac ffc9           	bgt	-84	;0 <text_label>
  58:	07a8 ffca           	bge	-88	;0 <text_label>
  5c:	07a4 ffcb           	blt	-92	;0 <text_label>
  60:	07a0 ffcc           	ble	-96	;0 <text_label>
  64:	079c ffcd           	bhi	-100	;0 <text_label>
  68:	0798 ffce           	bls	-104	;0 <text_label>
  6c:	0794 ffcf           	bpnz	-108	;0 <text_label>
  70:	0791 ffef           	b.d	-112	;0 <text_label>
  74:	264a 7000           	nop
  78:	0789 ffcf           	b	-120	;0 <text_label>
  7c:	0785 ffef           	b.d	-124	;0 <text_label>
  80:	264a 7000           	nop
  84:	077c ffe1           	beq.d	-132	;0 <text_label>
  88:	264a 7000           	nop
  8c:	0774 ffc2           	bne	-140	;0 <text_label>
  90:	0770 ffe6           	bnc.d	-144	;0 <text_label>
  94:	264a 7000           	nop
