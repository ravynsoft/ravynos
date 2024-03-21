# name: MVE vddup, vdwdup, vidup and viwdup instructions
# as: -march=armv8.1-m.main+mve.fp
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ee01 1f6e 	vddup.u8	q0, r0, #1
[^>]*> ee01 0f6e 	vidup.u8	q0, r0, #1
[^>]*> ee01 1f6f 	vddup.u8	q0, r0, #2
[^>]*> ee01 0f6f 	vidup.u8	q0, r0, #2
[^>]*> ee01 1fee 	vddup.u8	q0, r0, #4
[^>]*> ee01 0fee 	vidup.u8	q0, r0, #4
[^>]*> ee01 1fef 	vddup.u8	q0, r0, #8
[^>]*> ee01 0fef 	vidup.u8	q0, r0, #8
[^>]*> ee01 1f60 	vdwdup.u8	q0, r0, r1, #1
[^>]*> ee01 0f60 	viwdup.u8	q0, r0, r1, #1
[^>]*> ee01 1f61 	vdwdup.u8	q0, r0, r1, #2
[^>]*> ee01 0f61 	viwdup.u8	q0, r0, r1, #2
[^>]*> ee01 1fe0 	vdwdup.u8	q0, r0, r1, #4
[^>]*> ee01 0fe0 	viwdup.u8	q0, r0, r1, #4
[^>]*> ee01 1fe1 	vdwdup.u8	q0, r0, r1, #8
[^>]*> ee01 0fe1 	viwdup.u8	q0, r0, r1, #8
[^>]*> ee01 1f62 	vdwdup.u8	q0, r0, r3, #1
[^>]*> ee01 0f62 	viwdup.u8	q0, r0, r3, #1
[^>]*> ee01 1f63 	vdwdup.u8	q0, r0, r3, #2
[^>]*> ee01 0f63 	viwdup.u8	q0, r0, r3, #2
[^>]*> ee01 1fe2 	vdwdup.u8	q0, r0, r3, #4
[^>]*> ee01 0fe2 	viwdup.u8	q0, r0, r3, #4
[^>]*> ee01 1fe3 	vdwdup.u8	q0, r0, r3, #8
[^>]*> ee01 0fe3 	viwdup.u8	q0, r0, r3, #8
[^>]*> ee01 1f64 	vdwdup.u8	q0, r0, r5, #1
[^>]*> ee01 0f64 	viwdup.u8	q0, r0, r5, #1
[^>]*> ee01 1f65 	vdwdup.u8	q0, r0, r5, #2
[^>]*> ee01 0f65 	viwdup.u8	q0, r0, r5, #2
[^>]*> ee01 1fe4 	vdwdup.u8	q0, r0, r5, #4
[^>]*> ee01 0fe4 	viwdup.u8	q0, r0, r5, #4
[^>]*> ee01 1fe5 	vdwdup.u8	q0, r0, r5, #8
[^>]*> ee01 0fe5 	viwdup.u8	q0, r0, r5, #8
[^>]*> ee01 1f66 	vdwdup.u8	q0, r0, r7, #1
[^>]*> ee01 0f66 	viwdup.u8	q0, r0, r7, #1
[^>]*> ee01 1f67 	vdwdup.u8	q0, r0, r7, #2
[^>]*> ee01 0f67 	viwdup.u8	q0, r0, r7, #2
[^>]*> ee01 1fe6 	vdwdup.u8	q0, r0, r7, #4
[^>]*> ee01 0fe6 	viwdup.u8	q0, r0, r7, #4
[^>]*> ee01 1fe7 	vdwdup.u8	q0, r0, r7, #8
[^>]*> ee01 0fe7 	viwdup.u8	q0, r0, r7, #8
[^>]*> ee01 1f68 	vdwdup.u8	q0, r0, r9, #1
[^>]*> ee01 0f68 	viwdup.u8	q0, r0, r9, #1
[^>]*> ee01 1f69 	vdwdup.u8	q0, r0, r9, #2
[^>]*> ee01 0f69 	viwdup.u8	q0, r0, r9, #2
[^>]*> ee01 1fe8 	vdwdup.u8	q0, r0, r9, #4
[^>]*> ee01 0fe8 	viwdup.u8	q0, r0, r9, #4
[^>]*> ee01 1fe9 	vdwdup.u8	q0, r0, r9, #8
[^>]*> ee01 0fe9 	viwdup.u8	q0, r0, r9, #8
[^>]*> ee01 1f6a 	vdwdup.u8	q0, r0, fp, #1
[^>]*> ee01 0f6a 	viwdup.u8	q0, r0, fp, #1
[^>]*> ee01 1f6b 	vdwdup.u8	q0, r0, fp, #2
[^>]*> ee01 0f6b 	viwdup.u8	q0, r0, fp, #2
[^>]*> ee01 1fea 	vdwdup.u8	q0, r0, fp, #4
[^>]*> ee01 0fea 	viwdup.u8	q0, r0, fp, #4
[^>]*> ee01 1feb 	vdwdup.u8	q0, r0, fp, #8
[^>]*> ee01 0feb 	viwdup.u8	q0, r0, fp, #8
[^>]*> ee03 1f6e 	vddup.u8	q0, r2, #1
[^>]*> ee03 0f6e 	vidup.u8	q0, r2, #1
[^>]*> ee03 1f6f 	vddup.u8	q0, r2, #2
[^>]*> ee03 0f6f 	vidup.u8	q0, r2, #2
[^>]*> ee03 1fee 	vddup.u8	q0, r2, #4
[^>]*> ee03 0fee 	vidup.u8	q0, r2, #4
[^>]*> ee03 1fef 	vddup.u8	q0, r2, #8
[^>]*> ee03 0fef 	vidup.u8	q0, r2, #8
[^>]*> ee03 1f60 	vdwdup.u8	q0, r2, r1, #1
[^>]*> ee03 0f60 	viwdup.u8	q0, r2, r1, #1
[^>]*> ee03 1f61 	vdwdup.u8	q0, r2, r1, #2
[^>]*> ee03 0f61 	viwdup.u8	q0, r2, r1, #2
[^>]*> ee03 1fe0 	vdwdup.u8	q0, r2, r1, #4
[^>]*> ee03 0fe0 	viwdup.u8	q0, r2, r1, #4
[^>]*> ee03 1fe1 	vdwdup.u8	q0, r2, r1, #8
[^>]*> ee03 0fe1 	viwdup.u8	q0, r2, r1, #8
[^>]*> ee03 1f62 	vdwdup.u8	q0, r2, r3, #1
[^>]*> ee03 0f62 	viwdup.u8	q0, r2, r3, #1
[^>]*> ee03 1f63 	vdwdup.u8	q0, r2, r3, #2
[^>]*> ee03 0f63 	viwdup.u8	q0, r2, r3, #2
[^>]*> ee03 1fe2 	vdwdup.u8	q0, r2, r3, #4
[^>]*> ee03 0fe2 	viwdup.u8	q0, r2, r3, #4
[^>]*> ee03 1fe3 	vdwdup.u8	q0, r2, r3, #8
[^>]*> ee03 0fe3 	viwdup.u8	q0, r2, r3, #8
[^>]*> ee03 1f64 	vdwdup.u8	q0, r2, r5, #1
[^>]*> ee03 0f64 	viwdup.u8	q0, r2, r5, #1
[^>]*> ee03 1f65 	vdwdup.u8	q0, r2, r5, #2
[^>]*> ee03 0f65 	viwdup.u8	q0, r2, r5, #2
[^>]*> ee03 1fe4 	vdwdup.u8	q0, r2, r5, #4
[^>]*> ee03 0fe4 	viwdup.u8	q0, r2, r5, #4
[^>]*> ee03 1fe5 	vdwdup.u8	q0, r2, r5, #8
[^>]*> ee03 0fe5 	viwdup.u8	q0, r2, r5, #8
[^>]*> ee03 1f66 	vdwdup.u8	q0, r2, r7, #1
[^>]*> ee03 0f66 	viwdup.u8	q0, r2, r7, #1
[^>]*> ee03 1f67 	vdwdup.u8	q0, r2, r7, #2
[^>]*> ee03 0f67 	viwdup.u8	q0, r2, r7, #2
[^>]*> ee03 1fe6 	vdwdup.u8	q0, r2, r7, #4
[^>]*> ee03 0fe6 	viwdup.u8	q0, r2, r7, #4
[^>]*> ee03 1fe7 	vdwdup.u8	q0, r2, r7, #8
[^>]*> ee03 0fe7 	viwdup.u8	q0, r2, r7, #8
[^>]*> ee03 1f68 	vdwdup.u8	q0, r2, r9, #1
[^>]*> ee03 0f68 	viwdup.u8	q0, r2, r9, #1
[^>]*> ee03 1f69 	vdwdup.u8	q0, r2, r9, #2
[^>]*> ee03 0f69 	viwdup.u8	q0, r2, r9, #2
[^>]*> ee03 1fe8 	vdwdup.u8	q0, r2, r9, #4
[^>]*> ee03 0fe8 	viwdup.u8	q0, r2, r9, #4
[^>]*> ee03 1fe9 	vdwdup.u8	q0, r2, r9, #8
[^>]*> ee03 0fe9 	viwdup.u8	q0, r2, r9, #8
[^>]*> ee03 1f6a 	vdwdup.u8	q0, r2, fp, #1
[^>]*> ee03 0f6a 	viwdup.u8	q0, r2, fp, #1
[^>]*> ee03 1f6b 	vdwdup.u8	q0, r2, fp, #2
[^>]*> ee03 0f6b 	viwdup.u8	q0, r2, fp, #2
[^>]*> ee03 1fea 	vdwdup.u8	q0, r2, fp, #4
[^>]*> ee03 0fea 	viwdup.u8	q0, r2, fp, #4
[^>]*> ee03 1feb 	vdwdup.u8	q0, r2, fp, #8
[^>]*> ee03 0feb 	viwdup.u8	q0, r2, fp, #8
[^>]*> ee05 1f6e 	vddup.u8	q0, r4, #1
[^>]*> ee05 0f6e 	vidup.u8	q0, r4, #1
[^>]*> ee05 1f6f 	vddup.u8	q0, r4, #2
[^>]*> ee05 0f6f 	vidup.u8	q0, r4, #2
[^>]*> ee05 1fee 	vddup.u8	q0, r4, #4
[^>]*> ee05 0fee 	vidup.u8	q0, r4, #4
[^>]*> ee05 1fef 	vddup.u8	q0, r4, #8
[^>]*> ee05 0fef 	vidup.u8	q0, r4, #8
[^>]*> ee05 1f60 	vdwdup.u8	q0, r4, r1, #1
[^>]*> ee05 0f60 	viwdup.u8	q0, r4, r1, #1
[^>]*> ee05 1f61 	vdwdup.u8	q0, r4, r1, #2
[^>]*> ee05 0f61 	viwdup.u8	q0, r4, r1, #2
[^>]*> ee05 1fe0 	vdwdup.u8	q0, r4, r1, #4
[^>]*> ee05 0fe0 	viwdup.u8	q0, r4, r1, #4
[^>]*> ee05 1fe1 	vdwdup.u8	q0, r4, r1, #8
[^>]*> ee05 0fe1 	viwdup.u8	q0, r4, r1, #8
[^>]*> ee05 1f62 	vdwdup.u8	q0, r4, r3, #1
[^>]*> ee05 0f62 	viwdup.u8	q0, r4, r3, #1
[^>]*> ee05 1f63 	vdwdup.u8	q0, r4, r3, #2
[^>]*> ee05 0f63 	viwdup.u8	q0, r4, r3, #2
[^>]*> ee05 1fe2 	vdwdup.u8	q0, r4, r3, #4
[^>]*> ee05 0fe2 	viwdup.u8	q0, r4, r3, #4
[^>]*> ee05 1fe3 	vdwdup.u8	q0, r4, r3, #8
[^>]*> ee05 0fe3 	viwdup.u8	q0, r4, r3, #8
[^>]*> ee05 1f64 	vdwdup.u8	q0, r4, r5, #1
[^>]*> ee05 0f64 	viwdup.u8	q0, r4, r5, #1
[^>]*> ee05 1f65 	vdwdup.u8	q0, r4, r5, #2
[^>]*> ee05 0f65 	viwdup.u8	q0, r4, r5, #2
[^>]*> ee05 1fe4 	vdwdup.u8	q0, r4, r5, #4
[^>]*> ee05 0fe4 	viwdup.u8	q0, r4, r5, #4
[^>]*> ee05 1fe5 	vdwdup.u8	q0, r4, r5, #8
[^>]*> ee05 0fe5 	viwdup.u8	q0, r4, r5, #8
[^>]*> ee05 1f66 	vdwdup.u8	q0, r4, r7, #1
[^>]*> ee05 0f66 	viwdup.u8	q0, r4, r7, #1
[^>]*> ee05 1f67 	vdwdup.u8	q0, r4, r7, #2
[^>]*> ee05 0f67 	viwdup.u8	q0, r4, r7, #2
[^>]*> ee05 1fe6 	vdwdup.u8	q0, r4, r7, #4
[^>]*> ee05 0fe6 	viwdup.u8	q0, r4, r7, #4
[^>]*> ee05 1fe7 	vdwdup.u8	q0, r4, r7, #8
[^>]*> ee05 0fe7 	viwdup.u8	q0, r4, r7, #8
[^>]*> ee05 1f68 	vdwdup.u8	q0, r4, r9, #1
[^>]*> ee05 0f68 	viwdup.u8	q0, r4, r9, #1
[^>]*> ee05 1f69 	vdwdup.u8	q0, r4, r9, #2
[^>]*> ee05 0f69 	viwdup.u8	q0, r4, r9, #2
[^>]*> ee05 1fe8 	vdwdup.u8	q0, r4, r9, #4
[^>]*> ee05 0fe8 	viwdup.u8	q0, r4, r9, #4
[^>]*> ee05 1fe9 	vdwdup.u8	q0, r4, r9, #8
[^>]*> ee05 0fe9 	viwdup.u8	q0, r4, r9, #8
[^>]*> ee05 1f6a 	vdwdup.u8	q0, r4, fp, #1
[^>]*> ee05 0f6a 	viwdup.u8	q0, r4, fp, #1
[^>]*> ee05 1f6b 	vdwdup.u8	q0, r4, fp, #2
[^>]*> ee05 0f6b 	viwdup.u8	q0, r4, fp, #2
[^>]*> ee05 1fea 	vdwdup.u8	q0, r4, fp, #4
[^>]*> ee05 0fea 	viwdup.u8	q0, r4, fp, #4
[^>]*> ee05 1feb 	vdwdup.u8	q0, r4, fp, #8
[^>]*> ee05 0feb 	viwdup.u8	q0, r4, fp, #8
[^>]*> ee07 1f6e 	vddup.u8	q0, r6, #1
[^>]*> ee07 0f6e 	vidup.u8	q0, r6, #1
[^>]*> ee07 1f6f 	vddup.u8	q0, r6, #2
[^>]*> ee07 0f6f 	vidup.u8	q0, r6, #2
[^>]*> ee07 1fee 	vddup.u8	q0, r6, #4
[^>]*> ee07 0fee 	vidup.u8	q0, r6, #4
[^>]*> ee07 1fef 	vddup.u8	q0, r6, #8
[^>]*> ee07 0fef 	vidup.u8	q0, r6, #8
[^>]*> ee07 1f60 	vdwdup.u8	q0, r6, r1, #1
[^>]*> ee07 0f60 	viwdup.u8	q0, r6, r1, #1
[^>]*> ee07 1f61 	vdwdup.u8	q0, r6, r1, #2
[^>]*> ee07 0f61 	viwdup.u8	q0, r6, r1, #2
[^>]*> ee07 1fe0 	vdwdup.u8	q0, r6, r1, #4
[^>]*> ee07 0fe0 	viwdup.u8	q0, r6, r1, #4
[^>]*> ee07 1fe1 	vdwdup.u8	q0, r6, r1, #8
[^>]*> ee07 0fe1 	viwdup.u8	q0, r6, r1, #8
[^>]*> ee07 1f62 	vdwdup.u8	q0, r6, r3, #1
[^>]*> ee07 0f62 	viwdup.u8	q0, r6, r3, #1
[^>]*> ee07 1f63 	vdwdup.u8	q0, r6, r3, #2
[^>]*> ee07 0f63 	viwdup.u8	q0, r6, r3, #2
[^>]*> ee07 1fe2 	vdwdup.u8	q0, r6, r3, #4
[^>]*> ee07 0fe2 	viwdup.u8	q0, r6, r3, #4
[^>]*> ee07 1fe3 	vdwdup.u8	q0, r6, r3, #8
[^>]*> ee07 0fe3 	viwdup.u8	q0, r6, r3, #8
[^>]*> ee07 1f64 	vdwdup.u8	q0, r6, r5, #1
[^>]*> ee07 0f64 	viwdup.u8	q0, r6, r5, #1
[^>]*> ee07 1f65 	vdwdup.u8	q0, r6, r5, #2
[^>]*> ee07 0f65 	viwdup.u8	q0, r6, r5, #2
[^>]*> ee07 1fe4 	vdwdup.u8	q0, r6, r5, #4
[^>]*> ee07 0fe4 	viwdup.u8	q0, r6, r5, #4
[^>]*> ee07 1fe5 	vdwdup.u8	q0, r6, r5, #8
[^>]*> ee07 0fe5 	viwdup.u8	q0, r6, r5, #8
[^>]*> ee07 1f66 	vdwdup.u8	q0, r6, r7, #1
[^>]*> ee07 0f66 	viwdup.u8	q0, r6, r7, #1
[^>]*> ee07 1f67 	vdwdup.u8	q0, r6, r7, #2
[^>]*> ee07 0f67 	viwdup.u8	q0, r6, r7, #2
[^>]*> ee07 1fe6 	vdwdup.u8	q0, r6, r7, #4
[^>]*> ee07 0fe6 	viwdup.u8	q0, r6, r7, #4
[^>]*> ee07 1fe7 	vdwdup.u8	q0, r6, r7, #8
[^>]*> ee07 0fe7 	viwdup.u8	q0, r6, r7, #8
[^>]*> ee07 1f68 	vdwdup.u8	q0, r6, r9, #1
[^>]*> ee07 0f68 	viwdup.u8	q0, r6, r9, #1
[^>]*> ee07 1f69 	vdwdup.u8	q0, r6, r9, #2
[^>]*> ee07 0f69 	viwdup.u8	q0, r6, r9, #2
[^>]*> ee07 1fe8 	vdwdup.u8	q0, r6, r9, #4
[^>]*> ee07 0fe8 	viwdup.u8	q0, r6, r9, #4
[^>]*> ee07 1fe9 	vdwdup.u8	q0, r6, r9, #8
[^>]*> ee07 0fe9 	viwdup.u8	q0, r6, r9, #8
[^>]*> ee07 1f6a 	vdwdup.u8	q0, r6, fp, #1
[^>]*> ee07 0f6a 	viwdup.u8	q0, r6, fp, #1
[^>]*> ee07 1f6b 	vdwdup.u8	q0, r6, fp, #2
[^>]*> ee07 0f6b 	viwdup.u8	q0, r6, fp, #2
[^>]*> ee07 1fea 	vdwdup.u8	q0, r6, fp, #4
[^>]*> ee07 0fea 	viwdup.u8	q0, r6, fp, #4
[^>]*> ee07 1feb 	vdwdup.u8	q0, r6, fp, #8
[^>]*> ee07 0feb 	viwdup.u8	q0, r6, fp, #8
[^>]*> ee09 1f6e 	vddup.u8	q0, r8, #1
[^>]*> ee09 0f6e 	vidup.u8	q0, r8, #1
[^>]*> ee09 1f6f 	vddup.u8	q0, r8, #2
[^>]*> ee09 0f6f 	vidup.u8	q0, r8, #2
[^>]*> ee09 1fee 	vddup.u8	q0, r8, #4
[^>]*> ee09 0fee 	vidup.u8	q0, r8, #4
[^>]*> ee09 1fef 	vddup.u8	q0, r8, #8
[^>]*> ee09 0fef 	vidup.u8	q0, r8, #8
[^>]*> ee09 1f60 	vdwdup.u8	q0, r8, r1, #1
[^>]*> ee09 0f60 	viwdup.u8	q0, r8, r1, #1
[^>]*> ee09 1f61 	vdwdup.u8	q0, r8, r1, #2
[^>]*> ee09 0f61 	viwdup.u8	q0, r8, r1, #2
[^>]*> ee09 1fe0 	vdwdup.u8	q0, r8, r1, #4
[^>]*> ee09 0fe0 	viwdup.u8	q0, r8, r1, #4
[^>]*> ee09 1fe1 	vdwdup.u8	q0, r8, r1, #8
[^>]*> ee09 0fe1 	viwdup.u8	q0, r8, r1, #8
[^>]*> ee09 1f62 	vdwdup.u8	q0, r8, r3, #1
[^>]*> ee09 0f62 	viwdup.u8	q0, r8, r3, #1
[^>]*> ee09 1f63 	vdwdup.u8	q0, r8, r3, #2
[^>]*> ee09 0f63 	viwdup.u8	q0, r8, r3, #2
[^>]*> ee09 1fe2 	vdwdup.u8	q0, r8, r3, #4
[^>]*> ee09 0fe2 	viwdup.u8	q0, r8, r3, #4
[^>]*> ee09 1fe3 	vdwdup.u8	q0, r8, r3, #8
[^>]*> ee09 0fe3 	viwdup.u8	q0, r8, r3, #8
[^>]*> ee09 1f64 	vdwdup.u8	q0, r8, r5, #1
[^>]*> ee09 0f64 	viwdup.u8	q0, r8, r5, #1
[^>]*> ee09 1f65 	vdwdup.u8	q0, r8, r5, #2
[^>]*> ee09 0f65 	viwdup.u8	q0, r8, r5, #2
[^>]*> ee09 1fe4 	vdwdup.u8	q0, r8, r5, #4
[^>]*> ee09 0fe4 	viwdup.u8	q0, r8, r5, #4
[^>]*> ee09 1fe5 	vdwdup.u8	q0, r8, r5, #8
[^>]*> ee09 0fe5 	viwdup.u8	q0, r8, r5, #8
[^>]*> ee09 1f66 	vdwdup.u8	q0, r8, r7, #1
[^>]*> ee09 0f66 	viwdup.u8	q0, r8, r7, #1
[^>]*> ee09 1f67 	vdwdup.u8	q0, r8, r7, #2
[^>]*> ee09 0f67 	viwdup.u8	q0, r8, r7, #2
[^>]*> ee09 1fe6 	vdwdup.u8	q0, r8, r7, #4
[^>]*> ee09 0fe6 	viwdup.u8	q0, r8, r7, #4
[^>]*> ee09 1fe7 	vdwdup.u8	q0, r8, r7, #8
[^>]*> ee09 0fe7 	viwdup.u8	q0, r8, r7, #8
[^>]*> ee09 1f68 	vdwdup.u8	q0, r8, r9, #1
[^>]*> ee09 0f68 	viwdup.u8	q0, r8, r9, #1
[^>]*> ee09 1f69 	vdwdup.u8	q0, r8, r9, #2
[^>]*> ee09 0f69 	viwdup.u8	q0, r8, r9, #2
[^>]*> ee09 1fe8 	vdwdup.u8	q0, r8, r9, #4
[^>]*> ee09 0fe8 	viwdup.u8	q0, r8, r9, #4
[^>]*> ee09 1fe9 	vdwdup.u8	q0, r8, r9, #8
[^>]*> ee09 0fe9 	viwdup.u8	q0, r8, r9, #8
[^>]*> ee09 1f6a 	vdwdup.u8	q0, r8, fp, #1
[^>]*> ee09 0f6a 	viwdup.u8	q0, r8, fp, #1
[^>]*> ee09 1f6b 	vdwdup.u8	q0, r8, fp, #2
[^>]*> ee09 0f6b 	viwdup.u8	q0, r8, fp, #2
[^>]*> ee09 1fea 	vdwdup.u8	q0, r8, fp, #4
[^>]*> ee09 0fea 	viwdup.u8	q0, r8, fp, #4
[^>]*> ee09 1feb 	vdwdup.u8	q0, r8, fp, #8
[^>]*> ee09 0feb 	viwdup.u8	q0, r8, fp, #8
[^>]*> ee0b 1f6e 	vddup.u8	q0, sl, #1
[^>]*> ee0b 0f6e 	vidup.u8	q0, sl, #1
[^>]*> ee0b 1f6f 	vddup.u8	q0, sl, #2
[^>]*> ee0b 0f6f 	vidup.u8	q0, sl, #2
[^>]*> ee0b 1fee 	vddup.u8	q0, sl, #4
[^>]*> ee0b 0fee 	vidup.u8	q0, sl, #4
[^>]*> ee0b 1fef 	vddup.u8	q0, sl, #8
[^>]*> ee0b 0fef 	vidup.u8	q0, sl, #8
[^>]*> ee0b 1f60 	vdwdup.u8	q0, sl, r1, #1
[^>]*> ee0b 0f60 	viwdup.u8	q0, sl, r1, #1
[^>]*> ee0b 1f61 	vdwdup.u8	q0, sl, r1, #2
[^>]*> ee0b 0f61 	viwdup.u8	q0, sl, r1, #2
[^>]*> ee0b 1fe0 	vdwdup.u8	q0, sl, r1, #4
[^>]*> ee0b 0fe0 	viwdup.u8	q0, sl, r1, #4
[^>]*> ee0b 1fe1 	vdwdup.u8	q0, sl, r1, #8
[^>]*> ee0b 0fe1 	viwdup.u8	q0, sl, r1, #8
[^>]*> ee0b 1f62 	vdwdup.u8	q0, sl, r3, #1
[^>]*> ee0b 0f62 	viwdup.u8	q0, sl, r3, #1
[^>]*> ee0b 1f63 	vdwdup.u8	q0, sl, r3, #2
[^>]*> ee0b 0f63 	viwdup.u8	q0, sl, r3, #2
[^>]*> ee0b 1fe2 	vdwdup.u8	q0, sl, r3, #4
[^>]*> ee0b 0fe2 	viwdup.u8	q0, sl, r3, #4
[^>]*> ee0b 1fe3 	vdwdup.u8	q0, sl, r3, #8
[^>]*> ee0b 0fe3 	viwdup.u8	q0, sl, r3, #8
[^>]*> ee0b 1f64 	vdwdup.u8	q0, sl, r5, #1
[^>]*> ee0b 0f64 	viwdup.u8	q0, sl, r5, #1
[^>]*> ee0b 1f65 	vdwdup.u8	q0, sl, r5, #2
[^>]*> ee0b 0f65 	viwdup.u8	q0, sl, r5, #2
[^>]*> ee0b 1fe4 	vdwdup.u8	q0, sl, r5, #4
[^>]*> ee0b 0fe4 	viwdup.u8	q0, sl, r5, #4
[^>]*> ee0b 1fe5 	vdwdup.u8	q0, sl, r5, #8
[^>]*> ee0b 0fe5 	viwdup.u8	q0, sl, r5, #8
[^>]*> ee0b 1f66 	vdwdup.u8	q0, sl, r7, #1
[^>]*> ee0b 0f66 	viwdup.u8	q0, sl, r7, #1
[^>]*> ee0b 1f67 	vdwdup.u8	q0, sl, r7, #2
[^>]*> ee0b 0f67 	viwdup.u8	q0, sl, r7, #2
[^>]*> ee0b 1fe6 	vdwdup.u8	q0, sl, r7, #4
[^>]*> ee0b 0fe6 	viwdup.u8	q0, sl, r7, #4
[^>]*> ee0b 1fe7 	vdwdup.u8	q0, sl, r7, #8
[^>]*> ee0b 0fe7 	viwdup.u8	q0, sl, r7, #8
[^>]*> ee0b 1f68 	vdwdup.u8	q0, sl, r9, #1
[^>]*> ee0b 0f68 	viwdup.u8	q0, sl, r9, #1
[^>]*> ee0b 1f69 	vdwdup.u8	q0, sl, r9, #2
[^>]*> ee0b 0f69 	viwdup.u8	q0, sl, r9, #2
[^>]*> ee0b 1fe8 	vdwdup.u8	q0, sl, r9, #4
[^>]*> ee0b 0fe8 	viwdup.u8	q0, sl, r9, #4
[^>]*> ee0b 1fe9 	vdwdup.u8	q0, sl, r9, #8
[^>]*> ee0b 0fe9 	viwdup.u8	q0, sl, r9, #8
[^>]*> ee0b 1f6a 	vdwdup.u8	q0, sl, fp, #1
[^>]*> ee0b 0f6a 	viwdup.u8	q0, sl, fp, #1
[^>]*> ee0b 1f6b 	vdwdup.u8	q0, sl, fp, #2
[^>]*> ee0b 0f6b 	viwdup.u8	q0, sl, fp, #2
[^>]*> ee0b 1fea 	vdwdup.u8	q0, sl, fp, #4
[^>]*> ee0b 0fea 	viwdup.u8	q0, sl, fp, #4
[^>]*> ee0b 1feb 	vdwdup.u8	q0, sl, fp, #8
[^>]*> ee0b 0feb 	viwdup.u8	q0, sl, fp, #8
[^>]*> ee0d 1f6e 	vddup.u8	q0, ip, #1
[^>]*> ee0d 0f6e 	vidup.u8	q0, ip, #1
[^>]*> ee0d 1f6f 	vddup.u8	q0, ip, #2
[^>]*> ee0d 0f6f 	vidup.u8	q0, ip, #2
[^>]*> ee0d 1fee 	vddup.u8	q0, ip, #4
[^>]*> ee0d 0fee 	vidup.u8	q0, ip, #4
[^>]*> ee0d 1fef 	vddup.u8	q0, ip, #8
[^>]*> ee0d 0fef 	vidup.u8	q0, ip, #8
[^>]*> ee0d 1f60 	vdwdup.u8	q0, ip, r1, #1
[^>]*> ee0d 0f60 	viwdup.u8	q0, ip, r1, #1
[^>]*> ee0d 1f61 	vdwdup.u8	q0, ip, r1, #2
[^>]*> ee0d 0f61 	viwdup.u8	q0, ip, r1, #2
[^>]*> ee0d 1fe0 	vdwdup.u8	q0, ip, r1, #4
[^>]*> ee0d 0fe0 	viwdup.u8	q0, ip, r1, #4
[^>]*> ee0d 1fe1 	vdwdup.u8	q0, ip, r1, #8
[^>]*> ee0d 0fe1 	viwdup.u8	q0, ip, r1, #8
[^>]*> ee0d 1f62 	vdwdup.u8	q0, ip, r3, #1
[^>]*> ee0d 0f62 	viwdup.u8	q0, ip, r3, #1
[^>]*> ee0d 1f63 	vdwdup.u8	q0, ip, r3, #2
[^>]*> ee0d 0f63 	viwdup.u8	q0, ip, r3, #2
[^>]*> ee0d 1fe2 	vdwdup.u8	q0, ip, r3, #4
[^>]*> ee0d 0fe2 	viwdup.u8	q0, ip, r3, #4
[^>]*> ee0d 1fe3 	vdwdup.u8	q0, ip, r3, #8
[^>]*> ee0d 0fe3 	viwdup.u8	q0, ip, r3, #8
[^>]*> ee0d 1f64 	vdwdup.u8	q0, ip, r5, #1
[^>]*> ee0d 0f64 	viwdup.u8	q0, ip, r5, #1
[^>]*> ee0d 1f65 	vdwdup.u8	q0, ip, r5, #2
[^>]*> ee0d 0f65 	viwdup.u8	q0, ip, r5, #2
[^>]*> ee0d 1fe4 	vdwdup.u8	q0, ip, r5, #4
[^>]*> ee0d 0fe4 	viwdup.u8	q0, ip, r5, #4
[^>]*> ee0d 1fe5 	vdwdup.u8	q0, ip, r5, #8
[^>]*> ee0d 0fe5 	viwdup.u8	q0, ip, r5, #8
[^>]*> ee0d 1f66 	vdwdup.u8	q0, ip, r7, #1
[^>]*> ee0d 0f66 	viwdup.u8	q0, ip, r7, #1
[^>]*> ee0d 1f67 	vdwdup.u8	q0, ip, r7, #2
[^>]*> ee0d 0f67 	viwdup.u8	q0, ip, r7, #2
[^>]*> ee0d 1fe6 	vdwdup.u8	q0, ip, r7, #4
[^>]*> ee0d 0fe6 	viwdup.u8	q0, ip, r7, #4
[^>]*> ee0d 1fe7 	vdwdup.u8	q0, ip, r7, #8
[^>]*> ee0d 0fe7 	viwdup.u8	q0, ip, r7, #8
[^>]*> ee0d 1f68 	vdwdup.u8	q0, ip, r9, #1
[^>]*> ee0d 0f68 	viwdup.u8	q0, ip, r9, #1
[^>]*> ee0d 1f69 	vdwdup.u8	q0, ip, r9, #2
[^>]*> ee0d 0f69 	viwdup.u8	q0, ip, r9, #2
[^>]*> ee0d 1fe8 	vdwdup.u8	q0, ip, r9, #4
[^>]*> ee0d 0fe8 	viwdup.u8	q0, ip, r9, #4
[^>]*> ee0d 1fe9 	vdwdup.u8	q0, ip, r9, #8
[^>]*> ee0d 0fe9 	viwdup.u8	q0, ip, r9, #8
[^>]*> ee0d 1f6a 	vdwdup.u8	q0, ip, fp, #1
[^>]*> ee0d 0f6a 	viwdup.u8	q0, ip, fp, #1
[^>]*> ee0d 1f6b 	vdwdup.u8	q0, ip, fp, #2
[^>]*> ee0d 0f6b 	viwdup.u8	q0, ip, fp, #2
[^>]*> ee0d 1fea 	vdwdup.u8	q0, ip, fp, #4
[^>]*> ee0d 0fea 	viwdup.u8	q0, ip, fp, #4
[^>]*> ee0d 1feb 	vdwdup.u8	q0, ip, fp, #8
[^>]*> ee0d 0feb 	viwdup.u8	q0, ip, fp, #8
[^>]*> ee01 3f6e 	vddup.u8	q1, r0, #1
[^>]*> ee01 2f6e 	vidup.u8	q1, r0, #1
[^>]*> ee01 3f6f 	vddup.u8	q1, r0, #2
[^>]*> ee01 2f6f 	vidup.u8	q1, r0, #2
[^>]*> ee01 3fee 	vddup.u8	q1, r0, #4
[^>]*> ee01 2fee 	vidup.u8	q1, r0, #4
[^>]*> ee01 3fef 	vddup.u8	q1, r0, #8
[^>]*> ee01 2fef 	vidup.u8	q1, r0, #8
[^>]*> ee01 3f60 	vdwdup.u8	q1, r0, r1, #1
[^>]*> ee01 2f60 	viwdup.u8	q1, r0, r1, #1
[^>]*> ee01 3f61 	vdwdup.u8	q1, r0, r1, #2
[^>]*> ee01 2f61 	viwdup.u8	q1, r0, r1, #2
[^>]*> ee01 3fe0 	vdwdup.u8	q1, r0, r1, #4
[^>]*> ee01 2fe0 	viwdup.u8	q1, r0, r1, #4
[^>]*> ee01 3fe1 	vdwdup.u8	q1, r0, r1, #8
[^>]*> ee01 2fe1 	viwdup.u8	q1, r0, r1, #8
[^>]*> ee01 3f62 	vdwdup.u8	q1, r0, r3, #1
[^>]*> ee01 2f62 	viwdup.u8	q1, r0, r3, #1
[^>]*> ee01 3f63 	vdwdup.u8	q1, r0, r3, #2
[^>]*> ee01 2f63 	viwdup.u8	q1, r0, r3, #2
[^>]*> ee01 3fe2 	vdwdup.u8	q1, r0, r3, #4
[^>]*> ee01 2fe2 	viwdup.u8	q1, r0, r3, #4
[^>]*> ee01 3fe3 	vdwdup.u8	q1, r0, r3, #8
[^>]*> ee01 2fe3 	viwdup.u8	q1, r0, r3, #8
[^>]*> ee01 3f64 	vdwdup.u8	q1, r0, r5, #1
[^>]*> ee01 2f64 	viwdup.u8	q1, r0, r5, #1
[^>]*> ee01 3f65 	vdwdup.u8	q1, r0, r5, #2
[^>]*> ee01 2f65 	viwdup.u8	q1, r0, r5, #2
[^>]*> ee01 3fe4 	vdwdup.u8	q1, r0, r5, #4
[^>]*> ee01 2fe4 	viwdup.u8	q1, r0, r5, #4
[^>]*> ee01 3fe5 	vdwdup.u8	q1, r0, r5, #8
[^>]*> ee01 2fe5 	viwdup.u8	q1, r0, r5, #8
[^>]*> ee01 3f66 	vdwdup.u8	q1, r0, r7, #1
[^>]*> ee01 2f66 	viwdup.u8	q1, r0, r7, #1
[^>]*> ee01 3f67 	vdwdup.u8	q1, r0, r7, #2
[^>]*> ee01 2f67 	viwdup.u8	q1, r0, r7, #2
[^>]*> ee01 3fe6 	vdwdup.u8	q1, r0, r7, #4
[^>]*> ee01 2fe6 	viwdup.u8	q1, r0, r7, #4
[^>]*> ee01 3fe7 	vdwdup.u8	q1, r0, r7, #8
[^>]*> ee01 2fe7 	viwdup.u8	q1, r0, r7, #8
[^>]*> ee01 3f68 	vdwdup.u8	q1, r0, r9, #1
[^>]*> ee01 2f68 	viwdup.u8	q1, r0, r9, #1
[^>]*> ee01 3f69 	vdwdup.u8	q1, r0, r9, #2
[^>]*> ee01 2f69 	viwdup.u8	q1, r0, r9, #2
[^>]*> ee01 3fe8 	vdwdup.u8	q1, r0, r9, #4
[^>]*> ee01 2fe8 	viwdup.u8	q1, r0, r9, #4
[^>]*> ee01 3fe9 	vdwdup.u8	q1, r0, r9, #8
[^>]*> ee01 2fe9 	viwdup.u8	q1, r0, r9, #8
[^>]*> ee01 3f6a 	vdwdup.u8	q1, r0, fp, #1
[^>]*> ee01 2f6a 	viwdup.u8	q1, r0, fp, #1
[^>]*> ee01 3f6b 	vdwdup.u8	q1, r0, fp, #2
[^>]*> ee01 2f6b 	viwdup.u8	q1, r0, fp, #2
[^>]*> ee01 3fea 	vdwdup.u8	q1, r0, fp, #4
[^>]*> ee01 2fea 	viwdup.u8	q1, r0, fp, #4
[^>]*> ee01 3feb 	vdwdup.u8	q1, r0, fp, #8
[^>]*> ee01 2feb 	viwdup.u8	q1, r0, fp, #8
[^>]*> ee03 3f6e 	vddup.u8	q1, r2, #1
[^>]*> ee03 2f6e 	vidup.u8	q1, r2, #1
[^>]*> ee03 3f6f 	vddup.u8	q1, r2, #2
[^>]*> ee03 2f6f 	vidup.u8	q1, r2, #2
[^>]*> ee03 3fee 	vddup.u8	q1, r2, #4
[^>]*> ee03 2fee 	vidup.u8	q1, r2, #4
[^>]*> ee03 3fef 	vddup.u8	q1, r2, #8
[^>]*> ee03 2fef 	vidup.u8	q1, r2, #8
[^>]*> ee03 3f60 	vdwdup.u8	q1, r2, r1, #1
[^>]*> ee03 2f60 	viwdup.u8	q1, r2, r1, #1
[^>]*> ee03 3f61 	vdwdup.u8	q1, r2, r1, #2
[^>]*> ee03 2f61 	viwdup.u8	q1, r2, r1, #2
[^>]*> ee03 3fe0 	vdwdup.u8	q1, r2, r1, #4
[^>]*> ee03 2fe0 	viwdup.u8	q1, r2, r1, #4
[^>]*> ee03 3fe1 	vdwdup.u8	q1, r2, r1, #8
[^>]*> ee03 2fe1 	viwdup.u8	q1, r2, r1, #8
[^>]*> ee03 3f62 	vdwdup.u8	q1, r2, r3, #1
[^>]*> ee03 2f62 	viwdup.u8	q1, r2, r3, #1
[^>]*> ee03 3f63 	vdwdup.u8	q1, r2, r3, #2
[^>]*> ee03 2f63 	viwdup.u8	q1, r2, r3, #2
[^>]*> ee03 3fe2 	vdwdup.u8	q1, r2, r3, #4
[^>]*> ee03 2fe2 	viwdup.u8	q1, r2, r3, #4
[^>]*> ee03 3fe3 	vdwdup.u8	q1, r2, r3, #8
[^>]*> ee03 2fe3 	viwdup.u8	q1, r2, r3, #8
[^>]*> ee03 3f64 	vdwdup.u8	q1, r2, r5, #1
[^>]*> ee03 2f64 	viwdup.u8	q1, r2, r5, #1
[^>]*> ee03 3f65 	vdwdup.u8	q1, r2, r5, #2
[^>]*> ee03 2f65 	viwdup.u8	q1, r2, r5, #2
[^>]*> ee03 3fe4 	vdwdup.u8	q1, r2, r5, #4
[^>]*> ee03 2fe4 	viwdup.u8	q1, r2, r5, #4
[^>]*> ee03 3fe5 	vdwdup.u8	q1, r2, r5, #8
[^>]*> ee03 2fe5 	viwdup.u8	q1, r2, r5, #8
[^>]*> ee03 3f66 	vdwdup.u8	q1, r2, r7, #1
[^>]*> ee03 2f66 	viwdup.u8	q1, r2, r7, #1
[^>]*> ee03 3f67 	vdwdup.u8	q1, r2, r7, #2
[^>]*> ee03 2f67 	viwdup.u8	q1, r2, r7, #2
[^>]*> ee03 3fe6 	vdwdup.u8	q1, r2, r7, #4
[^>]*> ee03 2fe6 	viwdup.u8	q1, r2, r7, #4
[^>]*> ee03 3fe7 	vdwdup.u8	q1, r2, r7, #8
[^>]*> ee03 2fe7 	viwdup.u8	q1, r2, r7, #8
[^>]*> ee03 3f68 	vdwdup.u8	q1, r2, r9, #1
[^>]*> ee03 2f68 	viwdup.u8	q1, r2, r9, #1
[^>]*> ee03 3f69 	vdwdup.u8	q1, r2, r9, #2
[^>]*> ee03 2f69 	viwdup.u8	q1, r2, r9, #2
[^>]*> ee03 3fe8 	vdwdup.u8	q1, r2, r9, #4
[^>]*> ee03 2fe8 	viwdup.u8	q1, r2, r9, #4
[^>]*> ee03 3fe9 	vdwdup.u8	q1, r2, r9, #8
[^>]*> ee03 2fe9 	viwdup.u8	q1, r2, r9, #8
[^>]*> ee03 3f6a 	vdwdup.u8	q1, r2, fp, #1
[^>]*> ee03 2f6a 	viwdup.u8	q1, r2, fp, #1
[^>]*> ee03 3f6b 	vdwdup.u8	q1, r2, fp, #2
[^>]*> ee03 2f6b 	viwdup.u8	q1, r2, fp, #2
[^>]*> ee03 3fea 	vdwdup.u8	q1, r2, fp, #4
[^>]*> ee03 2fea 	viwdup.u8	q1, r2, fp, #4
[^>]*> ee03 3feb 	vdwdup.u8	q1, r2, fp, #8
[^>]*> ee03 2feb 	viwdup.u8	q1, r2, fp, #8
[^>]*> ee05 3f6e 	vddup.u8	q1, r4, #1
[^>]*> ee05 2f6e 	vidup.u8	q1, r4, #1
[^>]*> ee05 3f6f 	vddup.u8	q1, r4, #2
[^>]*> ee05 2f6f 	vidup.u8	q1, r4, #2
[^>]*> ee05 3fee 	vddup.u8	q1, r4, #4
[^>]*> ee05 2fee 	vidup.u8	q1, r4, #4
[^>]*> ee05 3fef 	vddup.u8	q1, r4, #8
[^>]*> ee05 2fef 	vidup.u8	q1, r4, #8
[^>]*> ee05 3f60 	vdwdup.u8	q1, r4, r1, #1
[^>]*> ee05 2f60 	viwdup.u8	q1, r4, r1, #1
[^>]*> ee05 3f61 	vdwdup.u8	q1, r4, r1, #2
[^>]*> ee05 2f61 	viwdup.u8	q1, r4, r1, #2
[^>]*> ee05 3fe0 	vdwdup.u8	q1, r4, r1, #4
[^>]*> ee05 2fe0 	viwdup.u8	q1, r4, r1, #4
[^>]*> ee05 3fe1 	vdwdup.u8	q1, r4, r1, #8
[^>]*> ee05 2fe1 	viwdup.u8	q1, r4, r1, #8
[^>]*> ee05 3f62 	vdwdup.u8	q1, r4, r3, #1
[^>]*> ee05 2f62 	viwdup.u8	q1, r4, r3, #1
[^>]*> ee05 3f63 	vdwdup.u8	q1, r4, r3, #2
[^>]*> ee05 2f63 	viwdup.u8	q1, r4, r3, #2
[^>]*> ee05 3fe2 	vdwdup.u8	q1, r4, r3, #4
[^>]*> ee05 2fe2 	viwdup.u8	q1, r4, r3, #4
[^>]*> ee05 3fe3 	vdwdup.u8	q1, r4, r3, #8
[^>]*> ee05 2fe3 	viwdup.u8	q1, r4, r3, #8
[^>]*> ee05 3f64 	vdwdup.u8	q1, r4, r5, #1
[^>]*> ee05 2f64 	viwdup.u8	q1, r4, r5, #1
[^>]*> ee05 3f65 	vdwdup.u8	q1, r4, r5, #2
[^>]*> ee05 2f65 	viwdup.u8	q1, r4, r5, #2
[^>]*> ee05 3fe4 	vdwdup.u8	q1, r4, r5, #4
[^>]*> ee05 2fe4 	viwdup.u8	q1, r4, r5, #4
[^>]*> ee05 3fe5 	vdwdup.u8	q1, r4, r5, #8
[^>]*> ee05 2fe5 	viwdup.u8	q1, r4, r5, #8
[^>]*> ee05 3f66 	vdwdup.u8	q1, r4, r7, #1
[^>]*> ee05 2f66 	viwdup.u8	q1, r4, r7, #1
[^>]*> ee05 3f67 	vdwdup.u8	q1, r4, r7, #2
[^>]*> ee05 2f67 	viwdup.u8	q1, r4, r7, #2
[^>]*> ee05 3fe6 	vdwdup.u8	q1, r4, r7, #4
[^>]*> ee05 2fe6 	viwdup.u8	q1, r4, r7, #4
[^>]*> ee05 3fe7 	vdwdup.u8	q1, r4, r7, #8
[^>]*> ee05 2fe7 	viwdup.u8	q1, r4, r7, #8
[^>]*> ee05 3f68 	vdwdup.u8	q1, r4, r9, #1
[^>]*> ee05 2f68 	viwdup.u8	q1, r4, r9, #1
[^>]*> ee05 3f69 	vdwdup.u8	q1, r4, r9, #2
[^>]*> ee05 2f69 	viwdup.u8	q1, r4, r9, #2
[^>]*> ee05 3fe8 	vdwdup.u8	q1, r4, r9, #4
[^>]*> ee05 2fe8 	viwdup.u8	q1, r4, r9, #4
[^>]*> ee05 3fe9 	vdwdup.u8	q1, r4, r9, #8
[^>]*> ee05 2fe9 	viwdup.u8	q1, r4, r9, #8
[^>]*> ee05 3f6a 	vdwdup.u8	q1, r4, fp, #1
[^>]*> ee05 2f6a 	viwdup.u8	q1, r4, fp, #1
[^>]*> ee05 3f6b 	vdwdup.u8	q1, r4, fp, #2
[^>]*> ee05 2f6b 	viwdup.u8	q1, r4, fp, #2
[^>]*> ee05 3fea 	vdwdup.u8	q1, r4, fp, #4
[^>]*> ee05 2fea 	viwdup.u8	q1, r4, fp, #4
[^>]*> ee05 3feb 	vdwdup.u8	q1, r4, fp, #8
[^>]*> ee05 2feb 	viwdup.u8	q1, r4, fp, #8
[^>]*> ee07 3f6e 	vddup.u8	q1, r6, #1
[^>]*> ee07 2f6e 	vidup.u8	q1, r6, #1
[^>]*> ee07 3f6f 	vddup.u8	q1, r6, #2
[^>]*> ee07 2f6f 	vidup.u8	q1, r6, #2
[^>]*> ee07 3fee 	vddup.u8	q1, r6, #4
[^>]*> ee07 2fee 	vidup.u8	q1, r6, #4
[^>]*> ee07 3fef 	vddup.u8	q1, r6, #8
[^>]*> ee07 2fef 	vidup.u8	q1, r6, #8
[^>]*> ee07 3f60 	vdwdup.u8	q1, r6, r1, #1
[^>]*> ee07 2f60 	viwdup.u8	q1, r6, r1, #1
[^>]*> ee07 3f61 	vdwdup.u8	q1, r6, r1, #2
[^>]*> ee07 2f61 	viwdup.u8	q1, r6, r1, #2
[^>]*> ee07 3fe0 	vdwdup.u8	q1, r6, r1, #4
[^>]*> ee07 2fe0 	viwdup.u8	q1, r6, r1, #4
[^>]*> ee07 3fe1 	vdwdup.u8	q1, r6, r1, #8
[^>]*> ee07 2fe1 	viwdup.u8	q1, r6, r1, #8
[^>]*> ee07 3f62 	vdwdup.u8	q1, r6, r3, #1
[^>]*> ee07 2f62 	viwdup.u8	q1, r6, r3, #1
[^>]*> ee07 3f63 	vdwdup.u8	q1, r6, r3, #2
[^>]*> ee07 2f63 	viwdup.u8	q1, r6, r3, #2
[^>]*> ee07 3fe2 	vdwdup.u8	q1, r6, r3, #4
[^>]*> ee07 2fe2 	viwdup.u8	q1, r6, r3, #4
[^>]*> ee07 3fe3 	vdwdup.u8	q1, r6, r3, #8
[^>]*> ee07 2fe3 	viwdup.u8	q1, r6, r3, #8
[^>]*> ee07 3f64 	vdwdup.u8	q1, r6, r5, #1
[^>]*> ee07 2f64 	viwdup.u8	q1, r6, r5, #1
[^>]*> ee07 3f65 	vdwdup.u8	q1, r6, r5, #2
[^>]*> ee07 2f65 	viwdup.u8	q1, r6, r5, #2
[^>]*> ee07 3fe4 	vdwdup.u8	q1, r6, r5, #4
[^>]*> ee07 2fe4 	viwdup.u8	q1, r6, r5, #4
[^>]*> ee07 3fe5 	vdwdup.u8	q1, r6, r5, #8
[^>]*> ee07 2fe5 	viwdup.u8	q1, r6, r5, #8
[^>]*> ee07 3f66 	vdwdup.u8	q1, r6, r7, #1
[^>]*> ee07 2f66 	viwdup.u8	q1, r6, r7, #1
[^>]*> ee07 3f67 	vdwdup.u8	q1, r6, r7, #2
[^>]*> ee07 2f67 	viwdup.u8	q1, r6, r7, #2
[^>]*> ee07 3fe6 	vdwdup.u8	q1, r6, r7, #4
[^>]*> ee07 2fe6 	viwdup.u8	q1, r6, r7, #4
[^>]*> ee07 3fe7 	vdwdup.u8	q1, r6, r7, #8
[^>]*> ee07 2fe7 	viwdup.u8	q1, r6, r7, #8
[^>]*> ee07 3f68 	vdwdup.u8	q1, r6, r9, #1
[^>]*> ee07 2f68 	viwdup.u8	q1, r6, r9, #1
[^>]*> ee07 3f69 	vdwdup.u8	q1, r6, r9, #2
[^>]*> ee07 2f69 	viwdup.u8	q1, r6, r9, #2
[^>]*> ee07 3fe8 	vdwdup.u8	q1, r6, r9, #4
[^>]*> ee07 2fe8 	viwdup.u8	q1, r6, r9, #4
[^>]*> ee07 3fe9 	vdwdup.u8	q1, r6, r9, #8
[^>]*> ee07 2fe9 	viwdup.u8	q1, r6, r9, #8
[^>]*> ee07 3f6a 	vdwdup.u8	q1, r6, fp, #1
[^>]*> ee07 2f6a 	viwdup.u8	q1, r6, fp, #1
[^>]*> ee07 3f6b 	vdwdup.u8	q1, r6, fp, #2
[^>]*> ee07 2f6b 	viwdup.u8	q1, r6, fp, #2
[^>]*> ee07 3fea 	vdwdup.u8	q1, r6, fp, #4
[^>]*> ee07 2fea 	viwdup.u8	q1, r6, fp, #4
[^>]*> ee07 3feb 	vdwdup.u8	q1, r6, fp, #8
[^>]*> ee07 2feb 	viwdup.u8	q1, r6, fp, #8
[^>]*> ee09 3f6e 	vddup.u8	q1, r8, #1
[^>]*> ee09 2f6e 	vidup.u8	q1, r8, #1
[^>]*> ee09 3f6f 	vddup.u8	q1, r8, #2
[^>]*> ee09 2f6f 	vidup.u8	q1, r8, #2
[^>]*> ee09 3fee 	vddup.u8	q1, r8, #4
[^>]*> ee09 2fee 	vidup.u8	q1, r8, #4
[^>]*> ee09 3fef 	vddup.u8	q1, r8, #8
[^>]*> ee09 2fef 	vidup.u8	q1, r8, #8
[^>]*> ee09 3f60 	vdwdup.u8	q1, r8, r1, #1
[^>]*> ee09 2f60 	viwdup.u8	q1, r8, r1, #1
[^>]*> ee09 3f61 	vdwdup.u8	q1, r8, r1, #2
[^>]*> ee09 2f61 	viwdup.u8	q1, r8, r1, #2
[^>]*> ee09 3fe0 	vdwdup.u8	q1, r8, r1, #4
[^>]*> ee09 2fe0 	viwdup.u8	q1, r8, r1, #4
[^>]*> ee09 3fe1 	vdwdup.u8	q1, r8, r1, #8
[^>]*> ee09 2fe1 	viwdup.u8	q1, r8, r1, #8
[^>]*> ee09 3f62 	vdwdup.u8	q1, r8, r3, #1
[^>]*> ee09 2f62 	viwdup.u8	q1, r8, r3, #1
[^>]*> ee09 3f63 	vdwdup.u8	q1, r8, r3, #2
[^>]*> ee09 2f63 	viwdup.u8	q1, r8, r3, #2
[^>]*> ee09 3fe2 	vdwdup.u8	q1, r8, r3, #4
[^>]*> ee09 2fe2 	viwdup.u8	q1, r8, r3, #4
[^>]*> ee09 3fe3 	vdwdup.u8	q1, r8, r3, #8
[^>]*> ee09 2fe3 	viwdup.u8	q1, r8, r3, #8
[^>]*> ee09 3f64 	vdwdup.u8	q1, r8, r5, #1
[^>]*> ee09 2f64 	viwdup.u8	q1, r8, r5, #1
[^>]*> ee09 3f65 	vdwdup.u8	q1, r8, r5, #2
[^>]*> ee09 2f65 	viwdup.u8	q1, r8, r5, #2
[^>]*> ee09 3fe4 	vdwdup.u8	q1, r8, r5, #4
[^>]*> ee09 2fe4 	viwdup.u8	q1, r8, r5, #4
[^>]*> ee09 3fe5 	vdwdup.u8	q1, r8, r5, #8
[^>]*> ee09 2fe5 	viwdup.u8	q1, r8, r5, #8
[^>]*> ee09 3f66 	vdwdup.u8	q1, r8, r7, #1
[^>]*> ee09 2f66 	viwdup.u8	q1, r8, r7, #1
[^>]*> ee09 3f67 	vdwdup.u8	q1, r8, r7, #2
[^>]*> ee09 2f67 	viwdup.u8	q1, r8, r7, #2
[^>]*> ee09 3fe6 	vdwdup.u8	q1, r8, r7, #4
[^>]*> ee09 2fe6 	viwdup.u8	q1, r8, r7, #4
[^>]*> ee09 3fe7 	vdwdup.u8	q1, r8, r7, #8
[^>]*> ee09 2fe7 	viwdup.u8	q1, r8, r7, #8
[^>]*> ee09 3f68 	vdwdup.u8	q1, r8, r9, #1
[^>]*> ee09 2f68 	viwdup.u8	q1, r8, r9, #1
[^>]*> ee09 3f69 	vdwdup.u8	q1, r8, r9, #2
[^>]*> ee09 2f69 	viwdup.u8	q1, r8, r9, #2
[^>]*> ee09 3fe8 	vdwdup.u8	q1, r8, r9, #4
[^>]*> ee09 2fe8 	viwdup.u8	q1, r8, r9, #4
[^>]*> ee09 3fe9 	vdwdup.u8	q1, r8, r9, #8
[^>]*> ee09 2fe9 	viwdup.u8	q1, r8, r9, #8
[^>]*> ee09 3f6a 	vdwdup.u8	q1, r8, fp, #1
[^>]*> ee09 2f6a 	viwdup.u8	q1, r8, fp, #1
[^>]*> ee09 3f6b 	vdwdup.u8	q1, r8, fp, #2
[^>]*> ee09 2f6b 	viwdup.u8	q1, r8, fp, #2
[^>]*> ee09 3fea 	vdwdup.u8	q1, r8, fp, #4
[^>]*> ee09 2fea 	viwdup.u8	q1, r8, fp, #4
[^>]*> ee09 3feb 	vdwdup.u8	q1, r8, fp, #8
[^>]*> ee09 2feb 	viwdup.u8	q1, r8, fp, #8
[^>]*> ee0b 3f6e 	vddup.u8	q1, sl, #1
[^>]*> ee0b 2f6e 	vidup.u8	q1, sl, #1
[^>]*> ee0b 3f6f 	vddup.u8	q1, sl, #2
[^>]*> ee0b 2f6f 	vidup.u8	q1, sl, #2
[^>]*> ee0b 3fee 	vddup.u8	q1, sl, #4
[^>]*> ee0b 2fee 	vidup.u8	q1, sl, #4
[^>]*> ee0b 3fef 	vddup.u8	q1, sl, #8
[^>]*> ee0b 2fef 	vidup.u8	q1, sl, #8
[^>]*> ee0b 3f60 	vdwdup.u8	q1, sl, r1, #1
[^>]*> ee0b 2f60 	viwdup.u8	q1, sl, r1, #1
[^>]*> ee0b 3f61 	vdwdup.u8	q1, sl, r1, #2
[^>]*> ee0b 2f61 	viwdup.u8	q1, sl, r1, #2
[^>]*> ee0b 3fe0 	vdwdup.u8	q1, sl, r1, #4
[^>]*> ee0b 2fe0 	viwdup.u8	q1, sl, r1, #4
[^>]*> ee0b 3fe1 	vdwdup.u8	q1, sl, r1, #8
[^>]*> ee0b 2fe1 	viwdup.u8	q1, sl, r1, #8
[^>]*> ee0b 3f62 	vdwdup.u8	q1, sl, r3, #1
[^>]*> ee0b 2f62 	viwdup.u8	q1, sl, r3, #1
[^>]*> ee0b 3f63 	vdwdup.u8	q1, sl, r3, #2
[^>]*> ee0b 2f63 	viwdup.u8	q1, sl, r3, #2
[^>]*> ee0b 3fe2 	vdwdup.u8	q1, sl, r3, #4
[^>]*> ee0b 2fe2 	viwdup.u8	q1, sl, r3, #4
[^>]*> ee0b 3fe3 	vdwdup.u8	q1, sl, r3, #8
[^>]*> ee0b 2fe3 	viwdup.u8	q1, sl, r3, #8
[^>]*> ee0b 3f64 	vdwdup.u8	q1, sl, r5, #1
[^>]*> ee0b 2f64 	viwdup.u8	q1, sl, r5, #1
[^>]*> ee0b 3f65 	vdwdup.u8	q1, sl, r5, #2
[^>]*> ee0b 2f65 	viwdup.u8	q1, sl, r5, #2
[^>]*> ee0b 3fe4 	vdwdup.u8	q1, sl, r5, #4
[^>]*> ee0b 2fe4 	viwdup.u8	q1, sl, r5, #4
[^>]*> ee0b 3fe5 	vdwdup.u8	q1, sl, r5, #8
[^>]*> ee0b 2fe5 	viwdup.u8	q1, sl, r5, #8
[^>]*> ee0b 3f66 	vdwdup.u8	q1, sl, r7, #1
[^>]*> ee0b 2f66 	viwdup.u8	q1, sl, r7, #1
[^>]*> ee0b 3f67 	vdwdup.u8	q1, sl, r7, #2
[^>]*> ee0b 2f67 	viwdup.u8	q1, sl, r7, #2
[^>]*> ee0b 3fe6 	vdwdup.u8	q1, sl, r7, #4
[^>]*> ee0b 2fe6 	viwdup.u8	q1, sl, r7, #4
[^>]*> ee0b 3fe7 	vdwdup.u8	q1, sl, r7, #8
[^>]*> ee0b 2fe7 	viwdup.u8	q1, sl, r7, #8
[^>]*> ee0b 3f68 	vdwdup.u8	q1, sl, r9, #1
[^>]*> ee0b 2f68 	viwdup.u8	q1, sl, r9, #1
[^>]*> ee0b 3f69 	vdwdup.u8	q1, sl, r9, #2
[^>]*> ee0b 2f69 	viwdup.u8	q1, sl, r9, #2
[^>]*> ee0b 3fe8 	vdwdup.u8	q1, sl, r9, #4
[^>]*> ee0b 2fe8 	viwdup.u8	q1, sl, r9, #4
[^>]*> ee0b 3fe9 	vdwdup.u8	q1, sl, r9, #8
[^>]*> ee0b 2fe9 	viwdup.u8	q1, sl, r9, #8
[^>]*> ee0b 3f6a 	vdwdup.u8	q1, sl, fp, #1
[^>]*> ee0b 2f6a 	viwdup.u8	q1, sl, fp, #1
[^>]*> ee0b 3f6b 	vdwdup.u8	q1, sl, fp, #2
[^>]*> ee0b 2f6b 	viwdup.u8	q1, sl, fp, #2
[^>]*> ee0b 3fea 	vdwdup.u8	q1, sl, fp, #4
[^>]*> ee0b 2fea 	viwdup.u8	q1, sl, fp, #4
[^>]*> ee0b 3feb 	vdwdup.u8	q1, sl, fp, #8
[^>]*> ee0b 2feb 	viwdup.u8	q1, sl, fp, #8
[^>]*> ee0d 3f6e 	vddup.u8	q1, ip, #1
[^>]*> ee0d 2f6e 	vidup.u8	q1, ip, #1
[^>]*> ee0d 3f6f 	vddup.u8	q1, ip, #2
[^>]*> ee0d 2f6f 	vidup.u8	q1, ip, #2
[^>]*> ee0d 3fee 	vddup.u8	q1, ip, #4
[^>]*> ee0d 2fee 	vidup.u8	q1, ip, #4
[^>]*> ee0d 3fef 	vddup.u8	q1, ip, #8
[^>]*> ee0d 2fef 	vidup.u8	q1, ip, #8
[^>]*> ee0d 3f60 	vdwdup.u8	q1, ip, r1, #1
[^>]*> ee0d 2f60 	viwdup.u8	q1, ip, r1, #1
[^>]*> ee0d 3f61 	vdwdup.u8	q1, ip, r1, #2
[^>]*> ee0d 2f61 	viwdup.u8	q1, ip, r1, #2
[^>]*> ee0d 3fe0 	vdwdup.u8	q1, ip, r1, #4
[^>]*> ee0d 2fe0 	viwdup.u8	q1, ip, r1, #4
[^>]*> ee0d 3fe1 	vdwdup.u8	q1, ip, r1, #8
[^>]*> ee0d 2fe1 	viwdup.u8	q1, ip, r1, #8
[^>]*> ee0d 3f62 	vdwdup.u8	q1, ip, r3, #1
[^>]*> ee0d 2f62 	viwdup.u8	q1, ip, r3, #1
[^>]*> ee0d 3f63 	vdwdup.u8	q1, ip, r3, #2
[^>]*> ee0d 2f63 	viwdup.u8	q1, ip, r3, #2
[^>]*> ee0d 3fe2 	vdwdup.u8	q1, ip, r3, #4
[^>]*> ee0d 2fe2 	viwdup.u8	q1, ip, r3, #4
[^>]*> ee0d 3fe3 	vdwdup.u8	q1, ip, r3, #8
[^>]*> ee0d 2fe3 	viwdup.u8	q1, ip, r3, #8
[^>]*> ee0d 3f64 	vdwdup.u8	q1, ip, r5, #1
[^>]*> ee0d 2f64 	viwdup.u8	q1, ip, r5, #1
[^>]*> ee0d 3f65 	vdwdup.u8	q1, ip, r5, #2
[^>]*> ee0d 2f65 	viwdup.u8	q1, ip, r5, #2
[^>]*> ee0d 3fe4 	vdwdup.u8	q1, ip, r5, #4
[^>]*> ee0d 2fe4 	viwdup.u8	q1, ip, r5, #4
[^>]*> ee0d 3fe5 	vdwdup.u8	q1, ip, r5, #8
[^>]*> ee0d 2fe5 	viwdup.u8	q1, ip, r5, #8
[^>]*> ee0d 3f66 	vdwdup.u8	q1, ip, r7, #1
[^>]*> ee0d 2f66 	viwdup.u8	q1, ip, r7, #1
[^>]*> ee0d 3f67 	vdwdup.u8	q1, ip, r7, #2
[^>]*> ee0d 2f67 	viwdup.u8	q1, ip, r7, #2
[^>]*> ee0d 3fe6 	vdwdup.u8	q1, ip, r7, #4
[^>]*> ee0d 2fe6 	viwdup.u8	q1, ip, r7, #4
[^>]*> ee0d 3fe7 	vdwdup.u8	q1, ip, r7, #8
[^>]*> ee0d 2fe7 	viwdup.u8	q1, ip, r7, #8
[^>]*> ee0d 3f68 	vdwdup.u8	q1, ip, r9, #1
[^>]*> ee0d 2f68 	viwdup.u8	q1, ip, r9, #1
[^>]*> ee0d 3f69 	vdwdup.u8	q1, ip, r9, #2
[^>]*> ee0d 2f69 	viwdup.u8	q1, ip, r9, #2
[^>]*> ee0d 3fe8 	vdwdup.u8	q1, ip, r9, #4
[^>]*> ee0d 2fe8 	viwdup.u8	q1, ip, r9, #4
[^>]*> ee0d 3fe9 	vdwdup.u8	q1, ip, r9, #8
[^>]*> ee0d 2fe9 	viwdup.u8	q1, ip, r9, #8
[^>]*> ee0d 3f6a 	vdwdup.u8	q1, ip, fp, #1
[^>]*> ee0d 2f6a 	viwdup.u8	q1, ip, fp, #1
[^>]*> ee0d 3f6b 	vdwdup.u8	q1, ip, fp, #2
[^>]*> ee0d 2f6b 	viwdup.u8	q1, ip, fp, #2
[^>]*> ee0d 3fea 	vdwdup.u8	q1, ip, fp, #4
[^>]*> ee0d 2fea 	viwdup.u8	q1, ip, fp, #4
[^>]*> ee0d 3feb 	vdwdup.u8	q1, ip, fp, #8
[^>]*> ee0d 2feb 	viwdup.u8	q1, ip, fp, #8
[^>]*> ee01 5f6e 	vddup.u8	q2, r0, #1
[^>]*> ee01 4f6e 	vidup.u8	q2, r0, #1
[^>]*> ee01 5f6f 	vddup.u8	q2, r0, #2
[^>]*> ee01 4f6f 	vidup.u8	q2, r0, #2
[^>]*> ee01 5fee 	vddup.u8	q2, r0, #4
[^>]*> ee01 4fee 	vidup.u8	q2, r0, #4
[^>]*> ee01 5fef 	vddup.u8	q2, r0, #8
[^>]*> ee01 4fef 	vidup.u8	q2, r0, #8
[^>]*> ee01 5f60 	vdwdup.u8	q2, r0, r1, #1
[^>]*> ee01 4f60 	viwdup.u8	q2, r0, r1, #1
[^>]*> ee01 5f61 	vdwdup.u8	q2, r0, r1, #2
[^>]*> ee01 4f61 	viwdup.u8	q2, r0, r1, #2
[^>]*> ee01 5fe0 	vdwdup.u8	q2, r0, r1, #4
[^>]*> ee01 4fe0 	viwdup.u8	q2, r0, r1, #4
[^>]*> ee01 5fe1 	vdwdup.u8	q2, r0, r1, #8
[^>]*> ee01 4fe1 	viwdup.u8	q2, r0, r1, #8
[^>]*> ee01 5f62 	vdwdup.u8	q2, r0, r3, #1
[^>]*> ee01 4f62 	viwdup.u8	q2, r0, r3, #1
[^>]*> ee01 5f63 	vdwdup.u8	q2, r0, r3, #2
[^>]*> ee01 4f63 	viwdup.u8	q2, r0, r3, #2
[^>]*> ee01 5fe2 	vdwdup.u8	q2, r0, r3, #4
[^>]*> ee01 4fe2 	viwdup.u8	q2, r0, r3, #4
[^>]*> ee01 5fe3 	vdwdup.u8	q2, r0, r3, #8
[^>]*> ee01 4fe3 	viwdup.u8	q2, r0, r3, #8
[^>]*> ee01 5f64 	vdwdup.u8	q2, r0, r5, #1
[^>]*> ee01 4f64 	viwdup.u8	q2, r0, r5, #1
[^>]*> ee01 5f65 	vdwdup.u8	q2, r0, r5, #2
[^>]*> ee01 4f65 	viwdup.u8	q2, r0, r5, #2
[^>]*> ee01 5fe4 	vdwdup.u8	q2, r0, r5, #4
[^>]*> ee01 4fe4 	viwdup.u8	q2, r0, r5, #4
[^>]*> ee01 5fe5 	vdwdup.u8	q2, r0, r5, #8
[^>]*> ee01 4fe5 	viwdup.u8	q2, r0, r5, #8
[^>]*> ee01 5f66 	vdwdup.u8	q2, r0, r7, #1
[^>]*> ee01 4f66 	viwdup.u8	q2, r0, r7, #1
[^>]*> ee01 5f67 	vdwdup.u8	q2, r0, r7, #2
[^>]*> ee01 4f67 	viwdup.u8	q2, r0, r7, #2
[^>]*> ee01 5fe6 	vdwdup.u8	q2, r0, r7, #4
[^>]*> ee01 4fe6 	viwdup.u8	q2, r0, r7, #4
[^>]*> ee01 5fe7 	vdwdup.u8	q2, r0, r7, #8
[^>]*> ee01 4fe7 	viwdup.u8	q2, r0, r7, #8
[^>]*> ee01 5f68 	vdwdup.u8	q2, r0, r9, #1
[^>]*> ee01 4f68 	viwdup.u8	q2, r0, r9, #1
[^>]*> ee01 5f69 	vdwdup.u8	q2, r0, r9, #2
[^>]*> ee01 4f69 	viwdup.u8	q2, r0, r9, #2
[^>]*> ee01 5fe8 	vdwdup.u8	q2, r0, r9, #4
[^>]*> ee01 4fe8 	viwdup.u8	q2, r0, r9, #4
[^>]*> ee01 5fe9 	vdwdup.u8	q2, r0, r9, #8
[^>]*> ee01 4fe9 	viwdup.u8	q2, r0, r9, #8
[^>]*> ee01 5f6a 	vdwdup.u8	q2, r0, fp, #1
[^>]*> ee01 4f6a 	viwdup.u8	q2, r0, fp, #1
[^>]*> ee01 5f6b 	vdwdup.u8	q2, r0, fp, #2
[^>]*> ee01 4f6b 	viwdup.u8	q2, r0, fp, #2
[^>]*> ee01 5fea 	vdwdup.u8	q2, r0, fp, #4
[^>]*> ee01 4fea 	viwdup.u8	q2, r0, fp, #4
[^>]*> ee01 5feb 	vdwdup.u8	q2, r0, fp, #8
[^>]*> ee01 4feb 	viwdup.u8	q2, r0, fp, #8
[^>]*> ee03 5f6e 	vddup.u8	q2, r2, #1
[^>]*> ee03 4f6e 	vidup.u8	q2, r2, #1
[^>]*> ee03 5f6f 	vddup.u8	q2, r2, #2
[^>]*> ee03 4f6f 	vidup.u8	q2, r2, #2
[^>]*> ee03 5fee 	vddup.u8	q2, r2, #4
[^>]*> ee03 4fee 	vidup.u8	q2, r2, #4
[^>]*> ee03 5fef 	vddup.u8	q2, r2, #8
[^>]*> ee03 4fef 	vidup.u8	q2, r2, #8
[^>]*> ee03 5f60 	vdwdup.u8	q2, r2, r1, #1
[^>]*> ee03 4f60 	viwdup.u8	q2, r2, r1, #1
[^>]*> ee03 5f61 	vdwdup.u8	q2, r2, r1, #2
[^>]*> ee03 4f61 	viwdup.u8	q2, r2, r1, #2
[^>]*> ee03 5fe0 	vdwdup.u8	q2, r2, r1, #4
[^>]*> ee03 4fe0 	viwdup.u8	q2, r2, r1, #4
[^>]*> ee03 5fe1 	vdwdup.u8	q2, r2, r1, #8
[^>]*> ee03 4fe1 	viwdup.u8	q2, r2, r1, #8
[^>]*> ee03 5f62 	vdwdup.u8	q2, r2, r3, #1
[^>]*> ee03 4f62 	viwdup.u8	q2, r2, r3, #1
[^>]*> ee03 5f63 	vdwdup.u8	q2, r2, r3, #2
[^>]*> ee03 4f63 	viwdup.u8	q2, r2, r3, #2
[^>]*> ee03 5fe2 	vdwdup.u8	q2, r2, r3, #4
[^>]*> ee03 4fe2 	viwdup.u8	q2, r2, r3, #4
[^>]*> ee03 5fe3 	vdwdup.u8	q2, r2, r3, #8
[^>]*> ee03 4fe3 	viwdup.u8	q2, r2, r3, #8
[^>]*> ee03 5f64 	vdwdup.u8	q2, r2, r5, #1
[^>]*> ee03 4f64 	viwdup.u8	q2, r2, r5, #1
[^>]*> ee03 5f65 	vdwdup.u8	q2, r2, r5, #2
[^>]*> ee03 4f65 	viwdup.u8	q2, r2, r5, #2
[^>]*> ee03 5fe4 	vdwdup.u8	q2, r2, r5, #4
[^>]*> ee03 4fe4 	viwdup.u8	q2, r2, r5, #4
[^>]*> ee03 5fe5 	vdwdup.u8	q2, r2, r5, #8
[^>]*> ee03 4fe5 	viwdup.u8	q2, r2, r5, #8
[^>]*> ee03 5f66 	vdwdup.u8	q2, r2, r7, #1
[^>]*> ee03 4f66 	viwdup.u8	q2, r2, r7, #1
[^>]*> ee03 5f67 	vdwdup.u8	q2, r2, r7, #2
[^>]*> ee03 4f67 	viwdup.u8	q2, r2, r7, #2
[^>]*> ee03 5fe6 	vdwdup.u8	q2, r2, r7, #4
[^>]*> ee03 4fe6 	viwdup.u8	q2, r2, r7, #4
[^>]*> ee03 5fe7 	vdwdup.u8	q2, r2, r7, #8
[^>]*> ee03 4fe7 	viwdup.u8	q2, r2, r7, #8
[^>]*> ee03 5f68 	vdwdup.u8	q2, r2, r9, #1
[^>]*> ee03 4f68 	viwdup.u8	q2, r2, r9, #1
[^>]*> ee03 5f69 	vdwdup.u8	q2, r2, r9, #2
[^>]*> ee03 4f69 	viwdup.u8	q2, r2, r9, #2
[^>]*> ee03 5fe8 	vdwdup.u8	q2, r2, r9, #4
[^>]*> ee03 4fe8 	viwdup.u8	q2, r2, r9, #4
[^>]*> ee03 5fe9 	vdwdup.u8	q2, r2, r9, #8
[^>]*> ee03 4fe9 	viwdup.u8	q2, r2, r9, #8
[^>]*> ee03 5f6a 	vdwdup.u8	q2, r2, fp, #1
[^>]*> ee03 4f6a 	viwdup.u8	q2, r2, fp, #1
[^>]*> ee03 5f6b 	vdwdup.u8	q2, r2, fp, #2
[^>]*> ee03 4f6b 	viwdup.u8	q2, r2, fp, #2
[^>]*> ee03 5fea 	vdwdup.u8	q2, r2, fp, #4
[^>]*> ee03 4fea 	viwdup.u8	q2, r2, fp, #4
[^>]*> ee03 5feb 	vdwdup.u8	q2, r2, fp, #8
[^>]*> ee03 4feb 	viwdup.u8	q2, r2, fp, #8
[^>]*> ee05 5f6e 	vddup.u8	q2, r4, #1
[^>]*> ee05 4f6e 	vidup.u8	q2, r4, #1
[^>]*> ee05 5f6f 	vddup.u8	q2, r4, #2
[^>]*> ee05 4f6f 	vidup.u8	q2, r4, #2
[^>]*> ee05 5fee 	vddup.u8	q2, r4, #4
[^>]*> ee05 4fee 	vidup.u8	q2, r4, #4
[^>]*> ee05 5fef 	vddup.u8	q2, r4, #8
[^>]*> ee05 4fef 	vidup.u8	q2, r4, #8
[^>]*> ee05 5f60 	vdwdup.u8	q2, r4, r1, #1
[^>]*> ee05 4f60 	viwdup.u8	q2, r4, r1, #1
[^>]*> ee05 5f61 	vdwdup.u8	q2, r4, r1, #2
[^>]*> ee05 4f61 	viwdup.u8	q2, r4, r1, #2
[^>]*> ee05 5fe0 	vdwdup.u8	q2, r4, r1, #4
[^>]*> ee05 4fe0 	viwdup.u8	q2, r4, r1, #4
[^>]*> ee05 5fe1 	vdwdup.u8	q2, r4, r1, #8
[^>]*> ee05 4fe1 	viwdup.u8	q2, r4, r1, #8
[^>]*> ee05 5f62 	vdwdup.u8	q2, r4, r3, #1
[^>]*> ee05 4f62 	viwdup.u8	q2, r4, r3, #1
[^>]*> ee05 5f63 	vdwdup.u8	q2, r4, r3, #2
[^>]*> ee05 4f63 	viwdup.u8	q2, r4, r3, #2
[^>]*> ee05 5fe2 	vdwdup.u8	q2, r4, r3, #4
[^>]*> ee05 4fe2 	viwdup.u8	q2, r4, r3, #4
[^>]*> ee05 5fe3 	vdwdup.u8	q2, r4, r3, #8
[^>]*> ee05 4fe3 	viwdup.u8	q2, r4, r3, #8
[^>]*> ee05 5f64 	vdwdup.u8	q2, r4, r5, #1
[^>]*> ee05 4f64 	viwdup.u8	q2, r4, r5, #1
[^>]*> ee05 5f65 	vdwdup.u8	q2, r4, r5, #2
[^>]*> ee05 4f65 	viwdup.u8	q2, r4, r5, #2
[^>]*> ee05 5fe4 	vdwdup.u8	q2, r4, r5, #4
[^>]*> ee05 4fe4 	viwdup.u8	q2, r4, r5, #4
[^>]*> ee05 5fe5 	vdwdup.u8	q2, r4, r5, #8
[^>]*> ee05 4fe5 	viwdup.u8	q2, r4, r5, #8
[^>]*> ee05 5f66 	vdwdup.u8	q2, r4, r7, #1
[^>]*> ee05 4f66 	viwdup.u8	q2, r4, r7, #1
[^>]*> ee05 5f67 	vdwdup.u8	q2, r4, r7, #2
[^>]*> ee05 4f67 	viwdup.u8	q2, r4, r7, #2
[^>]*> ee05 5fe6 	vdwdup.u8	q2, r4, r7, #4
[^>]*> ee05 4fe6 	viwdup.u8	q2, r4, r7, #4
[^>]*> ee05 5fe7 	vdwdup.u8	q2, r4, r7, #8
[^>]*> ee05 4fe7 	viwdup.u8	q2, r4, r7, #8
[^>]*> ee05 5f68 	vdwdup.u8	q2, r4, r9, #1
[^>]*> ee05 4f68 	viwdup.u8	q2, r4, r9, #1
[^>]*> ee05 5f69 	vdwdup.u8	q2, r4, r9, #2
[^>]*> ee05 4f69 	viwdup.u8	q2, r4, r9, #2
[^>]*> ee05 5fe8 	vdwdup.u8	q2, r4, r9, #4
[^>]*> ee05 4fe8 	viwdup.u8	q2, r4, r9, #4
[^>]*> ee05 5fe9 	vdwdup.u8	q2, r4, r9, #8
[^>]*> ee05 4fe9 	viwdup.u8	q2, r4, r9, #8
[^>]*> ee05 5f6a 	vdwdup.u8	q2, r4, fp, #1
[^>]*> ee05 4f6a 	viwdup.u8	q2, r4, fp, #1
[^>]*> ee05 5f6b 	vdwdup.u8	q2, r4, fp, #2
[^>]*> ee05 4f6b 	viwdup.u8	q2, r4, fp, #2
[^>]*> ee05 5fea 	vdwdup.u8	q2, r4, fp, #4
[^>]*> ee05 4fea 	viwdup.u8	q2, r4, fp, #4
[^>]*> ee05 5feb 	vdwdup.u8	q2, r4, fp, #8
[^>]*> ee05 4feb 	viwdup.u8	q2, r4, fp, #8
[^>]*> ee07 5f6e 	vddup.u8	q2, r6, #1
[^>]*> ee07 4f6e 	vidup.u8	q2, r6, #1
[^>]*> ee07 5f6f 	vddup.u8	q2, r6, #2
[^>]*> ee07 4f6f 	vidup.u8	q2, r6, #2
[^>]*> ee07 5fee 	vddup.u8	q2, r6, #4
[^>]*> ee07 4fee 	vidup.u8	q2, r6, #4
[^>]*> ee07 5fef 	vddup.u8	q2, r6, #8
[^>]*> ee07 4fef 	vidup.u8	q2, r6, #8
[^>]*> ee07 5f60 	vdwdup.u8	q2, r6, r1, #1
[^>]*> ee07 4f60 	viwdup.u8	q2, r6, r1, #1
[^>]*> ee07 5f61 	vdwdup.u8	q2, r6, r1, #2
[^>]*> ee07 4f61 	viwdup.u8	q2, r6, r1, #2
[^>]*> ee07 5fe0 	vdwdup.u8	q2, r6, r1, #4
[^>]*> ee07 4fe0 	viwdup.u8	q2, r6, r1, #4
[^>]*> ee07 5fe1 	vdwdup.u8	q2, r6, r1, #8
[^>]*> ee07 4fe1 	viwdup.u8	q2, r6, r1, #8
[^>]*> ee07 5f62 	vdwdup.u8	q2, r6, r3, #1
[^>]*> ee07 4f62 	viwdup.u8	q2, r6, r3, #1
[^>]*> ee07 5f63 	vdwdup.u8	q2, r6, r3, #2
[^>]*> ee07 4f63 	viwdup.u8	q2, r6, r3, #2
[^>]*> ee07 5fe2 	vdwdup.u8	q2, r6, r3, #4
[^>]*> ee07 4fe2 	viwdup.u8	q2, r6, r3, #4
[^>]*> ee07 5fe3 	vdwdup.u8	q2, r6, r3, #8
[^>]*> ee07 4fe3 	viwdup.u8	q2, r6, r3, #8
[^>]*> ee07 5f64 	vdwdup.u8	q2, r6, r5, #1
[^>]*> ee07 4f64 	viwdup.u8	q2, r6, r5, #1
[^>]*> ee07 5f65 	vdwdup.u8	q2, r6, r5, #2
[^>]*> ee07 4f65 	viwdup.u8	q2, r6, r5, #2
[^>]*> ee07 5fe4 	vdwdup.u8	q2, r6, r5, #4
[^>]*> ee07 4fe4 	viwdup.u8	q2, r6, r5, #4
[^>]*> ee07 5fe5 	vdwdup.u8	q2, r6, r5, #8
[^>]*> ee07 4fe5 	viwdup.u8	q2, r6, r5, #8
[^>]*> ee07 5f66 	vdwdup.u8	q2, r6, r7, #1
[^>]*> ee07 4f66 	viwdup.u8	q2, r6, r7, #1
[^>]*> ee07 5f67 	vdwdup.u8	q2, r6, r7, #2
[^>]*> ee07 4f67 	viwdup.u8	q2, r6, r7, #2
[^>]*> ee07 5fe6 	vdwdup.u8	q2, r6, r7, #4
[^>]*> ee07 4fe6 	viwdup.u8	q2, r6, r7, #4
[^>]*> ee07 5fe7 	vdwdup.u8	q2, r6, r7, #8
[^>]*> ee07 4fe7 	viwdup.u8	q2, r6, r7, #8
[^>]*> ee07 5f68 	vdwdup.u8	q2, r6, r9, #1
[^>]*> ee07 4f68 	viwdup.u8	q2, r6, r9, #1
[^>]*> ee07 5f69 	vdwdup.u8	q2, r6, r9, #2
[^>]*> ee07 4f69 	viwdup.u8	q2, r6, r9, #2
[^>]*> ee07 5fe8 	vdwdup.u8	q2, r6, r9, #4
[^>]*> ee07 4fe8 	viwdup.u8	q2, r6, r9, #4
[^>]*> ee07 5fe9 	vdwdup.u8	q2, r6, r9, #8
[^>]*> ee07 4fe9 	viwdup.u8	q2, r6, r9, #8
[^>]*> ee07 5f6a 	vdwdup.u8	q2, r6, fp, #1
[^>]*> ee07 4f6a 	viwdup.u8	q2, r6, fp, #1
[^>]*> ee07 5f6b 	vdwdup.u8	q2, r6, fp, #2
[^>]*> ee07 4f6b 	viwdup.u8	q2, r6, fp, #2
[^>]*> ee07 5fea 	vdwdup.u8	q2, r6, fp, #4
[^>]*> ee07 4fea 	viwdup.u8	q2, r6, fp, #4
[^>]*> ee07 5feb 	vdwdup.u8	q2, r6, fp, #8
[^>]*> ee07 4feb 	viwdup.u8	q2, r6, fp, #8
[^>]*> ee09 5f6e 	vddup.u8	q2, r8, #1
[^>]*> ee09 4f6e 	vidup.u8	q2, r8, #1
[^>]*> ee09 5f6f 	vddup.u8	q2, r8, #2
[^>]*> ee09 4f6f 	vidup.u8	q2, r8, #2
[^>]*> ee09 5fee 	vddup.u8	q2, r8, #4
[^>]*> ee09 4fee 	vidup.u8	q2, r8, #4
[^>]*> ee09 5fef 	vddup.u8	q2, r8, #8
[^>]*> ee09 4fef 	vidup.u8	q2, r8, #8
[^>]*> ee09 5f60 	vdwdup.u8	q2, r8, r1, #1
[^>]*> ee09 4f60 	viwdup.u8	q2, r8, r1, #1
[^>]*> ee09 5f61 	vdwdup.u8	q2, r8, r1, #2
[^>]*> ee09 4f61 	viwdup.u8	q2, r8, r1, #2
[^>]*> ee09 5fe0 	vdwdup.u8	q2, r8, r1, #4
[^>]*> ee09 4fe0 	viwdup.u8	q2, r8, r1, #4
[^>]*> ee09 5fe1 	vdwdup.u8	q2, r8, r1, #8
[^>]*> ee09 4fe1 	viwdup.u8	q2, r8, r1, #8
[^>]*> ee09 5f62 	vdwdup.u8	q2, r8, r3, #1
[^>]*> ee09 4f62 	viwdup.u8	q2, r8, r3, #1
[^>]*> ee09 5f63 	vdwdup.u8	q2, r8, r3, #2
[^>]*> ee09 4f63 	viwdup.u8	q2, r8, r3, #2
[^>]*> ee09 5fe2 	vdwdup.u8	q2, r8, r3, #4
[^>]*> ee09 4fe2 	viwdup.u8	q2, r8, r3, #4
[^>]*> ee09 5fe3 	vdwdup.u8	q2, r8, r3, #8
[^>]*> ee09 4fe3 	viwdup.u8	q2, r8, r3, #8
[^>]*> ee09 5f64 	vdwdup.u8	q2, r8, r5, #1
[^>]*> ee09 4f64 	viwdup.u8	q2, r8, r5, #1
[^>]*> ee09 5f65 	vdwdup.u8	q2, r8, r5, #2
[^>]*> ee09 4f65 	viwdup.u8	q2, r8, r5, #2
[^>]*> ee09 5fe4 	vdwdup.u8	q2, r8, r5, #4
[^>]*> ee09 4fe4 	viwdup.u8	q2, r8, r5, #4
[^>]*> ee09 5fe5 	vdwdup.u8	q2, r8, r5, #8
[^>]*> ee09 4fe5 	viwdup.u8	q2, r8, r5, #8
[^>]*> ee09 5f66 	vdwdup.u8	q2, r8, r7, #1
[^>]*> ee09 4f66 	viwdup.u8	q2, r8, r7, #1
[^>]*> ee09 5f67 	vdwdup.u8	q2, r8, r7, #2
[^>]*> ee09 4f67 	viwdup.u8	q2, r8, r7, #2
[^>]*> ee09 5fe6 	vdwdup.u8	q2, r8, r7, #4
[^>]*> ee09 4fe6 	viwdup.u8	q2, r8, r7, #4
[^>]*> ee09 5fe7 	vdwdup.u8	q2, r8, r7, #8
[^>]*> ee09 4fe7 	viwdup.u8	q2, r8, r7, #8
[^>]*> ee09 5f68 	vdwdup.u8	q2, r8, r9, #1
[^>]*> ee09 4f68 	viwdup.u8	q2, r8, r9, #1
[^>]*> ee09 5f69 	vdwdup.u8	q2, r8, r9, #2
[^>]*> ee09 4f69 	viwdup.u8	q2, r8, r9, #2
[^>]*> ee09 5fe8 	vdwdup.u8	q2, r8, r9, #4
[^>]*> ee09 4fe8 	viwdup.u8	q2, r8, r9, #4
[^>]*> ee09 5fe9 	vdwdup.u8	q2, r8, r9, #8
[^>]*> ee09 4fe9 	viwdup.u8	q2, r8, r9, #8
[^>]*> ee09 5f6a 	vdwdup.u8	q2, r8, fp, #1
[^>]*> ee09 4f6a 	viwdup.u8	q2, r8, fp, #1
[^>]*> ee09 5f6b 	vdwdup.u8	q2, r8, fp, #2
[^>]*> ee09 4f6b 	viwdup.u8	q2, r8, fp, #2
[^>]*> ee09 5fea 	vdwdup.u8	q2, r8, fp, #4
[^>]*> ee09 4fea 	viwdup.u8	q2, r8, fp, #4
[^>]*> ee09 5feb 	vdwdup.u8	q2, r8, fp, #8
[^>]*> ee09 4feb 	viwdup.u8	q2, r8, fp, #8
[^>]*> ee0b 5f6e 	vddup.u8	q2, sl, #1
[^>]*> ee0b 4f6e 	vidup.u8	q2, sl, #1
[^>]*> ee0b 5f6f 	vddup.u8	q2, sl, #2
[^>]*> ee0b 4f6f 	vidup.u8	q2, sl, #2
[^>]*> ee0b 5fee 	vddup.u8	q2, sl, #4
[^>]*> ee0b 4fee 	vidup.u8	q2, sl, #4
[^>]*> ee0b 5fef 	vddup.u8	q2, sl, #8
[^>]*> ee0b 4fef 	vidup.u8	q2, sl, #8
[^>]*> ee0b 5f60 	vdwdup.u8	q2, sl, r1, #1
[^>]*> ee0b 4f60 	viwdup.u8	q2, sl, r1, #1
[^>]*> ee0b 5f61 	vdwdup.u8	q2, sl, r1, #2
[^>]*> ee0b 4f61 	viwdup.u8	q2, sl, r1, #2
[^>]*> ee0b 5fe0 	vdwdup.u8	q2, sl, r1, #4
[^>]*> ee0b 4fe0 	viwdup.u8	q2, sl, r1, #4
[^>]*> ee0b 5fe1 	vdwdup.u8	q2, sl, r1, #8
[^>]*> ee0b 4fe1 	viwdup.u8	q2, sl, r1, #8
[^>]*> ee0b 5f62 	vdwdup.u8	q2, sl, r3, #1
[^>]*> ee0b 4f62 	viwdup.u8	q2, sl, r3, #1
[^>]*> ee0b 5f63 	vdwdup.u8	q2, sl, r3, #2
[^>]*> ee0b 4f63 	viwdup.u8	q2, sl, r3, #2
[^>]*> ee0b 5fe2 	vdwdup.u8	q2, sl, r3, #4
[^>]*> ee0b 4fe2 	viwdup.u8	q2, sl, r3, #4
[^>]*> ee0b 5fe3 	vdwdup.u8	q2, sl, r3, #8
[^>]*> ee0b 4fe3 	viwdup.u8	q2, sl, r3, #8
[^>]*> ee0b 5f64 	vdwdup.u8	q2, sl, r5, #1
[^>]*> ee0b 4f64 	viwdup.u8	q2, sl, r5, #1
[^>]*> ee0b 5f65 	vdwdup.u8	q2, sl, r5, #2
[^>]*> ee0b 4f65 	viwdup.u8	q2, sl, r5, #2
[^>]*> ee0b 5fe4 	vdwdup.u8	q2, sl, r5, #4
[^>]*> ee0b 4fe4 	viwdup.u8	q2, sl, r5, #4
[^>]*> ee0b 5fe5 	vdwdup.u8	q2, sl, r5, #8
[^>]*> ee0b 4fe5 	viwdup.u8	q2, sl, r5, #8
[^>]*> ee0b 5f66 	vdwdup.u8	q2, sl, r7, #1
[^>]*> ee0b 4f66 	viwdup.u8	q2, sl, r7, #1
[^>]*> ee0b 5f67 	vdwdup.u8	q2, sl, r7, #2
[^>]*> ee0b 4f67 	viwdup.u8	q2, sl, r7, #2
[^>]*> ee0b 5fe6 	vdwdup.u8	q2, sl, r7, #4
[^>]*> ee0b 4fe6 	viwdup.u8	q2, sl, r7, #4
[^>]*> ee0b 5fe7 	vdwdup.u8	q2, sl, r7, #8
[^>]*> ee0b 4fe7 	viwdup.u8	q2, sl, r7, #8
[^>]*> ee0b 5f68 	vdwdup.u8	q2, sl, r9, #1
[^>]*> ee0b 4f68 	viwdup.u8	q2, sl, r9, #1
[^>]*> ee0b 5f69 	vdwdup.u8	q2, sl, r9, #2
[^>]*> ee0b 4f69 	viwdup.u8	q2, sl, r9, #2
[^>]*> ee0b 5fe8 	vdwdup.u8	q2, sl, r9, #4
[^>]*> ee0b 4fe8 	viwdup.u8	q2, sl, r9, #4
[^>]*> ee0b 5fe9 	vdwdup.u8	q2, sl, r9, #8
[^>]*> ee0b 4fe9 	viwdup.u8	q2, sl, r9, #8
[^>]*> ee0b 5f6a 	vdwdup.u8	q2, sl, fp, #1
[^>]*> ee0b 4f6a 	viwdup.u8	q2, sl, fp, #1
[^>]*> ee0b 5f6b 	vdwdup.u8	q2, sl, fp, #2
[^>]*> ee0b 4f6b 	viwdup.u8	q2, sl, fp, #2
[^>]*> ee0b 5fea 	vdwdup.u8	q2, sl, fp, #4
[^>]*> ee0b 4fea 	viwdup.u8	q2, sl, fp, #4
[^>]*> ee0b 5feb 	vdwdup.u8	q2, sl, fp, #8
[^>]*> ee0b 4feb 	viwdup.u8	q2, sl, fp, #8
[^>]*> ee0d 5f6e 	vddup.u8	q2, ip, #1
[^>]*> ee0d 4f6e 	vidup.u8	q2, ip, #1
[^>]*> ee0d 5f6f 	vddup.u8	q2, ip, #2
[^>]*> ee0d 4f6f 	vidup.u8	q2, ip, #2
[^>]*> ee0d 5fee 	vddup.u8	q2, ip, #4
[^>]*> ee0d 4fee 	vidup.u8	q2, ip, #4
[^>]*> ee0d 5fef 	vddup.u8	q2, ip, #8
[^>]*> ee0d 4fef 	vidup.u8	q2, ip, #8
[^>]*> ee0d 5f60 	vdwdup.u8	q2, ip, r1, #1
[^>]*> ee0d 4f60 	viwdup.u8	q2, ip, r1, #1
[^>]*> ee0d 5f61 	vdwdup.u8	q2, ip, r1, #2
[^>]*> ee0d 4f61 	viwdup.u8	q2, ip, r1, #2
[^>]*> ee0d 5fe0 	vdwdup.u8	q2, ip, r1, #4
[^>]*> ee0d 4fe0 	viwdup.u8	q2, ip, r1, #4
[^>]*> ee0d 5fe1 	vdwdup.u8	q2, ip, r1, #8
[^>]*> ee0d 4fe1 	viwdup.u8	q2, ip, r1, #8
[^>]*> ee0d 5f62 	vdwdup.u8	q2, ip, r3, #1
[^>]*> ee0d 4f62 	viwdup.u8	q2, ip, r3, #1
[^>]*> ee0d 5f63 	vdwdup.u8	q2, ip, r3, #2
[^>]*> ee0d 4f63 	viwdup.u8	q2, ip, r3, #2
[^>]*> ee0d 5fe2 	vdwdup.u8	q2, ip, r3, #4
[^>]*> ee0d 4fe2 	viwdup.u8	q2, ip, r3, #4
[^>]*> ee0d 5fe3 	vdwdup.u8	q2, ip, r3, #8
[^>]*> ee0d 4fe3 	viwdup.u8	q2, ip, r3, #8
[^>]*> ee0d 5f64 	vdwdup.u8	q2, ip, r5, #1
[^>]*> ee0d 4f64 	viwdup.u8	q2, ip, r5, #1
[^>]*> ee0d 5f65 	vdwdup.u8	q2, ip, r5, #2
[^>]*> ee0d 4f65 	viwdup.u8	q2, ip, r5, #2
[^>]*> ee0d 5fe4 	vdwdup.u8	q2, ip, r5, #4
[^>]*> ee0d 4fe4 	viwdup.u8	q2, ip, r5, #4
[^>]*> ee0d 5fe5 	vdwdup.u8	q2, ip, r5, #8
[^>]*> ee0d 4fe5 	viwdup.u8	q2, ip, r5, #8
[^>]*> ee0d 5f66 	vdwdup.u8	q2, ip, r7, #1
[^>]*> ee0d 4f66 	viwdup.u8	q2, ip, r7, #1
[^>]*> ee0d 5f67 	vdwdup.u8	q2, ip, r7, #2
[^>]*> ee0d 4f67 	viwdup.u8	q2, ip, r7, #2
[^>]*> ee0d 5fe6 	vdwdup.u8	q2, ip, r7, #4
[^>]*> ee0d 4fe6 	viwdup.u8	q2, ip, r7, #4
[^>]*> ee0d 5fe7 	vdwdup.u8	q2, ip, r7, #8
[^>]*> ee0d 4fe7 	viwdup.u8	q2, ip, r7, #8
[^>]*> ee0d 5f68 	vdwdup.u8	q2, ip, r9, #1
[^>]*> ee0d 4f68 	viwdup.u8	q2, ip, r9, #1
[^>]*> ee0d 5f69 	vdwdup.u8	q2, ip, r9, #2
[^>]*> ee0d 4f69 	viwdup.u8	q2, ip, r9, #2
[^>]*> ee0d 5fe8 	vdwdup.u8	q2, ip, r9, #4
[^>]*> ee0d 4fe8 	viwdup.u8	q2, ip, r9, #4
[^>]*> ee0d 5fe9 	vdwdup.u8	q2, ip, r9, #8
[^>]*> ee0d 4fe9 	viwdup.u8	q2, ip, r9, #8
[^>]*> ee0d 5f6a 	vdwdup.u8	q2, ip, fp, #1
[^>]*> ee0d 4f6a 	viwdup.u8	q2, ip, fp, #1
[^>]*> ee0d 5f6b 	vdwdup.u8	q2, ip, fp, #2
[^>]*> ee0d 4f6b 	viwdup.u8	q2, ip, fp, #2
[^>]*> ee0d 5fea 	vdwdup.u8	q2, ip, fp, #4
[^>]*> ee0d 4fea 	viwdup.u8	q2, ip, fp, #4
[^>]*> ee0d 5feb 	vdwdup.u8	q2, ip, fp, #8
[^>]*> ee0d 4feb 	viwdup.u8	q2, ip, fp, #8
[^>]*> ee01 9f6e 	vddup.u8	q4, r0, #1
[^>]*> ee01 8f6e 	vidup.u8	q4, r0, #1
[^>]*> ee01 9f6f 	vddup.u8	q4, r0, #2
[^>]*> ee01 8f6f 	vidup.u8	q4, r0, #2
[^>]*> ee01 9fee 	vddup.u8	q4, r0, #4
[^>]*> ee01 8fee 	vidup.u8	q4, r0, #4
[^>]*> ee01 9fef 	vddup.u8	q4, r0, #8
[^>]*> ee01 8fef 	vidup.u8	q4, r0, #8
[^>]*> ee01 9f60 	vdwdup.u8	q4, r0, r1, #1
[^>]*> ee01 8f60 	viwdup.u8	q4, r0, r1, #1
[^>]*> ee01 9f61 	vdwdup.u8	q4, r0, r1, #2
[^>]*> ee01 8f61 	viwdup.u8	q4, r0, r1, #2
[^>]*> ee01 9fe0 	vdwdup.u8	q4, r0, r1, #4
[^>]*> ee01 8fe0 	viwdup.u8	q4, r0, r1, #4
[^>]*> ee01 9fe1 	vdwdup.u8	q4, r0, r1, #8
[^>]*> ee01 8fe1 	viwdup.u8	q4, r0, r1, #8
[^>]*> ee01 9f62 	vdwdup.u8	q4, r0, r3, #1
[^>]*> ee01 8f62 	viwdup.u8	q4, r0, r3, #1
[^>]*> ee01 9f63 	vdwdup.u8	q4, r0, r3, #2
[^>]*> ee01 8f63 	viwdup.u8	q4, r0, r3, #2
[^>]*> ee01 9fe2 	vdwdup.u8	q4, r0, r3, #4
[^>]*> ee01 8fe2 	viwdup.u8	q4, r0, r3, #4
[^>]*> ee01 9fe3 	vdwdup.u8	q4, r0, r3, #8
[^>]*> ee01 8fe3 	viwdup.u8	q4, r0, r3, #8
[^>]*> ee01 9f64 	vdwdup.u8	q4, r0, r5, #1
[^>]*> ee01 8f64 	viwdup.u8	q4, r0, r5, #1
[^>]*> ee01 9f65 	vdwdup.u8	q4, r0, r5, #2
[^>]*> ee01 8f65 	viwdup.u8	q4, r0, r5, #2
[^>]*> ee01 9fe4 	vdwdup.u8	q4, r0, r5, #4
[^>]*> ee01 8fe4 	viwdup.u8	q4, r0, r5, #4
[^>]*> ee01 9fe5 	vdwdup.u8	q4, r0, r5, #8
[^>]*> ee01 8fe5 	viwdup.u8	q4, r0, r5, #8
[^>]*> ee01 9f66 	vdwdup.u8	q4, r0, r7, #1
[^>]*> ee01 8f66 	viwdup.u8	q4, r0, r7, #1
[^>]*> ee01 9f67 	vdwdup.u8	q4, r0, r7, #2
[^>]*> ee01 8f67 	viwdup.u8	q4, r0, r7, #2
[^>]*> ee01 9fe6 	vdwdup.u8	q4, r0, r7, #4
[^>]*> ee01 8fe6 	viwdup.u8	q4, r0, r7, #4
[^>]*> ee01 9fe7 	vdwdup.u8	q4, r0, r7, #8
[^>]*> ee01 8fe7 	viwdup.u8	q4, r0, r7, #8
[^>]*> ee01 9f68 	vdwdup.u8	q4, r0, r9, #1
[^>]*> ee01 8f68 	viwdup.u8	q4, r0, r9, #1
[^>]*> ee01 9f69 	vdwdup.u8	q4, r0, r9, #2
[^>]*> ee01 8f69 	viwdup.u8	q4, r0, r9, #2
[^>]*> ee01 9fe8 	vdwdup.u8	q4, r0, r9, #4
[^>]*> ee01 8fe8 	viwdup.u8	q4, r0, r9, #4
[^>]*> ee01 9fe9 	vdwdup.u8	q4, r0, r9, #8
[^>]*> ee01 8fe9 	viwdup.u8	q4, r0, r9, #8
[^>]*> ee01 9f6a 	vdwdup.u8	q4, r0, fp, #1
[^>]*> ee01 8f6a 	viwdup.u8	q4, r0, fp, #1
[^>]*> ee01 9f6b 	vdwdup.u8	q4, r0, fp, #2
[^>]*> ee01 8f6b 	viwdup.u8	q4, r0, fp, #2
[^>]*> ee01 9fea 	vdwdup.u8	q4, r0, fp, #4
[^>]*> ee01 8fea 	viwdup.u8	q4, r0, fp, #4
[^>]*> ee01 9feb 	vdwdup.u8	q4, r0, fp, #8
[^>]*> ee01 8feb 	viwdup.u8	q4, r0, fp, #8
[^>]*> ee03 9f6e 	vddup.u8	q4, r2, #1
[^>]*> ee03 8f6e 	vidup.u8	q4, r2, #1
[^>]*> ee03 9f6f 	vddup.u8	q4, r2, #2
[^>]*> ee03 8f6f 	vidup.u8	q4, r2, #2
[^>]*> ee03 9fee 	vddup.u8	q4, r2, #4
[^>]*> ee03 8fee 	vidup.u8	q4, r2, #4
[^>]*> ee03 9fef 	vddup.u8	q4, r2, #8
[^>]*> ee03 8fef 	vidup.u8	q4, r2, #8
[^>]*> ee03 9f60 	vdwdup.u8	q4, r2, r1, #1
[^>]*> ee03 8f60 	viwdup.u8	q4, r2, r1, #1
[^>]*> ee03 9f61 	vdwdup.u8	q4, r2, r1, #2
[^>]*> ee03 8f61 	viwdup.u8	q4, r2, r1, #2
[^>]*> ee03 9fe0 	vdwdup.u8	q4, r2, r1, #4
[^>]*> ee03 8fe0 	viwdup.u8	q4, r2, r1, #4
[^>]*> ee03 9fe1 	vdwdup.u8	q4, r2, r1, #8
[^>]*> ee03 8fe1 	viwdup.u8	q4, r2, r1, #8
[^>]*> ee03 9f62 	vdwdup.u8	q4, r2, r3, #1
[^>]*> ee03 8f62 	viwdup.u8	q4, r2, r3, #1
[^>]*> ee03 9f63 	vdwdup.u8	q4, r2, r3, #2
[^>]*> ee03 8f63 	viwdup.u8	q4, r2, r3, #2
[^>]*> ee03 9fe2 	vdwdup.u8	q4, r2, r3, #4
[^>]*> ee03 8fe2 	viwdup.u8	q4, r2, r3, #4
[^>]*> ee03 9fe3 	vdwdup.u8	q4, r2, r3, #8
[^>]*> ee03 8fe3 	viwdup.u8	q4, r2, r3, #8
[^>]*> ee03 9f64 	vdwdup.u8	q4, r2, r5, #1
[^>]*> ee03 8f64 	viwdup.u8	q4, r2, r5, #1
[^>]*> ee03 9f65 	vdwdup.u8	q4, r2, r5, #2
[^>]*> ee03 8f65 	viwdup.u8	q4, r2, r5, #2
[^>]*> ee03 9fe4 	vdwdup.u8	q4, r2, r5, #4
[^>]*> ee03 8fe4 	viwdup.u8	q4, r2, r5, #4
[^>]*> ee03 9fe5 	vdwdup.u8	q4, r2, r5, #8
[^>]*> ee03 8fe5 	viwdup.u8	q4, r2, r5, #8
[^>]*> ee03 9f66 	vdwdup.u8	q4, r2, r7, #1
[^>]*> ee03 8f66 	viwdup.u8	q4, r2, r7, #1
[^>]*> ee03 9f67 	vdwdup.u8	q4, r2, r7, #2
[^>]*> ee03 8f67 	viwdup.u8	q4, r2, r7, #2
[^>]*> ee03 9fe6 	vdwdup.u8	q4, r2, r7, #4
[^>]*> ee03 8fe6 	viwdup.u8	q4, r2, r7, #4
[^>]*> ee03 9fe7 	vdwdup.u8	q4, r2, r7, #8
[^>]*> ee03 8fe7 	viwdup.u8	q4, r2, r7, #8
[^>]*> ee03 9f68 	vdwdup.u8	q4, r2, r9, #1
[^>]*> ee03 8f68 	viwdup.u8	q4, r2, r9, #1
[^>]*> ee03 9f69 	vdwdup.u8	q4, r2, r9, #2
[^>]*> ee03 8f69 	viwdup.u8	q4, r2, r9, #2
[^>]*> ee03 9fe8 	vdwdup.u8	q4, r2, r9, #4
[^>]*> ee03 8fe8 	viwdup.u8	q4, r2, r9, #4
[^>]*> ee03 9fe9 	vdwdup.u8	q4, r2, r9, #8
[^>]*> ee03 8fe9 	viwdup.u8	q4, r2, r9, #8
[^>]*> ee03 9f6a 	vdwdup.u8	q4, r2, fp, #1
[^>]*> ee03 8f6a 	viwdup.u8	q4, r2, fp, #1
[^>]*> ee03 9f6b 	vdwdup.u8	q4, r2, fp, #2
[^>]*> ee03 8f6b 	viwdup.u8	q4, r2, fp, #2
[^>]*> ee03 9fea 	vdwdup.u8	q4, r2, fp, #4
[^>]*> ee03 8fea 	viwdup.u8	q4, r2, fp, #4
[^>]*> ee03 9feb 	vdwdup.u8	q4, r2, fp, #8
[^>]*> ee03 8feb 	viwdup.u8	q4, r2, fp, #8
[^>]*> ee05 9f6e 	vddup.u8	q4, r4, #1
[^>]*> ee05 8f6e 	vidup.u8	q4, r4, #1
[^>]*> ee05 9f6f 	vddup.u8	q4, r4, #2
[^>]*> ee05 8f6f 	vidup.u8	q4, r4, #2
[^>]*> ee05 9fee 	vddup.u8	q4, r4, #4
[^>]*> ee05 8fee 	vidup.u8	q4, r4, #4
[^>]*> ee05 9fef 	vddup.u8	q4, r4, #8
[^>]*> ee05 8fef 	vidup.u8	q4, r4, #8
[^>]*> ee05 9f60 	vdwdup.u8	q4, r4, r1, #1
[^>]*> ee05 8f60 	viwdup.u8	q4, r4, r1, #1
[^>]*> ee05 9f61 	vdwdup.u8	q4, r4, r1, #2
[^>]*> ee05 8f61 	viwdup.u8	q4, r4, r1, #2
[^>]*> ee05 9fe0 	vdwdup.u8	q4, r4, r1, #4
[^>]*> ee05 8fe0 	viwdup.u8	q4, r4, r1, #4
[^>]*> ee05 9fe1 	vdwdup.u8	q4, r4, r1, #8
[^>]*> ee05 8fe1 	viwdup.u8	q4, r4, r1, #8
[^>]*> ee05 9f62 	vdwdup.u8	q4, r4, r3, #1
[^>]*> ee05 8f62 	viwdup.u8	q4, r4, r3, #1
[^>]*> ee05 9f63 	vdwdup.u8	q4, r4, r3, #2
[^>]*> ee05 8f63 	viwdup.u8	q4, r4, r3, #2
[^>]*> ee05 9fe2 	vdwdup.u8	q4, r4, r3, #4
[^>]*> ee05 8fe2 	viwdup.u8	q4, r4, r3, #4
[^>]*> ee05 9fe3 	vdwdup.u8	q4, r4, r3, #8
[^>]*> ee05 8fe3 	viwdup.u8	q4, r4, r3, #8
[^>]*> ee05 9f64 	vdwdup.u8	q4, r4, r5, #1
[^>]*> ee05 8f64 	viwdup.u8	q4, r4, r5, #1
[^>]*> ee05 9f65 	vdwdup.u8	q4, r4, r5, #2
[^>]*> ee05 8f65 	viwdup.u8	q4, r4, r5, #2
[^>]*> ee05 9fe4 	vdwdup.u8	q4, r4, r5, #4
[^>]*> ee05 8fe4 	viwdup.u8	q4, r4, r5, #4
[^>]*> ee05 9fe5 	vdwdup.u8	q4, r4, r5, #8
[^>]*> ee05 8fe5 	viwdup.u8	q4, r4, r5, #8
[^>]*> ee05 9f66 	vdwdup.u8	q4, r4, r7, #1
[^>]*> ee05 8f66 	viwdup.u8	q4, r4, r7, #1
[^>]*> ee05 9f67 	vdwdup.u8	q4, r4, r7, #2
[^>]*> ee05 8f67 	viwdup.u8	q4, r4, r7, #2
[^>]*> ee05 9fe6 	vdwdup.u8	q4, r4, r7, #4
[^>]*> ee05 8fe6 	viwdup.u8	q4, r4, r7, #4
[^>]*> ee05 9fe7 	vdwdup.u8	q4, r4, r7, #8
[^>]*> ee05 8fe7 	viwdup.u8	q4, r4, r7, #8
[^>]*> ee05 9f68 	vdwdup.u8	q4, r4, r9, #1
[^>]*> ee05 8f68 	viwdup.u8	q4, r4, r9, #1
[^>]*> ee05 9f69 	vdwdup.u8	q4, r4, r9, #2
[^>]*> ee05 8f69 	viwdup.u8	q4, r4, r9, #2
[^>]*> ee05 9fe8 	vdwdup.u8	q4, r4, r9, #4
[^>]*> ee05 8fe8 	viwdup.u8	q4, r4, r9, #4
[^>]*> ee05 9fe9 	vdwdup.u8	q4, r4, r9, #8
[^>]*> ee05 8fe9 	viwdup.u8	q4, r4, r9, #8
[^>]*> ee05 9f6a 	vdwdup.u8	q4, r4, fp, #1
[^>]*> ee05 8f6a 	viwdup.u8	q4, r4, fp, #1
[^>]*> ee05 9f6b 	vdwdup.u8	q4, r4, fp, #2
[^>]*> ee05 8f6b 	viwdup.u8	q4, r4, fp, #2
[^>]*> ee05 9fea 	vdwdup.u8	q4, r4, fp, #4
[^>]*> ee05 8fea 	viwdup.u8	q4, r4, fp, #4
[^>]*> ee05 9feb 	vdwdup.u8	q4, r4, fp, #8
[^>]*> ee05 8feb 	viwdup.u8	q4, r4, fp, #8
[^>]*> ee07 9f6e 	vddup.u8	q4, r6, #1
[^>]*> ee07 8f6e 	vidup.u8	q4, r6, #1
[^>]*> ee07 9f6f 	vddup.u8	q4, r6, #2
[^>]*> ee07 8f6f 	vidup.u8	q4, r6, #2
[^>]*> ee07 9fee 	vddup.u8	q4, r6, #4
[^>]*> ee07 8fee 	vidup.u8	q4, r6, #4
[^>]*> ee07 9fef 	vddup.u8	q4, r6, #8
[^>]*> ee07 8fef 	vidup.u8	q4, r6, #8
[^>]*> ee07 9f60 	vdwdup.u8	q4, r6, r1, #1
[^>]*> ee07 8f60 	viwdup.u8	q4, r6, r1, #1
[^>]*> ee07 9f61 	vdwdup.u8	q4, r6, r1, #2
[^>]*> ee07 8f61 	viwdup.u8	q4, r6, r1, #2
[^>]*> ee07 9fe0 	vdwdup.u8	q4, r6, r1, #4
[^>]*> ee07 8fe0 	viwdup.u8	q4, r6, r1, #4
[^>]*> ee07 9fe1 	vdwdup.u8	q4, r6, r1, #8
[^>]*> ee07 8fe1 	viwdup.u8	q4, r6, r1, #8
[^>]*> ee07 9f62 	vdwdup.u8	q4, r6, r3, #1
[^>]*> ee07 8f62 	viwdup.u8	q4, r6, r3, #1
[^>]*> ee07 9f63 	vdwdup.u8	q4, r6, r3, #2
[^>]*> ee07 8f63 	viwdup.u8	q4, r6, r3, #2
[^>]*> ee07 9fe2 	vdwdup.u8	q4, r6, r3, #4
[^>]*> ee07 8fe2 	viwdup.u8	q4, r6, r3, #4
[^>]*> ee07 9fe3 	vdwdup.u8	q4, r6, r3, #8
[^>]*> ee07 8fe3 	viwdup.u8	q4, r6, r3, #8
[^>]*> ee07 9f64 	vdwdup.u8	q4, r6, r5, #1
[^>]*> ee07 8f64 	viwdup.u8	q4, r6, r5, #1
[^>]*> ee07 9f65 	vdwdup.u8	q4, r6, r5, #2
[^>]*> ee07 8f65 	viwdup.u8	q4, r6, r5, #2
[^>]*> ee07 9fe4 	vdwdup.u8	q4, r6, r5, #4
[^>]*> ee07 8fe4 	viwdup.u8	q4, r6, r5, #4
[^>]*> ee07 9fe5 	vdwdup.u8	q4, r6, r5, #8
[^>]*> ee07 8fe5 	viwdup.u8	q4, r6, r5, #8
[^>]*> ee07 9f66 	vdwdup.u8	q4, r6, r7, #1
[^>]*> ee07 8f66 	viwdup.u8	q4, r6, r7, #1
[^>]*> ee07 9f67 	vdwdup.u8	q4, r6, r7, #2
[^>]*> ee07 8f67 	viwdup.u8	q4, r6, r7, #2
[^>]*> ee07 9fe6 	vdwdup.u8	q4, r6, r7, #4
[^>]*> ee07 8fe6 	viwdup.u8	q4, r6, r7, #4
[^>]*> ee07 9fe7 	vdwdup.u8	q4, r6, r7, #8
[^>]*> ee07 8fe7 	viwdup.u8	q4, r6, r7, #8
[^>]*> ee07 9f68 	vdwdup.u8	q4, r6, r9, #1
[^>]*> ee07 8f68 	viwdup.u8	q4, r6, r9, #1
[^>]*> ee07 9f69 	vdwdup.u8	q4, r6, r9, #2
[^>]*> ee07 8f69 	viwdup.u8	q4, r6, r9, #2
[^>]*> ee07 9fe8 	vdwdup.u8	q4, r6, r9, #4
[^>]*> ee07 8fe8 	viwdup.u8	q4, r6, r9, #4
[^>]*> ee07 9fe9 	vdwdup.u8	q4, r6, r9, #8
[^>]*> ee07 8fe9 	viwdup.u8	q4, r6, r9, #8
[^>]*> ee07 9f6a 	vdwdup.u8	q4, r6, fp, #1
[^>]*> ee07 8f6a 	viwdup.u8	q4, r6, fp, #1
[^>]*> ee07 9f6b 	vdwdup.u8	q4, r6, fp, #2
[^>]*> ee07 8f6b 	viwdup.u8	q4, r6, fp, #2
[^>]*> ee07 9fea 	vdwdup.u8	q4, r6, fp, #4
[^>]*> ee07 8fea 	viwdup.u8	q4, r6, fp, #4
[^>]*> ee07 9feb 	vdwdup.u8	q4, r6, fp, #8
[^>]*> ee07 8feb 	viwdup.u8	q4, r6, fp, #8
[^>]*> ee09 9f6e 	vddup.u8	q4, r8, #1
[^>]*> ee09 8f6e 	vidup.u8	q4, r8, #1
[^>]*> ee09 9f6f 	vddup.u8	q4, r8, #2
[^>]*> ee09 8f6f 	vidup.u8	q4, r8, #2
[^>]*> ee09 9fee 	vddup.u8	q4, r8, #4
[^>]*> ee09 8fee 	vidup.u8	q4, r8, #4
[^>]*> ee09 9fef 	vddup.u8	q4, r8, #8
[^>]*> ee09 8fef 	vidup.u8	q4, r8, #8
[^>]*> ee09 9f60 	vdwdup.u8	q4, r8, r1, #1
[^>]*> ee09 8f60 	viwdup.u8	q4, r8, r1, #1
[^>]*> ee09 9f61 	vdwdup.u8	q4, r8, r1, #2
[^>]*> ee09 8f61 	viwdup.u8	q4, r8, r1, #2
[^>]*> ee09 9fe0 	vdwdup.u8	q4, r8, r1, #4
[^>]*> ee09 8fe0 	viwdup.u8	q4, r8, r1, #4
[^>]*> ee09 9fe1 	vdwdup.u8	q4, r8, r1, #8
[^>]*> ee09 8fe1 	viwdup.u8	q4, r8, r1, #8
[^>]*> ee09 9f62 	vdwdup.u8	q4, r8, r3, #1
[^>]*> ee09 8f62 	viwdup.u8	q4, r8, r3, #1
[^>]*> ee09 9f63 	vdwdup.u8	q4, r8, r3, #2
[^>]*> ee09 8f63 	viwdup.u8	q4, r8, r3, #2
[^>]*> ee09 9fe2 	vdwdup.u8	q4, r8, r3, #4
[^>]*> ee09 8fe2 	viwdup.u8	q4, r8, r3, #4
[^>]*> ee09 9fe3 	vdwdup.u8	q4, r8, r3, #8
[^>]*> ee09 8fe3 	viwdup.u8	q4, r8, r3, #8
[^>]*> ee09 9f64 	vdwdup.u8	q4, r8, r5, #1
[^>]*> ee09 8f64 	viwdup.u8	q4, r8, r5, #1
[^>]*> ee09 9f65 	vdwdup.u8	q4, r8, r5, #2
[^>]*> ee09 8f65 	viwdup.u8	q4, r8, r5, #2
[^>]*> ee09 9fe4 	vdwdup.u8	q4, r8, r5, #4
[^>]*> ee09 8fe4 	viwdup.u8	q4, r8, r5, #4
[^>]*> ee09 9fe5 	vdwdup.u8	q4, r8, r5, #8
[^>]*> ee09 8fe5 	viwdup.u8	q4, r8, r5, #8
[^>]*> ee09 9f66 	vdwdup.u8	q4, r8, r7, #1
[^>]*> ee09 8f66 	viwdup.u8	q4, r8, r7, #1
[^>]*> ee09 9f67 	vdwdup.u8	q4, r8, r7, #2
[^>]*> ee09 8f67 	viwdup.u8	q4, r8, r7, #2
[^>]*> ee09 9fe6 	vdwdup.u8	q4, r8, r7, #4
[^>]*> ee09 8fe6 	viwdup.u8	q4, r8, r7, #4
[^>]*> ee09 9fe7 	vdwdup.u8	q4, r8, r7, #8
[^>]*> ee09 8fe7 	viwdup.u8	q4, r8, r7, #8
[^>]*> ee09 9f68 	vdwdup.u8	q4, r8, r9, #1
[^>]*> ee09 8f68 	viwdup.u8	q4, r8, r9, #1
[^>]*> ee09 9f69 	vdwdup.u8	q4, r8, r9, #2
[^>]*> ee09 8f69 	viwdup.u8	q4, r8, r9, #2
[^>]*> ee09 9fe8 	vdwdup.u8	q4, r8, r9, #4
[^>]*> ee09 8fe8 	viwdup.u8	q4, r8, r9, #4
[^>]*> ee09 9fe9 	vdwdup.u8	q4, r8, r9, #8
[^>]*> ee09 8fe9 	viwdup.u8	q4, r8, r9, #8
[^>]*> ee09 9f6a 	vdwdup.u8	q4, r8, fp, #1
[^>]*> ee09 8f6a 	viwdup.u8	q4, r8, fp, #1
[^>]*> ee09 9f6b 	vdwdup.u8	q4, r8, fp, #2
[^>]*> ee09 8f6b 	viwdup.u8	q4, r8, fp, #2
[^>]*> ee09 9fea 	vdwdup.u8	q4, r8, fp, #4
[^>]*> ee09 8fea 	viwdup.u8	q4, r8, fp, #4
[^>]*> ee09 9feb 	vdwdup.u8	q4, r8, fp, #8
[^>]*> ee09 8feb 	viwdup.u8	q4, r8, fp, #8
[^>]*> ee0b 9f6e 	vddup.u8	q4, sl, #1
[^>]*> ee0b 8f6e 	vidup.u8	q4, sl, #1
[^>]*> ee0b 9f6f 	vddup.u8	q4, sl, #2
[^>]*> ee0b 8f6f 	vidup.u8	q4, sl, #2
[^>]*> ee0b 9fee 	vddup.u8	q4, sl, #4
[^>]*> ee0b 8fee 	vidup.u8	q4, sl, #4
[^>]*> ee0b 9fef 	vddup.u8	q4, sl, #8
[^>]*> ee0b 8fef 	vidup.u8	q4, sl, #8
[^>]*> ee0b 9f60 	vdwdup.u8	q4, sl, r1, #1
[^>]*> ee0b 8f60 	viwdup.u8	q4, sl, r1, #1
[^>]*> ee0b 9f61 	vdwdup.u8	q4, sl, r1, #2
[^>]*> ee0b 8f61 	viwdup.u8	q4, sl, r1, #2
[^>]*> ee0b 9fe0 	vdwdup.u8	q4, sl, r1, #4
[^>]*> ee0b 8fe0 	viwdup.u8	q4, sl, r1, #4
[^>]*> ee0b 9fe1 	vdwdup.u8	q4, sl, r1, #8
[^>]*> ee0b 8fe1 	viwdup.u8	q4, sl, r1, #8
[^>]*> ee0b 9f62 	vdwdup.u8	q4, sl, r3, #1
[^>]*> ee0b 8f62 	viwdup.u8	q4, sl, r3, #1
[^>]*> ee0b 9f63 	vdwdup.u8	q4, sl, r3, #2
[^>]*> ee0b 8f63 	viwdup.u8	q4, sl, r3, #2
[^>]*> ee0b 9fe2 	vdwdup.u8	q4, sl, r3, #4
[^>]*> ee0b 8fe2 	viwdup.u8	q4, sl, r3, #4
[^>]*> ee0b 9fe3 	vdwdup.u8	q4, sl, r3, #8
[^>]*> ee0b 8fe3 	viwdup.u8	q4, sl, r3, #8
[^>]*> ee0b 9f64 	vdwdup.u8	q4, sl, r5, #1
[^>]*> ee0b 8f64 	viwdup.u8	q4, sl, r5, #1
[^>]*> ee0b 9f65 	vdwdup.u8	q4, sl, r5, #2
[^>]*> ee0b 8f65 	viwdup.u8	q4, sl, r5, #2
[^>]*> ee0b 9fe4 	vdwdup.u8	q4, sl, r5, #4
[^>]*> ee0b 8fe4 	viwdup.u8	q4, sl, r5, #4
[^>]*> ee0b 9fe5 	vdwdup.u8	q4, sl, r5, #8
[^>]*> ee0b 8fe5 	viwdup.u8	q4, sl, r5, #8
[^>]*> ee0b 9f66 	vdwdup.u8	q4, sl, r7, #1
[^>]*> ee0b 8f66 	viwdup.u8	q4, sl, r7, #1
[^>]*> ee0b 9f67 	vdwdup.u8	q4, sl, r7, #2
[^>]*> ee0b 8f67 	viwdup.u8	q4, sl, r7, #2
[^>]*> ee0b 9fe6 	vdwdup.u8	q4, sl, r7, #4
[^>]*> ee0b 8fe6 	viwdup.u8	q4, sl, r7, #4
[^>]*> ee0b 9fe7 	vdwdup.u8	q4, sl, r7, #8
[^>]*> ee0b 8fe7 	viwdup.u8	q4, sl, r7, #8
[^>]*> ee0b 9f68 	vdwdup.u8	q4, sl, r9, #1
[^>]*> ee0b 8f68 	viwdup.u8	q4, sl, r9, #1
[^>]*> ee0b 9f69 	vdwdup.u8	q4, sl, r9, #2
[^>]*> ee0b 8f69 	viwdup.u8	q4, sl, r9, #2
[^>]*> ee0b 9fe8 	vdwdup.u8	q4, sl, r9, #4
[^>]*> ee0b 8fe8 	viwdup.u8	q4, sl, r9, #4
[^>]*> ee0b 9fe9 	vdwdup.u8	q4, sl, r9, #8
[^>]*> ee0b 8fe9 	viwdup.u8	q4, sl, r9, #8
[^>]*> ee0b 9f6a 	vdwdup.u8	q4, sl, fp, #1
[^>]*> ee0b 8f6a 	viwdup.u8	q4, sl, fp, #1
[^>]*> ee0b 9f6b 	vdwdup.u8	q4, sl, fp, #2
[^>]*> ee0b 8f6b 	viwdup.u8	q4, sl, fp, #2
[^>]*> ee0b 9fea 	vdwdup.u8	q4, sl, fp, #4
[^>]*> ee0b 8fea 	viwdup.u8	q4, sl, fp, #4
[^>]*> ee0b 9feb 	vdwdup.u8	q4, sl, fp, #8
[^>]*> ee0b 8feb 	viwdup.u8	q4, sl, fp, #8
[^>]*> ee0d 9f6e 	vddup.u8	q4, ip, #1
[^>]*> ee0d 8f6e 	vidup.u8	q4, ip, #1
[^>]*> ee0d 9f6f 	vddup.u8	q4, ip, #2
[^>]*> ee0d 8f6f 	vidup.u8	q4, ip, #2
[^>]*> ee0d 9fee 	vddup.u8	q4, ip, #4
[^>]*> ee0d 8fee 	vidup.u8	q4, ip, #4
[^>]*> ee0d 9fef 	vddup.u8	q4, ip, #8
[^>]*> ee0d 8fef 	vidup.u8	q4, ip, #8
[^>]*> ee0d 9f60 	vdwdup.u8	q4, ip, r1, #1
[^>]*> ee0d 8f60 	viwdup.u8	q4, ip, r1, #1
[^>]*> ee0d 9f61 	vdwdup.u8	q4, ip, r1, #2
[^>]*> ee0d 8f61 	viwdup.u8	q4, ip, r1, #2
[^>]*> ee0d 9fe0 	vdwdup.u8	q4, ip, r1, #4
[^>]*> ee0d 8fe0 	viwdup.u8	q4, ip, r1, #4
[^>]*> ee0d 9fe1 	vdwdup.u8	q4, ip, r1, #8
[^>]*> ee0d 8fe1 	viwdup.u8	q4, ip, r1, #8
[^>]*> ee0d 9f62 	vdwdup.u8	q4, ip, r3, #1
[^>]*> ee0d 8f62 	viwdup.u8	q4, ip, r3, #1
[^>]*> ee0d 9f63 	vdwdup.u8	q4, ip, r3, #2
[^>]*> ee0d 8f63 	viwdup.u8	q4, ip, r3, #2
[^>]*> ee0d 9fe2 	vdwdup.u8	q4, ip, r3, #4
[^>]*> ee0d 8fe2 	viwdup.u8	q4, ip, r3, #4
[^>]*> ee0d 9fe3 	vdwdup.u8	q4, ip, r3, #8
[^>]*> ee0d 8fe3 	viwdup.u8	q4, ip, r3, #8
[^>]*> ee0d 9f64 	vdwdup.u8	q4, ip, r5, #1
[^>]*> ee0d 8f64 	viwdup.u8	q4, ip, r5, #1
[^>]*> ee0d 9f65 	vdwdup.u8	q4, ip, r5, #2
[^>]*> ee0d 8f65 	viwdup.u8	q4, ip, r5, #2
[^>]*> ee0d 9fe4 	vdwdup.u8	q4, ip, r5, #4
[^>]*> ee0d 8fe4 	viwdup.u8	q4, ip, r5, #4
[^>]*> ee0d 9fe5 	vdwdup.u8	q4, ip, r5, #8
[^>]*> ee0d 8fe5 	viwdup.u8	q4, ip, r5, #8
[^>]*> ee0d 9f66 	vdwdup.u8	q4, ip, r7, #1
[^>]*> ee0d 8f66 	viwdup.u8	q4, ip, r7, #1
[^>]*> ee0d 9f67 	vdwdup.u8	q4, ip, r7, #2
[^>]*> ee0d 8f67 	viwdup.u8	q4, ip, r7, #2
[^>]*> ee0d 9fe6 	vdwdup.u8	q4, ip, r7, #4
[^>]*> ee0d 8fe6 	viwdup.u8	q4, ip, r7, #4
[^>]*> ee0d 9fe7 	vdwdup.u8	q4, ip, r7, #8
[^>]*> ee0d 8fe7 	viwdup.u8	q4, ip, r7, #8
[^>]*> ee0d 9f68 	vdwdup.u8	q4, ip, r9, #1
[^>]*> ee0d 8f68 	viwdup.u8	q4, ip, r9, #1
[^>]*> ee0d 9f69 	vdwdup.u8	q4, ip, r9, #2
[^>]*> ee0d 8f69 	viwdup.u8	q4, ip, r9, #2
[^>]*> ee0d 9fe8 	vdwdup.u8	q4, ip, r9, #4
[^>]*> ee0d 8fe8 	viwdup.u8	q4, ip, r9, #4
[^>]*> ee0d 9fe9 	vdwdup.u8	q4, ip, r9, #8
[^>]*> ee0d 8fe9 	viwdup.u8	q4, ip, r9, #8
[^>]*> ee0d 9f6a 	vdwdup.u8	q4, ip, fp, #1
[^>]*> ee0d 8f6a 	viwdup.u8	q4, ip, fp, #1
[^>]*> ee0d 9f6b 	vdwdup.u8	q4, ip, fp, #2
[^>]*> ee0d 8f6b 	viwdup.u8	q4, ip, fp, #2
[^>]*> ee0d 9fea 	vdwdup.u8	q4, ip, fp, #4
[^>]*> ee0d 8fea 	viwdup.u8	q4, ip, fp, #4
[^>]*> ee0d 9feb 	vdwdup.u8	q4, ip, fp, #8
[^>]*> ee0d 8feb 	viwdup.u8	q4, ip, fp, #8
[^>]*> ee01 ff6e 	vddup.u8	q7, r0, #1
[^>]*> ee01 ef6e 	vidup.u8	q7, r0, #1
[^>]*> ee01 ff6f 	vddup.u8	q7, r0, #2
[^>]*> ee01 ef6f 	vidup.u8	q7, r0, #2
[^>]*> ee01 ffee 	vddup.u8	q7, r0, #4
[^>]*> ee01 efee 	vidup.u8	q7, r0, #4
[^>]*> ee01 ffef 	vddup.u8	q7, r0, #8
[^>]*> ee01 efef 	vidup.u8	q7, r0, #8
[^>]*> ee01 ff60 	vdwdup.u8	q7, r0, r1, #1
[^>]*> ee01 ef60 	viwdup.u8	q7, r0, r1, #1
[^>]*> ee01 ff61 	vdwdup.u8	q7, r0, r1, #2
[^>]*> ee01 ef61 	viwdup.u8	q7, r0, r1, #2
[^>]*> ee01 ffe0 	vdwdup.u8	q7, r0, r1, #4
[^>]*> ee01 efe0 	viwdup.u8	q7, r0, r1, #4
[^>]*> ee01 ffe1 	vdwdup.u8	q7, r0, r1, #8
[^>]*> ee01 efe1 	viwdup.u8	q7, r0, r1, #8
[^>]*> ee01 ff62 	vdwdup.u8	q7, r0, r3, #1
[^>]*> ee01 ef62 	viwdup.u8	q7, r0, r3, #1
[^>]*> ee01 ff63 	vdwdup.u8	q7, r0, r3, #2
[^>]*> ee01 ef63 	viwdup.u8	q7, r0, r3, #2
[^>]*> ee01 ffe2 	vdwdup.u8	q7, r0, r3, #4
[^>]*> ee01 efe2 	viwdup.u8	q7, r0, r3, #4
[^>]*> ee01 ffe3 	vdwdup.u8	q7, r0, r3, #8
[^>]*> ee01 efe3 	viwdup.u8	q7, r0, r3, #8
[^>]*> ee01 ff64 	vdwdup.u8	q7, r0, r5, #1
[^>]*> ee01 ef64 	viwdup.u8	q7, r0, r5, #1
[^>]*> ee01 ff65 	vdwdup.u8	q7, r0, r5, #2
[^>]*> ee01 ef65 	viwdup.u8	q7, r0, r5, #2
[^>]*> ee01 ffe4 	vdwdup.u8	q7, r0, r5, #4
[^>]*> ee01 efe4 	viwdup.u8	q7, r0, r5, #4
[^>]*> ee01 ffe5 	vdwdup.u8	q7, r0, r5, #8
[^>]*> ee01 efe5 	viwdup.u8	q7, r0, r5, #8
[^>]*> ee01 ff66 	vdwdup.u8	q7, r0, r7, #1
[^>]*> ee01 ef66 	viwdup.u8	q7, r0, r7, #1
[^>]*> ee01 ff67 	vdwdup.u8	q7, r0, r7, #2
[^>]*> ee01 ef67 	viwdup.u8	q7, r0, r7, #2
[^>]*> ee01 ffe6 	vdwdup.u8	q7, r0, r7, #4
[^>]*> ee01 efe6 	viwdup.u8	q7, r0, r7, #4
[^>]*> ee01 ffe7 	vdwdup.u8	q7, r0, r7, #8
[^>]*> ee01 efe7 	viwdup.u8	q7, r0, r7, #8
[^>]*> ee01 ff68 	vdwdup.u8	q7, r0, r9, #1
[^>]*> ee01 ef68 	viwdup.u8	q7, r0, r9, #1
[^>]*> ee01 ff69 	vdwdup.u8	q7, r0, r9, #2
[^>]*> ee01 ef69 	viwdup.u8	q7, r0, r9, #2
[^>]*> ee01 ffe8 	vdwdup.u8	q7, r0, r9, #4
[^>]*> ee01 efe8 	viwdup.u8	q7, r0, r9, #4
[^>]*> ee01 ffe9 	vdwdup.u8	q7, r0, r9, #8
[^>]*> ee01 efe9 	viwdup.u8	q7, r0, r9, #8
[^>]*> ee01 ff6a 	vdwdup.u8	q7, r0, fp, #1
[^>]*> ee01 ef6a 	viwdup.u8	q7, r0, fp, #1
[^>]*> ee01 ff6b 	vdwdup.u8	q7, r0, fp, #2
[^>]*> ee01 ef6b 	viwdup.u8	q7, r0, fp, #2
[^>]*> ee01 ffea 	vdwdup.u8	q7, r0, fp, #4
[^>]*> ee01 efea 	viwdup.u8	q7, r0, fp, #4
[^>]*> ee01 ffeb 	vdwdup.u8	q7, r0, fp, #8
[^>]*> ee01 efeb 	viwdup.u8	q7, r0, fp, #8
[^>]*> ee03 ff6e 	vddup.u8	q7, r2, #1
[^>]*> ee03 ef6e 	vidup.u8	q7, r2, #1
[^>]*> ee03 ff6f 	vddup.u8	q7, r2, #2
[^>]*> ee03 ef6f 	vidup.u8	q7, r2, #2
[^>]*> ee03 ffee 	vddup.u8	q7, r2, #4
[^>]*> ee03 efee 	vidup.u8	q7, r2, #4
[^>]*> ee03 ffef 	vddup.u8	q7, r2, #8
[^>]*> ee03 efef 	vidup.u8	q7, r2, #8
[^>]*> ee03 ff60 	vdwdup.u8	q7, r2, r1, #1
[^>]*> ee03 ef60 	viwdup.u8	q7, r2, r1, #1
[^>]*> ee03 ff61 	vdwdup.u8	q7, r2, r1, #2
[^>]*> ee03 ef61 	viwdup.u8	q7, r2, r1, #2
[^>]*> ee03 ffe0 	vdwdup.u8	q7, r2, r1, #4
[^>]*> ee03 efe0 	viwdup.u8	q7, r2, r1, #4
[^>]*> ee03 ffe1 	vdwdup.u8	q7, r2, r1, #8
[^>]*> ee03 efe1 	viwdup.u8	q7, r2, r1, #8
[^>]*> ee03 ff62 	vdwdup.u8	q7, r2, r3, #1
[^>]*> ee03 ef62 	viwdup.u8	q7, r2, r3, #1
[^>]*> ee03 ff63 	vdwdup.u8	q7, r2, r3, #2
[^>]*> ee03 ef63 	viwdup.u8	q7, r2, r3, #2
[^>]*> ee03 ffe2 	vdwdup.u8	q7, r2, r3, #4
[^>]*> ee03 efe2 	viwdup.u8	q7, r2, r3, #4
[^>]*> ee03 ffe3 	vdwdup.u8	q7, r2, r3, #8
[^>]*> ee03 efe3 	viwdup.u8	q7, r2, r3, #8
[^>]*> ee03 ff64 	vdwdup.u8	q7, r2, r5, #1
[^>]*> ee03 ef64 	viwdup.u8	q7, r2, r5, #1
[^>]*> ee03 ff65 	vdwdup.u8	q7, r2, r5, #2
[^>]*> ee03 ef65 	viwdup.u8	q7, r2, r5, #2
[^>]*> ee03 ffe4 	vdwdup.u8	q7, r2, r5, #4
[^>]*> ee03 efe4 	viwdup.u8	q7, r2, r5, #4
[^>]*> ee03 ffe5 	vdwdup.u8	q7, r2, r5, #8
[^>]*> ee03 efe5 	viwdup.u8	q7, r2, r5, #8
[^>]*> ee03 ff66 	vdwdup.u8	q7, r2, r7, #1
[^>]*> ee03 ef66 	viwdup.u8	q7, r2, r7, #1
[^>]*> ee03 ff67 	vdwdup.u8	q7, r2, r7, #2
[^>]*> ee03 ef67 	viwdup.u8	q7, r2, r7, #2
[^>]*> ee03 ffe6 	vdwdup.u8	q7, r2, r7, #4
[^>]*> ee03 efe6 	viwdup.u8	q7, r2, r7, #4
[^>]*> ee03 ffe7 	vdwdup.u8	q7, r2, r7, #8
[^>]*> ee03 efe7 	viwdup.u8	q7, r2, r7, #8
[^>]*> ee03 ff68 	vdwdup.u8	q7, r2, r9, #1
[^>]*> ee03 ef68 	viwdup.u8	q7, r2, r9, #1
[^>]*> ee03 ff69 	vdwdup.u8	q7, r2, r9, #2
[^>]*> ee03 ef69 	viwdup.u8	q7, r2, r9, #2
[^>]*> ee03 ffe8 	vdwdup.u8	q7, r2, r9, #4
[^>]*> ee03 efe8 	viwdup.u8	q7, r2, r9, #4
[^>]*> ee03 ffe9 	vdwdup.u8	q7, r2, r9, #8
[^>]*> ee03 efe9 	viwdup.u8	q7, r2, r9, #8
[^>]*> ee03 ff6a 	vdwdup.u8	q7, r2, fp, #1
[^>]*> ee03 ef6a 	viwdup.u8	q7, r2, fp, #1
[^>]*> ee03 ff6b 	vdwdup.u8	q7, r2, fp, #2
[^>]*> ee03 ef6b 	viwdup.u8	q7, r2, fp, #2
[^>]*> ee03 ffea 	vdwdup.u8	q7, r2, fp, #4
[^>]*> ee03 efea 	viwdup.u8	q7, r2, fp, #4
[^>]*> ee03 ffeb 	vdwdup.u8	q7, r2, fp, #8
[^>]*> ee03 efeb 	viwdup.u8	q7, r2, fp, #8
[^>]*> ee05 ff6e 	vddup.u8	q7, r4, #1
[^>]*> ee05 ef6e 	vidup.u8	q7, r4, #1
[^>]*> ee05 ff6f 	vddup.u8	q7, r4, #2
[^>]*> ee05 ef6f 	vidup.u8	q7, r4, #2
[^>]*> ee05 ffee 	vddup.u8	q7, r4, #4
[^>]*> ee05 efee 	vidup.u8	q7, r4, #4
[^>]*> ee05 ffef 	vddup.u8	q7, r4, #8
[^>]*> ee05 efef 	vidup.u8	q7, r4, #8
[^>]*> ee05 ff60 	vdwdup.u8	q7, r4, r1, #1
[^>]*> ee05 ef60 	viwdup.u8	q7, r4, r1, #1
[^>]*> ee05 ff61 	vdwdup.u8	q7, r4, r1, #2
[^>]*> ee05 ef61 	viwdup.u8	q7, r4, r1, #2
[^>]*> ee05 ffe0 	vdwdup.u8	q7, r4, r1, #4
[^>]*> ee05 efe0 	viwdup.u8	q7, r4, r1, #4
[^>]*> ee05 ffe1 	vdwdup.u8	q7, r4, r1, #8
[^>]*> ee05 efe1 	viwdup.u8	q7, r4, r1, #8
[^>]*> ee05 ff62 	vdwdup.u8	q7, r4, r3, #1
[^>]*> ee05 ef62 	viwdup.u8	q7, r4, r3, #1
[^>]*> ee05 ff63 	vdwdup.u8	q7, r4, r3, #2
[^>]*> ee05 ef63 	viwdup.u8	q7, r4, r3, #2
[^>]*> ee05 ffe2 	vdwdup.u8	q7, r4, r3, #4
[^>]*> ee05 efe2 	viwdup.u8	q7, r4, r3, #4
[^>]*> ee05 ffe3 	vdwdup.u8	q7, r4, r3, #8
[^>]*> ee05 efe3 	viwdup.u8	q7, r4, r3, #8
[^>]*> ee05 ff64 	vdwdup.u8	q7, r4, r5, #1
[^>]*> ee05 ef64 	viwdup.u8	q7, r4, r5, #1
[^>]*> ee05 ff65 	vdwdup.u8	q7, r4, r5, #2
[^>]*> ee05 ef65 	viwdup.u8	q7, r4, r5, #2
[^>]*> ee05 ffe4 	vdwdup.u8	q7, r4, r5, #4
[^>]*> ee05 efe4 	viwdup.u8	q7, r4, r5, #4
[^>]*> ee05 ffe5 	vdwdup.u8	q7, r4, r5, #8
[^>]*> ee05 efe5 	viwdup.u8	q7, r4, r5, #8
[^>]*> ee05 ff66 	vdwdup.u8	q7, r4, r7, #1
[^>]*> ee05 ef66 	viwdup.u8	q7, r4, r7, #1
[^>]*> ee05 ff67 	vdwdup.u8	q7, r4, r7, #2
[^>]*> ee05 ef67 	viwdup.u8	q7, r4, r7, #2
[^>]*> ee05 ffe6 	vdwdup.u8	q7, r4, r7, #4
[^>]*> ee05 efe6 	viwdup.u8	q7, r4, r7, #4
[^>]*> ee05 ffe7 	vdwdup.u8	q7, r4, r7, #8
[^>]*> ee05 efe7 	viwdup.u8	q7, r4, r7, #8
[^>]*> ee05 ff68 	vdwdup.u8	q7, r4, r9, #1
[^>]*> ee05 ef68 	viwdup.u8	q7, r4, r9, #1
[^>]*> ee05 ff69 	vdwdup.u8	q7, r4, r9, #2
[^>]*> ee05 ef69 	viwdup.u8	q7, r4, r9, #2
[^>]*> ee05 ffe8 	vdwdup.u8	q7, r4, r9, #4
[^>]*> ee05 efe8 	viwdup.u8	q7, r4, r9, #4
[^>]*> ee05 ffe9 	vdwdup.u8	q7, r4, r9, #8
[^>]*> ee05 efe9 	viwdup.u8	q7, r4, r9, #8
[^>]*> ee05 ff6a 	vdwdup.u8	q7, r4, fp, #1
[^>]*> ee05 ef6a 	viwdup.u8	q7, r4, fp, #1
[^>]*> ee05 ff6b 	vdwdup.u8	q7, r4, fp, #2
[^>]*> ee05 ef6b 	viwdup.u8	q7, r4, fp, #2
[^>]*> ee05 ffea 	vdwdup.u8	q7, r4, fp, #4
[^>]*> ee05 efea 	viwdup.u8	q7, r4, fp, #4
[^>]*> ee05 ffeb 	vdwdup.u8	q7, r4, fp, #8
[^>]*> ee05 efeb 	viwdup.u8	q7, r4, fp, #8
[^>]*> ee07 ff6e 	vddup.u8	q7, r6, #1
[^>]*> ee07 ef6e 	vidup.u8	q7, r6, #1
[^>]*> ee07 ff6f 	vddup.u8	q7, r6, #2
[^>]*> ee07 ef6f 	vidup.u8	q7, r6, #2
[^>]*> ee07 ffee 	vddup.u8	q7, r6, #4
[^>]*> ee07 efee 	vidup.u8	q7, r6, #4
[^>]*> ee07 ffef 	vddup.u8	q7, r6, #8
[^>]*> ee07 efef 	vidup.u8	q7, r6, #8
[^>]*> ee07 ff60 	vdwdup.u8	q7, r6, r1, #1
[^>]*> ee07 ef60 	viwdup.u8	q7, r6, r1, #1
[^>]*> ee07 ff61 	vdwdup.u8	q7, r6, r1, #2
[^>]*> ee07 ef61 	viwdup.u8	q7, r6, r1, #2
[^>]*> ee07 ffe0 	vdwdup.u8	q7, r6, r1, #4
[^>]*> ee07 efe0 	viwdup.u8	q7, r6, r1, #4
[^>]*> ee07 ffe1 	vdwdup.u8	q7, r6, r1, #8
[^>]*> ee07 efe1 	viwdup.u8	q7, r6, r1, #8
[^>]*> ee07 ff62 	vdwdup.u8	q7, r6, r3, #1
[^>]*> ee07 ef62 	viwdup.u8	q7, r6, r3, #1
[^>]*> ee07 ff63 	vdwdup.u8	q7, r6, r3, #2
[^>]*> ee07 ef63 	viwdup.u8	q7, r6, r3, #2
[^>]*> ee07 ffe2 	vdwdup.u8	q7, r6, r3, #4
[^>]*> ee07 efe2 	viwdup.u8	q7, r6, r3, #4
[^>]*> ee07 ffe3 	vdwdup.u8	q7, r6, r3, #8
[^>]*> ee07 efe3 	viwdup.u8	q7, r6, r3, #8
[^>]*> ee07 ff64 	vdwdup.u8	q7, r6, r5, #1
[^>]*> ee07 ef64 	viwdup.u8	q7, r6, r5, #1
[^>]*> ee07 ff65 	vdwdup.u8	q7, r6, r5, #2
[^>]*> ee07 ef65 	viwdup.u8	q7, r6, r5, #2
[^>]*> ee07 ffe4 	vdwdup.u8	q7, r6, r5, #4
[^>]*> ee07 efe4 	viwdup.u8	q7, r6, r5, #4
[^>]*> ee07 ffe5 	vdwdup.u8	q7, r6, r5, #8
[^>]*> ee07 efe5 	viwdup.u8	q7, r6, r5, #8
[^>]*> ee07 ff66 	vdwdup.u8	q7, r6, r7, #1
[^>]*> ee07 ef66 	viwdup.u8	q7, r6, r7, #1
[^>]*> ee07 ff67 	vdwdup.u8	q7, r6, r7, #2
[^>]*> ee07 ef67 	viwdup.u8	q7, r6, r7, #2
[^>]*> ee07 ffe6 	vdwdup.u8	q7, r6, r7, #4
[^>]*> ee07 efe6 	viwdup.u8	q7, r6, r7, #4
[^>]*> ee07 ffe7 	vdwdup.u8	q7, r6, r7, #8
[^>]*> ee07 efe7 	viwdup.u8	q7, r6, r7, #8
[^>]*> ee07 ff68 	vdwdup.u8	q7, r6, r9, #1
[^>]*> ee07 ef68 	viwdup.u8	q7, r6, r9, #1
[^>]*> ee07 ff69 	vdwdup.u8	q7, r6, r9, #2
[^>]*> ee07 ef69 	viwdup.u8	q7, r6, r9, #2
[^>]*> ee07 ffe8 	vdwdup.u8	q7, r6, r9, #4
[^>]*> ee07 efe8 	viwdup.u8	q7, r6, r9, #4
[^>]*> ee07 ffe9 	vdwdup.u8	q7, r6, r9, #8
[^>]*> ee07 efe9 	viwdup.u8	q7, r6, r9, #8
[^>]*> ee07 ff6a 	vdwdup.u8	q7, r6, fp, #1
[^>]*> ee07 ef6a 	viwdup.u8	q7, r6, fp, #1
[^>]*> ee07 ff6b 	vdwdup.u8	q7, r6, fp, #2
[^>]*> ee07 ef6b 	viwdup.u8	q7, r6, fp, #2
[^>]*> ee07 ffea 	vdwdup.u8	q7, r6, fp, #4
[^>]*> ee07 efea 	viwdup.u8	q7, r6, fp, #4
[^>]*> ee07 ffeb 	vdwdup.u8	q7, r6, fp, #8
[^>]*> ee07 efeb 	viwdup.u8	q7, r6, fp, #8
[^>]*> ee09 ff6e 	vddup.u8	q7, r8, #1
[^>]*> ee09 ef6e 	vidup.u8	q7, r8, #1
[^>]*> ee09 ff6f 	vddup.u8	q7, r8, #2
[^>]*> ee09 ef6f 	vidup.u8	q7, r8, #2
[^>]*> ee09 ffee 	vddup.u8	q7, r8, #4
[^>]*> ee09 efee 	vidup.u8	q7, r8, #4
[^>]*> ee09 ffef 	vddup.u8	q7, r8, #8
[^>]*> ee09 efef 	vidup.u8	q7, r8, #8
[^>]*> ee09 ff60 	vdwdup.u8	q7, r8, r1, #1
[^>]*> ee09 ef60 	viwdup.u8	q7, r8, r1, #1
[^>]*> ee09 ff61 	vdwdup.u8	q7, r8, r1, #2
[^>]*> ee09 ef61 	viwdup.u8	q7, r8, r1, #2
[^>]*> ee09 ffe0 	vdwdup.u8	q7, r8, r1, #4
[^>]*> ee09 efe0 	viwdup.u8	q7, r8, r1, #4
[^>]*> ee09 ffe1 	vdwdup.u8	q7, r8, r1, #8
[^>]*> ee09 efe1 	viwdup.u8	q7, r8, r1, #8
[^>]*> ee09 ff62 	vdwdup.u8	q7, r8, r3, #1
[^>]*> ee09 ef62 	viwdup.u8	q7, r8, r3, #1
[^>]*> ee09 ff63 	vdwdup.u8	q7, r8, r3, #2
[^>]*> ee09 ef63 	viwdup.u8	q7, r8, r3, #2
[^>]*> ee09 ffe2 	vdwdup.u8	q7, r8, r3, #4
[^>]*> ee09 efe2 	viwdup.u8	q7, r8, r3, #4
[^>]*> ee09 ffe3 	vdwdup.u8	q7, r8, r3, #8
[^>]*> ee09 efe3 	viwdup.u8	q7, r8, r3, #8
[^>]*> ee09 ff64 	vdwdup.u8	q7, r8, r5, #1
[^>]*> ee09 ef64 	viwdup.u8	q7, r8, r5, #1
[^>]*> ee09 ff65 	vdwdup.u8	q7, r8, r5, #2
[^>]*> ee09 ef65 	viwdup.u8	q7, r8, r5, #2
[^>]*> ee09 ffe4 	vdwdup.u8	q7, r8, r5, #4
[^>]*> ee09 efe4 	viwdup.u8	q7, r8, r5, #4
[^>]*> ee09 ffe5 	vdwdup.u8	q7, r8, r5, #8
[^>]*> ee09 efe5 	viwdup.u8	q7, r8, r5, #8
[^>]*> ee09 ff66 	vdwdup.u8	q7, r8, r7, #1
[^>]*> ee09 ef66 	viwdup.u8	q7, r8, r7, #1
[^>]*> ee09 ff67 	vdwdup.u8	q7, r8, r7, #2
[^>]*> ee09 ef67 	viwdup.u8	q7, r8, r7, #2
[^>]*> ee09 ffe6 	vdwdup.u8	q7, r8, r7, #4
[^>]*> ee09 efe6 	viwdup.u8	q7, r8, r7, #4
[^>]*> ee09 ffe7 	vdwdup.u8	q7, r8, r7, #8
[^>]*> ee09 efe7 	viwdup.u8	q7, r8, r7, #8
[^>]*> ee09 ff68 	vdwdup.u8	q7, r8, r9, #1
[^>]*> ee09 ef68 	viwdup.u8	q7, r8, r9, #1
[^>]*> ee09 ff69 	vdwdup.u8	q7, r8, r9, #2
[^>]*> ee09 ef69 	viwdup.u8	q7, r8, r9, #2
[^>]*> ee09 ffe8 	vdwdup.u8	q7, r8, r9, #4
[^>]*> ee09 efe8 	viwdup.u8	q7, r8, r9, #4
[^>]*> ee09 ffe9 	vdwdup.u8	q7, r8, r9, #8
[^>]*> ee09 efe9 	viwdup.u8	q7, r8, r9, #8
[^>]*> ee09 ff6a 	vdwdup.u8	q7, r8, fp, #1
[^>]*> ee09 ef6a 	viwdup.u8	q7, r8, fp, #1
[^>]*> ee09 ff6b 	vdwdup.u8	q7, r8, fp, #2
[^>]*> ee09 ef6b 	viwdup.u8	q7, r8, fp, #2
[^>]*> ee09 ffea 	vdwdup.u8	q7, r8, fp, #4
[^>]*> ee09 efea 	viwdup.u8	q7, r8, fp, #4
[^>]*> ee09 ffeb 	vdwdup.u8	q7, r8, fp, #8
[^>]*> ee09 efeb 	viwdup.u8	q7, r8, fp, #8
[^>]*> ee0b ff6e 	vddup.u8	q7, sl, #1
[^>]*> ee0b ef6e 	vidup.u8	q7, sl, #1
[^>]*> ee0b ff6f 	vddup.u8	q7, sl, #2
[^>]*> ee0b ef6f 	vidup.u8	q7, sl, #2
[^>]*> ee0b ffee 	vddup.u8	q7, sl, #4
[^>]*> ee0b efee 	vidup.u8	q7, sl, #4
[^>]*> ee0b ffef 	vddup.u8	q7, sl, #8
[^>]*> ee0b efef 	vidup.u8	q7, sl, #8
[^>]*> ee0b ff60 	vdwdup.u8	q7, sl, r1, #1
[^>]*> ee0b ef60 	viwdup.u8	q7, sl, r1, #1
[^>]*> ee0b ff61 	vdwdup.u8	q7, sl, r1, #2
[^>]*> ee0b ef61 	viwdup.u8	q7, sl, r1, #2
[^>]*> ee0b ffe0 	vdwdup.u8	q7, sl, r1, #4
[^>]*> ee0b efe0 	viwdup.u8	q7, sl, r1, #4
[^>]*> ee0b ffe1 	vdwdup.u8	q7, sl, r1, #8
[^>]*> ee0b efe1 	viwdup.u8	q7, sl, r1, #8
[^>]*> ee0b ff62 	vdwdup.u8	q7, sl, r3, #1
[^>]*> ee0b ef62 	viwdup.u8	q7, sl, r3, #1
[^>]*> ee0b ff63 	vdwdup.u8	q7, sl, r3, #2
[^>]*> ee0b ef63 	viwdup.u8	q7, sl, r3, #2
[^>]*> ee0b ffe2 	vdwdup.u8	q7, sl, r3, #4
[^>]*> ee0b efe2 	viwdup.u8	q7, sl, r3, #4
[^>]*> ee0b ffe3 	vdwdup.u8	q7, sl, r3, #8
[^>]*> ee0b efe3 	viwdup.u8	q7, sl, r3, #8
[^>]*> ee0b ff64 	vdwdup.u8	q7, sl, r5, #1
[^>]*> ee0b ef64 	viwdup.u8	q7, sl, r5, #1
[^>]*> ee0b ff65 	vdwdup.u8	q7, sl, r5, #2
[^>]*> ee0b ef65 	viwdup.u8	q7, sl, r5, #2
[^>]*> ee0b ffe4 	vdwdup.u8	q7, sl, r5, #4
[^>]*> ee0b efe4 	viwdup.u8	q7, sl, r5, #4
[^>]*> ee0b ffe5 	vdwdup.u8	q7, sl, r5, #8
[^>]*> ee0b efe5 	viwdup.u8	q7, sl, r5, #8
[^>]*> ee0b ff66 	vdwdup.u8	q7, sl, r7, #1
[^>]*> ee0b ef66 	viwdup.u8	q7, sl, r7, #1
[^>]*> ee0b ff67 	vdwdup.u8	q7, sl, r7, #2
[^>]*> ee0b ef67 	viwdup.u8	q7, sl, r7, #2
[^>]*> ee0b ffe6 	vdwdup.u8	q7, sl, r7, #4
[^>]*> ee0b efe6 	viwdup.u8	q7, sl, r7, #4
[^>]*> ee0b ffe7 	vdwdup.u8	q7, sl, r7, #8
[^>]*> ee0b efe7 	viwdup.u8	q7, sl, r7, #8
[^>]*> ee0b ff68 	vdwdup.u8	q7, sl, r9, #1
[^>]*> ee0b ef68 	viwdup.u8	q7, sl, r9, #1
[^>]*> ee0b ff69 	vdwdup.u8	q7, sl, r9, #2
[^>]*> ee0b ef69 	viwdup.u8	q7, sl, r9, #2
[^>]*> ee0b ffe8 	vdwdup.u8	q7, sl, r9, #4
[^>]*> ee0b efe8 	viwdup.u8	q7, sl, r9, #4
[^>]*> ee0b ffe9 	vdwdup.u8	q7, sl, r9, #8
[^>]*> ee0b efe9 	viwdup.u8	q7, sl, r9, #8
[^>]*> ee0b ff6a 	vdwdup.u8	q7, sl, fp, #1
[^>]*> ee0b ef6a 	viwdup.u8	q7, sl, fp, #1
[^>]*> ee0b ff6b 	vdwdup.u8	q7, sl, fp, #2
[^>]*> ee0b ef6b 	viwdup.u8	q7, sl, fp, #2
[^>]*> ee0b ffea 	vdwdup.u8	q7, sl, fp, #4
[^>]*> ee0b efea 	viwdup.u8	q7, sl, fp, #4
[^>]*> ee0b ffeb 	vdwdup.u8	q7, sl, fp, #8
[^>]*> ee0b efeb 	viwdup.u8	q7, sl, fp, #8
[^>]*> ee0d ff6e 	vddup.u8	q7, ip, #1
[^>]*> ee0d ef6e 	vidup.u8	q7, ip, #1
[^>]*> ee0d ff6f 	vddup.u8	q7, ip, #2
[^>]*> ee0d ef6f 	vidup.u8	q7, ip, #2
[^>]*> ee0d ffee 	vddup.u8	q7, ip, #4
[^>]*> ee0d efee 	vidup.u8	q7, ip, #4
[^>]*> ee0d ffef 	vddup.u8	q7, ip, #8
[^>]*> ee0d efef 	vidup.u8	q7, ip, #8
[^>]*> ee0d ff60 	vdwdup.u8	q7, ip, r1, #1
[^>]*> ee0d ef60 	viwdup.u8	q7, ip, r1, #1
[^>]*> ee0d ff61 	vdwdup.u8	q7, ip, r1, #2
[^>]*> ee0d ef61 	viwdup.u8	q7, ip, r1, #2
[^>]*> ee0d ffe0 	vdwdup.u8	q7, ip, r1, #4
[^>]*> ee0d efe0 	viwdup.u8	q7, ip, r1, #4
[^>]*> ee0d ffe1 	vdwdup.u8	q7, ip, r1, #8
[^>]*> ee0d efe1 	viwdup.u8	q7, ip, r1, #8
[^>]*> ee0d ff62 	vdwdup.u8	q7, ip, r3, #1
[^>]*> ee0d ef62 	viwdup.u8	q7, ip, r3, #1
[^>]*> ee0d ff63 	vdwdup.u8	q7, ip, r3, #2
[^>]*> ee0d ef63 	viwdup.u8	q7, ip, r3, #2
[^>]*> ee0d ffe2 	vdwdup.u8	q7, ip, r3, #4
[^>]*> ee0d efe2 	viwdup.u8	q7, ip, r3, #4
[^>]*> ee0d ffe3 	vdwdup.u8	q7, ip, r3, #8
[^>]*> ee0d efe3 	viwdup.u8	q7, ip, r3, #8
[^>]*> ee0d ff64 	vdwdup.u8	q7, ip, r5, #1
[^>]*> ee0d ef64 	viwdup.u8	q7, ip, r5, #1
[^>]*> ee0d ff65 	vdwdup.u8	q7, ip, r5, #2
[^>]*> ee0d ef65 	viwdup.u8	q7, ip, r5, #2
[^>]*> ee0d ffe4 	vdwdup.u8	q7, ip, r5, #4
[^>]*> ee0d efe4 	viwdup.u8	q7, ip, r5, #4
[^>]*> ee0d ffe5 	vdwdup.u8	q7, ip, r5, #8
[^>]*> ee0d efe5 	viwdup.u8	q7, ip, r5, #8
[^>]*> ee0d ff66 	vdwdup.u8	q7, ip, r7, #1
[^>]*> ee0d ef66 	viwdup.u8	q7, ip, r7, #1
[^>]*> ee0d ff67 	vdwdup.u8	q7, ip, r7, #2
[^>]*> ee0d ef67 	viwdup.u8	q7, ip, r7, #2
[^>]*> ee0d ffe6 	vdwdup.u8	q7, ip, r7, #4
[^>]*> ee0d efe6 	viwdup.u8	q7, ip, r7, #4
[^>]*> ee0d ffe7 	vdwdup.u8	q7, ip, r7, #8
[^>]*> ee0d efe7 	viwdup.u8	q7, ip, r7, #8
[^>]*> ee0d ff68 	vdwdup.u8	q7, ip, r9, #1
[^>]*> ee0d ef68 	viwdup.u8	q7, ip, r9, #1
[^>]*> ee0d ff69 	vdwdup.u8	q7, ip, r9, #2
[^>]*> ee0d ef69 	viwdup.u8	q7, ip, r9, #2
[^>]*> ee0d ffe8 	vdwdup.u8	q7, ip, r9, #4
[^>]*> ee0d efe8 	viwdup.u8	q7, ip, r9, #4
[^>]*> ee0d ffe9 	vdwdup.u8	q7, ip, r9, #8
[^>]*> ee0d efe9 	viwdup.u8	q7, ip, r9, #8
[^>]*> ee0d ff6a 	vdwdup.u8	q7, ip, fp, #1
[^>]*> ee0d ef6a 	viwdup.u8	q7, ip, fp, #1
[^>]*> ee0d ff6b 	vdwdup.u8	q7, ip, fp, #2
[^>]*> ee0d ef6b 	viwdup.u8	q7, ip, fp, #2
[^>]*> ee0d ffea 	vdwdup.u8	q7, ip, fp, #4
[^>]*> ee0d efea 	viwdup.u8	q7, ip, fp, #4
[^>]*> ee0d ffeb 	vdwdup.u8	q7, ip, fp, #8
[^>]*> ee0d efeb 	viwdup.u8	q7, ip, fp, #8
[^>]*> ee11 1f6e 	vddup.u16	q0, r0, #1
[^>]*> ee11 0f6e 	vidup.u16	q0, r0, #1
[^>]*> ee11 1f6f 	vddup.u16	q0, r0, #2
[^>]*> ee11 0f6f 	vidup.u16	q0, r0, #2
[^>]*> ee11 1fee 	vddup.u16	q0, r0, #4
[^>]*> ee11 0fee 	vidup.u16	q0, r0, #4
[^>]*> ee11 1fef 	vddup.u16	q0, r0, #8
[^>]*> ee11 0fef 	vidup.u16	q0, r0, #8
[^>]*> ee11 1f60 	vdwdup.u16	q0, r0, r1, #1
[^>]*> ee11 0f60 	viwdup.u16	q0, r0, r1, #1
[^>]*> ee11 1f61 	vdwdup.u16	q0, r0, r1, #2
[^>]*> ee11 0f61 	viwdup.u16	q0, r0, r1, #2
[^>]*> ee11 1fe0 	vdwdup.u16	q0, r0, r1, #4
[^>]*> ee11 0fe0 	viwdup.u16	q0, r0, r1, #4
[^>]*> ee11 1fe1 	vdwdup.u16	q0, r0, r1, #8
[^>]*> ee11 0fe1 	viwdup.u16	q0, r0, r1, #8
[^>]*> ee11 1f62 	vdwdup.u16	q0, r0, r3, #1
[^>]*> ee11 0f62 	viwdup.u16	q0, r0, r3, #1
[^>]*> ee11 1f63 	vdwdup.u16	q0, r0, r3, #2
[^>]*> ee11 0f63 	viwdup.u16	q0, r0, r3, #2
[^>]*> ee11 1fe2 	vdwdup.u16	q0, r0, r3, #4
[^>]*> ee11 0fe2 	viwdup.u16	q0, r0, r3, #4
[^>]*> ee11 1fe3 	vdwdup.u16	q0, r0, r3, #8
[^>]*> ee11 0fe3 	viwdup.u16	q0, r0, r3, #8
[^>]*> ee11 1f64 	vdwdup.u16	q0, r0, r5, #1
[^>]*> ee11 0f64 	viwdup.u16	q0, r0, r5, #1
[^>]*> ee11 1f65 	vdwdup.u16	q0, r0, r5, #2
[^>]*> ee11 0f65 	viwdup.u16	q0, r0, r5, #2
[^>]*> ee11 1fe4 	vdwdup.u16	q0, r0, r5, #4
[^>]*> ee11 0fe4 	viwdup.u16	q0, r0, r5, #4
[^>]*> ee11 1fe5 	vdwdup.u16	q0, r0, r5, #8
[^>]*> ee11 0fe5 	viwdup.u16	q0, r0, r5, #8
[^>]*> ee11 1f66 	vdwdup.u16	q0, r0, r7, #1
[^>]*> ee11 0f66 	viwdup.u16	q0, r0, r7, #1
[^>]*> ee11 1f67 	vdwdup.u16	q0, r0, r7, #2
[^>]*> ee11 0f67 	viwdup.u16	q0, r0, r7, #2
[^>]*> ee11 1fe6 	vdwdup.u16	q0, r0, r7, #4
[^>]*> ee11 0fe6 	viwdup.u16	q0, r0, r7, #4
[^>]*> ee11 1fe7 	vdwdup.u16	q0, r0, r7, #8
[^>]*> ee11 0fe7 	viwdup.u16	q0, r0, r7, #8
[^>]*> ee11 1f68 	vdwdup.u16	q0, r0, r9, #1
[^>]*> ee11 0f68 	viwdup.u16	q0, r0, r9, #1
[^>]*> ee11 1f69 	vdwdup.u16	q0, r0, r9, #2
[^>]*> ee11 0f69 	viwdup.u16	q0, r0, r9, #2
[^>]*> ee11 1fe8 	vdwdup.u16	q0, r0, r9, #4
[^>]*> ee11 0fe8 	viwdup.u16	q0, r0, r9, #4
[^>]*> ee11 1fe9 	vdwdup.u16	q0, r0, r9, #8
[^>]*> ee11 0fe9 	viwdup.u16	q0, r0, r9, #8
[^>]*> ee11 1f6a 	vdwdup.u16	q0, r0, fp, #1
[^>]*> ee11 0f6a 	viwdup.u16	q0, r0, fp, #1
[^>]*> ee11 1f6b 	vdwdup.u16	q0, r0, fp, #2
[^>]*> ee11 0f6b 	viwdup.u16	q0, r0, fp, #2
[^>]*> ee11 1fea 	vdwdup.u16	q0, r0, fp, #4
[^>]*> ee11 0fea 	viwdup.u16	q0, r0, fp, #4
[^>]*> ee11 1feb 	vdwdup.u16	q0, r0, fp, #8
[^>]*> ee11 0feb 	viwdup.u16	q0, r0, fp, #8
[^>]*> ee13 1f6e 	vddup.u16	q0, r2, #1
[^>]*> ee13 0f6e 	vidup.u16	q0, r2, #1
[^>]*> ee13 1f6f 	vddup.u16	q0, r2, #2
[^>]*> ee13 0f6f 	vidup.u16	q0, r2, #2
[^>]*> ee13 1fee 	vddup.u16	q0, r2, #4
[^>]*> ee13 0fee 	vidup.u16	q0, r2, #4
[^>]*> ee13 1fef 	vddup.u16	q0, r2, #8
[^>]*> ee13 0fef 	vidup.u16	q0, r2, #8
[^>]*> ee13 1f60 	vdwdup.u16	q0, r2, r1, #1
[^>]*> ee13 0f60 	viwdup.u16	q0, r2, r1, #1
[^>]*> ee13 1f61 	vdwdup.u16	q0, r2, r1, #2
[^>]*> ee13 0f61 	viwdup.u16	q0, r2, r1, #2
[^>]*> ee13 1fe0 	vdwdup.u16	q0, r2, r1, #4
[^>]*> ee13 0fe0 	viwdup.u16	q0, r2, r1, #4
[^>]*> ee13 1fe1 	vdwdup.u16	q0, r2, r1, #8
[^>]*> ee13 0fe1 	viwdup.u16	q0, r2, r1, #8
[^>]*> ee13 1f62 	vdwdup.u16	q0, r2, r3, #1
[^>]*> ee13 0f62 	viwdup.u16	q0, r2, r3, #1
[^>]*> ee13 1f63 	vdwdup.u16	q0, r2, r3, #2
[^>]*> ee13 0f63 	viwdup.u16	q0, r2, r3, #2
[^>]*> ee13 1fe2 	vdwdup.u16	q0, r2, r3, #4
[^>]*> ee13 0fe2 	viwdup.u16	q0, r2, r3, #4
[^>]*> ee13 1fe3 	vdwdup.u16	q0, r2, r3, #8
[^>]*> ee13 0fe3 	viwdup.u16	q0, r2, r3, #8
[^>]*> ee13 1f64 	vdwdup.u16	q0, r2, r5, #1
[^>]*> ee13 0f64 	viwdup.u16	q0, r2, r5, #1
[^>]*> ee13 1f65 	vdwdup.u16	q0, r2, r5, #2
[^>]*> ee13 0f65 	viwdup.u16	q0, r2, r5, #2
[^>]*> ee13 1fe4 	vdwdup.u16	q0, r2, r5, #4
[^>]*> ee13 0fe4 	viwdup.u16	q0, r2, r5, #4
[^>]*> ee13 1fe5 	vdwdup.u16	q0, r2, r5, #8
[^>]*> ee13 0fe5 	viwdup.u16	q0, r2, r5, #8
[^>]*> ee13 1f66 	vdwdup.u16	q0, r2, r7, #1
[^>]*> ee13 0f66 	viwdup.u16	q0, r2, r7, #1
[^>]*> ee13 1f67 	vdwdup.u16	q0, r2, r7, #2
[^>]*> ee13 0f67 	viwdup.u16	q0, r2, r7, #2
[^>]*> ee13 1fe6 	vdwdup.u16	q0, r2, r7, #4
[^>]*> ee13 0fe6 	viwdup.u16	q0, r2, r7, #4
[^>]*> ee13 1fe7 	vdwdup.u16	q0, r2, r7, #8
[^>]*> ee13 0fe7 	viwdup.u16	q0, r2, r7, #8
[^>]*> ee13 1f68 	vdwdup.u16	q0, r2, r9, #1
[^>]*> ee13 0f68 	viwdup.u16	q0, r2, r9, #1
[^>]*> ee13 1f69 	vdwdup.u16	q0, r2, r9, #2
[^>]*> ee13 0f69 	viwdup.u16	q0, r2, r9, #2
[^>]*> ee13 1fe8 	vdwdup.u16	q0, r2, r9, #4
[^>]*> ee13 0fe8 	viwdup.u16	q0, r2, r9, #4
[^>]*> ee13 1fe9 	vdwdup.u16	q0, r2, r9, #8
[^>]*> ee13 0fe9 	viwdup.u16	q0, r2, r9, #8
[^>]*> ee13 1f6a 	vdwdup.u16	q0, r2, fp, #1
[^>]*> ee13 0f6a 	viwdup.u16	q0, r2, fp, #1
[^>]*> ee13 1f6b 	vdwdup.u16	q0, r2, fp, #2
[^>]*> ee13 0f6b 	viwdup.u16	q0, r2, fp, #2
[^>]*> ee13 1fea 	vdwdup.u16	q0, r2, fp, #4
[^>]*> ee13 0fea 	viwdup.u16	q0, r2, fp, #4
[^>]*> ee13 1feb 	vdwdup.u16	q0, r2, fp, #8
[^>]*> ee13 0feb 	viwdup.u16	q0, r2, fp, #8
[^>]*> ee15 1f6e 	vddup.u16	q0, r4, #1
[^>]*> ee15 0f6e 	vidup.u16	q0, r4, #1
[^>]*> ee15 1f6f 	vddup.u16	q0, r4, #2
[^>]*> ee15 0f6f 	vidup.u16	q0, r4, #2
[^>]*> ee15 1fee 	vddup.u16	q0, r4, #4
[^>]*> ee15 0fee 	vidup.u16	q0, r4, #4
[^>]*> ee15 1fef 	vddup.u16	q0, r4, #8
[^>]*> ee15 0fef 	vidup.u16	q0, r4, #8
[^>]*> ee15 1f60 	vdwdup.u16	q0, r4, r1, #1
[^>]*> ee15 0f60 	viwdup.u16	q0, r4, r1, #1
[^>]*> ee15 1f61 	vdwdup.u16	q0, r4, r1, #2
[^>]*> ee15 0f61 	viwdup.u16	q0, r4, r1, #2
[^>]*> ee15 1fe0 	vdwdup.u16	q0, r4, r1, #4
[^>]*> ee15 0fe0 	viwdup.u16	q0, r4, r1, #4
[^>]*> ee15 1fe1 	vdwdup.u16	q0, r4, r1, #8
[^>]*> ee15 0fe1 	viwdup.u16	q0, r4, r1, #8
[^>]*> ee15 1f62 	vdwdup.u16	q0, r4, r3, #1
[^>]*> ee15 0f62 	viwdup.u16	q0, r4, r3, #1
[^>]*> ee15 1f63 	vdwdup.u16	q0, r4, r3, #2
[^>]*> ee15 0f63 	viwdup.u16	q0, r4, r3, #2
[^>]*> ee15 1fe2 	vdwdup.u16	q0, r4, r3, #4
[^>]*> ee15 0fe2 	viwdup.u16	q0, r4, r3, #4
[^>]*> ee15 1fe3 	vdwdup.u16	q0, r4, r3, #8
[^>]*> ee15 0fe3 	viwdup.u16	q0, r4, r3, #8
[^>]*> ee15 1f64 	vdwdup.u16	q0, r4, r5, #1
[^>]*> ee15 0f64 	viwdup.u16	q0, r4, r5, #1
[^>]*> ee15 1f65 	vdwdup.u16	q0, r4, r5, #2
[^>]*> ee15 0f65 	viwdup.u16	q0, r4, r5, #2
[^>]*> ee15 1fe4 	vdwdup.u16	q0, r4, r5, #4
[^>]*> ee15 0fe4 	viwdup.u16	q0, r4, r5, #4
[^>]*> ee15 1fe5 	vdwdup.u16	q0, r4, r5, #8
[^>]*> ee15 0fe5 	viwdup.u16	q0, r4, r5, #8
[^>]*> ee15 1f66 	vdwdup.u16	q0, r4, r7, #1
[^>]*> ee15 0f66 	viwdup.u16	q0, r4, r7, #1
[^>]*> ee15 1f67 	vdwdup.u16	q0, r4, r7, #2
[^>]*> ee15 0f67 	viwdup.u16	q0, r4, r7, #2
[^>]*> ee15 1fe6 	vdwdup.u16	q0, r4, r7, #4
[^>]*> ee15 0fe6 	viwdup.u16	q0, r4, r7, #4
[^>]*> ee15 1fe7 	vdwdup.u16	q0, r4, r7, #8
[^>]*> ee15 0fe7 	viwdup.u16	q0, r4, r7, #8
[^>]*> ee15 1f68 	vdwdup.u16	q0, r4, r9, #1
[^>]*> ee15 0f68 	viwdup.u16	q0, r4, r9, #1
[^>]*> ee15 1f69 	vdwdup.u16	q0, r4, r9, #2
[^>]*> ee15 0f69 	viwdup.u16	q0, r4, r9, #2
[^>]*> ee15 1fe8 	vdwdup.u16	q0, r4, r9, #4
[^>]*> ee15 0fe8 	viwdup.u16	q0, r4, r9, #4
[^>]*> ee15 1fe9 	vdwdup.u16	q0, r4, r9, #8
[^>]*> ee15 0fe9 	viwdup.u16	q0, r4, r9, #8
[^>]*> ee15 1f6a 	vdwdup.u16	q0, r4, fp, #1
[^>]*> ee15 0f6a 	viwdup.u16	q0, r4, fp, #1
[^>]*> ee15 1f6b 	vdwdup.u16	q0, r4, fp, #2
[^>]*> ee15 0f6b 	viwdup.u16	q0, r4, fp, #2
[^>]*> ee15 1fea 	vdwdup.u16	q0, r4, fp, #4
[^>]*> ee15 0fea 	viwdup.u16	q0, r4, fp, #4
[^>]*> ee15 1feb 	vdwdup.u16	q0, r4, fp, #8
[^>]*> ee15 0feb 	viwdup.u16	q0, r4, fp, #8
[^>]*> ee17 1f6e 	vddup.u16	q0, r6, #1
[^>]*> ee17 0f6e 	vidup.u16	q0, r6, #1
[^>]*> ee17 1f6f 	vddup.u16	q0, r6, #2
[^>]*> ee17 0f6f 	vidup.u16	q0, r6, #2
[^>]*> ee17 1fee 	vddup.u16	q0, r6, #4
[^>]*> ee17 0fee 	vidup.u16	q0, r6, #4
[^>]*> ee17 1fef 	vddup.u16	q0, r6, #8
[^>]*> ee17 0fef 	vidup.u16	q0, r6, #8
[^>]*> ee17 1f60 	vdwdup.u16	q0, r6, r1, #1
[^>]*> ee17 0f60 	viwdup.u16	q0, r6, r1, #1
[^>]*> ee17 1f61 	vdwdup.u16	q0, r6, r1, #2
[^>]*> ee17 0f61 	viwdup.u16	q0, r6, r1, #2
[^>]*> ee17 1fe0 	vdwdup.u16	q0, r6, r1, #4
[^>]*> ee17 0fe0 	viwdup.u16	q0, r6, r1, #4
[^>]*> ee17 1fe1 	vdwdup.u16	q0, r6, r1, #8
[^>]*> ee17 0fe1 	viwdup.u16	q0, r6, r1, #8
[^>]*> ee17 1f62 	vdwdup.u16	q0, r6, r3, #1
[^>]*> ee17 0f62 	viwdup.u16	q0, r6, r3, #1
[^>]*> ee17 1f63 	vdwdup.u16	q0, r6, r3, #2
[^>]*> ee17 0f63 	viwdup.u16	q0, r6, r3, #2
[^>]*> ee17 1fe2 	vdwdup.u16	q0, r6, r3, #4
[^>]*> ee17 0fe2 	viwdup.u16	q0, r6, r3, #4
[^>]*> ee17 1fe3 	vdwdup.u16	q0, r6, r3, #8
[^>]*> ee17 0fe3 	viwdup.u16	q0, r6, r3, #8
[^>]*> ee17 1f64 	vdwdup.u16	q0, r6, r5, #1
[^>]*> ee17 0f64 	viwdup.u16	q0, r6, r5, #1
[^>]*> ee17 1f65 	vdwdup.u16	q0, r6, r5, #2
[^>]*> ee17 0f65 	viwdup.u16	q0, r6, r5, #2
[^>]*> ee17 1fe4 	vdwdup.u16	q0, r6, r5, #4
[^>]*> ee17 0fe4 	viwdup.u16	q0, r6, r5, #4
[^>]*> ee17 1fe5 	vdwdup.u16	q0, r6, r5, #8
[^>]*> ee17 0fe5 	viwdup.u16	q0, r6, r5, #8
[^>]*> ee17 1f66 	vdwdup.u16	q0, r6, r7, #1
[^>]*> ee17 0f66 	viwdup.u16	q0, r6, r7, #1
[^>]*> ee17 1f67 	vdwdup.u16	q0, r6, r7, #2
[^>]*> ee17 0f67 	viwdup.u16	q0, r6, r7, #2
[^>]*> ee17 1fe6 	vdwdup.u16	q0, r6, r7, #4
[^>]*> ee17 0fe6 	viwdup.u16	q0, r6, r7, #4
[^>]*> ee17 1fe7 	vdwdup.u16	q0, r6, r7, #8
[^>]*> ee17 0fe7 	viwdup.u16	q0, r6, r7, #8
[^>]*> ee17 1f68 	vdwdup.u16	q0, r6, r9, #1
[^>]*> ee17 0f68 	viwdup.u16	q0, r6, r9, #1
[^>]*> ee17 1f69 	vdwdup.u16	q0, r6, r9, #2
[^>]*> ee17 0f69 	viwdup.u16	q0, r6, r9, #2
[^>]*> ee17 1fe8 	vdwdup.u16	q0, r6, r9, #4
[^>]*> ee17 0fe8 	viwdup.u16	q0, r6, r9, #4
[^>]*> ee17 1fe9 	vdwdup.u16	q0, r6, r9, #8
[^>]*> ee17 0fe9 	viwdup.u16	q0, r6, r9, #8
[^>]*> ee17 1f6a 	vdwdup.u16	q0, r6, fp, #1
[^>]*> ee17 0f6a 	viwdup.u16	q0, r6, fp, #1
[^>]*> ee17 1f6b 	vdwdup.u16	q0, r6, fp, #2
[^>]*> ee17 0f6b 	viwdup.u16	q0, r6, fp, #2
[^>]*> ee17 1fea 	vdwdup.u16	q0, r6, fp, #4
[^>]*> ee17 0fea 	viwdup.u16	q0, r6, fp, #4
[^>]*> ee17 1feb 	vdwdup.u16	q0, r6, fp, #8
[^>]*> ee17 0feb 	viwdup.u16	q0, r6, fp, #8
[^>]*> ee19 1f6e 	vddup.u16	q0, r8, #1
[^>]*> ee19 0f6e 	vidup.u16	q0, r8, #1
[^>]*> ee19 1f6f 	vddup.u16	q0, r8, #2
[^>]*> ee19 0f6f 	vidup.u16	q0, r8, #2
[^>]*> ee19 1fee 	vddup.u16	q0, r8, #4
[^>]*> ee19 0fee 	vidup.u16	q0, r8, #4
[^>]*> ee19 1fef 	vddup.u16	q0, r8, #8
[^>]*> ee19 0fef 	vidup.u16	q0, r8, #8
[^>]*> ee19 1f60 	vdwdup.u16	q0, r8, r1, #1
[^>]*> ee19 0f60 	viwdup.u16	q0, r8, r1, #1
[^>]*> ee19 1f61 	vdwdup.u16	q0, r8, r1, #2
[^>]*> ee19 0f61 	viwdup.u16	q0, r8, r1, #2
[^>]*> ee19 1fe0 	vdwdup.u16	q0, r8, r1, #4
[^>]*> ee19 0fe0 	viwdup.u16	q0, r8, r1, #4
[^>]*> ee19 1fe1 	vdwdup.u16	q0, r8, r1, #8
[^>]*> ee19 0fe1 	viwdup.u16	q0, r8, r1, #8
[^>]*> ee19 1f62 	vdwdup.u16	q0, r8, r3, #1
[^>]*> ee19 0f62 	viwdup.u16	q0, r8, r3, #1
[^>]*> ee19 1f63 	vdwdup.u16	q0, r8, r3, #2
[^>]*> ee19 0f63 	viwdup.u16	q0, r8, r3, #2
[^>]*> ee19 1fe2 	vdwdup.u16	q0, r8, r3, #4
[^>]*> ee19 0fe2 	viwdup.u16	q0, r8, r3, #4
[^>]*> ee19 1fe3 	vdwdup.u16	q0, r8, r3, #8
[^>]*> ee19 0fe3 	viwdup.u16	q0, r8, r3, #8
[^>]*> ee19 1f64 	vdwdup.u16	q0, r8, r5, #1
[^>]*> ee19 0f64 	viwdup.u16	q0, r8, r5, #1
[^>]*> ee19 1f65 	vdwdup.u16	q0, r8, r5, #2
[^>]*> ee19 0f65 	viwdup.u16	q0, r8, r5, #2
[^>]*> ee19 1fe4 	vdwdup.u16	q0, r8, r5, #4
[^>]*> ee19 0fe4 	viwdup.u16	q0, r8, r5, #4
[^>]*> ee19 1fe5 	vdwdup.u16	q0, r8, r5, #8
[^>]*> ee19 0fe5 	viwdup.u16	q0, r8, r5, #8
[^>]*> ee19 1f66 	vdwdup.u16	q0, r8, r7, #1
[^>]*> ee19 0f66 	viwdup.u16	q0, r8, r7, #1
[^>]*> ee19 1f67 	vdwdup.u16	q0, r8, r7, #2
[^>]*> ee19 0f67 	viwdup.u16	q0, r8, r7, #2
[^>]*> ee19 1fe6 	vdwdup.u16	q0, r8, r7, #4
[^>]*> ee19 0fe6 	viwdup.u16	q0, r8, r7, #4
[^>]*> ee19 1fe7 	vdwdup.u16	q0, r8, r7, #8
[^>]*> ee19 0fe7 	viwdup.u16	q0, r8, r7, #8
[^>]*> ee19 1f68 	vdwdup.u16	q0, r8, r9, #1
[^>]*> ee19 0f68 	viwdup.u16	q0, r8, r9, #1
[^>]*> ee19 1f69 	vdwdup.u16	q0, r8, r9, #2
[^>]*> ee19 0f69 	viwdup.u16	q0, r8, r9, #2
[^>]*> ee19 1fe8 	vdwdup.u16	q0, r8, r9, #4
[^>]*> ee19 0fe8 	viwdup.u16	q0, r8, r9, #4
[^>]*> ee19 1fe9 	vdwdup.u16	q0, r8, r9, #8
[^>]*> ee19 0fe9 	viwdup.u16	q0, r8, r9, #8
[^>]*> ee19 1f6a 	vdwdup.u16	q0, r8, fp, #1
[^>]*> ee19 0f6a 	viwdup.u16	q0, r8, fp, #1
[^>]*> ee19 1f6b 	vdwdup.u16	q0, r8, fp, #2
[^>]*> ee19 0f6b 	viwdup.u16	q0, r8, fp, #2
[^>]*> ee19 1fea 	vdwdup.u16	q0, r8, fp, #4
[^>]*> ee19 0fea 	viwdup.u16	q0, r8, fp, #4
[^>]*> ee19 1feb 	vdwdup.u16	q0, r8, fp, #8
[^>]*> ee19 0feb 	viwdup.u16	q0, r8, fp, #8
[^>]*> ee1b 1f6e 	vddup.u16	q0, sl, #1
[^>]*> ee1b 0f6e 	vidup.u16	q0, sl, #1
[^>]*> ee1b 1f6f 	vddup.u16	q0, sl, #2
[^>]*> ee1b 0f6f 	vidup.u16	q0, sl, #2
[^>]*> ee1b 1fee 	vddup.u16	q0, sl, #4
[^>]*> ee1b 0fee 	vidup.u16	q0, sl, #4
[^>]*> ee1b 1fef 	vddup.u16	q0, sl, #8
[^>]*> ee1b 0fef 	vidup.u16	q0, sl, #8
[^>]*> ee1b 1f60 	vdwdup.u16	q0, sl, r1, #1
[^>]*> ee1b 0f60 	viwdup.u16	q0, sl, r1, #1
[^>]*> ee1b 1f61 	vdwdup.u16	q0, sl, r1, #2
[^>]*> ee1b 0f61 	viwdup.u16	q0, sl, r1, #2
[^>]*> ee1b 1fe0 	vdwdup.u16	q0, sl, r1, #4
[^>]*> ee1b 0fe0 	viwdup.u16	q0, sl, r1, #4
[^>]*> ee1b 1fe1 	vdwdup.u16	q0, sl, r1, #8
[^>]*> ee1b 0fe1 	viwdup.u16	q0, sl, r1, #8
[^>]*> ee1b 1f62 	vdwdup.u16	q0, sl, r3, #1
[^>]*> ee1b 0f62 	viwdup.u16	q0, sl, r3, #1
[^>]*> ee1b 1f63 	vdwdup.u16	q0, sl, r3, #2
[^>]*> ee1b 0f63 	viwdup.u16	q0, sl, r3, #2
[^>]*> ee1b 1fe2 	vdwdup.u16	q0, sl, r3, #4
[^>]*> ee1b 0fe2 	viwdup.u16	q0, sl, r3, #4
[^>]*> ee1b 1fe3 	vdwdup.u16	q0, sl, r3, #8
[^>]*> ee1b 0fe3 	viwdup.u16	q0, sl, r3, #8
[^>]*> ee1b 1f64 	vdwdup.u16	q0, sl, r5, #1
[^>]*> ee1b 0f64 	viwdup.u16	q0, sl, r5, #1
[^>]*> ee1b 1f65 	vdwdup.u16	q0, sl, r5, #2
[^>]*> ee1b 0f65 	viwdup.u16	q0, sl, r5, #2
[^>]*> ee1b 1fe4 	vdwdup.u16	q0, sl, r5, #4
[^>]*> ee1b 0fe4 	viwdup.u16	q0, sl, r5, #4
[^>]*> ee1b 1fe5 	vdwdup.u16	q0, sl, r5, #8
[^>]*> ee1b 0fe5 	viwdup.u16	q0, sl, r5, #8
[^>]*> ee1b 1f66 	vdwdup.u16	q0, sl, r7, #1
[^>]*> ee1b 0f66 	viwdup.u16	q0, sl, r7, #1
[^>]*> ee1b 1f67 	vdwdup.u16	q0, sl, r7, #2
[^>]*> ee1b 0f67 	viwdup.u16	q0, sl, r7, #2
[^>]*> ee1b 1fe6 	vdwdup.u16	q0, sl, r7, #4
[^>]*> ee1b 0fe6 	viwdup.u16	q0, sl, r7, #4
[^>]*> ee1b 1fe7 	vdwdup.u16	q0, sl, r7, #8
[^>]*> ee1b 0fe7 	viwdup.u16	q0, sl, r7, #8
[^>]*> ee1b 1f68 	vdwdup.u16	q0, sl, r9, #1
[^>]*> ee1b 0f68 	viwdup.u16	q0, sl, r9, #1
[^>]*> ee1b 1f69 	vdwdup.u16	q0, sl, r9, #2
[^>]*> ee1b 0f69 	viwdup.u16	q0, sl, r9, #2
[^>]*> ee1b 1fe8 	vdwdup.u16	q0, sl, r9, #4
[^>]*> ee1b 0fe8 	viwdup.u16	q0, sl, r9, #4
[^>]*> ee1b 1fe9 	vdwdup.u16	q0, sl, r9, #8
[^>]*> ee1b 0fe9 	viwdup.u16	q0, sl, r9, #8
[^>]*> ee1b 1f6a 	vdwdup.u16	q0, sl, fp, #1
[^>]*> ee1b 0f6a 	viwdup.u16	q0, sl, fp, #1
[^>]*> ee1b 1f6b 	vdwdup.u16	q0, sl, fp, #2
[^>]*> ee1b 0f6b 	viwdup.u16	q0, sl, fp, #2
[^>]*> ee1b 1fea 	vdwdup.u16	q0, sl, fp, #4
[^>]*> ee1b 0fea 	viwdup.u16	q0, sl, fp, #4
[^>]*> ee1b 1feb 	vdwdup.u16	q0, sl, fp, #8
[^>]*> ee1b 0feb 	viwdup.u16	q0, sl, fp, #8
[^>]*> ee1d 1f6e 	vddup.u16	q0, ip, #1
[^>]*> ee1d 0f6e 	vidup.u16	q0, ip, #1
[^>]*> ee1d 1f6f 	vddup.u16	q0, ip, #2
[^>]*> ee1d 0f6f 	vidup.u16	q0, ip, #2
[^>]*> ee1d 1fee 	vddup.u16	q0, ip, #4
[^>]*> ee1d 0fee 	vidup.u16	q0, ip, #4
[^>]*> ee1d 1fef 	vddup.u16	q0, ip, #8
[^>]*> ee1d 0fef 	vidup.u16	q0, ip, #8
[^>]*> ee1d 1f60 	vdwdup.u16	q0, ip, r1, #1
[^>]*> ee1d 0f60 	viwdup.u16	q0, ip, r1, #1
[^>]*> ee1d 1f61 	vdwdup.u16	q0, ip, r1, #2
[^>]*> ee1d 0f61 	viwdup.u16	q0, ip, r1, #2
[^>]*> ee1d 1fe0 	vdwdup.u16	q0, ip, r1, #4
[^>]*> ee1d 0fe0 	viwdup.u16	q0, ip, r1, #4
[^>]*> ee1d 1fe1 	vdwdup.u16	q0, ip, r1, #8
[^>]*> ee1d 0fe1 	viwdup.u16	q0, ip, r1, #8
[^>]*> ee1d 1f62 	vdwdup.u16	q0, ip, r3, #1
[^>]*> ee1d 0f62 	viwdup.u16	q0, ip, r3, #1
[^>]*> ee1d 1f63 	vdwdup.u16	q0, ip, r3, #2
[^>]*> ee1d 0f63 	viwdup.u16	q0, ip, r3, #2
[^>]*> ee1d 1fe2 	vdwdup.u16	q0, ip, r3, #4
[^>]*> ee1d 0fe2 	viwdup.u16	q0, ip, r3, #4
[^>]*> ee1d 1fe3 	vdwdup.u16	q0, ip, r3, #8
[^>]*> ee1d 0fe3 	viwdup.u16	q0, ip, r3, #8
[^>]*> ee1d 1f64 	vdwdup.u16	q0, ip, r5, #1
[^>]*> ee1d 0f64 	viwdup.u16	q0, ip, r5, #1
[^>]*> ee1d 1f65 	vdwdup.u16	q0, ip, r5, #2
[^>]*> ee1d 0f65 	viwdup.u16	q0, ip, r5, #2
[^>]*> ee1d 1fe4 	vdwdup.u16	q0, ip, r5, #4
[^>]*> ee1d 0fe4 	viwdup.u16	q0, ip, r5, #4
[^>]*> ee1d 1fe5 	vdwdup.u16	q0, ip, r5, #8
[^>]*> ee1d 0fe5 	viwdup.u16	q0, ip, r5, #8
[^>]*> ee1d 1f66 	vdwdup.u16	q0, ip, r7, #1
[^>]*> ee1d 0f66 	viwdup.u16	q0, ip, r7, #1
[^>]*> ee1d 1f67 	vdwdup.u16	q0, ip, r7, #2
[^>]*> ee1d 0f67 	viwdup.u16	q0, ip, r7, #2
[^>]*> ee1d 1fe6 	vdwdup.u16	q0, ip, r7, #4
[^>]*> ee1d 0fe6 	viwdup.u16	q0, ip, r7, #4
[^>]*> ee1d 1fe7 	vdwdup.u16	q0, ip, r7, #8
[^>]*> ee1d 0fe7 	viwdup.u16	q0, ip, r7, #8
[^>]*> ee1d 1f68 	vdwdup.u16	q0, ip, r9, #1
[^>]*> ee1d 0f68 	viwdup.u16	q0, ip, r9, #1
[^>]*> ee1d 1f69 	vdwdup.u16	q0, ip, r9, #2
[^>]*> ee1d 0f69 	viwdup.u16	q0, ip, r9, #2
[^>]*> ee1d 1fe8 	vdwdup.u16	q0, ip, r9, #4
[^>]*> ee1d 0fe8 	viwdup.u16	q0, ip, r9, #4
[^>]*> ee1d 1fe9 	vdwdup.u16	q0, ip, r9, #8
[^>]*> ee1d 0fe9 	viwdup.u16	q0, ip, r9, #8
[^>]*> ee1d 1f6a 	vdwdup.u16	q0, ip, fp, #1
[^>]*> ee1d 0f6a 	viwdup.u16	q0, ip, fp, #1
[^>]*> ee1d 1f6b 	vdwdup.u16	q0, ip, fp, #2
[^>]*> ee1d 0f6b 	viwdup.u16	q0, ip, fp, #2
[^>]*> ee1d 1fea 	vdwdup.u16	q0, ip, fp, #4
[^>]*> ee1d 0fea 	viwdup.u16	q0, ip, fp, #4
[^>]*> ee1d 1feb 	vdwdup.u16	q0, ip, fp, #8
[^>]*> ee1d 0feb 	viwdup.u16	q0, ip, fp, #8
[^>]*> ee11 3f6e 	vddup.u16	q1, r0, #1
[^>]*> ee11 2f6e 	vidup.u16	q1, r0, #1
[^>]*> ee11 3f6f 	vddup.u16	q1, r0, #2
[^>]*> ee11 2f6f 	vidup.u16	q1, r0, #2
[^>]*> ee11 3fee 	vddup.u16	q1, r0, #4
[^>]*> ee11 2fee 	vidup.u16	q1, r0, #4
[^>]*> ee11 3fef 	vddup.u16	q1, r0, #8
[^>]*> ee11 2fef 	vidup.u16	q1, r0, #8
[^>]*> ee11 3f60 	vdwdup.u16	q1, r0, r1, #1
[^>]*> ee11 2f60 	viwdup.u16	q1, r0, r1, #1
[^>]*> ee11 3f61 	vdwdup.u16	q1, r0, r1, #2
[^>]*> ee11 2f61 	viwdup.u16	q1, r0, r1, #2
[^>]*> ee11 3fe0 	vdwdup.u16	q1, r0, r1, #4
[^>]*> ee11 2fe0 	viwdup.u16	q1, r0, r1, #4
[^>]*> ee11 3fe1 	vdwdup.u16	q1, r0, r1, #8
[^>]*> ee11 2fe1 	viwdup.u16	q1, r0, r1, #8
[^>]*> ee11 3f62 	vdwdup.u16	q1, r0, r3, #1
[^>]*> ee11 2f62 	viwdup.u16	q1, r0, r3, #1
[^>]*> ee11 3f63 	vdwdup.u16	q1, r0, r3, #2
[^>]*> ee11 2f63 	viwdup.u16	q1, r0, r3, #2
[^>]*> ee11 3fe2 	vdwdup.u16	q1, r0, r3, #4
[^>]*> ee11 2fe2 	viwdup.u16	q1, r0, r3, #4
[^>]*> ee11 3fe3 	vdwdup.u16	q1, r0, r3, #8
[^>]*> ee11 2fe3 	viwdup.u16	q1, r0, r3, #8
[^>]*> ee11 3f64 	vdwdup.u16	q1, r0, r5, #1
[^>]*> ee11 2f64 	viwdup.u16	q1, r0, r5, #1
[^>]*> ee11 3f65 	vdwdup.u16	q1, r0, r5, #2
[^>]*> ee11 2f65 	viwdup.u16	q1, r0, r5, #2
[^>]*> ee11 3fe4 	vdwdup.u16	q1, r0, r5, #4
[^>]*> ee11 2fe4 	viwdup.u16	q1, r0, r5, #4
[^>]*> ee11 3fe5 	vdwdup.u16	q1, r0, r5, #8
[^>]*> ee11 2fe5 	viwdup.u16	q1, r0, r5, #8
[^>]*> ee11 3f66 	vdwdup.u16	q1, r0, r7, #1
[^>]*> ee11 2f66 	viwdup.u16	q1, r0, r7, #1
[^>]*> ee11 3f67 	vdwdup.u16	q1, r0, r7, #2
[^>]*> ee11 2f67 	viwdup.u16	q1, r0, r7, #2
[^>]*> ee11 3fe6 	vdwdup.u16	q1, r0, r7, #4
[^>]*> ee11 2fe6 	viwdup.u16	q1, r0, r7, #4
[^>]*> ee11 3fe7 	vdwdup.u16	q1, r0, r7, #8
[^>]*> ee11 2fe7 	viwdup.u16	q1, r0, r7, #8
[^>]*> ee11 3f68 	vdwdup.u16	q1, r0, r9, #1
[^>]*> ee11 2f68 	viwdup.u16	q1, r0, r9, #1
[^>]*> ee11 3f69 	vdwdup.u16	q1, r0, r9, #2
[^>]*> ee11 2f69 	viwdup.u16	q1, r0, r9, #2
[^>]*> ee11 3fe8 	vdwdup.u16	q1, r0, r9, #4
[^>]*> ee11 2fe8 	viwdup.u16	q1, r0, r9, #4
[^>]*> ee11 3fe9 	vdwdup.u16	q1, r0, r9, #8
[^>]*> ee11 2fe9 	viwdup.u16	q1, r0, r9, #8
[^>]*> ee11 3f6a 	vdwdup.u16	q1, r0, fp, #1
[^>]*> ee11 2f6a 	viwdup.u16	q1, r0, fp, #1
[^>]*> ee11 3f6b 	vdwdup.u16	q1, r0, fp, #2
[^>]*> ee11 2f6b 	viwdup.u16	q1, r0, fp, #2
[^>]*> ee11 3fea 	vdwdup.u16	q1, r0, fp, #4
[^>]*> ee11 2fea 	viwdup.u16	q1, r0, fp, #4
[^>]*> ee11 3feb 	vdwdup.u16	q1, r0, fp, #8
[^>]*> ee11 2feb 	viwdup.u16	q1, r0, fp, #8
[^>]*> ee13 3f6e 	vddup.u16	q1, r2, #1
[^>]*> ee13 2f6e 	vidup.u16	q1, r2, #1
[^>]*> ee13 3f6f 	vddup.u16	q1, r2, #2
[^>]*> ee13 2f6f 	vidup.u16	q1, r2, #2
[^>]*> ee13 3fee 	vddup.u16	q1, r2, #4
[^>]*> ee13 2fee 	vidup.u16	q1, r2, #4
[^>]*> ee13 3fef 	vddup.u16	q1, r2, #8
[^>]*> ee13 2fef 	vidup.u16	q1, r2, #8
[^>]*> ee13 3f60 	vdwdup.u16	q1, r2, r1, #1
[^>]*> ee13 2f60 	viwdup.u16	q1, r2, r1, #1
[^>]*> ee13 3f61 	vdwdup.u16	q1, r2, r1, #2
[^>]*> ee13 2f61 	viwdup.u16	q1, r2, r1, #2
[^>]*> ee13 3fe0 	vdwdup.u16	q1, r2, r1, #4
[^>]*> ee13 2fe0 	viwdup.u16	q1, r2, r1, #4
[^>]*> ee13 3fe1 	vdwdup.u16	q1, r2, r1, #8
[^>]*> ee13 2fe1 	viwdup.u16	q1, r2, r1, #8
[^>]*> ee13 3f62 	vdwdup.u16	q1, r2, r3, #1
[^>]*> ee13 2f62 	viwdup.u16	q1, r2, r3, #1
[^>]*> ee13 3f63 	vdwdup.u16	q1, r2, r3, #2
[^>]*> ee13 2f63 	viwdup.u16	q1, r2, r3, #2
[^>]*> ee13 3fe2 	vdwdup.u16	q1, r2, r3, #4
[^>]*> ee13 2fe2 	viwdup.u16	q1, r2, r3, #4
[^>]*> ee13 3fe3 	vdwdup.u16	q1, r2, r3, #8
[^>]*> ee13 2fe3 	viwdup.u16	q1, r2, r3, #8
[^>]*> ee13 3f64 	vdwdup.u16	q1, r2, r5, #1
[^>]*> ee13 2f64 	viwdup.u16	q1, r2, r5, #1
[^>]*> ee13 3f65 	vdwdup.u16	q1, r2, r5, #2
[^>]*> ee13 2f65 	viwdup.u16	q1, r2, r5, #2
[^>]*> ee13 3fe4 	vdwdup.u16	q1, r2, r5, #4
[^>]*> ee13 2fe4 	viwdup.u16	q1, r2, r5, #4
[^>]*> ee13 3fe5 	vdwdup.u16	q1, r2, r5, #8
[^>]*> ee13 2fe5 	viwdup.u16	q1, r2, r5, #8
[^>]*> ee13 3f66 	vdwdup.u16	q1, r2, r7, #1
[^>]*> ee13 2f66 	viwdup.u16	q1, r2, r7, #1
[^>]*> ee13 3f67 	vdwdup.u16	q1, r2, r7, #2
[^>]*> ee13 2f67 	viwdup.u16	q1, r2, r7, #2
[^>]*> ee13 3fe6 	vdwdup.u16	q1, r2, r7, #4
[^>]*> ee13 2fe6 	viwdup.u16	q1, r2, r7, #4
[^>]*> ee13 3fe7 	vdwdup.u16	q1, r2, r7, #8
[^>]*> ee13 2fe7 	viwdup.u16	q1, r2, r7, #8
[^>]*> ee13 3f68 	vdwdup.u16	q1, r2, r9, #1
[^>]*> ee13 2f68 	viwdup.u16	q1, r2, r9, #1
[^>]*> ee13 3f69 	vdwdup.u16	q1, r2, r9, #2
[^>]*> ee13 2f69 	viwdup.u16	q1, r2, r9, #2
[^>]*> ee13 3fe8 	vdwdup.u16	q1, r2, r9, #4
[^>]*> ee13 2fe8 	viwdup.u16	q1, r2, r9, #4
[^>]*> ee13 3fe9 	vdwdup.u16	q1, r2, r9, #8
[^>]*> ee13 2fe9 	viwdup.u16	q1, r2, r9, #8
[^>]*> ee13 3f6a 	vdwdup.u16	q1, r2, fp, #1
[^>]*> ee13 2f6a 	viwdup.u16	q1, r2, fp, #1
[^>]*> ee13 3f6b 	vdwdup.u16	q1, r2, fp, #2
[^>]*> ee13 2f6b 	viwdup.u16	q1, r2, fp, #2
[^>]*> ee13 3fea 	vdwdup.u16	q1, r2, fp, #4
[^>]*> ee13 2fea 	viwdup.u16	q1, r2, fp, #4
[^>]*> ee13 3feb 	vdwdup.u16	q1, r2, fp, #8
[^>]*> ee13 2feb 	viwdup.u16	q1, r2, fp, #8
[^>]*> ee15 3f6e 	vddup.u16	q1, r4, #1
[^>]*> ee15 2f6e 	vidup.u16	q1, r4, #1
[^>]*> ee15 3f6f 	vddup.u16	q1, r4, #2
[^>]*> ee15 2f6f 	vidup.u16	q1, r4, #2
[^>]*> ee15 3fee 	vddup.u16	q1, r4, #4
[^>]*> ee15 2fee 	vidup.u16	q1, r4, #4
[^>]*> ee15 3fef 	vddup.u16	q1, r4, #8
[^>]*> ee15 2fef 	vidup.u16	q1, r4, #8
[^>]*> ee15 3f60 	vdwdup.u16	q1, r4, r1, #1
[^>]*> ee15 2f60 	viwdup.u16	q1, r4, r1, #1
[^>]*> ee15 3f61 	vdwdup.u16	q1, r4, r1, #2
[^>]*> ee15 2f61 	viwdup.u16	q1, r4, r1, #2
[^>]*> ee15 3fe0 	vdwdup.u16	q1, r4, r1, #4
[^>]*> ee15 2fe0 	viwdup.u16	q1, r4, r1, #4
[^>]*> ee15 3fe1 	vdwdup.u16	q1, r4, r1, #8
[^>]*> ee15 2fe1 	viwdup.u16	q1, r4, r1, #8
[^>]*> ee15 3f62 	vdwdup.u16	q1, r4, r3, #1
[^>]*> ee15 2f62 	viwdup.u16	q1, r4, r3, #1
[^>]*> ee15 3f63 	vdwdup.u16	q1, r4, r3, #2
[^>]*> ee15 2f63 	viwdup.u16	q1, r4, r3, #2
[^>]*> ee15 3fe2 	vdwdup.u16	q1, r4, r3, #4
[^>]*> ee15 2fe2 	viwdup.u16	q1, r4, r3, #4
[^>]*> ee15 3fe3 	vdwdup.u16	q1, r4, r3, #8
[^>]*> ee15 2fe3 	viwdup.u16	q1, r4, r3, #8
[^>]*> ee15 3f64 	vdwdup.u16	q1, r4, r5, #1
[^>]*> ee15 2f64 	viwdup.u16	q1, r4, r5, #1
[^>]*> ee15 3f65 	vdwdup.u16	q1, r4, r5, #2
[^>]*> ee15 2f65 	viwdup.u16	q1, r4, r5, #2
[^>]*> ee15 3fe4 	vdwdup.u16	q1, r4, r5, #4
[^>]*> ee15 2fe4 	viwdup.u16	q1, r4, r5, #4
[^>]*> ee15 3fe5 	vdwdup.u16	q1, r4, r5, #8
[^>]*> ee15 2fe5 	viwdup.u16	q1, r4, r5, #8
[^>]*> ee15 3f66 	vdwdup.u16	q1, r4, r7, #1
[^>]*> ee15 2f66 	viwdup.u16	q1, r4, r7, #1
[^>]*> ee15 3f67 	vdwdup.u16	q1, r4, r7, #2
[^>]*> ee15 2f67 	viwdup.u16	q1, r4, r7, #2
[^>]*> ee15 3fe6 	vdwdup.u16	q1, r4, r7, #4
[^>]*> ee15 2fe6 	viwdup.u16	q1, r4, r7, #4
[^>]*> ee15 3fe7 	vdwdup.u16	q1, r4, r7, #8
[^>]*> ee15 2fe7 	viwdup.u16	q1, r4, r7, #8
[^>]*> ee15 3f68 	vdwdup.u16	q1, r4, r9, #1
[^>]*> ee15 2f68 	viwdup.u16	q1, r4, r9, #1
[^>]*> ee15 3f69 	vdwdup.u16	q1, r4, r9, #2
[^>]*> ee15 2f69 	viwdup.u16	q1, r4, r9, #2
[^>]*> ee15 3fe8 	vdwdup.u16	q1, r4, r9, #4
[^>]*> ee15 2fe8 	viwdup.u16	q1, r4, r9, #4
[^>]*> ee15 3fe9 	vdwdup.u16	q1, r4, r9, #8
[^>]*> ee15 2fe9 	viwdup.u16	q1, r4, r9, #8
[^>]*> ee15 3f6a 	vdwdup.u16	q1, r4, fp, #1
[^>]*> ee15 2f6a 	viwdup.u16	q1, r4, fp, #1
[^>]*> ee15 3f6b 	vdwdup.u16	q1, r4, fp, #2
[^>]*> ee15 2f6b 	viwdup.u16	q1, r4, fp, #2
[^>]*> ee15 3fea 	vdwdup.u16	q1, r4, fp, #4
[^>]*> ee15 2fea 	viwdup.u16	q1, r4, fp, #4
[^>]*> ee15 3feb 	vdwdup.u16	q1, r4, fp, #8
[^>]*> ee15 2feb 	viwdup.u16	q1, r4, fp, #8
[^>]*> ee17 3f6e 	vddup.u16	q1, r6, #1
[^>]*> ee17 2f6e 	vidup.u16	q1, r6, #1
[^>]*> ee17 3f6f 	vddup.u16	q1, r6, #2
[^>]*> ee17 2f6f 	vidup.u16	q1, r6, #2
[^>]*> ee17 3fee 	vddup.u16	q1, r6, #4
[^>]*> ee17 2fee 	vidup.u16	q1, r6, #4
[^>]*> ee17 3fef 	vddup.u16	q1, r6, #8
[^>]*> ee17 2fef 	vidup.u16	q1, r6, #8
[^>]*> ee17 3f60 	vdwdup.u16	q1, r6, r1, #1
[^>]*> ee17 2f60 	viwdup.u16	q1, r6, r1, #1
[^>]*> ee17 3f61 	vdwdup.u16	q1, r6, r1, #2
[^>]*> ee17 2f61 	viwdup.u16	q1, r6, r1, #2
[^>]*> ee17 3fe0 	vdwdup.u16	q1, r6, r1, #4
[^>]*> ee17 2fe0 	viwdup.u16	q1, r6, r1, #4
[^>]*> ee17 3fe1 	vdwdup.u16	q1, r6, r1, #8
[^>]*> ee17 2fe1 	viwdup.u16	q1, r6, r1, #8
[^>]*> ee17 3f62 	vdwdup.u16	q1, r6, r3, #1
[^>]*> ee17 2f62 	viwdup.u16	q1, r6, r3, #1
[^>]*> ee17 3f63 	vdwdup.u16	q1, r6, r3, #2
[^>]*> ee17 2f63 	viwdup.u16	q1, r6, r3, #2
[^>]*> ee17 3fe2 	vdwdup.u16	q1, r6, r3, #4
[^>]*> ee17 2fe2 	viwdup.u16	q1, r6, r3, #4
[^>]*> ee17 3fe3 	vdwdup.u16	q1, r6, r3, #8
[^>]*> ee17 2fe3 	viwdup.u16	q1, r6, r3, #8
[^>]*> ee17 3f64 	vdwdup.u16	q1, r6, r5, #1
[^>]*> ee17 2f64 	viwdup.u16	q1, r6, r5, #1
[^>]*> ee17 3f65 	vdwdup.u16	q1, r6, r5, #2
[^>]*> ee17 2f65 	viwdup.u16	q1, r6, r5, #2
[^>]*> ee17 3fe4 	vdwdup.u16	q1, r6, r5, #4
[^>]*> ee17 2fe4 	viwdup.u16	q1, r6, r5, #4
[^>]*> ee17 3fe5 	vdwdup.u16	q1, r6, r5, #8
[^>]*> ee17 2fe5 	viwdup.u16	q1, r6, r5, #8
[^>]*> ee17 3f66 	vdwdup.u16	q1, r6, r7, #1
[^>]*> ee17 2f66 	viwdup.u16	q1, r6, r7, #1
[^>]*> ee17 3f67 	vdwdup.u16	q1, r6, r7, #2
[^>]*> ee17 2f67 	viwdup.u16	q1, r6, r7, #2
[^>]*> ee17 3fe6 	vdwdup.u16	q1, r6, r7, #4
[^>]*> ee17 2fe6 	viwdup.u16	q1, r6, r7, #4
[^>]*> ee17 3fe7 	vdwdup.u16	q1, r6, r7, #8
[^>]*> ee17 2fe7 	viwdup.u16	q1, r6, r7, #8
[^>]*> ee17 3f68 	vdwdup.u16	q1, r6, r9, #1
[^>]*> ee17 2f68 	viwdup.u16	q1, r6, r9, #1
[^>]*> ee17 3f69 	vdwdup.u16	q1, r6, r9, #2
[^>]*> ee17 2f69 	viwdup.u16	q1, r6, r9, #2
[^>]*> ee17 3fe8 	vdwdup.u16	q1, r6, r9, #4
[^>]*> ee17 2fe8 	viwdup.u16	q1, r6, r9, #4
[^>]*> ee17 3fe9 	vdwdup.u16	q1, r6, r9, #8
[^>]*> ee17 2fe9 	viwdup.u16	q1, r6, r9, #8
[^>]*> ee17 3f6a 	vdwdup.u16	q1, r6, fp, #1
[^>]*> ee17 2f6a 	viwdup.u16	q1, r6, fp, #1
[^>]*> ee17 3f6b 	vdwdup.u16	q1, r6, fp, #2
[^>]*> ee17 2f6b 	viwdup.u16	q1, r6, fp, #2
[^>]*> ee17 3fea 	vdwdup.u16	q1, r6, fp, #4
[^>]*> ee17 2fea 	viwdup.u16	q1, r6, fp, #4
[^>]*> ee17 3feb 	vdwdup.u16	q1, r6, fp, #8
[^>]*> ee17 2feb 	viwdup.u16	q1, r6, fp, #8
[^>]*> ee19 3f6e 	vddup.u16	q1, r8, #1
[^>]*> ee19 2f6e 	vidup.u16	q1, r8, #1
[^>]*> ee19 3f6f 	vddup.u16	q1, r8, #2
[^>]*> ee19 2f6f 	vidup.u16	q1, r8, #2
[^>]*> ee19 3fee 	vddup.u16	q1, r8, #4
[^>]*> ee19 2fee 	vidup.u16	q1, r8, #4
[^>]*> ee19 3fef 	vddup.u16	q1, r8, #8
[^>]*> ee19 2fef 	vidup.u16	q1, r8, #8
[^>]*> ee19 3f60 	vdwdup.u16	q1, r8, r1, #1
[^>]*> ee19 2f60 	viwdup.u16	q1, r8, r1, #1
[^>]*> ee19 3f61 	vdwdup.u16	q1, r8, r1, #2
[^>]*> ee19 2f61 	viwdup.u16	q1, r8, r1, #2
[^>]*> ee19 3fe0 	vdwdup.u16	q1, r8, r1, #4
[^>]*> ee19 2fe0 	viwdup.u16	q1, r8, r1, #4
[^>]*> ee19 3fe1 	vdwdup.u16	q1, r8, r1, #8
[^>]*> ee19 2fe1 	viwdup.u16	q1, r8, r1, #8
[^>]*> ee19 3f62 	vdwdup.u16	q1, r8, r3, #1
[^>]*> ee19 2f62 	viwdup.u16	q1, r8, r3, #1
[^>]*> ee19 3f63 	vdwdup.u16	q1, r8, r3, #2
[^>]*> ee19 2f63 	viwdup.u16	q1, r8, r3, #2
[^>]*> ee19 3fe2 	vdwdup.u16	q1, r8, r3, #4
[^>]*> ee19 2fe2 	viwdup.u16	q1, r8, r3, #4
[^>]*> ee19 3fe3 	vdwdup.u16	q1, r8, r3, #8
[^>]*> ee19 2fe3 	viwdup.u16	q1, r8, r3, #8
[^>]*> ee19 3f64 	vdwdup.u16	q1, r8, r5, #1
[^>]*> ee19 2f64 	viwdup.u16	q1, r8, r5, #1
[^>]*> ee19 3f65 	vdwdup.u16	q1, r8, r5, #2
[^>]*> ee19 2f65 	viwdup.u16	q1, r8, r5, #2
[^>]*> ee19 3fe4 	vdwdup.u16	q1, r8, r5, #4
[^>]*> ee19 2fe4 	viwdup.u16	q1, r8, r5, #4
[^>]*> ee19 3fe5 	vdwdup.u16	q1, r8, r5, #8
[^>]*> ee19 2fe5 	viwdup.u16	q1, r8, r5, #8
[^>]*> ee19 3f66 	vdwdup.u16	q1, r8, r7, #1
[^>]*> ee19 2f66 	viwdup.u16	q1, r8, r7, #1
[^>]*> ee19 3f67 	vdwdup.u16	q1, r8, r7, #2
[^>]*> ee19 2f67 	viwdup.u16	q1, r8, r7, #2
[^>]*> ee19 3fe6 	vdwdup.u16	q1, r8, r7, #4
[^>]*> ee19 2fe6 	viwdup.u16	q1, r8, r7, #4
[^>]*> ee19 3fe7 	vdwdup.u16	q1, r8, r7, #8
[^>]*> ee19 2fe7 	viwdup.u16	q1, r8, r7, #8
[^>]*> ee19 3f68 	vdwdup.u16	q1, r8, r9, #1
[^>]*> ee19 2f68 	viwdup.u16	q1, r8, r9, #1
[^>]*> ee19 3f69 	vdwdup.u16	q1, r8, r9, #2
[^>]*> ee19 2f69 	viwdup.u16	q1, r8, r9, #2
[^>]*> ee19 3fe8 	vdwdup.u16	q1, r8, r9, #4
[^>]*> ee19 2fe8 	viwdup.u16	q1, r8, r9, #4
[^>]*> ee19 3fe9 	vdwdup.u16	q1, r8, r9, #8
[^>]*> ee19 2fe9 	viwdup.u16	q1, r8, r9, #8
[^>]*> ee19 3f6a 	vdwdup.u16	q1, r8, fp, #1
[^>]*> ee19 2f6a 	viwdup.u16	q1, r8, fp, #1
[^>]*> ee19 3f6b 	vdwdup.u16	q1, r8, fp, #2
[^>]*> ee19 2f6b 	viwdup.u16	q1, r8, fp, #2
[^>]*> ee19 3fea 	vdwdup.u16	q1, r8, fp, #4
[^>]*> ee19 2fea 	viwdup.u16	q1, r8, fp, #4
[^>]*> ee19 3feb 	vdwdup.u16	q1, r8, fp, #8
[^>]*> ee19 2feb 	viwdup.u16	q1, r8, fp, #8
[^>]*> ee1b 3f6e 	vddup.u16	q1, sl, #1
[^>]*> ee1b 2f6e 	vidup.u16	q1, sl, #1
[^>]*> ee1b 3f6f 	vddup.u16	q1, sl, #2
[^>]*> ee1b 2f6f 	vidup.u16	q1, sl, #2
[^>]*> ee1b 3fee 	vddup.u16	q1, sl, #4
[^>]*> ee1b 2fee 	vidup.u16	q1, sl, #4
[^>]*> ee1b 3fef 	vddup.u16	q1, sl, #8
[^>]*> ee1b 2fef 	vidup.u16	q1, sl, #8
[^>]*> ee1b 3f60 	vdwdup.u16	q1, sl, r1, #1
[^>]*> ee1b 2f60 	viwdup.u16	q1, sl, r1, #1
[^>]*> ee1b 3f61 	vdwdup.u16	q1, sl, r1, #2
[^>]*> ee1b 2f61 	viwdup.u16	q1, sl, r1, #2
[^>]*> ee1b 3fe0 	vdwdup.u16	q1, sl, r1, #4
[^>]*> ee1b 2fe0 	viwdup.u16	q1, sl, r1, #4
[^>]*> ee1b 3fe1 	vdwdup.u16	q1, sl, r1, #8
[^>]*> ee1b 2fe1 	viwdup.u16	q1, sl, r1, #8
[^>]*> ee1b 3f62 	vdwdup.u16	q1, sl, r3, #1
[^>]*> ee1b 2f62 	viwdup.u16	q1, sl, r3, #1
[^>]*> ee1b 3f63 	vdwdup.u16	q1, sl, r3, #2
[^>]*> ee1b 2f63 	viwdup.u16	q1, sl, r3, #2
[^>]*> ee1b 3fe2 	vdwdup.u16	q1, sl, r3, #4
[^>]*> ee1b 2fe2 	viwdup.u16	q1, sl, r3, #4
[^>]*> ee1b 3fe3 	vdwdup.u16	q1, sl, r3, #8
[^>]*> ee1b 2fe3 	viwdup.u16	q1, sl, r3, #8
[^>]*> ee1b 3f64 	vdwdup.u16	q1, sl, r5, #1
[^>]*> ee1b 2f64 	viwdup.u16	q1, sl, r5, #1
[^>]*> ee1b 3f65 	vdwdup.u16	q1, sl, r5, #2
[^>]*> ee1b 2f65 	viwdup.u16	q1, sl, r5, #2
[^>]*> ee1b 3fe4 	vdwdup.u16	q1, sl, r5, #4
[^>]*> ee1b 2fe4 	viwdup.u16	q1, sl, r5, #4
[^>]*> ee1b 3fe5 	vdwdup.u16	q1, sl, r5, #8
[^>]*> ee1b 2fe5 	viwdup.u16	q1, sl, r5, #8
[^>]*> ee1b 3f66 	vdwdup.u16	q1, sl, r7, #1
[^>]*> ee1b 2f66 	viwdup.u16	q1, sl, r7, #1
[^>]*> ee1b 3f67 	vdwdup.u16	q1, sl, r7, #2
[^>]*> ee1b 2f67 	viwdup.u16	q1, sl, r7, #2
[^>]*> ee1b 3fe6 	vdwdup.u16	q1, sl, r7, #4
[^>]*> ee1b 2fe6 	viwdup.u16	q1, sl, r7, #4
[^>]*> ee1b 3fe7 	vdwdup.u16	q1, sl, r7, #8
[^>]*> ee1b 2fe7 	viwdup.u16	q1, sl, r7, #8
[^>]*> ee1b 3f68 	vdwdup.u16	q1, sl, r9, #1
[^>]*> ee1b 2f68 	viwdup.u16	q1, sl, r9, #1
[^>]*> ee1b 3f69 	vdwdup.u16	q1, sl, r9, #2
[^>]*> ee1b 2f69 	viwdup.u16	q1, sl, r9, #2
[^>]*> ee1b 3fe8 	vdwdup.u16	q1, sl, r9, #4
[^>]*> ee1b 2fe8 	viwdup.u16	q1, sl, r9, #4
[^>]*> ee1b 3fe9 	vdwdup.u16	q1, sl, r9, #8
[^>]*> ee1b 2fe9 	viwdup.u16	q1, sl, r9, #8
[^>]*> ee1b 3f6a 	vdwdup.u16	q1, sl, fp, #1
[^>]*> ee1b 2f6a 	viwdup.u16	q1, sl, fp, #1
[^>]*> ee1b 3f6b 	vdwdup.u16	q1, sl, fp, #2
[^>]*> ee1b 2f6b 	viwdup.u16	q1, sl, fp, #2
[^>]*> ee1b 3fea 	vdwdup.u16	q1, sl, fp, #4
[^>]*> ee1b 2fea 	viwdup.u16	q1, sl, fp, #4
[^>]*> ee1b 3feb 	vdwdup.u16	q1, sl, fp, #8
[^>]*> ee1b 2feb 	viwdup.u16	q1, sl, fp, #8
[^>]*> ee1d 3f6e 	vddup.u16	q1, ip, #1
[^>]*> ee1d 2f6e 	vidup.u16	q1, ip, #1
[^>]*> ee1d 3f6f 	vddup.u16	q1, ip, #2
[^>]*> ee1d 2f6f 	vidup.u16	q1, ip, #2
[^>]*> ee1d 3fee 	vddup.u16	q1, ip, #4
[^>]*> ee1d 2fee 	vidup.u16	q1, ip, #4
[^>]*> ee1d 3fef 	vddup.u16	q1, ip, #8
[^>]*> ee1d 2fef 	vidup.u16	q1, ip, #8
[^>]*> ee1d 3f60 	vdwdup.u16	q1, ip, r1, #1
[^>]*> ee1d 2f60 	viwdup.u16	q1, ip, r1, #1
[^>]*> ee1d 3f61 	vdwdup.u16	q1, ip, r1, #2
[^>]*> ee1d 2f61 	viwdup.u16	q1, ip, r1, #2
[^>]*> ee1d 3fe0 	vdwdup.u16	q1, ip, r1, #4
[^>]*> ee1d 2fe0 	viwdup.u16	q1, ip, r1, #4
[^>]*> ee1d 3fe1 	vdwdup.u16	q1, ip, r1, #8
[^>]*> ee1d 2fe1 	viwdup.u16	q1, ip, r1, #8
[^>]*> ee1d 3f62 	vdwdup.u16	q1, ip, r3, #1
[^>]*> ee1d 2f62 	viwdup.u16	q1, ip, r3, #1
[^>]*> ee1d 3f63 	vdwdup.u16	q1, ip, r3, #2
[^>]*> ee1d 2f63 	viwdup.u16	q1, ip, r3, #2
[^>]*> ee1d 3fe2 	vdwdup.u16	q1, ip, r3, #4
[^>]*> ee1d 2fe2 	viwdup.u16	q1, ip, r3, #4
[^>]*> ee1d 3fe3 	vdwdup.u16	q1, ip, r3, #8
[^>]*> ee1d 2fe3 	viwdup.u16	q1, ip, r3, #8
[^>]*> ee1d 3f64 	vdwdup.u16	q1, ip, r5, #1
[^>]*> ee1d 2f64 	viwdup.u16	q1, ip, r5, #1
[^>]*> ee1d 3f65 	vdwdup.u16	q1, ip, r5, #2
[^>]*> ee1d 2f65 	viwdup.u16	q1, ip, r5, #2
[^>]*> ee1d 3fe4 	vdwdup.u16	q1, ip, r5, #4
[^>]*> ee1d 2fe4 	viwdup.u16	q1, ip, r5, #4
[^>]*> ee1d 3fe5 	vdwdup.u16	q1, ip, r5, #8
[^>]*> ee1d 2fe5 	viwdup.u16	q1, ip, r5, #8
[^>]*> ee1d 3f66 	vdwdup.u16	q1, ip, r7, #1
[^>]*> ee1d 2f66 	viwdup.u16	q1, ip, r7, #1
[^>]*> ee1d 3f67 	vdwdup.u16	q1, ip, r7, #2
[^>]*> ee1d 2f67 	viwdup.u16	q1, ip, r7, #2
[^>]*> ee1d 3fe6 	vdwdup.u16	q1, ip, r7, #4
[^>]*> ee1d 2fe6 	viwdup.u16	q1, ip, r7, #4
[^>]*> ee1d 3fe7 	vdwdup.u16	q1, ip, r7, #8
[^>]*> ee1d 2fe7 	viwdup.u16	q1, ip, r7, #8
[^>]*> ee1d 3f68 	vdwdup.u16	q1, ip, r9, #1
[^>]*> ee1d 2f68 	viwdup.u16	q1, ip, r9, #1
[^>]*> ee1d 3f69 	vdwdup.u16	q1, ip, r9, #2
[^>]*> ee1d 2f69 	viwdup.u16	q1, ip, r9, #2
[^>]*> ee1d 3fe8 	vdwdup.u16	q1, ip, r9, #4
[^>]*> ee1d 2fe8 	viwdup.u16	q1, ip, r9, #4
[^>]*> ee1d 3fe9 	vdwdup.u16	q1, ip, r9, #8
[^>]*> ee1d 2fe9 	viwdup.u16	q1, ip, r9, #8
[^>]*> ee1d 3f6a 	vdwdup.u16	q1, ip, fp, #1
[^>]*> ee1d 2f6a 	viwdup.u16	q1, ip, fp, #1
[^>]*> ee1d 3f6b 	vdwdup.u16	q1, ip, fp, #2
[^>]*> ee1d 2f6b 	viwdup.u16	q1, ip, fp, #2
[^>]*> ee1d 3fea 	vdwdup.u16	q1, ip, fp, #4
[^>]*> ee1d 2fea 	viwdup.u16	q1, ip, fp, #4
[^>]*> ee1d 3feb 	vdwdup.u16	q1, ip, fp, #8
[^>]*> ee1d 2feb 	viwdup.u16	q1, ip, fp, #8
[^>]*> ee11 5f6e 	vddup.u16	q2, r0, #1
[^>]*> ee11 4f6e 	vidup.u16	q2, r0, #1
[^>]*> ee11 5f6f 	vddup.u16	q2, r0, #2
[^>]*> ee11 4f6f 	vidup.u16	q2, r0, #2
[^>]*> ee11 5fee 	vddup.u16	q2, r0, #4
[^>]*> ee11 4fee 	vidup.u16	q2, r0, #4
[^>]*> ee11 5fef 	vddup.u16	q2, r0, #8
[^>]*> ee11 4fef 	vidup.u16	q2, r0, #8
[^>]*> ee11 5f60 	vdwdup.u16	q2, r0, r1, #1
[^>]*> ee11 4f60 	viwdup.u16	q2, r0, r1, #1
[^>]*> ee11 5f61 	vdwdup.u16	q2, r0, r1, #2
[^>]*> ee11 4f61 	viwdup.u16	q2, r0, r1, #2
[^>]*> ee11 5fe0 	vdwdup.u16	q2, r0, r1, #4
[^>]*> ee11 4fe0 	viwdup.u16	q2, r0, r1, #4
[^>]*> ee11 5fe1 	vdwdup.u16	q2, r0, r1, #8
[^>]*> ee11 4fe1 	viwdup.u16	q2, r0, r1, #8
[^>]*> ee11 5f62 	vdwdup.u16	q2, r0, r3, #1
[^>]*> ee11 4f62 	viwdup.u16	q2, r0, r3, #1
[^>]*> ee11 5f63 	vdwdup.u16	q2, r0, r3, #2
[^>]*> ee11 4f63 	viwdup.u16	q2, r0, r3, #2
[^>]*> ee11 5fe2 	vdwdup.u16	q2, r0, r3, #4
[^>]*> ee11 4fe2 	viwdup.u16	q2, r0, r3, #4
[^>]*> ee11 5fe3 	vdwdup.u16	q2, r0, r3, #8
[^>]*> ee11 4fe3 	viwdup.u16	q2, r0, r3, #8
[^>]*> ee11 5f64 	vdwdup.u16	q2, r0, r5, #1
[^>]*> ee11 4f64 	viwdup.u16	q2, r0, r5, #1
[^>]*> ee11 5f65 	vdwdup.u16	q2, r0, r5, #2
[^>]*> ee11 4f65 	viwdup.u16	q2, r0, r5, #2
[^>]*> ee11 5fe4 	vdwdup.u16	q2, r0, r5, #4
[^>]*> ee11 4fe4 	viwdup.u16	q2, r0, r5, #4
[^>]*> ee11 5fe5 	vdwdup.u16	q2, r0, r5, #8
[^>]*> ee11 4fe5 	viwdup.u16	q2, r0, r5, #8
[^>]*> ee11 5f66 	vdwdup.u16	q2, r0, r7, #1
[^>]*> ee11 4f66 	viwdup.u16	q2, r0, r7, #1
[^>]*> ee11 5f67 	vdwdup.u16	q2, r0, r7, #2
[^>]*> ee11 4f67 	viwdup.u16	q2, r0, r7, #2
[^>]*> ee11 5fe6 	vdwdup.u16	q2, r0, r7, #4
[^>]*> ee11 4fe6 	viwdup.u16	q2, r0, r7, #4
[^>]*> ee11 5fe7 	vdwdup.u16	q2, r0, r7, #8
[^>]*> ee11 4fe7 	viwdup.u16	q2, r0, r7, #8
[^>]*> ee11 5f68 	vdwdup.u16	q2, r0, r9, #1
[^>]*> ee11 4f68 	viwdup.u16	q2, r0, r9, #1
[^>]*> ee11 5f69 	vdwdup.u16	q2, r0, r9, #2
[^>]*> ee11 4f69 	viwdup.u16	q2, r0, r9, #2
[^>]*> ee11 5fe8 	vdwdup.u16	q2, r0, r9, #4
[^>]*> ee11 4fe8 	viwdup.u16	q2, r0, r9, #4
[^>]*> ee11 5fe9 	vdwdup.u16	q2, r0, r9, #8
[^>]*> ee11 4fe9 	viwdup.u16	q2, r0, r9, #8
[^>]*> ee11 5f6a 	vdwdup.u16	q2, r0, fp, #1
[^>]*> ee11 4f6a 	viwdup.u16	q2, r0, fp, #1
[^>]*> ee11 5f6b 	vdwdup.u16	q2, r0, fp, #2
[^>]*> ee11 4f6b 	viwdup.u16	q2, r0, fp, #2
[^>]*> ee11 5fea 	vdwdup.u16	q2, r0, fp, #4
[^>]*> ee11 4fea 	viwdup.u16	q2, r0, fp, #4
[^>]*> ee11 5feb 	vdwdup.u16	q2, r0, fp, #8
[^>]*> ee11 4feb 	viwdup.u16	q2, r0, fp, #8
[^>]*> ee13 5f6e 	vddup.u16	q2, r2, #1
[^>]*> ee13 4f6e 	vidup.u16	q2, r2, #1
[^>]*> ee13 5f6f 	vddup.u16	q2, r2, #2
[^>]*> ee13 4f6f 	vidup.u16	q2, r2, #2
[^>]*> ee13 5fee 	vddup.u16	q2, r2, #4
[^>]*> ee13 4fee 	vidup.u16	q2, r2, #4
[^>]*> ee13 5fef 	vddup.u16	q2, r2, #8
[^>]*> ee13 4fef 	vidup.u16	q2, r2, #8
[^>]*> ee13 5f60 	vdwdup.u16	q2, r2, r1, #1
[^>]*> ee13 4f60 	viwdup.u16	q2, r2, r1, #1
[^>]*> ee13 5f61 	vdwdup.u16	q2, r2, r1, #2
[^>]*> ee13 4f61 	viwdup.u16	q2, r2, r1, #2
[^>]*> ee13 5fe0 	vdwdup.u16	q2, r2, r1, #4
[^>]*> ee13 4fe0 	viwdup.u16	q2, r2, r1, #4
[^>]*> ee13 5fe1 	vdwdup.u16	q2, r2, r1, #8
[^>]*> ee13 4fe1 	viwdup.u16	q2, r2, r1, #8
[^>]*> ee13 5f62 	vdwdup.u16	q2, r2, r3, #1
[^>]*> ee13 4f62 	viwdup.u16	q2, r2, r3, #1
[^>]*> ee13 5f63 	vdwdup.u16	q2, r2, r3, #2
[^>]*> ee13 4f63 	viwdup.u16	q2, r2, r3, #2
[^>]*> ee13 5fe2 	vdwdup.u16	q2, r2, r3, #4
[^>]*> ee13 4fe2 	viwdup.u16	q2, r2, r3, #4
[^>]*> ee13 5fe3 	vdwdup.u16	q2, r2, r3, #8
[^>]*> ee13 4fe3 	viwdup.u16	q2, r2, r3, #8
[^>]*> ee13 5f64 	vdwdup.u16	q2, r2, r5, #1
[^>]*> ee13 4f64 	viwdup.u16	q2, r2, r5, #1
[^>]*> ee13 5f65 	vdwdup.u16	q2, r2, r5, #2
[^>]*> ee13 4f65 	viwdup.u16	q2, r2, r5, #2
[^>]*> ee13 5fe4 	vdwdup.u16	q2, r2, r5, #4
[^>]*> ee13 4fe4 	viwdup.u16	q2, r2, r5, #4
[^>]*> ee13 5fe5 	vdwdup.u16	q2, r2, r5, #8
[^>]*> ee13 4fe5 	viwdup.u16	q2, r2, r5, #8
[^>]*> ee13 5f66 	vdwdup.u16	q2, r2, r7, #1
[^>]*> ee13 4f66 	viwdup.u16	q2, r2, r7, #1
[^>]*> ee13 5f67 	vdwdup.u16	q2, r2, r7, #2
[^>]*> ee13 4f67 	viwdup.u16	q2, r2, r7, #2
[^>]*> ee13 5fe6 	vdwdup.u16	q2, r2, r7, #4
[^>]*> ee13 4fe6 	viwdup.u16	q2, r2, r7, #4
[^>]*> ee13 5fe7 	vdwdup.u16	q2, r2, r7, #8
[^>]*> ee13 4fe7 	viwdup.u16	q2, r2, r7, #8
[^>]*> ee13 5f68 	vdwdup.u16	q2, r2, r9, #1
[^>]*> ee13 4f68 	viwdup.u16	q2, r2, r9, #1
[^>]*> ee13 5f69 	vdwdup.u16	q2, r2, r9, #2
[^>]*> ee13 4f69 	viwdup.u16	q2, r2, r9, #2
[^>]*> ee13 5fe8 	vdwdup.u16	q2, r2, r9, #4
[^>]*> ee13 4fe8 	viwdup.u16	q2, r2, r9, #4
[^>]*> ee13 5fe9 	vdwdup.u16	q2, r2, r9, #8
[^>]*> ee13 4fe9 	viwdup.u16	q2, r2, r9, #8
[^>]*> ee13 5f6a 	vdwdup.u16	q2, r2, fp, #1
[^>]*> ee13 4f6a 	viwdup.u16	q2, r2, fp, #1
[^>]*> ee13 5f6b 	vdwdup.u16	q2, r2, fp, #2
[^>]*> ee13 4f6b 	viwdup.u16	q2, r2, fp, #2
[^>]*> ee13 5fea 	vdwdup.u16	q2, r2, fp, #4
[^>]*> ee13 4fea 	viwdup.u16	q2, r2, fp, #4
[^>]*> ee13 5feb 	vdwdup.u16	q2, r2, fp, #8
[^>]*> ee13 4feb 	viwdup.u16	q2, r2, fp, #8
[^>]*> ee15 5f6e 	vddup.u16	q2, r4, #1
[^>]*> ee15 4f6e 	vidup.u16	q2, r4, #1
[^>]*> ee15 5f6f 	vddup.u16	q2, r4, #2
[^>]*> ee15 4f6f 	vidup.u16	q2, r4, #2
[^>]*> ee15 5fee 	vddup.u16	q2, r4, #4
[^>]*> ee15 4fee 	vidup.u16	q2, r4, #4
[^>]*> ee15 5fef 	vddup.u16	q2, r4, #8
[^>]*> ee15 4fef 	vidup.u16	q2, r4, #8
[^>]*> ee15 5f60 	vdwdup.u16	q2, r4, r1, #1
[^>]*> ee15 4f60 	viwdup.u16	q2, r4, r1, #1
[^>]*> ee15 5f61 	vdwdup.u16	q2, r4, r1, #2
[^>]*> ee15 4f61 	viwdup.u16	q2, r4, r1, #2
[^>]*> ee15 5fe0 	vdwdup.u16	q2, r4, r1, #4
[^>]*> ee15 4fe0 	viwdup.u16	q2, r4, r1, #4
[^>]*> ee15 5fe1 	vdwdup.u16	q2, r4, r1, #8
[^>]*> ee15 4fe1 	viwdup.u16	q2, r4, r1, #8
[^>]*> ee15 5f62 	vdwdup.u16	q2, r4, r3, #1
[^>]*> ee15 4f62 	viwdup.u16	q2, r4, r3, #1
[^>]*> ee15 5f63 	vdwdup.u16	q2, r4, r3, #2
[^>]*> ee15 4f63 	viwdup.u16	q2, r4, r3, #2
[^>]*> ee15 5fe2 	vdwdup.u16	q2, r4, r3, #4
[^>]*> ee15 4fe2 	viwdup.u16	q2, r4, r3, #4
[^>]*> ee15 5fe3 	vdwdup.u16	q2, r4, r3, #8
[^>]*> ee15 4fe3 	viwdup.u16	q2, r4, r3, #8
[^>]*> ee15 5f64 	vdwdup.u16	q2, r4, r5, #1
[^>]*> ee15 4f64 	viwdup.u16	q2, r4, r5, #1
[^>]*> ee15 5f65 	vdwdup.u16	q2, r4, r5, #2
[^>]*> ee15 4f65 	viwdup.u16	q2, r4, r5, #2
[^>]*> ee15 5fe4 	vdwdup.u16	q2, r4, r5, #4
[^>]*> ee15 4fe4 	viwdup.u16	q2, r4, r5, #4
[^>]*> ee15 5fe5 	vdwdup.u16	q2, r4, r5, #8
[^>]*> ee15 4fe5 	viwdup.u16	q2, r4, r5, #8
[^>]*> ee15 5f66 	vdwdup.u16	q2, r4, r7, #1
[^>]*> ee15 4f66 	viwdup.u16	q2, r4, r7, #1
[^>]*> ee15 5f67 	vdwdup.u16	q2, r4, r7, #2
[^>]*> ee15 4f67 	viwdup.u16	q2, r4, r7, #2
[^>]*> ee15 5fe6 	vdwdup.u16	q2, r4, r7, #4
[^>]*> ee15 4fe6 	viwdup.u16	q2, r4, r7, #4
[^>]*> ee15 5fe7 	vdwdup.u16	q2, r4, r7, #8
[^>]*> ee15 4fe7 	viwdup.u16	q2, r4, r7, #8
[^>]*> ee15 5f68 	vdwdup.u16	q2, r4, r9, #1
[^>]*> ee15 4f68 	viwdup.u16	q2, r4, r9, #1
[^>]*> ee15 5f69 	vdwdup.u16	q2, r4, r9, #2
[^>]*> ee15 4f69 	viwdup.u16	q2, r4, r9, #2
[^>]*> ee15 5fe8 	vdwdup.u16	q2, r4, r9, #4
[^>]*> ee15 4fe8 	viwdup.u16	q2, r4, r9, #4
[^>]*> ee15 5fe9 	vdwdup.u16	q2, r4, r9, #8
[^>]*> ee15 4fe9 	viwdup.u16	q2, r4, r9, #8
[^>]*> ee15 5f6a 	vdwdup.u16	q2, r4, fp, #1
[^>]*> ee15 4f6a 	viwdup.u16	q2, r4, fp, #1
[^>]*> ee15 5f6b 	vdwdup.u16	q2, r4, fp, #2
[^>]*> ee15 4f6b 	viwdup.u16	q2, r4, fp, #2
[^>]*> ee15 5fea 	vdwdup.u16	q2, r4, fp, #4
[^>]*> ee15 4fea 	viwdup.u16	q2, r4, fp, #4
[^>]*> ee15 5feb 	vdwdup.u16	q2, r4, fp, #8
[^>]*> ee15 4feb 	viwdup.u16	q2, r4, fp, #8
[^>]*> ee17 5f6e 	vddup.u16	q2, r6, #1
[^>]*> ee17 4f6e 	vidup.u16	q2, r6, #1
[^>]*> ee17 5f6f 	vddup.u16	q2, r6, #2
[^>]*> ee17 4f6f 	vidup.u16	q2, r6, #2
[^>]*> ee17 5fee 	vddup.u16	q2, r6, #4
[^>]*> ee17 4fee 	vidup.u16	q2, r6, #4
[^>]*> ee17 5fef 	vddup.u16	q2, r6, #8
[^>]*> ee17 4fef 	vidup.u16	q2, r6, #8
[^>]*> ee17 5f60 	vdwdup.u16	q2, r6, r1, #1
[^>]*> ee17 4f60 	viwdup.u16	q2, r6, r1, #1
[^>]*> ee17 5f61 	vdwdup.u16	q2, r6, r1, #2
[^>]*> ee17 4f61 	viwdup.u16	q2, r6, r1, #2
[^>]*> ee17 5fe0 	vdwdup.u16	q2, r6, r1, #4
[^>]*> ee17 4fe0 	viwdup.u16	q2, r6, r1, #4
[^>]*> ee17 5fe1 	vdwdup.u16	q2, r6, r1, #8
[^>]*> ee17 4fe1 	viwdup.u16	q2, r6, r1, #8
[^>]*> ee17 5f62 	vdwdup.u16	q2, r6, r3, #1
[^>]*> ee17 4f62 	viwdup.u16	q2, r6, r3, #1
[^>]*> ee17 5f63 	vdwdup.u16	q2, r6, r3, #2
[^>]*> ee17 4f63 	viwdup.u16	q2, r6, r3, #2
[^>]*> ee17 5fe2 	vdwdup.u16	q2, r6, r3, #4
[^>]*> ee17 4fe2 	viwdup.u16	q2, r6, r3, #4
[^>]*> ee17 5fe3 	vdwdup.u16	q2, r6, r3, #8
[^>]*> ee17 4fe3 	viwdup.u16	q2, r6, r3, #8
[^>]*> ee17 5f64 	vdwdup.u16	q2, r6, r5, #1
[^>]*> ee17 4f64 	viwdup.u16	q2, r6, r5, #1
[^>]*> ee17 5f65 	vdwdup.u16	q2, r6, r5, #2
[^>]*> ee17 4f65 	viwdup.u16	q2, r6, r5, #2
[^>]*> ee17 5fe4 	vdwdup.u16	q2, r6, r5, #4
[^>]*> ee17 4fe4 	viwdup.u16	q2, r6, r5, #4
[^>]*> ee17 5fe5 	vdwdup.u16	q2, r6, r5, #8
[^>]*> ee17 4fe5 	viwdup.u16	q2, r6, r5, #8
[^>]*> ee17 5f66 	vdwdup.u16	q2, r6, r7, #1
[^>]*> ee17 4f66 	viwdup.u16	q2, r6, r7, #1
[^>]*> ee17 5f67 	vdwdup.u16	q2, r6, r7, #2
[^>]*> ee17 4f67 	viwdup.u16	q2, r6, r7, #2
[^>]*> ee17 5fe6 	vdwdup.u16	q2, r6, r7, #4
[^>]*> ee17 4fe6 	viwdup.u16	q2, r6, r7, #4
[^>]*> ee17 5fe7 	vdwdup.u16	q2, r6, r7, #8
[^>]*> ee17 4fe7 	viwdup.u16	q2, r6, r7, #8
[^>]*> ee17 5f68 	vdwdup.u16	q2, r6, r9, #1
[^>]*> ee17 4f68 	viwdup.u16	q2, r6, r9, #1
[^>]*> ee17 5f69 	vdwdup.u16	q2, r6, r9, #2
[^>]*> ee17 4f69 	viwdup.u16	q2, r6, r9, #2
[^>]*> ee17 5fe8 	vdwdup.u16	q2, r6, r9, #4
[^>]*> ee17 4fe8 	viwdup.u16	q2, r6, r9, #4
[^>]*> ee17 5fe9 	vdwdup.u16	q2, r6, r9, #8
[^>]*> ee17 4fe9 	viwdup.u16	q2, r6, r9, #8
[^>]*> ee17 5f6a 	vdwdup.u16	q2, r6, fp, #1
[^>]*> ee17 4f6a 	viwdup.u16	q2, r6, fp, #1
[^>]*> ee17 5f6b 	vdwdup.u16	q2, r6, fp, #2
[^>]*> ee17 4f6b 	viwdup.u16	q2, r6, fp, #2
[^>]*> ee17 5fea 	vdwdup.u16	q2, r6, fp, #4
[^>]*> ee17 4fea 	viwdup.u16	q2, r6, fp, #4
[^>]*> ee17 5feb 	vdwdup.u16	q2, r6, fp, #8
[^>]*> ee17 4feb 	viwdup.u16	q2, r6, fp, #8
[^>]*> ee19 5f6e 	vddup.u16	q2, r8, #1
[^>]*> ee19 4f6e 	vidup.u16	q2, r8, #1
[^>]*> ee19 5f6f 	vddup.u16	q2, r8, #2
[^>]*> ee19 4f6f 	vidup.u16	q2, r8, #2
[^>]*> ee19 5fee 	vddup.u16	q2, r8, #4
[^>]*> ee19 4fee 	vidup.u16	q2, r8, #4
[^>]*> ee19 5fef 	vddup.u16	q2, r8, #8
[^>]*> ee19 4fef 	vidup.u16	q2, r8, #8
[^>]*> ee19 5f60 	vdwdup.u16	q2, r8, r1, #1
[^>]*> ee19 4f60 	viwdup.u16	q2, r8, r1, #1
[^>]*> ee19 5f61 	vdwdup.u16	q2, r8, r1, #2
[^>]*> ee19 4f61 	viwdup.u16	q2, r8, r1, #2
[^>]*> ee19 5fe0 	vdwdup.u16	q2, r8, r1, #4
[^>]*> ee19 4fe0 	viwdup.u16	q2, r8, r1, #4
[^>]*> ee19 5fe1 	vdwdup.u16	q2, r8, r1, #8
[^>]*> ee19 4fe1 	viwdup.u16	q2, r8, r1, #8
[^>]*> ee19 5f62 	vdwdup.u16	q2, r8, r3, #1
[^>]*> ee19 4f62 	viwdup.u16	q2, r8, r3, #1
[^>]*> ee19 5f63 	vdwdup.u16	q2, r8, r3, #2
[^>]*> ee19 4f63 	viwdup.u16	q2, r8, r3, #2
[^>]*> ee19 5fe2 	vdwdup.u16	q2, r8, r3, #4
[^>]*> ee19 4fe2 	viwdup.u16	q2, r8, r3, #4
[^>]*> ee19 5fe3 	vdwdup.u16	q2, r8, r3, #8
[^>]*> ee19 4fe3 	viwdup.u16	q2, r8, r3, #8
[^>]*> ee19 5f64 	vdwdup.u16	q2, r8, r5, #1
[^>]*> ee19 4f64 	viwdup.u16	q2, r8, r5, #1
[^>]*> ee19 5f65 	vdwdup.u16	q2, r8, r5, #2
[^>]*> ee19 4f65 	viwdup.u16	q2, r8, r5, #2
[^>]*> ee19 5fe4 	vdwdup.u16	q2, r8, r5, #4
[^>]*> ee19 4fe4 	viwdup.u16	q2, r8, r5, #4
[^>]*> ee19 5fe5 	vdwdup.u16	q2, r8, r5, #8
[^>]*> ee19 4fe5 	viwdup.u16	q2, r8, r5, #8
[^>]*> ee19 5f66 	vdwdup.u16	q2, r8, r7, #1
[^>]*> ee19 4f66 	viwdup.u16	q2, r8, r7, #1
[^>]*> ee19 5f67 	vdwdup.u16	q2, r8, r7, #2
[^>]*> ee19 4f67 	viwdup.u16	q2, r8, r7, #2
[^>]*> ee19 5fe6 	vdwdup.u16	q2, r8, r7, #4
[^>]*> ee19 4fe6 	viwdup.u16	q2, r8, r7, #4
[^>]*> ee19 5fe7 	vdwdup.u16	q2, r8, r7, #8
[^>]*> ee19 4fe7 	viwdup.u16	q2, r8, r7, #8
[^>]*> ee19 5f68 	vdwdup.u16	q2, r8, r9, #1
[^>]*> ee19 4f68 	viwdup.u16	q2, r8, r9, #1
[^>]*> ee19 5f69 	vdwdup.u16	q2, r8, r9, #2
[^>]*> ee19 4f69 	viwdup.u16	q2, r8, r9, #2
[^>]*> ee19 5fe8 	vdwdup.u16	q2, r8, r9, #4
[^>]*> ee19 4fe8 	viwdup.u16	q2, r8, r9, #4
[^>]*> ee19 5fe9 	vdwdup.u16	q2, r8, r9, #8
[^>]*> ee19 4fe9 	viwdup.u16	q2, r8, r9, #8
[^>]*> ee19 5f6a 	vdwdup.u16	q2, r8, fp, #1
[^>]*> ee19 4f6a 	viwdup.u16	q2, r8, fp, #1
[^>]*> ee19 5f6b 	vdwdup.u16	q2, r8, fp, #2
[^>]*> ee19 4f6b 	viwdup.u16	q2, r8, fp, #2
[^>]*> ee19 5fea 	vdwdup.u16	q2, r8, fp, #4
[^>]*> ee19 4fea 	viwdup.u16	q2, r8, fp, #4
[^>]*> ee19 5feb 	vdwdup.u16	q2, r8, fp, #8
[^>]*> ee19 4feb 	viwdup.u16	q2, r8, fp, #8
[^>]*> ee1b 5f6e 	vddup.u16	q2, sl, #1
[^>]*> ee1b 4f6e 	vidup.u16	q2, sl, #1
[^>]*> ee1b 5f6f 	vddup.u16	q2, sl, #2
[^>]*> ee1b 4f6f 	vidup.u16	q2, sl, #2
[^>]*> ee1b 5fee 	vddup.u16	q2, sl, #4
[^>]*> ee1b 4fee 	vidup.u16	q2, sl, #4
[^>]*> ee1b 5fef 	vddup.u16	q2, sl, #8
[^>]*> ee1b 4fef 	vidup.u16	q2, sl, #8
[^>]*> ee1b 5f60 	vdwdup.u16	q2, sl, r1, #1
[^>]*> ee1b 4f60 	viwdup.u16	q2, sl, r1, #1
[^>]*> ee1b 5f61 	vdwdup.u16	q2, sl, r1, #2
[^>]*> ee1b 4f61 	viwdup.u16	q2, sl, r1, #2
[^>]*> ee1b 5fe0 	vdwdup.u16	q2, sl, r1, #4
[^>]*> ee1b 4fe0 	viwdup.u16	q2, sl, r1, #4
[^>]*> ee1b 5fe1 	vdwdup.u16	q2, sl, r1, #8
[^>]*> ee1b 4fe1 	viwdup.u16	q2, sl, r1, #8
[^>]*> ee1b 5f62 	vdwdup.u16	q2, sl, r3, #1
[^>]*> ee1b 4f62 	viwdup.u16	q2, sl, r3, #1
[^>]*> ee1b 5f63 	vdwdup.u16	q2, sl, r3, #2
[^>]*> ee1b 4f63 	viwdup.u16	q2, sl, r3, #2
[^>]*> ee1b 5fe2 	vdwdup.u16	q2, sl, r3, #4
[^>]*> ee1b 4fe2 	viwdup.u16	q2, sl, r3, #4
[^>]*> ee1b 5fe3 	vdwdup.u16	q2, sl, r3, #8
[^>]*> ee1b 4fe3 	viwdup.u16	q2, sl, r3, #8
[^>]*> ee1b 5f64 	vdwdup.u16	q2, sl, r5, #1
[^>]*> ee1b 4f64 	viwdup.u16	q2, sl, r5, #1
[^>]*> ee1b 5f65 	vdwdup.u16	q2, sl, r5, #2
[^>]*> ee1b 4f65 	viwdup.u16	q2, sl, r5, #2
[^>]*> ee1b 5fe4 	vdwdup.u16	q2, sl, r5, #4
[^>]*> ee1b 4fe4 	viwdup.u16	q2, sl, r5, #4
[^>]*> ee1b 5fe5 	vdwdup.u16	q2, sl, r5, #8
[^>]*> ee1b 4fe5 	viwdup.u16	q2, sl, r5, #8
[^>]*> ee1b 5f66 	vdwdup.u16	q2, sl, r7, #1
[^>]*> ee1b 4f66 	viwdup.u16	q2, sl, r7, #1
[^>]*> ee1b 5f67 	vdwdup.u16	q2, sl, r7, #2
[^>]*> ee1b 4f67 	viwdup.u16	q2, sl, r7, #2
[^>]*> ee1b 5fe6 	vdwdup.u16	q2, sl, r7, #4
[^>]*> ee1b 4fe6 	viwdup.u16	q2, sl, r7, #4
[^>]*> ee1b 5fe7 	vdwdup.u16	q2, sl, r7, #8
[^>]*> ee1b 4fe7 	viwdup.u16	q2, sl, r7, #8
[^>]*> ee1b 5f68 	vdwdup.u16	q2, sl, r9, #1
[^>]*> ee1b 4f68 	viwdup.u16	q2, sl, r9, #1
[^>]*> ee1b 5f69 	vdwdup.u16	q2, sl, r9, #2
[^>]*> ee1b 4f69 	viwdup.u16	q2, sl, r9, #2
[^>]*> ee1b 5fe8 	vdwdup.u16	q2, sl, r9, #4
[^>]*> ee1b 4fe8 	viwdup.u16	q2, sl, r9, #4
[^>]*> ee1b 5fe9 	vdwdup.u16	q2, sl, r9, #8
[^>]*> ee1b 4fe9 	viwdup.u16	q2, sl, r9, #8
[^>]*> ee1b 5f6a 	vdwdup.u16	q2, sl, fp, #1
[^>]*> ee1b 4f6a 	viwdup.u16	q2, sl, fp, #1
[^>]*> ee1b 5f6b 	vdwdup.u16	q2, sl, fp, #2
[^>]*> ee1b 4f6b 	viwdup.u16	q2, sl, fp, #2
[^>]*> ee1b 5fea 	vdwdup.u16	q2, sl, fp, #4
[^>]*> ee1b 4fea 	viwdup.u16	q2, sl, fp, #4
[^>]*> ee1b 5feb 	vdwdup.u16	q2, sl, fp, #8
[^>]*> ee1b 4feb 	viwdup.u16	q2, sl, fp, #8
[^>]*> ee1d 5f6e 	vddup.u16	q2, ip, #1
[^>]*> ee1d 4f6e 	vidup.u16	q2, ip, #1
[^>]*> ee1d 5f6f 	vddup.u16	q2, ip, #2
[^>]*> ee1d 4f6f 	vidup.u16	q2, ip, #2
[^>]*> ee1d 5fee 	vddup.u16	q2, ip, #4
[^>]*> ee1d 4fee 	vidup.u16	q2, ip, #4
[^>]*> ee1d 5fef 	vddup.u16	q2, ip, #8
[^>]*> ee1d 4fef 	vidup.u16	q2, ip, #8
[^>]*> ee1d 5f60 	vdwdup.u16	q2, ip, r1, #1
[^>]*> ee1d 4f60 	viwdup.u16	q2, ip, r1, #1
[^>]*> ee1d 5f61 	vdwdup.u16	q2, ip, r1, #2
[^>]*> ee1d 4f61 	viwdup.u16	q2, ip, r1, #2
[^>]*> ee1d 5fe0 	vdwdup.u16	q2, ip, r1, #4
[^>]*> ee1d 4fe0 	viwdup.u16	q2, ip, r1, #4
[^>]*> ee1d 5fe1 	vdwdup.u16	q2, ip, r1, #8
[^>]*> ee1d 4fe1 	viwdup.u16	q2, ip, r1, #8
[^>]*> ee1d 5f62 	vdwdup.u16	q2, ip, r3, #1
[^>]*> ee1d 4f62 	viwdup.u16	q2, ip, r3, #1
[^>]*> ee1d 5f63 	vdwdup.u16	q2, ip, r3, #2
[^>]*> ee1d 4f63 	viwdup.u16	q2, ip, r3, #2
[^>]*> ee1d 5fe2 	vdwdup.u16	q2, ip, r3, #4
[^>]*> ee1d 4fe2 	viwdup.u16	q2, ip, r3, #4
[^>]*> ee1d 5fe3 	vdwdup.u16	q2, ip, r3, #8
[^>]*> ee1d 4fe3 	viwdup.u16	q2, ip, r3, #8
[^>]*> ee1d 5f64 	vdwdup.u16	q2, ip, r5, #1
[^>]*> ee1d 4f64 	viwdup.u16	q2, ip, r5, #1
[^>]*> ee1d 5f65 	vdwdup.u16	q2, ip, r5, #2
[^>]*> ee1d 4f65 	viwdup.u16	q2, ip, r5, #2
[^>]*> ee1d 5fe4 	vdwdup.u16	q2, ip, r5, #4
[^>]*> ee1d 4fe4 	viwdup.u16	q2, ip, r5, #4
[^>]*> ee1d 5fe5 	vdwdup.u16	q2, ip, r5, #8
[^>]*> ee1d 4fe5 	viwdup.u16	q2, ip, r5, #8
[^>]*> ee1d 5f66 	vdwdup.u16	q2, ip, r7, #1
[^>]*> ee1d 4f66 	viwdup.u16	q2, ip, r7, #1
[^>]*> ee1d 5f67 	vdwdup.u16	q2, ip, r7, #2
[^>]*> ee1d 4f67 	viwdup.u16	q2, ip, r7, #2
[^>]*> ee1d 5fe6 	vdwdup.u16	q2, ip, r7, #4
[^>]*> ee1d 4fe6 	viwdup.u16	q2, ip, r7, #4
[^>]*> ee1d 5fe7 	vdwdup.u16	q2, ip, r7, #8
[^>]*> ee1d 4fe7 	viwdup.u16	q2, ip, r7, #8
[^>]*> ee1d 5f68 	vdwdup.u16	q2, ip, r9, #1
[^>]*> ee1d 4f68 	viwdup.u16	q2, ip, r9, #1
[^>]*> ee1d 5f69 	vdwdup.u16	q2, ip, r9, #2
[^>]*> ee1d 4f69 	viwdup.u16	q2, ip, r9, #2
[^>]*> ee1d 5fe8 	vdwdup.u16	q2, ip, r9, #4
[^>]*> ee1d 4fe8 	viwdup.u16	q2, ip, r9, #4
[^>]*> ee1d 5fe9 	vdwdup.u16	q2, ip, r9, #8
[^>]*> ee1d 4fe9 	viwdup.u16	q2, ip, r9, #8
[^>]*> ee1d 5f6a 	vdwdup.u16	q2, ip, fp, #1
[^>]*> ee1d 4f6a 	viwdup.u16	q2, ip, fp, #1
[^>]*> ee1d 5f6b 	vdwdup.u16	q2, ip, fp, #2
[^>]*> ee1d 4f6b 	viwdup.u16	q2, ip, fp, #2
[^>]*> ee1d 5fea 	vdwdup.u16	q2, ip, fp, #4
[^>]*> ee1d 4fea 	viwdup.u16	q2, ip, fp, #4
[^>]*> ee1d 5feb 	vdwdup.u16	q2, ip, fp, #8
[^>]*> ee1d 4feb 	viwdup.u16	q2, ip, fp, #8
[^>]*> ee11 9f6e 	vddup.u16	q4, r0, #1
[^>]*> ee11 8f6e 	vidup.u16	q4, r0, #1
[^>]*> ee11 9f6f 	vddup.u16	q4, r0, #2
[^>]*> ee11 8f6f 	vidup.u16	q4, r0, #2
[^>]*> ee11 9fee 	vddup.u16	q4, r0, #4
[^>]*> ee11 8fee 	vidup.u16	q4, r0, #4
[^>]*> ee11 9fef 	vddup.u16	q4, r0, #8
[^>]*> ee11 8fef 	vidup.u16	q4, r0, #8
[^>]*> ee11 9f60 	vdwdup.u16	q4, r0, r1, #1
[^>]*> ee11 8f60 	viwdup.u16	q4, r0, r1, #1
[^>]*> ee11 9f61 	vdwdup.u16	q4, r0, r1, #2
[^>]*> ee11 8f61 	viwdup.u16	q4, r0, r1, #2
[^>]*> ee11 9fe0 	vdwdup.u16	q4, r0, r1, #4
[^>]*> ee11 8fe0 	viwdup.u16	q4, r0, r1, #4
[^>]*> ee11 9fe1 	vdwdup.u16	q4, r0, r1, #8
[^>]*> ee11 8fe1 	viwdup.u16	q4, r0, r1, #8
[^>]*> ee11 9f62 	vdwdup.u16	q4, r0, r3, #1
[^>]*> ee11 8f62 	viwdup.u16	q4, r0, r3, #1
[^>]*> ee11 9f63 	vdwdup.u16	q4, r0, r3, #2
[^>]*> ee11 8f63 	viwdup.u16	q4, r0, r3, #2
[^>]*> ee11 9fe2 	vdwdup.u16	q4, r0, r3, #4
[^>]*> ee11 8fe2 	viwdup.u16	q4, r0, r3, #4
[^>]*> ee11 9fe3 	vdwdup.u16	q4, r0, r3, #8
[^>]*> ee11 8fe3 	viwdup.u16	q4, r0, r3, #8
[^>]*> ee11 9f64 	vdwdup.u16	q4, r0, r5, #1
[^>]*> ee11 8f64 	viwdup.u16	q4, r0, r5, #1
[^>]*> ee11 9f65 	vdwdup.u16	q4, r0, r5, #2
[^>]*> ee11 8f65 	viwdup.u16	q4, r0, r5, #2
[^>]*> ee11 9fe4 	vdwdup.u16	q4, r0, r5, #4
[^>]*> ee11 8fe4 	viwdup.u16	q4, r0, r5, #4
[^>]*> ee11 9fe5 	vdwdup.u16	q4, r0, r5, #8
[^>]*> ee11 8fe5 	viwdup.u16	q4, r0, r5, #8
[^>]*> ee11 9f66 	vdwdup.u16	q4, r0, r7, #1
[^>]*> ee11 8f66 	viwdup.u16	q4, r0, r7, #1
[^>]*> ee11 9f67 	vdwdup.u16	q4, r0, r7, #2
[^>]*> ee11 8f67 	viwdup.u16	q4, r0, r7, #2
[^>]*> ee11 9fe6 	vdwdup.u16	q4, r0, r7, #4
[^>]*> ee11 8fe6 	viwdup.u16	q4, r0, r7, #4
[^>]*> ee11 9fe7 	vdwdup.u16	q4, r0, r7, #8
[^>]*> ee11 8fe7 	viwdup.u16	q4, r0, r7, #8
[^>]*> ee11 9f68 	vdwdup.u16	q4, r0, r9, #1
[^>]*> ee11 8f68 	viwdup.u16	q4, r0, r9, #1
[^>]*> ee11 9f69 	vdwdup.u16	q4, r0, r9, #2
[^>]*> ee11 8f69 	viwdup.u16	q4, r0, r9, #2
[^>]*> ee11 9fe8 	vdwdup.u16	q4, r0, r9, #4
[^>]*> ee11 8fe8 	viwdup.u16	q4, r0, r9, #4
[^>]*> ee11 9fe9 	vdwdup.u16	q4, r0, r9, #8
[^>]*> ee11 8fe9 	viwdup.u16	q4, r0, r9, #8
[^>]*> ee11 9f6a 	vdwdup.u16	q4, r0, fp, #1
[^>]*> ee11 8f6a 	viwdup.u16	q4, r0, fp, #1
[^>]*> ee11 9f6b 	vdwdup.u16	q4, r0, fp, #2
[^>]*> ee11 8f6b 	viwdup.u16	q4, r0, fp, #2
[^>]*> ee11 9fea 	vdwdup.u16	q4, r0, fp, #4
[^>]*> ee11 8fea 	viwdup.u16	q4, r0, fp, #4
[^>]*> ee11 9feb 	vdwdup.u16	q4, r0, fp, #8
[^>]*> ee11 8feb 	viwdup.u16	q4, r0, fp, #8
[^>]*> ee13 9f6e 	vddup.u16	q4, r2, #1
[^>]*> ee13 8f6e 	vidup.u16	q4, r2, #1
[^>]*> ee13 9f6f 	vddup.u16	q4, r2, #2
[^>]*> ee13 8f6f 	vidup.u16	q4, r2, #2
[^>]*> ee13 9fee 	vddup.u16	q4, r2, #4
[^>]*> ee13 8fee 	vidup.u16	q4, r2, #4
[^>]*> ee13 9fef 	vddup.u16	q4, r2, #8
[^>]*> ee13 8fef 	vidup.u16	q4, r2, #8
[^>]*> ee13 9f60 	vdwdup.u16	q4, r2, r1, #1
[^>]*> ee13 8f60 	viwdup.u16	q4, r2, r1, #1
[^>]*> ee13 9f61 	vdwdup.u16	q4, r2, r1, #2
[^>]*> ee13 8f61 	viwdup.u16	q4, r2, r1, #2
[^>]*> ee13 9fe0 	vdwdup.u16	q4, r2, r1, #4
[^>]*> ee13 8fe0 	viwdup.u16	q4, r2, r1, #4
[^>]*> ee13 9fe1 	vdwdup.u16	q4, r2, r1, #8
[^>]*> ee13 8fe1 	viwdup.u16	q4, r2, r1, #8
[^>]*> ee13 9f62 	vdwdup.u16	q4, r2, r3, #1
[^>]*> ee13 8f62 	viwdup.u16	q4, r2, r3, #1
[^>]*> ee13 9f63 	vdwdup.u16	q4, r2, r3, #2
[^>]*> ee13 8f63 	viwdup.u16	q4, r2, r3, #2
[^>]*> ee13 9fe2 	vdwdup.u16	q4, r2, r3, #4
[^>]*> ee13 8fe2 	viwdup.u16	q4, r2, r3, #4
[^>]*> ee13 9fe3 	vdwdup.u16	q4, r2, r3, #8
[^>]*> ee13 8fe3 	viwdup.u16	q4, r2, r3, #8
[^>]*> ee13 9f64 	vdwdup.u16	q4, r2, r5, #1
[^>]*> ee13 8f64 	viwdup.u16	q4, r2, r5, #1
[^>]*> ee13 9f65 	vdwdup.u16	q4, r2, r5, #2
[^>]*> ee13 8f65 	viwdup.u16	q4, r2, r5, #2
[^>]*> ee13 9fe4 	vdwdup.u16	q4, r2, r5, #4
[^>]*> ee13 8fe4 	viwdup.u16	q4, r2, r5, #4
[^>]*> ee13 9fe5 	vdwdup.u16	q4, r2, r5, #8
[^>]*> ee13 8fe5 	viwdup.u16	q4, r2, r5, #8
[^>]*> ee13 9f66 	vdwdup.u16	q4, r2, r7, #1
[^>]*> ee13 8f66 	viwdup.u16	q4, r2, r7, #1
[^>]*> ee13 9f67 	vdwdup.u16	q4, r2, r7, #2
[^>]*> ee13 8f67 	viwdup.u16	q4, r2, r7, #2
[^>]*> ee13 9fe6 	vdwdup.u16	q4, r2, r7, #4
[^>]*> ee13 8fe6 	viwdup.u16	q4, r2, r7, #4
[^>]*> ee13 9fe7 	vdwdup.u16	q4, r2, r7, #8
[^>]*> ee13 8fe7 	viwdup.u16	q4, r2, r7, #8
[^>]*> ee13 9f68 	vdwdup.u16	q4, r2, r9, #1
[^>]*> ee13 8f68 	viwdup.u16	q4, r2, r9, #1
[^>]*> ee13 9f69 	vdwdup.u16	q4, r2, r9, #2
[^>]*> ee13 8f69 	viwdup.u16	q4, r2, r9, #2
[^>]*> ee13 9fe8 	vdwdup.u16	q4, r2, r9, #4
[^>]*> ee13 8fe8 	viwdup.u16	q4, r2, r9, #4
[^>]*> ee13 9fe9 	vdwdup.u16	q4, r2, r9, #8
[^>]*> ee13 8fe9 	viwdup.u16	q4, r2, r9, #8
[^>]*> ee13 9f6a 	vdwdup.u16	q4, r2, fp, #1
[^>]*> ee13 8f6a 	viwdup.u16	q4, r2, fp, #1
[^>]*> ee13 9f6b 	vdwdup.u16	q4, r2, fp, #2
[^>]*> ee13 8f6b 	viwdup.u16	q4, r2, fp, #2
[^>]*> ee13 9fea 	vdwdup.u16	q4, r2, fp, #4
[^>]*> ee13 8fea 	viwdup.u16	q4, r2, fp, #4
[^>]*> ee13 9feb 	vdwdup.u16	q4, r2, fp, #8
[^>]*> ee13 8feb 	viwdup.u16	q4, r2, fp, #8
[^>]*> ee15 9f6e 	vddup.u16	q4, r4, #1
[^>]*> ee15 8f6e 	vidup.u16	q4, r4, #1
[^>]*> ee15 9f6f 	vddup.u16	q4, r4, #2
[^>]*> ee15 8f6f 	vidup.u16	q4, r4, #2
[^>]*> ee15 9fee 	vddup.u16	q4, r4, #4
[^>]*> ee15 8fee 	vidup.u16	q4, r4, #4
[^>]*> ee15 9fef 	vddup.u16	q4, r4, #8
[^>]*> ee15 8fef 	vidup.u16	q4, r4, #8
[^>]*> ee15 9f60 	vdwdup.u16	q4, r4, r1, #1
[^>]*> ee15 8f60 	viwdup.u16	q4, r4, r1, #1
[^>]*> ee15 9f61 	vdwdup.u16	q4, r4, r1, #2
[^>]*> ee15 8f61 	viwdup.u16	q4, r4, r1, #2
[^>]*> ee15 9fe0 	vdwdup.u16	q4, r4, r1, #4
[^>]*> ee15 8fe0 	viwdup.u16	q4, r4, r1, #4
[^>]*> ee15 9fe1 	vdwdup.u16	q4, r4, r1, #8
[^>]*> ee15 8fe1 	viwdup.u16	q4, r4, r1, #8
[^>]*> ee15 9f62 	vdwdup.u16	q4, r4, r3, #1
[^>]*> ee15 8f62 	viwdup.u16	q4, r4, r3, #1
[^>]*> ee15 9f63 	vdwdup.u16	q4, r4, r3, #2
[^>]*> ee15 8f63 	viwdup.u16	q4, r4, r3, #2
[^>]*> ee15 9fe2 	vdwdup.u16	q4, r4, r3, #4
[^>]*> ee15 8fe2 	viwdup.u16	q4, r4, r3, #4
[^>]*> ee15 9fe3 	vdwdup.u16	q4, r4, r3, #8
[^>]*> ee15 8fe3 	viwdup.u16	q4, r4, r3, #8
[^>]*> ee15 9f64 	vdwdup.u16	q4, r4, r5, #1
[^>]*> ee15 8f64 	viwdup.u16	q4, r4, r5, #1
[^>]*> ee15 9f65 	vdwdup.u16	q4, r4, r5, #2
[^>]*> ee15 8f65 	viwdup.u16	q4, r4, r5, #2
[^>]*> ee15 9fe4 	vdwdup.u16	q4, r4, r5, #4
[^>]*> ee15 8fe4 	viwdup.u16	q4, r4, r5, #4
[^>]*> ee15 9fe5 	vdwdup.u16	q4, r4, r5, #8
[^>]*> ee15 8fe5 	viwdup.u16	q4, r4, r5, #8
[^>]*> ee15 9f66 	vdwdup.u16	q4, r4, r7, #1
[^>]*> ee15 8f66 	viwdup.u16	q4, r4, r7, #1
[^>]*> ee15 9f67 	vdwdup.u16	q4, r4, r7, #2
[^>]*> ee15 8f67 	viwdup.u16	q4, r4, r7, #2
[^>]*> ee15 9fe6 	vdwdup.u16	q4, r4, r7, #4
[^>]*> ee15 8fe6 	viwdup.u16	q4, r4, r7, #4
[^>]*> ee15 9fe7 	vdwdup.u16	q4, r4, r7, #8
[^>]*> ee15 8fe7 	viwdup.u16	q4, r4, r7, #8
[^>]*> ee15 9f68 	vdwdup.u16	q4, r4, r9, #1
[^>]*> ee15 8f68 	viwdup.u16	q4, r4, r9, #1
[^>]*> ee15 9f69 	vdwdup.u16	q4, r4, r9, #2
[^>]*> ee15 8f69 	viwdup.u16	q4, r4, r9, #2
[^>]*> ee15 9fe8 	vdwdup.u16	q4, r4, r9, #4
[^>]*> ee15 8fe8 	viwdup.u16	q4, r4, r9, #4
[^>]*> ee15 9fe9 	vdwdup.u16	q4, r4, r9, #8
[^>]*> ee15 8fe9 	viwdup.u16	q4, r4, r9, #8
[^>]*> ee15 9f6a 	vdwdup.u16	q4, r4, fp, #1
[^>]*> ee15 8f6a 	viwdup.u16	q4, r4, fp, #1
[^>]*> ee15 9f6b 	vdwdup.u16	q4, r4, fp, #2
[^>]*> ee15 8f6b 	viwdup.u16	q4, r4, fp, #2
[^>]*> ee15 9fea 	vdwdup.u16	q4, r4, fp, #4
[^>]*> ee15 8fea 	viwdup.u16	q4, r4, fp, #4
[^>]*> ee15 9feb 	vdwdup.u16	q4, r4, fp, #8
[^>]*> ee15 8feb 	viwdup.u16	q4, r4, fp, #8
[^>]*> ee17 9f6e 	vddup.u16	q4, r6, #1
[^>]*> ee17 8f6e 	vidup.u16	q4, r6, #1
[^>]*> ee17 9f6f 	vddup.u16	q4, r6, #2
[^>]*> ee17 8f6f 	vidup.u16	q4, r6, #2
[^>]*> ee17 9fee 	vddup.u16	q4, r6, #4
[^>]*> ee17 8fee 	vidup.u16	q4, r6, #4
[^>]*> ee17 9fef 	vddup.u16	q4, r6, #8
[^>]*> ee17 8fef 	vidup.u16	q4, r6, #8
[^>]*> ee17 9f60 	vdwdup.u16	q4, r6, r1, #1
[^>]*> ee17 8f60 	viwdup.u16	q4, r6, r1, #1
[^>]*> ee17 9f61 	vdwdup.u16	q4, r6, r1, #2
[^>]*> ee17 8f61 	viwdup.u16	q4, r6, r1, #2
[^>]*> ee17 9fe0 	vdwdup.u16	q4, r6, r1, #4
[^>]*> ee17 8fe0 	viwdup.u16	q4, r6, r1, #4
[^>]*> ee17 9fe1 	vdwdup.u16	q4, r6, r1, #8
[^>]*> ee17 8fe1 	viwdup.u16	q4, r6, r1, #8
[^>]*> ee17 9f62 	vdwdup.u16	q4, r6, r3, #1
[^>]*> ee17 8f62 	viwdup.u16	q4, r6, r3, #1
[^>]*> ee17 9f63 	vdwdup.u16	q4, r6, r3, #2
[^>]*> ee17 8f63 	viwdup.u16	q4, r6, r3, #2
[^>]*> ee17 9fe2 	vdwdup.u16	q4, r6, r3, #4
[^>]*> ee17 8fe2 	viwdup.u16	q4, r6, r3, #4
[^>]*> ee17 9fe3 	vdwdup.u16	q4, r6, r3, #8
[^>]*> ee17 8fe3 	viwdup.u16	q4, r6, r3, #8
[^>]*> ee17 9f64 	vdwdup.u16	q4, r6, r5, #1
[^>]*> ee17 8f64 	viwdup.u16	q4, r6, r5, #1
[^>]*> ee17 9f65 	vdwdup.u16	q4, r6, r5, #2
[^>]*> ee17 8f65 	viwdup.u16	q4, r6, r5, #2
[^>]*> ee17 9fe4 	vdwdup.u16	q4, r6, r5, #4
[^>]*> ee17 8fe4 	viwdup.u16	q4, r6, r5, #4
[^>]*> ee17 9fe5 	vdwdup.u16	q4, r6, r5, #8
[^>]*> ee17 8fe5 	viwdup.u16	q4, r6, r5, #8
[^>]*> ee17 9f66 	vdwdup.u16	q4, r6, r7, #1
[^>]*> ee17 8f66 	viwdup.u16	q4, r6, r7, #1
[^>]*> ee17 9f67 	vdwdup.u16	q4, r6, r7, #2
[^>]*> ee17 8f67 	viwdup.u16	q4, r6, r7, #2
[^>]*> ee17 9fe6 	vdwdup.u16	q4, r6, r7, #4
[^>]*> ee17 8fe6 	viwdup.u16	q4, r6, r7, #4
[^>]*> ee17 9fe7 	vdwdup.u16	q4, r6, r7, #8
[^>]*> ee17 8fe7 	viwdup.u16	q4, r6, r7, #8
[^>]*> ee17 9f68 	vdwdup.u16	q4, r6, r9, #1
[^>]*> ee17 8f68 	viwdup.u16	q4, r6, r9, #1
[^>]*> ee17 9f69 	vdwdup.u16	q4, r6, r9, #2
[^>]*> ee17 8f69 	viwdup.u16	q4, r6, r9, #2
[^>]*> ee17 9fe8 	vdwdup.u16	q4, r6, r9, #4
[^>]*> ee17 8fe8 	viwdup.u16	q4, r6, r9, #4
[^>]*> ee17 9fe9 	vdwdup.u16	q4, r6, r9, #8
[^>]*> ee17 8fe9 	viwdup.u16	q4, r6, r9, #8
[^>]*> ee17 9f6a 	vdwdup.u16	q4, r6, fp, #1
[^>]*> ee17 8f6a 	viwdup.u16	q4, r6, fp, #1
[^>]*> ee17 9f6b 	vdwdup.u16	q4, r6, fp, #2
[^>]*> ee17 8f6b 	viwdup.u16	q4, r6, fp, #2
[^>]*> ee17 9fea 	vdwdup.u16	q4, r6, fp, #4
[^>]*> ee17 8fea 	viwdup.u16	q4, r6, fp, #4
[^>]*> ee17 9feb 	vdwdup.u16	q4, r6, fp, #8
[^>]*> ee17 8feb 	viwdup.u16	q4, r6, fp, #8
[^>]*> ee19 9f6e 	vddup.u16	q4, r8, #1
[^>]*> ee19 8f6e 	vidup.u16	q4, r8, #1
[^>]*> ee19 9f6f 	vddup.u16	q4, r8, #2
[^>]*> ee19 8f6f 	vidup.u16	q4, r8, #2
[^>]*> ee19 9fee 	vddup.u16	q4, r8, #4
[^>]*> ee19 8fee 	vidup.u16	q4, r8, #4
[^>]*> ee19 9fef 	vddup.u16	q4, r8, #8
[^>]*> ee19 8fef 	vidup.u16	q4, r8, #8
[^>]*> ee19 9f60 	vdwdup.u16	q4, r8, r1, #1
[^>]*> ee19 8f60 	viwdup.u16	q4, r8, r1, #1
[^>]*> ee19 9f61 	vdwdup.u16	q4, r8, r1, #2
[^>]*> ee19 8f61 	viwdup.u16	q4, r8, r1, #2
[^>]*> ee19 9fe0 	vdwdup.u16	q4, r8, r1, #4
[^>]*> ee19 8fe0 	viwdup.u16	q4, r8, r1, #4
[^>]*> ee19 9fe1 	vdwdup.u16	q4, r8, r1, #8
[^>]*> ee19 8fe1 	viwdup.u16	q4, r8, r1, #8
[^>]*> ee19 9f62 	vdwdup.u16	q4, r8, r3, #1
[^>]*> ee19 8f62 	viwdup.u16	q4, r8, r3, #1
[^>]*> ee19 9f63 	vdwdup.u16	q4, r8, r3, #2
[^>]*> ee19 8f63 	viwdup.u16	q4, r8, r3, #2
[^>]*> ee19 9fe2 	vdwdup.u16	q4, r8, r3, #4
[^>]*> ee19 8fe2 	viwdup.u16	q4, r8, r3, #4
[^>]*> ee19 9fe3 	vdwdup.u16	q4, r8, r3, #8
[^>]*> ee19 8fe3 	viwdup.u16	q4, r8, r3, #8
[^>]*> ee19 9f64 	vdwdup.u16	q4, r8, r5, #1
[^>]*> ee19 8f64 	viwdup.u16	q4, r8, r5, #1
[^>]*> ee19 9f65 	vdwdup.u16	q4, r8, r5, #2
[^>]*> ee19 8f65 	viwdup.u16	q4, r8, r5, #2
[^>]*> ee19 9fe4 	vdwdup.u16	q4, r8, r5, #4
[^>]*> ee19 8fe4 	viwdup.u16	q4, r8, r5, #4
[^>]*> ee19 9fe5 	vdwdup.u16	q4, r8, r5, #8
[^>]*> ee19 8fe5 	viwdup.u16	q4, r8, r5, #8
[^>]*> ee19 9f66 	vdwdup.u16	q4, r8, r7, #1
[^>]*> ee19 8f66 	viwdup.u16	q4, r8, r7, #1
[^>]*> ee19 9f67 	vdwdup.u16	q4, r8, r7, #2
[^>]*> ee19 8f67 	viwdup.u16	q4, r8, r7, #2
[^>]*> ee19 9fe6 	vdwdup.u16	q4, r8, r7, #4
[^>]*> ee19 8fe6 	viwdup.u16	q4, r8, r7, #4
[^>]*> ee19 9fe7 	vdwdup.u16	q4, r8, r7, #8
[^>]*> ee19 8fe7 	viwdup.u16	q4, r8, r7, #8
[^>]*> ee19 9f68 	vdwdup.u16	q4, r8, r9, #1
[^>]*> ee19 8f68 	viwdup.u16	q4, r8, r9, #1
[^>]*> ee19 9f69 	vdwdup.u16	q4, r8, r9, #2
[^>]*> ee19 8f69 	viwdup.u16	q4, r8, r9, #2
[^>]*> ee19 9fe8 	vdwdup.u16	q4, r8, r9, #4
[^>]*> ee19 8fe8 	viwdup.u16	q4, r8, r9, #4
[^>]*> ee19 9fe9 	vdwdup.u16	q4, r8, r9, #8
[^>]*> ee19 8fe9 	viwdup.u16	q4, r8, r9, #8
[^>]*> ee19 9f6a 	vdwdup.u16	q4, r8, fp, #1
[^>]*> ee19 8f6a 	viwdup.u16	q4, r8, fp, #1
[^>]*> ee19 9f6b 	vdwdup.u16	q4, r8, fp, #2
[^>]*> ee19 8f6b 	viwdup.u16	q4, r8, fp, #2
[^>]*> ee19 9fea 	vdwdup.u16	q4, r8, fp, #4
[^>]*> ee19 8fea 	viwdup.u16	q4, r8, fp, #4
[^>]*> ee19 9feb 	vdwdup.u16	q4, r8, fp, #8
[^>]*> ee19 8feb 	viwdup.u16	q4, r8, fp, #8
[^>]*> ee1b 9f6e 	vddup.u16	q4, sl, #1
[^>]*> ee1b 8f6e 	vidup.u16	q4, sl, #1
[^>]*> ee1b 9f6f 	vddup.u16	q4, sl, #2
[^>]*> ee1b 8f6f 	vidup.u16	q4, sl, #2
[^>]*> ee1b 9fee 	vddup.u16	q4, sl, #4
[^>]*> ee1b 8fee 	vidup.u16	q4, sl, #4
[^>]*> ee1b 9fef 	vddup.u16	q4, sl, #8
[^>]*> ee1b 8fef 	vidup.u16	q4, sl, #8
[^>]*> ee1b 9f60 	vdwdup.u16	q4, sl, r1, #1
[^>]*> ee1b 8f60 	viwdup.u16	q4, sl, r1, #1
[^>]*> ee1b 9f61 	vdwdup.u16	q4, sl, r1, #2
[^>]*> ee1b 8f61 	viwdup.u16	q4, sl, r1, #2
[^>]*> ee1b 9fe0 	vdwdup.u16	q4, sl, r1, #4
[^>]*> ee1b 8fe0 	viwdup.u16	q4, sl, r1, #4
[^>]*> ee1b 9fe1 	vdwdup.u16	q4, sl, r1, #8
[^>]*> ee1b 8fe1 	viwdup.u16	q4, sl, r1, #8
[^>]*> ee1b 9f62 	vdwdup.u16	q4, sl, r3, #1
[^>]*> ee1b 8f62 	viwdup.u16	q4, sl, r3, #1
[^>]*> ee1b 9f63 	vdwdup.u16	q4, sl, r3, #2
[^>]*> ee1b 8f63 	viwdup.u16	q4, sl, r3, #2
[^>]*> ee1b 9fe2 	vdwdup.u16	q4, sl, r3, #4
[^>]*> ee1b 8fe2 	viwdup.u16	q4, sl, r3, #4
[^>]*> ee1b 9fe3 	vdwdup.u16	q4, sl, r3, #8
[^>]*> ee1b 8fe3 	viwdup.u16	q4, sl, r3, #8
[^>]*> ee1b 9f64 	vdwdup.u16	q4, sl, r5, #1
[^>]*> ee1b 8f64 	viwdup.u16	q4, sl, r5, #1
[^>]*> ee1b 9f65 	vdwdup.u16	q4, sl, r5, #2
[^>]*> ee1b 8f65 	viwdup.u16	q4, sl, r5, #2
[^>]*> ee1b 9fe4 	vdwdup.u16	q4, sl, r5, #4
[^>]*> ee1b 8fe4 	viwdup.u16	q4, sl, r5, #4
[^>]*> ee1b 9fe5 	vdwdup.u16	q4, sl, r5, #8
[^>]*> ee1b 8fe5 	viwdup.u16	q4, sl, r5, #8
[^>]*> ee1b 9f66 	vdwdup.u16	q4, sl, r7, #1
[^>]*> ee1b 8f66 	viwdup.u16	q4, sl, r7, #1
[^>]*> ee1b 9f67 	vdwdup.u16	q4, sl, r7, #2
[^>]*> ee1b 8f67 	viwdup.u16	q4, sl, r7, #2
[^>]*> ee1b 9fe6 	vdwdup.u16	q4, sl, r7, #4
[^>]*> ee1b 8fe6 	viwdup.u16	q4, sl, r7, #4
[^>]*> ee1b 9fe7 	vdwdup.u16	q4, sl, r7, #8
[^>]*> ee1b 8fe7 	viwdup.u16	q4, sl, r7, #8
[^>]*> ee1b 9f68 	vdwdup.u16	q4, sl, r9, #1
[^>]*> ee1b 8f68 	viwdup.u16	q4, sl, r9, #1
[^>]*> ee1b 9f69 	vdwdup.u16	q4, sl, r9, #2
[^>]*> ee1b 8f69 	viwdup.u16	q4, sl, r9, #2
[^>]*> ee1b 9fe8 	vdwdup.u16	q4, sl, r9, #4
[^>]*> ee1b 8fe8 	viwdup.u16	q4, sl, r9, #4
[^>]*> ee1b 9fe9 	vdwdup.u16	q4, sl, r9, #8
[^>]*> ee1b 8fe9 	viwdup.u16	q4, sl, r9, #8
[^>]*> ee1b 9f6a 	vdwdup.u16	q4, sl, fp, #1
[^>]*> ee1b 8f6a 	viwdup.u16	q4, sl, fp, #1
[^>]*> ee1b 9f6b 	vdwdup.u16	q4, sl, fp, #2
[^>]*> ee1b 8f6b 	viwdup.u16	q4, sl, fp, #2
[^>]*> ee1b 9fea 	vdwdup.u16	q4, sl, fp, #4
[^>]*> ee1b 8fea 	viwdup.u16	q4, sl, fp, #4
[^>]*> ee1b 9feb 	vdwdup.u16	q4, sl, fp, #8
[^>]*> ee1b 8feb 	viwdup.u16	q4, sl, fp, #8
[^>]*> ee1d 9f6e 	vddup.u16	q4, ip, #1
[^>]*> ee1d 8f6e 	vidup.u16	q4, ip, #1
[^>]*> ee1d 9f6f 	vddup.u16	q4, ip, #2
[^>]*> ee1d 8f6f 	vidup.u16	q4, ip, #2
[^>]*> ee1d 9fee 	vddup.u16	q4, ip, #4
[^>]*> ee1d 8fee 	vidup.u16	q4, ip, #4
[^>]*> ee1d 9fef 	vddup.u16	q4, ip, #8
[^>]*> ee1d 8fef 	vidup.u16	q4, ip, #8
[^>]*> ee1d 9f60 	vdwdup.u16	q4, ip, r1, #1
[^>]*> ee1d 8f60 	viwdup.u16	q4, ip, r1, #1
[^>]*> ee1d 9f61 	vdwdup.u16	q4, ip, r1, #2
[^>]*> ee1d 8f61 	viwdup.u16	q4, ip, r1, #2
[^>]*> ee1d 9fe0 	vdwdup.u16	q4, ip, r1, #4
[^>]*> ee1d 8fe0 	viwdup.u16	q4, ip, r1, #4
[^>]*> ee1d 9fe1 	vdwdup.u16	q4, ip, r1, #8
[^>]*> ee1d 8fe1 	viwdup.u16	q4, ip, r1, #8
[^>]*> ee1d 9f62 	vdwdup.u16	q4, ip, r3, #1
[^>]*> ee1d 8f62 	viwdup.u16	q4, ip, r3, #1
[^>]*> ee1d 9f63 	vdwdup.u16	q4, ip, r3, #2
[^>]*> ee1d 8f63 	viwdup.u16	q4, ip, r3, #2
[^>]*> ee1d 9fe2 	vdwdup.u16	q4, ip, r3, #4
[^>]*> ee1d 8fe2 	viwdup.u16	q4, ip, r3, #4
[^>]*> ee1d 9fe3 	vdwdup.u16	q4, ip, r3, #8
[^>]*> ee1d 8fe3 	viwdup.u16	q4, ip, r3, #8
[^>]*> ee1d 9f64 	vdwdup.u16	q4, ip, r5, #1
[^>]*> ee1d 8f64 	viwdup.u16	q4, ip, r5, #1
[^>]*> ee1d 9f65 	vdwdup.u16	q4, ip, r5, #2
[^>]*> ee1d 8f65 	viwdup.u16	q4, ip, r5, #2
[^>]*> ee1d 9fe4 	vdwdup.u16	q4, ip, r5, #4
[^>]*> ee1d 8fe4 	viwdup.u16	q4, ip, r5, #4
[^>]*> ee1d 9fe5 	vdwdup.u16	q4, ip, r5, #8
[^>]*> ee1d 8fe5 	viwdup.u16	q4, ip, r5, #8
[^>]*> ee1d 9f66 	vdwdup.u16	q4, ip, r7, #1
[^>]*> ee1d 8f66 	viwdup.u16	q4, ip, r7, #1
[^>]*> ee1d 9f67 	vdwdup.u16	q4, ip, r7, #2
[^>]*> ee1d 8f67 	viwdup.u16	q4, ip, r7, #2
[^>]*> ee1d 9fe6 	vdwdup.u16	q4, ip, r7, #4
[^>]*> ee1d 8fe6 	viwdup.u16	q4, ip, r7, #4
[^>]*> ee1d 9fe7 	vdwdup.u16	q4, ip, r7, #8
[^>]*> ee1d 8fe7 	viwdup.u16	q4, ip, r7, #8
[^>]*> ee1d 9f68 	vdwdup.u16	q4, ip, r9, #1
[^>]*> ee1d 8f68 	viwdup.u16	q4, ip, r9, #1
[^>]*> ee1d 9f69 	vdwdup.u16	q4, ip, r9, #2
[^>]*> ee1d 8f69 	viwdup.u16	q4, ip, r9, #2
[^>]*> ee1d 9fe8 	vdwdup.u16	q4, ip, r9, #4
[^>]*> ee1d 8fe8 	viwdup.u16	q4, ip, r9, #4
[^>]*> ee1d 9fe9 	vdwdup.u16	q4, ip, r9, #8
[^>]*> ee1d 8fe9 	viwdup.u16	q4, ip, r9, #8
[^>]*> ee1d 9f6a 	vdwdup.u16	q4, ip, fp, #1
[^>]*> ee1d 8f6a 	viwdup.u16	q4, ip, fp, #1
[^>]*> ee1d 9f6b 	vdwdup.u16	q4, ip, fp, #2
[^>]*> ee1d 8f6b 	viwdup.u16	q4, ip, fp, #2
[^>]*> ee1d 9fea 	vdwdup.u16	q4, ip, fp, #4
[^>]*> ee1d 8fea 	viwdup.u16	q4, ip, fp, #4
[^>]*> ee1d 9feb 	vdwdup.u16	q4, ip, fp, #8
[^>]*> ee1d 8feb 	viwdup.u16	q4, ip, fp, #8
[^>]*> ee11 ff6e 	vddup.u16	q7, r0, #1
[^>]*> ee11 ef6e 	vidup.u16	q7, r0, #1
[^>]*> ee11 ff6f 	vddup.u16	q7, r0, #2
[^>]*> ee11 ef6f 	vidup.u16	q7, r0, #2
[^>]*> ee11 ffee 	vddup.u16	q7, r0, #4
[^>]*> ee11 efee 	vidup.u16	q7, r0, #4
[^>]*> ee11 ffef 	vddup.u16	q7, r0, #8
[^>]*> ee11 efef 	vidup.u16	q7, r0, #8
[^>]*> ee11 ff60 	vdwdup.u16	q7, r0, r1, #1
[^>]*> ee11 ef60 	viwdup.u16	q7, r0, r1, #1
[^>]*> ee11 ff61 	vdwdup.u16	q7, r0, r1, #2
[^>]*> ee11 ef61 	viwdup.u16	q7, r0, r1, #2
[^>]*> ee11 ffe0 	vdwdup.u16	q7, r0, r1, #4
[^>]*> ee11 efe0 	viwdup.u16	q7, r0, r1, #4
[^>]*> ee11 ffe1 	vdwdup.u16	q7, r0, r1, #8
[^>]*> ee11 efe1 	viwdup.u16	q7, r0, r1, #8
[^>]*> ee11 ff62 	vdwdup.u16	q7, r0, r3, #1
[^>]*> ee11 ef62 	viwdup.u16	q7, r0, r3, #1
[^>]*> ee11 ff63 	vdwdup.u16	q7, r0, r3, #2
[^>]*> ee11 ef63 	viwdup.u16	q7, r0, r3, #2
[^>]*> ee11 ffe2 	vdwdup.u16	q7, r0, r3, #4
[^>]*> ee11 efe2 	viwdup.u16	q7, r0, r3, #4
[^>]*> ee11 ffe3 	vdwdup.u16	q7, r0, r3, #8
[^>]*> ee11 efe3 	viwdup.u16	q7, r0, r3, #8
[^>]*> ee11 ff64 	vdwdup.u16	q7, r0, r5, #1
[^>]*> ee11 ef64 	viwdup.u16	q7, r0, r5, #1
[^>]*> ee11 ff65 	vdwdup.u16	q7, r0, r5, #2
[^>]*> ee11 ef65 	viwdup.u16	q7, r0, r5, #2
[^>]*> ee11 ffe4 	vdwdup.u16	q7, r0, r5, #4
[^>]*> ee11 efe4 	viwdup.u16	q7, r0, r5, #4
[^>]*> ee11 ffe5 	vdwdup.u16	q7, r0, r5, #8
[^>]*> ee11 efe5 	viwdup.u16	q7, r0, r5, #8
[^>]*> ee11 ff66 	vdwdup.u16	q7, r0, r7, #1
[^>]*> ee11 ef66 	viwdup.u16	q7, r0, r7, #1
[^>]*> ee11 ff67 	vdwdup.u16	q7, r0, r7, #2
[^>]*> ee11 ef67 	viwdup.u16	q7, r0, r7, #2
[^>]*> ee11 ffe6 	vdwdup.u16	q7, r0, r7, #4
[^>]*> ee11 efe6 	viwdup.u16	q7, r0, r7, #4
[^>]*> ee11 ffe7 	vdwdup.u16	q7, r0, r7, #8
[^>]*> ee11 efe7 	viwdup.u16	q7, r0, r7, #8
[^>]*> ee11 ff68 	vdwdup.u16	q7, r0, r9, #1
[^>]*> ee11 ef68 	viwdup.u16	q7, r0, r9, #1
[^>]*> ee11 ff69 	vdwdup.u16	q7, r0, r9, #2
[^>]*> ee11 ef69 	viwdup.u16	q7, r0, r9, #2
[^>]*> ee11 ffe8 	vdwdup.u16	q7, r0, r9, #4
[^>]*> ee11 efe8 	viwdup.u16	q7, r0, r9, #4
[^>]*> ee11 ffe9 	vdwdup.u16	q7, r0, r9, #8
[^>]*> ee11 efe9 	viwdup.u16	q7, r0, r9, #8
[^>]*> ee11 ff6a 	vdwdup.u16	q7, r0, fp, #1
[^>]*> ee11 ef6a 	viwdup.u16	q7, r0, fp, #1
[^>]*> ee11 ff6b 	vdwdup.u16	q7, r0, fp, #2
[^>]*> ee11 ef6b 	viwdup.u16	q7, r0, fp, #2
[^>]*> ee11 ffea 	vdwdup.u16	q7, r0, fp, #4
[^>]*> ee11 efea 	viwdup.u16	q7, r0, fp, #4
[^>]*> ee11 ffeb 	vdwdup.u16	q7, r0, fp, #8
[^>]*> ee11 efeb 	viwdup.u16	q7, r0, fp, #8
[^>]*> ee13 ff6e 	vddup.u16	q7, r2, #1
[^>]*> ee13 ef6e 	vidup.u16	q7, r2, #1
[^>]*> ee13 ff6f 	vddup.u16	q7, r2, #2
[^>]*> ee13 ef6f 	vidup.u16	q7, r2, #2
[^>]*> ee13 ffee 	vddup.u16	q7, r2, #4
[^>]*> ee13 efee 	vidup.u16	q7, r2, #4
[^>]*> ee13 ffef 	vddup.u16	q7, r2, #8
[^>]*> ee13 efef 	vidup.u16	q7, r2, #8
[^>]*> ee13 ff60 	vdwdup.u16	q7, r2, r1, #1
[^>]*> ee13 ef60 	viwdup.u16	q7, r2, r1, #1
[^>]*> ee13 ff61 	vdwdup.u16	q7, r2, r1, #2
[^>]*> ee13 ef61 	viwdup.u16	q7, r2, r1, #2
[^>]*> ee13 ffe0 	vdwdup.u16	q7, r2, r1, #4
[^>]*> ee13 efe0 	viwdup.u16	q7, r2, r1, #4
[^>]*> ee13 ffe1 	vdwdup.u16	q7, r2, r1, #8
[^>]*> ee13 efe1 	viwdup.u16	q7, r2, r1, #8
[^>]*> ee13 ff62 	vdwdup.u16	q7, r2, r3, #1
[^>]*> ee13 ef62 	viwdup.u16	q7, r2, r3, #1
[^>]*> ee13 ff63 	vdwdup.u16	q7, r2, r3, #2
[^>]*> ee13 ef63 	viwdup.u16	q7, r2, r3, #2
[^>]*> ee13 ffe2 	vdwdup.u16	q7, r2, r3, #4
[^>]*> ee13 efe2 	viwdup.u16	q7, r2, r3, #4
[^>]*> ee13 ffe3 	vdwdup.u16	q7, r2, r3, #8
[^>]*> ee13 efe3 	viwdup.u16	q7, r2, r3, #8
[^>]*> ee13 ff64 	vdwdup.u16	q7, r2, r5, #1
[^>]*> ee13 ef64 	viwdup.u16	q7, r2, r5, #1
[^>]*> ee13 ff65 	vdwdup.u16	q7, r2, r5, #2
[^>]*> ee13 ef65 	viwdup.u16	q7, r2, r5, #2
[^>]*> ee13 ffe4 	vdwdup.u16	q7, r2, r5, #4
[^>]*> ee13 efe4 	viwdup.u16	q7, r2, r5, #4
[^>]*> ee13 ffe5 	vdwdup.u16	q7, r2, r5, #8
[^>]*> ee13 efe5 	viwdup.u16	q7, r2, r5, #8
[^>]*> ee13 ff66 	vdwdup.u16	q7, r2, r7, #1
[^>]*> ee13 ef66 	viwdup.u16	q7, r2, r7, #1
[^>]*> ee13 ff67 	vdwdup.u16	q7, r2, r7, #2
[^>]*> ee13 ef67 	viwdup.u16	q7, r2, r7, #2
[^>]*> ee13 ffe6 	vdwdup.u16	q7, r2, r7, #4
[^>]*> ee13 efe6 	viwdup.u16	q7, r2, r7, #4
[^>]*> ee13 ffe7 	vdwdup.u16	q7, r2, r7, #8
[^>]*> ee13 efe7 	viwdup.u16	q7, r2, r7, #8
[^>]*> ee13 ff68 	vdwdup.u16	q7, r2, r9, #1
[^>]*> ee13 ef68 	viwdup.u16	q7, r2, r9, #1
[^>]*> ee13 ff69 	vdwdup.u16	q7, r2, r9, #2
[^>]*> ee13 ef69 	viwdup.u16	q7, r2, r9, #2
[^>]*> ee13 ffe8 	vdwdup.u16	q7, r2, r9, #4
[^>]*> ee13 efe8 	viwdup.u16	q7, r2, r9, #4
[^>]*> ee13 ffe9 	vdwdup.u16	q7, r2, r9, #8
[^>]*> ee13 efe9 	viwdup.u16	q7, r2, r9, #8
[^>]*> ee13 ff6a 	vdwdup.u16	q7, r2, fp, #1
[^>]*> ee13 ef6a 	viwdup.u16	q7, r2, fp, #1
[^>]*> ee13 ff6b 	vdwdup.u16	q7, r2, fp, #2
[^>]*> ee13 ef6b 	viwdup.u16	q7, r2, fp, #2
[^>]*> ee13 ffea 	vdwdup.u16	q7, r2, fp, #4
[^>]*> ee13 efea 	viwdup.u16	q7, r2, fp, #4
[^>]*> ee13 ffeb 	vdwdup.u16	q7, r2, fp, #8
[^>]*> ee13 efeb 	viwdup.u16	q7, r2, fp, #8
[^>]*> ee15 ff6e 	vddup.u16	q7, r4, #1
[^>]*> ee15 ef6e 	vidup.u16	q7, r4, #1
[^>]*> ee15 ff6f 	vddup.u16	q7, r4, #2
[^>]*> ee15 ef6f 	vidup.u16	q7, r4, #2
[^>]*> ee15 ffee 	vddup.u16	q7, r4, #4
[^>]*> ee15 efee 	vidup.u16	q7, r4, #4
[^>]*> ee15 ffef 	vddup.u16	q7, r4, #8
[^>]*> ee15 efef 	vidup.u16	q7, r4, #8
[^>]*> ee15 ff60 	vdwdup.u16	q7, r4, r1, #1
[^>]*> ee15 ef60 	viwdup.u16	q7, r4, r1, #1
[^>]*> ee15 ff61 	vdwdup.u16	q7, r4, r1, #2
[^>]*> ee15 ef61 	viwdup.u16	q7, r4, r1, #2
[^>]*> ee15 ffe0 	vdwdup.u16	q7, r4, r1, #4
[^>]*> ee15 efe0 	viwdup.u16	q7, r4, r1, #4
[^>]*> ee15 ffe1 	vdwdup.u16	q7, r4, r1, #8
[^>]*> ee15 efe1 	viwdup.u16	q7, r4, r1, #8
[^>]*> ee15 ff62 	vdwdup.u16	q7, r4, r3, #1
[^>]*> ee15 ef62 	viwdup.u16	q7, r4, r3, #1
[^>]*> ee15 ff63 	vdwdup.u16	q7, r4, r3, #2
[^>]*> ee15 ef63 	viwdup.u16	q7, r4, r3, #2
[^>]*> ee15 ffe2 	vdwdup.u16	q7, r4, r3, #4
[^>]*> ee15 efe2 	viwdup.u16	q7, r4, r3, #4
[^>]*> ee15 ffe3 	vdwdup.u16	q7, r4, r3, #8
[^>]*> ee15 efe3 	viwdup.u16	q7, r4, r3, #8
[^>]*> ee15 ff64 	vdwdup.u16	q7, r4, r5, #1
[^>]*> ee15 ef64 	viwdup.u16	q7, r4, r5, #1
[^>]*> ee15 ff65 	vdwdup.u16	q7, r4, r5, #2
[^>]*> ee15 ef65 	viwdup.u16	q7, r4, r5, #2
[^>]*> ee15 ffe4 	vdwdup.u16	q7, r4, r5, #4
[^>]*> ee15 efe4 	viwdup.u16	q7, r4, r5, #4
[^>]*> ee15 ffe5 	vdwdup.u16	q7, r4, r5, #8
[^>]*> ee15 efe5 	viwdup.u16	q7, r4, r5, #8
[^>]*> ee15 ff66 	vdwdup.u16	q7, r4, r7, #1
[^>]*> ee15 ef66 	viwdup.u16	q7, r4, r7, #1
[^>]*> ee15 ff67 	vdwdup.u16	q7, r4, r7, #2
[^>]*> ee15 ef67 	viwdup.u16	q7, r4, r7, #2
[^>]*> ee15 ffe6 	vdwdup.u16	q7, r4, r7, #4
[^>]*> ee15 efe6 	viwdup.u16	q7, r4, r7, #4
[^>]*> ee15 ffe7 	vdwdup.u16	q7, r4, r7, #8
[^>]*> ee15 efe7 	viwdup.u16	q7, r4, r7, #8
[^>]*> ee15 ff68 	vdwdup.u16	q7, r4, r9, #1
[^>]*> ee15 ef68 	viwdup.u16	q7, r4, r9, #1
[^>]*> ee15 ff69 	vdwdup.u16	q7, r4, r9, #2
[^>]*> ee15 ef69 	viwdup.u16	q7, r4, r9, #2
[^>]*> ee15 ffe8 	vdwdup.u16	q7, r4, r9, #4
[^>]*> ee15 efe8 	viwdup.u16	q7, r4, r9, #4
[^>]*> ee15 ffe9 	vdwdup.u16	q7, r4, r9, #8
[^>]*> ee15 efe9 	viwdup.u16	q7, r4, r9, #8
[^>]*> ee15 ff6a 	vdwdup.u16	q7, r4, fp, #1
[^>]*> ee15 ef6a 	viwdup.u16	q7, r4, fp, #1
[^>]*> ee15 ff6b 	vdwdup.u16	q7, r4, fp, #2
[^>]*> ee15 ef6b 	viwdup.u16	q7, r4, fp, #2
[^>]*> ee15 ffea 	vdwdup.u16	q7, r4, fp, #4
[^>]*> ee15 efea 	viwdup.u16	q7, r4, fp, #4
[^>]*> ee15 ffeb 	vdwdup.u16	q7, r4, fp, #8
[^>]*> ee15 efeb 	viwdup.u16	q7, r4, fp, #8
[^>]*> ee17 ff6e 	vddup.u16	q7, r6, #1
[^>]*> ee17 ef6e 	vidup.u16	q7, r6, #1
[^>]*> ee17 ff6f 	vddup.u16	q7, r6, #2
[^>]*> ee17 ef6f 	vidup.u16	q7, r6, #2
[^>]*> ee17 ffee 	vddup.u16	q7, r6, #4
[^>]*> ee17 efee 	vidup.u16	q7, r6, #4
[^>]*> ee17 ffef 	vddup.u16	q7, r6, #8
[^>]*> ee17 efef 	vidup.u16	q7, r6, #8
[^>]*> ee17 ff60 	vdwdup.u16	q7, r6, r1, #1
[^>]*> ee17 ef60 	viwdup.u16	q7, r6, r1, #1
[^>]*> ee17 ff61 	vdwdup.u16	q7, r6, r1, #2
[^>]*> ee17 ef61 	viwdup.u16	q7, r6, r1, #2
[^>]*> ee17 ffe0 	vdwdup.u16	q7, r6, r1, #4
[^>]*> ee17 efe0 	viwdup.u16	q7, r6, r1, #4
[^>]*> ee17 ffe1 	vdwdup.u16	q7, r6, r1, #8
[^>]*> ee17 efe1 	viwdup.u16	q7, r6, r1, #8
[^>]*> ee17 ff62 	vdwdup.u16	q7, r6, r3, #1
[^>]*> ee17 ef62 	viwdup.u16	q7, r6, r3, #1
[^>]*> ee17 ff63 	vdwdup.u16	q7, r6, r3, #2
[^>]*> ee17 ef63 	viwdup.u16	q7, r6, r3, #2
[^>]*> ee17 ffe2 	vdwdup.u16	q7, r6, r3, #4
[^>]*> ee17 efe2 	viwdup.u16	q7, r6, r3, #4
[^>]*> ee17 ffe3 	vdwdup.u16	q7, r6, r3, #8
[^>]*> ee17 efe3 	viwdup.u16	q7, r6, r3, #8
[^>]*> ee17 ff64 	vdwdup.u16	q7, r6, r5, #1
[^>]*> ee17 ef64 	viwdup.u16	q7, r6, r5, #1
[^>]*> ee17 ff65 	vdwdup.u16	q7, r6, r5, #2
[^>]*> ee17 ef65 	viwdup.u16	q7, r6, r5, #2
[^>]*> ee17 ffe4 	vdwdup.u16	q7, r6, r5, #4
[^>]*> ee17 efe4 	viwdup.u16	q7, r6, r5, #4
[^>]*> ee17 ffe5 	vdwdup.u16	q7, r6, r5, #8
[^>]*> ee17 efe5 	viwdup.u16	q7, r6, r5, #8
[^>]*> ee17 ff66 	vdwdup.u16	q7, r6, r7, #1
[^>]*> ee17 ef66 	viwdup.u16	q7, r6, r7, #1
[^>]*> ee17 ff67 	vdwdup.u16	q7, r6, r7, #2
[^>]*> ee17 ef67 	viwdup.u16	q7, r6, r7, #2
[^>]*> ee17 ffe6 	vdwdup.u16	q7, r6, r7, #4
[^>]*> ee17 efe6 	viwdup.u16	q7, r6, r7, #4
[^>]*> ee17 ffe7 	vdwdup.u16	q7, r6, r7, #8
[^>]*> ee17 efe7 	viwdup.u16	q7, r6, r7, #8
[^>]*> ee17 ff68 	vdwdup.u16	q7, r6, r9, #1
[^>]*> ee17 ef68 	viwdup.u16	q7, r6, r9, #1
[^>]*> ee17 ff69 	vdwdup.u16	q7, r6, r9, #2
[^>]*> ee17 ef69 	viwdup.u16	q7, r6, r9, #2
[^>]*> ee17 ffe8 	vdwdup.u16	q7, r6, r9, #4
[^>]*> ee17 efe8 	viwdup.u16	q7, r6, r9, #4
[^>]*> ee17 ffe9 	vdwdup.u16	q7, r6, r9, #8
[^>]*> ee17 efe9 	viwdup.u16	q7, r6, r9, #8
[^>]*> ee17 ff6a 	vdwdup.u16	q7, r6, fp, #1
[^>]*> ee17 ef6a 	viwdup.u16	q7, r6, fp, #1
[^>]*> ee17 ff6b 	vdwdup.u16	q7, r6, fp, #2
[^>]*> ee17 ef6b 	viwdup.u16	q7, r6, fp, #2
[^>]*> ee17 ffea 	vdwdup.u16	q7, r6, fp, #4
[^>]*> ee17 efea 	viwdup.u16	q7, r6, fp, #4
[^>]*> ee17 ffeb 	vdwdup.u16	q7, r6, fp, #8
[^>]*> ee17 efeb 	viwdup.u16	q7, r6, fp, #8
[^>]*> ee19 ff6e 	vddup.u16	q7, r8, #1
[^>]*> ee19 ef6e 	vidup.u16	q7, r8, #1
[^>]*> ee19 ff6f 	vddup.u16	q7, r8, #2
[^>]*> ee19 ef6f 	vidup.u16	q7, r8, #2
[^>]*> ee19 ffee 	vddup.u16	q7, r8, #4
[^>]*> ee19 efee 	vidup.u16	q7, r8, #4
[^>]*> ee19 ffef 	vddup.u16	q7, r8, #8
[^>]*> ee19 efef 	vidup.u16	q7, r8, #8
[^>]*> ee19 ff60 	vdwdup.u16	q7, r8, r1, #1
[^>]*> ee19 ef60 	viwdup.u16	q7, r8, r1, #1
[^>]*> ee19 ff61 	vdwdup.u16	q7, r8, r1, #2
[^>]*> ee19 ef61 	viwdup.u16	q7, r8, r1, #2
[^>]*> ee19 ffe0 	vdwdup.u16	q7, r8, r1, #4
[^>]*> ee19 efe0 	viwdup.u16	q7, r8, r1, #4
[^>]*> ee19 ffe1 	vdwdup.u16	q7, r8, r1, #8
[^>]*> ee19 efe1 	viwdup.u16	q7, r8, r1, #8
[^>]*> ee19 ff62 	vdwdup.u16	q7, r8, r3, #1
[^>]*> ee19 ef62 	viwdup.u16	q7, r8, r3, #1
[^>]*> ee19 ff63 	vdwdup.u16	q7, r8, r3, #2
[^>]*> ee19 ef63 	viwdup.u16	q7, r8, r3, #2
[^>]*> ee19 ffe2 	vdwdup.u16	q7, r8, r3, #4
[^>]*> ee19 efe2 	viwdup.u16	q7, r8, r3, #4
[^>]*> ee19 ffe3 	vdwdup.u16	q7, r8, r3, #8
[^>]*> ee19 efe3 	viwdup.u16	q7, r8, r3, #8
[^>]*> ee19 ff64 	vdwdup.u16	q7, r8, r5, #1
[^>]*> ee19 ef64 	viwdup.u16	q7, r8, r5, #1
[^>]*> ee19 ff65 	vdwdup.u16	q7, r8, r5, #2
[^>]*> ee19 ef65 	viwdup.u16	q7, r8, r5, #2
[^>]*> ee19 ffe4 	vdwdup.u16	q7, r8, r5, #4
[^>]*> ee19 efe4 	viwdup.u16	q7, r8, r5, #4
[^>]*> ee19 ffe5 	vdwdup.u16	q7, r8, r5, #8
[^>]*> ee19 efe5 	viwdup.u16	q7, r8, r5, #8
[^>]*> ee19 ff66 	vdwdup.u16	q7, r8, r7, #1
[^>]*> ee19 ef66 	viwdup.u16	q7, r8, r7, #1
[^>]*> ee19 ff67 	vdwdup.u16	q7, r8, r7, #2
[^>]*> ee19 ef67 	viwdup.u16	q7, r8, r7, #2
[^>]*> ee19 ffe6 	vdwdup.u16	q7, r8, r7, #4
[^>]*> ee19 efe6 	viwdup.u16	q7, r8, r7, #4
[^>]*> ee19 ffe7 	vdwdup.u16	q7, r8, r7, #8
[^>]*> ee19 efe7 	viwdup.u16	q7, r8, r7, #8
[^>]*> ee19 ff68 	vdwdup.u16	q7, r8, r9, #1
[^>]*> ee19 ef68 	viwdup.u16	q7, r8, r9, #1
[^>]*> ee19 ff69 	vdwdup.u16	q7, r8, r9, #2
[^>]*> ee19 ef69 	viwdup.u16	q7, r8, r9, #2
[^>]*> ee19 ffe8 	vdwdup.u16	q7, r8, r9, #4
[^>]*> ee19 efe8 	viwdup.u16	q7, r8, r9, #4
[^>]*> ee19 ffe9 	vdwdup.u16	q7, r8, r9, #8
[^>]*> ee19 efe9 	viwdup.u16	q7, r8, r9, #8
[^>]*> ee19 ff6a 	vdwdup.u16	q7, r8, fp, #1
[^>]*> ee19 ef6a 	viwdup.u16	q7, r8, fp, #1
[^>]*> ee19 ff6b 	vdwdup.u16	q7, r8, fp, #2
[^>]*> ee19 ef6b 	viwdup.u16	q7, r8, fp, #2
[^>]*> ee19 ffea 	vdwdup.u16	q7, r8, fp, #4
[^>]*> ee19 efea 	viwdup.u16	q7, r8, fp, #4
[^>]*> ee19 ffeb 	vdwdup.u16	q7, r8, fp, #8
[^>]*> ee19 efeb 	viwdup.u16	q7, r8, fp, #8
[^>]*> ee1b ff6e 	vddup.u16	q7, sl, #1
[^>]*> ee1b ef6e 	vidup.u16	q7, sl, #1
[^>]*> ee1b ff6f 	vddup.u16	q7, sl, #2
[^>]*> ee1b ef6f 	vidup.u16	q7, sl, #2
[^>]*> ee1b ffee 	vddup.u16	q7, sl, #4
[^>]*> ee1b efee 	vidup.u16	q7, sl, #4
[^>]*> ee1b ffef 	vddup.u16	q7, sl, #8
[^>]*> ee1b efef 	vidup.u16	q7, sl, #8
[^>]*> ee1b ff60 	vdwdup.u16	q7, sl, r1, #1
[^>]*> ee1b ef60 	viwdup.u16	q7, sl, r1, #1
[^>]*> ee1b ff61 	vdwdup.u16	q7, sl, r1, #2
[^>]*> ee1b ef61 	viwdup.u16	q7, sl, r1, #2
[^>]*> ee1b ffe0 	vdwdup.u16	q7, sl, r1, #4
[^>]*> ee1b efe0 	viwdup.u16	q7, sl, r1, #4
[^>]*> ee1b ffe1 	vdwdup.u16	q7, sl, r1, #8
[^>]*> ee1b efe1 	viwdup.u16	q7, sl, r1, #8
[^>]*> ee1b ff62 	vdwdup.u16	q7, sl, r3, #1
[^>]*> ee1b ef62 	viwdup.u16	q7, sl, r3, #1
[^>]*> ee1b ff63 	vdwdup.u16	q7, sl, r3, #2
[^>]*> ee1b ef63 	viwdup.u16	q7, sl, r3, #2
[^>]*> ee1b ffe2 	vdwdup.u16	q7, sl, r3, #4
[^>]*> ee1b efe2 	viwdup.u16	q7, sl, r3, #4
[^>]*> ee1b ffe3 	vdwdup.u16	q7, sl, r3, #8
[^>]*> ee1b efe3 	viwdup.u16	q7, sl, r3, #8
[^>]*> ee1b ff64 	vdwdup.u16	q7, sl, r5, #1
[^>]*> ee1b ef64 	viwdup.u16	q7, sl, r5, #1
[^>]*> ee1b ff65 	vdwdup.u16	q7, sl, r5, #2
[^>]*> ee1b ef65 	viwdup.u16	q7, sl, r5, #2
[^>]*> ee1b ffe4 	vdwdup.u16	q7, sl, r5, #4
[^>]*> ee1b efe4 	viwdup.u16	q7, sl, r5, #4
[^>]*> ee1b ffe5 	vdwdup.u16	q7, sl, r5, #8
[^>]*> ee1b efe5 	viwdup.u16	q7, sl, r5, #8
[^>]*> ee1b ff66 	vdwdup.u16	q7, sl, r7, #1
[^>]*> ee1b ef66 	viwdup.u16	q7, sl, r7, #1
[^>]*> ee1b ff67 	vdwdup.u16	q7, sl, r7, #2
[^>]*> ee1b ef67 	viwdup.u16	q7, sl, r7, #2
[^>]*> ee1b ffe6 	vdwdup.u16	q7, sl, r7, #4
[^>]*> ee1b efe6 	viwdup.u16	q7, sl, r7, #4
[^>]*> ee1b ffe7 	vdwdup.u16	q7, sl, r7, #8
[^>]*> ee1b efe7 	viwdup.u16	q7, sl, r7, #8
[^>]*> ee1b ff68 	vdwdup.u16	q7, sl, r9, #1
[^>]*> ee1b ef68 	viwdup.u16	q7, sl, r9, #1
[^>]*> ee1b ff69 	vdwdup.u16	q7, sl, r9, #2
[^>]*> ee1b ef69 	viwdup.u16	q7, sl, r9, #2
[^>]*> ee1b ffe8 	vdwdup.u16	q7, sl, r9, #4
[^>]*> ee1b efe8 	viwdup.u16	q7, sl, r9, #4
[^>]*> ee1b ffe9 	vdwdup.u16	q7, sl, r9, #8
[^>]*> ee1b efe9 	viwdup.u16	q7, sl, r9, #8
[^>]*> ee1b ff6a 	vdwdup.u16	q7, sl, fp, #1
[^>]*> ee1b ef6a 	viwdup.u16	q7, sl, fp, #1
[^>]*> ee1b ff6b 	vdwdup.u16	q7, sl, fp, #2
[^>]*> ee1b ef6b 	viwdup.u16	q7, sl, fp, #2
[^>]*> ee1b ffea 	vdwdup.u16	q7, sl, fp, #4
[^>]*> ee1b efea 	viwdup.u16	q7, sl, fp, #4
[^>]*> ee1b ffeb 	vdwdup.u16	q7, sl, fp, #8
[^>]*> ee1b efeb 	viwdup.u16	q7, sl, fp, #8
[^>]*> ee1d ff6e 	vddup.u16	q7, ip, #1
[^>]*> ee1d ef6e 	vidup.u16	q7, ip, #1
[^>]*> ee1d ff6f 	vddup.u16	q7, ip, #2
[^>]*> ee1d ef6f 	vidup.u16	q7, ip, #2
[^>]*> ee1d ffee 	vddup.u16	q7, ip, #4
[^>]*> ee1d efee 	vidup.u16	q7, ip, #4
[^>]*> ee1d ffef 	vddup.u16	q7, ip, #8
[^>]*> ee1d efef 	vidup.u16	q7, ip, #8
[^>]*> ee1d ff60 	vdwdup.u16	q7, ip, r1, #1
[^>]*> ee1d ef60 	viwdup.u16	q7, ip, r1, #1
[^>]*> ee1d ff61 	vdwdup.u16	q7, ip, r1, #2
[^>]*> ee1d ef61 	viwdup.u16	q7, ip, r1, #2
[^>]*> ee1d ffe0 	vdwdup.u16	q7, ip, r1, #4
[^>]*> ee1d efe0 	viwdup.u16	q7, ip, r1, #4
[^>]*> ee1d ffe1 	vdwdup.u16	q7, ip, r1, #8
[^>]*> ee1d efe1 	viwdup.u16	q7, ip, r1, #8
[^>]*> ee1d ff62 	vdwdup.u16	q7, ip, r3, #1
[^>]*> ee1d ef62 	viwdup.u16	q7, ip, r3, #1
[^>]*> ee1d ff63 	vdwdup.u16	q7, ip, r3, #2
[^>]*> ee1d ef63 	viwdup.u16	q7, ip, r3, #2
[^>]*> ee1d ffe2 	vdwdup.u16	q7, ip, r3, #4
[^>]*> ee1d efe2 	viwdup.u16	q7, ip, r3, #4
[^>]*> ee1d ffe3 	vdwdup.u16	q7, ip, r3, #8
[^>]*> ee1d efe3 	viwdup.u16	q7, ip, r3, #8
[^>]*> ee1d ff64 	vdwdup.u16	q7, ip, r5, #1
[^>]*> ee1d ef64 	viwdup.u16	q7, ip, r5, #1
[^>]*> ee1d ff65 	vdwdup.u16	q7, ip, r5, #2
[^>]*> ee1d ef65 	viwdup.u16	q7, ip, r5, #2
[^>]*> ee1d ffe4 	vdwdup.u16	q7, ip, r5, #4
[^>]*> ee1d efe4 	viwdup.u16	q7, ip, r5, #4
[^>]*> ee1d ffe5 	vdwdup.u16	q7, ip, r5, #8
[^>]*> ee1d efe5 	viwdup.u16	q7, ip, r5, #8
[^>]*> ee1d ff66 	vdwdup.u16	q7, ip, r7, #1
[^>]*> ee1d ef66 	viwdup.u16	q7, ip, r7, #1
[^>]*> ee1d ff67 	vdwdup.u16	q7, ip, r7, #2
[^>]*> ee1d ef67 	viwdup.u16	q7, ip, r7, #2
[^>]*> ee1d ffe6 	vdwdup.u16	q7, ip, r7, #4
[^>]*> ee1d efe6 	viwdup.u16	q7, ip, r7, #4
[^>]*> ee1d ffe7 	vdwdup.u16	q7, ip, r7, #8
[^>]*> ee1d efe7 	viwdup.u16	q7, ip, r7, #8
[^>]*> ee1d ff68 	vdwdup.u16	q7, ip, r9, #1
[^>]*> ee1d ef68 	viwdup.u16	q7, ip, r9, #1
[^>]*> ee1d ff69 	vdwdup.u16	q7, ip, r9, #2
[^>]*> ee1d ef69 	viwdup.u16	q7, ip, r9, #2
[^>]*> ee1d ffe8 	vdwdup.u16	q7, ip, r9, #4
[^>]*> ee1d efe8 	viwdup.u16	q7, ip, r9, #4
[^>]*> ee1d ffe9 	vdwdup.u16	q7, ip, r9, #8
[^>]*> ee1d efe9 	viwdup.u16	q7, ip, r9, #8
[^>]*> ee1d ff6a 	vdwdup.u16	q7, ip, fp, #1
[^>]*> ee1d ef6a 	viwdup.u16	q7, ip, fp, #1
[^>]*> ee1d ff6b 	vdwdup.u16	q7, ip, fp, #2
[^>]*> ee1d ef6b 	viwdup.u16	q7, ip, fp, #2
[^>]*> ee1d ffea 	vdwdup.u16	q7, ip, fp, #4
[^>]*> ee1d efea 	viwdup.u16	q7, ip, fp, #4
[^>]*> ee1d ffeb 	vdwdup.u16	q7, ip, fp, #8
[^>]*> ee1d efeb 	viwdup.u16	q7, ip, fp, #8
[^>]*> ee21 1f6e 	vddup.u32	q0, r0, #1
[^>]*> ee21 0f6e 	vidup.u32	q0, r0, #1
[^>]*> ee21 1f6f 	vddup.u32	q0, r0, #2
[^>]*> ee21 0f6f 	vidup.u32	q0, r0, #2
[^>]*> ee21 1fee 	vddup.u32	q0, r0, #4
[^>]*> ee21 0fee 	vidup.u32	q0, r0, #4
[^>]*> ee21 1fef 	vddup.u32	q0, r0, #8
[^>]*> ee21 0fef 	vidup.u32	q0, r0, #8
[^>]*> ee21 1f60 	vdwdup.u32	q0, r0, r1, #1
[^>]*> ee21 0f60 	viwdup.u32	q0, r0, r1, #1
[^>]*> ee21 1f61 	vdwdup.u32	q0, r0, r1, #2
[^>]*> ee21 0f61 	viwdup.u32	q0, r0, r1, #2
[^>]*> ee21 1fe0 	vdwdup.u32	q0, r0, r1, #4
[^>]*> ee21 0fe0 	viwdup.u32	q0, r0, r1, #4
[^>]*> ee21 1fe1 	vdwdup.u32	q0, r0, r1, #8
[^>]*> ee21 0fe1 	viwdup.u32	q0, r0, r1, #8
[^>]*> ee21 1f62 	vdwdup.u32	q0, r0, r3, #1
[^>]*> ee21 0f62 	viwdup.u32	q0, r0, r3, #1
[^>]*> ee21 1f63 	vdwdup.u32	q0, r0, r3, #2
[^>]*> ee21 0f63 	viwdup.u32	q0, r0, r3, #2
[^>]*> ee21 1fe2 	vdwdup.u32	q0, r0, r3, #4
[^>]*> ee21 0fe2 	viwdup.u32	q0, r0, r3, #4
[^>]*> ee21 1fe3 	vdwdup.u32	q0, r0, r3, #8
[^>]*> ee21 0fe3 	viwdup.u32	q0, r0, r3, #8
[^>]*> ee21 1f64 	vdwdup.u32	q0, r0, r5, #1
[^>]*> ee21 0f64 	viwdup.u32	q0, r0, r5, #1
[^>]*> ee21 1f65 	vdwdup.u32	q0, r0, r5, #2
[^>]*> ee21 0f65 	viwdup.u32	q0, r0, r5, #2
[^>]*> ee21 1fe4 	vdwdup.u32	q0, r0, r5, #4
[^>]*> ee21 0fe4 	viwdup.u32	q0, r0, r5, #4
[^>]*> ee21 1fe5 	vdwdup.u32	q0, r0, r5, #8
[^>]*> ee21 0fe5 	viwdup.u32	q0, r0, r5, #8
[^>]*> ee21 1f66 	vdwdup.u32	q0, r0, r7, #1
[^>]*> ee21 0f66 	viwdup.u32	q0, r0, r7, #1
[^>]*> ee21 1f67 	vdwdup.u32	q0, r0, r7, #2
[^>]*> ee21 0f67 	viwdup.u32	q0, r0, r7, #2
[^>]*> ee21 1fe6 	vdwdup.u32	q0, r0, r7, #4
[^>]*> ee21 0fe6 	viwdup.u32	q0, r0, r7, #4
[^>]*> ee21 1fe7 	vdwdup.u32	q0, r0, r7, #8
[^>]*> ee21 0fe7 	viwdup.u32	q0, r0, r7, #8
[^>]*> ee21 1f68 	vdwdup.u32	q0, r0, r9, #1
[^>]*> ee21 0f68 	viwdup.u32	q0, r0, r9, #1
[^>]*> ee21 1f69 	vdwdup.u32	q0, r0, r9, #2
[^>]*> ee21 0f69 	viwdup.u32	q0, r0, r9, #2
[^>]*> ee21 1fe8 	vdwdup.u32	q0, r0, r9, #4
[^>]*> ee21 0fe8 	viwdup.u32	q0, r0, r9, #4
[^>]*> ee21 1fe9 	vdwdup.u32	q0, r0, r9, #8
[^>]*> ee21 0fe9 	viwdup.u32	q0, r0, r9, #8
[^>]*> ee21 1f6a 	vdwdup.u32	q0, r0, fp, #1
[^>]*> ee21 0f6a 	viwdup.u32	q0, r0, fp, #1
[^>]*> ee21 1f6b 	vdwdup.u32	q0, r0, fp, #2
[^>]*> ee21 0f6b 	viwdup.u32	q0, r0, fp, #2
[^>]*> ee21 1fea 	vdwdup.u32	q0, r0, fp, #4
[^>]*> ee21 0fea 	viwdup.u32	q0, r0, fp, #4
[^>]*> ee21 1feb 	vdwdup.u32	q0, r0, fp, #8
[^>]*> ee21 0feb 	viwdup.u32	q0, r0, fp, #8
[^>]*> ee23 1f6e 	vddup.u32	q0, r2, #1
[^>]*> ee23 0f6e 	vidup.u32	q0, r2, #1
[^>]*> ee23 1f6f 	vddup.u32	q0, r2, #2
[^>]*> ee23 0f6f 	vidup.u32	q0, r2, #2
[^>]*> ee23 1fee 	vddup.u32	q0, r2, #4
[^>]*> ee23 0fee 	vidup.u32	q0, r2, #4
[^>]*> ee23 1fef 	vddup.u32	q0, r2, #8
[^>]*> ee23 0fef 	vidup.u32	q0, r2, #8
[^>]*> ee23 1f60 	vdwdup.u32	q0, r2, r1, #1
[^>]*> ee23 0f60 	viwdup.u32	q0, r2, r1, #1
[^>]*> ee23 1f61 	vdwdup.u32	q0, r2, r1, #2
[^>]*> ee23 0f61 	viwdup.u32	q0, r2, r1, #2
[^>]*> ee23 1fe0 	vdwdup.u32	q0, r2, r1, #4
[^>]*> ee23 0fe0 	viwdup.u32	q0, r2, r1, #4
[^>]*> ee23 1fe1 	vdwdup.u32	q0, r2, r1, #8
[^>]*> ee23 0fe1 	viwdup.u32	q0, r2, r1, #8
[^>]*> ee23 1f62 	vdwdup.u32	q0, r2, r3, #1
[^>]*> ee23 0f62 	viwdup.u32	q0, r2, r3, #1
[^>]*> ee23 1f63 	vdwdup.u32	q0, r2, r3, #2
[^>]*> ee23 0f63 	viwdup.u32	q0, r2, r3, #2
[^>]*> ee23 1fe2 	vdwdup.u32	q0, r2, r3, #4
[^>]*> ee23 0fe2 	viwdup.u32	q0, r2, r3, #4
[^>]*> ee23 1fe3 	vdwdup.u32	q0, r2, r3, #8
[^>]*> ee23 0fe3 	viwdup.u32	q0, r2, r3, #8
[^>]*> ee23 1f64 	vdwdup.u32	q0, r2, r5, #1
[^>]*> ee23 0f64 	viwdup.u32	q0, r2, r5, #1
[^>]*> ee23 1f65 	vdwdup.u32	q0, r2, r5, #2
[^>]*> ee23 0f65 	viwdup.u32	q0, r2, r5, #2
[^>]*> ee23 1fe4 	vdwdup.u32	q0, r2, r5, #4
[^>]*> ee23 0fe4 	viwdup.u32	q0, r2, r5, #4
[^>]*> ee23 1fe5 	vdwdup.u32	q0, r2, r5, #8
[^>]*> ee23 0fe5 	viwdup.u32	q0, r2, r5, #8
[^>]*> ee23 1f66 	vdwdup.u32	q0, r2, r7, #1
[^>]*> ee23 0f66 	viwdup.u32	q0, r2, r7, #1
[^>]*> ee23 1f67 	vdwdup.u32	q0, r2, r7, #2
[^>]*> ee23 0f67 	viwdup.u32	q0, r2, r7, #2
[^>]*> ee23 1fe6 	vdwdup.u32	q0, r2, r7, #4
[^>]*> ee23 0fe6 	viwdup.u32	q0, r2, r7, #4
[^>]*> ee23 1fe7 	vdwdup.u32	q0, r2, r7, #8
[^>]*> ee23 0fe7 	viwdup.u32	q0, r2, r7, #8
[^>]*> ee23 1f68 	vdwdup.u32	q0, r2, r9, #1
[^>]*> ee23 0f68 	viwdup.u32	q0, r2, r9, #1
[^>]*> ee23 1f69 	vdwdup.u32	q0, r2, r9, #2
[^>]*> ee23 0f69 	viwdup.u32	q0, r2, r9, #2
[^>]*> ee23 1fe8 	vdwdup.u32	q0, r2, r9, #4
[^>]*> ee23 0fe8 	viwdup.u32	q0, r2, r9, #4
[^>]*> ee23 1fe9 	vdwdup.u32	q0, r2, r9, #8
[^>]*> ee23 0fe9 	viwdup.u32	q0, r2, r9, #8
[^>]*> ee23 1f6a 	vdwdup.u32	q0, r2, fp, #1
[^>]*> ee23 0f6a 	viwdup.u32	q0, r2, fp, #1
[^>]*> ee23 1f6b 	vdwdup.u32	q0, r2, fp, #2
[^>]*> ee23 0f6b 	viwdup.u32	q0, r2, fp, #2
[^>]*> ee23 1fea 	vdwdup.u32	q0, r2, fp, #4
[^>]*> ee23 0fea 	viwdup.u32	q0, r2, fp, #4
[^>]*> ee23 1feb 	vdwdup.u32	q0, r2, fp, #8
[^>]*> ee23 0feb 	viwdup.u32	q0, r2, fp, #8
[^>]*> ee25 1f6e 	vddup.u32	q0, r4, #1
[^>]*> ee25 0f6e 	vidup.u32	q0, r4, #1
[^>]*> ee25 1f6f 	vddup.u32	q0, r4, #2
[^>]*> ee25 0f6f 	vidup.u32	q0, r4, #2
[^>]*> ee25 1fee 	vddup.u32	q0, r4, #4
[^>]*> ee25 0fee 	vidup.u32	q0, r4, #4
[^>]*> ee25 1fef 	vddup.u32	q0, r4, #8
[^>]*> ee25 0fef 	vidup.u32	q0, r4, #8
[^>]*> ee25 1f60 	vdwdup.u32	q0, r4, r1, #1
[^>]*> ee25 0f60 	viwdup.u32	q0, r4, r1, #1
[^>]*> ee25 1f61 	vdwdup.u32	q0, r4, r1, #2
[^>]*> ee25 0f61 	viwdup.u32	q0, r4, r1, #2
[^>]*> ee25 1fe0 	vdwdup.u32	q0, r4, r1, #4
[^>]*> ee25 0fe0 	viwdup.u32	q0, r4, r1, #4
[^>]*> ee25 1fe1 	vdwdup.u32	q0, r4, r1, #8
[^>]*> ee25 0fe1 	viwdup.u32	q0, r4, r1, #8
[^>]*> ee25 1f62 	vdwdup.u32	q0, r4, r3, #1
[^>]*> ee25 0f62 	viwdup.u32	q0, r4, r3, #1
[^>]*> ee25 1f63 	vdwdup.u32	q0, r4, r3, #2
[^>]*> ee25 0f63 	viwdup.u32	q0, r4, r3, #2
[^>]*> ee25 1fe2 	vdwdup.u32	q0, r4, r3, #4
[^>]*> ee25 0fe2 	viwdup.u32	q0, r4, r3, #4
[^>]*> ee25 1fe3 	vdwdup.u32	q0, r4, r3, #8
[^>]*> ee25 0fe3 	viwdup.u32	q0, r4, r3, #8
[^>]*> ee25 1f64 	vdwdup.u32	q0, r4, r5, #1
[^>]*> ee25 0f64 	viwdup.u32	q0, r4, r5, #1
[^>]*> ee25 1f65 	vdwdup.u32	q0, r4, r5, #2
[^>]*> ee25 0f65 	viwdup.u32	q0, r4, r5, #2
[^>]*> ee25 1fe4 	vdwdup.u32	q0, r4, r5, #4
[^>]*> ee25 0fe4 	viwdup.u32	q0, r4, r5, #4
[^>]*> ee25 1fe5 	vdwdup.u32	q0, r4, r5, #8
[^>]*> ee25 0fe5 	viwdup.u32	q0, r4, r5, #8
[^>]*> ee25 1f66 	vdwdup.u32	q0, r4, r7, #1
[^>]*> ee25 0f66 	viwdup.u32	q0, r4, r7, #1
[^>]*> ee25 1f67 	vdwdup.u32	q0, r4, r7, #2
[^>]*> ee25 0f67 	viwdup.u32	q0, r4, r7, #2
[^>]*> ee25 1fe6 	vdwdup.u32	q0, r4, r7, #4
[^>]*> ee25 0fe6 	viwdup.u32	q0, r4, r7, #4
[^>]*> ee25 1fe7 	vdwdup.u32	q0, r4, r7, #8
[^>]*> ee25 0fe7 	viwdup.u32	q0, r4, r7, #8
[^>]*> ee25 1f68 	vdwdup.u32	q0, r4, r9, #1
[^>]*> ee25 0f68 	viwdup.u32	q0, r4, r9, #1
[^>]*> ee25 1f69 	vdwdup.u32	q0, r4, r9, #2
[^>]*> ee25 0f69 	viwdup.u32	q0, r4, r9, #2
[^>]*> ee25 1fe8 	vdwdup.u32	q0, r4, r9, #4
[^>]*> ee25 0fe8 	viwdup.u32	q0, r4, r9, #4
[^>]*> ee25 1fe9 	vdwdup.u32	q0, r4, r9, #8
[^>]*> ee25 0fe9 	viwdup.u32	q0, r4, r9, #8
[^>]*> ee25 1f6a 	vdwdup.u32	q0, r4, fp, #1
[^>]*> ee25 0f6a 	viwdup.u32	q0, r4, fp, #1
[^>]*> ee25 1f6b 	vdwdup.u32	q0, r4, fp, #2
[^>]*> ee25 0f6b 	viwdup.u32	q0, r4, fp, #2
[^>]*> ee25 1fea 	vdwdup.u32	q0, r4, fp, #4
[^>]*> ee25 0fea 	viwdup.u32	q0, r4, fp, #4
[^>]*> ee25 1feb 	vdwdup.u32	q0, r4, fp, #8
[^>]*> ee25 0feb 	viwdup.u32	q0, r4, fp, #8
[^>]*> ee27 1f6e 	vddup.u32	q0, r6, #1
[^>]*> ee27 0f6e 	vidup.u32	q0, r6, #1
[^>]*> ee27 1f6f 	vddup.u32	q0, r6, #2
[^>]*> ee27 0f6f 	vidup.u32	q0, r6, #2
[^>]*> ee27 1fee 	vddup.u32	q0, r6, #4
[^>]*> ee27 0fee 	vidup.u32	q0, r6, #4
[^>]*> ee27 1fef 	vddup.u32	q0, r6, #8
[^>]*> ee27 0fef 	vidup.u32	q0, r6, #8
[^>]*> ee27 1f60 	vdwdup.u32	q0, r6, r1, #1
[^>]*> ee27 0f60 	viwdup.u32	q0, r6, r1, #1
[^>]*> ee27 1f61 	vdwdup.u32	q0, r6, r1, #2
[^>]*> ee27 0f61 	viwdup.u32	q0, r6, r1, #2
[^>]*> ee27 1fe0 	vdwdup.u32	q0, r6, r1, #4
[^>]*> ee27 0fe0 	viwdup.u32	q0, r6, r1, #4
[^>]*> ee27 1fe1 	vdwdup.u32	q0, r6, r1, #8
[^>]*> ee27 0fe1 	viwdup.u32	q0, r6, r1, #8
[^>]*> ee27 1f62 	vdwdup.u32	q0, r6, r3, #1
[^>]*> ee27 0f62 	viwdup.u32	q0, r6, r3, #1
[^>]*> ee27 1f63 	vdwdup.u32	q0, r6, r3, #2
[^>]*> ee27 0f63 	viwdup.u32	q0, r6, r3, #2
[^>]*> ee27 1fe2 	vdwdup.u32	q0, r6, r3, #4
[^>]*> ee27 0fe2 	viwdup.u32	q0, r6, r3, #4
[^>]*> ee27 1fe3 	vdwdup.u32	q0, r6, r3, #8
[^>]*> ee27 0fe3 	viwdup.u32	q0, r6, r3, #8
[^>]*> ee27 1f64 	vdwdup.u32	q0, r6, r5, #1
[^>]*> ee27 0f64 	viwdup.u32	q0, r6, r5, #1
[^>]*> ee27 1f65 	vdwdup.u32	q0, r6, r5, #2
[^>]*> ee27 0f65 	viwdup.u32	q0, r6, r5, #2
[^>]*> ee27 1fe4 	vdwdup.u32	q0, r6, r5, #4
[^>]*> ee27 0fe4 	viwdup.u32	q0, r6, r5, #4
[^>]*> ee27 1fe5 	vdwdup.u32	q0, r6, r5, #8
[^>]*> ee27 0fe5 	viwdup.u32	q0, r6, r5, #8
[^>]*> ee27 1f66 	vdwdup.u32	q0, r6, r7, #1
[^>]*> ee27 0f66 	viwdup.u32	q0, r6, r7, #1
[^>]*> ee27 1f67 	vdwdup.u32	q0, r6, r7, #2
[^>]*> ee27 0f67 	viwdup.u32	q0, r6, r7, #2
[^>]*> ee27 1fe6 	vdwdup.u32	q0, r6, r7, #4
[^>]*> ee27 0fe6 	viwdup.u32	q0, r6, r7, #4
[^>]*> ee27 1fe7 	vdwdup.u32	q0, r6, r7, #8
[^>]*> ee27 0fe7 	viwdup.u32	q0, r6, r7, #8
[^>]*> ee27 1f68 	vdwdup.u32	q0, r6, r9, #1
[^>]*> ee27 0f68 	viwdup.u32	q0, r6, r9, #1
[^>]*> ee27 1f69 	vdwdup.u32	q0, r6, r9, #2
[^>]*> ee27 0f69 	viwdup.u32	q0, r6, r9, #2
[^>]*> ee27 1fe8 	vdwdup.u32	q0, r6, r9, #4
[^>]*> ee27 0fe8 	viwdup.u32	q0, r6, r9, #4
[^>]*> ee27 1fe9 	vdwdup.u32	q0, r6, r9, #8
[^>]*> ee27 0fe9 	viwdup.u32	q0, r6, r9, #8
[^>]*> ee27 1f6a 	vdwdup.u32	q0, r6, fp, #1
[^>]*> ee27 0f6a 	viwdup.u32	q0, r6, fp, #1
[^>]*> ee27 1f6b 	vdwdup.u32	q0, r6, fp, #2
[^>]*> ee27 0f6b 	viwdup.u32	q0, r6, fp, #2
[^>]*> ee27 1fea 	vdwdup.u32	q0, r6, fp, #4
[^>]*> ee27 0fea 	viwdup.u32	q0, r6, fp, #4
[^>]*> ee27 1feb 	vdwdup.u32	q0, r6, fp, #8
[^>]*> ee27 0feb 	viwdup.u32	q0, r6, fp, #8
[^>]*> ee29 1f6e 	vddup.u32	q0, r8, #1
[^>]*> ee29 0f6e 	vidup.u32	q0, r8, #1
[^>]*> ee29 1f6f 	vddup.u32	q0, r8, #2
[^>]*> ee29 0f6f 	vidup.u32	q0, r8, #2
[^>]*> ee29 1fee 	vddup.u32	q0, r8, #4
[^>]*> ee29 0fee 	vidup.u32	q0, r8, #4
[^>]*> ee29 1fef 	vddup.u32	q0, r8, #8
[^>]*> ee29 0fef 	vidup.u32	q0, r8, #8
[^>]*> ee29 1f60 	vdwdup.u32	q0, r8, r1, #1
[^>]*> ee29 0f60 	viwdup.u32	q0, r8, r1, #1
[^>]*> ee29 1f61 	vdwdup.u32	q0, r8, r1, #2
[^>]*> ee29 0f61 	viwdup.u32	q0, r8, r1, #2
[^>]*> ee29 1fe0 	vdwdup.u32	q0, r8, r1, #4
[^>]*> ee29 0fe0 	viwdup.u32	q0, r8, r1, #4
[^>]*> ee29 1fe1 	vdwdup.u32	q0, r8, r1, #8
[^>]*> ee29 0fe1 	viwdup.u32	q0, r8, r1, #8
[^>]*> ee29 1f62 	vdwdup.u32	q0, r8, r3, #1
[^>]*> ee29 0f62 	viwdup.u32	q0, r8, r3, #1
[^>]*> ee29 1f63 	vdwdup.u32	q0, r8, r3, #2
[^>]*> ee29 0f63 	viwdup.u32	q0, r8, r3, #2
[^>]*> ee29 1fe2 	vdwdup.u32	q0, r8, r3, #4
[^>]*> ee29 0fe2 	viwdup.u32	q0, r8, r3, #4
[^>]*> ee29 1fe3 	vdwdup.u32	q0, r8, r3, #8
[^>]*> ee29 0fe3 	viwdup.u32	q0, r8, r3, #8
[^>]*> ee29 1f64 	vdwdup.u32	q0, r8, r5, #1
[^>]*> ee29 0f64 	viwdup.u32	q0, r8, r5, #1
[^>]*> ee29 1f65 	vdwdup.u32	q0, r8, r5, #2
[^>]*> ee29 0f65 	viwdup.u32	q0, r8, r5, #2
[^>]*> ee29 1fe4 	vdwdup.u32	q0, r8, r5, #4
[^>]*> ee29 0fe4 	viwdup.u32	q0, r8, r5, #4
[^>]*> ee29 1fe5 	vdwdup.u32	q0, r8, r5, #8
[^>]*> ee29 0fe5 	viwdup.u32	q0, r8, r5, #8
[^>]*> ee29 1f66 	vdwdup.u32	q0, r8, r7, #1
[^>]*> ee29 0f66 	viwdup.u32	q0, r8, r7, #1
[^>]*> ee29 1f67 	vdwdup.u32	q0, r8, r7, #2
[^>]*> ee29 0f67 	viwdup.u32	q0, r8, r7, #2
[^>]*> ee29 1fe6 	vdwdup.u32	q0, r8, r7, #4
[^>]*> ee29 0fe6 	viwdup.u32	q0, r8, r7, #4
[^>]*> ee29 1fe7 	vdwdup.u32	q0, r8, r7, #8
[^>]*> ee29 0fe7 	viwdup.u32	q0, r8, r7, #8
[^>]*> ee29 1f68 	vdwdup.u32	q0, r8, r9, #1
[^>]*> ee29 0f68 	viwdup.u32	q0, r8, r9, #1
[^>]*> ee29 1f69 	vdwdup.u32	q0, r8, r9, #2
[^>]*> ee29 0f69 	viwdup.u32	q0, r8, r9, #2
[^>]*> ee29 1fe8 	vdwdup.u32	q0, r8, r9, #4
[^>]*> ee29 0fe8 	viwdup.u32	q0, r8, r9, #4
[^>]*> ee29 1fe9 	vdwdup.u32	q0, r8, r9, #8
[^>]*> ee29 0fe9 	viwdup.u32	q0, r8, r9, #8
[^>]*> ee29 1f6a 	vdwdup.u32	q0, r8, fp, #1
[^>]*> ee29 0f6a 	viwdup.u32	q0, r8, fp, #1
[^>]*> ee29 1f6b 	vdwdup.u32	q0, r8, fp, #2
[^>]*> ee29 0f6b 	viwdup.u32	q0, r8, fp, #2
[^>]*> ee29 1fea 	vdwdup.u32	q0, r8, fp, #4
[^>]*> ee29 0fea 	viwdup.u32	q0, r8, fp, #4
[^>]*> ee29 1feb 	vdwdup.u32	q0, r8, fp, #8
[^>]*> ee29 0feb 	viwdup.u32	q0, r8, fp, #8
[^>]*> ee2b 1f6e 	vddup.u32	q0, sl, #1
[^>]*> ee2b 0f6e 	vidup.u32	q0, sl, #1
[^>]*> ee2b 1f6f 	vddup.u32	q0, sl, #2
[^>]*> ee2b 0f6f 	vidup.u32	q0, sl, #2
[^>]*> ee2b 1fee 	vddup.u32	q0, sl, #4
[^>]*> ee2b 0fee 	vidup.u32	q0, sl, #4
[^>]*> ee2b 1fef 	vddup.u32	q0, sl, #8
[^>]*> ee2b 0fef 	vidup.u32	q0, sl, #8
[^>]*> ee2b 1f60 	vdwdup.u32	q0, sl, r1, #1
[^>]*> ee2b 0f60 	viwdup.u32	q0, sl, r1, #1
[^>]*> ee2b 1f61 	vdwdup.u32	q0, sl, r1, #2
[^>]*> ee2b 0f61 	viwdup.u32	q0, sl, r1, #2
[^>]*> ee2b 1fe0 	vdwdup.u32	q0, sl, r1, #4
[^>]*> ee2b 0fe0 	viwdup.u32	q0, sl, r1, #4
[^>]*> ee2b 1fe1 	vdwdup.u32	q0, sl, r1, #8
[^>]*> ee2b 0fe1 	viwdup.u32	q0, sl, r1, #8
[^>]*> ee2b 1f62 	vdwdup.u32	q0, sl, r3, #1
[^>]*> ee2b 0f62 	viwdup.u32	q0, sl, r3, #1
[^>]*> ee2b 1f63 	vdwdup.u32	q0, sl, r3, #2
[^>]*> ee2b 0f63 	viwdup.u32	q0, sl, r3, #2
[^>]*> ee2b 1fe2 	vdwdup.u32	q0, sl, r3, #4
[^>]*> ee2b 0fe2 	viwdup.u32	q0, sl, r3, #4
[^>]*> ee2b 1fe3 	vdwdup.u32	q0, sl, r3, #8
[^>]*> ee2b 0fe3 	viwdup.u32	q0, sl, r3, #8
[^>]*> ee2b 1f64 	vdwdup.u32	q0, sl, r5, #1
[^>]*> ee2b 0f64 	viwdup.u32	q0, sl, r5, #1
[^>]*> ee2b 1f65 	vdwdup.u32	q0, sl, r5, #2
[^>]*> ee2b 0f65 	viwdup.u32	q0, sl, r5, #2
[^>]*> ee2b 1fe4 	vdwdup.u32	q0, sl, r5, #4
[^>]*> ee2b 0fe4 	viwdup.u32	q0, sl, r5, #4
[^>]*> ee2b 1fe5 	vdwdup.u32	q0, sl, r5, #8
[^>]*> ee2b 0fe5 	viwdup.u32	q0, sl, r5, #8
[^>]*> ee2b 1f66 	vdwdup.u32	q0, sl, r7, #1
[^>]*> ee2b 0f66 	viwdup.u32	q0, sl, r7, #1
[^>]*> ee2b 1f67 	vdwdup.u32	q0, sl, r7, #2
[^>]*> ee2b 0f67 	viwdup.u32	q0, sl, r7, #2
[^>]*> ee2b 1fe6 	vdwdup.u32	q0, sl, r7, #4
[^>]*> ee2b 0fe6 	viwdup.u32	q0, sl, r7, #4
[^>]*> ee2b 1fe7 	vdwdup.u32	q0, sl, r7, #8
[^>]*> ee2b 0fe7 	viwdup.u32	q0, sl, r7, #8
[^>]*> ee2b 1f68 	vdwdup.u32	q0, sl, r9, #1
[^>]*> ee2b 0f68 	viwdup.u32	q0, sl, r9, #1
[^>]*> ee2b 1f69 	vdwdup.u32	q0, sl, r9, #2
[^>]*> ee2b 0f69 	viwdup.u32	q0, sl, r9, #2
[^>]*> ee2b 1fe8 	vdwdup.u32	q0, sl, r9, #4
[^>]*> ee2b 0fe8 	viwdup.u32	q0, sl, r9, #4
[^>]*> ee2b 1fe9 	vdwdup.u32	q0, sl, r9, #8
[^>]*> ee2b 0fe9 	viwdup.u32	q0, sl, r9, #8
[^>]*> ee2b 1f6a 	vdwdup.u32	q0, sl, fp, #1
[^>]*> ee2b 0f6a 	viwdup.u32	q0, sl, fp, #1
[^>]*> ee2b 1f6b 	vdwdup.u32	q0, sl, fp, #2
[^>]*> ee2b 0f6b 	viwdup.u32	q0, sl, fp, #2
[^>]*> ee2b 1fea 	vdwdup.u32	q0, sl, fp, #4
[^>]*> ee2b 0fea 	viwdup.u32	q0, sl, fp, #4
[^>]*> ee2b 1feb 	vdwdup.u32	q0, sl, fp, #8
[^>]*> ee2b 0feb 	viwdup.u32	q0, sl, fp, #8
[^>]*> ee2d 1f6e 	vddup.u32	q0, ip, #1
[^>]*> ee2d 0f6e 	vidup.u32	q0, ip, #1
[^>]*> ee2d 1f6f 	vddup.u32	q0, ip, #2
[^>]*> ee2d 0f6f 	vidup.u32	q0, ip, #2
[^>]*> ee2d 1fee 	vddup.u32	q0, ip, #4
[^>]*> ee2d 0fee 	vidup.u32	q0, ip, #4
[^>]*> ee2d 1fef 	vddup.u32	q0, ip, #8
[^>]*> ee2d 0fef 	vidup.u32	q0, ip, #8
[^>]*> ee2d 1f60 	vdwdup.u32	q0, ip, r1, #1
[^>]*> ee2d 0f60 	viwdup.u32	q0, ip, r1, #1
[^>]*> ee2d 1f61 	vdwdup.u32	q0, ip, r1, #2
[^>]*> ee2d 0f61 	viwdup.u32	q0, ip, r1, #2
[^>]*> ee2d 1fe0 	vdwdup.u32	q0, ip, r1, #4
[^>]*> ee2d 0fe0 	viwdup.u32	q0, ip, r1, #4
[^>]*> ee2d 1fe1 	vdwdup.u32	q0, ip, r1, #8
[^>]*> ee2d 0fe1 	viwdup.u32	q0, ip, r1, #8
[^>]*> ee2d 1f62 	vdwdup.u32	q0, ip, r3, #1
[^>]*> ee2d 0f62 	viwdup.u32	q0, ip, r3, #1
[^>]*> ee2d 1f63 	vdwdup.u32	q0, ip, r3, #2
[^>]*> ee2d 0f63 	viwdup.u32	q0, ip, r3, #2
[^>]*> ee2d 1fe2 	vdwdup.u32	q0, ip, r3, #4
[^>]*> ee2d 0fe2 	viwdup.u32	q0, ip, r3, #4
[^>]*> ee2d 1fe3 	vdwdup.u32	q0, ip, r3, #8
[^>]*> ee2d 0fe3 	viwdup.u32	q0, ip, r3, #8
[^>]*> ee2d 1f64 	vdwdup.u32	q0, ip, r5, #1
[^>]*> ee2d 0f64 	viwdup.u32	q0, ip, r5, #1
[^>]*> ee2d 1f65 	vdwdup.u32	q0, ip, r5, #2
[^>]*> ee2d 0f65 	viwdup.u32	q0, ip, r5, #2
[^>]*> ee2d 1fe4 	vdwdup.u32	q0, ip, r5, #4
[^>]*> ee2d 0fe4 	viwdup.u32	q0, ip, r5, #4
[^>]*> ee2d 1fe5 	vdwdup.u32	q0, ip, r5, #8
[^>]*> ee2d 0fe5 	viwdup.u32	q0, ip, r5, #8
[^>]*> ee2d 1f66 	vdwdup.u32	q0, ip, r7, #1
[^>]*> ee2d 0f66 	viwdup.u32	q0, ip, r7, #1
[^>]*> ee2d 1f67 	vdwdup.u32	q0, ip, r7, #2
[^>]*> ee2d 0f67 	viwdup.u32	q0, ip, r7, #2
[^>]*> ee2d 1fe6 	vdwdup.u32	q0, ip, r7, #4
[^>]*> ee2d 0fe6 	viwdup.u32	q0, ip, r7, #4
[^>]*> ee2d 1fe7 	vdwdup.u32	q0, ip, r7, #8
[^>]*> ee2d 0fe7 	viwdup.u32	q0, ip, r7, #8
[^>]*> ee2d 1f68 	vdwdup.u32	q0, ip, r9, #1
[^>]*> ee2d 0f68 	viwdup.u32	q0, ip, r9, #1
[^>]*> ee2d 1f69 	vdwdup.u32	q0, ip, r9, #2
[^>]*> ee2d 0f69 	viwdup.u32	q0, ip, r9, #2
[^>]*> ee2d 1fe8 	vdwdup.u32	q0, ip, r9, #4
[^>]*> ee2d 0fe8 	viwdup.u32	q0, ip, r9, #4
[^>]*> ee2d 1fe9 	vdwdup.u32	q0, ip, r9, #8
[^>]*> ee2d 0fe9 	viwdup.u32	q0, ip, r9, #8
[^>]*> ee2d 1f6a 	vdwdup.u32	q0, ip, fp, #1
[^>]*> ee2d 0f6a 	viwdup.u32	q0, ip, fp, #1
[^>]*> ee2d 1f6b 	vdwdup.u32	q0, ip, fp, #2
[^>]*> ee2d 0f6b 	viwdup.u32	q0, ip, fp, #2
[^>]*> ee2d 1fea 	vdwdup.u32	q0, ip, fp, #4
[^>]*> ee2d 0fea 	viwdup.u32	q0, ip, fp, #4
[^>]*> ee2d 1feb 	vdwdup.u32	q0, ip, fp, #8
[^>]*> ee2d 0feb 	viwdup.u32	q0, ip, fp, #8
[^>]*> ee21 3f6e 	vddup.u32	q1, r0, #1
[^>]*> ee21 2f6e 	vidup.u32	q1, r0, #1
[^>]*> ee21 3f6f 	vddup.u32	q1, r0, #2
[^>]*> ee21 2f6f 	vidup.u32	q1, r0, #2
[^>]*> ee21 3fee 	vddup.u32	q1, r0, #4
[^>]*> ee21 2fee 	vidup.u32	q1, r0, #4
[^>]*> ee21 3fef 	vddup.u32	q1, r0, #8
[^>]*> ee21 2fef 	vidup.u32	q1, r0, #8
[^>]*> ee21 3f60 	vdwdup.u32	q1, r0, r1, #1
[^>]*> ee21 2f60 	viwdup.u32	q1, r0, r1, #1
[^>]*> ee21 3f61 	vdwdup.u32	q1, r0, r1, #2
[^>]*> ee21 2f61 	viwdup.u32	q1, r0, r1, #2
[^>]*> ee21 3fe0 	vdwdup.u32	q1, r0, r1, #4
[^>]*> ee21 2fe0 	viwdup.u32	q1, r0, r1, #4
[^>]*> ee21 3fe1 	vdwdup.u32	q1, r0, r1, #8
[^>]*> ee21 2fe1 	viwdup.u32	q1, r0, r1, #8
[^>]*> ee21 3f62 	vdwdup.u32	q1, r0, r3, #1
[^>]*> ee21 2f62 	viwdup.u32	q1, r0, r3, #1
[^>]*> ee21 3f63 	vdwdup.u32	q1, r0, r3, #2
[^>]*> ee21 2f63 	viwdup.u32	q1, r0, r3, #2
[^>]*> ee21 3fe2 	vdwdup.u32	q1, r0, r3, #4
[^>]*> ee21 2fe2 	viwdup.u32	q1, r0, r3, #4
[^>]*> ee21 3fe3 	vdwdup.u32	q1, r0, r3, #8
[^>]*> ee21 2fe3 	viwdup.u32	q1, r0, r3, #8
[^>]*> ee21 3f64 	vdwdup.u32	q1, r0, r5, #1
[^>]*> ee21 2f64 	viwdup.u32	q1, r0, r5, #1
[^>]*> ee21 3f65 	vdwdup.u32	q1, r0, r5, #2
[^>]*> ee21 2f65 	viwdup.u32	q1, r0, r5, #2
[^>]*> ee21 3fe4 	vdwdup.u32	q1, r0, r5, #4
[^>]*> ee21 2fe4 	viwdup.u32	q1, r0, r5, #4
[^>]*> ee21 3fe5 	vdwdup.u32	q1, r0, r5, #8
[^>]*> ee21 2fe5 	viwdup.u32	q1, r0, r5, #8
[^>]*> ee21 3f66 	vdwdup.u32	q1, r0, r7, #1
[^>]*> ee21 2f66 	viwdup.u32	q1, r0, r7, #1
[^>]*> ee21 3f67 	vdwdup.u32	q1, r0, r7, #2
[^>]*> ee21 2f67 	viwdup.u32	q1, r0, r7, #2
[^>]*> ee21 3fe6 	vdwdup.u32	q1, r0, r7, #4
[^>]*> ee21 2fe6 	viwdup.u32	q1, r0, r7, #4
[^>]*> ee21 3fe7 	vdwdup.u32	q1, r0, r7, #8
[^>]*> ee21 2fe7 	viwdup.u32	q1, r0, r7, #8
[^>]*> ee21 3f68 	vdwdup.u32	q1, r0, r9, #1
[^>]*> ee21 2f68 	viwdup.u32	q1, r0, r9, #1
[^>]*> ee21 3f69 	vdwdup.u32	q1, r0, r9, #2
[^>]*> ee21 2f69 	viwdup.u32	q1, r0, r9, #2
[^>]*> ee21 3fe8 	vdwdup.u32	q1, r0, r9, #4
[^>]*> ee21 2fe8 	viwdup.u32	q1, r0, r9, #4
[^>]*> ee21 3fe9 	vdwdup.u32	q1, r0, r9, #8
[^>]*> ee21 2fe9 	viwdup.u32	q1, r0, r9, #8
[^>]*> ee21 3f6a 	vdwdup.u32	q1, r0, fp, #1
[^>]*> ee21 2f6a 	viwdup.u32	q1, r0, fp, #1
[^>]*> ee21 3f6b 	vdwdup.u32	q1, r0, fp, #2
[^>]*> ee21 2f6b 	viwdup.u32	q1, r0, fp, #2
[^>]*> ee21 3fea 	vdwdup.u32	q1, r0, fp, #4
[^>]*> ee21 2fea 	viwdup.u32	q1, r0, fp, #4
[^>]*> ee21 3feb 	vdwdup.u32	q1, r0, fp, #8
[^>]*> ee21 2feb 	viwdup.u32	q1, r0, fp, #8
[^>]*> ee23 3f6e 	vddup.u32	q1, r2, #1
[^>]*> ee23 2f6e 	vidup.u32	q1, r2, #1
[^>]*> ee23 3f6f 	vddup.u32	q1, r2, #2
[^>]*> ee23 2f6f 	vidup.u32	q1, r2, #2
[^>]*> ee23 3fee 	vddup.u32	q1, r2, #4
[^>]*> ee23 2fee 	vidup.u32	q1, r2, #4
[^>]*> ee23 3fef 	vddup.u32	q1, r2, #8
[^>]*> ee23 2fef 	vidup.u32	q1, r2, #8
[^>]*> ee23 3f60 	vdwdup.u32	q1, r2, r1, #1
[^>]*> ee23 2f60 	viwdup.u32	q1, r2, r1, #1
[^>]*> ee23 3f61 	vdwdup.u32	q1, r2, r1, #2
[^>]*> ee23 2f61 	viwdup.u32	q1, r2, r1, #2
[^>]*> ee23 3fe0 	vdwdup.u32	q1, r2, r1, #4
[^>]*> ee23 2fe0 	viwdup.u32	q1, r2, r1, #4
[^>]*> ee23 3fe1 	vdwdup.u32	q1, r2, r1, #8
[^>]*> ee23 2fe1 	viwdup.u32	q1, r2, r1, #8
[^>]*> ee23 3f62 	vdwdup.u32	q1, r2, r3, #1
[^>]*> ee23 2f62 	viwdup.u32	q1, r2, r3, #1
[^>]*> ee23 3f63 	vdwdup.u32	q1, r2, r3, #2
[^>]*> ee23 2f63 	viwdup.u32	q1, r2, r3, #2
[^>]*> ee23 3fe2 	vdwdup.u32	q1, r2, r3, #4
[^>]*> ee23 2fe2 	viwdup.u32	q1, r2, r3, #4
[^>]*> ee23 3fe3 	vdwdup.u32	q1, r2, r3, #8
[^>]*> ee23 2fe3 	viwdup.u32	q1, r2, r3, #8
[^>]*> ee23 3f64 	vdwdup.u32	q1, r2, r5, #1
[^>]*> ee23 2f64 	viwdup.u32	q1, r2, r5, #1
[^>]*> ee23 3f65 	vdwdup.u32	q1, r2, r5, #2
[^>]*> ee23 2f65 	viwdup.u32	q1, r2, r5, #2
[^>]*> ee23 3fe4 	vdwdup.u32	q1, r2, r5, #4
[^>]*> ee23 2fe4 	viwdup.u32	q1, r2, r5, #4
[^>]*> ee23 3fe5 	vdwdup.u32	q1, r2, r5, #8
[^>]*> ee23 2fe5 	viwdup.u32	q1, r2, r5, #8
[^>]*> ee23 3f66 	vdwdup.u32	q1, r2, r7, #1
[^>]*> ee23 2f66 	viwdup.u32	q1, r2, r7, #1
[^>]*> ee23 3f67 	vdwdup.u32	q1, r2, r7, #2
[^>]*> ee23 2f67 	viwdup.u32	q1, r2, r7, #2
[^>]*> ee23 3fe6 	vdwdup.u32	q1, r2, r7, #4
[^>]*> ee23 2fe6 	viwdup.u32	q1, r2, r7, #4
[^>]*> ee23 3fe7 	vdwdup.u32	q1, r2, r7, #8
[^>]*> ee23 2fe7 	viwdup.u32	q1, r2, r7, #8
[^>]*> ee23 3f68 	vdwdup.u32	q1, r2, r9, #1
[^>]*> ee23 2f68 	viwdup.u32	q1, r2, r9, #1
[^>]*> ee23 3f69 	vdwdup.u32	q1, r2, r9, #2
[^>]*> ee23 2f69 	viwdup.u32	q1, r2, r9, #2
[^>]*> ee23 3fe8 	vdwdup.u32	q1, r2, r9, #4
[^>]*> ee23 2fe8 	viwdup.u32	q1, r2, r9, #4
[^>]*> ee23 3fe9 	vdwdup.u32	q1, r2, r9, #8
[^>]*> ee23 2fe9 	viwdup.u32	q1, r2, r9, #8
[^>]*> ee23 3f6a 	vdwdup.u32	q1, r2, fp, #1
[^>]*> ee23 2f6a 	viwdup.u32	q1, r2, fp, #1
[^>]*> ee23 3f6b 	vdwdup.u32	q1, r2, fp, #2
[^>]*> ee23 2f6b 	viwdup.u32	q1, r2, fp, #2
[^>]*> ee23 3fea 	vdwdup.u32	q1, r2, fp, #4
[^>]*> ee23 2fea 	viwdup.u32	q1, r2, fp, #4
[^>]*> ee23 3feb 	vdwdup.u32	q1, r2, fp, #8
[^>]*> ee23 2feb 	viwdup.u32	q1, r2, fp, #8
[^>]*> ee25 3f6e 	vddup.u32	q1, r4, #1
[^>]*> ee25 2f6e 	vidup.u32	q1, r4, #1
[^>]*> ee25 3f6f 	vddup.u32	q1, r4, #2
[^>]*> ee25 2f6f 	vidup.u32	q1, r4, #2
[^>]*> ee25 3fee 	vddup.u32	q1, r4, #4
[^>]*> ee25 2fee 	vidup.u32	q1, r4, #4
[^>]*> ee25 3fef 	vddup.u32	q1, r4, #8
[^>]*> ee25 2fef 	vidup.u32	q1, r4, #8
[^>]*> ee25 3f60 	vdwdup.u32	q1, r4, r1, #1
[^>]*> ee25 2f60 	viwdup.u32	q1, r4, r1, #1
[^>]*> ee25 3f61 	vdwdup.u32	q1, r4, r1, #2
[^>]*> ee25 2f61 	viwdup.u32	q1, r4, r1, #2
[^>]*> ee25 3fe0 	vdwdup.u32	q1, r4, r1, #4
[^>]*> ee25 2fe0 	viwdup.u32	q1, r4, r1, #4
[^>]*> ee25 3fe1 	vdwdup.u32	q1, r4, r1, #8
[^>]*> ee25 2fe1 	viwdup.u32	q1, r4, r1, #8
[^>]*> ee25 3f62 	vdwdup.u32	q1, r4, r3, #1
[^>]*> ee25 2f62 	viwdup.u32	q1, r4, r3, #1
[^>]*> ee25 3f63 	vdwdup.u32	q1, r4, r3, #2
[^>]*> ee25 2f63 	viwdup.u32	q1, r4, r3, #2
[^>]*> ee25 3fe2 	vdwdup.u32	q1, r4, r3, #4
[^>]*> ee25 2fe2 	viwdup.u32	q1, r4, r3, #4
[^>]*> ee25 3fe3 	vdwdup.u32	q1, r4, r3, #8
[^>]*> ee25 2fe3 	viwdup.u32	q1, r4, r3, #8
[^>]*> ee25 3f64 	vdwdup.u32	q1, r4, r5, #1
[^>]*> ee25 2f64 	viwdup.u32	q1, r4, r5, #1
[^>]*> ee25 3f65 	vdwdup.u32	q1, r4, r5, #2
[^>]*> ee25 2f65 	viwdup.u32	q1, r4, r5, #2
[^>]*> ee25 3fe4 	vdwdup.u32	q1, r4, r5, #4
[^>]*> ee25 2fe4 	viwdup.u32	q1, r4, r5, #4
[^>]*> ee25 3fe5 	vdwdup.u32	q1, r4, r5, #8
[^>]*> ee25 2fe5 	viwdup.u32	q1, r4, r5, #8
[^>]*> ee25 3f66 	vdwdup.u32	q1, r4, r7, #1
[^>]*> ee25 2f66 	viwdup.u32	q1, r4, r7, #1
[^>]*> ee25 3f67 	vdwdup.u32	q1, r4, r7, #2
[^>]*> ee25 2f67 	viwdup.u32	q1, r4, r7, #2
[^>]*> ee25 3fe6 	vdwdup.u32	q1, r4, r7, #4
[^>]*> ee25 2fe6 	viwdup.u32	q1, r4, r7, #4
[^>]*> ee25 3fe7 	vdwdup.u32	q1, r4, r7, #8
[^>]*> ee25 2fe7 	viwdup.u32	q1, r4, r7, #8
[^>]*> ee25 3f68 	vdwdup.u32	q1, r4, r9, #1
[^>]*> ee25 2f68 	viwdup.u32	q1, r4, r9, #1
[^>]*> ee25 3f69 	vdwdup.u32	q1, r4, r9, #2
[^>]*> ee25 2f69 	viwdup.u32	q1, r4, r9, #2
[^>]*> ee25 3fe8 	vdwdup.u32	q1, r4, r9, #4
[^>]*> ee25 2fe8 	viwdup.u32	q1, r4, r9, #4
[^>]*> ee25 3fe9 	vdwdup.u32	q1, r4, r9, #8
[^>]*> ee25 2fe9 	viwdup.u32	q1, r4, r9, #8
[^>]*> ee25 3f6a 	vdwdup.u32	q1, r4, fp, #1
[^>]*> ee25 2f6a 	viwdup.u32	q1, r4, fp, #1
[^>]*> ee25 3f6b 	vdwdup.u32	q1, r4, fp, #2
[^>]*> ee25 2f6b 	viwdup.u32	q1, r4, fp, #2
[^>]*> ee25 3fea 	vdwdup.u32	q1, r4, fp, #4
[^>]*> ee25 2fea 	viwdup.u32	q1, r4, fp, #4
[^>]*> ee25 3feb 	vdwdup.u32	q1, r4, fp, #8
[^>]*> ee25 2feb 	viwdup.u32	q1, r4, fp, #8
[^>]*> ee27 3f6e 	vddup.u32	q1, r6, #1
[^>]*> ee27 2f6e 	vidup.u32	q1, r6, #1
[^>]*> ee27 3f6f 	vddup.u32	q1, r6, #2
[^>]*> ee27 2f6f 	vidup.u32	q1, r6, #2
[^>]*> ee27 3fee 	vddup.u32	q1, r6, #4
[^>]*> ee27 2fee 	vidup.u32	q1, r6, #4
[^>]*> ee27 3fef 	vddup.u32	q1, r6, #8
[^>]*> ee27 2fef 	vidup.u32	q1, r6, #8
[^>]*> ee27 3f60 	vdwdup.u32	q1, r6, r1, #1
[^>]*> ee27 2f60 	viwdup.u32	q1, r6, r1, #1
[^>]*> ee27 3f61 	vdwdup.u32	q1, r6, r1, #2
[^>]*> ee27 2f61 	viwdup.u32	q1, r6, r1, #2
[^>]*> ee27 3fe0 	vdwdup.u32	q1, r6, r1, #4
[^>]*> ee27 2fe0 	viwdup.u32	q1, r6, r1, #4
[^>]*> ee27 3fe1 	vdwdup.u32	q1, r6, r1, #8
[^>]*> ee27 2fe1 	viwdup.u32	q1, r6, r1, #8
[^>]*> ee27 3f62 	vdwdup.u32	q1, r6, r3, #1
[^>]*> ee27 2f62 	viwdup.u32	q1, r6, r3, #1
[^>]*> ee27 3f63 	vdwdup.u32	q1, r6, r3, #2
[^>]*> ee27 2f63 	viwdup.u32	q1, r6, r3, #2
[^>]*> ee27 3fe2 	vdwdup.u32	q1, r6, r3, #4
[^>]*> ee27 2fe2 	viwdup.u32	q1, r6, r3, #4
[^>]*> ee27 3fe3 	vdwdup.u32	q1, r6, r3, #8
[^>]*> ee27 2fe3 	viwdup.u32	q1, r6, r3, #8
[^>]*> ee27 3f64 	vdwdup.u32	q1, r6, r5, #1
[^>]*> ee27 2f64 	viwdup.u32	q1, r6, r5, #1
[^>]*> ee27 3f65 	vdwdup.u32	q1, r6, r5, #2
[^>]*> ee27 2f65 	viwdup.u32	q1, r6, r5, #2
[^>]*> ee27 3fe4 	vdwdup.u32	q1, r6, r5, #4
[^>]*> ee27 2fe4 	viwdup.u32	q1, r6, r5, #4
[^>]*> ee27 3fe5 	vdwdup.u32	q1, r6, r5, #8
[^>]*> ee27 2fe5 	viwdup.u32	q1, r6, r5, #8
[^>]*> ee27 3f66 	vdwdup.u32	q1, r6, r7, #1
[^>]*> ee27 2f66 	viwdup.u32	q1, r6, r7, #1
[^>]*> ee27 3f67 	vdwdup.u32	q1, r6, r7, #2
[^>]*> ee27 2f67 	viwdup.u32	q1, r6, r7, #2
[^>]*> ee27 3fe6 	vdwdup.u32	q1, r6, r7, #4
[^>]*> ee27 2fe6 	viwdup.u32	q1, r6, r7, #4
[^>]*> ee27 3fe7 	vdwdup.u32	q1, r6, r7, #8
[^>]*> ee27 2fe7 	viwdup.u32	q1, r6, r7, #8
[^>]*> ee27 3f68 	vdwdup.u32	q1, r6, r9, #1
[^>]*> ee27 2f68 	viwdup.u32	q1, r6, r9, #1
[^>]*> ee27 3f69 	vdwdup.u32	q1, r6, r9, #2
[^>]*> ee27 2f69 	viwdup.u32	q1, r6, r9, #2
[^>]*> ee27 3fe8 	vdwdup.u32	q1, r6, r9, #4
[^>]*> ee27 2fe8 	viwdup.u32	q1, r6, r9, #4
[^>]*> ee27 3fe9 	vdwdup.u32	q1, r6, r9, #8
[^>]*> ee27 2fe9 	viwdup.u32	q1, r6, r9, #8
[^>]*> ee27 3f6a 	vdwdup.u32	q1, r6, fp, #1
[^>]*> ee27 2f6a 	viwdup.u32	q1, r6, fp, #1
[^>]*> ee27 3f6b 	vdwdup.u32	q1, r6, fp, #2
[^>]*> ee27 2f6b 	viwdup.u32	q1, r6, fp, #2
[^>]*> ee27 3fea 	vdwdup.u32	q1, r6, fp, #4
[^>]*> ee27 2fea 	viwdup.u32	q1, r6, fp, #4
[^>]*> ee27 3feb 	vdwdup.u32	q1, r6, fp, #8
[^>]*> ee27 2feb 	viwdup.u32	q1, r6, fp, #8
[^>]*> ee29 3f6e 	vddup.u32	q1, r8, #1
[^>]*> ee29 2f6e 	vidup.u32	q1, r8, #1
[^>]*> ee29 3f6f 	vddup.u32	q1, r8, #2
[^>]*> ee29 2f6f 	vidup.u32	q1, r8, #2
[^>]*> ee29 3fee 	vddup.u32	q1, r8, #4
[^>]*> ee29 2fee 	vidup.u32	q1, r8, #4
[^>]*> ee29 3fef 	vddup.u32	q1, r8, #8
[^>]*> ee29 2fef 	vidup.u32	q1, r8, #8
[^>]*> ee29 3f60 	vdwdup.u32	q1, r8, r1, #1
[^>]*> ee29 2f60 	viwdup.u32	q1, r8, r1, #1
[^>]*> ee29 3f61 	vdwdup.u32	q1, r8, r1, #2
[^>]*> ee29 2f61 	viwdup.u32	q1, r8, r1, #2
[^>]*> ee29 3fe0 	vdwdup.u32	q1, r8, r1, #4
[^>]*> ee29 2fe0 	viwdup.u32	q1, r8, r1, #4
[^>]*> ee29 3fe1 	vdwdup.u32	q1, r8, r1, #8
[^>]*> ee29 2fe1 	viwdup.u32	q1, r8, r1, #8
[^>]*> ee29 3f62 	vdwdup.u32	q1, r8, r3, #1
[^>]*> ee29 2f62 	viwdup.u32	q1, r8, r3, #1
[^>]*> ee29 3f63 	vdwdup.u32	q1, r8, r3, #2
[^>]*> ee29 2f63 	viwdup.u32	q1, r8, r3, #2
[^>]*> ee29 3fe2 	vdwdup.u32	q1, r8, r3, #4
[^>]*> ee29 2fe2 	viwdup.u32	q1, r8, r3, #4
[^>]*> ee29 3fe3 	vdwdup.u32	q1, r8, r3, #8
[^>]*> ee29 2fe3 	viwdup.u32	q1, r8, r3, #8
[^>]*> ee29 3f64 	vdwdup.u32	q1, r8, r5, #1
[^>]*> ee29 2f64 	viwdup.u32	q1, r8, r5, #1
[^>]*> ee29 3f65 	vdwdup.u32	q1, r8, r5, #2
[^>]*> ee29 2f65 	viwdup.u32	q1, r8, r5, #2
[^>]*> ee29 3fe4 	vdwdup.u32	q1, r8, r5, #4
[^>]*> ee29 2fe4 	viwdup.u32	q1, r8, r5, #4
[^>]*> ee29 3fe5 	vdwdup.u32	q1, r8, r5, #8
[^>]*> ee29 2fe5 	viwdup.u32	q1, r8, r5, #8
[^>]*> ee29 3f66 	vdwdup.u32	q1, r8, r7, #1
[^>]*> ee29 2f66 	viwdup.u32	q1, r8, r7, #1
[^>]*> ee29 3f67 	vdwdup.u32	q1, r8, r7, #2
[^>]*> ee29 2f67 	viwdup.u32	q1, r8, r7, #2
[^>]*> ee29 3fe6 	vdwdup.u32	q1, r8, r7, #4
[^>]*> ee29 2fe6 	viwdup.u32	q1, r8, r7, #4
[^>]*> ee29 3fe7 	vdwdup.u32	q1, r8, r7, #8
[^>]*> ee29 2fe7 	viwdup.u32	q1, r8, r7, #8
[^>]*> ee29 3f68 	vdwdup.u32	q1, r8, r9, #1
[^>]*> ee29 2f68 	viwdup.u32	q1, r8, r9, #1
[^>]*> ee29 3f69 	vdwdup.u32	q1, r8, r9, #2
[^>]*> ee29 2f69 	viwdup.u32	q1, r8, r9, #2
[^>]*> ee29 3fe8 	vdwdup.u32	q1, r8, r9, #4
[^>]*> ee29 2fe8 	viwdup.u32	q1, r8, r9, #4
[^>]*> ee29 3fe9 	vdwdup.u32	q1, r8, r9, #8
[^>]*> ee29 2fe9 	viwdup.u32	q1, r8, r9, #8
[^>]*> ee29 3f6a 	vdwdup.u32	q1, r8, fp, #1
[^>]*> ee29 2f6a 	viwdup.u32	q1, r8, fp, #1
[^>]*> ee29 3f6b 	vdwdup.u32	q1, r8, fp, #2
[^>]*> ee29 2f6b 	viwdup.u32	q1, r8, fp, #2
[^>]*> ee29 3fea 	vdwdup.u32	q1, r8, fp, #4
[^>]*> ee29 2fea 	viwdup.u32	q1, r8, fp, #4
[^>]*> ee29 3feb 	vdwdup.u32	q1, r8, fp, #8
[^>]*> ee29 2feb 	viwdup.u32	q1, r8, fp, #8
[^>]*> ee2b 3f6e 	vddup.u32	q1, sl, #1
[^>]*> ee2b 2f6e 	vidup.u32	q1, sl, #1
[^>]*> ee2b 3f6f 	vddup.u32	q1, sl, #2
[^>]*> ee2b 2f6f 	vidup.u32	q1, sl, #2
[^>]*> ee2b 3fee 	vddup.u32	q1, sl, #4
[^>]*> ee2b 2fee 	vidup.u32	q1, sl, #4
[^>]*> ee2b 3fef 	vddup.u32	q1, sl, #8
[^>]*> ee2b 2fef 	vidup.u32	q1, sl, #8
[^>]*> ee2b 3f60 	vdwdup.u32	q1, sl, r1, #1
[^>]*> ee2b 2f60 	viwdup.u32	q1, sl, r1, #1
[^>]*> ee2b 3f61 	vdwdup.u32	q1, sl, r1, #2
[^>]*> ee2b 2f61 	viwdup.u32	q1, sl, r1, #2
[^>]*> ee2b 3fe0 	vdwdup.u32	q1, sl, r1, #4
[^>]*> ee2b 2fe0 	viwdup.u32	q1, sl, r1, #4
[^>]*> ee2b 3fe1 	vdwdup.u32	q1, sl, r1, #8
[^>]*> ee2b 2fe1 	viwdup.u32	q1, sl, r1, #8
[^>]*> ee2b 3f62 	vdwdup.u32	q1, sl, r3, #1
[^>]*> ee2b 2f62 	viwdup.u32	q1, sl, r3, #1
[^>]*> ee2b 3f63 	vdwdup.u32	q1, sl, r3, #2
[^>]*> ee2b 2f63 	viwdup.u32	q1, sl, r3, #2
[^>]*> ee2b 3fe2 	vdwdup.u32	q1, sl, r3, #4
[^>]*> ee2b 2fe2 	viwdup.u32	q1, sl, r3, #4
[^>]*> ee2b 3fe3 	vdwdup.u32	q1, sl, r3, #8
[^>]*> ee2b 2fe3 	viwdup.u32	q1, sl, r3, #8
[^>]*> ee2b 3f64 	vdwdup.u32	q1, sl, r5, #1
[^>]*> ee2b 2f64 	viwdup.u32	q1, sl, r5, #1
[^>]*> ee2b 3f65 	vdwdup.u32	q1, sl, r5, #2
[^>]*> ee2b 2f65 	viwdup.u32	q1, sl, r5, #2
[^>]*> ee2b 3fe4 	vdwdup.u32	q1, sl, r5, #4
[^>]*> ee2b 2fe4 	viwdup.u32	q1, sl, r5, #4
[^>]*> ee2b 3fe5 	vdwdup.u32	q1, sl, r5, #8
[^>]*> ee2b 2fe5 	viwdup.u32	q1, sl, r5, #8
[^>]*> ee2b 3f66 	vdwdup.u32	q1, sl, r7, #1
[^>]*> ee2b 2f66 	viwdup.u32	q1, sl, r7, #1
[^>]*> ee2b 3f67 	vdwdup.u32	q1, sl, r7, #2
[^>]*> ee2b 2f67 	viwdup.u32	q1, sl, r7, #2
[^>]*> ee2b 3fe6 	vdwdup.u32	q1, sl, r7, #4
[^>]*> ee2b 2fe6 	viwdup.u32	q1, sl, r7, #4
[^>]*> ee2b 3fe7 	vdwdup.u32	q1, sl, r7, #8
[^>]*> ee2b 2fe7 	viwdup.u32	q1, sl, r7, #8
[^>]*> ee2b 3f68 	vdwdup.u32	q1, sl, r9, #1
[^>]*> ee2b 2f68 	viwdup.u32	q1, sl, r9, #1
[^>]*> ee2b 3f69 	vdwdup.u32	q1, sl, r9, #2
[^>]*> ee2b 2f69 	viwdup.u32	q1, sl, r9, #2
[^>]*> ee2b 3fe8 	vdwdup.u32	q1, sl, r9, #4
[^>]*> ee2b 2fe8 	viwdup.u32	q1, sl, r9, #4
[^>]*> ee2b 3fe9 	vdwdup.u32	q1, sl, r9, #8
[^>]*> ee2b 2fe9 	viwdup.u32	q1, sl, r9, #8
[^>]*> ee2b 3f6a 	vdwdup.u32	q1, sl, fp, #1
[^>]*> ee2b 2f6a 	viwdup.u32	q1, sl, fp, #1
[^>]*> ee2b 3f6b 	vdwdup.u32	q1, sl, fp, #2
[^>]*> ee2b 2f6b 	viwdup.u32	q1, sl, fp, #2
[^>]*> ee2b 3fea 	vdwdup.u32	q1, sl, fp, #4
[^>]*> ee2b 2fea 	viwdup.u32	q1, sl, fp, #4
[^>]*> ee2b 3feb 	vdwdup.u32	q1, sl, fp, #8
[^>]*> ee2b 2feb 	viwdup.u32	q1, sl, fp, #8
[^>]*> ee2d 3f6e 	vddup.u32	q1, ip, #1
[^>]*> ee2d 2f6e 	vidup.u32	q1, ip, #1
[^>]*> ee2d 3f6f 	vddup.u32	q1, ip, #2
[^>]*> ee2d 2f6f 	vidup.u32	q1, ip, #2
[^>]*> ee2d 3fee 	vddup.u32	q1, ip, #4
[^>]*> ee2d 2fee 	vidup.u32	q1, ip, #4
[^>]*> ee2d 3fef 	vddup.u32	q1, ip, #8
[^>]*> ee2d 2fef 	vidup.u32	q1, ip, #8
[^>]*> ee2d 3f60 	vdwdup.u32	q1, ip, r1, #1
[^>]*> ee2d 2f60 	viwdup.u32	q1, ip, r1, #1
[^>]*> ee2d 3f61 	vdwdup.u32	q1, ip, r1, #2
[^>]*> ee2d 2f61 	viwdup.u32	q1, ip, r1, #2
[^>]*> ee2d 3fe0 	vdwdup.u32	q1, ip, r1, #4
[^>]*> ee2d 2fe0 	viwdup.u32	q1, ip, r1, #4
[^>]*> ee2d 3fe1 	vdwdup.u32	q1, ip, r1, #8
[^>]*> ee2d 2fe1 	viwdup.u32	q1, ip, r1, #8
[^>]*> ee2d 3f62 	vdwdup.u32	q1, ip, r3, #1
[^>]*> ee2d 2f62 	viwdup.u32	q1, ip, r3, #1
[^>]*> ee2d 3f63 	vdwdup.u32	q1, ip, r3, #2
[^>]*> ee2d 2f63 	viwdup.u32	q1, ip, r3, #2
[^>]*> ee2d 3fe2 	vdwdup.u32	q1, ip, r3, #4
[^>]*> ee2d 2fe2 	viwdup.u32	q1, ip, r3, #4
[^>]*> ee2d 3fe3 	vdwdup.u32	q1, ip, r3, #8
[^>]*> ee2d 2fe3 	viwdup.u32	q1, ip, r3, #8
[^>]*> ee2d 3f64 	vdwdup.u32	q1, ip, r5, #1
[^>]*> ee2d 2f64 	viwdup.u32	q1, ip, r5, #1
[^>]*> ee2d 3f65 	vdwdup.u32	q1, ip, r5, #2
[^>]*> ee2d 2f65 	viwdup.u32	q1, ip, r5, #2
[^>]*> ee2d 3fe4 	vdwdup.u32	q1, ip, r5, #4
[^>]*> ee2d 2fe4 	viwdup.u32	q1, ip, r5, #4
[^>]*> ee2d 3fe5 	vdwdup.u32	q1, ip, r5, #8
[^>]*> ee2d 2fe5 	viwdup.u32	q1, ip, r5, #8
[^>]*> ee2d 3f66 	vdwdup.u32	q1, ip, r7, #1
[^>]*> ee2d 2f66 	viwdup.u32	q1, ip, r7, #1
[^>]*> ee2d 3f67 	vdwdup.u32	q1, ip, r7, #2
[^>]*> ee2d 2f67 	viwdup.u32	q1, ip, r7, #2
[^>]*> ee2d 3fe6 	vdwdup.u32	q1, ip, r7, #4
[^>]*> ee2d 2fe6 	viwdup.u32	q1, ip, r7, #4
[^>]*> ee2d 3fe7 	vdwdup.u32	q1, ip, r7, #8
[^>]*> ee2d 2fe7 	viwdup.u32	q1, ip, r7, #8
[^>]*> ee2d 3f68 	vdwdup.u32	q1, ip, r9, #1
[^>]*> ee2d 2f68 	viwdup.u32	q1, ip, r9, #1
[^>]*> ee2d 3f69 	vdwdup.u32	q1, ip, r9, #2
[^>]*> ee2d 2f69 	viwdup.u32	q1, ip, r9, #2
[^>]*> ee2d 3fe8 	vdwdup.u32	q1, ip, r9, #4
[^>]*> ee2d 2fe8 	viwdup.u32	q1, ip, r9, #4
[^>]*> ee2d 3fe9 	vdwdup.u32	q1, ip, r9, #8
[^>]*> ee2d 2fe9 	viwdup.u32	q1, ip, r9, #8
[^>]*> ee2d 3f6a 	vdwdup.u32	q1, ip, fp, #1
[^>]*> ee2d 2f6a 	viwdup.u32	q1, ip, fp, #1
[^>]*> ee2d 3f6b 	vdwdup.u32	q1, ip, fp, #2
[^>]*> ee2d 2f6b 	viwdup.u32	q1, ip, fp, #2
[^>]*> ee2d 3fea 	vdwdup.u32	q1, ip, fp, #4
[^>]*> ee2d 2fea 	viwdup.u32	q1, ip, fp, #4
[^>]*> ee2d 3feb 	vdwdup.u32	q1, ip, fp, #8
[^>]*> ee2d 2feb 	viwdup.u32	q1, ip, fp, #8
[^>]*> ee21 5f6e 	vddup.u32	q2, r0, #1
[^>]*> ee21 4f6e 	vidup.u32	q2, r0, #1
[^>]*> ee21 5f6f 	vddup.u32	q2, r0, #2
[^>]*> ee21 4f6f 	vidup.u32	q2, r0, #2
[^>]*> ee21 5fee 	vddup.u32	q2, r0, #4
[^>]*> ee21 4fee 	vidup.u32	q2, r0, #4
[^>]*> ee21 5fef 	vddup.u32	q2, r0, #8
[^>]*> ee21 4fef 	vidup.u32	q2, r0, #8
[^>]*> ee21 5f60 	vdwdup.u32	q2, r0, r1, #1
[^>]*> ee21 4f60 	viwdup.u32	q2, r0, r1, #1
[^>]*> ee21 5f61 	vdwdup.u32	q2, r0, r1, #2
[^>]*> ee21 4f61 	viwdup.u32	q2, r0, r1, #2
[^>]*> ee21 5fe0 	vdwdup.u32	q2, r0, r1, #4
[^>]*> ee21 4fe0 	viwdup.u32	q2, r0, r1, #4
[^>]*> ee21 5fe1 	vdwdup.u32	q2, r0, r1, #8
[^>]*> ee21 4fe1 	viwdup.u32	q2, r0, r1, #8
[^>]*> ee21 5f62 	vdwdup.u32	q2, r0, r3, #1
[^>]*> ee21 4f62 	viwdup.u32	q2, r0, r3, #1
[^>]*> ee21 5f63 	vdwdup.u32	q2, r0, r3, #2
[^>]*> ee21 4f63 	viwdup.u32	q2, r0, r3, #2
[^>]*> ee21 5fe2 	vdwdup.u32	q2, r0, r3, #4
[^>]*> ee21 4fe2 	viwdup.u32	q2, r0, r3, #4
[^>]*> ee21 5fe3 	vdwdup.u32	q2, r0, r3, #8
[^>]*> ee21 4fe3 	viwdup.u32	q2, r0, r3, #8
[^>]*> ee21 5f64 	vdwdup.u32	q2, r0, r5, #1
[^>]*> ee21 4f64 	viwdup.u32	q2, r0, r5, #1
[^>]*> ee21 5f65 	vdwdup.u32	q2, r0, r5, #2
[^>]*> ee21 4f65 	viwdup.u32	q2, r0, r5, #2
[^>]*> ee21 5fe4 	vdwdup.u32	q2, r0, r5, #4
[^>]*> ee21 4fe4 	viwdup.u32	q2, r0, r5, #4
[^>]*> ee21 5fe5 	vdwdup.u32	q2, r0, r5, #8
[^>]*> ee21 4fe5 	viwdup.u32	q2, r0, r5, #8
[^>]*> ee21 5f66 	vdwdup.u32	q2, r0, r7, #1
[^>]*> ee21 4f66 	viwdup.u32	q2, r0, r7, #1
[^>]*> ee21 5f67 	vdwdup.u32	q2, r0, r7, #2
[^>]*> ee21 4f67 	viwdup.u32	q2, r0, r7, #2
[^>]*> ee21 5fe6 	vdwdup.u32	q2, r0, r7, #4
[^>]*> ee21 4fe6 	viwdup.u32	q2, r0, r7, #4
[^>]*> ee21 5fe7 	vdwdup.u32	q2, r0, r7, #8
[^>]*> ee21 4fe7 	viwdup.u32	q2, r0, r7, #8
[^>]*> ee21 5f68 	vdwdup.u32	q2, r0, r9, #1
[^>]*> ee21 4f68 	viwdup.u32	q2, r0, r9, #1
[^>]*> ee21 5f69 	vdwdup.u32	q2, r0, r9, #2
[^>]*> ee21 4f69 	viwdup.u32	q2, r0, r9, #2
[^>]*> ee21 5fe8 	vdwdup.u32	q2, r0, r9, #4
[^>]*> ee21 4fe8 	viwdup.u32	q2, r0, r9, #4
[^>]*> ee21 5fe9 	vdwdup.u32	q2, r0, r9, #8
[^>]*> ee21 4fe9 	viwdup.u32	q2, r0, r9, #8
[^>]*> ee21 5f6a 	vdwdup.u32	q2, r0, fp, #1
[^>]*> ee21 4f6a 	viwdup.u32	q2, r0, fp, #1
[^>]*> ee21 5f6b 	vdwdup.u32	q2, r0, fp, #2
[^>]*> ee21 4f6b 	viwdup.u32	q2, r0, fp, #2
[^>]*> ee21 5fea 	vdwdup.u32	q2, r0, fp, #4
[^>]*> ee21 4fea 	viwdup.u32	q2, r0, fp, #4
[^>]*> ee21 5feb 	vdwdup.u32	q2, r0, fp, #8
[^>]*> ee21 4feb 	viwdup.u32	q2, r0, fp, #8
[^>]*> ee23 5f6e 	vddup.u32	q2, r2, #1
[^>]*> ee23 4f6e 	vidup.u32	q2, r2, #1
[^>]*> ee23 5f6f 	vddup.u32	q2, r2, #2
[^>]*> ee23 4f6f 	vidup.u32	q2, r2, #2
[^>]*> ee23 5fee 	vddup.u32	q2, r2, #4
[^>]*> ee23 4fee 	vidup.u32	q2, r2, #4
[^>]*> ee23 5fef 	vddup.u32	q2, r2, #8
[^>]*> ee23 4fef 	vidup.u32	q2, r2, #8
[^>]*> ee23 5f60 	vdwdup.u32	q2, r2, r1, #1
[^>]*> ee23 4f60 	viwdup.u32	q2, r2, r1, #1
[^>]*> ee23 5f61 	vdwdup.u32	q2, r2, r1, #2
[^>]*> ee23 4f61 	viwdup.u32	q2, r2, r1, #2
[^>]*> ee23 5fe0 	vdwdup.u32	q2, r2, r1, #4
[^>]*> ee23 4fe0 	viwdup.u32	q2, r2, r1, #4
[^>]*> ee23 5fe1 	vdwdup.u32	q2, r2, r1, #8
[^>]*> ee23 4fe1 	viwdup.u32	q2, r2, r1, #8
[^>]*> ee23 5f62 	vdwdup.u32	q2, r2, r3, #1
[^>]*> ee23 4f62 	viwdup.u32	q2, r2, r3, #1
[^>]*> ee23 5f63 	vdwdup.u32	q2, r2, r3, #2
[^>]*> ee23 4f63 	viwdup.u32	q2, r2, r3, #2
[^>]*> ee23 5fe2 	vdwdup.u32	q2, r2, r3, #4
[^>]*> ee23 4fe2 	viwdup.u32	q2, r2, r3, #4
[^>]*> ee23 5fe3 	vdwdup.u32	q2, r2, r3, #8
[^>]*> ee23 4fe3 	viwdup.u32	q2, r2, r3, #8
[^>]*> ee23 5f64 	vdwdup.u32	q2, r2, r5, #1
[^>]*> ee23 4f64 	viwdup.u32	q2, r2, r5, #1
[^>]*> ee23 5f65 	vdwdup.u32	q2, r2, r5, #2
[^>]*> ee23 4f65 	viwdup.u32	q2, r2, r5, #2
[^>]*> ee23 5fe4 	vdwdup.u32	q2, r2, r5, #4
[^>]*> ee23 4fe4 	viwdup.u32	q2, r2, r5, #4
[^>]*> ee23 5fe5 	vdwdup.u32	q2, r2, r5, #8
[^>]*> ee23 4fe5 	viwdup.u32	q2, r2, r5, #8
[^>]*> ee23 5f66 	vdwdup.u32	q2, r2, r7, #1
[^>]*> ee23 4f66 	viwdup.u32	q2, r2, r7, #1
[^>]*> ee23 5f67 	vdwdup.u32	q2, r2, r7, #2
[^>]*> ee23 4f67 	viwdup.u32	q2, r2, r7, #2
[^>]*> ee23 5fe6 	vdwdup.u32	q2, r2, r7, #4
[^>]*> ee23 4fe6 	viwdup.u32	q2, r2, r7, #4
[^>]*> ee23 5fe7 	vdwdup.u32	q2, r2, r7, #8
[^>]*> ee23 4fe7 	viwdup.u32	q2, r2, r7, #8
[^>]*> ee23 5f68 	vdwdup.u32	q2, r2, r9, #1
[^>]*> ee23 4f68 	viwdup.u32	q2, r2, r9, #1
[^>]*> ee23 5f69 	vdwdup.u32	q2, r2, r9, #2
[^>]*> ee23 4f69 	viwdup.u32	q2, r2, r9, #2
[^>]*> ee23 5fe8 	vdwdup.u32	q2, r2, r9, #4
[^>]*> ee23 4fe8 	viwdup.u32	q2, r2, r9, #4
[^>]*> ee23 5fe9 	vdwdup.u32	q2, r2, r9, #8
[^>]*> ee23 4fe9 	viwdup.u32	q2, r2, r9, #8
[^>]*> ee23 5f6a 	vdwdup.u32	q2, r2, fp, #1
[^>]*> ee23 4f6a 	viwdup.u32	q2, r2, fp, #1
[^>]*> ee23 5f6b 	vdwdup.u32	q2, r2, fp, #2
[^>]*> ee23 4f6b 	viwdup.u32	q2, r2, fp, #2
[^>]*> ee23 5fea 	vdwdup.u32	q2, r2, fp, #4
[^>]*> ee23 4fea 	viwdup.u32	q2, r2, fp, #4
[^>]*> ee23 5feb 	vdwdup.u32	q2, r2, fp, #8
[^>]*> ee23 4feb 	viwdup.u32	q2, r2, fp, #8
[^>]*> ee25 5f6e 	vddup.u32	q2, r4, #1
[^>]*> ee25 4f6e 	vidup.u32	q2, r4, #1
[^>]*> ee25 5f6f 	vddup.u32	q2, r4, #2
[^>]*> ee25 4f6f 	vidup.u32	q2, r4, #2
[^>]*> ee25 5fee 	vddup.u32	q2, r4, #4
[^>]*> ee25 4fee 	vidup.u32	q2, r4, #4
[^>]*> ee25 5fef 	vddup.u32	q2, r4, #8
[^>]*> ee25 4fef 	vidup.u32	q2, r4, #8
[^>]*> ee25 5f60 	vdwdup.u32	q2, r4, r1, #1
[^>]*> ee25 4f60 	viwdup.u32	q2, r4, r1, #1
[^>]*> ee25 5f61 	vdwdup.u32	q2, r4, r1, #2
[^>]*> ee25 4f61 	viwdup.u32	q2, r4, r1, #2
[^>]*> ee25 5fe0 	vdwdup.u32	q2, r4, r1, #4
[^>]*> ee25 4fe0 	viwdup.u32	q2, r4, r1, #4
[^>]*> ee25 5fe1 	vdwdup.u32	q2, r4, r1, #8
[^>]*> ee25 4fe1 	viwdup.u32	q2, r4, r1, #8
[^>]*> ee25 5f62 	vdwdup.u32	q2, r4, r3, #1
[^>]*> ee25 4f62 	viwdup.u32	q2, r4, r3, #1
[^>]*> ee25 5f63 	vdwdup.u32	q2, r4, r3, #2
[^>]*> ee25 4f63 	viwdup.u32	q2, r4, r3, #2
[^>]*> ee25 5fe2 	vdwdup.u32	q2, r4, r3, #4
[^>]*> ee25 4fe2 	viwdup.u32	q2, r4, r3, #4
[^>]*> ee25 5fe3 	vdwdup.u32	q2, r4, r3, #8
[^>]*> ee25 4fe3 	viwdup.u32	q2, r4, r3, #8
[^>]*> ee25 5f64 	vdwdup.u32	q2, r4, r5, #1
[^>]*> ee25 4f64 	viwdup.u32	q2, r4, r5, #1
[^>]*> ee25 5f65 	vdwdup.u32	q2, r4, r5, #2
[^>]*> ee25 4f65 	viwdup.u32	q2, r4, r5, #2
[^>]*> ee25 5fe4 	vdwdup.u32	q2, r4, r5, #4
[^>]*> ee25 4fe4 	viwdup.u32	q2, r4, r5, #4
[^>]*> ee25 5fe5 	vdwdup.u32	q2, r4, r5, #8
[^>]*> ee25 4fe5 	viwdup.u32	q2, r4, r5, #8
[^>]*> ee25 5f66 	vdwdup.u32	q2, r4, r7, #1
[^>]*> ee25 4f66 	viwdup.u32	q2, r4, r7, #1
[^>]*> ee25 5f67 	vdwdup.u32	q2, r4, r7, #2
[^>]*> ee25 4f67 	viwdup.u32	q2, r4, r7, #2
[^>]*> ee25 5fe6 	vdwdup.u32	q2, r4, r7, #4
[^>]*> ee25 4fe6 	viwdup.u32	q2, r4, r7, #4
[^>]*> ee25 5fe7 	vdwdup.u32	q2, r4, r7, #8
[^>]*> ee25 4fe7 	viwdup.u32	q2, r4, r7, #8
[^>]*> ee25 5f68 	vdwdup.u32	q2, r4, r9, #1
[^>]*> ee25 4f68 	viwdup.u32	q2, r4, r9, #1
[^>]*> ee25 5f69 	vdwdup.u32	q2, r4, r9, #2
[^>]*> ee25 4f69 	viwdup.u32	q2, r4, r9, #2
[^>]*> ee25 5fe8 	vdwdup.u32	q2, r4, r9, #4
[^>]*> ee25 4fe8 	viwdup.u32	q2, r4, r9, #4
[^>]*> ee25 5fe9 	vdwdup.u32	q2, r4, r9, #8
[^>]*> ee25 4fe9 	viwdup.u32	q2, r4, r9, #8
[^>]*> ee25 5f6a 	vdwdup.u32	q2, r4, fp, #1
[^>]*> ee25 4f6a 	viwdup.u32	q2, r4, fp, #1
[^>]*> ee25 5f6b 	vdwdup.u32	q2, r4, fp, #2
[^>]*> ee25 4f6b 	viwdup.u32	q2, r4, fp, #2
[^>]*> ee25 5fea 	vdwdup.u32	q2, r4, fp, #4
[^>]*> ee25 4fea 	viwdup.u32	q2, r4, fp, #4
[^>]*> ee25 5feb 	vdwdup.u32	q2, r4, fp, #8
[^>]*> ee25 4feb 	viwdup.u32	q2, r4, fp, #8
[^>]*> ee27 5f6e 	vddup.u32	q2, r6, #1
[^>]*> ee27 4f6e 	vidup.u32	q2, r6, #1
[^>]*> ee27 5f6f 	vddup.u32	q2, r6, #2
[^>]*> ee27 4f6f 	vidup.u32	q2, r6, #2
[^>]*> ee27 5fee 	vddup.u32	q2, r6, #4
[^>]*> ee27 4fee 	vidup.u32	q2, r6, #4
[^>]*> ee27 5fef 	vddup.u32	q2, r6, #8
[^>]*> ee27 4fef 	vidup.u32	q2, r6, #8
[^>]*> ee27 5f60 	vdwdup.u32	q2, r6, r1, #1
[^>]*> ee27 4f60 	viwdup.u32	q2, r6, r1, #1
[^>]*> ee27 5f61 	vdwdup.u32	q2, r6, r1, #2
[^>]*> ee27 4f61 	viwdup.u32	q2, r6, r1, #2
[^>]*> ee27 5fe0 	vdwdup.u32	q2, r6, r1, #4
[^>]*> ee27 4fe0 	viwdup.u32	q2, r6, r1, #4
[^>]*> ee27 5fe1 	vdwdup.u32	q2, r6, r1, #8
[^>]*> ee27 4fe1 	viwdup.u32	q2, r6, r1, #8
[^>]*> ee27 5f62 	vdwdup.u32	q2, r6, r3, #1
[^>]*> ee27 4f62 	viwdup.u32	q2, r6, r3, #1
[^>]*> ee27 5f63 	vdwdup.u32	q2, r6, r3, #2
[^>]*> ee27 4f63 	viwdup.u32	q2, r6, r3, #2
[^>]*> ee27 5fe2 	vdwdup.u32	q2, r6, r3, #4
[^>]*> ee27 4fe2 	viwdup.u32	q2, r6, r3, #4
[^>]*> ee27 5fe3 	vdwdup.u32	q2, r6, r3, #8
[^>]*> ee27 4fe3 	viwdup.u32	q2, r6, r3, #8
[^>]*> ee27 5f64 	vdwdup.u32	q2, r6, r5, #1
[^>]*> ee27 4f64 	viwdup.u32	q2, r6, r5, #1
[^>]*> ee27 5f65 	vdwdup.u32	q2, r6, r5, #2
[^>]*> ee27 4f65 	viwdup.u32	q2, r6, r5, #2
[^>]*> ee27 5fe4 	vdwdup.u32	q2, r6, r5, #4
[^>]*> ee27 4fe4 	viwdup.u32	q2, r6, r5, #4
[^>]*> ee27 5fe5 	vdwdup.u32	q2, r6, r5, #8
[^>]*> ee27 4fe5 	viwdup.u32	q2, r6, r5, #8
[^>]*> ee27 5f66 	vdwdup.u32	q2, r6, r7, #1
[^>]*> ee27 4f66 	viwdup.u32	q2, r6, r7, #1
[^>]*> ee27 5f67 	vdwdup.u32	q2, r6, r7, #2
[^>]*> ee27 4f67 	viwdup.u32	q2, r6, r7, #2
[^>]*> ee27 5fe6 	vdwdup.u32	q2, r6, r7, #4
[^>]*> ee27 4fe6 	viwdup.u32	q2, r6, r7, #4
[^>]*> ee27 5fe7 	vdwdup.u32	q2, r6, r7, #8
[^>]*> ee27 4fe7 	viwdup.u32	q2, r6, r7, #8
[^>]*> ee27 5f68 	vdwdup.u32	q2, r6, r9, #1
[^>]*> ee27 4f68 	viwdup.u32	q2, r6, r9, #1
[^>]*> ee27 5f69 	vdwdup.u32	q2, r6, r9, #2
[^>]*> ee27 4f69 	viwdup.u32	q2, r6, r9, #2
[^>]*> ee27 5fe8 	vdwdup.u32	q2, r6, r9, #4
[^>]*> ee27 4fe8 	viwdup.u32	q2, r6, r9, #4
[^>]*> ee27 5fe9 	vdwdup.u32	q2, r6, r9, #8
[^>]*> ee27 4fe9 	viwdup.u32	q2, r6, r9, #8
[^>]*> ee27 5f6a 	vdwdup.u32	q2, r6, fp, #1
[^>]*> ee27 4f6a 	viwdup.u32	q2, r6, fp, #1
[^>]*> ee27 5f6b 	vdwdup.u32	q2, r6, fp, #2
[^>]*> ee27 4f6b 	viwdup.u32	q2, r6, fp, #2
[^>]*> ee27 5fea 	vdwdup.u32	q2, r6, fp, #4
[^>]*> ee27 4fea 	viwdup.u32	q2, r6, fp, #4
[^>]*> ee27 5feb 	vdwdup.u32	q2, r6, fp, #8
[^>]*> ee27 4feb 	viwdup.u32	q2, r6, fp, #8
[^>]*> ee29 5f6e 	vddup.u32	q2, r8, #1
[^>]*> ee29 4f6e 	vidup.u32	q2, r8, #1
[^>]*> ee29 5f6f 	vddup.u32	q2, r8, #2
[^>]*> ee29 4f6f 	vidup.u32	q2, r8, #2
[^>]*> ee29 5fee 	vddup.u32	q2, r8, #4
[^>]*> ee29 4fee 	vidup.u32	q2, r8, #4
[^>]*> ee29 5fef 	vddup.u32	q2, r8, #8
[^>]*> ee29 4fef 	vidup.u32	q2, r8, #8
[^>]*> ee29 5f60 	vdwdup.u32	q2, r8, r1, #1
[^>]*> ee29 4f60 	viwdup.u32	q2, r8, r1, #1
[^>]*> ee29 5f61 	vdwdup.u32	q2, r8, r1, #2
[^>]*> ee29 4f61 	viwdup.u32	q2, r8, r1, #2
[^>]*> ee29 5fe0 	vdwdup.u32	q2, r8, r1, #4
[^>]*> ee29 4fe0 	viwdup.u32	q2, r8, r1, #4
[^>]*> ee29 5fe1 	vdwdup.u32	q2, r8, r1, #8
[^>]*> ee29 4fe1 	viwdup.u32	q2, r8, r1, #8
[^>]*> ee29 5f62 	vdwdup.u32	q2, r8, r3, #1
[^>]*> ee29 4f62 	viwdup.u32	q2, r8, r3, #1
[^>]*> ee29 5f63 	vdwdup.u32	q2, r8, r3, #2
[^>]*> ee29 4f63 	viwdup.u32	q2, r8, r3, #2
[^>]*> ee29 5fe2 	vdwdup.u32	q2, r8, r3, #4
[^>]*> ee29 4fe2 	viwdup.u32	q2, r8, r3, #4
[^>]*> ee29 5fe3 	vdwdup.u32	q2, r8, r3, #8
[^>]*> ee29 4fe3 	viwdup.u32	q2, r8, r3, #8
[^>]*> ee29 5f64 	vdwdup.u32	q2, r8, r5, #1
[^>]*> ee29 4f64 	viwdup.u32	q2, r8, r5, #1
[^>]*> ee29 5f65 	vdwdup.u32	q2, r8, r5, #2
[^>]*> ee29 4f65 	viwdup.u32	q2, r8, r5, #2
[^>]*> ee29 5fe4 	vdwdup.u32	q2, r8, r5, #4
[^>]*> ee29 4fe4 	viwdup.u32	q2, r8, r5, #4
[^>]*> ee29 5fe5 	vdwdup.u32	q2, r8, r5, #8
[^>]*> ee29 4fe5 	viwdup.u32	q2, r8, r5, #8
[^>]*> ee29 5f66 	vdwdup.u32	q2, r8, r7, #1
[^>]*> ee29 4f66 	viwdup.u32	q2, r8, r7, #1
[^>]*> ee29 5f67 	vdwdup.u32	q2, r8, r7, #2
[^>]*> ee29 4f67 	viwdup.u32	q2, r8, r7, #2
[^>]*> ee29 5fe6 	vdwdup.u32	q2, r8, r7, #4
[^>]*> ee29 4fe6 	viwdup.u32	q2, r8, r7, #4
[^>]*> ee29 5fe7 	vdwdup.u32	q2, r8, r7, #8
[^>]*> ee29 4fe7 	viwdup.u32	q2, r8, r7, #8
[^>]*> ee29 5f68 	vdwdup.u32	q2, r8, r9, #1
[^>]*> ee29 4f68 	viwdup.u32	q2, r8, r9, #1
[^>]*> ee29 5f69 	vdwdup.u32	q2, r8, r9, #2
[^>]*> ee29 4f69 	viwdup.u32	q2, r8, r9, #2
[^>]*> ee29 5fe8 	vdwdup.u32	q2, r8, r9, #4
[^>]*> ee29 4fe8 	viwdup.u32	q2, r8, r9, #4
[^>]*> ee29 5fe9 	vdwdup.u32	q2, r8, r9, #8
[^>]*> ee29 4fe9 	viwdup.u32	q2, r8, r9, #8
[^>]*> ee29 5f6a 	vdwdup.u32	q2, r8, fp, #1
[^>]*> ee29 4f6a 	viwdup.u32	q2, r8, fp, #1
[^>]*> ee29 5f6b 	vdwdup.u32	q2, r8, fp, #2
[^>]*> ee29 4f6b 	viwdup.u32	q2, r8, fp, #2
[^>]*> ee29 5fea 	vdwdup.u32	q2, r8, fp, #4
[^>]*> ee29 4fea 	viwdup.u32	q2, r8, fp, #4
[^>]*> ee29 5feb 	vdwdup.u32	q2, r8, fp, #8
[^>]*> ee29 4feb 	viwdup.u32	q2, r8, fp, #8
[^>]*> ee2b 5f6e 	vddup.u32	q2, sl, #1
[^>]*> ee2b 4f6e 	vidup.u32	q2, sl, #1
[^>]*> ee2b 5f6f 	vddup.u32	q2, sl, #2
[^>]*> ee2b 4f6f 	vidup.u32	q2, sl, #2
[^>]*> ee2b 5fee 	vddup.u32	q2, sl, #4
[^>]*> ee2b 4fee 	vidup.u32	q2, sl, #4
[^>]*> ee2b 5fef 	vddup.u32	q2, sl, #8
[^>]*> ee2b 4fef 	vidup.u32	q2, sl, #8
[^>]*> ee2b 5f60 	vdwdup.u32	q2, sl, r1, #1
[^>]*> ee2b 4f60 	viwdup.u32	q2, sl, r1, #1
[^>]*> ee2b 5f61 	vdwdup.u32	q2, sl, r1, #2
[^>]*> ee2b 4f61 	viwdup.u32	q2, sl, r1, #2
[^>]*> ee2b 5fe0 	vdwdup.u32	q2, sl, r1, #4
[^>]*> ee2b 4fe0 	viwdup.u32	q2, sl, r1, #4
[^>]*> ee2b 5fe1 	vdwdup.u32	q2, sl, r1, #8
[^>]*> ee2b 4fe1 	viwdup.u32	q2, sl, r1, #8
[^>]*> ee2b 5f62 	vdwdup.u32	q2, sl, r3, #1
[^>]*> ee2b 4f62 	viwdup.u32	q2, sl, r3, #1
[^>]*> ee2b 5f63 	vdwdup.u32	q2, sl, r3, #2
[^>]*> ee2b 4f63 	viwdup.u32	q2, sl, r3, #2
[^>]*> ee2b 5fe2 	vdwdup.u32	q2, sl, r3, #4
[^>]*> ee2b 4fe2 	viwdup.u32	q2, sl, r3, #4
[^>]*> ee2b 5fe3 	vdwdup.u32	q2, sl, r3, #8
[^>]*> ee2b 4fe3 	viwdup.u32	q2, sl, r3, #8
[^>]*> ee2b 5f64 	vdwdup.u32	q2, sl, r5, #1
[^>]*> ee2b 4f64 	viwdup.u32	q2, sl, r5, #1
[^>]*> ee2b 5f65 	vdwdup.u32	q2, sl, r5, #2
[^>]*> ee2b 4f65 	viwdup.u32	q2, sl, r5, #2
[^>]*> ee2b 5fe4 	vdwdup.u32	q2, sl, r5, #4
[^>]*> ee2b 4fe4 	viwdup.u32	q2, sl, r5, #4
[^>]*> ee2b 5fe5 	vdwdup.u32	q2, sl, r5, #8
[^>]*> ee2b 4fe5 	viwdup.u32	q2, sl, r5, #8
[^>]*> ee2b 5f66 	vdwdup.u32	q2, sl, r7, #1
[^>]*> ee2b 4f66 	viwdup.u32	q2, sl, r7, #1
[^>]*> ee2b 5f67 	vdwdup.u32	q2, sl, r7, #2
[^>]*> ee2b 4f67 	viwdup.u32	q2, sl, r7, #2
[^>]*> ee2b 5fe6 	vdwdup.u32	q2, sl, r7, #4
[^>]*> ee2b 4fe6 	viwdup.u32	q2, sl, r7, #4
[^>]*> ee2b 5fe7 	vdwdup.u32	q2, sl, r7, #8
[^>]*> ee2b 4fe7 	viwdup.u32	q2, sl, r7, #8
[^>]*> ee2b 5f68 	vdwdup.u32	q2, sl, r9, #1
[^>]*> ee2b 4f68 	viwdup.u32	q2, sl, r9, #1
[^>]*> ee2b 5f69 	vdwdup.u32	q2, sl, r9, #2
[^>]*> ee2b 4f69 	viwdup.u32	q2, sl, r9, #2
[^>]*> ee2b 5fe8 	vdwdup.u32	q2, sl, r9, #4
[^>]*> ee2b 4fe8 	viwdup.u32	q2, sl, r9, #4
[^>]*> ee2b 5fe9 	vdwdup.u32	q2, sl, r9, #8
[^>]*> ee2b 4fe9 	viwdup.u32	q2, sl, r9, #8
[^>]*> ee2b 5f6a 	vdwdup.u32	q2, sl, fp, #1
[^>]*> ee2b 4f6a 	viwdup.u32	q2, sl, fp, #1
[^>]*> ee2b 5f6b 	vdwdup.u32	q2, sl, fp, #2
[^>]*> ee2b 4f6b 	viwdup.u32	q2, sl, fp, #2
[^>]*> ee2b 5fea 	vdwdup.u32	q2, sl, fp, #4
[^>]*> ee2b 4fea 	viwdup.u32	q2, sl, fp, #4
[^>]*> ee2b 5feb 	vdwdup.u32	q2, sl, fp, #8
[^>]*> ee2b 4feb 	viwdup.u32	q2, sl, fp, #8
[^>]*> ee2d 5f6e 	vddup.u32	q2, ip, #1
[^>]*> ee2d 4f6e 	vidup.u32	q2, ip, #1
[^>]*> ee2d 5f6f 	vddup.u32	q2, ip, #2
[^>]*> ee2d 4f6f 	vidup.u32	q2, ip, #2
[^>]*> ee2d 5fee 	vddup.u32	q2, ip, #4
[^>]*> ee2d 4fee 	vidup.u32	q2, ip, #4
[^>]*> ee2d 5fef 	vddup.u32	q2, ip, #8
[^>]*> ee2d 4fef 	vidup.u32	q2, ip, #8
[^>]*> ee2d 5f60 	vdwdup.u32	q2, ip, r1, #1
[^>]*> ee2d 4f60 	viwdup.u32	q2, ip, r1, #1
[^>]*> ee2d 5f61 	vdwdup.u32	q2, ip, r1, #2
[^>]*> ee2d 4f61 	viwdup.u32	q2, ip, r1, #2
[^>]*> ee2d 5fe0 	vdwdup.u32	q2, ip, r1, #4
[^>]*> ee2d 4fe0 	viwdup.u32	q2, ip, r1, #4
[^>]*> ee2d 5fe1 	vdwdup.u32	q2, ip, r1, #8
[^>]*> ee2d 4fe1 	viwdup.u32	q2, ip, r1, #8
[^>]*> ee2d 5f62 	vdwdup.u32	q2, ip, r3, #1
[^>]*> ee2d 4f62 	viwdup.u32	q2, ip, r3, #1
[^>]*> ee2d 5f63 	vdwdup.u32	q2, ip, r3, #2
[^>]*> ee2d 4f63 	viwdup.u32	q2, ip, r3, #2
[^>]*> ee2d 5fe2 	vdwdup.u32	q2, ip, r3, #4
[^>]*> ee2d 4fe2 	viwdup.u32	q2, ip, r3, #4
[^>]*> ee2d 5fe3 	vdwdup.u32	q2, ip, r3, #8
[^>]*> ee2d 4fe3 	viwdup.u32	q2, ip, r3, #8
[^>]*> ee2d 5f64 	vdwdup.u32	q2, ip, r5, #1
[^>]*> ee2d 4f64 	viwdup.u32	q2, ip, r5, #1
[^>]*> ee2d 5f65 	vdwdup.u32	q2, ip, r5, #2
[^>]*> ee2d 4f65 	viwdup.u32	q2, ip, r5, #2
[^>]*> ee2d 5fe4 	vdwdup.u32	q2, ip, r5, #4
[^>]*> ee2d 4fe4 	viwdup.u32	q2, ip, r5, #4
[^>]*> ee2d 5fe5 	vdwdup.u32	q2, ip, r5, #8
[^>]*> ee2d 4fe5 	viwdup.u32	q2, ip, r5, #8
[^>]*> ee2d 5f66 	vdwdup.u32	q2, ip, r7, #1
[^>]*> ee2d 4f66 	viwdup.u32	q2, ip, r7, #1
[^>]*> ee2d 5f67 	vdwdup.u32	q2, ip, r7, #2
[^>]*> ee2d 4f67 	viwdup.u32	q2, ip, r7, #2
[^>]*> ee2d 5fe6 	vdwdup.u32	q2, ip, r7, #4
[^>]*> ee2d 4fe6 	viwdup.u32	q2, ip, r7, #4
[^>]*> ee2d 5fe7 	vdwdup.u32	q2, ip, r7, #8
[^>]*> ee2d 4fe7 	viwdup.u32	q2, ip, r7, #8
[^>]*> ee2d 5f68 	vdwdup.u32	q2, ip, r9, #1
[^>]*> ee2d 4f68 	viwdup.u32	q2, ip, r9, #1
[^>]*> ee2d 5f69 	vdwdup.u32	q2, ip, r9, #2
[^>]*> ee2d 4f69 	viwdup.u32	q2, ip, r9, #2
[^>]*> ee2d 5fe8 	vdwdup.u32	q2, ip, r9, #4
[^>]*> ee2d 4fe8 	viwdup.u32	q2, ip, r9, #4
[^>]*> ee2d 5fe9 	vdwdup.u32	q2, ip, r9, #8
[^>]*> ee2d 4fe9 	viwdup.u32	q2, ip, r9, #8
[^>]*> ee2d 5f6a 	vdwdup.u32	q2, ip, fp, #1
[^>]*> ee2d 4f6a 	viwdup.u32	q2, ip, fp, #1
[^>]*> ee2d 5f6b 	vdwdup.u32	q2, ip, fp, #2
[^>]*> ee2d 4f6b 	viwdup.u32	q2, ip, fp, #2
[^>]*> ee2d 5fea 	vdwdup.u32	q2, ip, fp, #4
[^>]*> ee2d 4fea 	viwdup.u32	q2, ip, fp, #4
[^>]*> ee2d 5feb 	vdwdup.u32	q2, ip, fp, #8
[^>]*> ee2d 4feb 	viwdup.u32	q2, ip, fp, #8
[^>]*> ee21 9f6e 	vddup.u32	q4, r0, #1
[^>]*> ee21 8f6e 	vidup.u32	q4, r0, #1
[^>]*> ee21 9f6f 	vddup.u32	q4, r0, #2
[^>]*> ee21 8f6f 	vidup.u32	q4, r0, #2
[^>]*> ee21 9fee 	vddup.u32	q4, r0, #4
[^>]*> ee21 8fee 	vidup.u32	q4, r0, #4
[^>]*> ee21 9fef 	vddup.u32	q4, r0, #8
[^>]*> ee21 8fef 	vidup.u32	q4, r0, #8
[^>]*> ee21 9f60 	vdwdup.u32	q4, r0, r1, #1
[^>]*> ee21 8f60 	viwdup.u32	q4, r0, r1, #1
[^>]*> ee21 9f61 	vdwdup.u32	q4, r0, r1, #2
[^>]*> ee21 8f61 	viwdup.u32	q4, r0, r1, #2
[^>]*> ee21 9fe0 	vdwdup.u32	q4, r0, r1, #4
[^>]*> ee21 8fe0 	viwdup.u32	q4, r0, r1, #4
[^>]*> ee21 9fe1 	vdwdup.u32	q4, r0, r1, #8
[^>]*> ee21 8fe1 	viwdup.u32	q4, r0, r1, #8
[^>]*> ee21 9f62 	vdwdup.u32	q4, r0, r3, #1
[^>]*> ee21 8f62 	viwdup.u32	q4, r0, r3, #1
[^>]*> ee21 9f63 	vdwdup.u32	q4, r0, r3, #2
[^>]*> ee21 8f63 	viwdup.u32	q4, r0, r3, #2
[^>]*> ee21 9fe2 	vdwdup.u32	q4, r0, r3, #4
[^>]*> ee21 8fe2 	viwdup.u32	q4, r0, r3, #4
[^>]*> ee21 9fe3 	vdwdup.u32	q4, r0, r3, #8
[^>]*> ee21 8fe3 	viwdup.u32	q4, r0, r3, #8
[^>]*> ee21 9f64 	vdwdup.u32	q4, r0, r5, #1
[^>]*> ee21 8f64 	viwdup.u32	q4, r0, r5, #1
[^>]*> ee21 9f65 	vdwdup.u32	q4, r0, r5, #2
[^>]*> ee21 8f65 	viwdup.u32	q4, r0, r5, #2
[^>]*> ee21 9fe4 	vdwdup.u32	q4, r0, r5, #4
[^>]*> ee21 8fe4 	viwdup.u32	q4, r0, r5, #4
[^>]*> ee21 9fe5 	vdwdup.u32	q4, r0, r5, #8
[^>]*> ee21 8fe5 	viwdup.u32	q4, r0, r5, #8
[^>]*> ee21 9f66 	vdwdup.u32	q4, r0, r7, #1
[^>]*> ee21 8f66 	viwdup.u32	q4, r0, r7, #1
[^>]*> ee21 9f67 	vdwdup.u32	q4, r0, r7, #2
[^>]*> ee21 8f67 	viwdup.u32	q4, r0, r7, #2
[^>]*> ee21 9fe6 	vdwdup.u32	q4, r0, r7, #4
[^>]*> ee21 8fe6 	viwdup.u32	q4, r0, r7, #4
[^>]*> ee21 9fe7 	vdwdup.u32	q4, r0, r7, #8
[^>]*> ee21 8fe7 	viwdup.u32	q4, r0, r7, #8
[^>]*> ee21 9f68 	vdwdup.u32	q4, r0, r9, #1
[^>]*> ee21 8f68 	viwdup.u32	q4, r0, r9, #1
[^>]*> ee21 9f69 	vdwdup.u32	q4, r0, r9, #2
[^>]*> ee21 8f69 	viwdup.u32	q4, r0, r9, #2
[^>]*> ee21 9fe8 	vdwdup.u32	q4, r0, r9, #4
[^>]*> ee21 8fe8 	viwdup.u32	q4, r0, r9, #4
[^>]*> ee21 9fe9 	vdwdup.u32	q4, r0, r9, #8
[^>]*> ee21 8fe9 	viwdup.u32	q4, r0, r9, #8
[^>]*> ee21 9f6a 	vdwdup.u32	q4, r0, fp, #1
[^>]*> ee21 8f6a 	viwdup.u32	q4, r0, fp, #1
[^>]*> ee21 9f6b 	vdwdup.u32	q4, r0, fp, #2
[^>]*> ee21 8f6b 	viwdup.u32	q4, r0, fp, #2
[^>]*> ee21 9fea 	vdwdup.u32	q4, r0, fp, #4
[^>]*> ee21 8fea 	viwdup.u32	q4, r0, fp, #4
[^>]*> ee21 9feb 	vdwdup.u32	q4, r0, fp, #8
[^>]*> ee21 8feb 	viwdup.u32	q4, r0, fp, #8
[^>]*> ee23 9f6e 	vddup.u32	q4, r2, #1
[^>]*> ee23 8f6e 	vidup.u32	q4, r2, #1
[^>]*> ee23 9f6f 	vddup.u32	q4, r2, #2
[^>]*> ee23 8f6f 	vidup.u32	q4, r2, #2
[^>]*> ee23 9fee 	vddup.u32	q4, r2, #4
[^>]*> ee23 8fee 	vidup.u32	q4, r2, #4
[^>]*> ee23 9fef 	vddup.u32	q4, r2, #8
[^>]*> ee23 8fef 	vidup.u32	q4, r2, #8
[^>]*> ee23 9f60 	vdwdup.u32	q4, r2, r1, #1
[^>]*> ee23 8f60 	viwdup.u32	q4, r2, r1, #1
[^>]*> ee23 9f61 	vdwdup.u32	q4, r2, r1, #2
[^>]*> ee23 8f61 	viwdup.u32	q4, r2, r1, #2
[^>]*> ee23 9fe0 	vdwdup.u32	q4, r2, r1, #4
[^>]*> ee23 8fe0 	viwdup.u32	q4, r2, r1, #4
[^>]*> ee23 9fe1 	vdwdup.u32	q4, r2, r1, #8
[^>]*> ee23 8fe1 	viwdup.u32	q4, r2, r1, #8
[^>]*> ee23 9f62 	vdwdup.u32	q4, r2, r3, #1
[^>]*> ee23 8f62 	viwdup.u32	q4, r2, r3, #1
[^>]*> ee23 9f63 	vdwdup.u32	q4, r2, r3, #2
[^>]*> ee23 8f63 	viwdup.u32	q4, r2, r3, #2
[^>]*> ee23 9fe2 	vdwdup.u32	q4, r2, r3, #4
[^>]*> ee23 8fe2 	viwdup.u32	q4, r2, r3, #4
[^>]*> ee23 9fe3 	vdwdup.u32	q4, r2, r3, #8
[^>]*> ee23 8fe3 	viwdup.u32	q4, r2, r3, #8
[^>]*> ee23 9f64 	vdwdup.u32	q4, r2, r5, #1
[^>]*> ee23 8f64 	viwdup.u32	q4, r2, r5, #1
[^>]*> ee23 9f65 	vdwdup.u32	q4, r2, r5, #2
[^>]*> ee23 8f65 	viwdup.u32	q4, r2, r5, #2
[^>]*> ee23 9fe4 	vdwdup.u32	q4, r2, r5, #4
[^>]*> ee23 8fe4 	viwdup.u32	q4, r2, r5, #4
[^>]*> ee23 9fe5 	vdwdup.u32	q4, r2, r5, #8
[^>]*> ee23 8fe5 	viwdup.u32	q4, r2, r5, #8
[^>]*> ee23 9f66 	vdwdup.u32	q4, r2, r7, #1
[^>]*> ee23 8f66 	viwdup.u32	q4, r2, r7, #1
[^>]*> ee23 9f67 	vdwdup.u32	q4, r2, r7, #2
[^>]*> ee23 8f67 	viwdup.u32	q4, r2, r7, #2
[^>]*> ee23 9fe6 	vdwdup.u32	q4, r2, r7, #4
[^>]*> ee23 8fe6 	viwdup.u32	q4, r2, r7, #4
[^>]*> ee23 9fe7 	vdwdup.u32	q4, r2, r7, #8
[^>]*> ee23 8fe7 	viwdup.u32	q4, r2, r7, #8
[^>]*> ee23 9f68 	vdwdup.u32	q4, r2, r9, #1
[^>]*> ee23 8f68 	viwdup.u32	q4, r2, r9, #1
[^>]*> ee23 9f69 	vdwdup.u32	q4, r2, r9, #2
[^>]*> ee23 8f69 	viwdup.u32	q4, r2, r9, #2
[^>]*> ee23 9fe8 	vdwdup.u32	q4, r2, r9, #4
[^>]*> ee23 8fe8 	viwdup.u32	q4, r2, r9, #4
[^>]*> ee23 9fe9 	vdwdup.u32	q4, r2, r9, #8
[^>]*> ee23 8fe9 	viwdup.u32	q4, r2, r9, #8
[^>]*> ee23 9f6a 	vdwdup.u32	q4, r2, fp, #1
[^>]*> ee23 8f6a 	viwdup.u32	q4, r2, fp, #1
[^>]*> ee23 9f6b 	vdwdup.u32	q4, r2, fp, #2
[^>]*> ee23 8f6b 	viwdup.u32	q4, r2, fp, #2
[^>]*> ee23 9fea 	vdwdup.u32	q4, r2, fp, #4
[^>]*> ee23 8fea 	viwdup.u32	q4, r2, fp, #4
[^>]*> ee23 9feb 	vdwdup.u32	q4, r2, fp, #8
[^>]*> ee23 8feb 	viwdup.u32	q4, r2, fp, #8
[^>]*> ee25 9f6e 	vddup.u32	q4, r4, #1
[^>]*> ee25 8f6e 	vidup.u32	q4, r4, #1
[^>]*> ee25 9f6f 	vddup.u32	q4, r4, #2
[^>]*> ee25 8f6f 	vidup.u32	q4, r4, #2
[^>]*> ee25 9fee 	vddup.u32	q4, r4, #4
[^>]*> ee25 8fee 	vidup.u32	q4, r4, #4
[^>]*> ee25 9fef 	vddup.u32	q4, r4, #8
[^>]*> ee25 8fef 	vidup.u32	q4, r4, #8
[^>]*> ee25 9f60 	vdwdup.u32	q4, r4, r1, #1
[^>]*> ee25 8f60 	viwdup.u32	q4, r4, r1, #1
[^>]*> ee25 9f61 	vdwdup.u32	q4, r4, r1, #2
[^>]*> ee25 8f61 	viwdup.u32	q4, r4, r1, #2
[^>]*> ee25 9fe0 	vdwdup.u32	q4, r4, r1, #4
[^>]*> ee25 8fe0 	viwdup.u32	q4, r4, r1, #4
[^>]*> ee25 9fe1 	vdwdup.u32	q4, r4, r1, #8
[^>]*> ee25 8fe1 	viwdup.u32	q4, r4, r1, #8
[^>]*> ee25 9f62 	vdwdup.u32	q4, r4, r3, #1
[^>]*> ee25 8f62 	viwdup.u32	q4, r4, r3, #1
[^>]*> ee25 9f63 	vdwdup.u32	q4, r4, r3, #2
[^>]*> ee25 8f63 	viwdup.u32	q4, r4, r3, #2
[^>]*> ee25 9fe2 	vdwdup.u32	q4, r4, r3, #4
[^>]*> ee25 8fe2 	viwdup.u32	q4, r4, r3, #4
[^>]*> ee25 9fe3 	vdwdup.u32	q4, r4, r3, #8
[^>]*> ee25 8fe3 	viwdup.u32	q4, r4, r3, #8
[^>]*> ee25 9f64 	vdwdup.u32	q4, r4, r5, #1
[^>]*> ee25 8f64 	viwdup.u32	q4, r4, r5, #1
[^>]*> ee25 9f65 	vdwdup.u32	q4, r4, r5, #2
[^>]*> ee25 8f65 	viwdup.u32	q4, r4, r5, #2
[^>]*> ee25 9fe4 	vdwdup.u32	q4, r4, r5, #4
[^>]*> ee25 8fe4 	viwdup.u32	q4, r4, r5, #4
[^>]*> ee25 9fe5 	vdwdup.u32	q4, r4, r5, #8
[^>]*> ee25 8fe5 	viwdup.u32	q4, r4, r5, #8
[^>]*> ee25 9f66 	vdwdup.u32	q4, r4, r7, #1
[^>]*> ee25 8f66 	viwdup.u32	q4, r4, r7, #1
[^>]*> ee25 9f67 	vdwdup.u32	q4, r4, r7, #2
[^>]*> ee25 8f67 	viwdup.u32	q4, r4, r7, #2
[^>]*> ee25 9fe6 	vdwdup.u32	q4, r4, r7, #4
[^>]*> ee25 8fe6 	viwdup.u32	q4, r4, r7, #4
[^>]*> ee25 9fe7 	vdwdup.u32	q4, r4, r7, #8
[^>]*> ee25 8fe7 	viwdup.u32	q4, r4, r7, #8
[^>]*> ee25 9f68 	vdwdup.u32	q4, r4, r9, #1
[^>]*> ee25 8f68 	viwdup.u32	q4, r4, r9, #1
[^>]*> ee25 9f69 	vdwdup.u32	q4, r4, r9, #2
[^>]*> ee25 8f69 	viwdup.u32	q4, r4, r9, #2
[^>]*> ee25 9fe8 	vdwdup.u32	q4, r4, r9, #4
[^>]*> ee25 8fe8 	viwdup.u32	q4, r4, r9, #4
[^>]*> ee25 9fe9 	vdwdup.u32	q4, r4, r9, #8
[^>]*> ee25 8fe9 	viwdup.u32	q4, r4, r9, #8
[^>]*> ee25 9f6a 	vdwdup.u32	q4, r4, fp, #1
[^>]*> ee25 8f6a 	viwdup.u32	q4, r4, fp, #1
[^>]*> ee25 9f6b 	vdwdup.u32	q4, r4, fp, #2
[^>]*> ee25 8f6b 	viwdup.u32	q4, r4, fp, #2
[^>]*> ee25 9fea 	vdwdup.u32	q4, r4, fp, #4
[^>]*> ee25 8fea 	viwdup.u32	q4, r4, fp, #4
[^>]*> ee25 9feb 	vdwdup.u32	q4, r4, fp, #8
[^>]*> ee25 8feb 	viwdup.u32	q4, r4, fp, #8
[^>]*> ee27 9f6e 	vddup.u32	q4, r6, #1
[^>]*> ee27 8f6e 	vidup.u32	q4, r6, #1
[^>]*> ee27 9f6f 	vddup.u32	q4, r6, #2
[^>]*> ee27 8f6f 	vidup.u32	q4, r6, #2
[^>]*> ee27 9fee 	vddup.u32	q4, r6, #4
[^>]*> ee27 8fee 	vidup.u32	q4, r6, #4
[^>]*> ee27 9fef 	vddup.u32	q4, r6, #8
[^>]*> ee27 8fef 	vidup.u32	q4, r6, #8
[^>]*> ee27 9f60 	vdwdup.u32	q4, r6, r1, #1
[^>]*> ee27 8f60 	viwdup.u32	q4, r6, r1, #1
[^>]*> ee27 9f61 	vdwdup.u32	q4, r6, r1, #2
[^>]*> ee27 8f61 	viwdup.u32	q4, r6, r1, #2
[^>]*> ee27 9fe0 	vdwdup.u32	q4, r6, r1, #4
[^>]*> ee27 8fe0 	viwdup.u32	q4, r6, r1, #4
[^>]*> ee27 9fe1 	vdwdup.u32	q4, r6, r1, #8
[^>]*> ee27 8fe1 	viwdup.u32	q4, r6, r1, #8
[^>]*> ee27 9f62 	vdwdup.u32	q4, r6, r3, #1
[^>]*> ee27 8f62 	viwdup.u32	q4, r6, r3, #1
[^>]*> ee27 9f63 	vdwdup.u32	q4, r6, r3, #2
[^>]*> ee27 8f63 	viwdup.u32	q4, r6, r3, #2
[^>]*> ee27 9fe2 	vdwdup.u32	q4, r6, r3, #4
[^>]*> ee27 8fe2 	viwdup.u32	q4, r6, r3, #4
[^>]*> ee27 9fe3 	vdwdup.u32	q4, r6, r3, #8
[^>]*> ee27 8fe3 	viwdup.u32	q4, r6, r3, #8
[^>]*> ee27 9f64 	vdwdup.u32	q4, r6, r5, #1
[^>]*> ee27 8f64 	viwdup.u32	q4, r6, r5, #1
[^>]*> ee27 9f65 	vdwdup.u32	q4, r6, r5, #2
[^>]*> ee27 8f65 	viwdup.u32	q4, r6, r5, #2
[^>]*> ee27 9fe4 	vdwdup.u32	q4, r6, r5, #4
[^>]*> ee27 8fe4 	viwdup.u32	q4, r6, r5, #4
[^>]*> ee27 9fe5 	vdwdup.u32	q4, r6, r5, #8
[^>]*> ee27 8fe5 	viwdup.u32	q4, r6, r5, #8
[^>]*> ee27 9f66 	vdwdup.u32	q4, r6, r7, #1
[^>]*> ee27 8f66 	viwdup.u32	q4, r6, r7, #1
[^>]*> ee27 9f67 	vdwdup.u32	q4, r6, r7, #2
[^>]*> ee27 8f67 	viwdup.u32	q4, r6, r7, #2
[^>]*> ee27 9fe6 	vdwdup.u32	q4, r6, r7, #4
[^>]*> ee27 8fe6 	viwdup.u32	q4, r6, r7, #4
[^>]*> ee27 9fe7 	vdwdup.u32	q4, r6, r7, #8
[^>]*> ee27 8fe7 	viwdup.u32	q4, r6, r7, #8
[^>]*> ee27 9f68 	vdwdup.u32	q4, r6, r9, #1
[^>]*> ee27 8f68 	viwdup.u32	q4, r6, r9, #1
[^>]*> ee27 9f69 	vdwdup.u32	q4, r6, r9, #2
[^>]*> ee27 8f69 	viwdup.u32	q4, r6, r9, #2
[^>]*> ee27 9fe8 	vdwdup.u32	q4, r6, r9, #4
[^>]*> ee27 8fe8 	viwdup.u32	q4, r6, r9, #4
[^>]*> ee27 9fe9 	vdwdup.u32	q4, r6, r9, #8
[^>]*> ee27 8fe9 	viwdup.u32	q4, r6, r9, #8
[^>]*> ee27 9f6a 	vdwdup.u32	q4, r6, fp, #1
[^>]*> ee27 8f6a 	viwdup.u32	q4, r6, fp, #1
[^>]*> ee27 9f6b 	vdwdup.u32	q4, r6, fp, #2
[^>]*> ee27 8f6b 	viwdup.u32	q4, r6, fp, #2
[^>]*> ee27 9fea 	vdwdup.u32	q4, r6, fp, #4
[^>]*> ee27 8fea 	viwdup.u32	q4, r6, fp, #4
[^>]*> ee27 9feb 	vdwdup.u32	q4, r6, fp, #8
[^>]*> ee27 8feb 	viwdup.u32	q4, r6, fp, #8
[^>]*> ee29 9f6e 	vddup.u32	q4, r8, #1
[^>]*> ee29 8f6e 	vidup.u32	q4, r8, #1
[^>]*> ee29 9f6f 	vddup.u32	q4, r8, #2
[^>]*> ee29 8f6f 	vidup.u32	q4, r8, #2
[^>]*> ee29 9fee 	vddup.u32	q4, r8, #4
[^>]*> ee29 8fee 	vidup.u32	q4, r8, #4
[^>]*> ee29 9fef 	vddup.u32	q4, r8, #8
[^>]*> ee29 8fef 	vidup.u32	q4, r8, #8
[^>]*> ee29 9f60 	vdwdup.u32	q4, r8, r1, #1
[^>]*> ee29 8f60 	viwdup.u32	q4, r8, r1, #1
[^>]*> ee29 9f61 	vdwdup.u32	q4, r8, r1, #2
[^>]*> ee29 8f61 	viwdup.u32	q4, r8, r1, #2
[^>]*> ee29 9fe0 	vdwdup.u32	q4, r8, r1, #4
[^>]*> ee29 8fe0 	viwdup.u32	q4, r8, r1, #4
[^>]*> ee29 9fe1 	vdwdup.u32	q4, r8, r1, #8
[^>]*> ee29 8fe1 	viwdup.u32	q4, r8, r1, #8
[^>]*> ee29 9f62 	vdwdup.u32	q4, r8, r3, #1
[^>]*> ee29 8f62 	viwdup.u32	q4, r8, r3, #1
[^>]*> ee29 9f63 	vdwdup.u32	q4, r8, r3, #2
[^>]*> ee29 8f63 	viwdup.u32	q4, r8, r3, #2
[^>]*> ee29 9fe2 	vdwdup.u32	q4, r8, r3, #4
[^>]*> ee29 8fe2 	viwdup.u32	q4, r8, r3, #4
[^>]*> ee29 9fe3 	vdwdup.u32	q4, r8, r3, #8
[^>]*> ee29 8fe3 	viwdup.u32	q4, r8, r3, #8
[^>]*> ee29 9f64 	vdwdup.u32	q4, r8, r5, #1
[^>]*> ee29 8f64 	viwdup.u32	q4, r8, r5, #1
[^>]*> ee29 9f65 	vdwdup.u32	q4, r8, r5, #2
[^>]*> ee29 8f65 	viwdup.u32	q4, r8, r5, #2
[^>]*> ee29 9fe4 	vdwdup.u32	q4, r8, r5, #4
[^>]*> ee29 8fe4 	viwdup.u32	q4, r8, r5, #4
[^>]*> ee29 9fe5 	vdwdup.u32	q4, r8, r5, #8
[^>]*> ee29 8fe5 	viwdup.u32	q4, r8, r5, #8
[^>]*> ee29 9f66 	vdwdup.u32	q4, r8, r7, #1
[^>]*> ee29 8f66 	viwdup.u32	q4, r8, r7, #1
[^>]*> ee29 9f67 	vdwdup.u32	q4, r8, r7, #2
[^>]*> ee29 8f67 	viwdup.u32	q4, r8, r7, #2
[^>]*> ee29 9fe6 	vdwdup.u32	q4, r8, r7, #4
[^>]*> ee29 8fe6 	viwdup.u32	q4, r8, r7, #4
[^>]*> ee29 9fe7 	vdwdup.u32	q4, r8, r7, #8
[^>]*> ee29 8fe7 	viwdup.u32	q4, r8, r7, #8
[^>]*> ee29 9f68 	vdwdup.u32	q4, r8, r9, #1
[^>]*> ee29 8f68 	viwdup.u32	q4, r8, r9, #1
[^>]*> ee29 9f69 	vdwdup.u32	q4, r8, r9, #2
[^>]*> ee29 8f69 	viwdup.u32	q4, r8, r9, #2
[^>]*> ee29 9fe8 	vdwdup.u32	q4, r8, r9, #4
[^>]*> ee29 8fe8 	viwdup.u32	q4, r8, r9, #4
[^>]*> ee29 9fe9 	vdwdup.u32	q4, r8, r9, #8
[^>]*> ee29 8fe9 	viwdup.u32	q4, r8, r9, #8
[^>]*> ee29 9f6a 	vdwdup.u32	q4, r8, fp, #1
[^>]*> ee29 8f6a 	viwdup.u32	q4, r8, fp, #1
[^>]*> ee29 9f6b 	vdwdup.u32	q4, r8, fp, #2
[^>]*> ee29 8f6b 	viwdup.u32	q4, r8, fp, #2
[^>]*> ee29 9fea 	vdwdup.u32	q4, r8, fp, #4
[^>]*> ee29 8fea 	viwdup.u32	q4, r8, fp, #4
[^>]*> ee29 9feb 	vdwdup.u32	q4, r8, fp, #8
[^>]*> ee29 8feb 	viwdup.u32	q4, r8, fp, #8
[^>]*> ee2b 9f6e 	vddup.u32	q4, sl, #1
[^>]*> ee2b 8f6e 	vidup.u32	q4, sl, #1
[^>]*> ee2b 9f6f 	vddup.u32	q4, sl, #2
[^>]*> ee2b 8f6f 	vidup.u32	q4, sl, #2
[^>]*> ee2b 9fee 	vddup.u32	q4, sl, #4
[^>]*> ee2b 8fee 	vidup.u32	q4, sl, #4
[^>]*> ee2b 9fef 	vddup.u32	q4, sl, #8
[^>]*> ee2b 8fef 	vidup.u32	q4, sl, #8
[^>]*> ee2b 9f60 	vdwdup.u32	q4, sl, r1, #1
[^>]*> ee2b 8f60 	viwdup.u32	q4, sl, r1, #1
[^>]*> ee2b 9f61 	vdwdup.u32	q4, sl, r1, #2
[^>]*> ee2b 8f61 	viwdup.u32	q4, sl, r1, #2
[^>]*> ee2b 9fe0 	vdwdup.u32	q4, sl, r1, #4
[^>]*> ee2b 8fe0 	viwdup.u32	q4, sl, r1, #4
[^>]*> ee2b 9fe1 	vdwdup.u32	q4, sl, r1, #8
[^>]*> ee2b 8fe1 	viwdup.u32	q4, sl, r1, #8
[^>]*> ee2b 9f62 	vdwdup.u32	q4, sl, r3, #1
[^>]*> ee2b 8f62 	viwdup.u32	q4, sl, r3, #1
[^>]*> ee2b 9f63 	vdwdup.u32	q4, sl, r3, #2
[^>]*> ee2b 8f63 	viwdup.u32	q4, sl, r3, #2
[^>]*> ee2b 9fe2 	vdwdup.u32	q4, sl, r3, #4
[^>]*> ee2b 8fe2 	viwdup.u32	q4, sl, r3, #4
[^>]*> ee2b 9fe3 	vdwdup.u32	q4, sl, r3, #8
[^>]*> ee2b 8fe3 	viwdup.u32	q4, sl, r3, #8
[^>]*> ee2b 9f64 	vdwdup.u32	q4, sl, r5, #1
[^>]*> ee2b 8f64 	viwdup.u32	q4, sl, r5, #1
[^>]*> ee2b 9f65 	vdwdup.u32	q4, sl, r5, #2
[^>]*> ee2b 8f65 	viwdup.u32	q4, sl, r5, #2
[^>]*> ee2b 9fe4 	vdwdup.u32	q4, sl, r5, #4
[^>]*> ee2b 8fe4 	viwdup.u32	q4, sl, r5, #4
[^>]*> ee2b 9fe5 	vdwdup.u32	q4, sl, r5, #8
[^>]*> ee2b 8fe5 	viwdup.u32	q4, sl, r5, #8
[^>]*> ee2b 9f66 	vdwdup.u32	q4, sl, r7, #1
[^>]*> ee2b 8f66 	viwdup.u32	q4, sl, r7, #1
[^>]*> ee2b 9f67 	vdwdup.u32	q4, sl, r7, #2
[^>]*> ee2b 8f67 	viwdup.u32	q4, sl, r7, #2
[^>]*> ee2b 9fe6 	vdwdup.u32	q4, sl, r7, #4
[^>]*> ee2b 8fe6 	viwdup.u32	q4, sl, r7, #4
[^>]*> ee2b 9fe7 	vdwdup.u32	q4, sl, r7, #8
[^>]*> ee2b 8fe7 	viwdup.u32	q4, sl, r7, #8
[^>]*> ee2b 9f68 	vdwdup.u32	q4, sl, r9, #1
[^>]*> ee2b 8f68 	viwdup.u32	q4, sl, r9, #1
[^>]*> ee2b 9f69 	vdwdup.u32	q4, sl, r9, #2
[^>]*> ee2b 8f69 	viwdup.u32	q4, sl, r9, #2
[^>]*> ee2b 9fe8 	vdwdup.u32	q4, sl, r9, #4
[^>]*> ee2b 8fe8 	viwdup.u32	q4, sl, r9, #4
[^>]*> ee2b 9fe9 	vdwdup.u32	q4, sl, r9, #8
[^>]*> ee2b 8fe9 	viwdup.u32	q4, sl, r9, #8
[^>]*> ee2b 9f6a 	vdwdup.u32	q4, sl, fp, #1
[^>]*> ee2b 8f6a 	viwdup.u32	q4, sl, fp, #1
[^>]*> ee2b 9f6b 	vdwdup.u32	q4, sl, fp, #2
[^>]*> ee2b 8f6b 	viwdup.u32	q4, sl, fp, #2
[^>]*> ee2b 9fea 	vdwdup.u32	q4, sl, fp, #4
[^>]*> ee2b 8fea 	viwdup.u32	q4, sl, fp, #4
[^>]*> ee2b 9feb 	vdwdup.u32	q4, sl, fp, #8
[^>]*> ee2b 8feb 	viwdup.u32	q4, sl, fp, #8
[^>]*> ee2d 9f6e 	vddup.u32	q4, ip, #1
[^>]*> ee2d 8f6e 	vidup.u32	q4, ip, #1
[^>]*> ee2d 9f6f 	vddup.u32	q4, ip, #2
[^>]*> ee2d 8f6f 	vidup.u32	q4, ip, #2
[^>]*> ee2d 9fee 	vddup.u32	q4, ip, #4
[^>]*> ee2d 8fee 	vidup.u32	q4, ip, #4
[^>]*> ee2d 9fef 	vddup.u32	q4, ip, #8
[^>]*> ee2d 8fef 	vidup.u32	q4, ip, #8
[^>]*> ee2d 9f60 	vdwdup.u32	q4, ip, r1, #1
[^>]*> ee2d 8f60 	viwdup.u32	q4, ip, r1, #1
[^>]*> ee2d 9f61 	vdwdup.u32	q4, ip, r1, #2
[^>]*> ee2d 8f61 	viwdup.u32	q4, ip, r1, #2
[^>]*> ee2d 9fe0 	vdwdup.u32	q4, ip, r1, #4
[^>]*> ee2d 8fe0 	viwdup.u32	q4, ip, r1, #4
[^>]*> ee2d 9fe1 	vdwdup.u32	q4, ip, r1, #8
[^>]*> ee2d 8fe1 	viwdup.u32	q4, ip, r1, #8
[^>]*> ee2d 9f62 	vdwdup.u32	q4, ip, r3, #1
[^>]*> ee2d 8f62 	viwdup.u32	q4, ip, r3, #1
[^>]*> ee2d 9f63 	vdwdup.u32	q4, ip, r3, #2
[^>]*> ee2d 8f63 	viwdup.u32	q4, ip, r3, #2
[^>]*> ee2d 9fe2 	vdwdup.u32	q4, ip, r3, #4
[^>]*> ee2d 8fe2 	viwdup.u32	q4, ip, r3, #4
[^>]*> ee2d 9fe3 	vdwdup.u32	q4, ip, r3, #8
[^>]*> ee2d 8fe3 	viwdup.u32	q4, ip, r3, #8
[^>]*> ee2d 9f64 	vdwdup.u32	q4, ip, r5, #1
[^>]*> ee2d 8f64 	viwdup.u32	q4, ip, r5, #1
[^>]*> ee2d 9f65 	vdwdup.u32	q4, ip, r5, #2
[^>]*> ee2d 8f65 	viwdup.u32	q4, ip, r5, #2
[^>]*> ee2d 9fe4 	vdwdup.u32	q4, ip, r5, #4
[^>]*> ee2d 8fe4 	viwdup.u32	q4, ip, r5, #4
[^>]*> ee2d 9fe5 	vdwdup.u32	q4, ip, r5, #8
[^>]*> ee2d 8fe5 	viwdup.u32	q4, ip, r5, #8
[^>]*> ee2d 9f66 	vdwdup.u32	q4, ip, r7, #1
[^>]*> ee2d 8f66 	viwdup.u32	q4, ip, r7, #1
[^>]*> ee2d 9f67 	vdwdup.u32	q4, ip, r7, #2
[^>]*> ee2d 8f67 	viwdup.u32	q4, ip, r7, #2
[^>]*> ee2d 9fe6 	vdwdup.u32	q4, ip, r7, #4
[^>]*> ee2d 8fe6 	viwdup.u32	q4, ip, r7, #4
[^>]*> ee2d 9fe7 	vdwdup.u32	q4, ip, r7, #8
[^>]*> ee2d 8fe7 	viwdup.u32	q4, ip, r7, #8
[^>]*> ee2d 9f68 	vdwdup.u32	q4, ip, r9, #1
[^>]*> ee2d 8f68 	viwdup.u32	q4, ip, r9, #1
[^>]*> ee2d 9f69 	vdwdup.u32	q4, ip, r9, #2
[^>]*> ee2d 8f69 	viwdup.u32	q4, ip, r9, #2
[^>]*> ee2d 9fe8 	vdwdup.u32	q4, ip, r9, #4
[^>]*> ee2d 8fe8 	viwdup.u32	q4, ip, r9, #4
[^>]*> ee2d 9fe9 	vdwdup.u32	q4, ip, r9, #8
[^>]*> ee2d 8fe9 	viwdup.u32	q4, ip, r9, #8
[^>]*> ee2d 9f6a 	vdwdup.u32	q4, ip, fp, #1
[^>]*> ee2d 8f6a 	viwdup.u32	q4, ip, fp, #1
[^>]*> ee2d 9f6b 	vdwdup.u32	q4, ip, fp, #2
[^>]*> ee2d 8f6b 	viwdup.u32	q4, ip, fp, #2
[^>]*> ee2d 9fea 	vdwdup.u32	q4, ip, fp, #4
[^>]*> ee2d 8fea 	viwdup.u32	q4, ip, fp, #4
[^>]*> ee2d 9feb 	vdwdup.u32	q4, ip, fp, #8
[^>]*> ee2d 8feb 	viwdup.u32	q4, ip, fp, #8
[^>]*> ee21 ff6e 	vddup.u32	q7, r0, #1
[^>]*> ee21 ef6e 	vidup.u32	q7, r0, #1
[^>]*> ee21 ff6f 	vddup.u32	q7, r0, #2
[^>]*> ee21 ef6f 	vidup.u32	q7, r0, #2
[^>]*> ee21 ffee 	vddup.u32	q7, r0, #4
[^>]*> ee21 efee 	vidup.u32	q7, r0, #4
[^>]*> ee21 ffef 	vddup.u32	q7, r0, #8
[^>]*> ee21 efef 	vidup.u32	q7, r0, #8
[^>]*> ee21 ff60 	vdwdup.u32	q7, r0, r1, #1
[^>]*> ee21 ef60 	viwdup.u32	q7, r0, r1, #1
[^>]*> ee21 ff61 	vdwdup.u32	q7, r0, r1, #2
[^>]*> ee21 ef61 	viwdup.u32	q7, r0, r1, #2
[^>]*> ee21 ffe0 	vdwdup.u32	q7, r0, r1, #4
[^>]*> ee21 efe0 	viwdup.u32	q7, r0, r1, #4
[^>]*> ee21 ffe1 	vdwdup.u32	q7, r0, r1, #8
[^>]*> ee21 efe1 	viwdup.u32	q7, r0, r1, #8
[^>]*> ee21 ff62 	vdwdup.u32	q7, r0, r3, #1
[^>]*> ee21 ef62 	viwdup.u32	q7, r0, r3, #1
[^>]*> ee21 ff63 	vdwdup.u32	q7, r0, r3, #2
[^>]*> ee21 ef63 	viwdup.u32	q7, r0, r3, #2
[^>]*> ee21 ffe2 	vdwdup.u32	q7, r0, r3, #4
[^>]*> ee21 efe2 	viwdup.u32	q7, r0, r3, #4
[^>]*> ee21 ffe3 	vdwdup.u32	q7, r0, r3, #8
[^>]*> ee21 efe3 	viwdup.u32	q7, r0, r3, #8
[^>]*> ee21 ff64 	vdwdup.u32	q7, r0, r5, #1
[^>]*> ee21 ef64 	viwdup.u32	q7, r0, r5, #1
[^>]*> ee21 ff65 	vdwdup.u32	q7, r0, r5, #2
[^>]*> ee21 ef65 	viwdup.u32	q7, r0, r5, #2
[^>]*> ee21 ffe4 	vdwdup.u32	q7, r0, r5, #4
[^>]*> ee21 efe4 	viwdup.u32	q7, r0, r5, #4
[^>]*> ee21 ffe5 	vdwdup.u32	q7, r0, r5, #8
[^>]*> ee21 efe5 	viwdup.u32	q7, r0, r5, #8
[^>]*> ee21 ff66 	vdwdup.u32	q7, r0, r7, #1
[^>]*> ee21 ef66 	viwdup.u32	q7, r0, r7, #1
[^>]*> ee21 ff67 	vdwdup.u32	q7, r0, r7, #2
[^>]*> ee21 ef67 	viwdup.u32	q7, r0, r7, #2
[^>]*> ee21 ffe6 	vdwdup.u32	q7, r0, r7, #4
[^>]*> ee21 efe6 	viwdup.u32	q7, r0, r7, #4
[^>]*> ee21 ffe7 	vdwdup.u32	q7, r0, r7, #8
[^>]*> ee21 efe7 	viwdup.u32	q7, r0, r7, #8
[^>]*> ee21 ff68 	vdwdup.u32	q7, r0, r9, #1
[^>]*> ee21 ef68 	viwdup.u32	q7, r0, r9, #1
[^>]*> ee21 ff69 	vdwdup.u32	q7, r0, r9, #2
[^>]*> ee21 ef69 	viwdup.u32	q7, r0, r9, #2
[^>]*> ee21 ffe8 	vdwdup.u32	q7, r0, r9, #4
[^>]*> ee21 efe8 	viwdup.u32	q7, r0, r9, #4
[^>]*> ee21 ffe9 	vdwdup.u32	q7, r0, r9, #8
[^>]*> ee21 efe9 	viwdup.u32	q7, r0, r9, #8
[^>]*> ee21 ff6a 	vdwdup.u32	q7, r0, fp, #1
[^>]*> ee21 ef6a 	viwdup.u32	q7, r0, fp, #1
[^>]*> ee21 ff6b 	vdwdup.u32	q7, r0, fp, #2
[^>]*> ee21 ef6b 	viwdup.u32	q7, r0, fp, #2
[^>]*> ee21 ffea 	vdwdup.u32	q7, r0, fp, #4
[^>]*> ee21 efea 	viwdup.u32	q7, r0, fp, #4
[^>]*> ee21 ffeb 	vdwdup.u32	q7, r0, fp, #8
[^>]*> ee21 efeb 	viwdup.u32	q7, r0, fp, #8
[^>]*> ee23 ff6e 	vddup.u32	q7, r2, #1
[^>]*> ee23 ef6e 	vidup.u32	q7, r2, #1
[^>]*> ee23 ff6f 	vddup.u32	q7, r2, #2
[^>]*> ee23 ef6f 	vidup.u32	q7, r2, #2
[^>]*> ee23 ffee 	vddup.u32	q7, r2, #4
[^>]*> ee23 efee 	vidup.u32	q7, r2, #4
[^>]*> ee23 ffef 	vddup.u32	q7, r2, #8
[^>]*> ee23 efef 	vidup.u32	q7, r2, #8
[^>]*> ee23 ff60 	vdwdup.u32	q7, r2, r1, #1
[^>]*> ee23 ef60 	viwdup.u32	q7, r2, r1, #1
[^>]*> ee23 ff61 	vdwdup.u32	q7, r2, r1, #2
[^>]*> ee23 ef61 	viwdup.u32	q7, r2, r1, #2
[^>]*> ee23 ffe0 	vdwdup.u32	q7, r2, r1, #4
[^>]*> ee23 efe0 	viwdup.u32	q7, r2, r1, #4
[^>]*> ee23 ffe1 	vdwdup.u32	q7, r2, r1, #8
[^>]*> ee23 efe1 	viwdup.u32	q7, r2, r1, #8
[^>]*> ee23 ff62 	vdwdup.u32	q7, r2, r3, #1
[^>]*> ee23 ef62 	viwdup.u32	q7, r2, r3, #1
[^>]*> ee23 ff63 	vdwdup.u32	q7, r2, r3, #2
[^>]*> ee23 ef63 	viwdup.u32	q7, r2, r3, #2
[^>]*> ee23 ffe2 	vdwdup.u32	q7, r2, r3, #4
[^>]*> ee23 efe2 	viwdup.u32	q7, r2, r3, #4
[^>]*> ee23 ffe3 	vdwdup.u32	q7, r2, r3, #8
[^>]*> ee23 efe3 	viwdup.u32	q7, r2, r3, #8
[^>]*> ee23 ff64 	vdwdup.u32	q7, r2, r5, #1
[^>]*> ee23 ef64 	viwdup.u32	q7, r2, r5, #1
[^>]*> ee23 ff65 	vdwdup.u32	q7, r2, r5, #2
[^>]*> ee23 ef65 	viwdup.u32	q7, r2, r5, #2
[^>]*> ee23 ffe4 	vdwdup.u32	q7, r2, r5, #4
[^>]*> ee23 efe4 	viwdup.u32	q7, r2, r5, #4
[^>]*> ee23 ffe5 	vdwdup.u32	q7, r2, r5, #8
[^>]*> ee23 efe5 	viwdup.u32	q7, r2, r5, #8
[^>]*> ee23 ff66 	vdwdup.u32	q7, r2, r7, #1
[^>]*> ee23 ef66 	viwdup.u32	q7, r2, r7, #1
[^>]*> ee23 ff67 	vdwdup.u32	q7, r2, r7, #2
[^>]*> ee23 ef67 	viwdup.u32	q7, r2, r7, #2
[^>]*> ee23 ffe6 	vdwdup.u32	q7, r2, r7, #4
[^>]*> ee23 efe6 	viwdup.u32	q7, r2, r7, #4
[^>]*> ee23 ffe7 	vdwdup.u32	q7, r2, r7, #8
[^>]*> ee23 efe7 	viwdup.u32	q7, r2, r7, #8
[^>]*> ee23 ff68 	vdwdup.u32	q7, r2, r9, #1
[^>]*> ee23 ef68 	viwdup.u32	q7, r2, r9, #1
[^>]*> ee23 ff69 	vdwdup.u32	q7, r2, r9, #2
[^>]*> ee23 ef69 	viwdup.u32	q7, r2, r9, #2
[^>]*> ee23 ffe8 	vdwdup.u32	q7, r2, r9, #4
[^>]*> ee23 efe8 	viwdup.u32	q7, r2, r9, #4
[^>]*> ee23 ffe9 	vdwdup.u32	q7, r2, r9, #8
[^>]*> ee23 efe9 	viwdup.u32	q7, r2, r9, #8
[^>]*> ee23 ff6a 	vdwdup.u32	q7, r2, fp, #1
[^>]*> ee23 ef6a 	viwdup.u32	q7, r2, fp, #1
[^>]*> ee23 ff6b 	vdwdup.u32	q7, r2, fp, #2
[^>]*> ee23 ef6b 	viwdup.u32	q7, r2, fp, #2
[^>]*> ee23 ffea 	vdwdup.u32	q7, r2, fp, #4
[^>]*> ee23 efea 	viwdup.u32	q7, r2, fp, #4
[^>]*> ee23 ffeb 	vdwdup.u32	q7, r2, fp, #8
[^>]*> ee23 efeb 	viwdup.u32	q7, r2, fp, #8
[^>]*> ee25 ff6e 	vddup.u32	q7, r4, #1
[^>]*> ee25 ef6e 	vidup.u32	q7, r4, #1
[^>]*> ee25 ff6f 	vddup.u32	q7, r4, #2
[^>]*> ee25 ef6f 	vidup.u32	q7, r4, #2
[^>]*> ee25 ffee 	vddup.u32	q7, r4, #4
[^>]*> ee25 efee 	vidup.u32	q7, r4, #4
[^>]*> ee25 ffef 	vddup.u32	q7, r4, #8
[^>]*> ee25 efef 	vidup.u32	q7, r4, #8
[^>]*> ee25 ff60 	vdwdup.u32	q7, r4, r1, #1
[^>]*> ee25 ef60 	viwdup.u32	q7, r4, r1, #1
[^>]*> ee25 ff61 	vdwdup.u32	q7, r4, r1, #2
[^>]*> ee25 ef61 	viwdup.u32	q7, r4, r1, #2
[^>]*> ee25 ffe0 	vdwdup.u32	q7, r4, r1, #4
[^>]*> ee25 efe0 	viwdup.u32	q7, r4, r1, #4
[^>]*> ee25 ffe1 	vdwdup.u32	q7, r4, r1, #8
[^>]*> ee25 efe1 	viwdup.u32	q7, r4, r1, #8
[^>]*> ee25 ff62 	vdwdup.u32	q7, r4, r3, #1
[^>]*> ee25 ef62 	viwdup.u32	q7, r4, r3, #1
[^>]*> ee25 ff63 	vdwdup.u32	q7, r4, r3, #2
[^>]*> ee25 ef63 	viwdup.u32	q7, r4, r3, #2
[^>]*> ee25 ffe2 	vdwdup.u32	q7, r4, r3, #4
[^>]*> ee25 efe2 	viwdup.u32	q7, r4, r3, #4
[^>]*> ee25 ffe3 	vdwdup.u32	q7, r4, r3, #8
[^>]*> ee25 efe3 	viwdup.u32	q7, r4, r3, #8
[^>]*> ee25 ff64 	vdwdup.u32	q7, r4, r5, #1
[^>]*> ee25 ef64 	viwdup.u32	q7, r4, r5, #1
[^>]*> ee25 ff65 	vdwdup.u32	q7, r4, r5, #2
[^>]*> ee25 ef65 	viwdup.u32	q7, r4, r5, #2
[^>]*> ee25 ffe4 	vdwdup.u32	q7, r4, r5, #4
[^>]*> ee25 efe4 	viwdup.u32	q7, r4, r5, #4
[^>]*> ee25 ffe5 	vdwdup.u32	q7, r4, r5, #8
[^>]*> ee25 efe5 	viwdup.u32	q7, r4, r5, #8
[^>]*> ee25 ff66 	vdwdup.u32	q7, r4, r7, #1
[^>]*> ee25 ef66 	viwdup.u32	q7, r4, r7, #1
[^>]*> ee25 ff67 	vdwdup.u32	q7, r4, r7, #2
[^>]*> ee25 ef67 	viwdup.u32	q7, r4, r7, #2
[^>]*> ee25 ffe6 	vdwdup.u32	q7, r4, r7, #4
[^>]*> ee25 efe6 	viwdup.u32	q7, r4, r7, #4
[^>]*> ee25 ffe7 	vdwdup.u32	q7, r4, r7, #8
[^>]*> ee25 efe7 	viwdup.u32	q7, r4, r7, #8
[^>]*> ee25 ff68 	vdwdup.u32	q7, r4, r9, #1
[^>]*> ee25 ef68 	viwdup.u32	q7, r4, r9, #1
[^>]*> ee25 ff69 	vdwdup.u32	q7, r4, r9, #2
[^>]*> ee25 ef69 	viwdup.u32	q7, r4, r9, #2
[^>]*> ee25 ffe8 	vdwdup.u32	q7, r4, r9, #4
[^>]*> ee25 efe8 	viwdup.u32	q7, r4, r9, #4
[^>]*> ee25 ffe9 	vdwdup.u32	q7, r4, r9, #8
[^>]*> ee25 efe9 	viwdup.u32	q7, r4, r9, #8
[^>]*> ee25 ff6a 	vdwdup.u32	q7, r4, fp, #1
[^>]*> ee25 ef6a 	viwdup.u32	q7, r4, fp, #1
[^>]*> ee25 ff6b 	vdwdup.u32	q7, r4, fp, #2
[^>]*> ee25 ef6b 	viwdup.u32	q7, r4, fp, #2
[^>]*> ee25 ffea 	vdwdup.u32	q7, r4, fp, #4
[^>]*> ee25 efea 	viwdup.u32	q7, r4, fp, #4
[^>]*> ee25 ffeb 	vdwdup.u32	q7, r4, fp, #8
[^>]*> ee25 efeb 	viwdup.u32	q7, r4, fp, #8
[^>]*> ee27 ff6e 	vddup.u32	q7, r6, #1
[^>]*> ee27 ef6e 	vidup.u32	q7, r6, #1
[^>]*> ee27 ff6f 	vddup.u32	q7, r6, #2
[^>]*> ee27 ef6f 	vidup.u32	q7, r6, #2
[^>]*> ee27 ffee 	vddup.u32	q7, r6, #4
[^>]*> ee27 efee 	vidup.u32	q7, r6, #4
[^>]*> ee27 ffef 	vddup.u32	q7, r6, #8
[^>]*> ee27 efef 	vidup.u32	q7, r6, #8
[^>]*> ee27 ff60 	vdwdup.u32	q7, r6, r1, #1
[^>]*> ee27 ef60 	viwdup.u32	q7, r6, r1, #1
[^>]*> ee27 ff61 	vdwdup.u32	q7, r6, r1, #2
[^>]*> ee27 ef61 	viwdup.u32	q7, r6, r1, #2
[^>]*> ee27 ffe0 	vdwdup.u32	q7, r6, r1, #4
[^>]*> ee27 efe0 	viwdup.u32	q7, r6, r1, #4
[^>]*> ee27 ffe1 	vdwdup.u32	q7, r6, r1, #8
[^>]*> ee27 efe1 	viwdup.u32	q7, r6, r1, #8
[^>]*> ee27 ff62 	vdwdup.u32	q7, r6, r3, #1
[^>]*> ee27 ef62 	viwdup.u32	q7, r6, r3, #1
[^>]*> ee27 ff63 	vdwdup.u32	q7, r6, r3, #2
[^>]*> ee27 ef63 	viwdup.u32	q7, r6, r3, #2
[^>]*> ee27 ffe2 	vdwdup.u32	q7, r6, r3, #4
[^>]*> ee27 efe2 	viwdup.u32	q7, r6, r3, #4
[^>]*> ee27 ffe3 	vdwdup.u32	q7, r6, r3, #8
[^>]*> ee27 efe3 	viwdup.u32	q7, r6, r3, #8
[^>]*> ee27 ff64 	vdwdup.u32	q7, r6, r5, #1
[^>]*> ee27 ef64 	viwdup.u32	q7, r6, r5, #1
[^>]*> ee27 ff65 	vdwdup.u32	q7, r6, r5, #2
[^>]*> ee27 ef65 	viwdup.u32	q7, r6, r5, #2
[^>]*> ee27 ffe4 	vdwdup.u32	q7, r6, r5, #4
[^>]*> ee27 efe4 	viwdup.u32	q7, r6, r5, #4
[^>]*> ee27 ffe5 	vdwdup.u32	q7, r6, r5, #8
[^>]*> ee27 efe5 	viwdup.u32	q7, r6, r5, #8
[^>]*> ee27 ff66 	vdwdup.u32	q7, r6, r7, #1
[^>]*> ee27 ef66 	viwdup.u32	q7, r6, r7, #1
[^>]*> ee27 ff67 	vdwdup.u32	q7, r6, r7, #2
[^>]*> ee27 ef67 	viwdup.u32	q7, r6, r7, #2
[^>]*> ee27 ffe6 	vdwdup.u32	q7, r6, r7, #4
[^>]*> ee27 efe6 	viwdup.u32	q7, r6, r7, #4
[^>]*> ee27 ffe7 	vdwdup.u32	q7, r6, r7, #8
[^>]*> ee27 efe7 	viwdup.u32	q7, r6, r7, #8
[^>]*> ee27 ff68 	vdwdup.u32	q7, r6, r9, #1
[^>]*> ee27 ef68 	viwdup.u32	q7, r6, r9, #1
[^>]*> ee27 ff69 	vdwdup.u32	q7, r6, r9, #2
[^>]*> ee27 ef69 	viwdup.u32	q7, r6, r9, #2
[^>]*> ee27 ffe8 	vdwdup.u32	q7, r6, r9, #4
[^>]*> ee27 efe8 	viwdup.u32	q7, r6, r9, #4
[^>]*> ee27 ffe9 	vdwdup.u32	q7, r6, r9, #8
[^>]*> ee27 efe9 	viwdup.u32	q7, r6, r9, #8
[^>]*> ee27 ff6a 	vdwdup.u32	q7, r6, fp, #1
[^>]*> ee27 ef6a 	viwdup.u32	q7, r6, fp, #1
[^>]*> ee27 ff6b 	vdwdup.u32	q7, r6, fp, #2
[^>]*> ee27 ef6b 	viwdup.u32	q7, r6, fp, #2
[^>]*> ee27 ffea 	vdwdup.u32	q7, r6, fp, #4
[^>]*> ee27 efea 	viwdup.u32	q7, r6, fp, #4
[^>]*> ee27 ffeb 	vdwdup.u32	q7, r6, fp, #8
[^>]*> ee27 efeb 	viwdup.u32	q7, r6, fp, #8
[^>]*> ee29 ff6e 	vddup.u32	q7, r8, #1
[^>]*> ee29 ef6e 	vidup.u32	q7, r8, #1
[^>]*> ee29 ff6f 	vddup.u32	q7, r8, #2
[^>]*> ee29 ef6f 	vidup.u32	q7, r8, #2
[^>]*> ee29 ffee 	vddup.u32	q7, r8, #4
[^>]*> ee29 efee 	vidup.u32	q7, r8, #4
[^>]*> ee29 ffef 	vddup.u32	q7, r8, #8
[^>]*> ee29 efef 	vidup.u32	q7, r8, #8
[^>]*> ee29 ff60 	vdwdup.u32	q7, r8, r1, #1
[^>]*> ee29 ef60 	viwdup.u32	q7, r8, r1, #1
[^>]*> ee29 ff61 	vdwdup.u32	q7, r8, r1, #2
[^>]*> ee29 ef61 	viwdup.u32	q7, r8, r1, #2
[^>]*> ee29 ffe0 	vdwdup.u32	q7, r8, r1, #4
[^>]*> ee29 efe0 	viwdup.u32	q7, r8, r1, #4
[^>]*> ee29 ffe1 	vdwdup.u32	q7, r8, r1, #8
[^>]*> ee29 efe1 	viwdup.u32	q7, r8, r1, #8
[^>]*> ee29 ff62 	vdwdup.u32	q7, r8, r3, #1
[^>]*> ee29 ef62 	viwdup.u32	q7, r8, r3, #1
[^>]*> ee29 ff63 	vdwdup.u32	q7, r8, r3, #2
[^>]*> ee29 ef63 	viwdup.u32	q7, r8, r3, #2
[^>]*> ee29 ffe2 	vdwdup.u32	q7, r8, r3, #4
[^>]*> ee29 efe2 	viwdup.u32	q7, r8, r3, #4
[^>]*> ee29 ffe3 	vdwdup.u32	q7, r8, r3, #8
[^>]*> ee29 efe3 	viwdup.u32	q7, r8, r3, #8
[^>]*> ee29 ff64 	vdwdup.u32	q7, r8, r5, #1
[^>]*> ee29 ef64 	viwdup.u32	q7, r8, r5, #1
[^>]*> ee29 ff65 	vdwdup.u32	q7, r8, r5, #2
[^>]*> ee29 ef65 	viwdup.u32	q7, r8, r5, #2
[^>]*> ee29 ffe4 	vdwdup.u32	q7, r8, r5, #4
[^>]*> ee29 efe4 	viwdup.u32	q7, r8, r5, #4
[^>]*> ee29 ffe5 	vdwdup.u32	q7, r8, r5, #8
[^>]*> ee29 efe5 	viwdup.u32	q7, r8, r5, #8
[^>]*> ee29 ff66 	vdwdup.u32	q7, r8, r7, #1
[^>]*> ee29 ef66 	viwdup.u32	q7, r8, r7, #1
[^>]*> ee29 ff67 	vdwdup.u32	q7, r8, r7, #2
[^>]*> ee29 ef67 	viwdup.u32	q7, r8, r7, #2
[^>]*> ee29 ffe6 	vdwdup.u32	q7, r8, r7, #4
[^>]*> ee29 efe6 	viwdup.u32	q7, r8, r7, #4
[^>]*> ee29 ffe7 	vdwdup.u32	q7, r8, r7, #8
[^>]*> ee29 efe7 	viwdup.u32	q7, r8, r7, #8
[^>]*> ee29 ff68 	vdwdup.u32	q7, r8, r9, #1
[^>]*> ee29 ef68 	viwdup.u32	q7, r8, r9, #1
[^>]*> ee29 ff69 	vdwdup.u32	q7, r8, r9, #2
[^>]*> ee29 ef69 	viwdup.u32	q7, r8, r9, #2
[^>]*> ee29 ffe8 	vdwdup.u32	q7, r8, r9, #4
[^>]*> ee29 efe8 	viwdup.u32	q7, r8, r9, #4
[^>]*> ee29 ffe9 	vdwdup.u32	q7, r8, r9, #8
[^>]*> ee29 efe9 	viwdup.u32	q7, r8, r9, #8
[^>]*> ee29 ff6a 	vdwdup.u32	q7, r8, fp, #1
[^>]*> ee29 ef6a 	viwdup.u32	q7, r8, fp, #1
[^>]*> ee29 ff6b 	vdwdup.u32	q7, r8, fp, #2
[^>]*> ee29 ef6b 	viwdup.u32	q7, r8, fp, #2
[^>]*> ee29 ffea 	vdwdup.u32	q7, r8, fp, #4
[^>]*> ee29 efea 	viwdup.u32	q7, r8, fp, #4
[^>]*> ee29 ffeb 	vdwdup.u32	q7, r8, fp, #8
[^>]*> ee29 efeb 	viwdup.u32	q7, r8, fp, #8
[^>]*> ee2b ff6e 	vddup.u32	q7, sl, #1
[^>]*> ee2b ef6e 	vidup.u32	q7, sl, #1
[^>]*> ee2b ff6f 	vddup.u32	q7, sl, #2
[^>]*> ee2b ef6f 	vidup.u32	q7, sl, #2
[^>]*> ee2b ffee 	vddup.u32	q7, sl, #4
[^>]*> ee2b efee 	vidup.u32	q7, sl, #4
[^>]*> ee2b ffef 	vddup.u32	q7, sl, #8
[^>]*> ee2b efef 	vidup.u32	q7, sl, #8
[^>]*> ee2b ff60 	vdwdup.u32	q7, sl, r1, #1
[^>]*> ee2b ef60 	viwdup.u32	q7, sl, r1, #1
[^>]*> ee2b ff61 	vdwdup.u32	q7, sl, r1, #2
[^>]*> ee2b ef61 	viwdup.u32	q7, sl, r1, #2
[^>]*> ee2b ffe0 	vdwdup.u32	q7, sl, r1, #4
[^>]*> ee2b efe0 	viwdup.u32	q7, sl, r1, #4
[^>]*> ee2b ffe1 	vdwdup.u32	q7, sl, r1, #8
[^>]*> ee2b efe1 	viwdup.u32	q7, sl, r1, #8
[^>]*> ee2b ff62 	vdwdup.u32	q7, sl, r3, #1
[^>]*> ee2b ef62 	viwdup.u32	q7, sl, r3, #1
[^>]*> ee2b ff63 	vdwdup.u32	q7, sl, r3, #2
[^>]*> ee2b ef63 	viwdup.u32	q7, sl, r3, #2
[^>]*> ee2b ffe2 	vdwdup.u32	q7, sl, r3, #4
[^>]*> ee2b efe2 	viwdup.u32	q7, sl, r3, #4
[^>]*> ee2b ffe3 	vdwdup.u32	q7, sl, r3, #8
[^>]*> ee2b efe3 	viwdup.u32	q7, sl, r3, #8
[^>]*> ee2b ff64 	vdwdup.u32	q7, sl, r5, #1
[^>]*> ee2b ef64 	viwdup.u32	q7, sl, r5, #1
[^>]*> ee2b ff65 	vdwdup.u32	q7, sl, r5, #2
[^>]*> ee2b ef65 	viwdup.u32	q7, sl, r5, #2
[^>]*> ee2b ffe4 	vdwdup.u32	q7, sl, r5, #4
[^>]*> ee2b efe4 	viwdup.u32	q7, sl, r5, #4
[^>]*> ee2b ffe5 	vdwdup.u32	q7, sl, r5, #8
[^>]*> ee2b efe5 	viwdup.u32	q7, sl, r5, #8
[^>]*> ee2b ff66 	vdwdup.u32	q7, sl, r7, #1
[^>]*> ee2b ef66 	viwdup.u32	q7, sl, r7, #1
[^>]*> ee2b ff67 	vdwdup.u32	q7, sl, r7, #2
[^>]*> ee2b ef67 	viwdup.u32	q7, sl, r7, #2
[^>]*> ee2b ffe6 	vdwdup.u32	q7, sl, r7, #4
[^>]*> ee2b efe6 	viwdup.u32	q7, sl, r7, #4
[^>]*> ee2b ffe7 	vdwdup.u32	q7, sl, r7, #8
[^>]*> ee2b efe7 	viwdup.u32	q7, sl, r7, #8
[^>]*> ee2b ff68 	vdwdup.u32	q7, sl, r9, #1
[^>]*> ee2b ef68 	viwdup.u32	q7, sl, r9, #1
[^>]*> ee2b ff69 	vdwdup.u32	q7, sl, r9, #2
[^>]*> ee2b ef69 	viwdup.u32	q7, sl, r9, #2
[^>]*> ee2b ffe8 	vdwdup.u32	q7, sl, r9, #4
[^>]*> ee2b efe8 	viwdup.u32	q7, sl, r9, #4
[^>]*> ee2b ffe9 	vdwdup.u32	q7, sl, r9, #8
[^>]*> ee2b efe9 	viwdup.u32	q7, sl, r9, #8
[^>]*> ee2b ff6a 	vdwdup.u32	q7, sl, fp, #1
[^>]*> ee2b ef6a 	viwdup.u32	q7, sl, fp, #1
[^>]*> ee2b ff6b 	vdwdup.u32	q7, sl, fp, #2
[^>]*> ee2b ef6b 	viwdup.u32	q7, sl, fp, #2
[^>]*> ee2b ffea 	vdwdup.u32	q7, sl, fp, #4
[^>]*> ee2b efea 	viwdup.u32	q7, sl, fp, #4
[^>]*> ee2b ffeb 	vdwdup.u32	q7, sl, fp, #8
[^>]*> ee2b efeb 	viwdup.u32	q7, sl, fp, #8
[^>]*> ee2d ff6e 	vddup.u32	q7, ip, #1
[^>]*> ee2d ef6e 	vidup.u32	q7, ip, #1
[^>]*> ee2d ff6f 	vddup.u32	q7, ip, #2
[^>]*> ee2d ef6f 	vidup.u32	q7, ip, #2
[^>]*> ee2d ffee 	vddup.u32	q7, ip, #4
[^>]*> ee2d efee 	vidup.u32	q7, ip, #4
[^>]*> ee2d ffef 	vddup.u32	q7, ip, #8
[^>]*> ee2d efef 	vidup.u32	q7, ip, #8
[^>]*> ee2d ff60 	vdwdup.u32	q7, ip, r1, #1
[^>]*> ee2d ef60 	viwdup.u32	q7, ip, r1, #1
[^>]*> ee2d ff61 	vdwdup.u32	q7, ip, r1, #2
[^>]*> ee2d ef61 	viwdup.u32	q7, ip, r1, #2
[^>]*> ee2d ffe0 	vdwdup.u32	q7, ip, r1, #4
[^>]*> ee2d efe0 	viwdup.u32	q7, ip, r1, #4
[^>]*> ee2d ffe1 	vdwdup.u32	q7, ip, r1, #8
[^>]*> ee2d efe1 	viwdup.u32	q7, ip, r1, #8
[^>]*> ee2d ff62 	vdwdup.u32	q7, ip, r3, #1
[^>]*> ee2d ef62 	viwdup.u32	q7, ip, r3, #1
[^>]*> ee2d ff63 	vdwdup.u32	q7, ip, r3, #2
[^>]*> ee2d ef63 	viwdup.u32	q7, ip, r3, #2
[^>]*> ee2d ffe2 	vdwdup.u32	q7, ip, r3, #4
[^>]*> ee2d efe2 	viwdup.u32	q7, ip, r3, #4
[^>]*> ee2d ffe3 	vdwdup.u32	q7, ip, r3, #8
[^>]*> ee2d efe3 	viwdup.u32	q7, ip, r3, #8
[^>]*> ee2d ff64 	vdwdup.u32	q7, ip, r5, #1
[^>]*> ee2d ef64 	viwdup.u32	q7, ip, r5, #1
[^>]*> ee2d ff65 	vdwdup.u32	q7, ip, r5, #2
[^>]*> ee2d ef65 	viwdup.u32	q7, ip, r5, #2
[^>]*> ee2d ffe4 	vdwdup.u32	q7, ip, r5, #4
[^>]*> ee2d efe4 	viwdup.u32	q7, ip, r5, #4
[^>]*> ee2d ffe5 	vdwdup.u32	q7, ip, r5, #8
[^>]*> ee2d efe5 	viwdup.u32	q7, ip, r5, #8
[^>]*> ee2d ff66 	vdwdup.u32	q7, ip, r7, #1
[^>]*> ee2d ef66 	viwdup.u32	q7, ip, r7, #1
[^>]*> ee2d ff67 	vdwdup.u32	q7, ip, r7, #2
[^>]*> ee2d ef67 	viwdup.u32	q7, ip, r7, #2
[^>]*> ee2d ffe6 	vdwdup.u32	q7, ip, r7, #4
[^>]*> ee2d efe6 	viwdup.u32	q7, ip, r7, #4
[^>]*> ee2d ffe7 	vdwdup.u32	q7, ip, r7, #8
[^>]*> ee2d efe7 	viwdup.u32	q7, ip, r7, #8
[^>]*> ee2d ff68 	vdwdup.u32	q7, ip, r9, #1
[^>]*> ee2d ef68 	viwdup.u32	q7, ip, r9, #1
[^>]*> ee2d ff69 	vdwdup.u32	q7, ip, r9, #2
[^>]*> ee2d ef69 	viwdup.u32	q7, ip, r9, #2
[^>]*> ee2d ffe8 	vdwdup.u32	q7, ip, r9, #4
[^>]*> ee2d efe8 	viwdup.u32	q7, ip, r9, #4
[^>]*> ee2d ffe9 	vdwdup.u32	q7, ip, r9, #8
[^>]*> ee2d efe9 	viwdup.u32	q7, ip, r9, #8
[^>]*> ee2d ff6a 	vdwdup.u32	q7, ip, fp, #1
[^>]*> ee2d ef6a 	viwdup.u32	q7, ip, fp, #1
[^>]*> ee2d ff6b 	vdwdup.u32	q7, ip, fp, #2
[^>]*> ee2d ef6b 	viwdup.u32	q7, ip, fp, #2
[^>]*> ee2d ffea 	vdwdup.u32	q7, ip, fp, #4
[^>]*> ee2d efea 	viwdup.u32	q7, ip, fp, #4
[^>]*> ee2d ffeb 	vdwdup.u32	q7, ip, fp, #8
[^>]*> ee2d efeb 	viwdup.u32	q7, ip, fp, #8
[^>]*> fe71 cf4d 	vpstet
[^>]*> ee01 1f60 	vdwdupt.u8	q0, r0, r1, #1
[^>]*> ee11 1fe0 	vdwdupe.u16	q0, r0, r1, #4
[^>]*> ee25 5f66 	vdwdupt.u32	q2, r4, r7, #1
[^>]*> fe71 cf4d 	vpstet
[^>]*> ee01 1f6f 	vddupt.u8	q0, r0, #2
[^>]*> ee11 ff6e 	vddupe.u16	q7, r0, #1
[^>]*> ee29 9f6e 	vddupt.u32	q4, r8, #1
[^>]*> fe71 cf4d 	vpstet
[^>]*> ee01 0f60 	viwdupt.u8	q0, r0, r1, #1
[^>]*> ee11 0fe0 	viwdupe.u16	q0, r0, r1, #4
[^>]*> ee25 4f66 	viwdupt.u32	q2, r4, r7, #1
[^>]*> fe71 cf4d 	vpstet
[^>]*> ee01 0f6f 	vidupt.u8	q0, r0, #2
[^>]*> ee11 ef6e 	vidupe.u16	q7, r0, #1
[^>]*> ee29 8f6e 	vidupt.u32	q4, r8, #1
