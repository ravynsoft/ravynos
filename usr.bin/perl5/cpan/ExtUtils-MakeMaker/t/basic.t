#!/usr/bin/perl -w

# This test puts MakeMaker through the paces of a basic perl module
# build, test and installation of the Big::Fat::Dummy module.

# Module::Install relies on being able to patch the generated Makefile
# to add flags to $(PERL)
# This test includes adding ' -Iinc' to $(PERL), and checking 'make install'
# after that works. Done here as back-compat is considered basic.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Config;
use ExtUtils::MakeMaker;
use utf8;

use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;
use Config;
use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : (tests => 188);
use File::Find;
use File::Spec;
use File::Path;
use File::Temp qw[tempdir];

my $perl = which_perl();
my $Is_VMS = $^O eq 'VMS';
my $OLD_CP; # crude but...
my $w32worked; # or whether we had to fallback to chcp
if ($^O eq "MSWin32") {
    eval {
        require Win32;
        local $SIG{__WARN__} = sub {} if ( "$]" < 5.014 ); # suppress deprecation warning for inherited AUTOLOAD of Win32::GetConsoleCP()
        $w32worked = $OLD_CP = Win32::GetConsoleCP();
    };
    $OLD_CP = $1 if !$w32worked and qx(chcp) =~ /(\d+)$/ and $? == 0;
    if (defined $OLD_CP) {
        if ($w32worked) {
            Win32::SetConsoleCP(1252)
        } else {
            qx(chcp 1252);
        }
    }
}
END {
    if ($^O eq "MSWin32" and defined $OLD_CP) {
        if ($w32worked) {
            Win32::SetConsoleCP($OLD_CP)
        } else {
            qx(chcp $OLD_CP);
        }
    }
}

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

my $Touch_Time = calibrate_mtime();

$| = 1;

ok( setup_recurs(), 'setup' );

ok( chdir('Big-Dummy'), "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

sub extrachar {
  return 's'
    if 1; # until Perl gains native support for Unicode filenames
#    if $] <= 5.008 || $ENV{PERL_CORE}
#      || $^O =~ /bsd|dragonfly|mswin32/i;
  'š';
}
my $DUMMYINST = '../dummy-in'.extrachar().'tall';
my @mpl_out = run(qq{$perl Makefile.PL "PREFIX=$DUMMYINST"});

cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) ||
  diag(@mpl_out);

my $makefile = makefile_name();
ok( grep(/^Writing $makefile for Big::Dummy/,
         @mpl_out) == 1,
                                           'Makefile.PL output looks right');

ok( grep(/^Current package is: main$/,
         @mpl_out) == 1,
                                           'Makefile.PL run in package main');

ok( -e $makefile,       'Makefile exists' );

# -M is flakey on VMS
my $mtime = (stat($makefile))[9];
cmp_ok( $Touch_Time, '<=', $mtime,  '  been touched' );

my $make = make_run();

{
    # Suppress 'make manifest' noise
    local $ENV{PERL_MM_MANIFEST_VERBOSE} = 0;
    my $manifest_out = run("$make manifest");
    ok( -e 'MANIFEST',      'make manifest created a MANIFEST' );
    ok( -s 'MANIFEST',      '  not empty' );
}

my $ppd_out = run("$make ppd");
is( $?, 0,                      '  exited normally' ) || diag $ppd_out;
ok( open(PPD, 'Big-Dummy.ppd'), '  .ppd file generated' );
my $ppd_html;
{ local $/; $ppd_html = <PPD> }
close PPD;
like( $ppd_html, qr{^<SOFTPKG NAME="Big-Dummy" VERSION="0.01">}m,
                                                           '  <SOFTPKG>' );
like( $ppd_html,
      qr{^\s*<ABSTRACT>Try "our" hot dog's, \$andwiche\$ and \$\(ub\)\$!</ABSTRACT>}m,
                                                           '  <ABSTRACT>');
like( $ppd_html,
      qr{^\s*<AUTHOR>Michael G Schwern &lt;schwern\@pobox.com&gt;</AUTHOR>}m,
                                                           '  <AUTHOR>'  );
like( $ppd_html, qr{^\s*<IMPLEMENTATION>}m,          '  <IMPLEMENTATION>');
like( $ppd_html, qr{^\s*<REQUIRE NAME="strict::" />}m,  '  <REQUIRE>' );
unlike( $ppd_html, qr{^\s*<REQUIRE NAME="warnings::" />}m,  'no <REQUIRE> for build_require' );

my $archname = $Config{archname};
# XXX This is a copy of the internal logic, so it's not a great test
if( "$]" >= 5.010) {
    $archname .= "-$^V->{version}->[0].$^V->{version}->[1]";
}
elsif( "$]" >= 5.008 ) {
    $archname .= "-$Config{PERL_REVISION}.$Config{PERL_VERSION}";
}
like( $ppd_html, qr{^\s*<ARCHITECTURE NAME="$archname" />}m,
                                                           '  <ARCHITECTURE>');
like( $ppd_html, qr{^\s*<CODEBASE HREF="" />}m,            '  <CODEBASE>');
like( $ppd_html, qr{^\s*</IMPLEMENTATION>}m,           '  </IMPLEMENTATION>');
like( $ppd_html, qr{^\s*</SOFTPKG>}m,                      '  </SOFTPKG>');

my $test_out = run("$make test");
like( $test_out, qr/All tests successful/, 'make test' );
is( $?, 0,                                 '  exited normally' ) ||
    diag $test_out;

# Test 'make test TEST_VERBOSE=1'
my $make_test_verbose = make_macro($make, 'test', TEST_VERBOSE => 1);
$test_out = run("$make_test_verbose");
like( $test_out, qr/ok \d+ - TEST_VERBOSE/, 'TEST_VERBOSE' );
like( $test_out, qr/ok \d+ - testing test.pl/, 'test.pl' ); # in test.pl
like( $test_out, qr/ok \d+ - testing t\/\*.t/, 't/*.t' ); # in *.t
like( $test_out, qr/All tests successful/,  '  successful' );
is( $?, 0,                                  '  exited normally' ) ||
    diag $test_out;

# Test 'make testdb TEST_FILE=t/compile.t'
# TESTDB_SW override is because perl -d is too clever for me to outwit
my $make_testdb_file = make_macro(
    $make,
    'testdb',
    TEST_FILE => 't/compile.t',
    TESTDB_SW => '-Ixyzzy',
);
$test_out = run($make_testdb_file);
unlike( $test_out, qr/harness/, 'no harness' );
unlike( $test_out, qr/sanity\.t/, 'no wrong test' );
like( $test_out, qr/compile\.t/, 'get right test' );
like( $test_out, qr/xyzzy/, 'signs of TESTDB_SW' );
is( $?, 0,                                  '  exited normally' ) ||
    diag $test_out;

# now simulate what Module::Install does, and edit $(PERL) to add flags
open my $fh, '<', $makefile;
my $mtext = join '', <$fh>;
close $fh;
$mtext =~ s/^(\s*PERL\s*=.*)$/$1 -Iinc/m;
open $fh, '>', $makefile;
print $fh $mtext;
close $fh;

my $install_out = run("$make install");
is( $?, 0, 'install' ) || diag $install_out;
like( $install_out, qr/^Installing /m );

sub check_dummy_inst {
    my ($loc, $skipsubdir) = @_;
    my %files = ();
    find( sub {
	# do it case-insensitive for non-case preserving OSs
	my $file = lc $_;
	# VMS likes to put dots on the end of things that don't have them.
	$file =~ s/\.$// if $Is_VMS;
	$files{$file} = $File::Find::name;
    }, $loc );
    ok( $files{'dummy.pm'},     '  Dummy.pm installed' );
    ok( $files{'liar.pm'},      '  Liar.pm installed'  ) unless $skipsubdir;
    ok( $files{'program'},      '  program installed'  );
    ok( $files{'.packlist'},    '  packlist created'   );
    ok( $files{'perllocal.pod'},'  perllocal.pod created' );
    \%files;
}

SKIP: {
    ok( -r $DUMMYINST,     '  install dir created' )
	or skip "$DUMMYINST doesn't exist", 5;
    check_dummy_inst($DUMMYINST);
}

SKIP: {
    skip 'VMS install targets do not preserve $(PREFIX)', 8 if $Is_VMS;

    $install_out = run("$make install PREFIX=elsewhere");
    is( $?, 0, 'install with PREFIX override' ) || diag $install_out;
    like( $install_out, qr/^Installing /m );

    ok( -r 'elsewhere',     '  install dir created' );
    check_dummy_inst('elsewhere');
    rmtree('elsewhere');
}


SKIP: {
    skip 'VMS install targets do not preserve $(DESTDIR)', 10 if $Is_VMS;

    $install_out = run("$make install PREFIX= DESTDIR=other");
    is( $?, 0, 'install with DESTDIR' ) ||
        diag $install_out;
    like( $install_out, qr/^Installing /m );

    ok( -d 'other',  '  destdir created' );
    my $files = check_dummy_inst('other');

    ok( open(PERLLOCAL, $files->{'perllocal.pod'} ) ) ||
        diag("Can't open $files->{'perllocal.pod'}: $!");
    { local $/;
      unlike(<PERLLOCAL>, qr/other/, 'DESTDIR should not appear in perllocal');
    }
    close PERLLOCAL;

# TODO not available in the min version of Test::Harness we require
#    ok( open(PACKLIST, $files{'.packlist'} ) ) ||
#        diag("Can't open $files{'.packlist'}: $!");
#    { local $/;
#      local $TODO = 'DESTDIR still in .packlist';
#      unlike(<PACKLIST>, qr/other/, 'DESTDIR should not appear in .packlist');
#    }
#    close PACKLIST;

    rmtree('other');
}


SKIP: {
    skip 'VMS install targets do not preserve $(PREFIX)', 9 if $Is_VMS;

    $install_out = run("$make install PREFIX=elsewhere DESTDIR=other/");
    is( $?, 0, 'install with PREFIX override and DESTDIR' ) ||
        diag $install_out;
    like( $install_out, qr/^Installing /m );

    ok( !-d 'elsewhere',       '  install dir not created' );
    ok( -d 'other/elsewhere',  '  destdir created' );
    check_dummy_inst('other/elsewhere');
    rmtree('other');
}

my ($dist_test_out, $distdir, $meta_yml, $mymeta_yml, $meta_json, $mymeta_json);
SKIP: {
    skip 'disttest depends on metafile, which is not run in core', 1 if $ENV{PERL_CORE};
    $dist_test_out = run("$make disttest");
    is( $?, 0, 'disttest' ) || diag($dist_test_out);

    # Test META.yml generation
    use ExtUtils::Manifest qw(maniread);

    $distdir  = 'Big-Dummy-0.01';
    $distdir =~ s/\./_/g if $Is_VMS;
    $meta_yml = "$distdir/META.yml";
    $mymeta_yml = "$distdir/MYMETA.yml";
    $meta_json = "$distdir/META.json";
    $mymeta_json = "$distdir/MYMETA.json";
}

note "META file validity"; SKIP: {
    skip 'disttest depends on metafile, which is not run in core', 104 if $ENV{PERL_CORE};

    eval { require CPAN::Meta; };
    skip 'Loading CPAN::Meta failed', 104 if $@;

    ok( !-f 'META.yml',  'META.yml not written to source dir' );
    ok( -f $meta_yml,    'META.yml written to dist dir' );
    ok( !-e "META_new.yml", 'temp META.yml file not left around' );

    ok( -f 'MYMETA.yml',  'MYMETA.yml is written to source dir' );
    ok( -f $mymeta_yml,    'MYMETA.yml is written to dist dir on disttest' );

    ok( !-f 'META.json',  'META.json not written to source dir' );
    ok( -f $meta_json,    'META.json written to dist dir' );
    ok( !-e "META_new.json", 'temp META.json file not left around' );

    ok( -f 'MYMETA.json',  'MYMETA.json is written to source dir' );
    ok( -f $mymeta_json,    'MYMETA.json is written to dist dir on disttest' );

    for my $case (
      ['META.yml', $meta_yml],
      ['MYMETA.yml', $mymeta_yml],
      ['META.json', $meta_json],
      ['MYMETA.json', $mymeta_json],
      ['MYMETA.yml', 'MYMETA.yml'],
      ['MYMETA.json', 'MYMETA.json'],
    ) {
      my ($label, $meta_name) = @$case;
      ok(
        my $obj = eval {
          CPAN::Meta->load_file($meta_name, {lazy_validation => 0})
        },
        "$label validates"
      );
      my $is = sub {
        my ($m,$e) = @_;
        is($obj->$m, $e, "$label -> $m")
      };
      my $is_list = sub {
        my ($m,$e) = @_;
        is_deeply([$obj->$m], $e, "$label -> $m")
      };
      my $is_map = sub {
        my ($m,$e) = @_;
        is_deeply($obj->$m, $e, "$label -> $m")
      };
      $is->( name => "Big-Dummy" );
      $is->( version => "0.01" );
      $is->( abstract => q{Try "our" hot dog's, $andwiche$ and $(ub)$!} );
      $is_list->( licenses => [q{unknown}] );
      $is_list->( authors => [ q{Michael G Schwern <schwern@pobox.com>} ] );
      $is_map->( prereqs => {
          configure => {
            requires => {
              'ExtUtils::MakeMaker' => 0
            },
          },
          build => {
            requires => {
              'warnings' => 0
            }
          },
          runtime => {
            requires => {
              'strict' => 0
            }
          },
        }
      );
      $is_map->(
        no_index => {
          directory => [qw/t inc/],
        }
      );
      $is->( dynamic_config => ($label =~ /MYMETA/) ? 0 : 1 );
    }

    my $manifest = maniread("$distdir/MANIFEST");
    # VMS is non-case preserving, so we can't know what the MANIFEST will
    # look like. :(
    _normalize($manifest);
    is( $manifest->{'meta.yml'}, 'Module YAML meta-data (added by MakeMaker)',
      "MANIFEST has META.yml"
    );
    is( $manifest->{'meta.json'}, 'Module JSON meta-data (added by MakeMaker)',
      "MANIFEST has META.json"
    );

    # Test NO_META META.yml suppression
    for my $f ( $meta_yml, $meta_json, 'MYMETA.yml', 'MYMETA.json' ) {
      1 while unlink $f;
    }
    ok( !-f $meta_yml,   'META.yml deleted' );
    ok( !-f 'MYMETA.yml','MYMETA.yml deleted' );
    ok( !-f $meta_json,   'META.json deleted' );
    ok( !-f 'MYMETA.json','MYMETA.json deleted' );

    @mpl_out = run(qq{$perl Makefile.PL "NO_META=1"});
    ok( -f 'MYMETA.yml', 'MYMETA.yml generation not suppressed by NO_META' );
    ok( -f 'MYMETA.json', 'MYMETA.json generation not suppressed by NO_META' );
    cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) || diag(@mpl_out);
    ok( !-f $meta_yml,   'META.yml generation suppressed by NO_META' );
    ok( !-f $meta_json,   'META.json generation suppressed by NO_META' );
    my $distdir_out = run("$make distdir");
    is( $?, 0, 'distdir' ) || diag($distdir_out);
    ok( !-f $meta_yml,   'META.yml generation suppressed by NO_META' );
    ok( !-f $meta_json,   'META.json generation suppressed by NO_META' );

    for my $f ( 'MYMETA.yml', 'MYMETA.json' ) {
      1 while unlink $f;
    }
    ok( !-f 'MYMETA.yml','MYMETA.yml deleted' );
    ok( !-f 'MYMETA.json','MYMETA.json deleted' );

    @mpl_out = run(qq{$perl Makefile.PL "NO_MYMETA=1"});
    cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) || diag(@mpl_out);
    $distdir_out = run("$make distdir");
    is( $?, 0, 'distdir' ) || diag($distdir_out);
    ok( !-f 'MYMETA.yml','MYMETA.yml generation suppressed by NO_MYMETA' );
    ok( !-f 'MYMETA.json','MYMETA.json generation suppressed by NO_MYMETA' );
    ok( -f $meta_yml,    'META.yml generation not suppressed by NO_MYMETA' );
    ok( -f $meta_json,    'META.json generation not suppressed by NO_MYMETA' );

    # Test MYMETA really comes from META except for prereqs
    for my $f ( $meta_yml, $meta_json, 'MYMETA.yml', 'MYMETA.json' ) {
      1 while unlink $f;
    }
    @mpl_out = run(qq{$perl Makefile.PL});
    cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) || diag(@mpl_out);
    $distdir_out = run("$make distdir");
    is( $?, 0, 'distdir' ) || diag($distdir_out);
    ok( -f $meta_yml,    'META.yml generated in distdir' );
    ok( -f $meta_json,    'META.json generated in distdir' );
    ok( ! -f $mymeta_yml,    'MYMETA.yml not yet generated in distdir' );
    ok( ! -f $mymeta_json,    'MYMETA.json generated in distdir' );
    my $edit_meta = CPAN::Meta->load_file($meta_json)->as_struct;
    $edit_meta->{abstract} = "New abstract";
    my $meta_obj = CPAN::Meta->new($edit_meta);
    is( $meta_obj->abstract, "New abstract", "MYMETA abstract from META, not Makefile.PL");
    ok( $meta_obj->save($meta_json), "Saved edited META.json in distdir" );
    ok( $meta_obj->save($meta_yml, {version => 1.4}), "Saved edited META.yml in distdir");
    ok( chdir $distdir );
    ok( -f 'META.yml',    'META.yml confirmed in distdir' );
    ok( -f 'META.json',    'META.json confirmed in distdir' );
    @mpl_out = run(qq{$perl Makefile.PL});
    cmp_ok( $?, '==', 0, 'Makefile.PL in distdir exited with zero' ) || diag(@mpl_out);
    ok( chdir File::Spec->updir );
    ok( -f $mymeta_yml,    'MYMETA.yml generated in distdir' );
    ok( -f $mymeta_json,    'MYMETA.json generated in distdir' );
    $meta_obj = CPAN::Meta->load_file($meta_json);
    is( $meta_obj->abstract, "New abstract", "META abstract is same as was saved");
    $meta_obj = CPAN::Meta->load_file($mymeta_json);
    is( $meta_obj->abstract, "New abstract", "MYMETA abstract from META, not Makefile.PL");
}


# Make sure init_dirscan doesn't go into the distdir
# also with a "messup.PL" that will make a build fail
open $fh, '>', 'messup.PL' or die "messup.PL: $!";
print $fh 'print "Extracting messup (with variable substitutions)\n";' . "\n";
print $fh 'die';
close $fh;
@mpl_out = run(qq{$perl Makefile.PL "PREFIX=$DUMMYINST"});

cmp_ok( $?, '==', 0, 'Makefile.PL exited with zero' ) || diag(@mpl_out);

ok( grep(/^Writing $makefile for Big::Dummy/, @mpl_out) == 1,
                                'init_dirscan skipped distdir') ||
  diag(@mpl_out);

# "make test" straight after "perl Makefile.PL" is expected to work same as
#   "make all test" so check that with "messup.PL" that will make the
#   build step fail
$test_out = run("$make test");
unlike( $test_out, qr/All tests successful/, 'make test caused build' );
isnt( $?, 0,                                 '  build should fail' ) ||
    diag $test_out;

# I know we'll get ignored errors from make here, that's ok.
# Send STDERR off to oblivion.
open(SAVERR, ">&STDERR") or die $!;
open(STDERR, ">",File::Spec->devnull) or die $!;

my $realclean_out = run("$make realclean");
is( $?, 0, 'realclean' ) || diag($realclean_out);
1 while unlink 'messup.PL'; # also zap deliberate build-breaker

open(STDERR, ">&SAVERR") or die $!;
close SAVERR;

# test linkext=>{LINKTYPE=>''} still installs a pure-perl installation
# warning, edits the Makefile.PL so either rewrite after this or do this last
my $file = 'Makefile.PL';
my $text = slurp $file;
ok(($text =~ s#\);#    linkext=>{LINKTYPE=>''},\n$&#), 'successful M.PL edit');
open $fh, '>', $file or die "$file: $!";
print $fh $text;
close $fh;
# now do with "Liar" subdir still there
rmtree $DUMMYINST; # so no false positive from before
@mpl_out = run(qq{$perl Makefile.PL "PREFIX=$DUMMYINST"});
$install_out = run("$make install");
check_dummy_inst($DUMMYINST);
# now clean, delete "Liar" subdir, do again
$realclean_out = run("$make realclean");
rmtree 'Liar';
rmtree $DUMMYINST; # so no false positive from before
@mpl_out = run(qq{$perl Makefile.PL "PREFIX=$DUMMYINST"});
$install_out = run("$make install");
check_dummy_inst($DUMMYINST, 1);

sub _normalize {
    my $hash = shift;

    %$hash= map { lc($_) => $hash->{$_} } keys %$hash;
}
