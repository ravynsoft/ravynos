#!perl

use v5.36;
use Test::More;

use XS::APItest;

my $plain_av = [1,2,3];
is_deeply newAVav($plain_av), [1,2,3], 'newAVav on plain array';

newAVav($plain_av)->[0]++;
is $plain_av->[0], 1, 'newAVav returns fresh storage';

{
    package TiedArray {
        sub TIEARRAY  { return bless [], "TiedArray"; }

        sub FETCHSIZE { return 3; }
        sub FETCH     { return $_[1] + 4; }
    }
    tie my @tied_av, "TiedArray";

    is_deeply newAVav(\@tied_av), [4,5,6], 'newAVav on tied array';
}

# Just use one key at first so order doesn't matter
my $plain_hv = {key => "value"};
is_deeply newAVhv($plain_hv), [key => "value"], 'newAVhv on plain hash';

newAVhv($plain_hv)->[1] .= "X";
is $plain_hv->{key}, "value", 'newAVhv returns fresh storage';

is_deeply [ sort +newAVhv({a => 1, b => 2, c => 3})->@* ], [ 1, 2, 3, "a", "b", "c" ],
    'newAVhv on multiple keys';

{
    package TiedHash {
        sub TIEHASH  { return bless [], "TiedHash"; }

        sub FETCH    { return $_[1] eq "k" ? "v" : undef }
        sub FIRSTKEY { return "k" }
        sub NEXTKEY  { return; }
    }
    tie my %tied_hv, "TiedHash";

    is_deeply newAVhv(\%tied_hv), [k => "v"], 'newAVhv on tied hash';
}

done_testing;
