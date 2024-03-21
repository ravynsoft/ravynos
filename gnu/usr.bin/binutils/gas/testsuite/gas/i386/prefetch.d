#as: --divide
#objdump: -dw
#name: i386 prefetch

.*: +file format .*

Disassembly of section .text:

0+ <amd_prefetch>:
\s*[a-f0-9]+:	0f 0d 00             	prefetch \(%eax\)
\s*[a-f0-9]+:	0f 0d 08             	prefetchw \(%eax\)
\s*[a-f0-9]+:	0f 0d 10             	prefetchwt1 \(%eax\)
\s*[a-f0-9]+:	0f 0d 18             	prefetch \(%eax\)
\s*[a-f0-9]+:	0f 0d 20             	prefetch \(%eax\)
\s*[a-f0-9]+:	0f 0d 28             	prefetch \(%eax\)
\s*[a-f0-9]+:	0f 0d 30             	prefetch \(%eax\)
\s*[a-f0-9]+:	0f 0d 38             	prefetch \(%eax\)

0+[0-9a-f]+ <intel_prefetch>:
\s*[a-f0-9]+:	0f 18 00             	prefetchnta \(%eax\)
\s*[a-f0-9]+:	0f 18 08             	prefetcht0 \(%eax\)
\s*[a-f0-9]+:	0f 18 10             	prefetcht1 \(%eax\)
\s*[a-f0-9]+:	0f 18 18             	prefetcht2 \(%eax\)
\s*[a-f0-9]+:	0f 18 20             	nopl   \(%eax\)
\s*[a-f0-9]+:	0f 18 28             	nopl   \(%eax\)
\s*[a-f0-9]+:	0f 18 30             	nopl   \(%eax\)
\s*[a-f0-9]+:	0f 18 38             	nopl   \(%eax\)
