#name: section size overflow
#source: over.s
#ld: -Ttext=0xfffffffc
#error: .* section .text VMA wraps around address space
