use strict; use warnings;

BEGIN { require './t/lib/ok.pl' }
use Text::Wrap;

print "1..1\n";

$Text::Wrap::columns = 1;
eval { wrap('', '', ''); };

ok( !$@ );

