#!perl

use strict;
use warnings;

use Test::More;
use HTTP::Tiny;

# test cases adapted from HTTP::Date
my $epoch = 760233600;

my @cases = (
  ['Thu, 03 Feb 1994 00:00:00 GMT',       'RFC822+RFC1123'],
  ['Thu,  3 Feb 1994 00:00:00 GMT',       'broken RFC822+RFC1123'],
  ['Thursday, 03-Feb-94 00:00:00 GMT',    'old rfc850 HTTP format'],
  ['Thursday, 03-Feb-1994 00:00:00 GMT',  'broken rfc850 HTTP format'],
  ['Thu Feb  3 00:00:00 GMT 1994',        'ctime format'],
  ['Thu Feb  3 00:00:00 1994',            'same as ctime, except no TZ'],
);

plan tests => 1 + @cases;

is(HTTP::Tiny->_http_date($epoch), $cases[0][0], "epoch -> RFC822/RFC1123");

for my $c ( @cases ) {
  is( HTTP::Tiny->_parse_http_date($c->[0]), $epoch, $c->[1] . " -> epoch");
}


