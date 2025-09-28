#!./perl -w
# t/sparseseen.t - Test Sparseseen()

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

run_tests_for_sparseseen();
SKIP: {
    skip "XS version was unavailable, so we already ran with pure Perl", 4
        if $Data::Dumper::Useperl;
    local $Data::Dumper::Useperl = 1;
    run_tests_for_sparseseen();
}

sub run_tests_for_sparseseen {
    note("\$Data::Dumper::Useperl = $Data::Dumper::Useperl");

    my ($obj, %dumps, $sparseseen, $starting);

    note("\$Data::Dumper::Sparseseen and Sparseseen() set to true value");

    $starting = $Data::Dumper::Sparseseen;
    $sparseseen = 1;
    local $Data::Dumper::Sparseseen = $sparseseen;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddssone'} = _dumptostr($obj);
    local $Data::Dumper::Sparseseen = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Sparseseen($sparseseen);
    $dumps{'objssone'} = _dumptostr($obj);

    is($dumps{'ddssone'}, $dumps{'objssone'},
        "\$Data::Dumper::Sparseseen = 1 and Sparseseen(1) are equivalent");
    %dumps = ();

    $sparseseen = 0;
    local $Data::Dumper::Sparseseen = $sparseseen;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddsszero'} = _dumptostr($obj);
    local $Data::Dumper::Sparseseen = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Sparseseen($sparseseen);
    $dumps{'objsszero'} = _dumptostr($obj);

    is($dumps{'ddsszero'}, $dumps{'objsszero'},
        "\$Data::Dumper::Sparseseen = 0 and Sparseseen(0) are equivalent");

    $sparseseen = undef;
    local $Data::Dumper::Sparseseen = $sparseseen;
    $obj = Data::Dumper->new( [ \%d ] );
    $dumps{'ddssundef'} = _dumptostr($obj);
    local $Data::Dumper::Sparseseen = $starting;

    $obj = Data::Dumper->new( [ \%d ] );
    $obj->Sparseseen($sparseseen);
    $dumps{'objssundef'} = _dumptostr($obj);

    is($dumps{'ddssundef'}, $dumps{'objssundef'},
        "\$Data::Dumper::Sparseseen = undef and Sparseseen(undef) are equivalent");
    is($dumps{'ddsszero'}, $dumps{'objssundef'},
        "\$Data::Dumper::Sparseseen = undef and = 0 are equivalent");
    %dumps = ();
}

