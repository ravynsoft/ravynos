#objdump: -dr --prefix-addresses --show-raw-insn -Mmsa
#name: MSA relax
#as: -32 -mmsa -relax-branch
#warning_output: msa-relax.l
#source: msa-relax.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 8380 fffe 	bnz\.b	\$w0,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83a1 fffe 	bnz\.h	\$w1,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83c2 fffe 	bnz\.w	\$w2,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83e3 fffe 	bnz\.d	\$w3,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8304 fffe 	bz\.b	\$w4,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8325 fffe 	bz\.h	\$w5,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8346 fffe 	bz\.w	\$w6,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8367 fffe 	bz\.d	\$w7,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 81e8 fffe 	bnz\.v	\$w8,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8169 fffe 	bz\.v	\$w9,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	bar
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 838a fffe 	bnz\.b	\$w10,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83ab fffe 	bnz\.h	\$w11,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83cc fffe 	bnz\.w	\$w12,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 83ed fffe 	bnz\.d	\$w13,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 830e fffe 	bz\.b	\$w14,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 832f fffe 	bz\.h	\$w15,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8350 fffe 	bz\.w	\$w16,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8371 fffe 	bz\.d	\$w17,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 81f2 fffe 	bnz\.v	\$w18,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 8173 fffe 	bz\.v	\$w19,[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> d400 0000 	j	[0-9a-f]+ <[^>]*>
[	]*[0-9a-f]+: R_MICROMIPS_26_S1	foo
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
