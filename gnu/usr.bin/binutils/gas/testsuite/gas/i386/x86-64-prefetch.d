#as: --divide
#objdump: -dw
#name: x86-64 prefetch
#source: prefetch.s

.*: +file format .*

Disassembly of section .text:

0+ <amd_prefetch>:
\s*[a-f0-9]+:	0f 0d 00             	prefetch \(%rax\)
\s*[a-f0-9]+:	0f 0d 08             	prefetchw \(%rax\)
\s*[a-f0-9]+:	0f 0d 10             	prefetchwt1 \(%rax\)
\s*[a-f0-9]+:	0f 0d 18             	prefetch \(%rax\)
\s*[a-f0-9]+:	0f 0d 20             	prefetch \(%rax\)
\s*[a-f0-9]+:	0f 0d 28             	prefetch \(%rax\)
\s*[a-f0-9]+:	0f 0d 30             	prefetch \(%rax\)
\s*[a-f0-9]+:	0f 0d 38             	prefetch \(%rax\)

0+[0-9a-f]+ <intel_prefetch>:
\s*[a-f0-9]+:	0f 18 00             	prefetchnta \(%rax\)
\s*[a-f0-9]+:	0f 18 08             	prefetcht0 \(%rax\)
\s*[a-f0-9]+:	0f 18 10             	prefetcht1 \(%rax\)
\s*[a-f0-9]+:	0f 18 18             	prefetcht2 \(%rax\)
\s*[a-f0-9]+:	0f 18 20             	nopl   \(%rax\)
\s*[a-f0-9]+:	0f 18 28             	nopl   \(%rax\)
\s*[a-f0-9]+:	0f 18 30             	nopl   \(%rax\)
\s*[a-f0-9]+:	0f 18 38             	nopl   \(%rax\)
