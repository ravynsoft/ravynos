#source: ../x86-64-sib.s
#as: -J
#objdump: -dw
#name: x86-64 (ILP32) SIB

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	8b 1c 25 e2 ff ff ff 	mov    0xffffffffffffffe2,%ebx
[ 	]*[a-f0-9]+:	8b 1c 25 e2 ff ff ff 	mov    0xffffffffffffffe2,%ebx
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    0xffffffffffffffe2,%eax
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    -0x1e\(,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    -0x1e\(,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    -0x1e\(,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    0x1e,%eax
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    0x1e,%eax
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    0x1e,%eax
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    0x1e\(,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    0x1e\(,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    0x1e\(,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 03                	mov    \(%rbx\),%eax
[ 	]*[a-f0-9]+:	8b 04 23             	mov    \(%rbx,%riz,1\),%eax
[ 	]*[a-f0-9]+:	8b 04 23             	mov    \(%rbx,%riz,1\),%eax
[ 	]*[a-f0-9]+:	8b 04 63             	mov    \(%rbx,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    \(%rbx,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    \(%rbx,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 64             	mov    \(%rsp,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    \(%rsp,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    \(%rsp,%riz,8\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 64          	mov    \(%r12,%riz,2\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 a4          	mov    \(%r12,%riz,4\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 e4          	mov    \(%r12,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 25 e2 ff ff ff 	mov    0xffffffffffffffe2,%eax
[ 	]*[a-f0-9]+:	8b 04 65 e2 ff ff ff 	mov    -0x1e\(,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a5 e2 ff ff ff 	mov    -0x1e\(,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e5 e2 ff ff ff 	mov    -0x1e\(,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 25 1e 00 00 00 	mov    0x1e,%eax
[ 	]*[a-f0-9]+:	8b 04 65 1e 00 00 00 	mov    0x1e\(,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a5 1e 00 00 00 	mov    0x1e\(,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e5 1e 00 00 00 	mov    0x1e\(,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 23             	mov    \(%rbx,%riz,1\),%eax
[ 	]*[a-f0-9]+:	8b 04 23             	mov    \(%rbx,%riz,1\),%eax
[ 	]*[a-f0-9]+:	8b 04 63             	mov    \(%rbx,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a3             	mov    \(%rbx,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e3             	mov    \(%rbx,%riz,8\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 24             	mov    \(%rsp\),%eax
[ 	]*[a-f0-9]+:	8b 04 64             	mov    \(%rsp,%riz,2\),%eax
[ 	]*[a-f0-9]+:	8b 04 a4             	mov    \(%rsp,%riz,4\),%eax
[ 	]*[a-f0-9]+:	8b 04 e4             	mov    \(%rsp,%riz,8\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 24          	mov    \(%r12\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 64          	mov    \(%r12,%riz,2\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 a4          	mov    \(%r12,%riz,4\),%eax
[ 	]*[a-f0-9]+:	41 8b 04 e4          	mov    \(%r12,%riz,8\),%eax
#pass
