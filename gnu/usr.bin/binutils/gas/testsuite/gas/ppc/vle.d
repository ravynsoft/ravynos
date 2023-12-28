#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: Validate VLE instructions

.*: +file format elf.*-powerpc.*

Disassembly of section \.text:

0+00 <.*>:
   0:	1c 83 00 1b 	e_add16i r4,r3,27
   4:	70 c0 8c 56 	e_add2i\. r0,13398
   8:	71 01 93 21 	e_add2is r1,17185
   c:	18 46 88 37 	e_addi\. r2,r6,55
  10:	18 65 81 37 	e_addi  r3,r5,14080
  14:	18 84 9a 37 	e_addic\. r4,r4,3604480
  18:	18 e8 93 37 	e_addic r7,r8,922746880
  1c:	71 3f ce ed 	e_and2i\. r9,65261
  20:	71 40 e8 05 	e_and2is\. r10,5
  24:	19 ab c8 39 	e_andi\. r11,r13,57
  28:	19 ec c2 37 	e_andi  r12,r15,3604480
  2c:	78 00 00 ec 	e_b     118 <middle_label>
  30:	78 00 00 01 	e_bl    30 <start_label\+0x30>
			30: R_PPC_VLE_REL24	extern_subr
  34:	7a 03 ff cc 	e_bns   0 <start_label>
  38:	7a 1f 00 01 	e_bsol  cr3,38 <start_label\+0x38>
			38: R_PPC_VLE_REL15	extern_subr
  3c:	70 c2 9b 33 	e_cmp16i r2,13107
  40:	18 46 a9 37 	e_cmpi  cr2,r6,14080
  44:	7c 87 58 1c 	e_cmph  cr1,r7,r11
  48:	73 ec b5 ef 	e_cmph16i r12,-529
  4c:	7c 06 40 5c 	e_cmphl cr0,r6,r8
  50:	70 4d ba 34 	e_cmphl16i r13,4660
  54:	73 e1 ae e0 	e_cmpl16i r1,65248
  58:	18 a3 ab 37 	e_cmpli cr1,r3,922746880
  5c:	7f a3 02 02 	e_crand 4\*cr7\+gt,so,lt
  60:	7c 02 e9 02 	e_crandc lt,eq,4\*cr7\+gt
  64:	7d f0 8a 42 	e_creqv 4\*cr3\+so,4\*cr4\+lt,4\*cr4\+gt
  68:	7d e0 19 c2 	e_crnand 4\*cr3\+so,lt,so
  6c:	7d e0 18 42 	e_crnor 4\*cr3\+so,lt,so
  70:	7d 8d 73 82 	e_cror  4\*cr3\+lt,4\*cr3\+gt,4\*cr3\+eq
  74:	7e 72 8b 42 	e_crorc 4\*cr4\+so,4\*cr4\+eq,4\*cr4\+gt
  78:	7c 00 01 82 	e_crclr lt
  7c:	30 e3 cc 0d 	e_lbz   r7,-13299\(r3\)
  80:	18 e5 00 cc 	e_lbzu  r7,-52\(r5\)
  84:	39 0a 01 ff 	e_lha   r8,511\(r10\)
  88:	19 01 03 ff 	e_lhau  r8,-1\(r1\)
  8c:	58 e0 18 38 	e_lhz   r7,6200\(0\)
  90:	18 e0 01 3e 	e_lhzu  r7,62\(0\)
  94:	70 06 1b 33 	e_li    r0,209715
  98:	70 26 e3 33 	e_lis   r1,13107
  9c:	18 a3 08 18 	e_lmw   r5,24\(r3\)
  a0:	50 a3 27 28 	e_lwz   r5,10024\(r3\)
  a4:	18 c2 02 72 	e_lwzu  r6,114\(r2\)
  a8:	7c 98 00 20 	e_mcrf  cr1,cr6
  ac:	19 2a a0 37 	e_mulli r9,r10,55
  b0:	70 01 a6 68 	e_mull2i r1,1640
  b4:	70 a4 c3 45 	e_or2i  r5,9029
  b8:	70 b4 d3 45 	e_or2is r5,41797
  bc:	19 27 d8 37 	e_ori\.  r7,r9,55
  c0:	19 07 d1 37 	e_ori   r7,r8,14080
  c4:	7e d2 02 30 	e_rlw   r18,r22,r0
  c8:	7c 48 02 31 	e_rlw\.  r8,r2,r0
  cc:	7c 74 aa 70 	e_rlwi  r20,r3,21
  d0:	7c 62 aa 71 	e_rlwi\. r2,r3,21
  d4:	76 64 6a 1e 	e_rlwimi r4,r19,13,8,15
  d8:	74 24 68 63 	e_rlwinm r4,r1,13,1,17
  dc:	7e 6c 30 70 	e_slwi  r12,r19,6
  e0:	7d 4c a0 71 	e_slwi\. r12,r10,20
  e4:	7c 20 84 70 	e_srwi  r0,r1,16
  e8:	7c 20 5c 71 	e_srwi\. r0,r1,11
  ec:	34 61 55 f0 	e_stb   r3,22000\(r1\)
  f0:	1a 76 04 fc 	e_stbu  r19,-4\(r22\)
  f4:	5c 15 02 9a 	e_sth   r0,666\(r21\)
  f8:	18 37 05 ff 	e_sthu  r1,-1\(r23\)
  fc:	18 03 09 04 	e_stmw  r0,4\(r3\)
 100:	54 60 3f 21 	e_stw   r3,16161\(0\)
 104:	1a c4 06 ee 	e_stwu  r22,-18\(r4\)
 108:	18 15 b2 37 	e_subfic r0,r21,3604480
 10c:	1a c0 bb 37 	e_subfic\. r22,r0,922746880
 110:	18 75 e1 37 	e_xori  r21,r3,14080
 114:	1a 80 e8 37 	e_xori\. r0,r20,55
0+0000118 <middle_label>:
 118:	04 7f       	se_add  r31,r7
 11a:	21 ec       	se_addi r28,31
 11c:	46 10       	se_and  r0,r1
 11e:	47 01       	se_and\. r1,r0
 120:	45 32       	se_andc r2,r3
 122:	2f 14       	se_andi r4,17
 124:	e8 fa       	se_b    118 <middle_label>
 126:	e9 00       	se_bl   126 <middle_label\+0xe>
			126: R_PPC_VLE_REL8	extern_subr
 128:	e7 14       	se_bso  150 <not_end_label>
 12a:	61 2b       	se_bclri r27,18
 12c:	00 06       	se_bctr
 12e:	00 07       	se_bctrl
 130:	63 17       	se_bgeni r7,17
 132:	00 04       	se_blr
 134:	00 05       	se_blrl
 136:	2c 06       	se_bmaski r6,0
 138:	64 10       	se_bseti r0,1
 13a:	66 74       	se_btsti r4,7
 13c:	0c 10       	se_cmp  r0,r1
 13e:	0e cf       	se_cmph r31,r28
 140:	0f 91       	se_cmphl r1,r25
 142:	2b 63       	se_cmpi r3,22
 144:	0d 76       	se_cmpl r6,r7
 146:	22 bc       	se_cmpli r28,12
 148:	00 d1       	se_extsb r1
 14a:	00 f2       	se_extsh r2
 14c:	00 ce       	se_extzb r30
 14e:	00 e8       	se_extzh r24
0+0000150 <not_end_label>:
 150:	00 00       	se_illegal
 152:	00 01       	se_isync
 154:	88 18       	se_lbz  r1,8\(r24\)
 156:	a9 84       	se_lhz  r24,18\(r4\)
 158:	4c f4       	se_li   r4,79
 15a:	cf 60       	se_lwz  r6,60\(r0\)
 15c:	03 07       	se_mfar r7,r8
 15e:	00 a3       	se_mfctr r3
 160:	00 84       	se_mflr r4
 162:	01 0f       	se_mr   r31,r0
 164:	02 2f       	se_mtar r23,r2
 166:	00 b6       	se_mtctr r6
 168:	00 9f       	se_mtlr r31
 16a:	05 43       	se_mullw r3,r4
 16c:	00 38       	se_neg  r24
 16e:	00 29       	se_not  r25
 170:	44 10       	se_or   r0,r1
 172:	00 09       	se_rfci
 174:	00 0a       	se_rfdi
 176:	00 08       	se_rfi
 178:	00 02       	se_sc
 17a:	42 65       	se_slw  r5,r6
 17c:	6c 77       	se_slwi r7,7
 17e:	41 e6       	se_sraw r6,r30
 180:	6a 89       	se_srawi r25,8
 182:	40 0e       	se_srw  r30,r0
 184:	69 9d       	se_srwi r29,25
 186:	9a 02       	se_stb  r0,10\(r2\)
 188:	b6 1e       	se_sth  r1,12\(r30\)
 18a:	d0 7d       	se_stw  r7,0\(r29\)
 18c:	06 21       	se_sub  r1,r2
 18e:	07 ad       	se_subf r29,r26
 190:	25 77       	se_subi r7,24
0+0000192 <end_label>:
 192:	27 29       	se_subi\. r25,19
 194:	e9 c2       	se_bl   118 <middle_label>
 196:	79 ff ff 82 	e_b     118 <middle_label>
 19a:	79 ff fe 67 	e_bl    0 <start_label>
 19e:	00 0c       	se_rfgi
 1a0:	7c 00 00 48 	e_sc
 1a4:	7c 00 00 48 	e_sc
 1a8:	7c 00 08 48 	e_sc    1
