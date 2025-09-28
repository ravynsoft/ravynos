.syntax unified
.thumb
.global _start
_start:
add.w r0, r1, r2

.section .after1
.word 0xeb010002

.section .after2
add.w r0, r1, r2

.section .after3
add.w r2, r1, r0

.section .after4
.word 0xeb010002

.section .after5
.word 0xeb010002
