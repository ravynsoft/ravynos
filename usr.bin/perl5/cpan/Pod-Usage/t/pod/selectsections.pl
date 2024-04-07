#!/usr/bin/env perl
use strict;
use warnings;

use Pod::Usage;

my @tests = (
  [ "NAME" , "ACTIONS", "ACTIONS/help" ],
  'DESCRIPTION|OPTIONS|ENVIRONMENT/Caveats',
);

my $idx = shift(@ARGV) || 0;

pod2usage(
  -exitstatus => 0,
  -verbose => 99,
  -sections => $tests[$idx],
  -noperldoc => 1
);
1;

__END__

=head1 NAME

trypodi - pod sections usage test

=head1 ACTIONS

Para for actions.

=head2 help

Help text.

=head1 DESCRIPTION

Description text.

=head2 Caveats

Description caveat text.

=head2 Other

Description other text.

=head1 OPTIONS

Options text.

=head2 Caveats

Options caveat text.

=head2 Other

Options other text.

=head1 ENVIRONMENT

Environment text.

=head2 Caveats

Environment caveat text.

=head2 Other

Environment other text.

=cut

