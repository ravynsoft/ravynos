use Test::More tests => 1;

use strict;
use warnings;

my $blib = $ENV{PERL_CORE} ? '-I../../lib' : '-Mblib';

my $pl = $0;
$pl =~ s{t$}{pl};

my $out = `$^X $blib $pl`;
$out =~ s{\s+}{ }gs;
$out =~ s{^\s+|\s+$}{}gs;
# we want to make sure that the marked-up text is not lost
is($out, 'backup pkg please dest: Para for backup.');

