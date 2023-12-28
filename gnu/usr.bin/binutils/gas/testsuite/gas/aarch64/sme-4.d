#name: SME extension (ZERO)
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	c0080000 	zero	{}
   4:	c00800ff 	zero	{za}
   8:	c00800ff 	zero	{za}
   c:	c00800ff 	zero	{za}
  10:	c00800ff 	zero	{za}
  14:	c00800ff 	zero	{za}
  18:	c0080001 	zero	{za0\.d}
  1c:	c0080002 	zero	{za1\.d}
  20:	c0080004 	zero	{za2\.d}
  24:	c0080008 	zero	{za3\.d}
  28:	c0080010 	zero	{za4\.d}
  2c:	c0080020 	zero	{za5\.d}
  30:	c0080040 	zero	{za6\.d}
  34:	c0080080 	zero	{za7\.d}
  38:	c0080001 	zero	{za0\.d}
  3c:	c0080003 	zero	{za0\.d, za1\.d}
  40:	c0080007 	zero	{za0\.d, za1\.d, za2\.d}
  44:	c008000f 	zero	{za0\.d, za1\.d, za2\.d, za3\.d}
  48:	c008001f 	zero	{za0\.s, za1\.d, za2\.d, za3\.d}
  4c:	c008003f 	zero	{za0\.s, za1\.s, za2\.d, za3\.d}
  50:	c008007f 	zero	{za0\.h, za1\.s, za3\.d}
  54:	c00800ff 	zero	{za}
  58:	c0080080 	zero	{za7\.d}
  5c:	c00800c0 	zero	{za6\.d, za7\.d}
  60:	c00800e0 	zero	{za5\.d, za6\.d, za7\.d}
  64:	c00800f0 	zero	{za4\.d, za5\.d, za6\.d, za7\.d}
  68:	c00800f8 	zero	{za3\.s, za4\.d, za5\.d, za6\.d}
  6c:	c00800fc 	zero	{za2\.s, za3\.s, za4\.d, za5\.d}
  70:	c00800fe 	zero	{za1\.h, za2\.s, za4\.d}
  74:	c00800ff 	zero	{za}
  78:	c00800fe 	zero	{za1\.h, za2\.s, za4\.d}
  7c:	c00800fd 	zero	{za0\.h, za3\.s, za5\.d}
  80:	c00800fb 	zero	{za1\.h, za0\.s, za6\.d}
  84:	c00800f7 	zero	{za0\.h, za1\.s, za7\.d}
  88:	c00800ef 	zero	{za1\.h, za2\.s, za0\.d}
  8c:	c00800df 	zero	{za0\.h, za3\.s, za1\.d}
  90:	c00800bf 	zero	{za1\.h, za0\.s, za2\.d}
  94:	c008007f 	zero	{za0\.h, za1\.s, za3\.d}
  98:	c0080055 	zero	{za0\.h}
  9c:	c00800aa 	zero	{za1\.h}
  a0:	c0080011 	zero	{za0\.s}
  a4:	c0080022 	zero	{za1\.s}
  a8:	c0080044 	zero	{za2\.s}
  ac:	c0080088 	zero	{za3\.s}
  b0:	c0080055 	zero	{za0\.h}
  b4:	c0080055 	zero	{za0\.h}
  b8:	c0080055 	zero	{za0\.h}
  bc:	c00800aa 	zero	{za1\.h}
  c0:	c00800aa 	zero	{za1\.h}
  c4:	c00800aa 	zero	{za1\.h}
  c8:	c0080011 	zero	{za0\.s}
  cc:	c0080022 	zero	{za1\.s}
  d0:	c0080044 	zero	{za2\.s}
  d4:	c0080088 	zero	{za3\.s}
  d8:	c00800d5 	zero	{za0.h, za7.d}
  dc:	c00800ab 	zero	{za1.h, za0.d}
  e0:	c0080015 	zero	{za0.s, za2.d}
  e4:	c008002a 	zero	{za1.s, za3.d}
  e8:	c0080054 	zero	{za2.s, za4.d}
  ec:	c00800a8 	zero	{za3.s, za5.d}
  f0:	c00800d5 	zero	{za0.h, za7.d}
  f4:	c0080015 	zero	{za0.s, za2.d}
