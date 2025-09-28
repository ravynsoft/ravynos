#!./perl -w
# t/toaster.t - Test Toaster()

use strict;
use warnings;

use Data::Dumper;
use Test::More tests =>  8;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

my %d = (
    delta   => 'd',
    beta    => 'b',
    gamma   => 'c',
    alpha   => 'a',
);

run_tests_for_toaster();
SKIP: {
    skip "XS version was unavailable, so we already ran with pure Perl", 4
        if $Data::Dumper::Useperl;
    local $Data::Dumper::Useperl = 1;
    run_tests_for_toaster();
}

sub run_tests_for_toaster {
    note("\$Data::Dumper::Useperl = $Data::Dumper::Useperl");

    my ($obj, %dumps, $toaster, $starting);

    note("\$Data::Dumper::Toaster and Toaster() set to true value");

    $starting = $Data::Dumper::Toaster;
    $toaster = 1;
    local $Data::Dumper::Toaster = $toaster;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddtoasterone'} = _dumptostr($obj);
    local $Data::Dumper::Toaster = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Toaster($toaster);
    $dumps{'objtoasterone'} = _dumptostr($obj);

    is($dumps{'ddtoasterone'}, $dumps{'objtoasterone'},
        "\$Data::Dumper::Toaster = 1 and Toaster(1) are equivalent");
    %dumps = ();

    $toaster = 0;
    local $Data::Dumper::Toaster = $toaster;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddtoasterzero'} = _dumptostr($obj);
    local $Data::Dumper::Toaster = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Toaster($toaster);
    $dumps{'objtoasterzero'} = _dumptostr($obj);

    is($dumps{'ddtoasterzero'}, $dumps{'objtoasterzero'},
        "\$Data::Dumper::Toaster = 0 and Toaster(0) are equivalent");

    $toaster = undef;
    local $Data::Dumper::Toaster = $toaster;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddtoasterundef'} = _dumptostr($obj);
    local $Data::Dumper::Toaster = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Toaster($toaster);
    $dumps{'objtoasterundef'} = _dumptostr($obj);

    is($dumps{'ddtoasterundef'}, $dumps{'objtoasterundef'},
        "\$Data::Dumper::Toaster = undef and Toaster(undef) are equivalent");
    is($dumps{'ddtoasterzero'}, $dumps{'objtoasterundef'},
        "\$Data::Dumper::Toaster = undef and = 0 are equivalent");
    %dumps = ();
}

