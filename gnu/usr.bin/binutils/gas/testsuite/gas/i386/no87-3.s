# Test .arch [.x87|.nox87]
	.text
	.arch generic32
	fneni
	.arch .8087
	fneni
	fsetpm
	.arch .287
	fsetpm
	fprem1
	.arch .387
	fprem1
	fcomi
	.arch .687
	fcomi
	.arch .no687
	fcomi
	fprem1
	.arch .no387
	fprem1
	fsetpm
	.arch .no287
	fsetpm
	fneni
	.arch .no87
	fneni
	.p2align 4
