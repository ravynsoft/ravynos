#as: -a32 -mbig -mvle
#objdump: -dr -Mvle
#name: VLE Simplified mnemonics 1

.*: +file format elf.*-powerpc.*

Disassembly of section \.text:

0+0 <target0>:
   0:	e6 03       	se_beq  6 <target3>

0+2 <target1>:
   2:	e1 03       	se_ble  8 <target4>

0+4 <target2>:
   4:	e0 0+0       	se_bge  4 <target2>

0+6 <target3>:
   6:	e5 fe       	se_bgt  2 <target1>

0+8 <target4>:
   8:	e1 ff       	se_ble  6 <target3>
   a:	e4 03       	se_blt  10 <target6>

0+c <target5>:
   c:	e2 fb       	se_bne  2 <target1>
   e:	e1 01       	se_ble  10 <target6>

0+10 <target6>:
  10:	e0 fc       	se_bge  8 <target4>
  12:	e3 fd       	se_bns  c <target5>

0+14 <target8>:
  14:	e3 f8       	se_bns  4 <target2>
  16:	e7 ff       	se_bso  14 <target8>

0+18 <target9>:
  18:	e6 fc       	se_beq  10 <target6>
  1a:	e7 ff       	se_bso  18 <target9>
