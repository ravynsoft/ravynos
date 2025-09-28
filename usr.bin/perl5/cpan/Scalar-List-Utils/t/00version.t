#!./perl

use strict;
use warnings;

use Scalar::Util ();
use List::Util ();
use List::Util::XS ();
use Sub::Util ();
use Test::More tests => 4;

is( $Scalar::Util::VERSION, $List::Util::VERSION, "VERSION mismatch between Scalar/List");
my $has_xs = eval { Scalar::Util->import('dualvar'); 1 };
my $xs_version = $has_xs ? $List::Util::VERSION : undef;
is( $List::Util::XS::VERSION, $xs_version, "VERSION mismatch between LU::XS and LU");
is( $Sub::Util::VERSION, $Scalar::Util::VERSION, "VERSION mistmatch between Sub/Scalar");
is( $Sub::Util::VERSION, $List::Util::VERSION, "VERSION mistmatch between Sub/List");

