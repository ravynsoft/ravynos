BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Pod::Simple::Search;
use Test;
BEGIN { plan tests => 16 }

print "# Some basic sanity tests...\n";

my $x = Pod::Simple::Search->new;
die "Couldn't make an object!?" unless ok defined $x;
print "# New object: $x\n";
print "# Version: ", $x->VERSION, "\n";
ok defined $x->can('callback');
ok defined $x->can('dir_prefix');
ok defined $x->can('inc');
ok defined $x->can('laborious');
ok defined $x->can('limit_glob');
ok defined $x->can('limit_re');
ok defined $x->can('recurse');
ok defined $x->can('shadows');
ok defined $x->can('verbose');
ok defined $x->can('survey');
ok defined $x->can('_state_as_string');
ok defined $x->can('contains_pod');
ok defined $x->can('find');
ok defined $x->can('simplify_name');

print "# Testing state dumping...\n";
print $x->_state_as_string;
$x->inc("I\nLike  Pie!\t!!");
print $x->_state_as_string;

print "# bye\n";
ok 1;

