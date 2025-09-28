#!perl -w
# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;

require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();
local $ENV{HARNESS_ACTIVE} = 0;


# Can't use Test.pm, that's a 5.005 thing.
package My::Test;

# This has to be a require or else the END block below runs before
# Test::Builder's own and the ending diagnostics don't come out right.
require Test::Builder;
my $TB = Test::Builder->create;
$TB->plan(tests => 81);

sub like ($$;$) {
    $TB->like(@_);
}

sub is ($$;$) {
    $TB->is_eq(@_);
}

sub main::out_ok ($$) {
    $TB->is_eq( $out->read, shift );
    $TB->is_eq( $err->read, shift );
}

sub main::out_warn_ok ($$$) {
    $TB->is_eq( $out->read, shift );
    $TB->is_eq( $err->read, shift );
    my $warning_expected = shift;
    $warning_expected =~ s/^# //mg;
    $TB->is_eq( $main::warning, $warning_expected );
}

sub main::out_like ($$) {
    my($output, $failure) = @_;

    $TB->like( $out->read, qr/$output/ );
    $TB->like( $err->read, qr/$failure/ );
}


package main;

require Test::More;
our $TODO;
my $Total = 38;
Test::More->import(tests => $Total);
$out->read;  # clear the plan from $out

# This should all work in the presence of a __DIE__ handler.
local $SIG{__DIE__} = sub { $TB->ok(0, "DIE handler called: ".join "", @_); };
local $SIG{__WARN__} = sub { $main::warning = $_[0]; };

my $tb = Test::More->builder;
$tb->use_numbers(0);

my $Filename = quotemeta $0;


#line 38
ok( 0, 'failing' );
out_ok( <<OUT, <<ERR );
not ok - failing
OUT
#   Failed test 'failing'
#   at $0 line 38.
ERR


#line 40
is( "foo", "bar", 'foo is bar?');
out_ok( <<OUT, <<ERR );
not ok - foo is bar?
OUT
#   Failed test 'foo is bar?'
#   at $0 line 40.
#          got: 'foo'
#     expected: 'bar'
ERR

#line 89
is( undef, '',    'undef is empty string?');
out_ok( <<OUT, <<ERR );
not ok - undef is empty string?
OUT
#   Failed test 'undef is empty string?'
#   at $0 line 89.
#          got: undef
#     expected: ''
ERR

#line 99
is( undef, 0,     'undef is 0?');
out_ok( <<OUT, <<ERR );
not ok - undef is 0?
OUT
#   Failed test 'undef is 0?'
#   at $0 line 99.
#          got: undef
#     expected: '0'
ERR

#line 110
is( '',    0,     'empty string is 0?' );
out_ok( <<OUT, <<ERR );
not ok - empty string is 0?
OUT
#   Failed test 'empty string is 0?'
#   at $0 line 110.
#          got: ''
#     expected: '0'
ERR

#line 121
isnt("foo", "foo", 'foo isnt foo?' );
out_ok( <<OUT, <<ERR );
not ok - foo isnt foo?
OUT
#   Failed test 'foo isnt foo?'
#   at $0 line 121.
#          got: 'foo'
#     expected: anything else
ERR

#line 132
isn::t("foo", "foo",'foo isn\'t foo?' );
out_warn_ok( <<OUT, <<ERR, <<WARN );
not ok - foo isn't foo?
OUT
#   Failed test 'foo isn\'t foo?'
#   at $0 line 132.
#          got: 'foo'
#     expected: anything else
ERR
# Use of apostrophe as package separator was deprecated in Perl 5.37.9,
# and will be removed in Perl 5.42.0.  You should change code that uses
# Test::More::isn't() to use Test::More::isnt() as a replacement at t/Legacy/fail-more.t line 132
WARN

#line 143
isnt(undef, undef, 'undef isnt undef?');
out_ok( <<OUT, <<ERR );
not ok - undef isnt undef?
OUT
#   Failed test 'undef isnt undef?'
#   at $0 line 143.
#          got: undef
#     expected: anything else
ERR

#line 154
like( "foo", '/that/',  'is foo like that' );
out_ok( <<OUT, <<ERR );
not ok - is foo like that
OUT
#   Failed test 'is foo like that'
#   at $0 line 154.
#                   'foo'
#     doesn't match '/that/'
ERR

#line 165
unlike( "foo", '/foo/', 'is foo unlike foo' );
out_ok( <<OUT, <<ERR );
not ok - is foo unlike foo
OUT
#   Failed test 'is foo unlike foo'
#   at $0 line 165.
#                   'foo'
#           matches '/foo/'
ERR

# Nick Clark found this was a bug.  Fixed in 0.40.
# line 177
like( "bug", '/(%)/',   'regex with % in it' );
out_ok( <<OUT, <<ERR );
not ok - regex with % in it
OUT
#   Failed test 'regex with % in it'
#   at $0 line 177.
#                   'bug'
#     doesn't match '/(%)/'
ERR

#line 188
fail('fail()');
out_ok( <<OUT, <<ERR );
not ok - fail()
OUT
#   Failed test 'fail()'
#   at $0 line 188.
ERR

#line 197
can_ok('Mooble::Hooble::Yooble', qw(this that));
out_ok( <<OUT, <<ERR );
not ok - Mooble::Hooble::Yooble->can(...)
OUT
#   Failed test 'Mooble::Hooble::Yooble->can(...)'
#   at $0 line 197.
#     Mooble::Hooble::Yooble->can('this') failed
#     Mooble::Hooble::Yooble->can('that') failed
ERR

#line 208
can_ok('Mooble::Hooble::Yooble', ());
out_ok( <<OUT, <<ERR );
not ok - Mooble::Hooble::Yooble->can(...)
OUT
#   Failed test 'Mooble::Hooble::Yooble->can(...)'
#   at $0 line 208.
#     can_ok() called with no methods
ERR

#line 218
can_ok(undef, undef);
out_ok( <<OUT, <<ERR );
not ok - ->can(...)
OUT
#   Failed test '->can(...)'
#   at $0 line 218.
#     can_ok() called with empty class or reference
ERR

#line 228
can_ok([], "foo");
out_ok( <<OUT, <<ERR );
not ok - ARRAY->can('foo')
OUT
#   Failed test 'ARRAY->can('foo')'
#   at $0 line 228.
#     ARRAY->can('foo') failed
ERR

#line 238
isa_ok(bless([], "Foo"), "Wibble");
out_ok( <<OUT, <<ERR );
not ok - An object of class 'Foo' isa 'Wibble'
OUT
#   Failed test 'An object of class 'Foo' isa 'Wibble''
#   at $0 line 238.
#     The object of class 'Foo' isn't a 'Wibble'
ERR

#line 248
isa_ok(42,    "Wibble", "My Wibble");
out_ok( <<OUT, <<ERR );
not ok - 'My Wibble' isa 'Wibble'
OUT
#   Failed test ''My Wibble' isa 'Wibble''
#   at $0 line 248.
#     'My Wibble' isn't a 'Wibble'
ERR

#line 252
isa_ok(42,    "Wibble");
out_ok( <<OUT, <<ERR );
not ok - The class (or class-like) '42' isa 'Wibble'
OUT
#   Failed test 'The class (or class-like) '42' isa 'Wibble''
#   at $0 line 252.
#     The class (or class-like) '42' isn't a 'Wibble'
ERR

#line 258
isa_ok(undef, "Wibble", "Another Wibble");
out_ok( <<OUT, <<ERR );
not ok - 'Another Wibble' isa 'Wibble'
OUT
#   Failed test ''Another Wibble' isa 'Wibble''
#   at $0 line 258.
#     'Another Wibble' isn't defined
ERR

#line 268
isa_ok([],    "HASH");
out_ok( <<OUT, <<ERR );
not ok - A reference of type 'ARRAY' isa 'HASH'
OUT
#   Failed test 'A reference of type 'ARRAY' isa 'HASH''
#   at $0 line 268.
#     The reference of type 'ARRAY' isn't a 'HASH'
ERR

#line 278
new_ok(undef);
out_like( <<OUT, <<ERR );
not ok - undef->new\\(\\) died
OUT
#   Failed test 'undef->new\\(\\) died'
#   at $Filename line 278.
#     Error was:  Can't call method "new" on an undefined value at .*
ERR

#line 288
new_ok( "Does::Not::Exist" );
out_like( <<OUT, <<ERR );
not ok - Does::Not::Exist->new\\(\\) died
OUT
#   Failed test 'Does::Not::Exist->new\\(\\) died'
#   at $Filename line 288.
#     Error was:  Can't locate object method "new" via package "Does::Not::Exist" .*
ERR


{ package Foo; sub new { } }
{ package Bar; sub new { {} } }
{ package Baz; sub new { bless {}, "Wibble" } }

#line 303
new_ok( "Foo" );
out_ok( <<OUT, <<ERR );
not ok - undef isa 'Foo'
OUT
#   Failed test 'undef isa 'Foo''
#   at $0 line 303.
#     undef isn't defined
ERR

# line 313
new_ok( "Bar" );
out_ok( <<OUT, <<ERR );
not ok - A reference of type 'HASH' isa 'Bar'
OUT
#   Failed test 'A reference of type 'HASH' isa 'Bar''
#   at $0 line 313.
#     The reference of type 'HASH' isn't a 'Bar'
ERR

#line 323
new_ok( "Baz" );
out_ok( <<OUT, <<ERR );
not ok - An object of class 'Wibble' isa 'Baz'
OUT
#   Failed test 'An object of class 'Wibble' isa 'Baz''
#   at $0 line 323.
#     The object of class 'Wibble' isn't a 'Baz'
ERR

#line 333
new_ok( "Baz", [], "no args" );
out_ok( <<OUT, <<ERR );
not ok - 'no args' isa 'Baz'
OUT
#   Failed test ''no args' isa 'Baz''
#   at $0 line 333.
#     'no args' isn't a 'Baz'
ERR

#line 343
cmp_ok( 'foo', 'eq', 'bar', 'cmp_ok eq' );
out_ok( <<OUT, <<ERR );
not ok - cmp_ok eq
OUT
#   Failed test 'cmp_ok eq'
#   at $0 line 343.
#          got: 'foo'
#     expected: 'bar'
ERR

#line 354
cmp_ok( 42.1,  '==', 23,  , '       ==' );
out_ok( <<OUT, <<ERR );
not ok -        ==
OUT
#   Failed test '       =='
#   at $0 line 354.
#          got: 42.1
#     expected: 23
ERR

#line 365
cmp_ok( 42,    '!=', 42   , '       !=' );
out_ok( <<OUT, <<ERR );
not ok -        !=
OUT
#   Failed test '       !='
#   at $0 line 365.
#          got: 42
#     expected: anything else
ERR

#line 376
cmp_ok( 1,     '&&', 0    , '       &&' );
out_ok( <<OUT, <<ERR );
not ok -        &&
OUT
#   Failed test '       &&'
#   at $0 line 376.
#     '1'
#         &&
#     '0'
ERR

# line 388
cmp_ok( 42,    'eq', "foo", '       eq with numbers' );
out_ok( <<OUT, <<ERR );
not ok -        eq with numbers
OUT
#   Failed test '       eq with numbers'
#   at $0 line 388.
#          got: '42'
#     expected: 'foo'
ERR

{
    my $warnings = '';
    local $SIG{__WARN__} = sub { $warnings .= join '', @_ };

# line 415
    cmp_ok( 42,    '==', "foo", '       == with strings' );
    out_ok( <<OUT, <<ERR );
not ok -        == with strings
OUT
#   Failed test '       == with strings'
#   at $0 line 415.
#          got: 42
#     expected: foo
ERR
    My::Test::like(
        $warnings,
        qr/^Argument "foo" isn't numeric in .* at \(eval in cmp_ok\) $Filename line 415\.\n$/
    );
    $warnings = '';
}


{
    my $warnings = '';
    local $SIG{__WARN__} = sub { $warnings .= join '', @_ };

#line 437
    cmp_ok( undef, "ne", "", "undef ne empty string" );

    $TB->is_eq( $out->read, <<OUT );
not ok - undef ne empty string
OUT

    $TB->is_eq( $err->read, <<ERR );
#   Failed test 'undef ne empty string'
#   at $0 line 437.
#     undef
#         ne
#     ''
ERR

    My::Test::like(
        $warnings,
        qr/^Use of uninitialized value.* in string ne at \(eval in cmp_ok\) $Filename line 437.\n\z/
    );
}


# generate a $!, it changes its value by context.
-e "wibblehibble";
my $Errno_Number = $!+0;
my $Errno_String = $!.'';
#line 425
cmp_ok( $!,    'eq', '',    '       eq with stringified errno' );
out_ok( <<OUT, <<ERR );
not ok -        eq with stringified errno
OUT
#   Failed test '       eq with stringified errno'
#   at $0 line 425.
#          got: '$Errno_String'
#     expected: ''
ERR

#line 436
cmp_ok( $!,    '==', -1,    '       eq with numerified errno' );
out_ok( <<OUT, <<ERR );
not ok -        eq with numerified errno
OUT
#   Failed test '       eq with numerified errno'
#   at $0 line 436.
#          got: $Errno_Number
#     expected: -1
ERR

#line 447
use_ok('Hooble::mooble::yooble');
my $more_err_re = <<ERR;
#   Failed test 'use Hooble::mooble::yooble;'
#   at $Filename line 447\\.
#     Tried to use 'Hooble::mooble::yooble'.
#     Error:  Can't locate Hooble.* in \\\@INC .*
ERR
out_like(
    qr/^\Qnot ok - use Hooble::mooble::yooble;\E\n\z/,
    qr/^$more_err_re/
);

#line 460
require_ok('ALL::YOUR::BASE::ARE::BELONG::TO::US::wibble');
$more_err_re = <<ERR;
#   Failed test 'require ALL::YOUR::BASE::ARE::BELONG::TO::US::wibble;'
#   at $Filename line 460\\.
#     Tried to require 'ALL::YOUR::BASE::ARE::BELONG::TO::US::wibble'.
#     Error:  Can't locate ALL.* in \\\@INC .*
ERR
out_like(
    qr/^\Qnot ok - require ALL::YOUR::BASE::ARE::BELONG::TO::US::wibble;\E\n\z/,
    qr/^$more_err_re/
);


END {
    out_like( <<OUT, <<ERR );
OUT
# Looks like you failed $Total tests of $Total.
ERR

    exit(0);
}
