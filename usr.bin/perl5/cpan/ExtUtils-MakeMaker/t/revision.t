#!/usr/bin/perl -w

use strict;
use warnings;

BEGIN {
    unshift @INC, 't/lib';
}

use Test::More tests => 4;

BEGIN {
    use_ok 'ExtUtils::MakeMaker';
    use_ok 'ExtUtils::MM_VMS';
}

# Why 1?  Because a common mistake is for the regex to run in scalar context
# thus getting the count of captured elements (1) rather than the value of $1
cmp_ok $ExtUtils::MakeMaker::Revision, '>', 1;
cmp_ok $ExtUtils::MM_VMS::Revision,    '>', 1;
