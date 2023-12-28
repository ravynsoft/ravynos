% { dg-do assemble { target mmix-*-* } }

% One more than greg9.s is one too many.
% The error is reported on the wrong line.  Methinks that error is
% attributable to the .rept machinery.  No xfail+bogus for this one.

Main SWYM 0
	.rept 223
	GREG	% { dg-error "too many GREG registers allocated" "" }
	.endr
