        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .text
        .align  1
        .thumb
        .thumb_func
        .global _start
_start:
        @ All LDM treatments for word access <= 8 go through the same
        @ replication code, but decoding may vary
        ldm.w  r9, {r1-r8}
        ldm.w  r9!, {r1-r8}
        ldmdb.w r9, {r1-r8}
        ldmdb.w r9!, {r1-r8}
        pop {r1-r8}

        @ All VLDM treatments for word access <= 8 go through the same
        @ replication code, but decoding may vary
        vldm r9, {s1-s8}
        vldm r6!, {s9-s16}
        vpop {s1-s8}
        vldm r9, {d1-d4}
        vldm r6!, {d8-d11}
        vpop {d1-d4}
