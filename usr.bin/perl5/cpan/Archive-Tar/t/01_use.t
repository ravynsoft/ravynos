use Test::More tests => 2;
use strict;

use_ok('Archive::Tar') or diag 'Archive::Tar not found -- exit' && die;

my $tar = Archive::Tar->new;
isa_ok( $tar, 'Archive::Tar', 'Object created' );
