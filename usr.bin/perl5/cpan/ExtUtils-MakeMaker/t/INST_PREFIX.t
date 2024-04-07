#!/usr/bin/perl -w

# Wherein we ensure the INST_* and INSTALL* variables are set correctly
# when various PREFIX variables are set.
#
# Essentially, this test is a Makefile.PL.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 52;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;
use ExtUtils::MakeMaker;
use File::Spec;
use TieOut;
use ExtUtils::MakeMaker::Config;

my $Is_VMS = $^O eq 'VMS';

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

$| = 1;

my $Makefile = makefile_name;
my $Curdir = File::Spec->curdir;
my $Updir  = File::Spec->updir;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir 'Big-Dummy', "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

my $stdout = tie *STDOUT, 'TieOut' or die;

my $mm = WriteMakefile(
    NAME          => 'Big::Dummy',
    VERSION_FROM  => 'lib/Big/Dummy.pm',
    PREREQ_PM     => {},
    PERL_CORE     => $ENV{PERL_CORE},
);

like( $stdout->read, qr{
                        (?:Generating\ a\ \w+?-style\ $Makefile\n)?
                        (?:Writing\ $Makefile\ for\ Big::Liar\n)?
                        (?:Writing\ MYMETA.yml\ and\ MYMETA.json\n)?
                        Big::Liar's\ vars\n
                        INST_LIB\ =\ \S+\n
                        INST_ARCHLIB\ =\ \S+\n
                        Generating\ a\ \w+?-style\ $Makefile\n
                        Writing\ $Makefile\ for\ Big::Dummy\n
                        (?:Writing\ MYMETA.yml\ and\ MYMETA.json\n)?
}x );

is( $mm->{PREFIX}, '$(SITEPREFIX)', 'PREFIX set based on INSTALLDIRS' );

isa_ok( $mm, 'ExtUtils::MakeMaker' );

is( $mm->{NAME}, 'Big::Dummy',  'NAME' );
is( $mm->{VERSION}, 0.01,            'VERSION' );

foreach my $prefix (qw(PREFIX PERLPREFIX SITEPREFIX VENDORPREFIX)) {
    unlike( $mm->{$prefix}, qr/\$\(PREFIX\)/ );
}


my $PREFIX = File::Spec->catdir('foo', 'bar');
$mm = WriteMakefile(
    NAME          => 'Big::Dummy',
    VERSION_FROM  => 'lib/Big/Dummy.pm',
    PREREQ_PM     => {},
    PERL_CORE     => $ENV{PERL_CORE},
    PREFIX        => $PREFIX,
);
like( $stdout->read, qr{
                        (?:Generating\ a\ \w+?-style\ $Makefile\n)?
                        (?:Writing\ $Makefile\ for\ Big::Liar\n)?
                        (?:Writing\ MYMETA.yml\ and\ MYMETA.json\n)?
                        Big::Liar's\ vars\n
                        INST_LIB\ =\ \S+\n
                        INST_ARCHLIB\ =\ \S+\n
                        Generating\ a\ \w+?-style\ $Makefile\n
                        Writing\ $Makefile\ for\ Big::Dummy\n
                        (?:Writing\ MYMETA.yml\ and\ MYMETA.json\n)?
}x );
undef $stdout;
untie *STDOUT;

is( $mm->{PREFIX}, $PREFIX,   'PREFIX' );

foreach my $prefix (qw(PERLPREFIX SITEPREFIX VENDORPREFIX)) {
    is( $mm->{$prefix}, '$(PREFIX)', "\$(PREFIX) overrides $prefix" );
}

is( !!$mm->{PERL_CORE}, !!$ENV{PERL_CORE}, 'PERL_CORE' );

my($perl_src, $mm_perl_src);
if( $ENV{PERL_CORE} ) {
    $perl_src = File::Spec->catdir($Updir, $Updir, $Updir, $Updir, $Updir, $Updir);
    $perl_src = File::Spec->canonpath($perl_src);
    $mm_perl_src = File::Spec->canonpath($mm->{PERL_SRC});
}
else {
    $mm_perl_src = $mm->{PERL_SRC};
}

is( $mm_perl_src, $perl_src,     'PERL_SRC' );


# Every INSTALL* variable must start with some PREFIX.
my %Install_Vars = (
 PERL   => [qw(archlib    privlib   bin       man1dir       man3dir   script)],
 SITE   => [qw(sitearch   sitelib   sitebin   siteman1dir   siteman3dir)],
 VENDOR => [qw(vendorarch vendorlib vendorbin vendorman1dir vendorman3dir)]
);

while( my($type, $vars) = each %Install_Vars) {
    SKIP: {
        skip "VMS must expand macros in INSTALL* vars", scalar @$vars
          if $Is_VMS;
        skip '$Config{usevendorprefix} not set', scalar @$vars
          if $type eq 'VENDOR' and !$Config{usevendorprefix};

        foreach my $var (@$vars) {
            my $installvar = "install$var";
            my $prefix = '$('.$type.'PREFIX)';

            SKIP: {
                skip uc($installvar).' set to another INSTALL variable', 1
                  if $mm->{uc $installvar} =~ /^\$\(INSTALL.*\)$/;

                # support for man page skipping
                $prefix = 'none' if $type eq 'PERL' &&
                                    $var =~ /man/ &&
                                    !$Config{$installvar};
                like( $mm->{uc $installvar}, qr/^\Q$prefix\E/,
                      "$prefix + $var" );
            }
        }
    }
}

# Check that when installman*dir isn't set in Config no man pages
# are generated.
{
    _set_config(installman1dir => '');
    _set_config(installman3dir => '');

    my $wibble = File::Spec->catdir(qw(wibble and such));
    my $stdout = tie *STDOUT, 'TieOut' or die;
    my $mm = WriteMakefile(
                           NAME          => 'Big::Dummy',
                           VERSION_FROM  => 'lib/Big/Dummy.pm',
                           PREREQ_PM     => {},
                           PERL_CORE     => $ENV{PERL_CORE},
                           PREFIX        => $PREFIX,
                           INSTALLMAN1DIR=> $wibble,
                          );

    is( $mm->{INSTALLMAN1DIR}, $wibble );
    is( $mm->{INSTALLMAN3DIR}, 'none'  );
}

# Check that when installvendorman*dir is set in Config it is honored
# [rt.cpan.org 2949]
{
    _set_config(installvendorman1dir => File::Spec->catdir('foo','bar') );
    _set_config(installvendorman3dir => '' );
    _set_config(usevendorprefix => 1 );
    _set_config(vendorprefixexp => 'something' );

    my $stdout = tie *STDOUT, 'TieOut' or die;
    my $mm = WriteMakefile(
                   NAME          => 'Big::Dummy',
                   VERSION_FROM  => 'lib/Big/Dummy.pm',
                   PREREQ_PM     => {},
                   PERL_CORE     => $ENV{PERL_CORE},

                   # In case the local installation doesn't have man pages.
                   INSTALLMAN1DIR=> 'foo/bar/baz',
                   INSTALLMAN3DIR=> 'foo/bar/baz',
                  );

    is( $mm->{INSTALLVENDORMAN1DIR}, File::Spec->catdir('foo','bar'),
                      'installvendorman1dir (in %Config) not modified' );
    isnt( $mm->{INSTALLVENDORMAN3DIR}, '',
                      'installvendorman3dir (not in %Config) set'  );
}

# Check that when installsiteman*dir isn't set in Config it falls back
# to installman*dir
{
    _set_config(installman1dir => File::Spec->catdir('foo', 'bar') );
    _set_config(installman3dir => File::Spec->catdir('foo', 'baz') );
    _set_config(installsiteman1dir => '' );
    _set_config(installsiteman3dir => '' );
    _set_config(installvendorman1dir => '' );
    _set_config(installvendorman3dir => '' );
    _set_config(usevendorprefix => 'define' );
    _set_config(vendorprefixexp => 'something' );

    my $wibble = File::Spec->catdir(qw(wibble and such));
    my $stdout = tie *STDOUT, 'TieOut' or die;
    my $mm = WriteMakefile(
                           NAME          => 'Big::Dummy',
                           VERSION_FROM  => 'lib/Big/Dummy.pm',
                           PERL_CORE     => $ENV{PERL_CORE},
                          );

    is( $mm->{INSTALLMAN1DIR}, File::Spec->catdir('foo', 'bar') );
    is( $mm->{INSTALLMAN3DIR}, File::Spec->catdir('foo', 'baz') );
    SKIP: {
        skip "VMS must expand macros in INSTALL* vars", 4 if $Is_VMS;

        is( $mm->{INSTALLSITEMAN1DIR},   '$(INSTALLMAN1DIR)' );
        is( $mm->{INSTALLSITEMAN3DIR},   '$(INSTALLMAN3DIR)' );
        is( $mm->{INSTALLVENDORMAN1DIR}, '$(INSTALLMAN1DIR)' );
        is( $mm->{INSTALLVENDORMAN3DIR}, '$(INSTALLMAN3DIR)' );
    }
}


# Check that when usevendoprefix and installvendorman*dir aren't set in
# Config it leaves them unset.
{
    _set_config(installman1dir => File::Spec->catdir('foo', 'bar') );
    _set_config(installman3dir => File::Spec->catdir('foo', 'baz') );
    _set_config(installsiteman1dir => '' );
    _set_config(installsiteman3dir => '' );
    _set_config(installvendorman1dir => '' );
    _set_config(installvendorman3dir => '' );
    _set_config(usevendorprefix => '' );
    _set_config(vendorprefixexp => '' );

    my $wibble = File::Spec->catdir(qw(wibble and such));
    my $stdout = tie *STDOUT, 'TieOut' or die;
    my $mm = WriteMakefile(
                           NAME          => 'Big::Dummy',
                           VERSION_FROM  => 'lib/Big/Dummy.pm',
                           PERL_CORE     => $ENV{PERL_CORE},
                          );

    is( $mm->{INSTALLMAN1DIR}, File::Spec->catdir('foo', 'bar') );
    is( $mm->{INSTALLMAN3DIR}, File::Spec->catdir('foo', 'baz') );
    SKIP: {
        skip "VMS must expand macros in INSTALL* vars", 2 if $Is_VMS;
        is( $mm->{INSTALLSITEMAN1DIR},   '$(INSTALLMAN1DIR)' );
        is( $mm->{INSTALLSITEMAN3DIR},   '$(INSTALLMAN3DIR)' );
    }
    is( $mm->{INSTALLVENDORMAN1DIR}, '' );
    is( $mm->{INSTALLVENDORMAN3DIR}, '' );
}


sub _set_config {
    my($k,$v) = @_;
    (my $k_no_install = $k) =~ s/^install//i;
    $Config{$k} = $v;

    # Because VMS's config has traditionally been underpopulated, it will
    # fall back to the install-less versions in desperation.
    $Config{$k_no_install} = $v if $Is_VMS;
    return;
}
