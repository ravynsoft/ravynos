use strict;
use warnings;

use 5.006;
use ExtUtils::MakeMaker;

my %WriteMakefile_args = (
    'NAME'             => 'FindBin',
    'VERSION_FROM'     => 'lib/FindBin.pm',
    'ABSTRACT_FROM'    => 'lib/FindBin.pm',
    'LICENSE'          => 'perl_5',
    'INSTALLDIRS'      => ($] >= '5.011' ? 'site' : 'perl'),
    'MIN_PERL_VERSION' => '5.006',
    'PREREQ_PM'        => {
        'Exporter'       => '0',
        'Carp'           => '0',
        'Cwd'            => '0',
        'File::Basename' => '0',
        'File::Spec'     => '0',
    },
    'dist'       => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
    'META_MERGE' => {
        'meta-spec'      => { 'version' => 2 },
        'dynamic_config' => 0,
        'resources'      => {
            'license'    => ['http://dev.perl.org/licenses/'],
            'bugtracker' => { 'web' => 'https://github.com/perl/perl5/issues' },
            'repository' => {
                'type' => 'git',
                'url'  => 'https://github.com/perl/perl5.git',
                'web'  => 'https://github.com/perl/perl5',
            },
        }
    },
);

delete $WriteMakefile_args{LICENSE}          unless eval { ExtUtils::MakeMaker->VERSION('6.31') };
delete $WriteMakefile_args{META_MERGE}       unless eval { ExtUtils::MakeMaker->VERSION('6.46') };
delete $WriteMakefile_args{MIN_PERL_VERSION} unless eval { ExtUtils::MakeMaker->VERSION('6.48') };

WriteMakefile(%WriteMakefile_args);
