        # Test for eBPF ADDW and ADDDW pseudo-C instructions
        .text
	lock *(u64 *)(r1 + 7919) += r2
	lock *(u32 *)(r1 + 7919) += r2
