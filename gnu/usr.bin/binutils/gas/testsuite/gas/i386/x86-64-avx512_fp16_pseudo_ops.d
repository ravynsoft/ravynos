#as:
#objdump: -dw
#name: x86_64 AVX512-FP16 PSEUDO-OPS insns
#source: x86-64-avx512_fp16_pseudo_ops.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 00[ 	]*vcmpeqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 00[ 	]*vcmpeqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 00[ 	]*vcmpeqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 00[ 	]*vcmpeqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 00[ 	]*vcmpeqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 00[ 	]*vcmpeqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 01[ 	]*vcmpltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 01[ 	]*vcmpltph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 01[ 	]*vcmpltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 01[ 	]*vcmpltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 01[ 	]*vcmpltph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 01[ 	]*vcmpltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 02[ 	]*vcmpleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 02[ 	]*vcmpleph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 02[ 	]*vcmpleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 02[ 	]*vcmpleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 02[ 	]*vcmpleph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 02[ 	]*vcmpleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 03[ 	]*vcmpunordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 03[ 	]*vcmpunordph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 03[ 	]*vcmpunordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 03[ 	]*vcmpunordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 03[ 	]*vcmpunordph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 03[ 	]*vcmpunordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 04[ 	]*vcmpneqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 04[ 	]*vcmpneqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 04[ 	]*vcmpneqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 04[ 	]*vcmpneqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 04[ 	]*vcmpneqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 04[ 	]*vcmpneqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 05[ 	]*vcmpnltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 05[ 	]*vcmpnltph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 05[ 	]*vcmpnltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 05[ 	]*vcmpnltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 05[ 	]*vcmpnltph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 05[ 	]*vcmpnltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 06[ 	]*vcmpnleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 06[ 	]*vcmpnleph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 06[ 	]*vcmpnleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 06[ 	]*vcmpnleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 06[ 	]*vcmpnleph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 06[ 	]*vcmpnleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 07[ 	]*vcmpordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 07[ 	]*vcmpordph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 07[ 	]*vcmpordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 07[ 	]*vcmpordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 07[ 	]*vcmpordph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 07[ 	]*vcmpordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 08[ 	]*vcmpeq_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 08[ 	]*vcmpeq_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 08[ 	]*vcmpeq_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 08[ 	]*vcmpeq_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 08[ 	]*vcmpeq_uqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 08[ 	]*vcmpeq_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 08[ 	]*vcmpeq_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 08[ 	]*vcmpeq_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 08[ 	]*vcmpeq_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 08[ 	]*vcmpeq_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 09[ 	]*vcmpngeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 09[ 	]*vcmpngeph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 09[ 	]*vcmpngeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 09[ 	]*vcmpngeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 09[ 	]*vcmpngeph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 09[ 	]*vcmpngeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0a[ 	]*vcmpngtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0a[ 	]*vcmpngtph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0a[ 	]*vcmpngtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0a[ 	]*vcmpngtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0a[ 	]*vcmpngtph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0a[ 	]*vcmpngtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0b[ 	]*vcmpfalseph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0b[ 	]*vcmpfalseph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0b[ 	]*vcmpfalseph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0b[ 	]*vcmpfalseph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0b[ 	]*vcmpfalseph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0b[ 	]*vcmpfalseph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0c[ 	]*vcmpneq_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0c[ 	]*vcmpneq_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0c[ 	]*vcmpneq_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0c[ 	]*vcmpneq_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0c[ 	]*vcmpneq_oqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0c[ 	]*vcmpneq_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0c[ 	]*vcmpneq_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0c[ 	]*vcmpneq_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0c[ 	]*vcmpneq_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0c[ 	]*vcmpneq_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0d[ 	]*vcmpgeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0d[ 	]*vcmpgeph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0d[ 	]*vcmpgeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0d[ 	]*vcmpgeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0d[ 	]*vcmpgeph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0d[ 	]*vcmpgeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0e[ 	]*vcmpgtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0e[ 	]*vcmpgtph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0e[ 	]*vcmpgtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0e[ 	]*vcmpgtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0e[ 	]*vcmpgtph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0e[ 	]*vcmpgtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0f[ 	]*vcmptrueph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0f[ 	]*vcmptrueph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0f[ 	]*vcmptrueph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0f[ 	]*vcmptrueph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 0f[ 	]*vcmptrueph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0f[ 	]*vcmptrueph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 10[ 	]*vcmpeq_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 10[ 	]*vcmpeq_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 10[ 	]*vcmpeq_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 10[ 	]*vcmpeq_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 10[ 	]*vcmpeq_osph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 10[ 	]*vcmpeq_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 10[ 	]*vcmpeq_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 10[ 	]*vcmpeq_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 10[ 	]*vcmpeq_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 10[ 	]*vcmpeq_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 10[ 	]*vcmpeq_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 10[ 	]*vcmpeq_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 11[ 	]*vcmplt_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 11[ 	]*vcmplt_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 11[ 	]*vcmplt_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 11[ 	]*vcmplt_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 11[ 	]*vcmplt_oqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 11[ 	]*vcmplt_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 11[ 	]*vcmplt_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 11[ 	]*vcmplt_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 11[ 	]*vcmplt_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 11[ 	]*vcmplt_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 12[ 	]*vcmple_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 12[ 	]*vcmple_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 12[ 	]*vcmple_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 12[ 	]*vcmple_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 12[ 	]*vcmple_oqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 12[ 	]*vcmple_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 12[ 	]*vcmple_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 12[ 	]*vcmple_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 12[ 	]*vcmple_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 12[ 	]*vcmple_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 12[ 	]*vcmple_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 12[ 	]*vcmple_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 13[ 	]*vcmpunord_sph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 13[ 	]*vcmpunord_sph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 13[ 	]*vcmpunord_sph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 13[ 	]*vcmpunord_sph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 13[ 	]*vcmpunord_sph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 13[ 	]*vcmpunord_sph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 13[ 	]*vcmpunord_sph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 13[ 	]*vcmpunord_sph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 13[ 	]*vcmpunord_sph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 13[ 	]*vcmpunord_sph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 13[ 	]*vcmpunord_sph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 13[ 	]*vcmpunord_sph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_sph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_sph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 14[ 	]*vcmpneq_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 14[ 	]*vcmpneq_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 14[ 	]*vcmpneq_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 14[ 	]*vcmpneq_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 14[ 	]*vcmpneq_usph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 14[ 	]*vcmpneq_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 14[ 	]*vcmpneq_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 14[ 	]*vcmpneq_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 14[ 	]*vcmpneq_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 14[ 	]*vcmpneq_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 14[ 	]*vcmpneq_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 14[ 	]*vcmpneq_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 15[ 	]*vcmpnlt_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 15[ 	]*vcmpnlt_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 15[ 	]*vcmpnlt_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 15[ 	]*vcmpnlt_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 15[ 	]*vcmpnlt_uqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 15[ 	]*vcmpnlt_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 15[ 	]*vcmpnlt_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 15[ 	]*vcmpnlt_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 15[ 	]*vcmpnlt_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 15[ 	]*vcmpnlt_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 16[ 	]*vcmpnle_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 16[ 	]*vcmpnle_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 16[ 	]*vcmpnle_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 16[ 	]*vcmpnle_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 16[ 	]*vcmpnle_uqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 16[ 	]*vcmpnle_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 16[ 	]*vcmpnle_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 16[ 	]*vcmpnle_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 16[ 	]*vcmpnle_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 16[ 	]*vcmpnle_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 17[ 	]*vcmpord_sph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 17[ 	]*vcmpord_sph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 17[ 	]*vcmpord_sph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 17[ 	]*vcmpord_sph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 17[ 	]*vcmpord_sph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 17[ 	]*vcmpord_sph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 17[ 	]*vcmpord_sph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 17[ 	]*vcmpord_sph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 17[ 	]*vcmpord_sph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 17[ 	]*vcmpord_sph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 17[ 	]*vcmpord_sph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 17[ 	]*vcmpord_sph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 17[ 	]*vcmpord_sph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 17[ 	]*vcmpord_sph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 18[ 	]*vcmpeq_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 18[ 	]*vcmpeq_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 18[ 	]*vcmpeq_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 18[ 	]*vcmpeq_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 18[ 	]*vcmpeq_usph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 18[ 	]*vcmpeq_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 18[ 	]*vcmpeq_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 18[ 	]*vcmpeq_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 18[ 	]*vcmpeq_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 18[ 	]*vcmpeq_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 18[ 	]*vcmpeq_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 18[ 	]*vcmpeq_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 19[ 	]*vcmpnge_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 19[ 	]*vcmpnge_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 19[ 	]*vcmpnge_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 19[ 	]*vcmpnge_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 19[ 	]*vcmpnge_uqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 19[ 	]*vcmpnge_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 19[ 	]*vcmpnge_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 19[ 	]*vcmpnge_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 19[ 	]*vcmpnge_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 19[ 	]*vcmpnge_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1a[ 	]*vcmpngt_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1a[ 	]*vcmpngt_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1a[ 	]*vcmpngt_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1a[ 	]*vcmpngt_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1a[ 	]*vcmpngt_uqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1a[ 	]*vcmpngt_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1a[ 	]*vcmpngt_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1a[ 	]*vcmpngt_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1a[ 	]*vcmpngt_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1a[ 	]*vcmpngt_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1b[ 	]*vcmpfalse_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1b[ 	]*vcmpfalse_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1b[ 	]*vcmpfalse_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1b[ 	]*vcmpfalse_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1b[ 	]*vcmpfalse_osph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1b[ 	]*vcmpfalse_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1b[ 	]*vcmpfalse_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1b[ 	]*vcmpfalse_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1b[ 	]*vcmpfalse_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1b[ 	]*vcmpfalse_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1c[ 	]*vcmpneq_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1c[ 	]*vcmpneq_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1c[ 	]*vcmpneq_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1c[ 	]*vcmpneq_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1c[ 	]*vcmpneq_osph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1c[ 	]*vcmpneq_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1c[ 	]*vcmpneq_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1c[ 	]*vcmpneq_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1c[ 	]*vcmpneq_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1c[ 	]*vcmpneq_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1d[ 	]*vcmpge_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1d[ 	]*vcmpge_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1d[ 	]*vcmpge_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1d[ 	]*vcmpge_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1d[ 	]*vcmpge_oqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1d[ 	]*vcmpge_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1d[ 	]*vcmpge_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1d[ 	]*vcmpge_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1d[ 	]*vcmpge_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1d[ 	]*vcmpge_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1e[ 	]*vcmpgt_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1e[ 	]*vcmpgt_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1e[ 	]*vcmpgt_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1e[ 	]*vcmpgt_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1e[ 	]*vcmpgt_oqph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1e[ 	]*vcmpgt_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1e[ 	]*vcmpgt_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1e[ 	]*vcmpgt_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1e[ 	]*vcmpgt_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1e[ 	]*vcmpgt_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1f[ 	]*vcmptrue_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1f[ 	]*vcmptrue_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1f[ 	]*vcmptrue_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1f[ 	]*vcmptrue_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 23 01 00 00 1f[ 	]*vcmptrue_usph 0x123\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1f[ 	]*vcmptrue_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1f[ 	]*vcmptrue_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1f[ 	]*vcmptrue_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1f[ 	]*vcmptrue_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1f[ 	]*vcmptrue_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 00[ 	]*vcmpeqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 00[ 	]*vcmpeqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 00[ 	]*vcmpeqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 00[ 	]*vcmpeqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 00[ 	]*vcmpeqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 00[ 	]*vcmpeqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 01[ 	]*vcmpltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 01[ 	]*vcmpltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 01[ 	]*vcmpltsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 01[ 	]*vcmpltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 01[ 	]*vcmpltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 01[ 	]*vcmpltsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 02[ 	]*vcmplesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 02[ 	]*vcmplesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 02[ 	]*vcmplesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 02[ 	]*vcmplesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 02[ 	]*vcmplesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 02[ 	]*vcmplesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 02[ 	]*vcmplesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 02[ 	]*vcmplesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 03[ 	]*vcmpunordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 03[ 	]*vcmpunordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 03[ 	]*vcmpunordsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 03[ 	]*vcmpunordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 03[ 	]*vcmpunordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 03[ 	]*vcmpunordsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 04[ 	]*vcmpneqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 04[ 	]*vcmpneqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 04[ 	]*vcmpneqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 04[ 	]*vcmpneqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 04[ 	]*vcmpneqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 04[ 	]*vcmpneqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 05[ 	]*vcmpnltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 05[ 	]*vcmpnltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 05[ 	]*vcmpnltsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 05[ 	]*vcmpnltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 05[ 	]*vcmpnltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 05[ 	]*vcmpnltsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 06[ 	]*vcmpnlesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 06[ 	]*vcmpnlesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 06[ 	]*vcmpnlesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 06[ 	]*vcmpnlesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 06[ 	]*vcmpnlesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 06[ 	]*vcmpnlesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 07[ 	]*vcmpordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 07[ 	]*vcmpordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 07[ 	]*vcmpordsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 07[ 	]*vcmpordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 07[ 	]*vcmpordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 07[ 	]*vcmpordsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 08[ 	]*vcmpeq_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 08[ 	]*vcmpeq_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 08[ 	]*vcmpeq_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 08[ 	]*vcmpeq_uqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 09[ 	]*vcmpngesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 09[ 	]*vcmpngesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 09[ 	]*vcmpngesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 09[ 	]*vcmpngesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 09[ 	]*vcmpngesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 09[ 	]*vcmpngesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0a[ 	]*vcmpngtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0a[ 	]*vcmpngtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0a[ 	]*vcmpngtsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0a[ 	]*vcmpngtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0a[ 	]*vcmpngtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0a[ 	]*vcmpngtsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0b[ 	]*vcmpfalsesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0b[ 	]*vcmpfalsesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0b[ 	]*vcmpfalsesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0b[ 	]*vcmpfalsesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0b[ 	]*vcmpfalsesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0b[ 	]*vcmpfalsesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0c[ 	]*vcmpneq_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0c[ 	]*vcmpneq_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0c[ 	]*vcmpneq_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0c[ 	]*vcmpneq_oqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0d[ 	]*vcmpgesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0d[ 	]*vcmpgesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0d[ 	]*vcmpgesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0d[ 	]*vcmpgesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0d[ 	]*vcmpgesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0d[ 	]*vcmpgesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0e[ 	]*vcmpgtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0e[ 	]*vcmpgtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0e[ 	]*vcmpgtsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0e[ 	]*vcmpgtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0e[ 	]*vcmpgtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0e[ 	]*vcmpgtsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0f[ 	]*vcmptruesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0f[ 	]*vcmptruesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0f[ 	]*vcmptruesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0f[ 	]*vcmptruesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0f[ 	]*vcmptruesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 0f[ 	]*vcmptruesh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 10[ 	]*vcmpeq_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 10[ 	]*vcmpeq_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 10[ 	]*vcmpeq_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 10[ 	]*vcmpeq_ossh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 10[ 	]*vcmpeq_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 10[ 	]*vcmpeq_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 11[ 	]*vcmplt_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 11[ 	]*vcmplt_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 11[ 	]*vcmplt_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 11[ 	]*vcmplt_oqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 12[ 	]*vcmple_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 12[ 	]*vcmple_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 12[ 	]*vcmple_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 12[ 	]*vcmple_oqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 12[ 	]*vcmple_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 12[ 	]*vcmple_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 13[ 	]*vcmpunord_ssh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 13[ 	]*vcmpunord_ssh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 13[ 	]*vcmpunord_ssh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 13[ 	]*vcmpunord_ssh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 13[ 	]*vcmpunord_ssh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 13[ 	]*vcmpunord_ssh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_ssh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_ssh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 14[ 	]*vcmpneq_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 14[ 	]*vcmpneq_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 14[ 	]*vcmpneq_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 14[ 	]*vcmpneq_ussh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 14[ 	]*vcmpneq_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 14[ 	]*vcmpneq_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 15[ 	]*vcmpnlt_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 15[ 	]*vcmpnlt_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 15[ 	]*vcmpnlt_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 15[ 	]*vcmpnlt_uqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 16[ 	]*vcmpnle_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 16[ 	]*vcmpnle_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 16[ 	]*vcmpnle_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 16[ 	]*vcmpnle_uqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 17[ 	]*vcmpord_ssh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 17[ 	]*vcmpord_ssh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 17[ 	]*vcmpord_ssh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 17[ 	]*vcmpord_ssh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 17[ 	]*vcmpord_ssh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 17[ 	]*vcmpord_ssh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 17[ 	]*vcmpord_ssh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 17[ 	]*vcmpord_ssh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 18[ 	]*vcmpeq_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 18[ 	]*vcmpeq_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 18[ 	]*vcmpeq_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 18[ 	]*vcmpeq_ussh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 18[ 	]*vcmpeq_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 18[ 	]*vcmpeq_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 19[ 	]*vcmpnge_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 19[ 	]*vcmpnge_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 19[ 	]*vcmpnge_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 19[ 	]*vcmpnge_uqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1a[ 	]*vcmpngt_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1a[ 	]*vcmpngt_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1a[ 	]*vcmpngt_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1a[ 	]*vcmpngt_uqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1b[ 	]*vcmpfalse_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1b[ 	]*vcmpfalse_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1b[ 	]*vcmpfalse_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1b[ 	]*vcmpfalse_ossh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1c[ 	]*vcmpneq_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1c[ 	]*vcmpneq_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1c[ 	]*vcmpneq_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1c[ 	]*vcmpneq_ossh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1d[ 	]*vcmpge_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1d[ 	]*vcmpge_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1d[ 	]*vcmpge_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1d[ 	]*vcmpge_oqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1e[ 	]*vcmpgt_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1e[ 	]*vcmpgt_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1e[ 	]*vcmpgt_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1e[ 	]*vcmpgt_oqsh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1f[ 	]*vcmptrue_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1f[ 	]*vcmptrue_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1f[ 	]*vcmptrue_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 23 01 00 00 1f[ 	]*vcmptrue_ussh 0x123\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 00[ 	]*vcmpeqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 00[ 	]*vcmpeqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 00[ 	]*vcmpeqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 00[ 	]*vcmpeqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 00[ 	]*vcmpeqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 00[ 	]*vcmpeqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 00[ 	]*vcmpeqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 01[ 	]*vcmpltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 01[ 	]*vcmpltph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 01[ 	]*vcmpltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 01[ 	]*vcmpltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 01[ 	]*vcmpltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 01[ 	]*vcmpltph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 01[ 	]*vcmpltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 02[ 	]*vcmpleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 02[ 	]*vcmpleph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 02[ 	]*vcmpleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 02[ 	]*vcmpleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 02[ 	]*vcmpleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 02[ 	]*vcmpleph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 02[ 	]*vcmpleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 03[ 	]*vcmpunordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 03[ 	]*vcmpunordph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 03[ 	]*vcmpunordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 03[ 	]*vcmpunordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 03[ 	]*vcmpunordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 03[ 	]*vcmpunordph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 03[ 	]*vcmpunordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 04[ 	]*vcmpneqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 04[ 	]*vcmpneqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 04[ 	]*vcmpneqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 04[ 	]*vcmpneqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 04[ 	]*vcmpneqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 04[ 	]*vcmpneqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 04[ 	]*vcmpneqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 05[ 	]*vcmpnltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 05[ 	]*vcmpnltph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 05[ 	]*vcmpnltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 05[ 	]*vcmpnltph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 05[ 	]*vcmpnltph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 05[ 	]*vcmpnltph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 05[ 	]*vcmpnltph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 06[ 	]*vcmpnleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 06[ 	]*vcmpnleph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 06[ 	]*vcmpnleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 06[ 	]*vcmpnleph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 06[ 	]*vcmpnleph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 06[ 	]*vcmpnleph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 06[ 	]*vcmpnleph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 07[ 	]*vcmpordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 07[ 	]*vcmpordph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 07[ 	]*vcmpordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 07[ 	]*vcmpordph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 07[ 	]*vcmpordph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 07[ 	]*vcmpordph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 07[ 	]*vcmpordph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 08[ 	]*vcmpeq_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 08[ 	]*vcmpeq_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 08[ 	]*vcmpeq_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 08[ 	]*vcmpeq_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 08[ 	]*vcmpeq_uqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 08[ 	]*vcmpeq_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 08[ 	]*vcmpeq_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 08[ 	]*vcmpeq_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 08[ 	]*vcmpeq_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 08[ 	]*vcmpeq_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 09[ 	]*vcmpngeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 09[ 	]*vcmpngeph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 09[ 	]*vcmpngeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 09[ 	]*vcmpngeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 09[ 	]*vcmpngeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 09[ 	]*vcmpngeph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 09[ 	]*vcmpngeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0a[ 	]*vcmpngtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0a[ 	]*vcmpngtph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0a[ 	]*vcmpngtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0a[ 	]*vcmpngtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0a[ 	]*vcmpngtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0a[ 	]*vcmpngtph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0a[ 	]*vcmpngtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0b[ 	]*vcmpfalseph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0b[ 	]*vcmpfalseph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0b[ 	]*vcmpfalseph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0b[ 	]*vcmpfalseph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0b[ 	]*vcmpfalseph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0b[ 	]*vcmpfalseph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0b[ 	]*vcmpfalseph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0c[ 	]*vcmpneq_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0c[ 	]*vcmpneq_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0c[ 	]*vcmpneq_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0c[ 	]*vcmpneq_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0c[ 	]*vcmpneq_oqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0c[ 	]*vcmpneq_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0c[ 	]*vcmpneq_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0c[ 	]*vcmpneq_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0c[ 	]*vcmpneq_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0c[ 	]*vcmpneq_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0d[ 	]*vcmpgeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0d[ 	]*vcmpgeph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0d[ 	]*vcmpgeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0d[ 	]*vcmpgeph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0d[ 	]*vcmpgeph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0d[ 	]*vcmpgeph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0d[ 	]*vcmpgeph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0e[ 	]*vcmpgtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0e[ 	]*vcmpgtph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0e[ 	]*vcmpgtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0e[ 	]*vcmpgtph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0e[ 	]*vcmpgtph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0e[ 	]*vcmpgtph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0e[ 	]*vcmpgtph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0f[ 	]*vcmptrueph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0f[ 	]*vcmptrueph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0f[ 	]*vcmptrueph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 0f[ 	]*vcmptrueph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 0f[ 	]*vcmptrueph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 0f[ 	]*vcmptrueph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 0f[ 	]*vcmptrueph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 10[ 	]*vcmpeq_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 10[ 	]*vcmpeq_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 10[ 	]*vcmpeq_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 10[ 	]*vcmpeq_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 10[ 	]*vcmpeq_osph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 10[ 	]*vcmpeq_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 10[ 	]*vcmpeq_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 10[ 	]*vcmpeq_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 10[ 	]*vcmpeq_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 10[ 	]*vcmpeq_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 10[ 	]*vcmpeq_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 10[ 	]*vcmpeq_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 11[ 	]*vcmplt_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 11[ 	]*vcmplt_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 11[ 	]*vcmplt_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 11[ 	]*vcmplt_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 11[ 	]*vcmplt_oqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 11[ 	]*vcmplt_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 11[ 	]*vcmplt_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 11[ 	]*vcmplt_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 11[ 	]*vcmplt_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 11[ 	]*vcmplt_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 12[ 	]*vcmple_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 12[ 	]*vcmple_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 12[ 	]*vcmple_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 12[ 	]*vcmple_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 12[ 	]*vcmple_oqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 12[ 	]*vcmple_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 12[ 	]*vcmple_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 12[ 	]*vcmple_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 12[ 	]*vcmple_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 12[ 	]*vcmple_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 12[ 	]*vcmple_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 12[ 	]*vcmple_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 13[ 	]*vcmpunord_sph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 13[ 	]*vcmpunord_sph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 13[ 	]*vcmpunord_sph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 13[ 	]*vcmpunord_sph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 13[ 	]*vcmpunord_sph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 13[ 	]*vcmpunord_sph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 13[ 	]*vcmpunord_sph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 13[ 	]*vcmpunord_sph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 13[ 	]*vcmpunord_sph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 13[ 	]*vcmpunord_sph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 13[ 	]*vcmpunord_sph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 13[ 	]*vcmpunord_sph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_sph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_sph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 14[ 	]*vcmpneq_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 14[ 	]*vcmpneq_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 14[ 	]*vcmpneq_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 14[ 	]*vcmpneq_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 14[ 	]*vcmpneq_usph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 14[ 	]*vcmpneq_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 14[ 	]*vcmpneq_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 14[ 	]*vcmpneq_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 14[ 	]*vcmpneq_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 14[ 	]*vcmpneq_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 14[ 	]*vcmpneq_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 14[ 	]*vcmpneq_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 15[ 	]*vcmpnlt_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 15[ 	]*vcmpnlt_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 15[ 	]*vcmpnlt_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 15[ 	]*vcmpnlt_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 15[ 	]*vcmpnlt_uqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 15[ 	]*vcmpnlt_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 15[ 	]*vcmpnlt_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 15[ 	]*vcmpnlt_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 15[ 	]*vcmpnlt_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 15[ 	]*vcmpnlt_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 16[ 	]*vcmpnle_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 16[ 	]*vcmpnle_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 16[ 	]*vcmpnle_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 16[ 	]*vcmpnle_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 16[ 	]*vcmpnle_uqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 16[ 	]*vcmpnle_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 16[ 	]*vcmpnle_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 16[ 	]*vcmpnle_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 16[ 	]*vcmpnle_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 16[ 	]*vcmpnle_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 17[ 	]*vcmpord_sph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 17[ 	]*vcmpord_sph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 17[ 	]*vcmpord_sph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 17[ 	]*vcmpord_sph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 17[ 	]*vcmpord_sph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 17[ 	]*vcmpord_sph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 17[ 	]*vcmpord_sph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 17[ 	]*vcmpord_sph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 17[ 	]*vcmpord_sph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 17[ 	]*vcmpord_sph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 17[ 	]*vcmpord_sph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 17[ 	]*vcmpord_sph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 17[ 	]*vcmpord_sph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 17[ 	]*vcmpord_sph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 18[ 	]*vcmpeq_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 18[ 	]*vcmpeq_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 18[ 	]*vcmpeq_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 18[ 	]*vcmpeq_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 18[ 	]*vcmpeq_usph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 18[ 	]*vcmpeq_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 18[ 	]*vcmpeq_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 18[ 	]*vcmpeq_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 18[ 	]*vcmpeq_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 18[ 	]*vcmpeq_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 18[ 	]*vcmpeq_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 18[ 	]*vcmpeq_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 19[ 	]*vcmpnge_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 19[ 	]*vcmpnge_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 19[ 	]*vcmpnge_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 19[ 	]*vcmpnge_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 19[ 	]*vcmpnge_uqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 19[ 	]*vcmpnge_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 19[ 	]*vcmpnge_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 19[ 	]*vcmpnge_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 19[ 	]*vcmpnge_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 19[ 	]*vcmpnge_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1a[ 	]*vcmpngt_uqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1a[ 	]*vcmpngt_uqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1a[ 	]*vcmpngt_uqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1a[ 	]*vcmpngt_uqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1a[ 	]*vcmpngt_uqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1a[ 	]*vcmpngt_uqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1a[ 	]*vcmpngt_uqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1a[ 	]*vcmpngt_uqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1a[ 	]*vcmpngt_uqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1a[ 	]*vcmpngt_uqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1b[ 	]*vcmpfalse_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1b[ 	]*vcmpfalse_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1b[ 	]*vcmpfalse_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1b[ 	]*vcmpfalse_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1b[ 	]*vcmpfalse_osph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1b[ 	]*vcmpfalse_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1b[ 	]*vcmpfalse_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1b[ 	]*vcmpfalse_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1b[ 	]*vcmpfalse_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1b[ 	]*vcmpfalse_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1c[ 	]*vcmpneq_osph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1c[ 	]*vcmpneq_osph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1c[ 	]*vcmpneq_osph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1c[ 	]*vcmpneq_osph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1c[ 	]*vcmpneq_osph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1c[ 	]*vcmpneq_osph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1c[ 	]*vcmpneq_osph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1c[ 	]*vcmpneq_osph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1c[ 	]*vcmpneq_osph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1c[ 	]*vcmpneq_osph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_osph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_osph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_osph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_osph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1d[ 	]*vcmpge_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1d[ 	]*vcmpge_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1d[ 	]*vcmpge_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1d[ 	]*vcmpge_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1d[ 	]*vcmpge_oqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1d[ 	]*vcmpge_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1d[ 	]*vcmpge_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1d[ 	]*vcmpge_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1d[ 	]*vcmpge_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1d[ 	]*vcmpge_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1e[ 	]*vcmpgt_oqph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1e[ 	]*vcmpgt_oqph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1e[ 	]*vcmpgt_oqph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1e[ 	]*vcmpgt_oqph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1e[ 	]*vcmpgt_oqph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1e[ 	]*vcmpgt_oqph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1e[ 	]*vcmpgt_oqph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1e[ 	]*vcmpgt_oqph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1e[ 	]*vcmpgt_oqph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1e[ 	]*vcmpgt_oqph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 40 c2 ed 1f[ 	]*vcmptrue_usph %zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 47 c2 ed 1f[ 	]*vcmptrue_usph %zmm29,%zmm30,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 0c 10 c2 ed 1f[ 	]*vcmptrue_usph \{sae\},%zmm29,%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 29 1f[ 	]*vcmptrue_usph \(%rcx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 b3 0c 40 c2 ac f0 34 12 00 00 1f[ 	]*vcmptrue_usph 0x1234\(%rax,%r14,8\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 29 1f[ 	]*vcmptrue_usph \(%rcx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 7f 1f[ 	]*vcmptrue_usph 0x1fc0\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa 00 20 00 00 1f[ 	]*vcmptrue_usph 0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 6a 80 1f[ 	]*vcmptrue_usph -0x2000\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 40 c2 aa c0 df ff ff 1f[ 	]*vcmptrue_usph -0x2040\(%rdx\),%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_usph 0x3f8\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_usph 0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_usph -0x400\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 0c 50 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_usph -0x408\(%rdx\)\{1to32\},%zmm30,%k5
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 00[ 	]*vcmpeqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 00[ 	]*vcmpeqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 00[ 	]*vcmpeqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 00[ 	]*vcmpeqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 00[ 	]*vcmpeqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 00[ 	]*vcmpeqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 01[ 	]*vcmpltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 01[ 	]*vcmpltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 01[ 	]*vcmpltsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 01[ 	]*vcmpltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 01[ 	]*vcmpltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 01[ 	]*vcmpltsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 02[ 	]*vcmplesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 02[ 	]*vcmplesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 02[ 	]*vcmplesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 02[ 	]*vcmplesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 02[ 	]*vcmplesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 02[ 	]*vcmplesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 02[ 	]*vcmplesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 02[ 	]*vcmplesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 03[ 	]*vcmpunordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 03[ 	]*vcmpunordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 03[ 	]*vcmpunordsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 03[ 	]*vcmpunordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 03[ 	]*vcmpunordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 03[ 	]*vcmpunordsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 04[ 	]*vcmpneqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 04[ 	]*vcmpneqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 04[ 	]*vcmpneqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 04[ 	]*vcmpneqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 04[ 	]*vcmpneqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 04[ 	]*vcmpneqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 05[ 	]*vcmpnltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 05[ 	]*vcmpnltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 05[ 	]*vcmpnltsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 05[ 	]*vcmpnltsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 05[ 	]*vcmpnltsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 05[ 	]*vcmpnltsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 06[ 	]*vcmpnlesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 06[ 	]*vcmpnlesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 06[ 	]*vcmpnlesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 06[ 	]*vcmpnlesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 06[ 	]*vcmpnlesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 06[ 	]*vcmpnlesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 07[ 	]*vcmpordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 07[ 	]*vcmpordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 07[ 	]*vcmpordsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 07[ 	]*vcmpordsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 07[ 	]*vcmpordsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 07[ 	]*vcmpordsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 08[ 	]*vcmpeq_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 08[ 	]*vcmpeq_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 08[ 	]*vcmpeq_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 08[ 	]*vcmpeq_uqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 09[ 	]*vcmpngesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 09[ 	]*vcmpngesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 09[ 	]*vcmpngesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 09[ 	]*vcmpngesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 09[ 	]*vcmpngesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 09[ 	]*vcmpngesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0a[ 	]*vcmpngtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0a[ 	]*vcmpngtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0a[ 	]*vcmpngtsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0a[ 	]*vcmpngtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0a[ 	]*vcmpngtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0a[ 	]*vcmpngtsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0b[ 	]*vcmpfalsesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0b[ 	]*vcmpfalsesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0b[ 	]*vcmpfalsesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0b[ 	]*vcmpfalsesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0b[ 	]*vcmpfalsesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0b[ 	]*vcmpfalsesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0c[ 	]*vcmpneq_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0c[ 	]*vcmpneq_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0c[ 	]*vcmpneq_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0c[ 	]*vcmpneq_oqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0d[ 	]*vcmpgesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0d[ 	]*vcmpgesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0d[ 	]*vcmpgesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0d[ 	]*vcmpgesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0d[ 	]*vcmpgesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0d[ 	]*vcmpgesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0e[ 	]*vcmpgtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0e[ 	]*vcmpgtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0e[ 	]*vcmpgtsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0e[ 	]*vcmpgtsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0e[ 	]*vcmpgtsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0e[ 	]*vcmpgtsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0f[ 	]*vcmptruesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0f[ 	]*vcmptruesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0f[ 	]*vcmptruesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 0f[ 	]*vcmptruesh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 0f[ 	]*vcmptruesh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 0f[ 	]*vcmptruesh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 10[ 	]*vcmpeq_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 10[ 	]*vcmpeq_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 10[ 	]*vcmpeq_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 10[ 	]*vcmpeq_ossh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 10[ 	]*vcmpeq_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 10[ 	]*vcmpeq_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 11[ 	]*vcmplt_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 11[ 	]*vcmplt_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 11[ 	]*vcmplt_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 11[ 	]*vcmplt_oqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 12[ 	]*vcmple_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 12[ 	]*vcmple_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 12[ 	]*vcmple_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 12[ 	]*vcmple_oqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 12[ 	]*vcmple_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 12[ 	]*vcmple_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 13[ 	]*vcmpunord_ssh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 13[ 	]*vcmpunord_ssh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 13[ 	]*vcmpunord_ssh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 13[ 	]*vcmpunord_ssh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 13[ 	]*vcmpunord_ssh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 13[ 	]*vcmpunord_ssh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_ssh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_ssh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 14[ 	]*vcmpneq_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 14[ 	]*vcmpneq_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 14[ 	]*vcmpneq_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 14[ 	]*vcmpneq_ussh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 14[ 	]*vcmpneq_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 14[ 	]*vcmpneq_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 15[ 	]*vcmpnlt_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 15[ 	]*vcmpnlt_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 15[ 	]*vcmpnlt_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 15[ 	]*vcmpnlt_uqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 16[ 	]*vcmpnle_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 16[ 	]*vcmpnle_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 16[ 	]*vcmpnle_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 16[ 	]*vcmpnle_uqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 17[ 	]*vcmpord_ssh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 17[ 	]*vcmpord_ssh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 17[ 	]*vcmpord_ssh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 17[ 	]*vcmpord_ssh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 17[ 	]*vcmpord_ssh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 17[ 	]*vcmpord_ssh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 17[ 	]*vcmpord_ssh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 17[ 	]*vcmpord_ssh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 18[ 	]*vcmpeq_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 18[ 	]*vcmpeq_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 18[ 	]*vcmpeq_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 18[ 	]*vcmpeq_ussh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 18[ 	]*vcmpeq_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 18[ 	]*vcmpeq_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 19[ 	]*vcmpnge_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 19[ 	]*vcmpnge_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 19[ 	]*vcmpnge_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 19[ 	]*vcmpnge_uqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1a[ 	]*vcmpngt_uqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1a[ 	]*vcmpngt_uqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1a[ 	]*vcmpngt_uqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1a[ 	]*vcmpngt_uqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1b[ 	]*vcmpfalse_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1b[ 	]*vcmpfalse_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1b[ 	]*vcmpfalse_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1b[ 	]*vcmpfalse_ossh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1c[ 	]*vcmpneq_ossh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1c[ 	]*vcmpneq_ossh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1c[ 	]*vcmpneq_ossh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1c[ 	]*vcmpneq_ossh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_ossh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_ossh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_ossh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_ossh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1d[ 	]*vcmpge_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1d[ 	]*vcmpge_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1d[ 	]*vcmpge_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1d[ 	]*vcmpge_oqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1e[ 	]*vcmpgt_oqsh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1e[ 	]*vcmpgt_oqsh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1e[ 	]*vcmpgt_oqsh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1e[ 	]*vcmpgt_oqsh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqsh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqsh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqsh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqsh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 07 c2 ec 1f[ 	]*vcmptrue_ussh %xmm28,%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 93 16 17 c2 ec 1f[ 	]*vcmptrue_ussh \{sae\},%xmm28,%xmm29,%k5{%k7}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 29 1f[ 	]*vcmptrue_ussh \(%rcx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 b3 16 07 c2 ac f0 34 12 00 00 1f[ 	]*vcmptrue_ussh 0x1234\(%rax,%r14,8\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_ussh 0x3f8\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_ussh 0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_ussh -0x400\(%rdx\),%xmm29,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 16 07 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_ussh -0x408\(%rdx\),%xmm29,%k5\{%k7\}
#pass
