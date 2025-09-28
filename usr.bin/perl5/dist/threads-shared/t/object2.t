use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
    if ($] < 5.010) {
        print("1..0 # SKIP Needs Perl 5.10.0 or later\n");
        exit(0);
    }
}

use ExtUtils::testlib;

BEGIN {
    $| = 1;
    print("1..133\n");   ### Number of tests that will be run ###
};

use threads;
use threads::shared;

my $TEST;
BEGIN {
    share($TEST);
    $TEST = 1;
}

sub ok {
    my ($ok, $name) = @_;

    lock($TEST);
    my $id = $TEST++;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}

ok(1, 'Loaded');

### Start of Testing ###

my $ID :shared = -1;
my (@created, @destroyed);

{ package HashObj;
   sub new {
       my $class = shift;
       my $self = &threads::shared::share({});
       $$self{'ID'} = ++$ID;
       $created[$ID] = 1;
       return bless($self, $class);
   }

   sub DESTROY {
       my $self = shift;
       $destroyed[$$self{'ID'}] = 1;
   }
}

{ package AryObj;
   sub new {
       my $class = shift;
       my $self = &threads::shared::share([]);
       $$self[0] = ++$ID;
       $created[$ID] = 1;
       return bless($self, $class);
   }

   sub DESTROY {
       my $self = shift;
       $destroyed[$$self[0]] = 1;
   }
}

{ package SclrObj;
   sub new {
       my $class = shift;
       my $self = \do{ my $scalar = ++$ID; };
       $created[$ID] = 1;
       threads::shared::share($self);
       return bless($self, $class);
   }

   sub DESTROY {
       my $self = shift;
       $destroyed[$$self] = 1;
   }
}

# Testing with normal array
my @normal_ary;

# Testing with hash object
$normal_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in normal array');
delete($normal_ary[0]);
ok($destroyed[$ID], 'Deleted hash object in normal array');

$normal_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in normal array');
$normal_ary[0] = undef;
ok($destroyed[$ID], 'Undef hash object in normal array');

$normal_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in normal array');
$normal_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in normal array');
ok($destroyed[$ID-1], 'Replaced hash object in normal array');
@normal_ary = ();
ok($destroyed[$ID], 'Hash object removed from cleared normal array');

$normal_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in normal array');
undef(@normal_ary);
ok($destroyed[$ID], 'Hash object removed from undef normal array');

# Testing with array object
$normal_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in normal array');
delete($normal_ary[0]);
ok($destroyed[$ID], 'Deleted array object in normal array');

$normal_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in normal array');
$normal_ary[0] = undef;
ok($destroyed[$ID], 'Undef array object in normal array');

$normal_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in normal array');
$normal_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in normal array');
ok($destroyed[$ID-1], 'Replaced array object in normal array');
@normal_ary = ();
ok($destroyed[$ID], 'Array object removed from cleared normal array');

$normal_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in normal array');
undef(@normal_ary);
ok($destroyed[$ID], 'Array object removed from undef normal array');

# Testing with scalar object
$normal_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal array');
delete($normal_ary[0]);
ok($destroyed[$ID], 'Deleted scalar object in normal array');

$normal_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal array');
$normal_ary[0] = undef;
ok($destroyed[$ID], 'Undef scalar object in normal array');

$normal_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal array');
$normal_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal array');
ok($destroyed[$ID-1], 'Replaced scalar object in normal array');
@normal_ary = ();
ok($destroyed[$ID], 'Scalar object removed from cleared normal array');

$normal_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal array');
undef(@normal_ary);
ok($destroyed[$ID], 'Scalar object removed from undef normal array');

# Testing with normal hash
my %normal_hash;

# Testing with hash object
$normal_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in normal hash');
delete($normal_hash{'obj'});
ok($destroyed[$ID], 'Deleted hash object in normal hash');

$normal_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in normal hash');
$normal_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef hash object in normal hash');

$normal_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in normal hash');
$normal_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in normal hash');
ok($destroyed[$ID-1], 'Replaced hash object in normal hash');
%normal_hash = ();
ok($destroyed[$ID], 'Hash object removed from cleared normal hash');

$normal_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in normal hash');
undef(%normal_hash);
ok($destroyed[$ID], 'Hash object removed from undef normal hash');

# Testing with array object
$normal_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in normal hash');
delete($normal_hash{'obj'});
ok($destroyed[$ID], 'Deleted array object in normal hash');

$normal_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in normal hash');
$normal_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef array object in normal hash');

$normal_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in normal hash');
$normal_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in normal hash');
ok($destroyed[$ID-1], 'Replaced array object in normal hash');
%normal_hash = ();
ok($destroyed[$ID], 'Array object removed from cleared normal hash');

$normal_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in normal hash');
undef(%normal_hash);
ok($destroyed[$ID], 'Array object removed from undef normal hash');

# Testing with scalar object
$normal_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal hash');
delete($normal_hash{'obj'});
ok($destroyed[$ID], 'Deleted scalar object in normal hash');

$normal_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal hash');
$normal_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef scalar object in normal hash');

$normal_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal hash');
$normal_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal hash');
ok($destroyed[$ID-1], 'Replaced scalar object in normal hash');
%normal_hash = ();
ok($destroyed[$ID], 'Scalar object removed from cleared normal hash');

$normal_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in normal hash');
undef(%normal_hash);
ok($destroyed[$ID], 'Scalar object removed from undef normal hash');

# Testing with shared array
my @shared_ary :shared;

# Testing with hash object
$shared_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in shared array');
delete($shared_ary[0]);
ok($destroyed[$ID], 'Deleted hash object in shared array');

$shared_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in shared array');
$shared_ary[0] = undef;
ok($destroyed[$ID], 'Undef hash object in shared array');

$shared_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in shared array');
$shared_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in shared array');
ok($destroyed[$ID-1], 'Replaced hash object in shared array');
@shared_ary = ();
ok($destroyed[$ID], 'Hash object removed from cleared shared array');

$shared_ary[0] = HashObj->new();
ok($created[$ID], 'Created hash object in shared array');
undef(@shared_ary);
ok($destroyed[$ID], 'Hash object removed from undef shared array');

# Testing with array object
$shared_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in shared array');
delete($shared_ary[0]);
ok($destroyed[$ID], 'Deleted array object in shared array');

$shared_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in shared array');
$shared_ary[0] = undef;
ok($destroyed[$ID], 'Undef array object in shared array');

$shared_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in shared array');
$shared_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in shared array');
ok($destroyed[$ID-1], 'Replaced array object in shared array');
@shared_ary = ();
ok($destroyed[$ID], 'Array object removed from cleared shared array');

$shared_ary[0] = AryObj->new();
ok($created[$ID], 'Created array object in shared array');
undef(@shared_ary);
ok($destroyed[$ID], 'Array object removed from undef shared array');

# Testing with scalar object
$shared_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared array');
delete($shared_ary[0]);
ok($destroyed[$ID], 'Deleted scalar object in shared array');

$shared_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared array');
$shared_ary[0] = undef;
ok($destroyed[$ID], 'Undef scalar object in shared array');

$shared_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared array');
$shared_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared array');
ok($destroyed[$ID-1], 'Replaced scalar object in shared array');
@shared_ary = ();
ok($destroyed[$ID], 'Scalar object removed from cleared shared array');

$shared_ary[0] = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared array');
undef(@shared_ary);
ok($destroyed[$ID], 'Scalar object removed from undef shared array');

# Testing with shared hash
my %shared_hash :shared;

# Testing with hash object
$shared_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in shared hash');
delete($shared_hash{'obj'});
ok($destroyed[$ID], 'Deleted hash object in shared hash');

$shared_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in shared hash');
$shared_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef hash object in shared hash');

$shared_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in shared hash');
$shared_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in shared hash');
ok($destroyed[$ID-1], 'Replaced hash object in shared hash');
%shared_hash = ();
ok($destroyed[$ID], 'Hash object removed from cleared shared hash');

$shared_hash{'obj'} = HashObj->new();
ok($created[$ID], 'Created hash object in shared hash');
undef(%shared_hash);
ok($destroyed[$ID], 'Hash object removed from undef shared hash');

# Testing with array object
$shared_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in shared hash');
delete($shared_hash{'obj'});
ok($destroyed[$ID], 'Deleted array object in shared hash');

$shared_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in shared hash');
$shared_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef array object in shared hash');

$shared_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in shared hash');
$shared_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in shared hash');
ok($destroyed[$ID-1], 'Replaced array object in shared hash');
%shared_hash = ();
ok($destroyed[$ID], 'Array object removed from cleared shared hash');

$shared_hash{'obj'} = AryObj->new();
ok($created[$ID], 'Created array object in shared hash');
undef(%shared_hash);
ok($destroyed[$ID], 'Array object removed from undef shared hash');

# Testing with scalar object
$shared_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared hash');
delete($shared_hash{'obj'});
ok($destroyed[$ID], 'Deleted scalar object in shared hash');

$shared_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared hash');
$shared_hash{'obj'} = undef;
ok($destroyed[$ID], 'Undef scalar object in shared hash');

$shared_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared hash');
$shared_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared hash');
ok($destroyed[$ID-1], 'Replaced scalar object in shared hash');
%shared_hash = ();
ok($destroyed[$ID], 'Scalar object removed from cleared shared hash');

$shared_hash{'obj'} = SclrObj->new();
ok($created[$ID], 'Created scalar object in shared hash');
undef(%shared_hash);
ok($destroyed[$ID], 'Scalar object removed from undef shared hash');

# Testing with shared scalar
{
    my $shared_scalar : shared;
    # Use a separate thread to make sure we have no private SV
    async { $shared_scalar = SclrObj->new(); }->join();
}
ok($destroyed[$ID], 'Scalar object removed from shared scalar');

#
# RT #122950 abandoning array elements (e.g. by setting $#ary)
# should trigger destructors

{
    package rt122950;

    my $count = 0;
    sub DESTROY { $count++ }

    my $n = 4;

    for my $type (0..1) {
        my @a : shared;
        $count = 0;
        push @a, bless &threads::shared::share({}) for 1..$n;
        for (1..$n) {
            { # new scope to ensure tmps are freed, destructors called
                if ($type) {
                    pop @a;
                }
                else {
                    $#a = $n - $_ - 1;
                }
            }
            ::ok($count == $_,
                "remove array object $_ by " . ($type ? "pop" : '$#a=N'));
        }
    }

    my @a : shared;
    $count = 0;
    push @a, bless &threads::shared::share({}) for 1..$n;
    {
        undef @a; # this is implemented internally as $#a = -01
    }
    ::ok($count == $n, "remove array object by undef");
}

# RT #131124
# Emptying a shared array creates new temp SVs. If there are no spare
# SVs, a new arena is allocated. shared.xs was mallocing a new arena
# with the wrong perl context set, meaning that when the arena was later
# freed, it would "panic: realloc from wrong pool"
#

{
    threads->new(sub {
        my @a :shared;
        push @a, bless &threads::shared::share({}) for 1..1000;
        undef @a; # this creates lots of temp SVs
    })->join;
    ok(1, "#131124 undef array doesnt panic");

    threads->new(sub {
        my @a :shared;
        push @a, bless &threads::shared::share({}) for 1..1000;
        @a = (); # this creates lots of temp SVs
    })->join;
    ok(1, "#131124 clear array doesnt panic");
}


# EOF
