use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
    if ($] >= 5.027000) {
        print("1..0 # SKIP 'unique' attribute no longer exists\n");
        exit(0);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        print("1..0 # SKIP threads::shared not available\n");
        exit(0);
    }

    $| = 1;
    print("1..6\n") ;           ### Number of tests that will be run ###
}

print("ok 1 - Loaded\n");

### Start of Testing ###

no warnings 'deprecated';       # Suppress warnings related to :unique

my $test :shared = 2;

# bugid 24383 - :unique hashes weren't being made readonly on interpreter
# clone; check that they are.

our $unique_scalar : unique;
our @unique_array : unique;
our %unique_hash : unique;
threads->create(sub {
        lock($test);
        my $TODO = ":unique needs to be re-implemented in a non-broken way";
        eval { $unique_scalar = 1 };
        print $@ =~ /read-only/
          ? '' : 'not ', "ok $test # TODO $TODO - unique_scalar\n";
        $test++;
        eval { $unique_array[0] = 1 };
        print $@ =~ /read-only/
          ? '' : 'not ', "ok $test # TODO $TODO - unique_array\n";
        $test++;
        if ($] >= 5.008003 && $^O ne 'MSWin32') {
            eval { $unique_hash{abc} = 1 };
            print $@ =~ /disallowed/
              ? '' : 'not ', "ok $test # TODO $TODO - unique_hash\n";
        } else {
            print("ok $test # SKIP $TODO - unique_hash\n");
        }
        $test++;
    })->join;

# bugid #24940 :unique should fail on my and sub declarations

for my $decl ('my $x : unique', 'sub foo : unique') {
    {
        lock($test);
        if ($] >= 5.008005) {
            eval $decl;
            print $@ =~ /^The 'unique' attribute may only be applied to 'our' variables/
                    ? '' : 'not ', "ok $test - $decl\n";
        } else {
            print("ok $test # SKIP $decl\n");
        }
        $test++;
    }
}


