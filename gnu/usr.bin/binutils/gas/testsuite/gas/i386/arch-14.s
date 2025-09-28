# Test -march=
	.text

#INVLPGB
        invlpgb
#TLBSYNC
        tlbsync
#SNP - Secure Nested Paging support
        pvalidate
#OSPKE
        rdpkru
        wrpkru
