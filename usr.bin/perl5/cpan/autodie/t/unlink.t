#!/usr/bin/perl -w
use strict;
use Test::More;
use FindBin qw($Bin);
use constant TMPFILE      => "$Bin/unlink_test_delete_me";
use constant NO_SUCH_FILE => 'this_file_had_better_not_be_here_at_all';

make_file(TMPFILE);

# Check that file now exists
-e TMPFILE or plan skip_all => "Failed to create test file";

# Check we can unlink
unlink TMPFILE;

# Check it's gone
if(-e TMPFILE) {plan skip_all => "Failed to delete test file: $!";}

# Re-create file
make_file(TMPFILE);

# Check that file now exists
-e TMPFILE or plan skip_all => "Failed to create test file";

plan tests => 10;

# Try to delete file (this should succeed)
eval {
	use autodie;

	unlink TMPFILE;
};
is($@, "", "Unlink appears to have been successful");
ok(! -e TMPFILE, "File does not exist");

# Try to delete file again (this should fail)
eval {
	use autodie;

	unlink TMPFILE;
};
ok($@, "Re-unlinking file causes failure.");
isa_ok($@, "autodie::exception", "... errors are of the correct type");
ok($@->matches("unlink"), "... it's also a unlink object");
ok($@->matches(":filesys"), "... and a filesys object");

# Autodie should throw if we delete a LIST of files, but can only
# delete some of them.

make_file(TMPFILE);
ok(-e TMPFILE, "Sanity: file exists");

eval {
    use autodie;

    unlink TMPFILE, NO_SUCH_FILE;
};

ok($@, "Failure when trying to delete missing file in list.");
isa_ok($@, "autodie::exception", "... errors are of the correct type");
is($@->return,1, "Failure on deleting missing file but true return value");

sub make_file {
    open(my $fh, ">", $_[0])
            or plan skip_all => "Unable to create test file $_[0]: $!";
    print {$fh} "Test\n";
    close $fh;
}
