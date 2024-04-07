use strict;
use warnings;

BEGIN {
    require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');

    use Config;
    if (! $Config{'useithreads'}) {
        skip_all(q/Perl not compiled with 'useithreads'/);
    }

    plan(10);
}

use ExtUtils::testlib;

use_ok('threads');

### Start of Testing ###

no warnings 'threads';

# Create a thread that generates an error
my $thr = threads->create(sub { my $x = Foo->new(); });

# Check that thread returns 'undef'
my $result = $thr->join();
ok(! defined($result), 'thread died');

# Check error
like($thr->error(), qr/^Can't locate object method/s, 'thread error');


# Create a thread that 'die's with an object
$thr = threads->create(sub {
                    threads->yield();
                    sleep(1);
                    die(bless({ error => 'bogus' }, 'Err::Class'));
                });

my $err = $thr->error();
ok(! defined($err), 'no error yet');

# Check that thread returns 'undef'
$result = $thr->join();
ok(! defined($result), 'thread died');

# Check that error object is retrieved
$err = $thr->error();
isa_ok($err, 'Err::Class', 'error object');
is($err->{error}, 'bogus', 'error field');

# Check that another thread can reference the error object
my $thrx = threads->create(sub { die(bless($thr->error(), 'Foo')); });

# Check that thread returns 'undef'
$result = $thrx->join();
ok(! defined($result), 'thread died');

# Check that the rethrown error object is retrieved
$err = $thrx->error();
isa_ok($err, 'Foo', 'error object');
is($err->{error}, 'bogus', 'error field');

exit(0);

# EOF
