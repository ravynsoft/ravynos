# Tests for the eBPF JUMP32 instructions
        .text
        ja 2f
        add %r1,%r1
1:      jeq32 %r3,3,2f
        jeq32 %r3,%r4,2f
2:      jge32 %r3,3,1b
        jge32 %r3,%r4,1b
1:      jlt32 %r3,3,1f
        jlt32 %r3,%r4,1f
1:      jle32 %r3,3,1f
        jle32 %r3,%r4,1f
1:      jset32 %r3,3,1f
        jset32 %r3,%r4,1f
1:      jne32 %r3,3,1f
        jne32 %r3,%r4,1f
1:      jsgt32 %r3,3,1f
        jsgt32 %r3,%r4,1f
1:      jsge32 %r3,3,1f
        jsge32 %r3,%r4,1f
1:      jslt32 %r3,3,1f
        jslt32 %r3,%r4,1f
1:      jsle32 %r3,3,1f
        jsle32 %r3,%r4,1f
1:      
