# name: MVE vmaxnm, vmaxnma, vminnm and vminnma instructions, part 4
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> fe3f 0e81 	vmaxnma.f16	q0, q0
[^>]*> fe3f 1e81 	vminnma.f16	q0, q0
[^>]*> ff10 0f50 	vmaxnm.f16	q0, q0, q0
[^>]*> ff30 0f50 	vminnm.f16	q0, q0, q0
[^>]*> ff10 0f52 	vmaxnm.f16	q0, q0, q1
[^>]*> ff30 0f52 	vminnm.f16	q0, q0, q1
[^>]*> ff10 0f54 	vmaxnm.f16	q0, q0, q2
[^>]*> ff30 0f54 	vminnm.f16	q0, q0, q2
[^>]*> ff10 0f58 	vmaxnm.f16	q0, q0, q4
[^>]*> ff30 0f58 	vminnm.f16	q0, q0, q4
[^>]*> ff10 0f5e 	vmaxnm.f16	q0, q0, q7
[^>]*> ff30 0f5e 	vminnm.f16	q0, q0, q7
[^>]*> fe3f 0e83 	vmaxnma.f16	q0, q1
[^>]*> fe3f 1e83 	vminnma.f16	q0, q1
[^>]*> ff12 0f50 	vmaxnm.f16	q0, q1, q0
[^>]*> ff32 0f50 	vminnm.f16	q0, q1, q0
[^>]*> ff12 0f52 	vmaxnm.f16	q0, q1, q1
[^>]*> ff32 0f52 	vminnm.f16	q0, q1, q1
[^>]*> ff12 0f54 	vmaxnm.f16	q0, q1, q2
[^>]*> ff32 0f54 	vminnm.f16	q0, q1, q2
[^>]*> ff12 0f58 	vmaxnm.f16	q0, q1, q4
[^>]*> ff32 0f58 	vminnm.f16	q0, q1, q4
[^>]*> ff12 0f5e 	vmaxnm.f16	q0, q1, q7
[^>]*> ff32 0f5e 	vminnm.f16	q0, q1, q7
[^>]*> fe3f 0e85 	vmaxnma.f16	q0, q2
[^>]*> fe3f 1e85 	vminnma.f16	q0, q2
[^>]*> ff14 0f50 	vmaxnm.f16	q0, q2, q0
[^>]*> ff34 0f50 	vminnm.f16	q0, q2, q0
[^>]*> ff14 0f52 	vmaxnm.f16	q0, q2, q1
[^>]*> ff34 0f52 	vminnm.f16	q0, q2, q1
[^>]*> ff14 0f54 	vmaxnm.f16	q0, q2, q2
[^>]*> ff34 0f54 	vminnm.f16	q0, q2, q2
[^>]*> ff14 0f58 	vmaxnm.f16	q0, q2, q4
[^>]*> ff34 0f58 	vminnm.f16	q0, q2, q4
[^>]*> ff14 0f5e 	vmaxnm.f16	q0, q2, q7
[^>]*> ff34 0f5e 	vminnm.f16	q0, q2, q7
[^>]*> fe3f 0e89 	vmaxnma.f16	q0, q4
[^>]*> fe3f 1e89 	vminnma.f16	q0, q4
[^>]*> ff18 0f50 	vmaxnm.f16	q0, q4, q0
[^>]*> ff38 0f50 	vminnm.f16	q0, q4, q0
[^>]*> ff18 0f52 	vmaxnm.f16	q0, q4, q1
[^>]*> ff38 0f52 	vminnm.f16	q0, q4, q1
[^>]*> ff18 0f54 	vmaxnm.f16	q0, q4, q2
[^>]*> ff38 0f54 	vminnm.f16	q0, q4, q2
[^>]*> ff18 0f58 	vmaxnm.f16	q0, q4, q4
[^>]*> ff38 0f58 	vminnm.f16	q0, q4, q4
[^>]*> ff18 0f5e 	vmaxnm.f16	q0, q4, q7
[^>]*> ff38 0f5e 	vminnm.f16	q0, q4, q7
[^>]*> fe3f 0e8f 	vmaxnma.f16	q0, q7
[^>]*> fe3f 1e8f 	vminnma.f16	q0, q7
[^>]*> ff1e 0f50 	vmaxnm.f16	q0, q7, q0
[^>]*> ff3e 0f50 	vminnm.f16	q0, q7, q0
[^>]*> ff1e 0f52 	vmaxnm.f16	q0, q7, q1
[^>]*> ff3e 0f52 	vminnm.f16	q0, q7, q1
[^>]*> ff1e 0f54 	vmaxnm.f16	q0, q7, q2
[^>]*> ff3e 0f54 	vminnm.f16	q0, q7, q2
[^>]*> ff1e 0f58 	vmaxnm.f16	q0, q7, q4
[^>]*> ff3e 0f58 	vminnm.f16	q0, q7, q4
[^>]*> ff1e 0f5e 	vmaxnm.f16	q0, q7, q7
[^>]*> ff3e 0f5e 	vminnm.f16	q0, q7, q7
[^>]*> fe3f 2e81 	vmaxnma.f16	q1, q0
[^>]*> fe3f 3e81 	vminnma.f16	q1, q0
[^>]*> ff10 2f50 	vmaxnm.f16	q1, q0, q0
[^>]*> ff30 2f50 	vminnm.f16	q1, q0, q0
[^>]*> ff10 2f52 	vmaxnm.f16	q1, q0, q1
[^>]*> ff30 2f52 	vminnm.f16	q1, q0, q1
[^>]*> ff10 2f54 	vmaxnm.f16	q1, q0, q2
[^>]*> ff30 2f54 	vminnm.f16	q1, q0, q2
[^>]*> ff10 2f58 	vmaxnm.f16	q1, q0, q4
[^>]*> ff30 2f58 	vminnm.f16	q1, q0, q4
[^>]*> ff10 2f5e 	vmaxnm.f16	q1, q0, q7
[^>]*> ff30 2f5e 	vminnm.f16	q1, q0, q7
[^>]*> fe3f 2e83 	vmaxnma.f16	q1, q1
[^>]*> fe3f 3e83 	vminnma.f16	q1, q1
[^>]*> ff12 2f50 	vmaxnm.f16	q1, q1, q0
[^>]*> ff32 2f50 	vminnm.f16	q1, q1, q0
[^>]*> ff12 2f52 	vmaxnm.f16	q1, q1, q1
[^>]*> ff32 2f52 	vminnm.f16	q1, q1, q1
[^>]*> ff12 2f54 	vmaxnm.f16	q1, q1, q2
[^>]*> ff32 2f54 	vminnm.f16	q1, q1, q2
[^>]*> ff12 2f58 	vmaxnm.f16	q1, q1, q4
[^>]*> ff32 2f58 	vminnm.f16	q1, q1, q4
[^>]*> ff12 2f5e 	vmaxnm.f16	q1, q1, q7
[^>]*> ff32 2f5e 	vminnm.f16	q1, q1, q7
[^>]*> fe3f 2e85 	vmaxnma.f16	q1, q2
[^>]*> fe3f 3e85 	vminnma.f16	q1, q2
[^>]*> ff14 2f50 	vmaxnm.f16	q1, q2, q0
[^>]*> ff34 2f50 	vminnm.f16	q1, q2, q0
[^>]*> ff14 2f52 	vmaxnm.f16	q1, q2, q1
[^>]*> ff34 2f52 	vminnm.f16	q1, q2, q1
[^>]*> ff14 2f54 	vmaxnm.f16	q1, q2, q2
[^>]*> ff34 2f54 	vminnm.f16	q1, q2, q2
[^>]*> ff14 2f58 	vmaxnm.f16	q1, q2, q4
[^>]*> ff34 2f58 	vminnm.f16	q1, q2, q4
[^>]*> ff14 2f5e 	vmaxnm.f16	q1, q2, q7
[^>]*> ff34 2f5e 	vminnm.f16	q1, q2, q7
[^>]*> fe3f 2e89 	vmaxnma.f16	q1, q4
[^>]*> fe3f 3e89 	vminnma.f16	q1, q4
[^>]*> ff18 2f50 	vmaxnm.f16	q1, q4, q0
[^>]*> ff38 2f50 	vminnm.f16	q1, q4, q0
[^>]*> ff18 2f52 	vmaxnm.f16	q1, q4, q1
[^>]*> ff38 2f52 	vminnm.f16	q1, q4, q1
[^>]*> ff18 2f54 	vmaxnm.f16	q1, q4, q2
[^>]*> ff38 2f54 	vminnm.f16	q1, q4, q2
[^>]*> ff18 2f58 	vmaxnm.f16	q1, q4, q4
[^>]*> ff38 2f58 	vminnm.f16	q1, q4, q4
[^>]*> ff18 2f5e 	vmaxnm.f16	q1, q4, q7
[^>]*> ff38 2f5e 	vminnm.f16	q1, q4, q7
[^>]*> fe3f 2e8f 	vmaxnma.f16	q1, q7
[^>]*> fe3f 3e8f 	vminnma.f16	q1, q7
[^>]*> ff1e 2f50 	vmaxnm.f16	q1, q7, q0
[^>]*> ff3e 2f50 	vminnm.f16	q1, q7, q0
[^>]*> ff1e 2f52 	vmaxnm.f16	q1, q7, q1
[^>]*> ff3e 2f52 	vminnm.f16	q1, q7, q1
[^>]*> ff1e 2f54 	vmaxnm.f16	q1, q7, q2
[^>]*> ff3e 2f54 	vminnm.f16	q1, q7, q2
[^>]*> ff1e 2f58 	vmaxnm.f16	q1, q7, q4
[^>]*> ff3e 2f58 	vminnm.f16	q1, q7, q4
[^>]*> ff1e 2f5e 	vmaxnm.f16	q1, q7, q7
[^>]*> ff3e 2f5e 	vminnm.f16	q1, q7, q7
[^>]*> fe3f 4e81 	vmaxnma.f16	q2, q0
[^>]*> fe3f 5e81 	vminnma.f16	q2, q0
[^>]*> ff10 4f50 	vmaxnm.f16	q2, q0, q0
[^>]*> ff30 4f50 	vminnm.f16	q2, q0, q0
[^>]*> ff10 4f52 	vmaxnm.f16	q2, q0, q1
[^>]*> ff30 4f52 	vminnm.f16	q2, q0, q1
[^>]*> ff10 4f54 	vmaxnm.f16	q2, q0, q2
[^>]*> ff30 4f54 	vminnm.f16	q2, q0, q2
[^>]*> ff10 4f58 	vmaxnm.f16	q2, q0, q4
[^>]*> ff30 4f58 	vminnm.f16	q2, q0, q4
[^>]*> ff10 4f5e 	vmaxnm.f16	q2, q0, q7
[^>]*> ff30 4f5e 	vminnm.f16	q2, q0, q7
[^>]*> fe3f 4e83 	vmaxnma.f16	q2, q1
[^>]*> fe3f 5e83 	vminnma.f16	q2, q1
[^>]*> ff12 4f50 	vmaxnm.f16	q2, q1, q0
[^>]*> ff32 4f50 	vminnm.f16	q2, q1, q0
[^>]*> ff12 4f52 	vmaxnm.f16	q2, q1, q1
[^>]*> ff32 4f52 	vminnm.f16	q2, q1, q1
[^>]*> ff12 4f54 	vmaxnm.f16	q2, q1, q2
[^>]*> ff32 4f54 	vminnm.f16	q2, q1, q2
[^>]*> ff12 4f58 	vmaxnm.f16	q2, q1, q4
[^>]*> ff32 4f58 	vminnm.f16	q2, q1, q4
[^>]*> ff12 4f5e 	vmaxnm.f16	q2, q1, q7
[^>]*> ff32 4f5e 	vminnm.f16	q2, q1, q7
[^>]*> fe3f 4e85 	vmaxnma.f16	q2, q2
[^>]*> fe3f 5e85 	vminnma.f16	q2, q2
[^>]*> ff14 4f50 	vmaxnm.f16	q2, q2, q0
[^>]*> ff34 4f50 	vminnm.f16	q2, q2, q0
[^>]*> ff14 4f52 	vmaxnm.f16	q2, q2, q1
[^>]*> ff34 4f52 	vminnm.f16	q2, q2, q1
[^>]*> ff14 4f54 	vmaxnm.f16	q2, q2, q2
[^>]*> ff34 4f54 	vminnm.f16	q2, q2, q2
[^>]*> ff14 4f58 	vmaxnm.f16	q2, q2, q4
[^>]*> ff34 4f58 	vminnm.f16	q2, q2, q4
[^>]*> ff14 4f5e 	vmaxnm.f16	q2, q2, q7
[^>]*> ff34 4f5e 	vminnm.f16	q2, q2, q7
[^>]*> fe3f 4e89 	vmaxnma.f16	q2, q4
[^>]*> fe3f 5e89 	vminnma.f16	q2, q4
[^>]*> ff18 4f50 	vmaxnm.f16	q2, q4, q0
[^>]*> ff38 4f50 	vminnm.f16	q2, q4, q0
[^>]*> ff18 4f52 	vmaxnm.f16	q2, q4, q1
[^>]*> ff38 4f52 	vminnm.f16	q2, q4, q1
[^>]*> ff18 4f54 	vmaxnm.f16	q2, q4, q2
[^>]*> ff38 4f54 	vminnm.f16	q2, q4, q2
[^>]*> ff18 4f58 	vmaxnm.f16	q2, q4, q4
[^>]*> ff38 4f58 	vminnm.f16	q2, q4, q4
[^>]*> ff18 4f5e 	vmaxnm.f16	q2, q4, q7
[^>]*> ff38 4f5e 	vminnm.f16	q2, q4, q7
[^>]*> fe3f 4e8f 	vmaxnma.f16	q2, q7
[^>]*> fe3f 5e8f 	vminnma.f16	q2, q7
[^>]*> ff1e 4f50 	vmaxnm.f16	q2, q7, q0
[^>]*> ff3e 4f50 	vminnm.f16	q2, q7, q0
[^>]*> ff1e 4f52 	vmaxnm.f16	q2, q7, q1
[^>]*> ff3e 4f52 	vminnm.f16	q2, q7, q1
[^>]*> ff1e 4f54 	vmaxnm.f16	q2, q7, q2
[^>]*> ff3e 4f54 	vminnm.f16	q2, q7, q2
[^>]*> ff1e 4f58 	vmaxnm.f16	q2, q7, q4
[^>]*> ff3e 4f58 	vminnm.f16	q2, q7, q4
[^>]*> ff1e 4f5e 	vmaxnm.f16	q2, q7, q7
[^>]*> ff3e 4f5e 	vminnm.f16	q2, q7, q7
[^>]*> fe3f 8e81 	vmaxnma.f16	q4, q0
[^>]*> fe3f 9e81 	vminnma.f16	q4, q0
[^>]*> ff10 8f50 	vmaxnm.f16	q4, q0, q0
[^>]*> ff30 8f50 	vminnm.f16	q4, q0, q0
[^>]*> ff10 8f52 	vmaxnm.f16	q4, q0, q1
[^>]*> ff30 8f52 	vminnm.f16	q4, q0, q1
[^>]*> ff10 8f54 	vmaxnm.f16	q4, q0, q2
[^>]*> ff30 8f54 	vminnm.f16	q4, q0, q2
[^>]*> ff10 8f58 	vmaxnm.f16	q4, q0, q4
[^>]*> ff30 8f58 	vminnm.f16	q4, q0, q4
[^>]*> ff10 8f5e 	vmaxnm.f16	q4, q0, q7
[^>]*> ff30 8f5e 	vminnm.f16	q4, q0, q7
[^>]*> fe3f 8e83 	vmaxnma.f16	q4, q1
[^>]*> fe3f 9e83 	vminnma.f16	q4, q1
[^>]*> ff12 8f50 	vmaxnm.f16	q4, q1, q0
[^>]*> ff32 8f50 	vminnm.f16	q4, q1, q0
[^>]*> ff12 8f52 	vmaxnm.f16	q4, q1, q1
[^>]*> ff32 8f52 	vminnm.f16	q4, q1, q1
[^>]*> ff12 8f54 	vmaxnm.f16	q4, q1, q2
[^>]*> ff32 8f54 	vminnm.f16	q4, q1, q2
[^>]*> ff12 8f58 	vmaxnm.f16	q4, q1, q4
[^>]*> ff32 8f58 	vminnm.f16	q4, q1, q4
[^>]*> ff12 8f5e 	vmaxnm.f16	q4, q1, q7
[^>]*> ff32 8f5e 	vminnm.f16	q4, q1, q7
[^>]*> fe3f 8e85 	vmaxnma.f16	q4, q2
[^>]*> fe3f 9e85 	vminnma.f16	q4, q2
[^>]*> ff14 8f50 	vmaxnm.f16	q4, q2, q0
[^>]*> ff34 8f50 	vminnm.f16	q4, q2, q0
[^>]*> ff14 8f52 	vmaxnm.f16	q4, q2, q1
[^>]*> ff34 8f52 	vminnm.f16	q4, q2, q1
[^>]*> ff14 8f54 	vmaxnm.f16	q4, q2, q2
[^>]*> ff34 8f54 	vminnm.f16	q4, q2, q2
[^>]*> ff14 8f58 	vmaxnm.f16	q4, q2, q4
[^>]*> ff34 8f58 	vminnm.f16	q4, q2, q4
[^>]*> ff14 8f5e 	vmaxnm.f16	q4, q2, q7
[^>]*> ff34 8f5e 	vminnm.f16	q4, q2, q7
[^>]*> fe3f 8e89 	vmaxnma.f16	q4, q4
[^>]*> fe3f 9e89 	vminnma.f16	q4, q4
[^>]*> ff18 8f50 	vmaxnm.f16	q4, q4, q0
[^>]*> ff38 8f50 	vminnm.f16	q4, q4, q0
[^>]*> ff18 8f52 	vmaxnm.f16	q4, q4, q1
[^>]*> ff38 8f52 	vminnm.f16	q4, q4, q1
[^>]*> ff18 8f54 	vmaxnm.f16	q4, q4, q2
[^>]*> ff38 8f54 	vminnm.f16	q4, q4, q2
[^>]*> ff18 8f58 	vmaxnm.f16	q4, q4, q4
[^>]*> ff38 8f58 	vminnm.f16	q4, q4, q4
[^>]*> ff18 8f5e 	vmaxnm.f16	q4, q4, q7
[^>]*> ff38 8f5e 	vminnm.f16	q4, q4, q7
[^>]*> fe3f 8e8f 	vmaxnma.f16	q4, q7
[^>]*> fe3f 9e8f 	vminnma.f16	q4, q7
[^>]*> ff1e 8f50 	vmaxnm.f16	q4, q7, q0
[^>]*> ff3e 8f50 	vminnm.f16	q4, q7, q0
[^>]*> ff1e 8f52 	vmaxnm.f16	q4, q7, q1
[^>]*> ff3e 8f52 	vminnm.f16	q4, q7, q1
[^>]*> ff1e 8f54 	vmaxnm.f16	q4, q7, q2
[^>]*> ff3e 8f54 	vminnm.f16	q4, q7, q2
[^>]*> ff1e 8f58 	vmaxnm.f16	q4, q7, q4
[^>]*> ff3e 8f58 	vminnm.f16	q4, q7, q4
[^>]*> ff1e 8f5e 	vmaxnm.f16	q4, q7, q7
[^>]*> ff3e 8f5e 	vminnm.f16	q4, q7, q7
[^>]*> fe3f ee81 	vmaxnma.f16	q7, q0
[^>]*> fe3f fe81 	vminnma.f16	q7, q0
[^>]*> ff10 ef50 	vmaxnm.f16	q7, q0, q0
[^>]*> ff30 ef50 	vminnm.f16	q7, q0, q0
[^>]*> ff10 ef52 	vmaxnm.f16	q7, q0, q1
[^>]*> ff30 ef52 	vminnm.f16	q7, q0, q1
[^>]*> ff10 ef54 	vmaxnm.f16	q7, q0, q2
[^>]*> ff30 ef54 	vminnm.f16	q7, q0, q2
[^>]*> ff10 ef58 	vmaxnm.f16	q7, q0, q4
[^>]*> ff30 ef58 	vminnm.f16	q7, q0, q4
[^>]*> ff10 ef5e 	vmaxnm.f16	q7, q0, q7
[^>]*> ff30 ef5e 	vminnm.f16	q7, q0, q7
[^>]*> fe3f ee83 	vmaxnma.f16	q7, q1
[^>]*> fe3f fe83 	vminnma.f16	q7, q1
[^>]*> ff12 ef50 	vmaxnm.f16	q7, q1, q0
[^>]*> ff32 ef50 	vminnm.f16	q7, q1, q0
[^>]*> ff12 ef52 	vmaxnm.f16	q7, q1, q1
[^>]*> ff32 ef52 	vminnm.f16	q7, q1, q1
[^>]*> ff12 ef54 	vmaxnm.f16	q7, q1, q2
[^>]*> ff32 ef54 	vminnm.f16	q7, q1, q2
[^>]*> ff12 ef58 	vmaxnm.f16	q7, q1, q4
[^>]*> ff32 ef58 	vminnm.f16	q7, q1, q4
[^>]*> ff12 ef5e 	vmaxnm.f16	q7, q1, q7
[^>]*> ff32 ef5e 	vminnm.f16	q7, q1, q7
[^>]*> fe3f ee85 	vmaxnma.f16	q7, q2
[^>]*> fe3f fe85 	vminnma.f16	q7, q2
[^>]*> ff14 ef50 	vmaxnm.f16	q7, q2, q0
[^>]*> ff34 ef50 	vminnm.f16	q7, q2, q0
[^>]*> ff14 ef52 	vmaxnm.f16	q7, q2, q1
[^>]*> ff34 ef52 	vminnm.f16	q7, q2, q1
[^>]*> ff14 ef54 	vmaxnm.f16	q7, q2, q2
[^>]*> ff34 ef54 	vminnm.f16	q7, q2, q2
[^>]*> ff14 ef58 	vmaxnm.f16	q7, q2, q4
[^>]*> ff34 ef58 	vminnm.f16	q7, q2, q4
[^>]*> ff14 ef5e 	vmaxnm.f16	q7, q2, q7
[^>]*> ff34 ef5e 	vminnm.f16	q7, q2, q7
[^>]*> fe3f ee89 	vmaxnma.f16	q7, q4
[^>]*> fe3f fe89 	vminnma.f16	q7, q4
[^>]*> ff18 ef50 	vmaxnm.f16	q7, q4, q0
[^>]*> ff38 ef50 	vminnm.f16	q7, q4, q0
[^>]*> ff18 ef52 	vmaxnm.f16	q7, q4, q1
[^>]*> ff38 ef52 	vminnm.f16	q7, q4, q1
[^>]*> ff18 ef54 	vmaxnm.f16	q7, q4, q2
[^>]*> ff38 ef54 	vminnm.f16	q7, q4, q2
[^>]*> ff18 ef58 	vmaxnm.f16	q7, q4, q4
[^>]*> ff38 ef58 	vminnm.f16	q7, q4, q4
[^>]*> ff18 ef5e 	vmaxnm.f16	q7, q4, q7
[^>]*> ff38 ef5e 	vminnm.f16	q7, q4, q7
[^>]*> fe3f ee8f 	vmaxnma.f16	q7, q7
[^>]*> fe3f fe8f 	vminnma.f16	q7, q7
[^>]*> ff1e ef50 	vmaxnm.f16	q7, q7, q0
[^>]*> ff3e ef50 	vminnm.f16	q7, q7, q0
[^>]*> ff1e ef52 	vmaxnm.f16	q7, q7, q1
[^>]*> ff3e ef52 	vminnm.f16	q7, q7, q1
[^>]*> ff1e ef54 	vmaxnm.f16	q7, q7, q2
[^>]*> ff3e ef54 	vminnm.f16	q7, q7, q2
[^>]*> ff1e ef58 	vmaxnm.f16	q7, q7, q4
[^>]*> ff3e ef58 	vminnm.f16	q7, q7, q4
[^>]*> ff1e ef5e 	vmaxnm.f16	q7, q7, q7
[^>]*> ff3e ef5e 	vminnm.f16	q7, q7, q7
[^>]*> ee3f 0e81 	vmaxnma.f32	q0, q0
[^>]*> ee3f 1e81 	vminnma.f32	q0, q0
[^>]*> ff00 0f50 	vmaxnm.f32	q0, q0, q0
[^>]*> ff20 0f50 	vminnm.f32	q0, q0, q0
[^>]*> ff00 0f52 	vmaxnm.f32	q0, q0, q1
[^>]*> ff20 0f52 	vminnm.f32	q0, q0, q1
[^>]*> ff00 0f54 	vmaxnm.f32	q0, q0, q2
[^>]*> ff20 0f54 	vminnm.f32	q0, q0, q2
[^>]*> ff00 0f58 	vmaxnm.f32	q0, q0, q4
[^>]*> ff20 0f58 	vminnm.f32	q0, q0, q4
[^>]*> ff00 0f5e 	vmaxnm.f32	q0, q0, q7
[^>]*> ff20 0f5e 	vminnm.f32	q0, q0, q7
[^>]*> ee3f 0e83 	vmaxnma.f32	q0, q1
[^>]*> ee3f 1e83 	vminnma.f32	q0, q1
[^>]*> ff02 0f50 	vmaxnm.f32	q0, q1, q0
[^>]*> ff22 0f50 	vminnm.f32	q0, q1, q0
[^>]*> ff02 0f52 	vmaxnm.f32	q0, q1, q1
[^>]*> ff22 0f52 	vminnm.f32	q0, q1, q1
[^>]*> ff02 0f54 	vmaxnm.f32	q0, q1, q2
[^>]*> ff22 0f54 	vminnm.f32	q0, q1, q2
[^>]*> ff02 0f58 	vmaxnm.f32	q0, q1, q4
[^>]*> ff22 0f58 	vminnm.f32	q0, q1, q4
[^>]*> ff02 0f5e 	vmaxnm.f32	q0, q1, q7
[^>]*> ff22 0f5e 	vminnm.f32	q0, q1, q7
[^>]*> ee3f 0e85 	vmaxnma.f32	q0, q2
[^>]*> ee3f 1e85 	vminnma.f32	q0, q2
[^>]*> ff04 0f50 	vmaxnm.f32	q0, q2, q0
[^>]*> ff24 0f50 	vminnm.f32	q0, q2, q0
[^>]*> ff04 0f52 	vmaxnm.f32	q0, q2, q1
[^>]*> ff24 0f52 	vminnm.f32	q0, q2, q1
[^>]*> ff04 0f54 	vmaxnm.f32	q0, q2, q2
[^>]*> ff24 0f54 	vminnm.f32	q0, q2, q2
[^>]*> ff04 0f58 	vmaxnm.f32	q0, q2, q4
[^>]*> ff24 0f58 	vminnm.f32	q0, q2, q4
[^>]*> ff04 0f5e 	vmaxnm.f32	q0, q2, q7
[^>]*> ff24 0f5e 	vminnm.f32	q0, q2, q7
[^>]*> ee3f 0e89 	vmaxnma.f32	q0, q4
[^>]*> ee3f 1e89 	vminnma.f32	q0, q4
[^>]*> ff08 0f50 	vmaxnm.f32	q0, q4, q0
[^>]*> ff28 0f50 	vminnm.f32	q0, q4, q0
[^>]*> ff08 0f52 	vmaxnm.f32	q0, q4, q1
[^>]*> ff28 0f52 	vminnm.f32	q0, q4, q1
[^>]*> ff08 0f54 	vmaxnm.f32	q0, q4, q2
[^>]*> ff28 0f54 	vminnm.f32	q0, q4, q2
[^>]*> ff08 0f58 	vmaxnm.f32	q0, q4, q4
[^>]*> ff28 0f58 	vminnm.f32	q0, q4, q4
[^>]*> ff08 0f5e 	vmaxnm.f32	q0, q4, q7
[^>]*> ff28 0f5e 	vminnm.f32	q0, q4, q7
[^>]*> ee3f 0e8f 	vmaxnma.f32	q0, q7
[^>]*> ee3f 1e8f 	vminnma.f32	q0, q7
[^>]*> ff0e 0f50 	vmaxnm.f32	q0, q7, q0
[^>]*> ff2e 0f50 	vminnm.f32	q0, q7, q0
[^>]*> ff0e 0f52 	vmaxnm.f32	q0, q7, q1
[^>]*> ff2e 0f52 	vminnm.f32	q0, q7, q1
[^>]*> ff0e 0f54 	vmaxnm.f32	q0, q7, q2
[^>]*> ff2e 0f54 	vminnm.f32	q0, q7, q2
[^>]*> ff0e 0f58 	vmaxnm.f32	q0, q7, q4
[^>]*> ff2e 0f58 	vminnm.f32	q0, q7, q4
[^>]*> ff0e 0f5e 	vmaxnm.f32	q0, q7, q7
[^>]*> ff2e 0f5e 	vminnm.f32	q0, q7, q7
[^>]*> ee3f 2e81 	vmaxnma.f32	q1, q0
[^>]*> ee3f 3e81 	vminnma.f32	q1, q0
[^>]*> ff00 2f50 	vmaxnm.f32	q1, q0, q0
[^>]*> ff20 2f50 	vminnm.f32	q1, q0, q0
[^>]*> ff00 2f52 	vmaxnm.f32	q1, q0, q1
[^>]*> ff20 2f52 	vminnm.f32	q1, q0, q1
[^>]*> ff00 2f54 	vmaxnm.f32	q1, q0, q2
[^>]*> ff20 2f54 	vminnm.f32	q1, q0, q2
[^>]*> ff00 2f58 	vmaxnm.f32	q1, q0, q4
[^>]*> ff20 2f58 	vminnm.f32	q1, q0, q4
[^>]*> ff00 2f5e 	vmaxnm.f32	q1, q0, q7
[^>]*> ff20 2f5e 	vminnm.f32	q1, q0, q7
[^>]*> ee3f 2e83 	vmaxnma.f32	q1, q1
[^>]*> ee3f 3e83 	vminnma.f32	q1, q1
[^>]*> ff02 2f50 	vmaxnm.f32	q1, q1, q0
[^>]*> ff22 2f50 	vminnm.f32	q1, q1, q0
[^>]*> ff02 2f52 	vmaxnm.f32	q1, q1, q1
[^>]*> ff22 2f52 	vminnm.f32	q1, q1, q1
[^>]*> ff02 2f54 	vmaxnm.f32	q1, q1, q2
[^>]*> ff22 2f54 	vminnm.f32	q1, q1, q2
[^>]*> ff02 2f58 	vmaxnm.f32	q1, q1, q4
[^>]*> ff22 2f58 	vminnm.f32	q1, q1, q4
[^>]*> ff02 2f5e 	vmaxnm.f32	q1, q1, q7
[^>]*> ff22 2f5e 	vminnm.f32	q1, q1, q7
[^>]*> ee3f 2e85 	vmaxnma.f32	q1, q2
[^>]*> ee3f 3e85 	vminnma.f32	q1, q2
[^>]*> ff04 2f50 	vmaxnm.f32	q1, q2, q0
[^>]*> ff24 2f50 	vminnm.f32	q1, q2, q0
[^>]*> ff04 2f52 	vmaxnm.f32	q1, q2, q1
[^>]*> ff24 2f52 	vminnm.f32	q1, q2, q1
[^>]*> ff04 2f54 	vmaxnm.f32	q1, q2, q2
[^>]*> ff24 2f54 	vminnm.f32	q1, q2, q2
[^>]*> ff04 2f58 	vmaxnm.f32	q1, q2, q4
[^>]*> ff24 2f58 	vminnm.f32	q1, q2, q4
[^>]*> ff04 2f5e 	vmaxnm.f32	q1, q2, q7
[^>]*> ff24 2f5e 	vminnm.f32	q1, q2, q7
[^>]*> ee3f 2e89 	vmaxnma.f32	q1, q4
[^>]*> ee3f 3e89 	vminnma.f32	q1, q4
[^>]*> ff08 2f50 	vmaxnm.f32	q1, q4, q0
[^>]*> ff28 2f50 	vminnm.f32	q1, q4, q0
[^>]*> ff08 2f52 	vmaxnm.f32	q1, q4, q1
[^>]*> ff28 2f52 	vminnm.f32	q1, q4, q1
[^>]*> ff08 2f54 	vmaxnm.f32	q1, q4, q2
[^>]*> ff28 2f54 	vminnm.f32	q1, q4, q2
[^>]*> ff08 2f58 	vmaxnm.f32	q1, q4, q4
[^>]*> ff28 2f58 	vminnm.f32	q1, q4, q4
[^>]*> ff08 2f5e 	vmaxnm.f32	q1, q4, q7
[^>]*> ff28 2f5e 	vminnm.f32	q1, q4, q7
[^>]*> ee3f 2e8f 	vmaxnma.f32	q1, q7
[^>]*> ee3f 3e8f 	vminnma.f32	q1, q7
[^>]*> ff0e 2f50 	vmaxnm.f32	q1, q7, q0
[^>]*> ff2e 2f50 	vminnm.f32	q1, q7, q0
[^>]*> ff0e 2f52 	vmaxnm.f32	q1, q7, q1
[^>]*> ff2e 2f52 	vminnm.f32	q1, q7, q1
[^>]*> ff0e 2f54 	vmaxnm.f32	q1, q7, q2
[^>]*> ff2e 2f54 	vminnm.f32	q1, q7, q2
[^>]*> ff0e 2f58 	vmaxnm.f32	q1, q7, q4
[^>]*> ff2e 2f58 	vminnm.f32	q1, q7, q4
[^>]*> ff0e 2f5e 	vmaxnm.f32	q1, q7, q7
[^>]*> ff2e 2f5e 	vminnm.f32	q1, q7, q7
[^>]*> ee3f 4e81 	vmaxnma.f32	q2, q0
[^>]*> ee3f 5e81 	vminnma.f32	q2, q0
[^>]*> ff00 4f50 	vmaxnm.f32	q2, q0, q0
[^>]*> ff20 4f50 	vminnm.f32	q2, q0, q0
[^>]*> ff00 4f52 	vmaxnm.f32	q2, q0, q1
[^>]*> ff20 4f52 	vminnm.f32	q2, q0, q1
[^>]*> ff00 4f54 	vmaxnm.f32	q2, q0, q2
[^>]*> ff20 4f54 	vminnm.f32	q2, q0, q2
[^>]*> ff00 4f58 	vmaxnm.f32	q2, q0, q4
[^>]*> ff20 4f58 	vminnm.f32	q2, q0, q4
[^>]*> ff00 4f5e 	vmaxnm.f32	q2, q0, q7
[^>]*> ff20 4f5e 	vminnm.f32	q2, q0, q7
[^>]*> ee3f 4e83 	vmaxnma.f32	q2, q1
[^>]*> ee3f 5e83 	vminnma.f32	q2, q1
[^>]*> ff02 4f50 	vmaxnm.f32	q2, q1, q0
[^>]*> ff22 4f50 	vminnm.f32	q2, q1, q0
[^>]*> ff02 4f52 	vmaxnm.f32	q2, q1, q1
[^>]*> ff22 4f52 	vminnm.f32	q2, q1, q1
[^>]*> ff02 4f54 	vmaxnm.f32	q2, q1, q2
[^>]*> ff22 4f54 	vminnm.f32	q2, q1, q2
[^>]*> ff02 4f58 	vmaxnm.f32	q2, q1, q4
[^>]*> ff22 4f58 	vminnm.f32	q2, q1, q4
[^>]*> ff02 4f5e 	vmaxnm.f32	q2, q1, q7
[^>]*> ff22 4f5e 	vminnm.f32	q2, q1, q7
[^>]*> ee3f 4e85 	vmaxnma.f32	q2, q2
[^>]*> ee3f 5e85 	vminnma.f32	q2, q2
[^>]*> ff04 4f50 	vmaxnm.f32	q2, q2, q0
[^>]*> ff24 4f50 	vminnm.f32	q2, q2, q0
[^>]*> ff04 4f52 	vmaxnm.f32	q2, q2, q1
[^>]*> ff24 4f52 	vminnm.f32	q2, q2, q1
[^>]*> ff04 4f54 	vmaxnm.f32	q2, q2, q2
[^>]*> ff24 4f54 	vminnm.f32	q2, q2, q2
[^>]*> ff04 4f58 	vmaxnm.f32	q2, q2, q4
[^>]*> ff24 4f58 	vminnm.f32	q2, q2, q4
[^>]*> ff04 4f5e 	vmaxnm.f32	q2, q2, q7
[^>]*> ff24 4f5e 	vminnm.f32	q2, q2, q7
[^>]*> ee3f 4e89 	vmaxnma.f32	q2, q4
[^>]*> ee3f 5e89 	vminnma.f32	q2, q4
[^>]*> ff08 4f50 	vmaxnm.f32	q2, q4, q0
[^>]*> ff28 4f50 	vminnm.f32	q2, q4, q0
[^>]*> ff08 4f52 	vmaxnm.f32	q2, q4, q1
[^>]*> ff28 4f52 	vminnm.f32	q2, q4, q1
[^>]*> ff08 4f54 	vmaxnm.f32	q2, q4, q2
[^>]*> ff28 4f54 	vminnm.f32	q2, q4, q2
[^>]*> ff08 4f58 	vmaxnm.f32	q2, q4, q4
[^>]*> ff28 4f58 	vminnm.f32	q2, q4, q4
[^>]*> ff08 4f5e 	vmaxnm.f32	q2, q4, q7
[^>]*> ff28 4f5e 	vminnm.f32	q2, q4, q7
[^>]*> ee3f 4e8f 	vmaxnma.f32	q2, q7
[^>]*> ee3f 5e8f 	vminnma.f32	q2, q7
[^>]*> ff0e 4f50 	vmaxnm.f32	q2, q7, q0
[^>]*> ff2e 4f50 	vminnm.f32	q2, q7, q0
[^>]*> ff0e 4f52 	vmaxnm.f32	q2, q7, q1
[^>]*> ff2e 4f52 	vminnm.f32	q2, q7, q1
[^>]*> ff0e 4f54 	vmaxnm.f32	q2, q7, q2
[^>]*> ff2e 4f54 	vminnm.f32	q2, q7, q2
[^>]*> ff0e 4f58 	vmaxnm.f32	q2, q7, q4
[^>]*> ff2e 4f58 	vminnm.f32	q2, q7, q4
[^>]*> ff0e 4f5e 	vmaxnm.f32	q2, q7, q7
[^>]*> ff2e 4f5e 	vminnm.f32	q2, q7, q7
[^>]*> ee3f 8e81 	vmaxnma.f32	q4, q0
[^>]*> ee3f 9e81 	vminnma.f32	q4, q0
[^>]*> ff00 8f50 	vmaxnm.f32	q4, q0, q0
[^>]*> ff20 8f50 	vminnm.f32	q4, q0, q0
[^>]*> ff00 8f52 	vmaxnm.f32	q4, q0, q1
[^>]*> ff20 8f52 	vminnm.f32	q4, q0, q1
[^>]*> ff00 8f54 	vmaxnm.f32	q4, q0, q2
[^>]*> ff20 8f54 	vminnm.f32	q4, q0, q2
[^>]*> ff00 8f58 	vmaxnm.f32	q4, q0, q4
[^>]*> ff20 8f58 	vminnm.f32	q4, q0, q4
[^>]*> ff00 8f5e 	vmaxnm.f32	q4, q0, q7
[^>]*> ff20 8f5e 	vminnm.f32	q4, q0, q7
[^>]*> ee3f 8e83 	vmaxnma.f32	q4, q1
[^>]*> ee3f 9e83 	vminnma.f32	q4, q1
[^>]*> ff02 8f50 	vmaxnm.f32	q4, q1, q0
[^>]*> ff22 8f50 	vminnm.f32	q4, q1, q0
[^>]*> ff02 8f52 	vmaxnm.f32	q4, q1, q1
[^>]*> ff22 8f52 	vminnm.f32	q4, q1, q1
[^>]*> ff02 8f54 	vmaxnm.f32	q4, q1, q2
[^>]*> ff22 8f54 	vminnm.f32	q4, q1, q2
[^>]*> ff02 8f58 	vmaxnm.f32	q4, q1, q4
[^>]*> ff22 8f58 	vminnm.f32	q4, q1, q4
[^>]*> ff02 8f5e 	vmaxnm.f32	q4, q1, q7
[^>]*> ff22 8f5e 	vminnm.f32	q4, q1, q7
[^>]*> ee3f 8e85 	vmaxnma.f32	q4, q2
[^>]*> ee3f 9e85 	vminnma.f32	q4, q2
[^>]*> ff04 8f50 	vmaxnm.f32	q4, q2, q0
[^>]*> ff24 8f50 	vminnm.f32	q4, q2, q0
[^>]*> ff04 8f52 	vmaxnm.f32	q4, q2, q1
[^>]*> ff24 8f52 	vminnm.f32	q4, q2, q1
[^>]*> ff04 8f54 	vmaxnm.f32	q4, q2, q2
[^>]*> ff24 8f54 	vminnm.f32	q4, q2, q2
[^>]*> ff04 8f58 	vmaxnm.f32	q4, q2, q4
[^>]*> ff24 8f58 	vminnm.f32	q4, q2, q4
[^>]*> ff04 8f5e 	vmaxnm.f32	q4, q2, q7
[^>]*> ff24 8f5e 	vminnm.f32	q4, q2, q7
[^>]*> ee3f 8e89 	vmaxnma.f32	q4, q4
[^>]*> ee3f 9e89 	vminnma.f32	q4, q4
[^>]*> ff08 8f50 	vmaxnm.f32	q4, q4, q0
[^>]*> ff28 8f50 	vminnm.f32	q4, q4, q0
[^>]*> ff08 8f52 	vmaxnm.f32	q4, q4, q1
[^>]*> ff28 8f52 	vminnm.f32	q4, q4, q1
[^>]*> ff08 8f54 	vmaxnm.f32	q4, q4, q2
[^>]*> ff28 8f54 	vminnm.f32	q4, q4, q2
[^>]*> ff08 8f58 	vmaxnm.f32	q4, q4, q4
[^>]*> ff28 8f58 	vminnm.f32	q4, q4, q4
[^>]*> ff08 8f5e 	vmaxnm.f32	q4, q4, q7
[^>]*> ff28 8f5e 	vminnm.f32	q4, q4, q7
[^>]*> ee3f 8e8f 	vmaxnma.f32	q4, q7
[^>]*> ee3f 9e8f 	vminnma.f32	q4, q7
[^>]*> ff0e 8f50 	vmaxnm.f32	q4, q7, q0
[^>]*> ff2e 8f50 	vminnm.f32	q4, q7, q0
[^>]*> ff0e 8f52 	vmaxnm.f32	q4, q7, q1
[^>]*> ff2e 8f52 	vminnm.f32	q4, q7, q1
[^>]*> ff0e 8f54 	vmaxnm.f32	q4, q7, q2
[^>]*> ff2e 8f54 	vminnm.f32	q4, q7, q2
[^>]*> ff0e 8f58 	vmaxnm.f32	q4, q7, q4
[^>]*> ff2e 8f58 	vminnm.f32	q4, q7, q4
[^>]*> ff0e 8f5e 	vmaxnm.f32	q4, q7, q7
[^>]*> ff2e 8f5e 	vminnm.f32	q4, q7, q7
[^>]*> ee3f ee81 	vmaxnma.f32	q7, q0
[^>]*> ee3f fe81 	vminnma.f32	q7, q0
[^>]*> ff00 ef50 	vmaxnm.f32	q7, q0, q0
[^>]*> ff20 ef50 	vminnm.f32	q7, q0, q0
[^>]*> ff00 ef52 	vmaxnm.f32	q7, q0, q1
[^>]*> ff20 ef52 	vminnm.f32	q7, q0, q1
[^>]*> ff00 ef54 	vmaxnm.f32	q7, q0, q2
[^>]*> ff20 ef54 	vminnm.f32	q7, q0, q2
[^>]*> ff00 ef58 	vmaxnm.f32	q7, q0, q4
[^>]*> ff20 ef58 	vminnm.f32	q7, q0, q4
[^>]*> ff00 ef5e 	vmaxnm.f32	q7, q0, q7
[^>]*> ff20 ef5e 	vminnm.f32	q7, q0, q7
[^>]*> ee3f ee83 	vmaxnma.f32	q7, q1
[^>]*> ee3f fe83 	vminnma.f32	q7, q1
[^>]*> ff02 ef50 	vmaxnm.f32	q7, q1, q0
[^>]*> ff22 ef50 	vminnm.f32	q7, q1, q0
[^>]*> ff02 ef52 	vmaxnm.f32	q7, q1, q1
[^>]*> ff22 ef52 	vminnm.f32	q7, q1, q1
[^>]*> ff02 ef54 	vmaxnm.f32	q7, q1, q2
[^>]*> ff22 ef54 	vminnm.f32	q7, q1, q2
[^>]*> ff02 ef58 	vmaxnm.f32	q7, q1, q4
[^>]*> ff22 ef58 	vminnm.f32	q7, q1, q4
[^>]*> ff02 ef5e 	vmaxnm.f32	q7, q1, q7
[^>]*> ff22 ef5e 	vminnm.f32	q7, q1, q7
[^>]*> ee3f ee85 	vmaxnma.f32	q7, q2
[^>]*> ee3f fe85 	vminnma.f32	q7, q2
[^>]*> ff04 ef50 	vmaxnm.f32	q7, q2, q0
[^>]*> ff24 ef50 	vminnm.f32	q7, q2, q0
[^>]*> ff04 ef52 	vmaxnm.f32	q7, q2, q1
[^>]*> ff24 ef52 	vminnm.f32	q7, q2, q1
[^>]*> ff04 ef54 	vmaxnm.f32	q7, q2, q2
[^>]*> ff24 ef54 	vminnm.f32	q7, q2, q2
[^>]*> ff04 ef58 	vmaxnm.f32	q7, q2, q4
[^>]*> ff24 ef58 	vminnm.f32	q7, q2, q4
[^>]*> ff04 ef5e 	vmaxnm.f32	q7, q2, q7
[^>]*> ff24 ef5e 	vminnm.f32	q7, q2, q7
[^>]*> ee3f ee89 	vmaxnma.f32	q7, q4
[^>]*> ee3f fe89 	vminnma.f32	q7, q4
[^>]*> ff08 ef50 	vmaxnm.f32	q7, q4, q0
[^>]*> ff28 ef50 	vminnm.f32	q7, q4, q0
[^>]*> ff08 ef52 	vmaxnm.f32	q7, q4, q1
[^>]*> ff28 ef52 	vminnm.f32	q7, q4, q1
[^>]*> ff08 ef54 	vmaxnm.f32	q7, q4, q2
[^>]*> ff28 ef54 	vminnm.f32	q7, q4, q2
[^>]*> ff08 ef58 	vmaxnm.f32	q7, q4, q4
[^>]*> ff28 ef58 	vminnm.f32	q7, q4, q4
[^>]*> ff08 ef5e 	vmaxnm.f32	q7, q4, q7
[^>]*> ff28 ef5e 	vminnm.f32	q7, q4, q7
[^>]*> ee3f ee8f 	vmaxnma.f32	q7, q7
[^>]*> ee3f fe8f 	vminnma.f32	q7, q7
[^>]*> ff0e ef50 	vmaxnm.f32	q7, q7, q0
[^>]*> ff2e ef50 	vminnm.f32	q7, q7, q0
[^>]*> ff0e ef52 	vmaxnm.f32	q7, q7, q1
[^>]*> ff2e ef52 	vminnm.f32	q7, q7, q1
[^>]*> ff0e ef54 	vmaxnm.f32	q7, q7, q2
[^>]*> ff2e ef54 	vminnm.f32	q7, q7, q2
[^>]*> ff0e ef58 	vmaxnm.f32	q7, q7, q4
[^>]*> ff2e ef58 	vminnm.f32	q7, q7, q4
[^>]*> ff0e ef5e 	vmaxnm.f32	q7, q7, q7
[^>]*> ff2e ef5e 	vminnm.f32	q7, q7, q7
[^>]*> fe71 ef4d 	vpstete
[^>]*> ff12 0f54 	vmaxnmt.f16	q0, q1, q2
[^>]*> ff0e ef5e 	vmaxnme.f32	q7, q7, q7
[^>]*> ff22 0f54 	vminnmt.f32	q0, q1, q2
[^>]*> ff3e ef5e 	vminnme.f16	q7, q7, q7
[^>]*> fe71 ef4d 	vpstete
[^>]*> fe3f 0e83 	vmaxnmat.f16	q0, q1
[^>]*> ee3f ee8f 	vmaxnmae.f32	q7, q7
[^>]*> ee3f 1e83 	vminnmat.f32	q0, q1
[^>]*> fe3f fe8f 	vminnmae.f16	q7, q7
