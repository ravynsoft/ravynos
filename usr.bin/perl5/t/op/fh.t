#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 8;

# symbolic filehandles should only result in glob entries with FH constructors

$|=1;
my $a = "SYM000";
ok(!defined(fileno($a)), 'initial file handle is undefined');
ok(!defined *{$a}, 'initial typeglob of file handle is undefined');

select select $a;
ok(defined *{$a}, 'typeglob of file handle defined after select');

$a++;
ok(!close $a, 'close does not succeed with incremented file handle');
ok(!defined *{$a}, 'typeglob of file handle not defined after increment');

ok(open($a, ">&STDOUT"), 'file handle used with open of standard output');
ok(defined *{$a}, 'typeglob of file handle defined after opening standard output');

ok(close $a, 'close standard output via file handle;');

