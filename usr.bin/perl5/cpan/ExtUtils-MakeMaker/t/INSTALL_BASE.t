#!/usr/bin/perl -w

# Tests INSTALL_BASE to a directory without AND with a space in the name

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use File::Path;
use Config;
my @INSTDIRS = ('../dummy-install', '../dummy  install');
my $CLEANUP = 1;
$CLEANUP &&= 1; # so always 1 or numerically 0

use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;
use Config;
use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : ();
plan tests => 4 + $CLEANUP + @INSTDIRS * (15 + $CLEANUP);

my $Is_VMS = $^O eq 'VMS';

my $perl = which_perl();

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => $CLEANUP );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

my $SPACEDIR = 'space dir';
ok( setup_recurs($SPACEDIR), 'setup' );
END {
    ok( chdir File::Spec->updir, 'chdir updir' );
    ok( chdir File::Spec->updir, 'chdir updir again' );
    ok( teardown_recurs(), 'teardown' ) if $CLEANUP;
    map { rmtree $_ } @INSTDIRS if $CLEANUP;
}

ok( chdir(File::Spec->catdir($SPACEDIR, 'Big-Dummy')), "chdir'd to Big-Dummy") || diag("chdir failed; $!");

for my $instdir (@INSTDIRS) {
  $instdir = File::Spec->rel2abs($instdir);
  $instdir = VMS::Filespec::unixpath($instdir) if $Is_VMS;
  my @mpl_out = run(qq{$perl Makefile.PL "INSTALL_BASE=$instdir"});

  cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) ||
    diag(@mpl_out);

  my $makefile = makefile_name();
  ok( grep(/^Writing $makefile for Big::Dummy/,
	   @mpl_out) == 1,
					     'Makefile.PL output looks right');

  my $make = make_run();
  run("$make");   # this is necessary due to a dmake bug.
  my $install_out = run("$make install");
  is( $?, 0, '  make install exited normally' ) || diag $install_out;
  like( $install_out, qr/^Installing /m, '"Installing" in output' );

  ok( -r $instdir,      '  install dir created' );

  my @installed_files =
    ("$instdir/lib/perl5/Big/Dummy.pm",
     "$instdir/lib/perl5/Big/Liar.pm",
     "$instdir/bin/program",
     "$instdir/lib/perl5/$Config{archname}/perllocal.pod",
     "$instdir/lib/perl5/$Config{archname}/auto/Big/Dummy/.packlist"
    );

  foreach my $file (@installed_files) {
      ok( -e $file, "  $file installed" );
      ok( -r $file, "  $file readable" );
  }


  # nmake outputs its damned logo
  # Send STDERR off to oblivion.
  open(SAVERR, ">&STDERR") or die $!;
  open(STDERR, ">".File::Spec->devnull) or die $!;

  if ($CLEANUP) {
      my $realclean_out = run("$make realclean");
      is( $?, 0, 'realclean' ) || diag($realclean_out);
  }

  open(STDERR, ">&SAVERR") or die $!;
  close SAVERR;
}
