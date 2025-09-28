#!perl -X
#
# Regression test for [perl #74170] (missing SPAGAIN after DD_Dump(...)):
# Since it’s so large, it gets its own file.

use strict;
use warnings;

use Test::More tests => 1;
use Data::Dumper;

our %repos = real_life_setup();

$Data::Dumper::Indent = 1;
# A custom sort sub is necessary for reproducing the bug, as this is where
# the stack gets reallocated.
$Data::Dumper::Sortkeys = sub { return [ reverse sort keys %{$_[0]} ]; }
    unless exists $ENV{NO_SORT_SUB};

ok(Data::Dumper->Dump([\%repos], [qw(*repos)]), "RT 74170 test");

sub real_life_setup {
    # set up the %repos hash in a manner that reflects a real run of
    # the gitolite "compiler" script:
    # Yes, all this is necessary to get the stack in such a state that the
    # custom sort sub will trigger a reallocation.
    my %repos;
    push @{ $repos{''}{'@all'} }, ();
    push @{ $repos{''}{'guser86'} }, ();
    push @{ $repos{''}{'guser87'} }, ();
    push @{ $repos{''}{'user88'} }, ();
    push @{ $repos{''}{'grussell'} }, ();
    push @{ $repos{''}{'guser0'} }, ();
    push @{ $repos{''}{'guser1'} }, ();
    push @{ $repos{''}{'guser10'} }, ();
    push @{ $repos{''}{'guser11'} }, ();
    push @{ $repos{''}{'guser12'} }, ();
    push @{ $repos{''}{'guser13'} }, ();
    push @{ $repos{''}{'guser14'} }, ();
    push @{ $repos{''}{'guser15'} }, ();
    push @{ $repos{''}{'guser16'} }, ();
    push @{ $repos{''}{'guser17'} }, ();
    push @{ $repos{''}{'guser18'} }, ();
    push @{ $repos{''}{'guser19'} }, ();
    push @{ $repos{''}{'guser2'} }, ();
    push @{ $repos{''}{'guser20'} }, ();
    push @{ $repos{''}{'guser21'} }, ();
    push @{ $repos{''}{'guser22'} }, ();
    push @{ $repos{''}{'guser23'} }, ();
    push @{ $repos{''}{'guser24'} }, ();
    push @{ $repos{''}{'guser25'} }, ();
    push @{ $repos{''}{'guser26'} }, ();
    push @{ $repos{''}{'guser27'} }, ();
    push @{ $repos{''}{'guser28'} }, ();
    push @{ $repos{''}{'guser29'} }, ();
    push @{ $repos{''}{'guser3'} }, ();
    push @{ $repos{''}{'guser30'} }, ();
    push @{ $repos{''}{'guser31'} }, ();
    push @{ $repos{''}{'guser32'} }, ();
    push @{ $repos{''}{'guser33'} }, ();
    push @{ $repos{''}{'guser34'} }, ();
    push @{ $repos{''}{'guser35'} }, ();
    push @{ $repos{''}{'guser36'} }, ();
    push @{ $repos{''}{'guser37'} }, ();
    push @{ $repos{''}{'guser38'} }, ();
    push @{ $repos{''}{'guser39'} }, ();
    push @{ $repos{''}{'guser4'} }, ();
    push @{ $repos{''}{'guser40'} }, ();
    push @{ $repos{''}{'guser41'} }, ();
    push @{ $repos{''}{'guser42'} }, ();
    push @{ $repos{''}{'guser43'} }, ();
    push @{ $repos{''}{'guser44'} }, ();
    push @{ $repos{''}{'guser45'} }, ();
    push @{ $repos{''}{'guser46'} }, ();
    push @{ $repos{''}{'guser47'} }, ();
    push @{ $repos{''}{'guser48'} }, ();
    push @{ $repos{''}{'guser49'} }, ();
    push @{ $repos{''}{'guser5'} }, ();
    push @{ $repos{''}{'guser50'} }, ();
    push @{ $repos{''}{'guser51'} }, ();
    push @{ $repos{''}{'guser52'} }, ();
    push @{ $repos{''}{'guser53'} }, ();
    push @{ $repos{''}{'guser54'} }, ();
    push @{ $repos{''}{'guser55'} }, ();
    push @{ $repos{''}{'guser56'} }, ();
    push @{ $repos{''}{'guser57'} }, ();
    push @{ $repos{''}{'guser58'} }, ();
    push @{ $repos{''}{'guser59'} }, ();
    push @{ $repos{''}{'guser6'} }, ();
    push @{ $repos{''}{'guser60'} }, ();
    push @{ $repos{''}{'guser61'} }, ();
    push @{ $repos{''}{'guser62'} }, ();
    push @{ $repos{''}{'guser63'} }, ();
    push @{ $repos{''}{'guser64'} }, ();
    push @{ $repos{''}{'guser65'} }, ();
    push @{ $repos{''}{'guser66'} }, ();
    push @{ $repos{''}{'guser67'} }, ();
    push @{ $repos{''}{'guser68'} }, ();
    push @{ $repos{''}{'guser69'} }, ();
    push @{ $repos{''}{'guser7'} }, ();
    push @{ $repos{''}{'guser70'} }, ();
    push @{ $repos{''}{'guser71'} }, ();
    push @{ $repos{''}{'guser72'} }, ();
    push @{ $repos{''}{'guser73'} }, ();
    push @{ $repos{''}{'guser74'} }, ();
    push @{ $repos{''}{'guser75'} }, ();
    push @{ $repos{''}{'guser76'} }, ();
    push @{ $repos{''}{'guser77'} }, ();
    push @{ $repos{''}{'guser78'} }, ();
    push @{ $repos{''}{'guser79'} }, ();
    push @{ $repos{''}{'guser8'} }, ();
    push @{ $repos{''}{'guser80'} }, ();
    push @{ $repos{''}{'guser81'} }, ();
    push @{ $repos{''}{'guser82'} }, ();
    push @{ $repos{''}{'guser83'} }, ();
    push @{ $repos{''}{'guser84'} }, ();
    push @{ $repos{''}{'guser85'} }, ();
    push @{ $repos{''}{'guser9'} }, ();
    push @{ $repos{''}{'user1'} }, ();
    push @{ $repos{''}{'user10'} }, ();
    push @{ $repos{''}{'user11'} }, ();
    push @{ $repos{''}{'user12'} }, ();
    push @{ $repos{''}{'user13'} }, ();
    push @{ $repos{''}{'user14'} }, ();
    push @{ $repos{''}{'user15'} }, ();
    push @{ $repos{''}{'user16'} }, ();
    push @{ $repos{''}{'user2'} }, ();
    push @{ $repos{''}{'user3'} }, ();
    push @{ $repos{''}{'user4'} }, ();
    push @{ $repos{''}{'user5'} }, ();
    push @{ $repos{''}{'user6'} }, ();
    push @{ $repos{''}{'user7'} }, ();
    $repos{''}{R}{'user8'} = 1;
    $repos{''}{W}{'user8'} = 1;
    push @{ $repos{''}{'user8'} }, ();
    return %repos;
}
