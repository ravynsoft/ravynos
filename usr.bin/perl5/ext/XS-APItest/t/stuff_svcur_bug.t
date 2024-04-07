use warnings;
use strict;

use Test::More tests => 1;
ok 1;

use XS::APItest qw(stufftest);

# In the buggy case, a syntax error occurs at EOF.
# Adding a semicolon, any following statements, or anything else
# causes the bug not to show itself.
stufftest+;()
