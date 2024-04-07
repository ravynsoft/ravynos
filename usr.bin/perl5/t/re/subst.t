#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config; import Config;
    require constant;
    constant->import(constcow => *Config::{NAME});
    require './charset_tools.pl';
    require './loc_tools.pl';
}

plan(tests => 281);

$_ = 'david';
$a = s/david/rules/r;
ok( $_ eq 'david' && $a eq 'rules', 'non-destructive substitute' );

$a = "david" =~ s/david/rules/r;
ok( $a eq 'rules', 's///r with constant' );

#[perl #127635] failed with -DPERL_NO_COW perl build (George smoker uses flag)
#Modification of a read-only value attempted at ../t/re/subst.t line 23.
$a = constcow =~ s/Config/David/r;
ok( $a eq 'David::', 's///r with COW constant' );

$a = "david" =~ s/david/"is"."great"/er;
ok( $a eq 'isgreat', 's///er' );

$a = "daviddavid" =~ s/david/cool/gr;
ok( $a eq 'coolcool', 's///gr' );

$a = 'david';
$b = $a =~ s/david/sucks/r =~ s/sucks/rules/r;
ok( $a eq 'david' && $b eq 'rules', 'chained s///r' );

$a = 'david';
$b = $a =~ s/xxx/sucks/r;
ok( $a eq 'david' && $b eq 'david', 'non matching s///r' );

$a = 'david';
for (0..2) {
    ok( 'david' =~ s/$a/rules/ro eq 'rules', 's///ro '.$_ );
}

$a = 'david';
eval '$b = $a !~ s/david/is great/r';
like( $@, qr{Using !~ with s///r doesn't make sense}, 's///r !~ operator gives error' );

{
        no warnings 'uninitialized';
        $a = undef;
        $b = $a =~ s/left/right/r;
        ok ( !defined $a && !defined $b, 's///r with undef input' );

        use warnings;
        warning_like(sub { $b = $a =~ s/left/right/r },
		     qr/^Use of uninitialized value/,
		     's///r Uninitialized warning');

        $a = 'david';
        warning_like(sub {eval 's/david/sucks/r; 1'},
		     qr/^Useless use of non-destructive substitution/,
		     's///r void context warning');
}

$a = '';
$b = $a =~ s/david/rules/r;
ok( $a eq '' && $b eq '', 's///r on empty string' );

$_ = 'david';
@b = s/david/rules/r;
ok( $_ eq 'david' && $b[0] eq 'rules', 's///r in list context' );

# Magic value and s///r
require Tie::Scalar;
tie $m, 'Tie::StdScalar';  # makes $a magical
$m = "david";
$b = $m =~ s/david/rules/r;
ok( $m eq 'david' && $b eq 'rules', 's///r with magic input' );

$m = $b =~ s/rules/david/r;
ok( defined tied($m), 's///r magic isn\'t lost' );

$b = $m =~ s/xxx/yyy/r;
ok( ! defined tied($b), 's///r magic isn\'t contagious' );

my $ref = \("aaa" =~ s/aaa/bbb/r);
refcount_is $ref, 1, 's///r does not leak';
$ref = \("aaa" =~ s/aaa/bbb/rg);
refcount_is $ref, 1, 's///rg does not leak';

$x = 'foo';
$_ = "x";
s/x/\$x/;
ok( $_ eq '$x', ":$_: eq :\$x:" );

$_ = "x";
s/x/$x/;
ok( $_ eq 'foo', ":$_: eq :foo:" );

$_ = "x";
s/x/\$x $x/;
ok( $_ eq '$x foo', ":$_: eq :\$x foo:" );

$b = 'cd';
($a = 'abcdef') =~ s<(b${b}e)>'\n$1';
ok( $1 eq 'bcde' && $a eq 'a\n$1f', ":$1: eq :bcde: ; :$a: eq :a\\n\$1f:" );

$a = 'abacada';
ok( ($a =~ s/a/x/g) == 4 && $a eq 'xbxcxdx' );

ok( ($a =~ s/a/y/g) == 0 && $a eq 'xbxcxdx' );

ok( ($a =~ s/b/y/g) == 1 && $a eq 'xyxcxdx' );

$_ = 'ABACADA';
ok( /a/i && s///gi && $_ eq 'BCD' );

$_ = '\\' x 4;
ok( length($_) == 4 );
$snum = s/\\/\\\\/g;
ok( $_ eq '\\' x 8 && $snum == 4 );

$_ = '\/' x 4;
ok( length($_) == 8 );
$snum = s/\//\/\//g;
ok( $_ eq '\\//' x 4 && $snum == 4 );
ok( length($_) == 12 );

$_ = 'aaaXXXXbbb';
s/^a//;
ok( $_ eq 'aaXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/a//;
ok( $_ eq 'aaXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/^a/b/;
ok( $_ eq 'baaXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/a/b/;
ok( $_ eq 'baaXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/aa//;
ok( $_ eq 'aXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/aa/b/;
ok( $_ eq 'baXXXXbbb' );

$_ = 'aaaXXXXbbb';
s/b$//;
ok( $_ eq 'aaaXXXXbb' );

$_ = 'aaaXXXXbbb';
s/b//;
ok( $_ eq 'aaaXXXXbb' );

$_ = 'aaaXXXXbbb';
s/bb//;
ok( $_ eq 'aaaXXXXb' );

$_ = 'aaaXXXXbbb';
s/aX/y/;
ok( $_ eq 'aayXXXbbb' );

$_ = 'aaaXXXXbbb';
s/Xb/z/;
ok( $_ eq 'aaaXXXzbb' );

$_ = 'aaaXXXXbbb';
s/aaX.*Xbb//;
ok( $_ eq 'ab' );

$_ = 'aaaXXXXbbb';
s/bb/x/;
ok( $_ eq 'aaaXXXXxb' );

# now for some unoptimized versions of the same.

$_ = 'aaaXXXXbbb';
$x ne $x || s/^a//;
ok( $_ eq 'aaXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/a//;
ok( $_ eq 'aaXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/^a/b/;
ok( $_ eq 'baaXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/a/b/;
ok( $_ eq 'baaXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/aa//;
ok( $_ eq 'aXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/aa/b/;
ok( $_ eq 'baXXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/b$//;
ok( $_ eq 'aaaXXXXbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/b//;
ok( $_ eq 'aaaXXXXbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/bb//;
ok( $_ eq 'aaaXXXXb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/aX/y/;
ok( $_ eq 'aayXXXbbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/Xb/z/;
ok( $_ eq 'aaaXXXzbb' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/aaX.*Xbb//;
ok( $_ eq 'ab' );

$_ = 'aaaXXXXbbb';
$x ne $x || s/bb/x/;
ok( $_ eq 'aaaXXXXxb' );

$_ = 'abc123xyz';
s/(\d+)/$1*2/e;              # yields 'abc246xyz'
ok( $_ eq 'abc246xyz' );
s/(\d+)/sprintf("%5d",$1)/e; # yields 'abc  246xyz'
ok( $_ eq 'abc  246xyz' );
s/(\w)/$1 x 2/eg;            # yields 'aabbcc  224466xxyyzz'
ok( $_ eq 'aabbcc  224466xxyyzz' );

$_ = "aaaaa";
ok( y/a/b/ == 5 );
ok( y/a/b/ == 0 );
ok( y/b// == 5 );
ok( y/b/c/s == 5 );
ok( y/c// == 1 );
ok( y/c//d == 1 );
ok( $_ eq "" );

$_ = "Now is the %#*! time for all good men...";
ok( ($x=(y/a-zA-Z //cd)) == 7 );
ok( y/ / /s == 8 );

$_ = 'abcdefghijklmnopqrstuvwxyz0123456789';
tr/a-z/A-Z/;

ok( $_ eq 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789' );

# same as tr/A-Z/a-z/;
if (defined $Config{ebcdic} && $Config{ebcdic} eq 'define') {	# EBCDIC.
    no utf8;
    y[\301-\351][\201-\251];
} else {		# Ye Olde ASCII.  Or something like it.
    y[\101-\132][\141-\172];
}

ok( $_ eq 'abcdefghijklmnopqrstuvwxyz0123456789' );

SKIP: {
    skip("ASCII-centric test",1) unless (ord("+") == ord(",") - 1
			              && ord(",") == ord("-") - 1
			              && ord("a") == ord("b") - 1
			              && ord("b") == ord("c") - 1);
    $_ = '+,-';
    tr/+--/a-c/;
    ok( $_ eq 'abc' );
}

$_ = '+,-';
tr/+\--/a\/c/;
ok( $_ eq 'a,/' );

$_ = '+,-';
tr/-+,/ab\-/;
ok( $_ eq 'b-a' );


# test recursive substitutions
# code based on the recursive expansion of makefile variables

my %MK = (
    AAAAA => '$(B)', B=>'$(C)', C => 'D',			# long->short
    E     => '$(F)', F=>'p $(G) q', G => 'HHHHH',	# short->long
    DIR => '$(UNDEFINEDNAME)/xxx',
);
sub var { 
    my($var,$level) = @_;
    return "\$($var)" unless exists $MK{$var};
    return exp_vars($MK{$var}, $level+1); # can recurse
}
sub exp_vars { 
    my($str,$level) = @_;
    $str =~ s/\$\((\w+)\)/var($1, $level+1)/ge; # can recurse
    #warn "exp_vars $level = '$str'\n";
    $str;
}

ok( exp_vars('$(AAAAA)',0)           eq 'D' );
ok( exp_vars('$(E)',0)               eq 'p HHHHH q' );
ok( exp_vars('$(DIR)',0)             eq '$(UNDEFINEDNAME)/xxx' );
ok( exp_vars('foo $(DIR)/yyy bar',0) eq 'foo $(UNDEFINEDNAME)/xxx/yyy bar' );

$_ = "abcd";
s/(..)/$x = $1, m#.#/eg;
ok( $x eq "cd", 'a match nested in the RHS of a substitution' );

# Subst and lookbehind

$_="ccccc";
$snum = s/(?<!x)c/x/g;
ok( $_ eq "xxxxx" && $snum == 5 );

$_="ccccc";
$snum = s/(?<!x)(c)/x/g;
ok( $_ eq "xxxxx" && $snum == 5 );

$_="foobbarfoobbar";
$snum = s/(?<!r)foobbar/foobar/g;
ok( $_ eq "foobarfoobbar" && $snum == 1 );

$_="foobbarfoobbar";
$snum = s/(?<!ar)(foobbar)/foobar/g;
ok( $_ eq "foobarfoobbar" && $snum == 1 );

$_="foobbarfoobbar";
$snum = s/(?<!ar)foobbar/foobar/g;
ok( $_ eq "foobarfoobbar" && $snum == 1 );

eval 's{foo} # this is a comment, not a delimiter
       {bar};';
ok( ! @?, 'parsing of split subst with comment' );

$snum = eval '$_="exactly"; s sxsys;m 3(yactl)3;$1';
is( $snum, 'yactl', 'alpha delimiters are allowed' );

$_="baacbaa";
$snum = tr/a/b/s;
ok( $_ eq "bbcbb" && $snum == 4,
    'check if squashing works at the end of string' );

$_ = "ab";
ok( s/a/b/ == 1 );

$_ = <<'EOL';
     $url = new URI::URL "http://www/";   die if $url eq "xXx";
EOL
$^R = 'junk';

$foo = ' $@%#lowercase $@%# lowercase UPPERCASE$@%#UPPERCASE' .
  ' $@%#lowercase$@%#lowercase$@%# lowercase lowercase $@%#lowercase' .
  ' lowercase $@%#MiXeD$@%# ';

$snum =
s{  \d+          \b [,.;]? (?{ 'digits' })
   |
    [a-z]+       \b [,.;]? (?{ 'lowercase' })
   |
    [A-Z]+       \b [,.;]? (?{ 'UPPERCASE' })
   |
    [A-Z] [a-z]+ \b [,.;]? (?{ 'Capitalized' })
   |
    [A-Za-z]+    \b [,.;]? (?{ 'MiXeD' })
   |
    [A-Za-z0-9]+ \b [,.;]? (?{ 'alphanumeric' })
   |
    \s+                    (?{ ' ' })
   |
    [^A-Za-z0-9\s]+          (?{ '$@%#' })
}{$^R}xg;
ok( $_ eq $foo );
ok( $snum == 31 );

$_ = 'a' x 6;
$snum = s/a(?{})//g;
ok( $_ eq '' && $snum == 6 );

$_ = 'x' x 20; 
$snum = s/(\d*|x)/<$1>/g; 
$foo = '<>' . ('<x><>' x 20) ;
ok( $_ eq $foo && $snum == 41 );

$t = 'aaaaaaaaa'; 

$_ = $t;
pos = 6;
$snum = s/\Ga/xx/g;
ok( $_ eq 'aaaaaaxxxxxx' && $snum == 3 );

$_ = $t;
pos = 6;
$snum = s/\Ga/x/g;
ok( $_ eq 'aaaaaaxxx' && $snum == 3 );

$_ = $t;
pos = 6;
s/\Ga/xx/;
ok( $_ eq 'aaaaaaxxaa' );

$_ = $t;
pos = 6;
s/\Ga/x/;
ok( $_ eq 'aaaaaaxaa' );

$_ = $t;
$snum = s/\Ga/xx/g;
ok( $_ eq 'xxxxxxxxxxxxxxxxxx' && $snum == 9 );

$_ = $t;
$snum = s/\Ga/x/g;
ok( $_ eq 'xxxxxxxxx' && $snum == 9 );

$_ = $t;
s/\Ga/xx/;
ok( $_ eq 'xxaaaaaaaa' );

$_ = $t;
s/\Ga/x/;
ok( $_ eq 'xaaaaaaaa' );

$_ = 'aaaa';
$snum = s/\ba/./g;
ok( $_ eq '.aaa' && $snum == 1 );

eval q% s/a/"b"}/e %;
ok( $@ =~ /Bad evalled substitution/ );
eval q% ($_ = "x") =~ s/(.)/"$1 "/e %;
ok( $_ eq "x " and !length $@ );
$x = $x = 'interp';
eval q% ($_ = "x") =~ s/x(($x)*)/"$1"/e %;
ok( $_ eq '' and !length $@ );

$_ = "C:/";
ok( !s/^([a-z]:)/\u$1/ );

$_ = "Charles Bronson";
$snum = s/\B\w//g;
ok( $_ eq "C B" && $snum == 12 );

{
    use utf8;
    my $s = "H\303\266he";
    my $l = my $r = $s;
    $l =~ s/[^\w]//g;
    $r =~ s/[^\w\.]//g;
    is($l, $r, "use utf8 \\w");
}

my $pv1 = my $pv2  = "Andreas J. K\303\266nig";
$pv1 =~ s/A/\x{100}/;
substr($pv2,0,1) = "\x{100}";
is($pv1, $pv2);

{
    {   
	# Gregor Chrupala <gregor.chrupala@star-group.net>
	use utf8;
	$a = 'Espa&ntilde;a';
	$a =~ s/&ntilde;/ñ/;
	like($a, qr/ñ/, "use utf8 RHS");
    }

    {
	use utf8;
	$a = 'España España';
	$a =~ s/ñ/&ntilde;/;
	like($a, qr/ñ/, "use utf8 LHS");
    }

    {
	use utf8;
	$a = 'España';
	$a =~ s/ñ/ñ/;
	like($a, qr/ñ/, "use utf8 LHS and RHS");
    }
}

{
    # SADAHIRO Tomoyuki <bqw10602@nifty.com>

    $a = "\x{100}\x{101}";
    $a =~ s/\x{101}/\xFF/;
    like($a, qr/\xFF/);
    is(length($a), 2, "SADAHIRO utf8 s///");

    $a = "\x{100}\x{101}";
    $a =~ s/\x{101}/"\xFF"/e;
    like($a, qr/\xFF/);
    is(length($a), 2);

    $a = "\x{100}\x{101}";
    $a =~ s/\x{101}/\xFF\xFF\xFF/;
    like($a, qr/\xFF\xFF\xFF/);
    is(length($a), 4);

    $a = "\x{100}\x{101}";
    $a =~ s/\x{101}/"\xFF\xFF\xFF"/e;
    like($a, qr/\xFF\xFF\xFF/);
    is(length($a), 4);

    $a = "\xFF\x{101}";
    $a =~ s/\xFF/\x{100}/;
    like($a, qr/\x{100}/);
    is(length($a), 2);

    $a = "\xFF\x{101}";
    $a =~ s/\xFF/"\x{100}"/e;
    like($a, qr/\x{100}/);
    is(length($a), 2);

    $a = "\xFF";
    $a =~ s/\xFF/\x{100}/;
    like($a, qr/\x{100}/);
    is(length($a), 1);

    $a = "\xFF";
    $a =~ s/\xFF/"\x{100}"/e;
    like($a, qr/\x{100}/);
    is(length($a), 1);
}

{
    # subst with mixed utf8/non-utf8 type
    my($ua, $ub, $uc, $ud) = ("\x{101}", "\x{102}", "\x{103}", "\x{104}");
    my($na, $nb) = ("\x{ff}", "\x{fe}");
    my $a = "$ua--$ub";
    my $b;
    ($b = $a) =~ s/--/$na/;
    is($b, "$ua$na$ub", "s///: replace non-utf8 into utf8");
    ($b = $a) =~ s/--/--$na--/;
    is($b, "$ua--$na--$ub", "s///: replace long non-utf8 into utf8");
    ($b = $a) =~ s/--/$uc/;
    is($b, "$ua$uc$ub", "s///: replace utf8 into utf8");
    ($b = $a) =~ s/--/--$uc--/;
    is($b, "$ua--$uc--$ub", "s///: replace long utf8 into utf8");
    $a = "$na--$nb";
    ($b = $a) =~ s/--/$ua/;
    is($b, "$na$ua$nb", "s///: replace utf8 into non-utf8");
    ($b = $a) =~ s/--/--$ua--/;
    is($b, "$na--$ua--$nb", "s///: replace long utf8 into non-utf8");

    # now with utf8 pattern
    $a = "$ua--$ub";
    ($b = $a) =~ s/-($ud)?-/$na/;
    is($b, "$ua$na$ub", "s///: replace non-utf8 into utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/--$na--/;
    is($b, "$ua--$na--$ub", "s///: replace long non-utf8 into utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/$uc/;
    is($b, "$ua$uc$ub", "s///: replace utf8 into utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/--$uc--/;
    is($b, "$ua--$uc--$ub", "s///: replace long utf8 into utf8 (utf8 pattern)");
    $a = "$na--$nb";
    ($b = $a) =~ s/-($ud)?-/$ua/;
    is($b, "$na$ua$nb", "s///: replace utf8 into non-utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/--$ua--/;
    is($b, "$na--$ua--$nb", "s///: replace long utf8 into non-utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/$na/;
    is($b, "$na$na$nb", "s///: replace non-utf8 into non-utf8 (utf8 pattern)");
    ($b = $a) =~ s/-($ud)?-/--$na--/;
    is($b, "$na--$na--$nb", "s///: replace long non-utf8 into non-utf8 (utf8 pattern)");
}

$_ = 'aaaa';
$r = 'x';
$s = s/a(?{})/$r/g;
is("<$_> <$s>", "<xxxx> <4>", "[perl #7806]");

$_ = 'aaaa';
$s = s/a(?{})//g;
is("<$_> <$s>", "<> <4>", "[perl #7806]");

# [perl #19048] Coredump in silly replacement
{
    local $^W = 0;
    $_="abcdef\n";
    s!.!!eg;
    is($_, "\n", "[perl #19048]");
}

# [perl #17757] interaction between saw_ampersand and study
{
    my $f = eval q{ $& };
    $f = "xx";
    study $f;
    $f =~ s/x/y/g;
    is($f, "yy", "[perl #17757]");
}

# [perl #20684] returned a zero count
$_ = "1111";
is(s/(??{1})/2/eg, 4, '#20684 s/// with (??{..}) inside');

# [perl #20682] @- not visible in replacement
$_ = "123";
/(2)/;	# seed @- with something else
s/(1)(2)(3)/$#- (@-)/;
is($_, "3 (0 0 1 2)", '#20682 @- not visible in replacement');

# [perl #20682] $^N not visible in replacement
$_ = "abc";
/(a)/; s/(b)|(c)/-$^N/g;
is($_,'a-b-c','#20682 $^N not visible in replacement');

# [perl #22351] perl bug with 'e' substitution modifier
my $name = "chris";
{
    no warnings 'uninitialized';
    $name =~ s/hr//e;
}
is($name, "cis", q[#22351 bug with 'e' substitution modifier]);


# [perl #34171] $1 didn't honour 'use bytes' in s//e
{
    my $s="\x{100}";
    my $x;
    {
	use bytes;
	$s=~ s/(..)/$x=$1/e
    }
    is(length($x), 2, '[perl #34171]');
}


{ # [perl #27940] perlbug: [\x00-\x1f] works, [\c@-\c_] does not
    my $c;

    ($c = "\x20\c@\x30\cA\x40\cZ\x50\c_\x60") =~ s/[\c@-\c_]//g;
    is($c, "\x20\x30\x40\x50\x60", "s/[\\c\@-\\c_]//g");

    ($c = "\x20\x00\x30\x01\x40\x1A\x50\x1F\x60") =~ s/[\x00-\x1f]//g;
    is($c, "\x20\x30\x40\x50\x60", "s/[\\x00-\\x1f]//g");
}
{
    $_ = "xy";
    no warnings 'uninitialized';
    /(((((((((x)))))))))(z)/;	# clear $10
    s/(((((((((x)))))))))(y)/${10}/;
    is($_,"y","RT#6006: \$_ eq '$_'");
    $_ = "xr";
    s/(((((((((x)))))))))(r)/fooba${10}/;
    is($_,"foobar","RT#6006: \$_ eq '$_'");
}
{
    my $want=("\n" x 11).("B\n" x 11)."B";
    $_="B";
    our $i;
    for $i(1..11){
	s/^.*$/$&/gm;
	$_="\n$_\n$&";
    }
    is($want,$_,"RT#17542");
}

{
    my @tests = ('ABC', "\xA3\xA4\xA5", "\x{410}\x{411}\x{412}");
    foreach (@tests) {
	my $id = ord $_;
	s/./pos/ge;
	is($_, "012", "RT#52104: $id");
    }
}

fresh_perl_is( '$_=q(foo);s/(.)\G//g;print' => 'foo', {},
                '[perl #69056] positive GPOS regex segfault' );
fresh_perl_is( '$_="abcdef"; s/bc|(.)\G(.)/$1 ? "[$1-$2]" : "XX"/ge; print' => 'aXXdef', {},
                'positive GPOS regex substitution failure (#69056, #114884)' );
fresh_perl_is( '$_="abcdefg123456"; s/(?<=...\G)?(\d)/($1)/; print' => 'abcdefg(1)23456', {},
                'positive GPOS lookbehind regex substitution failure #114884' );

# s/..\G//g should stop after the first iteration, rather than working its
# way backwards, or looping infinitely, or SEGVing (for example)
{
    my ($s, $count);

    # use a function to disable constant folding
    my $f = sub { substr("789", 0, $_[0]) };

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/7/g;
    is($count, 1, "..\\G count (short)");
    is($s, "12756", "..\\G s (short)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/78/g;
    is($count, 1, "..\\G count (equal)");
    is($s, "127856", "..\\G s (equal)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/789/g;
    is($count, 1, "..\\G count (long)");
    is($s, "1278956", "..\\G s (long)");


    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/$f->(1)/eg;
    is($count, 1, "..\\G count (short code)");
    is($s, "12756", "..\\G s (short code)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/$f->(2)/eg;
    is($count, 1, "..\\G count (equal code)");
    is($s, "127856", "..\\G s (equal code)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d\G/$f->(3)/eg;
    is($count, 1, "..\\G count (long code)");
    is($s, "1278956", "..\\G s (long code)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/7/g;
    is($count, 1, "..\\G count (lookahead short)");
    is($s, "17456", "..\\G s (lookahead short)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/78/g;
    is($count, 1, "..\\G count (lookahead equal)");
    is($s, "178456", "..\\G s (lookahead equal)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/789/g;
    is($count, 1, "..\\G count (lookahead long)");
    is($s, "1789456", "..\\G s (lookahead long)");


    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/$f->(1)/eg;
    is($count, 1, "..\\G count (lookahead short code)");
    is($s, "17456", "..\\G s (lookahead short code)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/$f->(2)/eg;
    is($count, 1, "..\\G count (lookahead equal code)");
    is($s, "178456", "..\\G s (lookahead equal code)");

    $s = '123456';
    pos($s) = 4;
    $count = $s =~ s/\d\d(?=\d\G)/$f->(3)/eg;
    is($count, 1, "..\\G count (lookahead long code)");
    is($s, "1789456", "..\\G s (lookahead long code)");
}


# [perl #71470] $var =~ s/$qr//e calling get-magic on $_ as well as $var
{
 local *_;
 my $scratch;
 sub qrBug::TIESCALAR { bless[pop], 'qrBug' }
 sub qrBug::FETCH { $scratch .= "[fetching $_[0][0]]"; 'prew' }
 sub qrBug::STORE{}
 tie my $kror, qrBug => '$kror';
 tie $_, qrBug => '$_';
 my $qr = qr/(?:)/;
 $kror =~ s/$qr/""/e;
 is(
   $scratch, '[fetching $kror]',
  'bug: $var =~ s/$qr//e calling get-magic on $_ as well as $var',
 );
}

{ # Bug #41530; replacing non-utf8 with a utf8 causes problems
    my $string = "a\x{a0}a";
    my $sub_string = $string;
    ok(! utf8::is_utf8($sub_string), "Verify that string isn't initially utf8");
    $sub_string =~ s/a/\x{100}/g;
    ok(utf8::is_utf8($sub_string),
                        'Verify replace of non-utf8 with utf8 upgrades to utf8');
    is($sub_string, "\x{100}\x{A0}\x{100}",
                            'Verify #41530 fixed: replace of non-utf8 with utf8');

    my $non_sub_string = $string;
    ok(! utf8::is_utf8($non_sub_string),
                                    "Verify that string isn't initially utf8");
    $non_sub_string =~ s/b/\x{100}/g;
    ok(! utf8::is_utf8($non_sub_string),
            "Verify that failed substitute doesn't change string's utf8ness");
    is($non_sub_string, $string,
                        "Verify that failed substitute doesn't change string");
}

{ # Verify largish octal in replacement pattern

    my $string = "a";
    $string =~ s/a/\400/;
    is($string, chr 0x100, "Verify that handles s/foo/\\400/");
    $string =~ s/./\600/;
    is($string, chr 0x180, "Verify that handles s/foo/\\600/");
    $string =~ s/./\777/;
    is($string, chr 0x1FF, "Verify that handles s/foo/\\777/");
}

# Scoping of s//the RHS/ when there is no /e
# Tests based on [perl #19078]
{
 local *_;
 my $output = ''; my %a;
 no warnings 'uninitialized';

 $_="CCCGGG";
 s!.!<@a{$output .= ("$&"),/[$&]/g}>!g;
 $output .= $_;
 is(
   $output, "CCCGGG<   ><  >< ><   ><  >< >",
  's/// sets PL_curpm for each iteration even when the RHS has set it'
 );
 
 s/C/$a{m\G\}/;
 is(
  "$&", G =>
  'Match vars reflect the last match after s/pat/$a{m|pat|}/ without /e'
 );
}

{
    # a tied scalar that returned a plain string, got messed up
    # when substituted with a UTF8 replacement string, due to
    # magic getting called multiple times, and pointers now pointing
    # to stale/freed strings
    # The original fix for this caused infinite loops for non- or cow-
    # strings, so we test those, too.
    package FOO;
    my $fc;
    sub TIESCALAR { bless [ "abcdefgh" ] }
    sub FETCH { $fc++; $_[0][0] }
    sub STORE { $_[0][0] = $_[1] }

    my $s;
    tie $s, 'FOO';
    $s =~ s/..../\x{101}/;
    ::is($fc, 1, "tied UTF8 stuff FETCH count");
    ::is("$s", "\x{101}efgh", "tied UTF8 stuff");

    ::watchdog(300);
    $fc = 0;
    $s = *foo;
    $s =~ s/..../\x{101}/;
    ::is($fc, 1, '$tied_glob =~ s/non-utf8/utf8/ fetch count');
    ::is("$s", "\x{101}::foo", '$tied_glob =~ s/non-utf8/utf8/ result');
    $fc = 0;
    $s = *foo;
    $s =~ s/(....)/\x{101}/g;
    ::is($fc, 1, '$tied_glob =~ s/(non-utf8)/utf8/g fetch count');
    ::is("$s", "\x{101}\x{101}o",
         '$tied_glob =~ s/(non-utf8)/utf8/g result');
    $fc = 0;
    $s = "\xff\xff\xff\xff\xff";
    $s =~ s/..../\x{101}/;
    ::is($fc, 1, '$tied_latin1 =~ s/non-utf8/utf8/ fetch count');
    ::is("$s", "\x{101}\xff", '$tied_latin1 =~ s/non-utf8/utf8/ result');
    $fc = 0;
    { package package_name; tied($s)->[0] = __PACKAGE__ };
    $s =~ s/..../\x{101}/;
    ::is($fc, 1, '$tied_cow =~ s/non-utf8/utf8/ fetch count');
    ::is("$s", "\x{101}age_name", '$tied_cow =~ s/non-utf8/utf8/ result');
    $fc = 0;
    $s = \1;
    $s =~ s/..../\x{101}/;
    ::is($fc, 1, '$tied_ref =~ s/non-utf8/utf8/ fetch count');
    ::like("$s", qr/^\x{101}AR\(0x.*\)\z/,
           '$tied_ref =~ s/non-utf8/utf8/ result');
}

# RT #97954
{
    my $count;

    sub bam::DESTROY {
	--$count;
    }

    my $z_zapp = bless [], 'bam';
    ++$count;

    is($count, 1, '1 object');
    is($z_zapp =~ s/.*/R/r, 'R', 'substitution happens');
    is(ref $z_zapp, 'bam', 'still 1 object');
    is($count, 1, 'still 1 object');
    undef $z_zapp;
    is($count, 0, 'now 0 objects');

    $z_zapp = bless [], 'bam';
    ++$count;

    is($count, 1, '1 object');
    like($z_zapp =~ s/./R/rg, qr/\AR{8,}\z/, 'substitution happens');
    is(ref $z_zapp, 'bam', 'still 1 object');
    is($count, 1, 'still 1 object');
    undef $z_zapp;
    is($count, 0, 'now 0 objects');
}

is(*bam =~ s/\*//r, 'main::bam', 'Can s///r a tyepglob');
is(*bam =~ s/\*//rg, 'main::bam', 'Can s///rg a tyepglob');

{
 sub cowBug::TIESCALAR { bless[], 'cowBug' }
 sub cowBug::FETCH { __PACKAGE__ }
 sub cowBug::STORE{}
 tie my $kror, cowBug =>;
 $kror =~ s/(?:)/""/e;
}
pass("s/// on tied var returning a cow");

# a test for 6502e08109cd003b2cdf39bc94ef35e52203240b
# previously this would segfault

{
    my $s = "abc";
    eval { $s =~ s/(.)/die/e; };
    like($@, qr/Died at/, "s//die/e");
}


# Test problems with constant replacement optimisation
# [perl #26986] logop in repl resulting in incorrect optimisation
"g" =~ /(.)/;
@l{'a'..'z'} = 'A'..':';
$_ = "hello";
{ s/(.)/$l{my $a||$1}/g }
is $_, "HELLO",
  'logop in s/// repl does not result in "constant" repl optimisation';
# Aliases to match vars
"g" =~ /(.)/;
$_ = "hello";
{
    local *a = *1;
    s/(.)\1/$a/g;
}
is $_, 'helo', 's/pat/$alias_to_match_var/';
"g" =~ /(.)/;
$_ = "hello";
{
    local *a = *1;
    s/e(.)\1/a$a/g;
}
is $_, 'halo', 's/pat/foo$alias_to_match_var/';
# Last-used pattern containing re-evals that modify "constant" rhs
{
    local *a;
    $x = "hello";
    $x =~ /(?{*a = \"a"})./;
    undef *a;
    $x =~ s//$a/g;
    is $x, 'aaaaa',
	'last-used pattern disables constant repl optimisation';
}


$_ = "\xc4\x80";
$a = "";
utf8::upgrade $a;
$_ =~ s/$/$a/;
is $_, "\xc4\x80", "empty utf8 repl does not result in mangled utf8";

$@ = "\x{30cb}eval 18";
$@ =~ s/eval \d+/eval 11/;
is $@, "\x{30cb}eval 11",
  'loading utf8 tables does not interfere with matches against $@';

$reftobe = 3;
$reftobe =~ s/3/$reftobe=\ 3;4/e;
is $reftobe, '4', 'clobbering target with ref in s//.../e';
$locker{key} = 3;
SKIP:{
    skip "no Hash::Util under miniperl", 2 if is_miniperl;
    require Hash::Util;
    eval {
	$locker{key} =~ s/3/
	    $locker{key} = 3;
	    &Hash::Util::lock_hash(\%locker);4
	/e;
    };
    is $locker{key}, '3', 'locking target in $hash{key} =~ s//.../e';
    like $@, qr/^Modification of a read-only value/, 'err msg' . ($@ ? ": $@" : "");
}
delete $::{does_not_exist}; # just in case
eval { no warnings; $::{does_not_exist}=~s/(?:)/*{"does_not_exist"}; 4/e };
like $@, qr/^Modification of a read-only value/,
    'vivifying stash elem in $that::{elem} =~ s//.../e';

# COWs should not be exempt from read-only checks.  s/// croaks on read-
# only values even when the pattern does not match, but it was not doing so
# for COWs.
eval { for (__PACKAGE__) { s/b/c/; } };
like $@, qr/^Modification of a read-only value/,
    'read-only COW =~ s/does not match// should croak';

{
    my $a_acute = chr utf8::unicode_to_native(0xE1); # LATIN SMALL LETTER A WITH ACUTE
    my $egrave = chr utf8::unicode_to_native(0xE8);  # LATIN SMALL LETTER E WITH GRAVE
    my $u_umlaut = chr utf8::unicode_to_native(0xFC);  # LATIN SMALL LETTER U WITH DIAERESIS
    my $division = chr utf8::unicode_to_native(0xF7);  # DIVISION SIGN

    is("ab.c" =~ s/\b/!/agr, "!ab!.!c!", '\\b matches ASCII before string, mid, and end, /a');
    is("$a_acute$egrave.$u_umlaut" =~ s/\b/!/agr, "$a_acute$egrave.$u_umlaut", '\\b matches Latin1 before string, mid, and end, /a');
    is("\x{100}\x{101}.\x{102}" =~ s/\b/!/agr, "\x{100}\x{101}.\x{102}", '\\b matches above-Latin1 before string, mid, and end, /a');

    is("..." =~ s/\B/!/agr, "!.!.!.!", '\\B matches ASCII before string, mid, and end, /a');
    is("$division$division$division" =~ s/\B/!/agr, "!$division!$division!$division!", '\\B matches Latin1 before string, mid, and end, /a');
    is("\x{2028}\x{2028}\x{2028}" =~ s/\B/!/agr, "!\x{2028}!\x{2028}!\x{2028}!", '\\B matches above-Latin1 before string, mid, and end, /a');

    is("ab.c" =~ s/\b/!/dgr, "!ab!.!c!", '\\b matches ASCII before string, mid, and end, /d');
    { is("$a_acute$egrave.$u_umlaut" =~ s/\b/!/dgr, "$a_acute$egrave.$u_umlaut", '\\b matches Latin1 before string, mid, and end, /d'); }
    is("\x{100}\x{101}.\x{102}" =~ s/\b/!/dgr, "!\x{100}\x{101}!.!\x{102}!", '\\b matches above-Latin1 before string, mid, and end, /d');

    is("..." =~ s/\B/!/dgr, "!.!.!.!", '\\B matches ASCII before string, mid, and end, /d');
    is("$division$division$division" =~ s/\B/!/dgr, "!$division!$division!$division!", '\\B matches Latin1 before string, mid, and end, /d');
    is("\x{2028}\x{2028}\x{2028}" =~ s/\B/!/dgr, "!\x{2028}!\x{2028}!\x{2028}!", '\\B matches above-Latin1 before string, mid, and end, /d');

    is("ab.c" =~ s/\b/!/ugr, "!ab!.!c!", '\\b matches ASCII before string, mid, and end, /u');
    is("$a_acute$egrave.$u_umlaut" =~ s/\b/!/ugr, "!$a_acute$egrave!.!$u_umlaut!", '\\b matches Latin1 before string, mid, and end, /u');
    is("\x{100}\x{101}.\x{102}" =~ s/\b/!/ugr, "!\x{100}\x{101}!.!\x{102}!", '\\b matches above-Latin1 before string, mid, and end, /u');

    is("..." =~ s/\B/!/ugr, "!.!.!.!", '\\B matches ASCII before string, mid, and end, /u');
    is("$division$division$division" =~ s/\B/!/ugr, "!$division!$division!$division!", '\\B matches Latin1 before string, mid, and end, /u');
    is("\x{2028}\x{2028}\x{2028}" =~ s/\B/!/ugr, "!\x{2028}!\x{2028}!\x{2028}!", '\\B matches above-Latin1 before string, mid, and end, /u');

    fresh_perl_like( '$_=""; /\b{gcb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{gcb}'
    );
    fresh_perl_like( '$_=""; /\B{gcb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{gcb}'
    );
    fresh_perl_like( '$_=""; /\b{wb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{wb}'
    );
    fresh_perl_like( '$_=""; /\B{wb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{wb}'
    );
    fresh_perl_like( '$_=""; /\b{sb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{sb}'
    );
    fresh_perl_like( '$_=""; /\B{sb}/;  s///g', qr/^$/, {},
        '[perl #126319: Segmentation fault in Perl_sv_catpvn_flags with \b{sb}'
    );

    SKIP: {
        if (! locales_enabled('LC_ALL')) {
            skip "Can't test locale (maybe you are missing POSIX)", 6;
        }

        setlocale(&POSIX::LC_ALL, "C");
        use locale;
        is("a.b" =~ s/\b/!/gr, "!a!.!b!", '\\b matches ASCII before string, mid, and end, /l');
        is("$a_acute.$egrave" =~ s/\b/!/gr, "$a_acute.$egrave", '\\b matches Latin1 before string, mid, and end, /l');
        is("\x{100}\x{101}.\x{102}" =~ s/\b/!/gr, "!\x{100}\x{101}!.!\x{102}!", '\\b matches above-Latin1 before string, mid, and end, /l');

        is("..." =~ s/\B/!/gr, "!.!.!.!", '\\B matches ASCII before string, mid, and end, /l');
        is("$division$division$division" =~ s/\B/!/gr, "!$division!$division!$division!", '\\B matches Latin1 before string, mid, and end, /l');
        is("\x{2028}\x{2028}\x{2028}" =~ s/\B/!/gr, "!\x{2028}!\x{2028}!\x{2028}!", '\\B matches above-Latin1 before string, mid, and end, /l');
    }

}

{
    # RT #123954 if the string getting matched against got converted during
    # s///e so that it was no longer SvPOK, an assertion would fail when
    # setting pos.
    my $s1 = 0;
    $s1 =~ s/.?/$s1++/ge;
    is($s1, "01","RT #123954 s1");
}
{
    # RT #126602 double free if the value being modified is freed in the replacement
    fresh_perl_is('s//*_=0;s|0||;00.y0/e; print qq(ok\n)', "ok\n", { stderr => 1 },
                  "[perl #126602] s//*_=0;s|0||/e crashes");
}

{
    #RT 126260 gofs is in chars, not bytes

    # in something like /..\G/, the engine should start matching two
    # chars before pos(). At one point it was matching two bytes before.

    my $s = "\x{121}\x{122}\x{123}";
    pos($s) = 2;
    $s =~ s/..\G//g;
    is($s, "\x{123}", "#RT 126260 gofs");
}

SKIP: {
    if (! locales_enabled('LC_CTYPE')) {
        skip "Can't test locale", 1;
    }

    #  To cause breakeage, we need a locale in which \xff matches whatever
    #  POSIX class is used in the pattern.  Easiest is C, with \W.
    fresh_perl_is('    use POSIX qw(locale_h);
                       setlocale(&POSIX::LC_CTYPE, "C");
                       my $s = "\xff";
                       $s =~ s/\W//l;
                       print qq(ok$s\n)',
                   "ok\n",
                   {stderr => 1 },
                   '[perl #129038 ] s/\xff//l no longer crashes');
}

 SKIP: {
    skip("no Tie::Hash::NamedCapture under miniperl", 3) if is_miniperl;

    # RT #23624 scoping of @+/@- when used with tie()
    #! /usr/bin/perl -w

    package Tie::Prematch;
    sub TIEHASH { bless \my $dummy => __PACKAGE__ }
    sub FETCH   { return substr $_[1], 0, $-[0] }

    package main;

    eval <<'__EOF__';
    tie my %pre, 'Tie::Prematch';
    my $foo = 'foobar';
    $foo =~ s/.ob/$pre{ $foo }/;
    is($foo, 'ffar', 'RT #23624');

    $foo = 'foobar';
    $foo =~ s/.ob/tied(%pre)->FETCH($foo)/e;
    is($foo, 'ffar', 'RT #23624');

    tie %-, 'Tie::Prematch';
    $foo = 'foobar';
    $foo =~ s/.ob/$-{$foo}/;
    is($foo, 'ffar', 'RT #23624');

    undef *Tie::Prematch::TIEHASH;
    undef *Tie::Prematch::FETCH;
__EOF__
}

# [perl #130188] crash on return from substitution in subroutine
# make sure returning from s///e doesn't SEGV
{
    my $f = sub {
        my $x = 'a';
        $x =~ s/./return;/e;
    };
    my $x = $f->();
    pass("RT #130188");
}

# RT #131930
# a multi-line s/// wasn't resetting the cop_line correctly
{
    my $l0 = __LINE__;
    my $s = "a";
    $s =~ s[a]
           [b];
    my $lines = __LINE__ - $l0;
    is $lines, 4, "RT #131930";
}

{   # [perl #133899], would panic

    fresh_perl_is('my $a = "ha"; $a =~ s!|0?h\x{300}(?{})!!gi', "", {},
                  "[perl #133899] s!|0?h\\x{300}(?{})!!gi panics");
}

{
    fresh_perl_is("s//00000000000format            \0          '0000000\\x{800}/;eval", "", {}, "RT #133882");
}

{   # GH Issue 20690
    my @ret;
    my $str = "abc";
    for my $upgrade (0,1) {
        my $copy = $str;
        utf8::upgrade($copy) if $upgrade;
        my $r= $copy=~s/b{0}//gr;
        push @ret, $r;
    }
    is( $ret[1], $ret[0], 
        "Issue #20690 - s/b{0}//gr should work the same for utf8 and non-utf8 strings");
    is( $ret[0], $str,
        "Issue #20690 - s/b{0}//gr on non-utf8 string should not remove anything");
    is( $ret[1], $str,
        "Issue #20690 - s/b{0}//gr on utf8 string should not remove anything");
}
