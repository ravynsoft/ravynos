;;; Test basic JLI relocs and constructs.

	.cpu em4

	jli_s	__jli.foo	; Generates R_ARC_JLI_SECTOFF

	.section	.jlitab,"axG",%progbits,jlitab.foo,comdat
        .align  4
__jli.foo:
        .weak   __jli.foo
        b       @foo
