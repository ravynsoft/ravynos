#!/usr/bin/perl -w
use strict;
use Test::More;
use FindBin qw($Bin);
use constant TMPDIR => "$Bin/mkdir_test_delete_me";
use constant ERROR_REGEXP => qr{Can't mkdir\('\Q${\(TMPDIR)}\E', 0777\):};
use constant SINGLE_DIGIT_ERROR_REGEXP => qr{Can't mkdir\('\Q${\(TMPDIR)}\E', 0010\):};

# Delete our directory if it's there
rmdir TMPDIR;

# See if we can create directories and remove them
mkdir TMPDIR or plan skip_all => "Failed to make test directory";

# Test the directory was created
-d TMPDIR or plan skip_all => "Failed to make test directory";

# Try making it a second time (this should fail)
if(mkdir TMPDIR) { plan skip_all => "Attempt to remake a directory succeeded";}

# See if we can remove the directory
rmdir TMPDIR or plan skip_all => "Failed to remove directory";

# Check that the directory was removed
if(-d TMPDIR) { plan skip_all => "Failed to delete test directory"; }

# Try to delete second time
if(rmdir TMPDIR) { plan skip_all => "Able to rmdir directory twice"; }

plan tests => 18;

# Create a directory (this should succeed)
eval {
	use autodie;

	mkdir TMPDIR;
};
is($@, "", "mkdir returned success");
ok(-d TMPDIR, "Successfully created test directory");

# Try to create it again (this should fail)
eval {
	use autodie;

	mkdir TMPDIR, 0777;
};
ok($@, "Re-creating directory causes failure.");
isa_ok($@, "autodie::exception", "... errors are of the correct type");
ok($@->matches("mkdir"), "... it's also a mkdir object");
ok($@->matches(":filesys"), "... and a filesys object");
like($@, ERROR_REGEXP, "Message should include numeric mask in octal form");

eval {
        use autodie;

        mkdir TMPDIR, 8;
};
ok($@, "Re-creating directory causes failure.");
isa_ok($@, "autodie::exception", "... errors are of the correct type");
ok($@->matches("mkdir"), "... it's also a mkdir object");
ok($@->matches(":filesys"), "... and a filesys object");
like($@, SINGLE_DIGIT_ERROR_REGEXP, "Message should include numeric mask in octal form");

# Try to delete directory (this should succeed)
eval {
	use autodie;

	rmdir TMPDIR;
};
is($@, "", "rmdir returned success");
ok(! -d TMPDIR, "Successfully removed test directory");

# Try to delete directory again (this should fail)
eval {
	use autodie;

	rmdir TMPDIR;
};
ok($@, "Re-deleting directory causes failure.");
isa_ok($@, "autodie::exception", "... errors are of the correct type");
ok($@->matches("rmdir"), "... it's also a rmdir object");
ok($@->matches(":filesys"), "... and a filesys object");

