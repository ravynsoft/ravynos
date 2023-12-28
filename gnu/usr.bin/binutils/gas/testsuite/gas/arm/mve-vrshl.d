# name: MVE vrmlaldavh(a)(x), vrmlalvh(a), vrmlsldavh(a)(x) instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ff00 0540 	vrshl.u8	q0, q0, q0
[^>]*> ff02 0540 	vrshl.u8	q0, q0, q1
[^>]*> ff04 0540 	vrshl.u8	q0, q0, q2
[^>]*> ff08 0540 	vrshl.u8	q0, q0, q4
[^>]*> ff0e 0540 	vrshl.u8	q0, q0, q7
[^>]*> ff00 0542 	vrshl.u8	q0, q1, q0
[^>]*> ff02 0542 	vrshl.u8	q0, q1, q1
[^>]*> ff04 0542 	vrshl.u8	q0, q1, q2
[^>]*> ff08 0542 	vrshl.u8	q0, q1, q4
[^>]*> ff0e 0542 	vrshl.u8	q0, q1, q7
[^>]*> ff00 0544 	vrshl.u8	q0, q2, q0
[^>]*> ff02 0544 	vrshl.u8	q0, q2, q1
[^>]*> ff04 0544 	vrshl.u8	q0, q2, q2
[^>]*> ff08 0544 	vrshl.u8	q0, q2, q4
[^>]*> ff0e 0544 	vrshl.u8	q0, q2, q7
[^>]*> ff00 0548 	vrshl.u8	q0, q4, q0
[^>]*> ff02 0548 	vrshl.u8	q0, q4, q1
[^>]*> ff04 0548 	vrshl.u8	q0, q4, q2
[^>]*> ff08 0548 	vrshl.u8	q0, q4, q4
[^>]*> ff0e 0548 	vrshl.u8	q0, q4, q7
[^>]*> ff00 054e 	vrshl.u8	q0, q7, q0
[^>]*> ff02 054e 	vrshl.u8	q0, q7, q1
[^>]*> ff04 054e 	vrshl.u8	q0, q7, q2
[^>]*> ff08 054e 	vrshl.u8	q0, q7, q4
[^>]*> ff0e 054e 	vrshl.u8	q0, q7, q7
[^>]*> fe33 1e60 	vrshl.u8	q0, r0
[^>]*> fe33 1e61 	vrshl.u8	q0, r1
[^>]*> fe33 1e62 	vrshl.u8	q0, r2
[^>]*> fe33 1e64 	vrshl.u8	q0, r4
[^>]*> fe33 1e67 	vrshl.u8	q0, r7
[^>]*> fe33 1e68 	vrshl.u8	q0, r8
[^>]*> fe33 1e6a 	vrshl.u8	q0, sl
[^>]*> fe33 1e6c 	vrshl.u8	q0, ip
[^>]*> fe33 1e6e 	vrshl.u8	q0, lr
[^>]*> ff00 2540 	vrshl.u8	q1, q0, q0
[^>]*> ff02 2540 	vrshl.u8	q1, q0, q1
[^>]*> ff04 2540 	vrshl.u8	q1, q0, q2
[^>]*> ff08 2540 	vrshl.u8	q1, q0, q4
[^>]*> ff0e 2540 	vrshl.u8	q1, q0, q7
[^>]*> ff00 2542 	vrshl.u8	q1, q1, q0
[^>]*> ff02 2542 	vrshl.u8	q1, q1, q1
[^>]*> ff04 2542 	vrshl.u8	q1, q1, q2
[^>]*> ff08 2542 	vrshl.u8	q1, q1, q4
[^>]*> ff0e 2542 	vrshl.u8	q1, q1, q7
[^>]*> ff00 2544 	vrshl.u8	q1, q2, q0
[^>]*> ff02 2544 	vrshl.u8	q1, q2, q1
[^>]*> ff04 2544 	vrshl.u8	q1, q2, q2
[^>]*> ff08 2544 	vrshl.u8	q1, q2, q4
[^>]*> ff0e 2544 	vrshl.u8	q1, q2, q7
[^>]*> ff00 2548 	vrshl.u8	q1, q4, q0
[^>]*> ff02 2548 	vrshl.u8	q1, q4, q1
[^>]*> ff04 2548 	vrshl.u8	q1, q4, q2
[^>]*> ff08 2548 	vrshl.u8	q1, q4, q4
[^>]*> ff0e 2548 	vrshl.u8	q1, q4, q7
[^>]*> ff00 254e 	vrshl.u8	q1, q7, q0
[^>]*> ff02 254e 	vrshl.u8	q1, q7, q1
[^>]*> ff04 254e 	vrshl.u8	q1, q7, q2
[^>]*> ff08 254e 	vrshl.u8	q1, q7, q4
[^>]*> ff0e 254e 	vrshl.u8	q1, q7, q7
[^>]*> fe33 3e60 	vrshl.u8	q1, r0
[^>]*> fe33 3e61 	vrshl.u8	q1, r1
[^>]*> fe33 3e62 	vrshl.u8	q1, r2
[^>]*> fe33 3e64 	vrshl.u8	q1, r4
[^>]*> fe33 3e67 	vrshl.u8	q1, r7
[^>]*> fe33 3e68 	vrshl.u8	q1, r8
[^>]*> fe33 3e6a 	vrshl.u8	q1, sl
[^>]*> fe33 3e6c 	vrshl.u8	q1, ip
[^>]*> fe33 3e6e 	vrshl.u8	q1, lr
[^>]*> ff00 4540 	vrshl.u8	q2, q0, q0
[^>]*> ff02 4540 	vrshl.u8	q2, q0, q1
[^>]*> ff04 4540 	vrshl.u8	q2, q0, q2
[^>]*> ff08 4540 	vrshl.u8	q2, q0, q4
[^>]*> ff0e 4540 	vrshl.u8	q2, q0, q7
[^>]*> ff00 4542 	vrshl.u8	q2, q1, q0
[^>]*> ff02 4542 	vrshl.u8	q2, q1, q1
[^>]*> ff04 4542 	vrshl.u8	q2, q1, q2
[^>]*> ff08 4542 	vrshl.u8	q2, q1, q4
[^>]*> ff0e 4542 	vrshl.u8	q2, q1, q7
[^>]*> ff00 4544 	vrshl.u8	q2, q2, q0
[^>]*> ff02 4544 	vrshl.u8	q2, q2, q1
[^>]*> ff04 4544 	vrshl.u8	q2, q2, q2
[^>]*> ff08 4544 	vrshl.u8	q2, q2, q4
[^>]*> ff0e 4544 	vrshl.u8	q2, q2, q7
[^>]*> ff00 4548 	vrshl.u8	q2, q4, q0
[^>]*> ff02 4548 	vrshl.u8	q2, q4, q1
[^>]*> ff04 4548 	vrshl.u8	q2, q4, q2
[^>]*> ff08 4548 	vrshl.u8	q2, q4, q4
[^>]*> ff0e 4548 	vrshl.u8	q2, q4, q7
[^>]*> ff00 454e 	vrshl.u8	q2, q7, q0
[^>]*> ff02 454e 	vrshl.u8	q2, q7, q1
[^>]*> ff04 454e 	vrshl.u8	q2, q7, q2
[^>]*> ff08 454e 	vrshl.u8	q2, q7, q4
[^>]*> ff0e 454e 	vrshl.u8	q2, q7, q7
[^>]*> fe33 5e60 	vrshl.u8	q2, r0
[^>]*> fe33 5e61 	vrshl.u8	q2, r1
[^>]*> fe33 5e62 	vrshl.u8	q2, r2
[^>]*> fe33 5e64 	vrshl.u8	q2, r4
[^>]*> fe33 5e67 	vrshl.u8	q2, r7
[^>]*> fe33 5e68 	vrshl.u8	q2, r8
[^>]*> fe33 5e6a 	vrshl.u8	q2, sl
[^>]*> fe33 5e6c 	vrshl.u8	q2, ip
[^>]*> fe33 5e6e 	vrshl.u8	q2, lr
[^>]*> ff00 8540 	vrshl.u8	q4, q0, q0
[^>]*> ff02 8540 	vrshl.u8	q4, q0, q1
[^>]*> ff04 8540 	vrshl.u8	q4, q0, q2
[^>]*> ff08 8540 	vrshl.u8	q4, q0, q4
[^>]*> ff0e 8540 	vrshl.u8	q4, q0, q7
[^>]*> ff00 8542 	vrshl.u8	q4, q1, q0
[^>]*> ff02 8542 	vrshl.u8	q4, q1, q1
[^>]*> ff04 8542 	vrshl.u8	q4, q1, q2
[^>]*> ff08 8542 	vrshl.u8	q4, q1, q4
[^>]*> ff0e 8542 	vrshl.u8	q4, q1, q7
[^>]*> ff00 8544 	vrshl.u8	q4, q2, q0
[^>]*> ff02 8544 	vrshl.u8	q4, q2, q1
[^>]*> ff04 8544 	vrshl.u8	q4, q2, q2
[^>]*> ff08 8544 	vrshl.u8	q4, q2, q4
[^>]*> ff0e 8544 	vrshl.u8	q4, q2, q7
[^>]*> ff00 8548 	vrshl.u8	q4, q4, q0
[^>]*> ff02 8548 	vrshl.u8	q4, q4, q1
[^>]*> ff04 8548 	vrshl.u8	q4, q4, q2
[^>]*> ff08 8548 	vrshl.u8	q4, q4, q4
[^>]*> ff0e 8548 	vrshl.u8	q4, q4, q7
[^>]*> ff00 854e 	vrshl.u8	q4, q7, q0
[^>]*> ff02 854e 	vrshl.u8	q4, q7, q1
[^>]*> ff04 854e 	vrshl.u8	q4, q7, q2
[^>]*> ff08 854e 	vrshl.u8	q4, q7, q4
[^>]*> ff0e 854e 	vrshl.u8	q4, q7, q7
[^>]*> fe33 9e60 	vrshl.u8	q4, r0
[^>]*> fe33 9e61 	vrshl.u8	q4, r1
[^>]*> fe33 9e62 	vrshl.u8	q4, r2
[^>]*> fe33 9e64 	vrshl.u8	q4, r4
[^>]*> fe33 9e67 	vrshl.u8	q4, r7
[^>]*> fe33 9e68 	vrshl.u8	q4, r8
[^>]*> fe33 9e6a 	vrshl.u8	q4, sl
[^>]*> fe33 9e6c 	vrshl.u8	q4, ip
[^>]*> fe33 9e6e 	vrshl.u8	q4, lr
[^>]*> ff00 e540 	vrshl.u8	q7, q0, q0
[^>]*> ff02 e540 	vrshl.u8	q7, q0, q1
[^>]*> ff04 e540 	vrshl.u8	q7, q0, q2
[^>]*> ff08 e540 	vrshl.u8	q7, q0, q4
[^>]*> ff0e e540 	vrshl.u8	q7, q0, q7
[^>]*> ff00 e542 	vrshl.u8	q7, q1, q0
[^>]*> ff02 e542 	vrshl.u8	q7, q1, q1
[^>]*> ff04 e542 	vrshl.u8	q7, q1, q2
[^>]*> ff08 e542 	vrshl.u8	q7, q1, q4
[^>]*> ff0e e542 	vrshl.u8	q7, q1, q7
[^>]*> ff00 e544 	vrshl.u8	q7, q2, q0
[^>]*> ff02 e544 	vrshl.u8	q7, q2, q1
[^>]*> ff04 e544 	vrshl.u8	q7, q2, q2
[^>]*> ff08 e544 	vrshl.u8	q7, q2, q4
[^>]*> ff0e e544 	vrshl.u8	q7, q2, q7
[^>]*> ff00 e548 	vrshl.u8	q7, q4, q0
[^>]*> ff02 e548 	vrshl.u8	q7, q4, q1
[^>]*> ff04 e548 	vrshl.u8	q7, q4, q2
[^>]*> ff08 e548 	vrshl.u8	q7, q4, q4
[^>]*> ff0e e548 	vrshl.u8	q7, q4, q7
[^>]*> ff00 e54e 	vrshl.u8	q7, q7, q0
[^>]*> ff02 e54e 	vrshl.u8	q7, q7, q1
[^>]*> ff04 e54e 	vrshl.u8	q7, q7, q2
[^>]*> ff08 e54e 	vrshl.u8	q7, q7, q4
[^>]*> ff0e e54e 	vrshl.u8	q7, q7, q7
[^>]*> fe33 fe60 	vrshl.u8	q7, r0
[^>]*> fe33 fe61 	vrshl.u8	q7, r1
[^>]*> fe33 fe62 	vrshl.u8	q7, r2
[^>]*> fe33 fe64 	vrshl.u8	q7, r4
[^>]*> fe33 fe67 	vrshl.u8	q7, r7
[^>]*> fe33 fe68 	vrshl.u8	q7, r8
[^>]*> fe33 fe6a 	vrshl.u8	q7, sl
[^>]*> fe33 fe6c 	vrshl.u8	q7, ip
[^>]*> fe33 fe6e 	vrshl.u8	q7, lr
[^>]*> ef00 0540 	vrshl.s8	q0, q0, q0
[^>]*> ef02 0540 	vrshl.s8	q0, q0, q1
[^>]*> ef04 0540 	vrshl.s8	q0, q0, q2
[^>]*> ef08 0540 	vrshl.s8	q0, q0, q4
[^>]*> ef0e 0540 	vrshl.s8	q0, q0, q7
[^>]*> ef00 0542 	vrshl.s8	q0, q1, q0
[^>]*> ef02 0542 	vrshl.s8	q0, q1, q1
[^>]*> ef04 0542 	vrshl.s8	q0, q1, q2
[^>]*> ef08 0542 	vrshl.s8	q0, q1, q4
[^>]*> ef0e 0542 	vrshl.s8	q0, q1, q7
[^>]*> ef00 0544 	vrshl.s8	q0, q2, q0
[^>]*> ef02 0544 	vrshl.s8	q0, q2, q1
[^>]*> ef04 0544 	vrshl.s8	q0, q2, q2
[^>]*> ef08 0544 	vrshl.s8	q0, q2, q4
[^>]*> ef0e 0544 	vrshl.s8	q0, q2, q7
[^>]*> ef00 0548 	vrshl.s8	q0, q4, q0
[^>]*> ef02 0548 	vrshl.s8	q0, q4, q1
[^>]*> ef04 0548 	vrshl.s8	q0, q4, q2
[^>]*> ef08 0548 	vrshl.s8	q0, q4, q4
[^>]*> ef0e 0548 	vrshl.s8	q0, q4, q7
[^>]*> ef00 054e 	vrshl.s8	q0, q7, q0
[^>]*> ef02 054e 	vrshl.s8	q0, q7, q1
[^>]*> ef04 054e 	vrshl.s8	q0, q7, q2
[^>]*> ef08 054e 	vrshl.s8	q0, q7, q4
[^>]*> ef0e 054e 	vrshl.s8	q0, q7, q7
[^>]*> ee33 1e60 	vrshl.s8	q0, r0
[^>]*> ee33 1e61 	vrshl.s8	q0, r1
[^>]*> ee33 1e62 	vrshl.s8	q0, r2
[^>]*> ee33 1e64 	vrshl.s8	q0, r4
[^>]*> ee33 1e67 	vrshl.s8	q0, r7
[^>]*> ee33 1e68 	vrshl.s8	q0, r8
[^>]*> ee33 1e6a 	vrshl.s8	q0, sl
[^>]*> ee33 1e6c 	vrshl.s8	q0, ip
[^>]*> ee33 1e6e 	vrshl.s8	q0, lr
[^>]*> ef00 2540 	vrshl.s8	q1, q0, q0
[^>]*> ef02 2540 	vrshl.s8	q1, q0, q1
[^>]*> ef04 2540 	vrshl.s8	q1, q0, q2
[^>]*> ef08 2540 	vrshl.s8	q1, q0, q4
[^>]*> ef0e 2540 	vrshl.s8	q1, q0, q7
[^>]*> ef00 2542 	vrshl.s8	q1, q1, q0
[^>]*> ef02 2542 	vrshl.s8	q1, q1, q1
[^>]*> ef04 2542 	vrshl.s8	q1, q1, q2
[^>]*> ef08 2542 	vrshl.s8	q1, q1, q4
[^>]*> ef0e 2542 	vrshl.s8	q1, q1, q7
[^>]*> ef00 2544 	vrshl.s8	q1, q2, q0
[^>]*> ef02 2544 	vrshl.s8	q1, q2, q1
[^>]*> ef04 2544 	vrshl.s8	q1, q2, q2
[^>]*> ef08 2544 	vrshl.s8	q1, q2, q4
[^>]*> ef0e 2544 	vrshl.s8	q1, q2, q7
[^>]*> ef00 2548 	vrshl.s8	q1, q4, q0
[^>]*> ef02 2548 	vrshl.s8	q1, q4, q1
[^>]*> ef04 2548 	vrshl.s8	q1, q4, q2
[^>]*> ef08 2548 	vrshl.s8	q1, q4, q4
[^>]*> ef0e 2548 	vrshl.s8	q1, q4, q7
[^>]*> ef00 254e 	vrshl.s8	q1, q7, q0
[^>]*> ef02 254e 	vrshl.s8	q1, q7, q1
[^>]*> ef04 254e 	vrshl.s8	q1, q7, q2
[^>]*> ef08 254e 	vrshl.s8	q1, q7, q4
[^>]*> ef0e 254e 	vrshl.s8	q1, q7, q7
[^>]*> ee33 3e60 	vrshl.s8	q1, r0
[^>]*> ee33 3e61 	vrshl.s8	q1, r1
[^>]*> ee33 3e62 	vrshl.s8	q1, r2
[^>]*> ee33 3e64 	vrshl.s8	q1, r4
[^>]*> ee33 3e67 	vrshl.s8	q1, r7
[^>]*> ee33 3e68 	vrshl.s8	q1, r8
[^>]*> ee33 3e6a 	vrshl.s8	q1, sl
[^>]*> ee33 3e6c 	vrshl.s8	q1, ip
[^>]*> ee33 3e6e 	vrshl.s8	q1, lr
[^>]*> ef00 4540 	vrshl.s8	q2, q0, q0
[^>]*> ef02 4540 	vrshl.s8	q2, q0, q1
[^>]*> ef04 4540 	vrshl.s8	q2, q0, q2
[^>]*> ef08 4540 	vrshl.s8	q2, q0, q4
[^>]*> ef0e 4540 	vrshl.s8	q2, q0, q7
[^>]*> ef00 4542 	vrshl.s8	q2, q1, q0
[^>]*> ef02 4542 	vrshl.s8	q2, q1, q1
[^>]*> ef04 4542 	vrshl.s8	q2, q1, q2
[^>]*> ef08 4542 	vrshl.s8	q2, q1, q4
[^>]*> ef0e 4542 	vrshl.s8	q2, q1, q7
[^>]*> ef00 4544 	vrshl.s8	q2, q2, q0
[^>]*> ef02 4544 	vrshl.s8	q2, q2, q1
[^>]*> ef04 4544 	vrshl.s8	q2, q2, q2
[^>]*> ef08 4544 	vrshl.s8	q2, q2, q4
[^>]*> ef0e 4544 	vrshl.s8	q2, q2, q7
[^>]*> ef00 4548 	vrshl.s8	q2, q4, q0
[^>]*> ef02 4548 	vrshl.s8	q2, q4, q1
[^>]*> ef04 4548 	vrshl.s8	q2, q4, q2
[^>]*> ef08 4548 	vrshl.s8	q2, q4, q4
[^>]*> ef0e 4548 	vrshl.s8	q2, q4, q7
[^>]*> ef00 454e 	vrshl.s8	q2, q7, q0
[^>]*> ef02 454e 	vrshl.s8	q2, q7, q1
[^>]*> ef04 454e 	vrshl.s8	q2, q7, q2
[^>]*> ef08 454e 	vrshl.s8	q2, q7, q4
[^>]*> ef0e 454e 	vrshl.s8	q2, q7, q7
[^>]*> ee33 5e60 	vrshl.s8	q2, r0
[^>]*> ee33 5e61 	vrshl.s8	q2, r1
[^>]*> ee33 5e62 	vrshl.s8	q2, r2
[^>]*> ee33 5e64 	vrshl.s8	q2, r4
[^>]*> ee33 5e67 	vrshl.s8	q2, r7
[^>]*> ee33 5e68 	vrshl.s8	q2, r8
[^>]*> ee33 5e6a 	vrshl.s8	q2, sl
[^>]*> ee33 5e6c 	vrshl.s8	q2, ip
[^>]*> ee33 5e6e 	vrshl.s8	q2, lr
[^>]*> ef00 8540 	vrshl.s8	q4, q0, q0
[^>]*> ef02 8540 	vrshl.s8	q4, q0, q1
[^>]*> ef04 8540 	vrshl.s8	q4, q0, q2
[^>]*> ef08 8540 	vrshl.s8	q4, q0, q4
[^>]*> ef0e 8540 	vrshl.s8	q4, q0, q7
[^>]*> ef00 8542 	vrshl.s8	q4, q1, q0
[^>]*> ef02 8542 	vrshl.s8	q4, q1, q1
[^>]*> ef04 8542 	vrshl.s8	q4, q1, q2
[^>]*> ef08 8542 	vrshl.s8	q4, q1, q4
[^>]*> ef0e 8542 	vrshl.s8	q4, q1, q7
[^>]*> ef00 8544 	vrshl.s8	q4, q2, q0
[^>]*> ef02 8544 	vrshl.s8	q4, q2, q1
[^>]*> ef04 8544 	vrshl.s8	q4, q2, q2
[^>]*> ef08 8544 	vrshl.s8	q4, q2, q4
[^>]*> ef0e 8544 	vrshl.s8	q4, q2, q7
[^>]*> ef00 8548 	vrshl.s8	q4, q4, q0
[^>]*> ef02 8548 	vrshl.s8	q4, q4, q1
[^>]*> ef04 8548 	vrshl.s8	q4, q4, q2
[^>]*> ef08 8548 	vrshl.s8	q4, q4, q4
[^>]*> ef0e 8548 	vrshl.s8	q4, q4, q7
[^>]*> ef00 854e 	vrshl.s8	q4, q7, q0
[^>]*> ef02 854e 	vrshl.s8	q4, q7, q1
[^>]*> ef04 854e 	vrshl.s8	q4, q7, q2
[^>]*> ef08 854e 	vrshl.s8	q4, q7, q4
[^>]*> ef0e 854e 	vrshl.s8	q4, q7, q7
[^>]*> ee33 9e60 	vrshl.s8	q4, r0
[^>]*> ee33 9e61 	vrshl.s8	q4, r1
[^>]*> ee33 9e62 	vrshl.s8	q4, r2
[^>]*> ee33 9e64 	vrshl.s8	q4, r4
[^>]*> ee33 9e67 	vrshl.s8	q4, r7
[^>]*> ee33 9e68 	vrshl.s8	q4, r8
[^>]*> ee33 9e6a 	vrshl.s8	q4, sl
[^>]*> ee33 9e6c 	vrshl.s8	q4, ip
[^>]*> ee33 9e6e 	vrshl.s8	q4, lr
[^>]*> ef00 e540 	vrshl.s8	q7, q0, q0
[^>]*> ef02 e540 	vrshl.s8	q7, q0, q1
[^>]*> ef04 e540 	vrshl.s8	q7, q0, q2
[^>]*> ef08 e540 	vrshl.s8	q7, q0, q4
[^>]*> ef0e e540 	vrshl.s8	q7, q0, q7
[^>]*> ef00 e542 	vrshl.s8	q7, q1, q0
[^>]*> ef02 e542 	vrshl.s8	q7, q1, q1
[^>]*> ef04 e542 	vrshl.s8	q7, q1, q2
[^>]*> ef08 e542 	vrshl.s8	q7, q1, q4
[^>]*> ef0e e542 	vrshl.s8	q7, q1, q7
[^>]*> ef00 e544 	vrshl.s8	q7, q2, q0
[^>]*> ef02 e544 	vrshl.s8	q7, q2, q1
[^>]*> ef04 e544 	vrshl.s8	q7, q2, q2
[^>]*> ef08 e544 	vrshl.s8	q7, q2, q4
[^>]*> ef0e e544 	vrshl.s8	q7, q2, q7
[^>]*> ef00 e548 	vrshl.s8	q7, q4, q0
[^>]*> ef02 e548 	vrshl.s8	q7, q4, q1
[^>]*> ef04 e548 	vrshl.s8	q7, q4, q2
[^>]*> ef08 e548 	vrshl.s8	q7, q4, q4
[^>]*> ef0e e548 	vrshl.s8	q7, q4, q7
[^>]*> ef00 e54e 	vrshl.s8	q7, q7, q0
[^>]*> ef02 e54e 	vrshl.s8	q7, q7, q1
[^>]*> ef04 e54e 	vrshl.s8	q7, q7, q2
[^>]*> ef08 e54e 	vrshl.s8	q7, q7, q4
[^>]*> ef0e e54e 	vrshl.s8	q7, q7, q7
[^>]*> ee33 fe60 	vrshl.s8	q7, r0
[^>]*> ee33 fe61 	vrshl.s8	q7, r1
[^>]*> ee33 fe62 	vrshl.s8	q7, r2
[^>]*> ee33 fe64 	vrshl.s8	q7, r4
[^>]*> ee33 fe67 	vrshl.s8	q7, r7
[^>]*> ee33 fe68 	vrshl.s8	q7, r8
[^>]*> ee33 fe6a 	vrshl.s8	q7, sl
[^>]*> ee33 fe6c 	vrshl.s8	q7, ip
[^>]*> ee33 fe6e 	vrshl.s8	q7, lr
[^>]*> ff10 0540 	vrshl.u16	q0, q0, q0
[^>]*> ff12 0540 	vrshl.u16	q0, q0, q1
[^>]*> ff14 0540 	vrshl.u16	q0, q0, q2
[^>]*> ff18 0540 	vrshl.u16	q0, q0, q4
[^>]*> ff1e 0540 	vrshl.u16	q0, q0, q7
[^>]*> ff10 0542 	vrshl.u16	q0, q1, q0
[^>]*> ff12 0542 	vrshl.u16	q0, q1, q1
[^>]*> ff14 0542 	vrshl.u16	q0, q1, q2
[^>]*> ff18 0542 	vrshl.u16	q0, q1, q4
[^>]*> ff1e 0542 	vrshl.u16	q0, q1, q7
[^>]*> ff10 0544 	vrshl.u16	q0, q2, q0
[^>]*> ff12 0544 	vrshl.u16	q0, q2, q1
[^>]*> ff14 0544 	vrshl.u16	q0, q2, q2
[^>]*> ff18 0544 	vrshl.u16	q0, q2, q4
[^>]*> ff1e 0544 	vrshl.u16	q0, q2, q7
[^>]*> ff10 0548 	vrshl.u16	q0, q4, q0
[^>]*> ff12 0548 	vrshl.u16	q0, q4, q1
[^>]*> ff14 0548 	vrshl.u16	q0, q4, q2
[^>]*> ff18 0548 	vrshl.u16	q0, q4, q4
[^>]*> ff1e 0548 	vrshl.u16	q0, q4, q7
[^>]*> ff10 054e 	vrshl.u16	q0, q7, q0
[^>]*> ff12 054e 	vrshl.u16	q0, q7, q1
[^>]*> ff14 054e 	vrshl.u16	q0, q7, q2
[^>]*> ff18 054e 	vrshl.u16	q0, q7, q4
[^>]*> ff1e 054e 	vrshl.u16	q0, q7, q7
[^>]*> fe37 1e60 	vrshl.u16	q0, r0
[^>]*> fe37 1e61 	vrshl.u16	q0, r1
[^>]*> fe37 1e62 	vrshl.u16	q0, r2
[^>]*> fe37 1e64 	vrshl.u16	q0, r4
[^>]*> fe37 1e67 	vrshl.u16	q0, r7
[^>]*> fe37 1e68 	vrshl.u16	q0, r8
[^>]*> fe37 1e6a 	vrshl.u16	q0, sl
[^>]*> fe37 1e6c 	vrshl.u16	q0, ip
[^>]*> fe37 1e6e 	vrshl.u16	q0, lr
[^>]*> ff10 2540 	vrshl.u16	q1, q0, q0
[^>]*> ff12 2540 	vrshl.u16	q1, q0, q1
[^>]*> ff14 2540 	vrshl.u16	q1, q0, q2
[^>]*> ff18 2540 	vrshl.u16	q1, q0, q4
[^>]*> ff1e 2540 	vrshl.u16	q1, q0, q7
[^>]*> ff10 2542 	vrshl.u16	q1, q1, q0
[^>]*> ff12 2542 	vrshl.u16	q1, q1, q1
[^>]*> ff14 2542 	vrshl.u16	q1, q1, q2
[^>]*> ff18 2542 	vrshl.u16	q1, q1, q4
[^>]*> ff1e 2542 	vrshl.u16	q1, q1, q7
[^>]*> ff10 2544 	vrshl.u16	q1, q2, q0
[^>]*> ff12 2544 	vrshl.u16	q1, q2, q1
[^>]*> ff14 2544 	vrshl.u16	q1, q2, q2
[^>]*> ff18 2544 	vrshl.u16	q1, q2, q4
[^>]*> ff1e 2544 	vrshl.u16	q1, q2, q7
[^>]*> ff10 2548 	vrshl.u16	q1, q4, q0
[^>]*> ff12 2548 	vrshl.u16	q1, q4, q1
[^>]*> ff14 2548 	vrshl.u16	q1, q4, q2
[^>]*> ff18 2548 	vrshl.u16	q1, q4, q4
[^>]*> ff1e 2548 	vrshl.u16	q1, q4, q7
[^>]*> ff10 254e 	vrshl.u16	q1, q7, q0
[^>]*> ff12 254e 	vrshl.u16	q1, q7, q1
[^>]*> ff14 254e 	vrshl.u16	q1, q7, q2
[^>]*> ff18 254e 	vrshl.u16	q1, q7, q4
[^>]*> ff1e 254e 	vrshl.u16	q1, q7, q7
[^>]*> fe37 3e60 	vrshl.u16	q1, r0
[^>]*> fe37 3e61 	vrshl.u16	q1, r1
[^>]*> fe37 3e62 	vrshl.u16	q1, r2
[^>]*> fe37 3e64 	vrshl.u16	q1, r4
[^>]*> fe37 3e67 	vrshl.u16	q1, r7
[^>]*> fe37 3e68 	vrshl.u16	q1, r8
[^>]*> fe37 3e6a 	vrshl.u16	q1, sl
[^>]*> fe37 3e6c 	vrshl.u16	q1, ip
[^>]*> fe37 3e6e 	vrshl.u16	q1, lr
[^>]*> ff10 4540 	vrshl.u16	q2, q0, q0
[^>]*> ff12 4540 	vrshl.u16	q2, q0, q1
[^>]*> ff14 4540 	vrshl.u16	q2, q0, q2
[^>]*> ff18 4540 	vrshl.u16	q2, q0, q4
[^>]*> ff1e 4540 	vrshl.u16	q2, q0, q7
[^>]*> ff10 4542 	vrshl.u16	q2, q1, q0
[^>]*> ff12 4542 	vrshl.u16	q2, q1, q1
[^>]*> ff14 4542 	vrshl.u16	q2, q1, q2
[^>]*> ff18 4542 	vrshl.u16	q2, q1, q4
[^>]*> ff1e 4542 	vrshl.u16	q2, q1, q7
[^>]*> ff10 4544 	vrshl.u16	q2, q2, q0
[^>]*> ff12 4544 	vrshl.u16	q2, q2, q1
[^>]*> ff14 4544 	vrshl.u16	q2, q2, q2
[^>]*> ff18 4544 	vrshl.u16	q2, q2, q4
[^>]*> ff1e 4544 	vrshl.u16	q2, q2, q7
[^>]*> ff10 4548 	vrshl.u16	q2, q4, q0
[^>]*> ff12 4548 	vrshl.u16	q2, q4, q1
[^>]*> ff14 4548 	vrshl.u16	q2, q4, q2
[^>]*> ff18 4548 	vrshl.u16	q2, q4, q4
[^>]*> ff1e 4548 	vrshl.u16	q2, q4, q7
[^>]*> ff10 454e 	vrshl.u16	q2, q7, q0
[^>]*> ff12 454e 	vrshl.u16	q2, q7, q1
[^>]*> ff14 454e 	vrshl.u16	q2, q7, q2
[^>]*> ff18 454e 	vrshl.u16	q2, q7, q4
[^>]*> ff1e 454e 	vrshl.u16	q2, q7, q7
[^>]*> fe37 5e60 	vrshl.u16	q2, r0
[^>]*> fe37 5e61 	vrshl.u16	q2, r1
[^>]*> fe37 5e62 	vrshl.u16	q2, r2
[^>]*> fe37 5e64 	vrshl.u16	q2, r4
[^>]*> fe37 5e67 	vrshl.u16	q2, r7
[^>]*> fe37 5e68 	vrshl.u16	q2, r8
[^>]*> fe37 5e6a 	vrshl.u16	q2, sl
[^>]*> fe37 5e6c 	vrshl.u16	q2, ip
[^>]*> fe37 5e6e 	vrshl.u16	q2, lr
[^>]*> ff10 8540 	vrshl.u16	q4, q0, q0
[^>]*> ff12 8540 	vrshl.u16	q4, q0, q1
[^>]*> ff14 8540 	vrshl.u16	q4, q0, q2
[^>]*> ff18 8540 	vrshl.u16	q4, q0, q4
[^>]*> ff1e 8540 	vrshl.u16	q4, q0, q7
[^>]*> ff10 8542 	vrshl.u16	q4, q1, q0
[^>]*> ff12 8542 	vrshl.u16	q4, q1, q1
[^>]*> ff14 8542 	vrshl.u16	q4, q1, q2
[^>]*> ff18 8542 	vrshl.u16	q4, q1, q4
[^>]*> ff1e 8542 	vrshl.u16	q4, q1, q7
[^>]*> ff10 8544 	vrshl.u16	q4, q2, q0
[^>]*> ff12 8544 	vrshl.u16	q4, q2, q1
[^>]*> ff14 8544 	vrshl.u16	q4, q2, q2
[^>]*> ff18 8544 	vrshl.u16	q4, q2, q4
[^>]*> ff1e 8544 	vrshl.u16	q4, q2, q7
[^>]*> ff10 8548 	vrshl.u16	q4, q4, q0
[^>]*> ff12 8548 	vrshl.u16	q4, q4, q1
[^>]*> ff14 8548 	vrshl.u16	q4, q4, q2
[^>]*> ff18 8548 	vrshl.u16	q4, q4, q4
[^>]*> ff1e 8548 	vrshl.u16	q4, q4, q7
[^>]*> ff10 854e 	vrshl.u16	q4, q7, q0
[^>]*> ff12 854e 	vrshl.u16	q4, q7, q1
[^>]*> ff14 854e 	vrshl.u16	q4, q7, q2
[^>]*> ff18 854e 	vrshl.u16	q4, q7, q4
[^>]*> ff1e 854e 	vrshl.u16	q4, q7, q7
[^>]*> fe37 9e60 	vrshl.u16	q4, r0
[^>]*> fe37 9e61 	vrshl.u16	q4, r1
[^>]*> fe37 9e62 	vrshl.u16	q4, r2
[^>]*> fe37 9e64 	vrshl.u16	q4, r4
[^>]*> fe37 9e67 	vrshl.u16	q4, r7
[^>]*> fe37 9e68 	vrshl.u16	q4, r8
[^>]*> fe37 9e6a 	vrshl.u16	q4, sl
[^>]*> fe37 9e6c 	vrshl.u16	q4, ip
[^>]*> fe37 9e6e 	vrshl.u16	q4, lr
[^>]*> ff10 e540 	vrshl.u16	q7, q0, q0
[^>]*> ff12 e540 	vrshl.u16	q7, q0, q1
[^>]*> ff14 e540 	vrshl.u16	q7, q0, q2
[^>]*> ff18 e540 	vrshl.u16	q7, q0, q4
[^>]*> ff1e e540 	vrshl.u16	q7, q0, q7
[^>]*> ff10 e542 	vrshl.u16	q7, q1, q0
[^>]*> ff12 e542 	vrshl.u16	q7, q1, q1
[^>]*> ff14 e542 	vrshl.u16	q7, q1, q2
[^>]*> ff18 e542 	vrshl.u16	q7, q1, q4
[^>]*> ff1e e542 	vrshl.u16	q7, q1, q7
[^>]*> ff10 e544 	vrshl.u16	q7, q2, q0
[^>]*> ff12 e544 	vrshl.u16	q7, q2, q1
[^>]*> ff14 e544 	vrshl.u16	q7, q2, q2
[^>]*> ff18 e544 	vrshl.u16	q7, q2, q4
[^>]*> ff1e e544 	vrshl.u16	q7, q2, q7
[^>]*> ff10 e548 	vrshl.u16	q7, q4, q0
[^>]*> ff12 e548 	vrshl.u16	q7, q4, q1
[^>]*> ff14 e548 	vrshl.u16	q7, q4, q2
[^>]*> ff18 e548 	vrshl.u16	q7, q4, q4
[^>]*> ff1e e548 	vrshl.u16	q7, q4, q7
[^>]*> ff10 e54e 	vrshl.u16	q7, q7, q0
[^>]*> ff12 e54e 	vrshl.u16	q7, q7, q1
[^>]*> ff14 e54e 	vrshl.u16	q7, q7, q2
[^>]*> ff18 e54e 	vrshl.u16	q7, q7, q4
[^>]*> ff1e e54e 	vrshl.u16	q7, q7, q7
[^>]*> fe37 fe60 	vrshl.u16	q7, r0
[^>]*> fe37 fe61 	vrshl.u16	q7, r1
[^>]*> fe37 fe62 	vrshl.u16	q7, r2
[^>]*> fe37 fe64 	vrshl.u16	q7, r4
[^>]*> fe37 fe67 	vrshl.u16	q7, r7
[^>]*> fe37 fe68 	vrshl.u16	q7, r8
[^>]*> fe37 fe6a 	vrshl.u16	q7, sl
[^>]*> fe37 fe6c 	vrshl.u16	q7, ip
[^>]*> fe37 fe6e 	vrshl.u16	q7, lr
[^>]*> ef10 0540 	vrshl.s16	q0, q0, q0
[^>]*> ef12 0540 	vrshl.s16	q0, q0, q1
[^>]*> ef14 0540 	vrshl.s16	q0, q0, q2
[^>]*> ef18 0540 	vrshl.s16	q0, q0, q4
[^>]*> ef1e 0540 	vrshl.s16	q0, q0, q7
[^>]*> ef10 0542 	vrshl.s16	q0, q1, q0
[^>]*> ef12 0542 	vrshl.s16	q0, q1, q1
[^>]*> ef14 0542 	vrshl.s16	q0, q1, q2
[^>]*> ef18 0542 	vrshl.s16	q0, q1, q4
[^>]*> ef1e 0542 	vrshl.s16	q0, q1, q7
[^>]*> ef10 0544 	vrshl.s16	q0, q2, q0
[^>]*> ef12 0544 	vrshl.s16	q0, q2, q1
[^>]*> ef14 0544 	vrshl.s16	q0, q2, q2
[^>]*> ef18 0544 	vrshl.s16	q0, q2, q4
[^>]*> ef1e 0544 	vrshl.s16	q0, q2, q7
[^>]*> ef10 0548 	vrshl.s16	q0, q4, q0
[^>]*> ef12 0548 	vrshl.s16	q0, q4, q1
[^>]*> ef14 0548 	vrshl.s16	q0, q4, q2
[^>]*> ef18 0548 	vrshl.s16	q0, q4, q4
[^>]*> ef1e 0548 	vrshl.s16	q0, q4, q7
[^>]*> ef10 054e 	vrshl.s16	q0, q7, q0
[^>]*> ef12 054e 	vrshl.s16	q0, q7, q1
[^>]*> ef14 054e 	vrshl.s16	q0, q7, q2
[^>]*> ef18 054e 	vrshl.s16	q0, q7, q4
[^>]*> ef1e 054e 	vrshl.s16	q0, q7, q7
[^>]*> ee37 1e60 	vrshl.s16	q0, r0
[^>]*> ee37 1e61 	vrshl.s16	q0, r1
[^>]*> ee37 1e62 	vrshl.s16	q0, r2
[^>]*> ee37 1e64 	vrshl.s16	q0, r4
[^>]*> ee37 1e67 	vrshl.s16	q0, r7
[^>]*> ee37 1e68 	vrshl.s16	q0, r8
[^>]*> ee37 1e6a 	vrshl.s16	q0, sl
[^>]*> ee37 1e6c 	vrshl.s16	q0, ip
[^>]*> ee37 1e6e 	vrshl.s16	q0, lr
[^>]*> ef10 2540 	vrshl.s16	q1, q0, q0
[^>]*> ef12 2540 	vrshl.s16	q1, q0, q1
[^>]*> ef14 2540 	vrshl.s16	q1, q0, q2
[^>]*> ef18 2540 	vrshl.s16	q1, q0, q4
[^>]*> ef1e 2540 	vrshl.s16	q1, q0, q7
[^>]*> ef10 2542 	vrshl.s16	q1, q1, q0
[^>]*> ef12 2542 	vrshl.s16	q1, q1, q1
[^>]*> ef14 2542 	vrshl.s16	q1, q1, q2
[^>]*> ef18 2542 	vrshl.s16	q1, q1, q4
[^>]*> ef1e 2542 	vrshl.s16	q1, q1, q7
[^>]*> ef10 2544 	vrshl.s16	q1, q2, q0
[^>]*> ef12 2544 	vrshl.s16	q1, q2, q1
[^>]*> ef14 2544 	vrshl.s16	q1, q2, q2
[^>]*> ef18 2544 	vrshl.s16	q1, q2, q4
[^>]*> ef1e 2544 	vrshl.s16	q1, q2, q7
[^>]*> ef10 2548 	vrshl.s16	q1, q4, q0
[^>]*> ef12 2548 	vrshl.s16	q1, q4, q1
[^>]*> ef14 2548 	vrshl.s16	q1, q4, q2
[^>]*> ef18 2548 	vrshl.s16	q1, q4, q4
[^>]*> ef1e 2548 	vrshl.s16	q1, q4, q7
[^>]*> ef10 254e 	vrshl.s16	q1, q7, q0
[^>]*> ef12 254e 	vrshl.s16	q1, q7, q1
[^>]*> ef14 254e 	vrshl.s16	q1, q7, q2
[^>]*> ef18 254e 	vrshl.s16	q1, q7, q4
[^>]*> ef1e 254e 	vrshl.s16	q1, q7, q7
[^>]*> ee37 3e60 	vrshl.s16	q1, r0
[^>]*> ee37 3e61 	vrshl.s16	q1, r1
[^>]*> ee37 3e62 	vrshl.s16	q1, r2
[^>]*> ee37 3e64 	vrshl.s16	q1, r4
[^>]*> ee37 3e67 	vrshl.s16	q1, r7
[^>]*> ee37 3e68 	vrshl.s16	q1, r8
[^>]*> ee37 3e6a 	vrshl.s16	q1, sl
[^>]*> ee37 3e6c 	vrshl.s16	q1, ip
[^>]*> ee37 3e6e 	vrshl.s16	q1, lr
[^>]*> ef10 4540 	vrshl.s16	q2, q0, q0
[^>]*> ef12 4540 	vrshl.s16	q2, q0, q1
[^>]*> ef14 4540 	vrshl.s16	q2, q0, q2
[^>]*> ef18 4540 	vrshl.s16	q2, q0, q4
[^>]*> ef1e 4540 	vrshl.s16	q2, q0, q7
[^>]*> ef10 4542 	vrshl.s16	q2, q1, q0
[^>]*> ef12 4542 	vrshl.s16	q2, q1, q1
[^>]*> ef14 4542 	vrshl.s16	q2, q1, q2
[^>]*> ef18 4542 	vrshl.s16	q2, q1, q4
[^>]*> ef1e 4542 	vrshl.s16	q2, q1, q7
[^>]*> ef10 4544 	vrshl.s16	q2, q2, q0
[^>]*> ef12 4544 	vrshl.s16	q2, q2, q1
[^>]*> ef14 4544 	vrshl.s16	q2, q2, q2
[^>]*> ef18 4544 	vrshl.s16	q2, q2, q4
[^>]*> ef1e 4544 	vrshl.s16	q2, q2, q7
[^>]*> ef10 4548 	vrshl.s16	q2, q4, q0
[^>]*> ef12 4548 	vrshl.s16	q2, q4, q1
[^>]*> ef14 4548 	vrshl.s16	q2, q4, q2
[^>]*> ef18 4548 	vrshl.s16	q2, q4, q4
[^>]*> ef1e 4548 	vrshl.s16	q2, q4, q7
[^>]*> ef10 454e 	vrshl.s16	q2, q7, q0
[^>]*> ef12 454e 	vrshl.s16	q2, q7, q1
[^>]*> ef14 454e 	vrshl.s16	q2, q7, q2
[^>]*> ef18 454e 	vrshl.s16	q2, q7, q4
[^>]*> ef1e 454e 	vrshl.s16	q2, q7, q7
[^>]*> ee37 5e60 	vrshl.s16	q2, r0
[^>]*> ee37 5e61 	vrshl.s16	q2, r1
[^>]*> ee37 5e62 	vrshl.s16	q2, r2
[^>]*> ee37 5e64 	vrshl.s16	q2, r4
[^>]*> ee37 5e67 	vrshl.s16	q2, r7
[^>]*> ee37 5e68 	vrshl.s16	q2, r8
[^>]*> ee37 5e6a 	vrshl.s16	q2, sl
[^>]*> ee37 5e6c 	vrshl.s16	q2, ip
[^>]*> ee37 5e6e 	vrshl.s16	q2, lr
[^>]*> ef10 8540 	vrshl.s16	q4, q0, q0
[^>]*> ef12 8540 	vrshl.s16	q4, q0, q1
[^>]*> ef14 8540 	vrshl.s16	q4, q0, q2
[^>]*> ef18 8540 	vrshl.s16	q4, q0, q4
[^>]*> ef1e 8540 	vrshl.s16	q4, q0, q7
[^>]*> ef10 8542 	vrshl.s16	q4, q1, q0
[^>]*> ef12 8542 	vrshl.s16	q4, q1, q1
[^>]*> ef14 8542 	vrshl.s16	q4, q1, q2
[^>]*> ef18 8542 	vrshl.s16	q4, q1, q4
[^>]*> ef1e 8542 	vrshl.s16	q4, q1, q7
[^>]*> ef10 8544 	vrshl.s16	q4, q2, q0
[^>]*> ef12 8544 	vrshl.s16	q4, q2, q1
[^>]*> ef14 8544 	vrshl.s16	q4, q2, q2
[^>]*> ef18 8544 	vrshl.s16	q4, q2, q4
[^>]*> ef1e 8544 	vrshl.s16	q4, q2, q7
[^>]*> ef10 8548 	vrshl.s16	q4, q4, q0
[^>]*> ef12 8548 	vrshl.s16	q4, q4, q1
[^>]*> ef14 8548 	vrshl.s16	q4, q4, q2
[^>]*> ef18 8548 	vrshl.s16	q4, q4, q4
[^>]*> ef1e 8548 	vrshl.s16	q4, q4, q7
[^>]*> ef10 854e 	vrshl.s16	q4, q7, q0
[^>]*> ef12 854e 	vrshl.s16	q4, q7, q1
[^>]*> ef14 854e 	vrshl.s16	q4, q7, q2
[^>]*> ef18 854e 	vrshl.s16	q4, q7, q4
[^>]*> ef1e 854e 	vrshl.s16	q4, q7, q7
[^>]*> ee37 9e60 	vrshl.s16	q4, r0
[^>]*> ee37 9e61 	vrshl.s16	q4, r1
[^>]*> ee37 9e62 	vrshl.s16	q4, r2
[^>]*> ee37 9e64 	vrshl.s16	q4, r4
[^>]*> ee37 9e67 	vrshl.s16	q4, r7
[^>]*> ee37 9e68 	vrshl.s16	q4, r8
[^>]*> ee37 9e6a 	vrshl.s16	q4, sl
[^>]*> ee37 9e6c 	vrshl.s16	q4, ip
[^>]*> ee37 9e6e 	vrshl.s16	q4, lr
[^>]*> ef10 e540 	vrshl.s16	q7, q0, q0
[^>]*> ef12 e540 	vrshl.s16	q7, q0, q1
[^>]*> ef14 e540 	vrshl.s16	q7, q0, q2
[^>]*> ef18 e540 	vrshl.s16	q7, q0, q4
[^>]*> ef1e e540 	vrshl.s16	q7, q0, q7
[^>]*> ef10 e542 	vrshl.s16	q7, q1, q0
[^>]*> ef12 e542 	vrshl.s16	q7, q1, q1
[^>]*> ef14 e542 	vrshl.s16	q7, q1, q2
[^>]*> ef18 e542 	vrshl.s16	q7, q1, q4
[^>]*> ef1e e542 	vrshl.s16	q7, q1, q7
[^>]*> ef10 e544 	vrshl.s16	q7, q2, q0
[^>]*> ef12 e544 	vrshl.s16	q7, q2, q1
[^>]*> ef14 e544 	vrshl.s16	q7, q2, q2
[^>]*> ef18 e544 	vrshl.s16	q7, q2, q4
[^>]*> ef1e e544 	vrshl.s16	q7, q2, q7
[^>]*> ef10 e548 	vrshl.s16	q7, q4, q0
[^>]*> ef12 e548 	vrshl.s16	q7, q4, q1
[^>]*> ef14 e548 	vrshl.s16	q7, q4, q2
[^>]*> ef18 e548 	vrshl.s16	q7, q4, q4
[^>]*> ef1e e548 	vrshl.s16	q7, q4, q7
[^>]*> ef10 e54e 	vrshl.s16	q7, q7, q0
[^>]*> ef12 e54e 	vrshl.s16	q7, q7, q1
[^>]*> ef14 e54e 	vrshl.s16	q7, q7, q2
[^>]*> ef18 e54e 	vrshl.s16	q7, q7, q4
[^>]*> ef1e e54e 	vrshl.s16	q7, q7, q7
[^>]*> ee37 fe60 	vrshl.s16	q7, r0
[^>]*> ee37 fe61 	vrshl.s16	q7, r1
[^>]*> ee37 fe62 	vrshl.s16	q7, r2
[^>]*> ee37 fe64 	vrshl.s16	q7, r4
[^>]*> ee37 fe67 	vrshl.s16	q7, r7
[^>]*> ee37 fe68 	vrshl.s16	q7, r8
[^>]*> ee37 fe6a 	vrshl.s16	q7, sl
[^>]*> ee37 fe6c 	vrshl.s16	q7, ip
[^>]*> ee37 fe6e 	vrshl.s16	q7, lr
[^>]*> ff20 0540 	vrshl.u32	q0, q0, q0
[^>]*> ff22 0540 	vrshl.u32	q0, q0, q1
[^>]*> ff24 0540 	vrshl.u32	q0, q0, q2
[^>]*> ff28 0540 	vrshl.u32	q0, q0, q4
[^>]*> ff2e 0540 	vrshl.u32	q0, q0, q7
[^>]*> ff20 0542 	vrshl.u32	q0, q1, q0
[^>]*> ff22 0542 	vrshl.u32	q0, q1, q1
[^>]*> ff24 0542 	vrshl.u32	q0, q1, q2
[^>]*> ff28 0542 	vrshl.u32	q0, q1, q4
[^>]*> ff2e 0542 	vrshl.u32	q0, q1, q7
[^>]*> ff20 0544 	vrshl.u32	q0, q2, q0
[^>]*> ff22 0544 	vrshl.u32	q0, q2, q1
[^>]*> ff24 0544 	vrshl.u32	q0, q2, q2
[^>]*> ff28 0544 	vrshl.u32	q0, q2, q4
[^>]*> ff2e 0544 	vrshl.u32	q0, q2, q7
[^>]*> ff20 0548 	vrshl.u32	q0, q4, q0
[^>]*> ff22 0548 	vrshl.u32	q0, q4, q1
[^>]*> ff24 0548 	vrshl.u32	q0, q4, q2
[^>]*> ff28 0548 	vrshl.u32	q0, q4, q4
[^>]*> ff2e 0548 	vrshl.u32	q0, q4, q7
[^>]*> ff20 054e 	vrshl.u32	q0, q7, q0
[^>]*> ff22 054e 	vrshl.u32	q0, q7, q1
[^>]*> ff24 054e 	vrshl.u32	q0, q7, q2
[^>]*> ff28 054e 	vrshl.u32	q0, q7, q4
[^>]*> ff2e 054e 	vrshl.u32	q0, q7, q7
[^>]*> fe3b 1e60 	vrshl.u32	q0, r0
[^>]*> fe3b 1e61 	vrshl.u32	q0, r1
[^>]*> fe3b 1e62 	vrshl.u32	q0, r2
[^>]*> fe3b 1e64 	vrshl.u32	q0, r4
[^>]*> fe3b 1e67 	vrshl.u32	q0, r7
[^>]*> fe3b 1e68 	vrshl.u32	q0, r8
[^>]*> fe3b 1e6a 	vrshl.u32	q0, sl
[^>]*> fe3b 1e6c 	vrshl.u32	q0, ip
[^>]*> fe3b 1e6e 	vrshl.u32	q0, lr
[^>]*> ff20 2540 	vrshl.u32	q1, q0, q0
[^>]*> ff22 2540 	vrshl.u32	q1, q0, q1
[^>]*> ff24 2540 	vrshl.u32	q1, q0, q2
[^>]*> ff28 2540 	vrshl.u32	q1, q0, q4
[^>]*> ff2e 2540 	vrshl.u32	q1, q0, q7
[^>]*> ff20 2542 	vrshl.u32	q1, q1, q0
[^>]*> ff22 2542 	vrshl.u32	q1, q1, q1
[^>]*> ff24 2542 	vrshl.u32	q1, q1, q2
[^>]*> ff28 2542 	vrshl.u32	q1, q1, q4
[^>]*> ff2e 2542 	vrshl.u32	q1, q1, q7
[^>]*> ff20 2544 	vrshl.u32	q1, q2, q0
[^>]*> ff22 2544 	vrshl.u32	q1, q2, q1
[^>]*> ff24 2544 	vrshl.u32	q1, q2, q2
[^>]*> ff28 2544 	vrshl.u32	q1, q2, q4
[^>]*> ff2e 2544 	vrshl.u32	q1, q2, q7
[^>]*> ff20 2548 	vrshl.u32	q1, q4, q0
[^>]*> ff22 2548 	vrshl.u32	q1, q4, q1
[^>]*> ff24 2548 	vrshl.u32	q1, q4, q2
[^>]*> ff28 2548 	vrshl.u32	q1, q4, q4
[^>]*> ff2e 2548 	vrshl.u32	q1, q4, q7
[^>]*> ff20 254e 	vrshl.u32	q1, q7, q0
[^>]*> ff22 254e 	vrshl.u32	q1, q7, q1
[^>]*> ff24 254e 	vrshl.u32	q1, q7, q2
[^>]*> ff28 254e 	vrshl.u32	q1, q7, q4
[^>]*> ff2e 254e 	vrshl.u32	q1, q7, q7
[^>]*> fe3b 3e60 	vrshl.u32	q1, r0
[^>]*> fe3b 3e61 	vrshl.u32	q1, r1
[^>]*> fe3b 3e62 	vrshl.u32	q1, r2
[^>]*> fe3b 3e64 	vrshl.u32	q1, r4
[^>]*> fe3b 3e67 	vrshl.u32	q1, r7
[^>]*> fe3b 3e68 	vrshl.u32	q1, r8
[^>]*> fe3b 3e6a 	vrshl.u32	q1, sl
[^>]*> fe3b 3e6c 	vrshl.u32	q1, ip
[^>]*> fe3b 3e6e 	vrshl.u32	q1, lr
[^>]*> ff20 4540 	vrshl.u32	q2, q0, q0
[^>]*> ff22 4540 	vrshl.u32	q2, q0, q1
[^>]*> ff24 4540 	vrshl.u32	q2, q0, q2
[^>]*> ff28 4540 	vrshl.u32	q2, q0, q4
[^>]*> ff2e 4540 	vrshl.u32	q2, q0, q7
[^>]*> ff20 4542 	vrshl.u32	q2, q1, q0
[^>]*> ff22 4542 	vrshl.u32	q2, q1, q1
[^>]*> ff24 4542 	vrshl.u32	q2, q1, q2
[^>]*> ff28 4542 	vrshl.u32	q2, q1, q4
[^>]*> ff2e 4542 	vrshl.u32	q2, q1, q7
[^>]*> ff20 4544 	vrshl.u32	q2, q2, q0
[^>]*> ff22 4544 	vrshl.u32	q2, q2, q1
[^>]*> ff24 4544 	vrshl.u32	q2, q2, q2
[^>]*> ff28 4544 	vrshl.u32	q2, q2, q4
[^>]*> ff2e 4544 	vrshl.u32	q2, q2, q7
[^>]*> ff20 4548 	vrshl.u32	q2, q4, q0
[^>]*> ff22 4548 	vrshl.u32	q2, q4, q1
[^>]*> ff24 4548 	vrshl.u32	q2, q4, q2
[^>]*> ff28 4548 	vrshl.u32	q2, q4, q4
[^>]*> ff2e 4548 	vrshl.u32	q2, q4, q7
[^>]*> ff20 454e 	vrshl.u32	q2, q7, q0
[^>]*> ff22 454e 	vrshl.u32	q2, q7, q1
[^>]*> ff24 454e 	vrshl.u32	q2, q7, q2
[^>]*> ff28 454e 	vrshl.u32	q2, q7, q4
[^>]*> ff2e 454e 	vrshl.u32	q2, q7, q7
[^>]*> fe3b 5e60 	vrshl.u32	q2, r0
[^>]*> fe3b 5e61 	vrshl.u32	q2, r1
[^>]*> fe3b 5e62 	vrshl.u32	q2, r2
[^>]*> fe3b 5e64 	vrshl.u32	q2, r4
[^>]*> fe3b 5e67 	vrshl.u32	q2, r7
[^>]*> fe3b 5e68 	vrshl.u32	q2, r8
[^>]*> fe3b 5e6a 	vrshl.u32	q2, sl
[^>]*> fe3b 5e6c 	vrshl.u32	q2, ip
[^>]*> fe3b 5e6e 	vrshl.u32	q2, lr
[^>]*> ff20 8540 	vrshl.u32	q4, q0, q0
[^>]*> ff22 8540 	vrshl.u32	q4, q0, q1
[^>]*> ff24 8540 	vrshl.u32	q4, q0, q2
[^>]*> ff28 8540 	vrshl.u32	q4, q0, q4
[^>]*> ff2e 8540 	vrshl.u32	q4, q0, q7
[^>]*> ff20 8542 	vrshl.u32	q4, q1, q0
[^>]*> ff22 8542 	vrshl.u32	q4, q1, q1
[^>]*> ff24 8542 	vrshl.u32	q4, q1, q2
[^>]*> ff28 8542 	vrshl.u32	q4, q1, q4
[^>]*> ff2e 8542 	vrshl.u32	q4, q1, q7
[^>]*> ff20 8544 	vrshl.u32	q4, q2, q0
[^>]*> ff22 8544 	vrshl.u32	q4, q2, q1
[^>]*> ff24 8544 	vrshl.u32	q4, q2, q2
[^>]*> ff28 8544 	vrshl.u32	q4, q2, q4
[^>]*> ff2e 8544 	vrshl.u32	q4, q2, q7
[^>]*> ff20 8548 	vrshl.u32	q4, q4, q0
[^>]*> ff22 8548 	vrshl.u32	q4, q4, q1
[^>]*> ff24 8548 	vrshl.u32	q4, q4, q2
[^>]*> ff28 8548 	vrshl.u32	q4, q4, q4
[^>]*> ff2e 8548 	vrshl.u32	q4, q4, q7
[^>]*> ff20 854e 	vrshl.u32	q4, q7, q0
[^>]*> ff22 854e 	vrshl.u32	q4, q7, q1
[^>]*> ff24 854e 	vrshl.u32	q4, q7, q2
[^>]*> ff28 854e 	vrshl.u32	q4, q7, q4
[^>]*> ff2e 854e 	vrshl.u32	q4, q7, q7
[^>]*> fe3b 9e60 	vrshl.u32	q4, r0
[^>]*> fe3b 9e61 	vrshl.u32	q4, r1
[^>]*> fe3b 9e62 	vrshl.u32	q4, r2
[^>]*> fe3b 9e64 	vrshl.u32	q4, r4
[^>]*> fe3b 9e67 	vrshl.u32	q4, r7
[^>]*> fe3b 9e68 	vrshl.u32	q4, r8
[^>]*> fe3b 9e6a 	vrshl.u32	q4, sl
[^>]*> fe3b 9e6c 	vrshl.u32	q4, ip
[^>]*> fe3b 9e6e 	vrshl.u32	q4, lr
[^>]*> ff20 e540 	vrshl.u32	q7, q0, q0
[^>]*> ff22 e540 	vrshl.u32	q7, q0, q1
[^>]*> ff24 e540 	vrshl.u32	q7, q0, q2
[^>]*> ff28 e540 	vrshl.u32	q7, q0, q4
[^>]*> ff2e e540 	vrshl.u32	q7, q0, q7
[^>]*> ff20 e542 	vrshl.u32	q7, q1, q0
[^>]*> ff22 e542 	vrshl.u32	q7, q1, q1
[^>]*> ff24 e542 	vrshl.u32	q7, q1, q2
[^>]*> ff28 e542 	vrshl.u32	q7, q1, q4
[^>]*> ff2e e542 	vrshl.u32	q7, q1, q7
[^>]*> ff20 e544 	vrshl.u32	q7, q2, q0
[^>]*> ff22 e544 	vrshl.u32	q7, q2, q1
[^>]*> ff24 e544 	vrshl.u32	q7, q2, q2
[^>]*> ff28 e544 	vrshl.u32	q7, q2, q4
[^>]*> ff2e e544 	vrshl.u32	q7, q2, q7
[^>]*> ff20 e548 	vrshl.u32	q7, q4, q0
[^>]*> ff22 e548 	vrshl.u32	q7, q4, q1
[^>]*> ff24 e548 	vrshl.u32	q7, q4, q2
[^>]*> ff28 e548 	vrshl.u32	q7, q4, q4
[^>]*> ff2e e548 	vrshl.u32	q7, q4, q7
[^>]*> ff20 e54e 	vrshl.u32	q7, q7, q0
[^>]*> ff22 e54e 	vrshl.u32	q7, q7, q1
[^>]*> ff24 e54e 	vrshl.u32	q7, q7, q2
[^>]*> ff28 e54e 	vrshl.u32	q7, q7, q4
[^>]*> ff2e e54e 	vrshl.u32	q7, q7, q7
[^>]*> fe3b fe60 	vrshl.u32	q7, r0
[^>]*> fe3b fe61 	vrshl.u32	q7, r1
[^>]*> fe3b fe62 	vrshl.u32	q7, r2
[^>]*> fe3b fe64 	vrshl.u32	q7, r4
[^>]*> fe3b fe67 	vrshl.u32	q7, r7
[^>]*> fe3b fe68 	vrshl.u32	q7, r8
[^>]*> fe3b fe6a 	vrshl.u32	q7, sl
[^>]*> fe3b fe6c 	vrshl.u32	q7, ip
[^>]*> fe3b fe6e 	vrshl.u32	q7, lr
[^>]*> ef20 0540 	vrshl.s32	q0, q0, q0
[^>]*> ef22 0540 	vrshl.s32	q0, q0, q1
[^>]*> ef24 0540 	vrshl.s32	q0, q0, q2
[^>]*> ef28 0540 	vrshl.s32	q0, q0, q4
[^>]*> ef2e 0540 	vrshl.s32	q0, q0, q7
[^>]*> ef20 0542 	vrshl.s32	q0, q1, q0
[^>]*> ef22 0542 	vrshl.s32	q0, q1, q1
[^>]*> ef24 0542 	vrshl.s32	q0, q1, q2
[^>]*> ef28 0542 	vrshl.s32	q0, q1, q4
[^>]*> ef2e 0542 	vrshl.s32	q0, q1, q7
[^>]*> ef20 0544 	vrshl.s32	q0, q2, q0
[^>]*> ef22 0544 	vrshl.s32	q0, q2, q1
[^>]*> ef24 0544 	vrshl.s32	q0, q2, q2
[^>]*> ef28 0544 	vrshl.s32	q0, q2, q4
[^>]*> ef2e 0544 	vrshl.s32	q0, q2, q7
[^>]*> ef20 0548 	vrshl.s32	q0, q4, q0
[^>]*> ef22 0548 	vrshl.s32	q0, q4, q1
[^>]*> ef24 0548 	vrshl.s32	q0, q4, q2
[^>]*> ef28 0548 	vrshl.s32	q0, q4, q4
[^>]*> ef2e 0548 	vrshl.s32	q0, q4, q7
[^>]*> ef20 054e 	vrshl.s32	q0, q7, q0
[^>]*> ef22 054e 	vrshl.s32	q0, q7, q1
[^>]*> ef24 054e 	vrshl.s32	q0, q7, q2
[^>]*> ef28 054e 	vrshl.s32	q0, q7, q4
[^>]*> ef2e 054e 	vrshl.s32	q0, q7, q7
[^>]*> ee3b 1e60 	vrshl.s32	q0, r0
[^>]*> ee3b 1e61 	vrshl.s32	q0, r1
[^>]*> ee3b 1e62 	vrshl.s32	q0, r2
[^>]*> ee3b 1e64 	vrshl.s32	q0, r4
[^>]*> ee3b 1e67 	vrshl.s32	q0, r7
[^>]*> ee3b 1e68 	vrshl.s32	q0, r8
[^>]*> ee3b 1e6a 	vrshl.s32	q0, sl
[^>]*> ee3b 1e6c 	vrshl.s32	q0, ip
[^>]*> ee3b 1e6e 	vrshl.s32	q0, lr
[^>]*> ef20 2540 	vrshl.s32	q1, q0, q0
[^>]*> ef22 2540 	vrshl.s32	q1, q0, q1
[^>]*> ef24 2540 	vrshl.s32	q1, q0, q2
[^>]*> ef28 2540 	vrshl.s32	q1, q0, q4
[^>]*> ef2e 2540 	vrshl.s32	q1, q0, q7
[^>]*> ef20 2542 	vrshl.s32	q1, q1, q0
[^>]*> ef22 2542 	vrshl.s32	q1, q1, q1
[^>]*> ef24 2542 	vrshl.s32	q1, q1, q2
[^>]*> ef28 2542 	vrshl.s32	q1, q1, q4
[^>]*> ef2e 2542 	vrshl.s32	q1, q1, q7
[^>]*> ef20 2544 	vrshl.s32	q1, q2, q0
[^>]*> ef22 2544 	vrshl.s32	q1, q2, q1
[^>]*> ef24 2544 	vrshl.s32	q1, q2, q2
[^>]*> ef28 2544 	vrshl.s32	q1, q2, q4
[^>]*> ef2e 2544 	vrshl.s32	q1, q2, q7
[^>]*> ef20 2548 	vrshl.s32	q1, q4, q0
[^>]*> ef22 2548 	vrshl.s32	q1, q4, q1
[^>]*> ef24 2548 	vrshl.s32	q1, q4, q2
[^>]*> ef28 2548 	vrshl.s32	q1, q4, q4
[^>]*> ef2e 2548 	vrshl.s32	q1, q4, q7
[^>]*> ef20 254e 	vrshl.s32	q1, q7, q0
[^>]*> ef22 254e 	vrshl.s32	q1, q7, q1
[^>]*> ef24 254e 	vrshl.s32	q1, q7, q2
[^>]*> ef28 254e 	vrshl.s32	q1, q7, q4
[^>]*> ef2e 254e 	vrshl.s32	q1, q7, q7
[^>]*> ee3b 3e60 	vrshl.s32	q1, r0
[^>]*> ee3b 3e61 	vrshl.s32	q1, r1
[^>]*> ee3b 3e62 	vrshl.s32	q1, r2
[^>]*> ee3b 3e64 	vrshl.s32	q1, r4
[^>]*> ee3b 3e67 	vrshl.s32	q1, r7
[^>]*> ee3b 3e68 	vrshl.s32	q1, r8
[^>]*> ee3b 3e6a 	vrshl.s32	q1, sl
[^>]*> ee3b 3e6c 	vrshl.s32	q1, ip
[^>]*> ee3b 3e6e 	vrshl.s32	q1, lr
[^>]*> ef20 4540 	vrshl.s32	q2, q0, q0
[^>]*> ef22 4540 	vrshl.s32	q2, q0, q1
[^>]*> ef24 4540 	vrshl.s32	q2, q0, q2
[^>]*> ef28 4540 	vrshl.s32	q2, q0, q4
[^>]*> ef2e 4540 	vrshl.s32	q2, q0, q7
[^>]*> ef20 4542 	vrshl.s32	q2, q1, q0
[^>]*> ef22 4542 	vrshl.s32	q2, q1, q1
[^>]*> ef24 4542 	vrshl.s32	q2, q1, q2
[^>]*> ef28 4542 	vrshl.s32	q2, q1, q4
[^>]*> ef2e 4542 	vrshl.s32	q2, q1, q7
[^>]*> ef20 4544 	vrshl.s32	q2, q2, q0
[^>]*> ef22 4544 	vrshl.s32	q2, q2, q1
[^>]*> ef24 4544 	vrshl.s32	q2, q2, q2
[^>]*> ef28 4544 	vrshl.s32	q2, q2, q4
[^>]*> ef2e 4544 	vrshl.s32	q2, q2, q7
[^>]*> ef20 4548 	vrshl.s32	q2, q4, q0
[^>]*> ef22 4548 	vrshl.s32	q2, q4, q1
[^>]*> ef24 4548 	vrshl.s32	q2, q4, q2
[^>]*> ef28 4548 	vrshl.s32	q2, q4, q4
[^>]*> ef2e 4548 	vrshl.s32	q2, q4, q7
[^>]*> ef20 454e 	vrshl.s32	q2, q7, q0
[^>]*> ef22 454e 	vrshl.s32	q2, q7, q1
[^>]*> ef24 454e 	vrshl.s32	q2, q7, q2
[^>]*> ef28 454e 	vrshl.s32	q2, q7, q4
[^>]*> ef2e 454e 	vrshl.s32	q2, q7, q7
[^>]*> ee3b 5e60 	vrshl.s32	q2, r0
[^>]*> ee3b 5e61 	vrshl.s32	q2, r1
[^>]*> ee3b 5e62 	vrshl.s32	q2, r2
[^>]*> ee3b 5e64 	vrshl.s32	q2, r4
[^>]*> ee3b 5e67 	vrshl.s32	q2, r7
[^>]*> ee3b 5e68 	vrshl.s32	q2, r8
[^>]*> ee3b 5e6a 	vrshl.s32	q2, sl
[^>]*> ee3b 5e6c 	vrshl.s32	q2, ip
[^>]*> ee3b 5e6e 	vrshl.s32	q2, lr
[^>]*> ef20 8540 	vrshl.s32	q4, q0, q0
[^>]*> ef22 8540 	vrshl.s32	q4, q0, q1
[^>]*> ef24 8540 	vrshl.s32	q4, q0, q2
[^>]*> ef28 8540 	vrshl.s32	q4, q0, q4
[^>]*> ef2e 8540 	vrshl.s32	q4, q0, q7
[^>]*> ef20 8542 	vrshl.s32	q4, q1, q0
[^>]*> ef22 8542 	vrshl.s32	q4, q1, q1
[^>]*> ef24 8542 	vrshl.s32	q4, q1, q2
[^>]*> ef28 8542 	vrshl.s32	q4, q1, q4
[^>]*> ef2e 8542 	vrshl.s32	q4, q1, q7
[^>]*> ef20 8544 	vrshl.s32	q4, q2, q0
[^>]*> ef22 8544 	vrshl.s32	q4, q2, q1
[^>]*> ef24 8544 	vrshl.s32	q4, q2, q2
[^>]*> ef28 8544 	vrshl.s32	q4, q2, q4
[^>]*> ef2e 8544 	vrshl.s32	q4, q2, q7
[^>]*> ef20 8548 	vrshl.s32	q4, q4, q0
[^>]*> ef22 8548 	vrshl.s32	q4, q4, q1
[^>]*> ef24 8548 	vrshl.s32	q4, q4, q2
[^>]*> ef28 8548 	vrshl.s32	q4, q4, q4
[^>]*> ef2e 8548 	vrshl.s32	q4, q4, q7
[^>]*> ef20 854e 	vrshl.s32	q4, q7, q0
[^>]*> ef22 854e 	vrshl.s32	q4, q7, q1
[^>]*> ef24 854e 	vrshl.s32	q4, q7, q2
[^>]*> ef28 854e 	vrshl.s32	q4, q7, q4
[^>]*> ef2e 854e 	vrshl.s32	q4, q7, q7
[^>]*> ee3b 9e60 	vrshl.s32	q4, r0
[^>]*> ee3b 9e61 	vrshl.s32	q4, r1
[^>]*> ee3b 9e62 	vrshl.s32	q4, r2
[^>]*> ee3b 9e64 	vrshl.s32	q4, r4
[^>]*> ee3b 9e67 	vrshl.s32	q4, r7
[^>]*> ee3b 9e68 	vrshl.s32	q4, r8
[^>]*> ee3b 9e6a 	vrshl.s32	q4, sl
[^>]*> ee3b 9e6c 	vrshl.s32	q4, ip
[^>]*> ee3b 9e6e 	vrshl.s32	q4, lr
[^>]*> ef20 e540 	vrshl.s32	q7, q0, q0
[^>]*> ef22 e540 	vrshl.s32	q7, q0, q1
[^>]*> ef24 e540 	vrshl.s32	q7, q0, q2
[^>]*> ef28 e540 	vrshl.s32	q7, q0, q4
[^>]*> ef2e e540 	vrshl.s32	q7, q0, q7
[^>]*> ef20 e542 	vrshl.s32	q7, q1, q0
[^>]*> ef22 e542 	vrshl.s32	q7, q1, q1
[^>]*> ef24 e542 	vrshl.s32	q7, q1, q2
[^>]*> ef28 e542 	vrshl.s32	q7, q1, q4
[^>]*> ef2e e542 	vrshl.s32	q7, q1, q7
[^>]*> ef20 e544 	vrshl.s32	q7, q2, q0
[^>]*> ef22 e544 	vrshl.s32	q7, q2, q1
[^>]*> ef24 e544 	vrshl.s32	q7, q2, q2
[^>]*> ef28 e544 	vrshl.s32	q7, q2, q4
[^>]*> ef2e e544 	vrshl.s32	q7, q2, q7
[^>]*> ef20 e548 	vrshl.s32	q7, q4, q0
[^>]*> ef22 e548 	vrshl.s32	q7, q4, q1
[^>]*> ef24 e548 	vrshl.s32	q7, q4, q2
[^>]*> ef28 e548 	vrshl.s32	q7, q4, q4
[^>]*> ef2e e548 	vrshl.s32	q7, q4, q7
[^>]*> ef20 e54e 	vrshl.s32	q7, q7, q0
[^>]*> ef22 e54e 	vrshl.s32	q7, q7, q1
[^>]*> ef24 e54e 	vrshl.s32	q7, q7, q2
[^>]*> ef28 e54e 	vrshl.s32	q7, q7, q4
[^>]*> ef2e e54e 	vrshl.s32	q7, q7, q7
[^>]*> ee3b fe60 	vrshl.s32	q7, r0
[^>]*> ee3b fe61 	vrshl.s32	q7, r1
[^>]*> ee3b fe62 	vrshl.s32	q7, r2
[^>]*> ee3b fe64 	vrshl.s32	q7, r4
[^>]*> ee3b fe67 	vrshl.s32	q7, r7
[^>]*> ee3b fe68 	vrshl.s32	q7, r8
[^>]*> ee3b fe6a 	vrshl.s32	q7, sl
[^>]*> ee3b fe6c 	vrshl.s32	q7, ip
[^>]*> ee3b fe6e 	vrshl.s32	q7, lr
[^>]*> fe71 ef4d 	vpstete
[^>]*> ef04 0542 	vrshlt.s8	q0, q1, q2
[^>]*> ff1e e54e 	vrshle.u16	q7, q7, q7
[^>]*> ee3b 1e62 	vrshlt.s32	q0, r2
[^>]*> fe33 fe6e 	vrshle.u8	q7, lr
