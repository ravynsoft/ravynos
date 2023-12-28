	.type a_val, %gnu_unique_object
a_val:	.long 0
	.size a_val, .-a_val

        .type main,"function"
        .global main
main:
        .long 0

	.section	.note.GNU-stack,""

