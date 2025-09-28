#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <text_label>:
   0:	0802 0000           	bl	0	;0 <text_label>
   4:	0ffc ffc0           	bl	-4	;0 <text_label>
   8:	0ff8 ffc0           	bl	-8	;0 <text_label>
   c:	0ff4 ffc1           	bleq	-12	;0 <text_label>
  10:	0ff0 ffc1           	bleq	-16	;0 <text_label>
  14:	0fec ffc2           	blne	-20	;0 <text_label>
  18:	0fe8 ffc2           	blne	-24	;0 <text_label>
  1c:	0fe4 ffc3           	blp	-28	;0 <text_label>
  20:	0fe0 ffc3           	blp	-32	;0 <text_label>
  24:	0fdc ffc4           	bln	-36	;0 <text_label>
  28:	0fd8 ffc4           	bln	-40	;0 <text_label>
  2c:	0fd4 ffc5           	blc	-44	;0 <text_label>
  30:	0fd0 ffc5           	blc	-48	;0 <text_label>
  34:	0fcc ffc5           	blc	-52	;0 <text_label>
  38:	0fc8 ffc6           	blnc	-56	;0 <text_label>
  3c:	0fc4 ffc6           	blnc	-60	;0 <text_label>
  40:	0fc0 ffc6           	blnc	-64	;0 <text_label>
  44:	0fbc ffc7           	blv	-68	;0 <text_label>
  48:	0fb8 ffc7           	blv	-72	;0 <text_label>
  4c:	0fb4 ffc8           	blnv	-76	;0 <text_label>
  50:	0fb0 ffc8           	blnv	-80	;0 <text_label>
  54:	0fac ffc9           	blgt	-84	;0 <text_label>
  58:	0fa8 ffca           	blge	-88	;0 <text_label>
  5c:	0fa4 ffcb           	bllt	-92	;0 <text_label>
  60:	0fa0 ffcc           	blle	-96	;0 <text_label>
  64:	0f9c ffcd           	blhi	-100	;0 <text_label>
  68:	0f98 ffce           	blls	-104	;0 <text_label>
  6c:	0f94 ffcf           	blpnz	-108	;0 <text_label>
  70:	0f92 ffef           	bl.d	-112	;0 <text_label>
  74:	78e0                	nop_s
  76:	0f8e ffcf           	bl	-116	;0 <text_label>
  7a:	78e0                	nop_s
  7c:	0f84 ffe1           	bleq.d	-124	;0 <text_label>
  80:	78e0                	nop_s
  82:	0f80 ffc2           	blne	-128	;0 <text_label>
  86:	78e0                	nop_s
  88:	0f78 ffe6           	blnc.d	-136	;0 <text_label>
  8c:	78e0                	nop_s
