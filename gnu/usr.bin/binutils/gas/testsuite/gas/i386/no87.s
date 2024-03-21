# Test .arch .no87
	.text
	.arch generic32
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
	.arch .no87
	fcomi
	fisttpl (%eax)
