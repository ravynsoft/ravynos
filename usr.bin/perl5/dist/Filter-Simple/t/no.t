BEGIN {
    unshift @INC, 't/lib/';
}

print "1..2\n";

use Filter::Simple::FilterTest qr/ok/ => "not ok", pass => "fail";
no Filter::Simple::FilterTest;

sub pass { print "ok ", $_[0], "\n" }

print "ok 1\n";
("pa"."ss")->(2);
