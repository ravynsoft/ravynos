# 20 skip under 5.8

use strict;
use warnings;

BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

print "1..2\n";
  print "# Running under Perl v $]\n";
if($] < 5.008) {
  print "ok 1 # Skip under Perl before 5.8 ($])\n";
} else {
  print "ok 1\n";
  print "# ^ not skipping\n";
}

print "ok 2\n";

