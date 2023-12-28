.*:     file format elf32-.*


Disassembly of section .text:

00008000 <arm>:
    8000:	eb03dffe 	bl	100000 <foo>
    8004:	ea03dffd 	b	100000 <foo>
    8008:	fa03dffc 	blx	100000 <foo>
    800c:	eb03dffb 	bl	100000 <foo>
00008010 <thumb>:
    8010:	f0f7 fff6 	bl	100000 <foo>
    8014:	f0f7 bff4 	b\.w	100000 <foo>
    8018:	f0f7 eff2 	blx	100000 <foo>
