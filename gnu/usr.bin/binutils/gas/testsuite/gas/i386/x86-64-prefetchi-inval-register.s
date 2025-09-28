.text
        #prefetchit0 (%rcx) PREFETCHIT0/1 apply without RIP-relative addressing, should stay NOPs.
        .insn 0x0f18/7, (%rcx)

        #prefetchit1 (%rcx) PREFETCHIT1/1 apply without RIP-relative addressing, should stay NOPs.
        .insn 0x0f18/6, (%rcx)
