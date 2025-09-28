#!perl -w

use strict;
use warnings;

use Config qw(%Config);
use ExtUtils::MakeMaker;

my @extra;
push(@extra, INSTALLDIRS => 'perl') if $] >= 5.008 && $] < 5.012;

if ($^O eq 'VMS') {
    if (defined($Config{ccname})) {
        if (grep(/VMS_VAX/, @INC) && ($Config{ccname} eq 'DEC')) {
            # VAX compiler optimizer even as late as v6.4 gets stuck
            push(@extra, OPTIMIZE => "/Optimize=(NODISJOINT)");
        }
    }
}


WriteMakefile(
    'NAME'	   => 'Digest::MD5',
    'VERSION_FROM' => 'MD5.pm',
    'ABSTRACT'     => 'Perl interface to the MD-5 algorithm',
    'AUTHOR'       => 'Gisle Aas <gisle@activestate.com>',
    'LICENSE'      => 'perl',
    'MIN_PERL_VERSION' => 5.006,
    'PREREQ_PM'    => {
			'Digest::base' => '1.00',
			'XSLoader' => 0,
		      },
    'META_MERGE'   => {
        resources  => {
            license    => 'http://dev.perl.org/licenses/',
            bugtracker => 'https://github.com/Dual-Life/digest-md5/issues',
            repository => 'https://github.com/dual-Life/digest-md5/',
        }
    },
    @extra,
);



BEGIN {
    # compatibility with older versions of MakeMaker
    my $developer = -d ".git";
    my %mm_req = (
        LICENCE => 6.31,
        META_MERGE => 6.45,
        META_ADD => 6.45,
        MIN_PERL_VERSION => 6.48,
    );
    undef(*WriteMakefile);
    *WriteMakefile = sub {
        my %arg = @_;
        for (keys %mm_req) {
            unless (eval { ExtUtils::MakeMaker->VERSION($mm_req{$_}) }) {
                warn "$_ $@" if $developer;
                delete $arg{$_};
            }
        }
        ExtUtils::MakeMaker::WriteMakefile(%arg);
    };
}
