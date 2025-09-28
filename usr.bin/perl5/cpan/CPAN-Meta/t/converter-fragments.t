use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta::Converter;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my $spec2 = {
    version => '2',
    url => 'http://search.cpan.org/perldoc?CPAN::Meta::Spec',
};

my @cases = (
    #<<< No perltidy
    {
        label  => "v1.4 requires -> v2 prereqs",
        from   => "1.4",
        to     => "2",
        input  => {
            requires => {
                'File::Spec' => "0.80",
            },
        },
        expect => {
            'meta-spec' => $spec2,
            prereqs => {
                runtime => {
                    requires => {
                        'File::Spec' => "0.80",
                    },
                }
            }
        },
    },
    {
        label  => "v1.4 x_custom -> v2 x_custom",
        from   => "1.4",
        to     => "2",
        input  => {
            x_authority => 'DAGOLDEN',
        },
        expect => {
            'meta-spec' => $spec2,
            x_authority => 'DAGOLDEN',
        },
    },
    {
        label  => "meta-spec included",
        to     => "2",
        input  => {
            'meta-spec' => { version => '1.0' },
            requires => {
                'File::Spec' => "0.80",
            },
        },
        expect => {
            'meta-spec' => $spec2,
            prereqs => {
                runtime => {
                    requires => {
                        'File::Spec' => "0.80",
                    },
                }
            }
        },
    },
    {
        # this is a test of default version and intentionally gives bad
        # data that will get dropped by the conversion
        label  => "default version",
        from   => "2",
        to     => "2",
        input  => {
            requires => {
                'File::Spec' => "0.80",
            },
        },
        expect => {
            'meta-spec' => $spec2,
        },
    },
    {
        # fields deprecated from older versions
        label  => "v1.4 prereq stuff -> v2 prereqs",
        from   => "1.4",
        to     => "2",
        input  => {
            configure_requires => {
                'File::Spec' => "0.80",
            },
            build_requires => {
                'Scalar::Util' => '1.0',
            },
            requires => {
                'B' => '3.1',
            },
            recommends => {
                'Config' => '4.0',
            },
            conflicts => {
                'File::Temp' => "0.2",
            },
        },
        expect => {
            'meta-spec' => $spec2,
            prereqs => {
                configure => {
                    requires => {
                        'File::Spec' => "0.80",
                    },
                },
                build => {
                    requires => {
                        'Scalar::Util' => '1.0',
                    },
                },
                runtime => {
                    conflicts => {
                        'File::Temp' => "0.2",
                    },
                    requires => {
                        'B' => '3.1',
                    },
                    recommends => {
                        'Config' => '4.0',
                    },
                },
            },
        },
    },
    {
        label  => "v1.1 license_url: -> v2 license",
        from   => "1.1",
        to     => "2",
        input  => {
            license_url => 'http://opensource.org/licenses/Artistic-1.0',
            license => 'perl',
        },
        expect => {
            'meta-spec' => $spec2,
            license => [ 'perl_5' ],
        },
    },
);

for my $c (@cases) {
    my $cmc = CPAN::Meta::Converter->new(
        $c->{input}, $c->{from} ? (default_version => $c->{from} ) : ()
    );
    my $got = $cmc->upgrade_fragment;
    my $exp = $c->{expect};
    is_deeply( $got, $exp, $c->{label} )
      or diag "GOT:\n", explain($got), "EXPECTED:\n", explain($exp);
}

done_testing;
# vim: ts=8 sts=4 sw=4 et :
