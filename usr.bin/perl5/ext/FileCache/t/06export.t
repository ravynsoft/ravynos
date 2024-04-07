#!./perl
our (@funcs, $i);

BEGIN {
    # Functions exported by FileCache;
    @funcs  = qw[cacheout cacheout_close];
    $i      = 0;
}

use Test::More tests => 8;

# Test 6: Test that exporting both works to package main and
# other packages. Now using Exporter.

# First, we shouldn't be able to have these in our namespace
# Add them to BEGIN so the later 'use' doesn't influence this
# test
BEGIN {   
    ok(not __PACKAGE__->can($_)) foreach @funcs;
}

# With an empty import list, we also shouldn't have them in
# our namespace.
# Add them to BEGIN so the later 'use' doesn't influence this
# test
BEGIN {   
    use FileCache ();
    ok(not __PACKAGE__->can($_)) foreach @funcs;
}


# Now, we use FileCache in 'main'
{
    use FileCache;
    ok(__PACKAGE__->can($_)) foreach @funcs;
}

# Now we use them in another package
{
    package X;
    use FileCache;
    ::ok(__PACKAGE__->can($_)) foreach @main::funcs;
}

