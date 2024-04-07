#!./perl
# Tests for caller()

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );

plan( tests => 18 );

package ｍａｉｎ;

{
    local $@;
    eval 'ok(1);';
    ::like $@, qr/Undefined subroutine &ｍａｉｎ::ok called at/u;
}
my @c;

sub { @c = caller(0) } -> ();
::is( $c[3], "ｍａｉｎ::__ANON__", "anonymous subroutine name" );
::ok( $c[4], "hasargs true with anon sub" );

# Bug 20020517.003 (#9367), used to dump core
sub ｆｏｏ { @c = caller(0) }
# The subroutine only gets anonymised if it is relying on a real GV
# for its name.
() = *{"ｆｏｏ"}; # with quotes so that the op tree doesn’t reference the GV
my $fooref = delete $ｍａｉｎ::{ｆｏｏ};
$fooref -> ();
::is( $c[3], "ｍａｉｎ::__ANON__", "deleted subroutine name" );
::ok( $c[4], "hasargs true with deleted sub" );

print "# Tests with caller(1)\n";

sub ｆ { @c = caller(1) }

sub ｃａｌｌｆ { ｆ(); }
ｃａｌｌｆ();
::is( $c[3], "ｍａｉｎ::ｃａｌｌｆ", "subroutine name" );
::ok( $c[4], "hasargs true with ｃａｌｌｆ()" );
&ｃａｌｌｆ;
::ok( !$c[4], "hasargs false with &ｃａｌｌｆ" );

eval { ｆ() };
::is( $c[3], "(eval)", "subroutine name in an eval {}" );
::ok( !$c[4], "hasargs false in an eval {}" );

eval q{ ｆ() };
::is( $c[3], "(eval)", "subroutine name in an eval ''" );
::ok( !$c[4], "hasargs false in an eval ''" );

sub { ｆ() } -> ();
::is( $c[3], "ｍａｉｎ::__ANON__", "anonymous subroutine name" );
::ok( $c[4], "hasargs true with anon sub" );

sub ｆｏｏ2 { ｆ() }
() = *{"ｆｏｏ2"}; # see ｆｏｏ notes above
my $fooref2 = delete $ｍａｉｎ::{ｆｏｏ2};
$fooref2 -> ();
::is( $c[3], "ｍａｉｎ::__ANON__", "deleted subroutine name" );
::ok( $c[4], "hasargs true with deleted sub" );

sub ｐｂ { return (caller(0))[3] }

::is( eval 'ｐｂ()', 'ｍａｉｎ::ｐｂ', "actually return the right function name" );

my $saved_perldb = $^P;
$^P = 16;
$^P = $saved_perldb;

::is( eval 'ｐｂ()', 'ｍａｉｎ::ｐｂ', 'actually return the right function name even if $^P had been on at some point' );
