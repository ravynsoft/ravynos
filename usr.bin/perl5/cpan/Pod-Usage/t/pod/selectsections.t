use Test::More tests => 2;

use strict;
use warnings;

my $blib = $ENV{PERL_CORE} ? '-I../../lib' : '-Mblib';

my $pl = $0;
$pl =~ s{t$}{pl};

my $out = `$^X $blib $pl 0`;
$out =~ s{\s+}{ }gs;
$out =~ s{^\s+|\s+$}{}gs;
is($out, 'Name: trypodi - pod sections usage test Actions: Para for actions. help: Help text.', 'selection of specific sections');

$out = `$^X $blib $pl 1`;
$out =~ s{\s+}{ }gs;
$out =~ s{^\s+|\s+$}{}gs;
is($out, 'Caveats: Description caveat text. Caveats: Options caveat text. Caveats: Environment caveat text.', 'selection of caveats sections');

