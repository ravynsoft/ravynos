        .text
        .global __start
__start:
foo:
        ld  r0, [pcl, foo@gotpc]
