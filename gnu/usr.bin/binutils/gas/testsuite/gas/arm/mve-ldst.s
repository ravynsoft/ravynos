.arch armv8.1-m.main
.arch_extension dsp
.arch_extension mve
.syntax unified
.thumb
.text

.irp op0, r0, r1, r2, r4, r7, r8, r10, r12, r14
vldmia.32 \op0!,{s16}
vldmdb.32 \op0!,{s16}
vstmia.32 \op0!,{s16}
vstmdb.32 \op0!,{s16}
.endr
