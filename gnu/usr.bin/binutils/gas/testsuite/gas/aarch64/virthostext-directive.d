#objdump: -dr
#as: --defsym DIRECTIVE=1
#source: virthostext.s


.*:     file format .*


Disassembly of section \.text:

0+ <.*>:
   [0-9a-f]+:	d51d4007 	msr	spsr_el12, x7
   [0-9a-f]+:	d53d4007 	mrs	x7, spsr_el12
   [0-9a-f]+:	d51d4027 	msr	elr_el12, x7
   [0-9a-f]+:	d53d4027 	mrs	x7, elr_el12
  [0-9a-f]+:	d51d1007 	msr	sctlr_el12, x7
  [0-9a-f]+:	d53d1007 	mrs	x7, sctlr_el12
  [0-9a-f]+:	d51d1047 	msr	cpacr_el12, x7
  [0-9a-f]+:	d53d1047 	mrs	x7, cpacr_el12
  [0-9a-f]+:	d51c2027 	msr	ttbr1_el2, x7
  [0-9a-f]+:	d53c2027 	mrs	x7, ttbr1_el2
  [0-9a-f]+:	d51d2007 	msr	ttbr0_el12, x7
  [0-9a-f]+:	d53d2007 	mrs	x7, ttbr0_el12
  [0-9a-f]+:	d51d2027 	msr	ttbr1_el12, x7
  [0-9a-f]+:	d53d2027 	mrs	x7, ttbr1_el12
  [0-9a-f]+:	d51d2047 	msr	tcr_el12, x7
  [0-9a-f]+:	d53d2047 	mrs	x7, tcr_el12
  [0-9a-f]+:	d51d5107 	msr	afsr0_el12, x7
  [0-9a-f]+:	d53d5107 	mrs	x7, afsr0_el12
  [0-9a-f]+:	d51d5127 	msr	afsr1_el12, x7
  [0-9a-f]+:	d53d5127 	mrs	x7, afsr1_el12
  [0-9a-f]+:	d51d5207 	msr	esr_el12, x7
  [0-9a-f]+:	d53d5207 	mrs	x7, esr_el12
  [0-9a-f]+:	d51d6007 	msr	far_el12, x7
  [0-9a-f]+:	d53d6007 	mrs	x7, far_el12
  [0-9a-f]+:	d51da207 	msr	mair_el12, x7
  [0-9a-f]+:	d53da207 	mrs	x7, mair_el12
  [0-9a-f]+:	d51da307 	msr	amair_el12, x7
  [0-9a-f]+:	d53da307 	mrs	x7, amair_el12
  [0-9a-f]+:	d51dc007 	msr	vbar_el12, x7
  [0-9a-f]+:	d53dc007 	mrs	x7, vbar_el12
  [0-9a-f]+:	d51cd027 	msr	contextidr_el2, x7
  [0-9a-f]+:	d53cd027 	mrs	x7, contextidr_el2
  [0-9a-f]+:	d51dd027 	msr	contextidr_el12, x7
  [0-9a-f]+:	d53dd027 	mrs	x7, contextidr_el12
  [0-9a-f]+:	d51de107 	msr	cntkctl_el12, x7
  [0-9a-f]+:	d53de107 	mrs	x7, cntkctl_el12
  [0-9a-f]+:	d51de207 	msr	cntp_tval_el02, x7
  [0-9a-f]+:	d53de207 	mrs	x7, cntp_tval_el02
  [0-9a-f]+:	d51de227 	msr	cntp_ctl_el02, x7
  [0-9a-f]+:	d53de227 	mrs	x7, cntp_ctl_el02
  [0-9a-f]+:	d51de247 	msr	cntp_cval_el02, x7
  [0-9a-f]+:	d53de247 	mrs	x7, cntp_cval_el02
  [0-9a-f]+:	d51de307 	msr	cntv_tval_el02, x7
  [0-9a-f]+:	d53de307 	mrs	x7, cntv_tval_el02
  [0-9a-f]+:	d51de327 	msr	cntv_ctl_el02, x7
  [0-9a-f]+:	d53de327 	mrs	x7, cntv_ctl_el02
  [0-9a-f]+:	d51de347 	msr	cntv_cval_el02, x7
  [0-9a-f]+:	d53de347 	mrs	x7, cntv_cval_el02
  [0-9a-f]+:	d51ce307 	msr	cnthv_tval_el2, x7
  [0-9a-f]+:	d53ce307 	mrs	x7, cnthv_tval_el2
  [0-9a-f]+:	d51ce327 	msr	cnthv_ctl_el2, x7
  [0-9a-f]+:	d53ce327 	mrs	x7, cnthv_ctl_el2
  [0-9a-f]+:	d51ce347 	msr	cnthv_cval_el2, x7
  [0-9a-f]+:	d53ce347 	mrs	x7, cnthv_cval_el2
