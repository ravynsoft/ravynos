# Tests for the JUMP instructions
        .text
        ja 2f
        add %r1,%r1
1:      jeq %r3,3,2f
        jeq %r3,%r4,2f
2:      jge %r3,3,1b
        jge %r3,%r4,1b
1:      jlt %r3,3,1f
        jlt %r3,%r4,1f
1:      jle %r3,3,1f
        jle %r3,%r4,1f
1:      jset %r3,3,1f
        jset %r3,%r4,1f
1:      jne %r3,3,1f
        jne %r3,%r4,1f
1:      jsgt %r3,3,1f
        jsgt %r3,%r4,1f
1:      jsge %r3,3,1f
        jsge %r3,%r4,1f
1:      jslt %r3,3,1f
        jslt %r3,%r4,1f
1:      jsle %r3,3,1f
        jsle %r3,%r4,1f
1:      
