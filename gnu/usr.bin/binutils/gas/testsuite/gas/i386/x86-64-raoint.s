# Check 64bit RAO_INT instructions

	.allow_index_reg
	.text
_start:
        aadd    %rdx, (%rax)     #RAO-INT
        aand    %rdx, (%rax)     #RAO-INT
        aor     %rdx, (%rax)     #RAO-INT
        axor    %rdx, (%rax)     #RAO-INT

.intel_syntax noprefix
        aadd    QWORD PTR [rax], %rdx    #RAO-INT
        aand    QWORD PTR [rax], %rdx    #RAO-INT
        aor     QWORD PTR [rax], %rdx    #RAO-INT
        axor    QWORD PTR [rax], %rdx    #RAO-INT
