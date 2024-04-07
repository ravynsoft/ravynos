#!perl
use strict;
use warnings;

# As perlfunc.pod says:
# Note that the file will not be included twice under the same specified name.
# So ensure that this, textually, is the same name as all the loaded tests use.
# Otherwise if we require 'test.pl' and they require './test.pl', it is loaded
# twice.
require './test.pl';
skip_all_without_config('useithreads');
skip_all_if_miniperl("no dynamic loading on miniperl, no threads");

require threads;

# Which file called us?
my $caller = (caller)[1];

die "Can't figure out which test to run from filename '$caller'"
    unless $caller =~ m!((?:op|re)/[-_a-z0-9A-Z]+)_thr\.t\z!;

my $file = "./$1.t";

$::running_as_thread = "running tests in a new thread";
require $file;

note('running tests in a new thread');

# Currently 59*4096 is the minimum stack size to just get t/re/pat_thr.t to
# pass on HP-UX 64bit PA-RISC. The test for capture buffers (eg \87)
# recurses heavily, and busts the default stack size (65536 on PA-RISC)
# On Mac OS X under gcc and g++, the default stack size is also too small.
# Ditto on VMS, although threshold varies by platform and -Dusevmsdebug.
# Same on AIX
my $curr = threads->create({
                            stack_size => $^O eq 'hpux'   ? 524288 :
                                          $^O eq 'darwin' ? 2000000:
                                          $^O eq 'VMS'    ? 150000 :
                                          $^O eq 'aix'    ? 1500000 : 0,
                           }, sub {
			       run_tests();
			       return defined &curr_test ? curr_test() : ()
			   })->join();

curr_test($curr) if defined $curr;

1;
