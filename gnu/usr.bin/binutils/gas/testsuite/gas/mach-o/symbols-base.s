# baseline symbols in sections.
Lzt0:	.space 1
	.globl ztg0
ztg0:	.space 1
Lmt0:	.space 1
	.globl mtg0
mtg0:	.space 1
Lat0:	.space 1
	.globl atg0
atg0:	.space 1

	.long	_zut0
	.long	_mut0
	.long	_aut0

	.comm zcommon0, 10, 3
	.comm mcommon0, 10, 3
	.comm acommon0, 10, 3

	.data
Lzd0:	.space 1
	.globl zdg0
zdg0:	.space 1
Lmd0:	.space 1
	.globl mdg0
mdg0:	.space 1
adg0:	.space 1
	.globl adg0
Lad0:	.space 1

	.long	_zud0
	.long	_mud0
	.long	_aud0

	.lcomm zlcomm0, 5, 1
	.lcomm mlcomm0, 5, 1
	.lcomm alcomm0, 5, 1

	.section __HERE,__there
Lzs0:	.space 1
	.globl zsg0
zsg0:	.space 1
Lms0:	.space 1
	.globl msg0
msg0:	.space 1
Las0:	.space 1
asg0:	.space 1
	.globl asg0

	.long	_zus0
	.long	_mus0
	.long	_aus0

	.text
Lzt1:	.space 1
	.globl ztg1
ztg1:	.space 1
	.globl mtg1
mtg1:	.space 1
Lmt1:	.space 1
atg1:	.space 1
	.globl atg1
Lat1:	.space 1

	.comm zcommon1, 10, 3
	.comm mcommon1, 10, 3
	.comm acommon1, 10, 3

	.data
Lzd1:	.space 1
	.globl zdg1, mdg1, adg1
zdg1:	.space 1
Lmd1:	.space 1
mdg1:	.space 1
adg1:	.space 1
Lad1:	.space 1

	.long	_zud1
	.long	_mud1
	.long	_aud1

	.lcomm zlcomm1, 5, 1
	.lcomm mlcomm1, 5, 1
	.lcomm alcomm1, 5, 1

	.section __HERE,__there
	.long	_zus1
Lzs1:	.space 1
zsg1:	.space 1
	.long	_mus1
msg1:	.space 1
asg1:	.space 1
	.globl zsg1, msg1, asg1
Lms1:	.space 1
Las1:	.space 1

	.long	_aus1
