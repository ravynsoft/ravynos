#as:
#objdump: -dw
#name: i386 AVX512CD insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 f5    	vpconflictd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f c4 f5    	vpconflictd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf c4 f5    	vpconflictd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 31    	vpconflictd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b4 f4 c0 1d fe ff 	vpconflictd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 30    	vpconflictd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 7f 	vpconflictd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 00 20 00 00 	vpconflictd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 80 	vpconflictd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 c0 df ff ff 	vpconflictd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 7f 	vpconflictd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 00 02 00 00 	vpconflictd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 80 	vpconflictd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 fc fd ff ff 	vpconflictd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 f5    	vpconflictq %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f c4 f5    	vpconflictq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf c4 f5    	vpconflictq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 31    	vpconflictq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b4 f4 c0 1d fe ff 	vpconflictq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 30    	vpconflictq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 7f 	vpconflictq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 00 20 00 00 	vpconflictq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 80 	vpconflictq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 c0 df ff ff 	vpconflictq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 7f 	vpconflictq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 00 04 00 00 	vpconflictq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 80 	vpconflictq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 f8 fb ff ff 	vpconflictq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 f5    	vplzcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f 44 f5    	vplzcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf 44 f5    	vplzcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 31    	vplzcntd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b4 f4 c0 1d fe ff 	vplzcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 30    	vplzcntd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 7f 	vplzcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 00 20 00 00 	vplzcntd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 80 	vplzcntd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 c0 df ff ff 	vplzcntd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 7f 	vplzcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 00 02 00 00 	vplzcntd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 80 	vplzcntd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 fc fd ff ff 	vplzcntd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 f5    	vplzcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f 44 f5    	vplzcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 44 f5    	vplzcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 31    	vplzcntq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b4 f4 c0 1d fe ff 	vplzcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 30    	vplzcntq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 7f 	vplzcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 00 20 00 00 	vplzcntq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 80 	vplzcntq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 c0 df ff ff 	vplzcntq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 7f 	vplzcntq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 00 04 00 00 	vplzcntq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 80 	vplzcntq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 f8 fb ff ff 	vplzcntq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7e 48 3a f6    	vpbroadcastmw2d %k6,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fe 48 2a f6    	vpbroadcastmb2q %k6,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 f5    	vpconflictd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f c4 f5    	vpconflictd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf c4 f5    	vpconflictd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 31    	vpconflictd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b4 f4 c0 1d fe ff 	vpconflictd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 30    	vpconflictd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 7f 	vpconflictd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 00 20 00 00 	vpconflictd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 72 80 	vpconflictd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c4 b2 c0 df ff ff 	vpconflictd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 7f 	vpconflictd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 00 02 00 00 	vpconflictd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 72 80 	vpconflictd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c4 b2 fc fd ff ff 	vpconflictd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 f5    	vpconflictq %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f c4 f5    	vpconflictq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf c4 f5    	vpconflictq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 31    	vpconflictq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b4 f4 c0 1d fe ff 	vpconflictq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 30    	vpconflictq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 7f 	vpconflictq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 00 20 00 00 	vpconflictq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 72 80 	vpconflictq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c4 b2 c0 df ff ff 	vpconflictq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 7f 	vpconflictq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 00 04 00 00 	vpconflictq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 72 80 	vpconflictq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c4 b2 f8 fb ff ff 	vpconflictq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 f5    	vplzcntd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f 44 f5    	vplzcntd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf 44 f5    	vplzcntd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 31    	vplzcntd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b4 f4 c0 1d fe ff 	vplzcntd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 30    	vplzcntd \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 7f 	vplzcntd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 00 20 00 00 	vplzcntd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 72 80 	vplzcntd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 44 b2 c0 df ff ff 	vplzcntd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 7f 	vplzcntd 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 00 02 00 00 	vplzcntd 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 72 80 	vplzcntd -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 44 b2 fc fd ff ff 	vplzcntd -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 f5    	vplzcntq %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f 44 f5    	vplzcntq %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf 44 f5    	vplzcntq %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 31    	vplzcntq \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b4 f4 c0 1d fe ff 	vplzcntq -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 30    	vplzcntq \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 7f 	vplzcntq 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 00 20 00 00 	vplzcntq 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 72 80 	vplzcntq -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 44 b2 c0 df ff ff 	vplzcntq -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 7f 	vplzcntq 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 00 04 00 00 	vplzcntq 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 72 80 	vplzcntq -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 44 b2 f8 fb ff ff 	vplzcntq -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7e 48 3a f6    	vpbroadcastmw2d %k6,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fe 48 2a f6    	vpbroadcastmb2q %k6,%zmm6
#pass
