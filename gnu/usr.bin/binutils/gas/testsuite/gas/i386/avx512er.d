#as:
#objdump: -dw
#name: i386 AVX512ER insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 f5    	vexp2ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 18 c8 f5    	vexp2ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 31    	vexp2ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b4 f4 c0 1d fe ff 	vexp2ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 30    	vexp2ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 7f 	vexp2ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 00 20 00 00 	vexp2ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 80 	vexp2ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 c0 df ff ff 	vexp2ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 7f 	vexp2ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 00 02 00 00 	vexp2ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 80 	vexp2ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 fc fd ff ff 	vexp2ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 f5    	vexp2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 18 c8 f5    	vexp2pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 31    	vexp2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b4 f4 c0 1d fe ff 	vexp2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 30    	vexp2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 7f 	vexp2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 00 20 00 00 	vexp2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 80 	vexp2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 c0 df ff ff 	vexp2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 7f 	vexp2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 00 04 00 00 	vexp2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 80 	vexp2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 f8 fb ff ff 	vexp2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca f5    	vrcp28ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f ca f5    	vrcp28ps %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf ca f5    	vrcp28ps %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 18 ca f5    	vrcp28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 31    	vrcp28ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b4 f4 c0 1d fe ff 	vrcp28ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 30    	vrcp28ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 7f 	vrcp28ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 00 20 00 00 	vrcp28ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 80 	vrcp28ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 c0 df ff ff 	vrcp28ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 7f 	vrcp28ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 00 02 00 00 	vrcp28ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 80 	vrcp28ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 fc fd ff ff 	vrcp28ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca f5    	vrcp28pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f ca f5    	vrcp28pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf ca f5    	vrcp28pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 18 ca f5    	vrcp28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 31    	vrcp28pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b4 f4 c0 1d fe ff 	vrcp28pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 30    	vrcp28pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 7f 	vrcp28pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 00 20 00 00 	vrcp28pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 80 	vrcp28pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 c0 df ff ff 	vrcp28pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 7f 	vrcp28pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 00 04 00 00 	vrcp28pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 80 	vrcp28pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 f8 fb ff ff 	vrcp28pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 55 0f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 8f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 31    	vrcp28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b4 f4 c0 1d fe ff 	vrcp28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 7f 	vrcp28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 00 02 00 00 	vrcp28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 80 	vrcp28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 fc fd ff ff 	vrcp28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 8f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 31    	vrcp28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b4 f4 c0 1d fe ff 	vrcp28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 7f 	vrcp28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 00 04 00 00 	vrcp28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 80 	vrcp28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc f5    	vrsqrt28ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f cc f5    	vrsqrt28ps %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf cc f5    	vrsqrt28ps %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 18 cc f5    	vrsqrt28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 31    	vrsqrt28ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b4 f4 c0 1d fe ff 	vrsqrt28ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 30    	vrsqrt28ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 7f 	vrsqrt28ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 00 20 00 00 	vrsqrt28ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 80 	vrsqrt28ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 7f 	vrsqrt28ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 00 02 00 00 	vrsqrt28ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 80 	vrsqrt28ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc f5    	vrsqrt28pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f cc f5    	vrsqrt28pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf cc f5    	vrsqrt28pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 18 cc f5    	vrsqrt28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 31    	vrsqrt28pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b4 f4 c0 1d fe ff 	vrsqrt28pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 30    	vrsqrt28pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 7f 	vrsqrt28pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 00 20 00 00 	vrsqrt28pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 80 	vrsqrt28pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 7f 	vrsqrt28pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 00 04 00 00 	vrsqrt28pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 80 	vrsqrt28pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 55 0f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 8f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 31    	vrsqrt28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b4 f4 c0 1d fe ff 	vrsqrt28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 7f 	vrsqrt28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 80 	vrsqrt28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 8f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 31    	vrsqrt28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b4 f4 c0 1d fe ff 	vrsqrt28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 7f 	vrsqrt28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 80 	vrsqrt28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 f5    	vexp2ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 18 c8 f5    	vexp2ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 31    	vexp2ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b4 f4 c0 1d fe ff 	vexp2ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 30    	vexp2ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 7f 	vexp2ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 00 20 00 00 	vexp2ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 72 80 	vexp2ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 c8 b2 c0 df ff ff 	vexp2ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 7f 	vexp2ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 00 02 00 00 	vexp2ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 72 80 	vexp2ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 c8 b2 fc fd ff ff 	vexp2ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 f5    	vexp2pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 18 c8 f5    	vexp2pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 31    	vexp2pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b4 f4 c0 1d fe ff 	vexp2pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 30    	vexp2pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 7f 	vexp2pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 00 20 00 00 	vexp2pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 72 80 	vexp2pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 c8 b2 c0 df ff ff 	vexp2pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 7f 	vexp2pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 00 04 00 00 	vexp2pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 72 80 	vexp2pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 c8 b2 f8 fb ff ff 	vexp2pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca f5    	vrcp28ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f ca f5    	vrcp28ps %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf ca f5    	vrcp28ps %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 18 ca f5    	vrcp28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 31    	vrcp28ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b4 f4 c0 1d fe ff 	vrcp28ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 30    	vrcp28ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 7f 	vrcp28ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 00 20 00 00 	vrcp28ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca 72 80 	vrcp28ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 ca b2 c0 df ff ff 	vrcp28ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 7f 	vrcp28ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 00 02 00 00 	vrcp28ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca 72 80 	vrcp28ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 ca b2 fc fd ff ff 	vrcp28ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca f5    	vrcp28pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f ca f5    	vrcp28pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf ca f5    	vrcp28pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 18 ca f5    	vrcp28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 31    	vrcp28pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b4 f4 c0 1d fe ff 	vrcp28pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 30    	vrcp28pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 7f 	vrcp28pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 00 20 00 00 	vrcp28pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca 72 80 	vrcp28pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 ca b2 c0 df ff ff 	vrcp28pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 7f 	vrcp28pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 00 04 00 00 	vrcp28pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca 72 80 	vrcp28pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 ca b2 f8 fb ff ff 	vrcp28pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 55 0f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 8f cb f4    	vrcp28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cb f4    	vrcp28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 31    	vrcp28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b4 f4 c0 1d fe ff 	vrcp28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 7f 	vrcp28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 00 02 00 00 	vrcp28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb 72 80 	vrcp28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cb b2 fc fd ff ff 	vrcp28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 8f cb f4    	vrcp28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cb f4    	vrcp28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 31    	vrcp28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b4 f4 c0 1d fe ff 	vrcp28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 7f 	vrcp28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 00 04 00 00 	vrcp28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb 72 80 	vrcp28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc f5    	vrsqrt28ps %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 4f cc f5    	vrsqrt28ps %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 7d cf cc f5    	vrsqrt28ps %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 7d 18 cc f5    	vrsqrt28ps \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 31    	vrsqrt28ps \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b4 f4 c0 1d fe ff 	vrsqrt28ps -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 30    	vrsqrt28ps \(%eax\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 7f 	vrsqrt28ps 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 00 20 00 00 	vrsqrt28ps 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc 72 80 	vrsqrt28ps -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 7f 	vrsqrt28ps 0x1fc\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 00 02 00 00 	vrsqrt28ps 0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc 72 80 	vrsqrt28ps -0x200\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps -0x204\(%edx\)\{1to16\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc f5    	vrsqrt28pd %zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 4f cc f5    	vrsqrt28pd %zmm5,%zmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 fd cf cc f5    	vrsqrt28pd %zmm5,%zmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 fd 18 cc f5    	vrsqrt28pd \{sae\},%zmm5,%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 31    	vrsqrt28pd \(%ecx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b4 f4 c0 1d fe ff 	vrsqrt28pd -0x1e240\(%esp,%esi,8\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 30    	vrsqrt28pd \(%eax\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 7f 	vrsqrt28pd 0x1fc0\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 00 20 00 00 	vrsqrt28pd 0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc 72 80 	vrsqrt28pd -0x2000\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd -0x2040\(%edx\),%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 7f 	vrsqrt28pd 0x3f8\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 00 04 00 00 	vrsqrt28pd 0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc 72 80 	vrsqrt28pd -0x400\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd -0x408\(%edx\)\{1to8\},%zmm6
[ 	]*[a-f0-9]+:	62 f2 55 0f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 8f cd f4    	vrsqrt28ss %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 55 1f cd f4    	vrsqrt28ss \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 31    	vrsqrt28ss \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b4 f4 c0 1d fe ff 	vrsqrt28ss -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 7f 	vrsqrt28ss 0x1fc\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd 72 80 	vrsqrt28ss -0x200\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 55 0f cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 8f cd f4    	vrsqrt28sd %xmm4,%xmm5,%xmm6\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 f2 d5 1f cd f4    	vrsqrt28sd \{sae\},%xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 31    	vrsqrt28sd \(%ecx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b4 f4 c0 1d fe ff 	vrsqrt28sd -0x1e240\(%esp,%esi,8\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 7f 	vrsqrt28sd 0x3f8\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd 72 80 	vrsqrt28sd -0x400\(%edx\),%xmm5,%xmm6\{%k7\}
[ 	]*[a-f0-9]+:	62 f2 d5 0f cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%edx\),%xmm5,%xmm6\{%k7\}
#pass
