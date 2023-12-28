#source: x86-64-arch-2.s
#as: -march=generic64+avx+vmx+smx+xsave+xsaveopt+aes+pclmul+fma+movbe+cx16+lahf_sahf+ept+clflush+syscall+rdtscp+3dnowa+sse4a+svme+lzcnt+padlock+bmi+tbm
#objdump: -dw
#name: x86-64 arch 2 (lzcnt)
#dump: x86-64-arch-2.d
