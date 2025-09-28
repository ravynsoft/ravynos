#!./perl

use warnings;
use strict;
use Config;
use Fcntl;
use Test::More;
use DB_File;
use File::Temp qw(tempdir) ;

if (-d "lib" && -f "TEST") {
    if ($Config{'extensions'} !~ /\bDB_File\b/ ) {
        plan skip_all => 'DB_File was not built';
    }
}
plan skip_all => 'Threads are disabled'
    unless $Config{usethreads};

plan skip_all => 'Thread test needs Perl 5.8.7 or greater'
    unless $] >= 5.008007;

plan tests => 7;

# Check DBM back-ends do not destroy objects from then-spawned threads.
# RT#61912.
use_ok('threads');

my $TEMPDIR = tempdir( CLEANUP => 1 );
chdir $TEMPDIR;

my %h;
unlink <threads*>;

my $db = tie %h, 'DB_File', 'threads', O_RDWR|O_CREAT, 0640;
isa_ok($db, 'DB_File');

for (1 .. 2) {
    ok(threads->create(
        sub {
            $SIG{'__WARN__'} = sub { fail(shift) }; # debugging perl panics
                # report it by spurious TAP line
            1;
        }), "Thread $_ created");
}
for (threads->list) {
    is($_->join, 1, "A thread exited successfully");
}

pass("Tied object survived exiting threads");

undef $db;
untie %h;
unlink <threads*>;
