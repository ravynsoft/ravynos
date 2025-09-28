#objdump: -dw
#name: x86-64 SHA

.*:     file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 3a cc ca 09       	sha1rnds4 \$0x9,%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 3a cc 10 07       	sha1rnds4 \$0x7,\(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 3a cc 58 12 05    	sha1rnds4 \$0x5,0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 3a cc 24 58 01    	sha1rnds4 \$0x1,\(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 c8 fa          	sha1nexte %xmm2,%xmm7
[ 	]*[a-f0-9]+:	44 0f 38 c8 00       	sha1nexte \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	44 0f 38 c8 48 12    	sha1nexte 0x12\(%rax\),%xmm9
[ 	]*[a-f0-9]+:	44 0f 38 c8 14 58    	sha1nexte \(%rax,%rbx,2\),%xmm10
[ 	]*[a-f0-9]+:	0f 38 c9 fa          	sha1msg1 %xmm2,%xmm7
[ 	]*[a-f0-9]+:	44 0f 38 c9 00       	sha1msg1 \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	44 0f 38 c9 48 12    	sha1msg1 0x12\(%rax\),%xmm9
[ 	]*[a-f0-9]+:	44 0f 38 c9 14 58    	sha1msg1 \(%rax,%rbx,2\),%xmm10
[ 	]*[a-f0-9]+:	0f 38 ca fa          	sha1msg2 %xmm2,%xmm7
[ 	]*[a-f0-9]+:	44 0f 38 ca 00       	sha1msg2 \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	44 0f 38 ca 48 12    	sha1msg2 0x12\(%rax\),%xmm9
[ 	]*[a-f0-9]+:	44 0f 38 ca 14 58    	sha1msg2 \(%rax,%rbx,2\),%xmm10
[ 	]*[a-f0-9]+:	0f 38 cb ca          	sha256rnds2 (%xmm0,)?%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 08          	sha256rnds2 (%xmm0,)?\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 48 12       	sha256rnds2 (%xmm0,)?0x12\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 0c 58       	sha256rnds2 (%xmm0,)?\(%rax,%rbx,2\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb ca          	sha256rnds2 (%xmm0,)?%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 08          	sha256rnds2 (%xmm0,)?\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 48 12       	sha256rnds2 (%xmm0,)?0x12\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 0c 58       	sha256rnds2 (%xmm0,)?\(%rax,%rbx,2\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cc ca          	sha256msg1 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cc 08          	sha256msg1 \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cc 48 12       	sha256msg1 0x12\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cc 0c 58       	sha256msg1 \(%rax,%rbx,2\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cd ca          	sha256msg2 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cd 08          	sha256msg2 \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cd 48 12       	sha256msg2 0x12\(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 38 cd 0c 58       	sha256msg2 \(%rax,%rbx,2\),%xmm1
[ 	]*[a-f0-9]+:	0f 3a cc ca 09       	sha1rnds4 \$0x9,%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 3a cc 10 07       	sha1rnds4 \$0x7,\(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 3a cc 58 12 05    	sha1rnds4 \$0x5,0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 3a cc 24 58 01    	sha1rnds4 \$0x1,\(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 c8 ca          	sha1nexte %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 c8 10          	sha1nexte \(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 c8 58 12       	sha1nexte 0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 c8 24 58       	sha1nexte \(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 c9 ca          	sha1msg1 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 c9 10          	sha1msg1 \(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 c9 58 12       	sha1msg1 0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 c9 24 58       	sha1msg1 \(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 ca ca          	sha1msg2 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 ca 10          	sha1msg2 \(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 ca 58 12       	sha1msg2 0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 ca 24 58       	sha1msg2 \(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 cb ca          	sha256rnds2 (%xmm0,)?%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 10          	sha256rnds2 (%xmm0,)?\(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 cb 58 12       	sha256rnds2 (%xmm0,)?0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 cb 24 58       	sha256rnds2 (%xmm0,)?\(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 cb ca          	sha256rnds2 (%xmm0,)?%xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cb 10          	sha256rnds2 (%xmm0,)?\(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 cb 58 12       	sha256rnds2 (%xmm0,)?0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 cb 24 58       	sha256rnds2 (%xmm0,)?\(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 cc ca          	sha256msg1 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cc 10          	sha256msg1 \(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 cc 58 12       	sha256msg1 0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 cc 24 58       	sha256msg1 \(%rax,%rbx,2\),%xmm4
[ 	]*[a-f0-9]+:	0f 38 cd ca          	sha256msg2 %xmm2,%xmm1
[ 	]*[a-f0-9]+:	0f 38 cd 10          	sha256msg2 \(%rax\),%xmm2
[ 	]*[a-f0-9]+:	0f 38 cd 58 12       	sha256msg2 0x12\(%rax\),%xmm3
[ 	]*[a-f0-9]+:	0f 38 cd 24 58       	sha256msg2 \(%rax,%rbx,2\),%xmm4
#pass
