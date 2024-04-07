use strict;
use warnings;
use Config;
use IPC::Open3 1.0103 qw(open3);

BEGIN {
    if ($^O eq 'VMS') {
        print "1..0 # IPC::Open3 needs porting\n";
        exit;
    }
}

my @tests=(
    # Make sure we don’t try to load modules on demand in the presence of over-
    # loaded args.  If there has been a syntax error, they won’t load.
    [   'Carp does not try to load modules on demand for overloaded args',
        "", qr/Looks lark.*o=ARRAY.* CODE/s,
    ],
    # Run the test also in the presence of
    #  a) A UNIVERSAL::can module
    #  b) A UNIVERSAL::isa module
    #  c) Both
    # since they follow slightly different code paths on old pre-5.10.1 perls.
    [   'StrVal fallback in the presence of UNIVERSAL::isa',
        'BEGIN { $UNIVERSAL::isa::VERSION = 1 }',
        qr/Looks lark.*o=ARRAY.* CODE/s,
    ],
    [   'StrVal fallback in the presence of UNIVERSAL::can',
        'BEGIN { $UNIVERSAL::can::VERSION = 1 }',
        qr/Looks lark.*o=ARRAY.* CODE/s,
    ],
    [   'StrVal fallback in the presence of UNIVERSAL::can/isa',
        'BEGIN { $UNIVERSAL::can::VERSION = $UNIVERSAL::isa::VERSION = 1 }',
        qr/Looks lark.*o=ARRAY.* CODE/s,
    ],
);

my ($test_num)= @ARGV;
if (!$test_num) {
    eval sprintf "use Test::More tests => %d; 1", 0+@tests
        or die "Failed to use Test::More: $@";
    local $ENV{PERL5LIB} = join ($Config::Config{path_sep}, @INC);
    foreach my $i (1 .. @tests) {
        my($w, $r);
        my $pid = open3($w, $r, undef, $^X, $0, $i);
        close $w;
        my $output = do{ local $/; <$r> };
        waitpid($pid, 0);
        like($output, $tests[$i-1][2], $tests[$i-1][0]);
    }
} else {
    eval $tests[$test_num-1][1] . <<'END_OF_TEST_CODE'
        no strict;
        no warnings;
        use Carp;
        sub foom {
          Carp::confess("Looks lark we got a error: $_[0]")
        }
        BEGIN {
          *{"o::()"} = sub {};
          *{'o::(""'} = sub {"hay"};
          $o::OVERLOAD{dummy}++; # perls before 5.18 need this
          *{"CODE::()"} = sub {};
          $SIG{__DIE__} = sub { foom (@_, bless([], o), sub {}) }
        }
        $a +
END_OF_TEST_CODE
    or die $@;
}
