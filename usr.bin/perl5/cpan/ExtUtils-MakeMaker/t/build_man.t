#!/usr/bin/perl -w

# Test if MakeMaker declines to build man pages under the right conditions.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 50;

use File::Spec;
use File::Temp qw[tempdir];
use TieOut;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

use ExtUtils::MakeMaker;
use ExtUtils::MakeMaker::Config;

# Simulate an installation which has man page generation turned off to
# ensure these tests will still work.
$Config{installman3dir} = 'none';

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

ok( setup_recurs(), 'setup' );
END {
    ok chdir File::Spec->updir, 'chdir updir';
    ok teardown_recurs(), 'teardown';
}

ok( chdir 'Big-Dummy', "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");
my $README = 'README.pod';
{ open my $fh, '>', $README or die "$README: $!"; }

ok((my $stdout = tie *STDOUT, 'TieOut'), 'tie stdout');

{
    local $Config{installman3dir} = File::Spec->catdir(qw(t lib));
    my $mm;
    {
        # suppress noisy & unnecessary "WARNING: Older versions of ExtUtils::MakeMaker may errantly install README.pod..."
        my @warnings = ();
        local $SIG{__WARN__} = sub { push @warnings, shift; };
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
        );
        # verify that suppressed warnings are present
        isnt (scalar(@warnings), 0);
        if (scalar(@warnings)) {
            note (sprintf('suppressed warnings: [ "%s" ]', do { my $s = join(q/" , "/, @warnings); $s =~ s/([^[:print:]])/sprintf('\x{%x}', ord($1))/egmsx; $s; }));
        }
    }
    my %got = %{ $mm->{MAN3PODS} };
    # because value too OS-specific
    my $delete_key = $^O eq 'VMS' ? '[.lib.Big]Dummy.pm' : 'lib/Big/Dummy.pm';
    ok delete($got{$delete_key}), 'normal man3pod';
    is_deeply \%got, {}, 'no extra man3pod';
}

{
    my $mm;
    {
        # suppress noisy & unnecessary "WARNING: Older versions of ExtUtils::MakeMaker may errantly install README.pod..."
        my @warnings = ();
        local $SIG{__WARN__} = sub { push @warnings, shift; };
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            INSTALLMAN3DIR  => 'none'
        );
        # verify that suppressed warnings are present
        isnt (scalar(@warnings), 0);
        if (scalar(@warnings)) {
            note (sprintf('suppressed warnings: [ "%s" ]', do { my $s = join(q/" , "/, @warnings); $s =~ s/([^[:print:]])/sprintf('\x{%x}', ord($1))/egmsx; $s; }));
        }
    }
    is_deeply $mm->{MAN3PODS}, {}, 'suppress man3pod with "none"';
}

{
    my $mm;
    {
        # suppress noisy & unnecessary "WARNING: Older versions of ExtUtils::MakeMaker may errantly install README.pod..."
        my @warnings = ();
        local $SIG{__WARN__} = sub { push @warnings, shift; };
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            MAN3PODS        => {}
        );
        # verify that suppressed warnings are present
        isnt (scalar(@warnings), 0);
        if (scalar(@warnings)) {
            note (sprintf('suppressed warnings: [ "%s" ]', do { my $s = join(q/" , "/, @warnings); $s =~ s/([^[:print:]])/sprintf('\x{%x}', ord($1))/egmsx; $s; }));
        }
    }
    is_deeply $mm->{MAN3PODS}, {}, 'suppress man3pod with {}';
}

{
    my $mm;
    {
        # suppress noisy & unnecessary "WARNING: Older versions of ExtUtils::MakeMaker may errantly install README.pod..."
        my @warnings = ();
        local $SIG{__WARN__} = sub { push @warnings, shift; };
        $mm = WriteMakefile(
            NAME            => 'Big::Dummy',
            VERSION_FROM    => 'lib/Big/Dummy.pm',
            MAN3PODS        => { "Foo.pm" => "Foo.1" }
        );
        # verify that suppressed warnings are present
        isnt (scalar(@warnings), 0);
        if (scalar(@warnings)) {
            note (sprintf('suppressed warnings: [ "%s" ]', do { my $s = join(q/" , "/, @warnings); $s =~ s/([^[:print:]])/sprintf('\x{%x}', ord($1))/egmsx; $s; }));
        }
    }
    is_deeply $mm->{MAN3PODS}, { "Foo.pm" => "Foo.1" }, 'override man3pod';
}

unlink $README;

# Check that we find the manage section from the directory
{
    local $Config{installman1dir}       = '';
    local $Config{installman3dir}       = '';
    local $Config{installsiteman1dir}   = '';
    local $Config{installsiteman3dir}   = '';
    local $Config{installvendorman1dir} = '';
    local $Config{installvendorman3dir} = '';
    local $Config{usevendorprefix}      = '';
    local $Config{vendorprefixexp}      = '';

    my $INSTALLDIRS = 'site';

    my $sections_ok = sub {
        my ( $man1section, $man3section, $m ) = @_;
        local $Test::Builder::Level = $Test::Builder::Level + 1;

        my $stdout = tie *STDOUT, 'TieOut' or die;
        my $mm     = WriteMakefile(
            NAME         => 'Big::Dummy',
            VERSION_FROM => 'lib/Big/Dummy.pm',
            INSTALLDIRS  => $INSTALLDIRS,
        );

        is( $mm->{MAN1SECTION}, $man1section,
            "$m man1section is $man1section" );
        is( $mm->{MAN3SECTION}, $man3section,
            "$m man3section is $man3section" );
    };

    # Correctly detect known man sections
    foreach my $s ( '{num}', '{num}p', '{num}pm', qw< l n o C L >, "L{num}", )
    {
        ( my $man1section = $s ) =~ s/\{num\}/1/;
        ( my $man3section = $s ) =~ s/\{num\}/3/;

        $Config{installman1dir}
            = File::Spec->catdir( 'foo', "man$man1section" );
        $Config{installman3dir}
            = File::Spec->catdir( 'foo', "man$man3section" );

        $sections_ok->( $man1section, $man3section, "From main [$s]" );
    }

    # Ignore unknown man sections
    foreach my $s ( '', qw< 2 2p 33 >, "C{num}" ) {
        ( my $man1section = $s ) =~ s/\{num\}/1/;
        ( my $man3section = $s ) =~ s/\{num\}/3/;

        $Config{installman1dir}
            = File::Spec->catdir( 'foo', "man$man1section" );
        $Config{installman3dir}
            = File::Spec->catdir( 'foo', "man$man3section" );

        $sections_ok->( 1, 3, "Ignore unrecognized [$s]" );
    }

    # Look in the right installman?dir based on INSTALLDIRS
    {
        $Config{installman1dir}     = File::Spec->catdir( 'foo', 'cat1p' );
        $Config{installman3dir}     = File::Spec->catdir( 'foo', 'cat3p' );
        $Config{installsiteman1dir} = File::Spec->catdir( 'foo', 'catL' );
        $Config{installsiteman3dir} = File::Spec->catdir( 'foo', 'catL3' );

        $sections_ok->( 'L', 'L3', "From site" );

        my $installwas = $INSTALLDIRS;
        $INSTALLDIRS = 'perl';
        $sections_ok->( '1p', '3p', "From main" );
        $INSTALLDIRS = $installwas;

    }

    # Set MAN?SECTION in Makefile
    {
        $Config{installman1dir} = File::Spec->catdir( 'foo', 'man1pm' );
        $Config{installman3dir} = File::Spec->catdir( 'foo', 'man3pm' );
        $Config{installsiteman1dir} = '';
        $Config{installsiteman3dir} = '';

        my $stdout = tie *STDOUT, 'TieOut' or die;
        my $mm     = WriteMakefile(
            NAME         => 'Big::Dummy',
            VERSION_FROM => 'lib/Big/Dummy.pm',
            MAN1PODS     => { foo => 'foo.1' },
            INSTALLDIRS  => $INSTALLDIRS,
        );

        my $makefile = slurp($mm->{MAKEFILE});

        like $makefile, qr/\QMAN1SECTION = 1pm\E/xms, "Set MAN1SECTION";
        like $makefile, qr/\QMAN3SECTION = 3pm\E/xms, "Set MAN3SECTION";

        like $makefile, qr/\Q$(POD2MAN) --section=$(MAN1SECTION) \E/,
            "Set POD2MAN section to \$(MAN1SECTION)";
        like $makefile, qr/\Q$(POD2MAN) --section=$(MAN3SECTION) \E/,
            "Set POD2MAN section to \$(MAN3SECTION)";
    }
}
