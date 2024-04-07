#!/usr/bin/perl -w

my ($Has_PH, $Field);
BEGIN { 
    $Has_PH = $] < 5.009;
    $Field = $Has_PH ? "pseudo-hash field" : "class field";
}

my $W;

BEGIN {
    $W = 0;
    $SIG{__WARN__} = sub {
        if ($_[0] =~ /^Hides field '.*?' in base class/) {
            $W++;
        }
        else {
            warn $_[0];
        }
    };
}

use strict;
use Test::More tests => 29;

BEGIN { use_ok('base'); }

package B1;
use fields qw(b1 b2 b3);

package B2;
use fields '_b1';
use fields qw(b1 _b2 b2);

sub new { fields::new(shift) }

package B3;
use fields qw(b4 _b5 b6 _b7);

package D1;
use base 'B1';
use fields qw(d1 d2 d3);

package D2;
use base 'B1';
use fields qw(_d1 _d2);
use fields qw(d1 d2);


package D3;
use base 'B2';
use fields qw(b1 d1 _b1 _d1);  # hide b1

package D4;
use base 'D3';
use fields qw(_d3 d3);

package M;
sub m {}

package D5;
use base qw(M B2);

# Test that multiple inheritance fails.
package D6;
eval { 'base'->import(qw(B2 M B3)); };
::like($@, qr/can't multiply inherit fields/i, 
    'No multiple field inheritance');

package Foo::Bar;
use base 'B1';

package Foo::Bar::Baz;
use base 'Foo::Bar';
use fields qw(foo bar baz);

# Test repeatability for when modules get reloaded.
package B1;
use fields qw(b1 b2 b3);

package D3;
use base 'B2';
use fields qw(b1 d1 _b1 _d1);  # hide b1


# Test that a package with only private fields gets inherited properly
package B7;
use fields qw(_b1);

package D7;
use base qw(B7);
use fields qw(b1);


# Test that an intermediate package with no fields doesn't cause a problem.
package B8;
use fields qw(_b1);

package D8;
use base qw(B8);

package D8A;
use base qw(D8);
use fields qw(b1);


package main;

my %EXPECT = (
              B1 => [qw(b1 b2 b3)],
              D1 => [qw(b1 b2 b3 d1 d2 d3)],
              D2 => [qw(b1 b2 b3 _d1 _d2 d1 d2)],

              M  => [qw()],
              B2 => [qw(_b1 b1 _b2 b2)],
              D3 => [(undef,undef,undef,
                                qw(b2 b1 d1 _b1 _d1))],     # b1 is hidden
              D4 => [(undef,undef,undef,
                                qw(b2 b1 d1),undef,undef,qw(_d3 d3))],

              D5 => [undef, 'b1', undef, 'b2'],

              B3 => [qw(b4 _b5 b6 _b7)],

              B7 => [qw(_b1)],
              D7 => [undef, 'b1'],

              B8  => [qw(_b1)],
              D8  => [undef],
              D8A => [undef, 'b1'],

              'Foo::Bar'        => [qw(b1 b2 b3)],
              'Foo::Bar::Baz'   => [qw(b1 b2 b3 foo bar baz)],
             );

while(my($class, $efields) = each %EXPECT) {
    no strict 'refs';
    my %fields = %{$class.'::FIELDS'};
    my %expected_fields;
    foreach my $idx (1..@$efields) {
        my $key = $efields->[$idx-1];
        next unless $key;
        $expected_fields{$key} = $idx;
    }

    ::is_deeply(\%fields, \%expected_fields, "%FIELDS check:  $class");
}

# Did we get the appropriate amount of warnings?
is( $W, 1, 'right warnings' );


# A simple object creation and attribute access test
my B2 $obj1 = D3->new;
$obj1->{b1} = "B2";
my D3 $obj2 = $obj1;
$obj2->{b1} = "D3";

# We should get compile time failures field name typos
eval q(return; my D3 $obj3 = $obj2; $obj3->{notthere} = "");
like $@, 
    qr/^No such $Field "notthere" in variable \$obj3 of type D3/,
    "Compile failure of undeclared fields (helem)";

SKIP: {
    # Slices
    # We should get compile time failures field name typos
    skip "Doesn't work before 5.9", 2 if $] < 5.009;
    eval q(return; my D3 $obj3 = $obj2; my $k; @$obj3{$k,'notthere'} = ());
    like $@, 
	qr/^No such $Field "notthere" in variable \$obj3 of type D3/,
	"Compile failure of undeclared fields (hslice)";
    eval q(return; my D3 $obj3 = $obj2; my $k; @{$obj3}{$k,'notthere'} = ());
    like 
	$@, qr/^No such $Field "notthere" in variable \$obj3 of type D3/,
	"Compile failure of undeclared fields (hslice (block form))";
}

@$obj1{"_b1", "b1"} = (17, 29);
is( $obj1->{_b1}, 17 );
is( $obj1->{b1},  29 );

@$obj1{'_b1', 'b1'} = (44,28);
is( $obj1->{_b1}, 44 );
is( $obj1->{b1},  28 );



# Break multiple inheritance with a field name clash.
package E1;
use fields qw(yo this _lah meep 42);

package E2;
use fields qw(_yo ahhh this);

eval {
    package Broken;

    # The error must occur at run time for the eval to catch it.
    require base;
    'base'->import(qw(E1 E2));
};
::like( $@, qr/Can't multiply inherit fields/i, 'Again, no multi inherit' );


# Test that a package with no fields can inherit from a package with
# fields, and that pseudohash messages don't show up

package B9;
use fields qw(b1);

sub _mk_obj { fields::new($_[0])->{'b1'} };

package D9;
use base qw(B9);

package main;

{
    my $w = 0;
    local $SIG{__WARN__} = sub { $w++ };
    
    B9->_mk_obj();
    # used tp emit a warning that pseudohashes are deprecated, because
    # %FIELDS wasn't blessed.
    D9->_mk_obj();
    
    is ($w, 0, "pseudohash warnings in derived class with no fields of it's own");	
}

# [perl #31078] an intermediate class with no additional fields caused
# hidden fields in base class to get stomped on

{
    package X;
    use fields qw(X1 _X2);
    sub new {
	my X $self = shift;
	$self = fields::new($self) unless ref $self;
	$self->{X1} = "x1";
	# FIXME. This code is dead on blead because the test is skipped.
	# The test states that it's being skipped because restricted hashes
	# don't support a feature. Presumably we need to make that feature
	# supported. Bah.
	# use Devel::Peek; Dump($self);
	$self->{_X2} = "_x2";
	return $self;
    }
    sub get_X2 { my X $self = shift; $self->{_X2} }

    package Y;
    use base qw(X);

    sub new {
	my Y $self = shift;
	$self = fields::new($self) unless ref $self;
	$self->SUPER::new();
	return $self;
    }


    package Z;
    use base qw(Y);
    use fields qw(Z1);

    sub new {
	my Z $self = shift;
	$self = fields::new($self) unless ref $self;
	$self->SUPER::new();
	$self->{Z1} = 'z1';
	return $self;
    }

    package main;

    if ($Has_PH) {
	my Z $c = Z->new();
	is($c->get_X2, '_x2', "empty intermediate class");
    }
    else {
	SKIP: {
	    skip "restricted hashes don't support private fields properly", 1;
	}
    }
}
