	.text
	.global _start
_start:
	add r2,zero,zero
	j external1

        add r3,r2,r2
	beqzt zero,external2

	{ movei r2,external_8a; movei r3,external_8b }
	{ movei r2,external_8a; movei r3,external_8b; ld zero,zero }
	{ mtspr external_8a,zero }
	{ mfspr zero,external_8a }
	{ moveli r2,external_16a; moveli r3,external_16b }

	{ moveli r2,hw1_last(external_32a); moveli r3,hw1_last(external_32b) }
	{ shl16insli r2,r2,hw0(external_32a); shl16insli r3,r3,hw0(external_32b) }

	{ moveli r2,hw2_last(external_48a); moveli r3,hw2_last(external_48b) }
	{ shl16insli r2,r2,hw1(external_48a); shl16insli r3,r3,hw1(external_48b) }
	{ shl16insli r2,r2,hw0(external_48a); shl16insli r3,r3,hw0(external_48b) }

	{ moveli r2,hw3_last(external_64a); moveli r3,hw3_last(external_64b) }
	{ shl16insli r2,r2,hw2(external_64a); shl16insli r3,r3,hw2(external_64b) }
	{ shl16insli r2,r2,hw1(external_64a); shl16insli r3,r3,hw1(external_64b) }
	{ shl16insli r2,r2,hw0(external_64a); shl16insli r3,r3,hw0(external_64b) }

	{ ld_add r0,r0,external_8a }
	{ st_add r0,r0,external_8a }
	{ mm r2,r3,external_5a,external_5b }
	{ shli r2,r3,external_5a; shli r4,r5,external_5b }
	{ shli r2,r3,external_5a; shli r4,r5,external_5b; ld zero,zero }

        { moveli r0, external1 - .; moveli r1, external1 - . }
        { moveli r0, hw1_last(external_data1 - .)
	  moveli r1, hw1_last(external_data1 - .) }
        { moveli r0, hw0(external_data1 - .)
	  moveli r1, hw0(external_data1 - .) }
        { moveli r0, hw2_last(external_data1 - . + 0x100000000000)
	  moveli r1, hw2_last(external_data1 - . + 0x100000000000) }
        { moveli r0, hw1(external_data1      - . + 0x100000000000)
	  moveli r1, hw1(external_data1      - . + 0x100000000000) }
        { moveli r0, hw0(external_data1      - . + 0x100000000000)
	  moveli r1, hw0(external_data1      - . + 0x100000000000) }
        { moveli r0, hw3_last(external_data1 - . + 0x1000000000000000)
	  moveli r1, hw3_last(external_data1 - . + 0x1000000000000000) }
        { moveli r0, hw2(external_data1      - . + 0x1000000000000000)
	  moveli r1, hw2(external_data1      - . + 0x1000000000000000) }
        { moveli r0, hw1(external_data1      - . + 0x1000000000000000)
	  moveli r1, hw1(external_data1      - . + 0x1000000000000000) }
        { moveli r0, hw0(external_data1      - . + 0x1000000000000000)
	  moveli r1, hw0(external_data1      - . + 0x1000000000000000) }
        
	.data
	.align 0x20
	.int external1
	.int external2
	.short external_16a, external_16b
	.byte external_8a, external_8b

	.int (external_data1-.)
	.short (external_data1-.)
	.byte (external_data1-.)

	.short hw0_last(external_16a)

	.short hw1_last(external_32a)
	.short hw0(external_32a)

	.short hw2_last(external_48a)
	.short hw1(external_48a)
	.short hw0(external_48a)

	.short hw3(external_64a)
	.short hw2(external_64a)
	.short hw1(external_64a)
	.short hw0(external_64a)
