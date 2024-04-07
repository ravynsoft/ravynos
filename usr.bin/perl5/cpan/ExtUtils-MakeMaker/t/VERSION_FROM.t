#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

chdir 't';

use strict;
use warnings;
use Test::More tests => 1;
use MakeMaker::Test::Utils;
use ExtUtils::MakeMaker;
use TieOut;
use File::Path;

perl_lib();

mkdir('Odd-Version', 0777);
END { chdir File::Spec->updir;  rmtree 'Odd-Version' }
chdir 'Odd-Version';

open(MPL, ">Version") || die $!;
print MPL "\$VERSION = 0\n";
close MPL;
END { unlink 'Version' }

my $stdout = tie *STDOUT, 'TieOut' or die;
my $mm = WriteMakefile(
    NAME         => 'Version',
    VERSION_FROM => 'Version'
);

is( $mm->{VERSION}, 0, 'VERSION_FROM when $VERSION = 0' );
