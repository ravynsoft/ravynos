#!/usr/bin/perl -w

my $Has_PH;
BEGIN {
    $Has_PH = $] < 5.009;
}

use strict;
use Test::More tests => 18;

BEGIN { use_ok('fields'); }


package Foo;

use fields qw(_no Pants who _up_yours);
use fields qw(what);

sub new { fields::new(shift) }
sub magic_new { bless [] }  # Doesn't 100% work, perl's problem.

package main;

is_deeply( [sort keys %Foo::FIELDS], 
           [sort qw(_no Pants who _up_yours what)]
);

sub show_fields {
    my($base, $mask) = @_;
    no strict 'refs';
    my $fields = \%{$base.'::FIELDS'};
    return grep { ($fields::attr{$base}[$fields->{$_}] & $mask) == $mask} 
                keys %$fields;
}

is_deeply( [sort &show_fields('Foo', fields::PUBLIC)],
           [sort qw(Pants who what)]);
is_deeply( [sort &show_fields('Foo', fields::PRIVATE)],
           [sort qw(_no _up_yours)]);

# We should get compile time failures field name typos
eval q(my Foo $obj = Foo->new; $obj->{notthere} = "");

like $@, qr/^No such .*field "notthere"/i;


foreach (Foo->new) {
    my Foo $obj = $_;
    my %test = ( Pants => 'Whatever', _no => 'Yeah',
                 what  => 'Ahh',      who => 'Moo',
                 _up_yours => 'Yip' );

    $obj->{Pants} = 'Whatever';
    $obj->{_no}   = 'Yeah';
    @{$obj}{qw(what who _up_yours)} = ('Ahh', 'Moo', 'Yip');

    while(my($k,$v) = each %test) {
        is($obj->{$k}, $v);
    }
}

{
    local $SIG{__WARN__} = sub {
        return if $_[0] =~ /^Pseudo-hashes are deprecated/ 
    };
    my $phash;
    eval { $phash = fields::phash(name => "Joe", rank => "Captain") };
    if( $Has_PH ) {
        is( $phash->{rank}, "Captain" );
    }
    else {
        like $@, qr/^Pseudo-hashes have been removed from Perl/;
    }
}


# check if fields autovivify
{
    package Foo::Autoviv;
    use fields qw(foo bar);
    sub new { fields::new($_[0]) }

    package main;
    my Foo::Autoviv $a = Foo::Autoviv->new();
    $a->{foo} = ['a', 'ok', 'c'];
    $a->{bar} = { A => 'ok' };
    is( $a->{foo}[1],    'ok' );
    is( $a->{bar}->{A},, 'ok' );
}

package Test::FooBar;

use fields qw(a b c);

sub new {
    my $self = fields::new(shift);
    %$self = @_ if @_;
    $self;
}

package main;

{
    my $x = Test::FooBar->new( a => 1, b => 2);

    is(ref $x, 'Test::FooBar', 'x is a Test::FooBar');
    ok(exists $x->{a}, 'x has a');
    ok(exists $x->{b}, 'x has b');

    SKIP: {
        skip "These tests trigger a perl bug", 2 if $] < 5.015;
        $x->{a} = __PACKAGE__;
        ok eval { delete $x->{a}; 1 }, 'deleting COW values' or diag $@;
        $x->{a} = __PACKAGE__;
        ok eval { %$x = (); 1 }, 'clearing a restr hash containing COWs' or diag $@;
    }
}
