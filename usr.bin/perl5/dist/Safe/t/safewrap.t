#!perl -w

$|=1;
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

use strict;
use Safe 1.00;
use Test::More tests => 10;

my $safe = Safe->new('PLPerl');
$safe->permit_only(qw(:default sort));

# eval within an eval: the outer eval is compiled into the sub, the inner is
# compiled (by the outer) at runtime and so is subject to runtime opmask
my $sub1 = sub { eval " eval '1+1' " };
is $sub1->(), 2;

my $sub1w = $safe->wrap_code_ref($sub1);
is ref $sub1w, 'CODE';
is eval { $sub1w->() }, undef;
like $@, qr/eval .* trapped by operation mask/;

is $sub1->(), 2, 'original ref should be unaffected';

# setup args for wrap_code_refs_within including nested data
my @args = (42, [[ 0, { sub => $sub1 }, 2 ]], 24);
is $args[1][0][1]{sub}, $sub1;

$safe->wrap_code_refs_within(@args);
my $sub1w2 = $args[1][0][1]{sub};
isnt $sub1w2, $sub1;
is eval { $sub1w2->() }, undef;
like $@, qr/eval .* trapped by operation mask/;

# Avoid infinite recursion when looking for coderefs
my $r = $safe->reval(<<'END');
%a = ();
%b = (a => \%a);
$a{b} = \%b;
42;
END
is($r, 42);
