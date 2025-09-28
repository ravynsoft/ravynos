#!./perl -w
#
# test a few problems with the Freezer option, not a complete Freezer
# test suite yet

use strict;
use warnings;

use Test::More tests =>  8;
use Data::Dumper;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

{
    local $Data::Dumper::Freezer = 'freeze';

    # test for seg-fault bug when freeze() returns a non-ref
    {
        my $foo = Test1->new("foo");
        my $dumped_foo = Dumper($foo);
        ok($dumped_foo,
           "Use of freezer sub which returns non-ref worked.");
        like($dumped_foo, qr/frozed/,
             "Dumped string has the key added by Freezer with useperl.");
        like(join(" ", Dumper($foo)), qr/\A\$VAR1 = /,
             "Dumped list doesn't begin with Freezer's return value with useperl");
    }


    # test for warning when an object does not have a freeze()
    {
        my $warned = 0;
        local $SIG{__WARN__} = sub { $warned++ };
        my $bar = Test2->new("bar");
        my $dumped_bar = Dumper($bar);
        is($warned, 0, "A missing freeze() shouldn't warn.");
    }


    # a freeze() which die()s should still trigger the warning
    {
        my $warned = 0;
        local $SIG{__WARN__} = sub { $warned++; };
        my $bar = Test3->new("bar");
        my $dumped_bar = Dumper($bar);
        is($warned, 1, "A freeze() which die()s should warn.");
    }

}

{
    my ($obj, %dumps);
    my $foo = Test1->new("foo");

    local $Data::Dumper::Freezer = 'freeze';
    $obj = Data::Dumper->new( [ $foo ] );
    $dumps{'ddftrue'} = _dumptostr($obj);
    local $Data::Dumper::Freezer = '';

    $obj = Data::Dumper->new( [ $foo ] );
    $obj->Freezer('freeze');
    $dumps{'objset'} = _dumptostr($obj);

    is($dumps{'ddftrue'}, $dumps{'objset'},
        "\$Data::Dumper::Freezer and Freezer() are equivalent");
}

{
    my ($obj, %dumps);
    my $foo = Test1->new("foo");

    local $Data::Dumper::Freezer = '';
    $obj = Data::Dumper->new( [ $foo ] );
    $dumps{'ddfemptystr'} = _dumptostr($obj);

    local $Data::Dumper::Freezer = undef;
    $obj = Data::Dumper->new( [ $foo ] );
    $dumps{'ddfundef'} = _dumptostr($obj);

    is($dumps{'ddfundef'}, $dumps{'ddfemptystr'},
        "\$Data::Dumper::Freezer same with empty string or undef");
}

{
    my ($obj, %dumps);
    my $foo = Test1->new("foo");

    $obj = Data::Dumper->new( [ $foo ] );
    $obj->Freezer('');
    $dumps{'objemptystr'} = _dumptostr($obj);

    $obj = Data::Dumper->new( [ $foo ] );
    $obj->Freezer(undef);
    $dumps{'objundef'} = _dumptostr($obj);

    is($dumps{'objundef'}, $dumps{'objemptystr'},
        "Freezer() same with empty string or undef");
}


# a package with a freeze() which returns a non-ref
package Test1;
sub new { bless({name => $_[1]}, $_[0]) }
sub freeze {
    my $self = shift;
    $self->{frozed} = 1;
}

# a package without a freeze()
package Test2;
sub new { bless({name => $_[1]}, $_[0]) }

# a package with a freeze() which dies
package Test3;
sub new { bless({name => $_[1]}, $_[0]) }
sub freeze { die "freeze() is broken" }
