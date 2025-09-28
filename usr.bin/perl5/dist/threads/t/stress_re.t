use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

my $test = 0;
sub ok {
    my ($ok, $name) = @_;
    $test++;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $test - $name\n");
    } else {
        print("not ok $test - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}

BEGIN {
    $| = 1;
    print("1..61\n");   ### Number of tests that will be run ###
};

use threads;
ok(1, 'Loaded');

### Start of Testing ###

my $cnt = 30;

sub stress_re {
    my $s = "abcd" x (1000 + $_[0]);
    my $t = '';
    while ($s =~ /(.)/g) { $t .= $1 }
    return ($s eq $t) ? 'ok' : 'not';
}

my @threads;
for (1..$cnt) {
    my $thr = threads->create('stress_re', $_);
    ok($thr, "Thread created - iter $_");
    push(@threads, $thr);
}

for (1..$cnt) {
    my ($result, $thr);
    $thr = $threads[$_-1];
    $result = $thr->join if $thr;
    ok($thr && defined($result) && ($result eq 'ok'), "Thread joined - iter $_");
}

exit(0);

# EOF
