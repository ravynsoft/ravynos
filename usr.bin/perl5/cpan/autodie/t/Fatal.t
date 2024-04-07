#!/usr/bin/perl -w
use strict;

use constant NO_SUCH_FILE => "this_file_or_dir_had_better_not_exist_XYZZY";

use Test::More tests => 17;

use Fatal qw(:io :void opendir);

eval { open FOO, "<".NO_SUCH_FILE };	# Two arg open
like($@, qr/^Can't open/, q{Package Fatal::open});
is(ref $@, "", "Regular fatal throws a string");

my $foo = 'FOO';
for ('$foo', "'$foo'", "*$foo", "\\*$foo") {
    eval qq{ open $_, '<$0' };

    is($@,"", "Open using filehandle named - $_");

    like(scalar(<$foo>), qr{^#!.*/perl}, "File contents using - $_");
    eval qq{ close FOO };

    is($@,"", "Close filehandle using - $_");
}

eval { opendir FOO, NO_SUCH_FILE };
like($@, qr{^Can't open}, "Package :void Fatal::opendir");

eval { my $a = opendir FOO, NO_SUCH_FILE };
is($@, "", "Package :void Fatal::opendir in scalar context");

eval { Fatal->import(qw(print)) };
like(
	$@, qr{Cannot make the non-overridable builtin print fatal},
	"Can't override print"
);
