#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib' );
}
use strict;
no warnings 'void';
use Errno qw(ENOENT EISDIR);

my $called;
my $result = do{ ++$called; 'value';};
is($called, 1, 'do block called');
is($result, 'value', 'do block returns correct value');

unshift @INC, '.';

my $file16 = tempfile();
if (open my $do, '>', $file16) {
    print $do "isnt(wantarray, undef, 'do in scalar context');\n";
    print $do "cmp_ok(wantarray, '==', 0, 'do in scalar context');\n";
    close $do or die "Could not close: $!";
}

my $a = do $file16; die $@ if $@;

my $file17 = tempfile();
if (open my $do, '>', $file17) {
    print $do "isnt(wantarray, undef, 'do in list context');\n";
    print $do "cmp_ok(wantarray, '!=', 0, 'do in list context');\n";
    close $do or die "Could not close: $!";
}

my @a = do $file17; die $@ if $@;

my $file18 = tempfile();
if (open my $do, '>', $file18) {
    print $do "is(wantarray, undef, 'do in void context');\n";
    close $do or die "Could not close: $!";
}

do $file18; die $@ if $@;

# bug ID 20010920.007 (#7713)
eval qq{ do qq(a file that does not exist); };
is($@, '', "do on a non-existing file, first try");

eval qq{ do uc qq(a file that does not exist); };
is($@, '', "do on a non-existing file, second try");

# 6 must be interpreted as a file name here
$! = 0;
my $do6 = do 6;
my $errno = $1;
is($do6, undef, 'do 6 must be interpreted as a filename');
isnt($!, 0, 'and should set $!');

# [perl #19545]
my ($u, @t);
{
    no warnings 'uninitialized';
    push @t, ($u = (do {} . "This should be pushed."));
}
is($#t, 0, "empty do result value" );

my $zok = '';
my $owww = do { 1 if $zok };
is($owww, '', 'last is unless');
$owww = do { 2 unless not $zok };
is($owww, 1, 'last is if not');

$zok = 'swish';
$owww = do { 3 unless $zok };
is($owww, 'swish', 'last is unless');
$owww = do { 4 if not $zok };
is($owww, '', 'last is if not');

# [perl #38809]
@a = (7);
my $x = sub { do { return do { @a } }; 2 }->();
is($x, 1, 'return do { } receives caller scalar context');
my @x = sub { do { return do { @a } }; 2 }->();
is("@x", "7", 'return do { } receives caller list context');

@a = (7, 8);
$x = sub { do { return do { 1; @a } }; 3 }->();
is($x, 2, 'return do { ; } receives caller scalar context');
@x = sub { do { return do { 1; @a } }; 3 }->();
is("@x", "7 8", 'return do { ; } receives caller list context');

my @b = (11 .. 15);
$x = sub { do { return do { 1; @a, @b } }; 3 }->();
is($x, 5, 'return do { ; , } receives caller scalar context');
@x = sub { do { return do { 1; @a, @b } }; 3 }->();
is("@x", "7 8 11 12 13 14 15", 'return do { ; , } receives caller list context');

$x = sub { do { return do { 1; @a }, do { 2; @b } }; 3 }->();
is($x, 5, 'return do { ; }, do { ; } receives caller scalar context');
@x = sub { do { return do { 1; @a }, do { 2; @b } }; 3 }->();
is("@x", "7 8 11 12 13 14 15", 'return do { ; }, do { ; } receives caller list context');

@a = (7, 8, 9);
$x = sub { do { do { 1; return @a } }; 4 }->();
is($x, 3, 'do { return } receives caller scalar context');
@x = sub { do { do { 1; return @a } }; 4 }->();
is("@x", "7 8 9", 'do { return } receives caller list context');

@a = (7, 8, 9, 10);
$x = sub { do { return do { 1; do { 2; @a } } }; 5 }->();
is($x, 4, 'return do { do { ; } } receives caller scalar context');
@x = sub { do { return do { 1; do { 2; @a } } }; 5 }->();
is("@x", "7 8 9 10", 'return do { do { ; } } receives caller list context');

# More tests about context propagation below return()
@a = (11, 12);
@b = (21, 22, 23);

my $test_code = sub {
    my ($x, $y) = @_;
    if ($x) {
	return $y ? do { my $z; @a } : do { my $z; @b };
    } else {
	return (
	    do { my $z; @a },
	    (do { my$z; @b }) x $y
	);
    }
    'xxx';
};

$x = $test_code->(1, 1);
is($x, 2, 'return $y ? do { } : do { } - scalar context 1');
$x = $test_code->(1, 0);
is($x, 3, 'return $y ? do { } : do { } - scalar context 2');
@x = $test_code->(1, 1);
is("@x", '11 12', 'return $y ? do { } : do { } - list context 1');
@x = $test_code->(1, 0);
is("@x", '21 22 23', 'return $y ? do { } : do { } - list context 2');

$x = $test_code->(0, 0);
is($x, "", 'return (do { }, (do { }) x ...) - scalar context 1');
$x = $test_code->(0, 1);
is($x, 3, 'return (do { }, (do { }) x ...) - scalar context 2');
@x = $test_code->(0, 0);
is("@x", '11 12', 'return (do { }, (do { }) x ...) - list context 1');
@x = $test_code->(0, 1);
is("@x", '11 12 21 22 23', 'return (do { }, (do { }) x ...) - list context 2');

$test_code = sub {
    my ($x, $y) = @_;
    if ($x) {
	return do {
	    if ($y == 0) {
		my $z;
		@a;
	    } elsif ($y == 1) {
		my $z;
		@b;
	    } else {
		my $z;
		(wantarray ? reverse(@a) : '99');
	    }
	};
    }
    'xxx';
};

$x = $test_code->(1, 0);
is($x, 2, 'return do { if () { } elsif () { } else { } } - scalar 1');
$x = $test_code->(1, 1);
is($x, 3, 'return do { if () { } elsif () { } else { } } - scalar 2');
$x = $test_code->(1, 2);
is($x, 99, 'return do { if () { } elsif () { } else { } } - scalar 3');
@x = $test_code->(1, 0);
is("@x", '11 12', 'return do { if () { } elsif () { } else { } } - list 1');
@x = $test_code->(1, 1);
is("@x", '21 22 23', 'return do { if () { } elsif () { } else { } } - list 2');
@x = $test_code->(1, 2);
is("@x", '12 11', 'return do { if () { } elsif () { } else { } } - list 3');

# Do blocks created by constant folding
# [perl #68108]
$x = sub { if (1) { 20 } }->();
is($x, 20, 'if (1) { $x } receives caller scalar context');

@a = (21 .. 23);
$x = sub { if (1) { @a } }->();
is($x, 3, 'if (1) { @a } receives caller scalar context');
@x = sub { if (1) { @a } }->();
is("@x", "21 22 23", 'if (1) { @a } receives caller list context');

$x = sub { if (1) { 0; 20 } }->();
is($x, 20, 'if (1) { ...; $x } receives caller scalar context');

@a = (24 .. 27);
$x = sub { if (1) { 0; @a } }->();
is($x, 4, 'if (1) { ...; @a } receives caller scalar context');
@x = sub { if (1) { 0; @a } }->();
is("@x", "24 25 26 27", 'if (1) { ...; @a } receives caller list context');

$x = sub { if (1) { 0; 20 } else{} }->();
is($x, 20, 'if (1) { ...; $x } else{} receives caller scalar context');

@a = (24 .. 27);
$x = sub { if (1) { 0; @a } else{} }->();
is($x, 4, 'if (1) { ...; @a } else{} receives caller scalar context');
@x = sub { if (1) { 0; @a } else{} }->();
is("@x", "24 25 26 27", 'if (1) { ...; @a } else{} receives caller list context');

$x = sub { if (0){} else { 0; 20 } }->();
is($x, 20, 'if (0){} else { ...; $x } receives caller scalar context');

@a = (24 .. 27);
$x = sub { if (0){} else { 0; @a } }->();
is($x, 4, 'if (0){} else { ...; @a } receives caller scalar context');
@x = sub { if (0){} else { 0; @a } }->();
is("@x", "24 25 26 27", 'if (0){} else { ...; @a } receives caller list context');

# [rt.cpan.org #72767] do "string" should not propagate warning hints
SKIP: {
  skip_if_miniperl("no in-memory files under miniperl", 1);

  my $code = '42; 1';
  # Based on Eval::WithLexicals::_eval_do
  local @INC = (sub {
    if ($_[1] eq '/eval_do') {
      open my $fh, '<', \$code;
      $fh;
    } else {
      ();
    }
  }, @INC);
  local $^W;
  use warnings;
  my $w;
  local $SIG{__WARN__} = sub { warn shift; ++$w };
  do '/eval_do' or die $@;
  is($w, undef, 'do STRING does not propagate warning hints');
}

# RT#113730 - $@ should be cleared on IO error.
{
    $@ = "should not see";
    $! = 0;
    my $rv = do("some nonexistent file");
    my $saved_error = $@;
    my $saved_errno = $!;
    ok(!$rv,          "do returns false on io errror");
    ok(!$saved_error, "\$\@ not set on io error");
    ok($saved_errno == ENOENT, "\$! is ENOENT for nonexistent file");
}

# do subname should not be do "subname"
{
    my $called;
    sub fungi { $called .= "fungible" }
    $@ = "scrimptious scrobblings";
    do fungi;
    is $called, "fungible", "do-file does not force bareword";
    isnt $@, "scrimptious scrobblings", "It was interpreted as do-file";
}

# do CORE () has always been do-file
{
    my $called;
    sub CORE { $called .= "fungible" }
    $@ = "scromptious scrimblings";
    do CORE();
    is $called, "fungible", "do CORE() calls &CORE";
    isnt $@, "scromptious scrimblings", "It was interpreted as do-file";
}

# do subname() and $subname() are no longer allowed
{
    sub subname { fail('do subname('. ($_[0] || '') .') called') };
    my $subref = sub { fail('do $subref('. ($_[0] || '') .') called') };
    foreach my $mode (qw(subname("arg") subname() $subref("arg") $subref())) {
        eval "do $mode";
        like $@, qr/\Asyntax error/, "do $mode is syntax error";
    }
}

{
    # follow-up to [perl #91844]: a do should always return a copy,
    # not the original

    my %foo;
    $foo{bar} = 7;
    my $r = \$foo{bar};
    sub {
        $$r++;
        isnt($_[0], $$r, "result of delete(helem) is copied: practical test");
    }->(do { 1; delete $foo{bar} });
}

# A do block should FREETMPS on exit
# RT #124248

{
    package p124248;
    my $d = 0;
    sub DESTROY { $d++ }
    sub f { ::is($d, 1, "RT 124248"); }
    f(do { 1; !!(my $x = bless []); });
}


# do file $!s must be correct
{
    local @INC = ('.'); #want EISDIR not ENOENT
    my $rv = do 'op'; # /t/op dir
    my $saved_error = $@;
    my $saved_errno = $!+0;
    ok(!$rv,                    "do dir returns false");
    ok(!$saved_error,           "\$\@ is false on do dir");
    ok($saved_errno == EISDIR,  "\$! is EISDIR on do dir");
}

done_testing();
