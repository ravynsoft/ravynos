#!/usr/bin/perl -w
use strict;

use Test::More;
use File::Temp qw(tempfile);
use IO::Handle;
use File::Spec;
use FindBin qw($Bin);

my ($truncate_status, $tmpfh, $tmpfile);

# Some systems have a screwy tempfile. We don't run our tests there.
eval {
    ($tmpfh, $tmpfile) = tempfile(UNLINK => 1);
};

if ($@ or !defined $tmpfh) {
    plan skip_all => 'tempfile() not happy on this system.';
}

eval {
    $truncate_status = truncate($tmpfh, 0);
};

if ($@ || !defined($truncate_status)) {
    plan skip_all => 'Truncate not implemented or not working on this system';
}

plan tests => 12;

SKIP: {
    my $can_truncate_stdout = truncate(\*STDOUT,0);

    if ($can_truncate_stdout) {
        skip("This system thinks we can truncate STDOUT. Suuure!", 1);
    }

    eval {
        use autodie;
        truncate(\*STDOUT,0);
    };

    isa_ok($@, 'autodie::exception', "Truncating STDOUT should throw an exception");

}

eval {
    use autodie;
    no warnings 'once';
    truncate(\*FOO, 0);
};

isa_ok($@, 'autodie::exception', "Truncating an unopened file is wrong.");

$tmpfh->print("Hello World");
$tmpfh->flush;

eval {
    use autodie;
    truncate($tmpfh, 0);
};

is($@, "", "Truncating a normal file should be fine");

$tmpfh->close;

# Time to test truncating via globs.

# Firstly, truncating a closed filehandle should fail.
# I know we tested this above, but we'll do a full dance of
# opening and closing TRUNCATE_FH here.

eval {
    use autodie qw(truncate);
    truncate(\*TRUNCATE_FH, 0);
};

isa_ok($@, 'autodie::exception', "Truncating unopened file (TRUNCATE_FH)");

# Now open the file. If this throws an exception, there's something
# wrong with our tests, or autodie...
{
    use autodie qw(open);
    open(TRUNCATE_FH, '+<', $tmpfile);
}

# Now try truncating the filehandle. This should succeed.

eval {
    use autodie qw(truncate);
    truncate(\*TRUNCATE_FH,0);
};

is($@, "", 'Truncating an opened glob (\*TRUNCATE_FH)');

eval {
    use autodie qw(truncate);
    truncate(*TRUNCATE_FH,0);
};

is($@, "", 'Truncating an opened glob (*TRUNCATE_FH)');

# Now let's change packages, since globs are package dependent

eval {
    package Fatal::Test;
    no warnings 'once';
    use autodie qw(truncate);
    truncate(\*TRUNCATE_FH,0);  # Should die, as now unopened
};

isa_ok($@, 'autodie::exception', 'Truncating unopened file in different package (\*TRUNCATE_FH)');

eval {
    package Fatal::Test;
    no warnings 'once';
    use autodie qw(truncate);
    truncate(*TRUNCATE_FH,0);  # Should die, as now unopened
};

isa_ok($@, 'autodie::exception', 'Truncating unopened file in different package (*TRUNCATE_FH)');

# Now back to our previous test, just to make sure it hasn't changed
# the original file.

eval {
    use autodie qw(truncate);
    truncate(\*TRUNCATE_FH,0);
};

is($@, "", 'Truncating an opened glob #2 (\*TRUNCATE_FH)');

eval {
    use autodie qw(truncate);
    truncate(*TRUNCATE_FH,0);
};

is($@, "", 'Truncating an opened glob #2 (*TRUNCATE_FH)');

# Now to close the file and retry.
{
    use autodie qw(close);
    close(TRUNCATE_FH);
}

eval {
    use autodie qw(truncate);
    truncate(\*TRUNCATE_FH,0);
};

isa_ok($@, 'autodie::exception', 'Truncating freshly closed glob (\*TRUNCATE_FH)');

eval {
    use autodie qw(truncate);
    truncate(*TRUNCATE_FH,0);
};

isa_ok($@, 'autodie::exception', 'Truncating freshly closed glob (*TRUNCATE_FH)');
