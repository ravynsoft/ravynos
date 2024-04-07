#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib', '.');
}   
# Avoid using eq_array below as it uses .. internally.

use Config;

plan (162);

is(join(':',1..5), '1:2:3:4:5');

@foo = (1,2,3,4,5,6,7,8,9);
@foo[2..4] = ('c','d','e');

is(join(':',@foo[$foo[0]..5]), '2:c:d:e:6');

@bar[2..4] = ('c','d','e');
is(join(':',@bar[1..5]), ':c:d:e:');

($a,@bcd[0..2],$e) = ('a','b','c','d','e');
is(join(':',$a,@bcd[0..2],$e), 'a:b:c:d:e');

$x = 0;
for (1..100) {
    $x += $_;
}
is($x, 5050);

$x = 0;
for ((100,2..99,1)) {
    $x += $_;
}
is($x, 5050);

$x = join('','a'..'z');
is($x, 'abcdefghijklmnopqrstuvwxyz');

@x = 'A'..'ZZ';
is (scalar @x, 27 * 26);

foreach (0, 1) {
    use feature 'unicode_strings';
    $s = "a";
    $e = "\xFF";
    utf8::upgrade($e) if $_;
    @x = $s .. $e;
    is (scalar @x, 26, "list-context range with rhs 0xFF, utf8=$_");
    @y = ();
    foreach ($s .. $e) {
        push @y, $_;
    }
    is(join(",", @y), join(",", @x), "foreach range with rhs 0xFF, utf8=$_");
}

@x = '09' .. '08';  # should produce '09', '10',... '99' (strange but true)
is(join(",", @x), join(",", map {sprintf "%02d",$_} 9..99));

# same test with foreach (which is a separate implementation)
@y = ();
foreach ('09'..'08') {
    push(@y, $_);
}
is(join(",", @y), join(",", @x));

# check bounds
if ($Config{ivsize} == 8) {
  @a = eval "0x7ffffffffffffffe..0x7fffffffffffffff";
  $a = "9223372036854775806 9223372036854775807";
  @b = eval "-0x7fffffffffffffff..-0x7ffffffffffffffe";
  $b = "-9223372036854775807 -9223372036854775806";
}
else {
  @a = eval "0x7ffffffe..0x7fffffff";
  $a = "2147483646 2147483647";
  @b = eval "-0x7fffffff..-0x7ffffffe";
  $b = "-2147483647 -2147483646";
}

is ("@a", $a);

is ("@b", $b);

# check magic
{
    my $bad = 0;
    local $SIG{'__WARN__'} = sub { $bad = 1 };
    my $x = 'a-e';
    $x =~ s/(\w)-(\w)/join ':', $1 .. $2/e;
    is ($x, 'a:b:c:d:e');
}

# Should use magical autoinc only when both are strings
{
    my $scalar = (() = "0"..-1);
    is ($scalar, 0);
}
{
    my $fail = 0;
    for my $x ("0"..-1) {
	$fail++;
    }
    is ($fail, 0);
}

# [#18165] Should allow "-4".."0", broken by #4730. (AMS 20021031)
is(join(":","-4".."0")     , "-4:-3:-2:-1:0");
is(join(":","-4".."-0")    , "-4:-3:-2:-1:0");
is(join(":","-4\n".."0\n") , "-4:-3:-2:-1:0");
is(join(":","-4\n".."-0\n"), "-4:-3:-2:-1:0");

# [#133695] "0".."-1" should be the same as 0..-1
is(join(":","-2".."-1")    , "-2:-1");
is(join(":","-1".."-1")    , "-1");
is(join(":","0".."-1")     , "");
is(join(":","1".."-1")     , "");

# these test the statements made in the documentation
# regarding the rules of string ranges
is(join(":","-2".."2"),      join(":",-2..2));
is(join(":","2.18".."3.14"), "2:3");
is(join(":","01".."04"),     "01:02:03:04");
is(join(":","00".."-1"),     "00:01:02:03:04:05:06:07:08:09:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31:32:33:34:35:36:37:38:39:40:41:42:43:44:45:46:47:48:49:50:51:52:53:54:55:56:57:58:59:60:61:62:63:64:65:66:67:68:69:70:71:72:73:74:75:76:77:78:79:80:81:82:83:84:85:86:87:88:89:90:91:92:93:94:95:96:97:98:99");
is(join(":","00".."31"),     "00:01:02:03:04:05:06:07:08:09:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31");
is(join(":","ax".."az"),     "ax:ay:az");
is(join(":","*x".."az"),     "*x");
is(join(":","A".."Z"),       "A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z");
is(join(":", 0..9,"a".."f"), "0:1:2:3:4:5:6:7:8:9:a:b:c:d:e:f");
is(join(":","a".."--"),      join(":","a".."zz"));
is(join(":","0".."xx"),      "0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31:32:33:34:35:36:37:38:39:40:41:42:43:44:45:46:47:48:49:50:51:52:53:54:55:56:57:58:59:60:61:62:63:64:65:66:67:68:69:70:71:72:73:74:75:76:77:78:79:80:81:82:83:84:85:86:87:88:89:90:91:92:93:94:95:96:97:98:99");
is(join(":","aaa".."--"),    "");

# undef should be treated as 0 for numerical range
is(join(":",undef..2), '0:1:2');
is(join(":",-2..undef), '-2:-1:0');
is(join(":",undef..'2'), '0:1:2');
is(join(":",'-2'..undef), '-2:-1:0');

# undef should be treated as "" for magical range
is(join(":", map "[$_]", "".."B"), '[]');
is(join(":", map "[$_]", undef.."B"), '[]');
is(join(":", map "[$_]", "B"..""), '');
is(join(":", map "[$_]", "B"..undef), '');

# undef..undef used to segfault
is(join(":", map "[$_]", undef..undef), '[]');

# also test undef in foreach loops
@foo=(); push @foo, $_ for undef..2;
is(join(":", @foo), '0:1:2');

@foo=(); push @foo, $_ for -2..undef;
is(join(":", @foo), '-2:-1:0');

@foo=(); push @foo, $_ for undef..'2';
is(join(":", @foo), '0:1:2');

@foo=(); push @foo, $_ for '-2'..undef;
is(join(":", @foo), '-2:-1:0');

@foo=(); push @foo, $_ for undef.."B";
is(join(":", map "[$_]", @foo), '[]');

@foo=(); push @foo, $_ for "".."B";
is(join(":", map "[$_]", @foo), '[]');

@foo=(); push @foo, $_ for "B"..undef;
is(join(":", map "[$_]", @foo), '');

@foo=(); push @foo, $_ for "B".."";
is(join(":", map "[$_]", @foo), '');

@foo=(); push @foo, $_ for undef..undef;
is(join(":", map "[$_]", @foo), '[]');

# again with magic
{
    my @a = (1..3);
    @foo=(); push @foo, $_ for undef..$#a;
    is(join(":", @foo), '0:1:2');
}
{
    my @a = ();
    @foo=(); push @foo, $_ for $#a..undef;
    is(join(":", @foo), '-1:0');
}
{
    local $1;
    "2" =~ /(.+)/;
    @foo=(); push @foo, $_ for undef..$1;
    is(join(":", @foo), '0:1:2');
}
{
    local $1;
    "-2" =~ /(.+)/;
    @foo=(); push @foo, $_ for $1..undef;
    is(join(":", @foo), '-2:-1:0');
}
{
    local $1;
    "B" =~ /(.+)/;
    @foo=(); push @foo, $_ for undef..$1;
    is(join(":", map "[$_]", @foo), '[]');
}
{
    local $1;
    "B" =~ /(.+)/;
    @foo=(); push @foo, $_ for ""..$1;
    is(join(":", map "[$_]", @foo), '[]');
}
{
    local $1;
    "B" =~ /(.+)/;
    @foo=(); push @foo, $_ for $1..undef;
    is(join(":", map "[$_]", @foo), '');
}
{
    local $1;
    "B" =~ /(.+)/;
    @foo=(); push @foo, $_ for $1.."";
    is(join(":", map "[$_]", @foo), '');
}

# Test upper range limit
my $MAX_INT = ~0>>1;

foreach my $ii (-3 .. 3) {
    my ($first, $last);
    eval {
        my $lim=0;
        for ($MAX_INT-10 .. $MAX_INT+$ii) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);   # Protect against integer wrap
        }
    };
    if ($ii <= 0) {
        ok(! $@, 'Upper bound accepted: ' . ($MAX_INT+$ii));
        is($first, $MAX_INT-10, 'Lower bound okay');
        is($last, $MAX_INT+$ii, 'Upper bound okay');
    } else {
        ok($@, 'Upper bound rejected: ' . ($MAX_INT+$ii));
    }
}

foreach my $ii (-3 .. 3) {
    my ($first, $last);
    eval {
        my $lim=0;
        for ($MAX_INT+$ii .. $MAX_INT) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);
        }
    };
    if ($ii <= 0) {
        ok(! $@, 'Lower bound accepted: ' . ($MAX_INT+$ii));
        is($first, $MAX_INT+$ii, 'Lower bound okay');
        is($last, $MAX_INT, 'Upper bound okay');
    } else {
        ok($@, 'Lower bound rejected: ' . ($MAX_INT+$ii));
    }
}

{
    my $first;
    eval {
        my $lim=0;
        for ($MAX_INT .. $MAX_INT-1) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);
        }
    };
    ok(! $@, 'Range accepted');
    ok(! defined($first), 'Range ineffectual');
}

foreach my $ii (~0, ~0+1, ~0+(~0>>4)) {
    eval {
        my $lim=0;
        for ($MAX_INT-10 .. $ii) {
            last if ($lim++ > 100);
        }
    };
    ok($@, 'Upper bound rejected: ' . $ii);
}

# Test lower range limit
my $MIN_INT = -1-$MAX_INT;

if (! $Config{d_nv_preserves_uv}) {
    # $MIN_INT needs adjustment when IV won't fit into an NV
    my $NV = $MIN_INT - 1;
    my $OFFSET = 1;
    while (($NV + $OFFSET) == $MIN_INT) {
        $OFFSET++
    }
    $MIN_INT += $OFFSET;
}

foreach my $ii (-3 .. 3) {
    my ($first, $last);
    eval {
        my $lim=0;
        for ($MIN_INT+$ii .. $MIN_INT+10) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);
        }
    };
    if ($ii >= 0) {
        ok(! $@, 'Lower bound accepted: ' . ($MIN_INT+$ii));
        is($first, $MIN_INT+$ii, 'Lower bound okay');
        is($last, $MIN_INT+10, 'Upper bound okay');
    } else {
        ok($@, 'Lower bound rejected: ' . ($MIN_INT+$ii));
    }
}

foreach my $ii (-3 .. 3) {
    my ($first, $last);
    eval {
        my $lim=0;
        for ($MIN_INT .. $MIN_INT+$ii) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);
        }
    };
    if ($ii >= 0) {
        ok(! $@, 'Upper bound accepted: ' . ($MIN_INT+$ii));
        is($first, $MIN_INT, 'Lower bound okay');
        is($last, $MIN_INT+$ii, 'Upper bound okay');
    } else {
        ok($@, 'Upper bound rejected: ' . ($MIN_INT+$ii));
    }
}

{
    my $first;
    eval {
        my $lim=0;
        for ($MIN_INT+1 .. $MIN_INT) {
            if (! defined($first)) {
                $first = $_;
            }
            $last = $_;
            last if ($lim++ > 100);
        }
    };
    ok(! $@, 'Range accepted');
    ok(! defined($first), 'Range ineffectual');
}

foreach my $ii (~0, ~0+1, ~0+(~0>>4)) {
    eval {
        my $lim=0;
        for (-$ii .. $MIN_INT+10) {
            last if ($lim++ > 100);
        }
    };
    ok($@, 'Lower bound rejected: ' . -$ii);
}

# double/triple magic tests
sub TIESCALAR { bless { value => $_[1], orig => $_[1] } }
sub STORE { $_[0]{store}++; $_[0]{value} = $_[1] }
sub FETCH { $_[0]{fetch}++; $_[0]{value} }
sub stores { tied($_[0])->{value} = tied($_[0])->{orig};
             delete(tied($_[0])->{store}) || 0 }
sub fetches { delete(tied($_[0])->{fetch}) || 0 }
    
tie $x, "main", 6;

my @foo;
@foo = 4 .. $x;
is(scalar @foo, 3);
is("@foo", "4 5 6");
is(fetches($x), 1);
is(stores($x), 0);

@foo = $x .. 8;
is(scalar @foo, 3);
is("@foo", "6 7 8");
is(fetches($x), 1);
is(stores($x), 0);

@foo = $x .. $x + 1;
is(scalar @foo, 2);
is("@foo", "6 7");
is(fetches($x), 2);
is(stores($x), 0);

@foo = ();
for (4 .. $x) {
  push @foo, $_;
}
is(scalar @foo, 3);
is("@foo", "4 5 6");
is(fetches($x), 1);
is(stores($x), 0);

@foo = ();
for (reverse 4 .. $x) {
  push @foo, $_;
}
is(scalar @foo, 3);
is("@foo", "6 5 4");
is(fetches($x), 1);
is(stores($x), 0);

is( ( join ' ', map { join '', map ++$_, ($x=1)..4 } 1..2 ), '2345 2345',
    'modifiable variable num range' );
is( ( join ' ', map { join '', map ++$_, 1..4      } 1..2 ), '2345 2345',
    'modifiable const num range' );  # RT#3105
$s = ''; for (1..2) { for (1..4) { $s .= ++$_ } $s.=' ' if $_==1; }
is( $s, '2345 2345','modifiable num counting loop counter' );


is( ( join ' ', map { join '', map ++$_, ($x='a')..'d' } 1..2 ), 'bcde bcde',
    'modifiable variable alpha range' );
is( ( join ' ', map { join '', map ++$_, 'a'..'d'      } 1..2 ), 'bcde bcde',
    'modifiable const alpha range' );  # RT#3105
$s = ''; for (1..2) { for ('a'..'d') { $s .= ++$_ } $s.=' ' if $_==1; }
is( $s, 'bcde bcde','modifiable alpha counting loop counter' );

# RT #130841
# generating an extreme range triggered a croak, which if caught,
# left the temps stack small but with a very large PL_tmps_max

SKIP: {
    skip 'mem wrap check disabled' unless $Config{usemallocwrap};
    fresh_perl_like(<<'EOF', qr/\Aok 1 ok 2\Z/, {}, "RT #130841");
my $max_iv = (~0 >> 1);
eval {
    my @range = 1..($max_iv - 1);
};
if ($@ =~ /panic: memory wrap|Out of memory/) {
    print "ok 1";
}
else {
    print "unexpected err status: [$@]";
}

# create and push lots of temps
my $max = 10_000;
my @ints = map $_+1, 0..($max-1);
my $sum = 0;
$sum += $_ for @ints;
my $exp = $max*($max+1)/2;
if ($sum == $exp) {
    print " ok 2";
}
else {
    print " unexpected sum: [$sum]; expected: [$exp]";
}
EOF
}
