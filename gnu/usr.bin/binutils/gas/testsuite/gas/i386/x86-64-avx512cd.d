#as:
#objdump: -dw
#name: x86_64 AVX512CD insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 02 7d 48 c4 f5    	vpconflictd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f c4 f5    	vpconflictd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf c4 f5    	vpconflictd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 31    	vpconflictd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 c4 b4 f0 23 01 00 00 	vpconflictd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 31    	vpconflictd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 7f 	vpconflictd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 00 20 00 00 	vpconflictd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 80 	vpconflictd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 c0 df ff ff 	vpconflictd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 7f 	vpconflictd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 00 02 00 00 	vpconflictd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 80 	vpconflictd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 fc fd ff ff 	vpconflictd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 c4 f5    	vpconflictq %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f c4 f5    	vpconflictq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf c4 f5    	vpconflictq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 31    	vpconflictq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 c4 b4 f0 23 01 00 00 	vpconflictq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 31    	vpconflictq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 7f 	vpconflictq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 00 20 00 00 	vpconflictq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 80 	vpconflictq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 c0 df ff ff 	vpconflictq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 7f 	vpconflictq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 00 04 00 00 	vpconflictq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 80 	vpconflictq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 f8 fb ff ff 	vpconflictq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 48 44 f5    	vplzcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f 44 f5    	vplzcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf 44 f5    	vplzcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 7d 48 44 31    	vplzcntd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 44 b4 f0 23 01 00 00 	vplzcntd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 31    	vplzcntd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 7f 	vplzcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 00 20 00 00 	vplzcntd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 80 	vplzcntd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 c0 df ff ff 	vplzcntd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 7f 	vplzcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 00 02 00 00 	vplzcntd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 80 	vplzcntd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 fc fd ff ff 	vplzcntd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 44 f5    	vplzcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f 44 f5    	vplzcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 44 f5    	vplzcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 48 44 31    	vplzcntq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 44 b4 f0 23 01 00 00 	vplzcntq 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 31    	vplzcntq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 7f 	vplzcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 00 20 00 00 	vplzcntq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 80 	vplzcntq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 c0 df ff ff 	vplzcntq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 7f 	vplzcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 00 04 00 00 	vplzcntq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 80 	vplzcntq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 f8 fb ff ff 	vplzcntq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7e 48 3a f6    	vpbroadcastmw2d %k6,%zmm30
[ 	]*[a-f0-9]+:	62 62 fe 48 2a f6    	vpbroadcastmb2q %k6,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 48 c4 f5    	vpconflictd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f c4 f5    	vpconflictd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf c4 f5    	vpconflictd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 31    	vpconflictd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 c4 b4 f0 34 12 00 00 	vpconflictd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 31    	vpconflictd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 7f 	vpconflictd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 00 20 00 00 	vpconflictd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 72 80 	vpconflictd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c4 b2 c0 df ff ff 	vpconflictd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 7f 	vpconflictd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 00 02 00 00 	vpconflictd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 72 80 	vpconflictd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c4 b2 fc fd ff ff 	vpconflictd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 c4 f5    	vpconflictq %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f c4 f5    	vpconflictq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf c4 f5    	vpconflictq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 31    	vpconflictq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 c4 b4 f0 34 12 00 00 	vpconflictq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 31    	vpconflictq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 7f 	vpconflictq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 00 20 00 00 	vpconflictq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 72 80 	vpconflictq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c4 b2 c0 df ff ff 	vpconflictq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 7f 	vpconflictq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 00 04 00 00 	vpconflictq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 72 80 	vpconflictq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c4 b2 f8 fb ff ff 	vpconflictq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 48 44 f5    	vplzcntd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f 44 f5    	vplzcntd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf 44 f5    	vplzcntd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 7d 48 44 31    	vplzcntd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 44 b4 f0 34 12 00 00 	vplzcntd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 31    	vplzcntd \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 7f 	vplzcntd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 00 20 00 00 	vplzcntd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 72 80 	vplzcntd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 44 b2 c0 df ff ff 	vplzcntd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 7f 	vplzcntd 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 00 02 00 00 	vplzcntd 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 72 80 	vplzcntd -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 44 b2 fc fd ff ff 	vplzcntd -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 44 f5    	vplzcntq %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f 44 f5    	vplzcntq %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf 44 f5    	vplzcntq %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 62 fd 48 44 31    	vplzcntq \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 44 b4 f0 34 12 00 00 	vplzcntq 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 31    	vplzcntq \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 7f 	vplzcntq 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 00 20 00 00 	vplzcntq 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 72 80 	vplzcntq -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 44 b2 c0 df ff ff 	vplzcntq -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 7f 	vplzcntq 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 00 04 00 00 	vplzcntq 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 72 80 	vplzcntq -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 44 b2 f8 fb ff ff 	vplzcntq -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7e 48 3a f6    	vpbroadcastmw2d %k6,%zmm30
[ 	]*[a-f0-9]+:	62 62 fe 48 2a f6    	vpbroadcastmb2q %k6,%zmm30
#pass
