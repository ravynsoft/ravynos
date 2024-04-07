#!/usr/bin/perl

use strict;
use Test::More tests => 4;

require_ok('Pod::Checker');

my $infile  = $0; # self
(my $outfile = $infile) =~ s/\..*?$/.OUT/;

if ($^O eq 'VMS') {
    for ($infile, $outfile) {
        $_ = VMS::Filespec::unixify($_)  unless  ref;
    }
}

my $checker = Pod::Checker->new();
ok($checker, 'Checker object successfully created');

ok($checker->parse_from_file($infile, $outfile), "$0 successfully parsed");

is($checker->name(), 'podname');

$checker->{'-quiet'} = 1; # we can't write now
$checker->poderror('* HORROR: You better run');

END {
  unlink($outfile);
}

__END__

# this lone =cut triggers the call to scream()

=cut

=head1 NAME

podname - check the name() method of Pod::Checker

=cut

