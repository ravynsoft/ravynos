#!/usr/bin/perl -w
use strict;
use warnings;

use Test::More tests => 7;

require Fatal;

my @default = expand(':default');
my @threads = expand(':threads');
my @io = expand(':io');
my %io_hash = map { $_ => 1 } @io;
my @default_minus_io = grep { !exists($io_hash{$_}) } @default;

is_deeply(translate('!a', 'a'), ['!a'], 'Keeps insist variant');

is_deeply(translate(':default'), \@default,
          'translate and expand agrees');

is_deeply(translate(':default', ':void', ':io'),
          [@default_minus_io, ':void', @io],
          ':void position is respected');

is_deeply(translate(':default', ':void', ':io', ':void', ':threads'),
          [':void', @io, ':void', @threads],
          ':void (twice) position are respected');

is_deeply(translate(':default', '!', ':io'),
    [@default_minus_io, '!', @io], '! position is respected');

is_deeply(translate(':default', '!', ':io', '!', ':threads'),
          ['!', @io, '!', @threads],
          '! (twice) positions are respected');

is_deeply(translate(':default', '!open', '!', ':io'),
    [@default_minus_io, '!open', '!', grep { $_ ne 'open' } @io],
          '!open ! :io works as well');

sub expand {
    # substr is to strip "CORE::" without modifying $_
    return map { substr($_, 6) } @{Fatal->_expand_tag(@_)};
}

sub translate {
    return [Fatal->_translate_import_args(@_)];
}
