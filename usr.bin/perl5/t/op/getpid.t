#!perl -w

# Tests if $$ and getppid return consistent values across threads

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(../lib) );
    skip_all_if_miniperl(
	"no dynamic loading on miniperl, no threads/attributes"
    );
}

use strict;
use Config;

{
    skip_all_without_config(qw(useithreads d_getppid));
    eval 'use threads; use threads::shared';
    plan tests => 3;
    if ($@) {
	fail("unable to load thread modules");
    }
    else {
	pass("thread modules loaded");
    }
}

my ($pid, $ppid) = ($$, getppid());
my $pid2 : shared = 0;
my $ppid2 : shared = 0;

new threads( sub { ($pid2, $ppid2) = ($$, getppid()); } ) -> join();

# If this breaks you're either running under LinuxThreads (and we
# haven't detected it) or your system doesn't have POSIX thread
# semantics.
# Newer linuxthreads from gnukfreebsd (0.11) does have POSIX thread
# semantics, so include a version check
# <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=675606>
my $thread_version = qx[getconf GNU_LIBPTHREAD_VERSION 2>&1] // '';
chomp $thread_version;
if ($^O =~ /^(?:gnukfreebsd|linux)$/ and
    $thread_version =~ /linuxthreads/ and
    !($thread_version =~ /linuxthreads-(.*)/ && $1 >= 0.11)) {
    diag "We're running under $^O with linuxthreads <$thread_version>";
    isnt($pid,  $pid2, "getpid() in a thread is different from the parent on this non-POSIX system");
    isnt($ppid, $ppid2, "getppid() in a thread is different from the parent on this non-POSIX system");
} else {
    is($pid,  $pid2, 'getpid() in a thread is the same as in the parent');
    is($ppid, $ppid2, 'getppid() in a thread is the same as in the parent');
}
