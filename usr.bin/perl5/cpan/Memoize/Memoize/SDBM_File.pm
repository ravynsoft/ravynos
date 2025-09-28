use strict; use warnings;

package Memoize::SDBM_File;
our $VERSION = '1.16';

use SDBM_File 1.01; # for EXISTS support
our @ISA = qw(SDBM_File);

1;

__END__

=pod

=head1 NAME

Memoize::SDBM_File - DEPRECATED compability shim

=head1 DESCRIPTION

This class used to provide L<EXISTS|perltie/C<EXISTS>> support for L<SDBM_File>
before support for C<EXISTS> was added to L<SDBM_File> itself
L<in Perl 5.6.0|perl56delta/SDBM_File>.

Any code still using this class should be rewritten to use L<SBDM_File> directly.

=cut
