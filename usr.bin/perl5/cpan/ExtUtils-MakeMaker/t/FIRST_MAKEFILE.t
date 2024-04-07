#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 7;

use MakeMaker::Test::Setup::BFD;
use MakeMaker::Test::Utils;

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

my $perl = which_perl();
my $make = make_run();

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir, 'chdir updir' );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir('Big-Dummy'), "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

my @mpl_out = run(qq{$perl Makefile.PL FIRST_MAKEFILE=jakefile});
cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) || diag @mpl_out;

ok( -e 'jakefile', 'FIRST_MAKEFILE honored' );

ok( grep(/^Writing jakefile(?:\.)? for Big::Dummy/, @mpl_out) == 1,
    'Makefile.PL output looks right' );
