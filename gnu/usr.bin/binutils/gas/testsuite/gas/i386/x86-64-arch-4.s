# Test -march=
	.text
#INVLPGB
        invlpgb
#TLBSYNC
        tlbsync
#VPCLMUL 256 datapath
        vpclmulqdq      $0xab, %ymm8, %ymm9, %ymm10
        vpclmulqdq      $123, 0x124(%rax,%r14,8), %ymm9, %ymm10
        vpclmulqdq      $123, 4064(%rdx), %ymm9, %ymm10

        vpclmulhqhqdq   %ymm10, %ymm11, %ymm12
        vpclmulhqlqdq   %ymm11, %ymm12, %ymm13
        vpclmullqhqdq   %ymm12, %ymm13, %ymm14
        vpclmullqlqdq   %ymm13, %ymm14, %ymm15
#VAES
        vaesenc %ymm4,%ymm6,%ymm2
        vaesenc (%rcx),%ymm6,%ymm7
        vaesenclast %ymm4,%ymm6,%ymm2
        vaesenclast (%rcx),%ymm6,%ymm7
        vaesdec %ymm4,%ymm6,%ymm2
        vaesdec (%rcx),%ymm6,%ymm7
        vaesdeclast %ymm4,%ymm6,%ymm2
        vaesdeclast (%rcx),%ymm6,%ymm7
#SNP - Secure Nested Paging support
        psmash
        pvalidate
        rmpupdate
        rmpadjust
#INVPCID
        invpcid (%rax), %rdx
#OSPKE
        rdpkru
        wrpkru
