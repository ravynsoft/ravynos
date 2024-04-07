#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

my $hires;

BEGIN {
    $hires = eval 'use Time::HiRes qw(sleep); 1';
}

use Test::More (
      $^O eq 'VMS' ? ( skip_all => 'VMS' )
    : $hires ? ( tests => 9 * 3 )
    : ( skip_all => 'Need Time::HiRes' )
);

use File::Spec;
use TAP::Parser::Iterator::Process;

my @expect = (
    '1..5',
    'ok 1 00000',
    'ok 2',
    'not ok 3',
    'ok 4',
    'ok 5 00000',
);

my $source = File::Spec->catfile(
    't',
    'sample-tests',
    'delayed'
);

for my $chunk_size ( 1, 4, 65536 ) {
    for my $where ( 0 .. 8 ) {

        my $proc = TAP::Parser::Iterator::Process->new(
            {   _chunk_size => $chunk_size,
                command     => [ $^X, $source, ( 1 << $where ) ]
            }
        );

        my @got = ();
        while ( defined( my $line = $proc->next_raw ) ) {
            push @got, $line;
        }

        is_deeply \@got, \@expect,
          "I/O ok with delay at position $where, chunk size $chunk_size";
    }
}
