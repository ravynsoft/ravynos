BEGIN { chdir 't' if -d 't' }

use Test::More 'no_plan';
use strict;
use lib '../lib';

use Archive::Tar;
use File::Spec;

$Archive::Tar::WARN = 0;

my $t1 = Archive::Tar->new;
my $t2 = Archive::Tar->new;

is($Archive::Tar::error, "", "global error string is empty");
is($t1->error, "", "error string of object 1 is empty");
is($t2->error, "", "error string of object 2 is empty");

ok(!$t1->read(), "can't read without a file");

isnt($t1->error, "", "error string of object 1 is set");
is($Archive::Tar::error, $t1->error, "global error string equals that of object 1");
is($Archive::Tar::error, Archive::Tar->error, "the class error method returns the global error");
is($t2->error, "", "error string of object 2 is still empty");

my $src = File::Spec->catfile( qw[src short b] );
ok(!$t2->read($src), "error when opening $src");

isnt($t2->error, "", "error string of object 1 is set");
isnt($t2->error, $t1->error, "error strings of objects 1 and 2 differ");
is($Archive::Tar::error, $t2->error, "global error string equals that of object 2");
is($Archive::Tar::error, Archive::Tar->error, "the class error method returns the global error");
