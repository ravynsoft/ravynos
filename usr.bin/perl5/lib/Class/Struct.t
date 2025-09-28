#!./perl -w

BEGIN {
	chdir 't' if -d 't';
	@INC = '../lib';
}

#
# A couple of simple classes to use as struct elements.
#
package aClass;
sub new { bless {}, shift }
sub meth { 42 }

package RecClass;
sub new { bless {}, shift }

#
# The first of our Class::Struct based objects.
#
package MyObj;
use Class::Struct;
use Class::Struct 'struct'; # test out both forms
use Class::Struct SomeClass => { SomeElem => '$' };

struct( s => '$', a => '@', h => '%', c => 'aClass' );

#
# The second Class::Struct objects:
# test the 'compile-time without package name' feature.
#
package MyOther;
use Class::Struct s => '$', a => '@', h => '%', c => 'aClass';

#
# test overriden accessors
#
package OverrideAccessor;
use Class::Struct;

{ 
 no warnings qw(Class::Struct);
 struct( 'OverrideAccessor', { count => '$' } );
}

sub count {
  my ($self,$count) = @_;

  if ( @_ >= 2 ) {
    $self->{'OverrideAccessor::count'} = $count + 9;
  }

  return $self->{'OverrideAccessor::count'};
}

#
# back to main...
#
package main;

use Test::More;

my $obj = MyObj->new;
isa_ok $obj, 'MyObj';

$obj->s('foo');
is $obj->s(), 'foo';

isa_ok $obj->a, 'ARRAY';
$obj->a(2, 'secundus');
is $obj->a(2), 'secundus';

$obj->a([4,5,6]);
is $obj->a(1), 5;

isa_ok $obj->h, 'HASH';
$obj->h('x', 10);
is $obj->h('x'), 10;

$obj->h({h=>7,r=>8,f=>9});
is $obj->h('r'), 8;

is $obj->c, undef;

$obj = MyObj->new( c => aClass->new );
isa_ok $obj->c, 'aClass';
is $obj->c->meth(), 42;


$obj = MyOther->new;
isa_ok $obj, 'MyOther';

$obj->s('foo');
is $obj->s(), 'foo';

isa_ok $obj->a, 'ARRAY';
$obj->a(2, 'secundus');
is $obj->a(2), 'secundus';

$obj->a([4,5,6]);
is $obj->a(1), 5;

isa_ok $obj->h, 'HASH';
$obj->h('x', 10);
is $obj->h('x'), 10;

$obj->h({h=>7,r=>8,f=>9});
is $obj->h('r'), 8;

is $obj->c, undef;

$obj = MyOther->new( c => aClass->new );
isa_ok $obj->c, 'aClass';
is $obj->c->meth(), 42;



my $obk = SomeClass->new();
$obk->SomeElem(123);
is $obk->SomeElem(), 123;

my $recobj = RecClass->new();
isa_ok $recobj, 'RecClass';

my $override_obj = OverrideAccessor->new( count => 3 );
is $override_obj->count, 12;

$override_obj->count( 1 );
is $override_obj->count, 10;


use Class::Struct Kapow => { z_zwap => 'Regexp', sploosh => 'MyObj' };

is eval { main->new(); }, undef,
    'No new method injected into current package';

my $obj3 = Kapow->new();

isa_ok $obj3, 'Kapow';
is $obj3->z_zwap, undef, 'No z_zwap member by default';
is $obj3->sploosh, undef, 'No sploosh member by default';
$obj3->z_zwap(qr//);
isa_ok $obj3->z_zwap, 'Regexp', 'Can set z_zwap member';
$obj3->sploosh(MyObj->new(s => 'pie'));
isa_ok $obj3->sploosh, 'MyObj',
    'Can set sploosh member to object of correct class';
is $obj3->sploosh->s, 'pie', 'Can set sploosh member to correct object';

my $obj4 = Kapow->new( z_zwap => qr//, sploosh => MyObj->new(a => ['Good']) );

isa_ok $obj4, 'Kapow';
isa_ok $obj4->z_zwap, 'Regexp', 'Initialised z_zwap member';
isa_ok $obj4->sploosh, 'MyObj', 'Initialised sploosh member';
is_deeply $obj4->sploosh->a, ['Good'], 'with correct object';

my $obj5 = Kapow->new( sploosh => { h => {perl => 'rules'} } );

isa_ok $obj5, 'Kapow';
is $obj5->z_zwap, undef, 'No z_zwap member by default';
isa_ok $obj5->sploosh, 'MyObj', 'Initialised sploosh member from hash';
is_deeply $obj5->sploosh->h, { perl => 'rules'} , 'with correct object';

is eval {
    package MyObj;
    struct( s => '$', a => '@', h => '%', c => 'aClass' );
}, undef, 'Calling struct a second time fails';

like $@, qr/^function 'new' already defined in package MyObj/,
    'fails with the expected error';

is eval { MyObj->new( a => {} ) }, undef,
    'Using a hash where an array reference is expected';
like $@, qr/^Initializer for a must be array reference/,
    'fails with the expected error';

is eval { MyObj->new( h => [] ) }, undef,
    'Using an array where a hash reference is expected';
like $@, qr/^Initializer for h must be hash reference/,
    'fails with the expected error';

is eval { Kapow->new( sploosh => { h => [perl => 'rules'] } ); }, undef,
    'Using an array where a hash reference is expected in an initialiser list';
like $@, qr/^Initializer for h must be hash reference/,
    'fails with the expected error';

is eval { Kapow->new( sploosh => [ h => {perl => 'rules'} ] ); }, undef,
    "Using an array for a member object's initialiser list";
like $@, qr/^Initializer for sploosh must be hash or MyObj reference/,
    'fails with the expected error';

is eval {
    package Crraack;
    use Class::Struct 'struct';
    struct( 'pow' => '@$%!' );
}, undef, 'Bad type fails';
like $@, qr/^'\@\$\%\!' is not a valid struct element type/,
    'with the expected error';

is eval {
    $obj3->sploosh(MyOther->new(s => 3.14));
}, undef, 'Setting member to the wrong class of object fails';
like $@, qr/^sploosh argument is wrong class/,
    'with the expected error';
is $obj3->sploosh->s, 'pie', 'Object is unchanged';

is eval {
    $obj3->sploosh(MyObj->new(s => 3.14), 'plop');
}, undef, 'Too many arguments to setter fails';
like $@, qr/^Too many args to sploosh/,
    'with the expected error';
is $obj3->sploosh->s, 'pie', 'Object is unchanged';

is eval {
    package Blurp;
    use Class::Struct 'struct';
    struct( Blurp => {}, 'Bonus!' );
}, undef, 'hash based class with extra argument fails';
like $@, qr/\Astruct usage error.*\n.*\n/,
    'with the expected confession';

is eval {
    package Zamm;
    use Class::Struct 'struct';
    struct( Zamm => [], 'Bonus!' );
}, undef, 'array based class with extra argument fails';
like $@, qr/\Astruct usage error.*\n.*\n/,
    'with the expected confession';

is eval {
    package Thwapp;
    use Class::Struct 'struct';
    struct( Thwapp => ['Bonus!'] );
}, undef, 'array based class with extra constructor argument fails';
like $@, qr/\Astruct usage error.*\n.*\n/,
    'with the expected confession';

is eval {
    package Rakkk;
    use Class::Struct 'struct';
    struct( z_zwap => 'Regexp', sploosh => 'MyObj', 'Bonus' );
}, undef, 'default array based class with extra constructor argument fails';
like $@, qr/\Astruct usage error.*\n.*\n/,
    'with the expected confession';

is eval {
    package Awk;
    use parent -norequire, 'Urkkk';
    use Class::Struct 'struct';
    struct( beer => 'foamy' );
}, undef, '@ISA is not allowed';
like $@, qr/^struct class cannot be a subclass \(\@ISA not allowed\)/,
    'with the expected error';

done_testing;
