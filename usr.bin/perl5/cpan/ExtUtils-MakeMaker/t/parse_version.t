#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use Test::More;
use ExtUtils::MakeMaker;
use File::Temp qw[tempfile];

my $Has_Version = eval 'require version; "version"->import; 1';

# "undef" - means we expect "undef", undef - eval should be never called for this string
my %versions = (q[$VERSION = '1.00']            => '1.00',
                q[*VERSION = \'1.01']           => '1.01',
                q[($VERSION) = q$Revision: 32208 $ =~ /(\d+)/g;] => 32208,
                q[$FOO::VERSION = '1.10';]      => '1.10',
                q[*FOO::VERSION = \'1.11';]     => '1.11',
                '$VERSION = 0.02'               => 0.02,
                '$VERSION = 0.0'                => 0.0,
                '$VERSION = -1.0'               => 'undef',
                '$VERSION = undef'              => 'undef',
                '$wibble  = 1.0'                => undef,
                q[my $VERSION = '1.01']         => 'undef',
                q[local $VERSION = '1.02']      => 'undef',
                q[local $FOO::VERSION = '1.30'] => 'undef',
                q[if( $Foo::VERSION >= 3.00 ) {]=> undef,
                q[our $VERSION = '1.23';]       => '1.23',
                q[$CGI::VERSION='3.63']         => '3.63',
                q[$VERSION = "1.627"; # ==> ALSO update the version in the pod text below!] => '1.627',
                q[BEGIN { our $VERSION = '1.23' }]       => '1.23',

                '$Something::VERSION == 1.0'    => undef,
                '$Something::VERSION <= 1.0'    => undef,
                '$Something::VERSION >= 1.0'    => undef,
                '$Something::VERSION != 1.0'    => undef,
                'my $meta_coder = ($JSON::XS::VERSION >= 1.4) ?' => undef,

                qq[\$Something::VERSION == 1.0\n\$VERSION = 2.3\n]                     => '2.3',
                qq[\$Something::VERSION == 1.0\n\$VERSION = 2.3\n\$VERSION = 4.5\n]    => '2.3',

                '$VERSION = sprintf("%d.%03d", q$Revision: 3.74 $ =~ /(\d+)\.(\d+)/);' => '3.074',
                '$VERSION = substr(q$Revision: 2.8 $, 10) + 2 . "";'                   => '4.8',
                q[our $VERSION = do { my @r = ( q$Revision: 2.7 $ =~ /\d+/g ); sprintf "%d." . "%02d" x $#r, @r };] => '2.07', # Fucking seriously?
                'elsif ( $Something::VERSION >= 1.99 )' => undef,
               );

if( $Has_Version ) {
    $versions{q[use version; $VERSION = qv("1.2.3");]} = qv("1.2.3");
    $versions{q[$VERSION = qv("1.2.3")]}               = qv("1.2.3");
    $versions{q[$VERSION = v1.2.3]} = 'v1.2.3';
}

if( "$]" >= 5.011001 ) {
    $versions{'package Foo 1.23;'         } = '1.23';
    $versions{'package Foo::Bar 1.23;'    } = '1.23';
    $versions{'package Foo v1.2.3;'       } = 'v1.2.3';
    $versions{'package Foo::Bar v1.2.3;'  } = 'v1.2.3';
    $versions{' package Foo::Bar 1.23 ;'  } = '1.23';
    $versions{"package Foo'Bar 1.23;"     } = '1.23';
    $versions{'package Foo 1.230;'        } = '1.230';
    $versions{q["package Foo 1.23"]}        = undef;
    $versions{q[our $VERSION = "1.00 / the fucking fuck";]} = 'undef';
    $versions{<<'END'}                      = '1.23';
package Foo 1.23;
our $VERSION = 2.34;
END

    $versions{<<'END'}                      = '2.34';
our $VERSION = 2.34;
package Foo 1.23;
END

    $versions{<<'END'}                      = '2.34';
package Foo::100;
our $VERSION = 2.34;
END
}

if( "$]" >= 5.014 ) {
    $versions{'package Foo 1.23 { }'         } = '1.23';
    $versions{'package Foo::Bar 1.23 { }'    } = '1.23';
    $versions{'package Foo v1.2.3 { }'       } = 'v1.2.3';
    $versions{'package Foo::Bar v1.2.3 { }'  } = 'v1.2.3';
    $versions{' package Foo::Bar 1.23 { }'   } = '1.23';
    $versions{"package Foo'Bar 1.23 { }"     } = '1.23';
    $versions{'package Foo 1.230 { }'        } = '1.230';
    $versions{<<'END'}                      = '1.23';
package Foo 1.23 {
our $VERSION = 2.34;
}
END

    $versions{<<'END'}                      = '2.34';
our $VERSION = 2.34;
package Foo 1.23 { }
END

    $versions{<<'END'}                      = '2.34';
package Foo::100 {
our $VERSION = 2.34;
}
END
}

if ( "$]" < 5.012 ) {
  delete $versions{'$VERSION = -1.0'};
}

plan tests => (3 * keys %versions) + 4 + grep { !defined} (values %versions);

for my $code ( sort keys %versions ) {
    my $expect = $versions{$code};
    (my $label = $code) =~ s/\n/\\n/g;
    my $warnings = "";
    local $SIG{__WARN__} = sub { $warnings .= "@_\n"; };
	if (defined $expect) {
		is( parse_version_string($code), $expect, $label );
	} else {
		my $is_called = 0;
		no warnings qw[redefine once];
		local *MM::get_version = sub {
			$is_called = 1;
		};
		ok !$is_called;
		is( parse_version_string($code), 'undef', $label );
	}
    is($warnings, '', "$label does not cause warnings");
}


sub parse_version_string {
    my $code = shift;

    my ($fh,$file) = tempfile( DIR => '.', UNLINK => 1 );
    print $fh "$code\n";
    close $fh;

    $_ = 'foo';
    my $version = MM->parse_version( $file );
    is( $_, 'foo', '$_ not leaked by parse_version' );

    return $version;
}


# This is a specific test to see if a version subroutine in the $VERSION
# declaration confuses later calls to the version class.
# [rt.cpan.org 30747]
SKIP: {
    skip "need version.pm", 4 unless $Has_Version;
    is parse_version_string(q[ $VERSION = '1.00'; sub version { $VERSION } ]),
       '1.00', "eval 'sub version {...} in version string";
    is parse_version_string(q[ use version; $VERSION = version->new("1.2.3") ]),
       qv("1.2.3"), "version.pm not confused by version sub";
}
