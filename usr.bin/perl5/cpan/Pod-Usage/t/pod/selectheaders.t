use Test::More tests => 2;

use strict;
use warnings;

my $blib = $ENV{PERL_CORE} ? '-I../../lib' : '-Mblib';

my $pl = $0;
$pl =~ s{t$}{pl};

my $out = `$^X $blib $pl Foo`;
$out =~ s{\s+}{ }gs;
$out =~ s{^\s+|\s+$}{}gs;
is($out, 'Foo: This is foo', 'selection of Foo section');

$out = `$^X $blib $pl Bar`;
$out =~ s{\s+}{ }gs;
$out =~ s{^\s+|\s+$}{}gs;
is($out, 'Bar: This is bar.', 'selection of Bar section');

