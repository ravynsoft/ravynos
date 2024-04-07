#!./perl
#
# check UNIVERSAL
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../dist/base/lib');
    $| = 1;
    require "./test.pl";
}

plan tests => 143;

$a = {};
bless $a, "Bob";
ok $a->isa("Bob");

package Human;
sub eat {}

package Female;
@ISA=qw(Human);

package Alice;
@ISA=qw(Bob Female);
sub sing;
sub drink { return "drinking " . $_[1]  }
sub new { bless {} }

$Alice::VERSION = 2.718;

{
    package Cedric;
    our @ISA;
    use base qw(Human);
}

{
    package Programmer;
    our $VERSION = 1.667;

    sub write_perl { 1 }
}

package main;



$a = new Alice;

ok $a->isa("Alice");
ok $a->isa("main::Alice");    # check that alternate class names work

ok(("main::Alice"->new)->isa("Alice"));

ok $a->isa("Bob");
ok $a->isa("main::Bob");

ok $a->isa("Female");

ok ! $a->isa("Female\0NOT REALLY!"), "->isa is nul-clean.";

ok $a->isa("Human");

ok ! $a->isa("Male");

ok ! $a->isa('Programmer');

ok $a->isa("HASH");

ok $a->can("eat");
ok ! $a->can("eat\0Except not!"), "->can is nul-clean.";
ok ! $a->can("sleep");
ok my $ref = $a->can("drink");        # returns a coderef
is $a->$ref("tea"), "drinking tea"; # ... which works
ok $ref = $a->can("sing");
eval { $a->$ref() };
ok $@;                                # ... but not if no actual subroutine

ok (!Cedric->isa('Programmer'));

ok (Cedric->isa('Human'));

push(@Cedric::ISA,'Programmer');

ok (Cedric->isa('Programmer'));

{
    package Alice;
    base::->import('Programmer');
}

ok $a->isa('Programmer');
ok $a->isa("Female");

@Cedric::ISA = qw(Bob);

ok (!Cedric->isa('Programmer'));

my $b = 'abc';
my @refs = qw(SCALAR SCALAR     LVALUE      GLOB ARRAY HASH CODE);
my @vals = (  \$b,   \3.14, \substr($b,1,1), \*b,  [],  {}, sub {} );
for ($p=0; $p < @refs; $p++) {
    for ($q=0; $q < @vals; $q++) {
        is UNIVERSAL::isa($vals[$p], $refs[$q]), ($p==$q or $p+$q==1);
    };
};

ok UNIVERSAL::can(23, "can");
++${"23::foo"};
ok UNIVERSAL::can("23", "can"), '"23" can can when the pack exists';
ok UNIVERSAL::can(23, "can"), '23 can can when the pack exists';
sub IO::Handle::turn {}
ok UNIVERSAL::can(*STDOUT, 'turn'), 'globs with IOs can';
ok UNIVERSAL::can(\*STDOUT, 'turn'), 'globrefs with IOs can';
ok UNIVERSAL::can("STDOUT", 'turn'), 'IO barewords can';

ok $a->can("VERSION");

ok $a->can("can");
ok ! $a->can("export_tags");	# a method in Exporter

cmp_ok eval { $a->VERSION }, '==', 2.718;

ok ! (eval { $a->VERSION(2.719) });
like $@, qr/^Alice version 2.719 required--this is only version 2.718 at /;

ok (eval { $a->VERSION(2.718) });
is $@, '';

ok ! (eval { $a->VERSION("version") });
like $@, qr/^Invalid version format/;

$aversion::VERSION = "version";
ok ! (eval { aversion->VERSION(2.719) });
like $@, qr/^Invalid version format/;

my $subs = join ' ', sort grep { defined &{"UNIVERSAL::$_"} } keys %UNIVERSAL::;
if ('a' lt 'A') {
    is $subs, "can isa DOES VERSION";
} else {
    is $subs, "DOES VERSION can isa";
}

ok $a->isa("UNIVERSAL");

ok ! UNIVERSAL::isa([], "UNIVERSAL");

ok ! UNIVERSAL::can({}, "can");

ok UNIVERSAL::isa(Alice => "UNIVERSAL");

cmp_ok UNIVERSAL::can(Alice => "can"), '==', \&UNIVERSAL::can;

# now use UNIVERSAL.pm and see what changes
eval "use UNIVERSAL";

ok $a->isa("UNIVERSAL");

my $sub2 = join ' ', sort grep { defined &{"UNIVERSAL::$_"} } keys %UNIVERSAL::;
# XXX import being here is really a bug
if ('a' lt 'A') {
    is $sub2, "can import isa DOES VERSION";
} else {
    is $sub2, "DOES VERSION can import isa";
}

eval 'sub UNIVERSAL::sleep {}';
ok $a->can("sleep");

ok UNIVERSAL::can($b, "can");

ok ! $a->can("export_tags");	# a method in Exporter

ok ! UNIVERSAL::isa("\xff\xff\xff\0", 'HASH');

{
    # test isa() and can() on magic variables
    "Human" =~ /(.*)/;
    ok $1->isa("Human");
    ok $1->can("eat");
    package HumanTie;
    sub TIESCALAR { bless {} }
    sub FETCH { "Human" }
    tie my($x), "HumanTie";
    ::ok $x->isa("Human");
    ::ok $x->can("eat");
}

# bugid 3284
# a second call to isa('UNIVERSAL') when @ISA is null failed due to caching

@X::ISA=();
my $x = {}; bless $x, 'X';
ok $x->isa('UNIVERSAL');
ok $x->isa('UNIVERSAL');


# Check that the "historical accident" of UNIVERSAL having an import()
# method doesn't effect anyone else.
eval { Some::Package->import("bar") };
is $@, '';


# This segfaulted in a blead.
fresh_perl_is('package Foo; Foo->VERSION;  print "ok"', 'ok');

# So did this.
fresh_perl_is('$:; UNIVERSAL::isa(":","Unicode::String");print "ok"','ok');

package Foo;

sub DOES { 1 }

package Bar;

@Bar::ISA = 'Foo';

package Baz;

package main;
ok( Foo->DOES( 'bar' ), 'DOES() should call DOES() on class' );
ok( Bar->DOES( 'Bar' ), '... and should fall back to isa()' );
ok( Bar->DOES( 'Foo' ), '... even when inherited' );
ok( Baz->DOES( 'Baz' ), '... even without inheriting any other DOES()' );
ok( ! Baz->DOES( 'Foo' ), '... returning true or false appropriately' );

ok( ! "T"->DOES( "T\0" ), 'DOES() is nul-clean' );
ok( ! Baz->DOES( "Baz\0Boy howdy" ), 'DOES() is nul-clean' );

package Pig;
package Bodine;
Bodine->isa('Pig');
*isa = \&UNIVERSAL::isa;
eval { isa({}, 'HASH') };
::is($@, '', "*isa correctly found");

package main;
eval { UNIVERSAL::DOES([], "foo") };
like( $@, qr/Can't call method "DOES" on unblessed reference/,
    'DOES call error message says DOES, not isa' );

# Tests for can seem to be split between here and method.t
# Add the verbatim perl code mentioned in the comments of
# Message-ID: E14ufZD-0007kD-00@libra.cus.cam.ac.uk
# https://www.nntp.perl.org/group/perl.perl5.porters/2001/05/msg35327.html
# but never actually tested.
is(UNIVERSAL->can("NoSuchPackage::foo"), undef);

@splatt::ISA = 'zlopp';
ok (splatt->isa('zlopp'));
ok (!splatt->isa('plop'));

# This should reset the ->isa lookup cache
@splatt::ISA = 'plop';
# And here is the new truth.
ok (!splatt->isa('zlopp'));
ok (splatt->isa('plop'));

use warnings "deprecated";
{
    my $m;
    local $SIG{__WARN__} = sub { $m = $_[0] };
    eval "use UNIVERSAL 'can'";
    like($@, qr/^UNIVERSAL does not export anything\b/,
	"error for UNIVERSAL->import('can')");
    is($m, undef,
	"no deprecation warning for UNIVERSAL->import('can')");

	  undef $m;
    eval "use UNIVERSAL";
    is($@, "",
	"no error for UNIVERSAL->import");
    is($m, undef,
	"no deprecation warning for UNIVERSAL->import");
}

# Test: [perl #66112]: change @ISA inside  sub isa
{
    package RT66112::A;

    package RT66112::B;

    sub isa {
	my $self = shift;
	@ISA = qw/RT66112::A/;
	return $self->SUPER::isa(@_);
    }

    package RT66112::C;

    package RT66112::D;

    sub isa {
	my $self = shift;
	@RT66112::E::ISA = qw/RT66112::A/;
	return $self->SUPER::isa(@_);
    }

    package RT66112::E;

    package main;

    @RT66112::B::ISA = qw//;
    @RT66112::C::ISA = qw/RT66112::B/;
    @RT66112::T1::ISA = qw/RT66112::C/;
    ok(RT66112::T1->isa('RT66112::C'), "modify \@ISA in isa (RT66112::T1 isa RT66112::C)");

    @RT66112::B::ISA = qw//;
    @RT66112::C::ISA = qw/RT66112::B/;
    @RT66112::T2::ISA = qw/RT66112::C/;
    ok(RT66112::T2->isa('RT66112::B'), "modify \@ISA in isa (RT66112::T2 isa RT66112::B)");

    @RT66112::B::ISA = qw//;
    @RT66112::C::ISA = qw/RT66112::B/;
    @RT66112::T3::ISA = qw/RT66112::C/;
    ok(RT66112::T3->isa('RT66112::A'), "modify \@ISA in isa (RT66112::T3 isa RT66112::A)") or require mro, diag "@{mro::get_linear_isa('RT66112::T3')}";

    @RT66112::E::ISA = qw/RT66112::D/;
    @RT66112::T4::ISA = qw/RT66112::E/;
    ok(RT66112::T4->isa('RT66112::E'), "modify \@ISA in isa (RT66112::T4 isa RT66112::E)");

    @RT66112::E::ISA = qw/RT66112::D/;
    @RT66112::T5::ISA = qw/RT66112::E/;
    ok(! RT66112::T5->isa('RT66112::D'), "modify \@ISA in isa (RT66112::T5 not isa RT66112::D)");

    @RT66112::E::ISA = qw/RT66112::D/;
    @RT66112::T6::ISA = qw/RT66112::E/;
    ok(RT66112::T6->isa('RT66112::A'), "modify \@ISA in isa (RT66112::T6 isa RT66112::A)");
}

ok(Undeclared->can("can"));
sub Undeclared::foo { }
ok(Undeclared->can("foo"));
ok(!Undeclared->can("something_else"));

ok(Undeclared->isa("UNIVERSAL"));

# keep this at the end to avoid messing up earlier tests, since it modifies
# @UNIVERSAL::ISA
@UNIVERSAL::ISA = ('UniversalParent');
{ package UniversalIsaTest1; }
ok(UniversalIsaTest1->isa('UniversalParent'));
ok(UniversalIsaTest2->isa('UniversalParent'));
