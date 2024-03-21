#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 2

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

0+0 <target0>:
   0:	7a 20 00 0c 	e_bdnz  c <target1>
   4:	7a 20 00 09 	e_bdnzl c <target1>
   8:	7a 30 00 10 	e_bdz   18 <target2>

0+c <target1>:
   c:	7a 30 ff f5 	e_bdzl  0 <target0>
  10:	7a 12 ff f0 	e_beq   0 <target0>
  14:	7a 16 00 8c 	e_beq   cr1,a0 <target8>

0+18 <target2>:
  18:	7a 12 ff f5 	e_beql  c <target1>
  1c:	7a 12 00 4d 	e_beql  68 <target6>
  20:	7a 01 00 04 	e_ble   24 <target3>

0+24 <target3>:
  24:	7a 03 ff dd 	e_bnsl  0 <target0>
  28:	7a 04 ff e4 	e_bge   cr1,c <target1>
  2c:	7a 00 00 24 	e_bge   50 <target5>

0+30 <target4>:
  30:	7a 08 ff f5 	e_bgel  cr2,24 <target3>
  34:	7a 00 ff fd 	e_bgel  30 <target4>
  38:	7a 11 ff c8 	e_bgt   0 <target0>
  3c:	7a 11 ff c4 	e_bgt   0 <target0>
  40:	7a 19 ff d9 	e_bgtl  cr2,18 <target2>
  44:	7a 11 ff d5 	e_bgtl  18 <target2>
  48:	7a 0d 00 08 	e_ble   cr3,50 <target5>
  4c:	7a 01 00 04 	e_ble   50 <target5>

0+50 <target5>:
  50:	7a 01 ff e1 	e_blel  30 <target4>
  54:	7a 01 ff dd 	e_blel  30 <target4>
  58:	7a 14 ff cc 	e_blt   cr1,24 <target3>
  5c:	7a 10 ff c8 	e_blt   24 <target3>
  60:	7a 10 ff a1 	e_bltl  0 <target0>
  64:	7a 14 ff 9d 	e_bltl  cr1,0 <target0>

0+68 <target6>:
  68:	7a 02 00 18 	e_bne   80 <target7>
  6c:	7a 06 ff 94 	e_bne   cr1,0 <target0>
  70:	7a 02 ff e1 	e_bnel  50 <target5>
  74:	7a 02 ff dd 	e_bnel  50 <target5>
  78:	7a 01 00 48 	e_ble   c0 <target9>
  7c:	7a 05 ff b4 	e_ble   cr1,30 <target4>

0+80 <target7>:
  80:	7a 09 ff e9 	e_blel  cr2,68 <target6>
  84:	7a 01 00 1d 	e_blel  a0 <target8>
  88:	7a 04 ff c8 	e_bge   cr1,50 <target5>
  8c:	7a 00 ff c4 	e_bge   50 <target5>
  90:	7a 0c ff 95 	e_bgel  cr3,24 <target3>
  94:	7a 00 ff 91 	e_bgel  24 <target3>
  98:	7a 03 ff 80 	e_bns   18 <target2>
  9c:	7a 03 ff 7c 	e_bns   18 <target2>

0+a0 <target8>:
  a0:	7a 0b ff 61 	e_bnsl  cr2,0 <target0>
  a4:	7a 03 ff c5 	e_bnsl  68 <target6>
  a8:	7a 07 ff 64 	e_bns   cr1,c <target1>
  ac:	7a 03 ff 60 	e_bns   c <target1>
  b0:	7a 03 ff d1 	e_bnsl  80 <target7>
  b4:	7a 03 ff 71 	e_bnsl  24 <target3>
  b8:	7a 17 ff 78 	e_bso   cr1,30 <target4>
  bc:	7a 13 ff 74 	e_bso   30 <target4>

0+c0 <target9>:
  c0:	7a 13 ff e1 	e_bsol  a0 <target8>
  c4:	7a 13 ff dd 	e_bsol  a0 <target8>
  c8:	7a 11 ff b8 	e_bgt   80 <target7>
  cc:	7a 10 ff 85 	e_bltl  50 <target5>
  d0:	7a 17 ff 60 	e_bso   cr1,30 <target4>
  d4:	7a 13 ff 5c 	e_bso   30 <target4>
  d8:	7a 1b ff 29 	e_bsol  cr2,0 <target0>
  dc:	7a 13 ff e5 	e_bsol  c0 <target9>
