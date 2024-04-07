BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bXS\/Typemap\b/) {
        print "1..0 # Skip: XS::Typemap was not built\n";
        exit 0;
    }
}

use Test::More tests => 170;

use strict;
#catch WARN_INTERNAL type errors, and anything else unexpected
use warnings FATAL => 'all';
use XS::Typemap;

pass();

# Some inheritance trees to check ISA relationships
BEGIN {
  package intObjPtr::SubClass;
  use parent '-norequire', qw/ intObjPtr /;
  sub xxx { 1; }
}

BEGIN {
  package intRefIvPtr::SubClass;
  use parent '-norequire', qw/ intRefIvPtr /;
  sub xxx { 1 }
}

# T_SV - standard perl scalar value
note("T_SV");
my $sv = "Testing T_SV";
is( T_SV($sv), $sv);

# T_SV with output
is_deeply([ T_SV_output($sv) ], [], "T_SV_output: no return value");
is($sv, "test", "T_SV_output: output written to");

# T_SVREF - reference to Scalar
note("T_SVREF");
$sv .= "REF";
my $svref = \$sv;
is( T_SVREF($svref), $svref );
is( ${ T_SVREF($svref) }, $$svref );

# Now test that a non reference is rejected
# the typemaps croak
eval { T_SVREF( "fail - not ref" ) };
ok( $@ );

note("T_SVREF_REFCOUNT_FIXED");
is( T_SVREF_REFCOUNT_FIXED($svref), $svref );
is( ${ T_SVREF_REFCOUNT_FIXED($svref) }, $$svref );
eval { T_SVREF_REFCOUNT_FIXED( "fail - not ref" ) };
ok( $@ );

# output only
SKIP:{
   my $svr;
   is_deeply([ T_SVREF_REFCOUNT_FIXED_output($svr) ], [ ], "call with non-ref lvalue, no return value");
   ok(ref $svr, "output parameter now a reference")
     or skip "Not a reference", 1;
   is($$svr, "test", "reference to correct value");
}

# T_AVREF - reference to a perl Array
note("T_AVREF");
my @array;
is( T_AVREF(\@array), \@array);
# Now test that a non array ref is rejected
eval { T_AVREF( \$sv ) };
ok( $@ );

# T_AVREF_REFCOUNT_FIXED  - reference to a perl Array, refcount fixed
note("T_AVREF_REFCOUNT_FIXED");
is( T_AVREF_REFCOUNT_FIXED(\@array), \@array);
# Now test that a non array ref is rejected
eval { T_AVREF_REFCOUNT_FIXED( \$sv ) };
ok( $@ );

# output only
SKIP:{
   my $avr;
   is_deeply([ T_AVREF_REFCOUNT_FIXED_output($avr) ], [ ], "call with non-ref lvalue, no return value");
   ok(ref $avr, "output parameter now a reference")
     or skip "Not a reference", 1;
   is_deeply($avr, [ "test" ], "has expected entry");
}

# T_HVREF - reference to a perl Hash
note("T_HVREF");
my %hash;
is( T_HVREF(\%hash), \%hash);
# Now test that a non hash ref is rejected
eval { T_HVREF( \@array ) };
ok( $@ );


# T_HVREF_REFCOUNT_FIXED - reference to a perl Hash, refcount fixed
note("T_HVREF_REFCOUNT_FIXED");
is( T_HVREF_REFCOUNT_FIXED(\%hash), \%hash);
# Now test that a non hash ref is rejected
eval { T_HVREF_REFCOUNT_FIXED( \@array ) };
ok( $@ );

# output only
SKIP:{
   my $hvr;
   is_deeply([ T_HVREF_REFCOUNT_FIXED_output($hvr) ], [ ], "call with non-ref lvalue, no return value");
   ok(ref $hvr, "output parameter now a reference")
     or skip "Not a reference", 1;
   is($hvr->{test}, "value", "has expected key");
}

# T_CVREF - reference to perl subroutine
note("T_CVREF");
my $sub = sub { 1 };
is( T_CVREF($sub), $sub );
# Now test that a non code ref is rejected
eval { T_CVREF( \@array ) };
ok( $@ );

is( T_CVREF_REFCOUNT_FIXED($sub), $sub );
# Now test that a non code ref is rejected
eval { T_CVREF_REFCOUNT_FIXED( \@array ) };
ok( $@ );

# output only
SKIP:{
   my $cvr;
   is_deeply([ T_CVREF_REFCOUNT_FIXED_output($cvr) ], [ ], "call with non-ref lvalue, no return value");
   ok(ref $cvr, "output parameter now a reference")
     or skip "Not a reference", 1;
   is($cvr, \&XSLoader::load, "ref to expected sub");
}

# T_SYSRET - system return values
note("T_SYSRET");
# first check success
ok( T_SYSRET_pass );
# ... now failure
is( T_SYSRET_fail, undef);

# T_UV - unsigned integer
note("T_UV");
is( T_UV(5), 5 );    # pass
isnt( T_UV(-4), -4); # fail

# T_U_INT - unsigned integer with (unsigned int) cast
note("T_U_INT");
is( T_U_INT(5), 5 );    # pass
isnt( T_U_INT(-4), -4); # fail

# T_IV - signed integer
# T_INT - signed integer with cast
# T_LONG - signed integer with cast to IV
# T_SHORT - signed short
for my $t (['T_IV', \&T_IV],
           ['T_INT', \&T_INT],
           ['T_LONG', \&T_LONG],
           ['T_SHORT', \&T_SHORT])
{
  note($t->[0]);
  is( $t->[1]->(5), 5);
  is( $t->[1]->(-4), -4);
  is( $t->[1]->(4.1), int(4.1));
  is( $t->[1]->("52"), "52");
  isnt( $t->[1]->(4.5), 4.5); # failure
}

if ($Config{shortsize} == 2) {
  isnt( T_SHORT(32801), 32801 );
}
else {
  pass(); # e.g. Crays have shortsize 4 (T3X) or 8 (CXX and SVX)
}

# T_ENUM - enum list
ok( T_ENUM(), 'T_ENUM' ); # just hope for a true value

# T_BOOL - boolean
note("T_BOOL");

ok( T_BOOL(52) );
ok( ! T_BOOL(0) );
ok( ! T_BOOL('') );
ok( ! T_BOOL(undef) );

{
  # these attempt to modify a read-only value
  ok( !eval { T_BOOL_2(52); 1 } );
  ok( !eval { T_BOOL_2(0); 1 } );
  ok( !eval { T_BOOL_2(''); 1 } );
  ok( !eval { T_BOOL_2(undef); 1 } );
}

{
    my ($in, $out);
    $in = 1;
    T_BOOL_OUT($out, $in);
    ok($out, "T_BOOL_OUT, true in");
    $in = 0;
    $out = 1;
    T_BOOL_OUT($out, $in);
    ok(!$out, "T_BOOL_OUT, false in");
}

# T_U_SHORT aka U16
note("T_U_SHORT");
is( T_U_SHORT(32000), 32000);
if ($Config{shortsize} == 2) {
  isnt( T_U_SHORT(65536), 65536); # probably dont want to test edge cases
} else {
  ok(1); # e.g. Crays have shortsize 4 (T3X) or 8 (CXX and SVX)
}

# T_U_LONG aka U32
note("T_U_LONG");
is( T_U_LONG(65536), 65536);
isnt( T_U_LONG(-1), -1);

# T_CHAR
note("T_CHAR");
is( T_CHAR("a"), "a");
is( T_CHAR("-"), "-");
is( T_CHAR(chr(128)),chr(128));
isnt( T_CHAR(chr(256)), chr(256));

# T_U_CHAR
note("T_U_CHAR");
is( T_U_CHAR(127), 127);
is( T_U_CHAR(128), 128);
isnt( T_U_CHAR(-1), -1);
isnt( T_U_CHAR(300), 300);

# T_FLOAT
# limited precision
is( sprintf("%6.3f",T_FLOAT(52.345)), sprintf("%6.3f",52.345), "T_FLOAT");

# T_NV
is( T_NV(52.345), 52.345, "T_NV" );

# T_DOUBLE
is( sprintf("%6.3f",T_DOUBLE(52.345)), sprintf("%6.3f",52.345), "T_DOUBLE" );

# T_PV
note("T_PV");
is( T_PV("a string"), "a string");
is( T_PV(52), 52);
ok !defined T_PV_null, 'RETVAL = NULL returns undef for char*';
{
    use warnings NONFATAL => 'all';
    my $uninit;
    local $SIG{__WARN__} = sub { ++$uninit if shift =~ /uninit/ };
    () = ''.T_PV_null;
    is $uninit, 1, 'uninit warning from NULL returned from char* func';
}

# T_PTR
my $t = 5;
my $ptr = T_PTR_OUT($t);
is( T_PTR_IN( $ptr ), $t, "T_PTR" );

# T_PTRREF
note("T_PTRREF");
$t = -52;
$ptr = T_PTRREF_OUT( $t );
is( ref($ptr), "SCALAR");
is( T_PTRREF_IN( $ptr ), $t );

# test that a non-scalar ref is rejected
eval { T_PTRREF_IN( $t ); };
ok( $@ );

# T_PTROBJ
note("T_PTROBJ");
$t = 256;
$ptr = T_PTROBJ_OUT( $t );
is( ref($ptr), "intObjPtr");
is( $ptr->T_PTROBJ_IN, $t );

# check that normal scalar refs fail
eval {intObjPtr::T_PTROBJ_IN( \$t );};
ok( $@ );

# check that inheritance works
bless $ptr, "intObjPtr::SubClass";
is( ref($ptr), "intObjPtr::SubClass");
is( $ptr->T_PTROBJ_IN, $t );

# Skip T_REF_IV_REF

# T_REF_IV_PTR
note("T_REF_IV_PTR");
$t = -365;
$ptr = T_REF_IV_PTR_OUT( $t );
is( ref($ptr), "intRefIvPtr");
is( $ptr->T_REF_IV_PTR_IN(), $t);

# inheritance should not work
bless $ptr, "intRefIvPtr::SubClass";
eval { $ptr->T_REF_IV_PTR_IN };
ok( $@ );

# Skip T_PTRDESC

# Skip T_REFREF

# Skip T_REFOBJ

# T_OPAQUEPTR
note("T_OPAQUEPTR");
$t = 22;
my $p = T_OPAQUEPTR_IN( $t );
is( T_OPAQUEPTR_OUT($p), $t);

# T_OPAQUEPTR with a struct
note("T_OPAQUEPTR with a struct");
my @test = (5,6,7);
$p = T_OPAQUEPTR_IN_struct(@test);
my @result = T_OPAQUEPTR_OUT_struct($p);
is(scalar(@result),scalar(@test));
for (0..$#test) {
  is($result[$_], $test[$_]);
}

# T_OPAQUE
note("T_OPAQUE");
$t = 48;
$p = T_OPAQUE_IN( $t );
is(T_OPAQUEPTR_OUT_short( $p ), $t); # Test using T_OPAQUEPTR
is(T_OPAQUE_OUT( $p ), $t );         # Test using T_OPQAQUE

# T_OPAQUE_array
note("T_OPAQUE: A packed array");

my @opq = (2,4,8);
my $packed = T_OPAQUE_array(@opq);
my @uopq = unpack("i*",$packed);
is(scalar(@uopq), scalar(@opq));
for (0..$#opq) {
  is( $uopq[$_], $opq[$_]);
}

# T_PACKED
note("T_PACKED");
my $struct = T_PACKED_out(-4, 3, 2.1);
ok(ref($struct) eq 'HASH');
is_approx($struct->{a}, -4);
is_approx($struct->{b}, 3);
is_approx($struct->{c}, 2.1);
my @rv = T_PACKED_in($struct);
is(scalar(@rv), 3);
is_approx($rv[0], -4);
is_approx($rv[1], 3);
is_approx($rv[2], 2.1);

# T_PACKEDARRAY
SCOPE: {
  note("T_PACKED_ARRAY");
  my @d = (
    -4, 3, 2.1,
    2, 1, -15.3,
    1,1,1
  );
  my @out;
  push @out, {a => $d[$_*3], b => $d[$_*3+1], c => $d[$_*3+2]} for (0..2);
  my $structs = T_PACKEDARRAY_out(@d);
  ok(ref($structs) eq 'ARRAY');
  is(scalar(@$structs), 3);
  foreach my $i (0..2) {
    my $s = $structs->[$i];
    is(ref($s), 'HASH');
    is_approx($s->{a}, $d[$i*3+0]);
    is_approx($s->{b}, $d[$i*3+1]);
    is_approx($s->{c}, $d[$i*3+2]);
  }
  my @rv = T_PACKEDARRAY_in($structs);
  is(scalar(@rv), scalar(@d));
  foreach my $i (0..$#d) {
    is_approx($rv[$i], $d[$i]);
  }
}

# Skip T_DATAUNIT

# Skip T_CALLBACK

# T_ARRAY
my @inarr = (1,2,3,4,5,6,7,8,9,10);
my @outarr = T_ARRAY( 5, @inarr );
is_deeply(\@outarr, \@inarr, "T_ARRAY");

# T_STDIO
note("T_STDIO");

# open a file in XS for write
my $testfile= "stdio.tmp";
# not everything below cleans up
END { 1 while unlink $testfile; }
my $fh = T_STDIO_open( $testfile );
ok( $fh );

# write to it using perl
if (defined $fh) {

  my @lines = ("NormalSTDIO\n", "PerlIO\n");

  # print to it using FILE* through XS
  is( T_STDIO_print($fh, $lines[0]), length($lines[0]));

  # print to it using normal perl
  ok(print $fh "$lines[1]");

  # close it using XS if using perlio, using Perl otherwise
  ok( $Config{useperlio} ? T_STDIO_close( $fh ) : close( $fh ) );

  # open from perl, and check contents
  open($fh, '<', $testfile);
  ok($fh);
  my $line = <$fh>;
  is($line,$lines[0]);
  $line = <$fh>;
  is($line,$lines[1]);

  ok(close($fh));
  ok(unlink($testfile));

} else {
  for (1..8) {
    skip("Skip Test not relevant since file was not opened correctly",0);
  }
}

$fh = "FOO";
T_STDIO_open_ret_in_arg( $testfile, $fh);
ok( $fh ne "FOO", 'return io in arg open succeeds');
ok( print($fh "first line\n"), 'can print to return io in arg');
ok( close($fh), 'can close return io in arg');
$fh = "FOO";
#now with a bad file name to make sure $fh is written to on failure
my $badfile = $^O eq 'VMS' ? '?' : '';
T_STDIO_open_ret_in_arg( $badfile, $fh);
ok( !defined$fh, 'return io in arg open failed successfully');

# T_INOUT
note("T_INOUT");
SCOPE: {
  my $buf = '';
  local $| = 1;
  open my $fh, "+<", \$buf or die $!;
  my $str = "Fooo!\n";
  print $fh $str;
  my $fh2 = T_INOUT($fh);
  seek($fh2, 0, 0);
  is(readline($fh2), $str);
  ok(print $fh2 "foo\n");
  ok(close $fh);
  # this fails because the underlying shared handle is already closed
  ok(!close $fh2);
}

# T_IN
note("T_IN");
SCOPE: {
  my $buf = "Hello!\n";
  local $| = 1;
  open my $fh, "<", \$buf or die $!;
  my $fh2 = T_IN($fh);
  is(readline($fh2), $buf);
  local $SIG{__WARN__} = sub {die};
  ok(not(eval {print $fh2 "foo\n"; 1}));
}

# T_OUT
note("T_OUT");
SCOPE: {
  my $buf = '';
  local $| = 1;
  open my $fh, "+<", \$buf or die $!;
  my $str = "Fooo!\n";
  print $fh $str;
  my $fh2 = T_OUT($fh);
  seek($fh2, 0, 0);
  is(readline($fh2), $str);
  ok(eval {print $fh2 "foo\n"; 1});
  ok(close $fh);
  # this fails because the underlying shared handle is already closed
  ok(!close $fh2);
}

# Perl RT #124181 SEGV due to double free in typemap
# "Attempt to free unreferenced scalar"
%{*{main::XS::}{HASH}} = ();

sub is_approx {
  my ($l, $r, $n) = @_;
  if (not defined $l or not defined $r) {
    fail(defined($n) ? $n : ());
  }
  else {
    ok($l < $r+1e-6 && $r < $l+1e-6, defined($n) ? $n : ())
      or note("$l and $r seem to be different given a fuzz of 1e-6");
  }
}
