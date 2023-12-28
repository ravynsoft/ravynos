#define NT_GNU_PROPERTY_TYPE_0 5

#define GNU_PROPERTY_STACK_SIZE 1
#define GNU_PROPERTY_X86_ISA_1_USED 0xc0010002
#define GNU_PROPERTY_X86_ISA_1_NEEDED 0xc0008002
#define GNU_PROPERTY_X86_FEATURE_1_AND 0xc0000002

#if __SIZEOF_PTRDIFF_T__  == 8
# define ALIGN 3
#elif __SIZEOF_PTRDIFF_T__  == 4
# define ALIGN 2
#endif

	.text
	.globl _start
_start:
	ret

        .section ".note.gnu.property", "a"
        .p2align ALIGN

        .long 1f - 0f           /* name length */
        .long 5f - 2f           /* data length */
        .long NT_GNU_PROPERTY_TYPE_0    /* note type */
0:      .asciz "GNU"            /* vendor name */
1:
        .p2align ALIGN
2:      .long GNU_PROPERTY_STACK_SIZE   /* pr_type.  */
        .long 4f - 3f   /* pr_datasz.  */
3:
        .dc.a 0x800     /* Stack size.  */
4:
        .p2align ALIGN
	.long GNU_PROPERTY_X86_ISA_1_USED
	.long 4
	.byte 0x01,0x10,0x00,0x00
        .p2align ALIGN
	.long GNU_PROPERTY_X86_ISA_1_NEEDED
	.long 4
	.byte 0x01,0x10,0x00,0x00
        .p2align ALIGN
	.long GNU_PROPERTY_X86_FEATURE_1_AND
	.long 4
	.byte 0x01,0x00,0x00,0x00
        .p2align ALIGN
5:
