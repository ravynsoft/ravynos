#!perl -w

use strict;
use warnings;
use Test::More;

use XS::APItest;

my %ix;
sub showix {
    diag join ", ", map { $ix{$_} > 1 ? "$_ x $ix{$_}" : $_ } sort { $a <=> $b } keys %ix;
}
my $len = 100;
my $str= "a" x $len;
my $pat= join "|", map { "a" x $_ } 1 .. $len;

$str=~/^($pat)(??{ $ix{get_savestack_ix()}++; "(?!)" })/;
my $keys= 0+keys %ix;
cmp_ok($keys,">",0, "We expect at least one key in %ix for (??{ ... }) test");
cmp_ok($keys,"<=", 2, "We expect no more than two keys in %ix if (??{ ... }) does not leak")
    or showix();

%ix= ();
$str=~/^($pat)(?{ $ix{my $x=get_savestack_ix()}++; })(?!)/;
$keys= 0+keys %ix;
cmp_ok($keys,">",0, "We expect at least one key in %ix for (?{ ...  }) test");
cmp_ok($keys, "<=", 2, "We expect no more than two keys in %ix if (?{ ... }) does not leak")
    or showix();

%ix= ();
$str=~/^($pat)(?(?{ $ix{my $x=get_savestack_ix()}++; })x|y)(?!)/;
$keys= 0+keys %ix;
cmp_ok($keys,">",0, "We expect at least one key in %ix for (?(?{ ... })yes|no) test");
cmp_ok($keys, "<=", 2, "We expect no more than two keys in %ix if (?(?{ ... })yes|no) does not leak")
    or showix();

done_testing();
