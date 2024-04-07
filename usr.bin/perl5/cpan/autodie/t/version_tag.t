#!/usr/bin/perl -w
use strict;
use warnings;
use Test::More tests => 10;
use constant NO_SUCH_FILE => 'THIS_FILE_HAD_BETTER_NOT_EXIST';

eval {
    use autodie qw(:1.994);

    open(my $fh, '<', 'this_file_had_better_not_exist.txt');
};

isa_ok($@, 'autodie::exception', "Basic version tags work");

# Expanding :1.00 should fail, there was no autodie :1.00
eval { my $foo = autodie->_expand_tag(":1.00"); };

isnt($@,"","Expanding :1.00 should fail");

my $version = $autodie::VERSION;

SKIP: {

    if (not defined($version) or $version =~ /_/) {
	skip "Tag test skipped on dev release", 1;
    }

    # Expanding our current version should work!
    eval { my $foo = autodie->_expand_tag(":$version"); };

    is($@,"","Expanding :$version should succeed");
}

eval {
    use autodie qw(:2.07);

    # 2.07 didn't support chmod.  This shouldn't throw an
    # exception.

    chmod(0644,NO_SUCH_FILE);
};

is($@,"","chmod wasn't supported in 2.07");

eval {
    use autodie;

    chmod(0644,NO_SUCH_FILE);
};

isa_ok($@, 'autodie::exception', 'Our current version supports chmod');

eval {
    use autodie qw(:2.13);

    # 2.13 didn't support chown.  This shouldn't throw an
    # exception.

    chown(12345, 12345, NO_SUCH_FILE);
};

is($@,"","chown wasn't supported in 2.13");

SKIP: {

    if ($^O eq "MSWin32") { skip("chown() on Windows always succeeds.", 1) }

    eval {
        use autodie;

        chown(12345, 12345, NO_SUCH_FILE);
    };

    isa_ok($@, 'autodie::exception', 'Our current version supports chown');
}

# The patch in RT 46984 would have utime being set even if an
# older version of autodie semantics was requested. Let's see if
# it's coming from outside the eval context below.

eval { utime undef, undef, NO_SUCH_FILE; };
is($@,"","utime is not autodying outside of any autodie context.");

# Now do our regular versioning checks for utime.

eval {
    use autodie qw(:2.13);

    utime undef, undef, NO_SUCH_FILE;
};

is($@,"","utime wasn't supported in 2.13");

eval {
    use autodie;

    utime undef, undef, NO_SUCH_FILE;
};

isa_ok($@, 'autodie::exception', 'Our current version supports utime');
