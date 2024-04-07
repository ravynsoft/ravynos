#!/usr/bin/perl

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use strict;
use warnings;
use Test::More;

BEGIN {
	if ($^O =~ /beos/i or $^O eq 'haiku') {
		plan tests => 4;
	} else {
		plan skip_all => 'This is not BeOS';
	}
}

use Config;
use File::Spec;
use File::Basename;

# tels - Taken from MM_Win32.t - I must not understand why this works, right?
# Does this mimic ExtUtils::MakeMaker ok?
{
    @MM::ISA = qw(
        ExtUtils::MM_Unix
        ExtUtils::Liblist::Kid
        ExtUtils::MakeMaker
    );
    # MM package faked up by messy MI entanglement
    package MM;
    sub DESTROY {}
}

require_ok( 'ExtUtils::MM_BeOS' );

my $MM = bless { NAME => "Foo" }, 'MM';

# init_linker
{
    my $libperl = File::Spec->catfile('$(PERL_INC)',
                                      $Config{libperl} || 'libperl.a' );
    my $export  = '';
    my $after   = '';
    $MM->init_linker;

    is( $MM->{PERL_ARCHIVE},        $libperl,   'PERL_ARCHIVE' );
    is( $MM->{PERL_ARCHIVE_AFTER},  $after,     'PERL_ARCHIVE_AFTER' );
    is( $MM->{EXPORT_LIST},         $export,    'EXPORT_LIST' );
}
