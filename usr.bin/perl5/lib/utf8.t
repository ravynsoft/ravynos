#!./perl 

my $has_perlio;

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl'; require './charset_tools.pl';
    unless ($has_perlio = find PerlIO::Layer 'perlio') {
	print <<EOF;
# Since you don't have perlio you might get failures with UTF-8 locales.
EOF
    }
}

use strict;
use warnings;
no utf8; # Ironic, no?

# NOTE!
#
# Think carefully before adding tests here.  In general this should be
# used only for about three categories of tests:
#
# (1) tests that absolutely require 'use utf8', and since that in general
#     shouldn't be needed as the utf8 is being obsoleted, this should
#     have rather few tests.  If you want to test Unicode and regexes,
#     you probably want to go to op/regexp or op/pat; if you want to test
#     split, go to op/split; pack, op/pack; appending or joining,
#     op/append or op/join, and so forth
#
# (2) tests that have to do with Unicode tokenizing (though it's likely
#     that all the other Unicode tests sprinkled around the t/**/*.t are
#     going to catch that)
#
# (3) complicated tests that simultaneously stress so many Unicode features
#     that deciding into which other test script the tests should go to
#     is hard -- maybe consider breaking up the complicated test
#
#

{
    # bug id 20001009.001 (#4409)

    my ($a, $b);

    { use bytes; $a = byte_utf8a_to_utf8n("\xc3\xa4") }
    { use utf8;  $b = uni_to_native("\xe4")     }

    my $test = 68;

    ok($a ne $b);

    { use utf8; ok($a ne $b) }
}


{
    # bug id 20000730.004 (#3599)

    my $smiley = "\x{263a}";

    for my $s ("\x{263a}",
	       $smiley,
		
	       "" . $smiley,
	       "" . "\x{263a}",

	       $smiley    . "",
	       "\x{263a}" . "",
	       ) {
	my $length_chars = length($s);
	my $length_bytes;
	{ use bytes; $length_bytes = length($s) }
	my @regex_chars = $s =~ m/(.)/g;
	my $regex_chars = @regex_chars;
	my @split_chars = split //, $s;
	my $split_chars = @split_chars;
	ok("$length_chars/$regex_chars/$split_chars/$length_bytes" eq
	   "1/1/1/3");
    }

    for my $s ("\x{263a}" . "\x{263a}",
	       $smiley    . $smiley,

	       "\x{263a}\x{263a}",
	       "$smiley$smiley",
	       
	       "\x{263a}" x 2,
	       $smiley    x 2,
	       ) {
	my $length_chars = length($s);
	my $length_bytes;
	{ use bytes; $length_bytes = length($s) }
	my @regex_chars = $s =~ m/(.)/g;
	my $regex_chars = @regex_chars;
	my @split_chars = split //, $s;
	my $split_chars = @split_chars;
	ok("$length_chars/$regex_chars/$split_chars/$length_bytes" eq
	   "2/2/2/6");
    }
}


{
    my $w = 0;
    local $SIG{__WARN__} = sub { print "#($_[0])\n"; $w++ };
    my $x = eval q/"\\/ . "\x{100}" . q/"/;;
   
    ok($w == 0 && $x eq "\x{100}");
}

{
    my $show = q(
                 sub show {
                   my $result;
                   $result .= '>' . join (',', map {ord} split //, $_) . '<'
                     foreach @_;
                   $result;
                 }
                 1;
                );
    eval $show or die $@; # We don't expect this sub definition to fail.
    my $progfile = 'utf' . $$;
    END {unlink_all $progfile}

    # 64 is '@' in ASCII, ' ' in EBCDIC
    # 193 is not punctuation in either ASCII nor EBCDIC
    my (@char);
    foreach (64, 193, 257, 65532) {
      my $char = chr $_;
      utf8::encode($char);
      # I don't want to use map {ord} and I've no need to hardcode the UTF
      # version
      my $charsubst = $char;
      $charsubst =~ s/(.)/ord ($1) . ','/ge;
      chop $charsubst;
      # Not testing this one against map {ord}
      my $char_as_ord
          = join " . ", map {sprintf 'chr (%d)', ord $_} split //, $char;
      push @char, [$_, $char, $charsubst, $char_as_ord];
    }
    my $malformed = $::IS_ASCII
                    ? "\xE1\xA0"
                    : I8_to_native("\xE6\xA0");
    # Now we've done all the UTF8 munching hopefully we're safe
    my @tests = (
             ['check our detection program works',
              'my @a = ("'.chr(64).'\x2A", ""); $b = show @a', qr/^>64,42<><$/],
             ['check literal 8 bit input',
              '$a = "' . chr (193) . '"; $b = show $a', qr/^>193<$/],
             ['check no utf8; makes no change',
              'no utf8; $a = "' . chr (193) . '"; $b = show $a', qr/^>193<$/],
             # Now we do the real byte sequences that are valid UTF8
             (map {
               ["the utf8 sequence for chr $_->[0]",
                qq{\$a = "$_->[1]"; \$b = show \$a}, qr/^>$_->[2]<$/],
               ["no utf8; for the utf8 sequence for chr $_->[0]",
                qq(no utf8; \$a = "$_->[1]"; \$b = show \$a), qr/^>$_->[2]<$/],
               ["use utf8; for the utf8 sequence for chr $_->[0]",
                qq(use utf8; \$a = "$_->[1]"; \$b = show \$a), qr/^>$_->[0]<$/],
              } @char),
             # Interpolation of hex characters needs to take place now, as we're
             # testing feeding malformed utf8 into perl. Bug now fixed was an
             # "out of memory" error. We really need the "" [rather than qq()
             # or q()] to get the best explosion.
             ["!Feed malformed utf8 into perl.", <<"BANG",
    use utf8; %a = ("$malformed" =>"sterling");
    print 'start'; printf '%x,', ord \$_ foreach keys %a; print "end\n";
BANG
	      qr/^Malformed UTF-8 character: .*? \(unexpected non-continuation byte/
	     ],
            );
    foreach (@tests) {
        my ($why, $prog, $expect) = @$_;
        open P, ">", $progfile or die "Can't open '$progfile': $!";
        binmode(P, ":bytes") if $has_perlio;
	print P $show, $prog, '; print $b'
            or die "Print to 'progfile' failed: $!";
        close P or die "Can't close '$progfile': $!";
        if ($why =~ s/^!//) {
            print "# Possible delay...\n";
        } else {
            print "# $prog\n";
        }
        my $result = runperl ( stderr => 1, progfile => $progfile );
        like ($result, $expect, $why);
    }
    print
        "# Again! Again! [but this time as eval, and not the explosive one]\n";
    # and now we've safely done them all as separate files, check that the
    # evals do the same thing. Hopefully doing it later successfully decouples
    # the previous tests from anything messy that may go wrong with the evals.
    foreach (@tests) {
        my ($why, $prog, $expect) = @$_;
        next if $why =~ m/^!/; # Goes bang.
        my $result = eval $prog;
        if ($@) {
            print "# prog is $prog\n";
            print "# \$\@=", _qq($@), "\n";
        }
        like ($result, $expect, $why);
    }

    # See what the tokeniser does with hash keys.
    print "# What does the tokeniser do with utf8 hash keys?\n";
    @tests = (map {
        # This is the control - I don't expect it to fail
        ["assign utf8 for chr $_->[0] to a hash",
         qq(my \$a = "$_->[1]"; my %h; \$h{\$a} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["no utf8; assign utf8 for chr $_->[0] to a hash",
         qq(no utf8; my \$a = "$_->[1]"; my %h; \$h{\$a} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["use utf8; assign utf8 for chr $_->[0] to a hash",
         qq(use utf8; my \$a = "$_->[1]"; my %h; \$h{\$a} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{chr $_->[0]}; \$b),
         qr/^>$_->[0]<$/],
        # Now check literal $h{"x"} constructions.
        ["\$h{\"x\"} construction, where x is utf8 for chr $_->[0]",
         qq(my \$a = "$_->[1]"; my %h; \$h{"$_->[1]"} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["no utf8; \$h{\"x\"} construction, where x is utf8 for chr $_->[0]",
         qq(no utf8; my \$a = "$_->[1]"; my %h; \$h{"$_->[1]"} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["use utf8; \$h{\"x\"} construction, where x is utf8 for chr $_->[0]",
         qq(use utf8; my \$a = "$_->[1]"; my %h; \$h{"$_->[1]"} = 1;
            my \$b = show keys %h; \$b .= 'F' unless \$h{chr $_->[0]}; \$b),
         qr/^>$_->[0]<$/],
        # Now check "x" => constructions.
        ["assign \"x\"=>1 to a hash, where x is utf8 for chr $_->[0]",
         qq(my \$a = "$_->[1]"; my %h; %h = ("$_->[1]" => 1);
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["no utf8; assign \"x\"=>1 to a hash, where x is utf8 for chr $_->[0]",
         qq(no utf8; my \$a = "$_->[1]"; my %h; %h = ("$_->[1]" => 1);
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["use utf8; assign \"x\"=>1 to a hash, where x is utf8 for chr $_->[0]",
         qq(use utf8; my \$a = "$_->[1]"; my %h; %h = ("$_->[1]" => 1);
            my \$b = show keys %h; \$b .= 'F' unless \$h{chr $_->[0]}; \$b),
         qr/^>$_->[0]<$/],
        # Check copies of hashes made from literal utf8 keys
        ["assign utf8 for chr $_->[0] to a hash, then copy it",
         qq(my \$a = "$_->[1]"; my %i; \$i{\$a} = 1; my %h = %i;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["no utf8; assign utf8 for chr $_->[0] to a hash, then copy it",
         qq(no utf8; my \$a = "$_->[1]"; my %i; \$i{\$a} = 1;; my %h = %i;
            my \$b = show keys %h; \$b .= 'F' unless \$h{$_->[3]}; \$b),
         qr/^>$_->[2]<$/],
        ["use utf8; assign utf8 for chr $_->[0] to a hash, then copy it",
         qq(use utf8; my \$a = "$_->[1]"; my %i; \$i{\$a} = 1; my %h = %i;
            my \$b = show keys %h; \$b .= 'F' unless \$h{chr $_->[0]}; \$b),
         qr/^>$_->[0]<$/],
     } @char);
    foreach (@tests) {
        my ($why, $prog, $expect) = @$_;
        # print "# $prog\n";
        my $result = eval $prog;
        like ($result, $expect, $why);
    }
}

#
# bug fixed by change #17928
# separate perl used because we rely on 'strict' not yet loaded;
# before the patch, the eval died with an error like:
#   "my" variable $strict::VERSION can't be in a package
#
SKIP: {
    skip("Haven't bothered to port this to EBCDIC non-1047", 1) if $::IS_EBCDIC
                                                                && ord '^' != 95;
    if ($::IS_ASCII) {
        ok('' eq runperl(prog => <<'CODE'), "change #17928");
            my $code = qq{ my \$\xe3\x83\x95\xe3\x83\xbc = 5; };
        {
            use utf8;
            eval $code;
            print $@ if $@;
        }
CODE
    }
    else {
        ok('' eq runperl(prog => <<'CODE'), "change #17928");
            my $code = qq{ my \$\xCE\x47\x64\xCE\x48\x70 = 5; };
        {
            use utf8;
            eval $code;
            print $@ if $@;
        }
CODE
    }
}

{
    use utf8;
    $a = <<'END';
0 ....... 1 ....... 2 ....... 3 ....... 4 ....... 5 ....... 6 ....... 7 ....... 
END
    my (@i, $s);

    @i = ();
    push @i, $s = index($a, '6');     # 60
    push @i, $s = index($a, '.', $s); # next . after 60 is 62
    push @i, $s = index($a, '5');     # 50
    push @i, $s = index($a, '.', $s); # next . after 52 is 52
    push @i, $s = index($a, '7');     # 70 
    push @i, $s = index($a, '.', $s); # next . after 70 is 72
    push @i, $s = index($a, '4');     # 40
    push @i, $s = index($a, '.', $s); # next . after 40 is 42
    is("@i", "60 62 50 52 70 72 40 42", "utf8 heredoc index");

    @i = ();
    push @i, $s = rindex($a, '6');     # 60
    push @i, $s = rindex($a, '.', $s); # previous . before 60 is 58
    push @i, $s = rindex($a, '5');     # 50
    push @i, $s = rindex($a, '.', $s); # previous . before 52 is 48
    push @i, $s = rindex($a, '7');     # 70 
    push @i, $s = rindex($a, '.', $s); # previous . before 70 is 68
    push @i, $s = rindex($a, '4');     # 40
    push @i, $s = rindex($a, '.', $s); # previous . before 40 is 38
    is("@i", "60 58 50 48 70 68 40 38", "utf8 heredoc rindex");

    @i = ();
    push @i, $s =  index($a, '6');     # 60
    push @i,  index($a, '.', $s);      # next     . after  60 is 62
    push @i, rindex($a, '.', $s);      # previous . before 60 is 58
    push @i, $s = rindex($a, '5');     # 60
    push @i,  index($a, '.', $s);      # next     . after  50 is 52
    push @i, rindex($a, '.', $s);      # previous . before 50 is 48
    push @i, $s =  index($a, '7', $s); # 70
    push @i,  index($a, '.', $s);      # next     . after  70 is 72
    push @i, rindex($a, '.', $s);      # previous . before 70 is 68
    is("@i", "60 62 58 50 52 48 70 72 68", "utf8 heredoc index and rindex");
}

SKIP: {
    skip("Haven't bothered to port this to EBCDIC non-1047", 1) if $::IS_EBCDIC
                                                                && ord '^' != 95;
    use utf8;
    if ($::IS_ASCII) {
        is eval qq{q \xc3\xbc test \xc3\xbc . qq\xc2\xb7 test \xc2\xb7},
        ' test  test ',
        "utf8 quote delimiters [perl #16823]";
    }
    else {
        is eval qq{q \x8B\x70 test \x8B\x70 . qq\x80\x66 test \x80\x66},
        ' test  test ',
        "utf8 quote delimiters [perl #16823]";
    }
}

# Test the "internals".

{
    my $a = "A";
    my $b = chr(0x0FF);
    my $c = chr(0x0DF);  # FF is invariant in many EBCDIC pages, so is not a
                         # fair test of 'beyond'; but DF is variant (in all
                         # supported EBCDIC pages so far), so make 2 'beyond'
                         # tests
    my $d = chr(0x100);

    ok( utf8::valid($a), "utf8::valid basic");
    ok( utf8::valid($b), "utf8::valid beyond");
    ok( utf8::valid($c), "utf8::valid beyond");
    ok( utf8::valid($d), "utf8::valid unicode");

    ok(!utf8::is_utf8($a), "!utf8::is_utf8 basic");
    ok(!utf8::is_utf8($b), "!utf8::is_utf8 beyond");
    ok(!utf8::is_utf8($c), "!utf8::is_utf8 beyond");
    ok( utf8::is_utf8($d), "utf8::is_utf8 unicode");

    is(utf8::upgrade($a), 1, "utf8::upgrade basic");
    if ($::IS_EBCDIC) { # EBCDIC.
	is(utf8::upgrade($b), 1, "utf8::upgrade beyond");
    } else {
	is(utf8::upgrade($b), 2, "utf8::upgrade beyond");
    }
    is(utf8::upgrade($c), 2, "utf8::upgrade beyond");
    is(utf8::upgrade($d), 2, "utf8::upgrade unicode");

    is($a, "A",       "basic");
    is($b, "\xFF",    "beyond");
    is($c, "\xDF",    "beyond");
    is($d, "\x{100}", "unicode");

    ok( utf8::valid($a), "utf8::valid basic");
    ok( utf8::valid($b), "utf8::valid beyond");
    ok( utf8::valid($c), "utf8::valid beyond");
    ok( utf8::valid($d), "utf8::valid unicode");

    ok( utf8::is_utf8($a), "utf8::is_utf8 basic");
    ok( utf8::is_utf8($b), "utf8::is_utf8 beyond");
    ok( utf8::is_utf8($c), "utf8::is_utf8 beyond");
    ok( utf8::is_utf8($d), "utf8::is_utf8 unicode");

    is(utf8::downgrade($a), 1, "utf8::downgrade basic");
    is(utf8::downgrade($b), 1, "utf8::downgrade beyond");
    is(utf8::downgrade($c), 1, "utf8::downgrade beyond");

    is($a, "A",       "basic");
    is($b, "\xFF",    "beyond");
    is($c, "\xDF",    "beyond");

    ok( utf8::valid($a), "utf8::valid basic");
    ok( utf8::valid($b), "utf8::valid beyond");
    ok( utf8::valid($c), "utf8::valid beyond");

    ok(!utf8::is_utf8($a), "!utf8::is_utf8 basic");
    ok(!utf8::is_utf8($b), "!utf8::is_utf8 beyond");
    ok(!utf8::is_utf8($c), "!utf8::is_utf8 beyond");

    utf8::encode($a);
    utf8::encode($b);
    utf8::encode($c);
    utf8::encode($d);

    is($a, "A",       "basic");
    if ($::IS_EBCDIC) { # EBCDIC.
	is(length($b), 1, "beyond length");
    } else {
	is(length($b), 2, "beyond length");
    }
    is(length($c), 2, "beyond length");
    is(length($d), 2, "unicode length");

    ok(utf8::valid($a), "utf8::valid basic");
    ok(utf8::valid($b), "utf8::valid beyond");
    ok(utf8::valid($c), "utf8::valid beyond");
    ok(utf8::valid($d), "utf8::valid unicode");

    # encode() clears the UTF-8 flag (unlike upgrade()).
    ok(!utf8::is_utf8($a), "!utf8::is_utf8 basic");
    ok(!utf8::is_utf8($b), "!utf8::is_utf8 beyond");
    ok(!utf8::is_utf8($c), "!utf8::is_utf8 beyond");
    ok(!utf8::is_utf8($d), "!utf8::is_utf8 unicode");

    utf8::decode($a);
    utf8::decode($b);
    utf8::decode($c);
    utf8::decode($d);

    is($a, "A",       "basic");
    is($b, "\xFF",    "beyond");
    is($c, "\xDF",    "beyond");
    is($d, "\x{100}", "unicode");

    ok(utf8::valid($a), "!utf8::valid basic");
    ok(utf8::valid($b), "!utf8::valid beyond");
    ok(utf8::valid($c), "!utf8::valid beyond");
    ok(utf8::valid($d), " utf8::valid unicode");

    ok(!utf8::is_utf8($a), "!utf8::is_utf8 basic");
    if ($::IS_EBCDIC) { # EBCDIC.
	ok( utf8::is_utf8(pack('U',0x0ff)), " utf8::is_utf8 beyond");
    } else {
	ok( utf8::is_utf8($b), " utf8::is_utf8 beyond"); # $b stays in UTF-8.
    }
    ok( utf8::is_utf8($c), " utf8::is_utf8 beyond"); # $c stays in UTF-8.
    ok( utf8::is_utf8($d), " utf8::is_utf8 unicode");
}

{
    eval {utf8::encode("£")};
    like($@, qr/^Modification of a read-only value attempted/,
	 "utf8::encode should refuse to touch read-only values");
}

{
    # Make sure utf8::decode respects copy-on-write [perl #91834].
    # Hash keys are the easiest way to test this.
    my $name = byte_utf8a_to_utf8n("\x{c3}\x{b3}");
    my ($k1) = keys %{ { $name=>undef } };
    my $k2 = $name;
    utf8::decode($k1);
    utf8::decode($k2);
    my $h = { $k1 => 1, $k2 => 2 };
    is join('', keys %$h), $k2, 'utf8::decode respects copy-on-write';
}

{
    # Make sure utf8::decode does not modify read-only scalars
    # [perl #91850].
    
    my $name = byte_utf8a_to_utf8n("\x{c3}\x{b3}");
    Internals::SvREADONLY($name, 1);
    eval { utf8::decode($name) };
    like $@, qr/^Modification of a read-only/,
	'utf8::decode respects readonliness';
}

{
    # utf8::decode should stringify refs [perl #91852].

    package eieifg { use overload '""'      => sub { main::byte_utf8a_to_utf8n("\x{c3}\x{b3}") },
                                   fallback => 1 }

    my $name = bless[], eieifg::;
    utf8::decode($name);
    is $name, uni_to_native("\xf3"), 'utf8::decode flattens references';
}

{
    # What do the utf8::* functions do when given a reference? A test
    # for a behavior change that made this start dying as of
    # v5.15.6-407-gc710240 due to a fix for [perl #91852]:
    #
    #    ./miniperl -Ilib -wle 'use strict; print $]; my $s = shift; my $s_ref = \$s; utf8::decode($s_ref); print $$s_ref' hlagh
    my %expected = (
        'utf8::is_utf8'           => { returns => "hlagh" },
        'utf8::valid'             => { returns => "hlagh" },
        'utf8::encode'            => { error => qr/Can't use string .*? as a SCALAR ref/},
        'utf8::decode'            => { error => qr/Can't use string .*? as a SCALAR ref/},
        'utf8::upgrade'           => { error => qr/Can't use string .*? as a SCALAR ref/ },
        'utf8::downgrade'         => { returns => "hlagh" },
        'utf8::native_to_unicode' => { returns => "hlagh" },
        'utf8::unicode_to_native' => { returns => "hlagh" },
    );
    for my $func (sort keys %expected) { # sort just so it's deterministic wrt diffing *.t output
        my $code = sprintf q[
            use strict;
            my $s = "hlagh";
            my $r = \$s;
            my $dummy = %s($r);
            $$r;
        ], $func;
        my $ret = eval $code or my $error = $@;
        if (my $error_rx = $expected{$func}->{error}) {
            if (defined $error) {
                like $error, $error_rx, "The $func function should die with an error matching $error_rx";
            } else {
                fail("We were expecting an error when calling the $func function but got a value of '$ret' instead");
            }
        } elsif (my $returns = $expected{$func}->{returns}) {
            is($ret, $returns, "The $func function lives and returns '$returns' as expected");
        } else {
            die "PANIC: Internal Error"
        }
    }
}

{
    my $a = "456" . uni_to_native("\xb6");
    utf8::upgrade($a);

    my $b = "123456" . uni_to_native("\xb6");
    $b =~ s/^...//;
    utf8::upgrade($b);
    is($b, $a, "utf8::upgrade OffsetOK");
}

{
    fresh_perl_like ('use utf8; utf8::moo()',
		     qr/Undefined subroutine utf8::moo/, {stderr=>1},
		    "Check Carp is loaded for AUTOLOADing errors")
}

{
    # failure of is_utf8_char() without NATIVE_TO_UTF on EBCDIC (0260..027F)
    ok(utf8::valid(chr(0x250)), "0x250");
    ok(utf8::valid(chr(0x260)), "0x260");
    ok(utf8::valid(chr(0x270)), "0x270");
    ok(utf8::valid(chr(0x280)), "0x280");
}

{
   use utf8;
   ok( !utf8::is_utf8( "asd"         ), "Wasteful format - qq{}" );
   ok( !utf8::is_utf8( 'asd'         ), "Wasteful format - q{}" );
   ok( !utf8::is_utf8( qw(asd)       ), "Wasteful format - qw{}" );
   ok( !utf8::is_utf8( (asd => 1)[0] ), "Wasteful format - =>" );
   ok( !utf8::is_utf8( -asd          ), "Wasteful format - -word" );
   no warnings 'bareword';
   ok( !utf8::is_utf8( asd::         ), "Wasteful format - word::" );
   no warnings 'reserved';
   no strict 'subs';
   ok( !utf8::is_utf8( asd           ), "Wasteful format - bareword" );
}

{
    my @highest =
	(undef, 0x7F, 0x7FF, 0xFFFF, 0x1FFFFF, 0x3FFFFFF, 0x7FFFFFFF);
    my @step =
	(undef, undef, 0x40, 0x1000, 0x40000, 0x1000000, 0x40000000);

    foreach my $length (6, 5, 4, 3, 2) {
	my $high = $highest[$length];
	while ($high > $highest[$length - 1]) {
	    my $low = $high - $step[$length] + 1;
	    $low = $highest[$length - 1] + 1 if $low <= $highest[$length - 1];
	    ok(utf8::valid(do {no warnings 'utf8'; chr $low}),
	       sprintf "chr %x, length $length is valid", $low);
	    ok(utf8::valid(do {no warnings 'utf8'; chr $high}),
	       sprintf "chr %x, length $length is valid", $high);
	    $high -= $step[$length];
	}
    }
}

# #80190 update pos, and cached length/position-mapping after
# utf8 upgrade/downgrade, encode/decode

for my $pos (0..5) {

    my $p;
    my $utf8_bytes = byte_utf8a_to_utf8n("\xc8\x81\xe3\xbf\xbf");
    my $s = "A$utf8_bytes\x{100}";
    chop($s);

    pos($s) = $pos;
    # also sets cache
    is(length($s), 6,		   "(pos $pos) len before    utf8::downgrade");
    is(pos($s),    $pos,	   "(pos $pos) pos before    utf8::downgrade");
    utf8::downgrade($s);
    is(length($s), 6,		   "(pos $pos) len after     utf8::downgrade");
    is(pos($s),    $pos,	   "(pos $pos) pos after     utf8::downgrade");
    is($s, "A$utf8_bytes","(pos $pos) str after     utf8::downgrade");
    utf8::decode($s);
    is(length($s), 3,		   "(pos $pos) len after  D; utf8::decode");
    is(pos($s),    undef,	   "(pos $pos) pos after  D; utf8::decode");
    is($s, "A\x{201}\x{3fff}",	   "(pos $pos) str after  D; utf8::decode");
    utf8::encode($s);
    is(length($s), 6,		   "(pos $pos) len after  D; utf8::encode");
    is(pos($s),    undef,	   "(pos $pos) pos after  D; utf8::encode");
    is($s, "A$utf8_bytes","(pos $pos) str after  D; utf8::encode");

    $s = "A$utf8_bytes";

    pos($s) = $pos;
    is(length($s), 6,		   "(pos $pos) len before    utf8::upgrade");
    is(pos($s),    $pos,	   "(pos $pos) pos before    utf8::upgrade");
    utf8::upgrade($s);
    is(length($s), 6,		   "(pos $pos) len after     utf8::upgrade");
    is(pos($s),    $pos,	   "(pos $pos) pos after     utf8::upgrade");
    is($s, "A$utf8_bytes","(pos $pos) str after     utf8::upgrade");
    utf8::decode($s);
    is(length($s), 3,		   "(pos $pos) len after  U; utf8::decode");
    is(pos($s),    undef,	   "(pos $pos) pos after  U; utf8::decode");
    is($s, "A\x{201}\x{3fff}",	   "(pos $pos) str after  U; utf8::decode");
    utf8::encode($s);
    is(length($s), 6,		   "(pos $pos) len after  U; utf8::encode");
    is(pos($s),    undef,	   "(pos $pos) pos after  U; utf8::encode");
    is($s, "A$utf8_bytes","(pos $pos) str after  U; utf8::encode");
}

SKIP: {
    skip("Test only valid on ASCII platform", 1) unless $::IS_ASCII;
    require Config;
    skip("Test needs a B module, which is lacking in this Perl", 1)
        if $Config::Config{'extensions'} !~ /\bB\b/;

    my $out = runperl ( switches => ["-XMO=Concise"],
                    prog => 'utf8::unicode_to_native(0x41);
                             utf8::native_to_unicode(0x42)',
                    stderr => 1 );
    unlike($out, qr/entersub/,
            "utf8::unicode_to_native() and native_to_unicode() optimized out");
}


# [perl #119043] utf8::upgrade should not croak on read-only COWs
for(__PACKAGE__) {
	eval { utf8::upgrade($_) };
	is $@, "", 'no error with utf8::upgrade on read-only COW';
}

is(utf8::upgrade(undef), undef, "Returns undef for undef input"); # GH #20419

# This one croaks, but not because the scalar is read-only
eval "package \x{100};\n" . <<'END'
    for(__PACKAGE__) {
	eval { utf8::downgrade($_) };
	::like $@, qr/^Wide character/,
	    'right error with utf8::downgrade on read-only COW';
    }
    1
END
or die $@;

done_testing();
