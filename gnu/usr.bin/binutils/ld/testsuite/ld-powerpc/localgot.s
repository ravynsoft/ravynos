 .text
 .global _start
_start:
x:
 ld 3,x@got(2) # isn't correct for 32-bit, but hey this is just a testcase
