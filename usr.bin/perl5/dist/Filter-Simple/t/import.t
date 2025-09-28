BEGIN {
    unshift @INC, 't/lib';
}

BEGIN { print "1..4\n" }

use Filter::Simple::ImportTest (1..3);

say "not ok 4\n";
