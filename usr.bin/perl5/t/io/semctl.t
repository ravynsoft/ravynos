BEGIN {
  chdir 't' if -d 't';
  @INC = '../lib' if -d '../lib' && -d '../ext';

  require "./test.pl";
  require Config; import Config;
}

use strict;
use warnings;

skip_all('no SysV semaphores on this platform') if !$Config{d_sem};
skip_all('SysV semaphore structures not detected')
    if !$Config{d_semctl_semid_ds} && !$Config{d_semctl_semun};

my @warnings;
{
    local $SIG{__WARN__} = sub { push @warnings, $_[0] };
    local $SIG{SYS} = sub { skip_all("SIGSYS caught") } if exists $SIG{SYS};
    my $test = (semctl(-1,0,0,0))[0];
    ok(!defined $test, "erroneous semctl list slice yields undef");
}

is(scalar @warnings, 0, "no warnings from erroneous semctl list slice")
    or diag("warnings found: @warnings");

done_testing;
