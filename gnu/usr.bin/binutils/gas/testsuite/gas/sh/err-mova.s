! { dg-do assemble }

! Check that an error occurs on mova instructions with an unaligned or
! negative offset.

negative:
	.word 0

	.align 2
start:
	mova start, r0      ! { dg-error "negative offset|pcrel too far" }
	mova negative, r0   ! { dg-error "negative offset|pcrel too far" }
	mova aligned, r0    ! ok
	mova unaligned, r0  ! { dg-error "unaligned destination" }

	.align 2
aligned:
	.word 1
unaligned:
	.word 2
