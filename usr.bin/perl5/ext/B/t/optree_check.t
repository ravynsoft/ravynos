#!perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use OptreeCheck;

=head1 OptreeCheck selftest harness

This file is primarily to test services of OptreeCheck itself, ie
checkOptree().  %gOpts provides test-state info, it is 'exported' into
main::  

doing use OptreeCheck runs import(), which processes @ARGV to process
cmdline args in 'standard' way across all clients of OptreeCheck.

=cut

plan tests =>     11 # REGEX TEST HARNESS SELFTEST
		+  3 # TEST FATAL ERRS
		+ 11 # TEST -e \$srcCode
		+  5 # REFTEXT FIXUP TESTS
		+  5 # CANONICAL B::Concise EXAMPLE
		+ 16 * $gOpts{selftest}; # XXX I don't understand this - DAPM

pass("REGEX TEST HARNESS SELFTEST");

checkOptree ( name	=> "bare minimum opcode search",
	      bcopts	=> '-exec',
	      code	=> sub {my $a},
	      noanchors	=> 1, # unanchored match
	      expect	=> 'leavesub',
	      expect_nt	=> 'leavesub');

checkOptree ( name	=> "found print opcode",
	      bcopts	=> '-exec',
	      code	=> sub {print 1},
	      noanchors	=> 1, # unanchored match
	      expect	=> 'print',
	      expect_nt	=> 'leavesub');

checkOptree ( name	=> 'test skip itself',
	      skip	=> 'this is skip-reason',
	      bcopts	=> '-exec',
	      code	=> sub {print 1},
	      expect	=> 'dont-care, skipping',
	      expect_nt	=> 'this insures failure');

# This test 'unexpectedly succeeds', but that is "expected".  Theres
# no good way to expect a successful todo, and inducing a failure
# causes the harness to print verbose errors, which is NOT helpful.

checkOptree ( name	=> 'test todo itself',
	      todo	=> "your excuse here ;-)",
	      bcopts	=> '-exec',
	      code	=> sub {print 1},
	      noanchors	=> 1, # unanchored match
	      expect	=> 'print',
	      expect_nt	=> 'print') if 0;

checkOptree ( name	=> 'impossible match, remove skip to see failure',
	      todo	=> "see! it breaks!",
	      skip	=> 'skip the failure',
	      code	=> sub {print 1},
	      expect	=> 'look out ! Boy Wonder',
	      expect_nt	=> 'holy near earth asteroid Batman !');

pass ("TEST FATAL ERRS");

if (1) {
    # test for fatal errors. Im unsettled on fail vs die.
    # calling fail isnt good enough by itself.

    $@='';
    eval {
	checkOptree ( name	=> 'test against empty expectations',
		      bcopts	=> '-exec',
		      code	=> sub {print 1},
		      expect	=> '',
		      expect_nt	=> '');
    };
    like($@, qr/no '\w+' golden-sample found/, "empty expectations prevented");
    
    $@='';
    eval {
	checkOptree ( name	=> 'prevent whitespace only expectations',
		      bcopts	=> '-exec',
		      code	=> sub {my $a},
		      #skip	=> 1,
		      expect_nt	=> "\n",
		      expect	=> "\n");
    };
    like($@, qr/whitespace only reftext found for '\w+'/,
	 "just whitespace expectations prevented");
}
    
pass ("TEST -e \$srcCode");

checkOptree ( name	=> 'empty code or prog',
	      skip	=> 'or fails',
	      todo	=> "your excuse here ;-)",
	      code	=> '',
	      prog	=> '',
	      );
    
checkOptree
    (  name	=> "self strict, catch err",
       prog	=> 'use strict; bogus',
       errs	=> 'Bareword "bogus" not allowed while "strict subs" in use at -e line 1.',
       expect	=> "nextstate",	# simple expectations
       expect_nt => "nextstate",
       noanchors => 1,		# allow them to work
       );
    
checkOptree ( name	=> "sort lK - flag specific search",
	      prog	=> 'our (@a,@b); @b = sort @a',
	      noanchors	=> 1,
	      expect	=> '<@> sort lK ',
	      expect_nt	=> '<@> sort lK ');

checkOptree ( name	=> "sort vK - flag specific search",
	      prog	=> 'sort our @a',
	      errs	=> 'Useless use of sort in void context at -e line 1.',
	      noanchors	=> 1,
	      expect	=> '<@> sort vK',
	      expect_nt	=> '<@> sort vK');

checkOptree ( name	=> "'code' => 'sort our \@a'",
	      code	=> 'sort our @a',
	      noanchors	=> 1,
	      expect	=> '<@> sort K',
	      expect_nt	=> '<@> sort K');

pass ("REFTEXT FIXUP TESTS");

checkOptree ( name	=> 'fixup nextstate (in reftext)',
	      bcopts	=> '-exec',
	      code	=> sub {my $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate( NOTE THAT THIS CAN BE ANYTHING ) v:>,<,%
# 2  <0> padsv[$a:54,55] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 54 optree_concise.t:84) v:>,<,%
# 2  <0> padsv[$a:54,55] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'fixup opcode args',
	      bcopts	=> '-exec',
	      #fail	=> 1, # uncomment to see real padsv args: [$a:491,492] 
	      code	=> sub {my $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 56 optree_concise.t:96) v:>,<,%
# 2  <0> padsv[$a:56,57] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 56 optree_concise.t:96) v:>,<,%
# 2  <0> padsv[$a:56,57] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

#################################
pass("CANONICAL B::Concise EXAMPLE");

checkOptree ( name	=> 'canonical example w -basic',
	      bcopts	=> '-basic',
	      code	=>  sub{$a=$b+42},
	      crossfail => 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 7  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->7
# 1        <;> nextstate(main 380 optree_selftest.t:139) v:>,<,%,{ ->2
# 6        <2> sassign sKS/2 ->7
# 4           <2> add[t3] sK/2 ->5
# -              <1> ex-rv2sv sK/1 ->3
# 2                 <#> gvsv[*b] s ->3
# 3              <$> const[IV 42] s ->4
# -           <1> ex-rv2sv sKRM*/1 ->6
# 5              <#> gvsv[*a] s ->6
EOT_EOT
# 7  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->7
# 1        <;> nextstate(main 60 optree_concise.t:122) v:>,<,%,{ ->2
# 6        <2> sassign sKS/2 ->7
# 4           <2> add[t1] sK/2 ->5
# -              <1> ex-rv2sv sK/1 ->3
# 2                 <$> gvsv(*b) s ->3
# 3              <$> const(IV 42) s ->4
# -           <1> ex-rv2sv sKRM*/1 ->6
# 5              <$> gvsv(*a) s ->6
EONT_EONT

checkOptree ( code	=> '$a=$b+42',
	      bcopts	=> '-exec',
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 837 (eval 24):1) v:{
# 2  <#> gvsv[*b] s
# 3  <$> const[IV 42] s
# 4  <2> add[t3] sK/2
# 5  <#> gvsv[*a] s
# 6  <2> sassign sKS/2
# 7  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 837 (eval 24):1) v:{
# 2  <$> gvsv(*b) s
# 3  <$> const(IV 42) s
# 4  <2> add[t1] sK/2
# 5  <$> gvsv(*a) s
# 6  <2> sassign sKS/2
# 7  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
