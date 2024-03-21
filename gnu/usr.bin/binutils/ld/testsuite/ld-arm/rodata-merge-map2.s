@ This file contains the 2nd contribution, which is expected to
@ be fully merged into the 1st contribution (from
@ rodata-merge-map1.s), and generate no mapping symbol (which
@ would otherwise be converted in a symbol table entry with no
@ name).

        .section        .rodata.str1.1,"aMS",%progbits,1
.LC0:
        .string "world"
