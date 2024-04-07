#!perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = qw(../lib ../lib/Test/Simple/t/lib);
    }
}

use lib 't/lib';
use Test::More tests => 57;

# Make sure we don't mess with $@ or $!.  Test at bottom.
my $Err   = "this should not be touched";
my $Errno = 42;
$@ = $Err;
$! = $Errno;

use_ok('Dummy');
is( $Dummy::VERSION, '0.01', 'use_ok() loads a module' );
require_ok('Test::More');


ok( 2 eq 2,             'two is two is two is two' );
is(   "foo", "foo",       'foo is foo' );
isnt( "foo", "bar",     'foo isnt bar');
{
    use warnings;
    my $warning;
    local $SIG{__WARN__}= sub { $warning = $_[0] };
    isn::t("foo", "bar",     'foo isn\'t bar');
    is($warning, "Use of apostrophe as package separator was deprecated in Perl 5.37.9,\n"
               . "and will be removed in Perl 5.42.0.  You should change code that uses\n"
               . "Test::More::isn't() to use Test::More::isnt() as a replacement"
               . " at t/Legacy/More.t line 31\n",
            "Got expected warning from isn::t() under use warnings");
}
{
    no warnings "deprecated";
    my $warning;
    local $SIG{__WARN__}= sub { $warning = $_[0] };
    isn::t("foo", "bar",     'foo isn\'t bar');
    is($warning, undef, "No warnings from isn::t() under no warnings deprecated");
}

#'#
like("fooble", '/^foo/',    'foo is like fooble');
like("FooBle", '/foo/i',   'foo is like FooBle');
like("/usr/local/pr0n/", '/^\/usr\/local/',   'regexes with slashes in like' );

unlike("fbar", '/^bar/',    'unlike bar');
unlike("FooBle", '/foo/',   'foo is unlike FooBle');
unlike("/var/local/pr0n/", '/^\/usr\/local/','regexes with slashes in unlike' );

my @foo = qw(foo bar baz);
unlike(@foo, '/foo/');

can_ok('Test::More', qw(require_ok use_ok ok is isnt like skip can_ok
                        pass fail eq_array eq_hash eq_set));
can_ok(bless({}, "Test::More"), qw(require_ok use_ok ok is isnt like skip 
                                   can_ok pass fail eq_array eq_hash eq_set));


isa_ok(bless([], "Foo"), "Foo");
isa_ok([], 'ARRAY');
isa_ok(\42, 'SCALAR');
{
    local %Bar::;
    local @Foo::ISA = 'Bar';
    isa_ok( "Foo", "Bar" );
}


# can_ok() & isa_ok should call can() & isa() on the given object, not 
# just class, in case of custom can()
{
       local *Foo::can;
       local *Foo::isa;
       *Foo::can = sub { $_[0]->[0] };
       *Foo::isa = sub { $_[0]->[0] };
       my $foo = bless([0], 'Foo');
       ok( ! $foo->can('bar') );
       ok( ! $foo->isa('bar') );
       $foo->[0] = 1;
       can_ok( $foo, 'blah');
       isa_ok( $foo, 'blah');
}


pass('pass() passed');

ok( eq_array([qw(this that whatever)], [qw(this that whatever)]),
    'eq_array with simple arrays' );
is @Test::More::Data_Stack, 0, '@Data_Stack not holding onto things';

ok( eq_hash({ foo => 42, bar => 23 }, {bar => 23, foo => 42}),
    'eq_hash with simple hashes' );
is @Test::More::Data_Stack, 0;

ok( eq_set([qw(this that whatever)], [qw(that whatever this)]),
    'eq_set with simple sets' );
is @Test::More::Data_Stack, 0;

my @complex_array1 = (
                      [qw(this that whatever)],
                      {foo => 23, bar => 42},
                      "moo",
                      "yarrow",
                      [qw(498 10 29)],
                     );
my @complex_array2 = (
                      [qw(this that whatever)],
                      {foo => 23, bar => 42},
                      "moo",
                      "yarrow",
                      [qw(498 10 29)],
                     );

is_deeply( \@complex_array1, \@complex_array2,    'is_deeply with arrays' );
ok( eq_array(\@complex_array1, \@complex_array2),
    'eq_array with complicated arrays' );
ok( eq_set(\@complex_array1, \@complex_array2),
    'eq_set with complicated arrays' );

my @array1 = (qw(this that whatever),
              {foo => 23, bar => 42} );
my @array2 = (qw(this that whatever),
              {foo => 24, bar => 42} );

ok( !eq_array(\@array1, \@array2),
    'eq_array with slightly different complicated arrays' );
is @Test::More::Data_Stack, 0;

ok( !eq_set(\@array1, \@array2),
    'eq_set with slightly different complicated arrays' );
is @Test::More::Data_Stack, 0;

my %hash1 = ( foo => 23,
              bar => [qw(this that whatever)],
              har => { foo => 24, bar => 42 },
            );
my %hash2 = ( foo => 23,
              bar => [qw(this that whatever)],
              har => { foo => 24, bar => 42 },
            );

is_deeply( \%hash1, \%hash2,    'is_deeply with complicated hashes' );
ok( eq_hash(\%hash1, \%hash2),  'eq_hash with complicated hashes');

%hash1 = ( foo => 23,
           bar => [qw(this that whatever)],
           har => { foo => 24, bar => 42 },
         );
%hash2 = ( foo => 23,
           bar => [qw(this tha whatever)],
           har => { foo => 24, bar => 42 },
         );

ok( !eq_hash(\%hash1, \%hash2),
    'eq_hash with slightly different complicated hashes' );
is @Test::More::Data_Stack, 0;

is( Test::Builder->new, Test::More->builder,    'builder()' );


cmp_ok(42, '==', 42,        'cmp_ok ==');
cmp_ok('foo', 'eq', 'foo',  '       eq');
cmp_ok(42.5, '<', 42.6,     '       <');
cmp_ok(0, '||', 1,          '       ||');


# Piers pointed out sometimes people override isa().
{
    package Wibble;
    sub isa {
        my($self, $class) = @_;
        return 1 if $class eq 'Wibblemeister';
    }
    sub new { bless {} }
}
isa_ok( Wibble->new, 'Wibblemeister' );

my $sub = sub {};
is_deeply( $sub, $sub, 'the same function ref' );

use Symbol;
my $glob = gensym;
is_deeply( $glob, $glob, 'the same glob' );

is_deeply( { foo => $sub, bar => [1, $glob] },
           { foo => $sub, bar => [1, $glob] }
         );


# rt.cpan.org 53469  is_deeply with regexes
is_deeply( qr/a/, qr/a/, "same regex" );


# These two tests must remain at the end.
is( $@, $Err,               '$@ untouched' );
cmp_ok( $!, '==', $Errno,   '$! untouched' );
