#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
    require "./charset_tools.pl";
    skip_all_without_perlio();
}

use Config;


my $file = tempfile();
my $crlf = uni_to_native("\015\012");
my $crcr = uni_to_native("\x0d\x0d");

my $ungetc_count = 8200;    # Somewhat over the likely buffer size

{
    plan(tests => 21 + 2 * $ungetc_count);
    ok(open(FOO,">:crlf",$file));
    ok(print FOO 'a'.((('a' x 14).qq{\n}) x 2000) || close(FOO));
    ok(open(FOO,"<:crlf",$file));

    my $text;
    { local $/; $text = <FOO> }
    is(count_chars($text, $crlf), 0);
    is(count_chars($text, "\n"), 2000);

    binmode(FOO);
    seek(FOO,0,0);
    { local $/; $text = <FOO> }
    is(count_chars($text, $crlf), 2000);

    SKIP:
    {
	skip_if_miniperl("miniperl can't rely on loading PerlIO::scalar",
			  2 * $ungetc_count + 1);
	skip("no PerlIO::scalar", 2 * $ungetc_count + 1)
	    unless $Config{extensions} =~ m!\bPerlIO/scalar\b!;
	require PerlIO::scalar;
	my $fcontents = join "", map {"$_$crlf"} "a".."zzz";
	open my $fh, "<:crlf", \$fcontents;
	local $/ = "xxx";
	local $_ = <$fh>;
	my $pos = tell $fh; # pos must be behind "xxx", before "\nxxy\n"
	seek $fh, $pos, 0;
	$/ = "\n";
	$s = <$fh>.<$fh>;
	is($s, "\nxxy\n");

        for my $i (0 .. $ungetc_count - 1) {
            my $j = $i % 256;
            is($fh->ungetc($j), $j, "ungetc of $j returns itself");
        }

        for (my $i = $ungetc_count - 1; $i >= 0; $i--) {
            my $j = $i % 256;
            is(ord($fh->getc()), $j, "getc gets back $j");
        }
    }

    ok(close(FOO));

    # binmode :crlf should not cumulate.
    # Try it first once and then twice so that even UNIXy boxes
    # get to exercise this, for DOSish boxes even once is enough.
    # Try also pushing :utf8 first so that there are other layers
    # in between (this should not matter: CRLF layers still should
    # not accumulate).
    for my $utf8 ('', ':utf8') {
	for my $binmode (1..2) {
	    open(FOO, ">$file");
	    # require PerlIO; print PerlIO::get_layers(FOO), "\n";
	    binmode(FOO, "$utf8:crlf") for 1..$binmode;
	    # require PerlIO; print PerlIO::get_layers(FOO), "\n";
	    print FOO "Hello\n";
	    close FOO;
	    open(FOO, "<$file");
	    binmode(FOO);
	    my $foo = scalar <FOO>;
	    close FOO;
	    print join(" ", "#", map { sprintf("%02x", $_) } unpack("C*", $foo)),
	    "\n";
	    like($foo, qr/$crlf$/);
	    unlike($foo, qr/$crcr/);
	}
    }

    {
        # check binmode removes :utf8
        # 133604 - on Win32 :crlf is the base buffer layer, so
        # binmode doesn't remove it, but the binmode handler didn't
        # remove :utf8 either
        ok(open(my $fh, ">", $file), "open a file");
        ok(binmode($fh, ":utf8"), "add :utf8");
        ok((() = grep($_ eq "utf8", PerlIO::get_layers($fh))),
           "check :utf8 set");
        ok(binmode($fh), "remove :utf8");
        ok(!(() = grep($_ eq "utf8", PerlIO::get_layers($fh))),
           "check :utf8 removed");
        close $fh;
    }
}

sub count_chars {
    my($text, $chars) = @_;
    my $seen = 0;
    $seen++ while $text =~ /$chars/g;
    return $seen;
}
