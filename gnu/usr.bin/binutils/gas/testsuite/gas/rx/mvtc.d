#source: ./mvtc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 77 00 80                   	mvtc	#-128, psw
   4:	fd 77 03 80                   	mvtc	#-128, fpsw
   8:	fd 77 02 80                   	mvtc	#-128, usp
   c:	fd 77 0a 80                   	mvtc	#-128, isp
  10:	fd 77 0c 80                   	mvtc	#-128, intb
  14:	fd 77 08 80                   	mvtc	#-128, bpsw
  18:	fd 77 09 80                   	mvtc	#-128, bpc
  1c:	fd 77 0b 80                   	mvtc	#-128, fintv
  20:	fd 77 0d 80                   	mvtc	#-128, extb
  24:	fd 77 00 7f                   	mvtc	#127, psw
  28:	fd 77 03 7f                   	mvtc	#127, fpsw
  2c:	fd 77 02 7f                   	mvtc	#127, usp
  30:	fd 77 0a 7f                   	mvtc	#127, isp
  34:	fd 77 0c 7f                   	mvtc	#127, intb
  38:	fd 77 08 7f                   	mvtc	#127, bpsw
  3c:	fd 77 09 7f                   	mvtc	#127, bpc
  40:	fd 77 0b 7f                   	mvtc	#127, fintv
  44:	fd 77 0d 7f                   	mvtc	#127, extb
  ..:	fd 7b 00 00 80                	mvtc	#0xffff8000, psw
  ..:	fd 7b 03 00 80                	mvtc	#0xffff8000, fpsw
  ..:	fd 7b 02 00 80                	mvtc	#0xffff8000, usp
  ..:	fd 7b 0a 00 80                	mvtc	#0xffff8000, isp
  ..:	fd 7b 0c 00 80                	mvtc	#0xffff8000, intb
  ..:	fd 7b 08 00 80                	mvtc	#0xffff8000, bpsw
  ..:	fd 7b 09 00 80                	mvtc	#0xffff8000, bpc
  ..:	fd 7b 0b 00 80                	mvtc	#0xffff8000, fintv
  ..:	fd 7b 0d 00 80                	mvtc	#0xffff8000, extb
  ..:	fd 7f 00 00 80 00             	mvtc	#0x8000, psw
  ..:	fd 7f 03 00 80 00             	mvtc	#0x8000, fpsw
  ..:	fd 7f 02 00 80 00             	mvtc	#0x8000, usp
  ..:	fd 7f 0a 00 80 00             	mvtc	#0x8000, isp
  ..:	fd 7f 0c 00 80 00             	mvtc	#0x8000, intb
  ..:	fd 7f 08 00 80 00             	mvtc	#0x8000, bpsw
  ..:	fd 7f 09 00 80 00             	mvtc	#0x8000, bpc
  ..:	fd 7f 0b 00 80 00             	mvtc	#0x8000, fintv
  ..:	fd 7f 0d 00 80 00             	mvtc	#0x8000, extb
  ..:	fd 7f 00 00 00 80             	mvtc	#0xff800000, psw
  ..:	fd 7f 03 00 00 80             	mvtc	#0xff800000, fpsw
  ..:	fd 7f 02 00 00 80             	mvtc	#0xff800000, usp
  ..:	fd 7f 0a 00 00 80             	mvtc	#0xff800000, isp
  ..:	fd 7f 0c 00 00 80             	mvtc	#0xff800000, intb
  ..:	fd 7f 08 00 00 80             	mvtc	#0xff800000, bpsw
  ..:	fd 7f 09 00 00 80             	mvtc	#0xff800000, bpc
  ..:	fd 7f 0b 00 00 80             	mvtc	#0xff800000, fintv
  ..:	fd 7f 0d 00 00 80             	mvtc	#0xff800000, extb
  ..:	fd 7f 00 ff ff 7f             	mvtc	#0x7fffff, psw
  ..:	fd 7f 03 ff ff 7f             	mvtc	#0x7fffff, fpsw
  ..:	fd 7f 02 ff ff 7f             	mvtc	#0x7fffff, usp
  ..:	fd 7f 0a ff ff 7f             	mvtc	#0x7fffff, isp
  ..:	fd 7f 0c ff ff 7f             	mvtc	#0x7fffff, intb
 ...:	fd 7f 08 ff ff 7f             	mvtc	#0x7fffff, bpsw
 ...:	fd 7f 09 ff ff 7f             	mvtc	#0x7fffff, bpc
 ...:	fd 7f 0b ff ff 7f             	mvtc	#0x7fffff, fintv
 ...:	fd 7f 0d ff ff 7f             	mvtc	#0x7fffff, extb
 ...:	fd 73 00 00 00 00 80          	mvtc	#0x80000000, psw
 ...:	fd 73 03 00 00 00 80          	mvtc	#0x80000000, fpsw
 ...:	fd 73 02 00 00 00 80          	mvtc	#0x80000000, usp
 ...:	fd 73 0a 00 00 00 80          	mvtc	#0x80000000, isp
 ...:	fd 73 0c 00 00 00 80          	mvtc	#0x80000000, intb
 ...:	fd 73 08 00 00 00 80          	mvtc	#0x80000000, bpsw
 ...:	fd 73 09 00 00 00 80          	mvtc	#0x80000000, bpc
 ...:	fd 73 0b 00 00 00 80          	mvtc	#0x80000000, fintv
 ...:	fd 73 0d 00 00 00 80          	mvtc	#0x80000000, extb
 ...:	fd 73 00 ff ff ff 7f          	mvtc	#0x7fffffff, psw
 ...:	fd 73 03 ff ff ff 7f          	mvtc	#0x7fffffff, fpsw
 ...:	fd 73 02 ff ff ff 7f          	mvtc	#0x7fffffff, usp
 ...:	fd 73 0a ff ff ff 7f          	mvtc	#0x7fffffff, isp
 ...:	fd 73 0c ff ff ff 7f          	mvtc	#0x7fffffff, intb
 ...:	fd 73 08 ff ff ff 7f          	mvtc	#0x7fffffff, bpsw
 ...:	fd 73 09 ff ff ff 7f          	mvtc	#0x7fffffff, bpc
 ...:	fd 73 0b ff ff ff 7f          	mvtc	#0x7fffffff, fintv
 ...:	fd 73 0d ff ff ff 7f          	mvtc	#0x7fffffff, extb
 ...:	fd 68 00                      	mvtc	r0, psw
 ...:	fd 68 03                      	mvtc	r0, fpsw
 ...:	fd 68 02                      	mvtc	r0, usp
 ...:	fd 68 0a                      	mvtc	r0, isp
 ...:	fd 68 0c                      	mvtc	r0, intb
 ...:	fd 68 08                      	mvtc	r0, bpsw
 ...:	fd 68 09                      	mvtc	r0, bpc
 ...:	fd 68 0b                      	mvtc	r0, fintv
 ...:	fd 68 0d                      	mvtc	r0, extb
 ...:	fd 68 f0                      	mvtc	r15, psw
 ...:	fd 68 f3                      	mvtc	r15, fpsw
 ...:	fd 68 f2                      	mvtc	r15, usp
 ...:	fd 68 fa                      	mvtc	r15, isp
 ...:	fd 68 fc                      	mvtc	r15, intb
 ...:	fd 68 f8                      	mvtc	r15, bpsw
 ...:	fd 68 f9                      	mvtc	r15, bpc
 ...:	fd 68 fb                      	mvtc	r15, fintv
 ...:	fd 68 fd                      	mvtc	r15, extb

