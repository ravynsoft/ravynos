# name: MVE vshl instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ef88 0550 	vshl.i8	q0, q0, #0
[^>]*> ef89 0550 	vshl.i8	q0, q0, #1
[^>]*> ef8a 0550 	vshl.i8	q0, q0, #2
[^>]*> ef8c 0550 	vshl.i8	q0, q0, #4
[^>]*> ef8f 0550 	vshl.i8	q0, q0, #7
[^>]*> ef90 0550 	vshl.i16	q0, q0, #0
[^>]*> ef91 0550 	vshl.i16	q0, q0, #1
[^>]*> ef92 0550 	vshl.i16	q0, q0, #2
[^>]*> ef94 0550 	vshl.i16	q0, q0, #4
[^>]*> ef97 0550 	vshl.i16	q0, q0, #7
[^>]*> ef98 0550 	vshl.i16	q0, q0, #8
[^>]*> ef9a 0550 	vshl.i16	q0, q0, #10
[^>]*> ef9c 0550 	vshl.i16	q0, q0, #12
[^>]*> ef9f 0550 	vshl.i16	q0, q0, #15
[^>]*> efa0 0550 	vshl.i32	q0, q0, #0
[^>]*> efa1 0550 	vshl.i32	q0, q0, #1
[^>]*> efa2 0550 	vshl.i32	q0, q0, #2
[^>]*> efa4 0550 	vshl.i32	q0, q0, #4
[^>]*> efa7 0550 	vshl.i32	q0, q0, #7
[^>]*> efa8 0550 	vshl.i32	q0, q0, #8
[^>]*> efaa 0550 	vshl.i32	q0, q0, #10
[^>]*> efac 0550 	vshl.i32	q0, q0, #12
[^>]*> efaf 0550 	vshl.i32	q0, q0, #15
[^>]*> efb0 0550 	vshl.i32	q0, q0, #16
[^>]*> efb2 0550 	vshl.i32	q0, q0, #18
[^>]*> efb4 0550 	vshl.i32	q0, q0, #20
[^>]*> efb8 0550 	vshl.i32	q0, q0, #24
[^>]*> efbe 0550 	vshl.i32	q0, q0, #30
[^>]*> ef88 0552 	vshl.i8	q0, q1, #0
[^>]*> ef89 0552 	vshl.i8	q0, q1, #1
[^>]*> ef8a 0552 	vshl.i8	q0, q1, #2
[^>]*> ef8c 0552 	vshl.i8	q0, q1, #4
[^>]*> ef8f 0552 	vshl.i8	q0, q1, #7
[^>]*> ef90 0552 	vshl.i16	q0, q1, #0
[^>]*> ef91 0552 	vshl.i16	q0, q1, #1
[^>]*> ef92 0552 	vshl.i16	q0, q1, #2
[^>]*> ef94 0552 	vshl.i16	q0, q1, #4
[^>]*> ef97 0552 	vshl.i16	q0, q1, #7
[^>]*> ef98 0552 	vshl.i16	q0, q1, #8
[^>]*> ef9a 0552 	vshl.i16	q0, q1, #10
[^>]*> ef9c 0552 	vshl.i16	q0, q1, #12
[^>]*> ef9f 0552 	vshl.i16	q0, q1, #15
[^>]*> efa0 0552 	vshl.i32	q0, q1, #0
[^>]*> efa1 0552 	vshl.i32	q0, q1, #1
[^>]*> efa2 0552 	vshl.i32	q0, q1, #2
[^>]*> efa4 0552 	vshl.i32	q0, q1, #4
[^>]*> efa7 0552 	vshl.i32	q0, q1, #7
[^>]*> efa8 0552 	vshl.i32	q0, q1, #8
[^>]*> efaa 0552 	vshl.i32	q0, q1, #10
[^>]*> efac 0552 	vshl.i32	q0, q1, #12
[^>]*> efaf 0552 	vshl.i32	q0, q1, #15
[^>]*> efb0 0552 	vshl.i32	q0, q1, #16
[^>]*> efb2 0552 	vshl.i32	q0, q1, #18
[^>]*> efb4 0552 	vshl.i32	q0, q1, #20
[^>]*> efb8 0552 	vshl.i32	q0, q1, #24
[^>]*> efbe 0552 	vshl.i32	q0, q1, #30
[^>]*> ef88 0554 	vshl.i8	q0, q2, #0
[^>]*> ef89 0554 	vshl.i8	q0, q2, #1
[^>]*> ef8a 0554 	vshl.i8	q0, q2, #2
[^>]*> ef8c 0554 	vshl.i8	q0, q2, #4
[^>]*> ef8f 0554 	vshl.i8	q0, q2, #7
[^>]*> ef90 0554 	vshl.i16	q0, q2, #0
[^>]*> ef91 0554 	vshl.i16	q0, q2, #1
[^>]*> ef92 0554 	vshl.i16	q0, q2, #2
[^>]*> ef94 0554 	vshl.i16	q0, q2, #4
[^>]*> ef97 0554 	vshl.i16	q0, q2, #7
[^>]*> ef98 0554 	vshl.i16	q0, q2, #8
[^>]*> ef9a 0554 	vshl.i16	q0, q2, #10
[^>]*> ef9c 0554 	vshl.i16	q0, q2, #12
[^>]*> ef9f 0554 	vshl.i16	q0, q2, #15
[^>]*> efa0 0554 	vshl.i32	q0, q2, #0
[^>]*> efa1 0554 	vshl.i32	q0, q2, #1
[^>]*> efa2 0554 	vshl.i32	q0, q2, #2
[^>]*> efa4 0554 	vshl.i32	q0, q2, #4
[^>]*> efa7 0554 	vshl.i32	q0, q2, #7
[^>]*> efa8 0554 	vshl.i32	q0, q2, #8
[^>]*> efaa 0554 	vshl.i32	q0, q2, #10
[^>]*> efac 0554 	vshl.i32	q0, q2, #12
[^>]*> efaf 0554 	vshl.i32	q0, q2, #15
[^>]*> efb0 0554 	vshl.i32	q0, q2, #16
[^>]*> efb2 0554 	vshl.i32	q0, q2, #18
[^>]*> efb4 0554 	vshl.i32	q0, q2, #20
[^>]*> efb8 0554 	vshl.i32	q0, q2, #24
[^>]*> efbe 0554 	vshl.i32	q0, q2, #30
[^>]*> ef88 0558 	vshl.i8	q0, q4, #0
[^>]*> ef89 0558 	vshl.i8	q0, q4, #1
[^>]*> ef8a 0558 	vshl.i8	q0, q4, #2
[^>]*> ef8c 0558 	vshl.i8	q0, q4, #4
[^>]*> ef8f 0558 	vshl.i8	q0, q4, #7
[^>]*> ef90 0558 	vshl.i16	q0, q4, #0
[^>]*> ef91 0558 	vshl.i16	q0, q4, #1
[^>]*> ef92 0558 	vshl.i16	q0, q4, #2
[^>]*> ef94 0558 	vshl.i16	q0, q4, #4
[^>]*> ef97 0558 	vshl.i16	q0, q4, #7
[^>]*> ef98 0558 	vshl.i16	q0, q4, #8
[^>]*> ef9a 0558 	vshl.i16	q0, q4, #10
[^>]*> ef9c 0558 	vshl.i16	q0, q4, #12
[^>]*> ef9f 0558 	vshl.i16	q0, q4, #15
[^>]*> efa0 0558 	vshl.i32	q0, q4, #0
[^>]*> efa1 0558 	vshl.i32	q0, q4, #1
[^>]*> efa2 0558 	vshl.i32	q0, q4, #2
[^>]*> efa4 0558 	vshl.i32	q0, q4, #4
[^>]*> efa7 0558 	vshl.i32	q0, q4, #7
[^>]*> efa8 0558 	vshl.i32	q0, q4, #8
[^>]*> efaa 0558 	vshl.i32	q0, q4, #10
[^>]*> efac 0558 	vshl.i32	q0, q4, #12
[^>]*> efaf 0558 	vshl.i32	q0, q4, #15
[^>]*> efb0 0558 	vshl.i32	q0, q4, #16
[^>]*> efb2 0558 	vshl.i32	q0, q4, #18
[^>]*> efb4 0558 	vshl.i32	q0, q4, #20
[^>]*> efb8 0558 	vshl.i32	q0, q4, #24
[^>]*> efbe 0558 	vshl.i32	q0, q4, #30
[^>]*> ef88 055e 	vshl.i8	q0, q7, #0
[^>]*> ef89 055e 	vshl.i8	q0, q7, #1
[^>]*> ef8a 055e 	vshl.i8	q0, q7, #2
[^>]*> ef8c 055e 	vshl.i8	q0, q7, #4
[^>]*> ef8f 055e 	vshl.i8	q0, q7, #7
[^>]*> ef90 055e 	vshl.i16	q0, q7, #0
[^>]*> ef91 055e 	vshl.i16	q0, q7, #1
[^>]*> ef92 055e 	vshl.i16	q0, q7, #2
[^>]*> ef94 055e 	vshl.i16	q0, q7, #4
[^>]*> ef97 055e 	vshl.i16	q0, q7, #7
[^>]*> ef98 055e 	vshl.i16	q0, q7, #8
[^>]*> ef9a 055e 	vshl.i16	q0, q7, #10
[^>]*> ef9c 055e 	vshl.i16	q0, q7, #12
[^>]*> ef9f 055e 	vshl.i16	q0, q7, #15
[^>]*> efa0 055e 	vshl.i32	q0, q7, #0
[^>]*> efa1 055e 	vshl.i32	q0, q7, #1
[^>]*> efa2 055e 	vshl.i32	q0, q7, #2
[^>]*> efa4 055e 	vshl.i32	q0, q7, #4
[^>]*> efa7 055e 	vshl.i32	q0, q7, #7
[^>]*> efa8 055e 	vshl.i32	q0, q7, #8
[^>]*> efaa 055e 	vshl.i32	q0, q7, #10
[^>]*> efac 055e 	vshl.i32	q0, q7, #12
[^>]*> efaf 055e 	vshl.i32	q0, q7, #15
[^>]*> efb0 055e 	vshl.i32	q0, q7, #16
[^>]*> efb2 055e 	vshl.i32	q0, q7, #18
[^>]*> efb4 055e 	vshl.i32	q0, q7, #20
[^>]*> efb8 055e 	vshl.i32	q0, q7, #24
[^>]*> efbe 055e 	vshl.i32	q0, q7, #30
[^>]*> ef88 2550 	vshl.i8	q1, q0, #0
[^>]*> ef89 2550 	vshl.i8	q1, q0, #1
[^>]*> ef8a 2550 	vshl.i8	q1, q0, #2
[^>]*> ef8c 2550 	vshl.i8	q1, q0, #4
[^>]*> ef8f 2550 	vshl.i8	q1, q0, #7
[^>]*> ef90 2550 	vshl.i16	q1, q0, #0
[^>]*> ef91 2550 	vshl.i16	q1, q0, #1
[^>]*> ef92 2550 	vshl.i16	q1, q0, #2
[^>]*> ef94 2550 	vshl.i16	q1, q0, #4
[^>]*> ef97 2550 	vshl.i16	q1, q0, #7
[^>]*> ef98 2550 	vshl.i16	q1, q0, #8
[^>]*> ef9a 2550 	vshl.i16	q1, q0, #10
[^>]*> ef9c 2550 	vshl.i16	q1, q0, #12
[^>]*> ef9f 2550 	vshl.i16	q1, q0, #15
[^>]*> efa0 2550 	vshl.i32	q1, q0, #0
[^>]*> efa1 2550 	vshl.i32	q1, q0, #1
[^>]*> efa2 2550 	vshl.i32	q1, q0, #2
[^>]*> efa4 2550 	vshl.i32	q1, q0, #4
[^>]*> efa7 2550 	vshl.i32	q1, q0, #7
[^>]*> efa8 2550 	vshl.i32	q1, q0, #8
[^>]*> efaa 2550 	vshl.i32	q1, q0, #10
[^>]*> efac 2550 	vshl.i32	q1, q0, #12
[^>]*> efaf 2550 	vshl.i32	q1, q0, #15
[^>]*> efb0 2550 	vshl.i32	q1, q0, #16
[^>]*> efb2 2550 	vshl.i32	q1, q0, #18
[^>]*> efb4 2550 	vshl.i32	q1, q0, #20
[^>]*> efb8 2550 	vshl.i32	q1, q0, #24
[^>]*> efbe 2550 	vshl.i32	q1, q0, #30
[^>]*> ef88 2552 	vshl.i8	q1, q1, #0
[^>]*> ef89 2552 	vshl.i8	q1, q1, #1
[^>]*> ef8a 2552 	vshl.i8	q1, q1, #2
[^>]*> ef8c 2552 	vshl.i8	q1, q1, #4
[^>]*> ef8f 2552 	vshl.i8	q1, q1, #7
[^>]*> ef90 2552 	vshl.i16	q1, q1, #0
[^>]*> ef91 2552 	vshl.i16	q1, q1, #1
[^>]*> ef92 2552 	vshl.i16	q1, q1, #2
[^>]*> ef94 2552 	vshl.i16	q1, q1, #4
[^>]*> ef97 2552 	vshl.i16	q1, q1, #7
[^>]*> ef98 2552 	vshl.i16	q1, q1, #8
[^>]*> ef9a 2552 	vshl.i16	q1, q1, #10
[^>]*> ef9c 2552 	vshl.i16	q1, q1, #12
[^>]*> ef9f 2552 	vshl.i16	q1, q1, #15
[^>]*> efa0 2552 	vshl.i32	q1, q1, #0
[^>]*> efa1 2552 	vshl.i32	q1, q1, #1
[^>]*> efa2 2552 	vshl.i32	q1, q1, #2
[^>]*> efa4 2552 	vshl.i32	q1, q1, #4
[^>]*> efa7 2552 	vshl.i32	q1, q1, #7
[^>]*> efa8 2552 	vshl.i32	q1, q1, #8
[^>]*> efaa 2552 	vshl.i32	q1, q1, #10
[^>]*> efac 2552 	vshl.i32	q1, q1, #12
[^>]*> efaf 2552 	vshl.i32	q1, q1, #15
[^>]*> efb0 2552 	vshl.i32	q1, q1, #16
[^>]*> efb2 2552 	vshl.i32	q1, q1, #18
[^>]*> efb4 2552 	vshl.i32	q1, q1, #20
[^>]*> efb8 2552 	vshl.i32	q1, q1, #24
[^>]*> efbe 2552 	vshl.i32	q1, q1, #30
[^>]*> ef88 2554 	vshl.i8	q1, q2, #0
[^>]*> ef89 2554 	vshl.i8	q1, q2, #1
[^>]*> ef8a 2554 	vshl.i8	q1, q2, #2
[^>]*> ef8c 2554 	vshl.i8	q1, q2, #4
[^>]*> ef8f 2554 	vshl.i8	q1, q2, #7
[^>]*> ef90 2554 	vshl.i16	q1, q2, #0
[^>]*> ef91 2554 	vshl.i16	q1, q2, #1
[^>]*> ef92 2554 	vshl.i16	q1, q2, #2
[^>]*> ef94 2554 	vshl.i16	q1, q2, #4
[^>]*> ef97 2554 	vshl.i16	q1, q2, #7
[^>]*> ef98 2554 	vshl.i16	q1, q2, #8
[^>]*> ef9a 2554 	vshl.i16	q1, q2, #10
[^>]*> ef9c 2554 	vshl.i16	q1, q2, #12
[^>]*> ef9f 2554 	vshl.i16	q1, q2, #15
[^>]*> efa0 2554 	vshl.i32	q1, q2, #0
[^>]*> efa1 2554 	vshl.i32	q1, q2, #1
[^>]*> efa2 2554 	vshl.i32	q1, q2, #2
[^>]*> efa4 2554 	vshl.i32	q1, q2, #4
[^>]*> efa7 2554 	vshl.i32	q1, q2, #7
[^>]*> efa8 2554 	vshl.i32	q1, q2, #8
[^>]*> efaa 2554 	vshl.i32	q1, q2, #10
[^>]*> efac 2554 	vshl.i32	q1, q2, #12
[^>]*> efaf 2554 	vshl.i32	q1, q2, #15
[^>]*> efb0 2554 	vshl.i32	q1, q2, #16
[^>]*> efb2 2554 	vshl.i32	q1, q2, #18
[^>]*> efb4 2554 	vshl.i32	q1, q2, #20
[^>]*> efb8 2554 	vshl.i32	q1, q2, #24
[^>]*> efbe 2554 	vshl.i32	q1, q2, #30
[^>]*> ef88 2558 	vshl.i8	q1, q4, #0
[^>]*> ef89 2558 	vshl.i8	q1, q4, #1
[^>]*> ef8a 2558 	vshl.i8	q1, q4, #2
[^>]*> ef8c 2558 	vshl.i8	q1, q4, #4
[^>]*> ef8f 2558 	vshl.i8	q1, q4, #7
[^>]*> ef90 2558 	vshl.i16	q1, q4, #0
[^>]*> ef91 2558 	vshl.i16	q1, q4, #1
[^>]*> ef92 2558 	vshl.i16	q1, q4, #2
[^>]*> ef94 2558 	vshl.i16	q1, q4, #4
[^>]*> ef97 2558 	vshl.i16	q1, q4, #7
[^>]*> ef98 2558 	vshl.i16	q1, q4, #8
[^>]*> ef9a 2558 	vshl.i16	q1, q4, #10
[^>]*> ef9c 2558 	vshl.i16	q1, q4, #12
[^>]*> ef9f 2558 	vshl.i16	q1, q4, #15
[^>]*> efa0 2558 	vshl.i32	q1, q4, #0
[^>]*> efa1 2558 	vshl.i32	q1, q4, #1
[^>]*> efa2 2558 	vshl.i32	q1, q4, #2
[^>]*> efa4 2558 	vshl.i32	q1, q4, #4
[^>]*> efa7 2558 	vshl.i32	q1, q4, #7
[^>]*> efa8 2558 	vshl.i32	q1, q4, #8
[^>]*> efaa 2558 	vshl.i32	q1, q4, #10
[^>]*> efac 2558 	vshl.i32	q1, q4, #12
[^>]*> efaf 2558 	vshl.i32	q1, q4, #15
[^>]*> efb0 2558 	vshl.i32	q1, q4, #16
[^>]*> efb2 2558 	vshl.i32	q1, q4, #18
[^>]*> efb4 2558 	vshl.i32	q1, q4, #20
[^>]*> efb8 2558 	vshl.i32	q1, q4, #24
[^>]*> efbe 2558 	vshl.i32	q1, q4, #30
[^>]*> ef88 255e 	vshl.i8	q1, q7, #0
[^>]*> ef89 255e 	vshl.i8	q1, q7, #1
[^>]*> ef8a 255e 	vshl.i8	q1, q7, #2
[^>]*> ef8c 255e 	vshl.i8	q1, q7, #4
[^>]*> ef8f 255e 	vshl.i8	q1, q7, #7
[^>]*> ef90 255e 	vshl.i16	q1, q7, #0
[^>]*> ef91 255e 	vshl.i16	q1, q7, #1
[^>]*> ef92 255e 	vshl.i16	q1, q7, #2
[^>]*> ef94 255e 	vshl.i16	q1, q7, #4
[^>]*> ef97 255e 	vshl.i16	q1, q7, #7
[^>]*> ef98 255e 	vshl.i16	q1, q7, #8
[^>]*> ef9a 255e 	vshl.i16	q1, q7, #10
[^>]*> ef9c 255e 	vshl.i16	q1, q7, #12
[^>]*> ef9f 255e 	vshl.i16	q1, q7, #15
[^>]*> efa0 255e 	vshl.i32	q1, q7, #0
[^>]*> efa1 255e 	vshl.i32	q1, q7, #1
[^>]*> efa2 255e 	vshl.i32	q1, q7, #2
[^>]*> efa4 255e 	vshl.i32	q1, q7, #4
[^>]*> efa7 255e 	vshl.i32	q1, q7, #7
[^>]*> efa8 255e 	vshl.i32	q1, q7, #8
[^>]*> efaa 255e 	vshl.i32	q1, q7, #10
[^>]*> efac 255e 	vshl.i32	q1, q7, #12
[^>]*> efaf 255e 	vshl.i32	q1, q7, #15
[^>]*> efb0 255e 	vshl.i32	q1, q7, #16
[^>]*> efb2 255e 	vshl.i32	q1, q7, #18
[^>]*> efb4 255e 	vshl.i32	q1, q7, #20
[^>]*> efb8 255e 	vshl.i32	q1, q7, #24
[^>]*> efbe 255e 	vshl.i32	q1, q7, #30
[^>]*> ef88 4550 	vshl.i8	q2, q0, #0
[^>]*> ef89 4550 	vshl.i8	q2, q0, #1
[^>]*> ef8a 4550 	vshl.i8	q2, q0, #2
[^>]*> ef8c 4550 	vshl.i8	q2, q0, #4
[^>]*> ef8f 4550 	vshl.i8	q2, q0, #7
[^>]*> ef90 4550 	vshl.i16	q2, q0, #0
[^>]*> ef91 4550 	vshl.i16	q2, q0, #1
[^>]*> ef92 4550 	vshl.i16	q2, q0, #2
[^>]*> ef94 4550 	vshl.i16	q2, q0, #4
[^>]*> ef97 4550 	vshl.i16	q2, q0, #7
[^>]*> ef98 4550 	vshl.i16	q2, q0, #8
[^>]*> ef9a 4550 	vshl.i16	q2, q0, #10
[^>]*> ef9c 4550 	vshl.i16	q2, q0, #12
[^>]*> ef9f 4550 	vshl.i16	q2, q0, #15
[^>]*> efa0 4550 	vshl.i32	q2, q0, #0
[^>]*> efa1 4550 	vshl.i32	q2, q0, #1
[^>]*> efa2 4550 	vshl.i32	q2, q0, #2
[^>]*> efa4 4550 	vshl.i32	q2, q0, #4
[^>]*> efa7 4550 	vshl.i32	q2, q0, #7
[^>]*> efa8 4550 	vshl.i32	q2, q0, #8
[^>]*> efaa 4550 	vshl.i32	q2, q0, #10
[^>]*> efac 4550 	vshl.i32	q2, q0, #12
[^>]*> efaf 4550 	vshl.i32	q2, q0, #15
[^>]*> efb0 4550 	vshl.i32	q2, q0, #16
[^>]*> efb2 4550 	vshl.i32	q2, q0, #18
[^>]*> efb4 4550 	vshl.i32	q2, q0, #20
[^>]*> efb8 4550 	vshl.i32	q2, q0, #24
[^>]*> efbe 4550 	vshl.i32	q2, q0, #30
[^>]*> ef88 4552 	vshl.i8	q2, q1, #0
[^>]*> ef89 4552 	vshl.i8	q2, q1, #1
[^>]*> ef8a 4552 	vshl.i8	q2, q1, #2
[^>]*> ef8c 4552 	vshl.i8	q2, q1, #4
[^>]*> ef8f 4552 	vshl.i8	q2, q1, #7
[^>]*> ef90 4552 	vshl.i16	q2, q1, #0
[^>]*> ef91 4552 	vshl.i16	q2, q1, #1
[^>]*> ef92 4552 	vshl.i16	q2, q1, #2
[^>]*> ef94 4552 	vshl.i16	q2, q1, #4
[^>]*> ef97 4552 	vshl.i16	q2, q1, #7
[^>]*> ef98 4552 	vshl.i16	q2, q1, #8
[^>]*> ef9a 4552 	vshl.i16	q2, q1, #10
[^>]*> ef9c 4552 	vshl.i16	q2, q1, #12
[^>]*> ef9f 4552 	vshl.i16	q2, q1, #15
[^>]*> efa0 4552 	vshl.i32	q2, q1, #0
[^>]*> efa1 4552 	vshl.i32	q2, q1, #1
[^>]*> efa2 4552 	vshl.i32	q2, q1, #2
[^>]*> efa4 4552 	vshl.i32	q2, q1, #4
[^>]*> efa7 4552 	vshl.i32	q2, q1, #7
[^>]*> efa8 4552 	vshl.i32	q2, q1, #8
[^>]*> efaa 4552 	vshl.i32	q2, q1, #10
[^>]*> efac 4552 	vshl.i32	q2, q1, #12
[^>]*> efaf 4552 	vshl.i32	q2, q1, #15
[^>]*> efb0 4552 	vshl.i32	q2, q1, #16
[^>]*> efb2 4552 	vshl.i32	q2, q1, #18
[^>]*> efb4 4552 	vshl.i32	q2, q1, #20
[^>]*> efb8 4552 	vshl.i32	q2, q1, #24
[^>]*> efbe 4552 	vshl.i32	q2, q1, #30
[^>]*> ef88 4554 	vshl.i8	q2, q2, #0
[^>]*> ef89 4554 	vshl.i8	q2, q2, #1
[^>]*> ef8a 4554 	vshl.i8	q2, q2, #2
[^>]*> ef8c 4554 	vshl.i8	q2, q2, #4
[^>]*> ef8f 4554 	vshl.i8	q2, q2, #7
[^>]*> ef90 4554 	vshl.i16	q2, q2, #0
[^>]*> ef91 4554 	vshl.i16	q2, q2, #1
[^>]*> ef92 4554 	vshl.i16	q2, q2, #2
[^>]*> ef94 4554 	vshl.i16	q2, q2, #4
[^>]*> ef97 4554 	vshl.i16	q2, q2, #7
[^>]*> ef98 4554 	vshl.i16	q2, q2, #8
[^>]*> ef9a 4554 	vshl.i16	q2, q2, #10
[^>]*> ef9c 4554 	vshl.i16	q2, q2, #12
[^>]*> ef9f 4554 	vshl.i16	q2, q2, #15
[^>]*> efa0 4554 	vshl.i32	q2, q2, #0
[^>]*> efa1 4554 	vshl.i32	q2, q2, #1
[^>]*> efa2 4554 	vshl.i32	q2, q2, #2
[^>]*> efa4 4554 	vshl.i32	q2, q2, #4
[^>]*> efa7 4554 	vshl.i32	q2, q2, #7
[^>]*> efa8 4554 	vshl.i32	q2, q2, #8
[^>]*> efaa 4554 	vshl.i32	q2, q2, #10
[^>]*> efac 4554 	vshl.i32	q2, q2, #12
[^>]*> efaf 4554 	vshl.i32	q2, q2, #15
[^>]*> efb0 4554 	vshl.i32	q2, q2, #16
[^>]*> efb2 4554 	vshl.i32	q2, q2, #18
[^>]*> efb4 4554 	vshl.i32	q2, q2, #20
[^>]*> efb8 4554 	vshl.i32	q2, q2, #24
[^>]*> efbe 4554 	vshl.i32	q2, q2, #30
[^>]*> ef88 4558 	vshl.i8	q2, q4, #0
[^>]*> ef89 4558 	vshl.i8	q2, q4, #1
[^>]*> ef8a 4558 	vshl.i8	q2, q4, #2
[^>]*> ef8c 4558 	vshl.i8	q2, q4, #4
[^>]*> ef8f 4558 	vshl.i8	q2, q4, #7
[^>]*> ef90 4558 	vshl.i16	q2, q4, #0
[^>]*> ef91 4558 	vshl.i16	q2, q4, #1
[^>]*> ef92 4558 	vshl.i16	q2, q4, #2
[^>]*> ef94 4558 	vshl.i16	q2, q4, #4
[^>]*> ef97 4558 	vshl.i16	q2, q4, #7
[^>]*> ef98 4558 	vshl.i16	q2, q4, #8
[^>]*> ef9a 4558 	vshl.i16	q2, q4, #10
[^>]*> ef9c 4558 	vshl.i16	q2, q4, #12
[^>]*> ef9f 4558 	vshl.i16	q2, q4, #15
[^>]*> efa0 4558 	vshl.i32	q2, q4, #0
[^>]*> efa1 4558 	vshl.i32	q2, q4, #1
[^>]*> efa2 4558 	vshl.i32	q2, q4, #2
[^>]*> efa4 4558 	vshl.i32	q2, q4, #4
[^>]*> efa7 4558 	vshl.i32	q2, q4, #7
[^>]*> efa8 4558 	vshl.i32	q2, q4, #8
[^>]*> efaa 4558 	vshl.i32	q2, q4, #10
[^>]*> efac 4558 	vshl.i32	q2, q4, #12
[^>]*> efaf 4558 	vshl.i32	q2, q4, #15
[^>]*> efb0 4558 	vshl.i32	q2, q4, #16
[^>]*> efb2 4558 	vshl.i32	q2, q4, #18
[^>]*> efb4 4558 	vshl.i32	q2, q4, #20
[^>]*> efb8 4558 	vshl.i32	q2, q4, #24
[^>]*> efbe 4558 	vshl.i32	q2, q4, #30
[^>]*> ef88 455e 	vshl.i8	q2, q7, #0
[^>]*> ef89 455e 	vshl.i8	q2, q7, #1
[^>]*> ef8a 455e 	vshl.i8	q2, q7, #2
[^>]*> ef8c 455e 	vshl.i8	q2, q7, #4
[^>]*> ef8f 455e 	vshl.i8	q2, q7, #7
[^>]*> ef90 455e 	vshl.i16	q2, q7, #0
[^>]*> ef91 455e 	vshl.i16	q2, q7, #1
[^>]*> ef92 455e 	vshl.i16	q2, q7, #2
[^>]*> ef94 455e 	vshl.i16	q2, q7, #4
[^>]*> ef97 455e 	vshl.i16	q2, q7, #7
[^>]*> ef98 455e 	vshl.i16	q2, q7, #8
[^>]*> ef9a 455e 	vshl.i16	q2, q7, #10
[^>]*> ef9c 455e 	vshl.i16	q2, q7, #12
[^>]*> ef9f 455e 	vshl.i16	q2, q7, #15
[^>]*> efa0 455e 	vshl.i32	q2, q7, #0
[^>]*> efa1 455e 	vshl.i32	q2, q7, #1
[^>]*> efa2 455e 	vshl.i32	q2, q7, #2
[^>]*> efa4 455e 	vshl.i32	q2, q7, #4
[^>]*> efa7 455e 	vshl.i32	q2, q7, #7
[^>]*> efa8 455e 	vshl.i32	q2, q7, #8
[^>]*> efaa 455e 	vshl.i32	q2, q7, #10
[^>]*> efac 455e 	vshl.i32	q2, q7, #12
[^>]*> efaf 455e 	vshl.i32	q2, q7, #15
[^>]*> efb0 455e 	vshl.i32	q2, q7, #16
[^>]*> efb2 455e 	vshl.i32	q2, q7, #18
[^>]*> efb4 455e 	vshl.i32	q2, q7, #20
[^>]*> efb8 455e 	vshl.i32	q2, q7, #24
[^>]*> efbe 455e 	vshl.i32	q2, q7, #30
[^>]*> ef88 8550 	vshl.i8	q4, q0, #0
[^>]*> ef89 8550 	vshl.i8	q4, q0, #1
[^>]*> ef8a 8550 	vshl.i8	q4, q0, #2
[^>]*> ef8c 8550 	vshl.i8	q4, q0, #4
[^>]*> ef8f 8550 	vshl.i8	q4, q0, #7
[^>]*> ef90 8550 	vshl.i16	q4, q0, #0
[^>]*> ef91 8550 	vshl.i16	q4, q0, #1
[^>]*> ef92 8550 	vshl.i16	q4, q0, #2
[^>]*> ef94 8550 	vshl.i16	q4, q0, #4
[^>]*> ef97 8550 	vshl.i16	q4, q0, #7
[^>]*> ef98 8550 	vshl.i16	q4, q0, #8
[^>]*> ef9a 8550 	vshl.i16	q4, q0, #10
[^>]*> ef9c 8550 	vshl.i16	q4, q0, #12
[^>]*> ef9f 8550 	vshl.i16	q4, q0, #15
[^>]*> efa0 8550 	vshl.i32	q4, q0, #0
[^>]*> efa1 8550 	vshl.i32	q4, q0, #1
[^>]*> efa2 8550 	vshl.i32	q4, q0, #2
[^>]*> efa4 8550 	vshl.i32	q4, q0, #4
[^>]*> efa7 8550 	vshl.i32	q4, q0, #7
[^>]*> efa8 8550 	vshl.i32	q4, q0, #8
[^>]*> efaa 8550 	vshl.i32	q4, q0, #10
[^>]*> efac 8550 	vshl.i32	q4, q0, #12
[^>]*> efaf 8550 	vshl.i32	q4, q0, #15
[^>]*> efb0 8550 	vshl.i32	q4, q0, #16
[^>]*> efb2 8550 	vshl.i32	q4, q0, #18
[^>]*> efb4 8550 	vshl.i32	q4, q0, #20
[^>]*> efb8 8550 	vshl.i32	q4, q0, #24
[^>]*> efbe 8550 	vshl.i32	q4, q0, #30
[^>]*> ef88 8552 	vshl.i8	q4, q1, #0
[^>]*> ef89 8552 	vshl.i8	q4, q1, #1
[^>]*> ef8a 8552 	vshl.i8	q4, q1, #2
[^>]*> ef8c 8552 	vshl.i8	q4, q1, #4
[^>]*> ef8f 8552 	vshl.i8	q4, q1, #7
[^>]*> ef90 8552 	vshl.i16	q4, q1, #0
[^>]*> ef91 8552 	vshl.i16	q4, q1, #1
[^>]*> ef92 8552 	vshl.i16	q4, q1, #2
[^>]*> ef94 8552 	vshl.i16	q4, q1, #4
[^>]*> ef97 8552 	vshl.i16	q4, q1, #7
[^>]*> ef98 8552 	vshl.i16	q4, q1, #8
[^>]*> ef9a 8552 	vshl.i16	q4, q1, #10
[^>]*> ef9c 8552 	vshl.i16	q4, q1, #12
[^>]*> ef9f 8552 	vshl.i16	q4, q1, #15
[^>]*> efa0 8552 	vshl.i32	q4, q1, #0
[^>]*> efa1 8552 	vshl.i32	q4, q1, #1
[^>]*> efa2 8552 	vshl.i32	q4, q1, #2
[^>]*> efa4 8552 	vshl.i32	q4, q1, #4
[^>]*> efa7 8552 	vshl.i32	q4, q1, #7
[^>]*> efa8 8552 	vshl.i32	q4, q1, #8
[^>]*> efaa 8552 	vshl.i32	q4, q1, #10
[^>]*> efac 8552 	vshl.i32	q4, q1, #12
[^>]*> efaf 8552 	vshl.i32	q4, q1, #15
[^>]*> efb0 8552 	vshl.i32	q4, q1, #16
[^>]*> efb2 8552 	vshl.i32	q4, q1, #18
[^>]*> efb4 8552 	vshl.i32	q4, q1, #20
[^>]*> efb8 8552 	vshl.i32	q4, q1, #24
[^>]*> efbe 8552 	vshl.i32	q4, q1, #30
[^>]*> ef88 8554 	vshl.i8	q4, q2, #0
[^>]*> ef89 8554 	vshl.i8	q4, q2, #1
[^>]*> ef8a 8554 	vshl.i8	q4, q2, #2
[^>]*> ef8c 8554 	vshl.i8	q4, q2, #4
[^>]*> ef8f 8554 	vshl.i8	q4, q2, #7
[^>]*> ef90 8554 	vshl.i16	q4, q2, #0
[^>]*> ef91 8554 	vshl.i16	q4, q2, #1
[^>]*> ef92 8554 	vshl.i16	q4, q2, #2
[^>]*> ef94 8554 	vshl.i16	q4, q2, #4
[^>]*> ef97 8554 	vshl.i16	q4, q2, #7
[^>]*> ef98 8554 	vshl.i16	q4, q2, #8
[^>]*> ef9a 8554 	vshl.i16	q4, q2, #10
[^>]*> ef9c 8554 	vshl.i16	q4, q2, #12
[^>]*> ef9f 8554 	vshl.i16	q4, q2, #15
[^>]*> efa0 8554 	vshl.i32	q4, q2, #0
[^>]*> efa1 8554 	vshl.i32	q4, q2, #1
[^>]*> efa2 8554 	vshl.i32	q4, q2, #2
[^>]*> efa4 8554 	vshl.i32	q4, q2, #4
[^>]*> efa7 8554 	vshl.i32	q4, q2, #7
[^>]*> efa8 8554 	vshl.i32	q4, q2, #8
[^>]*> efaa 8554 	vshl.i32	q4, q2, #10
[^>]*> efac 8554 	vshl.i32	q4, q2, #12
[^>]*> efaf 8554 	vshl.i32	q4, q2, #15
[^>]*> efb0 8554 	vshl.i32	q4, q2, #16
[^>]*> efb2 8554 	vshl.i32	q4, q2, #18
[^>]*> efb4 8554 	vshl.i32	q4, q2, #20
[^>]*> efb8 8554 	vshl.i32	q4, q2, #24
[^>]*> efbe 8554 	vshl.i32	q4, q2, #30
[^>]*> ef88 8558 	vshl.i8	q4, q4, #0
[^>]*> ef89 8558 	vshl.i8	q4, q4, #1
[^>]*> ef8a 8558 	vshl.i8	q4, q4, #2
[^>]*> ef8c 8558 	vshl.i8	q4, q4, #4
[^>]*> ef8f 8558 	vshl.i8	q4, q4, #7
[^>]*> ef90 8558 	vshl.i16	q4, q4, #0
[^>]*> ef91 8558 	vshl.i16	q4, q4, #1
[^>]*> ef92 8558 	vshl.i16	q4, q4, #2
[^>]*> ef94 8558 	vshl.i16	q4, q4, #4
[^>]*> ef97 8558 	vshl.i16	q4, q4, #7
[^>]*> ef98 8558 	vshl.i16	q4, q4, #8
[^>]*> ef9a 8558 	vshl.i16	q4, q4, #10
[^>]*> ef9c 8558 	vshl.i16	q4, q4, #12
[^>]*> ef9f 8558 	vshl.i16	q4, q4, #15
[^>]*> efa0 8558 	vshl.i32	q4, q4, #0
[^>]*> efa1 8558 	vshl.i32	q4, q4, #1
[^>]*> efa2 8558 	vshl.i32	q4, q4, #2
[^>]*> efa4 8558 	vshl.i32	q4, q4, #4
[^>]*> efa7 8558 	vshl.i32	q4, q4, #7
[^>]*> efa8 8558 	vshl.i32	q4, q4, #8
[^>]*> efaa 8558 	vshl.i32	q4, q4, #10
[^>]*> efac 8558 	vshl.i32	q4, q4, #12
[^>]*> efaf 8558 	vshl.i32	q4, q4, #15
[^>]*> efb0 8558 	vshl.i32	q4, q4, #16
[^>]*> efb2 8558 	vshl.i32	q4, q4, #18
[^>]*> efb4 8558 	vshl.i32	q4, q4, #20
[^>]*> efb8 8558 	vshl.i32	q4, q4, #24
[^>]*> efbe 8558 	vshl.i32	q4, q4, #30
[^>]*> ef88 855e 	vshl.i8	q4, q7, #0
[^>]*> ef89 855e 	vshl.i8	q4, q7, #1
[^>]*> ef8a 855e 	vshl.i8	q4, q7, #2
[^>]*> ef8c 855e 	vshl.i8	q4, q7, #4
[^>]*> ef8f 855e 	vshl.i8	q4, q7, #7
[^>]*> ef90 855e 	vshl.i16	q4, q7, #0
[^>]*> ef91 855e 	vshl.i16	q4, q7, #1
[^>]*> ef92 855e 	vshl.i16	q4, q7, #2
[^>]*> ef94 855e 	vshl.i16	q4, q7, #4
[^>]*> ef97 855e 	vshl.i16	q4, q7, #7
[^>]*> ef98 855e 	vshl.i16	q4, q7, #8
[^>]*> ef9a 855e 	vshl.i16	q4, q7, #10
[^>]*> ef9c 855e 	vshl.i16	q4, q7, #12
[^>]*> ef9f 855e 	vshl.i16	q4, q7, #15
[^>]*> efa0 855e 	vshl.i32	q4, q7, #0
[^>]*> efa1 855e 	vshl.i32	q4, q7, #1
[^>]*> efa2 855e 	vshl.i32	q4, q7, #2
[^>]*> efa4 855e 	vshl.i32	q4, q7, #4
[^>]*> efa7 855e 	vshl.i32	q4, q7, #7
[^>]*> efa8 855e 	vshl.i32	q4, q7, #8
[^>]*> efaa 855e 	vshl.i32	q4, q7, #10
[^>]*> efac 855e 	vshl.i32	q4, q7, #12
[^>]*> efaf 855e 	vshl.i32	q4, q7, #15
[^>]*> efb0 855e 	vshl.i32	q4, q7, #16
[^>]*> efb2 855e 	vshl.i32	q4, q7, #18
[^>]*> efb4 855e 	vshl.i32	q4, q7, #20
[^>]*> efb8 855e 	vshl.i32	q4, q7, #24
[^>]*> efbe 855e 	vshl.i32	q4, q7, #30
[^>]*> ef88 e550 	vshl.i8	q7, q0, #0
[^>]*> ef89 e550 	vshl.i8	q7, q0, #1
[^>]*> ef8a e550 	vshl.i8	q7, q0, #2
[^>]*> ef8c e550 	vshl.i8	q7, q0, #4
[^>]*> ef8f e550 	vshl.i8	q7, q0, #7
[^>]*> ef90 e550 	vshl.i16	q7, q0, #0
[^>]*> ef91 e550 	vshl.i16	q7, q0, #1
[^>]*> ef92 e550 	vshl.i16	q7, q0, #2
[^>]*> ef94 e550 	vshl.i16	q7, q0, #4
[^>]*> ef97 e550 	vshl.i16	q7, q0, #7
[^>]*> ef98 e550 	vshl.i16	q7, q0, #8
[^>]*> ef9a e550 	vshl.i16	q7, q0, #10
[^>]*> ef9c e550 	vshl.i16	q7, q0, #12
[^>]*> ef9f e550 	vshl.i16	q7, q0, #15
[^>]*> efa0 e550 	vshl.i32	q7, q0, #0
[^>]*> efa1 e550 	vshl.i32	q7, q0, #1
[^>]*> efa2 e550 	vshl.i32	q7, q0, #2
[^>]*> efa4 e550 	vshl.i32	q7, q0, #4
[^>]*> efa7 e550 	vshl.i32	q7, q0, #7
[^>]*> efa8 e550 	vshl.i32	q7, q0, #8
[^>]*> efaa e550 	vshl.i32	q7, q0, #10
[^>]*> efac e550 	vshl.i32	q7, q0, #12
[^>]*> efaf e550 	vshl.i32	q7, q0, #15
[^>]*> efb0 e550 	vshl.i32	q7, q0, #16
[^>]*> efb2 e550 	vshl.i32	q7, q0, #18
[^>]*> efb4 e550 	vshl.i32	q7, q0, #20
[^>]*> efb8 e550 	vshl.i32	q7, q0, #24
[^>]*> efbe e550 	vshl.i32	q7, q0, #30
[^>]*> ef88 e552 	vshl.i8	q7, q1, #0
[^>]*> ef89 e552 	vshl.i8	q7, q1, #1
[^>]*> ef8a e552 	vshl.i8	q7, q1, #2
[^>]*> ef8c e552 	vshl.i8	q7, q1, #4
[^>]*> ef8f e552 	vshl.i8	q7, q1, #7
[^>]*> ef90 e552 	vshl.i16	q7, q1, #0
[^>]*> ef91 e552 	vshl.i16	q7, q1, #1
[^>]*> ef92 e552 	vshl.i16	q7, q1, #2
[^>]*> ef94 e552 	vshl.i16	q7, q1, #4
[^>]*> ef97 e552 	vshl.i16	q7, q1, #7
[^>]*> ef98 e552 	vshl.i16	q7, q1, #8
[^>]*> ef9a e552 	vshl.i16	q7, q1, #10
[^>]*> ef9c e552 	vshl.i16	q7, q1, #12
[^>]*> ef9f e552 	vshl.i16	q7, q1, #15
[^>]*> efa0 e552 	vshl.i32	q7, q1, #0
[^>]*> efa1 e552 	vshl.i32	q7, q1, #1
[^>]*> efa2 e552 	vshl.i32	q7, q1, #2
[^>]*> efa4 e552 	vshl.i32	q7, q1, #4
[^>]*> efa7 e552 	vshl.i32	q7, q1, #7
[^>]*> efa8 e552 	vshl.i32	q7, q1, #8
[^>]*> efaa e552 	vshl.i32	q7, q1, #10
[^>]*> efac e552 	vshl.i32	q7, q1, #12
[^>]*> efaf e552 	vshl.i32	q7, q1, #15
[^>]*> efb0 e552 	vshl.i32	q7, q1, #16
[^>]*> efb2 e552 	vshl.i32	q7, q1, #18
[^>]*> efb4 e552 	vshl.i32	q7, q1, #20
[^>]*> efb8 e552 	vshl.i32	q7, q1, #24
[^>]*> efbe e552 	vshl.i32	q7, q1, #30
[^>]*> ef88 e554 	vshl.i8	q7, q2, #0
[^>]*> ef89 e554 	vshl.i8	q7, q2, #1
[^>]*> ef8a e554 	vshl.i8	q7, q2, #2
[^>]*> ef8c e554 	vshl.i8	q7, q2, #4
[^>]*> ef8f e554 	vshl.i8	q7, q2, #7
[^>]*> ef90 e554 	vshl.i16	q7, q2, #0
[^>]*> ef91 e554 	vshl.i16	q7, q2, #1
[^>]*> ef92 e554 	vshl.i16	q7, q2, #2
[^>]*> ef94 e554 	vshl.i16	q7, q2, #4
[^>]*> ef97 e554 	vshl.i16	q7, q2, #7
[^>]*> ef98 e554 	vshl.i16	q7, q2, #8
[^>]*> ef9a e554 	vshl.i16	q7, q2, #10
[^>]*> ef9c e554 	vshl.i16	q7, q2, #12
[^>]*> ef9f e554 	vshl.i16	q7, q2, #15
[^>]*> efa0 e554 	vshl.i32	q7, q2, #0
[^>]*> efa1 e554 	vshl.i32	q7, q2, #1
[^>]*> efa2 e554 	vshl.i32	q7, q2, #2
[^>]*> efa4 e554 	vshl.i32	q7, q2, #4
[^>]*> efa7 e554 	vshl.i32	q7, q2, #7
[^>]*> efa8 e554 	vshl.i32	q7, q2, #8
[^>]*> efaa e554 	vshl.i32	q7, q2, #10
[^>]*> efac e554 	vshl.i32	q7, q2, #12
[^>]*> efaf e554 	vshl.i32	q7, q2, #15
[^>]*> efb0 e554 	vshl.i32	q7, q2, #16
[^>]*> efb2 e554 	vshl.i32	q7, q2, #18
[^>]*> efb4 e554 	vshl.i32	q7, q2, #20
[^>]*> efb8 e554 	vshl.i32	q7, q2, #24
[^>]*> efbe e554 	vshl.i32	q7, q2, #30
[^>]*> ef88 e558 	vshl.i8	q7, q4, #0
[^>]*> ef89 e558 	vshl.i8	q7, q4, #1
[^>]*> ef8a e558 	vshl.i8	q7, q4, #2
[^>]*> ef8c e558 	vshl.i8	q7, q4, #4
[^>]*> ef8f e558 	vshl.i8	q7, q4, #7
[^>]*> ef90 e558 	vshl.i16	q7, q4, #0
[^>]*> ef91 e558 	vshl.i16	q7, q4, #1
[^>]*> ef92 e558 	vshl.i16	q7, q4, #2
[^>]*> ef94 e558 	vshl.i16	q7, q4, #4
[^>]*> ef97 e558 	vshl.i16	q7, q4, #7
[^>]*> ef98 e558 	vshl.i16	q7, q4, #8
[^>]*> ef9a e558 	vshl.i16	q7, q4, #10
[^>]*> ef9c e558 	vshl.i16	q7, q4, #12
[^>]*> ef9f e558 	vshl.i16	q7, q4, #15
[^>]*> efa0 e558 	vshl.i32	q7, q4, #0
[^>]*> efa1 e558 	vshl.i32	q7, q4, #1
[^>]*> efa2 e558 	vshl.i32	q7, q4, #2
[^>]*> efa4 e558 	vshl.i32	q7, q4, #4
[^>]*> efa7 e558 	vshl.i32	q7, q4, #7
[^>]*> efa8 e558 	vshl.i32	q7, q4, #8
[^>]*> efaa e558 	vshl.i32	q7, q4, #10
[^>]*> efac e558 	vshl.i32	q7, q4, #12
[^>]*> efaf e558 	vshl.i32	q7, q4, #15
[^>]*> efb0 e558 	vshl.i32	q7, q4, #16
[^>]*> efb2 e558 	vshl.i32	q7, q4, #18
[^>]*> efb4 e558 	vshl.i32	q7, q4, #20
[^>]*> efb8 e558 	vshl.i32	q7, q4, #24
[^>]*> efbe e558 	vshl.i32	q7, q4, #30
[^>]*> ef88 e55e 	vshl.i8	q7, q7, #0
[^>]*> ef89 e55e 	vshl.i8	q7, q7, #1
[^>]*> ef8a e55e 	vshl.i8	q7, q7, #2
[^>]*> ef8c e55e 	vshl.i8	q7, q7, #4
[^>]*> ef8f e55e 	vshl.i8	q7, q7, #7
[^>]*> ef90 e55e 	vshl.i16	q7, q7, #0
[^>]*> ef91 e55e 	vshl.i16	q7, q7, #1
[^>]*> ef92 e55e 	vshl.i16	q7, q7, #2
[^>]*> ef94 e55e 	vshl.i16	q7, q7, #4
[^>]*> ef97 e55e 	vshl.i16	q7, q7, #7
[^>]*> ef98 e55e 	vshl.i16	q7, q7, #8
[^>]*> ef9a e55e 	vshl.i16	q7, q7, #10
[^>]*> ef9c e55e 	vshl.i16	q7, q7, #12
[^>]*> ef9f e55e 	vshl.i16	q7, q7, #15
[^>]*> efa0 e55e 	vshl.i32	q7, q7, #0
[^>]*> efa1 e55e 	vshl.i32	q7, q7, #1
[^>]*> efa2 e55e 	vshl.i32	q7, q7, #2
[^>]*> efa4 e55e 	vshl.i32	q7, q7, #4
[^>]*> efa7 e55e 	vshl.i32	q7, q7, #7
[^>]*> efa8 e55e 	vshl.i32	q7, q7, #8
[^>]*> efaa e55e 	vshl.i32	q7, q7, #10
[^>]*> efac e55e 	vshl.i32	q7, q7, #12
[^>]*> efaf e55e 	vshl.i32	q7, q7, #15
[^>]*> efb0 e55e 	vshl.i32	q7, q7, #16
[^>]*> efb2 e55e 	vshl.i32	q7, q7, #18
[^>]*> efb4 e55e 	vshl.i32	q7, q7, #20
[^>]*> efb8 e55e 	vshl.i32	q7, q7, #24
[^>]*> efbe e55e 	vshl.i32	q7, q7, #30
[^>]*> ff00 0440 	vshl.u8	q0, q0, q0
[^>]*> ff02 0440 	vshl.u8	q0, q0, q1
[^>]*> ff04 0440 	vshl.u8	q0, q0, q2
[^>]*> ff08 0440 	vshl.u8	q0, q0, q4
[^>]*> ff0e 0440 	vshl.u8	q0, q0, q7
[^>]*> ff00 0442 	vshl.u8	q0, q1, q0
[^>]*> ff02 0442 	vshl.u8	q0, q1, q1
[^>]*> ff04 0442 	vshl.u8	q0, q1, q2
[^>]*> ff08 0442 	vshl.u8	q0, q1, q4
[^>]*> ff0e 0442 	vshl.u8	q0, q1, q7
[^>]*> ff00 0444 	vshl.u8	q0, q2, q0
[^>]*> ff02 0444 	vshl.u8	q0, q2, q1
[^>]*> ff04 0444 	vshl.u8	q0, q2, q2
[^>]*> ff08 0444 	vshl.u8	q0, q2, q4
[^>]*> ff0e 0444 	vshl.u8	q0, q2, q7
[^>]*> ff00 0448 	vshl.u8	q0, q4, q0
[^>]*> ff02 0448 	vshl.u8	q0, q4, q1
[^>]*> ff04 0448 	vshl.u8	q0, q4, q2
[^>]*> ff08 0448 	vshl.u8	q0, q4, q4
[^>]*> ff0e 0448 	vshl.u8	q0, q4, q7
[^>]*> ff00 044e 	vshl.u8	q0, q7, q0
[^>]*> ff02 044e 	vshl.u8	q0, q7, q1
[^>]*> ff04 044e 	vshl.u8	q0, q7, q2
[^>]*> ff08 044e 	vshl.u8	q0, q7, q4
[^>]*> ff0e 044e 	vshl.u8	q0, q7, q7
[^>]*> fe31 1e60 	vshl.u8	q0, r0
[^>]*> fe31 1e61 	vshl.u8	q0, r1
[^>]*> fe31 1e62 	vshl.u8	q0, r2
[^>]*> fe31 1e64 	vshl.u8	q0, r4
[^>]*> fe31 1e67 	vshl.u8	q0, r7
[^>]*> fe31 1e68 	vshl.u8	q0, r8
[^>]*> fe31 1e6a 	vshl.u8	q0, sl
[^>]*> fe31 1e6c 	vshl.u8	q0, ip
[^>]*> fe31 1e6e 	vshl.u8	q0, lr
[^>]*> ff00 2440 	vshl.u8	q1, q0, q0
[^>]*> ff02 2440 	vshl.u8	q1, q0, q1
[^>]*> ff04 2440 	vshl.u8	q1, q0, q2
[^>]*> ff08 2440 	vshl.u8	q1, q0, q4
[^>]*> ff0e 2440 	vshl.u8	q1, q0, q7
[^>]*> ff00 2442 	vshl.u8	q1, q1, q0
[^>]*> ff02 2442 	vshl.u8	q1, q1, q1
[^>]*> ff04 2442 	vshl.u8	q1, q1, q2
[^>]*> ff08 2442 	vshl.u8	q1, q1, q4
[^>]*> ff0e 2442 	vshl.u8	q1, q1, q7
[^>]*> ff00 2444 	vshl.u8	q1, q2, q0
[^>]*> ff02 2444 	vshl.u8	q1, q2, q1
[^>]*> ff04 2444 	vshl.u8	q1, q2, q2
[^>]*> ff08 2444 	vshl.u8	q1, q2, q4
[^>]*> ff0e 2444 	vshl.u8	q1, q2, q7
[^>]*> ff00 2448 	vshl.u8	q1, q4, q0
[^>]*> ff02 2448 	vshl.u8	q1, q4, q1
[^>]*> ff04 2448 	vshl.u8	q1, q4, q2
[^>]*> ff08 2448 	vshl.u8	q1, q4, q4
[^>]*> ff0e 2448 	vshl.u8	q1, q4, q7
[^>]*> ff00 244e 	vshl.u8	q1, q7, q0
[^>]*> ff02 244e 	vshl.u8	q1, q7, q1
[^>]*> ff04 244e 	vshl.u8	q1, q7, q2
[^>]*> ff08 244e 	vshl.u8	q1, q7, q4
[^>]*> ff0e 244e 	vshl.u8	q1, q7, q7
[^>]*> fe31 3e60 	vshl.u8	q1, r0
[^>]*> fe31 3e61 	vshl.u8	q1, r1
[^>]*> fe31 3e62 	vshl.u8	q1, r2
[^>]*> fe31 3e64 	vshl.u8	q1, r4
[^>]*> fe31 3e67 	vshl.u8	q1, r7
[^>]*> fe31 3e68 	vshl.u8	q1, r8
[^>]*> fe31 3e6a 	vshl.u8	q1, sl
[^>]*> fe31 3e6c 	vshl.u8	q1, ip
[^>]*> fe31 3e6e 	vshl.u8	q1, lr
[^>]*> ff00 4440 	vshl.u8	q2, q0, q0
[^>]*> ff02 4440 	vshl.u8	q2, q0, q1
[^>]*> ff04 4440 	vshl.u8	q2, q0, q2
[^>]*> ff08 4440 	vshl.u8	q2, q0, q4
[^>]*> ff0e 4440 	vshl.u8	q2, q0, q7
[^>]*> ff00 4442 	vshl.u8	q2, q1, q0
[^>]*> ff02 4442 	vshl.u8	q2, q1, q1
[^>]*> ff04 4442 	vshl.u8	q2, q1, q2
[^>]*> ff08 4442 	vshl.u8	q2, q1, q4
[^>]*> ff0e 4442 	vshl.u8	q2, q1, q7
[^>]*> ff00 4444 	vshl.u8	q2, q2, q0
[^>]*> ff02 4444 	vshl.u8	q2, q2, q1
[^>]*> ff04 4444 	vshl.u8	q2, q2, q2
[^>]*> ff08 4444 	vshl.u8	q2, q2, q4
[^>]*> ff0e 4444 	vshl.u8	q2, q2, q7
[^>]*> ff00 4448 	vshl.u8	q2, q4, q0
[^>]*> ff02 4448 	vshl.u8	q2, q4, q1
[^>]*> ff04 4448 	vshl.u8	q2, q4, q2
[^>]*> ff08 4448 	vshl.u8	q2, q4, q4
[^>]*> ff0e 4448 	vshl.u8	q2, q4, q7
[^>]*> ff00 444e 	vshl.u8	q2, q7, q0
[^>]*> ff02 444e 	vshl.u8	q2, q7, q1
[^>]*> ff04 444e 	vshl.u8	q2, q7, q2
[^>]*> ff08 444e 	vshl.u8	q2, q7, q4
[^>]*> ff0e 444e 	vshl.u8	q2, q7, q7
[^>]*> fe31 5e60 	vshl.u8	q2, r0
[^>]*> fe31 5e61 	vshl.u8	q2, r1
[^>]*> fe31 5e62 	vshl.u8	q2, r2
[^>]*> fe31 5e64 	vshl.u8	q2, r4
[^>]*> fe31 5e67 	vshl.u8	q2, r7
[^>]*> fe31 5e68 	vshl.u8	q2, r8
[^>]*> fe31 5e6a 	vshl.u8	q2, sl
[^>]*> fe31 5e6c 	vshl.u8	q2, ip
[^>]*> fe31 5e6e 	vshl.u8	q2, lr
[^>]*> ff00 8440 	vshl.u8	q4, q0, q0
[^>]*> ff02 8440 	vshl.u8	q4, q0, q1
[^>]*> ff04 8440 	vshl.u8	q4, q0, q2
[^>]*> ff08 8440 	vshl.u8	q4, q0, q4
[^>]*> ff0e 8440 	vshl.u8	q4, q0, q7
[^>]*> ff00 8442 	vshl.u8	q4, q1, q0
[^>]*> ff02 8442 	vshl.u8	q4, q1, q1
[^>]*> ff04 8442 	vshl.u8	q4, q1, q2
[^>]*> ff08 8442 	vshl.u8	q4, q1, q4
[^>]*> ff0e 8442 	vshl.u8	q4, q1, q7
[^>]*> ff00 8444 	vshl.u8	q4, q2, q0
[^>]*> ff02 8444 	vshl.u8	q4, q2, q1
[^>]*> ff04 8444 	vshl.u8	q4, q2, q2
[^>]*> ff08 8444 	vshl.u8	q4, q2, q4
[^>]*> ff0e 8444 	vshl.u8	q4, q2, q7
[^>]*> ff00 8448 	vshl.u8	q4, q4, q0
[^>]*> ff02 8448 	vshl.u8	q4, q4, q1
[^>]*> ff04 8448 	vshl.u8	q4, q4, q2
[^>]*> ff08 8448 	vshl.u8	q4, q4, q4
[^>]*> ff0e 8448 	vshl.u8	q4, q4, q7
[^>]*> ff00 844e 	vshl.u8	q4, q7, q0
[^>]*> ff02 844e 	vshl.u8	q4, q7, q1
[^>]*> ff04 844e 	vshl.u8	q4, q7, q2
[^>]*> ff08 844e 	vshl.u8	q4, q7, q4
[^>]*> ff0e 844e 	vshl.u8	q4, q7, q7
[^>]*> fe31 9e60 	vshl.u8	q4, r0
[^>]*> fe31 9e61 	vshl.u8	q4, r1
[^>]*> fe31 9e62 	vshl.u8	q4, r2
[^>]*> fe31 9e64 	vshl.u8	q4, r4
[^>]*> fe31 9e67 	vshl.u8	q4, r7
[^>]*> fe31 9e68 	vshl.u8	q4, r8
[^>]*> fe31 9e6a 	vshl.u8	q4, sl
[^>]*> fe31 9e6c 	vshl.u8	q4, ip
[^>]*> fe31 9e6e 	vshl.u8	q4, lr
[^>]*> ff00 e440 	vshl.u8	q7, q0, q0
[^>]*> ff02 e440 	vshl.u8	q7, q0, q1
[^>]*> ff04 e440 	vshl.u8	q7, q0, q2
[^>]*> ff08 e440 	vshl.u8	q7, q0, q4
[^>]*> ff0e e440 	vshl.u8	q7, q0, q7
[^>]*> ff00 e442 	vshl.u8	q7, q1, q0
[^>]*> ff02 e442 	vshl.u8	q7, q1, q1
[^>]*> ff04 e442 	vshl.u8	q7, q1, q2
[^>]*> ff08 e442 	vshl.u8	q7, q1, q4
[^>]*> ff0e e442 	vshl.u8	q7, q1, q7
[^>]*> ff00 e444 	vshl.u8	q7, q2, q0
[^>]*> ff02 e444 	vshl.u8	q7, q2, q1
[^>]*> ff04 e444 	vshl.u8	q7, q2, q2
[^>]*> ff08 e444 	vshl.u8	q7, q2, q4
[^>]*> ff0e e444 	vshl.u8	q7, q2, q7
[^>]*> ff00 e448 	vshl.u8	q7, q4, q0
[^>]*> ff02 e448 	vshl.u8	q7, q4, q1
[^>]*> ff04 e448 	vshl.u8	q7, q4, q2
[^>]*> ff08 e448 	vshl.u8	q7, q4, q4
[^>]*> ff0e e448 	vshl.u8	q7, q4, q7
[^>]*> ff00 e44e 	vshl.u8	q7, q7, q0
[^>]*> ff02 e44e 	vshl.u8	q7, q7, q1
[^>]*> ff04 e44e 	vshl.u8	q7, q7, q2
[^>]*> ff08 e44e 	vshl.u8	q7, q7, q4
[^>]*> ff0e e44e 	vshl.u8	q7, q7, q7
[^>]*> fe31 fe60 	vshl.u8	q7, r0
[^>]*> fe31 fe61 	vshl.u8	q7, r1
[^>]*> fe31 fe62 	vshl.u8	q7, r2
[^>]*> fe31 fe64 	vshl.u8	q7, r4
[^>]*> fe31 fe67 	vshl.u8	q7, r7
[^>]*> fe31 fe68 	vshl.u8	q7, r8
[^>]*> fe31 fe6a 	vshl.u8	q7, sl
[^>]*> fe31 fe6c 	vshl.u8	q7, ip
[^>]*> fe31 fe6e 	vshl.u8	q7, lr
[^>]*> ef00 0440 	vshl.s8	q0, q0, q0
[^>]*> ef02 0440 	vshl.s8	q0, q0, q1
[^>]*> ef04 0440 	vshl.s8	q0, q0, q2
[^>]*> ef08 0440 	vshl.s8	q0, q0, q4
[^>]*> ef0e 0440 	vshl.s8	q0, q0, q7
[^>]*> ef00 0442 	vshl.s8	q0, q1, q0
[^>]*> ef02 0442 	vshl.s8	q0, q1, q1
[^>]*> ef04 0442 	vshl.s8	q0, q1, q2
[^>]*> ef08 0442 	vshl.s8	q0, q1, q4
[^>]*> ef0e 0442 	vshl.s8	q0, q1, q7
[^>]*> ef00 0444 	vshl.s8	q0, q2, q0
[^>]*> ef02 0444 	vshl.s8	q0, q2, q1
[^>]*> ef04 0444 	vshl.s8	q0, q2, q2
[^>]*> ef08 0444 	vshl.s8	q0, q2, q4
[^>]*> ef0e 0444 	vshl.s8	q0, q2, q7
[^>]*> ef00 0448 	vshl.s8	q0, q4, q0
[^>]*> ef02 0448 	vshl.s8	q0, q4, q1
[^>]*> ef04 0448 	vshl.s8	q0, q4, q2
[^>]*> ef08 0448 	vshl.s8	q0, q4, q4
[^>]*> ef0e 0448 	vshl.s8	q0, q4, q7
[^>]*> ef00 044e 	vshl.s8	q0, q7, q0
[^>]*> ef02 044e 	vshl.s8	q0, q7, q1
[^>]*> ef04 044e 	vshl.s8	q0, q7, q2
[^>]*> ef08 044e 	vshl.s8	q0, q7, q4
[^>]*> ef0e 044e 	vshl.s8	q0, q7, q7
[^>]*> ee31 1e60 	vshl.s8	q0, r0
[^>]*> ee31 1e61 	vshl.s8	q0, r1
[^>]*> ee31 1e62 	vshl.s8	q0, r2
[^>]*> ee31 1e64 	vshl.s8	q0, r4
[^>]*> ee31 1e67 	vshl.s8	q0, r7
[^>]*> ee31 1e68 	vshl.s8	q0, r8
[^>]*> ee31 1e6a 	vshl.s8	q0, sl
[^>]*> ee31 1e6c 	vshl.s8	q0, ip
[^>]*> ee31 1e6e 	vshl.s8	q0, lr
[^>]*> ef00 2440 	vshl.s8	q1, q0, q0
[^>]*> ef02 2440 	vshl.s8	q1, q0, q1
[^>]*> ef04 2440 	vshl.s8	q1, q0, q2
[^>]*> ef08 2440 	vshl.s8	q1, q0, q4
[^>]*> ef0e 2440 	vshl.s8	q1, q0, q7
[^>]*> ef00 2442 	vshl.s8	q1, q1, q0
[^>]*> ef02 2442 	vshl.s8	q1, q1, q1
[^>]*> ef04 2442 	vshl.s8	q1, q1, q2
[^>]*> ef08 2442 	vshl.s8	q1, q1, q4
[^>]*> ef0e 2442 	vshl.s8	q1, q1, q7
[^>]*> ef00 2444 	vshl.s8	q1, q2, q0
[^>]*> ef02 2444 	vshl.s8	q1, q2, q1
[^>]*> ef04 2444 	vshl.s8	q1, q2, q2
[^>]*> ef08 2444 	vshl.s8	q1, q2, q4
[^>]*> ef0e 2444 	vshl.s8	q1, q2, q7
[^>]*> ef00 2448 	vshl.s8	q1, q4, q0
[^>]*> ef02 2448 	vshl.s8	q1, q4, q1
[^>]*> ef04 2448 	vshl.s8	q1, q4, q2
[^>]*> ef08 2448 	vshl.s8	q1, q4, q4
[^>]*> ef0e 2448 	vshl.s8	q1, q4, q7
[^>]*> ef00 244e 	vshl.s8	q1, q7, q0
[^>]*> ef02 244e 	vshl.s8	q1, q7, q1
[^>]*> ef04 244e 	vshl.s8	q1, q7, q2
[^>]*> ef08 244e 	vshl.s8	q1, q7, q4
[^>]*> ef0e 244e 	vshl.s8	q1, q7, q7
[^>]*> ee31 3e60 	vshl.s8	q1, r0
[^>]*> ee31 3e61 	vshl.s8	q1, r1
[^>]*> ee31 3e62 	vshl.s8	q1, r2
[^>]*> ee31 3e64 	vshl.s8	q1, r4
[^>]*> ee31 3e67 	vshl.s8	q1, r7
[^>]*> ee31 3e68 	vshl.s8	q1, r8
[^>]*> ee31 3e6a 	vshl.s8	q1, sl
[^>]*> ee31 3e6c 	vshl.s8	q1, ip
[^>]*> ee31 3e6e 	vshl.s8	q1, lr
[^>]*> ef00 4440 	vshl.s8	q2, q0, q0
[^>]*> ef02 4440 	vshl.s8	q2, q0, q1
[^>]*> ef04 4440 	vshl.s8	q2, q0, q2
[^>]*> ef08 4440 	vshl.s8	q2, q0, q4
[^>]*> ef0e 4440 	vshl.s8	q2, q0, q7
[^>]*> ef00 4442 	vshl.s8	q2, q1, q0
[^>]*> ef02 4442 	vshl.s8	q2, q1, q1
[^>]*> ef04 4442 	vshl.s8	q2, q1, q2
[^>]*> ef08 4442 	vshl.s8	q2, q1, q4
[^>]*> ef0e 4442 	vshl.s8	q2, q1, q7
[^>]*> ef00 4444 	vshl.s8	q2, q2, q0
[^>]*> ef02 4444 	vshl.s8	q2, q2, q1
[^>]*> ef04 4444 	vshl.s8	q2, q2, q2
[^>]*> ef08 4444 	vshl.s8	q2, q2, q4
[^>]*> ef0e 4444 	vshl.s8	q2, q2, q7
[^>]*> ef00 4448 	vshl.s8	q2, q4, q0
[^>]*> ef02 4448 	vshl.s8	q2, q4, q1
[^>]*> ef04 4448 	vshl.s8	q2, q4, q2
[^>]*> ef08 4448 	vshl.s8	q2, q4, q4
[^>]*> ef0e 4448 	vshl.s8	q2, q4, q7
[^>]*> ef00 444e 	vshl.s8	q2, q7, q0
[^>]*> ef02 444e 	vshl.s8	q2, q7, q1
[^>]*> ef04 444e 	vshl.s8	q2, q7, q2
[^>]*> ef08 444e 	vshl.s8	q2, q7, q4
[^>]*> ef0e 444e 	vshl.s8	q2, q7, q7
[^>]*> ee31 5e60 	vshl.s8	q2, r0
[^>]*> ee31 5e61 	vshl.s8	q2, r1
[^>]*> ee31 5e62 	vshl.s8	q2, r2
[^>]*> ee31 5e64 	vshl.s8	q2, r4
[^>]*> ee31 5e67 	vshl.s8	q2, r7
[^>]*> ee31 5e68 	vshl.s8	q2, r8
[^>]*> ee31 5e6a 	vshl.s8	q2, sl
[^>]*> ee31 5e6c 	vshl.s8	q2, ip
[^>]*> ee31 5e6e 	vshl.s8	q2, lr
[^>]*> ef00 8440 	vshl.s8	q4, q0, q0
[^>]*> ef02 8440 	vshl.s8	q4, q0, q1
[^>]*> ef04 8440 	vshl.s8	q4, q0, q2
[^>]*> ef08 8440 	vshl.s8	q4, q0, q4
[^>]*> ef0e 8440 	vshl.s8	q4, q0, q7
[^>]*> ef00 8442 	vshl.s8	q4, q1, q0
[^>]*> ef02 8442 	vshl.s8	q4, q1, q1
[^>]*> ef04 8442 	vshl.s8	q4, q1, q2
[^>]*> ef08 8442 	vshl.s8	q4, q1, q4
[^>]*> ef0e 8442 	vshl.s8	q4, q1, q7
[^>]*> ef00 8444 	vshl.s8	q4, q2, q0
[^>]*> ef02 8444 	vshl.s8	q4, q2, q1
[^>]*> ef04 8444 	vshl.s8	q4, q2, q2
[^>]*> ef08 8444 	vshl.s8	q4, q2, q4
[^>]*> ef0e 8444 	vshl.s8	q4, q2, q7
[^>]*> ef00 8448 	vshl.s8	q4, q4, q0
[^>]*> ef02 8448 	vshl.s8	q4, q4, q1
[^>]*> ef04 8448 	vshl.s8	q4, q4, q2
[^>]*> ef08 8448 	vshl.s8	q4, q4, q4
[^>]*> ef0e 8448 	vshl.s8	q4, q4, q7
[^>]*> ef00 844e 	vshl.s8	q4, q7, q0
[^>]*> ef02 844e 	vshl.s8	q4, q7, q1
[^>]*> ef04 844e 	vshl.s8	q4, q7, q2
[^>]*> ef08 844e 	vshl.s8	q4, q7, q4
[^>]*> ef0e 844e 	vshl.s8	q4, q7, q7
[^>]*> ee31 9e60 	vshl.s8	q4, r0
[^>]*> ee31 9e61 	vshl.s8	q4, r1
[^>]*> ee31 9e62 	vshl.s8	q4, r2
[^>]*> ee31 9e64 	vshl.s8	q4, r4
[^>]*> ee31 9e67 	vshl.s8	q4, r7
[^>]*> ee31 9e68 	vshl.s8	q4, r8
[^>]*> ee31 9e6a 	vshl.s8	q4, sl
[^>]*> ee31 9e6c 	vshl.s8	q4, ip
[^>]*> ee31 9e6e 	vshl.s8	q4, lr
[^>]*> ef00 e440 	vshl.s8	q7, q0, q0
[^>]*> ef02 e440 	vshl.s8	q7, q0, q1
[^>]*> ef04 e440 	vshl.s8	q7, q0, q2
[^>]*> ef08 e440 	vshl.s8	q7, q0, q4
[^>]*> ef0e e440 	vshl.s8	q7, q0, q7
[^>]*> ef00 e442 	vshl.s8	q7, q1, q0
[^>]*> ef02 e442 	vshl.s8	q7, q1, q1
[^>]*> ef04 e442 	vshl.s8	q7, q1, q2
[^>]*> ef08 e442 	vshl.s8	q7, q1, q4
[^>]*> ef0e e442 	vshl.s8	q7, q1, q7
[^>]*> ef00 e444 	vshl.s8	q7, q2, q0
[^>]*> ef02 e444 	vshl.s8	q7, q2, q1
[^>]*> ef04 e444 	vshl.s8	q7, q2, q2
[^>]*> ef08 e444 	vshl.s8	q7, q2, q4
[^>]*> ef0e e444 	vshl.s8	q7, q2, q7
[^>]*> ef00 e448 	vshl.s8	q7, q4, q0
[^>]*> ef02 e448 	vshl.s8	q7, q4, q1
[^>]*> ef04 e448 	vshl.s8	q7, q4, q2
[^>]*> ef08 e448 	vshl.s8	q7, q4, q4
[^>]*> ef0e e448 	vshl.s8	q7, q4, q7
[^>]*> ef00 e44e 	vshl.s8	q7, q7, q0
[^>]*> ef02 e44e 	vshl.s8	q7, q7, q1
[^>]*> ef04 e44e 	vshl.s8	q7, q7, q2
[^>]*> ef08 e44e 	vshl.s8	q7, q7, q4
[^>]*> ef0e e44e 	vshl.s8	q7, q7, q7
[^>]*> ee31 fe60 	vshl.s8	q7, r0
[^>]*> ee31 fe61 	vshl.s8	q7, r1
[^>]*> ee31 fe62 	vshl.s8	q7, r2
[^>]*> ee31 fe64 	vshl.s8	q7, r4
[^>]*> ee31 fe67 	vshl.s8	q7, r7
[^>]*> ee31 fe68 	vshl.s8	q7, r8
[^>]*> ee31 fe6a 	vshl.s8	q7, sl
[^>]*> ee31 fe6c 	vshl.s8	q7, ip
[^>]*> ee31 fe6e 	vshl.s8	q7, lr
[^>]*> ff10 0440 	vshl.u16	q0, q0, q0
[^>]*> ff12 0440 	vshl.u16	q0, q0, q1
[^>]*> ff14 0440 	vshl.u16	q0, q0, q2
[^>]*> ff18 0440 	vshl.u16	q0, q0, q4
[^>]*> ff1e 0440 	vshl.u16	q0, q0, q7
[^>]*> ff10 0442 	vshl.u16	q0, q1, q0
[^>]*> ff12 0442 	vshl.u16	q0, q1, q1
[^>]*> ff14 0442 	vshl.u16	q0, q1, q2
[^>]*> ff18 0442 	vshl.u16	q0, q1, q4
[^>]*> ff1e 0442 	vshl.u16	q0, q1, q7
[^>]*> ff10 0444 	vshl.u16	q0, q2, q0
[^>]*> ff12 0444 	vshl.u16	q0, q2, q1
[^>]*> ff14 0444 	vshl.u16	q0, q2, q2
[^>]*> ff18 0444 	vshl.u16	q0, q2, q4
[^>]*> ff1e 0444 	vshl.u16	q0, q2, q7
[^>]*> ff10 0448 	vshl.u16	q0, q4, q0
[^>]*> ff12 0448 	vshl.u16	q0, q4, q1
[^>]*> ff14 0448 	vshl.u16	q0, q4, q2
[^>]*> ff18 0448 	vshl.u16	q0, q4, q4
[^>]*> ff1e 0448 	vshl.u16	q0, q4, q7
[^>]*> ff10 044e 	vshl.u16	q0, q7, q0
[^>]*> ff12 044e 	vshl.u16	q0, q7, q1
[^>]*> ff14 044e 	vshl.u16	q0, q7, q2
[^>]*> ff18 044e 	vshl.u16	q0, q7, q4
[^>]*> ff1e 044e 	vshl.u16	q0, q7, q7
[^>]*> fe35 1e60 	vshl.u16	q0, r0
[^>]*> fe35 1e61 	vshl.u16	q0, r1
[^>]*> fe35 1e62 	vshl.u16	q0, r2
[^>]*> fe35 1e64 	vshl.u16	q0, r4
[^>]*> fe35 1e67 	vshl.u16	q0, r7
[^>]*> fe35 1e68 	vshl.u16	q0, r8
[^>]*> fe35 1e6a 	vshl.u16	q0, sl
[^>]*> fe35 1e6c 	vshl.u16	q0, ip
[^>]*> fe35 1e6e 	vshl.u16	q0, lr
[^>]*> ff10 2440 	vshl.u16	q1, q0, q0
[^>]*> ff12 2440 	vshl.u16	q1, q0, q1
[^>]*> ff14 2440 	vshl.u16	q1, q0, q2
[^>]*> ff18 2440 	vshl.u16	q1, q0, q4
[^>]*> ff1e 2440 	vshl.u16	q1, q0, q7
[^>]*> ff10 2442 	vshl.u16	q1, q1, q0
[^>]*> ff12 2442 	vshl.u16	q1, q1, q1
[^>]*> ff14 2442 	vshl.u16	q1, q1, q2
[^>]*> ff18 2442 	vshl.u16	q1, q1, q4
[^>]*> ff1e 2442 	vshl.u16	q1, q1, q7
[^>]*> ff10 2444 	vshl.u16	q1, q2, q0
[^>]*> ff12 2444 	vshl.u16	q1, q2, q1
[^>]*> ff14 2444 	vshl.u16	q1, q2, q2
[^>]*> ff18 2444 	vshl.u16	q1, q2, q4
[^>]*> ff1e 2444 	vshl.u16	q1, q2, q7
[^>]*> ff10 2448 	vshl.u16	q1, q4, q0
[^>]*> ff12 2448 	vshl.u16	q1, q4, q1
[^>]*> ff14 2448 	vshl.u16	q1, q4, q2
[^>]*> ff18 2448 	vshl.u16	q1, q4, q4
[^>]*> ff1e 2448 	vshl.u16	q1, q4, q7
[^>]*> ff10 244e 	vshl.u16	q1, q7, q0
[^>]*> ff12 244e 	vshl.u16	q1, q7, q1
[^>]*> ff14 244e 	vshl.u16	q1, q7, q2
[^>]*> ff18 244e 	vshl.u16	q1, q7, q4
[^>]*> ff1e 244e 	vshl.u16	q1, q7, q7
[^>]*> fe35 3e60 	vshl.u16	q1, r0
[^>]*> fe35 3e61 	vshl.u16	q1, r1
[^>]*> fe35 3e62 	vshl.u16	q1, r2
[^>]*> fe35 3e64 	vshl.u16	q1, r4
[^>]*> fe35 3e67 	vshl.u16	q1, r7
[^>]*> fe35 3e68 	vshl.u16	q1, r8
[^>]*> fe35 3e6a 	vshl.u16	q1, sl
[^>]*> fe35 3e6c 	vshl.u16	q1, ip
[^>]*> fe35 3e6e 	vshl.u16	q1, lr
[^>]*> ff10 4440 	vshl.u16	q2, q0, q0
[^>]*> ff12 4440 	vshl.u16	q2, q0, q1
[^>]*> ff14 4440 	vshl.u16	q2, q0, q2
[^>]*> ff18 4440 	vshl.u16	q2, q0, q4
[^>]*> ff1e 4440 	vshl.u16	q2, q0, q7
[^>]*> ff10 4442 	vshl.u16	q2, q1, q0
[^>]*> ff12 4442 	vshl.u16	q2, q1, q1
[^>]*> ff14 4442 	vshl.u16	q2, q1, q2
[^>]*> ff18 4442 	vshl.u16	q2, q1, q4
[^>]*> ff1e 4442 	vshl.u16	q2, q1, q7
[^>]*> ff10 4444 	vshl.u16	q2, q2, q0
[^>]*> ff12 4444 	vshl.u16	q2, q2, q1
[^>]*> ff14 4444 	vshl.u16	q2, q2, q2
[^>]*> ff18 4444 	vshl.u16	q2, q2, q4
[^>]*> ff1e 4444 	vshl.u16	q2, q2, q7
[^>]*> ff10 4448 	vshl.u16	q2, q4, q0
[^>]*> ff12 4448 	vshl.u16	q2, q4, q1
[^>]*> ff14 4448 	vshl.u16	q2, q4, q2
[^>]*> ff18 4448 	vshl.u16	q2, q4, q4
[^>]*> ff1e 4448 	vshl.u16	q2, q4, q7
[^>]*> ff10 444e 	vshl.u16	q2, q7, q0
[^>]*> ff12 444e 	vshl.u16	q2, q7, q1
[^>]*> ff14 444e 	vshl.u16	q2, q7, q2
[^>]*> ff18 444e 	vshl.u16	q2, q7, q4
[^>]*> ff1e 444e 	vshl.u16	q2, q7, q7
[^>]*> fe35 5e60 	vshl.u16	q2, r0
[^>]*> fe35 5e61 	vshl.u16	q2, r1
[^>]*> fe35 5e62 	vshl.u16	q2, r2
[^>]*> fe35 5e64 	vshl.u16	q2, r4
[^>]*> fe35 5e67 	vshl.u16	q2, r7
[^>]*> fe35 5e68 	vshl.u16	q2, r8
[^>]*> fe35 5e6a 	vshl.u16	q2, sl
[^>]*> fe35 5e6c 	vshl.u16	q2, ip
[^>]*> fe35 5e6e 	vshl.u16	q2, lr
[^>]*> ff10 8440 	vshl.u16	q4, q0, q0
[^>]*> ff12 8440 	vshl.u16	q4, q0, q1
[^>]*> ff14 8440 	vshl.u16	q4, q0, q2
[^>]*> ff18 8440 	vshl.u16	q4, q0, q4
[^>]*> ff1e 8440 	vshl.u16	q4, q0, q7
[^>]*> ff10 8442 	vshl.u16	q4, q1, q0
[^>]*> ff12 8442 	vshl.u16	q4, q1, q1
[^>]*> ff14 8442 	vshl.u16	q4, q1, q2
[^>]*> ff18 8442 	vshl.u16	q4, q1, q4
[^>]*> ff1e 8442 	vshl.u16	q4, q1, q7
[^>]*> ff10 8444 	vshl.u16	q4, q2, q0
[^>]*> ff12 8444 	vshl.u16	q4, q2, q1
[^>]*> ff14 8444 	vshl.u16	q4, q2, q2
[^>]*> ff18 8444 	vshl.u16	q4, q2, q4
[^>]*> ff1e 8444 	vshl.u16	q4, q2, q7
[^>]*> ff10 8448 	vshl.u16	q4, q4, q0
[^>]*> ff12 8448 	vshl.u16	q4, q4, q1
[^>]*> ff14 8448 	vshl.u16	q4, q4, q2
[^>]*> ff18 8448 	vshl.u16	q4, q4, q4
[^>]*> ff1e 8448 	vshl.u16	q4, q4, q7
[^>]*> ff10 844e 	vshl.u16	q4, q7, q0
[^>]*> ff12 844e 	vshl.u16	q4, q7, q1
[^>]*> ff14 844e 	vshl.u16	q4, q7, q2
[^>]*> ff18 844e 	vshl.u16	q4, q7, q4
[^>]*> ff1e 844e 	vshl.u16	q4, q7, q7
[^>]*> fe35 9e60 	vshl.u16	q4, r0
[^>]*> fe35 9e61 	vshl.u16	q4, r1
[^>]*> fe35 9e62 	vshl.u16	q4, r2
[^>]*> fe35 9e64 	vshl.u16	q4, r4
[^>]*> fe35 9e67 	vshl.u16	q4, r7
[^>]*> fe35 9e68 	vshl.u16	q4, r8
[^>]*> fe35 9e6a 	vshl.u16	q4, sl
[^>]*> fe35 9e6c 	vshl.u16	q4, ip
[^>]*> fe35 9e6e 	vshl.u16	q4, lr
[^>]*> ff10 e440 	vshl.u16	q7, q0, q0
[^>]*> ff12 e440 	vshl.u16	q7, q0, q1
[^>]*> ff14 e440 	vshl.u16	q7, q0, q2
[^>]*> ff18 e440 	vshl.u16	q7, q0, q4
[^>]*> ff1e e440 	vshl.u16	q7, q0, q7
[^>]*> ff10 e442 	vshl.u16	q7, q1, q0
[^>]*> ff12 e442 	vshl.u16	q7, q1, q1
[^>]*> ff14 e442 	vshl.u16	q7, q1, q2
[^>]*> ff18 e442 	vshl.u16	q7, q1, q4
[^>]*> ff1e e442 	vshl.u16	q7, q1, q7
[^>]*> ff10 e444 	vshl.u16	q7, q2, q0
[^>]*> ff12 e444 	vshl.u16	q7, q2, q1
[^>]*> ff14 e444 	vshl.u16	q7, q2, q2
[^>]*> ff18 e444 	vshl.u16	q7, q2, q4
[^>]*> ff1e e444 	vshl.u16	q7, q2, q7
[^>]*> ff10 e448 	vshl.u16	q7, q4, q0
[^>]*> ff12 e448 	vshl.u16	q7, q4, q1
[^>]*> ff14 e448 	vshl.u16	q7, q4, q2
[^>]*> ff18 e448 	vshl.u16	q7, q4, q4
[^>]*> ff1e e448 	vshl.u16	q7, q4, q7
[^>]*> ff10 e44e 	vshl.u16	q7, q7, q0
[^>]*> ff12 e44e 	vshl.u16	q7, q7, q1
[^>]*> ff14 e44e 	vshl.u16	q7, q7, q2
[^>]*> ff18 e44e 	vshl.u16	q7, q7, q4
[^>]*> ff1e e44e 	vshl.u16	q7, q7, q7
[^>]*> fe35 fe60 	vshl.u16	q7, r0
[^>]*> fe35 fe61 	vshl.u16	q7, r1
[^>]*> fe35 fe62 	vshl.u16	q7, r2
[^>]*> fe35 fe64 	vshl.u16	q7, r4
[^>]*> fe35 fe67 	vshl.u16	q7, r7
[^>]*> fe35 fe68 	vshl.u16	q7, r8
[^>]*> fe35 fe6a 	vshl.u16	q7, sl
[^>]*> fe35 fe6c 	vshl.u16	q7, ip
[^>]*> fe35 fe6e 	vshl.u16	q7, lr
[^>]*> ef10 0440 	vshl.s16	q0, q0, q0
[^>]*> ef12 0440 	vshl.s16	q0, q0, q1
[^>]*> ef14 0440 	vshl.s16	q0, q0, q2
[^>]*> ef18 0440 	vshl.s16	q0, q0, q4
[^>]*> ef1e 0440 	vshl.s16	q0, q0, q7
[^>]*> ef10 0442 	vshl.s16	q0, q1, q0
[^>]*> ef12 0442 	vshl.s16	q0, q1, q1
[^>]*> ef14 0442 	vshl.s16	q0, q1, q2
[^>]*> ef18 0442 	vshl.s16	q0, q1, q4
[^>]*> ef1e 0442 	vshl.s16	q0, q1, q7
[^>]*> ef10 0444 	vshl.s16	q0, q2, q0
[^>]*> ef12 0444 	vshl.s16	q0, q2, q1
[^>]*> ef14 0444 	vshl.s16	q0, q2, q2
[^>]*> ef18 0444 	vshl.s16	q0, q2, q4
[^>]*> ef1e 0444 	vshl.s16	q0, q2, q7
[^>]*> ef10 0448 	vshl.s16	q0, q4, q0
[^>]*> ef12 0448 	vshl.s16	q0, q4, q1
[^>]*> ef14 0448 	vshl.s16	q0, q4, q2
[^>]*> ef18 0448 	vshl.s16	q0, q4, q4
[^>]*> ef1e 0448 	vshl.s16	q0, q4, q7
[^>]*> ef10 044e 	vshl.s16	q0, q7, q0
[^>]*> ef12 044e 	vshl.s16	q0, q7, q1
[^>]*> ef14 044e 	vshl.s16	q0, q7, q2
[^>]*> ef18 044e 	vshl.s16	q0, q7, q4
[^>]*> ef1e 044e 	vshl.s16	q0, q7, q7
[^>]*> ee35 1e60 	vshl.s16	q0, r0
[^>]*> ee35 1e61 	vshl.s16	q0, r1
[^>]*> ee35 1e62 	vshl.s16	q0, r2
[^>]*> ee35 1e64 	vshl.s16	q0, r4
[^>]*> ee35 1e67 	vshl.s16	q0, r7
[^>]*> ee35 1e68 	vshl.s16	q0, r8
[^>]*> ee35 1e6a 	vshl.s16	q0, sl
[^>]*> ee35 1e6c 	vshl.s16	q0, ip
[^>]*> ee35 1e6e 	vshl.s16	q0, lr
[^>]*> ef10 2440 	vshl.s16	q1, q0, q0
[^>]*> ef12 2440 	vshl.s16	q1, q0, q1
[^>]*> ef14 2440 	vshl.s16	q1, q0, q2
[^>]*> ef18 2440 	vshl.s16	q1, q0, q4
[^>]*> ef1e 2440 	vshl.s16	q1, q0, q7
[^>]*> ef10 2442 	vshl.s16	q1, q1, q0
[^>]*> ef12 2442 	vshl.s16	q1, q1, q1
[^>]*> ef14 2442 	vshl.s16	q1, q1, q2
[^>]*> ef18 2442 	vshl.s16	q1, q1, q4
[^>]*> ef1e 2442 	vshl.s16	q1, q1, q7
[^>]*> ef10 2444 	vshl.s16	q1, q2, q0
[^>]*> ef12 2444 	vshl.s16	q1, q2, q1
[^>]*> ef14 2444 	vshl.s16	q1, q2, q2
[^>]*> ef18 2444 	vshl.s16	q1, q2, q4
[^>]*> ef1e 2444 	vshl.s16	q1, q2, q7
[^>]*> ef10 2448 	vshl.s16	q1, q4, q0
[^>]*> ef12 2448 	vshl.s16	q1, q4, q1
[^>]*> ef14 2448 	vshl.s16	q1, q4, q2
[^>]*> ef18 2448 	vshl.s16	q1, q4, q4
[^>]*> ef1e 2448 	vshl.s16	q1, q4, q7
[^>]*> ef10 244e 	vshl.s16	q1, q7, q0
[^>]*> ef12 244e 	vshl.s16	q1, q7, q1
[^>]*> ef14 244e 	vshl.s16	q1, q7, q2
[^>]*> ef18 244e 	vshl.s16	q1, q7, q4
[^>]*> ef1e 244e 	vshl.s16	q1, q7, q7
[^>]*> ee35 3e60 	vshl.s16	q1, r0
[^>]*> ee35 3e61 	vshl.s16	q1, r1
[^>]*> ee35 3e62 	vshl.s16	q1, r2
[^>]*> ee35 3e64 	vshl.s16	q1, r4
[^>]*> ee35 3e67 	vshl.s16	q1, r7
[^>]*> ee35 3e68 	vshl.s16	q1, r8
[^>]*> ee35 3e6a 	vshl.s16	q1, sl
[^>]*> ee35 3e6c 	vshl.s16	q1, ip
[^>]*> ee35 3e6e 	vshl.s16	q1, lr
[^>]*> ef10 4440 	vshl.s16	q2, q0, q0
[^>]*> ef12 4440 	vshl.s16	q2, q0, q1
[^>]*> ef14 4440 	vshl.s16	q2, q0, q2
[^>]*> ef18 4440 	vshl.s16	q2, q0, q4
[^>]*> ef1e 4440 	vshl.s16	q2, q0, q7
[^>]*> ef10 4442 	vshl.s16	q2, q1, q0
[^>]*> ef12 4442 	vshl.s16	q2, q1, q1
[^>]*> ef14 4442 	vshl.s16	q2, q1, q2
[^>]*> ef18 4442 	vshl.s16	q2, q1, q4
[^>]*> ef1e 4442 	vshl.s16	q2, q1, q7
[^>]*> ef10 4444 	vshl.s16	q2, q2, q0
[^>]*> ef12 4444 	vshl.s16	q2, q2, q1
[^>]*> ef14 4444 	vshl.s16	q2, q2, q2
[^>]*> ef18 4444 	vshl.s16	q2, q2, q4
[^>]*> ef1e 4444 	vshl.s16	q2, q2, q7
[^>]*> ef10 4448 	vshl.s16	q2, q4, q0
[^>]*> ef12 4448 	vshl.s16	q2, q4, q1
[^>]*> ef14 4448 	vshl.s16	q2, q4, q2
[^>]*> ef18 4448 	vshl.s16	q2, q4, q4
[^>]*> ef1e 4448 	vshl.s16	q2, q4, q7
[^>]*> ef10 444e 	vshl.s16	q2, q7, q0
[^>]*> ef12 444e 	vshl.s16	q2, q7, q1
[^>]*> ef14 444e 	vshl.s16	q2, q7, q2
[^>]*> ef18 444e 	vshl.s16	q2, q7, q4
[^>]*> ef1e 444e 	vshl.s16	q2, q7, q7
[^>]*> ee35 5e60 	vshl.s16	q2, r0
[^>]*> ee35 5e61 	vshl.s16	q2, r1
[^>]*> ee35 5e62 	vshl.s16	q2, r2
[^>]*> ee35 5e64 	vshl.s16	q2, r4
[^>]*> ee35 5e67 	vshl.s16	q2, r7
[^>]*> ee35 5e68 	vshl.s16	q2, r8
[^>]*> ee35 5e6a 	vshl.s16	q2, sl
[^>]*> ee35 5e6c 	vshl.s16	q2, ip
[^>]*> ee35 5e6e 	vshl.s16	q2, lr
[^>]*> ef10 8440 	vshl.s16	q4, q0, q0
[^>]*> ef12 8440 	vshl.s16	q4, q0, q1
[^>]*> ef14 8440 	vshl.s16	q4, q0, q2
[^>]*> ef18 8440 	vshl.s16	q4, q0, q4
[^>]*> ef1e 8440 	vshl.s16	q4, q0, q7
[^>]*> ef10 8442 	vshl.s16	q4, q1, q0
[^>]*> ef12 8442 	vshl.s16	q4, q1, q1
[^>]*> ef14 8442 	vshl.s16	q4, q1, q2
[^>]*> ef18 8442 	vshl.s16	q4, q1, q4
[^>]*> ef1e 8442 	vshl.s16	q4, q1, q7
[^>]*> ef10 8444 	vshl.s16	q4, q2, q0
[^>]*> ef12 8444 	vshl.s16	q4, q2, q1
[^>]*> ef14 8444 	vshl.s16	q4, q2, q2
[^>]*> ef18 8444 	vshl.s16	q4, q2, q4
[^>]*> ef1e 8444 	vshl.s16	q4, q2, q7
[^>]*> ef10 8448 	vshl.s16	q4, q4, q0
[^>]*> ef12 8448 	vshl.s16	q4, q4, q1
[^>]*> ef14 8448 	vshl.s16	q4, q4, q2
[^>]*> ef18 8448 	vshl.s16	q4, q4, q4
[^>]*> ef1e 8448 	vshl.s16	q4, q4, q7
[^>]*> ef10 844e 	vshl.s16	q4, q7, q0
[^>]*> ef12 844e 	vshl.s16	q4, q7, q1
[^>]*> ef14 844e 	vshl.s16	q4, q7, q2
[^>]*> ef18 844e 	vshl.s16	q4, q7, q4
[^>]*> ef1e 844e 	vshl.s16	q4, q7, q7
[^>]*> ee35 9e60 	vshl.s16	q4, r0
[^>]*> ee35 9e61 	vshl.s16	q4, r1
[^>]*> ee35 9e62 	vshl.s16	q4, r2
[^>]*> ee35 9e64 	vshl.s16	q4, r4
[^>]*> ee35 9e67 	vshl.s16	q4, r7
[^>]*> ee35 9e68 	vshl.s16	q4, r8
[^>]*> ee35 9e6a 	vshl.s16	q4, sl
[^>]*> ee35 9e6c 	vshl.s16	q4, ip
[^>]*> ee35 9e6e 	vshl.s16	q4, lr
[^>]*> ef10 e440 	vshl.s16	q7, q0, q0
[^>]*> ef12 e440 	vshl.s16	q7, q0, q1
[^>]*> ef14 e440 	vshl.s16	q7, q0, q2
[^>]*> ef18 e440 	vshl.s16	q7, q0, q4
[^>]*> ef1e e440 	vshl.s16	q7, q0, q7
[^>]*> ef10 e442 	vshl.s16	q7, q1, q0
[^>]*> ef12 e442 	vshl.s16	q7, q1, q1
[^>]*> ef14 e442 	vshl.s16	q7, q1, q2
[^>]*> ef18 e442 	vshl.s16	q7, q1, q4
[^>]*> ef1e e442 	vshl.s16	q7, q1, q7
[^>]*> ef10 e444 	vshl.s16	q7, q2, q0
[^>]*> ef12 e444 	vshl.s16	q7, q2, q1
[^>]*> ef14 e444 	vshl.s16	q7, q2, q2
[^>]*> ef18 e444 	vshl.s16	q7, q2, q4
[^>]*> ef1e e444 	vshl.s16	q7, q2, q7
[^>]*> ef10 e448 	vshl.s16	q7, q4, q0
[^>]*> ef12 e448 	vshl.s16	q7, q4, q1
[^>]*> ef14 e448 	vshl.s16	q7, q4, q2
[^>]*> ef18 e448 	vshl.s16	q7, q4, q4
[^>]*> ef1e e448 	vshl.s16	q7, q4, q7
[^>]*> ef10 e44e 	vshl.s16	q7, q7, q0
[^>]*> ef12 e44e 	vshl.s16	q7, q7, q1
[^>]*> ef14 e44e 	vshl.s16	q7, q7, q2
[^>]*> ef18 e44e 	vshl.s16	q7, q7, q4
[^>]*> ef1e e44e 	vshl.s16	q7, q7, q7
[^>]*> ee35 fe60 	vshl.s16	q7, r0
[^>]*> ee35 fe61 	vshl.s16	q7, r1
[^>]*> ee35 fe62 	vshl.s16	q7, r2
[^>]*> ee35 fe64 	vshl.s16	q7, r4
[^>]*> ee35 fe67 	vshl.s16	q7, r7
[^>]*> ee35 fe68 	vshl.s16	q7, r8
[^>]*> ee35 fe6a 	vshl.s16	q7, sl
[^>]*> ee35 fe6c 	vshl.s16	q7, ip
[^>]*> ee35 fe6e 	vshl.s16	q7, lr
[^>]*> ff20 0440 	vshl.u32	q0, q0, q0
[^>]*> ff22 0440 	vshl.u32	q0, q0, q1
[^>]*> ff24 0440 	vshl.u32	q0, q0, q2
[^>]*> ff28 0440 	vshl.u32	q0, q0, q4
[^>]*> ff2e 0440 	vshl.u32	q0, q0, q7
[^>]*> ff20 0442 	vshl.u32	q0, q1, q0
[^>]*> ff22 0442 	vshl.u32	q0, q1, q1
[^>]*> ff24 0442 	vshl.u32	q0, q1, q2
[^>]*> ff28 0442 	vshl.u32	q0, q1, q4
[^>]*> ff2e 0442 	vshl.u32	q0, q1, q7
[^>]*> ff20 0444 	vshl.u32	q0, q2, q0
[^>]*> ff22 0444 	vshl.u32	q0, q2, q1
[^>]*> ff24 0444 	vshl.u32	q0, q2, q2
[^>]*> ff28 0444 	vshl.u32	q0, q2, q4
[^>]*> ff2e 0444 	vshl.u32	q0, q2, q7
[^>]*> ff20 0448 	vshl.u32	q0, q4, q0
[^>]*> ff22 0448 	vshl.u32	q0, q4, q1
[^>]*> ff24 0448 	vshl.u32	q0, q4, q2
[^>]*> ff28 0448 	vshl.u32	q0, q4, q4
[^>]*> ff2e 0448 	vshl.u32	q0, q4, q7
[^>]*> ff20 044e 	vshl.u32	q0, q7, q0
[^>]*> ff22 044e 	vshl.u32	q0, q7, q1
[^>]*> ff24 044e 	vshl.u32	q0, q7, q2
[^>]*> ff28 044e 	vshl.u32	q0, q7, q4
[^>]*> ff2e 044e 	vshl.u32	q0, q7, q7
[^>]*> fe39 1e60 	vshl.u32	q0, r0
[^>]*> fe39 1e61 	vshl.u32	q0, r1
[^>]*> fe39 1e62 	vshl.u32	q0, r2
[^>]*> fe39 1e64 	vshl.u32	q0, r4
[^>]*> fe39 1e67 	vshl.u32	q0, r7
[^>]*> fe39 1e68 	vshl.u32	q0, r8
[^>]*> fe39 1e6a 	vshl.u32	q0, sl
[^>]*> fe39 1e6c 	vshl.u32	q0, ip
[^>]*> fe39 1e6e 	vshl.u32	q0, lr
[^>]*> ff20 2440 	vshl.u32	q1, q0, q0
[^>]*> ff22 2440 	vshl.u32	q1, q0, q1
[^>]*> ff24 2440 	vshl.u32	q1, q0, q2
[^>]*> ff28 2440 	vshl.u32	q1, q0, q4
[^>]*> ff2e 2440 	vshl.u32	q1, q0, q7
[^>]*> ff20 2442 	vshl.u32	q1, q1, q0
[^>]*> ff22 2442 	vshl.u32	q1, q1, q1
[^>]*> ff24 2442 	vshl.u32	q1, q1, q2
[^>]*> ff28 2442 	vshl.u32	q1, q1, q4
[^>]*> ff2e 2442 	vshl.u32	q1, q1, q7
[^>]*> ff20 2444 	vshl.u32	q1, q2, q0
[^>]*> ff22 2444 	vshl.u32	q1, q2, q1
[^>]*> ff24 2444 	vshl.u32	q1, q2, q2
[^>]*> ff28 2444 	vshl.u32	q1, q2, q4
[^>]*> ff2e 2444 	vshl.u32	q1, q2, q7
[^>]*> ff20 2448 	vshl.u32	q1, q4, q0
[^>]*> ff22 2448 	vshl.u32	q1, q4, q1
[^>]*> ff24 2448 	vshl.u32	q1, q4, q2
[^>]*> ff28 2448 	vshl.u32	q1, q4, q4
[^>]*> ff2e 2448 	vshl.u32	q1, q4, q7
[^>]*> ff20 244e 	vshl.u32	q1, q7, q0
[^>]*> ff22 244e 	vshl.u32	q1, q7, q1
[^>]*> ff24 244e 	vshl.u32	q1, q7, q2
[^>]*> ff28 244e 	vshl.u32	q1, q7, q4
[^>]*> ff2e 244e 	vshl.u32	q1, q7, q7
[^>]*> fe39 3e60 	vshl.u32	q1, r0
[^>]*> fe39 3e61 	vshl.u32	q1, r1
[^>]*> fe39 3e62 	vshl.u32	q1, r2
[^>]*> fe39 3e64 	vshl.u32	q1, r4
[^>]*> fe39 3e67 	vshl.u32	q1, r7
[^>]*> fe39 3e68 	vshl.u32	q1, r8
[^>]*> fe39 3e6a 	vshl.u32	q1, sl
[^>]*> fe39 3e6c 	vshl.u32	q1, ip
[^>]*> fe39 3e6e 	vshl.u32	q1, lr
[^>]*> ff20 4440 	vshl.u32	q2, q0, q0
[^>]*> ff22 4440 	vshl.u32	q2, q0, q1
[^>]*> ff24 4440 	vshl.u32	q2, q0, q2
[^>]*> ff28 4440 	vshl.u32	q2, q0, q4
[^>]*> ff2e 4440 	vshl.u32	q2, q0, q7
[^>]*> ff20 4442 	vshl.u32	q2, q1, q0
[^>]*> ff22 4442 	vshl.u32	q2, q1, q1
[^>]*> ff24 4442 	vshl.u32	q2, q1, q2
[^>]*> ff28 4442 	vshl.u32	q2, q1, q4
[^>]*> ff2e 4442 	vshl.u32	q2, q1, q7
[^>]*> ff20 4444 	vshl.u32	q2, q2, q0
[^>]*> ff22 4444 	vshl.u32	q2, q2, q1
[^>]*> ff24 4444 	vshl.u32	q2, q2, q2
[^>]*> ff28 4444 	vshl.u32	q2, q2, q4
[^>]*> ff2e 4444 	vshl.u32	q2, q2, q7
[^>]*> ff20 4448 	vshl.u32	q2, q4, q0
[^>]*> ff22 4448 	vshl.u32	q2, q4, q1
[^>]*> ff24 4448 	vshl.u32	q2, q4, q2
[^>]*> ff28 4448 	vshl.u32	q2, q4, q4
[^>]*> ff2e 4448 	vshl.u32	q2, q4, q7
[^>]*> ff20 444e 	vshl.u32	q2, q7, q0
[^>]*> ff22 444e 	vshl.u32	q2, q7, q1
[^>]*> ff24 444e 	vshl.u32	q2, q7, q2
[^>]*> ff28 444e 	vshl.u32	q2, q7, q4
[^>]*> ff2e 444e 	vshl.u32	q2, q7, q7
[^>]*> fe39 5e60 	vshl.u32	q2, r0
[^>]*> fe39 5e61 	vshl.u32	q2, r1
[^>]*> fe39 5e62 	vshl.u32	q2, r2
[^>]*> fe39 5e64 	vshl.u32	q2, r4
[^>]*> fe39 5e67 	vshl.u32	q2, r7
[^>]*> fe39 5e68 	vshl.u32	q2, r8
[^>]*> fe39 5e6a 	vshl.u32	q2, sl
[^>]*> fe39 5e6c 	vshl.u32	q2, ip
[^>]*> fe39 5e6e 	vshl.u32	q2, lr
[^>]*> ff20 8440 	vshl.u32	q4, q0, q0
[^>]*> ff22 8440 	vshl.u32	q4, q0, q1
[^>]*> ff24 8440 	vshl.u32	q4, q0, q2
[^>]*> ff28 8440 	vshl.u32	q4, q0, q4
[^>]*> ff2e 8440 	vshl.u32	q4, q0, q7
[^>]*> ff20 8442 	vshl.u32	q4, q1, q0
[^>]*> ff22 8442 	vshl.u32	q4, q1, q1
[^>]*> ff24 8442 	vshl.u32	q4, q1, q2
[^>]*> ff28 8442 	vshl.u32	q4, q1, q4
[^>]*> ff2e 8442 	vshl.u32	q4, q1, q7
[^>]*> ff20 8444 	vshl.u32	q4, q2, q0
[^>]*> ff22 8444 	vshl.u32	q4, q2, q1
[^>]*> ff24 8444 	vshl.u32	q4, q2, q2
[^>]*> ff28 8444 	vshl.u32	q4, q2, q4
[^>]*> ff2e 8444 	vshl.u32	q4, q2, q7
[^>]*> ff20 8448 	vshl.u32	q4, q4, q0
[^>]*> ff22 8448 	vshl.u32	q4, q4, q1
[^>]*> ff24 8448 	vshl.u32	q4, q4, q2
[^>]*> ff28 8448 	vshl.u32	q4, q4, q4
[^>]*> ff2e 8448 	vshl.u32	q4, q4, q7
[^>]*> ff20 844e 	vshl.u32	q4, q7, q0
[^>]*> ff22 844e 	vshl.u32	q4, q7, q1
[^>]*> ff24 844e 	vshl.u32	q4, q7, q2
[^>]*> ff28 844e 	vshl.u32	q4, q7, q4
[^>]*> ff2e 844e 	vshl.u32	q4, q7, q7
[^>]*> fe39 9e60 	vshl.u32	q4, r0
[^>]*> fe39 9e61 	vshl.u32	q4, r1
[^>]*> fe39 9e62 	vshl.u32	q4, r2
[^>]*> fe39 9e64 	vshl.u32	q4, r4
[^>]*> fe39 9e67 	vshl.u32	q4, r7
[^>]*> fe39 9e68 	vshl.u32	q4, r8
[^>]*> fe39 9e6a 	vshl.u32	q4, sl
[^>]*> fe39 9e6c 	vshl.u32	q4, ip
[^>]*> fe39 9e6e 	vshl.u32	q4, lr
[^>]*> ff20 e440 	vshl.u32	q7, q0, q0
[^>]*> ff22 e440 	vshl.u32	q7, q0, q1
[^>]*> ff24 e440 	vshl.u32	q7, q0, q2
[^>]*> ff28 e440 	vshl.u32	q7, q0, q4
[^>]*> ff2e e440 	vshl.u32	q7, q0, q7
[^>]*> ff20 e442 	vshl.u32	q7, q1, q0
[^>]*> ff22 e442 	vshl.u32	q7, q1, q1
[^>]*> ff24 e442 	vshl.u32	q7, q1, q2
[^>]*> ff28 e442 	vshl.u32	q7, q1, q4
[^>]*> ff2e e442 	vshl.u32	q7, q1, q7
[^>]*> ff20 e444 	vshl.u32	q7, q2, q0
[^>]*> ff22 e444 	vshl.u32	q7, q2, q1
[^>]*> ff24 e444 	vshl.u32	q7, q2, q2
[^>]*> ff28 e444 	vshl.u32	q7, q2, q4
[^>]*> ff2e e444 	vshl.u32	q7, q2, q7
[^>]*> ff20 e448 	vshl.u32	q7, q4, q0
[^>]*> ff22 e448 	vshl.u32	q7, q4, q1
[^>]*> ff24 e448 	vshl.u32	q7, q4, q2
[^>]*> ff28 e448 	vshl.u32	q7, q4, q4
[^>]*> ff2e e448 	vshl.u32	q7, q4, q7
[^>]*> ff20 e44e 	vshl.u32	q7, q7, q0
[^>]*> ff22 e44e 	vshl.u32	q7, q7, q1
[^>]*> ff24 e44e 	vshl.u32	q7, q7, q2
[^>]*> ff28 e44e 	vshl.u32	q7, q7, q4
[^>]*> ff2e e44e 	vshl.u32	q7, q7, q7
[^>]*> fe39 fe60 	vshl.u32	q7, r0
[^>]*> fe39 fe61 	vshl.u32	q7, r1
[^>]*> fe39 fe62 	vshl.u32	q7, r2
[^>]*> fe39 fe64 	vshl.u32	q7, r4
[^>]*> fe39 fe67 	vshl.u32	q7, r7
[^>]*> fe39 fe68 	vshl.u32	q7, r8
[^>]*> fe39 fe6a 	vshl.u32	q7, sl
[^>]*> fe39 fe6c 	vshl.u32	q7, ip
[^>]*> fe39 fe6e 	vshl.u32	q7, lr
[^>]*> ef20 0440 	vshl.s32	q0, q0, q0
[^>]*> ef22 0440 	vshl.s32	q0, q0, q1
[^>]*> ef24 0440 	vshl.s32	q0, q0, q2
[^>]*> ef28 0440 	vshl.s32	q0, q0, q4
[^>]*> ef2e 0440 	vshl.s32	q0, q0, q7
[^>]*> ef20 0442 	vshl.s32	q0, q1, q0
[^>]*> ef22 0442 	vshl.s32	q0, q1, q1
[^>]*> ef24 0442 	vshl.s32	q0, q1, q2
[^>]*> ef28 0442 	vshl.s32	q0, q1, q4
[^>]*> ef2e 0442 	vshl.s32	q0, q1, q7
[^>]*> ef20 0444 	vshl.s32	q0, q2, q0
[^>]*> ef22 0444 	vshl.s32	q0, q2, q1
[^>]*> ef24 0444 	vshl.s32	q0, q2, q2
[^>]*> ef28 0444 	vshl.s32	q0, q2, q4
[^>]*> ef2e 0444 	vshl.s32	q0, q2, q7
[^>]*> ef20 0448 	vshl.s32	q0, q4, q0
[^>]*> ef22 0448 	vshl.s32	q0, q4, q1
[^>]*> ef24 0448 	vshl.s32	q0, q4, q2
[^>]*> ef28 0448 	vshl.s32	q0, q4, q4
[^>]*> ef2e 0448 	vshl.s32	q0, q4, q7
[^>]*> ef20 044e 	vshl.s32	q0, q7, q0
[^>]*> ef22 044e 	vshl.s32	q0, q7, q1
[^>]*> ef24 044e 	vshl.s32	q0, q7, q2
[^>]*> ef28 044e 	vshl.s32	q0, q7, q4
[^>]*> ef2e 044e 	vshl.s32	q0, q7, q7
[^>]*> ee39 1e60 	vshl.s32	q0, r0
[^>]*> ee39 1e61 	vshl.s32	q0, r1
[^>]*> ee39 1e62 	vshl.s32	q0, r2
[^>]*> ee39 1e64 	vshl.s32	q0, r4
[^>]*> ee39 1e67 	vshl.s32	q0, r7
[^>]*> ee39 1e68 	vshl.s32	q0, r8
[^>]*> ee39 1e6a 	vshl.s32	q0, sl
[^>]*> ee39 1e6c 	vshl.s32	q0, ip
[^>]*> ee39 1e6e 	vshl.s32	q0, lr
[^>]*> ef20 2440 	vshl.s32	q1, q0, q0
[^>]*> ef22 2440 	vshl.s32	q1, q0, q1
[^>]*> ef24 2440 	vshl.s32	q1, q0, q2
[^>]*> ef28 2440 	vshl.s32	q1, q0, q4
[^>]*> ef2e 2440 	vshl.s32	q1, q0, q7
[^>]*> ef20 2442 	vshl.s32	q1, q1, q0
[^>]*> ef22 2442 	vshl.s32	q1, q1, q1
[^>]*> ef24 2442 	vshl.s32	q1, q1, q2
[^>]*> ef28 2442 	vshl.s32	q1, q1, q4
[^>]*> ef2e 2442 	vshl.s32	q1, q1, q7
[^>]*> ef20 2444 	vshl.s32	q1, q2, q0
[^>]*> ef22 2444 	vshl.s32	q1, q2, q1
[^>]*> ef24 2444 	vshl.s32	q1, q2, q2
[^>]*> ef28 2444 	vshl.s32	q1, q2, q4
[^>]*> ef2e 2444 	vshl.s32	q1, q2, q7
[^>]*> ef20 2448 	vshl.s32	q1, q4, q0
[^>]*> ef22 2448 	vshl.s32	q1, q4, q1
[^>]*> ef24 2448 	vshl.s32	q1, q4, q2
[^>]*> ef28 2448 	vshl.s32	q1, q4, q4
[^>]*> ef2e 2448 	vshl.s32	q1, q4, q7
[^>]*> ef20 244e 	vshl.s32	q1, q7, q0
[^>]*> ef22 244e 	vshl.s32	q1, q7, q1
[^>]*> ef24 244e 	vshl.s32	q1, q7, q2
[^>]*> ef28 244e 	vshl.s32	q1, q7, q4
[^>]*> ef2e 244e 	vshl.s32	q1, q7, q7
[^>]*> ee39 3e60 	vshl.s32	q1, r0
[^>]*> ee39 3e61 	vshl.s32	q1, r1
[^>]*> ee39 3e62 	vshl.s32	q1, r2
[^>]*> ee39 3e64 	vshl.s32	q1, r4
[^>]*> ee39 3e67 	vshl.s32	q1, r7
[^>]*> ee39 3e68 	vshl.s32	q1, r8
[^>]*> ee39 3e6a 	vshl.s32	q1, sl
[^>]*> ee39 3e6c 	vshl.s32	q1, ip
[^>]*> ee39 3e6e 	vshl.s32	q1, lr
[^>]*> ef20 4440 	vshl.s32	q2, q0, q0
[^>]*> ef22 4440 	vshl.s32	q2, q0, q1
[^>]*> ef24 4440 	vshl.s32	q2, q0, q2
[^>]*> ef28 4440 	vshl.s32	q2, q0, q4
[^>]*> ef2e 4440 	vshl.s32	q2, q0, q7
[^>]*> ef20 4442 	vshl.s32	q2, q1, q0
[^>]*> ef22 4442 	vshl.s32	q2, q1, q1
[^>]*> ef24 4442 	vshl.s32	q2, q1, q2
[^>]*> ef28 4442 	vshl.s32	q2, q1, q4
[^>]*> ef2e 4442 	vshl.s32	q2, q1, q7
[^>]*> ef20 4444 	vshl.s32	q2, q2, q0
[^>]*> ef22 4444 	vshl.s32	q2, q2, q1
[^>]*> ef24 4444 	vshl.s32	q2, q2, q2
[^>]*> ef28 4444 	vshl.s32	q2, q2, q4
[^>]*> ef2e 4444 	vshl.s32	q2, q2, q7
[^>]*> ef20 4448 	vshl.s32	q2, q4, q0
[^>]*> ef22 4448 	vshl.s32	q2, q4, q1
[^>]*> ef24 4448 	vshl.s32	q2, q4, q2
[^>]*> ef28 4448 	vshl.s32	q2, q4, q4
[^>]*> ef2e 4448 	vshl.s32	q2, q4, q7
[^>]*> ef20 444e 	vshl.s32	q2, q7, q0
[^>]*> ef22 444e 	vshl.s32	q2, q7, q1
[^>]*> ef24 444e 	vshl.s32	q2, q7, q2
[^>]*> ef28 444e 	vshl.s32	q2, q7, q4
[^>]*> ef2e 444e 	vshl.s32	q2, q7, q7
[^>]*> ee39 5e60 	vshl.s32	q2, r0
[^>]*> ee39 5e61 	vshl.s32	q2, r1
[^>]*> ee39 5e62 	vshl.s32	q2, r2
[^>]*> ee39 5e64 	vshl.s32	q2, r4
[^>]*> ee39 5e67 	vshl.s32	q2, r7
[^>]*> ee39 5e68 	vshl.s32	q2, r8
[^>]*> ee39 5e6a 	vshl.s32	q2, sl
[^>]*> ee39 5e6c 	vshl.s32	q2, ip
[^>]*> ee39 5e6e 	vshl.s32	q2, lr
[^>]*> ef20 8440 	vshl.s32	q4, q0, q0
[^>]*> ef22 8440 	vshl.s32	q4, q0, q1
[^>]*> ef24 8440 	vshl.s32	q4, q0, q2
[^>]*> ef28 8440 	vshl.s32	q4, q0, q4
[^>]*> ef2e 8440 	vshl.s32	q4, q0, q7
[^>]*> ef20 8442 	vshl.s32	q4, q1, q0
[^>]*> ef22 8442 	vshl.s32	q4, q1, q1
[^>]*> ef24 8442 	vshl.s32	q4, q1, q2
[^>]*> ef28 8442 	vshl.s32	q4, q1, q4
[^>]*> ef2e 8442 	vshl.s32	q4, q1, q7
[^>]*> ef20 8444 	vshl.s32	q4, q2, q0
[^>]*> ef22 8444 	vshl.s32	q4, q2, q1
[^>]*> ef24 8444 	vshl.s32	q4, q2, q2
[^>]*> ef28 8444 	vshl.s32	q4, q2, q4
[^>]*> ef2e 8444 	vshl.s32	q4, q2, q7
[^>]*> ef20 8448 	vshl.s32	q4, q4, q0
[^>]*> ef22 8448 	vshl.s32	q4, q4, q1
[^>]*> ef24 8448 	vshl.s32	q4, q4, q2
[^>]*> ef28 8448 	vshl.s32	q4, q4, q4
[^>]*> ef2e 8448 	vshl.s32	q4, q4, q7
[^>]*> ef20 844e 	vshl.s32	q4, q7, q0
[^>]*> ef22 844e 	vshl.s32	q4, q7, q1
[^>]*> ef24 844e 	vshl.s32	q4, q7, q2
[^>]*> ef28 844e 	vshl.s32	q4, q7, q4
[^>]*> ef2e 844e 	vshl.s32	q4, q7, q7
[^>]*> ee39 9e60 	vshl.s32	q4, r0
[^>]*> ee39 9e61 	vshl.s32	q4, r1
[^>]*> ee39 9e62 	vshl.s32	q4, r2
[^>]*> ee39 9e64 	vshl.s32	q4, r4
[^>]*> ee39 9e67 	vshl.s32	q4, r7
[^>]*> ee39 9e68 	vshl.s32	q4, r8
[^>]*> ee39 9e6a 	vshl.s32	q4, sl
[^>]*> ee39 9e6c 	vshl.s32	q4, ip
[^>]*> ee39 9e6e 	vshl.s32	q4, lr
[^>]*> ef20 e440 	vshl.s32	q7, q0, q0
[^>]*> ef22 e440 	vshl.s32	q7, q0, q1
[^>]*> ef24 e440 	vshl.s32	q7, q0, q2
[^>]*> ef28 e440 	vshl.s32	q7, q0, q4
[^>]*> ef2e e440 	vshl.s32	q7, q0, q7
[^>]*> ef20 e442 	vshl.s32	q7, q1, q0
[^>]*> ef22 e442 	vshl.s32	q7, q1, q1
[^>]*> ef24 e442 	vshl.s32	q7, q1, q2
[^>]*> ef28 e442 	vshl.s32	q7, q1, q4
[^>]*> ef2e e442 	vshl.s32	q7, q1, q7
[^>]*> ef20 e444 	vshl.s32	q7, q2, q0
[^>]*> ef22 e444 	vshl.s32	q7, q2, q1
[^>]*> ef24 e444 	vshl.s32	q7, q2, q2
[^>]*> ef28 e444 	vshl.s32	q7, q2, q4
[^>]*> ef2e e444 	vshl.s32	q7, q2, q7
[^>]*> ef20 e448 	vshl.s32	q7, q4, q0
[^>]*> ef22 e448 	vshl.s32	q7, q4, q1
[^>]*> ef24 e448 	vshl.s32	q7, q4, q2
[^>]*> ef28 e448 	vshl.s32	q7, q4, q4
[^>]*> ef2e e448 	vshl.s32	q7, q4, q7
[^>]*> ef20 e44e 	vshl.s32	q7, q7, q0
[^>]*> ef22 e44e 	vshl.s32	q7, q7, q1
[^>]*> ef24 e44e 	vshl.s32	q7, q7, q2
[^>]*> ef28 e44e 	vshl.s32	q7, q7, q4
[^>]*> ef2e e44e 	vshl.s32	q7, q7, q7
[^>]*> ee39 fe60 	vshl.s32	q7, r0
[^>]*> ee39 fe61 	vshl.s32	q7, r1
[^>]*> ee39 fe62 	vshl.s32	q7, r2
[^>]*> ee39 fe64 	vshl.s32	q7, r4
[^>]*> ee39 fe67 	vshl.s32	q7, r7
[^>]*> ee39 fe68 	vshl.s32	q7, r8
[^>]*> ee39 fe6a 	vshl.s32	q7, sl
[^>]*> ee39 fe6c 	vshl.s32	q7, ip
[^>]*> ee39 fe6e 	vshl.s32	q7, lr
[^>]*> fe71 ef4d 	vpstete
[^>]*> ef88 0552 	vshlt.i8	q0, q1, #0
[^>]*> ef95 e550 	vshle.i16	q7, q0, #5
[^>]*> ee39 1e64 	vshlt.s32	q0, r4
[^>]*> fe31 be6c 	vshle.u8	q5, ip
[^>]*> fe71 8f4d 	vpste
[^>]*> ef1c 0448 	vshlt.s16	q0, q4, q6
[^>]*> ff2e 444a 	vshle.u32	q2, q5, q7
