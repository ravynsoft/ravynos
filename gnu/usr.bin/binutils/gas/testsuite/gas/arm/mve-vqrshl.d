# name: MVE vqrshl instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ef00 0550 	vqrshl.s8	q0, q0, q0
[^>]*> ef02 0550 	vqrshl.s8	q0, q0, q1
[^>]*> ef04 0550 	vqrshl.s8	q0, q0, q2
[^>]*> ef08 0550 	vqrshl.s8	q0, q0, q4
[^>]*> ef0e 0550 	vqrshl.s8	q0, q0, q7
[^>]*> ef00 0552 	vqrshl.s8	q0, q1, q0
[^>]*> ef02 0552 	vqrshl.s8	q0, q1, q1
[^>]*> ef04 0552 	vqrshl.s8	q0, q1, q2
[^>]*> ef08 0552 	vqrshl.s8	q0, q1, q4
[^>]*> ef0e 0552 	vqrshl.s8	q0, q1, q7
[^>]*> ef00 0554 	vqrshl.s8	q0, q2, q0
[^>]*> ef02 0554 	vqrshl.s8	q0, q2, q1
[^>]*> ef04 0554 	vqrshl.s8	q0, q2, q2
[^>]*> ef08 0554 	vqrshl.s8	q0, q2, q4
[^>]*> ef0e 0554 	vqrshl.s8	q0, q2, q7
[^>]*> ef00 0558 	vqrshl.s8	q0, q4, q0
[^>]*> ef02 0558 	vqrshl.s8	q0, q4, q1
[^>]*> ef04 0558 	vqrshl.s8	q0, q4, q2
[^>]*> ef08 0558 	vqrshl.s8	q0, q4, q4
[^>]*> ef0e 0558 	vqrshl.s8	q0, q4, q7
[^>]*> ef00 055e 	vqrshl.s8	q0, q7, q0
[^>]*> ef02 055e 	vqrshl.s8	q0, q7, q1
[^>]*> ef04 055e 	vqrshl.s8	q0, q7, q2
[^>]*> ef08 055e 	vqrshl.s8	q0, q7, q4
[^>]*> ef0e 055e 	vqrshl.s8	q0, q7, q7
[^>]*> ee33 1ee0 	vqrshl.s8	q0, r0
[^>]*> ee33 1ee1 	vqrshl.s8	q0, r1
[^>]*> ee33 1ee2 	vqrshl.s8	q0, r2
[^>]*> ee33 1ee4 	vqrshl.s8	q0, r4
[^>]*> ee33 1ee7 	vqrshl.s8	q0, r7
[^>]*> ee33 1ee8 	vqrshl.s8	q0, r8
[^>]*> ee33 1eea 	vqrshl.s8	q0, sl
[^>]*> ee33 1eec 	vqrshl.s8	q0, ip
[^>]*> ee33 1eee 	vqrshl.s8	q0, lr
[^>]*> ef00 2550 	vqrshl.s8	q1, q0, q0
[^>]*> ef02 2550 	vqrshl.s8	q1, q0, q1
[^>]*> ef04 2550 	vqrshl.s8	q1, q0, q2
[^>]*> ef08 2550 	vqrshl.s8	q1, q0, q4
[^>]*> ef0e 2550 	vqrshl.s8	q1, q0, q7
[^>]*> ef00 2552 	vqrshl.s8	q1, q1, q0
[^>]*> ef02 2552 	vqrshl.s8	q1, q1, q1
[^>]*> ef04 2552 	vqrshl.s8	q1, q1, q2
[^>]*> ef08 2552 	vqrshl.s8	q1, q1, q4
[^>]*> ef0e 2552 	vqrshl.s8	q1, q1, q7
[^>]*> ef00 2554 	vqrshl.s8	q1, q2, q0
[^>]*> ef02 2554 	vqrshl.s8	q1, q2, q1
[^>]*> ef04 2554 	vqrshl.s8	q1, q2, q2
[^>]*> ef08 2554 	vqrshl.s8	q1, q2, q4
[^>]*> ef0e 2554 	vqrshl.s8	q1, q2, q7
[^>]*> ef00 2558 	vqrshl.s8	q1, q4, q0
[^>]*> ef02 2558 	vqrshl.s8	q1, q4, q1
[^>]*> ef04 2558 	vqrshl.s8	q1, q4, q2
[^>]*> ef08 2558 	vqrshl.s8	q1, q4, q4
[^>]*> ef0e 2558 	vqrshl.s8	q1, q4, q7
[^>]*> ef00 255e 	vqrshl.s8	q1, q7, q0
[^>]*> ef02 255e 	vqrshl.s8	q1, q7, q1
[^>]*> ef04 255e 	vqrshl.s8	q1, q7, q2
[^>]*> ef08 255e 	vqrshl.s8	q1, q7, q4
[^>]*> ef0e 255e 	vqrshl.s8	q1, q7, q7
[^>]*> ee33 3ee0 	vqrshl.s8	q1, r0
[^>]*> ee33 3ee1 	vqrshl.s8	q1, r1
[^>]*> ee33 3ee2 	vqrshl.s8	q1, r2
[^>]*> ee33 3ee4 	vqrshl.s8	q1, r4
[^>]*> ee33 3ee7 	vqrshl.s8	q1, r7
[^>]*> ee33 3ee8 	vqrshl.s8	q1, r8
[^>]*> ee33 3eea 	vqrshl.s8	q1, sl
[^>]*> ee33 3eec 	vqrshl.s8	q1, ip
[^>]*> ee33 3eee 	vqrshl.s8	q1, lr
[^>]*> ef00 4550 	vqrshl.s8	q2, q0, q0
[^>]*> ef02 4550 	vqrshl.s8	q2, q0, q1
[^>]*> ef04 4550 	vqrshl.s8	q2, q0, q2
[^>]*> ef08 4550 	vqrshl.s8	q2, q0, q4
[^>]*> ef0e 4550 	vqrshl.s8	q2, q0, q7
[^>]*> ef00 4552 	vqrshl.s8	q2, q1, q0
[^>]*> ef02 4552 	vqrshl.s8	q2, q1, q1
[^>]*> ef04 4552 	vqrshl.s8	q2, q1, q2
[^>]*> ef08 4552 	vqrshl.s8	q2, q1, q4
[^>]*> ef0e 4552 	vqrshl.s8	q2, q1, q7
[^>]*> ef00 4554 	vqrshl.s8	q2, q2, q0
[^>]*> ef02 4554 	vqrshl.s8	q2, q2, q1
[^>]*> ef04 4554 	vqrshl.s8	q2, q2, q2
[^>]*> ef08 4554 	vqrshl.s8	q2, q2, q4
[^>]*> ef0e 4554 	vqrshl.s8	q2, q2, q7
[^>]*> ef00 4558 	vqrshl.s8	q2, q4, q0
[^>]*> ef02 4558 	vqrshl.s8	q2, q4, q1
[^>]*> ef04 4558 	vqrshl.s8	q2, q4, q2
[^>]*> ef08 4558 	vqrshl.s8	q2, q4, q4
[^>]*> ef0e 4558 	vqrshl.s8	q2, q4, q7
[^>]*> ef00 455e 	vqrshl.s8	q2, q7, q0
[^>]*> ef02 455e 	vqrshl.s8	q2, q7, q1
[^>]*> ef04 455e 	vqrshl.s8	q2, q7, q2
[^>]*> ef08 455e 	vqrshl.s8	q2, q7, q4
[^>]*> ef0e 455e 	vqrshl.s8	q2, q7, q7
[^>]*> ee33 5ee0 	vqrshl.s8	q2, r0
[^>]*> ee33 5ee1 	vqrshl.s8	q2, r1
[^>]*> ee33 5ee2 	vqrshl.s8	q2, r2
[^>]*> ee33 5ee4 	vqrshl.s8	q2, r4
[^>]*> ee33 5ee7 	vqrshl.s8	q2, r7
[^>]*> ee33 5ee8 	vqrshl.s8	q2, r8
[^>]*> ee33 5eea 	vqrshl.s8	q2, sl
[^>]*> ee33 5eec 	vqrshl.s8	q2, ip
[^>]*> ee33 5eee 	vqrshl.s8	q2, lr
[^>]*> ef00 8550 	vqrshl.s8	q4, q0, q0
[^>]*> ef02 8550 	vqrshl.s8	q4, q0, q1
[^>]*> ef04 8550 	vqrshl.s8	q4, q0, q2
[^>]*> ef08 8550 	vqrshl.s8	q4, q0, q4
[^>]*> ef0e 8550 	vqrshl.s8	q4, q0, q7
[^>]*> ef00 8552 	vqrshl.s8	q4, q1, q0
[^>]*> ef02 8552 	vqrshl.s8	q4, q1, q1
[^>]*> ef04 8552 	vqrshl.s8	q4, q1, q2
[^>]*> ef08 8552 	vqrshl.s8	q4, q1, q4
[^>]*> ef0e 8552 	vqrshl.s8	q4, q1, q7
[^>]*> ef00 8554 	vqrshl.s8	q4, q2, q0
[^>]*> ef02 8554 	vqrshl.s8	q4, q2, q1
[^>]*> ef04 8554 	vqrshl.s8	q4, q2, q2
[^>]*> ef08 8554 	vqrshl.s8	q4, q2, q4
[^>]*> ef0e 8554 	vqrshl.s8	q4, q2, q7
[^>]*> ef00 8558 	vqrshl.s8	q4, q4, q0
[^>]*> ef02 8558 	vqrshl.s8	q4, q4, q1
[^>]*> ef04 8558 	vqrshl.s8	q4, q4, q2
[^>]*> ef08 8558 	vqrshl.s8	q4, q4, q4
[^>]*> ef0e 8558 	vqrshl.s8	q4, q4, q7
[^>]*> ef00 855e 	vqrshl.s8	q4, q7, q0
[^>]*> ef02 855e 	vqrshl.s8	q4, q7, q1
[^>]*> ef04 855e 	vqrshl.s8	q4, q7, q2
[^>]*> ef08 855e 	vqrshl.s8	q4, q7, q4
[^>]*> ef0e 855e 	vqrshl.s8	q4, q7, q7
[^>]*> ee33 9ee0 	vqrshl.s8	q4, r0
[^>]*> ee33 9ee1 	vqrshl.s8	q4, r1
[^>]*> ee33 9ee2 	vqrshl.s8	q4, r2
[^>]*> ee33 9ee4 	vqrshl.s8	q4, r4
[^>]*> ee33 9ee7 	vqrshl.s8	q4, r7
[^>]*> ee33 9ee8 	vqrshl.s8	q4, r8
[^>]*> ee33 9eea 	vqrshl.s8	q4, sl
[^>]*> ee33 9eec 	vqrshl.s8	q4, ip
[^>]*> ee33 9eee 	vqrshl.s8	q4, lr
[^>]*> ef00 e550 	vqrshl.s8	q7, q0, q0
[^>]*> ef02 e550 	vqrshl.s8	q7, q0, q1
[^>]*> ef04 e550 	vqrshl.s8	q7, q0, q2
[^>]*> ef08 e550 	vqrshl.s8	q7, q0, q4
[^>]*> ef0e e550 	vqrshl.s8	q7, q0, q7
[^>]*> ef00 e552 	vqrshl.s8	q7, q1, q0
[^>]*> ef02 e552 	vqrshl.s8	q7, q1, q1
[^>]*> ef04 e552 	vqrshl.s8	q7, q1, q2
[^>]*> ef08 e552 	vqrshl.s8	q7, q1, q4
[^>]*> ef0e e552 	vqrshl.s8	q7, q1, q7
[^>]*> ef00 e554 	vqrshl.s8	q7, q2, q0
[^>]*> ef02 e554 	vqrshl.s8	q7, q2, q1
[^>]*> ef04 e554 	vqrshl.s8	q7, q2, q2
[^>]*> ef08 e554 	vqrshl.s8	q7, q2, q4
[^>]*> ef0e e554 	vqrshl.s8	q7, q2, q7
[^>]*> ef00 e558 	vqrshl.s8	q7, q4, q0
[^>]*> ef02 e558 	vqrshl.s8	q7, q4, q1
[^>]*> ef04 e558 	vqrshl.s8	q7, q4, q2
[^>]*> ef08 e558 	vqrshl.s8	q7, q4, q4
[^>]*> ef0e e558 	vqrshl.s8	q7, q4, q7
[^>]*> ef00 e55e 	vqrshl.s8	q7, q7, q0
[^>]*> ef02 e55e 	vqrshl.s8	q7, q7, q1
[^>]*> ef04 e55e 	vqrshl.s8	q7, q7, q2
[^>]*> ef08 e55e 	vqrshl.s8	q7, q7, q4
[^>]*> ef0e e55e 	vqrshl.s8	q7, q7, q7
[^>]*> ee33 fee0 	vqrshl.s8	q7, r0
[^>]*> ee33 fee1 	vqrshl.s8	q7, r1
[^>]*> ee33 fee2 	vqrshl.s8	q7, r2
[^>]*> ee33 fee4 	vqrshl.s8	q7, r4
[^>]*> ee33 fee7 	vqrshl.s8	q7, r7
[^>]*> ee33 fee8 	vqrshl.s8	q7, r8
[^>]*> ee33 feea 	vqrshl.s8	q7, sl
[^>]*> ee33 feec 	vqrshl.s8	q7, ip
[^>]*> ee33 feee 	vqrshl.s8	q7, lr
[^>]*> ff00 0550 	vqrshl.u8	q0, q0, q0
[^>]*> ff02 0550 	vqrshl.u8	q0, q0, q1
[^>]*> ff04 0550 	vqrshl.u8	q0, q0, q2
[^>]*> ff08 0550 	vqrshl.u8	q0, q0, q4
[^>]*> ff0e 0550 	vqrshl.u8	q0, q0, q7
[^>]*> ff00 0552 	vqrshl.u8	q0, q1, q0
[^>]*> ff02 0552 	vqrshl.u8	q0, q1, q1
[^>]*> ff04 0552 	vqrshl.u8	q0, q1, q2
[^>]*> ff08 0552 	vqrshl.u8	q0, q1, q4
[^>]*> ff0e 0552 	vqrshl.u8	q0, q1, q7
[^>]*> ff00 0554 	vqrshl.u8	q0, q2, q0
[^>]*> ff02 0554 	vqrshl.u8	q0, q2, q1
[^>]*> ff04 0554 	vqrshl.u8	q0, q2, q2
[^>]*> ff08 0554 	vqrshl.u8	q0, q2, q4
[^>]*> ff0e 0554 	vqrshl.u8	q0, q2, q7
[^>]*> ff00 0558 	vqrshl.u8	q0, q4, q0
[^>]*> ff02 0558 	vqrshl.u8	q0, q4, q1
[^>]*> ff04 0558 	vqrshl.u8	q0, q4, q2
[^>]*> ff08 0558 	vqrshl.u8	q0, q4, q4
[^>]*> ff0e 0558 	vqrshl.u8	q0, q4, q7
[^>]*> ff00 055e 	vqrshl.u8	q0, q7, q0
[^>]*> ff02 055e 	vqrshl.u8	q0, q7, q1
[^>]*> ff04 055e 	vqrshl.u8	q0, q7, q2
[^>]*> ff08 055e 	vqrshl.u8	q0, q7, q4
[^>]*> ff0e 055e 	vqrshl.u8	q0, q7, q7
[^>]*> fe33 1ee0 	vqrshl.u8	q0, r0
[^>]*> fe33 1ee1 	vqrshl.u8	q0, r1
[^>]*> fe33 1ee2 	vqrshl.u8	q0, r2
[^>]*> fe33 1ee4 	vqrshl.u8	q0, r4
[^>]*> fe33 1ee7 	vqrshl.u8	q0, r7
[^>]*> fe33 1ee8 	vqrshl.u8	q0, r8
[^>]*> fe33 1eea 	vqrshl.u8	q0, sl
[^>]*> fe33 1eec 	vqrshl.u8	q0, ip
[^>]*> fe33 1eee 	vqrshl.u8	q0, lr
[^>]*> ff00 2550 	vqrshl.u8	q1, q0, q0
[^>]*> ff02 2550 	vqrshl.u8	q1, q0, q1
[^>]*> ff04 2550 	vqrshl.u8	q1, q0, q2
[^>]*> ff08 2550 	vqrshl.u8	q1, q0, q4
[^>]*> ff0e 2550 	vqrshl.u8	q1, q0, q7
[^>]*> ff00 2552 	vqrshl.u8	q1, q1, q0
[^>]*> ff02 2552 	vqrshl.u8	q1, q1, q1
[^>]*> ff04 2552 	vqrshl.u8	q1, q1, q2
[^>]*> ff08 2552 	vqrshl.u8	q1, q1, q4
[^>]*> ff0e 2552 	vqrshl.u8	q1, q1, q7
[^>]*> ff00 2554 	vqrshl.u8	q1, q2, q0
[^>]*> ff02 2554 	vqrshl.u8	q1, q2, q1
[^>]*> ff04 2554 	vqrshl.u8	q1, q2, q2
[^>]*> ff08 2554 	vqrshl.u8	q1, q2, q4
[^>]*> ff0e 2554 	vqrshl.u8	q1, q2, q7
[^>]*> ff00 2558 	vqrshl.u8	q1, q4, q0
[^>]*> ff02 2558 	vqrshl.u8	q1, q4, q1
[^>]*> ff04 2558 	vqrshl.u8	q1, q4, q2
[^>]*> ff08 2558 	vqrshl.u8	q1, q4, q4
[^>]*> ff0e 2558 	vqrshl.u8	q1, q4, q7
[^>]*> ff00 255e 	vqrshl.u8	q1, q7, q0
[^>]*> ff02 255e 	vqrshl.u8	q1, q7, q1
[^>]*> ff04 255e 	vqrshl.u8	q1, q7, q2
[^>]*> ff08 255e 	vqrshl.u8	q1, q7, q4
[^>]*> ff0e 255e 	vqrshl.u8	q1, q7, q7
[^>]*> fe33 3ee0 	vqrshl.u8	q1, r0
[^>]*> fe33 3ee1 	vqrshl.u8	q1, r1
[^>]*> fe33 3ee2 	vqrshl.u8	q1, r2
[^>]*> fe33 3ee4 	vqrshl.u8	q1, r4
[^>]*> fe33 3ee7 	vqrshl.u8	q1, r7
[^>]*> fe33 3ee8 	vqrshl.u8	q1, r8
[^>]*> fe33 3eea 	vqrshl.u8	q1, sl
[^>]*> fe33 3eec 	vqrshl.u8	q1, ip
[^>]*> fe33 3eee 	vqrshl.u8	q1, lr
[^>]*> ff00 4550 	vqrshl.u8	q2, q0, q0
[^>]*> ff02 4550 	vqrshl.u8	q2, q0, q1
[^>]*> ff04 4550 	vqrshl.u8	q2, q0, q2
[^>]*> ff08 4550 	vqrshl.u8	q2, q0, q4
[^>]*> ff0e 4550 	vqrshl.u8	q2, q0, q7
[^>]*> ff00 4552 	vqrshl.u8	q2, q1, q0
[^>]*> ff02 4552 	vqrshl.u8	q2, q1, q1
[^>]*> ff04 4552 	vqrshl.u8	q2, q1, q2
[^>]*> ff08 4552 	vqrshl.u8	q2, q1, q4
[^>]*> ff0e 4552 	vqrshl.u8	q2, q1, q7
[^>]*> ff00 4554 	vqrshl.u8	q2, q2, q0
[^>]*> ff02 4554 	vqrshl.u8	q2, q2, q1
[^>]*> ff04 4554 	vqrshl.u8	q2, q2, q2
[^>]*> ff08 4554 	vqrshl.u8	q2, q2, q4
[^>]*> ff0e 4554 	vqrshl.u8	q2, q2, q7
[^>]*> ff00 4558 	vqrshl.u8	q2, q4, q0
[^>]*> ff02 4558 	vqrshl.u8	q2, q4, q1
[^>]*> ff04 4558 	vqrshl.u8	q2, q4, q2
[^>]*> ff08 4558 	vqrshl.u8	q2, q4, q4
[^>]*> ff0e 4558 	vqrshl.u8	q2, q4, q7
[^>]*> ff00 455e 	vqrshl.u8	q2, q7, q0
[^>]*> ff02 455e 	vqrshl.u8	q2, q7, q1
[^>]*> ff04 455e 	vqrshl.u8	q2, q7, q2
[^>]*> ff08 455e 	vqrshl.u8	q2, q7, q4
[^>]*> ff0e 455e 	vqrshl.u8	q2, q7, q7
[^>]*> fe33 5ee0 	vqrshl.u8	q2, r0
[^>]*> fe33 5ee1 	vqrshl.u8	q2, r1
[^>]*> fe33 5ee2 	vqrshl.u8	q2, r2
[^>]*> fe33 5ee4 	vqrshl.u8	q2, r4
[^>]*> fe33 5ee7 	vqrshl.u8	q2, r7
[^>]*> fe33 5ee8 	vqrshl.u8	q2, r8
[^>]*> fe33 5eea 	vqrshl.u8	q2, sl
[^>]*> fe33 5eec 	vqrshl.u8	q2, ip
[^>]*> fe33 5eee 	vqrshl.u8	q2, lr
[^>]*> ff00 8550 	vqrshl.u8	q4, q0, q0
[^>]*> ff02 8550 	vqrshl.u8	q4, q0, q1
[^>]*> ff04 8550 	vqrshl.u8	q4, q0, q2
[^>]*> ff08 8550 	vqrshl.u8	q4, q0, q4
[^>]*> ff0e 8550 	vqrshl.u8	q4, q0, q7
[^>]*> ff00 8552 	vqrshl.u8	q4, q1, q0
[^>]*> ff02 8552 	vqrshl.u8	q4, q1, q1
[^>]*> ff04 8552 	vqrshl.u8	q4, q1, q2
[^>]*> ff08 8552 	vqrshl.u8	q4, q1, q4
[^>]*> ff0e 8552 	vqrshl.u8	q4, q1, q7
[^>]*> ff00 8554 	vqrshl.u8	q4, q2, q0
[^>]*> ff02 8554 	vqrshl.u8	q4, q2, q1
[^>]*> ff04 8554 	vqrshl.u8	q4, q2, q2
[^>]*> ff08 8554 	vqrshl.u8	q4, q2, q4
[^>]*> ff0e 8554 	vqrshl.u8	q4, q2, q7
[^>]*> ff00 8558 	vqrshl.u8	q4, q4, q0
[^>]*> ff02 8558 	vqrshl.u8	q4, q4, q1
[^>]*> ff04 8558 	vqrshl.u8	q4, q4, q2
[^>]*> ff08 8558 	vqrshl.u8	q4, q4, q4
[^>]*> ff0e 8558 	vqrshl.u8	q4, q4, q7
[^>]*> ff00 855e 	vqrshl.u8	q4, q7, q0
[^>]*> ff02 855e 	vqrshl.u8	q4, q7, q1
[^>]*> ff04 855e 	vqrshl.u8	q4, q7, q2
[^>]*> ff08 855e 	vqrshl.u8	q4, q7, q4
[^>]*> ff0e 855e 	vqrshl.u8	q4, q7, q7
[^>]*> fe33 9ee0 	vqrshl.u8	q4, r0
[^>]*> fe33 9ee1 	vqrshl.u8	q4, r1
[^>]*> fe33 9ee2 	vqrshl.u8	q4, r2
[^>]*> fe33 9ee4 	vqrshl.u8	q4, r4
[^>]*> fe33 9ee7 	vqrshl.u8	q4, r7
[^>]*> fe33 9ee8 	vqrshl.u8	q4, r8
[^>]*> fe33 9eea 	vqrshl.u8	q4, sl
[^>]*> fe33 9eec 	vqrshl.u8	q4, ip
[^>]*> fe33 9eee 	vqrshl.u8	q4, lr
[^>]*> ff00 e550 	vqrshl.u8	q7, q0, q0
[^>]*> ff02 e550 	vqrshl.u8	q7, q0, q1
[^>]*> ff04 e550 	vqrshl.u8	q7, q0, q2
[^>]*> ff08 e550 	vqrshl.u8	q7, q0, q4
[^>]*> ff0e e550 	vqrshl.u8	q7, q0, q7
[^>]*> ff00 e552 	vqrshl.u8	q7, q1, q0
[^>]*> ff02 e552 	vqrshl.u8	q7, q1, q1
[^>]*> ff04 e552 	vqrshl.u8	q7, q1, q2
[^>]*> ff08 e552 	vqrshl.u8	q7, q1, q4
[^>]*> ff0e e552 	vqrshl.u8	q7, q1, q7
[^>]*> ff00 e554 	vqrshl.u8	q7, q2, q0
[^>]*> ff02 e554 	vqrshl.u8	q7, q2, q1
[^>]*> ff04 e554 	vqrshl.u8	q7, q2, q2
[^>]*> ff08 e554 	vqrshl.u8	q7, q2, q4
[^>]*> ff0e e554 	vqrshl.u8	q7, q2, q7
[^>]*> ff00 e558 	vqrshl.u8	q7, q4, q0
[^>]*> ff02 e558 	vqrshl.u8	q7, q4, q1
[^>]*> ff04 e558 	vqrshl.u8	q7, q4, q2
[^>]*> ff08 e558 	vqrshl.u8	q7, q4, q4
[^>]*> ff0e e558 	vqrshl.u8	q7, q4, q7
[^>]*> ff00 e55e 	vqrshl.u8	q7, q7, q0
[^>]*> ff02 e55e 	vqrshl.u8	q7, q7, q1
[^>]*> ff04 e55e 	vqrshl.u8	q7, q7, q2
[^>]*> ff08 e55e 	vqrshl.u8	q7, q7, q4
[^>]*> ff0e e55e 	vqrshl.u8	q7, q7, q7
[^>]*> fe33 fee0 	vqrshl.u8	q7, r0
[^>]*> fe33 fee1 	vqrshl.u8	q7, r1
[^>]*> fe33 fee2 	vqrshl.u8	q7, r2
[^>]*> fe33 fee4 	vqrshl.u8	q7, r4
[^>]*> fe33 fee7 	vqrshl.u8	q7, r7
[^>]*> fe33 fee8 	vqrshl.u8	q7, r8
[^>]*> fe33 feea 	vqrshl.u8	q7, sl
[^>]*> fe33 feec 	vqrshl.u8	q7, ip
[^>]*> fe33 feee 	vqrshl.u8	q7, lr
[^>]*> ef10 0550 	vqrshl.s16	q0, q0, q0
[^>]*> ef12 0550 	vqrshl.s16	q0, q0, q1
[^>]*> ef14 0550 	vqrshl.s16	q0, q0, q2
[^>]*> ef18 0550 	vqrshl.s16	q0, q0, q4
[^>]*> ef1e 0550 	vqrshl.s16	q0, q0, q7
[^>]*> ef10 0552 	vqrshl.s16	q0, q1, q0
[^>]*> ef12 0552 	vqrshl.s16	q0, q1, q1
[^>]*> ef14 0552 	vqrshl.s16	q0, q1, q2
[^>]*> ef18 0552 	vqrshl.s16	q0, q1, q4
[^>]*> ef1e 0552 	vqrshl.s16	q0, q1, q7
[^>]*> ef10 0554 	vqrshl.s16	q0, q2, q0
[^>]*> ef12 0554 	vqrshl.s16	q0, q2, q1
[^>]*> ef14 0554 	vqrshl.s16	q0, q2, q2
[^>]*> ef18 0554 	vqrshl.s16	q0, q2, q4
[^>]*> ef1e 0554 	vqrshl.s16	q0, q2, q7
[^>]*> ef10 0558 	vqrshl.s16	q0, q4, q0
[^>]*> ef12 0558 	vqrshl.s16	q0, q4, q1
[^>]*> ef14 0558 	vqrshl.s16	q0, q4, q2
[^>]*> ef18 0558 	vqrshl.s16	q0, q4, q4
[^>]*> ef1e 0558 	vqrshl.s16	q0, q4, q7
[^>]*> ef10 055e 	vqrshl.s16	q0, q7, q0
[^>]*> ef12 055e 	vqrshl.s16	q0, q7, q1
[^>]*> ef14 055e 	vqrshl.s16	q0, q7, q2
[^>]*> ef18 055e 	vqrshl.s16	q0, q7, q4
[^>]*> ef1e 055e 	vqrshl.s16	q0, q7, q7
[^>]*> ee37 1ee0 	vqrshl.s16	q0, r0
[^>]*> ee37 1ee1 	vqrshl.s16	q0, r1
[^>]*> ee37 1ee2 	vqrshl.s16	q0, r2
[^>]*> ee37 1ee4 	vqrshl.s16	q0, r4
[^>]*> ee37 1ee7 	vqrshl.s16	q0, r7
[^>]*> ee37 1ee8 	vqrshl.s16	q0, r8
[^>]*> ee37 1eea 	vqrshl.s16	q0, sl
[^>]*> ee37 1eec 	vqrshl.s16	q0, ip
[^>]*> ee37 1eee 	vqrshl.s16	q0, lr
[^>]*> ef10 2550 	vqrshl.s16	q1, q0, q0
[^>]*> ef12 2550 	vqrshl.s16	q1, q0, q1
[^>]*> ef14 2550 	vqrshl.s16	q1, q0, q2
[^>]*> ef18 2550 	vqrshl.s16	q1, q0, q4
[^>]*> ef1e 2550 	vqrshl.s16	q1, q0, q7
[^>]*> ef10 2552 	vqrshl.s16	q1, q1, q0
[^>]*> ef12 2552 	vqrshl.s16	q1, q1, q1
[^>]*> ef14 2552 	vqrshl.s16	q1, q1, q2
[^>]*> ef18 2552 	vqrshl.s16	q1, q1, q4
[^>]*> ef1e 2552 	vqrshl.s16	q1, q1, q7
[^>]*> ef10 2554 	vqrshl.s16	q1, q2, q0
[^>]*> ef12 2554 	vqrshl.s16	q1, q2, q1
[^>]*> ef14 2554 	vqrshl.s16	q1, q2, q2
[^>]*> ef18 2554 	vqrshl.s16	q1, q2, q4
[^>]*> ef1e 2554 	vqrshl.s16	q1, q2, q7
[^>]*> ef10 2558 	vqrshl.s16	q1, q4, q0
[^>]*> ef12 2558 	vqrshl.s16	q1, q4, q1
[^>]*> ef14 2558 	vqrshl.s16	q1, q4, q2
[^>]*> ef18 2558 	vqrshl.s16	q1, q4, q4
[^>]*> ef1e 2558 	vqrshl.s16	q1, q4, q7
[^>]*> ef10 255e 	vqrshl.s16	q1, q7, q0
[^>]*> ef12 255e 	vqrshl.s16	q1, q7, q1
[^>]*> ef14 255e 	vqrshl.s16	q1, q7, q2
[^>]*> ef18 255e 	vqrshl.s16	q1, q7, q4
[^>]*> ef1e 255e 	vqrshl.s16	q1, q7, q7
[^>]*> ee37 3ee0 	vqrshl.s16	q1, r0
[^>]*> ee37 3ee1 	vqrshl.s16	q1, r1
[^>]*> ee37 3ee2 	vqrshl.s16	q1, r2
[^>]*> ee37 3ee4 	vqrshl.s16	q1, r4
[^>]*> ee37 3ee7 	vqrshl.s16	q1, r7
[^>]*> ee37 3ee8 	vqrshl.s16	q1, r8
[^>]*> ee37 3eea 	vqrshl.s16	q1, sl
[^>]*> ee37 3eec 	vqrshl.s16	q1, ip
[^>]*> ee37 3eee 	vqrshl.s16	q1, lr
[^>]*> ef10 4550 	vqrshl.s16	q2, q0, q0
[^>]*> ef12 4550 	vqrshl.s16	q2, q0, q1
[^>]*> ef14 4550 	vqrshl.s16	q2, q0, q2
[^>]*> ef18 4550 	vqrshl.s16	q2, q0, q4
[^>]*> ef1e 4550 	vqrshl.s16	q2, q0, q7
[^>]*> ef10 4552 	vqrshl.s16	q2, q1, q0
[^>]*> ef12 4552 	vqrshl.s16	q2, q1, q1
[^>]*> ef14 4552 	vqrshl.s16	q2, q1, q2
[^>]*> ef18 4552 	vqrshl.s16	q2, q1, q4
[^>]*> ef1e 4552 	vqrshl.s16	q2, q1, q7
[^>]*> ef10 4554 	vqrshl.s16	q2, q2, q0
[^>]*> ef12 4554 	vqrshl.s16	q2, q2, q1
[^>]*> ef14 4554 	vqrshl.s16	q2, q2, q2
[^>]*> ef18 4554 	vqrshl.s16	q2, q2, q4
[^>]*> ef1e 4554 	vqrshl.s16	q2, q2, q7
[^>]*> ef10 4558 	vqrshl.s16	q2, q4, q0
[^>]*> ef12 4558 	vqrshl.s16	q2, q4, q1
[^>]*> ef14 4558 	vqrshl.s16	q2, q4, q2
[^>]*> ef18 4558 	vqrshl.s16	q2, q4, q4
[^>]*> ef1e 4558 	vqrshl.s16	q2, q4, q7
[^>]*> ef10 455e 	vqrshl.s16	q2, q7, q0
[^>]*> ef12 455e 	vqrshl.s16	q2, q7, q1
[^>]*> ef14 455e 	vqrshl.s16	q2, q7, q2
[^>]*> ef18 455e 	vqrshl.s16	q2, q7, q4
[^>]*> ef1e 455e 	vqrshl.s16	q2, q7, q7
[^>]*> ee37 5ee0 	vqrshl.s16	q2, r0
[^>]*> ee37 5ee1 	vqrshl.s16	q2, r1
[^>]*> ee37 5ee2 	vqrshl.s16	q2, r2
[^>]*> ee37 5ee4 	vqrshl.s16	q2, r4
[^>]*> ee37 5ee7 	vqrshl.s16	q2, r7
[^>]*> ee37 5ee8 	vqrshl.s16	q2, r8
[^>]*> ee37 5eea 	vqrshl.s16	q2, sl
[^>]*> ee37 5eec 	vqrshl.s16	q2, ip
[^>]*> ee37 5eee 	vqrshl.s16	q2, lr
[^>]*> ef10 8550 	vqrshl.s16	q4, q0, q0
[^>]*> ef12 8550 	vqrshl.s16	q4, q0, q1
[^>]*> ef14 8550 	vqrshl.s16	q4, q0, q2
[^>]*> ef18 8550 	vqrshl.s16	q4, q0, q4
[^>]*> ef1e 8550 	vqrshl.s16	q4, q0, q7
[^>]*> ef10 8552 	vqrshl.s16	q4, q1, q0
[^>]*> ef12 8552 	vqrshl.s16	q4, q1, q1
[^>]*> ef14 8552 	vqrshl.s16	q4, q1, q2
[^>]*> ef18 8552 	vqrshl.s16	q4, q1, q4
[^>]*> ef1e 8552 	vqrshl.s16	q4, q1, q7
[^>]*> ef10 8554 	vqrshl.s16	q4, q2, q0
[^>]*> ef12 8554 	vqrshl.s16	q4, q2, q1
[^>]*> ef14 8554 	vqrshl.s16	q4, q2, q2
[^>]*> ef18 8554 	vqrshl.s16	q4, q2, q4
[^>]*> ef1e 8554 	vqrshl.s16	q4, q2, q7
[^>]*> ef10 8558 	vqrshl.s16	q4, q4, q0
[^>]*> ef12 8558 	vqrshl.s16	q4, q4, q1
[^>]*> ef14 8558 	vqrshl.s16	q4, q4, q2
[^>]*> ef18 8558 	vqrshl.s16	q4, q4, q4
[^>]*> ef1e 8558 	vqrshl.s16	q4, q4, q7
[^>]*> ef10 855e 	vqrshl.s16	q4, q7, q0
[^>]*> ef12 855e 	vqrshl.s16	q4, q7, q1
[^>]*> ef14 855e 	vqrshl.s16	q4, q7, q2
[^>]*> ef18 855e 	vqrshl.s16	q4, q7, q4
[^>]*> ef1e 855e 	vqrshl.s16	q4, q7, q7
[^>]*> ee37 9ee0 	vqrshl.s16	q4, r0
[^>]*> ee37 9ee1 	vqrshl.s16	q4, r1
[^>]*> ee37 9ee2 	vqrshl.s16	q4, r2
[^>]*> ee37 9ee4 	vqrshl.s16	q4, r4
[^>]*> ee37 9ee7 	vqrshl.s16	q4, r7
[^>]*> ee37 9ee8 	vqrshl.s16	q4, r8
[^>]*> ee37 9eea 	vqrshl.s16	q4, sl
[^>]*> ee37 9eec 	vqrshl.s16	q4, ip
[^>]*> ee37 9eee 	vqrshl.s16	q4, lr
[^>]*> ef10 e550 	vqrshl.s16	q7, q0, q0
[^>]*> ef12 e550 	vqrshl.s16	q7, q0, q1
[^>]*> ef14 e550 	vqrshl.s16	q7, q0, q2
[^>]*> ef18 e550 	vqrshl.s16	q7, q0, q4
[^>]*> ef1e e550 	vqrshl.s16	q7, q0, q7
[^>]*> ef10 e552 	vqrshl.s16	q7, q1, q0
[^>]*> ef12 e552 	vqrshl.s16	q7, q1, q1
[^>]*> ef14 e552 	vqrshl.s16	q7, q1, q2
[^>]*> ef18 e552 	vqrshl.s16	q7, q1, q4
[^>]*> ef1e e552 	vqrshl.s16	q7, q1, q7
[^>]*> ef10 e554 	vqrshl.s16	q7, q2, q0
[^>]*> ef12 e554 	vqrshl.s16	q7, q2, q1
[^>]*> ef14 e554 	vqrshl.s16	q7, q2, q2
[^>]*> ef18 e554 	vqrshl.s16	q7, q2, q4
[^>]*> ef1e e554 	vqrshl.s16	q7, q2, q7
[^>]*> ef10 e558 	vqrshl.s16	q7, q4, q0
[^>]*> ef12 e558 	vqrshl.s16	q7, q4, q1
[^>]*> ef14 e558 	vqrshl.s16	q7, q4, q2
[^>]*> ef18 e558 	vqrshl.s16	q7, q4, q4
[^>]*> ef1e e558 	vqrshl.s16	q7, q4, q7
[^>]*> ef10 e55e 	vqrshl.s16	q7, q7, q0
[^>]*> ef12 e55e 	vqrshl.s16	q7, q7, q1
[^>]*> ef14 e55e 	vqrshl.s16	q7, q7, q2
[^>]*> ef18 e55e 	vqrshl.s16	q7, q7, q4
[^>]*> ef1e e55e 	vqrshl.s16	q7, q7, q7
[^>]*> ee37 fee0 	vqrshl.s16	q7, r0
[^>]*> ee37 fee1 	vqrshl.s16	q7, r1
[^>]*> ee37 fee2 	vqrshl.s16	q7, r2
[^>]*> ee37 fee4 	vqrshl.s16	q7, r4
[^>]*> ee37 fee7 	vqrshl.s16	q7, r7
[^>]*> ee37 fee8 	vqrshl.s16	q7, r8
[^>]*> ee37 feea 	vqrshl.s16	q7, sl
[^>]*> ee37 feec 	vqrshl.s16	q7, ip
[^>]*> ee37 feee 	vqrshl.s16	q7, lr
[^>]*> ff10 0550 	vqrshl.u16	q0, q0, q0
[^>]*> ff12 0550 	vqrshl.u16	q0, q0, q1
[^>]*> ff14 0550 	vqrshl.u16	q0, q0, q2
[^>]*> ff18 0550 	vqrshl.u16	q0, q0, q4
[^>]*> ff1e 0550 	vqrshl.u16	q0, q0, q7
[^>]*> ff10 0552 	vqrshl.u16	q0, q1, q0
[^>]*> ff12 0552 	vqrshl.u16	q0, q1, q1
[^>]*> ff14 0552 	vqrshl.u16	q0, q1, q2
[^>]*> ff18 0552 	vqrshl.u16	q0, q1, q4
[^>]*> ff1e 0552 	vqrshl.u16	q0, q1, q7
[^>]*> ff10 0554 	vqrshl.u16	q0, q2, q0
[^>]*> ff12 0554 	vqrshl.u16	q0, q2, q1
[^>]*> ff14 0554 	vqrshl.u16	q0, q2, q2
[^>]*> ff18 0554 	vqrshl.u16	q0, q2, q4
[^>]*> ff1e 0554 	vqrshl.u16	q0, q2, q7
[^>]*> ff10 0558 	vqrshl.u16	q0, q4, q0
[^>]*> ff12 0558 	vqrshl.u16	q0, q4, q1
[^>]*> ff14 0558 	vqrshl.u16	q0, q4, q2
[^>]*> ff18 0558 	vqrshl.u16	q0, q4, q4
[^>]*> ff1e 0558 	vqrshl.u16	q0, q4, q7
[^>]*> ff10 055e 	vqrshl.u16	q0, q7, q0
[^>]*> ff12 055e 	vqrshl.u16	q0, q7, q1
[^>]*> ff14 055e 	vqrshl.u16	q0, q7, q2
[^>]*> ff18 055e 	vqrshl.u16	q0, q7, q4
[^>]*> ff1e 055e 	vqrshl.u16	q0, q7, q7
[^>]*> fe37 1ee0 	vqrshl.u16	q0, r0
[^>]*> fe37 1ee1 	vqrshl.u16	q0, r1
[^>]*> fe37 1ee2 	vqrshl.u16	q0, r2
[^>]*> fe37 1ee4 	vqrshl.u16	q0, r4
[^>]*> fe37 1ee7 	vqrshl.u16	q0, r7
[^>]*> fe37 1ee8 	vqrshl.u16	q0, r8
[^>]*> fe37 1eea 	vqrshl.u16	q0, sl
[^>]*> fe37 1eec 	vqrshl.u16	q0, ip
[^>]*> fe37 1eee 	vqrshl.u16	q0, lr
[^>]*> ff10 2550 	vqrshl.u16	q1, q0, q0
[^>]*> ff12 2550 	vqrshl.u16	q1, q0, q1
[^>]*> ff14 2550 	vqrshl.u16	q1, q0, q2
[^>]*> ff18 2550 	vqrshl.u16	q1, q0, q4
[^>]*> ff1e 2550 	vqrshl.u16	q1, q0, q7
[^>]*> ff10 2552 	vqrshl.u16	q1, q1, q0
[^>]*> ff12 2552 	vqrshl.u16	q1, q1, q1
[^>]*> ff14 2552 	vqrshl.u16	q1, q1, q2
[^>]*> ff18 2552 	vqrshl.u16	q1, q1, q4
[^>]*> ff1e 2552 	vqrshl.u16	q1, q1, q7
[^>]*> ff10 2554 	vqrshl.u16	q1, q2, q0
[^>]*> ff12 2554 	vqrshl.u16	q1, q2, q1
[^>]*> ff14 2554 	vqrshl.u16	q1, q2, q2
[^>]*> ff18 2554 	vqrshl.u16	q1, q2, q4
[^>]*> ff1e 2554 	vqrshl.u16	q1, q2, q7
[^>]*> ff10 2558 	vqrshl.u16	q1, q4, q0
[^>]*> ff12 2558 	vqrshl.u16	q1, q4, q1
[^>]*> ff14 2558 	vqrshl.u16	q1, q4, q2
[^>]*> ff18 2558 	vqrshl.u16	q1, q4, q4
[^>]*> ff1e 2558 	vqrshl.u16	q1, q4, q7
[^>]*> ff10 255e 	vqrshl.u16	q1, q7, q0
[^>]*> ff12 255e 	vqrshl.u16	q1, q7, q1
[^>]*> ff14 255e 	vqrshl.u16	q1, q7, q2
[^>]*> ff18 255e 	vqrshl.u16	q1, q7, q4
[^>]*> ff1e 255e 	vqrshl.u16	q1, q7, q7
[^>]*> fe37 3ee0 	vqrshl.u16	q1, r0
[^>]*> fe37 3ee1 	vqrshl.u16	q1, r1
[^>]*> fe37 3ee2 	vqrshl.u16	q1, r2
[^>]*> fe37 3ee4 	vqrshl.u16	q1, r4
[^>]*> fe37 3ee7 	vqrshl.u16	q1, r7
[^>]*> fe37 3ee8 	vqrshl.u16	q1, r8
[^>]*> fe37 3eea 	vqrshl.u16	q1, sl
[^>]*> fe37 3eec 	vqrshl.u16	q1, ip
[^>]*> fe37 3eee 	vqrshl.u16	q1, lr
[^>]*> ff10 4550 	vqrshl.u16	q2, q0, q0
[^>]*> ff12 4550 	vqrshl.u16	q2, q0, q1
[^>]*> ff14 4550 	vqrshl.u16	q2, q0, q2
[^>]*> ff18 4550 	vqrshl.u16	q2, q0, q4
[^>]*> ff1e 4550 	vqrshl.u16	q2, q0, q7
[^>]*> ff10 4552 	vqrshl.u16	q2, q1, q0
[^>]*> ff12 4552 	vqrshl.u16	q2, q1, q1
[^>]*> ff14 4552 	vqrshl.u16	q2, q1, q2
[^>]*> ff18 4552 	vqrshl.u16	q2, q1, q4
[^>]*> ff1e 4552 	vqrshl.u16	q2, q1, q7
[^>]*> ff10 4554 	vqrshl.u16	q2, q2, q0
[^>]*> ff12 4554 	vqrshl.u16	q2, q2, q1
[^>]*> ff14 4554 	vqrshl.u16	q2, q2, q2
[^>]*> ff18 4554 	vqrshl.u16	q2, q2, q4
[^>]*> ff1e 4554 	vqrshl.u16	q2, q2, q7
[^>]*> ff10 4558 	vqrshl.u16	q2, q4, q0
[^>]*> ff12 4558 	vqrshl.u16	q2, q4, q1
[^>]*> ff14 4558 	vqrshl.u16	q2, q4, q2
[^>]*> ff18 4558 	vqrshl.u16	q2, q4, q4
[^>]*> ff1e 4558 	vqrshl.u16	q2, q4, q7
[^>]*> ff10 455e 	vqrshl.u16	q2, q7, q0
[^>]*> ff12 455e 	vqrshl.u16	q2, q7, q1
[^>]*> ff14 455e 	vqrshl.u16	q2, q7, q2
[^>]*> ff18 455e 	vqrshl.u16	q2, q7, q4
[^>]*> ff1e 455e 	vqrshl.u16	q2, q7, q7
[^>]*> fe37 5ee0 	vqrshl.u16	q2, r0
[^>]*> fe37 5ee1 	vqrshl.u16	q2, r1
[^>]*> fe37 5ee2 	vqrshl.u16	q2, r2
[^>]*> fe37 5ee4 	vqrshl.u16	q2, r4
[^>]*> fe37 5ee7 	vqrshl.u16	q2, r7
[^>]*> fe37 5ee8 	vqrshl.u16	q2, r8
[^>]*> fe37 5eea 	vqrshl.u16	q2, sl
[^>]*> fe37 5eec 	vqrshl.u16	q2, ip
[^>]*> fe37 5eee 	vqrshl.u16	q2, lr
[^>]*> ff10 8550 	vqrshl.u16	q4, q0, q0
[^>]*> ff12 8550 	vqrshl.u16	q4, q0, q1
[^>]*> ff14 8550 	vqrshl.u16	q4, q0, q2
[^>]*> ff18 8550 	vqrshl.u16	q4, q0, q4
[^>]*> ff1e 8550 	vqrshl.u16	q4, q0, q7
[^>]*> ff10 8552 	vqrshl.u16	q4, q1, q0
[^>]*> ff12 8552 	vqrshl.u16	q4, q1, q1
[^>]*> ff14 8552 	vqrshl.u16	q4, q1, q2
[^>]*> ff18 8552 	vqrshl.u16	q4, q1, q4
[^>]*> ff1e 8552 	vqrshl.u16	q4, q1, q7
[^>]*> ff10 8554 	vqrshl.u16	q4, q2, q0
[^>]*> ff12 8554 	vqrshl.u16	q4, q2, q1
[^>]*> ff14 8554 	vqrshl.u16	q4, q2, q2
[^>]*> ff18 8554 	vqrshl.u16	q4, q2, q4
[^>]*> ff1e 8554 	vqrshl.u16	q4, q2, q7
[^>]*> ff10 8558 	vqrshl.u16	q4, q4, q0
[^>]*> ff12 8558 	vqrshl.u16	q4, q4, q1
[^>]*> ff14 8558 	vqrshl.u16	q4, q4, q2
[^>]*> ff18 8558 	vqrshl.u16	q4, q4, q4
[^>]*> ff1e 8558 	vqrshl.u16	q4, q4, q7
[^>]*> ff10 855e 	vqrshl.u16	q4, q7, q0
[^>]*> ff12 855e 	vqrshl.u16	q4, q7, q1
[^>]*> ff14 855e 	vqrshl.u16	q4, q7, q2
[^>]*> ff18 855e 	vqrshl.u16	q4, q7, q4
[^>]*> ff1e 855e 	vqrshl.u16	q4, q7, q7
[^>]*> fe37 9ee0 	vqrshl.u16	q4, r0
[^>]*> fe37 9ee1 	vqrshl.u16	q4, r1
[^>]*> fe37 9ee2 	vqrshl.u16	q4, r2
[^>]*> fe37 9ee4 	vqrshl.u16	q4, r4
[^>]*> fe37 9ee7 	vqrshl.u16	q4, r7
[^>]*> fe37 9ee8 	vqrshl.u16	q4, r8
[^>]*> fe37 9eea 	vqrshl.u16	q4, sl
[^>]*> fe37 9eec 	vqrshl.u16	q4, ip
[^>]*> fe37 9eee 	vqrshl.u16	q4, lr
[^>]*> ff10 e550 	vqrshl.u16	q7, q0, q0
[^>]*> ff12 e550 	vqrshl.u16	q7, q0, q1
[^>]*> ff14 e550 	vqrshl.u16	q7, q0, q2
[^>]*> ff18 e550 	vqrshl.u16	q7, q0, q4
[^>]*> ff1e e550 	vqrshl.u16	q7, q0, q7
[^>]*> ff10 e552 	vqrshl.u16	q7, q1, q0
[^>]*> ff12 e552 	vqrshl.u16	q7, q1, q1
[^>]*> ff14 e552 	vqrshl.u16	q7, q1, q2
[^>]*> ff18 e552 	vqrshl.u16	q7, q1, q4
[^>]*> ff1e e552 	vqrshl.u16	q7, q1, q7
[^>]*> ff10 e554 	vqrshl.u16	q7, q2, q0
[^>]*> ff12 e554 	vqrshl.u16	q7, q2, q1
[^>]*> ff14 e554 	vqrshl.u16	q7, q2, q2
[^>]*> ff18 e554 	vqrshl.u16	q7, q2, q4
[^>]*> ff1e e554 	vqrshl.u16	q7, q2, q7
[^>]*> ff10 e558 	vqrshl.u16	q7, q4, q0
[^>]*> ff12 e558 	vqrshl.u16	q7, q4, q1
[^>]*> ff14 e558 	vqrshl.u16	q7, q4, q2
[^>]*> ff18 e558 	vqrshl.u16	q7, q4, q4
[^>]*> ff1e e558 	vqrshl.u16	q7, q4, q7
[^>]*> ff10 e55e 	vqrshl.u16	q7, q7, q0
[^>]*> ff12 e55e 	vqrshl.u16	q7, q7, q1
[^>]*> ff14 e55e 	vqrshl.u16	q7, q7, q2
[^>]*> ff18 e55e 	vqrshl.u16	q7, q7, q4
[^>]*> ff1e e55e 	vqrshl.u16	q7, q7, q7
[^>]*> fe37 fee0 	vqrshl.u16	q7, r0
[^>]*> fe37 fee1 	vqrshl.u16	q7, r1
[^>]*> fe37 fee2 	vqrshl.u16	q7, r2
[^>]*> fe37 fee4 	vqrshl.u16	q7, r4
[^>]*> fe37 fee7 	vqrshl.u16	q7, r7
[^>]*> fe37 fee8 	vqrshl.u16	q7, r8
[^>]*> fe37 feea 	vqrshl.u16	q7, sl
[^>]*> fe37 feec 	vqrshl.u16	q7, ip
[^>]*> fe37 feee 	vqrshl.u16	q7, lr
[^>]*> ef20 0550 	vqrshl.s32	q0, q0, q0
[^>]*> ef22 0550 	vqrshl.s32	q0, q0, q1
[^>]*> ef24 0550 	vqrshl.s32	q0, q0, q2
[^>]*> ef28 0550 	vqrshl.s32	q0, q0, q4
[^>]*> ef2e 0550 	vqrshl.s32	q0, q0, q7
[^>]*> ef20 0552 	vqrshl.s32	q0, q1, q0
[^>]*> ef22 0552 	vqrshl.s32	q0, q1, q1
[^>]*> ef24 0552 	vqrshl.s32	q0, q1, q2
[^>]*> ef28 0552 	vqrshl.s32	q0, q1, q4
[^>]*> ef2e 0552 	vqrshl.s32	q0, q1, q7
[^>]*> ef20 0554 	vqrshl.s32	q0, q2, q0
[^>]*> ef22 0554 	vqrshl.s32	q0, q2, q1
[^>]*> ef24 0554 	vqrshl.s32	q0, q2, q2
[^>]*> ef28 0554 	vqrshl.s32	q0, q2, q4
[^>]*> ef2e 0554 	vqrshl.s32	q0, q2, q7
[^>]*> ef20 0558 	vqrshl.s32	q0, q4, q0
[^>]*> ef22 0558 	vqrshl.s32	q0, q4, q1
[^>]*> ef24 0558 	vqrshl.s32	q0, q4, q2
[^>]*> ef28 0558 	vqrshl.s32	q0, q4, q4
[^>]*> ef2e 0558 	vqrshl.s32	q0, q4, q7
[^>]*> ef20 055e 	vqrshl.s32	q0, q7, q0
[^>]*> ef22 055e 	vqrshl.s32	q0, q7, q1
[^>]*> ef24 055e 	vqrshl.s32	q0, q7, q2
[^>]*> ef28 055e 	vqrshl.s32	q0, q7, q4
[^>]*> ef2e 055e 	vqrshl.s32	q0, q7, q7
[^>]*> ee3b 1ee0 	vqrshl.s32	q0, r0
[^>]*> ee3b 1ee1 	vqrshl.s32	q0, r1
[^>]*> ee3b 1ee2 	vqrshl.s32	q0, r2
[^>]*> ee3b 1ee4 	vqrshl.s32	q0, r4
[^>]*> ee3b 1ee7 	vqrshl.s32	q0, r7
[^>]*> ee3b 1ee8 	vqrshl.s32	q0, r8
[^>]*> ee3b 1eea 	vqrshl.s32	q0, sl
[^>]*> ee3b 1eec 	vqrshl.s32	q0, ip
[^>]*> ee3b 1eee 	vqrshl.s32	q0, lr
[^>]*> ef20 2550 	vqrshl.s32	q1, q0, q0
[^>]*> ef22 2550 	vqrshl.s32	q1, q0, q1
[^>]*> ef24 2550 	vqrshl.s32	q1, q0, q2
[^>]*> ef28 2550 	vqrshl.s32	q1, q0, q4
[^>]*> ef2e 2550 	vqrshl.s32	q1, q0, q7
[^>]*> ef20 2552 	vqrshl.s32	q1, q1, q0
[^>]*> ef22 2552 	vqrshl.s32	q1, q1, q1
[^>]*> ef24 2552 	vqrshl.s32	q1, q1, q2
[^>]*> ef28 2552 	vqrshl.s32	q1, q1, q4
[^>]*> ef2e 2552 	vqrshl.s32	q1, q1, q7
[^>]*> ef20 2554 	vqrshl.s32	q1, q2, q0
[^>]*> ef22 2554 	vqrshl.s32	q1, q2, q1
[^>]*> ef24 2554 	vqrshl.s32	q1, q2, q2
[^>]*> ef28 2554 	vqrshl.s32	q1, q2, q4
[^>]*> ef2e 2554 	vqrshl.s32	q1, q2, q7
[^>]*> ef20 2558 	vqrshl.s32	q1, q4, q0
[^>]*> ef22 2558 	vqrshl.s32	q1, q4, q1
[^>]*> ef24 2558 	vqrshl.s32	q1, q4, q2
[^>]*> ef28 2558 	vqrshl.s32	q1, q4, q4
[^>]*> ef2e 2558 	vqrshl.s32	q1, q4, q7
[^>]*> ef20 255e 	vqrshl.s32	q1, q7, q0
[^>]*> ef22 255e 	vqrshl.s32	q1, q7, q1
[^>]*> ef24 255e 	vqrshl.s32	q1, q7, q2
[^>]*> ef28 255e 	vqrshl.s32	q1, q7, q4
[^>]*> ef2e 255e 	vqrshl.s32	q1, q7, q7
[^>]*> ee3b 3ee0 	vqrshl.s32	q1, r0
[^>]*> ee3b 3ee1 	vqrshl.s32	q1, r1
[^>]*> ee3b 3ee2 	vqrshl.s32	q1, r2
[^>]*> ee3b 3ee4 	vqrshl.s32	q1, r4
[^>]*> ee3b 3ee7 	vqrshl.s32	q1, r7
[^>]*> ee3b 3ee8 	vqrshl.s32	q1, r8
[^>]*> ee3b 3eea 	vqrshl.s32	q1, sl
[^>]*> ee3b 3eec 	vqrshl.s32	q1, ip
[^>]*> ee3b 3eee 	vqrshl.s32	q1, lr
[^>]*> ef20 4550 	vqrshl.s32	q2, q0, q0
[^>]*> ef22 4550 	vqrshl.s32	q2, q0, q1
[^>]*> ef24 4550 	vqrshl.s32	q2, q0, q2
[^>]*> ef28 4550 	vqrshl.s32	q2, q0, q4
[^>]*> ef2e 4550 	vqrshl.s32	q2, q0, q7
[^>]*> ef20 4552 	vqrshl.s32	q2, q1, q0
[^>]*> ef22 4552 	vqrshl.s32	q2, q1, q1
[^>]*> ef24 4552 	vqrshl.s32	q2, q1, q2
[^>]*> ef28 4552 	vqrshl.s32	q2, q1, q4
[^>]*> ef2e 4552 	vqrshl.s32	q2, q1, q7
[^>]*> ef20 4554 	vqrshl.s32	q2, q2, q0
[^>]*> ef22 4554 	vqrshl.s32	q2, q2, q1
[^>]*> ef24 4554 	vqrshl.s32	q2, q2, q2
[^>]*> ef28 4554 	vqrshl.s32	q2, q2, q4
[^>]*> ef2e 4554 	vqrshl.s32	q2, q2, q7
[^>]*> ef20 4558 	vqrshl.s32	q2, q4, q0
[^>]*> ef22 4558 	vqrshl.s32	q2, q4, q1
[^>]*> ef24 4558 	vqrshl.s32	q2, q4, q2
[^>]*> ef28 4558 	vqrshl.s32	q2, q4, q4
[^>]*> ef2e 4558 	vqrshl.s32	q2, q4, q7
[^>]*> ef20 455e 	vqrshl.s32	q2, q7, q0
[^>]*> ef22 455e 	vqrshl.s32	q2, q7, q1
[^>]*> ef24 455e 	vqrshl.s32	q2, q7, q2
[^>]*> ef28 455e 	vqrshl.s32	q2, q7, q4
[^>]*> ef2e 455e 	vqrshl.s32	q2, q7, q7
[^>]*> ee3b 5ee0 	vqrshl.s32	q2, r0
[^>]*> ee3b 5ee1 	vqrshl.s32	q2, r1
[^>]*> ee3b 5ee2 	vqrshl.s32	q2, r2
[^>]*> ee3b 5ee4 	vqrshl.s32	q2, r4
[^>]*> ee3b 5ee7 	vqrshl.s32	q2, r7
[^>]*> ee3b 5ee8 	vqrshl.s32	q2, r8
[^>]*> ee3b 5eea 	vqrshl.s32	q2, sl
[^>]*> ee3b 5eec 	vqrshl.s32	q2, ip
[^>]*> ee3b 5eee 	vqrshl.s32	q2, lr
[^>]*> ef20 8550 	vqrshl.s32	q4, q0, q0
[^>]*> ef22 8550 	vqrshl.s32	q4, q0, q1
[^>]*> ef24 8550 	vqrshl.s32	q4, q0, q2
[^>]*> ef28 8550 	vqrshl.s32	q4, q0, q4
[^>]*> ef2e 8550 	vqrshl.s32	q4, q0, q7
[^>]*> ef20 8552 	vqrshl.s32	q4, q1, q0
[^>]*> ef22 8552 	vqrshl.s32	q4, q1, q1
[^>]*> ef24 8552 	vqrshl.s32	q4, q1, q2
[^>]*> ef28 8552 	vqrshl.s32	q4, q1, q4
[^>]*> ef2e 8552 	vqrshl.s32	q4, q1, q7
[^>]*> ef20 8554 	vqrshl.s32	q4, q2, q0
[^>]*> ef22 8554 	vqrshl.s32	q4, q2, q1
[^>]*> ef24 8554 	vqrshl.s32	q4, q2, q2
[^>]*> ef28 8554 	vqrshl.s32	q4, q2, q4
[^>]*> ef2e 8554 	vqrshl.s32	q4, q2, q7
[^>]*> ef20 8558 	vqrshl.s32	q4, q4, q0
[^>]*> ef22 8558 	vqrshl.s32	q4, q4, q1
[^>]*> ef24 8558 	vqrshl.s32	q4, q4, q2
[^>]*> ef28 8558 	vqrshl.s32	q4, q4, q4
[^>]*> ef2e 8558 	vqrshl.s32	q4, q4, q7
[^>]*> ef20 855e 	vqrshl.s32	q4, q7, q0
[^>]*> ef22 855e 	vqrshl.s32	q4, q7, q1
[^>]*> ef24 855e 	vqrshl.s32	q4, q7, q2
[^>]*> ef28 855e 	vqrshl.s32	q4, q7, q4
[^>]*> ef2e 855e 	vqrshl.s32	q4, q7, q7
[^>]*> ee3b 9ee0 	vqrshl.s32	q4, r0
[^>]*> ee3b 9ee1 	vqrshl.s32	q4, r1
[^>]*> ee3b 9ee2 	vqrshl.s32	q4, r2
[^>]*> ee3b 9ee4 	vqrshl.s32	q4, r4
[^>]*> ee3b 9ee7 	vqrshl.s32	q4, r7
[^>]*> ee3b 9ee8 	vqrshl.s32	q4, r8
[^>]*> ee3b 9eea 	vqrshl.s32	q4, sl
[^>]*> ee3b 9eec 	vqrshl.s32	q4, ip
[^>]*> ee3b 9eee 	vqrshl.s32	q4, lr
[^>]*> ef20 e550 	vqrshl.s32	q7, q0, q0
[^>]*> ef22 e550 	vqrshl.s32	q7, q0, q1
[^>]*> ef24 e550 	vqrshl.s32	q7, q0, q2
[^>]*> ef28 e550 	vqrshl.s32	q7, q0, q4
[^>]*> ef2e e550 	vqrshl.s32	q7, q0, q7
[^>]*> ef20 e552 	vqrshl.s32	q7, q1, q0
[^>]*> ef22 e552 	vqrshl.s32	q7, q1, q1
[^>]*> ef24 e552 	vqrshl.s32	q7, q1, q2
[^>]*> ef28 e552 	vqrshl.s32	q7, q1, q4
[^>]*> ef2e e552 	vqrshl.s32	q7, q1, q7
[^>]*> ef20 e554 	vqrshl.s32	q7, q2, q0
[^>]*> ef22 e554 	vqrshl.s32	q7, q2, q1
[^>]*> ef24 e554 	vqrshl.s32	q7, q2, q2
[^>]*> ef28 e554 	vqrshl.s32	q7, q2, q4
[^>]*> ef2e e554 	vqrshl.s32	q7, q2, q7
[^>]*> ef20 e558 	vqrshl.s32	q7, q4, q0
[^>]*> ef22 e558 	vqrshl.s32	q7, q4, q1
[^>]*> ef24 e558 	vqrshl.s32	q7, q4, q2
[^>]*> ef28 e558 	vqrshl.s32	q7, q4, q4
[^>]*> ef2e e558 	vqrshl.s32	q7, q4, q7
[^>]*> ef20 e55e 	vqrshl.s32	q7, q7, q0
[^>]*> ef22 e55e 	vqrshl.s32	q7, q7, q1
[^>]*> ef24 e55e 	vqrshl.s32	q7, q7, q2
[^>]*> ef28 e55e 	vqrshl.s32	q7, q7, q4
[^>]*> ef2e e55e 	vqrshl.s32	q7, q7, q7
[^>]*> ee3b fee0 	vqrshl.s32	q7, r0
[^>]*> ee3b fee1 	vqrshl.s32	q7, r1
[^>]*> ee3b fee2 	vqrshl.s32	q7, r2
[^>]*> ee3b fee4 	vqrshl.s32	q7, r4
[^>]*> ee3b fee7 	vqrshl.s32	q7, r7
[^>]*> ee3b fee8 	vqrshl.s32	q7, r8
[^>]*> ee3b feea 	vqrshl.s32	q7, sl
[^>]*> ee3b feec 	vqrshl.s32	q7, ip
[^>]*> ee3b feee 	vqrshl.s32	q7, lr
[^>]*> ff20 0550 	vqrshl.u32	q0, q0, q0
[^>]*> ff22 0550 	vqrshl.u32	q0, q0, q1
[^>]*> ff24 0550 	vqrshl.u32	q0, q0, q2
[^>]*> ff28 0550 	vqrshl.u32	q0, q0, q4
[^>]*> ff2e 0550 	vqrshl.u32	q0, q0, q7
[^>]*> ff20 0552 	vqrshl.u32	q0, q1, q0
[^>]*> ff22 0552 	vqrshl.u32	q0, q1, q1
[^>]*> ff24 0552 	vqrshl.u32	q0, q1, q2
[^>]*> ff28 0552 	vqrshl.u32	q0, q1, q4
[^>]*> ff2e 0552 	vqrshl.u32	q0, q1, q7
[^>]*> ff20 0554 	vqrshl.u32	q0, q2, q0
[^>]*> ff22 0554 	vqrshl.u32	q0, q2, q1
[^>]*> ff24 0554 	vqrshl.u32	q0, q2, q2
[^>]*> ff28 0554 	vqrshl.u32	q0, q2, q4
[^>]*> ff2e 0554 	vqrshl.u32	q0, q2, q7
[^>]*> ff20 0558 	vqrshl.u32	q0, q4, q0
[^>]*> ff22 0558 	vqrshl.u32	q0, q4, q1
[^>]*> ff24 0558 	vqrshl.u32	q0, q4, q2
[^>]*> ff28 0558 	vqrshl.u32	q0, q4, q4
[^>]*> ff2e 0558 	vqrshl.u32	q0, q4, q7
[^>]*> ff20 055e 	vqrshl.u32	q0, q7, q0
[^>]*> ff22 055e 	vqrshl.u32	q0, q7, q1
[^>]*> ff24 055e 	vqrshl.u32	q0, q7, q2
[^>]*> ff28 055e 	vqrshl.u32	q0, q7, q4
[^>]*> ff2e 055e 	vqrshl.u32	q0, q7, q7
[^>]*> fe3b 1ee0 	vqrshl.u32	q0, r0
[^>]*> fe3b 1ee1 	vqrshl.u32	q0, r1
[^>]*> fe3b 1ee2 	vqrshl.u32	q0, r2
[^>]*> fe3b 1ee4 	vqrshl.u32	q0, r4
[^>]*> fe3b 1ee7 	vqrshl.u32	q0, r7
[^>]*> fe3b 1ee8 	vqrshl.u32	q0, r8
[^>]*> fe3b 1eea 	vqrshl.u32	q0, sl
[^>]*> fe3b 1eec 	vqrshl.u32	q0, ip
[^>]*> fe3b 1eee 	vqrshl.u32	q0, lr
[^>]*> ff20 2550 	vqrshl.u32	q1, q0, q0
[^>]*> ff22 2550 	vqrshl.u32	q1, q0, q1
[^>]*> ff24 2550 	vqrshl.u32	q1, q0, q2
[^>]*> ff28 2550 	vqrshl.u32	q1, q0, q4
[^>]*> ff2e 2550 	vqrshl.u32	q1, q0, q7
[^>]*> ff20 2552 	vqrshl.u32	q1, q1, q0
[^>]*> ff22 2552 	vqrshl.u32	q1, q1, q1
[^>]*> ff24 2552 	vqrshl.u32	q1, q1, q2
[^>]*> ff28 2552 	vqrshl.u32	q1, q1, q4
[^>]*> ff2e 2552 	vqrshl.u32	q1, q1, q7
[^>]*> ff20 2554 	vqrshl.u32	q1, q2, q0
[^>]*> ff22 2554 	vqrshl.u32	q1, q2, q1
[^>]*> ff24 2554 	vqrshl.u32	q1, q2, q2
[^>]*> ff28 2554 	vqrshl.u32	q1, q2, q4
[^>]*> ff2e 2554 	vqrshl.u32	q1, q2, q7
[^>]*> ff20 2558 	vqrshl.u32	q1, q4, q0
[^>]*> ff22 2558 	vqrshl.u32	q1, q4, q1
[^>]*> ff24 2558 	vqrshl.u32	q1, q4, q2
[^>]*> ff28 2558 	vqrshl.u32	q1, q4, q4
[^>]*> ff2e 2558 	vqrshl.u32	q1, q4, q7
[^>]*> ff20 255e 	vqrshl.u32	q1, q7, q0
[^>]*> ff22 255e 	vqrshl.u32	q1, q7, q1
[^>]*> ff24 255e 	vqrshl.u32	q1, q7, q2
[^>]*> ff28 255e 	vqrshl.u32	q1, q7, q4
[^>]*> ff2e 255e 	vqrshl.u32	q1, q7, q7
[^>]*> fe3b 3ee0 	vqrshl.u32	q1, r0
[^>]*> fe3b 3ee1 	vqrshl.u32	q1, r1
[^>]*> fe3b 3ee2 	vqrshl.u32	q1, r2
[^>]*> fe3b 3ee4 	vqrshl.u32	q1, r4
[^>]*> fe3b 3ee7 	vqrshl.u32	q1, r7
[^>]*> fe3b 3ee8 	vqrshl.u32	q1, r8
[^>]*> fe3b 3eea 	vqrshl.u32	q1, sl
[^>]*> fe3b 3eec 	vqrshl.u32	q1, ip
[^>]*> fe3b 3eee 	vqrshl.u32	q1, lr
[^>]*> ff20 4550 	vqrshl.u32	q2, q0, q0
[^>]*> ff22 4550 	vqrshl.u32	q2, q0, q1
[^>]*> ff24 4550 	vqrshl.u32	q2, q0, q2
[^>]*> ff28 4550 	vqrshl.u32	q2, q0, q4
[^>]*> ff2e 4550 	vqrshl.u32	q2, q0, q7
[^>]*> ff20 4552 	vqrshl.u32	q2, q1, q0
[^>]*> ff22 4552 	vqrshl.u32	q2, q1, q1
[^>]*> ff24 4552 	vqrshl.u32	q2, q1, q2
[^>]*> ff28 4552 	vqrshl.u32	q2, q1, q4
[^>]*> ff2e 4552 	vqrshl.u32	q2, q1, q7
[^>]*> ff20 4554 	vqrshl.u32	q2, q2, q0
[^>]*> ff22 4554 	vqrshl.u32	q2, q2, q1
[^>]*> ff24 4554 	vqrshl.u32	q2, q2, q2
[^>]*> ff28 4554 	vqrshl.u32	q2, q2, q4
[^>]*> ff2e 4554 	vqrshl.u32	q2, q2, q7
[^>]*> ff20 4558 	vqrshl.u32	q2, q4, q0
[^>]*> ff22 4558 	vqrshl.u32	q2, q4, q1
[^>]*> ff24 4558 	vqrshl.u32	q2, q4, q2
[^>]*> ff28 4558 	vqrshl.u32	q2, q4, q4
[^>]*> ff2e 4558 	vqrshl.u32	q2, q4, q7
[^>]*> ff20 455e 	vqrshl.u32	q2, q7, q0
[^>]*> ff22 455e 	vqrshl.u32	q2, q7, q1
[^>]*> ff24 455e 	vqrshl.u32	q2, q7, q2
[^>]*> ff28 455e 	vqrshl.u32	q2, q7, q4
[^>]*> ff2e 455e 	vqrshl.u32	q2, q7, q7
[^>]*> fe3b 5ee0 	vqrshl.u32	q2, r0
[^>]*> fe3b 5ee1 	vqrshl.u32	q2, r1
[^>]*> fe3b 5ee2 	vqrshl.u32	q2, r2
[^>]*> fe3b 5ee4 	vqrshl.u32	q2, r4
[^>]*> fe3b 5ee7 	vqrshl.u32	q2, r7
[^>]*> fe3b 5ee8 	vqrshl.u32	q2, r8
[^>]*> fe3b 5eea 	vqrshl.u32	q2, sl
[^>]*> fe3b 5eec 	vqrshl.u32	q2, ip
[^>]*> fe3b 5eee 	vqrshl.u32	q2, lr
[^>]*> ff20 8550 	vqrshl.u32	q4, q0, q0
[^>]*> ff22 8550 	vqrshl.u32	q4, q0, q1
[^>]*> ff24 8550 	vqrshl.u32	q4, q0, q2
[^>]*> ff28 8550 	vqrshl.u32	q4, q0, q4
[^>]*> ff2e 8550 	vqrshl.u32	q4, q0, q7
[^>]*> ff20 8552 	vqrshl.u32	q4, q1, q0
[^>]*> ff22 8552 	vqrshl.u32	q4, q1, q1
[^>]*> ff24 8552 	vqrshl.u32	q4, q1, q2
[^>]*> ff28 8552 	vqrshl.u32	q4, q1, q4
[^>]*> ff2e 8552 	vqrshl.u32	q4, q1, q7
[^>]*> ff20 8554 	vqrshl.u32	q4, q2, q0
[^>]*> ff22 8554 	vqrshl.u32	q4, q2, q1
[^>]*> ff24 8554 	vqrshl.u32	q4, q2, q2
[^>]*> ff28 8554 	vqrshl.u32	q4, q2, q4
[^>]*> ff2e 8554 	vqrshl.u32	q4, q2, q7
[^>]*> ff20 8558 	vqrshl.u32	q4, q4, q0
[^>]*> ff22 8558 	vqrshl.u32	q4, q4, q1
[^>]*> ff24 8558 	vqrshl.u32	q4, q4, q2
[^>]*> ff28 8558 	vqrshl.u32	q4, q4, q4
[^>]*> ff2e 8558 	vqrshl.u32	q4, q4, q7
[^>]*> ff20 855e 	vqrshl.u32	q4, q7, q0
[^>]*> ff22 855e 	vqrshl.u32	q4, q7, q1
[^>]*> ff24 855e 	vqrshl.u32	q4, q7, q2
[^>]*> ff28 855e 	vqrshl.u32	q4, q7, q4
[^>]*> ff2e 855e 	vqrshl.u32	q4, q7, q7
[^>]*> fe3b 9ee0 	vqrshl.u32	q4, r0
[^>]*> fe3b 9ee1 	vqrshl.u32	q4, r1
[^>]*> fe3b 9ee2 	vqrshl.u32	q4, r2
[^>]*> fe3b 9ee4 	vqrshl.u32	q4, r4
[^>]*> fe3b 9ee7 	vqrshl.u32	q4, r7
[^>]*> fe3b 9ee8 	vqrshl.u32	q4, r8
[^>]*> fe3b 9eea 	vqrshl.u32	q4, sl
[^>]*> fe3b 9eec 	vqrshl.u32	q4, ip
[^>]*> fe3b 9eee 	vqrshl.u32	q4, lr
[^>]*> ff20 e550 	vqrshl.u32	q7, q0, q0
[^>]*> ff22 e550 	vqrshl.u32	q7, q0, q1
[^>]*> ff24 e550 	vqrshl.u32	q7, q0, q2
[^>]*> ff28 e550 	vqrshl.u32	q7, q0, q4
[^>]*> ff2e e550 	vqrshl.u32	q7, q0, q7
[^>]*> ff20 e552 	vqrshl.u32	q7, q1, q0
[^>]*> ff22 e552 	vqrshl.u32	q7, q1, q1
[^>]*> ff24 e552 	vqrshl.u32	q7, q1, q2
[^>]*> ff28 e552 	vqrshl.u32	q7, q1, q4
[^>]*> ff2e e552 	vqrshl.u32	q7, q1, q7
[^>]*> ff20 e554 	vqrshl.u32	q7, q2, q0
[^>]*> ff22 e554 	vqrshl.u32	q7, q2, q1
[^>]*> ff24 e554 	vqrshl.u32	q7, q2, q2
[^>]*> ff28 e554 	vqrshl.u32	q7, q2, q4
[^>]*> ff2e e554 	vqrshl.u32	q7, q2, q7
[^>]*> ff20 e558 	vqrshl.u32	q7, q4, q0
[^>]*> ff22 e558 	vqrshl.u32	q7, q4, q1
[^>]*> ff24 e558 	vqrshl.u32	q7, q4, q2
[^>]*> ff28 e558 	vqrshl.u32	q7, q4, q4
[^>]*> ff2e e558 	vqrshl.u32	q7, q4, q7
[^>]*> ff20 e55e 	vqrshl.u32	q7, q7, q0
[^>]*> ff22 e55e 	vqrshl.u32	q7, q7, q1
[^>]*> ff24 e55e 	vqrshl.u32	q7, q7, q2
[^>]*> ff28 e55e 	vqrshl.u32	q7, q7, q4
[^>]*> ff2e e55e 	vqrshl.u32	q7, q7, q7
[^>]*> fe3b fee0 	vqrshl.u32	q7, r0
[^>]*> fe3b fee1 	vqrshl.u32	q7, r1
[^>]*> fe3b fee2 	vqrshl.u32	q7, r2
[^>]*> fe3b fee4 	vqrshl.u32	q7, r4
[^>]*> fe3b fee7 	vqrshl.u32	q7, r7
[^>]*> fe3b fee8 	vqrshl.u32	q7, r8
[^>]*> fe3b feea 	vqrshl.u32	q7, sl
[^>]*> fe3b feec 	vqrshl.u32	q7, ip
[^>]*> fe3b feee 	vqrshl.u32	q7, lr
[^>]*> fe71 ef4d 	vpstete
[^>]*> ef04 0552 	vqrshlt.s8	q0, q1, q2
[^>]*> ff1e e55e 	vqrshle.u16	q7, q7, q7
[^>]*> ee3b 1ee1 	vqrshlt.s32	q0, r1
[^>]*> fe33 feee 	vqrshle.u8	q7, lr
