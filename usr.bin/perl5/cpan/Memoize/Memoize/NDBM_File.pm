use strict; use warnings;

package Memoize::NDBM_File;
our $VERSION = '1.16';

use NDBM_File;
our @ISA = qw(NDBM_File);

# NDBM_File cannot store undef and will store an empty string if you try
# but it does return undef if you try to read a non-existent key
# so we can emulate exists() using defined()
sub EXISTS {
	defined shift->FETCH(@_);
}

# Perl 5.37.3 adds this EXISTS emulation to NDBM_File itself
delete $Memoize::NDBM_File::{'EXISTS'}
	if eval { NDBM_File->VERSION( '1.16' ) };

1;

__END__

=pod

=head1 NAME

Memoize::NDBM_File - glue to provide EXISTS for NDBM_File for Storable use

=head1 DESCRIPTION

This class provides L<EXISTS|perltie/C<EXISTS>> support for L<NDBM_File>.

L<In Perl 5.37.3|https://github.com/Perl/perl5/commit/c0a1a377c02ed789f5eff667f46a2314a05c5a4c>,
support for C<EXISTS> was added to L<NDBM_File> itself.
Code which requires such a perl should simply use L<NBDM_File> directly.

=cut
