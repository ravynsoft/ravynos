#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  Copyright (c) 2017, Reini Urban
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

use strict;
use warnings;
use ExtUtils::MakeMaker 6.31;
use Config;

WriteMakefile(
    NAME                => 'Storable',
    AUTHOR              => 'Perl 5 Porters',
    LICENSE             => 'perl',
    DISTNAME            => "Storable",
    PREREQ_PM           =>
      {
          XSLoader => 0,
      },
    ( $ExtUtils::MakeMaker::VERSION >= 6.64 ?
        (
            CONFIGURE_REQUIRES => {
                'ExtUtils::MakeMaker' => '6.31',
            },
            BUILD_REQUIRES => {
                'ExtUtils::MakeMaker' => '6.31',
            },
            TEST_REQUIRES => {
                'Test::More' => '0.82',
            },
           )
        : () ),
    INSTALLDIRS => ($] >= 5.007 && $] < 5.012) ? 'perl' : 'site',
    VERSION_FROM    => 'Storable.pm',
    ABSTRACT_FROM   => 'Storable.pm',
    ($ExtUtils::MakeMaker::VERSION > 6.45 ?
     (META_MERGE        => { resources =>
                               { bugtracker => 'https://github.com/Perl/perl5/issues' },
                            provides    => {
                                'Storable'  => {
                                    file        => 'Storable.pm',
                                    version     => MM->parse_version('Storable.pm'),
                                },
                            },

                           },
    ) : ()),
    dist                => { SUFFIX => 'gz', COMPRESS => 'gzip -f' },
    clean               => { FILES => 'Storable-*' },
);

my $ivtype = $Config{ivtype};

# I don't know if the VMS folks ever supported long long on 5.6.x
if ($ivtype and $ivtype eq 'long long' and $^O !~ /^MSWin/) {
  print <<'EOM';

You appear to have a perl configured to use 64 bit integers in its scalar
variables.  If you have existing data written with an earlier version of
Storable which this version of Storable refuses to load with a

   Byte order is not compatible

error, then please read the section "64 bit data in perl 5.6.0 and 5.6.1"
in the Storable documentation for instructions on how to read your data.

(You can find the documentation at the end of Storable.pm in POD format)

EOM
}

package MY;

sub depend {
    "

release : dist
	git tag \$(VERSION)
	cpan-upload \$(DISTVNAME).tar\$(SUFFIX)
	git push
	git push --tags
"
}
