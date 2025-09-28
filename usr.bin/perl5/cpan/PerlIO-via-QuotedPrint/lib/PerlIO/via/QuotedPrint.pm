# Copyright (C) 2002-2004, 2012 Elizabeth Mattijsen.  All rights reserved.
# Copyright (C) 2015 Steve Hay.  All rights reserved.

# This module is free software; you can redistribute it and/or modify it under
# the same terms as Perl itself, i.e. under the terms of either the GNU General
# Public License or the Artistic License, as specified in the F<LICENCE> file.

package PerlIO::via::QuotedPrint;

use 5.008001;

# be as strict as possible
use strict;

our $VERSION = '0.10';

# modules that we need
use MIME::QuotedPrint (); # no need to pollute this namespace

# satisfy -require-
1;

#-------------------------------------------------------------------------------
#
# Standard Perl features
#
#-------------------------------------------------------------------------------
#  IN: 1 class to bless with
#      2 mode string (ignored)
#      3 file handle of PerlIO layer below (ignored)
# OUT: 1 blessed object

sub PUSHED { bless \*PUSHED,$_[0] } #PUSHED

#-------------------------------------------------------------------------------
#  IN: 1 instantiated object (ignored)
#      2 handle to read from
# OUT: 1 decoded string

sub FILL {

    # decode and return
    my $line= readline( $_[1] );
    return ( defined $line )
      ? MIME::QuotedPrint::decode_qp($line)
      : undef;
} #FILL

#-------------------------------------------------------------------------------
#  IN: 1 instantiated object (ignored)
#      2 buffer to be written
#      3 handle to write to
# OUT: 1 number of bytes written

sub WRITE {

    # encode and write to handle: indicate result
    return ( print { $_[2] } MIME::QuotedPrint::encode_qp( $_[1] ) )
      ? length( $_[1] )
      : -1;
} #WRITE

#-------------------------------------------------------------------------------

__END__

=head1 NAME

PerlIO::via::QuotedPrint - PerlIO layer for quoted-printable strings

=head1 SYNOPSIS

    use PerlIO::via::QuotedPrint;

    open(my $in, '<:via(QuotedPrint)', 'file.qp') or
        die "Can't open file.qp for reading: $!\n";

    open(my $out, '>:via(QuotedPrint)', 'file.qp') or
        die "Can't open file.qp for writing: $!\n";

=head1 DESCRIPTION

This module implements a PerlIO layer that works on files encoded in the
quoted-printable format.  It will decode from quoted-printable while reading
from a handle, and it will encode as quoted-printable while writing to a handle.

=head1 EXPORTS

I<None>.

=head1 KNOWN BUGS

I<None>.

=head1 FEEDBACK

Patches, bug reports, suggestions or any other feedback is welcome.

Patches can be sent as GitHub pull requests at
L<https://github.com/steve-m-hay/PerlIO-via-QuotedPrint/pulls>.

Bug reports and suggestions can be made on the CPAN Request Tracker at
L<https://rt.cpan.org/Public/Bug/Report.html?Queue=PerlIO-via-QuotedPrint>.

Currently active requests on the CPAN Request Tracker can be viewed at
L<https://rt.cpan.org/Public/Dist/Display.html?Status=Active;Queue=PerlIO-via-QuotedPrint>.

Please test this distribution.  See CPAN Testers Reports at
L<https://www.cpantesters.org/> for details of how to get involved.

Previous test results on CPAN Testers Reports can be viewed at
L<https://www.cpantesters.org/distro/P/PerlIO-via-QuotedPrint.html>.

Please rate this distribution on CPAN Ratings at
L<https://cpanratings.perl.org/rate/?distribution=PerlIO-via-QuotedPrint>.

=head1 SEE ALSO

L<PerlIO::via>,
L<MIME::QuotedPrint>.

=head1 ACKNOWLEDGEMENTS

Based on an example in the standard library module MIME::QuotedPrint in Perl
(version 5.8.0).

=head1 AVAILABILITY

The latest version of this module is available from CPAN (see
L<perlmodlib/"CPAN"> for details) at

L<https://metacpan.org/release/PerlIO-via-QuotedPrint> or

L<https://www.cpan.org/authors/id/S/SH/SHAY/> or

L<https://www.cpan.org/modules/by-module/PerlIO/>.

The latest source code is available from GitHub at
L<https://github.com/steve-m-hay/PerlIO-via-QuotedPrint>.

=head1 INSTALLATION

See the F<INSTALL> file.

=head1 AUTHOR

Elizabeth Mattijsen E<lt>L<liz@dijkmat.nl|mailto:liz@dijkmat.nl>E<gt>.

Steve Hay E<lt>L<shay@cpan.org|mailto:shay@cpan.org>E<gt> is now maintaining
PerlIO::via::QuotedPrint as of version 0.08.

=head1 COPYRIGHT

Copyright (C) 2002-2004, 2012 Elizabeth Mattijsen.  All rights reserved.

Copyright (C) 2015, 2020 Steve Hay.  All rights reserved.

=head1 LICENCE

This module is free software; you can redistribute it and/or modify it under
the same terms as Perl itself, i.e. under the terms of either the GNU General
Public License or the Artistic License, as specified in the F<LICENCE> file.

=head1 VERSION

Version 0.10

=head1 DATE

22 May 2022

=head1 HISTORY

See the F<Changes> file.

=cut
