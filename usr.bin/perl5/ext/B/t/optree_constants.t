#!perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use OptreeCheck;	# ALSO DOES @ARGV HANDLING !!!!!!

plan tests => 99;

#################################

my sub lleexx {}
sub tsub0 {}
sub tsub1 {} $tsub1 = 1;
sub t::tsub2 {}
sub t::tsub3 {} $tsub3 = 1;
{
    package t;
    sub tsub4 {}
    sub tsub5 {} $tsub5 = 1;
}

use constant {		# see also t/op/gv.t line 358
    myaref	=> [ 1,2,3 ],
    myfl	=> 1.414213,
    myglob	=> \*STDIN,
    myhref	=> { a	=> 1 },
    myint	=> 42,
    myrex	=> qr/foo/,
    mystr	=> 'hithere',
    mysub	=> \&ok,
    myundef	=> undef,
    myunsub	=> \&nosuch,
    myanonsub	=> sub {},
    mylexsub	=> \&lleexx,
    tsub0	=> \&tsub0,
    tsub1	=> \&tsub1,
    tsub2	=> \&t::tsub2,
    tsub3	=> \&t::tsub3,
    tsub4	=> \&t::tsub4,
    tsub5	=> \&t::tsub5,
};

sub myyes() { 1==1 }
sub myno () { return 1!=1 }
sub pi () { 3.14159 };

my $want = {	# expected types, how value renders in-line, todos (maybe)
    mystr	=> [ 'PV', '"'.mystr.'"' ],
    myhref	=> [ 'IV', '\\\\HASH'],
    pi		=> [ 'NV', pi ],
    myglob	=> [ 'IV', '\\\\' ],
    mysub	=> [ 'IV', '\\\\&main::ok' ],
    myunsub	=> [ 'IV', '\\\\&main::nosuch' ],
    myanonsub	=> [ 'IV', '\\\\CODE' ],
    mylexsub	=> [ 'IV', '\\\\&lleexx' ],
    tsub0	=> [ 'IV', '\\\\&main::tsub0' ],
    tsub1	=> [ 'IV', '\\\\&main::tsub1' ],
    tsub2	=> [ 'IV', '\\\\&t::tsub2' ],
    tsub3	=> [ 'IV', '\\\\&t::tsub3' ],
    tsub4	=> [ 'IV', '\\\\&t::tsub4' ],
    tsub5	=> [ 'IV', '\\\\&t::tsub5' ],
    # these are not inlined, at least not per BC::Concise
    #myyes	=> [ 'IV', ],
    #myno	=> [ 'IV', ],
    myaref	=> [ 'IV', '\\\\ARRAY' ],
    myfl	=> [ 'NV', myfl ],
    myint	=> [ 'IV', myint ],
    myrex	=> [ 'IV', '\\\\"\\(?^:Foo\\)"' ],
    myundef	=> [ 'NULL', ],
};

use constant WEEKDAYS
    => qw ( Sunday Monday Tuesday Wednesday Thursday Friday Saturday );


$::{napier} = \2.71828;	# counter-example (doesn't get optimized).
eval "sub napier ();";


# should be able to undefine constant::import here ???
INIT { 
    # eval 'sub constant::import () {}';
    # undef *constant::import::{CODE};
};

#################################
pass("RENDER CONSTANT SUBS RETURNING SCALARS");

for $func (sort keys %$want) {
    # no strict 'refs';	# why not needed ?
    checkOptree ( name      => "$func() as a coderef",
		  code      => \&{$func},
		  noanchors => 1,
		  expect    => <<EOT_EOT, expect_nt => <<EONT_EONT);
 is a constant sub, optimized to a $want->{$func}[0]
EOT_EOT
 is a constant sub, optimized to a $want->{$func}[0]
EONT_EONT

}

pass("RENDER CALLS TO THOSE CONSTANT SUBS");

for $func (sort keys %$want) {
    # print "# doing $func\n";
    checkOptree ( name    => "call $func",
		  code    => "$func",
		  ($want->{$func}[2]) ? ( todo => $want->{$func}[2]) : (),
		  bc_opts => '-nobanner',
		  expect  => <<EOT_EOT, expect_nt => <<EONT_EONT);
3  <1> leavesub[2 refs] K/REFC,1 ->(end)
-     <\@> lineseq KP ->3
1        <;> dbstate(main 833 (eval 44):1) v ->2
2        <\$> const[$want->{$func}[0] $want->{$func}[1]] s*/FOLD ->3
EOT_EOT
3  <1> leavesub[2 refs] K/REFC,1 ->(end)
-     <\@> lineseq KP ->3
1        <;> dbstate(main 833 (eval 44):1) v ->2
2        <\$> const($want->{$func}[0] $want->{$func}[1]) s*/FOLD ->3
EONT_EONT

}

##############
pass("MORE TESTS");

checkOptree ( name	=> 'myyes() as coderef',
	      code	=> sub () { 1==1 },
	      noanchors => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
 is a constant sub, optimized to a SPECIAL
EOT_EOT
 is a constant sub, optimized to a SPECIAL
EONT_EONT


checkOptree ( name	=> 'myyes() as coderef',
	      prog	=> 'sub a() { 1==1 }; print a',
	      noanchors => 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const[SPECIAL sv_yes] s*/FOLD ->5
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const(SPECIAL sv_yes) s*/FOLD ->5
EONT_EONT


# Need to do this as a prog, not code, as only the first constant to use
# PL_sv_no actually gets to use the real thing - every one following is
# copied.
checkOptree ( name	=> 'myno() as coderef',
	      prog	=> 'sub a() { 1!=1 }; print a',
	      noanchors => 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const[SPECIAL sv_no] s*/FOLD ->5
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 2 -e:1) v:>,<,%,{ ->3
# 5     <@> print vK ->6
# 3        <0> pushmark s ->4
# 4        <$> const(SPECIAL sv_no) s*/FOLD ->5
EONT_EONT


my ($expect, $expect_nt) = (" is a constant sub, optimized to a AV\n") x 2;


checkOptree ( name	=> 'constant sub returning list',
	      code	=> \&WEEKDAYS,
	      noanchors => 1,
	      expect => $expect, expect_nt => $expect_nt);


sub printem {
    printf "myint %d mystr %s myfl %f pi %f\n"
	, myint, mystr, myfl, pi;
}

my ($expect, $expect_nt) = (<<'EOT_EOT', <<'EONT_EONT');
# 9  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->9
# 1        <;> nextstate(main 635 optree_constants.t:163) v:>,<,% ->2
# 8        <@> prtf sK ->9
# 2           <0> pushmark sM ->3
# 3           <$> const[PV "myint %d mystr %s myfl %f pi %f\n"] sM/FOLD ->4
# 4           <$> const[IV 42] sM*/FOLD ->5
# 5           <$> const[PV "hithere"] sM*/FOLD ->6
# 6           <$> const[NV 1.414213] sM*/FOLD ->7
# 7           <$> const[NV 3.14159] sM*/FOLD ->8
EOT_EOT
# 9  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->9
# 1        <;> nextstate(main 635 optree_constants.t:163) v:>,<,% ->2
# 8        <@> prtf sK ->9
# 2           <0> pushmark sM ->3
# 3           <$> const(PV "myint %d mystr %s myfl %f pi %f\n") sM/FOLD ->4
# 4           <$> const(IV 42) sM*/FOLD ->5
# 5           <$> const(PV "hithere") sM*/FOLD ->6
# 6           <$> const(NV 1.414213) sM*/FOLD ->7
# 7           <$> const(NV 3.14159) sM*/FOLD ->8
EONT_EONT

s|\\n"[])] sM\K/FOLD|| for $expect, $expect_nt;

checkOptree ( name	=> 'call many in a print statement',
	      code	=> \&printem,
	      strip_open_hints => 1,
	      expect => $expect, expect_nt => $expect_nt);

# test constant expression folding

checkOptree ( name	=> 'arithmetic constant folding in print',
	      code	=> 'print 1+2+3',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 937 (eval 53):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[IV 6] s/FOLD ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 937 (eval 53):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(IV 6) s/FOLD ->4
EONT_EONT

checkOptree ( name	=> 'string constant folding in print',
	      code	=> 'print "foo"."bar"',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar"] s/FOLD ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar") s/FOLD ->4
EONT_EONT

checkOptree ( name	=> 'boolean or folding',
	      code	=> 'print "foobar" if 1 or 0',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK/FOLD ->5
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar"] s ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 942 (eval 55):1) v ->2
# 4        <@> print sK/FOLD ->5
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar") s ->4
EONT_EONT

checkOptree ( name	=> 'lc*,uc*,gt,lt,ge,le,cmp',
	      code	=> sub {
		  $s = uc('foo.').ucfirst('bar.').lc('LOW.').lcfirst('LOW');
		  print "a-lt-b" if "a" lt "b";
		  print "b-gt-a" if "b" gt "a";
		  print "a-le-b" if "a" le "b";
		  print "b-ge-a" if "b" ge "a";
		  print "b-cmp-a" if "b" cmp "a";
		  print "a-gt-b" if "a" gt "b";	# should be suppressed
	      },
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# 1        <;> nextstate(main 916 optree_constants.t:307) v:>,<,%,{ ->2
# 4        <2> sassign vKS/2 ->5
# 2           <$> const[PV "FOO.Bar.low.lOW"] s/FOLD ->3
# -           <1> ex-rv2sv sKRM*/1 ->4
# 3              <#> gvsv[*s] s ->4
# 5        <;> nextstate(main 916 optree_constants.t:308) v:>,<,%,{ ->6
# 8        <@> print vK/FOLD ->9
# 6           <0> pushmark s ->7
# 7           <$> const[PV "a-lt-b"] s ->8
# 9        <;> nextstate(main 916 optree_constants.t:309) v:>,<,%,{ ->a
# c        <@> print vK/FOLD ->d
# a           <0> pushmark s ->b
# b           <$> const[PV "b-gt-a"] s ->c
# d        <;> nextstate(main 916 optree_constants.t:310) v:>,<,%,{ ->e
# g        <@> print vK/FOLD ->h
# e           <0> pushmark s ->f
# f           <$> const[PV "a-le-b"] s ->g
# h        <;> nextstate(main 916 optree_constants.t:311) v:>,<,%,{ ->i
# k        <@> print vK/FOLD ->l
# i           <0> pushmark s ->j
# j           <$> const[PV "b-ge-a"] s ->k
# l        <;> nextstate(main 916 optree_constants.t:312) v:>,<,%,{ ->m
# o        <@> print vK/FOLD ->p
# m           <0> pushmark s ->n
# n           <$> const[PV "b-cmp-a"] s ->o
# p        <;> nextstate(main 916 optree_constants.t:313) v:>,<,%,{ ->q
# q        <$> const[SPECIAL sv_no] s/SHORT,FOLD ->r
EOT_EOT
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# 1        <;> nextstate(main 916 optree_constants.t:307) v:>,<,%,{ ->2
# 4        <2> sassign vKS/2 ->5
# 2           <$> const(PV "FOO.Bar.low.lOW") s/FOLD ->3
# -           <1> ex-rv2sv sKRM*/1 ->4
# 3              <$> gvsv(*s) s ->4
# 5        <;> nextstate(main 916 optree_constants.t:308) v:>,<,%,{ ->6
# 8        <@> print vK/FOLD ->9
# 6           <0> pushmark s ->7
# 7           <$> const(PV "a-lt-b") s ->8
# 9        <;> nextstate(main 916 optree_constants.t:309) v:>,<,%,{ ->a
# c        <@> print vK/FOLD ->d
# a           <0> pushmark s ->b
# b           <$> const(PV "b-gt-a") s ->c
# d        <;> nextstate(main 916 optree_constants.t:310) v:>,<,%,{ ->e
# g        <@> print vK/FOLD ->h
# e           <0> pushmark s ->f
# f           <$> const(PV "a-le-b") s ->g
# h        <;> nextstate(main 916 optree_constants.t:311) v:>,<,%,{ ->i
# k        <@> print vK/FOLD ->l
# i           <0> pushmark s ->j
# j           <$> const(PV "b-ge-a") s ->k
# l        <;> nextstate(main 916 optree_constants.t:312) v:>,<,%,{ ->m
# o        <@> print vK/FOLD ->p
# m           <0> pushmark s ->n
# n           <$> const(PV "b-cmp-a") s ->o
# p        <;> nextstate(main 916 optree_constants.t:313) v:>,<,%,{ ->q
# q        <$> const(SPECIAL sv_no) s/SHORT,FOLD ->r
EONT_EONT

checkOptree ( name	=> 'mixed constant folding, with explicit braces',
	      code	=> 'print "foo"."bar".(2+3)',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 977 (eval 28):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const[PV "foobar5"] s/FOLD ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 977 (eval 28):1) v ->2
# 4        <@> print sK ->5
# 2           <0> pushmark s ->3
# 3           <$> const(PV "foobar5") s/FOLD ->4
EONT_EONT

__END__

=head NB

Optimized constant subs are stored as bare scalars in the stash
(package hash), which formerly held only GVs (typeglobs).

But you cant create them manually - you cant assign a scalar to a
stash element, and expect it to work like a constant-sub, even if you
provide a prototype.

This is a feature; alternative is too much action-at-a-distance.  The
following test demonstrates - napier is not seen as a function at all,
much less an optimized one.

=cut

checkOptree ( name	=> 'not evertnapier',
	      code	=> \&napier,
	      noanchors => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
 has no START
EOT_EOT
 has no START
EONT_EONT


