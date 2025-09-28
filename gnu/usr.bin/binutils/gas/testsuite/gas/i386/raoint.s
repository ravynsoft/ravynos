# Check 32bit RAO-INT instructions

	.allow_index_reg
	.text
_start:
        aadd    %edx, (%eax)     #RAO-INT
        aand    %edx, (%eax)     #RAO-INT
        aor     %edx, (%eax)     #RAO-INT
        axor    %edx, (%eax)     #RAO-INT

.intel_syntax noprefix
        aadd    DWORD PTR [eax], %edx    #RAO-INT
        aand    DWORD PTR [eax], %edx    #RAO-INT
        aor     DWORD PTR [eax], %edx    #RAO-INT
        axor    DWORD PTR [eax], %edx    #RAO-INT
