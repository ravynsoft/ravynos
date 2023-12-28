#as: -a64
#source: xcoff-tls.s
#objdump: -Dr -j.data -j.tdata -j.tbss
#name: XCOFF TLS relocation (64 bit)

.*


Disassembly of section .data:

0000000000000000 <TOC>:
   0:	00 00 00 00.*
			0: R_TLS	tdata_ext.*
   4:	00 00 00 a8.*

0000000000000008 <.tdata_ext_gd>:
	...
			8: R_TLSM	tdata_ext.*

0000000000000010 <tdata_ext_ld>:
  10:	00 00 00 00.*
			10: R_TLS_LD	tdata_ext.*
  14:	00 00 00 a8.*

0000000000000018 <tdata_ext_ie>:
  18:	00 00 00 00.*
			18: R_TLS_IE	tdata_ext.*
  1c:	00 00 00 a8.*

0000000000000020 <tdata_ext_le>:
  20:	00 00 00 00.*
			20: R_TLS_LE	tdata_ext.*
  24:	00 00 00 a8.*

0000000000000028 <tdata_int1_gd>:
  28:	00 00 00 00.*
			28: R_TLS	tdata_int_csect.*
  2c:	00 00 00 ac.*

0000000000000030 <.tdata_int1_gd>:
	...
			30: R_TLSM	tdata_int_csect.*

0000000000000038 <tdata_int1_ld>:
  38:	00 00 00 00.*
			38: R_TLS_LD	tdata_int_csect.*
  3c:	00 00 00 ac.*

0000000000000040 <tdata_int1_ie>:
  40:	00 00 00 00.*
			40: R_TLS_IE	tdata_int_csect.*
  44:	00 00 00 ac.*

0000000000000048 <tdata_int1_le>:
  48:	00 00 00 00.*
			48: R_TLS_LE	tdata_int_csect.*
  4c:	00 00 00 ac.*

0000000000000050 <tdata_int2_gd>:
  50:	00 00 00 00.*
			50: R_TLS	tdata_int_csect.*
  54:	00 00 00 b0.*

0000000000000058 <.tdata_int2_gd>:
	...
			58: R_TLSM	tdata_int_csect.*

0000000000000060 <tdata_int2_ld>:
  60:	00 00 00 00.*
			60: R_TLS_LD	tdata_int_csect.*
  64:	00 00 00 b0.*

0000000000000068 <tdata_int2_ie>:
  68:	00 00 00 00.*
			68: R_TLS_IE	tdata_int_csect.*
  6c:	00 00 00 b0.*

0000000000000070 <tdata_int2_le>:
  70:	00 00 00 00.*
			70: R_TLS_LE	tdata_int_csect.*
  74:	00 00 00 b0.*

0000000000000078 <tbss_ext_gd>:
  78:	00 00 00 00.*
			78: R_TLS	tbss_ext.*
  7c:	00 00 00 b8.*

0000000000000080 <.tbss_ext_gd>:
	...
			80: R_TLSM	tbss_ext.*

0000000000000088 <tbss_ext_ld>:
  88:	00 00 00 00.*
			88: R_TLS_LD	tbss_ext.*
  8c:	00 00 00 b8.*

0000000000000090 <tbss_ext_ie>:
  90:	00 00 00 00.*
			90: R_TLS_IE	tbss_ext.*
  94:	00 00 00 b8.*

0000000000000098 <tbss_ext_le>:
  98:	00 00 00 00.*
			98: R_TLS_LE	tbss_ext.*
  9c:	00 00 00 b8.*

00000000000000a0 <_\$TLSML>:
	...
			a0: R_TLSML	_\$TLSML-0xa0

Disassembly of section .tdata:

00000000000000a8 <tdata_ext>:
  a8:	00 00 00 01 	.long 0x1

00000000000000ac <tdata_int_csect>:
  ac:	00 00 00 02 	.long 0x2
  b0:	00 00 00 03 	.long 0x3
  b4:.*

Disassembly of section .tbss:

00000000000000b8 <tbss_ext>:
	...
