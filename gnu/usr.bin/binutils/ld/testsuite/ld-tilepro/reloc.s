	.text
	.global _start
_start:
	add r2,zero,zero
	j external1

        add r3,r2,r2
	bzt zero,external2

	{ movei r2,external_8a; movei r3,external_8b }
	{ movei r2,external_8a; movei r3,external_8b; lw zero,zero }
	{ mtspr external_8a,zero }
	{ mfspr zero,external_8a }
	{ moveli r2,external_16a; moveli r3,external_16b }

	{ moveli r2,lo16(external_32a); moveli r3,lo16(external_32b) }
	{ addli r2,r2,hi16(external_32a); addli r3,r3,hi16(external_32b) }
	{ auli r2,r2,ha16(external_32a); auli r3,r3,ha16(external_32b) }

	{ swadd r0,r0,external_8a }
	{ mm r2,r3,r4,external_5a,external_5b }
        { nop; mm r5,r6,r7,external_5a,external_5b }
	{ shli r2,r3,external_5a; shli r4,r5,external_5b }
	{ shli r2,r3,external_5a; shli r4,r5,external_5b; lw zero,zero }

        moveli r0, external1 - .
        moveli r0, lo16(external_data1 - .)
        moveli r0, hi16(external_data1 - . + 30000)
        moveli r0, ha16(external_data1 - . + 30000)
        
	.data
	.align 0x20
	.int external1
	.int external2
	.short external_16a, external_16b
	.byte external_8a, external_8b

	.int (external_data1-.)
	.short (external_data1-.)
	.byte (external_data1-.)

	.short lo16(external_32a)
	.short lo16(external_32b)
	.short hi16(external_32a)
	.short hi16(external_32b)
	.short ha16(external_32a)
	.short ha16(external_32b)
