#!/usr/bin/perl

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use strict;
use warnings;
use Test::More;

BEGIN {
	if ($^O =~ /cygwin/i) {
		plan tests => 14;
	} else {
		plan skip_all => "This is not cygwin";
	}
}

use Config;
use File::Spec;
use ExtUtils::MM;
use Config;

use_ok( 'ExtUtils::MM_Cygwin' );

# test canonpath
my $path = File::Spec->canonpath('/a/../../c');
is( MM->canonpath('/a/../../c'), $path,
	'canonpath() method should work just like the one in File::Spec' );

# test cflags, with the fake package below
my $MM = bless({
	CFLAGS	=> 'fakeflags',
	CCFLAGS	=> '',
}, 'MM');

# with CFLAGS set, it should be returned
is( $MM->cflags(), 'fakeflags',
	'cflags() should return CFLAGS member data, if set' );

delete $MM->{CFLAGS};

# ExtUtils::MM_Cygwin::cflags() calls this, fake the output
{
    local $SIG{__WARN__} = sub {
        warn @_ unless $_[0] =~ /^Subroutine .* redefined/;
    };
    *ExtUtils::MM_Unix::cflags = sub { return $_[1] };
}

# respects the config setting, should ignore whitespace around equal sign
my $ccflags = $Config{useshrplib} eq 'true' ? ' -DUSEIMPORTLIB' : '';
{
    local $MM->{NEEDS_LINKING} = 1;
    $MM->cflags(<<FLAGS);
OPTIMIZE = opt
PERLTYPE  =pt
FLAGS
}

like( $MM->{CFLAGS}, qr/OPTIMIZE = opt/, '... should set OPTIMIZE' );
like( $MM->{CFLAGS}, qr/PERLTYPE = pt/, '... should set PERLTYPE' );
like( $MM->{CFLAGS}, qr/CCFLAGS = $ccflags/, '... should set CCFLAGS' );

# test manifypods
$MM = bless({
	NOECHO => 'noecho',
	MAN3PODS => {},
	MAN1PODS => {},
    MAKEFILE => 'Makefile',
}, 'MM');
unlike( $MM->manifypods(), qr/foo/,
	'manifypods() should return without PODS values set' );

$MM->{MAN3PODS} = { foo => 'foo.1' };
my $res = $MM->manifypods();
like( $res, qr/manifypods.*foo.*foo.1/s, '... should add MAN3PODS targets' );


# init_linker
{
    my $libperl = $Config{libperl} || 'libperl.a';
    $libperl =~ s/\.a/.dll.a/ if "$]" >= 5.006002;
    $libperl = "\$(PERL_INC)/$libperl";

    my $export  = '';
    my $after   = '';
    $MM->init_linker;

    is( $MM->{PERL_ARCHIVE},        $libperl,   'PERL_ARCHIVE' );
    is( $MM->{PERL_ARCHIVE_AFTER},  $after,     'PERL_ARCHIVE_AFTER' );
    is( $MM->{EXPORT_LIST},         $export,    'EXPORT_LIST' );
}

# Tests for correct handling of maybe_command in /cygdrive/*
# and c:/*.  $ENV{COMSPEC}, if it exists, should always be executable.
SKIP: {
    skip "Needs Cygwin::win_to_posix_path()", 2 unless defined &Cygwin::win_to_posix_path;

    SKIP: {
        my $comspec = $ENV{COMSPEC};
        skip(q[$ENV{COMSPEC} does not exist], 1) unless $comspec;

        $comspec = Cygwin::win_to_posix_path($comspec);

        ok(MM->maybe_command($comspec), qq{'$comspec' should be executable"});
    }

    # 'C:/' should *never* be executable, it's a directory.
    {
        my $cdrive = Cygwin::win_to_posix_path("C:/");

        ok(!MM->maybe_command($cdrive), qq{'$cdrive' should never be executable});
    }
}

# Our copy of Perl (with a unix-path) should always be executable.
SKIP: {
  skip "The Perl may not be installed yet when in core", 1 if $ENV{PERL_CORE};
  ok(MM->maybe_command($Config{perlpath}), qq{'$Config{perlpath}' should be executable});
}

package FakeOut;

sub TIEHANDLE {
	bless(\(my $scalar), $_[0]);
}

sub PRINT {
	my $self = shift;
	$$self .= shift;
}
