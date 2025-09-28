package B::success;
use strict;

$| = 1;
print "Compiling!\n";

sub compile {
    return 'fail' if ($_[0] eq 'fail');
    print "($_[0]) <$_[1]>\n";
    return sub { print "[$O::BEGIN_output]\n" };
}

1;
