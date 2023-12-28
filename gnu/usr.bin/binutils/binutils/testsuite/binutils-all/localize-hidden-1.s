	.globl		Gdefault
	.globl		Ghidden
	.globl		Ginternal
	.globl		Gprotected

	.weak		Wdefault
	.weak		Whidden
	.weak		Winternal
	.weak		Wprotected

	.hidden		Lhidden
	.hidden		Ghidden
	.hidden		Whidden

	.internal	Linternal
	.internal	Ginternal
	.internal	Winternal

	.protected	Lprotected
	.protected	Gprotected
	.protected	Wprotected

	Ldefault == 0x1100
	Lhidden == 0x1200
	Linternal == 0x1300
	Lprotected == 0x1400

	Gdefault == 0x2100
	Ghidden == 0x2200
	Ginternal == 0x2300
	Gprotected == 0x2400

	Wdefault == 0x3100
	Whidden == 0x3200
	Winternal == 0x3300
	Wprotected == 0x3400
