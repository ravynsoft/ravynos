#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Config;

use Test::More;
use File::Temp qw[tempdir];

unless( eval { require Data::Dumper } ) {
    plan skip_all => 'Data::Dumper not available';
}

plan tests => 11;


use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

# 'make disttest' sets a bunch of environment variables which interfere
# with our testing.
delete @ENV{qw(PREFIX LIB MAKEFLAGS)};

my $Perl = which_perl();
my $Makefile = makefile_name();
my $Is_VMS = $^O eq 'VMS';

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

$| = 1;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir('Big-Dummy'), "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

unlink $Makefile;
my $prereq_out = run(qq{$Perl Makefile.PL "PREREQ_PRINT=1"});
ok( !-r $Makefile, "PREREQ_PRINT produces no $Makefile" );
is( $?, 0,         '  exited normally' );
$prereq_out =~ s/^'chcp' is not recognized.*batch file\.//s; # remove errors
{
    package _Prereq::Print;
    no strict;
    $PREREQ_PM = undef;  # shut up "used only once" warning.
    eval $prereq_out;
    ::is_deeply( $PREREQ_PM, { strict => 0 }, 'prereqs dumped' );
    ::is( $@, '',                             '  without error' );
}


$prereq_out = run(qq{$Perl Makefile.PL "PRINT_PREREQ=1"});
ok( !-r $Makefile, "PRINT_PREREQ produces no $Makefile" );
is( $?, 0,         '  exited normally' );
::like( $prereq_out, qr/^perl\(strict\) \s* >= \s* 0 \s*$/mx,
                                                      'prereqs dumped' );


# Currently a bug.
#my $prereq_out = run(qq{$Perl Makefile.PL "PREREQ_PRINT=0"});
#ok( -r $Makefile, "PREREQ_PRINT=0 produces a $Makefile" );
#is( $?, 0,         '  exited normally' );
#unlink $Makefile;

# Currently a bug.
#my $prereq_out = run(qq{$Perl Makefile.PL "PRINT_PREREQ=1"});
#ok( -r $Makefile, "PRINT_PREREQ=0 produces a $Makefile" );
#is( $?, 0,         '  exited normally' );
#unlink $Makefile;
