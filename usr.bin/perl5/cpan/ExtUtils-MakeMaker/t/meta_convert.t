#!perl -w

use strict;
use warnings;

BEGIN { unshift @INC, 't/lib'; }
use Test::More eval { require CPAN::Meta; CPAN::Meta->VERSION(2.143240) } ? ()
  : (skip_all => 'CPAN::Meta 2.143240 required for this test');
use File::Temp qw[tempdir];
require ExtUtils::MM_Any;

my $tmpdir = tempdir( DIR => 't', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir or die "chdir $tmpdir: $!";

my $EMPTY = qr/['"]?version['"]?\s*:\s*['"]['"]/;
my @DATA = (
    [
        [ DISTNAME => 'Net::FTP::Recursive', VERSION  => 'Recursive.pm', ],
        qr{Can't parse version 'Recursive.pm'},
        'VERSION => filename',
        $EMPTY,
    ],
    [
        [ DISTNAME => 'Image::Imgur', VERSION  => 'undef', ],
        qr{Can't parse version 'undef'},
        'no $VERSION in file -> VERSION=>"undef"',
        $EMPTY,
    ],
    [
        [ DISTNAME => 'SQL::Library', VERSION  => 0.0.3, ],
        qr{Can't parse version '\x00\x00\x03'},
        "x.y.z version",
        $EMPTY,
    ],
    [
        [ DISTNAME => 'Array::Suffix', VERSION  => '.5', ],
        qr{Can't parse version '.5'},
        ".5 version",
        $EMPTY,
    ],
    [
        [
            DISTNAME   => 'Attribute::Signature',
            META_MERGE => {
                resources => {
                    repository         => 'http://github.com/chorny/Attribute-Signature',
                    'Repository-clone' => 'git://github.com/chorny/Attribute-Signature.git',
                },
            },
        ],
        qr/^$/,
        "Non-camel case metadata",
        qr/x_Repositoryclone/,
    ],
    [
        [
            DISTNAME   => 'CPAN::Testers::ParseReport',
            VERSION    => '2.34',
            META_ADD => {
                provides => {
                    "CPAN::Testers::ParseReport" => {
                        version => version->new("v1.2.3"),
                        file    => "lib/CPAN/Testers/ParseReport.pm"
                    }
                }
            },
        ],
        qr/^$/,
        "version object in provides",
        qr/['"]?version['"]?\s*:\s*['"]v1\.2\.3['"]/,
    ],
    [
        [
            DISTNAME   => 'Bad::License',
            VERSION    => '2.34',
            LICENSE   => 'death and retribution',
        ],
        qr/Invalid LICENSE value/,
        "Bad licence warns",
        qr/['"]?version['"]?\s*:\s*['"]2\.34['"]/,
    ],
);

plan tests => 3 * @DATA;
run_test(@$_) for @DATA;

sub ExtUtils::MM_Any::quote_literal { $_[1] }

sub run_test {
    my ($mmargs, $expected, $label, $metadata_re) = @_;
    my $mm = bless { ARGS => {@$mmargs}, @$mmargs }, 'ExtUtils::MM_Any';
    my @warnings;
    my $ret;
    {
        local $SIG{__WARN__} = sub { push @warnings, @_ };
        eval { $ret = $mm->metafile_target; };
    }
    SKIP: {
        if ($@) {
            diag $@;
            skip "$label got exception", 3 if $@;
        }
        ok 1, "$label metafile_target";
        like join("", @warnings), $expected, "$label right warning";
        like $ret, $metadata_re, "$label metadata";
    }
}
