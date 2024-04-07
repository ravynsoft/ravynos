#!./perl -w
# t/purity_deepcopy_maxdepth.t - Test Purity(), Deepcopy(),
# Maxdepth() and recursive structures

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 22;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

my ($a, $b, $c, @d);
my ($d, $e, $f);

note("\$Data::Dumper::Purity and Purity()");

{
    my ($obj, %dumps, $purity);

    # Adapted from example in Dumper.pm POD:
    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    note("Discrepancy between Dumpxs() and Dumpperl() behavior with respect to \$Data::Dumper::Purity = undef");
    local $Data::Dumper::Useperl = 1;
    $purity = undef;
    local $Data::Dumper::Purity = $purity;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'ddpundef'} = _dumptostr($obj);

    $purity = 0;
    local $Data::Dumper::Purity = $purity;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'ddpzero'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'ddpundef'},
        "No previous Purity setting equivalent to \$Data::Dumper::Purity = undef");

    is($dumps{'noprev'}, $dumps{'ddpzero'},
        "No previous Purity setting equivalent to \$Data::Dumper::Purity = 0");
}

{
    my ($obj, %dumps, $purity);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $purity = 0;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Purity($purity);
    $dumps{'objzero'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'objzero'},
        "No previous Purity setting equivalent to Purity(0)");
}

{
    my ($obj, %dumps, $purity);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $purity = 1;
    local $Data::Dumper::Purity = $purity;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'ddpone'} = _dumptostr($obj);

    isnt($dumps{'noprev'}, $dumps{'ddpone'},
        "No previous Purity setting different from \$Data::Dumper::Purity = 1");
}

{
    my ($obj, %dumps, $purity);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $purity = 1;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Purity(1);
    $dumps{'objone'} = _dumptostr($obj);

    isnt($dumps{'noprev'}, $dumps{'objone'},
        "No previous Purity setting different from Purity(0)");
}

{
    my ($obj, %dumps, $purity);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $purity = 1;
    local $Data::Dumper::Purity = $purity;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'ddpone'} = _dumptostr($obj);
    local $Data::Dumper::Purity = undef;

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Purity(1);
    $dumps{'objone'} = _dumptostr($obj);

    is($dumps{'ddpone'}, $dumps{'objone'},
        "\$Data::Dumper::Purity = 1 and Purity(1) are equivalent");
}

note("\$Data::Dumper::Deepcopy and Deepcopy()");

{
    my ($obj, %dumps, $deepcopy);

    # Adapted from example in Dumper.pm POD:
    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $deepcopy = undef;
    local $Data::Dumper::Deepcopy = $deepcopy;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'dddundef'} = _dumptostr($obj);

    $deepcopy = 0;
    local $Data::Dumper::Deepcopy = $deepcopy;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'dddzero'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'dddundef'},
        "No previous Deepcopy setting equivalent to \$Data::Dumper::Deepcopy = undef");

    is($dumps{'noprev'}, $dumps{'dddzero'},
        "No previous Deepcopy setting equivalent to \$Data::Dumper::Deepcopy = 0");
}

{
    my ($obj, %dumps, $deepcopy);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $deepcopy = 0;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Deepcopy($deepcopy);
    $dumps{'objzero'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'objzero'},
        "No previous Deepcopy setting equivalent to Deepcopy(0)");

    $deepcopy = undef;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Deepcopy($deepcopy);
    $dumps{'objundef'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'objundef'},
        "No previous Deepcopy setting equivalent to Deepcopy(undef)");
}

{
    my ($obj, %dumps, $deepcopy);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $deepcopy = 1;
    local $Data::Dumper::Deepcopy = $deepcopy;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'dddone'} = _dumptostr($obj);

    isnt($dumps{'noprev'}, $dumps{'dddone'},
        "No previous Deepcopy setting different from \$Data::Dumper::Deepcopy = 1");
}

{
    my ($obj, %dumps, $deepcopy);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $deepcopy = 1;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Deepcopy(1);
    $dumps{'objone'} = _dumptostr($obj);

    isnt($dumps{'noprev'}, $dumps{'objone'},
        "No previous Deepcopy setting different from Deepcopy(0)");
}

{
    my ($obj, %dumps, $deepcopy);

    @d = ('c');
    $c = \@d;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];

    $deepcopy = 1;
    local $Data::Dumper::Deepcopy = $deepcopy;
    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $dumps{'dddone'} = _dumptostr($obj);
    local $Data::Dumper::Deepcopy = undef;

    $obj = Data::Dumper->new([$a,$b,$c], [qw(a b c)]);
    $obj->Deepcopy(1);
    $dumps{'objone'} = _dumptostr($obj);

    is($dumps{'dddone'}, $dumps{'objone'},
        "\$Data::Dumper::Deepcopy = 1 and Deepcopy(1) are equivalent");
}

note("\$Data::Dumper::Maxdepth and Maxdepth()");

{
    # Adapted from Dumper.pm POD

    my ($obj, %dumps, $maxdepth);

    $a = "pearl";
    $b = [ $a ];
    $c = { 'b' => $b };
    $d = [ $c ];
    $e = { 'd' => $d };
    $f = { 'e' => $e };

    note("Discrepancy between Dumpxs() and Dumpperl() behavior with respect to \$Data::Dumper::Maxdepth = undef");
    local $Data::Dumper::Useperl = 1;

    $obj = Data::Dumper->new([$f], [qw(f)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $Data::Dumper::Maxdepth = undef;
    $obj = Data::Dumper->new([$f], [qw(f)]);
    $dumps{'ddmundef'} = _dumptostr($obj);

    $maxdepth = 3;
    local $Data::Dumper::Maxdepth = $maxdepth;
    $obj = Data::Dumper->new([$f], [qw(f)]);
    $dumps{'ddm'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'ddmundef'},
        "No previous Maxdepth setting equivalent to \$Data::Dumper::Maxdepth = undef");

    like($dumps{'noprev'}, qr/$a/s,
        "Without Maxdepth, got output from deepest level");

    isnt($dumps{'noprev'}, $dumps{'ddm'},
        "No previous Maxdepth setting differs from setting a shallow Maxdepth");

    unlike($dumps{'ddm'}, qr/$a/s,
        "With Maxdepth, did not get output from deepest level");
}

{
    # Adapted from Dumper.pm POD

    my ($obj, %dumps, $maxdepth);

    $a = "pearl";
    $b = [ $a ];
    $c = { 'b' => $b };
    $d = [ $c ];
    $e = { 'd' => $d };
    $f = { 'e' => $e };

    note("Discrepancy between Dumpxs() and Dumpperl() behavior with respect to \$Data::Dumper::Maxdepth = undef");
    local $Data::Dumper::Useperl = 1;

    $obj = Data::Dumper->new([$f], [qw(f)]);
    $dumps{'noprev'} = _dumptostr($obj);

    $obj = Data::Dumper->new([$f], [qw(f)]);
    $obj->Maxdepth();
    $dumps{'maxdepthempty'} = _dumptostr($obj);

    is($dumps{'noprev'}, $dumps{'maxdepthempty'},
        "No previous Maxdepth setting equivalent to Maxdepth() with no argument");

    $maxdepth = 3;
    $obj = Data::Dumper->new([$f], [qw(f)]);
    $obj->Maxdepth($maxdepth);
    $dumps{'maxdepthset'} = _dumptostr($obj);

    isnt($dumps{'noprev'}, $dumps{'maxdepthset'},
        "No previous Maxdepth setting differs from Maxdepth() with shallow depth");

    local $Data::Dumper::Maxdepth = 3;
    $obj = Data::Dumper->new([$f], [qw(f)]);
    $dumps{'ddmset'} = _dumptostr($obj);

    is($dumps{'maxdepthset'}, $dumps{'ddmset'},
        "Maxdepth set and \$Data::Dumper::Maxdepth are equivalent");
}

{
    my ($obj, %dumps);

    my $warning = '';
    local $SIG{__WARN__} = sub { $warning = $_[0] };

    local $Data::Dumper::Deparse = 0;
    local $Data::Dumper::Purity  = 1;
    local $Data::Dumper::Useperl = 1;
    sub hello { print "Hello world\n"; }
    $obj = Data::Dumper->new( [ \&hello ] );
    $dumps{'ddsksub'} = _dumptostr($obj);
    like($warning, qr/^Encountered CODE ref, using dummy placeholder/,
        "Got expected warning: dummy placeholder under Purity = 1");
}

{
    my ($obj, %dumps);

    my $warning = '';
    local $SIG{__WARN__} = sub { $warning = $_[0] };

    local $Data::Dumper::Deparse = 0;
    local $Data::Dumper::Useperl = 1;
    sub jello { print "Jello world\n"; }
    $obj = Data::Dumper->new( [ \&hello ] );
    $dumps{'ddsksub'} = _dumptostr($obj);
    ok(! $warning, "Encountered CODE ref, but no Purity, hence no warning");
}
