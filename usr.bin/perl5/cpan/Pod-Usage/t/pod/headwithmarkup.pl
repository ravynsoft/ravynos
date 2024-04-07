#!/usr/bin/env perl
use strict;
use warnings;
use Pod::Usage;
pod2usage(
    -exitstatus => 0,
    -verbose => 99,
    -sections => 'ACTIONS/back.*',
    -noperldoc => 1
);

__END__

=head1 ACTIONS

Para for actions.

=head2 backup I<pkg> B<please> dest

Para for backup.

=cut
