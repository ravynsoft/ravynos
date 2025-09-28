use strict;
use warnings;
use lib 't/lib/';
use Test::More 0.88;
use TestBridge;
use File::Spec::Functions 'catfile';
use File::Temp 0.19; # newdir

#--------------------------------------------------------------------------#
# This file test that the YAML.pm compatible Dump/Load/DumpFile/LoadFile
# work as documented
#--------------------------------------------------------------------------#

use CPAN::Meta::YAML;

{
    my $scalar = 'this is a string';
    my $arrayref = [ 1 .. 5 ];
    my $hashref = { alpha => 'beta', gamma => 'delta' };

    my $yamldump = CPAN::Meta::YAML::Dump( $scalar, $arrayref, $hashref );
    my @yamldocsloaded = CPAN::Meta::YAML::Load($yamldump);
    cmp_deeply(
        [ @yamldocsloaded ],
        [ $scalar, $arrayref, $hashref ],
        "Functional interface: Dump to Load roundtrip works as expected"
    );
}

{
    my $scalar = 'this is a string';
    my $arrayref = [ 1 .. 5 ];
    my $hashref = { alpha => 'beta', gamma => 'delta' };

    my $tempdir = File::Temp->newdir("YTXXXXXX", TMPDIR => 1 );
    my $filename = catfile($tempdir, 'compat');

    my $rv = CPAN::Meta::YAML::DumpFile(
        $filename, $scalar, $arrayref, $hashref);
    ok($rv, "DumpFile returned true value");

    my @yamldocsloaded = CPAN::Meta::YAML::LoadFile($filename);
    cmp_deeply(
        [ @yamldocsloaded ],
        [ $scalar, $arrayref, $hashref ],
        "Functional interface: DumpFile to LoadFile roundtrip works as expected"
    );
}

{
    my $str = "This is not real YAML";
    my @yamldocsloaded;
    eval { @yamldocsloaded = CPAN::Meta::YAML::Load("$str\n"); };
    error_like(
        qr/CPAN::Meta::YAML failed to classify line '$str'/,
        "Correctly failed to load non-YAML string"
    );
}

done_testing;
