use strict;
use warnings;

use Thread::Semaphore;

use Test::More 'tests' => 9;

my $err = qr/^Semaphore initializer is not an integer: /;

eval { Thread::Semaphore->new(undef); };
like($@, $err, $@);
eval { Thread::Semaphore->new(0.5); };
like($@, $err, $@);
eval { Thread::Semaphore->new('foo'); };
like($@, $err, $@);

my $s = Thread::Semaphore->new();
ok($s, 'New semaphore');

$err = qr/^Argument to semaphore method .* is not a positive integer: /;

eval { $s->down(undef); };
like($@, $err, $@);
eval { $s->down(0); };
like($@, $err, $@);
eval { $s->down(-1); };
like($@, $err, $@);
eval { $s->down(1.5); };
like($@, $err, $@);
eval { $s->down('foo'); };
like($@, $err, $@);

# No need to test ->up(), etc. as the arg validation code is common to them all

exit(0);

# EOF
