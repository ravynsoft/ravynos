#name: RME System registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:

[^:]*:	d53e60a0 	mrs	x0, mfar_el3
[^:]*:	d53e21c0 	mrs	x0, gpccr_el3
[^:]*:	d53e2180 	mrs	x0, gptbr_el3
[^:]*:	d51e60a0 	msr	mfar_el3, x0
[^:]*:	d51e21c0 	msr	gpccr_el3, x0
[^:]*:	d51e2180 	msr	gptbr_el3, x0
[^:]*:	d50e7e20 	dc	cipapa, x0
[^:]*:	d50e7ea0 	dc	cigdpapa, x0
[^:]*:	d50e8460 	tlbi	rpaos, x0
[^:]*:	d50e84e0 	tlbi	rpalos, x0
[^:]*:	d50e819f 	tlbi	paallos
[^:]*:	d50e879f 	tlbi	paall
