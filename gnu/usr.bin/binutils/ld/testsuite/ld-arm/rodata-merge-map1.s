@ Test to ensure that no nameless mapping symbol is inserted
@ within a merged section.
@ This file contains the 1st contribution, which is expected to
@ generate a $d symbol at its beginning.

        .section        .rodata.str1.1,"aMS",%progbits,1
.LC0:
        .string "Hello world"
