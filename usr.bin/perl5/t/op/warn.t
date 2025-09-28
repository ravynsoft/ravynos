#!./perl
#line 3 warn.t

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

plan 33;

my @warnings;
my $wa = []; my $ea = [];
$SIG{__WARN__} = sub { push @warnings, $_[0] };

@warnings = ();
$@ = "";
warn "foo\n";
ok @warnings==1 && $warnings[0] eq "foo\n";

@warnings = ();
$@ = "";
warn "foo", "bar\n";
ok @warnings==1 && $warnings[0] eq "foobar\n";

@warnings = ();
$@ = "";
warn "foo";
ok @warnings==1 && $warnings[0] eq "foo at warn.t line 29.\n";

@warnings = ();
$@ = "";
warn $wa;
ok @warnings==1 && ref($warnings[0]) eq "ARRAY" && $warnings[0] == $wa;

@warnings = ();
$@ = "";
warn "";
ok @warnings==1 &&
    $warnings[0] eq "Warning: something's wrong at warn.t line 39.\n";

@warnings = ();
$@ = "";
warn;
ok @warnings==1 &&
    $warnings[0] eq "Warning: something's wrong at warn.t line 45.\n";

@warnings = ();
$@ = "ERR\n";
warn "foo\n";
ok @warnings==1 && $warnings[0] eq "foo\n";

@warnings = ();
$@ = "ERR\n";
warn "foo", "bar\n";
ok @warnings==1 && $warnings[0] eq "foobar\n";

@warnings = ();
$@ = "ERR\n";
warn "foo";
ok @warnings==1 && $warnings[0] eq "foo at warn.t line 61.\n";

@warnings = ();
$@ = "ERR\n";
warn $wa;
ok @warnings==1 && ref($warnings[0]) eq "ARRAY" && $warnings[0] == $wa;

@warnings = ();
$@ = "ERR\n";
warn "";
ok @warnings==1 &&
    $warnings[0] eq "ERR\n\t...caught at warn.t line 71.\n";

@warnings = ();
$@ = "ERR\n";
warn;
ok @warnings==1 &&
    $warnings[0] eq "ERR\n\t...caught at warn.t line 77.\n";

@warnings = ();
$@ = $ea;
warn "foo\n";
ok @warnings==1 && $warnings[0] eq "foo\n";

@warnings = ();
$@ = $ea;
warn "foo", "bar\n";
ok @warnings==1 && $warnings[0] eq "foobar\n";

@warnings = ();
$@ = $ea;
warn "foo";
ok @warnings==1 && $warnings[0] eq "foo at warn.t line 93.\n";

@warnings = ();
$@ = $ea;
warn $wa;
ok @warnings==1 && ref($warnings[0]) eq "ARRAY" && $warnings[0] == $wa;

@warnings = ();
$@ = $ea;
warn "";
ok @warnings==1 && ref($warnings[0]) eq "ARRAY" && $warnings[0] == $ea;

@warnings = ();
$@ = $ea;
warn;
ok @warnings==1 && ref($warnings[0]) eq "ARRAY" && $warnings[0] == $ea;

fresh_perl_like(
 '
   $a = "\xee\n";
   print STDERR $a; warn $a;
   utf8::upgrade($a);
   print STDERR $a; warn $a;
 ',
  qr/^\xee(?:\r?\n\xee){3}/,
  { switches => [ "-C0" ] },
 'warn emits logical characters, not internal bytes [perl #45549]'  
);

SKIP: {
    skip_if_miniperl('miniperl ignores -C', 1);
   $ee = uni_to_native("\xee");
   $bytes = byte_utf8a_to_utf8n("\xc3\xae");
fresh_perl_like(
 "
   \$a = \"$ee\n\";
   print STDERR \$a; warn \$a;
   utf8::upgrade(\$a);
   print STDERR \$a; warn \$a;
 ",
  qr/^$bytes(?:\r?\n$bytes){3}/,
  { switches => ['-CE'] },
 'warn respects :utf8 layer'
);
}

$bytes = byte_utf8a_to_utf8n("\xc4\xac");
fresh_perl_like(
 'warn chr 300',
  qr/^Wide character in warn .*\n$bytes at /,
  { switches => [ "-C0" ] },
 'Wide character in warn (not print)'
);

fresh_perl_like(
 'warn []',
  qr/^ARRAY\(0x[\da-f]+\) at /a,
  { },
 'warn stringifies in the absence of $SIG{__WARN__}'
);

use Tie::Scalar;
tie $@, "Tie::StdScalar";

$@ = "foo\n";
@warnings = ();
warn;
is @warnings, 1;
like $warnings[0], qr/^foo\n\t\.\.\.caught at warn\.t /,
    '...caught is appended to tied $@';

$@ = \$_;
@warnings = ();
{
  local *{ref(tied $@) . "::STORE"} = sub {};
  undef $@;
}
warn;
is @warnings, 1;
is $warnings[0], \$_, '!SvOK tied $@ that returns ref is used';

untie $@;

@warnings = ();
{
  package o;
  use overload '""' => sub { "" };
}
tie $t, Tie::StdScalar;
$t = bless [], o;
{
  local *{ref(tied $t) . "::STORE"} = sub {};
  undef $t;
}
warn $t;
is @warnings, 1;
object_ok $warnings[0], 'o',
  'warn $tie_returning_object_that_stringifes_emptily';

@warnings = ();
eval "#line 42 Cholmondeley\n \$\@ = '3'; warn";
eval "#line 42 Cholmondeley\n \$\@ = 3; warn";
is @warnings, 2;
is $warnings[1], $warnings[0], 'warn treats $@=3 and $@="3" the same way';

fresh_perl_is(<<'EOF', "should be line 4 at - line 4.\n", {stderr => 1}, "");
${
    foo
} = "should be line 4";
warn $foo;
EOF

TODO: {
    local $::TODO = "Line numbers don't yet match up for \${ EXPR }";
    my $expected = <<'EOF';
line 1 at - line 1.
line 4 at - line 3.
also line 4 at - line 4.
line 5 at - line 5.
EOF
    fresh_perl_is(<<'EOF', $expected, {stderr => 1}, "");
warn "line 1";
(${
    foo
} = "line 5") && warn("line 4"); warn("also line 4");
warn $foo;
EOF
}

1;
# RT #132602 pp_warn in scalar context was extending the stack then
# setting SP back to the old, freed stack frame

fresh_perl_is(<<'EOF', "OK\n", {stderr => 1}, "RT #132602");
$SIG{__WARN__} = sub {};

my (@a, @b);
for my $i (1..300) {
    push @a, $i;
    () = (@a, warn);
}

# mess with the stack some more for ASan's benefit
for my $i (1..100) {
    push @a, $i;
    @b = @a;
}
print "OK\n";
EOF
