#objdump: -dr --prefix-addresses --show-raw-insn -Mmsa
#name: MSA branch reorder
#as: -32 -mmsa
#source: msa-branch.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8300 fffe 	bz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8301 fffe 	bz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8302 fffe 	bz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8300 fffe 	bz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8301 fffe 	bz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8302 fffe 	bz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8300 fffe 	bz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8301 fffe 	bz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8302 fffe 	bz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8320 fffe 	bz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8321 fffe 	bz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8322 fffe 	bz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8320 fffe 	bz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8321 fffe 	bz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8322 fffe 	bz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8320 fffe 	bz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8321 fffe 	bz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8322 fffe 	bz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8340 fffe 	bz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8341 fffe 	bz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8342 fffe 	bz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8340 fffe 	bz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8341 fffe 	bz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8342 fffe 	bz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8340 fffe 	bz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8341 fffe 	bz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8342 fffe 	bz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8360 fffe 	bz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8361 fffe 	bz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8362 fffe 	bz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8360 fffe 	bz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8361 fffe 	bz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8362 fffe 	bz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8360 fffe 	bz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8361 fffe 	bz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8362 fffe 	bz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8160 fffe 	bz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8161 fffe 	bz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8162 fffe 	bz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8160 fffe 	bz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8161 fffe 	bz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8162 fffe 	bz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8160 fffe 	bz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8161 fffe 	bz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8162 fffe 	bz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8380 fffe 	bnz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8381 fffe 	bnz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 8382 fffe 	bnz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8380 fffe 	bnz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8381 fffe 	bnz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 8382 fffe 	bnz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8380 fffe 	bnz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8381 fffe 	bnz\.b	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 8382 fffe 	bnz\.b	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83a0 fffe 	bnz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83a1 fffe 	bnz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83a2 fffe 	bnz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83a0 fffe 	bnz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83a1 fffe 	bnz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83a2 fffe 	bnz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83a0 fffe 	bnz\.h	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83a1 fffe 	bnz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83a2 fffe 	bnz\.h	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83c0 fffe 	bnz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83c1 fffe 	bnz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83c2 fffe 	bnz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83c0 fffe 	bnz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83c1 fffe 	bnz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83c2 fffe 	bnz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83c0 fffe 	bnz\.w	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83c1 fffe 	bnz\.w	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83c2 fffe 	bnz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83e0 fffe 	bnz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83e1 fffe 	bnz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 83e2 fffe 	bnz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83e0 fffe 	bnz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83e1 fffe 	bnz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 83e2 fffe 	bnz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83e0 fffe 	bnz\.d	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83e1 fffe 	bnz\.d	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 83e2 fffe 	bnz\.d	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 81e0 fffe 	bnz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 81e1 fffe 	bnz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 81e2 fffe 	bnz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5aa2 080e 	fsune\.d	\$w0,\$w1,\$w2
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 81e0 fffe 	bnz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 81e1 fffe 	bnz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 81e2 fffe 	bnz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5441 0030 	add\.s	\$f0,\$f1,\$f2
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 81e0 fffe 	bnz\.v	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 81e1 fffe 	bnz\.v	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
[0-9a-f]+ <[^>]*> 81e2 fffe 	bnz\.v	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	test
[0-9a-f]+ <[^>]*> 5482 0130 	add\.d	\$f0,\$f2,\$f4
	\.\.\.
