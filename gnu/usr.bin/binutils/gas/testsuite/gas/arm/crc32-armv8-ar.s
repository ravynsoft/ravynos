.section .text
.syntax unified
.arm
crc32b r0, r1, r2
crc32h r0, r1, r2
crc32w r0, r1, r2
crc32cb r0, r1, r2
crc32ch r0, r1, r2
crc32cw r0, r1, r2

.thumb
crc32b r0, r1, r2
crc32h r0, r1, r2
crc32w r0, r1, r2
crc32cb r0, r1, r2
crc32ch r0, r1, r2
crc32cw r0, r1, r2

.arm
crc32b sp, r1, r2
crc32h r11, sp, r2
crc32w r0, r1, sp
crc32cb r9, sp, r2
crc32ch sp, r1, r8
crc32cw r10, r1, sp

.thumb
crc32b r12, r1, sp
crc32h r10, sp, r2
crc32w sp, r1, r7
crc32cb r0, sp, r2
crc32ch r0, r5, sp
crc32cw sp, r1, r9
