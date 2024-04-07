#!./perl

print "1..120\n";

$x = 'x';

print "#1	:$x: eq :x:\n";
if ($x eq 'x') {print "ok 1\n";} else {print "not ok 1\n";}

$x = $#[0];

if ($x eq '') {print "ok 2\n";} else {print "not ok 2\n";}

$x = $#x;

if ($x eq '-1') {print "ok 3\n";} else {print "not ok 3\n";}

$x = '\\'; # ';

if (length($x) == 1) {print "ok 4\n";} else {print "not ok 4\n";}

eval 'while (0) {
    print "foo\n";
}
/^/ && (print "ok 5\n");
';

eval '$foo{1} / 1;';
if (!$@) {print "ok 6\n";} else {print "not ok 6 $@\n";}

eval '$foo = 123+123.4+123e4+123.4E5+123.4e+5+.12;';

$foo = int($foo * 100 + .5);
if ($foo eq 2591024652) {print "ok 7\n";} else {print "not ok 7 :$foo:\n";}

print <<'EOF';
ok 8
EOF

$foo = 'ok 9';
print <<EOF;
$foo
EOF

eval <<\EOE, print $@;
print <<'EOF';
ok 10
EOF

$foo = 'ok 11';
print <<EOF;
$foo
EOF
EOE

print <<'EOS' . <<\EOF;
ok 12 - make sure single quotes are honored \nnot ok
EOS
ok 13
EOF

print qq/ok 14\n/;
print qq(ok 15\n);

print qq
[ok 16\n]
;

print q<ok 17
>;

print "ok 18 - was the test for the deprecated use of bare << to mean <<\"\"\n";
#print <<;   # Yow!
#ok 18
#
## previous line intentionally left blank.

print <<E1 eq "foo\n\n" ? "ok 19\n" : "not ok 19\n";
@{[ <<E2 ]}
foo
E2
E1

print <<E1 eq "foo\n\n" ? "ok 20\n" : "not ok 20\n";
@{[
  <<E2
foo
E2
]}
E1

$foo = FOO;
$bar = BAR;
$foo{$bar} = BAZ;
$ary[0] = ABC;

print "$foo{$bar}" eq "BAZ" ? "ok 21\n" : "not ok 21\n";

print "${foo}{$bar}" eq "FOO{BAR}" ? "ok 22\n" : "not ok 22\n";
print "${foo{$bar}}" eq "BAZ" ? "ok 23\n" : "not ok 23\n";

print "FOO:" =~ /$foo[:]/ ? "ok 24\n" : "not ok 24\n";
print "ABC" =~ /^$ary[$A]$/ ? "ok 25\n" : "not ok 25\n";
print "FOOZ" =~ /^$foo[$A-Z]$/ ? "ok 26\n" : "not ok 26\n";

# MJD 19980425
($X, @X) = qw(a b c d); 
print "d" =~ /^$X[-1]$/ ? "ok 27\n" : "not ok 27\n";
print "a1" !~ /^$X[-1]$/ ? "ok 28\n" : "not ok 28\n";

print (((q{{\{\(}} . q{{\)\}}}) eq '{{\(}{\)}}') ? "ok 29\n" : "not ok 29\n");


$foo = "not ok 30\n";
$foo =~ s/^not /substr(<<EOF, 0, 0)/e;
  Ignored
EOF
print $foo;

# Tests for new extended control-character variables
# MJD 19990227

my $test = 31;

{ my $CX = "\cX";
  my $CXY  ="\cXY";
  $ {$CX} = 17;
  $ {$CXY} = 23;
  if ($ {^XY} != 23) { print "not "  }
  print "ok $test\n"; $test++;
 
# Does the old UNBRACED syntax still do what it used to?
  if ("$^XY" ne "17Y") { print "not " }
  print "ok $test\n"; $test++;

  sub XX () { 6 }
  $ {"\cQ\cXX"} = 119; 
  $^Q = 5; #  This should be an unused ^Var.
  $N = 5;
  # The second caret here should be interpreted as an xor
  if (($^Q^XX) != 3) { print "not " } 
  print "ok $test\n"; $test++;

  # These next two tests are trying to make sure that
  # $^FOO is always global; it doesn't make sense to 'my' it.
  # 

  eval 'my $^X;';
  print "not " unless index ($@, 'Can\'t use global $^X in "my"') > -1;
  print "ok $test\n"; $test++;
#  print "($@)\n" if $@;

  eval 'my $ {^XYZ};';
  print "not " unless index ($@, 'Can\'t use global $^XYZ in "my"') > -1;
  print "ok $test\n"; $test++;
#  print "($@)\n" if $@;
#
  ${^TEST}= "splat";
  @{^TEST}= ("foo", "bar");
  %{^TEST}= ("foo" => "FOO", "bar" => "BAR" );
  
  print "not " if "${^TEST}" ne "splat";
  print "ok $test\n"; $test++;
  
  print "not " if "${ ^TEST }" ne "splat";
  print "ok $test\n"; $test++;

  print "not " if "${^TEST}[0]" ne "splat[0]";
  print "ok $test\n"; $test++;

  print "not " if "${^TEST[0]}" ne "foo";
  print "ok $test\n"; $test++;

  print "not " if "${ ^TEST [1] }" ne "bar";
  print "ok $test\n"; $test++;

  print "not " if "${^TEST}{foo}" ne "splat{foo}";
  print "ok $test\n"; $test++;

  print "not " if "${^TEST{foo}}" ne "FOO";
  print "ok $test\n"; $test++;

  print "not " if "${ ^TEST {bar} }" ne "BAR";
  print "ok $test\n"; $test++;


# Now let's make sure that caret variables are all forced into the main package.
  package Someother;
  $^Q = 'Someother';
  $ {^Quixote} = 'Someother 2';
  $ {^M} = 'Someother 3';
  package main;
  print "not " unless $^Q eq 'Someother';
  print "ok $test\n"; $test++;
  print "not " unless $ {^Quixote} eq 'Someother 2';
  print "ok $test\n"; $test++;
  print "not " unless $ {^M} eq 'Someother 3';
  print "ok $test\n"; $test++;

  
}

# see if eval '', s///e, and heredocs mix

sub T {
    my ($where, $num) = @_;
    my ($p,$f,$l) = caller;
    print "# $p:$f:$l vs /$where/\nnot " unless "$p:$f:$l" =~ /$where/;
    print "ok $num\n";
}

{
# line 42 "plink"
    local $_ = "not ok ";
    eval q{
	s/^not /<<EOT/e and T '^main:\(eval \d+\):2$', $test++;
# uggedaboudit
EOT
        print $_, $test++, "\n";
	T('^main:\(eval \d+\):6$', $test++);
# line 1 "plunk"
	T('^main:plunk:1$', $test++);
    };
    print "# $@\nnot ok $test\n" if $@;
    T '^main:plink:53$', $test++;
}

# tests 47--51 start here
# tests for new array interpolation semantics:
# arrays now *always* interpolate into "..." strings.
# 20000522 MJD (mjd@plover.com)
{
  eval(q(">@nosuch<" eq "><")) || print "# $@", "not ";
  print "ok $test\n";
  ++$test;

  # Look at this!  This is going to be a common error in the future:
  eval(q("fred@example.com" eq "fred.com")) || print "# $@", "not ";
  print "ok $test\n";
  ++$test;

  # Let's make sure that normal array interpolation still works right
  # For some reason, this appears not to be tested anywhere else.
  my @a = (1,2,3);
  print +((">@a<" eq ">1 2 3<") ? '' : 'not '), "ok $test\n";
  ++$test;

  # Ditto.
  eval(q{@nosuch = ('a', 'b', 'c'); ">@nosuch<" eq ">a b c<"}) 
      || print "# $@", "not ";
  print "ok $test\n";
  ++$test;

  # This isn't actually a lex test, but it's testing the same feature
  sub makearray {
    my @array = ('fish', 'dog', 'carrot');
    *R::crackers = \@array;
  }

  eval(q{makearray(); ">@R::crackers<" eq ">fish dog carrot<"})
    || print "# $@", "not ";
  print "ok $test\n";
  ++$test;
}

# Tests 52-54
# => should only quote foo::bar if it isn't a real sub. AMS, 20010621

sub xyz::foo { "bar" }
my %str = (
    foo      => 1,
    xyz::foo => 1,
    xyz::bar => 1,
);

print ((exists $str{foo}      ? "" : "not ")."ok $test\n"); ++$test;
print ((exists $str{bar}      ? "" : "not ")."ok $test\n"); ++$test;
print ((exists $str{xyz::bar} ? "" : "not ")."ok $test\n"); ++$test;

sub foo::::::bar { print "ok $test\n"; $test++ }
foo::::::bar;

# \xDF is a non-ASCII alpha on both ASCII and EBCDIC.
eval "\$x =\xDFfoo";
if ($@ =~ /Unrecognized character \\xDF; marked by <-- HERE after \$x =<-- HERE near column 5/) { print "ok $test\n"; } else { print "not ok $test\n"; }
$test++;

# Is "[~" scanned correctly?
@a = (1,2,3);
print "not " unless($a[~~2] == 3);
print "ok $test\n"; $test++;

$_ = "";
eval 's/(?:)/"ok $test" . "${\q||}".<<\END/e;
 - heredoc after "" in s/// in eval
END
';
print $_ || "not ok $test\n"; $test++;

$_ = "";
eval 's|(?:)|"ok $test" . "${\<<\END}"
 - heredoc in "" in multiline s///e in eval
END
|e
';
print $_ || "not ok $test\n"; $test++;

$_ = "";
eval "s/(?:)/<<foo/e #\0
ok $test - null on same line as heredoc in s/// in eval
foo
";
print $_ || "not ok $test\n"; $test++;

$_ = "";
eval ' s/(?:)/"${\<<END}"/e;
ok $test - heredoc in "" in single-line s///e in eval
END
';
print $_ || "not ok $test\n"; $test++;

$_ = "";
s|(?:)|"${\<<END}"
ok $test - heredoc in "" in multiline s///e outside eval
END
|e;
print $_ || "not ok $test\n"; $test++;

$_ = "not ok $test - s/// in s/// pattern\n";
s/${s|||;\""}not //;
print; $test++;

/(?{print <<END
ok $test - here-doc in re-eval
END
})/; $test++;

eval '/(?{print <<END
ok $test - here-doc in re-eval in string eval
END
})/'; $test++;

eval 'print qq ;ok $test - eval ending with semicolon\n;'
  or print "not ok $test - eval ending with semicolon\n"; $test++;

print "not " unless qr/(?{<<END})/ eq '(?^:(?{<<END}))';
foo
END
print "ok $test - here-doc in single-line re-eval\n"; $test++;

$_ = qr/(?{"${<<END}"
foo
END
})/;
print "not " unless /foo/;
print "ok $test - here-doc in quotes in multiline re-eval\n"; $test++;

eval 's//<<END/e if 0; $_ = "a
END
b"';
print "not " if $_ =~ /\n\n/;
print "ok $test - eval 's//<<END/' does not leave extra newlines\n"; $test++;

$_ = a;
eval "s/a/'b\0'#/e";
print 'not ' unless $_ eq "b\0";
print "ok $test - # after null in s/// repl\n"; $test++;

s//"#" . <<END/e;
foo
END
print "ok $test - s//'#' . <<END/e\n"; $test++;

eval "s//3}->{3/e";
print "not " unless $@;
print "ok $test - s//3}->{3/e\n"; $test++;

$_ = "not ok $test";
$x{3} = "not ";
eval 's/${\%x}{3}//e';
print "$_ - s//\${\\%x}{3}/e\n"; $test++;

eval 's/${foo#}//e';
print "not " unless $@;
print "ok $test - s/\${foo#}//e\n"; $test++;

eval 'warn ({$_ => 1} + 1) if 0';
print "not " if $@;
print "ok $test - listop({$_ => 1} + 1)\n"; $test++;
print "# $@" if $@;

for(qw< require goto last next redo CORE::dump >) {
    eval "sub { $_ foo << 2 }";
    print "not " if $@;
    print "ok ", $test++, " - [perl #105924] $_ WORD << ...\n";
    print "# $@" if $@;
}

# https://github.com/Perl/perl5/issues/9415
my $counter = 0;
eval 'v23: $counter++; goto v23 unless $counter == 2';
print "not " unless $counter == 2;
print "ok $test - Use v[0-9]+ as a label\n"; $test++;
$counter = 0;
eval 'v23 : $counter++; goto v23 unless $counter == 2';
print "not " unless $counter == 2;
print "ok $test - Use v[0-9]+ as a label with space before colon\n"; $test++;
 
my $output = "";
eval "package v10::foo; sub test2 { return 'v10::foo' }
      package v10; sub test { return v10::foo::test2(); }
      package main; \$output = v10::test(); "; 
print "not " unless $output eq 'v10::foo';
print "ok $test - call a function in package v10::foo\n"; $test++;

print "not " unless (1?v65:"bar") eq chr(65);
print "ok $test - colon detection after vstring does not break ? vstring :\n"; $test++;

print ((ord("A") == 65) ? v35 : v123);  # NUMBER SIGN is the same for all
                                        # supported EBCDIC platforms
print "not ";
print ((ord("A") == 65) ? v10 : "\n");  # LF varies on EBCDIC, if the v123 for
                                        # '#' works above, consider it good
                                        # enough.
    print "ok $test - print vstring prints the vstring\n";
$test++;

# Test pyoq ops with comments before the first delim
q # comment
 "b"#
  eq 'b' or print "not ";
print "ok $test - q <comment> <newline> ...\n"; $test++;
qq # comment
 "b"#
  eq 'b' or print "not ";
print "ok $test - qq <comment> <newline> ...\n"; $test++;
qw # comment
 "b"#
  [0] eq 'b' or print "not ";
print "ok $test - qw <comment> <newline> ...\n"; $test++;
"b" =~ m # comment
 "b"#
   or print "not ";
print "ok $test - m <comment> <newline> ...\n"; $test++;
qr # comment
 "b"#
   eq qr/b/ or print "not ";
print "ok $test - qr <comment> <newline> ...\n"; $test++;
$_ = "a";
s # comment
 [a] #
 [b] #
 ;
print "not " unless $_ eq 'b';
print "ok $test - s <comment> <newline> ...\n"; $test++;
$_ = "a";
tr # comment
 [a] #
 [b] #
 ;
print "not " unless $_ eq 'b';
print "ok $test - tr <comment> <newline> ...\n"; $test++;
$_ = "a";
y # comment
 [a] #
 [b] #
 ;
print "not " unless $_ eq 'b';
print "ok $test - y <comment> <newline> ...\n"; $test++;

print "not " unless (time
                     =>) eq time=>;
print "ok $test - => quotes keywords across lines\n"; $test++;

# [perl #80368]
print "not " unless eval '"a\U="' eq "a=";
print "ok $test - [perl #80368] qq <a\\U=>\n"; $test++;

sub Function_with_side_effects { $_ = "sidekick function called" }
print "not " unless
    (eval '${Function_with_side_effects,\$_}' || $@)
      eq "sidekick function called";
print "ok $test - \${...} where {...} looks like hash\n"; $test++;

@_ = map{BEGIN {$_122782 = 'tst2'}; "rhu$_"} 'barb2';
print "not " unless "@_" eq 'rhubarb2';
print "ok $test - map{BEGIN...\n"; $test++;
print "not " unless $_122782 eq 'tst2';
print "ok $test - map{BEGIN...\n"; $test++;
${
=pod
blah blah blah
=cut
\$_ } = 42;
print "not "unless $_ == 42;
print "ok $test - \${ <newline> =pod\n"; $test++;
@_ = map{
=pod
blah blah blah
=cut
$_+1 } 1;
print "not "unless "@_" eq 2;
print "ok $test - map{ <newline> =pod\n"; $test++;
eval { ${...}++ };
print "not " unless $@ =~ /^Unimplemented at /;
print "ok $test - \${...} (literal triple-dot)\n"; $test++;
eval { () = map{...} @_ };
print "not " unless $@ =~ /^Unimplemented at /;
print "ok $test - map{...} (literal triple-dot)\n"; $test++;
print "not " unless &{sub :lvalue { "a" }} eq "a";
print "ok $test - &{sub :lvalue...}\n"; $test++;
print "not " unless ref +(map{sub :lvalue { "a" }} 1)[0] eq "CODE";
print "ok $test - map{sub :lvalue...}\n"; $test++;

# Used to crash [perl #123711]
0-5x-l{0};

# Used to fail an assertion [perl #123617] [perl #123955]
eval '"$a{ 1 m// }"; //';
eval '"@0{0s 000";eval"$"';

# Pending token stack overflow [perl #123677]
{
 local $SIG{__WARN__}=sub{};
 eval q|s)$0{0h());qx(@0);qx(@0);qx(@0)|;
}

# Used to crash [perl #123801]
eval q|s##[}#e|;

# Used to fail an assertion [perl #123763]
{
 local $SIG{__WARN__}=sub{};
 eval q|my($_);0=split|;
 eval q|my $_; @x = split|;
}

{
 # Used to crash [perl #124187]
 eval q|qq{@{[{}}*sub{]]}}}=u|;
}

{
 # Used to crash [perl #124385]
 eval '0; qq{@{sub{]]}}}}}';
 print "ok $test - 124385\n"; $test++;
}

{
 # Used to crash [perl #125350]
 eval ('qq{@{[0}*sub{]]}}}=sub{0' . "\c[");
 print "ok $test - 125350\n"; $test++;
}

{
 # Used to crash [perl #128171]
 eval ('/@0{0*->@*/*]');
 print "ok $test - 128171\n"; $test++;
}
{
  # various sub-parse recovery issues that crashed perl
  eval 's//${sub{b{]]]{}#$/ sub{}';
  print "ok $test - 132640\n"; $test++;
  eval 'qq{@{sub{]]}}}};shift';
  print "ok $test - 125351\n"; $test++;
  eval 'qq{@{sub{]]}}}}-shift';
  print "ok $test - 126192\n"; $test++;
}

$foo = "WRONG"; $foo:: = "bar"; $bar = "baz";
print "not " unless "$foo::$bar" eq "barbaz";
print qq|ok $test - [perl #128478] "\$foo::\$bar"\n|; $test++;
@bar = ("baz","bonk");
print "not " unless "$foo::@bar" eq "barbaz bonk";
print qq|ok $test - [perl #128478] "\$foo::\@bar"\n|; $test ++;

# Test that compilation of tentative indirect method call syntax which
# turns out not to be such does not upgrade constants to full globs in the
# symbol table.
sub fop() { 0 }
sub bas() { 0 }
{ local $SIG{__WARN__}=sub{}; eval 'fop bas'; }
print "not " unless ref $::{fop} eq 'SCALAR';
print "ok $test - first constant in 'const1 const2' is not upgraded\n";
$test++;
print "not " unless ref $::{bas} eq 'SCALAR';
print "ok $test - second constant in 'const1 const2' is not upgraded\n";
$test++;
