#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
    require 'test.pl';		# we use runperl from 'test.pl', so can't use Test::More
}

plan tests => 167;

require_ok("B::Concise");

$out = runperl(switches => ["-MO=Concise"], prog => '$a', stderr => 1);

# If either of the next two tests fail, it probably means you need to
# fix the section labeled 'fragile kludge' in Concise.pm

($op_base) = ($out =~ /^(\d+)\s*<0>\s*enter/m);

is($op_base, 1, "Smallest OP sequence number");

($op_base_p1, $cop_base)
  = ($out =~ /^(\d+)\s*<;>\s*nextstate\(main (-?\d+) /m);

is($op_base_p1, 2, "Second-smallest OP sequence number");

is($cop_base, 1, "Smallest COP sequence number");

# test that with -exec B::Concise navigates past logops (bug #18175)

$out = runperl(
    switches => ["-MO=Concise,-exec"],
    prog => q{$a=$b && print q/foo/},
    stderr => 1,
);
#diag($out);
like($out, qr/print/, "'-exec' option output has print opcode");

######## API tests v.60 

B::Concise->import(qw( set_style set_style_standard add_callback 
		       add_style walk_output reset_sequence ));

## walk_output argument checking

# test that walk_output rejects non-HANDLE args
foreach my $foo ("string", [], {}) {
    eval {  walk_output($foo) };
    isnt ($@, '', "walk_output() rejects arg '$foo'");
    $@=''; # clear the fail for next test
}
# test accessor mode when arg undefd or 0
foreach my $foo (undef, 0) {
    my $handle = walk_output($foo);
    is ($handle, \*STDOUT, "walk_output set to STDOUT (default)");
}

{   # any object that can print should be ok for walk_output
    package Hugo;
    sub new { my $foo = bless {} };
    sub print { CORE::print @_ }
}
my $foo = new Hugo;	# suggested this API fix
eval {  walk_output($foo) };
is ($@, '', "walk_output() accepts obj that can print");

# test that walk_output accepts a HANDLE arg
foreach my $foo (\*STDOUT, \*STDERR) {
    eval {  walk_output($foo) };
    is ($@, '', "walk_output() accepts STD* " . ref $foo);
}

# now test a ref to scalar
eval {  walk_output(\my $junk) };
is ($@, '', "walk_output() accepts ref-to-sprintf target");

$junk = "non-empty";
eval {  walk_output(\$junk) };
is ($@, '', "walk_output() accepts ref-to-non-empty-scalar");

## add_style
my @stylespec;
$@='';
eval { add_style ('junk_B' => @stylespec) };
like ($@, qr/expecting 3 style-format args/,
    "add_style rejects insufficient args");

@stylespec = (0,0,0); # right length, invalid values
$@='';
eval { add_style ('junk' => @stylespec) };
is ($@, '', "add_style accepts: stylename => 3-arg-array");

$@='';
eval { add_style (junk => @stylespec) };
like ($@, qr/style 'junk' already exists, choose a new name/,
    "add_style correctly disallows re-adding same style-name" );

# test new arg-checks on set_style
$@='';
eval { set_style (@stylespec) };
is ($@, '', "set_style accepts 3 style-format args");

@stylespec = (); # bad style

eval { set_style (@stylespec) };
like ($@, qr/expecting 3 style-format args/,
      "set_style rejects bad style-format args");

#### for content with doc'd options

our($a, $b);
my $func = sub{ $a = $b+42 };	# canonical example asub

sub render {
    walk_output(\my $out);
    eval { B::Concise::compile(@_)->() };
    # diag "rendering $@\n";
    return ($out, $@) if wantarray;
    return $out;
}

# tests output to GLOB, using perlio feature directly
set_style_standard('concise');  # MUST CALL before output needed

@options = qw(
                 -basic -exec -tree -compact -loose -vt -ascii
                 -base10 -bigendian -littleendian
         );
foreach $opt (@options) {
    ($out) = render($opt, $func);
    isnt($out, '', "got output with option $opt");
}

## test output control via walk_output

my $treegen = B::Concise::compile('-basic', $func); # reused

{ # test output into a package global string (sprintf-ish)
    our $thing;
    walk_output(\$thing);
    $treegen->();
    ok($thing, "walk_output to our SCALAR, output seen");
}

# test walkoutput acceptance of a scalar-bound IO handle
open (my $fh, '>', \my $buf);
walk_output($fh);
$treegen->();
ok($buf, "walk_output to GLOB, output seen");

## test B::Concise::compile error checking

# call compile on non-CODE ref items
if (0) {
    # pending STASH splaying

    foreach my $ref ([], {}) {
        my $typ = ref $ref;
        walk_output(\my $out);
        eval { B::Concise::compile('-basic', $ref)->() };
        like ($@, qr/^err: not a coderef: $typ/,
              "compile detects $typ-ref where expecting subref");
        is($out,'', "no output when errd"); # announcement prints
    }
}

# test against a bogus autovivified subref.
# in debugger, it should look like:
#  1  CODE(0x84840cc)
#      -> &CODE(0x84840cc) in ???

my ($res,$err);
TODO: {
    #local $TODO = "\tdoes this handling make sense ?";

    sub declared_only;
    ($res,$err) = render('-basic', \&declared_only);
    like ($res, qr/coderef CODE\(0x[0-9a-fA-F]+\) has no START/,
          "'sub decl_only' seen as having no START");

    sub defd_empty {};
    ($res,$err) = render('-basic', \&defd_empty);
    my @lines = split(/\n/, $res);
    is(scalar @lines, 3,
       "'sub defd_empty {}' seen as 3 liner");

    is(1, $res =~ /leavesub/ && $res =~ /(next|db)state/,
       "'sub defd_empty {}' seen as 2 ops: leavesub,nextstate");

    ($res,$err) = render('-basic', \&not_even_declared);
    like ($res, qr/coderef CODE\(0x[0-9a-fA-F]+\) has no START/,
          "'\&not_even_declared' seen as having no START");

    {
        package Bar;
        our $AUTOLOAD = 'garbage';
        sub AUTOLOAD { print "# in AUTOLOAD body: $AUTOLOAD\n" }
    }
    ($res,$err) = render('-basic', Bar::auto_func);
    like ($res, qr/unknown function \(Bar::auto_func\)/,
          "Bar::auto_func seen as unknown function");

    ($res,$err) = render('-basic', \&Bar::auto_func);
    like ($res, qr/coderef CODE\(0x[0-9a-fA-F]+\) has no START/,
          "'\&Bar::auto_func' seen as having no START");

    ($res,$err) = render('-basic', \&Bar::AUTOLOAD);
    like ($res, qr/in AUTOLOAD body: /, "found body of Bar::AUTOLOAD");

}
($res,$err) = render('-basic', Foo::bar);
like ($res, qr/unknown function \(Foo::bar\)/,
      "BC::compile detects fn-name as unknown function");

# v.62 tests

pass ("TEST POST-COMPILE OPTION-HANDLING IN WALKER SUBROUTINE");

my $sample;

my $walker = B::Concise::compile('-basic', $func);
walk_output(\$sample);
$walker->('-exec');
like($sample, qr/goto/m, "post-compile -exec");

walk_output(\$sample);
$walker->('-basic');
unlike($sample, qr/goto/m, "post-compile -basic");


# bang at it combinatorically
my %combos;
my @modes = qw( -basic -exec );
my @styles = qw( -concise -debug -linenoise -terse );

# prep samples
for $style (@styles) {
    for $mode (@modes) {
        walk_output(\$sample);
        reset_sequence();
        $walker->($style, $mode);
        $combos{"$style$mode"} = $sample;
    }
}
# crosscheck that samples are all text-different
@list = sort keys %combos;
for $i (0..$#list) {
    for $j ($i+1..$#list) {
        isnt ($combos{$list[$i]}, $combos{$list[$j]},
              "combos for $list[$i] and $list[$j] are different, as expected");
    }
}

# add samples with styles in different order
for $mode (@modes) {
    for $style (@styles) {
        reset_sequence();
        walk_output(\$sample);
        $walker->($mode, $style);
        $combos{"$mode$style"} = $sample;
    }
}
# test commutativity of flags, ie that AB == BA
for $mode (@modes) {
    for $style (@styles) {
        is ( $combos{"$style$mode"},
             $combos{"$mode$style"},
             "results for $style$mode vs $mode$style are the same" );
    }
}

my %save = %combos;
%combos = ();	# outputs for $mode=any($order) and any($style)

# add more samples with switching modes & sticky styles
for $style (@styles) {
    walk_output(\$sample);
    reset_sequence();
    $walker->($style);
    for $mode (@modes) {
        walk_output(\$sample);
        reset_sequence();
        $walker->($mode);
        $combos{"$style/$mode"} = $sample;
    }
}
# crosscheck that samples are all text-different
@nm = sort keys %combos;
for $i (0..$#nm) {
    for $j ($i+1..$#nm) {
        isnt ($combos{$nm[$i]}, $combos{$nm[$j]},
              "results for $nm[$i] and $nm[$j] are different, as expected");
    }
}

# add samples with switching styles & sticky modes
for $mode (@modes) {
    walk_output(\$sample);
    reset_sequence();
    $walker->($mode);
    for $style (@styles) {
        walk_output(\$sample);
        reset_sequence();
        $walker->($style);
        $combos{"$mode/$style"} = $sample;
    }
}
# test commutativity of flags, ie that AB == BA
for $mode (@modes) {
    for $style (@styles) {
        is ( $combos{"$style/$mode"},
             $combos{"$mode/$style"},
             "results for $style/$mode vs $mode/$style are the same" );
    }
}


#now do double crosschecks: commutativity across stick / nostick
%combos = (%combos, %save);

# test commutativity of flags, ie that AB == BA
for $mode (@modes) {
    for $style (@styles) {

        is ( $combos{"$style$mode"},
             $combos{"$style/$mode"},
             "$style$mode VS $style/$mode are the same" );

        is ( $combos{"$mode$style"},
             $combos{"$mode/$style"},
             "$mode$style VS $mode/$style are the same" );

        is ( $combos{"$style$mode"},
             $combos{"$mode/$style"},
             "$style$mode VS $mode/$style are the same" );

        is ( $combos{"$mode$style"},
             $combos{"$style/$mode"},
             "$mode$style VS $style/$mode are the same" );
    }
}


# test proper NULLING of pointer, derefd by CvSTART, when a coderef is
# undefd.  W/o this, the pointer can dangle into freed and reused
# optree mem, which no longer points to opcodes.

# Using B::Concise to render Config::AUTOLOAD's optree at BEGIN-time
# triggers this obscure bug, cuz AUTOLOAD has a bootstrap version,
# which is used at load-time then undeffed.  It is normally
# re-vivified later, but not in time for this (BEGIN/CHECK)-time
# rendering.

$out = runperl ( switches => ["-MO=Concise,Config::AUTOLOAD"],
		 prog => 'use Config; BEGIN { $Config{awk} }',
		 stderr => 1 );

like($out, qr/Config::AUTOLOAD exists in stash, but has no START/,
    "coderef properly undefined");

$out = runperl ( switches => ["-MO=Concise,Config::AUTOLOAD"],
		 prog => 'use Config; CHECK { $Config{awk} }',
		 stderr => 1 );

like($out, qr/Config::AUTOLOAD exists in stash, but has no START/,
    "coderef properly undefined");

# test -stash and -src rendering
$out = runperl ( switches => ["-MO=-qq,Concise,-stash=B::Concise,-src"],
		 prog => '-e 1', stderr => 1 );

like($out, qr/FUNC: \*B::Concise::concise_cv_obj/,
     "stash rendering of B::Concise includes Concise::concise_cv_obj");

like($out, qr/FUNC: \*B::Concise::walk_output/,
     "stash rendering includes Concise::walk_output");

like($out, qr/\# 4\d\d: \s+ \$l->concise\(\$level\);/,
     "src-line rendering works");

$out = runperl ( switches => ["-MStorable", "-MO=Concise,-stash=Storable,-src"],
		 prog => '-e 1', stderr => 1 );

like($out, qr/FUNC: \*Storable::BIN_MAJOR/,
     "stash rendering has constant sub: Storable::BIN_MAJOR");

like($out, qr/BIN_MAJOR is a constant sub, optimized to a IV/,
     "stash rendering identifies it as constant");

$out = runperl ( switches => ["-MO=Concise,-stash=ExtUtils::Mksymlists,-src,-exec"],
		 prog => '-e 1', stderr => 1 );

like($out, qr/FUNC: \*ExtUtils::Mksymlists::_write_vms/,
     "stash rendering loads package as needed");

$out = runperl ( switches => ["-MO=Concise,-stash=Data::Dumper,-src,-exec"],
		 prog => '-e 1', stderr => 1 );

SKIP: {
    skip "Data::Dumper is statically linked", 1
	if $Config::Config{static_ext} =~ m|\bData/Dumper\b|;
    like($out, qr/FUNC: \*Data::Dumper::format_refaddr/,
	"stash rendering loads package as needed");
}

my $prog = q{package FOO; sub bar { print q{bar} } package main; FOO::bar(); };

# this would fail if %INC used for -stash test
$out = runperl ( switches => ["-MO=Concise,-src,-stash=FOO,-main"],
		 prog => $prog, stderr => 1 );

like($out, qr/FUNC: \*FOO::bar/,
     "stash rendering works on inlined package");

# Test that consecutive nextstate ops are not nulled out when PERLDBf_NOOPT
# is set.
# XXX Does this test belong here?

$out = runperl ( switches => ["-MO=Concise"],
		 prog => 'BEGIN{$^P = 0x04} 1 if 0; print',
		 stderr => 1 );
like $out, qr/nextstate.*nextstate/s,
  'nulling of nextstate-nextstate happeneth not when $^P | PERLDBf_NOOPT';


# A very basic test for -tree output
$out =
 runperl(
  switches => ["-MO=Concise,-tree"], prog => 'print', stderr => 1
 );
ok index $out=~s/\r\n/\n/gr=~s/gvsv\(\*_\)/gvsv[*_]/r, <<'end'=~s/\r\n/\n/gr =>>= 0, '-tree output';
<6>leave[1 ref]-+-<1>enter
                |-<2>nextstate(main 1 -e:1)
                `-<5>print-+-<3>pushmark
                           `-ex-rv2sv---<4>gvsv[*_]
end

# -nobanner
$out =
 runperl(
  switches => ["-MO=Concise,-nobanner,foo"], prog=>'sub foo{}', stderr => 1
 );
unlike $out, qr/main::foo/, '-nobanner';

# glob
$out =
 runperl(
  switches => ["-MO=Concise"], prog=>'glob(q{.})', stderr => 1
 );
like $out, qr/\*<none>::/, 'glob(q{.})';

# Test op_other in -debug
$out = runperl(
    switches => ["-MO=Concise,-debug,xx"],
    prog => q{sub xx { if ($a) { return $b } }},
    stderr => 1,
);

$out =~s/\r\n/\n/g;

# Look for OP_AND
$end = <<'EOF';
LOGOP \(0x\w+\)
	op_next		0x\w+
	op_other	(0x\w+)
	op_sibling	0
	op_ppaddr	PL_ppaddr\[OP_AND\]
EOF

$end =~ s/\r\n/\n/g;

like $out, qr/$end/, 'OP_AND has op_other';

# like(..) above doesn't fill in $1
$out =~ $end;
my $next = $1;

# Check it points to a PUSHMARK
$end = <<'EOF';
OP \(<NEXT>\)
	op_next		0x\w+
	op_sibling	0x\w+
	op_ppaddr	PL_ppaddr\[OP_PUSHMARK\]
EOF

$end =~ s/<NEXT>/$next/;

like $out, qr/$end/, 'OP_AND->op_other points correctly';

# test nextstate hints display

{

    $out = runperl(
        switches => ["-MO=Concise"],
        prog => q{my $x; use strict; use warnings; $x++; use feature q(:5.11); $x++},
        stderr => 1,
    );

    my @hints = $out =~ /nextstate\([^)]+\) (.*) ->/g;

    # handle test script run with PERL_UNICODE=""
    s/>,<,// for @hints;
    s/%,// for @hints;

    is(scalar(@hints), 3, "3 hints");
    is($hints[0], 'v:{',                           "hints[0]");
    is($hints[1], 'v:*,&,{,x*,x&,x$,$',            "hints[1]");
    is($hints[2], 'v:us,*,&,{,x*,x&,x$,$,fea=15',  "hints[2]");
}

__END__
