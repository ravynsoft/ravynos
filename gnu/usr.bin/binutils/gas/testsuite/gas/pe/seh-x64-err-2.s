	.file	"t2.c"
	.text

# missing endproc
	.seh_proc test_missing_endproc
test_missing_endproc:
        .seh_setframe   %rbp, 0
        .seh_endprologue
	ret
