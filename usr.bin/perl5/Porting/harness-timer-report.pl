#!perl -w
#
# harness-timer-report.pl
#
# - read in the time-annotated outputs of
#   "env HARNESS_TIMER=1 make test" or
#   "make test_harness"
# - convert the milliseconds to seconds
# - compute a couple of derived values
#   - cpu: the sum of 'self' and 'kids'
#   - ratio of the wallclock and the cpu
# - optionally show header, the sum, or the max of each colum
# - sort the rows in various ways
#   - default ordering by 'cpu' seconds
# - optionally scale the column values by either the sum or the max
# - optionally display only rows that have rows of at least / at most a limit
#
# The --sort option has a few canned sorting rules.  If those are
# not to your liking, there is always sort(1).
#
# Example usages:
#
# perl harness-timer-report.pl log
# perl harness-timer-report.pl --sort=wall log
# perl harness-timer-report.pl --scale=sum log
# perl harness-timer-report.pl --scale=sum --min=0.01 log
# perl harness-timer-report.pl --show=header,max,sum log
# perl harness-timer-report.pl --min=wall=10 log

use strict;
use warnings;

use File::Basename qw[basename];

our $ME = basename($0);

use Getopt::Long;

sub usage {
    die <<__EOF__;
$ME: Usage:
$ME [--scale=[sum|max]]
    [--sort=[cpu|wall|ratio|self|kids|test|name]]
    [--show=header,sum,max]
    [--min=[[cpu|wall|ratio|self|kids]=value,...]]
    [--max=[[cpu|wall|ratio|self|kids]=value,...]]
    [--order]
    [--help|--usage]
    [logfile]

The --order includes the original test order as the last column.
The logfile default is STDIN.
__EOF__
}

my %Opt;
usage()
    unless
    GetOptions(
	'scale=s' => \$Opt{scale},
	'sort=s'  => \$Opt{sort},
	'show=s' => \$Opt{show},
	'min=s' => \$Opt{min},
	'max=s' => \$Opt{max},
	'order' => \$Opt{order},
	'help|usage' => \$Opt{help},
    );
usage() if $Opt{help};

my %SHOW;
if (defined $Opt{show}) {
    for my $s (split(/,/, $Opt{show})) {
	if ($s =~ /^(header|sum|max)$/) {
	    $SHOW{$s}++;
	} else {
	    die "$ME: Unexpected --show='$s'\n";
	}
    }
}
my %MIN;
if (defined $Opt{min}) {
    for my $s (split(/,/, $Opt{min})) {
	if ($s =~ /^(wall|cpu|kids|self|ratio)=(\d+(?:\.\d+)?)$/) {
	    $MIN{$1} = $2;
	} else {
	    die "$ME: Unexpected --min='$s'\n";
	}
    }
}
my %MAX;
if (defined $Opt{max}) {
    for my $s (split(/,/, $Opt{max})) {
	if ($s =~ /^(wall|cpu|kids|self|ratio)=(\d+(?:\.\d+)?)$/) {
	    $MAX{$1} = $2;
	} else {
	    die "$ME: Unexpected --max='$s'\n";
	}
    }
}

use List::Util qw[max];

my ($sa, $sb, $sc, $sd, $se);
my ($ma, $mb, $mc, $md, $me);

my $logfn;
my $logfh;
if (@ARGV == 1) {
    $logfn = $ARGV[0];
    open($logfh, "<", $logfn) or die "$ME: Failed to open logfn: $logfn\n";
} elsif (@ARGV == 0) {
    $logfn = "-";
    $logfh = *STDIN;
} else {
    die "$ME: Unexpected logfile arguments: @ARGV\n";
}

my $order = 0;
my @t;

while (<$logfh>) {
    my ($test, $wall, $self, $kids);
    # Output of "env HARNESS_TIMER=1 make test":
    # t/re/pat ....................................................... ok     2876 ms  2660 ms   210 ms
    if (m{^#\s+(\S+)\s+\.+\s+ok\s+(\d+)\s+ms\s+(\d+)\s+ms\s+(\d+)\s+ms$}) {
	($test, $wall, $self, $kids) = ($1, $2, $3, $4);
    }
    # Output of "env HARNESS_TIMER=1 make test_harness":
    # [08:26:11] base/cond.t ........................................................ ok        2 ms ( 0.00 usr  0.00 sys +  0.00 cusr  0.00 csys =  0.00 CPU)
    if (m{^\[.+?\]+\s+(\S+)\s+\.+\s+ok\s+(\d+)\s+ms\s+\(\s*(\d+\.\d+)\s+usr\s+\s+(\d+\.\d+)\s+sys\s+\+\s+(\d+\.\d+)\s+cusr\s+(\d+\.\d+)\s+csys\s+=\s+(\d+\.\d+)\s+CPU\)}) {
        $test = $1;
        $wall = $2;
        $self = $3 + $4;
        $kids = $5 + $6;
        $test =~ s{^\.\./}{};  # "../lib/foo" -> "../lib/foo"
    }
    next unless defined $test && defined $wall && $wall > 0;
    # Milliseconds to seconds.
    $wall /= 1000;
    $self /= 1000;
    $kids /= 1000;
    my $cpu = $self + $kids;
    my $ratio = $cpu / $wall;
    push @t, [ $test, $wall, $self, $kids, $cpu, $ratio, $order++ ];
    $sa += $wall;
    $sb += $self;
    $sc += $kids;
    $sd += $cpu;
    $ma = max($wall,  $ma // $wall);
    $mb = max($self,  $mb // $self);
    $mc = max($kids,  $mc // $kids);
    $md = max($cpu,   $md // $cpu);
    $me = max($ratio, $md // $ratio);
}

die "$ME: No input detected in '$logfn'\n" unless @t;

# Compute the sum for the ratio only after the loop.
$se = $sd / $sa;

my %SORTER =
    (
     'cpu' =>
      sub { $b->[4] <=> $a->[4] ||
	    $b->[1] <=> $a->[1] ||
	    $a->[0] cmp $b->[0] },
     'wall' =>
      sub { $b->[1] <=> $a->[1] ||
	    $b->[4] <=> $a->[4] ||
	    $a->[0] cmp $b->[0] },
     'ratio' =>
      sub { $b->[5] <=> $a->[5] ||
	    $b->[4] <=> $a->[4] ||
	    $b->[1] <=> $a->[1] ||
	    $a->[0] cmp $b->[0] },
     'self' =>
      sub { $b->[2] <=> $a->[2] ||
	    $b->[3] <=> $a->[3] ||
	    $a->[0] cmp $b->[0] },
     'kids' =>
      sub { $b->[3] <=> $a->[3] ||
	    $b->[2] <=> $a->[2] ||
	    $a->[0] cmp $b->[0] },
     'test' =>
      sub { $a->[6] <=> $b->[6] },
     'name' =>
      sub { $a->[0] cmp $b->[0] },
    );
my $sorter;

$Opt{sort} //= 'cpu';

die "$ME: Unexpected --sort='$Opt{sort}'\n"
    unless defined $SORTER{$Opt{sort}};

@t = sort { $SORTER{$Opt{sort}}->() } @t;

if (defined $Opt{scale}) {
    my ($ta, $tb, $tc, $td, $te) =
	$Opt{scale} eq 'sum' ?
	($sa, $sb, $sc, $sd, $se) :
	$Opt{scale} eq 'max' ?
	($ma, $mb, $mc, $md, $me) :
	die "$ME: Unexpected --scale='$Opt{scale}'";

    my @u;
    for my $t (@t) {
    push @u, [ $t->[0],
	       $t->[1] / $ta, $t->[2] / $tb,
	       $t->[3] / $tc, $t->[4] / $td,
               $t->[5] / $te, $t->[6] ];
    }
    @t = @u;
}

if ($SHOW{header}) {
    my @header = qw[TEST WALL SELF KIDS CPU RATIO];
    if ($Opt{order}) {
        push @header, 'ORDER';
    }
    print join(" ", @header), "\n";
}
if ($SHOW{sum}) {
    print join(" ", "SUM",
	       map { sprintf("%.6f", $_) } $sa, $sb, $sc, $sd, $se),
          "\n";
}
if ($SHOW{max}) {
    print join(" ", "MAX",
	       map { sprintf("%.6f", $_) } $ma, $mb, $mc, $md, $me),
          "\n";
}

my %N2I = (wall  => 1,
	   self  => 2,
	   kids  => 3,
	   cpu   => 4,
	   ratio => 5);

sub row_is_skippable {
    my ($t) = @_;
    if (scalar keys %MIN) {
	for my $k (grep { exists $MIN{$_} } keys %N2I) {
	    if ($t->[$N2I{$k}] < $MIN{$k}) {
		return 1;
	    }
	}
    }
    if (scalar keys %MAX) {
	for my $k (grep { exists $MAX{$_} } keys %N2I) {
	    if ($t->[$N2I{$k}] > $MAX{$k}) {
		return 1;
	    }
	}
    }
    return 0;
}

for my $t (@t) {
    next if row_is_skippable($t);
    my $out = sprintf("%s %.6f %.6f %.6f %.6f %.6f",
                      $t->[0], $t->[1], $t->[2], $t->[3], $t->[4], $t->[5]);
    if ($Opt{order}) {
        $out .= " $t->[6]";
    }
    print $out, "\n";
}

exit(0);
