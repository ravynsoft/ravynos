#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d50c8027 	tlbi	ipas2e1is, x7
   4:	d50c80a7 	tlbi	ipas2le1is, x7
   8:	d508831f 	tlbi	vmalle1is
   c:	d50c831f 	tlbi	alle2is
  10:	d50e831f 	tlbi	alle3is
  14:	d5088327 	tlbi	vae1is, x7
  18:	d50c8327 	tlbi	vae2is, x7
  1c:	d50e8327 	tlbi	vae3is, x7
  20:	d5088347 	tlbi	aside1is, x7
  24:	d5088367 	tlbi	vaae1is, x7
  28:	d50c839f 	tlbi	alle1is
  2c:	d50883a7 	tlbi	vale1is, x7
  30:	d50c83a7 	tlbi	vale2is, x7
  34:	d50e83a7 	tlbi	vale3is, x7
  38:	d50c83df 	tlbi	vmalls12e1is
  3c:	d50883e7 	tlbi	vaale1is, x7
  40:	d50c8427 	tlbi	ipas2e1, x7
  44:	d50c84a7 	tlbi	ipas2le1, x7
  48:	d508871f 	tlbi	vmalle1
  4c:	d50c871f 	tlbi	alle2
  50:	d50e871f 	tlbi	alle3
  54:	d5088727 	tlbi	vae1, x7
  58:	d50c8727 	tlbi	vae2, x7
  5c:	d50e8727 	tlbi	vae3, x7
  60:	d5088747 	tlbi	aside1, x7
  64:	d5088767 	tlbi	vaae1, x7
  68:	d50c879f 	tlbi	alle1
  6c:	d50887a7 	tlbi	vale1, x7
  70:	d50c87a7 	tlbi	vale2, x7
  74:	d50e87a7 	tlbi	vale3, x7
  78:	d50c87df 	tlbi	vmalls12e1
  7c:	d50887e7 	tlbi	vaale1, x7
