#!/usr/bin/env perl

use strict;
use warnings;

use Test::More tests => 4;

# To find Digest::Dummy
use lib 't/lib';

use Digest;

$@ = "rt#50663";
my $d;
$d = Digest->new("Dummy");
is $@, "rt#50663";
is $d->digest, "ooo";

$d = Digest->Dummy;
is $d->digest, "ooo";

$Digest::MMAP{"Dummy-24"} = [ ["NotThere"], "NotThereEither", [ "Digest::Dummy", 24 ] ];
$d = Digest->new("Dummy-24");
is $d->digest, "24";
