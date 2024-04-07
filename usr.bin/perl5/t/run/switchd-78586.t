#!perl -Ilib -d:switchd_empty

BEGIN {
    $^P = 0x122;
    chdir 't' if -d 't';
    @INC = ('../lib', 'lib');
    require './test.pl';
}

use strict;
use warnings;
no warnings 'redefine';

plan 2;

our @lines;
sub DB::DB {
  my ($p, $f, $l) = caller;
  return unless $f =~ /^\(eval \d+\)\[.*78586\.t:\d+\]/;
  push @lines, $l;
}

sub trace_lines {
    my ($src) = @_;
    local @lines;
    eval $src;
    die if $@;
    return join " ", @lines;
}

is trace_lines(<<'END'), "1 3 3 5";
    ++$b;
    {
        ++$b;
    }
    ++$b;
END

is trace_lines(<<'END'), "1 2 3 3 5";
    ++$b;
    for (my $a=1; $a <= 2; ++$a) {
        ++$b;
    }
    ++$b;
END

