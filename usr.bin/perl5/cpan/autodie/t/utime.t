#!/usr/bin/perl -w
use strict;
use Test::More tests => 4;
use constant NO_SUCH_FILE => "this_file_had_better_not_exist";
use FindBin qw($Bin);
use File::Spec;
use constant TOUCH_ME     => File::Spec->catfile($Bin, 'touch_me');
use autodie;

eval { utime(undef, undef, NO_SUCH_FILE); };
isa_ok($@, 'autodie::exception', 'exception thrown for utime');

my($atime, $mtime) = (stat TOUCH_ME)[8, 9];

eval { utime(undef, undef, TOUCH_ME); };
ok(! $@, "We can utime a file just fine.") or diag $@;

eval { utime(undef, undef, NO_SUCH_FILE, TOUCH_ME); };
isa_ok($@, 'autodie::exception', 'utime exception on single failure.');
is($@->return, 1, "utime fails correctly on a 'true' failure.");

# Reset timestamps so that Git doesn't think the file has changed when
# running the test in the core perl distribution.
utime($atime, $mtime, TOUCH_ME);
