#objdump: -dr
#name: tls

.*: +file format .*

Disassembly of section \.text:

00000000 <.text>:
   0:	03180000 	          ADD       D1Ar1,D1Ar1,#0
			0: R_METAG_TLS_GD	_a
   4:	03180000 	          ADD       D1Ar1,D1Ar1,#0
			4: R_METAG_TLS_LDM	_b
   8:	02000001 	          ADDT      D0Re0,D0Re0,#0
			8: R_METAG_TLS_LDO_HI16	_b
   c:	02000000 	          ADD       D0Re0,D0Re0,#0
			c: R_METAG_TLS_LDO_LO16	_b
  10:	a720000d 	          GETD      D0FrT,\[A1LbP\]
			10: R_METAG_TLS_IE	_b
  14:	02000005 	          MOVT      D0Re0,#0
			14: R_METAG_TLS_IENONPIC_HI16	_b
  18:	02000000 	          ADD       D0Re0,D0Re0,#0
			18: R_METAG_TLS_IENONPIC_LO16	_b
  1c:	02000001 	          ADDT      D0Re0,D0Re0,#0
			1c: R_METAG_TLS_LE_HI16	_b
  20:	02000000 	          ADD       D0Re0,D0Re0,#0
			20: R_METAG_TLS_LE_LO16	_b
