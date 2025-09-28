	.file	"t1.c"
	.text
#seh pseudos out of seh_proc block

	.seh_endproc
	.seh_stackalloc 8
	.seh_setframe   %rbp, 0
	.seh_endprologue
	.seh_pushreg	%rbp
	.seh_savereg	%rbp
	.seh_savexmm	%xmm1
	.seh_handler	dummy_handler
	.seh_handler	dummy_handler, @unwind
	.seh_handler	dummy_handler, @except
	.seh_handler	dummy_handler, @unwind,@except
	.seh_handlerdata
	.long 0
	.text
	.seh_proc

	.seh_proc test_foreign_directives
test_foreign_directives:
	.seh_eh
	.seh_32
	.seh_no32
	.long 0
	.seh_endproc

# test for wrong segment pseudos.
	.seh_proc test_wrong_segment
test_wrong_segment:
	.data
        .seh_stackalloc 8
        .seh_setframe   %rbp, 0
        .seh_endprologue
        .seh_pushreg    %rbp
        .seh_savereg    %rbp
        .seh_savexmm    %xmm1
	.seh_endproc

