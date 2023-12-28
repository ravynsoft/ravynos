.*:     file format .*


Disassembly of section .text:

01800054 <sub1>:
 1800054:	00 04       	se_blr

01800056 <sub2>:
 1800056:	00 04       	se_blr

01800058 <vle_reloc>:
 1800058:	e8 fe       	se_b    1800054 <sub1>
 180005a:	e9 fd       	se_bl   1800054 <sub1>
 180005c:	e1 fd       	se_ble  1800056 <sub2>
 180005e:	e6 fc       	se_beq  1800056 <sub2>
 1800060:	78 00 00 10 	e_b     1800070 <sub3>
 1800064:	78 00 00 0f 	e_bl    1800072 <sub4>
 1800068:	7a 05 00 0c 	e_ble   cr1,1800074 <sub5>
 180006c:	7a 1a 00 09 	e_beql  cr2,1800074 <sub5>

01800070 <sub3>:
 1800070:	00 04       	se_blr

01800072 <sub4>:
 1800072:	00 04       	se_blr

01800074 <sub5>:
 1800074:	00 04       	se_blr
