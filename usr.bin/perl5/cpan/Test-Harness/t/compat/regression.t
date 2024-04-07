#!/usr/bin/perl -w

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 1;
use Test::Harness;

# 28567
my ( @before, @after );
{
    local @INC;
    unshift @INC, 'wibble';
    @before = Test::Harness::_filtered_inc();
    unshift @INC, sub {die};
    @after = Test::Harness::_filtered_inc();
}

is_deeply \@after, \@before, 'subref removed from @INC';
