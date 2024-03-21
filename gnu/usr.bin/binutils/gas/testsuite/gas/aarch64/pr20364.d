# Check that ".align <size>, <fill>" does not set the mapping state to DATA, causing unnecessary frag generation.
#name: PR20364 
#objdump: -d

.*:     file format .*

Disassembly of section \.vectors:

0+000 <.*>:
   0:	d2800000 	mov	x0, #0x0                   	// #0
   4:	94000000 	bl	0 <plat_report_exception>
   8:	17fffffe 	b	0 <bl1_exceptions>
#pass
