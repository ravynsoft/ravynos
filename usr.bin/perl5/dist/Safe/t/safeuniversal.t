#!perl

BEGIN {
    require Config;
    import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/) {
	print "1..0\n";
	exit 0;
    }
}

use strict;
use warnings;
use Test::More;
use Safe;
plan(tests => 6);

my $c = new Safe;
$c->permit(qw(require caller));

my $no_warn_redef = ($] != 5.008009)
    ? q(no warnings 'redefine';)
    : q($SIG{__WARN__}=sub{};);
my $r = $c->reval($no_warn_redef . q!
    sub UNIVERSAL::isa { "pwned" }
    (bless[],"Foo")->isa("Foo");
!);

is( $r, "pwned", "isa overridden in compartment" );
is( (bless[],"Foo")->isa("Foo"), 1, "... but not outside" );

sub Foo::foo {}

$r = $c->reval($no_warn_redef . q!
    sub UNIVERSAL::can { "pwned" }
    (bless[],"Foo")->can("foo");
!);

is( $r, "pwned", "can overridden in compartment" );
is( (bless[],"Foo")->can("foo"), \&Foo::foo, "... but not outside" );

$r = $c->reval(q!
    utf8::is_utf8("\x{100}");
!);
is( $@, '', 'can call utf8::is_valid' );
is( $r, 1, '... returns 1' );
