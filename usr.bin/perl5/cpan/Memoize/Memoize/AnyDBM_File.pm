use strict; use warnings;

package Memoize::AnyDBM_File;
our $VERSION = '1.16';

our @ISA = qw(DB_File GDBM_File Memoize::NDBM_File SDBM_File ODBM_File) unless @ISA;

for my $mod (@ISA) {
  if (eval "require $mod") {
    $mod = 'NDBM_File'
		if $mod eq 'Memoize::NDBM_File'
		and eval { NDBM_File->VERSION( '1.16' ) };
    print STDERR "AnyDBM_File => Selected $mod.\n" if our $Verbose;
    @ISA = $mod;
    return 1;
  }
}

die "No DBM package was successfully found or installed";

__END__

=pod

=head1 NAME

Memoize::AnyDBM_File - glue to provide EXISTS for AnyDBM_File for Storable use

=head1 DESCRIPTION

This class does the same thing as L<AnyDBM_File>, except that instead of
L<NDBM_File> itself it loads L<Memoize::NDBM_File> if L<NDBM_File> lacks
L<EXISTS|perltie/C<EXISTS>> support.

Code which requires perl 5.37.3 or newer should simply use L<AnyBDM_File> directly.

=cut
