#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use lib 't/lib';
use Test::More tests => 1;
use File::Spec;
use App::Prove;

my $prove = App::Prove->new;

$prove->add_rc_file(
    File::Spec->catfile(
        't', 'data',
        'proverc'
    )
);

is_deeply $prove->{rc_opts},
  [ '--should', 'be', '--split', 'correctly', 'Can', 'quote things',
    'using single or', 'double quotes', '--this', 'is', 'OK?'
  ],
  'options parsed';

