#as: -march=loongson3a -mabi=o64
#objdump: -M reg-names=numeric -dr
#name: Loongson-3A tests 2

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <.text>:
.*:	716c0026 	gsle	\$11,\$12
.*:	71ae0027 	gsgt	\$13,\$14
.*:	c8622010 	gslble	\$2,\$3,\$4
.*:	c8c53811 	gslbgt	\$5,\$6,\$7
.*:	c9285012 	gslhle	\$8,\$9,\$10
.*:	c98b6813 	gslhgt	\$11,\$12,\$13
.*:	c9ee8014 	gslwle	\$14,\$15,\$16
.*:	ca519815 	gslwgt	\$17,\$18,\$19
.*:	cab4b016 	gsldle	\$20,\$21,\$22
.*:	cb17c817 	gsldgt	\$23,\$24,\$25
.*:	e8622010 	gssble	\$2,\$3,\$4
.*:	e8c53811 	gssbgt	\$5,\$6,\$7
.*:	e9285012 	gsshle	\$8,\$9,\$10
.*:	e98b6813 	gsshgt	\$11,\$12,\$13
.*:	e9ee8014 	gsswle	\$14,\$15,\$16
.*:	ea519815 	gsswgt	\$17,\$18,\$19
.*:	eab4b016 	gssdle	\$20,\$21,\$22
.*:	eb17c817 	gssdgt	\$23,\$24,\$25
.*:	c8401818 	gslwlec1	\$f0,\$2,\$3
.*:	c8812819 	gslwgtc1	\$f1,\$4,\$5
.*:	c8c2381a 	gsldlec1	\$f2,\$6,\$7
.*:	c903481b 	gsldgtc1	\$f3,\$8,\$9
.*:	e944581c 	gsswlec1	\$f4,\$10,\$11
.*:	e985681d 	gsswgtc1	\$f5,\$12,\$13
.*:	e9c6781e 	gssdlec1	\$f6,\$14,\$15
.*:	ea07881f 	gssdgtc1	\$f7,\$16,\$17
.*:	ca480004 	gslwlc1	\$f8,0\(\$18\)
.*:	ca690045 	gslwrc1	\$f9,1\(\$19\)
.*:	ca8a0086 	gsldlc1	\$f10,2\(\$20\)
.*:	caab00c7 	gsldrc1	\$f11,3\(\$21\)
.*:	eacc0104 	gsswlc1	\$f12,4\(\$22\)
.*:	eaed0145 	gsswrc1	\$f13,5\(\$23\)
.*:	eb0e0186 	gssdlc1	\$f14,6\(\$24\)
.*:	eb2f01c7 	gssdrc1	\$f15,7\(\$25\)
.*:	d8622000 	gslbx	\$2,0\(\$3,\$4\)
.*:	d8c53ff9 	gslhx	\$5,-1\(\$6,\$7\)
.*:	d92857f2 	gslwx	\$8,-2\(\$9,\$10\)
.*:	d98b6feb 	gsldx	\$11,-3\(\$12,\$13\)
.*:	f9ee87e0 	gssbx	\$14,-4\(\$15,\$16\)
.*:	fa519fd9 	gsshx	\$17,-5\(\$18,\$19\)
.*:	fab4b7d2 	gsswx	\$20,-6\(\$21,\$22\)
.*:	fb17cfcb 	gssdx	\$23,-7\(\$24,\$25\)
.*:	d8501bfe 	gslwxc1	\$f16,127\(\$2,\$3\)
.*:	d8912c07 	gsldxc1	\$f17,-128\(\$4,\$5\)
.*:	f8d23bfe 	gsswxc1	\$f18,127\(\$6,\$7\)
.*:	f9134c07 	gssdxc1	\$f19,-128\(\$8,\$9\)
.*:	c98b3fea 	gslq	\$10,\$11,4080\(\$12\)
.*:	e9ee402d 	gssq	\$13,\$14,-4096\(\$15\)
.*:	ca15bff4 	gslqc1	\$f20,\$f21,4080\(\$16\)
.*:	ea37c036 	gssqc1	\$f22,\$f23,-4096\(\$17\)
#pass
