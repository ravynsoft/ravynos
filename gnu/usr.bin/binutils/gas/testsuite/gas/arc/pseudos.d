#as:
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0]+ <.text>:
   0:	1cfc b008           	st.aw	r0,\[sp,-4\]
   4:	1404 3401           	ld.ab	r1,\[sp,4\]
   8:	0901 0002           	brlt.*	r1,r0,0	;0x8
   c:	08fd 8013           	brge.*	r0,0,-4	;0x8
  10:	0ef9 f002 0000 003f 	brlt.*	0x3f,r0,-8	;0x8
  18:	0ef1 f002 ffff fffe 	brlt.*	0xfffffffe,r0,-16	;0x8
  20:	08e9 8f82 ffff fffe 	brlt.*	r0,0xfffffffe,-24	;0x8
  28:	0ee1 f013 ffff fffe 	brge.*	0xfffffffe,0,-32	;0x8
  30:	0ed9 ffd3 ffff fffe 	brge.*	0xfffffffe,0x3f,-40	;0x8
  38:	09d1 8044           	brlo.*	r1,r1,-48	;0x8
  3c:	09cd 8015           	brhs.*	r1,0,-52	;0x8
  40:	0ec9 f044 0000 003f 	brlo.*	0x3f,r1,-56	;0x8
  48:	0ec1 f044 ffff fffe 	brlo.*	0xfffffffe,r1,-64	;0x8
  50:	08b9 8f84 ffff fffe 	brlo.*	r0,0xfffffffe,-72	;0x8
  58:	0eb1 f015 ffff fffe 	brhs.*	0xfffffffe,0,-80	;0x8
  60:	0ea9 ffd5 ffff fffe 	brhs.*	0xfffffffe,0x3f,-88	;0x8
  68:	09a1 8043           	brge.*	r1,r1,-96	;0x8
  6c:	099d 8012           	brlt.*	r1,0,-100	;0x8
  70:	0e99 f043 0000 003f 	brge.*	0x3f,r1,-104	;0x8
  78:	0e91 f043 ffff fffe 	brge.*	0xfffffffe,r1,-112	;0x8
  80:	0889 8f83 ffff fffe 	brge.*	r0,0xfffffffe,-120	;0x8
  88:	0e81 f012 ffff fffe 	brlt.*	0xfffffffe,0,-128	;0x8
  90:	0e79 ffd2 ffff fffe 	brlt.*	0xfffffffe,0x3f,-136	;0x8
  98:	0971 8043           	brge.*	r1,r1,-144	;0x8
  9c:	096d 8012           	brlt.*	r1,0,-148	;0x8
  a0:	0e69 f043 0000 003f 	brge.*	0x3f,r1,-152	;0x8
  a8:	0e61 f043 ffff fffe 	brge.*	0xfffffffe,r1,-160	;0x8
  b0:	0859 8f83 ffff fffe 	brge.*	r0,0xfffffffe,-168	;0x8
  b8:	0e51 f012 ffff fffe 	brlt.*	0xfffffffe,0,-176	;0x8
  c0:	0e49 ffd2 ffff fffe 	brlt.*	0xfffffffe,0x3f,-184	;0x8
