#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
    require 'st-dump.pl';
}

use Storable qw(freeze nfreeze thaw);

$Storable::flags = Storable::FLAGS_COMPAT;

use Test::More tests => 21;

$a = 'toto';
$b = \$a;
$c = bless {}, CLASS;
$c->{attribute} = $b;
$d = {};
$e = [];
$d->{'a'} = $e;
$e->[0] = $d;
%a = ('key', 'value', 1, 0, $a, $b, 'cvar', \$c);
@a = ('first', undef, 3, -4, -3.14159, 456, 4.5, $d, \$d, \$e, $e,
	$b, \$a, $a, $c, \$c, \%a);

my $f1 = freeze(\@a);
isnt($f1, undef);

$dumped = &dump(\@a);
isnt($dumped, undef);

$root = thaw($f1);
isnt($root, undef);

$got = &dump($root);
isnt($got, undef);

is($got, $dumped);

package FOO; @ISA = qw(Storable);

sub make {
	my $self = bless {};
	$self->{key} = \%main::a;
	return $self;
};

package main;

$foo = FOO->make;
my $f2 = $foo->freeze;
isnt($f2, undef);

my $f3 = $foo->nfreeze;
isnt($f3, undef);

$root3 = thaw($f3);
isnt($root3, undef);

is(&dump($foo), &dump($root3));

$root = thaw($f2);
is(&dump($foo), &dump($root));

is(&dump($root3), &dump($root));

$other = freeze($root);
is(length$other, length $f2);

$root2 = thaw($other);
is(&dump($root2), &dump($root));

$VAR1 = [
	'method',
	1,
	'prepare',
	'SELECT table_name, table_owner, num_rows FROM iitables
                  where table_owner != \'$ingres\' and table_owner != \'DBA\''
];

$x = nfreeze($VAR1);
$VAR2 = thaw($x);
is($VAR2->[3], $VAR1->[3]);

# Test the workaround for LVALUE bug in perl 5.004_04 -- from Gisle Aas
sub foo { $_[0] = 1 }
$foo = [];
foo($foo->[1]);
eval { freeze($foo) };
is($@, '');

# Test cleanup bug found by Claudio Garcia -- RAM, 08/06/2001
my $thaw_me = 'asdasdasdasd';

eval {
	my $thawed = thaw $thaw_me;
};
isnt($@, '');

my %to_be_frozen = (foo => 'bar');
my $frozen;
eval {
	$frozen = freeze \%to_be_frozen;
};
is($@, '');

freeze {};
eval { thaw $thaw_me };
eval { $frozen = freeze { foo => {} } };
is($@, '');

thaw $frozen;			# used to segfault here
pass("Didn't segfault");

SKIP: {
    my (@a, @b);
    eval '
        $a = []; $#$a = 2; $a->[1] = undef;
        $b = thaw freeze $a;
        @a = map { ~~ exists $a->[$_] } 0 .. $#$a;
        @b = map { ~~ exists $b->[$_] } 0 .. $#$b;
    ';
    is($@, '');
    is("@a", "@b");
}
