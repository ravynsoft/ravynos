#!./perl -w
# t/deparse.t - Test Deparse()

use strict;
use warnings;

use Data::Dumper;
use Test::More tests =>  16;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

# Thanks to Arthur Axel "fREW" Schmidt:
# http://search.cpan.org/~frew/Data-Dumper-Concise-2.020/lib/Data/Dumper/Concise.pm

note("\$Data::Dumper::Deparse and Deparse()");

for my $useperl (0, 1) {
    local $Data::Dumper::Useperl = $useperl;

    my ($obj, %dumps, $deparse, $starting);
    use strict;
    my $struct = { foo => "bar\nbaz", quux => sub { "fleem" } };
    $obj = Data::Dumper->new( [ $struct ] );
    $dumps{'noprev'} = _dumptostr($obj);

    $starting = $Data::Dumper::Deparse;
    local $Data::Dumper::Deparse = 0;
    $obj = Data::Dumper->new( [ $struct ] );
    $dumps{'dddzero'} = _dumptostr($obj);
    local $Data::Dumper::Deparse = $starting;

    $obj = Data::Dumper->new( [ $struct ] );
    $obj->Deparse();
    $dumps{'objempty'} = _dumptostr($obj);

    $obj = Data::Dumper->new( [ $struct ] );
    $obj->Deparse(0);
    $dumps{'objzero'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'dddzero'},
        "No previous setting and \$Data::Dumper::Deparse = 0 are equivalent (useperl=$useperl)");
    is($dumps{'noprev'}, $dumps{'objempty'},
        "No previous setting and Deparse() are equivalent (useperl=$useperl)");
    is($dumps{'noprev'}, $dumps{'objzero'},
        "No previous setting and Deparse(0) are equivalent (useperl=$useperl)");

    local $Data::Dumper::Deparse = 1;
    $obj = Data::Dumper->new( [ $struct ] );
    $dumps{'dddtrue'} = _dumptostr($obj);
    local $Data::Dumper::Deparse = $starting;

    $obj = Data::Dumper->new( [ $struct ] );
    $obj->Deparse(1);
    $dumps{'objone'} = _dumptostr($obj);

    is($dumps{'dddtrue'}, $dumps{'objone'},
        "\$Data::Dumper::Deparse = 1 and Deparse(1) are equivalent (useperl=$useperl)");

    isnt($dumps{'dddzero'}, $dumps{'dddtrue'},
        "\$Data::Dumper::Deparse = 0 differs from \$Data::Dumper::Deparse = 1 (useperl=$useperl)");

    like($dumps{'dddzero'},
        qr/quux.*?sub.*?DUMMY/s,
        "\$Data::Dumper::Deparse = 0 reports DUMMY instead of deparsing coderef (useperl=$useperl)");
    unlike($dumps{'dddtrue'},
        qr/quux.*?sub.*?DUMMY/s,
        "\$Data::Dumper::Deparse = 1 does not report DUMMY (useperl=$useperl)");
    like($dumps{'dddtrue'},
        qr/quux.*?sub.*?use\sstrict.*?fleem/s,
        "\$Data::Dumper::Deparse = 1 deparses coderef (useperl=$useperl)");
}

