my @symbols;
BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_without_dynamic_extension($_) foreach qw(B Fcntl);
    # S_IFMT is a real subroutine, and acts as control
    # SEEK_SET is a proxy constant subroutine.
    @symbols = qw(S_IFMT SEEK_SET);
}

use strict;
use warnings;
plan(4 * @symbols);
use B qw(svref_2object GVf_IMPORTED_CV);
use Fcntl @symbols;

# GVf_IMPORTED_CV should not be set on the original, but should be set on the
# imported GV.

foreach my $symbol (@symbols) {
    my ($ps, $ms);
    {
	no strict 'refs';
	$ps = svref_2object(\*{"Fcntl::$symbol"});
	$ms = svref_2object(\*{"::$symbol"});
    }
    object_ok($ps, 'B::GV');
    is($ps->GvFLAGS() & GVf_IMPORTED_CV, 0,
       "GVf_IMPORTED_CV not set on original");
    object_ok($ms, 'B::GV');
    is($ms->GvFLAGS() & GVf_IMPORTED_CV, GVf_IMPORTED_CV,
       "GVf_IMPORTED_CV set on imported GV");
}
