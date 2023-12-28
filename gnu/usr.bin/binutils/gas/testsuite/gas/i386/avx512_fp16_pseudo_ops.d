#as:
#objdump: -dw
#name: i386 VCM.*{PH,SH} insns
#source: avx512_fp16_pseudo_ops.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 00[ 	]*vcmpeqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 00[ 	]*vcmpeqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 00[ 	]*vcmpeqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 00[ 	]*vcmpeqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 01[ 	]*vcmpltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 01[ 	]*vcmpltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 01[ 	]*vcmpltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 01[ 	]*vcmpltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 02[ 	]*vcmpleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 02[ 	]*vcmpleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 02[ 	]*vcmpleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 02[ 	]*vcmpleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 02[ 	]*vcmpleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 02[ 	]*vcmpleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 03[ 	]*vcmpunordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 03[ 	]*vcmpunordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 03[ 	]*vcmpunordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 03[ 	]*vcmpunordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 04[ 	]*vcmpneqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 04[ 	]*vcmpneqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 04[ 	]*vcmpneqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 04[ 	]*vcmpneqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 05[ 	]*vcmpnltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 05[ 	]*vcmpnltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 05[ 	]*vcmpnltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 05[ 	]*vcmpnltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 06[ 	]*vcmpnleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 06[ 	]*vcmpnleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 06[ 	]*vcmpnleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 06[ 	]*vcmpnleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 07[ 	]*vcmpordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 07[ 	]*vcmpordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 07[ 	]*vcmpordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 07[ 	]*vcmpordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 08[ 	]*vcmpeq_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 08[ 	]*vcmpeq_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 08[ 	]*vcmpeq_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 08[ 	]*vcmpeq_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 08[ 	]*vcmpeq_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 08[ 	]*vcmpeq_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 08[ 	]*vcmpeq_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 08[ 	]*vcmpeq_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 08[ 	]*vcmpeq_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 08[ 	]*vcmpeq_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 09[ 	]*vcmpngeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 09[ 	]*vcmpngeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 09[ 	]*vcmpngeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 09[ 	]*vcmpngeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0a[ 	]*vcmpngtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0a[ 	]*vcmpngtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0a[ 	]*vcmpngtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0a[ 	]*vcmpngtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0b[ 	]*vcmpfalseph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalseph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0b[ 	]*vcmpfalseph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0b[ 	]*vcmpfalseph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalseph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0b[ 	]*vcmpfalseph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0c[ 	]*vcmpneq_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0c[ 	]*vcmpneq_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0c[ 	]*vcmpneq_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0c[ 	]*vcmpneq_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0c[ 	]*vcmpneq_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0c[ 	]*vcmpneq_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0c[ 	]*vcmpneq_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0c[ 	]*vcmpneq_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0c[ 	]*vcmpneq_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0c[ 	]*vcmpneq_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0d[ 	]*vcmpgeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0d[ 	]*vcmpgeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0d[ 	]*vcmpgeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0d[ 	]*vcmpgeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0e[ 	]*vcmpgtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0e[ 	]*vcmpgtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0e[ 	]*vcmpgtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0e[ 	]*vcmpgtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0f[ 	]*vcmptrueph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptrueph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0f[ 	]*vcmptrueph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0f[ 	]*vcmptrueph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptrueph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0f[ 	]*vcmptrueph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 10[ 	]*vcmpeq_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 10[ 	]*vcmpeq_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 10[ 	]*vcmpeq_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 10[ 	]*vcmpeq_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 10[ 	]*vcmpeq_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 10[ 	]*vcmpeq_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 10[ 	]*vcmpeq_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 10[ 	]*vcmpeq_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 10[ 	]*vcmpeq_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 10[ 	]*vcmpeq_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 10[ 	]*vcmpeq_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 10[ 	]*vcmpeq_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 11[ 	]*vcmplt_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 11[ 	]*vcmplt_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 11[ 	]*vcmplt_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 11[ 	]*vcmplt_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 11[ 	]*vcmplt_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 11[ 	]*vcmplt_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 11[ 	]*vcmplt_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 11[ 	]*vcmplt_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 11[ 	]*vcmplt_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 11[ 	]*vcmplt_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 12[ 	]*vcmple_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 12[ 	]*vcmple_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 12[ 	]*vcmple_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 12[ 	]*vcmple_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 12[ 	]*vcmple_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 12[ 	]*vcmple_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 12[ 	]*vcmple_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 12[ 	]*vcmple_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 12[ 	]*vcmple_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 12[ 	]*vcmple_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 12[ 	]*vcmple_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 12[ 	]*vcmple_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 13[ 	]*vcmpunord_sph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 13[ 	]*vcmpunord_sph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 13[ 	]*vcmpunord_sph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 13[ 	]*vcmpunord_sph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 13[ 	]*vcmpunord_sph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 13[ 	]*vcmpunord_sph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 13[ 	]*vcmpunord_sph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 13[ 	]*vcmpunord_sph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 13[ 	]*vcmpunord_sph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 13[ 	]*vcmpunord_sph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 13[ 	]*vcmpunord_sph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 13[ 	]*vcmpunord_sph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_sph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_sph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 14[ 	]*vcmpneq_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 14[ 	]*vcmpneq_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 14[ 	]*vcmpneq_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 14[ 	]*vcmpneq_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 14[ 	]*vcmpneq_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 14[ 	]*vcmpneq_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 14[ 	]*vcmpneq_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 14[ 	]*vcmpneq_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 14[ 	]*vcmpneq_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 14[ 	]*vcmpneq_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 14[ 	]*vcmpneq_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 14[ 	]*vcmpneq_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 15[ 	]*vcmpnlt_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 15[ 	]*vcmpnlt_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 15[ 	]*vcmpnlt_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 15[ 	]*vcmpnlt_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 15[ 	]*vcmpnlt_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 15[ 	]*vcmpnlt_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 15[ 	]*vcmpnlt_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 15[ 	]*vcmpnlt_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 15[ 	]*vcmpnlt_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 15[ 	]*vcmpnlt_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 16[ 	]*vcmpnle_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 16[ 	]*vcmpnle_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 16[ 	]*vcmpnle_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 16[ 	]*vcmpnle_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 16[ 	]*vcmpnle_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 16[ 	]*vcmpnle_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 16[ 	]*vcmpnle_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 16[ 	]*vcmpnle_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 16[ 	]*vcmpnle_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 16[ 	]*vcmpnle_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 17[ 	]*vcmpord_sph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 17[ 	]*vcmpord_sph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 17[ 	]*vcmpord_sph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 17[ 	]*vcmpord_sph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 17[ 	]*vcmpord_sph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 17[ 	]*vcmpord_sph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 17[ 	]*vcmpord_sph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 17[ 	]*vcmpord_sph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 17[ 	]*vcmpord_sph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 17[ 	]*vcmpord_sph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 17[ 	]*vcmpord_sph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 17[ 	]*vcmpord_sph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 17[ 	]*vcmpord_sph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 17[ 	]*vcmpord_sph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 18[ 	]*vcmpeq_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 18[ 	]*vcmpeq_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 18[ 	]*vcmpeq_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 18[ 	]*vcmpeq_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 18[ 	]*vcmpeq_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 18[ 	]*vcmpeq_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 18[ 	]*vcmpeq_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 18[ 	]*vcmpeq_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 18[ 	]*vcmpeq_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 18[ 	]*vcmpeq_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 18[ 	]*vcmpeq_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 18[ 	]*vcmpeq_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 19[ 	]*vcmpnge_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 19[ 	]*vcmpnge_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 19[ 	]*vcmpnge_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 19[ 	]*vcmpnge_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 19[ 	]*vcmpnge_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 19[ 	]*vcmpnge_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 19[ 	]*vcmpnge_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 19[ 	]*vcmpnge_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 19[ 	]*vcmpnge_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 19[ 	]*vcmpnge_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1a[ 	]*vcmpngt_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1a[ 	]*vcmpngt_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1a[ 	]*vcmpngt_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1a[ 	]*vcmpngt_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1a[ 	]*vcmpngt_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1a[ 	]*vcmpngt_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1a[ 	]*vcmpngt_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1a[ 	]*vcmpngt_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1a[ 	]*vcmpngt_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1a[ 	]*vcmpngt_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1b[ 	]*vcmpfalse_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1b[ 	]*vcmpfalse_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1b[ 	]*vcmpfalse_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1b[ 	]*vcmpfalse_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1b[ 	]*vcmpfalse_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1b[ 	]*vcmpfalse_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1b[ 	]*vcmpfalse_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1b[ 	]*vcmpfalse_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1b[ 	]*vcmpfalse_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1b[ 	]*vcmpfalse_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1c[ 	]*vcmpneq_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1c[ 	]*vcmpneq_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1c[ 	]*vcmpneq_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1c[ 	]*vcmpneq_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1c[ 	]*vcmpneq_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1c[ 	]*vcmpneq_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1c[ 	]*vcmpneq_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1c[ 	]*vcmpneq_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1c[ 	]*vcmpneq_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1c[ 	]*vcmpneq_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1d[ 	]*vcmpge_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1d[ 	]*vcmpge_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1d[ 	]*vcmpge_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1d[ 	]*vcmpge_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1d[ 	]*vcmpge_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1d[ 	]*vcmpge_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1d[ 	]*vcmpge_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1d[ 	]*vcmpge_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1d[ 	]*vcmpge_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1d[ 	]*vcmpge_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1e[ 	]*vcmpgt_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1e[ 	]*vcmpgt_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1e[ 	]*vcmpgt_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1e[ 	]*vcmpgt_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1e[ 	]*vcmpgt_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1e[ 	]*vcmpgt_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1e[ 	]*vcmpgt_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1e[ 	]*vcmpgt_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1e[ 	]*vcmpgt_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1e[ 	]*vcmpgt_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1f[ 	]*vcmptrue_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1f[ 	]*vcmptrue_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1f[ 	]*vcmptrue_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1f[ 	]*vcmptrue_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1f[ 	]*vcmptrue_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1f[ 	]*vcmptrue_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1f[ 	]*vcmptrue_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1f[ 	]*vcmptrue_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1f[ 	]*vcmptrue_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1f[ 	]*vcmptrue_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 00[ 	]*vcmpeqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 00[ 	]*vcmpeqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 00[ 	]*vcmpeqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 00[ 	]*vcmpeqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 01[ 	]*vcmpltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 01[ 	]*vcmpltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 01[ 	]*vcmpltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 01[ 	]*vcmpltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 02[ 	]*vcmplesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 02[ 	]*vcmplesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 02[ 	]*vcmplesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 02[ 	]*vcmplesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 02[ 	]*vcmplesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 02[ 	]*vcmplesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 02[ 	]*vcmplesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 02[ 	]*vcmplesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 03[ 	]*vcmpunordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 03[ 	]*vcmpunordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 03[ 	]*vcmpunordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 03[ 	]*vcmpunordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 04[ 	]*vcmpneqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 04[ 	]*vcmpneqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 04[ 	]*vcmpneqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 04[ 	]*vcmpneqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 05[ 	]*vcmpnltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 05[ 	]*vcmpnltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 05[ 	]*vcmpnltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 05[ 	]*vcmpnltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 06[ 	]*vcmpnlesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 06[ 	]*vcmpnlesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnlesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 06[ 	]*vcmpnlesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 06[ 	]*vcmpnlesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnlesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 07[ 	]*vcmpordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 07[ 	]*vcmpordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 07[ 	]*vcmpordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 07[ 	]*vcmpordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 08[ 	]*vcmpeq_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 08[ 	]*vcmpeq_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 08[ 	]*vcmpeq_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 08[ 	]*vcmpeq_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 09[ 	]*vcmpngesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 09[ 	]*vcmpngesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 09[ 	]*vcmpngesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 09[ 	]*vcmpngesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0a[ 	]*vcmpngtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0a[ 	]*vcmpngtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0a[ 	]*vcmpngtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0a[ 	]*vcmpngtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0b[ 	]*vcmpfalsesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0b[ 	]*vcmpfalsesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalsesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0b[ 	]*vcmpfalsesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0b[ 	]*vcmpfalsesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalsesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0c[ 	]*vcmpneq_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0c[ 	]*vcmpneq_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0c[ 	]*vcmpneq_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0c[ 	]*vcmpneq_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0d[ 	]*vcmpgesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0d[ 	]*vcmpgesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0d[ 	]*vcmpgesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0d[ 	]*vcmpgesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0e[ 	]*vcmpgtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0e[ 	]*vcmpgtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0e[ 	]*vcmpgtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0e[ 	]*vcmpgtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0f[ 	]*vcmptruesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0f[ 	]*vcmptruesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptruesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0f[ 	]*vcmptruesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0f[ 	]*vcmptruesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptruesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 10[ 	]*vcmpeq_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 10[ 	]*vcmpeq_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 10[ 	]*vcmpeq_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 10[ 	]*vcmpeq_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 10[ 	]*vcmpeq_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 10[ 	]*vcmpeq_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 10[ 	]*vcmpeq_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 10[ 	]*vcmpeq_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 11[ 	]*vcmplt_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 11[ 	]*vcmplt_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 11[ 	]*vcmplt_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 11[ 	]*vcmplt_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 11[ 	]*vcmplt_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 11[ 	]*vcmplt_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 12[ 	]*vcmple_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 12[ 	]*vcmple_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 12[ 	]*vcmple_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 12[ 	]*vcmple_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 12[ 	]*vcmple_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 12[ 	]*vcmple_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 12[ 	]*vcmple_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 12[ 	]*vcmple_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 13[ 	]*vcmpunord_ssh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 13[ 	]*vcmpunord_ssh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 13[ 	]*vcmpunord_ssh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 13[ 	]*vcmpunord_ssh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 13[ 	]*vcmpunord_ssh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 13[ 	]*vcmpunord_ssh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 13[ 	]*vcmpunord_ssh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 13[ 	]*vcmpunord_ssh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 14[ 	]*vcmpneq_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 14[ 	]*vcmpneq_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 14[ 	]*vcmpneq_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 14[ 	]*vcmpneq_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 14[ 	]*vcmpneq_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 14[ 	]*vcmpneq_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 14[ 	]*vcmpneq_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 14[ 	]*vcmpneq_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 15[ 	]*vcmpnlt_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 15[ 	]*vcmpnlt_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 15[ 	]*vcmpnlt_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 15[ 	]*vcmpnlt_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 16[ 	]*vcmpnle_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 16[ 	]*vcmpnle_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 16[ 	]*vcmpnle_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 16[ 	]*vcmpnle_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 17[ 	]*vcmpord_ssh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 17[ 	]*vcmpord_ssh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 17[ 	]*vcmpord_ssh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 17[ 	]*vcmpord_ssh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 17[ 	]*vcmpord_ssh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 17[ 	]*vcmpord_ssh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 17[ 	]*vcmpord_ssh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 17[ 	]*vcmpord_ssh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 18[ 	]*vcmpeq_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 18[ 	]*vcmpeq_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 18[ 	]*vcmpeq_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 18[ 	]*vcmpeq_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 18[ 	]*vcmpeq_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 18[ 	]*vcmpeq_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 18[ 	]*vcmpeq_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 18[ 	]*vcmpeq_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 19[ 	]*vcmpnge_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 19[ 	]*vcmpnge_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 19[ 	]*vcmpnge_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 19[ 	]*vcmpnge_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1a[ 	]*vcmpngt_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1a[ 	]*vcmpngt_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1a[ 	]*vcmpngt_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1a[ 	]*vcmpngt_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1b[ 	]*vcmpfalse_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1b[ 	]*vcmpfalse_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1b[ 	]*vcmpfalse_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1b[ 	]*vcmpfalse_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1c[ 	]*vcmpneq_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1c[ 	]*vcmpneq_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1c[ 	]*vcmpneq_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1c[ 	]*vcmpneq_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1c[ 	]*vcmpneq_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1c[ 	]*vcmpneq_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1d[ 	]*vcmpge_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1d[ 	]*vcmpge_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1d[ 	]*vcmpge_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1d[ 	]*vcmpge_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1e[ 	]*vcmpgt_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1e[ 	]*vcmpgt_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1e[ 	]*vcmpgt_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1e[ 	]*vcmpgt_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1f[ 	]*vcmptrue_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1f[ 	]*vcmptrue_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1f[ 	]*vcmptrue_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1f[ 	]*vcmptrue_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1f[ 	]*vcmptrue_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1f[ 	]*vcmptrue_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 00[ 	]*vcmpeqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 00[ 	]*vcmpeqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 00[ 	]*vcmpeqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 00[ 	]*vcmpeqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 00[ 	]*vcmpeqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 00[ 	]*vcmpeqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 00[ 	]*vcmpeqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 00[ 	]*vcmpeqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 00[ 	]*vcmpeqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 00[ 	]*vcmpeqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 00[ 	]*vcmpeqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 00[ 	]*vcmpeqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 00[ 	]*vcmpeqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 00[ 	]*vcmpeqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 01[ 	]*vcmpltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 01[ 	]*vcmpltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 01[ 	]*vcmpltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 01[ 	]*vcmpltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 01[ 	]*vcmpltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 01[ 	]*vcmpltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 01[ 	]*vcmpltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 01[ 	]*vcmpltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 01[ 	]*vcmpltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 01[ 	]*vcmpltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 01[ 	]*vcmpltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 01[ 	]*vcmpltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 01[ 	]*vcmpltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 01[ 	]*vcmpltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 02[ 	]*vcmpleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 02[ 	]*vcmpleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 02[ 	]*vcmpleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 02[ 	]*vcmpleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 02[ 	]*vcmpleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 02[ 	]*vcmpleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 02[ 	]*vcmpleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 02[ 	]*vcmpleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 02[ 	]*vcmpleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 02[ 	]*vcmpleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 02[ 	]*vcmpleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 02[ 	]*vcmpleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 02[ 	]*vcmpleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 02[ 	]*vcmpleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 02[ 	]*vcmpleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 02[ 	]*vcmpleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 03[ 	]*vcmpunordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 03[ 	]*vcmpunordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 03[ 	]*vcmpunordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 03[ 	]*vcmpunordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 03[ 	]*vcmpunordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 03[ 	]*vcmpunordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 03[ 	]*vcmpunordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 03[ 	]*vcmpunordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 03[ 	]*vcmpunordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 03[ 	]*vcmpunordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 03[ 	]*vcmpunordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 03[ 	]*vcmpunordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 03[ 	]*vcmpunordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 03[ 	]*vcmpunordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 04[ 	]*vcmpneqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 04[ 	]*vcmpneqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 04[ 	]*vcmpneqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 04[ 	]*vcmpneqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 04[ 	]*vcmpneqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 04[ 	]*vcmpneqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 04[ 	]*vcmpneqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 04[ 	]*vcmpneqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 04[ 	]*vcmpneqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 04[ 	]*vcmpneqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 04[ 	]*vcmpneqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 04[ 	]*vcmpneqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 04[ 	]*vcmpneqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 04[ 	]*vcmpneqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 05[ 	]*vcmpnltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 05[ 	]*vcmpnltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 05[ 	]*vcmpnltph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 05[ 	]*vcmpnltph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 05[ 	]*vcmpnltph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 05[ 	]*vcmpnltph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 05[ 	]*vcmpnltph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 05[ 	]*vcmpnltph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 05[ 	]*vcmpnltph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 05[ 	]*vcmpnltph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 05[ 	]*vcmpnltph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 05[ 	]*vcmpnltph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 05[ 	]*vcmpnltph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 05[ 	]*vcmpnltph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 06[ 	]*vcmpnleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 06[ 	]*vcmpnleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 06[ 	]*vcmpnleph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 06[ 	]*vcmpnleph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 06[ 	]*vcmpnleph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnleph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 06[ 	]*vcmpnleph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 06[ 	]*vcmpnleph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 06[ 	]*vcmpnleph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 06[ 	]*vcmpnleph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 06[ 	]*vcmpnleph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 06[ 	]*vcmpnleph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 06[ 	]*vcmpnleph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 06[ 	]*vcmpnleph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 06[ 	]*vcmpnleph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 07[ 	]*vcmpordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 07[ 	]*vcmpordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 07[ 	]*vcmpordph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 07[ 	]*vcmpordph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 07[ 	]*vcmpordph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 07[ 	]*vcmpordph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 07[ 	]*vcmpordph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 07[ 	]*vcmpordph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 07[ 	]*vcmpordph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 07[ 	]*vcmpordph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 07[ 	]*vcmpordph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 07[ 	]*vcmpordph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 07[ 	]*vcmpordph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 07[ 	]*vcmpordph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 08[ 	]*vcmpeq_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 08[ 	]*vcmpeq_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 08[ 	]*vcmpeq_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 08[ 	]*vcmpeq_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 08[ 	]*vcmpeq_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 08[ 	]*vcmpeq_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 08[ 	]*vcmpeq_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 08[ 	]*vcmpeq_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 08[ 	]*vcmpeq_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 08[ 	]*vcmpeq_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 09[ 	]*vcmpngeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 09[ 	]*vcmpngeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 09[ 	]*vcmpngeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 09[ 	]*vcmpngeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 09[ 	]*vcmpngeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 09[ 	]*vcmpngeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 09[ 	]*vcmpngeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 09[ 	]*vcmpngeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 09[ 	]*vcmpngeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 09[ 	]*vcmpngeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 09[ 	]*vcmpngeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 09[ 	]*vcmpngeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 09[ 	]*vcmpngeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 09[ 	]*vcmpngeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0a[ 	]*vcmpngtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0a[ 	]*vcmpngtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0a[ 	]*vcmpngtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0a[ 	]*vcmpngtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0a[ 	]*vcmpngtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0a[ 	]*vcmpngtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0a[ 	]*vcmpngtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0a[ 	]*vcmpngtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0a[ 	]*vcmpngtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0a[ 	]*vcmpngtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0a[ 	]*vcmpngtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0a[ 	]*vcmpngtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0a[ 	]*vcmpngtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0a[ 	]*vcmpngtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0b[ 	]*vcmpfalseph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalseph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0b[ 	]*vcmpfalseph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0b[ 	]*vcmpfalseph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0b[ 	]*vcmpfalseph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0b[ 	]*vcmpfalseph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalseph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0b[ 	]*vcmpfalseph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0b[ 	]*vcmpfalseph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0b[ 	]*vcmpfalseph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0b[ 	]*vcmpfalseph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0b[ 	]*vcmpfalseph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0b[ 	]*vcmpfalseph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0b[ 	]*vcmpfalseph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0b[ 	]*vcmpfalseph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0b[ 	]*vcmpfalseph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0c[ 	]*vcmpneq_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0c[ 	]*vcmpneq_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0c[ 	]*vcmpneq_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0c[ 	]*vcmpneq_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0c[ 	]*vcmpneq_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0c[ 	]*vcmpneq_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0c[ 	]*vcmpneq_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0c[ 	]*vcmpneq_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0c[ 	]*vcmpneq_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0c[ 	]*vcmpneq_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0d[ 	]*vcmpgeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0d[ 	]*vcmpgeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0d[ 	]*vcmpgeph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0d[ 	]*vcmpgeph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0d[ 	]*vcmpgeph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgeph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0d[ 	]*vcmpgeph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0d[ 	]*vcmpgeph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0d[ 	]*vcmpgeph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0d[ 	]*vcmpgeph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0d[ 	]*vcmpgeph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0d[ 	]*vcmpgeph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0d[ 	]*vcmpgeph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0d[ 	]*vcmpgeph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0d[ 	]*vcmpgeph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0e[ 	]*vcmpgtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0e[ 	]*vcmpgtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0e[ 	]*vcmpgtph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0e[ 	]*vcmpgtph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0e[ 	]*vcmpgtph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0e[ 	]*vcmpgtph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0e[ 	]*vcmpgtph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0e[ 	]*vcmpgtph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0e[ 	]*vcmpgtph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0e[ 	]*vcmpgtph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0e[ 	]*vcmpgtph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0e[ 	]*vcmpgtph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0e[ 	]*vcmpgtph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0e[ 	]*vcmpgtph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0f[ 	]*vcmptrueph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptrueph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0f[ 	]*vcmptrueph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 0f[ 	]*vcmptrueph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 0f[ 	]*vcmptrueph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 0f[ 	]*vcmptrueph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptrueph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 0f[ 	]*vcmptrueph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 0f[ 	]*vcmptrueph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 0f[ 	]*vcmptrueph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 0f[ 	]*vcmptrueph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 0f[ 	]*vcmptrueph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 0f[ 	]*vcmptrueph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 0f[ 	]*vcmptrueph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 0f[ 	]*vcmptrueph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 0f[ 	]*vcmptrueph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 10[ 	]*vcmpeq_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 10[ 	]*vcmpeq_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 10[ 	]*vcmpeq_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 10[ 	]*vcmpeq_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 10[ 	]*vcmpeq_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 10[ 	]*vcmpeq_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 10[ 	]*vcmpeq_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 10[ 	]*vcmpeq_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 10[ 	]*vcmpeq_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 10[ 	]*vcmpeq_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 10[ 	]*vcmpeq_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 10[ 	]*vcmpeq_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 10[ 	]*vcmpeq_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 10[ 	]*vcmpeq_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 11[ 	]*vcmplt_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 11[ 	]*vcmplt_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 11[ 	]*vcmplt_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 11[ 	]*vcmplt_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 11[ 	]*vcmplt_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 11[ 	]*vcmplt_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 11[ 	]*vcmplt_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 11[ 	]*vcmplt_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 11[ 	]*vcmplt_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 11[ 	]*vcmplt_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 11[ 	]*vcmplt_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 11[ 	]*vcmplt_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 12[ 	]*vcmple_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 12[ 	]*vcmple_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 12[ 	]*vcmple_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 12[ 	]*vcmple_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 12[ 	]*vcmple_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 12[ 	]*vcmple_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 12[ 	]*vcmple_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 12[ 	]*vcmple_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 12[ 	]*vcmple_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 12[ 	]*vcmple_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 12[ 	]*vcmple_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 12[ 	]*vcmple_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 12[ 	]*vcmple_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 12[ 	]*vcmple_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 13[ 	]*vcmpunord_sph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 13[ 	]*vcmpunord_sph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 13[ 	]*vcmpunord_sph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 13[ 	]*vcmpunord_sph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 13[ 	]*vcmpunord_sph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 13[ 	]*vcmpunord_sph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 13[ 	]*vcmpunord_sph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 13[ 	]*vcmpunord_sph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 13[ 	]*vcmpunord_sph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 13[ 	]*vcmpunord_sph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 13[ 	]*vcmpunord_sph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 13[ 	]*vcmpunord_sph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 13[ 	]*vcmpunord_sph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 13[ 	]*vcmpunord_sph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 14[ 	]*vcmpneq_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 14[ 	]*vcmpneq_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 14[ 	]*vcmpneq_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 14[ 	]*vcmpneq_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 14[ 	]*vcmpneq_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 14[ 	]*vcmpneq_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 14[ 	]*vcmpneq_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 14[ 	]*vcmpneq_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 14[ 	]*vcmpneq_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 14[ 	]*vcmpneq_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 14[ 	]*vcmpneq_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 14[ 	]*vcmpneq_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 14[ 	]*vcmpneq_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 14[ 	]*vcmpneq_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 15[ 	]*vcmpnlt_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 15[ 	]*vcmpnlt_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 15[ 	]*vcmpnlt_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 15[ 	]*vcmpnlt_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 15[ 	]*vcmpnlt_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 15[ 	]*vcmpnlt_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 15[ 	]*vcmpnlt_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 15[ 	]*vcmpnlt_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 15[ 	]*vcmpnlt_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 15[ 	]*vcmpnlt_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 16[ 	]*vcmpnle_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 16[ 	]*vcmpnle_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 16[ 	]*vcmpnle_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 16[ 	]*vcmpnle_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 16[ 	]*vcmpnle_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 16[ 	]*vcmpnle_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 16[ 	]*vcmpnle_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 16[ 	]*vcmpnle_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 16[ 	]*vcmpnle_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 16[ 	]*vcmpnle_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 17[ 	]*vcmpord_sph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 17[ 	]*vcmpord_sph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 17[ 	]*vcmpord_sph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 17[ 	]*vcmpord_sph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 17[ 	]*vcmpord_sph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 17[ 	]*vcmpord_sph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 17[ 	]*vcmpord_sph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 17[ 	]*vcmpord_sph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 17[ 	]*vcmpord_sph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 17[ 	]*vcmpord_sph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 17[ 	]*vcmpord_sph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 17[ 	]*vcmpord_sph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 17[ 	]*vcmpord_sph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 17[ 	]*vcmpord_sph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 18[ 	]*vcmpeq_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 18[ 	]*vcmpeq_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 18[ 	]*vcmpeq_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 18[ 	]*vcmpeq_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 18[ 	]*vcmpeq_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 18[ 	]*vcmpeq_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 18[ 	]*vcmpeq_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 18[ 	]*vcmpeq_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 18[ 	]*vcmpeq_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 18[ 	]*vcmpeq_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 18[ 	]*vcmpeq_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 18[ 	]*vcmpeq_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 18[ 	]*vcmpeq_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 18[ 	]*vcmpeq_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 19[ 	]*vcmpnge_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 19[ 	]*vcmpnge_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 19[ 	]*vcmpnge_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 19[ 	]*vcmpnge_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 19[ 	]*vcmpnge_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 19[ 	]*vcmpnge_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 19[ 	]*vcmpnge_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 19[ 	]*vcmpnge_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 19[ 	]*vcmpnge_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 19[ 	]*vcmpnge_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1a[ 	]*vcmpngt_uqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1a[ 	]*vcmpngt_uqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1a[ 	]*vcmpngt_uqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1a[ 	]*vcmpngt_uqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1a[ 	]*vcmpngt_uqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1a[ 	]*vcmpngt_uqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1a[ 	]*vcmpngt_uqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1a[ 	]*vcmpngt_uqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1a[ 	]*vcmpngt_uqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1a[ 	]*vcmpngt_uqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1b[ 	]*vcmpfalse_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1b[ 	]*vcmpfalse_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1b[ 	]*vcmpfalse_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1b[ 	]*vcmpfalse_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1b[ 	]*vcmpfalse_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1b[ 	]*vcmpfalse_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1b[ 	]*vcmpfalse_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1b[ 	]*vcmpfalse_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1b[ 	]*vcmpfalse_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1b[ 	]*vcmpfalse_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1c[ 	]*vcmpneq_osph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1c[ 	]*vcmpneq_osph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1c[ 	]*vcmpneq_osph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1c[ 	]*vcmpneq_osph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1c[ 	]*vcmpneq_osph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1c[ 	]*vcmpneq_osph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1c[ 	]*vcmpneq_osph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1c[ 	]*vcmpneq_osph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1c[ 	]*vcmpneq_osph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1c[ 	]*vcmpneq_osph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1c[ 	]*vcmpneq_osph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1c[ 	]*vcmpneq_osph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_osph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_osph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1d[ 	]*vcmpge_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1d[ 	]*vcmpge_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1d[ 	]*vcmpge_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1d[ 	]*vcmpge_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1d[ 	]*vcmpge_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1d[ 	]*vcmpge_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1d[ 	]*vcmpge_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1d[ 	]*vcmpge_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1d[ 	]*vcmpge_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1d[ 	]*vcmpge_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1e[ 	]*vcmpgt_oqph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1e[ 	]*vcmpgt_oqph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1e[ 	]*vcmpgt_oqph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1e[ 	]*vcmpgt_oqph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1e[ 	]*vcmpgt_oqph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1e[ 	]*vcmpgt_oqph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1e[ 	]*vcmpgt_oqph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1e[ 	]*vcmpgt_oqph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1e[ 	]*vcmpgt_oqph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1e[ 	]*vcmpgt_oqph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ed 1f[ 	]*vcmptrue_usph %zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 4f c2 ed 1f[ 	]*vcmptrue_usph %zmm5,%zmm6,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 18 c2 ed 1f[ 	]*vcmptrue_usph \{sae\},%zmm5,%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 29 1f[ 	]*vcmptrue_usph \(%ecx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 ac f4 c0 1d fe ff 1f[ 	]*vcmptrue_usph -0x1e240\(%esp,%esi,8\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 28 1f[ 	]*vcmptrue_usph \(%eax\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 7f 1f[ 	]*vcmptrue_usph 0x1fc0\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa 00 20 00 00 1f[ 	]*vcmptrue_usph 0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 6a 80 1f[ 	]*vcmptrue_usph -0x2000\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 48 c2 aa c0 df ff ff 1f[ 	]*vcmptrue_usph -0x2040\(%edx\),%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 03 00 00 1f[ 	]*vcmptrue_usph 0x3f8\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 04 00 00 1f[ 	]*vcmptrue_usph 0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_usph -0x400\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4c 58 c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_usph -0x408\(%edx\)\{1to32\},%zmm6,%k5
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 00[ 	]*vcmpeqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 00[ 	]*vcmpeqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 00[ 	]*vcmpeqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 00[ 	]*vcmpeqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 00[ 	]*vcmpeqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 00[ 	]*vcmpeqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 00[ 	]*vcmpeqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 00[ 	]*vcmpeqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 00[ 	]*vcmpeqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 00[ 	]*vcmpeqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 01[ 	]*vcmpltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 01[ 	]*vcmpltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 01[ 	]*vcmpltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 01[ 	]*vcmpltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 01[ 	]*vcmpltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 01[ 	]*vcmpltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 01[ 	]*vcmpltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 01[ 	]*vcmpltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 01[ 	]*vcmpltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 01[ 	]*vcmpltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 02[ 	]*vcmplesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 02[ 	]*vcmplesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 02[ 	]*vcmplesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 02[ 	]*vcmplesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 02[ 	]*vcmplesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 02[ 	]*vcmplesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 02[ 	]*vcmplesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 02[ 	]*vcmplesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 02[ 	]*vcmplesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 02[ 	]*vcmplesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 02[ 	]*vcmplesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 02[ 	]*vcmplesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 03[ 	]*vcmpunordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 03[ 	]*vcmpunordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 03[ 	]*vcmpunordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 03[ 	]*vcmpunordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 03[ 	]*vcmpunordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 03[ 	]*vcmpunordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 03[ 	]*vcmpunordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 03[ 	]*vcmpunordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 03[ 	]*vcmpunordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 03[ 	]*vcmpunordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 04[ 	]*vcmpneqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 04[ 	]*vcmpneqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 04[ 	]*vcmpneqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 04[ 	]*vcmpneqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 04[ 	]*vcmpneqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 04[ 	]*vcmpneqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 04[ 	]*vcmpneqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 04[ 	]*vcmpneqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 04[ 	]*vcmpneqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 04[ 	]*vcmpneqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 05[ 	]*vcmpnltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 05[ 	]*vcmpnltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 05[ 	]*vcmpnltsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 05[ 	]*vcmpnltsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 05[ 	]*vcmpnltsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 05[ 	]*vcmpnltsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 05[ 	]*vcmpnltsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 05[ 	]*vcmpnltsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 05[ 	]*vcmpnltsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 05[ 	]*vcmpnltsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 06[ 	]*vcmpnlesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 06[ 	]*vcmpnlesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnlesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 06[ 	]*vcmpnlesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 06[ 	]*vcmpnlesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 06[ 	]*vcmpnlesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 06[ 	]*vcmpnlesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 06[ 	]*vcmpnlesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 06[ 	]*vcmpnlesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 06[ 	]*vcmpnlesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 06[ 	]*vcmpnlesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 07[ 	]*vcmpordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 07[ 	]*vcmpordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 07[ 	]*vcmpordsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 07[ 	]*vcmpordsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 07[ 	]*vcmpordsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 07[ 	]*vcmpordsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 07[ 	]*vcmpordsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 07[ 	]*vcmpordsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 07[ 	]*vcmpordsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 07[ 	]*vcmpordsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 08[ 	]*vcmpeq_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 08[ 	]*vcmpeq_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 08[ 	]*vcmpeq_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 08[ 	]*vcmpeq_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 08[ 	]*vcmpeq_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 08[ 	]*vcmpeq_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 08[ 	]*vcmpeq_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 08[ 	]*vcmpeq_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 09[ 	]*vcmpngesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 09[ 	]*vcmpngesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 09[ 	]*vcmpngesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 09[ 	]*vcmpngesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 09[ 	]*vcmpngesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 09[ 	]*vcmpngesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 09[ 	]*vcmpngesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 09[ 	]*vcmpngesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 09[ 	]*vcmpngesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 09[ 	]*vcmpngesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0a[ 	]*vcmpngtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0a[ 	]*vcmpngtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0a[ 	]*vcmpngtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0a[ 	]*vcmpngtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0a[ 	]*vcmpngtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0a[ 	]*vcmpngtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0a[ 	]*vcmpngtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0a[ 	]*vcmpngtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0a[ 	]*vcmpngtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0a[ 	]*vcmpngtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0b[ 	]*vcmpfalsesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0b[ 	]*vcmpfalsesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalsesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0b[ 	]*vcmpfalsesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0b[ 	]*vcmpfalsesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0b[ 	]*vcmpfalsesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0b[ 	]*vcmpfalsesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0b[ 	]*vcmpfalsesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0b[ 	]*vcmpfalsesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0b[ 	]*vcmpfalsesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0b[ 	]*vcmpfalsesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0c[ 	]*vcmpneq_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0c[ 	]*vcmpneq_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0c[ 	]*vcmpneq_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0c[ 	]*vcmpneq_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0c[ 	]*vcmpneq_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0c[ 	]*vcmpneq_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0c[ 	]*vcmpneq_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0c[ 	]*vcmpneq_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0d[ 	]*vcmpgesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0d[ 	]*vcmpgesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0d[ 	]*vcmpgesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0d[ 	]*vcmpgesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0d[ 	]*vcmpgesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0d[ 	]*vcmpgesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0d[ 	]*vcmpgesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0d[ 	]*vcmpgesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0d[ 	]*vcmpgesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0d[ 	]*vcmpgesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0e[ 	]*vcmpgtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0e[ 	]*vcmpgtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0e[ 	]*vcmpgtsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0e[ 	]*vcmpgtsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0e[ 	]*vcmpgtsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0e[ 	]*vcmpgtsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0e[ 	]*vcmpgtsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0e[ 	]*vcmpgtsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0e[ 	]*vcmpgtsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0e[ 	]*vcmpgtsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0f[ 	]*vcmptruesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0f[ 	]*vcmptruesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptruesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 0f[ 	]*vcmptruesh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 0f[ 	]*vcmptruesh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 0f[ 	]*vcmptruesh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 0f[ 	]*vcmptruesh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 0f[ 	]*vcmptruesh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 0f[ 	]*vcmptruesh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 0f[ 	]*vcmptruesh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 0f[ 	]*vcmptruesh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 10[ 	]*vcmpeq_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 10[ 	]*vcmpeq_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 10[ 	]*vcmpeq_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 10[ 	]*vcmpeq_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 10[ 	]*vcmpeq_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 10[ 	]*vcmpeq_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 10[ 	]*vcmpeq_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 10[ 	]*vcmpeq_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 11[ 	]*vcmplt_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 11[ 	]*vcmplt_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 11[ 	]*vcmplt_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 11[ 	]*vcmplt_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 11[ 	]*vcmplt_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 11[ 	]*vcmplt_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 11[ 	]*vcmplt_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 11[ 	]*vcmplt_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 12[ 	]*vcmple_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 12[ 	]*vcmple_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 12[ 	]*vcmple_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 12[ 	]*vcmple_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 12[ 	]*vcmple_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 12[ 	]*vcmple_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 12[ 	]*vcmple_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 12[ 	]*vcmple_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 13[ 	]*vcmpunord_ssh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 13[ 	]*vcmpunord_ssh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 13[ 	]*vcmpunord_ssh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 13[ 	]*vcmpunord_ssh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 13[ 	]*vcmpunord_ssh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 13[ 	]*vcmpunord_ssh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 13[ 	]*vcmpunord_ssh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 13[ 	]*vcmpunord_ssh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 14[ 	]*vcmpneq_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 14[ 	]*vcmpneq_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 14[ 	]*vcmpneq_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 14[ 	]*vcmpneq_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 14[ 	]*vcmpneq_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 14[ 	]*vcmpneq_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 14[ 	]*vcmpneq_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 14[ 	]*vcmpneq_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 15[ 	]*vcmpnlt_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 15[ 	]*vcmpnlt_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 15[ 	]*vcmpnlt_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 15[ 	]*vcmpnlt_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 15[ 	]*vcmpnlt_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 15[ 	]*vcmpnlt_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 15[ 	]*vcmpnlt_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 15[ 	]*vcmpnlt_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 16[ 	]*vcmpnle_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 16[ 	]*vcmpnle_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 16[ 	]*vcmpnle_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 16[ 	]*vcmpnle_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 16[ 	]*vcmpnle_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 16[ 	]*vcmpnle_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 16[ 	]*vcmpnle_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 16[ 	]*vcmpnle_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 17[ 	]*vcmpord_ssh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 17[ 	]*vcmpord_ssh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 17[ 	]*vcmpord_ssh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 17[ 	]*vcmpord_ssh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 17[ 	]*vcmpord_ssh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 17[ 	]*vcmpord_ssh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 17[ 	]*vcmpord_ssh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 17[ 	]*vcmpord_ssh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 18[ 	]*vcmpeq_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 18[ 	]*vcmpeq_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 18[ 	]*vcmpeq_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 18[ 	]*vcmpeq_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 18[ 	]*vcmpeq_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 18[ 	]*vcmpeq_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 18[ 	]*vcmpeq_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 18[ 	]*vcmpeq_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 19[ 	]*vcmpnge_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 19[ 	]*vcmpnge_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 19[ 	]*vcmpnge_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 19[ 	]*vcmpnge_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 19[ 	]*vcmpnge_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 19[ 	]*vcmpnge_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 19[ 	]*vcmpnge_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 19[ 	]*vcmpnge_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1a[ 	]*vcmpngt_uqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1a[ 	]*vcmpngt_uqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1a[ 	]*vcmpngt_uqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1a[ 	]*vcmpngt_uqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1a[ 	]*vcmpngt_uqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1a[ 	]*vcmpngt_uqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1a[ 	]*vcmpngt_uqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1a[ 	]*vcmpngt_uqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1b[ 	]*vcmpfalse_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1b[ 	]*vcmpfalse_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1b[ 	]*vcmpfalse_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1b[ 	]*vcmpfalse_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1b[ 	]*vcmpfalse_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1b[ 	]*vcmpfalse_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1b[ 	]*vcmpfalse_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1b[ 	]*vcmpfalse_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1c[ 	]*vcmpneq_ossh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1c[ 	]*vcmpneq_ossh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1c[ 	]*vcmpneq_ossh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1c[ 	]*vcmpneq_ossh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1c[ 	]*vcmpneq_ossh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1c[ 	]*vcmpneq_ossh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1c[ 	]*vcmpneq_ossh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1c[ 	]*vcmpneq_ossh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1d[ 	]*vcmpge_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1d[ 	]*vcmpge_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1d[ 	]*vcmpge_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1d[ 	]*vcmpge_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1d[ 	]*vcmpge_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1d[ 	]*vcmpge_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1d[ 	]*vcmpge_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1d[ 	]*vcmpge_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1e[ 	]*vcmpgt_oqsh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1e[ 	]*vcmpgt_oqsh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1e[ 	]*vcmpgt_oqsh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1e[ 	]*vcmpgt_oqsh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1e[ 	]*vcmpgt_oqsh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1e[ 	]*vcmpgt_oqsh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1e[ 	]*vcmpgt_oqsh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1e[ 	]*vcmpgt_oqsh -0x408\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ec 1f[ 	]*vcmptrue_ussh %xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 1f[ 	]*vcmptrue_ussh \{sae\},%xmm4,%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 29 1f[ 	]*vcmptrue_ussh \(%ecx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 c0 1d fe ff 1f[ 	]*vcmptrue_ussh -0x1e240\(%esp,%esi,8\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 03 00 00 1f[ 	]*vcmptrue_ussh 0x3f8\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 04 00 00 1f[ 	]*vcmptrue_ussh 0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa 00 fc ff ff 1f[ 	]*vcmptrue_ussh -0x400\(%edx\),%xmm5,%k5\{%k7\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 aa f8 fb ff ff 1f[ 	]*vcmptrue_ussh -0x408\(%edx\),%xmm5,%k5\{%k7\}
#pass
