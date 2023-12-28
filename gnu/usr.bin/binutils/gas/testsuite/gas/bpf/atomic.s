        # Test for eBPF ADDW and ADDDW instructions
        .text
        xadddw	[%r1+0x1eef], %r2
        xaddw	[%r1+0x1eef], %r2
        
