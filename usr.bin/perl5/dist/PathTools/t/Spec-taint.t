#!./perl -Tw
# Testing File::Spec under taint mode.

use strict;

chdir 't' unless $ENV{PERL_CORE};

use File::Spec;
use lib File::Spec->catdir('t', 'lib');
use Test::More tests => 2;

use Scalar::Util qw/tainted/;

my $ret;
eval { $ret = File::Spec->tmpdir };
is( $@, '',		"tmpdir should not explode under taint mode" );
ok( !tainted($ret),	"its return value should not be tainted" );
