# name: MVE Floating point load multiple and store multiple instructions.
# as: -march=armv8.1-m.main+mve
# objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
[^>]*> ecb0 8a01 	vldmia	r0!, {s16}
[^>]*> ed30 8a01 	vldmdb	r0!, {s16}
[^>]*> eca0 8a01 	vstmia	r0!, {s16}
[^>]*> ed20 8a01 	vstmdb	r0!, {s16}
[^>]*> ecb1 8a01 	vldmia	r1!, {s16}
[^>]*> ed31 8a01 	vldmdb	r1!, {s16}
[^>]*> eca1 8a01 	vstmia	r1!, {s16}
[^>]*> ed21 8a01 	vstmdb	r1!, {s16}
[^>]*> ecb2 8a01 	vldmia	r2!, {s16}
[^>]*> ed32 8a01 	vldmdb	r2!, {s16}
[^>]*> eca2 8a01 	vstmia	r2!, {s16}
[^>]*> ed22 8a01 	vstmdb	r2!, {s16}
[^>]*> ecb4 8a01 	vldmia	r4!, {s16}
[^>]*> ed34 8a01 	vldmdb	r4!, {s16}
[^>]*> eca4 8a01 	vstmia	r4!, {s16}
[^>]*> ed24 8a01 	vstmdb	r4!, {s16}
[^>]*> ecb7 8a01 	vldmia	r7!, {s16}
[^>]*> ed37 8a01 	vldmdb	r7!, {s16}
[^>]*> eca7 8a01 	vstmia	r7!, {s16}
[^>]*> ed27 8a01 	vstmdb	r7!, {s16}
[^>]*> ecb8 8a01 	vldmia	r8!, {s16}
[^>]*> ed38 8a01 	vldmdb	r8!, {s16}
[^>]*> eca8 8a01 	vstmia	r8!, {s16}
[^>]*> ed28 8a01 	vstmdb	r8!, {s16}
[^>]*> ecba 8a01 	vldmia	sl!, {s16}
[^>]*> ed3a 8a01 	vldmdb	sl!, {s16}
[^>]*> ecaa 8a01 	vstmia	sl!, {s16}
[^>]*> ed2a 8a01 	vstmdb	sl!, {s16}
[^>]*> ecbc 8a01 	vldmia	ip!, {s16}
[^>]*> ed3c 8a01 	vldmdb	ip!, {s16}
[^>]*> ecac 8a01 	vstmia	ip!, {s16}
[^>]*> ed2c 8a01 	vstmdb	ip!, {s16}
[^>]*> ecbe 8a01 	vldmia	lr!, {s16}
[^>]*> ed3e 8a01 	vldmdb	lr!, {s16}
[^>]*> ecae 8a01 	vstmia	lr!, {s16}
[^>]*> ed2e 8a01 	vstmdb	lr!, {s16}
