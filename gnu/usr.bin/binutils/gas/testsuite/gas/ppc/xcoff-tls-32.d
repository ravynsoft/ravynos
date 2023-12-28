#as: -a32
#source: xcoff-tls.s
#objdump: -Dr -j.data -j.tdata -j.tbss
#name: XCOFF TLS relocation (32 bit)

.*
Disassembly of section .data:

00000000 <TOC>:
   0:	00 00 00 58.*
			0: R_TLS	tdata_ext.*

00000004 <.tdata_ext_gd>:
   4:	00 00 00 00.*
			4: R_TLSM	tdata_ext.*

00000008 <tdata_ext_ld>:
   8:	00 00 00 58.*
			8: R_TLS_LD	tdata_ext.*

0000000c <tdata_ext_ie>:
   c:	00 00 00 58.*
			c: R_TLS_IE	tdata_ext.*

00000010 <tdata_ext_le>:
  10:	00 00 00 58.*
			10: R_TLS_LE	tdata_ext.*

00000014 <tdata_int1_gd>:
  14:	00 00 00 5c.*
			14: R_TLS	tdata_int_csect.*

00000018 <.tdata_int1_gd>:
  18:	00 00 00 00.*
			18: R_TLSM	tdata_int_csect.*

0000001c <tdata_int1_ld>:
  1c:	00 00 00 5c.*
			1c: R_TLS_LD	tdata_int_csect.*

00000020 <tdata_int1_ie>:
  20:	00 00 00 5c.*
			20: R_TLS_IE	tdata_int_csect.*

00000024 <tdata_int1_le>:
  24:	00 00 00 5c.*
			24: R_TLS_LE	tdata_int_csect.*

00000028 <tdata_int2_gd>:
  28:	00 00 00 60.*
			28: R_TLS	tdata_int_csect.*

0000002c <.tdata_int2_gd>:
  2c:	00 00 00 00.*
			2c: R_TLSM	tdata_int_csect.*

00000030 <tdata_int2_ld>:
  30:	00 00 00 60.*
			30: R_TLS_LD	tdata_int_csect.*

00000034 <tdata_int2_ie>:
  34:	00 00 00 60.*
			34: R_TLS_IE	tdata_int_csect.*

00000038 <tdata_int2_le>:
  38:	00 00 00 60.*
			38: R_TLS_LE	tdata_int_csect.*

0000003c <tbss_ext_gd>:
  3c:	00 00 00 68.*
			3c: R_TLS	tbss_ext.*

00000040 <.tbss_ext_gd>:
  40:	00 00 00 00.*
			40: R_TLSM	tbss_ext.*

00000044 <tbss_ext_ld>:
  44:	00 00 00 68.*
			44: R_TLS_LD	tbss_ext.*

00000048 <tbss_ext_ie>:
  48:	00 00 00 68.*
			48: R_TLS_IE	tbss_ext.*

0000004c <tbss_ext_le>:
  4c:	00 00 00 68.*
			4c: R_TLS_LE	tbss_ext.*

00000050 <_\$TLSML>:
	...
			50: R_TLSML	_\$TLSML.*

Disassembly of section .tdata:

00000058 <tdata_ext>:
  58:	00 00 00 01 	.long 0x1

0000005c <tdata_int_csect>:
  5c:	00 00 00 02 	.long 0x2
  60:	00 00 00 03 	.long 0x3
  64:.*

Disassembly of section .tbss:

00000068 <tbss_ext>:
	...
