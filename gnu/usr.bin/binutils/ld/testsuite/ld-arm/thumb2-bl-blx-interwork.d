
.*:     file format .*


Disassembly of section \.text:

00008000 <_start>:
    8000:	f000 e81a 	blx	8038 <arm0>
    8004:	f000 e81e 	blx	8044 <arm4>
    8008:	bf00      	nop
    800a:	f000 e816 	blx	8038 <arm0>
    800e:	f000 e81a 	blx	8044 <arm4>
    8012:	bf00      	nop
    8014:	f000 f818 	bl	8048 <thumb0>
    8018:	f000 f81b 	bl	8052 <thumb2>
    801c:	f000 f81e 	bl	805c <thumb4>
    8020:	f000 f821 	bl	8066 <thumb6>
    8024:	bf00      	nop
    8026:	f000 f80f 	bl	8048 <thumb0>
    802a:	f000 f812 	bl	8052 <thumb2>
    802e:	f000 f815 	bl	805c <thumb4>
    8032:	f000 f818 	bl	8066 <thumb6>
    8036:	bf00      	nop

00008038 <arm0>:
    8038:	e12fff1e 	bx	lr
    803c:	e320f000 	nop	\{0\}
    8040:	e320f000 	nop	\{0\}

00008044 <arm4>:
    8044:	e12fff1e 	bx	lr

00008048 <thumb0>:
    8048:	4770      	bx	lr
    804a:	bf00      	nop
    804c:	f3af 8000 	nop\.w
    8050:	bf00      	nop

00008052 <thumb2>:
    8052:	4770      	bx	lr
    8054:	f3af 8000 	nop\.w
    8058:	bf00      	nop
    805a:	bf00      	nop

0000805c <thumb4>:
    805c:	4770      	bx	lr
    805e:	bf00      	nop
    8060:	bf00      	nop
    8062:	bf00      	nop
    8064:	bf00      	nop

00008066 <thumb6>:
    8066:	4770      	bx	lr

00008068 <backwards>:
    8068:	f7ff efe6 	blx	8038 <arm0>
    806c:	f7ff efea 	blx	8044 <arm4>
    8070:	bf00      	nop
    8072:	f7ff efe2 	blx	8038 <arm0>
    8076:	f7ff efe6 	blx	8044 <arm4>
    807a:	bf00      	nop
    807c:	f7ff ffe4 	bl	8048 <thumb0>
    8080:	f7ff ffe7 	bl	8052 <thumb2>
    8084:	f7ff ffea 	bl	805c <thumb4>
    8088:	f7ff ffed 	bl	8066 <thumb6>
    808c:	bf00      	nop
    808e:	f7ff ffdb 	bl	8048 <thumb0>
    8092:	f7ff ffde 	bl	8052 <thumb2>
    8096:	f7ff ffe1 	bl	805c <thumb4>
    809a:	f7ff ffe4 	bl	8066 <thumb6>
    809e:	bf00      	nop
