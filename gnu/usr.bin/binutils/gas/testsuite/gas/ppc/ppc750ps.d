#as: -m750cl
#objdump: -dr -Mppcps
#name: PPC750CL paired single tests

.*

Disassembly of section \.text:

0+0000000 <start>:
   0:	(e0 03 d0 04|04 d0 03 e0) 	psq_l   f0,4\(r3\),1,5
   4:	(e4 22 30 08|08 30 22 e4) 	psq_lu  f1,8\(r2\),0,3
   8:	(10 45 25 4c|4c 25 45 10) 	psq_lux f2,r5,r4,1,2
   c:	(10 62 22 8c|8c 22 62 10) 	psq_lx  f3,r2,r4,0,5
  10:	(f0 62 30 08|08 30 62 f0) 	psq_st  f3,8\(r2\),0,3
  14:	(f4 62 70 08|08 70 62 f4) 	psq_stu f3,8\(r2\),0,7
  18:	(10 43 22 ce|ce 22 43 10) 	psq_stux f2,r3,r4,0,5
  1c:	(10 c7 46 0e|0e 46 c7 10) 	psq_stx f6,r7,r8,1,4
  20:	(10 a0 3a 10|10 3a a0 10) 	ps_abs  f5,f7
  24:	(10 a0 3a 11|11 3a a0 10) 	ps_abs. f5,f7
  28:	(10 22 18 2a|2a 18 22 10) 	ps_add  f1,f2,f3
  2c:	(10 22 18 2b|2b 18 22 10) 	ps_add. f1,f2,f3
  30:	(11 82 20 40|40 20 82 11) 	ps_cmpo0 cr3,f2,f4
  34:	(11 82 20 c0|c0 20 82 11) 	ps_cmpo1 cr3,f2,f4
  38:	(11 82 20 00|00 20 82 11) 	ps_cmpu0 cr3,f2,f4
  3c:	(11 82 20 80|80 20 82 11) 	ps_cmpu1 cr3,f2,f4
  40:	(10 44 30 24|24 30 44 10) 	ps_div  f2,f4,f6
  44:	(10 44 30 25|25 30 44 10) 	ps_div. f2,f4,f6
  48:	(10 01 18 ba|ba 18 01 10) 	ps_madd f0,f1,f2,f3
  4c:	(10 01 18 bb|bb 18 01 10) 	ps_madd. f0,f1,f2,f3
  50:	(10 22 20 dc|dc 20 22 10) 	ps_madds0 f1,f2,f3,f4
  54:	(10 22 20 dd|dd 20 22 10) 	ps_madds0. f1,f2,f3,f4
  58:	(10 22 20 de|de 20 22 10) 	ps_madds1 f1,f2,f3,f4
  5c:	(10 22 20 df|df 20 22 10) 	ps_madds1. f1,f2,f3,f4
  60:	(10 44 34 20|20 34 44 10) 	ps_merge00 f2,f4,f6
  64:	(10 44 34 21|21 34 44 10) 	ps_merge00. f2,f4,f6
  68:	(10 44 34 60|60 34 44 10) 	ps_merge01 f2,f4,f6
  6c:	(10 44 34 61|61 34 44 10) 	ps_merge01. f2,f4,f6
  70:	(10 44 34 a0|a0 34 44 10) 	ps_merge10 f2,f4,f6
  74:	(10 44 34 a1|a1 34 44 10) 	ps_merge10. f2,f4,f6
  78:	(10 44 34 e0|e0 34 44 10) 	ps_merge11 f2,f4,f6
  7c:	(10 44 34 e1|e1 34 44 10) 	ps_merge11. f2,f4,f6
  80:	(10 60 28 90|90 28 60 10) 	ps_mr   f3,f5
  84:	(10 60 28 91|91 28 60 10) 	ps_mr.  f3,f5
  88:	(10 44 41 b8|b8 41 44 10) 	ps_msub f2,f4,f6,f8
  8c:	(10 44 41 b9|b9 41 44 10) 	ps_msub. f2,f4,f6,f8
  90:	(10 43 01 72|72 01 43 10) 	ps_mul  f2,f3,f5
  94:	(10 43 01 73|73 01 43 10) 	ps_mul. f2,f3,f5
  98:	(10 64 01 d8|d8 01 64 10) 	ps_muls0 f3,f4,f7
  9c:	(10 64 01 d9|d9 01 64 10) 	ps_muls0. f3,f4,f7
  a0:	(10 64 01 da|da 01 64 10) 	ps_muls1 f3,f4,f7
  a4:	(10 64 01 db|db 01 64 10) 	ps_muls1. f3,f4,f7
  a8:	(10 20 29 10|10 29 20 10) 	ps_nabs f1,f5
  ac:	(10 20 29 11|11 29 20 10) 	ps_nabs. f1,f5
  b0:	(10 20 28 50|50 28 20 10) 	ps_neg  f1,f5
  b4:	(10 20 28 51|51 28 20 10) 	ps_neg. f1,f5
  b8:	(10 23 39 7e|7e 39 23 10) 	ps_nmadd f1,f3,f5,f7
  bc:	(10 23 39 7f|7f 39 23 10) 	ps_nmadd. f1,f3,f5,f7
  c0:	(10 23 39 7c|7c 39 23 10) 	ps_nmsub f1,f3,f5,f7
  c4:	(10 23 39 7d|7d 39 23 10) 	ps_nmsub. f1,f3,f5,f7
  c8:	(11 20 18 30|30 18 20 11) 	ps_res  f9,f3
  cc:	(11 20 18 31|31 18 20 11) 	ps_res. f9,f3
  d0:	(11 20 18 34|34 18 20 11) 	ps_rsqrte f9,f3
  d4:	(11 20 18 35|35 18 20 11) 	ps_rsqrte. f9,f3
  d8:	(10 22 20 ee|ee 20 22 10) 	ps_sel  f1,f2,f3,f4
  dc:	(10 22 20 ef|ef 20 22 10) 	ps_sel. f1,f2,f3,f4
  e0:	(10 ab 10 28|28 10 ab 10) 	ps_sub  f5,f11,f2
  e4:	(10 ab 10 29|29 10 ab 10) 	ps_sub. f5,f11,f2
  e8:	(10 45 52 54|54 52 45 10) 	ps_sum0 f2,f5,f9,f10
  ec:	(10 45 52 55|55 52 45 10) 	ps_sum0. f2,f5,f9,f10
  f0:	(10 45 52 56|56 52 45 10) 	ps_sum1 f2,f5,f9,f10
  f4:	(10 45 52 57|57 52 45 10) 	ps_sum1. f2,f5,f9,f10
  f8:	(10 03 2f ec|ec 2f 03 10) 	dcbz_l  r3,r5
#pass
