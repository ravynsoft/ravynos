#source: ../sse-check-none.s
#as: -msse-check=error -I${srcdir}/$subdir/..
#objdump: -dw
#name: x86-64 (ILP32) SSE check (.sse_check none)
#dump: ../sse-check.d
