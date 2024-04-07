#!./perl

# This file tests the results of calling subroutines in the CORE::
# namespace with ampersand syntax.  In other words, it tests the bodies of
# the subroutines themselves, not the ops that they might inline themselves
# as when called as barewords.

# Other tests for CORE subs are in coresubs.t

BEGIN {
  chdir 't' if -d 't';
  require "./test.pl";
  set_up_inc( qw(. ../lib ../dist/if) );
  require './charset_tools.pl';
  $^P |= 0x100; # Provide informative "file" names for evals
}

sub lis($$;$) {
  &is(map(@$_ ? "[@{[map $_//'~~u~~', @$_]}]" : 'nought', @_[0,1]), $_[2]);
}

package hov {
  use overload '%{}' => sub { +{} }
}
package aov {
  use overload '@{}' => sub { [] }
}
package sov {
  use overload '${}' => sub { \my $x }
}

my %op_desc = (
  evalbytes => 'eval "string"',
  join      => 'join or string',
  pos       => 'match position',
  prototype => 'subroutine prototype',
  readline  => '<HANDLE>',
  readpipe  => 'quoted execution (``, qx)',
  reset     => 'symbol reset',
  ref       => 'reference-type operator',
  undef     => 'undef operator',
);
sub op_desc($) {
  return $op_desc{$_[0]} || $_[0];
}


# This tests that the &{} syntax respects the number of arguments implied
# by the prototype, plus some extra tests for the (_) prototype.
sub test_proto {
  my($o) = shift;

  # Create an alias, for the caller’s convenience.
  *{"my$o"} = \&{"CORE::$o"};

  my $p = prototype "CORE::$o";
  $p = '$;$' if $p eq '$_';

  if ($p eq '') {
    $tests ++;

    eval " &CORE::$o(1) ";
    like $@, qr/^Too many arguments for $o at /, "&$o with too many args";

  }
  elsif ($p =~ /^_;?\z/) {
    $tests ++;

    eval " &CORE::$o(1,2) ";
    my $desc = quotemeta op_desc($o);
    like $@, qr/^Too many arguments for $desc at /,
      "&$o with too many args";

    if (!@_) { return }

    $tests += 3;

    my($in,$out) = @_; # for testing implied $_

    # Since we have $in and $out values, we might as well test basic amper-
    # sand calls, too.

    is &{"CORE::$o"}($in), $out, "&$o";
    lis [&{"CORE::$o"}($in)], [$out], "&$o in list context";

    $_ = $in;
    is &{"CORE::$o"}(), $out, "&$o with no args";
  }
  elsif ($p =~ '^;([$*]+)\z') { # ;$ ;* ;$$ etc.
    my $maxargs = length $1;
    $tests += 1;    
    eval " &CORE::$o((1)x($maxargs+1)) ";
    my $desc = quotemeta op_desc($o);
    like $@, qr/^Too many arguments for $desc at /,
      "&$o with too many args";
  }
  elsif ($p =~ '^([$*]+);?\z') { # Fixed-length $$$ or ***
    my $args = length $1;
    $tests += 2;    
    my $desc = quotemeta op_desc($o);
    eval " &CORE::$o((1)x($args-1)) ";
    like $@, qr/^Not enough arguments for $desc at /, "&$o w/too few args";
    eval " &CORE::$o((1)x($args+1)) ";
    like $@, qr/^Too many arguments for $desc at /, "&$o w/too many args";
  }
  elsif ($p =~ '^([$*]+);([$*]+)\z') { # Variable-length $$$ or ***
    my $minargs = length $1;
    my $maxargs = $minargs + length $2;
    $tests += 2;    
    eval " &CORE::$o((1)x($minargs-1)) ";
    like $@, qr/^Not enough arguments for $o at /, "&$o with too few args";
    eval " &CORE::$o((1)x($maxargs+1)) ";
    like $@, qr/^Too many arguments for $o at /, "&$o with too many args";
  }
  elsif ($p eq '_;$') {
    $tests += 1;

    eval " &CORE::$o(1,2,3) ";
    like $@, qr/^Too many arguments for $o at /, "&$o with too many args";
  }
  elsif ($p eq '@') {
    # Do nothing, as we cannot test for too few or too many arguments.
  }
  elsif ($p =~ '^[$*;]+@\z') {
    $tests ++;    
    $p =~ ';@';
    my $minargs = $-[0];
    eval " &CORE::$o((1)x($minargs-1)) ";
    my $desc = quotemeta op_desc($o);
    like $@, qr/^Not enough arguments for $desc at /,
      "&$o with too few args";
  }
  elsif ($p =~ /^\*\\\$\$(;?)\$\z/) { #  *\$$$ and *\$$;$
    $tests += 5;

    eval "&CORE::$o(1,1,1,1,1)";
    like $@, qr/^Too many arguments for $o at /,
      "&$o with too many args";
    eval " &CORE::$o((1)x(\$1?2:3)) ";
    like $@, qr/^Not enough arguments for $o at /,
      "&$o with too few args";
    eval " &CORE::$o(1,[],1,1) ";
    like $@, qr/^Type of arg 2 to &CORE::$o must be scalar reference at /,
      "&$o with array ref arg";
    eval " &CORE::$o(1,1,1,1) ";
    like $@, qr/^Type of arg 2 to &CORE::$o must be scalar reference at /,
      "&$o with scalar arg";
    eval " &CORE::$o(1,bless([], 'sov'),1,1) ";
    like $@, qr/^Type of arg 2 to &CORE::$o must be scalar reference at /,
      "&$o with non-scalar arg w/scalar overload (which does not count)";
  }
  elsif ($p =~ /^\\%\$*\z/) { #  \% and \%$$
    $tests += 5;

    eval "&CORE::$o(" . join(",", (1) x length $p) . ")";
    like $@, qr/^Too many arguments for $o at /,
      "&$o with too many args";
    eval " &CORE::$o(" . join(",", (1) x (length($p)-2)) . ") ";
    like $@, qr/^Not enough arguments for $o at /,
      "&$o with too few args";
    my $moreargs = ",1" x (length($p) - 2);
    eval " &CORE::$o([]$moreargs) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash reference at /,
      "&$o with array ref arg";
    eval " &CORE::$o(*foo$moreargs) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash reference at /,
      "&$o with typeglob arg";
    eval " &CORE::$o(bless([], 'hov')$moreargs) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash reference at /,
      "&$o with non-hash arg with hash overload (which does not count)";
  }
  elsif ($p =~ /^(;)?\\\[(\$\@%&?\*)](\$\@)?\z/) {
    $tests += 3;

    unless ($3) {
      $tests ++;
      eval " &CORE::$o(1,2) ";
      like $@, qr/^Too many arguments for ${\op_desc($o)} at /,
        "&$o with too many args";
    }
    unless ($1) {
      $tests ++;
      eval { &{"CORE::$o"}($3 ? 1 : ()) };
      like $@, qr/^Not enough arguments for $o at /,
         "&$o with too few args";
    }
    my $more_args = $3 ? ',1' : '';
    eval " &CORE::$o(2$more_args) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be reference to one of(?x:
                ) \[\Q$2\E\] at /,
      "&$o with non-ref arg";
    eval " &CORE::$o(*STDOUT{IO}$more_args) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be reference to one of(?x:
                ) \[\Q$2\E\] at /,
      "&$o with ioref arg";
    my $class = ref *DATA{IO};
    eval " &CORE::$o(bless(*DATA{IO}, 'hov')$more_args) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be reference to one of(?x:
                ) \[\Q$2\E\] at /,
      "&$o with ioref arg with hash overload (which does not count)";
    bless *DATA{IO}, $class;
    if (do {$2 !~ /&/}) {
      $tests++;
      eval " &CORE::$o(\\&scriggle$more_args) ";
      like $@, qr/^Type of arg 1 to &CORE::$o must be reference to one (?x:
                  )of \[\Q$2\E\] at /,
        "&$o with coderef arg";
    }
  }
  elsif ($p =~ /^;?\\\@([\@;])?/) { #   ;\@   \@@   \@;$$@
    $tests += 7;

    if ($1) {
      eval { &{"CORE::$o"}() };
      like $@, qr/^Not enough arguments for $o at /,
        "&$o with too few args";
    }
    else {
      eval " &CORE::$o(\\\@1,2) ";
      like $@, qr/^Too many arguments for $o at /,
        "&$o with too many args";
    }
    eval " &CORE::$o(2) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with non-ref arg";
    eval " &CORE::$o(*STDOUT{IO}) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with ioref arg";
    my $class = ref *DATA{IO};
    eval " &CORE::$o(bless(*DATA{IO}, 'aov')) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with ioref arg with array overload (which does not count)";
    bless *DATA{IO}, $class;
    eval " &CORE::$o(\\&scriggle) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with coderef arg";
    eval " &CORE::$o(\\\$_) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with scalarref arg";
    eval " &CORE::$o({}) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be array reference at /,
      "&$o with hashref arg";
  }
  elsif ($p eq '\[%@]') {
    $tests += 7;

    eval " &CORE::$o(\\%1,2) ";
    like $@, qr/^Too many arguments for ${\op_desc($o)} at /,
      "&$o with too many args";
    eval { &{"CORE::$o"}() };
    like $@, qr/^Not enough arguments for $o at /,
      "&$o with too few args";
    eval " &CORE::$o(2) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash or array (?x:
                )reference at /,
      "&$o with non-ref arg";
    eval " &CORE::$o(*STDOUT{IO}) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash or array (?x:
                )reference at /,
      "&$o with ioref arg";
    my $class = ref *DATA{IO};
    eval " &CORE::$o(bless(*DATA{IO}, 'hov')) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash or array (?x:
                )reference at /,
      "&$o with ioref arg with hash overload (which does not count)";
    bless *DATA{IO}, $class;
    eval " &CORE::$o(\\&scriggle) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash or array (?x:
                )reference at /,
      "&$o with coderef arg";
    eval " &CORE::$o(\\\$_) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be hash or array (?x:
                )reference at /,
      "&$o with scalarref arg";
  }
  elsif ($p eq ';\[$*]') {
    $tests += 4;

    my $desc = quotemeta op_desc($o);
    eval " &CORE::$o(1,2) ";
    like $@, qr/^Too many arguments for $desc at /,
      "&$o with too many args";
    eval " &CORE::$o([]) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be scalar reference at /,
      "&$o with array ref arg";
    eval " &CORE::$o(1) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be scalar reference at /,
      "&$o with scalar arg";
    eval " &CORE::$o(bless([], 'sov')) ";
    like $@, qr/^Type of arg 1 to &CORE::$o must be scalar reference at /,
      "&$o with non-scalar arg w/scalar overload (which does not count)";
  }

  else {
    die "Please add tests for the $p prototype";
  }
}

# Test that &CORE::foo calls without parentheses (no new @_) can handle the
# total absence of any @_ without crashing.
undef *_;
&CORE::wantarray;
$tests++;
pass('no crash with &CORE::foo when *_{ARRAY} is undef');

test_proto '__FILE__';
test_proto '__LINE__';
test_proto '__PACKAGE__';
test_proto '__SUB__';

is file(), 'frob'    , '__FILE__ does check its caller'   ; ++ $tests;
is line(),  5        , '__LINE__ does check its caller'   ; ++ $tests;
is pakg(), 'stribble', '__PACKAGE__ does check its caller'; ++ $tests;
sub __SUB__test { &my__SUB__ }
is __SUB__test, \&__SUB__test, '&__SUB__';                  ++ $tests;

test_proto 'abs', -5, 5;

SKIP:
{
  if ($^O eq "MSWin32" && is_miniperl) {
    $tests += 8;
    skip "accept() not available in Win32 miniperl", 8
  }
  $tests += 6;
  test_proto 'accept';
  eval q{
    is &CORE::accept(qw{foo bar}), undef, "&accept";
    lis [&{"CORE::accept"}(qw{foo bar})], [undef], "&accept in list context";

    &myaccept(my $foo, my $bar);
    is ref $foo, 'GLOB', 'CORE::accept autovivifies its first argument';
    is $bar, undef, 'CORE::accept does not autovivify its second argument';
    use strict;
    undef $foo;
    eval { 'myaccept'->($foo, $bar) };
    like $@, qr/^Can't use an undefined value as a symbol reference at/,
    'CORE::accept will not accept undef 2nd arg under strict';
    is ref $foo, 'GLOB', 'CORE::accept autovivs its first arg under strict';
  };
}

test_proto 'alarm';
test_proto 'atan2';

test_proto 'bind';
$tests += 3;
SKIP:
{
  skip "bind() not available in Win32 miniperl", 3
    if $^O eq "MSWin32" && is_miniperl();
  is &CORE::bind('foo', 'bear'), undef, "&bind";
  lis [&CORE::bind('foo', 'bear')], [undef], "&bind in list context";
  eval { &mybind(my $foo, "bear") };
  like $@, qr/^Bad symbol for filehandle at/,
    'CORE::bind dies with undef first arg';
}

test_proto 'binmode';
$tests += 3;
is &CORE::binmode(qw[foo bar]), undef, "&binmode";
lis [&CORE::binmode(qw[foo bar])], [undef], "&binmode in list context";
is &mybinmode(foo), undef, '&binmode with one arg';

test_proto 'bless';
$tests += 3;
like &CORE::bless([],'parcel'), qr/^parcel=ARRAY/, "&bless";
like join(" ", &CORE::bless([],'parcel')), qr/^parcel=ARRAY(?!.* )/,
  "&bless in list context";
like &mybless([]), qr/^main=ARRAY/, '&bless with one arg';

test_proto 'break';
{
  $tests ++;
  my $tmp;
  no warnings 'deprecated';
  CORE::given(1) {
    CORE::when(1) {
      &mybreak;
      $tmp = 'bad';
    }
  }
  is $tmp, undef, '&break';
}

test_proto 'caller';
$tests += 4;
sub caller_test {
  is scalar &CORE::caller, 'hadhad', '&caller';
  is scalar &CORE::caller(1), 'main', '&caller(1)';
  lis [&CORE::caller], [caller], '&caller in list context';
  # The last element of caller in list context is a hint hash, which
  # may be a different hash for caller vs &CORE::caller, so an eq com-
  # parison (which lis() uses for convenience) won’t work.  So just
  # pop the last element, since the rest are sufficient to prove that
  # &CORE::caller works.
  my @ampcaller = &CORE::caller(1);
  my @caller    = caller(1);
  pop @ampcaller; pop @caller;
  lis \@ampcaller, \@caller, '&caller(1) in list context';
}
sub {
  package hadhad;
  ::caller_test();
}->();

test_proto 'chmod';
$tests += 3;
is &CORE::chmod(), 0, '&chmod with no args';
is &CORE::chmod(0666), 0, '&chmod';
lis [&CORE::chmod(0666)], [0], '&chmod in list context';

test_proto 'chown';
$tests += 4;
is &CORE::chown(), 0, '&chown with no args';
is &CORE::chown(1), 0, '&chown with 1 arg';
is &CORE::chown(1,2), 0, '&chown';
lis [&CORE::chown(1,2)], [0], '&chown in list context';

test_proto 'chr', 5, "\5";
test_proto 'chroot';

test_proto 'close';
{
  last if is_miniperl;
  $tests += 3;

  open my $fh, ">", \my $buffalo;
  print $fh 'an address in the outskirts of Jersey';
  ok &CORE::close($fh), '&CORE::close retval';
  print $fh 'lalala';
  is $buffalo, 'an address in the outskirts of Jersey',
    'effect of &CORE::close';
  # This has to be a separate variable from $fh, as re-using the same
  # variable can cause the tests to pass by accident.  That actually hap-
  # pened during developement, because the second close() was reading
  # beyond the end of the stack and finding a $fh left over from before.
  open my $fh2, ">", \($buffalo = '');
  select+(select($fh2), do {
    print "Nasusiro Tokasoni";
    &CORE::close();
    print "jfd";
    is $buffalo, "Nasusiro Tokasoni", '&CORE::close with no args';
  })[0];
}
lis [&CORE::close('tototootot')], [''], '&close in list context'; ++$tests;

test_proto 'closedir';
$tests += 2;
is &CORE::closedir(foo), undef, '&CORE::closedir';
lis [&CORE::closedir(foo)], [undef], '&CORE::closedir in list context';

test_proto 'connect';
$tests += 2;
SKIP:
{
  skip "connect() not available in Win32 miniperl", 2
    if $^O eq "MSWin32" && is_miniperl();
  is &CORE::connect('foo','bar'), undef, '&connect';
  lis [&myconnect('foo','bar')], [undef], '&connect in list context';
}

test_proto 'continue';
$tests ++;
no warnings 'deprecated';
CORE::given(1) {
  CORE::when(1) {
    &mycontinue();
  }
  pass "&continue";
}

test_proto 'cos';
test_proto 'crypt';

test_proto 'dbmclose';
test_proto 'dbmopen';
{
  last unless eval { require AnyDBM_File };
  $tests ++;
  my $filename = tempfile();
  &mydbmopen(\my %db, $filename, 0666);
  $db{1} = 2; $db{3} = 4;
  &mydbmclose(\%db);
  is scalar keys %db, 0, '&dbmopen and &dbmclose';
  my $Dfile = "$filename.pag";
  if (! -e $Dfile) {
    ($Dfile) = <$filename*>;
  }
  if ($^O eq 'VMS') {
    unlink "$filename.sdbm_dir", $Dfile;
  } else {
    unlink "$filename.dir", $Dfile;
  }
}

test_proto 'die';
eval { dier('quinquangle') };
is $@, "quinquangle at frob line 6.\n", '&CORE::die'; $tests ++;

test_proto $_ for qw(
  endgrent endhostent endnetent endprotoent endpwent endservent
);

test_proto 'evalbytes';
$tests += 4;
{
  my $U_100_bytes = byte_utf8a_to_utf8n("\xc4\x80");
  chop(my $upgraded = "use utf8; $U_100_bytes" . chr 256);
  is &myevalbytes($upgraded), chr 256, '&evalbytes';
  # Test hints
  require strict;
  strict->import;
  &myevalbytes('
    is someone, "someone", "run-time hint bits do not leak into &evalbytes"
  ');
  use strict;
  BEGIN { $^H{coreamp} = 42 }
  $^H{coreamp} = 75;
  &myevalbytes('
    BEGIN {
      is $^H{coreamp}, 42, "compile-time hh propagates into &evalbytes";
    }
    ${"frobnicate"}
  ');
  like $@, qr/strict/, 'compile-time hint bits propagate into &evalbytes';
}

test_proto 'exit';
$tests ++;
is runperl(prog => '&CORE::exit; END { print qq-ok\n- }'), "ok\n",
  '&exit with no args';

test_proto 'fork';

test_proto 'formline';
$tests += 3;
is &myformline(' @<<< @>>>', 1, 2), 1, '&myformline retval';
is $^A,        ' 1       2', 'effect of &myformline';
lis [&myformline('@')], [1], '&myformline in list context';

test_proto 'each';
$tests += 4;
is &myeach({ "a","b" }), "a", '&myeach(\%hash) in scalar cx';
lis [&myeach({qw<a b>})], [qw<a b>], '&myeach(\%hash) in list cx';
is &myeach([ "a","b" ]), 0, '&myeach(\@array) in scalar cx';
lis [&myeach([qw<a b>])], [qw<0 a>], '&myeach(\@array) in list cx';

test_proto 'exp';

test_proto 'fc';
$tests += 2;
{
  my $sharp_s = uni_to_native("\xdf");
  is &myfc($sharp_s), $sharp_s, '&fc, no unicode_strings';
  use feature 'unicode_strings';
  is &myfc($sharp_s), "ss", '&fc, unicode_strings';
}

test_proto 'fcntl';

test_proto 'fileno';
$tests += 2;
is &CORE::fileno(\*STDIN), fileno STDIN, '&CORE::fileno';
lis [&CORE::fileno(\*STDIN)], [fileno STDIN], '&CORE::fileno in list cx';

test_proto 'flock';
test_proto 'fork';

test_proto 'getc';
{
  last if is_miniperl;
  $tests += 3;
  local *STDIN;
  open my $fh, "<", \(my $buf='falo');
  open STDIN, "<", \(my $buf2 = 'bison');
  is &mygetc($fh), 'f', '&mygetc';
  is &mygetc(), 'b', '&mygetc with no args';
  lis [&mygetc($fh)], ['a'], '&mygetc in list context';
}

test_proto "get$_" for qw '
  grent grgid grnam hostbyaddr hostbyname hostent login netbyaddr netbyname
  netent peername
';

test_proto 'getpgrp';
eval {&mygetpgrp()};
pass '&getpgrp with no args does not crash'; $tests++;

test_proto "get$_" for qw '
  ppid priority protobyname protobynumber protoent
  pwent pwnam pwuid servbyname servbyport servent sockname sockopt
';

# Make sure the following tests test what we think they are testing.
ok ! $CORE::{glob}, '*CORE::glob not autovivified yet'; $tests ++;
{
  # Make sure ck_glob does not respect the override when &CORE::glob is
  # autovivified (by test_proto).
  local *CORE::GLOBAL::glob = sub {};
  test_proto 'glob';
}
$_ = "t/*.t";
@_ = &myglob($_);
is join($", &myglob()), "@_", '&glob without arguments';
is join($", &myglob("t/*.t")), "@_", '&glob with an arg';
$tests += 2;

test_proto 'gmtime';
&CORE::gmtime;
pass '&gmtime without args does not crash'; ++$tests;

test_proto 'hex', ff=>255;

test_proto 'index';
$tests += 3;
is &myindex("foffooo","o",2),4,'&index';
lis [&myindex("foffooo","o",2)],[4],'&index in list context';
is &myindex("foffooo","o"),1,'&index with 2 args';

test_proto 'int', 1.5=>1;
test_proto 'ioctl';

test_proto 'join';
$tests += 2;
is &myjoin('a','b','c'), 'bac', '&join';
lis [&myjoin('a','b','c')], ['bac'], '&join in list context';

test_proto 'keys';
$tests += 6;
is &mykeys({ 1..4 }), 2, '&mykeys(\%hash) in scalar cx';
lis [sort &mykeys({1..4})], [1,3], '&mykeys(\%hash) in list cx';
is &mykeys([ 1..4 ]), 4, '&mykeys(\@array) in scalar cx';
lis [&mykeys([ 1..4 ])], [0..3], '&mykeys(\@array) in list cx';

SKIP: {
  skip "no Hash::Util on miniperl", 2, if is_miniperl;
  require Hash::Util;
  sub Hash::Util::bucket_ratio (\%);

  my %h = 1..2;
  &mykeys(\%h) = 1024;
  like Hash::Util::bucket_ratio(%h), qr!/(?:1024|2048)\z!, '&mykeys = changed number of buckets allocated';
  eval { (&mykeys(\%h)) = 1025; };
  like $@, qr/^Can't modify keys in list assignment at /;
}

test_proto 'kill'; # set up mykill alias
if ($^O ne 'riscos') {
  $tests ++;
  ok( &mykill(0, $$), '&kill' );
}

test_proto 'lc', 'A', 'a';
test_proto 'lcfirst', 'AA', 'aA';
test_proto 'length', 'aaa', 3;
test_proto 'link';
test_proto 'listen';

test_proto 'localtime';
&CORE::localtime;
pass '&localtime without args does not crash'; ++$tests;

test_proto 'lock';
$tests += 6;
is \&mylock(\$foo), \$foo, '&lock retval when passed a scalar ref';
lis [\&mylock(\$foo)], [\$foo], '&lock in list context';
is &mylock(\@foo), \@foo, '&lock retval when passed an array ref';
is &mylock(\%foo), \%foo, '&lock retval when passed a ash ref';
is &mylock(\&foo), \&foo, '&lock retval when passed a code ref';
is \&mylock(\*foo), \*foo, '&lock retval when passed a glob ref';

test_proto 'log';

test_proto 'mkdir';
# mkdir is tested with implicit $_ at the end, to make the test easier

test_proto "msg$_" for qw( ctl get rcv snd );

test_proto 'not';
$tests += 2;
is &mynot(1), !1, '&not';
lis [&mynot(0)], [!0], '&not in list context';

test_proto 'oct', '666', 438;

test_proto 'open';
$tests += 5;
$file = 'test.pl';
ok &myopen('file'), '&open with 1 arg' or warn "1-arg open: $!";
like <file>, qr|^#|, 'result of &open with 1 arg';
close file;
{
  ok &myopen(my $fh, "test.pl"), 'two-arg &open';
  ok $fh, '&open autovivifies';
  like <$fh>, qr '^#', 'result of &open with 2 args';
  last if is_miniperl;
  $tests +=2;
  ok &myopen(my $fh2, "<", \"sharummbles"), 'retval of 3-arg &open';
  is <$fh2>, 'sharummbles', 'result of three-arg &open';
}

test_proto 'opendir';
test_proto 'ord', chr(utf8::unicode_to_native(64)), utf8::unicode_to_native(64);

test_proto 'pack';
$tests += 2;
my $Perl_as_a_hex_string =
  join "", map { sprintf("%2X", utf8::unicode_to_native($_)) } 0x50, 0x65, 0x72, 0x6c;
is &mypack("H*", $Perl_as_a_hex_string), 'Perl', '&pack';
lis [&mypack("H*", $Perl_as_a_hex_string)], ['Perl'], '&pack in list context';

test_proto 'pipe';

test_proto 'pop';
$tests += 6;
@ARGV = qw<a b c>;
is &mypop(), 'c', 'retval of &pop with no args (@ARGV)';
is "@ARGV", "a b", 'effect of &pop on @ARGV';
sub {
  is &mypop(), 'k', 'retval of &pop with no args (@_)';
  is "@_", "q j", 'effect of &pop on @_';
}->(qw(q j k));
{
  my @a = 1..4;
  is &mypop(\@a), 4, 'retval of &pop';
  lis [@a], [1..3], 'effect of &pop';
}

test_proto 'pos';
$tests += 4;
$_ = "hello";
pos = 3;
is &mypos, 3, 'reading &pos without args';
&mypos = 4;
is pos, 4, 'writing to &pos without args';
{
  my $x = "gubai";
  pos $x = 3;
  is &mypos(\$x), 3, 'reading &pos without args';
  &mypos(\$x) = 4;
  is pos $x, 4, 'writing to &pos without args';
}

test_proto 'prototype';
$tests++;
is &myprototype(\&myprototype), prototype("CORE::prototype"), '&prototype';

test_proto 'push';
$tests += 2;
{
  my @a = qw<a b c>;
  is &mypush(\@a, "d", "e"), 5, 'retval of &push';
  is "@a", "a b c d e", 'effect of &push';
}

test_proto 'quotemeta', '$', '\$';

test_proto 'rand';
$tests += 3;
my $r = &CORE::rand;
ok eval {
  use warnings FATAL => qw{numeric uninitialized};
  $r >= 0 && $r < 1;
}, '&rand returns a valid number';
unlike join(" ", &CORE::rand), qr/ /, '&rand in list context';
&cmp_ok(&CORE::rand(78), qw '< 78', '&rand with 1 arg');

test_proto 'read';
{
  last if is_miniperl;
  $tests += 5;
  open my $fh, "<", \(my $buff = 'morays have their mores');
  ok &myread($fh, \my $input, 6), '&read with 3 args';
  is $input, 'morays', 'value read by 3-arg &read';
  ok &myread($fh, \$input, 6, 6), '&read with 4 args';
  is $input, 'morays have ', 'value read by 4-arg &read';
  is +()=&myread($fh, \$input, 6), 1, '&read in list context';
}

test_proto 'readdir';

test_proto 'readline';
{
  local *ARGV = *DATA;
  $tests ++;
  is scalar &myreadline,
    "I wandered lonely as a cloud\n", '&readline w/no args';
}
{
  last if is_miniperl;
  $tests += 2;
  open my $fh, "<", \(my $buff = <<END);
The Recursive Problem
---------------------
I have a problem I cannot solve.
The problem is that I cannot solve it.
END
  is &myreadline($fh), "The Recursive Problem\n",
    '&readline with 1 arg';
  lis [&myreadline($fh)], [
       "---------------------\n",
       "I have a problem I cannot solve.\n",
       "The problem is that I cannot solve it.\n",
      ], '&readline in list context';
}

test_proto 'readlink';
test_proto 'readpipe';
test_proto 'recv';

use if !is_miniperl, File::Spec::Functions, qw "catfile";
use if !is_miniperl, File::Temp, 'tempdir';

test_proto 'rename';
{
  last if is_miniperl;
  $tests ++;
  my $dir = tempdir(uc cleanup => 1);
  my $tmpfilenam = catfile $dir, 'aaa';
  open my $fh, ">", $tmpfilenam or die "cannot open $tmpfilenam: $!";
  close $fh or die "cannot close $tmpfilenam: $!";
  &myrename("$tmpfilenam", $tmpfilenam = catfile $dir,'bbb');
  ok open(my $fh, '>', $tmpfilenam), '&rename';
}

test_proto 'ref', [], 'ARRAY';

test_proto 'reset';
$tests += 2;
my $oncer = sub { "a" =~ m?a? };
&$oncer;
&myreset;
ok &$oncer, '&reset with no args';
package resettest {
  $b = "c";
  $banana = "cream";
  &::myreset('b');
  ::lis [$b,$banana],[(undef)x2], '1-arg &reset';
}

test_proto 'reverse';
$tests += 2;
is &myreverse('reward'), 'drawer', '&reverse';
lis [&myreverse(qw 'dog bites man')], [qw 'man bites dog'],
  '&reverse in list context';

test_proto 'rewinddir';

test_proto 'rindex';
$tests += 3;
is &myrindex("foffooo","o",2),1,'&rindex';
lis [&myrindex("foffooo","o",2)],[1],'&rindex in list context';
is &myrindex("foffooo","o"),6,'&rindex with 2 args';

test_proto 'rmdir';

test_proto 'scalar';
$tests += 2;
is &myscalar(3), 3, '&scalar';
lis [&myscalar(3)], [3], '&scalar in list cx';

test_proto 'seek';
{
  last if is_miniperl;
  $tests += 1;
  open my $fh, "<", \"misled" or die $!;
  &myseek($fh, 2, 0);
  is <$fh>, 'sled', '&seek in action';
}

test_proto 'seekdir';

# Can’t test_proto, as it has none
$tests += 8;
*myselect = \&CORE::select;
is defined prototype &myselect, defined prototype "CORE::select",
  'prototype of &select (or lack thereof)';
is &myselect, select, '&select with no args';
{
  my $prev = select;
  is &myselect(my $fh), $prev, '&select($arg) retval';
  is lc ref $fh, 'glob', '&select autovivifies';
  is select, $fh, '&select selects';
  select $prev;
}
eval { &myselect(1,2) };
like $@, qr/^Not enough arguments for select system call at /,
  '&myselect($two,$args)';
eval { &myselect(1,2,3) };
like $@, qr/^Not enough arguments for select system call at /,
  '&myselect($with,$three,$args)';
eval { &myselect(1,2,3,4,5) };
like $@, qr/^Too many arguments for select system call at /,
  '&myselect($a,$total,$of,$five,$args)';
unless ($^O eq "MSWin32" && is_miniperl) {
  &myselect((undef)x3,.25);
  # Just have to assume that worked. :-) If we get here, at least it didn’t
  # crash or anything.
  # select() is unimplemented in Win32 miniperl
}

test_proto "sem$_" for qw "ctl get op";

test_proto 'send';

test_proto "set$_" for qw '
  grent hostent netent
';

test_proto 'setpgrp';
$tests +=2;
eval { &mysetpgrp( 0) };
pass "&setpgrp with one argument";
eval { &mysetpgrp };
pass "&setpgrp with no arguments";

test_proto "set$_" for qw '
  priority protoent pwent servent sockopt
';

test_proto 'shift';
$tests += 6;
@ARGV = qw<a b c>;
is &myshift(), 'a', 'retval of &shift with no args (@ARGV)';
is "@ARGV", "b c", 'effect of &shift on @ARGV';
sub {
  is &myshift(), 'q', 'retval of &shift with no args (@_)';
  is "@_", "j k", 'effect of &shift on @_';
}->(qw(q j k));
{
  my @a = 1..4;
  is &myshift(\@a), 1, 'retval of &shift';
  lis [@a], [2..4], 'effect of &shift';
}

test_proto "shm$_" for qw "ctl get read write";
test_proto 'shutdown';
test_proto 'sin';
test_proto 'sleep';
test_proto "socket$_" for "", "pair";

test_proto 'splice';
$tests += 8;
{
  my @a = qw<a b c>;
  is &mysplice(\@a, 1), 'c', 'retval of 2-arg &splice in scalar context';
  lis \@a, ['a'], 'effect of 2-arg &splice in scalar context';
  @a = qw<a b c>;
  lis [&mysplice(\@a, 1)], ['b','c'], 'retval of 2-arg &splice in list cx';
  lis \@a, ['a'], 'effect of 2-arg &splice in list context';
  @a = qw<a b c d>;
  lis [&mysplice(\@a,1,2)],['b','c'], 'retval of 3-arg &splice in list cx';
  lis \@a, ['a','d'], 'effect of 3-arg &splice in list context';
  @a = qw<a b c d>;
  lis [&mysplice(\@a,1,1,'e')],['b'], 'retval of 4-arg &splice in list cx';
  lis \@a, [qw<a e c d>], 'effect of 4-arg &splice in list context';
}

test_proto 'sprintf';
$tests += 2;
is &mysprintf("%x", 65), '41', '&sprintf';
lis [&mysprintf("%x", '65')], ['41'], '&sprintf in list context';

test_proto 'sqrt', 4, 2;

test_proto 'srand';
$tests ++;
&CORE::srand;
() = &CORE::srand;
pass '&srand with no args does not crash';

test_proto 'study';

test_proto 'substr';
$tests += 5;
$_ = "abc";
is &mysubstr($_, 1, 1, "d"), 'b', '4-arg &substr';
is $_, 'adc', 'what 4-arg &substr does';
is &mysubstr("abc", 1, 1), 'b', '3-arg &substr';
is &mysubstr("abc", 1), 'bc', '2-arg &substr';
&mysubstr($_, 1) = 'long';
is $_, 'along', 'lvalue &substr';

test_proto 'symlink';
test_proto 'syscall';

test_proto 'sysopen';
$tests +=2;
{
  &mysysopen(my $fh, 'test.pl', 0);
  pass '&sysopen does not crash with 3 args';
  ok $fh, 'sysopen autovivifies';
}

test_proto 'sysread';
test_proto 'sysseek';
test_proto 'syswrite';

test_proto 'tell';
{
  $tests += 2;
  open my $fh, "test.pl" or die "Cannot open test.pl";
  <$fh>;
  is &mytell(), tell($fh), '&tell with no args';
  is &mytell($fh), tell($fh), '&tell with an arg';
}

test_proto 'telldir';

test_proto 'tie';
test_proto 'tied';
$tests += 3;
{
  my $fetches;
  package tier {
    sub TIESCALAR { bless[] }
    sub FETCH { ++$fetches }
  }
  my $tied;
  my $obj = &mytie(\$tied, 'tier');
  is &mytied(\$tied), $obj, '&tie and &tied retvals';
  () = "$tied";
  is $fetches, 1, '&tie actually ties';
  &CORE::untie(\$tied);
  () = "$tied";
  is $fetches, 1, '&untie unties';
}

test_proto 'time';
$tests += 2;
like &mytime, qr/^\d+\z/, '&time in scalar context';
like join('-', &mytime), qr/^\d+\z/, '&time in list context';

test_proto 'times';
$tests += 2;
like &mytimes, qr/^[\d.]+\z/, '&times in scalar context';
like join('-',&mytimes), qr/^[\d.]+-[\d.]+-[\d.]+-[\d.]+\z/,
  '&times in list context';

test_proto 'uc', 'aa', 'AA';
test_proto 'ucfirst', 'aa', "Aa";

test_proto 'umask';
$tests ++;
is &myumask, umask, '&umask with no args';

test_proto 'undef';
$tests += 12;
is &myundef(), undef, '&undef returns undef';
lis [&myundef()], [undef], '&undef returns undef in list cx';
lis [&myundef(\$_)], [undef], '&undef(...) returns undef in list cx';
is \&myundef(), \undef, '&undef returns the right undef';
$_ = 'anserine questions';
&myundef(\$_);
is $_, undef, '&undef(\$_) undefines $_';
@_ = 1..3;
&myundef(\@_);
is @_, 0, '&undef(\@_) undefines @_';
%_ = 1..4;
&myundef(\%_);
ok !%_, '&undef(\%_) undefines %_';
&myundef(\&utf8::valid); # nobody should be using this :-)
ok !defined &utf8::valid, '&undef(\&foo) undefines &foo';
@_ = \*_;
&myundef;
is *_{ARRAY}, undef, '@_=\*_, &undef undefines *_';
@_ = \*_;
&myundef(\*_);
is *_{ARRAY}, undef, '&undef(\*_) undefines *_';
(&myundef(), @_) = 1..10;
lis \@_, [2..10], 'list assignment to &undef()';
ok !defined undef, 'list assignment to &undef() does not affect undef'; 
undef @_;

test_proto 'unpack';
$tests += 2;
my $abcd_as_a_hex_string =
  join "", map { sprintf("%2X", utf8::unicode_to_native($_)) } 0x61, 0x62, 0x63, 0x64;
my $bcde_as_a_hex_string =
  join "", map { sprintf("%2X", utf8::unicode_to_native($_)) } 0x62, 0x63, 0x64, 0x65;
$_ = 'abcd';
is &myunpack("H*"), $abcd_as_a_hex_string, '&unpack with one arg';
is &myunpack("H*", "bcde"), $bcde_as_a_hex_string, '&unpack with two arg';


test_proto 'unshift';
$tests += 2;
{
  my @a = qw<a b c>;
  is &myunshift(\@a, "d", "e"), 5, 'retval of &unshift';
  is "@a", "d e a b c", 'effect of &unshift';
}

test_proto 'untie'; # behaviour already tested along with tie(d)

test_proto 'utime';
$tests += 2;
is &myutime(undef,undef), 0, '&utime';
lis [&myutime(undef,undef)], [0], '&utime in list context';

test_proto 'values';
$tests += 4;
is &myvalues({ 1..4 }), 2, '&myvalues(\%hash) in scalar cx';
lis [sort &myvalues({1..4})], [2,4], '&myvalues(\%hash) in list cx';
is &myvalues([ 1..4 ]), 4, '&myvalues(\@array) in scalar cx';
lis [&myvalues([ 1..4 ])], [1..4], '&myvalues(\@array) in list cx';

test_proto 'vec';
$tests += 3;
is &myvec("foo", 0, 4), 6, '&vec';
lis [&myvec("foo", 0, 4)], [6], '&vec in list context';
$tmp = "foo";
++&myvec($tmp,0,4);
is $tmp, "goo", 'lvalue &vec';

test_proto 'wait';
test_proto 'waitpid';

test_proto 'wantarray';
$tests += 4;
my $context;
my $cx_sub = sub {
  $context = qw[void scalar list][&mywantarray + defined mywantarray()]
};
() = &$cx_sub;
is $context, 'list', '&wantarray with caller in list context';
scalar &$cx_sub;
is($context, 'scalar', '&wantarray with caller in scalar context');
&$cx_sub;
is($context, 'void', '&wantarray with caller in void context');
lis [&mywantarray],[wantarray], '&wantarray itself in list context';

test_proto 'warn';
{ $tests += 3;
  my $w;
  local $SIG{__WARN__} = sub { $w = shift };
  is &mywarn('a'), 1, '&warn retval';
  is $w, "a at " . __FILE__ . " line " . (__LINE__-1) . ".\n", 'warning';
  lis [&mywarn()], [1], '&warn retval in list context';
}

test_proto 'write';
$tests ++;
eval {&mywrite};
like $@, qr'^Undefined format "STDOUT" called',
  "&write without arguments can handle the null";

# This is just a check to make sure we have tested everything.  If we
# haven’t, then either the sub needs to be tested or the list in
# gv.c is wrong.
{
  last if is_miniperl;
  require File::Spec::Functions;
  my $keywords_file =
    File::Spec::Functions::catfile(
      File::Spec::Functions::updir,'regen','keywords.pl'
    );
  my %nottest_words = map { $_ => 1 } qw(
    ADJUST AUTOLOAD BEGIN CHECK CORE DESTROY END INIT UNITCHECK
    __DATA__ __END__
    and catch class cmp default defer do dump else elsif eq eval field finally
    for foreach format ge given goto grep gt if isa last le local lt m map
    method my ne next no or our package print printf q qq qr qw qx redo require
    return s say sort state sub tr try unless until use when while x xor y
  );
  open my $kh, $keywords_file
    or die "$0 cannot open $keywords_file: $!";
  while(<$kh>) {
    if (m?__END__?..${\0} and /^[-+](.*)/) {
      my $word = $1;
      next if $nottest_words{$word};
      $tests ++;
      ok   exists &{"my$word"}
        || (eval{&{"CORE::$word"}}, $@ =~ /cannot be called directly/),
        "$word either has been tested or is not ampable";
    }
  }
}

# Add new tests above this line.

# This test must come last (before the test count test):

{
  last if is_miniperl;
  require Cwd;
  import Cwd;
  $tests += 3;
  require File::Temp ;
  my $dir = File::Temp::tempdir(uc cleanup => 1);
  my $cwd = cwd();
  chdir($dir);

  # Make sure that implicit $_ is not applied to mkdir’s second argument.
  local $^W = 1;
  my $warnings;
  local $SIG{__WARN__} = sub { ++$warnings };

  local $_ = 'Phoo';
  ok &mymkdir(), '&mkdir';
  like <*>, qr/^phoo(.DIR)?\z/i, 'mkdir works with implicit $_';

  is $warnings, undef, 'no implicit $_ for second argument to mkdir';

  chdir($cwd); # so auto-cleanup can remove $dir
}

# ------------ END TESTING ----------- #

done_testing $tests;

#line 3 frob

sub file { &CORE::__FILE__ }
sub line { &CORE::__LINE__ } # 5
sub dier { &CORE::die(@_)  } # 6
package stribble;
sub main::pakg { &CORE::__PACKAGE__ }

# Please do not add new tests here.
package main;
CORE::__DATA__
I wandered lonely as a cloud
That floats on high o'er vales and hills,
And all at once I saw a crowd, 
A host of golden daffodils!
Beside the lake, beneath the trees,
Fluttering, dancing, in the breeze.
-- Wordsworth
