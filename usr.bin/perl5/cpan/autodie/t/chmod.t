#!/usr/bin/perl -w
use strict;
use Test::More tests => 7;
use constant NO_SUCH_FILE => "this_file_had_better_not_exist";
use constant ERROR_REGEXP => qr{Can't chmod\(0755, '${\(NO_SUCH_FILE)}'\):};
use constant SINGLE_DIGIT_ERROR_REGEXP => qr{Can't chmod\(0010, '${\(NO_SUCH_FILE)}'\):};
use autodie;

# This tests RT #50423, Debian #550462

eval { chmod(0755, NO_SUCH_FILE); };
isa_ok($@, 'autodie::exception', 'exception thrown for chmod');
like($@, ERROR_REGEXP, "Message should include numeric mode in octal form");

eval { chmod(8, NO_SUCH_FILE); };
isa_ok($@, 'autodie::exception', 'exception thrown for chmod');
like($@, SINGLE_DIGIT_ERROR_REGEXP, "Message should include numeric mode in octal form");

eval { chmod(0755, $0); };
ok(! $@, "We can chmod ourselves just fine.");

eval { chmod(0755, $0, NO_SUCH_FILE) };
isa_ok($@, 'autodie::exception', 'chmod exception on any file failure.');
is($@->return,1,"Confirm autodie on a 'true' chown failure.");
