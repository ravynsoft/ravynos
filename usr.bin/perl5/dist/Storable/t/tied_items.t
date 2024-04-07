#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

#
# Tests ref to items in tied hash/array structures.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

$^W = 0;

use Storable qw(dclone);
use Test::More tests => 8;

$Storable::flags = Storable::FLAGS_COMPAT;

$h_fetches = 0;

sub H::TIEHASH { bless \(my $x), "H" }
sub H::FETCH { $h_fetches++; $_[1] - 70 }

tie %h, "H";

$ref = \$h{77};
$ref2 = dclone $ref;

is($h_fetches, 0);
is($$ref2, $$ref);
is($$ref2, 7);
is($h_fetches, 2);

$a_fetches = 0;

sub A::TIEARRAY { bless \(my $x), "A" }
sub A::FETCH { $a_fetches++; $_[1] - 70 }

tie @a, "A";

$ref = \$a[78];
$ref2 = dclone $ref;

is($a_fetches, 0);
is($$ref2, $$ref);
is($$ref2, 8);
# a bug in 5.12 and earlier caused an extra FETCH
is($a_fetches, $] < 5.013 ? 3 : 2);
