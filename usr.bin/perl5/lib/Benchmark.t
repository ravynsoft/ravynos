#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    @INC = ('../lib');
}

use warnings;
use strict;
our ($foo, $bar, $baz, $ballast);
use Test::More;

use Benchmark qw(:all);

my $DELTA = 0.4;

# Some timing ballast
sub fib {
  my $n = shift;
  return $n if $n < 2;
  fib($n-1) + fib($n-2);
}
$ballast = 15;

my $All_Pattern =
    qr/(\d+) +wallclock secs? +\( *(-?\d+\.\d\d) +usr +(-?\d+\.\d\d) +sys +\+ +(-?\d+\.\d\d) +cusr +(-?\d+\.\d\d) +csys += +(-?\d+\.\d\d) +CPU\)/;
my $Noc_Pattern =
    qr/(\d+) +wallclock secs? +\( *(-?\d+\.\d\d) +usr +\+ +(-?\d+\.\d\d) +sys += +(-?\d+\.\d\d) +CPU\)/;
my $Nop_Pattern =
    qr/(\d+) +wallclock secs? +\( *(-?\d+\.\d\d) +cusr +\+ +(-?\d+\.\d\d) +csys += +\d+\.\d\d +CPU\)/;
# Please don't trust the matching parentheses to be useful in this :-)
my $Default_Pattern = qr/$All_Pattern|$Noc_Pattern/;

# see if the ratio of two integer values is within (1+$delta)

sub cmp_delta {
    my ($min, $max, $delta) = @_;
    ($min, $max) = ($max, $min) if $max < $min;
    return 0 if $min < 1; # avoid / 0
    return $max/$min <= (1+$delta);
}

sub splatter {
    my ($message) = @_;
    my $splatter = <<~'EOF_SPLATTER';
    Please file a ticket to report this. Our bug tracker can be found at

        https://github.com/Perl/perl5/issues

    Make sure you include the full output of perl -V, also uname -a,
    and the version details for the C compiler you are using are
    very helpful.

    Please also try compiling and running the C program that can
    be found at

        https://github.com/Perl/perl5/issues/20839#issuecomment-1439286875

    and provide the results (or compile errors) as part of your
    bug report.

    EOF_SPLATTER

    if ( $message =~ s/\.\.\.//) {
        $splatter =~ s/Please/please/;
    }
    die $message, $splatter;
}

{
    # Benchmark may end up "looping forever" if time() or times() are
    # broken such that they do not return different values over time.
    # The following crude test is intended to ensure that we can rely
    # on them and be confident that we won't infinite loop in the
    # following tests.
    #
    # You can simulate a broken time or times() function by setting
    # the appropriate env var to a true value:
    #
    #   time()    -> SIMULATE_BROKEN_TIME_FUNCTION
    #   times()   -> SIMULATE_BROKEN_TIMES_FUNCTION
    #
    # If you have a very fast box you may need to set the FAST_CPU env
    # var to a number larger than 1 to require these tests to perform
    # more iterations to see the time actually tick over. (You could
    # also set it to a value between 0 and 1 to speed this up, but I
    # don't see why you would...)
    #
    # See https://github.com/Perl/perl5/issues/20839 for the ticket
    # that motivated this test. - Yves

    my @times0;
    for ( 1 .. 3 ) {
        my $end_time = time + 1;
        my $count = 0;
        my $scale = $ENV{FAST_CPU} || 1;
        my $count_threshold = 20_000;
        while ( $ENV{SIMULATE_BROKEN_TIME_FUNCTION} || time < $end_time ) {
            my $x = 0.0;
            for ( 1 .. 10_000 ) {
                $x += sqrt(time);
            }
            if (++$count > $count_threshold * $scale) {
                last;
            }
        }
        cmp_ok($count,"<",$count_threshold * $scale,
            "expecting \$count < ($count_threshold * $scale)")
        or splatter(<<~'EOF_SPLATTER');
        Either this system is extremely fast, or the time() function
        is broken.

        If you think this system is extremely fast you may scale up the
        number of iterations allowed by this test by setting FAST_CPU=N
        in the environment. Higher N will allow more ops-per-second
        before we decide time() is broken.

        If setting a higher FAST_CPU value does not fix this problem then ...
        EOF_SPLATTER
        push @times0, $ENV{SIMULATE_BROKEN_TIMES_FUNCTION} ? 0 : (times)[0];
    }
    isnt("@times0", "0 0 0", "Make sure times() does not always return 0.")
        or splatter("It appears you have a broken a times() function.\n\n");
}

my $t0 = new Benchmark;
isa_ok ($t0, 'Benchmark', "Ensure we can create a benchmark object");

# We use the benchmark object once we've done some work:

isa_ok(timeit(5, sub {++$foo}), 'Benchmark', "timeit CODEREF");
is ($foo, 5, "benchmarked code was run 5 times");

isa_ok(timeit(5, '++$bar'), 'Benchmark', "timeit eval");
is ($bar, 5, "benchmarked code was run 5 times");

# is coderef called with spurious arguments?
timeit( 1, sub { $foo = @_ });
is ($foo, 0, "benchmarked code called without arguments");


print "# Burning CPU to benchmark things; will take time...\n";

# We need to do something fairly slow in the coderef.
# Same coderef. Same place in memory.
my $coderef = sub {$baz += fib($ballast)};

# The default is three.
$baz = 0;
my $threesecs = countit(0, $coderef);
isa_ok($threesecs, 'Benchmark', "countit 0, CODEREF");
isnt ($baz, 0, "benchmarked code was run");
my $in_threesecs = $threesecs->iters;
print "# in_threesecs=$in_threesecs iterations\n";
cmp_ok($in_threesecs, '>', 0, "iters returned positive iterations");
my $cpu3 = $threesecs->[1]; # user
my $sys3 = $threesecs->[2]; # sys
cmp_ok($cpu3+$sys3, '>=', 3.0, "3s cpu3 is at least 3s");
my $in_threesecs_adj = $in_threesecs;
$in_threesecs_adj *= (3/$cpu3); # adjust because may not have run for exactly 3s
print "# in_threesecs_adj=$in_threesecs_adj adjusted iterations\n";

my $estimate = int (100 * $in_threesecs_adj / 3) / 100;
print "# from the 3 second run estimate $estimate iterations in 1 second...\n";
$baz = 0;
my $onesec = countit(1, $coderef);
isa_ok($onesec, 'Benchmark', "countit 1, CODEREF");
isnt ($baz, 0, "benchmarked code was run");
my $in_onesec = $onesec->iters;
print "# in_onesec=$in_onesec iterations\n";
cmp_ok($in_onesec, '>',  0, "iters returned positive iterations");
my $cpu1 = $onesec->[1]; # user
my $sys1 = $onesec->[2]; # sys
cmp_ok($cpu1+$sys1, '>=', 1.0, "is cpu1 is at least 1s");
my $in_onesec_adj = $in_onesec;
$in_onesec_adj *= (1/$cpu1); # adjust because may not have run for exactly 1s
print "# in_onesec_adj=$in_onesec_adj adjusted iterations\n";


# I found that the eval'ed version was 3 times faster than the coderef.
# (now it has a different ballast value)
$baz = 0;
my $again = countit(1, '$baz += fib($ballast)');
isa_ok($onesec, 'Benchmark', "countit 1, eval");
isnt ($baz, 0, "benchmarked code was run");
my $in_again = $again->iters;
print "# $in_again iterations\n";
cmp_ok($in_again, '>', 0, "iters returned positive iterations");


my $t1 = new Benchmark;
isa_ok ($t1, 'Benchmark', "Create another benchmark object now we're finished");

my $diff = timediff ($t1, $t0);
isa_ok ($diff, 'Benchmark', "Get the time difference");
isa_ok (timesum ($t0, $t1), 'Benchmark', "check timesum");

my $default = timestr ($diff);
isnt ($default, '', 'timestr ($diff)');
my $auto = timestr ($diff, 'auto');
is ($auto, $default, 'timestr ($diff, "auto") matches timestr ($diff)');

{
    my $all = timestr ($diff, 'all');
    like ($all, $All_Pattern, 'timestr ($diff, "all")');
    print "# $all\n";

    my ($wallclock, $usr, $sys, $cusr, $csys, $cpu) = $all =~ $All_Pattern;

    is (timestr ($diff, 'none'), '', "none suppresses output");

    my $noc = timestr ($diff, 'noc');
    like ($noc, qr/$wallclock +wallclock secs? +\( *$usr +usr +\+ +$sys +sys += +\d+\.\d\d +CPU\)/, 'timestr ($diff, "noc")');

    my $nop = timestr ($diff, 'nop');
    like ($nop, qr/$wallclock +wallclock secs? +\( *$cusr +cusr +\+ +$csys +csys += +\d+\.\d\d +CPU\)/, 'timestr ($diff, "nop")');

    if ($auto eq $noc) {
        pass ('"auto" is "noc"');
    } else {
        is ($auto, $all, '"auto" isn\'t "noc", so should be eq to "all"');
    }

    like (timestr ($diff, 'all', 'E'),
          qr/(\d+) +wallclock secs? +\( *\d\.\d+E[-+]?\d\d\d? +usr +\d\.\d+E[-+]?\d\d\d? +sys +\+ +\d\.\d+E[-+]?\d\d\d? +cusr +\d\.\d+E[-+]?\d\d\d? +csys += +\d\.\d+E[-+]?\d\d\d? +CPU\)/, 'timestr ($diff, "all", "E") [sprintf format of "E"]');
}

my $out = tie *OUT, 'TieOut';

my $iterations = 100;

$foo = 0;
select(OUT);
my $got = timethis($iterations, sub {++$foo});
select(STDOUT);
isa_ok($got, 'Benchmark', "timethis CODEREF");
is ($foo, $iterations, "benchmarked code was run $iterations times");

$got = $out->read();
like ($got, qr/^timethis $iterations/, 'default title');
like ($got, $Default_Pattern, 'default format is all or noc');

$bar = 0;
select(OUT);
$got = timethis($iterations, '++$bar');
select(STDOUT);
isa_ok($got, 'Benchmark', "timethis eval");
is ($bar, $iterations, "benchmarked code was run $iterations times");

$got = $out->read();
like ($got, qr/^timethis $iterations/, 'default title');
like ($got, $Default_Pattern, 'default format is all or noc');

my $title = 'lies, damn lies and benchmarks';
$foo = 0;
select(OUT);
$got = timethis($iterations, sub {++$foo}, $title);
select(STDOUT);
isa_ok($got, 'Benchmark', "timethis with title");
is ($foo, $iterations, "benchmarked code was run $iterations times");

$got = $out->read();
like ($got, qr/^$title:/, 'specify title');
like ($got, $Default_Pattern, 'default format is all or noc');

# default is auto, which is all or noc. nop can never match the default
$foo = 0;
select(OUT);
$got = timethis($iterations, sub {++$foo}, $title, 'nop');
select(STDOUT);
isa_ok($got, 'Benchmark', "timethis with format");
is ($foo, $iterations, "benchmarked code was run $iterations times");

$got = $out->read();
like ($got, qr/^$title:/, 'specify title');
like ($got, $Nop_Pattern, 'specify format as nop');

{
    $foo = 0;
    select(OUT);
    my $start = time;
    $got = timethis(-2, sub {$foo+= fib($ballast)}, $title, 'none');
    my $end = time;
    select(STDOUT);
    isa_ok($got, 'Benchmark',
           "timethis, at least 2 seconds with format 'none'");
    cmp_ok($foo, '>', 0, "benchmarked code was run");
    cmp_ok($end - $start, '>', 1, "benchmarked code ran for over 1 second");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;
    $got =~ s/^[ \t\n]+//s; # Remove all the whitespace from the beginning

    is ($got, '', "format 'none' should suppress output");
}

$foo = $bar = $baz = 0;
select(OUT);
$got = timethese($iterations, { Foo => sub {++$foo}, Bar => '++$bar',
                                Baz => sub {++$baz} });
select(STDOUT);
is(ref ($got), 'HASH', "timethese should return a hashref");
isa_ok($got->{Foo}, 'Benchmark', "Foo value");
isa_ok($got->{Bar}, 'Benchmark', "Bar value");
isa_ok($got->{Baz}, 'Benchmark', "Baz value");
is_deeply([sort keys %$got], [sort qw(Foo Bar Baz)], 'should be exactly three objects');
is ($foo, $iterations, "Foo code was run $iterations times");
is ($bar, $iterations, "Bar code was run $iterations times");
is ($baz, $iterations, "Baz code was run $iterations times");

$got = $out->read();
# Remove any warnings about having too few iterations.
$got =~ s/\(warning:[^\)]+\)//gs;

like ($got, qr/timing $iterations iterations of\s+Bar\W+Baz\W+Foo\W*?\.\.\./s,
      'check title');
# Remove the title
$got =~ s/.*\.\.\.//s;
like ($got, qr/\bBar\b.*\bBaz\b.*\bFoo\b/s, 'check output is in sorted order');
like ($got, $Default_Pattern, 'should find default format somewhere');


{ # ensure 'use strict' does not leak from Benchmark.pm into benchmarked code
    no strict;
    select OUT;

    eval {
        timethese( 1,
                   { undeclared_var => q{ $i++; $i-- },
                     symbolic_ref   => q{ $bar = 42;
                                          $foo = 'bar';
                                          $q = ${$foo} },
                   },
                   'none'
                  );

    };
    is( $@, '', q{no strict leakage in name => 'code'} );

    eval {
        timethese( 1,
                   { undeclared_var => sub { $i++; $i-- },
                     symbolic_ref   => sub { $bar = 42;
                                             $foo = 'bar';
                                             return ${$foo} },
                   },
                   'none'
                 );
    };
    is( $@, '', q{no strict leakage in name => sub { code }} );

    # clear out buffer
    $out->read;
}


my $code_to_test =  { Foo => sub {$foo+=fib($ballast-2)},
                      Bar => sub {$bar+=fib($ballast)}};
# Keep these for later.
my $results;
{
    $foo = $bar = 0;
    select(OUT);
    my $start = times;
    $results = timethese(-0.1, $code_to_test, 'none');
    my $end = times;
    select(STDOUT);

    is(ref ($results), 'HASH', "timethese should return a hashref");
    isa_ok($results->{Foo}, 'Benchmark', "Foo value");
    isa_ok($results->{Bar}, 'Benchmark', "Bar value");
    is_deeply([sort keys %$results], [sort qw(Foo Bar)], 'should be exactly two objects');
    cmp_ok($foo, '>', 0, "Foo code was run");
    cmp_ok($bar, '>', 0, "Bar code was run");

    cmp_ok($end-$start, '>', 0.1, "benchmarked code ran for over 0.1 seconds");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;
    is ($got =~ tr/ \t\n//c, 0, "format 'none' should suppress output");
}
my $graph_dissassembly =
    qr!^[ \t]+(\S+)[ \t]+(\w+)[ \t]+(\w+)[ \t]*		# Title line
    \n[ \t]*(\w+)[ \t]+([0-9.]+(?:/s)?)[ \t]+(-+)[ \t]+(-?\d+%)[ \t]*
    \n[ \t]*(\w+)[ \t]+([0-9.]+(?:/s)?)[ \t]+(-?\d+%)[ \t]+(-+)[ \t]*$!xm;

sub check_graph_consistency {
    my (	$ratetext, $slowc, $fastc,
        $slowr, $slowratet, $slowslow, $slowfastt,
        $fastr, $fastratet, $fastslowt, $fastfast)
        = @_;
    note("calling check_graph_consistency from line " . (caller(1))[2]);
    my $all_passed = 1;
    $all_passed
      &= is ($slowc, $slowr, "left col tag should be top row tag");
    $all_passed
      &= is ($fastc, $fastr, "right col tag should be bottom row tag");
    $all_passed &=
      like ($slowslow, qr/^-+/, "should be dash for comparing slow with slow");
    $all_passed
      &= is ($slowslow, $fastfast, "slow v slow should be same as fast v fast");
    my $slowrate = $slowratet;
    my $fastrate = $fastratet;
    my ($slow_is_rate, $fast_is_rate);
    unless ($slow_is_rate = $slowrate =~ s!/s!!) {
        # Slow is expressed as iters per second.
        $slowrate = 1/$slowrate if $slowrate;
    }
    unless ($fast_is_rate = $fastrate =~ s!/s!!) {
        # Fast is expressed as iters per second.
        $fastrate = 1/$fastrate if $fastrate;
    }
    if ($ratetext =~ /rate/i) {
        $all_passed
          &= ok ($slow_is_rate, "slow should be expressed as a rate");
        $all_passed
          &= ok ($fast_is_rate, "fast should be expressed as a rate");
    } else {
        $all_passed &=
          ok (!$slow_is_rate, "slow should be expressed as a iters per second");
        $all_passed &=
          ok (!$fast_is_rate, "fast should be expressed as a iters per second");
    }

    (my $slowfast = $slowfastt) =~ s!%!!;
    (my $fastslow = $fastslowt) =~ s!%!!;
    if ($slowrate < $fastrate) {
        pass ("slow rate is less than fast rate");
        unless (ok ($slowfast <= 0 && $slowfast >= -100,
                    "slowfast should be less than or equal to zero, and >= -100")) {
          diag("slowfast=$slowfast");
          $all_passed = 0;
        }
        unless (cmp_ok($fastslow, '>', 0, "fastslow should be > 0")) {
          $all_passed = 0;
        }
    } else {
        $all_passed
          &= is ($slowrate, $fastrate,
                 "slow rate isn't less than fast rate, so should be the same");
	# In OpenBSD the $slowfast is sometimes a really, really, really
	# small number less than zero, and this gets stringified as -0.
        $all_passed
          &= like ($slowfast, qr/^-?0$/, "slowfast should be zero");
        $all_passed
          &= like ($fastslow, qr/^-?0$/, "fastslow should be zero");
    }
    return $all_passed;
}

sub check_graph_vs_output {
    my ($chart, $got) = @_;
    my (	$ratetext, $slowc, $fastc,
        $slowr, $slowratet, $slowslow, $slowfastt,
        $fastr, $fastratet, $fastslowt, $fastfast)
        = $got =~ $graph_dissassembly;
    my $all_passed
      = check_graph_consistency (        $ratetext, $slowc, $fastc,
                                 $slowr, $slowratet, $slowslow, $slowfastt,
                                 $fastr, $fastratet, $fastslowt, $fastfast);
    $all_passed
      &= is_deeply ($chart, [['', $ratetext, $slowc, $fastc],
                             [$slowr, $slowratet, $slowslow, $slowfastt],
                             [$fastr, $fastratet, $fastslowt, $fastfast]],
                    "check the chart layout matches the formatted output");
    unless ($all_passed) {
      diag("Something went wrong there. I got this chart:\n$got");
    }
}

sub check_graph {
    my ($title, $row1, $row2) = @_;
    is (scalar @$title, 4, "Four entries in title row");
    is (scalar @$row1, 4, "Four entries in first row");
    is (scalar @$row2, 4, "Four entries in second row");
    is (shift @$title, '', "First entry of output graph should be ''");
    check_graph_consistency (@$title, @$row1, @$row2);
}

{
    select(OUT);
    my $start = times;
    my $chart = cmpthese( -0.1, { a => "\$i = sqrt(\$i++) * sqrt(\$i) for 1..10",
                                  b => "\$i = sqrt(\$i++)",
                                }, "auto" ) ;
    my $end = times;
    select(STDOUT);
    cmp_ok($end - $start, '>', 0.05,
                            "benchmarked code ran for over 0.05 seconds");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;

    like ($got, qr/running\W+a\W+b.*?for at least 0\.1 CPU second/s,
          'check title');
    # Remove the title
    $got =~ s/.*\.\.\.//s;
    like ($got, $Default_Pattern, 'should find default format somewhere');
    like ($got, $graph_dissassembly, "Should find the output graph somewhere");
    check_graph_vs_output ($chart, $got);
}

# Not giving auto should suppress timethese results.
{
    select(OUT);
    my $start = times;
    my $chart = cmpthese( -0.1, { a => "\$i = sqrt(\$i++) * sqrt(\$i) for 1..10",
                                  b => "\$i = sqrt(\$i++)" });
    my $end = times;
    select(STDOUT);
    cmp_ok($end - $start, '>', 0.05,
            "benchmarked code ran for over 0.05 seconds");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;

    unlike ($got, qr/running\W+a\W+b.*?for at least 0\.1 CPU second/s,
          'should not have title');
    # Remove the title
    $got =~ s/.*\.\.\.//s;
    unlike ($got, $Default_Pattern, 'should not find default format somewhere');
    like ($got, $graph_dissassembly, "Should find the output graph somewhere");
    check_graph_vs_output ($chart, $got);
}

{
    $foo = $bar = 0;
    select(OUT);
    my $chart = cmpthese($iterations, $code_to_test, 'nop' ) ;
    select(STDOUT);
    cmp_ok($foo, '>', 0, "Foo code was run");
    cmp_ok($bar, '>', 0, "Bar code was run");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;
    like ($got, qr/timing $iterations iterations of\s+Bar\W+Foo\W*?\.\.\./s,
      'check title');
    # Remove the title
    $got =~ s/.*\.\.\.//s;
    like ($got, $Nop_Pattern, 'specify format as nop');
    like ($got, $graph_dissassembly, "Should find the output graph somewhere");
    check_graph_vs_output ($chart, $got);
}

{
    $foo = $bar = 0;
    select(OUT);
    my $chart = cmpthese($iterations, $code_to_test, 'none' ) ;
    select(STDOUT);
    cmp_ok($foo, '>', 0, "Foo code was run");
    cmp_ok($bar, '>', 0, "Bar code was run");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;
    $got =~ s/^[ \t\n]+//s; # Remove all the whitespace from the beginning
    is ($got, '', "format 'none' should suppress output");
    is (ref $chart, 'ARRAY', "output should be an array ref");
    # Some of these will go bang if the preceding test fails. There will be
    # a big clue as to why, from the previous test's diagnostic
    is (ref $chart->[0], 'ARRAY', "output should be an array of arrays");
    check_graph (@$chart);
}

# this is a repeat of the above test, but with the timing and charting
# steps split.

{
    $foo = $bar = 0;
    select(OUT);
    my $res = timethese($iterations, $code_to_test, 'none' ) ;
    my $chart = cmpthese($res, 'none' ) ;
    select(STDOUT);
    cmp_ok($foo, '>', 0, "Foo code was run");
    cmp_ok($bar, '>', 0, "Bar code was run");

    $got = $out->read();
    # Remove any warnings about having too few iterations.
    $got =~ s/\(warning:[^\)]+\)//gs;
    $got =~ s/^[ \t\n]+//s; # Remove all the whitespace from the beginning
    is ($got, '', "format 'none' should suppress output");
    is (ref $chart, 'ARRAY', "output should be an array ref");
    # Some of these will go bang if the preceding test fails. There will be
    # a big clue as to why, from the previous test's diagnostic
    is (ref $chart->[0], 'ARRAY', "output should be an array of arrays");
    use Data::Dumper;
    check_graph(@$chart)
        or diag(Data::Dumper->Dump([$res, $chart], ['$res', '$chart']));
}

{
    $foo = $bar = 0;
    select(OUT);
    my $chart = cmpthese( $results ) ;
    select(STDOUT);
    is ($foo, 0, "Foo code was not run");
    is ($bar, 0, "Bar code was not run");

    $got = $out->read();
    unlike($got, qr/\.\.\./s, 'check that there is no title');
    like ($got, $graph_dissassembly, "Should find the output graph somewhere");
    check_graph_vs_output ($chart, $got);
}

{
    $foo = $bar = 0;
    select(OUT);
    my $chart = cmpthese( $results, 'none' ) ;
    select(STDOUT);
    is ($foo, 0, "Foo code was not run");
    is ($bar, 0, "Bar code was not run");

    $got = $out->read();
    is ($got, '', "'none' should suppress all output");
    is (ref $chart, 'ARRAY', "output should be an array ref");
    # Some of these will go bang if the preceding test fails. There will be
    # a big clue as to why, from the previous test's diagnostic
    is (ref $chart->[0], 'ARRAY', "output should be an array of arrays");
    check_graph (@$chart);
}

###}my $out = tie *OUT, 'TieOut'; my ($got); ###

my $debug = tie *STDERR, 'TieOut';

$bar = 0;
isa_ok(timeit(5, '++$bar'), 'Benchmark', "timeit eval");
is ($bar, 5, "benchmarked code was run 5 times");
is ($debug->read(), '', "There was no debug output");

Benchmark->debug(1);

$bar = 0;
select(OUT);
$got = timeit(5, '++$bar');
select(STDOUT);
isa_ok($got, 'Benchmark', "timeit eval");
is ($bar, 5, "benchmarked code was run 5 times");
is ($out->read(), '', "There was no STDOUT output with debug enabled");
isnt ($debug->read(), '', "There was STDERR debug output with debug enabled");

Benchmark->debug(0);

$bar = 0;
isa_ok(timeit(5, '++$bar'), 'Benchmark', "timeit eval");
is ($bar, 5, "benchmarked code was run 5 times");
is ($debug->read(), '', "There was no debug output debug disabled");

undef $debug;
untie *STDERR;

# To check the cache we are poking where we don't belong, inside the namespace.
# The way benchmark is written we can't actually check whether the cache is
# being used, merely what's become cached.

clearallcache();
my @before_keys = keys %Benchmark::Cache;
$bar = 0;
isa_ok(timeit(5, '++$bar'), 'Benchmark', "timeit eval");
is ($bar, 5, "benchmarked code was run 5 times");
my @after5_keys = keys %Benchmark::Cache;
$bar = 0;
isa_ok(timeit(10, '++$bar'), 'Benchmark', "timeit eval");
is ($bar, 10, "benchmarked code was run 10 times");
cmp_ok (scalar keys %Benchmark::Cache, '>', scalar @after5_keys, "10 differs from 5");

clearcache(10);
# Hash key order will be the same if there are the same keys.
is_deeply ([keys %Benchmark::Cache], \@after5_keys,
           "cleared 10, only cached results for 5 should remain");

clearallcache();
is_deeply ([keys %Benchmark::Cache], \@before_keys,
           "back to square 1 when we clear the cache again?");


{   # Check usage error messages
    my %usage = %Benchmark::_Usage;
    delete $usage{runloop};  # not public, not worrying about it just now

    my @takes_no_args = qw(clearallcache disablecache enablecache);

    my %cmpthese = ('forgot {}' => 'cmpthese( 42, foo => sub { 1 } )',
                     'not result' => 'cmpthese(42)',
                     'array ref'  => 'cmpthese( 42, [ foo => sub { 1 } ] )',
                    );
    while( my($name, $code) = each %cmpthese ) {
        eval $code;
        is( $@, $usage{cmpthese}, "cmpthese usage: $name" );
    }

    my %timethese = ('forgot {}'  => 'timethese( 42, foo => sub { 1 } )',
                       'no code'    => 'timethese(42)',
                       'array ref'  => 'timethese( 42, [ foo => sub { 1 } ] )',
                      );

    while( my($name, $code) = each %timethese ) {
        eval $code;
        is( $@, $usage{timethese}, "timethese usage: $name" );
    }


    while( my($func, $usage) = each %usage ) {
        next if grep $func eq $_, @takes_no_args;
        eval "$func()";
        is( $@, $usage, "$func usage: no args" );
    }

    foreach my $func (@takes_no_args) {
        eval "$func(42)";
        is( $@, $usage{$func}, "$func usage: with args" );
    }
}

done_testing();

package TieOut;

sub TIEHANDLE {
    my $class = shift;
    bless(\( my $ref = ''), $class);
}

sub PRINT {
    my $self = shift;
    $$self .= join('', @_);
}

sub PRINTF {
    my $self = shift;
    $$self .= sprintf shift, @_;
}

sub read {
    my $self = shift;
    return substr($$self, 0, length($$self), '');
}
