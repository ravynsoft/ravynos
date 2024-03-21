! Test architecture mismatch warnings from v9b upwards.
! We don't test every possible mismatch, we just want to be reasonable sure
! the mismatch checking code works.
!
! { dg-do assemble { target sparc*-*-* } }
! { dg-options -Av9b }

! v9d

        addxc %g1,%g2,%g3               ! { dg-error "mismatch|v9d" "v9d addxc mismatch" }
        
! v9e

	aes_kexpand0 %f0,%f2,%f4	! { dg-error "mismatch|v9e" "v9b aes_kexpand0  mismatch" }

! v9v

	fnumaddd %f0,%f2,%f0,%f4	! { dg-error "mismatch|v9v" "v9v fnumaddd  mismatch" }

! v9m

	xmpmul 4			! { dg-error "mismatch|v9m" "v9m xmpmul  mismatch" }
foo:
