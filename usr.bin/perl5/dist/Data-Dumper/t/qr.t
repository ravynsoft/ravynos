#!perl -X

use strict;
use warnings;

use Test::More tests => 2;
use Data::Dumper;

{
    my $q = q| \/ |;
    use Data::Dumper;
    my $qr = qr{$q};
    {
        no strict 'vars';
        eval Dumper $qr;
    }
    ok(!$@, "Dumping $qr with XS") or diag $@, Dumper $qr;
    local $Data::Dumper::Useperl = 1;
    {
        no strict 'vars';
        eval Dumper $qr;
    }
    ok(!$@, "Dumping $qr with PP") or diag $@, Dumper $qr;
}
