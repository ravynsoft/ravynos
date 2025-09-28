#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

skip_all_if_miniperl("no dynamic loading on miniperl, no File::Spec (used by charnames)");
plan(tests => 147);

{
    # check the special casing of split /\s/ and unicode
    use charnames qw(:full);
    # below test data is extracted from
    # PropList-5.0.0.txt
    # Date: 2006-06-07, 23:22:52 GMT [MD]
    #
    # Unicode Character Database
    # Copyright (c) 1991-2006 Unicode, Inc.
    # For terms of use, see http://www.unicode.org/terms_of_use.html
    # For documentation, see UCD.html
    my @spaces=(
	ord("\t"),      # Cc       <control-0009>
	ord("\n"),      # Cc       <control-000A>
	# not PerlSpace # Cc       <control-000B>
	ord("\f"),      # Cc       <control-000C>
	ord("\r"),      # Cc       <control-000D>
	ord(" "),       # Zs       SPACE
	ord("\N{NEL}"), # Cc       <control-0085>
	ord("\N{NO-BREAK SPACE}"),
			# Zs       NO-BREAK SPACE
        0x1680,         # Zs       OGHAM SPACE MARK
        0x2000..0x200A, # Zs  [11] EN QUAD..HAIR SPACE
        0x2028,         # Zl       LINE SEPARATOR
        0x2029,         # Zp       PARAGRAPH SEPARATOR
        0x202F,         # Zs       NARROW NO-BREAK SPACE
        0x205F,         # Zs       MEDIUM MATHEMATICAL SPACE
        0x3000          # Zs       IDEOGRAPHIC SPACE
    );
    #diag "Have @{[0+@spaces]} to test\n";
    foreach my $cp (@spaces) {
	my $msg = sprintf "Space: U+%04x", $cp;
        my $space = chr($cp);
        my $str="A:$space:B\x{FFFD}";
        chop $str;

        my @res=split(/\s+/,$str);
        my $cnt=split(/\s+/,$str);
        ok(@res == 2 && join('-',@res) eq "A:-:B", "$msg - /\\s+/");
	is($cnt, scalar(@res), "$msg - /\\s+/ (count)");

        my $s2 = "$space$space:A:$space$space:B\x{FFFD}";
        chop $s2;

        my @r2 = split(' ',$s2);
	my $c2 = split(' ',$s2);
        ok(@r2 == 2 && join('-', @r2) eq ":A:-:B",  "$msg - ' '");
	is($c2, scalar(@r2), "$msg - ' ' (count)");

        my @r3 = split(/\s+/, $s2);
        my $c3 = split(/\s+/, $s2);
        ok(@r3 == 3 && join('-', @r3) eq "-:A:-:B", "$msg - /\\s+/ No.2");
	is($c3, scalar(@r3), "$msg - /\\s+/ No.2 (count)");
    }

    { # RT #114808
        warning_is(
            sub {
                $p=chr(0x100);
                for (".","ab\x{101}def") {
                    @q = split /$p/
                }
            },
            undef,
            'no warnings when part of split cant match non-utf8'
        );
    }

}

{
    # Check empty pattern with specified field count on Unicode string
    my $string = "\x{100}\x{101}\x{102}";
    $_ = join(':', split(//, $string, 2));
    is($_, "\x{100}:\x{101}\x{102}",
            "Split into specified number of fields with empty pattern");
    @ary = split(//, $string, 2);
    $cnt = split(//, $string, 2);
    is($cnt, scalar(@ary), "Check element count from previous test");
}
