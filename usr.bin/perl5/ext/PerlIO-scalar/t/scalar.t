#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bPerlIO/scalar\b!) ){
        print "1..0 # Skip -- Perl configured without PerlIO::scalar module\n";
        exit 0;
    }
}

use Fcntl qw(SEEK_SET SEEK_CUR SEEK_END); # Not 0, 1, 2 everywhere.
use Errno qw(EACCES);

$| = 1;

use Test::More tests => 125;

my $fh;
my $var = "aaa\n";
ok(open($fh,"+<",\$var));

is(<$fh>, $var);

ok(eof($fh));

ok(seek($fh,0,SEEK_SET));
ok(!eof($fh));

ok(print $fh "bbb\n");
is($var, "bbb\n");
$var = "foo\nbar\n";
ok(seek($fh,0,SEEK_SET));
ok(!eof($fh));
is(<$fh>, "foo\n");
ok(close $fh, $!);

# Test that semantics are similar to normal file-based I/O
# Check that ">" clobbers the scalar
$var = "Something";
open $fh, ">", \$var;
is($var, "");
#  Check that file offset set to beginning of scalar
my $off = tell($fh);
is($off, 0);
# Check that writes go where they should and update the offset
$var = "Something";
print $fh "Brea";
$off = tell($fh);
is($off, 4);
is($var, "Breathing");
close $fh;

# Check that ">>" appends to the scalar
$var = "Something ";
open $fh, ">>", \$var;
$off = tell($fh);
is($off, 10);
is($var, "Something ");
#  Check that further writes go to the very end of the scalar
$var .= "else ";
is($var, "Something else ");

$off = tell($fh);
is($off, 10);

print $fh "is here";
is($var, "Something else is here");
close $fh;

# Check that updates to the scalar from elsewhere do not
# cause problems
$var = "line one\nline two\line three\n";
open $fh, "<", \$var;
while (<$fh>) {
    $var = "foo";
}
close $fh;
is($var, "foo");

# Check that dup'ing the handle works

$var = '';
open $fh, "+>", \$var;
print $fh "xxx\n";
open $dup,'+<&',$fh;
print $dup "yyy\n";
seek($dup,0,SEEK_SET);
is(<$dup>, "xxx\n");
is(<$dup>, "yyy\n");
close($fh);
close($dup);

open $fh, '<', \42;
is(<$fh>, "42", "reading from non-string scalars");
close $fh;

{ package P; sub TIESCALAR {bless{}} sub FETCH { "shazam" } sub STORE {} }
tie $p, P; open $fh, '<', \$p;
is(<$fh>, "shazam", "reading from magic scalars");

{
    use warnings;
    my $warn = 0;
    local $SIG{__WARN__} = sub { $warn++ };
    open my $fh, '>', \my $scalar;
    print $fh "foo";
    close $fh;
    is($warn, 0, "no warnings when writing to an undefined scalar");
    undef $scalar;
    open $fh, '>>', \$scalar;
    print $fh "oof";
    close $fh;
    is($warn, 0, "no warnings when appending to an undefined scalar");
}

{
    use warnings;
    my $warn = 0;
    local $SIG{__WARN__} = sub { $warn++ };
    for (1..2) {
        open my $fh, '>', \my $scalar;
        close $fh;
    }
    is($warn, 0, "no warnings when reusing a lexical");
}

{
    use warnings;
    my $warn = 0;
    local $SIG{__WARN__} = sub { $warn++ };

    my $fetch = 0;
    {
        package MgUndef;
        sub TIESCALAR { bless [] }
        sub FETCH { $fetch++; return undef }
	sub STORE {}
    }
    tie my $scalar, MgUndef;

    open my $fh, '<', \$scalar;
    close $fh;
    is($warn, 0, "no warnings reading a magical undef scalar");
    is($fetch, 1, "FETCH only called once");
}

{
    use warnings;
    my $warn = 0;
    local $SIG{__WARN__} = sub { $warn++ };
    my $scalar = 3;
    undef $scalar;
    open my $fh, '<', \$scalar;
    close $fh;
    is($warn, 0, "no warnings reading an undef, allocated scalar");
}

my $data = "a non-empty PV";
$data = undef;
open(MEM, '<', \$data) or die "Fail: $!\n";
my $x = join '', <MEM>;
is($x, '');

{
    # [perl #35929] verify that works with $/ (i.e. test PerlIOScalar_unread)
    my $s = <<'EOF';
line A
line B
a third line
EOF
    open(F, '<', \$s) or die "Could not open string as a file";
    local $/ = "";
    my $ln = <F>;
    close F;
    is($ln, $s, "[perl #35929]");
}

# [perl #40267] PerlIO::scalar doesn't respect readonly-ness
{
    my $warn;
    local $SIG{__WARN__} = sub { $warn = "@_" };
    ok(!(defined open(F, '>', \undef)), "[perl #40267] - $!");
    is($warn, undef, "no warning with warnings off");
    close F;

    use warnings 'layer';
    undef $warn;
    my $ro = \43;
    ok(!(defined open(F, '>', $ro)), $!);
    is($!+0, EACCES, "check we get a read-onlyish error code");
    like($warn, qr/Modification of a read-only value attempted/,
         "check we did warn");
    close F;
    # but we can read from it
    ok(open(F, '<', $ro), $!);
    is(<F>, 43);
    close F;
}

{
    # Check that we zero fill when needed when seeking,
    # and that seeking negative off the string does not do bad things.

    my $foo;

    ok(open(F, '>', \$foo));

    # Seeking forward should zero fill.

    ok(seek(F, 50, SEEK_SET));
    print F "x";
    is(length($foo), 51);
    like($foo, qr/^\0{50}x$/);

    is(tell(F), 51);
    ok(seek(F, 0, SEEK_SET));
    is(length($foo), 51);

    # Seeking forward again should zero fill but only the new bytes.

    ok(seek(F, 100, SEEK_SET));
    print F "y";
    is(length($foo), 101);
    like($foo, qr/^\0{50}x\0{49}y$/);
    is(tell(F), 101);

    # Seeking back and writing should not zero fill.

    ok(seek(F, 75, SEEK_SET));
    print F "z";
    is(length($foo), 101);
    like($foo, qr/^\0{50}x\0{24}z\0{24}y$/);
    is(tell(F), 76);

    # Seeking negative should not do funny business.

    ok(!seek(F,  -50, SEEK_SET), $!);
    ok(seek(F, 0, SEEK_SET));
    ok(!seek(F,  -50, SEEK_CUR), $!);
    ok(!seek(F, -150, SEEK_END), $!);
}

# RT #43789: should respect tied scalar

{
    package TS;
    my $s;
    sub TIESCALAR { bless \my $x }
    sub FETCH { $s .= ':F'; ${$_[0]} }
    sub STORE { $s .= ":S($_[1])"; ${$_[0]} = $_[1] }

    package main;

    my $x;
    $s = '';
    tie $x, 'TS';
    my $fh;

    ok(open($fh, '>', \$x), 'open-write tied scalar');
    $s .= ':O';
    print($fh 'ABC');
    $s .= ':P';
    ok(seek($fh, 0, SEEK_SET));
    $s .= ':SK';
    print($fh 'DEF');
    $s .= ':P';
    ok(close($fh), 'close tied scalar - write');
    is($s, ':F:S():O:F:S(ABC):P:SK:F:S(DEF):P', 'tied actions - write');
    is($x, 'DEF', 'new value preserved');

    $x = 'GHI';
    $s = '';
    ok(open($fh, '+<', \$x), 'open-read tied scalar');
    $s .= ':O';
    my $buf;
    is(read($fh,$buf,2), 2, 'read1');
    $s .= ':R';
    is($buf, 'GH', 'buf1');
    is(read($fh,$buf,2), 1, 'read2');
    $s .= ':R';
    is($buf, 'I', 'buf2');
    is(read($fh,$buf,2), 0, 'read3');
    $s .= ':R';
    is($buf, '', 'buf3');
    ok(close($fh), 'close tied scalar - read');
    is($s, ':F:S(GHI):O:F:R:F:R:F:R', 'tied actions - read');
}

# [perl #78716] Seeking beyond the end of the string, then reading
{
    my $str = '1234567890';
    open my $strIn, '<', \$str;
    seek $strIn, 15, 1;
    is read($strIn, my $buffer, 5), 0,
     'seek beyond end end of string followed by read';
}

# Writing to COW scalars and non-PVs
{
    my $bovid = __PACKAGE__;
    open my $handel, ">", \$bovid;
    print $handel "the COW with the crumpled horn";
    is $bovid, "the COW with the crumpled horn", 'writing to COW scalars';

    package lrcg { use overload fallback => 1, '""'=>sub { 'chin' } }
    seek $handel, 3, 0;
    $bovid = bless [], lrcg::;
    print $handel 'mney';
    is $bovid, 'chimney', 'writing to refs';

    seek $handel, 1, 0;
    $bovid = 42;  # still has a PV
    print $handel 5;
    is $bovid, 45, 'writing to numeric scalar';

    seek $handel, 1, 0;
    undef $bovid;
    $bovid = 42;   # just IOK
    print $handel 5;
    is $bovid, 45, 'writing to numeric scalar';
}

# [perl #92706]
{
    open my $fh, "<", \(my $f=*f); seek $fh, 2,1;
    pass 'seeking on a glob copy';
    open my $fh, "<", \(my $f=*f); seek $fh, -2,2;
    pass 'seeking on a glob copy from the end';
}

# [perl #108398]
sub has_trailing_nul(\$) {
    my ($ref) = @_;
    my $sv = B::svref_2object($ref);
    return undef if !$sv->isa('B::PV');

    my $cur = $sv->CUR;
    my $len = $sv->LEN;
    return 0 if $cur >= $len;

    my $ptrlen = length(pack('P', ''));
    my $ptrfmt
	= $ptrlen == length(pack('J', 0)) ? 'J'
	: $ptrlen == length(pack('I', 0)) ? 'I'
	: die "Can't determine pointer format";

    my $pv_addr = unpack $ptrfmt, pack 'P', $$ref;
    my $trailing = unpack 'P', pack $ptrfmt, $pv_addr+$cur;
    return $trailing eq "\0";
}
SKIP: {
    if ($Config::Config{'extensions'} !~ m!\bB\b!) {
	skip "no B", 4;
    }
    require B;

    open my $fh, ">", \my $memfile or die $!;

    print $fh "abc";
    ok has_trailing_nul $memfile,
	 'write appends trailing null when growing string';

    seek $fh, 0,SEEK_SET;
    print $fh "abc";
    ok has_trailing_nul $memfile,
	 'write appends trailing null when not growing string';

    seek $fh, 200, SEEK_SET;
    print $fh "abc";
    ok has_trailing_nul $memfile,
	 'write appends null when growing string after seek past end';

    open $fh, ">", \($memfile = "hello");
    ok has_trailing_nul $memfile,
	 'initial truncation in ">" mode provides trailing null';
}

# [perl #112780] Cloning of in-memory handles
SKIP: {
  skip "no threads", 2 if !$Config::Config{useithreads};
  require threads;
  my $str = '';
  open my $fh, ">", \$str;
  $str = 'a';
  is scalar threads::async(sub { my $foo = $str; $foo })->join, "a",
    'scalars behind in-memory handles are cloned properly';
  print $fh "a";
  is scalar threads::async(sub { print $fh "b"; $str })->join, "ab",
    'printing to a cloned in-memory handle works';
}

# [perl #113764] Duping via >&= (broken by the fix for #112870)
{
  open FILE, '>', \my $content or die "Couldn't open scalar filehandle";
  open my $fh, ">&=FILE" or die "Couldn't open: $!";
  print $fh "Foo-Bar\n";
  close $fh;
  close FILE;
  is $content, "Foo-Bar\n", 'duping via >&=';
}

# [perl #109828] PerlIO::scalar does not handle UTF-8
my $byte_warning = "Strings with code points over 0xFF may not be mapped into in-memory file handles\n";
{
    use Errno qw(EINVAL);
    my @warnings;
    local $SIG{__WARN__} = sub { push @warnings, "@_" };
    my $content = "12\x{101}";
    $! = 0;
    ok(!open(my $fh, "<", \$content), "non-byte open should fail");
    is(0+$!, EINVAL, "check \$! is updated");
    is_deeply(\@warnings, [], "should be no warnings (yet)");
    use warnings "utf8";
    $! = 0;
    ok(!open(my $fh, "<", \$content), "non byte open should fail (and warn)");
    is(0+$!, EINVAL, "check \$! is updated even when we warn");
    is_deeply(\@warnings, [ $byte_warning ], "should have warned");

    @warnings = ();
    $content = "12\xA1";
    utf8::upgrade($content);
    ok(open(my $fh, "<", \$content), "open upgraded scalar");
    binmode $fh;
    my $tmp;
    is(read($fh, $tmp, 4), 3, "read should get the downgraded bytes");
    is($tmp, "12\xA1", "check we got the expected bytes");
    close $fh;
    is_deeply(\@warnings, [], "should be no more warnings");
}
{ # changes after open
    my $content = "abc";
    ok(open(my $fh, "+<", \$content), "open a scalar");
    binmode $fh;
    my $tmp;
    is(read($fh, $tmp, 1), 1, "basic read");
    seek($fh, 1, SEEK_SET);
    $content = "\xA1\xA2\xA3";
    utf8::upgrade($content);
    is(read($fh, $tmp, 1), 1, "read from post-open upgraded scalar");
    is($tmp, "\xA2", "check we read the correct value");
    seek($fh, 1, SEEK_SET);
    $content = "\x{101}\x{102}\x{103}";

    my @warnings;
    local $SIG{__WARN__} = sub { push @warnings, "@_" };

    $! = 0;
    is(read($fh, $tmp, 1), undef, "read from scalar with >0xff chars");
    is(0+$!, EINVAL, "check errno set correctly");
    is_deeply(\@warnings, [], "should be no warning (yet)");
    use warnings "utf8";
    seek($fh, 1, SEEK_SET);
    is(read($fh, $tmp, 1), undef, "read from scalar with >0xff chars");
    is_deeply(\@warnings, [ $byte_warning ], "check warning");

    select $fh; # make sure print fails rather tha buffers
    $| = 1;
    select STDERR;
    no warnings "utf8";
    @warnings = ();
    $content = "\xA1\xA2\xA3";
    utf8::upgrade($content);
    seek($fh, 1, SEEK_SET);
    ok((print $fh "A"), "print to an upgraded byte string");
    seek($fh, 1, SEEK_SET);
    is($content, "\xA1A\xA3", "check result");

    $content = "\x{101}\x{102}\x{103}";
    $! = 0;
    ok(!(print $fh "B"), "write to an non-downgradable SV");
    is(0+$!, EINVAL, "check errno set");

    is_deeply(\@warnings, [], "should be no warning");

    use warnings "utf8";
    ok(!(print $fh "B"), "write to an non-downgradable SV (and warn)");
    is_deeply(\@warnings, [ $byte_warning ], "check warning");
}

#  RT #119529: Reading refs should not loop

{
    my $x = \42;
    open my $fh, "<", \$x;
    my $got = <$fh>; # this used to loop
    like($got, qr/^SCALAR\(0x[0-9a-f]+\)$/, "ref to a ref");
    is ref $x, "SCALAR", "target scalar is still a reference";
}

# Appending to refs
{
    my $x = \42;
    my $as_string = "$x";
    open my $refh, ">>", \$x;
    is ref $x, "SCALAR", 'still a ref after opening for appending';
    print $refh "boo\n";
    is $x, $as_string."boo\n", 'string gets appended to ref';
}

SKIP:
{ # [perl #123443]
    skip "Can't seek over 4GB with a small off_t", 4
      if $Config::Config{lseeksize} < 8;
    my $buf0 = "hello";
    open my $fh, "<", \$buf0 or die $!;
    ok(seek($fh, 2**32, SEEK_SET), "seek to a large position");
    is(read($fh, my $tmp, 1), 0, "read from a large offset");
    is($tmp, "", "should have read nothing");
    ok(eof($fh), "fh should be eof");
}

{
    my $buf0 = "hello";
    open my $fh, "<", \$buf0 or die $!;
    ok(!seek($fh, -10, SEEK_CUR), "seek to negative position");
    is(tell($fh), 0, "shouldn't change the position");
}

SKIP:
{ # write() beyond SSize_t limit
    skip "Can't overflow SSize_t with Off_t", 2
      if $Config::Config{lseeksize} <= $Config::Config{sizesize};
    my $buf0 = "hello";
    open my $fh, "+<", \$buf0 or die $!;
    ok(seek($fh, 2**32, SEEK_SET), "seek to a large position");
    select((select($fh), ++$|)[0]);
    ok(!(print $fh "x"), "write to a large offset");
}
