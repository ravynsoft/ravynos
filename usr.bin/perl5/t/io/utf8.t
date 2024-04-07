#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
	require './charset_tools.pl';
}
skip_all_without_perlio();

no utf8; # needed for use utf8 not griping about the raw octets


plan(tests => 62);

$| = 1;

my $a_file = tempfile();

open(F,"+>:utf8",$a_file);
print F chr(0x100).'£';
cmp_ok( tell(F), '==', 4, tell(F) );
print F "\n";
cmp_ok( tell(F), '>=', 5, tell(F) );
seek(F,0,0);
is( getc(F), chr(0x100) );
is( getc(F), "£" );
is( getc(F), "\n" );
seek(F,0,0);
binmode(F,":bytes");

# Byte representation of these characters
my $U_100 = byte_utf8a_to_utf8n("\xc4\x80");
my $POUND_SIGN = byte_utf8a_to_utf8n("\xc2\xa3");

my $chr = substr($U_100, 0, 1);
is( getc(F), $chr );
$chr = substr($U_100, 1, 1);
is( getc(F), $chr );
$chr = substr($POUND_SIGN, 0, 1);
is( getc(F), $chr );
$chr = substr($POUND_SIGN, 1, 1);
is( getc(F), $chr );
is( getc(F), "\n" );
seek(F,0,0);
binmode(F,":utf8");
is( scalar(<F>), "\x{100}£\n" );
seek(F,0,0);
$buf = chr(0x200);
$count = read(F,$buf,2,1);
cmp_ok( $count, '==', 2 );
is( $buf, "\x{200}\x{100}£" );
close(F);

{
    $a = chr(300); # This *is* UTF-encoded
    $b = chr(130); # This is not.

    open F, ">:utf8", $a_file or die $!;
    print F $a,"\n";
    close F;

    open F, "<:utf8", $a_file or die $!;
    $x = <F>;
    chomp($x);
    is( $x, chr(300) );

    open F, $a_file or die $!; # Not UTF
    binmode(F, ":bytes");
    $x = <F>;
    chomp($x);
    $chr = byte_utf8a_to_utf8n(chr(196).chr(172));
    is( $x, $chr );
    close F;

    open F, ">:utf8", $a_file or die $!;
    binmode(F);  # we write a "\n" and then tell() - avoid CRLF issues.
    binmode(F,":utf8"); # turn UTF-8-ness back on
    print F $a;
    my $y;
    { my $x = tell(F);
      { use bytes; $y = length($a);}
      cmp_ok( $x, '==', $y );
  }

    { # Check byte length of $b
	use bytes; my $y = length($b);
	cmp_ok( $y, '==', 1 );
    }

    print F $b,"\n"; # Don't upgrade $b

    { # Check byte length of $b
	use bytes; my $y = length($b);
	cmp_ok( $y, '==', 1 );
    }

    {
	my $x = tell(F);
	{ use bytes; if ($::IS_EBCDIC){$y += 2;}else{$y += 3;}} # EBCDIC ASCII
	cmp_ok( $x, '==', $y );
    }

    close F;

    open F, $a_file or die $!; # Not UTF
    binmode(F, ":bytes");
    $x = <F>;
    chomp($x);
    $chr = v196.172.194.130;
    if ($::IS_EBCDIC) { $chr = v141.83.130; } # EBCDIC
    is( $x, $chr, sprintf('(%vd)', $x) );

    open F, "<:utf8", $a_file or die $!;
    $x = <F>;
    chomp($x);
    close F;
    is( $x, chr(300).chr(130), sprintf('(%vd)', $x) );

    open F, ">", $a_file or die $!;
    binmode(F, ":bytes:");

    # Now let's make it suffer.
    my $w;
    {
	use warnings 'utf8';
	local $SIG{__WARN__} = sub { $w = $_[0] };
	print F $a;
        ok( (!$@));
	like($w, qr/Wide character in print/i );
    }
}

# Hm. Time to get more evil.
open F, ">:utf8", $a_file or die $!;
print F $a;
binmode(F, ":bytes");
print F chr(130)."\n";
close F;

open F, "<", $a_file or die $!;
binmode(F, ":bytes");
$x = <F>; chomp $x;
$chr = v196.172.130;
if ($::IS_EBCDIC) { $chr = v141.83.130; } # EBCDIC
is( $x, $chr );

# Right.
open F, ">:utf8", $a_file or die $!;
print F $a;
close F;
open F, ">>", $a_file or die $!;
binmode(F, ":bytes");
print F chr(130)."\n";
close F;

open F, "<", $a_file or die $!;
binmode(F, ":bytes");
$x = <F>; chomp $x;
SKIP: {
    skip("Defaulting to UTF-8 output means that we can't generate a mangled file")
	if $UTF8_OUTPUT;
    is( $x, $chr );
}

# Now we have a deformed file.

SKIP: {
    if ($::IS_EBCDIC) {
	skip("EBCDIC The file isn't deformed in UTF-EBCDIC", 2);
    } else {
	my @warnings;
	open F, "<:utf8", $a_file or die $!;
	$x = <F>; chomp $x;
	local $SIG{__WARN__} = sub { push @warnings, $_[0]; };
	eval { sprintf "%vd\n", $x };
	is (scalar @warnings, 1);
	like ($warnings[0], qr/Malformed UTF-8 character: \\x82 \(unexpected continuation byte 0x82, with no preceding start byte/);
    }
}

close F;
unlink($a_file);

open F, ">:utf8", $a_file;
@a = map { chr(1 << ($_ << 2)) } 0..5; # 0x1, 0x10, .., 0x100000
unshift @a, chr(0); # ... and a null byte in front just for fun
print F @a;
close F;

my $c;

# read() should work on characters, not bytes
open F, "<:utf8", $a_file;
$a = 0;
my $failed;
for (@a) {
    unless (($c = read(F, $b, 1) == 1)  &&
            length($b)           == 1  &&
            ord($b)              == ord($_) &&
            tell(F)              == ($a += bytes::length($b))) {
        print '# ord($_)           == ', ord($_), "\n";
        print '# ord($b)           == ', ord($b), "\n";
        print '# length($b)        == ', length($b), "\n";
        print '# bytes::length($b) == ', bytes::length($b), "\n";
        print '# tell(F)           == ', tell(F), "\n";
        print '# $a                == ', $a, "\n";
        print '# $c                == ', $c, "\n";
	$failed++;
        last;
    }
}
close F;
is($failed, undef);

{
    # Check that warnings are on on I/O, and that they can be muffled.

    local $SIG{__WARN__} = sub { $@ = shift };

    undef $@;
    open F, ">$a_file";
    binmode(F, ":bytes");
    print F chr(0x100);
    close(F);

    like( $@, qr/Wide character in print/ );

    undef $@;
    open F, ">:utf8", $a_file;
    print F chr(0x100);
    close(F);

    isnt( defined $@, !0 );

    undef $@;
    open F, ">$a_file";
    binmode(F, ":utf8");
    print F chr(0x100);
    close(F);

    isnt( defined $@, !0 );

    no warnings 'utf8';

    undef $@;
    open F, ">$a_file";
    print F chr(0x100);
    close(F);

    isnt( defined $@, !0 );

    use warnings 'utf8';

    undef $@;
    open F, ">$a_file";
    binmode(F, ":bytes");
    print F chr(0x100);
    close(F);

    like( $@, qr/Wide character in print/ );
}

{
    open F, ">:bytes",$a_file; print F "\xde"; close F;

    open F, "<:bytes", $a_file;
    my $b = chr 0x100;
    $b .= <F>;
    is( $b, chr(0x100).chr(0xde), "21395 '.= <>' utf8 vs. bytes" );
    close F;
}

{
    open F, ">:utf8",$a_file; print F chr 0x100; close F;

    open F, "<:utf8", $a_file;
    my $b = "\xde";
    $b .= <F>;
    is( $b, chr(0xde).chr(0x100), "21395 '.= <>' bytes vs. utf8" );
    close F;
}

{
    my @a = ( [ 0x007F, "bytes" ],
	      [ 0x0080, "bytes" ],
	      [ 0x0080, "utf8"  ],
	      [ 0x0100, "utf8"  ] );
    my $t = 34;
    for my $u (@a) {
	for my $v (@a) {
	    # print "# @$u - @$v\n";
	    open F, ">$a_file";
	    binmode(F, ":" . $u->[1]);
	    print F chr($u->[0]);
	    close F;

	    open F, "<$a_file";
	    binmode(F, ":" . $u->[1]);

	    my $s = chr($v->[0]);
	    utf8::upgrade($s) if $v->[1] eq "utf8";

	    $s .= <F>;
	    is( $s, chr($v->[0]) . chr($u->[0]), 'rcatline utf8' );
	    close F;
	    $t++;
	}
    }
    # last test here 49
}

{
    # [perl #23428] Somethings rotten in unicode semantics
    open F, ">$a_file";
    binmode F;
    $a = "A";
    utf8::upgrade($a);
    syswrite(F, $a);
    close F;
    ok(utf8::is_utf8($a), '23428 syswrite should not downgrade scalar' );
}

{
    # <FH> on a :utf8 stream should complain immediately with -w
    # if it finds bad UTF-8 (:encoding(utf8) works this way)
    use warnings 'utf8';
    undef $@;
    local $SIG{__WARN__} = sub { $@ = shift };
    open F, ">$a_file";
    binmode F;
    my ($chrE4, $chrF6) = (chr(0xE4), chr(0xF6));
    if ($::IS_EBCDIC)	# EBCDIC
    { ($chrE4, $chrF6) = (chr(0x43), chr(0xEC)); }
    print F "foo", $chrE4, "\n";
    print F "foo", $chrF6, "\n";
    close F;
    open F, "<:utf8", $a_file;
    undef $@;
    my $line = <F>;
    my ($chrE4, $chrF6) = ("E4", "F6");
    if ($::IS_EBCDIC) { ($chrE4, $chrF6) = ("43", "EC"); } # EBCDIC
    like( $@, qr/utf8 "\\x$chrE4" does not map to Unicode .+ <F> line 1/,
	  "<:utf8 readline must warn about bad utf8");
    undef $@;
    $line .= <F>;
    like( $@, qr/utf8 "\\x$chrF6" does not map to Unicode .+ <F> line 2/,
	  "<:utf8 rcatline must warn about bad utf8");
    close F;
}

{
    # fixed record reads
    open F, ">:utf8", $a_file;
    print F "foo\xE4";
    print F "bar\xFE";
    print F "\xC0\xC8\xCC\xD2";
    print F "a\xE4ab";
    print F "a\xE4a";
    close F;
    open F, "<:utf8", $a_file;
    local $/ = \4;
    my $line = <F>;
    is($line, "foo\xE4", "readline with \$/ = \\4");
    $line .= <F>;
    is($line, "foo\xE4bar\xFE", "rcatline with \$/ = \\4");
    $line = <F>;
    is($line, "\xC0\xC8\xCC\xD2", "readline with several encoded characters");
    $line = <F>;
    is($line, "a\xE4ab", "readline with another boundary condition");
    $line = <F>;
    is($line, "a\xE4a", "readline with boundary condition");
    close F;

    # badly encoded at EOF
    open F, ">:raw", $a_file;
    print F "foo\xEF\xAC"; # truncated \x{FB04} small ligature ffl
    close F;

    use warnings 'utf8';
    open F, "<:utf8", $a_file;
    undef $@;
    local $SIG{__WARN__} = sub { $@ = shift };
    $line = <F>;

    like( $@, qr/utf8 "\\xEF" does not map to Unicode .+ <F> chunk 1/,
	  "<:utf8 readline (fixed) must warn about bad utf8");
    close F;
}

# getc should reset the utf8 flag and not be affected by previous
# return values
SKIP: {
    skip "no PerlIO::scalar on miniperl", 2, if is_miniperl();
    open my $fh, "<:raw",  \($buf = chr 255);
    open my $uh, "<:utf8", \($uuf = $U_100);
    for([$uh,chr 256], [$fh,chr 255]) {
	is getc $$_[0], $$_[1],
	  'getc returning non-utf8 after utf8';
    }
}
