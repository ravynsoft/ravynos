#as:
#objdump: -dw
#name: x86_64 AVX-NE-CONVERT insns
#source: x86-64-avx-ne-convert.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 a2 7a b1 b4 f5 00 00 00 10\s+vbcstnebf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a b1 31\s+vbcstnebf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b1 b1 fe 00 00 00\s+vbcstnebf162ps 0xfe\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b1 b2 00 ff ff ff\s+vbcstnebf162ps -0x100\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7e b1 b4 f5 00 00 00 10\s+vbcstnebf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7e b1 31\s+vbcstnebf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b1 b1 fe 00 00 00\s+vbcstnebf162ps 0xfe\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b1 b2 00 ff ff ff\s+vbcstnebf162ps -0x100\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 79 b1 b4 f5 00 00 00 10\s+vbcstnesh2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 79 b1 31\s+vbcstnesh2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b1 b1 fe 00 00 00\s+vbcstnesh2ps 0xfe\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b1 b2 00 ff ff ff\s+vbcstnesh2ps -0x100\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7d b1 b4 f5 00 00 00 10\s+vbcstnesh2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7d b1 31\s+vbcstnesh2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b1 b1 fe 00 00 00\s+vbcstnesh2ps 0xfe\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b1 b2 00 ff ff ff\s+vbcstnesh2ps -0x100\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 7a b0 b4 f5 00 00 00 10\s+vcvtneebf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a b0 31\s+vcvtneebf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b0 b1 f0 07 00 00\s+vcvtneebf162ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b0 b2 00 f8 ff ff\s+vcvtneebf162ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7e b0 b4 f5 00 00 00 10\s+vcvtneebf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7e b0 31\s+vcvtneebf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b0 b1 e0 0f 00 00\s+vcvtneebf162ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b0 b2 00 f0 ff ff\s+vcvtneebf162ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 79 b0 b4 f5 00 00 00 10\s+vcvtneeph2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 79 b0 31\s+vcvtneeph2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b0 b1 f0 07 00 00\s+vcvtneeph2ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b0 b2 00 f8 ff ff\s+vcvtneeph2ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7d b0 b4 f5 00 00 00 10\s+vcvtneeph2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7d b0 31\s+vcvtneeph2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b0 b1 e0 0f 00 00\s+vcvtneeph2ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b0 b2 00 f0 ff ff\s+vcvtneeph2ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 7b b0 b4 f5 00 00 00 10\s+vcvtneobf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7b b0 31\s+vcvtneobf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7b b0 b1 f0 07 00 00\s+vcvtneobf162ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7b b0 b2 00 f8 ff ff\s+vcvtneobf162ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7f b0 b4 f5 00 00 00 10\s+vcvtneobf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7f b0 31\s+vcvtneobf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7f b0 b1 e0 0f 00 00\s+vcvtneobf162ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7f b0 b2 00 f0 ff ff\s+vcvtneobf162ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 78 b0 b4 f5 00 00 00 10\s+vcvtneoph2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 78 b0 31\s+vcvtneoph2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 78 b0 b1 f0 07 00 00\s+vcvtneoph2ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 78 b0 b2 00 f8 ff ff\s+vcvtneoph2ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7c b0 b4 f5 00 00 00 10\s+vcvtneoph2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7c b0 31\s+vcvtneoph2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7c b0 b1 e0 0f 00 00\s+vcvtneoph2ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7c b0 b2 00 f0 ff ff\s+vcvtneoph2ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 f5\s+vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 f5\s+vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 f5\s+\{vex\} vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 f5\s+\{vex\} vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 f5\s+vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 f5\s+vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 f5\s+\{vex\} vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 f5\s+\{vex\} vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*62 b2 7e 08 72 b4 f5 00 00 00 10\s+vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*62 b2 7e 08 72 b4 f5 00 00 00 10\s+vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7a 72 b4 f5 00 00 00 10\s+\{vex\} vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7a 72 b4 f5 00 00 00 10\s+\{vex\} vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*62 d2 7e 08 72 31\s+vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*62 d2 7e 08 72 31\s+vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a 72 31\s+\{vex\} vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a 72 31\s+\{vex\} vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 71 7f\s+vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 71 7f\s+vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b1 f0 07 00 00\s+\{vex\} vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b1 f0 07 00 00\s+\{vex\} vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 72 80\s+vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 72 80\s+vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b2 00 f8 ff ff\s+\{vex\} vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b2 00 f8 ff ff\s+\{vex\} vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 71 7f\s+vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 71 7f\s+vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b1 e0 0f 00 00\s+\{vex\} vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b1 e0 0f 00 00\s+\{vex\} vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 72 80\s+vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 72 80\s+vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b2 00 f0 ff ff\s+\{vex\} vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b2 00 f0 ff ff\s+\{vex\} vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7a b1 b4 f5 00 00 00 10\s+vbcstnebf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a b1 31\s+vbcstnebf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b1 b1 fe 00 00 00\s+vbcstnebf162ps 0xfe\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b1 b2 00 ff ff ff\s+vbcstnebf162ps -0x100\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7e b1 b4 f5 00 00 00 10\s+vbcstnebf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7e b1 31\s+vbcstnebf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b1 b1 fe 00 00 00\s+vbcstnebf162ps 0xfe\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b1 b2 00 ff ff ff\s+vbcstnebf162ps -0x100\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 79 b1 b4 f5 00 00 00 10\s+vbcstnesh2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 79 b1 31\s+vbcstnesh2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b1 b1 fe 00 00 00\s+vbcstnesh2ps 0xfe\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b1 b2 00 ff ff ff\s+vbcstnesh2ps -0x100\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7d b1 b4 f5 00 00 00 10\s+vbcstnesh2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7d b1 31\s+vbcstnesh2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b1 b1 fe 00 00 00\s+vbcstnesh2ps 0xfe\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b1 b2 00 ff ff ff\s+vbcstnesh2ps -0x100\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 7a b0 b4 f5 00 00 00 10\s+vcvtneebf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a b0 31\s+vcvtneebf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b0 b1 f0 07 00 00\s+vcvtneebf162ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a b0 b2 00 f8 ff ff\s+vcvtneebf162ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7e b0 b4 f5 00 00 00 10\s+vcvtneebf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7e b0 31\s+vcvtneebf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b0 b1 e0 0f 00 00\s+vcvtneebf162ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7e b0 b2 00 f0 ff ff\s+vcvtneebf162ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 79 b0 b4 f5 00 00 00 10\s+vcvtneeph2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 79 b0 31\s+vcvtneeph2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b0 b1 f0 07 00 00\s+vcvtneeph2ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 79 b0 b2 00 f8 ff ff\s+vcvtneeph2ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7d b0 b4 f5 00 00 00 10\s+vcvtneeph2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7d b0 31\s+vcvtneeph2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b0 b1 e0 0f 00 00\s+vcvtneeph2ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7d b0 b2 00 f0 ff ff\s+vcvtneeph2ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 7b b0 b4 f5 00 00 00 10\s+vcvtneobf162ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7b b0 31\s+vcvtneobf162ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7b b0 b1 f0 07 00 00\s+vcvtneobf162ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7b b0 b2 00 f8 ff ff\s+vcvtneobf162ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7f b0 b4 f5 00 00 00 10\s+vcvtneobf162ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7f b0 31\s+vcvtneobf162ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7f b0 b1 e0 0f 00 00\s+vcvtneobf162ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7f b0 b2 00 f0 ff ff\s+vcvtneobf162ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*c4 a2 78 b0 b4 f5 00 00 00 10\s+vcvtneoph2ps 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 78 b0 31\s+vcvtneoph2ps \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 78 b0 b1 f0 07 00 00\s+vcvtneoph2ps 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 78 b0 b2 00 f8 ff ff\s+vcvtneoph2ps -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7c b0 b4 f5 00 00 00 10\s+vcvtneoph2ps 0x10000000\(%rbp,%r14,8\),%ymm6
\s*[a-f0-9]+:\s*c4 c2 7c b0 31\s+vcvtneoph2ps \(%r9\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7c b0 b1 e0 0f 00 00\s+vcvtneoph2ps 0xfe0\(%rcx\),%ymm6
\s*[a-f0-9]+:\s*c4 e2 7c b0 b2 00 f0 ff ff\s+vcvtneoph2ps -0x1000\(%rdx\),%ymm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 f5\s+vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 f5\s+vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 f5\s+\{vex\} vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 f5\s+\{vex\} vcvtneps2bf16 %xmm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 f5\s+vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 f5\s+vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 f5\s+\{vex\} vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 f5\s+\{vex\} vcvtneps2bf16 %ymm5,%xmm6
\s*[a-f0-9]+:\s*62 b2 7e 08 72 b4 f5 00 00 00 10\s+vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*62 b2 7e 08 72 b4 f5 00 00 00 10\s+vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7a 72 b4 f5 00 00 00 10\s+\{vex\} vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*c4 a2 7a 72 b4 f5 00 00 00 10\s+\{vex\} vcvtneps2bf16x 0x10000000\(%rbp,%r14,8\),%xmm6
\s*[a-f0-9]+:\s*62 d2 7e 08 72 31\s+vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*62 d2 7e 08 72 31\s+vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a 72 31\s+\{vex\} vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*c4 c2 7a 72 31\s+\{vex\} vcvtneps2bf16x \(%r9\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 71 7f\s+vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 71 7f\s+vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b1 f0 07 00 00\s+\{vex\} vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b1 f0 07 00 00\s+\{vex\} vcvtneps2bf16x 0x7f0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 72 80\s+vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 08 72 72 80\s+vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b2 00 f8 ff ff\s+\{vex\} vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7a 72 b2 00 f8 ff ff\s+\{vex\} vcvtneps2bf16x -0x800\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 71 7f\s+vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 71 7f\s+vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b1 e0 0f 00 00\s+\{vex\} vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b1 e0 0f 00 00\s+\{vex\} vcvtneps2bf16y 0xfe0\(%rcx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 72 80\s+vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*62 f2 7e 28 72 72 80\s+vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b2 00 f0 ff ff\s+\{vex\} vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
\s*[a-f0-9]+:\s*c4 e2 7e 72 b2 00 f0 ff ff\s+\{vex\} vcvtneps2bf16y -0x1000\(%rdx\),%xmm6
#pass
