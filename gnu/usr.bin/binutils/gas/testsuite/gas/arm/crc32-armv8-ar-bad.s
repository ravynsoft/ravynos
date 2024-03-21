.section .text
.syntax unified
.arm
crc32b r15, r1, r2
crc32h r0, r15, r2
crc32w r0, r1, r15
crc32cb r0, r15, r2
crc32ch r15, r1, r2
crc32cw r0, r15, r2

.thumb
crc32b r15, r1, r2
crc32h r0, r15, r2
crc32w r0, r1, r15
crc32cb r0, r15, r2
crc32ch r15, r1, r2
crc32cw r0, r15, r2
