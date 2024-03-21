	.text
	.global foo
foo:
	jmp local
	jmp hidden_def
	jmp global_def
	jmp global_def@PLT
	jmp weak_def
	jmp weak_hidden_undef
	jmp weak_hidden_def
	jmp hidden_undef

	.hidden hidden_undef

	.global hidden_def
	.hidden hidden_def
hidden_def:
	ret

	.global weak_hidden_def
	.hidden weak_hidden_def
	.weak weak_hidden_def
weak_hidden_def:
	ret

	.global global_def
global_def:
	ret

	.global weak_def
	.weak weak_def
weak_def:
	ret

local:
	ret

	.global weak_hidden_undef
	.weak weak_hidden_undef
	.hidden weak_hidden_undef
