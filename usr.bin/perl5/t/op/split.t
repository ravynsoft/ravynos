#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

plan tests => 197;

$FS = ':';

$_ = 'a:b:c';

($a,$b,$c) = split($FS,$_);

is(join(';',$a,$b,$c), 'a;b;c', 'Split a simple string into scalars.');

@ary = split(/:b:/);
$cnt = split(/:b:/);
is(join("$_",@ary), 'aa:b:cc');
is($cnt, scalar(@ary));

$_ = "abc\n";
my @xyz = (@ary = split(//));
$cnt = split(//);
is(join(".",@ary), "a.b.c.\n");
is($cnt, scalar(@ary));

$_ = "a:b:c::::";
@ary = split(/:/);
$cnt = split(/:/);
is(join(".",@ary), "a.b.c");
is($cnt, scalar(@ary));

$_ = join(':',split(' ',"    a b\tc \t d "));
is($_, 'a:b:c:d');
@ary = split(' ',"    a b\tc \t d ");
$cnt = split(' ',"    a b\tc \t d ");
is($cnt, scalar(@ary));

$_ = join(':',split(/ */,"foo  bar bie\tdoll"));
is($_ , "f:o:o:b:a:r:b:i:e:\t:d:o:l:l");
@ary = split(/ */,"foo  bar bie\tdoll");
$cnt = split(/ */,"foo  bar bie\tdoll");
is($cnt, scalar(@ary));

$_ = join(':', 'foo', split(/ /,'a b  c'), 'bar');
is($_, "foo:a:b::c:bar");
@ary = split(/ /,'a b  c');
$cnt = split(/ /,'a b  c');
is($cnt, scalar(@ary));

# Can we say how many fields to split to?
$_ = join(':', split(' ','1 2 3 4 5 6', 3));
is($_, '1:2:3 4 5 6', "Split into a specified number of fields, defined by a literal");
@ary = split(' ','1 2 3 4 5 6', 3);
$cnt = split(' ','1 2 3 4 5 6', 3);
is($cnt, scalar(@ary), "Check element count from previous test");

# Can we do it as a variable?
$x = 4;
$_ = join(':', split(' ','1 2 3 4 5 6', $x));
is($_, '1:2:3:4 5 6', "Split into a specified number of fields, defined by a scalar variable");
@ary = split(' ','1 2 3 4 5 6', $x);
$cnt = split(' ','1 2 3 4 5 6', $x);
is($cnt, scalar(@ary), "Check element count from previous test");

# Can we do it with the empty pattern?
$_ = join(':', split(//, '123', -1));
is($_, '1:2:3:', "Split with empty pattern and LIMIT == -1");
$_ = join(':', split(//, '123', 0));
is($_, '1:2:3', "Split with empty pattern and LIMIT == 0");
$_ = join(':', split(//, '123', 2));
is($_, '1:23', "Split into specified number of fields with empty pattern");
$_ = join(':', split(//, '123', 6));
is($_, '1:2:3:', "Split with empty pattern and LIMIT > length");
for (-1..5) {
    @ary = split(//, '123', $_);
    $cnt = split(//, '123', $_);
    is($cnt, scalar(@ary), "Check empty pattern element count with LIMIT == $_");
}

# Does the 999 suppress null field chopping?
$_ = join(':', split(/:/,'1:2:3:4:5:6:::', 999));
is($_ , '1:2:3:4:5:6:::');
@ary = split(/:/,'1:2:3:4:5:6:::', 999);
$cnt = split(/:/,'1:2:3:4:5:6:::', 999);
is($cnt, scalar(@ary));

# Splitting without pattern
$_ = "1 2 3 4";
$_ = join(':', split);
is($_ , '1:2:3:4', "Split and join without specifying a split pattern");

# Does assignment to a list imply split to one more field than that?
$foo = runperl( switches => ['-Dt'], stderr => 1, prog => '($a,$b)=split;' );
ok($foo =~ /DEBUGGING/ || $foo =~ /const\n?\Q(IV(3))\E/);

# Can we say how many fields to split to when assigning to a list?
($a,$b) = split(' ','1 2 3 4 5 6', 2);
$_ = join(':',$a,$b);
is($_, '1:2 3 4 5 6', "Storing split output into list of scalars");

# do subpatterns generate additional fields (without trailing nulls)?
$_ = join '|', split(/,|(-)/, "1-10,20,,,");
is($_, "1|-|10||20");
@ary = split(/,|(-)/, "1-10,20,,,");
$cnt = split(/,|(-)/, "1-10,20,,,");
is($cnt, scalar(@ary));

# do subpatterns generate additional fields (with a limit)?
$_ = join '|', split(/,|(-)/, "1-10,20,,,", 10);
is($_, "1|-|10||20||||||");
@ary = split(/,|(-)/, "1-10,20,,,", 10);
$cnt = split(/,|(-)/, "1-10,20,,,", 10);
is($cnt, scalar(@ary));

# is the 'two undefs' bug fixed?
(undef, $a, undef, $b) = qw(1 2 3 4);
is("$a|$b", "2|4");

# .. even for locals?
{
  local(undef, $a, undef, $b) = qw(1 2 3 4);
  is("$a|$b", "2|4");
}

# check splitting of null string
$_ = join('|', split(/x/,   '',-1), 'Z');
is($_, "Z");
@ary = split(/x/,   '',-1);
$cnt = split(/x/,   '',-1);
is($cnt, scalar(@ary));

$_ = join('|', split(/x/,   '', 1), 'Z');
is($_, "Z");
@ary = split(/x/,   '', 1);
$cnt = split(/x/,   '', 1);
is($cnt, scalar(@ary));

$_ = join('|', split(/(p+)/,'',-1), 'Z');
is($_, "Z");
@ary = split(/(p+)/,'',-1);
$cnt = split(/(p+)/,'',-1);
is($cnt, scalar(@ary));

$_ = join('|', split(/.?/,  '',-1), 'Z');
is($_, "Z");
@ary = split(/.?/,  '',-1);
$cnt = split(/.?/,  '',-1);
is($cnt, scalar(@ary));


# Are /^/m patterns scanned?
$_ = join '|', split(/^a/m, "a b a\na d a", 20);
is($_, "| b a\n| d a");
@ary = split(/^a/m, "a b a\na d a", 20);
$cnt = split(/^a/m, "a b a\na d a", 20);
is($cnt, scalar(@ary));

# Are /$/m patterns scanned?
$_ = join '|', split(/a$/m, "a b a\na d a", 20);
is($_, "a b |\na d |");
@ary = split(/a$/m, "a b a\na d a", 20);
$cnt = split(/a$/m, "a b a\na d a", 20);
is($cnt, scalar(@ary));

# Are /^/m patterns scanned?
$_ = join '|', split(/^aa/m, "aa b aa\naa d aa", 20);
is($_, "| b aa\n| d aa");
@ary = split(/^aa/m, "aa b aa\naa d aa", 20);
$cnt = split(/^aa/m, "aa b aa\naa d aa", 20);
is($cnt, scalar(@ary));

# Are /$/m patterns scanned?
$_ = join '|', split(/aa$/m, "aa b aa\naa d aa", 20);
is($_, "aa b |\naa d |");
@ary = split(/aa$/m, "aa b aa\naa d aa", 20);
$cnt = split(/aa$/m, "aa b aa\naa d aa", 20);
is($cnt, scalar(@ary));

# Greedyness:
$_ = "a : b :c: d";
@ary = split(/\s*:\s*/);
$cnt = split(/\s*:\s*/);
is(($res = join(".",@ary)), "a.b.c.d", $res);
is($cnt, scalar(@ary));

# use of match result as pattern (!)
is('p:q:r:s', join ':', split('abc' =~ /b/, 'p1q1r1s'));
@ary = split('abc' =~ /b/, 'p1q1r1s');
$cnt = split('abc' =~ /b/, 'p1q1r1s');
is($cnt, scalar(@ary));

# /^/ treated as /^/m
$_ = join ':', split /^/, "ab\ncd\nef\n";
is($_, "ab\n:cd\n:ef\n","check that split /^/ is treated as split /^/m");

$_ = join ':', split /\A/, "ab\ncd\nef\n";
is($_, "ab\ncd\nef\n","check that split /\A/ is NOT treated as split /^/m");

# see if @a = @b = split(...) optimization works
@list1 = @list2 = split ('p',"a p b c p");
ok(@list1 == @list2 &&
   "@list1" eq "@list2" &&
   @list1 == 2 &&
   "@list1" eq "a   b c ");

# zero-width assertion
$_ = join ':', split /(?=\w)/, "rm b";
is($_, "r:m :b");
@ary = split /(?=\w)/, "rm b";
$cnt = split /(?=\w)/, "rm b";
is($cnt, scalar(@ary));

# unicode splittage

@ary = map {ord} split //, v1.20.300.4000.50000.4000.300.20.1;
$cnt =           split //, v1.20.300.4000.50000.4000.300.20.1;
is("@ary", "1 20 300 4000 50000 4000 300 20 1");
is($cnt, scalar(@ary));

@ary = split(/\x{FE}/, "\x{FF}\x{FE}\x{FD}"); # bug id 20010105.016 (#5088)
$cnt = split(/\x{FE}/, "\x{FF}\x{FE}\x{FD}"); # bug id 20010105.016 (#5088)
ok(@ary == 2 &&
   $ary[0] eq "\xFF"   && $ary[1] eq "\xFD" &&
   $ary[0] eq "\x{FF}" && $ary[1] eq "\x{FD}");
is($cnt, scalar(@ary));

@ary = split(/(\x{FE}\xFE)/, "\xFF\x{FF}\xFE\x{FE}\xFD\x{FD}"); # variant of 31
$cnt = split(/(\x{FE}\xFE)/, "\xFF\x{FF}\xFE\x{FE}\xFD\x{FD}"); # variant of 31
ok(@ary == 3 &&
   $ary[0] eq "\xFF\xFF"     &&
   $ary[0] eq "\x{FF}\xFF"   &&
   $ary[0] eq "\x{FF}\x{FF}" &&
   $ary[1] eq "\xFE\xFE"     &&
   $ary[1] eq "\x{FE}\xFE"   &&
   $ary[1] eq "\x{FE}\x{FE}" &&
   $ary[2] eq "\xFD\xFD"     &&
   $ary[2] eq "\x{FD}\xFD"   &&
   $ary[2] eq "\x{FD}\x{FD}");
is($cnt, scalar(@ary));

{
    my @a = map ord, split(//, join("", map chr, (1234, 123, 2345)));
    my $c =          split(//, join("", map chr, (1234, 123, 2345)));
    is("@a", "1234 123 2345");
    is($c, scalar(@a));
}

{
    my $x = 'A';
    my @a = map ord, split(/$x/, join("", map chr, (1234, ord($x), 2345)));
    my $c =          split(/$x/, join("", map chr, (1234, ord($x), 2345)));
    is("@a", "1234 2345");
    is($c, scalar(@a));
}

{
    # bug id 20000427.003 (#3173)

    use warnings;
    use strict;

    my $sushi = "\x{b36c}\x{5a8c}\x{ff5b}\x{5079}\x{505b}";

    my @charlist = split //, $sushi;
    my $charnum  = split //, $sushi;
    is($charnum, scalar(@charlist));
    my $r = '';
    foreach my $ch (@charlist) {
	$r = $r . " " . sprintf "U+%04X", ord($ch);
    }

    is($r, " U+B36C U+5A8C U+FF5B U+5079 U+505B");
}

{
    my $s = "\x20\x40\x{80}\x{100}\x{80}\x40\x20";

  {
	# bug id 20000426.003 (#3166)

	my ($a, $b, $c) = split(/\x40/, $s);
	ok($a eq "\x20" && $b eq "\x{80}\x{100}\x{80}" && $c eq $a);
  }

    my ($a, $b) = split(/\x{100}/, $s);
    ok($a eq "\x20\x40\x{80}" && $b eq "\x{80}\x40\x20");

    my ($a, $b) = split(/\x{80}\x{100}\x{80}/, $s);
    ok($a eq "\x20\x40" && $b eq "\x40\x20");

  {
	my ($a, $b) = split(/\x40\x{80}/, $s);
	ok($a eq "\x20" && $b eq "\x{100}\x{80}\x40\x20");
  }

    my ($a, $b, $c) = split(/[\x40\x{80}]+/, $s);
    ok($a eq "\x20" && $b eq "\x{100}" && $c eq "\x20");
}

{
    # 20001205.014 (#4844)

    my $a = "ABC\x{263A}";

    my @b = split( //, $a );
    my $c = split( //, $a );
    is($c, scalar(@b));

    is(scalar @b, 4);

    ok(length($b[3]) == 1 && $b[3] eq "\x{263A}");

    $a =~ s/^A/Z/;
    ok(length($a) == 4 && $a eq "ZBC\x{263A}");
}

{
    my @a = split(/\xFE/, "\xFF\xFE\xFD");
    my $b = split(/\xFE/, "\xFF\xFE\xFD");

    ok(@a == 2 && $a[0] eq "\xFF" && $a[1] eq "\xFD");
    is($b, scalar(@a));
}

{
    # check that PMf_WHITE is cleared after \s+ is used
    # reported in <20010627113312.RWGY6087.viemta06@localhost>
    my $r;
    foreach my $pat ( qr/\s+/, qr/ll/ ) {
	$r = join ':' => split($pat, "hello cruel world");
    }
    is($r, "he:o cruel world");
}


{
    # split /(A)|B/, "1B2" should return (1, undef, 2)
    my @x = split /(A)|B/, "1B2";
    my $y = split /(A)|B/, "1B2";
    is($y, scalar(@x));
    ok($x[0] eq '1' and (not defined $x[1]) and $x[2] eq '2');
}

{
    # [perl #17064]
    my $warn;
    local $SIG{__WARN__} = sub { $warn = join '', @_; chomp $warn };
    my $char = "\x{10f1ff}";
    my @a = split /\r?\n/, "$char\n";
    my $b = split /\r?\n/, "$char\n";
    is($b, scalar(@a));
    ok(@a == 1 && $a[0] eq $char && !defined($warn));
}

{
    # [perl #18195]
    for my $u (0, 1) {
	for my $a (0, 1) {
	    $_ = 'readin,database,readout';
	    utf8::upgrade $_ if $u;
	    /(.+)/;
	    my @d = split /[,]/,$1;
	    my $e = split /[,]/,$1;
	    is($e, scalar(@d));
	    is(join (':',@d), 'readin:database:readout', "[perl #18195]");
	}
    }
}

{
    $p="a,b";
    utf8::upgrade $p;
    eval { @a=split(/[, ]+/,$p) };
    eval { $b=split(/[, ]+/,$p) };
    is($b, scalar(@a));
    is ("$@-@a-", '-a b-', '#20912 - split() to array with /[]+/ and utf8');
}

{
    # LATIN SMALL LETTER A WITH DIAERESIS, CYRILLIC SMALL LETTER I
    for my $pattern ("\N{U+E4}", "\x{0437}") {
        utf8::upgrade $pattern;
        my @res;
        for my $str ("a${pattern}b", "axb", "a${pattern}b") {
            @split = split /$pattern/, $str;
            push @res, scalar(@split);
        }
        is($res[0], 2);
        is($res[1], 1);
        is($res[2], 2, '#123469 - split with utf8 pattern after handling non-utf8 EXPR');
    }
}

{
    is (\@a, \@{"a"}, '@a must be global for following test');
    $p="";
    $n = @a = split /,/,$p;
    is ($n, 0, '#21765 - pmreplroot hack used to return undef for 0 iters');
}

{
    # [perl #28938]
    # assigning off the end of the array after a split could leave garbage
    # in the inner elements

    my $x;
    @a = split /,/, ',,,,,';
    $a[3]=1;
    $x = \$a[2];
    is (ref $x, 'SCALAR', '#28938 - garbage after extend');
}

{
    my $src = "ABC \0 FOO \0  XYZ";
    my @s = split(" \0 ", $src);
    my @r = split(/ \0 /, $src);
    my $cs = split(" \0 ", $src);
    my $cr = split(/ \0 /, $src);
    is(scalar(@s), 3);
    is($cs, 3);
    is($cr, 3);
    is($s[0], "ABC");
    is($s[1], "FOO");
    is($s[2]," XYZ");
    is(join(':',@s), join(':',@r));
}

{
    use constant BANG => {};
    () = split m/,/, "", BANG;
    ok(1);
}

{
    # Bug #69875
    # 'Hybrid' scalar-and-array context
    scalar(our @PATH = split /::/, "Font::GlyphNames");
           # 'my' doesn't trigger the bug
    is "@PATH", "Font GlyphNames", "hybrid scalar-and-array context";
}

{
    my @results;
    my $expr= "foo  bar";
    my $cond;

    @results= split(0||" ", $expr);
    is @results, 2, 'split(0||" ") is treated like split(" ")'; #'

    $cond= 0;
    @results= split $cond ? " " : qr/ /, $expr;
    is @results, 3, 'split($cond ? " " : qr/ /, $expr) works as expected (like qr/ /)';
    $cond= 1;
    @results= split $cond ? " " : qr/ /, $expr;
    is @results, 2, 'split($cond ? " " : qr/ /, $expr) works as expected (like " ")';

    $expr = ' a b c ';
    @results = split /\s/, $expr;
    is @results, 4,
        "split on regex of single space metacharacter: captured 4 elements";
    is $results[0], '',
        "split on regex of single space metacharacter: first element is empty string";

    @results = split / /, $expr;
    is @results, 4,
        "split on regex of single whitespace: captured 4 elements";
    is $results[0], '',
        "split on regex of single whitespace: first element is empty string";

    @results = split " ", $expr;
    is @results, 3,
        "split on string of single whitespace: captured 3 elements";
    is $results[0], 'a',
        "split on string of single whitespace: first element is non-empty";

    $expr = " a \tb c ";
    @results = split " ", $expr;
    is @results, 3,
        "split on string of single whitespace: captured 3 elements";
    is $results[0], 'a',
        "split on string of single whitespace: first element is non-empty; multiple contiguous space characters";

    my @seq;
    for my $cond (0,1,0,1,0) {
        $expr = "  foo  ";
        @results = split $cond ? qr/ / : " ", $expr;
        push @seq, scalar(@results) . ":" . $results[-1];
    }
    is join(" ", @seq), "1:foo 3:foo 1:foo 3:foo 1:foo",
        qq{split(\$cond ? qr/ / : " ", "$exp") behaves as expected over repeated similar patterns};
}

SKIP: {
    # RT #130907: unicode_strings feature doesn't work with split ' '

    my ($sp) = grep /\s/u, map chr, reverse 128 .. 255 # prefer \xA0 over \x85
        or skip 'no unicode whitespace found in high-8-bit range', 9;

    for (["$sp$sp. /", "leading unicode whitespace"],
         [".$sp$sp/",  "unicode whitespace separator"],
         [". /$sp$sp", "trailing unicode whitespace"]) {
        my ($str, $desc) = @$_;
        use feature "unicode_strings";
        my @got = split " ", $str;
        is @got, 2, "whitespace split: $desc: field count";
        is $got[0], '.', "whitespace split: $desc: field 0";
        is $got[1], '/', "whitespace split: $desc: field 1";
    }
}

{
    # 'RT #116086: split "\x20" does not work as documented';
    my @results;
    my $expr;
    $expr = ' a b c ';
    @results = split uni_to_native("\x20"), $expr;
    is @results, 3,
        "RT #116086: split on string of single hex-20: captured 3 elements";
    is $results[0], 'a',
        "RT #116086: split on string of single hex-20: first element is non-empty";

    $expr = " a \tb c ";
    @results = split uni_to_native("\x20"), $expr;
    is @results, 3,
        "RT #116086: split on string of single hex-20: captured 3 elements";
    is $results[0], 'a',
        "RT #116086: split on string of single hex-20: first element is non-empty; multiple contiguous space characters";
}

# Nasty interaction between split and use constant
use constant nought => 0;
($a,$b,$c) = split //, $foo, nought;
is nought, 0, 'split does not mangle 0 constants';

*aaa = *bbb;
$aaa[1] = "foobarbaz";
$aaa[1] .= "";
@aaa = split //, $bbb[1];
is "@aaa", "f o o b a r b a z",
   'split-to-array does not free its own argument';

() = @a = split //, "abc";
is "@a", "a b c", '() = split-to-array';

(@a = split //, "abc") = 1..10;
is "@a", '1 2 3', 'assignment to split-to-array (pmtarget/package array)';
{
  my @a;
  (@a = split //, "abc") = 1..10;
  is "@a", '1 2 3', 'assignment to split-to-array (targ/lexical)';
}
(@{\@a} = split //, "abc") = 1..10;
is "@a", '1 2 3', 'assignment to split-to-array (stacked)';

# check that re-evals work

{
    my $c = 0;
    @a = split /-(?{ $c++ })/, "a-b-c";
    is "@a", "a b c", "compile-time re-eval";
    is $c, 2, "compile-time re-eval count";

    my $sep = '-';
    $c = 0;
    @a = split /$sep(?{ $c++ })/, "a-b-c";
    is "@a", "a b c", "run-time re-eval";
    is $c, 2, "run-time re-eval count";
}

# check that my/local @array = split works

{
    my $s = "a:b:c";

    local @a = qw(x y z);
    {
        local @a = split /:/, $s;
        is "@a", "a b c", "local split inside";
    }
    is "@a", "x y z", "local split outside";

    my @b = qw(x y z);
    {
        my @b = split /:/, $s;
        is "@b", "a b c", "my split inside";
    }
    is "@b", "x y z", "my split outside";
}

# check that the (@a = split) optimisation works in scalar/list context

{
    my $s = "a:b:c:d:e";
    my @outer;
    my $outer;
    my @lex;
    local our @pkg;

    $outer = (@lex = split /:/, $s);
    is "@lex",   "a b c d e", "array split: scalar cx lex: inner";
    is $outer,   5,           "array split: scalar cx lex: outer";

    @outer = (@lex = split /:/, $s);
    is "@lex",   "a b c d e", "array split: list cx lex: inner";
    is "@outer", "a b c d e", "array split: list cx lex: outer";

    $outer = (@pkg = split /:/, $s);
    is "@pkg",   "a b c d e", "array split: scalar cx pkg inner";
    is $outer,   5,           "array split: scalar cx pkg outer";

    @outer = (@pkg = split /:/, $s);
    is "@pkg",   "a b c d e", "array split: list cx pkg inner";
    is "@outer", "a b c d e", "array split: list cx pkg outer";

    $outer = (my @a1 = split /:/, $s);
    is "@a1",    "a b c d e", "array split: scalar cx my lex: inner";
    is $outer,   5,           "array split: scalar cx my lex: outer";

    @outer = (my @a2 = split /:/, $s);
    is "@a2",    "a b c d e", "array split: list cx my lex: inner";
    is "@outer", "a b c d e", "array split: list cx my lex: outer";

    $outer = (local @pkg = split /:/, $s);
    is "@pkg",   "a b c d e", "array split: scalar cx local pkg inner";
    is $outer,   5,           "array split: scalar cx local pkg outer";

    @outer = (local @pkg = split /:/, $s);
    is "@pkg",   "a b c d e", "array split: list cx local pkg inner";
    is "@outer", "a b c d e", "array split: list cx local pkg outer";

    $outer = (@{\@lex} = split /:/, $s);
    is "@lex",   "a b c d e", "array split: scalar cx lexref inner";
    is $outer,   5,           "array split: scalar cx lexref outer";

    @outer = (@{\@pkg} = split /:/, $s);
    is "@pkg",   "a b c d e", "array split: list cx pkgref inner";
    is "@outer", "a b c d e", "array split: list cx pkgref outer";


}

# splitting directly to an array wasn't filling unused AvARRAY slots with
# NULL

{
    my @a;
    @a = split(/-/,"-");
    $a[1] = 'b';
    ok eval { $a[0] = 'a'; 1; }, "array split filling AvARRAY: assign 0";
    is "@a", "a b", "array split filling AvARRAY: result";
}

# splitting an empty utf8 string gave an assert failure
{
    my $s = "\x{100}";
    chop $s;
    my @a = split ' ', $s;
    is (+@a, 0, "empty utf8 string");
}

# correct stack adjustments (gh#18232)
{
    sub foo { return @_ }
    my @a = foo(1, scalar split " ", "a b");
    is(join('', @a), "12", "Scalar split to a sub parameter");
}

{
    sub foo { return @_ }
    my @a = foo(1, scalar(@x = split " ", "a b"));
    is(join('', @a), "12", "Split to @x then use scalar result as a sub parameter");
}

fresh_perl_is(<<'CODE', '', {}, "scalar split stack overflow");
map{int"";split//.0>60for"0000000000000000"}split// for"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
CODE

# RT #132334: /o modifier no longer has side effects on split
{
    my @records = (
        { separator => '0', effective => '',  text => 'ab' },
        { separator => ';', effective => ';', text => 'a;b' },
    );

    for (@records) {
        my ($separator, $effective, $text) = @$_{qw(separator effective text)};
        $separator =~ s/0//o;
        is($separator,$effective,"Going to split '$text' with '$separator'");
        my @result = split($separator,$text);
        ok(eq_array(\@result,['a','b']), "Resulting in ('a','b')");
    }
}

# check that the (@ary = split) optimisation survives @ary being modified

fresh_perl_is('my @ary; @ary = split(/\w(?{ @ary[1000] = 1 })/, "abc");',
        '',{},'(@ary = split ...) survives @ary being Renew()ed');
fresh_perl_is('my @ary; @ary = split(/\w(?{ undef @ary })/, "abc");',
        '',{},'(@ary = split ...) survives an (undef @ary)');

# check the (@ary = split) optimisation survives stack-not-refcounted bugs
fresh_perl_is('our @ary; @ary = split(/\w(?{ *ary = 0 })/, "abc");',
        '',{},'(@ary = split ...) survives @ary destruction via typeglob');
fresh_perl_is('my $ary = []; @$ary = split(/\w(?{ $ary = [] })/, "abc");',
        '',{},'(@ary = split ...) survives @ary destruction via reassignment');

# gh18515: check that we spot and flag specific regexps for special treatment
SKIP: {
	skip_if_miniperl("special-case patterns: need dynamic loading", 4);
	for ([ q{" "}, 'WHITE' ],
		[ q{/\\s+/}, 'WHITE' ],
		[ q{/^/}, 'START_ONLY' ],
		[ q{//}, 'NULL' ],
	) {
		my($pattern, $flag) = @$_;
		my $prog = "split $pattern";
		my $expect = qr{^r->extflags:.*\b$flag\b}m;
		fresh_perl_like($prog, $expect, {
			switches => [ '-Mre=Debug,COMPILE', '-c' ],
		}, "special-case pattern for $prog");
	}
}
