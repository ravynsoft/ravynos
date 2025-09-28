#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
    require Config;
    $Config::Config{d_pseudofork}
        or skip_all("no pseudo-fork");
    eval 'use Errno';
    die $@ if $@ and !is_miniperl();
}

# [perl #77672] backticks capture text printed to stdout when working
# with multiple threads on windows
watchdog(20); # before the fix this would often lock up

fresh_perl_like(<<'PERL', qr/\A[z\n]+\z/, {}, "popen and threads");
if (!defined fork) { die "can't fork" }
for(1..100) {
  print "zzzzzzzzzzzzz\n";
  my $r=`perl -v`;
  print $r if($r=~/zzzzzzzzzzzzz/);
}
PERL

done_testing();
