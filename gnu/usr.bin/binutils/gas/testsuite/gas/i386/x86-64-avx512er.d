#as:
#objdump: -dw
#name: x86_64 AVX512ER insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 02 7d 48 c8 f5    	vexp2ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 18 c8 f5    	vexp2ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 31    	vexp2ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 c8 b4 f0 23 01 00 00 	vexp2ps 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 31    	vexp2ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 72 7f 	vexp2ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 b2 00 20 00 00 	vexp2ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 72 80 	vexp2ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 b2 c0 df ff ff 	vexp2ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 72 7f 	vexp2ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 b2 00 02 00 00 	vexp2ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 72 80 	vexp2ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 b2 fc fd ff ff 	vexp2ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 c8 f5    	vexp2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 18 c8 f5    	vexp2pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 31    	vexp2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 c8 b4 f0 23 01 00 00 	vexp2pd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 31    	vexp2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 72 7f 	vexp2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 b2 00 20 00 00 	vexp2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 72 80 	vexp2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 b2 c0 df ff ff 	vexp2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 72 7f 	vexp2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 b2 00 04 00 00 	vexp2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 72 80 	vexp2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 b2 f8 fb ff ff 	vexp2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 48 ca f5    	vrcp28ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f ca f5    	vrcp28ps %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf ca f5    	vrcp28ps %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7d 18 ca f5    	vrcp28ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 31    	vrcp28ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 ca b4 f0 23 01 00 00 	vrcp28ps 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 31    	vrcp28ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 72 7f 	vrcp28ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca b2 00 20 00 00 	vrcp28ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 72 80 	vrcp28ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca b2 c0 df ff ff 	vrcp28ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 72 7f 	vrcp28ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca b2 00 02 00 00 	vrcp28ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 72 80 	vrcp28ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca b2 fc fd ff ff 	vrcp28ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 ca f5    	vrcp28pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f ca f5    	vrcp28pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf ca f5    	vrcp28pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 fd 18 ca f5    	vrcp28pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 31    	vrcp28pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 ca b4 f0 23 01 00 00 	vrcp28pd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 31    	vrcp28pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 72 7f 	vrcp28pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca b2 00 20 00 00 	vrcp28pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 72 80 	vrcp28pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca b2 c0 df ff ff 	vrcp28pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 72 7f 	vrcp28pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca b2 00 04 00 00 	vrcp28pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 72 80 	vrcp28pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca b2 f8 fb ff ff 	vrcp28pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 15 07 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 87 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 31    	vrcp28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 07 cb b4 f0 23 01 00 00 	vrcp28ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 72 7f 	vrcp28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb b2 00 02 00 00 	vrcp28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 72 80 	vrcp28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb b2 fc fd ff ff 	vrcp28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 07 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 87 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 31    	vrcp28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 07 cb b4 f0 23 01 00 00 	vrcp28sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 72 7f 	vrcp28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb b2 00 04 00 00 	vrcp28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 72 80 	vrcp28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d 48 cc f5    	vrsqrt28ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f cc f5    	vrsqrt28ps %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf cc f5    	vrsqrt28ps %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7d 18 cc f5    	vrsqrt28ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 31    	vrsqrt28ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 cc b4 f0 23 01 00 00 	vrsqrt28ps 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 31    	vrsqrt28ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 72 7f 	vrsqrt28ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc b2 00 20 00 00 	vrsqrt28ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 72 80 	vrsqrt28ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 72 7f 	vrsqrt28ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc b2 00 02 00 00 	vrsqrt28ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 72 80 	vrsqrt28ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 cc f5    	vrsqrt28pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f cc f5    	vrsqrt28pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf cc f5    	vrsqrt28pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 fd 18 cc f5    	vrsqrt28pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 31    	vrsqrt28pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 cc b4 f0 23 01 00 00 	vrsqrt28pd 0x123\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 31    	vrsqrt28pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 72 7f 	vrsqrt28pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc b2 00 20 00 00 	vrsqrt28pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 72 80 	vrsqrt28pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 72 7f 	vrsqrt28pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc b2 00 04 00 00 	vrsqrt28pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 72 80 	vrsqrt28pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 15 07 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 87 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 31    	vrsqrt28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 07 cd b4 f0 23 01 00 00 	vrsqrt28ss 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 72 7f 	vrsqrt28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 72 80 	vrsqrt28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 07 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 87 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 31    	vrsqrt28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 07 cd b4 f0 23 01 00 00 	vrsqrt28sd 0x123\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 72 7f 	vrsqrt28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 72 80 	vrsqrt28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d 48 c8 f5    	vexp2ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 18 c8 f5    	vexp2ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 31    	vexp2ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 c8 b4 f0 34 12 00 00 	vexp2ps 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 31    	vexp2ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 72 7f 	vexp2ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 b2 00 20 00 00 	vexp2ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 72 80 	vexp2ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 c8 b2 c0 df ff ff 	vexp2ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 72 7f 	vexp2ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 b2 00 02 00 00 	vexp2ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 72 80 	vexp2ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 c8 b2 fc fd ff ff 	vexp2ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 c8 f5    	vexp2pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 18 c8 f5    	vexp2pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 31    	vexp2pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 c8 b4 f0 34 12 00 00 	vexp2pd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 31    	vexp2pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 72 7f 	vexp2pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 b2 00 20 00 00 	vexp2pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 72 80 	vexp2pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 c8 b2 c0 df ff ff 	vexp2pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 72 7f 	vexp2pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 b2 00 04 00 00 	vexp2pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 72 80 	vexp2pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 c8 b2 f8 fb ff ff 	vexp2pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 48 ca f5    	vrcp28ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f ca f5    	vrcp28ps %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf ca f5    	vrcp28ps %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7d 18 ca f5    	vrcp28ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 31    	vrcp28ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 ca b4 f0 34 12 00 00 	vrcp28ps 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 31    	vrcp28ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 72 7f 	vrcp28ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca b2 00 20 00 00 	vrcp28ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca 72 80 	vrcp28ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 ca b2 c0 df ff ff 	vrcp28ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 72 7f 	vrcp28ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca b2 00 02 00 00 	vrcp28ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca 72 80 	vrcp28ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 ca b2 fc fd ff ff 	vrcp28ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 ca f5    	vrcp28pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f ca f5    	vrcp28pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf ca f5    	vrcp28pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 fd 18 ca f5    	vrcp28pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 31    	vrcp28pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 ca b4 f0 34 12 00 00 	vrcp28pd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 31    	vrcp28pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 72 7f 	vrcp28pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca b2 00 20 00 00 	vrcp28pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca 72 80 	vrcp28pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 ca b2 c0 df ff ff 	vrcp28pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 72 7f 	vrcp28pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca b2 00 04 00 00 	vrcp28pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca 72 80 	vrcp28pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 ca b2 f8 fb ff ff 	vrcp28pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 15 07 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 87 cb f4    	vrcp28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cb f4    	vrcp28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 31    	vrcp28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 07 cb b4 f0 34 12 00 00 	vrcp28ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 72 7f 	vrcp28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb b2 00 02 00 00 	vrcp28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb 72 80 	vrcp28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cb b2 fc fd ff ff 	vrcp28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 07 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 87 cb f4    	vrcp28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cb f4    	vrcp28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 31    	vrcp28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 07 cb b4 f0 34 12 00 00 	vrcp28sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 72 7f 	vrcp28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb b2 00 04 00 00 	vrcp28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb 72 80 	vrcp28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cb b2 f8 fb ff ff 	vrcp28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d 48 cc f5    	vrsqrt28ps %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 7d 4f cc f5    	vrsqrt28ps %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 7d cf cc f5    	vrsqrt28ps %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 7d 18 cc f5    	vrsqrt28ps \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 31    	vrsqrt28ps \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 7d 48 cc b4 f0 34 12 00 00 	vrsqrt28ps 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 31    	vrsqrt28ps \(%rcx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 72 7f 	vrsqrt28ps 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc b2 00 20 00 00 	vrsqrt28ps 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc 72 80 	vrsqrt28ps -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 48 cc b2 c0 df ff ff 	vrsqrt28ps -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 72 7f 	vrsqrt28ps 0x1fc\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc b2 00 02 00 00 	vrsqrt28ps 0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc 72 80 	vrsqrt28ps -0x200\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 62 7d 58 cc b2 fc fd ff ff 	vrsqrt28ps -0x204\(%rdx\)\{1to16\},%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 48 cc f5    	vrsqrt28pd %zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 02 fd 4f cc f5    	vrsqrt28pd %zmm29,%zmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 fd cf cc f5    	vrsqrt28pd %zmm29,%zmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 fd 18 cc f5    	vrsqrt28pd \{sae\},%zmm29,%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 31    	vrsqrt28pd \(%rcx\),%zmm30
[ 	]*[a-f0-9]+:	62 22 fd 48 cc b4 f0 34 12 00 00 	vrsqrt28pd 0x1234\(%rax,%r14,8\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 31    	vrsqrt28pd \(%rcx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 72 7f 	vrsqrt28pd 0x1fc0\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc b2 00 20 00 00 	vrsqrt28pd 0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc 72 80 	vrsqrt28pd -0x2000\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 48 cc b2 c0 df ff ff 	vrsqrt28pd -0x2040\(%rdx\),%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 72 7f 	vrsqrt28pd 0x3f8\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc b2 00 04 00 00 	vrsqrt28pd 0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc 72 80 	vrsqrt28pd -0x400\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 62 fd 58 cc b2 f8 fb ff ff 	vrsqrt28pd -0x408\(%rdx\)\{1to8\},%zmm30
[ 	]*[a-f0-9]+:	62 02 15 07 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 15 87 cd f4    	vrsqrt28ss %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 15 17 cd f4    	vrsqrt28ss \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 31    	vrsqrt28ss \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 15 07 cd b4 f0 34 12 00 00 	vrsqrt28ss 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 72 7f 	vrsqrt28ss 0x1fc\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd b2 00 02 00 00 	vrsqrt28ss 0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd 72 80 	vrsqrt28ss -0x200\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 15 07 cd b2 fc fd ff ff 	vrsqrt28ss -0x204\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 07 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 02 95 87 cd f4    	vrsqrt28sd %xmm28,%xmm29,%xmm30\{%k7\}\{z\}
[ 	]*[a-f0-9]+:	62 02 95 17 cd f4    	vrsqrt28sd \{sae\},%xmm28,%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 31    	vrsqrt28sd \(%rcx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 22 95 07 cd b4 f0 34 12 00 00 	vrsqrt28sd 0x1234\(%rax,%r14,8\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 72 7f 	vrsqrt28sd 0x3f8\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd b2 00 04 00 00 	vrsqrt28sd 0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd 72 80 	vrsqrt28sd -0x400\(%rdx\),%xmm29,%xmm30\{%k7\}
[ 	]*[a-f0-9]+:	62 62 95 07 cd b2 f8 fb ff ff 	vrsqrt28sd -0x408\(%rdx\),%xmm29,%xmm30\{%k7\}
#pass
