use strict;
use warnings;

use ExtUtils::MakeMaker;

my $clean = {};

my $mm_ver = ExtUtils::MakeMaker->VERSION;

if (-f "Normalize.xs") {
    print "Making header files for XS...\n";

    do './mkheader' or die $@ || "mkheader: $!";

    $clean = { FILES => 'unfcan.h unfcmb.h unfcmp.h unfcpt.h unfexc.h' };
}

WriteMakefile(
    ($mm_ver < 6.58)
    ? ('AUTHOR' => 'SADAHIRO Tomoyuki <SADAHIRO@cpan.org>, Karl Williamson <khw@cpan.org>')
    : ('AUTHOR' =>         [
                           'SADAHIRO Tomoyuki <SADAHIRO@cpan.org>',
                           'Karl Williamson <khw@cpan.org>',
                           ]),
    'ABSTRACT'          => 'Unicode Normalization Forms',
    'INSTALLDIRS'       => ($] >= 5.007002 && $] < 5.011) ? 'perl' : 'site',
                            # see perl5110delta, @INC reorganization
    'LICENSE'           => 'perl',
    'NAME'              => 'Unicode::Normalize',
    'VERSION_FROM'      => 'Normalize.pm', # finds $VERSION
    'clean'             => $clean,
    'depend'            => { 'Normalize.o' => '$(H_FILES)' },
    'PREREQ_PM'         => {
        'Carp'          => 0,
        'constant'      => 0,
        'DynaLoader'    => 0,
        'Exporter'      => 0,
        'File::Spec'    => 0,
        'strict'        => 0,
        'warnings'      => 0,
        'SelectSaver'   => 0,
    },
    ($mm_ver < 6.48 ? () : MIN_PERL_VERSION => 5.6.0),
    ($mm_ver < 6.46 ? () : (META_MERGE => {
        'meta-spec' => { version => 2 },
        resources       => {
            repository    => {
                url  => 'https://github.com/khwilliamson/Unicode-Normalize.git',
                web  => 'https://github.com/khwilliamson/Unicode-Normalize',
                type => 'git',
            },
            bugtracker    => {
                web  => 'https://rt.cpan.org/Public/Dist/Display.html?Name=Unicode-Normalize',
            },
        },
    })),
);
