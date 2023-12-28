#name: GB(r28) relative .data section references
#source: data.s
#source: data_section.s
#ld: -e __start
#objdump: -dr

.*:     file format .*


Disassembly of section .text:

[0-9a-f]+ <__start>:
\s*[0-9a-f]+:	cd400004 	lrs.b      	r10, \[0x4\]	// the offset is based on .data
\s*[0-9a-f]+:	cd440002 	lrs.h      	r10, \[0x2\]	// the offset is based on .data
\s*[0-9a-f]+:	cd480001 	lrs.w      	r10, \[0x1\]	// the offset is based on .data
\s*[0-9a-f]+:	cd700004 	srs.b      	r11, \[0x4\]	// the offset is based on .data
\s*[0-9a-f]+:	cd740002 	srs.h      	r11, \[0x2\]	// the offset is based on .data
\s*[0-9a-f]+:	cd780001 	srs.w      	r11, \[0x1\]	// the offset is based on .data
