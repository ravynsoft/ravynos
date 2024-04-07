use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

print "1..2\n";

my $r;
my $s = q{xx xxxxxxxx xxxxxxxxx xx xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=xxxxxxxxxxxxxxxxxxxxxxxx};

eval { $r = wrap("", "", $s) };
ok(! $@, $@);
ok($r eq "xx xxxxxxxx xxxxxxxxx xx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=xxxxxxxxxxxxx
xxxxxxxxxxx", "match");
