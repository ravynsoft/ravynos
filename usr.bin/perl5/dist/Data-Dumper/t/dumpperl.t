#!./perl -w
# t/dumpperl.t - test all branches of, and modes of triggering, Dumpperl()

use strict;
use warnings;

use Carp;
use Data::Dumper;
use Test::More tests => 31;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

$Data::Dumper::Indent=1;

{
    local $Data::Dumper::Useperl=1;
    local $Data::Dumper::Useqq=0;
    local $Data::Dumper::Deparse=0;
    note('$Data::Dumper::Useperl => 1');
    run_tests_for_pure_perl_implementations();
}

{
    local $Data::Dumper::Useperl=0;
    local $Data::Dumper::Useqq=1;
    local $Data::Dumper::Deparse=0;
    note('$Data::Dumper::Useqq => 1');
    run_tests_for_pure_perl_implementations();
}
    
{
    local $Data::Dumper::Useperl=0;
    local $Data::Dumper::Useqq=0;
    local $Data::Dumper::Deparse=1;
    note('$Data::Dumper::Deparse => 1');
    run_tests_for_pure_perl_implementations();
}
    
    

sub run_tests_for_pure_perl_implementations {

    my ($a, $b, $obj);
    my (@names);
    my (@newnames, $objagain, %newnames);
    my $dumpstr;
    $a = 'alpha';
    $b = 'beta';
    my @c = ( qw| eta theta | );
    my %d = ( iota => 'kappa' );

    note('names not provided');
    $obj = Data::Dumper->new([$a, $b]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$VAR1.+alpha.+\$VAR2.+beta/s,
        "Dump: two strings"
    );
    
    $obj = Data::Dumper->new([$a, \@c]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$VAR1.+alpha.+\$VAR2.+\[.+eta.+theta.+\]/s,
        "Dump: one string, one array ref"
    );
    
    $obj = Data::Dumper->new([$a, \%d]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$VAR1.+alpha.+\$VAR2.+\{.+iota.+kappa.+\}/s,
        "Dump: one string, one hash ref"
    );
    
    $obj = Data::Dumper->new([$a, undef]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$VAR1.+alpha.+\$VAR2.+undef/s,
        "Dump: one string, one undef"
    );
    
    note('names provided');
    
    $obj = Data::Dumper->new([$a, $b], [ qw( a b ) ]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$a.+alpha.+\$b.+beta/s,
        "Dump: names: two strings"
    );
    
    $obj = Data::Dumper->new([$a, \@c], [ qw( a *c ) ]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$a.+alpha.+\@c.+eta.+theta/s,
        "Dump: names: one string, one array ref"
    );
    
    $obj = Data::Dumper->new([$a, \%d], [ qw( a *d ) ]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$a.+alpha.+\%d.+iota.+kappa/s,
        "Dump: names: one string, one hash ref"
    );
    
    $obj = Data::Dumper->new([$a,undef], [qw(a *c)]);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$a.+alpha.+\$c.+undef/s,
        "Dump: names: one string, one undef"
    );
    
    $obj = Data::Dumper->new([$a, $b], [ 'a', '']);
    $dumpstr = _dumptostr($obj);
    like($dumpstr,
        qr/\$a.+alpha.+\$.+beta/s,
        "Dump: names: two strings: one name empty"
    );
    
    $obj = Data::Dumper->new([$a, $b], [ 'a', '$foo']);
    $dumpstr = _dumptostr($obj);
    no warnings 'uninitialized';
    like($dumpstr,
        qr/\$a.+alpha.+\$foo.+beta/s,
        "Dump: names: two strings: one name start with '\$'"
    );
    use warnings;
}

{
    my ($obj, $dumpstr, $realtype);
    $obj = Data::Dumper->new([ {IO => *{$::{STDERR}}{IO}} ]);
    $obj->Useperl(1);
    eval { $dumpstr = _dumptostr($obj); };
    $realtype = 'IO';
    like($@, qr/Can't handle '$realtype' type/,
        "Got expected error: pure-perl: Data-Dumper does not handle $realtype");
}
