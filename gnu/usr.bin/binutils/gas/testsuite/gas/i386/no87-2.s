# Test -march=...+no87
	.text
	fninit
	.arch .287
	fninit
	fsincos
	.arch .387
	fsincos
	fcomi
	.arch i686
	fcomi
	fisttpl (%eax)
	.arch prescott
	fisttpl (%eax)
