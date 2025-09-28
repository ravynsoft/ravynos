#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

$|=1;

{
    my $wide = v256;
    use bytes;
    my $ordwide = ord($wide);
    printf "# under use bytes ord(v256) = 0x%02x\n", $ordwide;
    skip_all('UTF-8-centric tests (not valid for UTF-EBCDIC)') if $ordwide == 140;
    # This could be ported to EBCDIC, but a lot of trouble.
    # ext/XS-APItest/t/utf8.t contains comprehensive tests for both platforms

    if ($ordwide != 196) {
	printf "# v256 starts with 0x%02x\n", $ordwide;
    }
}

no utf8;

my $is64bit = length sprintf("%x", ~0) > 8;

foreach (<DATA>) {
    if (/^(?:\d+(?:\.\d+)?)\s/ || /^#/) {
	# print "# $_\n";
    } elsif (my ($id, $okay, $Unicode, $byteslen, $hex, $charslen, $experr)
	     = /^(\d+\.\d+\.\d+[bu]?)   # ID
		\s+(y|n|N-?\d+(?:,\d+)?)  # expect to pass or fail
                                          # 'n' means expect one diagnostic
                                          # 'N\d+'     means expect this
                                          #            number of diagnostics
                                          # 'N\d+,\d+' means expect the first
                                          #            number of diagnostics
                                          #            on a 32-bit system; the
                                          #            second number on a
                                          #            64-bit one
                \s+([0-9a-f]{1,8}(?:,[0-9a-f]{1,8})*|-) # Unicode characters
                \s+(\d+)                # number of octets
                \s+([0-9a-f]{2}(?::[0-9a-f]{2})*)       # octets in hex
                \s+(\d+|-)              # number of characters
                (?:\s+(.+))?            # expected error (or comment)
                $/x) {
	my @hex = split(/:/, $hex);
	is(scalar @hex, $byteslen, 'Amount of hex tallies with byteslen');
	my $octets = join '', map {chr hex $_} @hex;
	is(length $octets, $byteslen, 'Number of octets tallies with byteslen');
	if ($okay eq 'y') {
	    my @chars = map {hex $_} split ',', $Unicode;
	    is(scalar @chars, $charslen, 'Amount of hex tallies with charslen');
	    my @got;
	    warning_is(sub {@got = unpack 'C0U*', $octets}, undef,
		       "No warnings expected for $id");
	    is("@got", "@chars", 'Got expected Unicode characters');
	} elsif ($okay eq 'n') {
	    isnt($experr, '', "Expected warning for $id provided");
	    warnings_like(sub {unpack 'C0U*', $octets}, [qr/$experr/],
			 "Only expected warning for $id");
	} elsif ($okay !~ /^N-?(\d+)(?:,(\d+))?/) {
	    is($okay, 'n', "Confused test description for $id");
	} else {
	    my $expect32 = $1;
            my $expect64 = $2 // $expect32;
            my $expect = ($is64bit) ? $expect64 : $expect32;
	    my @warnings;

	    {
		local $SIG{__WARN__} = sub {
		    print "# $id: @_";
		    push @warnings, "@_";
		};
		unpack 'C0U*', $octets;
	    }

	    unless (is(scalar @warnings, $expect, "Expected number of warnings for $id seen")) {
                note(join "", "Got:\n", @warnings);
            }
	    isnt($experr, '', "Expected first warning for $id provided");

            my $message;
            my $after = "";
            if ($expect64 < $expect32 && ! $is64bit) {
                # This is needed for code points above IV_MAX
                #if (       substr($octets, 0, 1) gt "\xfe"
                #    || (   substr($octets, 0, 1) eq "\xfe"
                #        && length $octets > 1
                #        && substr($octets, 1, 1) le "\xbf"
                #        && substr($octets, 1, 1) ge "\x80"))
                #{
                    like($warnings[0], qr/overflow/, "overflow warning for $id seen");
                    shift @warnings;
                    $after .= "overflow";
                #}
            }

            # The data below assumes that if there is both a 'short' and
            # 'non-continuation' malformation, the latter has precedence.  But
            # that has changed, and rather than mess with the data, this works
            # around that.
            if (   @warnings > 1
                && $warnings[0] =~ /short/
                && $warnings[1] =~ /unexpected non-continuation/)
            {
                $after .= " and " if $after;
                $after .= "short";
                shift @warnings;
            }
            $after = "after $after " if $after;

	    like($warnings[0], qr/$experr/, "Expected first warning ${after}for $id seen");
	    local $::TODO;
	    if ($expect < 0) {
		$expect = -$expect;
		$::TODO = "Markus Kuhn states that $expect invalid sequences should be signalled";
	    }

	}
    } else {
	fail("unknown format '$_'");
    }
}

done_testing();

# This table is based on Markus Kuhn's UTF-8 Decode Stress Tester,
# http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt,
# version dated 2015-08-28.
#
# See the code that parses these lines for comments as to the column meanings

__DATA__
1	Correct UTF-8
1.1.1 y 3ba,1f79,3c3,3bc,3b5	11	ce:ba:e1:bd:b9:cf:83:ce:bc:ce:b5	5
2	Boundary conditions
2.1	First possible sequence of certain length
2.1.1 y 0		1	00	1
2.1.2 y 80		2	c2:80	1
2.1.3 y 800		3	e0:a0:80	1
2.1.4 y 10000		4	f0:90:80:80	1
2.1.5 y 200000		5	f8:88:80:80:80	1
2.1.6 y 4000000		6	fc:84:80:80:80:80	1
2.2	Last possible sequence of certain length
2.2.1 y 7f		1	7f	1
2.2.2 y 7ff		2	df:bf	1
# The ffff is legal by default since 872c91ae155f6880
2.2.3 y ffff		3	ef:bf:bf	1	character 0xffff
2.2.4 y 1fffff		4	f7:bf:bf:bf	1
2.2.5 y 3ffffff		5	fb:bf:bf:bf:bf	1
2.2.6 y 7fffffff	6	fd:bf:bf:bf:bf:bf	1
2.3	Other boundary conditions
2.3.1 y d7ff		3	ed:9f:bf	1
2.3.2 y e000		3	ee:80:80	1
2.3.3 y fffd		3	ef:bf:bd	1
2.3.4 y 10ffff		4	f4:8f:bf:bf	1
2.3.5 y 110000		4	f4:90:80:80	1
3	Malformed sequences
3.1	Unexpected continuation bytes
3.1.1 n -		1	80	-	unexpected continuation byte 0x80
3.1.2 n -		1	bf	-	unexpected continuation byte 0xbf
3.1.3 N2 -		2	80:bf	-	unexpected continuation byte 0x80
3.1.4 N3 -		3	80:bf:80	-	unexpected continuation byte 0x80
3.1.5 N4 -		4	80:bf:80:bf	-	unexpected continuation byte 0x80
3.1.6 N5 -		5	80:bf:80:bf:80	-	unexpected continuation byte 0x80
3.1.7 N6 -		6	80:bf:80:bf:80:bf	-	unexpected continuation byte 0x80
3.1.8 N7 -		7	80:bf:80:bf:80:bf:80	-	unexpected continuation byte 0x80
3.1.9 N64 -	64	80:81:82:83:84:85:86:87:88:89:8a:8b:8c:8d:8e:8f:90:91:92:93:94:95:96:97:98:99:9a:9b:9c:9d:9e:9f:a0:a1:a2:a3:a4:a5:a6:a7:a8:a9:aa:ab:ac:ad:ae:af:b0:b1:b2:b3:b4:b5:b6:b7:b8:b9:ba:bb:bc:bd:be:bf	-	unexpected continuation byte 0x80
3.2	Lonely start characters
3.2.1 N34 -	64 	c0:20:c1:20:c2:20:c3:20:c4:20:c5:20:c6:20:c7:20:c8:20:c9:20:ca:20:cb:20:cc:20:cd:20:ce:20:cf:20:d0:20:d1:20:d2:20:d3:20:d4:20:d5:20:d6:20:d7:20:d8:20:d9:20:da:20:db:20:dc:20:dd:20:de:20:df:20	-	unexpected non-continuation byte 0x20, immediately after start byte 0xc0
3.2.2 N17 -	32	e0:20:e1:20:e2:20:e3:20:e4:20:e5:20:e6:20:e7:20:e8:20:e9:20:ea:20:eb:20:ec:20:ed:20:ee:20:ef:20	-	unexpected non-continuation byte 0x20, immediately after start byte 0xe0
3.2.3 N9 -	16	f0:20:f1:20:f2:20:f3:20:f4:20:f5:20:f6:20:f7:20	-	unexpected non-continuation byte 0x20, immediately after start byte 0xf0
3.2.4 N6 -	8	f8:20:f9:20:fa:20:fb:20	-	unexpected non-continuation byte 0x20, immediately after start byte 0xf8
3.2.5 N4 -	4	fc:20:fd:20	-	unexpected non-continuation byte 0x20, immediately after start byte 0xfc
3.3	Sequences with last continuation byte missing
3.3.1 N2 -	1	c0	-	1 byte available, need 2
3.3.2 N2 -	2	e0:80	-	2 bytes available, need 3
3.3.3 N2 -	3	f0:80:80	-	3 bytes available, need 4
3.3.4 N2 -	4	f8:80:80:80	-	4 bytes available, need 5
3.3.5 N2 -	5	fc:80:80:80:80	-	5 bytes available, need 6
3.3.6 n -	1	df	-	1 byte available, need 2
3.3.7 n -	2	ef:bf	-	2 bytes available, need 3
3.3.8 n -	3	f7:bf:bf	-	3 bytes available, need 4
3.3.9 n -	4	fb:bf:bf:bf	-	4 bytes available, need 5
3.3.10 n -	5	fd:bf:bf:bf:bf	-	5 bytes available, need 6
3.4	Concatenation of incomplete sequences
3.4.1 N15 -	30	c0:e0:80:f0:80:80:f8:80:80:80:fc:80:80:80:80:df:ef:bf:f7:bf:bf:fb:bf:bf:bf:fd:bf:bf:bf:bf	-	unexpected non-continuation byte 0xe0, immediately after start byte 0xc0
3.5	Impossible bytes (but not with Perl's extended UTF-8)
3.5.1 n -	1	fe	-	1 byte available, need 7
3.5.2 n -	1	ff	-	1 byte available, need 13
3.5.3 N7 -	4	fe:fe:ff:ff	-	byte 0xfe
4	Overlong sequences
4.1	Examples of an overlong ASCII character
4.1.1 n -	2	c0:af	-	overlong
4.1.2 n -	3	e0:80:af	-	overlong
4.1.3 n -	4	f0:80:80:af	-	overlong
4.1.4 n -	5	f8:80:80:80:af	-	overlong
4.1.5 n -	6	fc:80:80:80:80:af	-	overlong
4.2	Maximum overlong sequences
4.2.1 n -	2	c1:bf	-	overlong
4.2.2 n -	3	e0:9f:bf	-	overlong
4.2.3 n -	4	f0:8f:bf:bf	-	overlong
4.2.4 n -	5	f8:87:bf:bf:bf	-	overlong
4.2.5 n -	6	fc:83:bf:bf:bf:bf	-	overlong
4.3	Overlong representation of the NUL character
4.3.1 n -	2	c0:80	-	overlong
4.3.2 n -	3	e0:80:80	-	overlong
4.3.3 n -	4	f0:80:80:80	-	overlong
4.3.4 n -	5	f8:80:80:80:80	-	overlong
4.3.5 n -	6	fc:80:80:80:80:80	-	overlong
5	Illegal code positions
5.1	Single UTF-16 surrogates
5.1.1 y d800	3	ed:a0:80	1	UTF-16 surrogate 0xd800
5.1.2 y db7f	3	ed:ad:bf	1	UTF-16 surrogate 0xdb7f
5.1.3 y db80	3	ed:ae:80	1	UTF-16 surrogate 0xdb80
5.1.4 y dbff	3	ed:af:bf	1	UTF-16 surrogate 0xdbff
5.1.5 y dc00	3	ed:b0:80	1	UTF-16 surrogate 0xdc00
5.1.6 y df80	3	ed:be:80	1	UTF-16 surrogate 0xdf80
5.1.7 y dfff	3	ed:bf:bf	1	UTF-16 surrogate 0xdfff
5.2	Paired UTF-16 surrogates
5.2.1 y d800,dc00	6	ed:a0:80:ed:b0:80	2	UTF-16 surrogates 0xd800, dc00
5.2.2 y d800,dfff	6	ed:a0:80:ed:bf:bf	2	UTF-16 surrogates 0xd800, dfff
5.2.3 y db7f,dc00	6	ed:ad:bf:ed:b0:80	2	UTF-16 surrogates 0xdb7f, dc00
5.2.4 y db7f,dfff	6	ed:ad:bf:ed:bf:bf	2	UTF-16 surrogates 0xdb7f, dfff
5.2.5 y db80,dc00	6	ed:ae:80:ed:b0:80	2	UTF-16 surrogates 0xdb80, dc00
5.2.6 y db80,dfff	6	ed:ae:80:ed:bf:bf	2	UTF-16 surrogates 0xdb80, dfff
5.2.7 y dbff,dc00	6	ed:af:bf:ed:b0:80	2	UTF-16 surrogates 0xdbff, dc00
5.2.8 y dbff,dfff	6	ed:af:bf:ed:bf:bf	2	UTF-16 surrogates 0xdbff, dfff
5.3	Other illegal code positions
5.3.1 y fffe	3	ef:bf:be	1	byte order mark 0xfffe
# The ffff is legal by default since 872c91ae155f6880
5.3.2 y ffff	3	ef:bf:bf	1	non-character 0xffff
5.3.3 y fdd0	3	ef:b7:90	1	non-character 0xfdd0
5.3.3 y	fdd1	3	ef:b7:91	1	non-character 0xfdd1
5.3.3 y	fdd2	3	ef:b7:92	1	non-character 0xfdd2
5.3.3 y	fdd3	3	ef:b7:93	1	non-character 0xfdd3
5.3.3 y	fdd4	3	ef:b7:94	1	non-character 0xfdd4
5.3.3 y	fdd5	3	ef:b7:95	1	non-character 0xfdd5
5.3.3 y	fdd6	3	ef:b7:96	1	non-character 0xfdd6
5.3.3 y	fdd7	3	ef:b7:97	1	non-character 0xfdd7
5.3.3 y	fdd8	3	ef:b7:98	1	non-character 0xfdd8
5.3.3 y	fdd9	3	ef:b7:99	1	non-character 0xfdd9
5.3.3 y	fdda	3	ef:b7:9a	1	non-character 0xfdda
5.3.3 y	fddb	3	ef:b7:9b	1	non-character 0xfddb
5.3.3 y	fddc	3	ef:b7:9c	1	non-character 0xfddc
5.3.3 y	fddd	3	ef:b7:9d	1	non-character 0xfddd
5.3.3 y	fdde	3	ef:b7:9e	1	non-character 0xfdde
5.3.3 y	fddf	3	ef:b7:9f	1	non-character 0xfddf
5.3.3 y	fde0	3	ef:b7:a0	1	non-character 0xfde0
5.3.3 y	fde1	3	ef:b7:a1	1	non-character 0xfde1
5.3.3 y	fde2	3	ef:b7:a2	1	non-character 0xfde2
5.3.3 y	fde3	3	ef:b7:a3	1	non-character 0xfde3
5.3.3 y	fde4	3	ef:b7:a4	1	non-character 0xfde4
5.3.3 y	fde5	3	ef:b7:a5	1	non-character 0xfde5
5.3.3 y	fde6	3	ef:b7:a6	1	non-character 0xfde6
5.3.3 y	fde7	3	ef:b7:a7	1	non-character 0xfde7
5.3.3 y	fde8	3	ef:b7:a8	1	non-character 0xfde8
5.3.3 y	fde9	3	ef:b7:a9	1	non-character 0xfde9
5.3.3 y	fdea	3	ef:b7:aa	1	non-character 0xfdea
5.3.3 y	fdeb	3	ef:b7:ab	1	non-character 0xfdeb
5.3.3 y	fdec	3	ef:b7:ac	1	non-character 0xfdec
5.3.3 y	fded	3	ef:b7:ad	1	non-character 0xfded
5.3.3 y	fdee	3	ef:b7:ae	1	non-character 0xfdee
5.3.3 y	fdef	3	ef:b7:af	1	non-character 0xfdef
5.3.4 y 1fffe	4	f0:9f:bf:be	1	non-character 0x1fffe
5.3.4 y 1ffff	4	f0:9f:bf:bf	1	non-character 0x1ffff
5.3.4 y 2fffe	4	f0:af:bf:be	1	non-character 0x2fffe
5.3.4 y 2ffff	4	f0:af:bf:bf	1	non-character 0x2ffff
5.3.4 y 3fffe	4	f0:bf:bf:be	1	non-character 0x3fffe
5.3.4 y 3ffff	4	f0:bf:bf:bf	1	non-character 0x3ffff
5.3.4 y 4fffe	4	f1:8f:bf:be	1	non-character 0x4fffe
5.3.4 y 4ffff	4	f1:8f:bf:bf	1	non-character 0x4ffff
5.3.4 y 5fffe	4	f1:9f:bf:be	1	non-character 0x5fffe
5.3.4 y 5ffff	4	f1:9f:bf:bf	1	non-character 0x5ffff
5.3.4 y 6fffe	4	f1:af:bf:be	1	non-character 0x6fffe
5.3.4 y 6ffff	4	f1:af:bf:bf	1	non-character 0x6ffff
5.3.4 y 7fffe	4	f1:bf:bf:be	1	non-character 0x7fffe
5.3.4 y 7ffff	4	f1:bf:bf:bf	1	non-character 0x7ffff
5.3.4 y 8fffe	4	f2:8f:bf:be	1	non-character 0x8fffe
5.3.4 y 8ffff	4	f2:8f:bf:bf	1	non-character 0x8ffff
5.3.4 y 9fffe	4	f2:9f:bf:be	1	non-character 0x9fffe
5.3.4 y 9ffff	4	f2:9f:bf:bf	1	non-character 0x9ffff
5.3.4 y afffe	4	f2:af:bf:be	1	non-character 0xafffe
5.3.4 y affff	4	f2:af:bf:bf	1	non-character 0xaffff
5.3.4 y bfffe	4	f2:bf:bf:be	1	non-character 0xbfffe
5.3.4 y bffff	4	f2:bf:bf:bf	1	non-character 0xbffff
5.3.4 y cfffe	4	f3:8f:bf:be	1	non-character 0xcfffe
5.3.4 y cffff	4	f3:8f:bf:bf	1	non-character 0xcffff
5.3.4 y dfffe	4	f3:9f:bf:be	1	non-character 0xdfffe
5.3.4 y dffff	4	f3:9f:bf:bf	1	non-character 0xdffff
5.3.4 y efffe	4	f3:af:bf:be	1	non-character 0xefffe
5.3.4 y effff	4	f3:af:bf:bf	1	non-character 0xeffff
5.3.4 y ffffe	4	f3:bf:bf:be	1	non-character 0xffffe
5.3.4 y fffff	4	f3:bf:bf:bf	1	non-character 0xfffff
5.3.4 y 10fffe	4	f4:8f:bf:be	1	non-character 0x10fffe
5.3.4 y 10ffff	4	f4:8f:bf:bf	1	non-character 0x10ffff
