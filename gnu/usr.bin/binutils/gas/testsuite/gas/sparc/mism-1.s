! Test architecture mismatch warnings.
! We don't test every possible mismatch, we just want to be reasonable sure
! the mismatch checking code works.
!
! { dg-do assemble { target sparc*-*-* } }
! { dg-options -Av6 }

! sparclite

	divscc %g1,%g2,%g3	! { dg-error "mismatch|sparclite" "sparclite divscc mismatch" }

	scan %g1,%g2,%g3	! { dg-error "mismatch|sparclite" "sparclite scan mismatch" }

! v9

	movrz %g1,%g2,%g3	! { dg-error "mismatch|v9" "v9 fp reg mismatch" }

! v9a

	shutdown		! { dg-error "mismatch|v9a" "v9a shutdown mismatch" }

! v9b

	edge8n %g1,%g2,%g3	! { dg-error "mismatch|v9b" "v9b edge8n mismatch" }

! v9e

	aes_kexpand0 %f0,%f2,%f4 ! { dg-error "mismatch|v9e" "v9b aes_kexpand0  mismatch" }
foo:
