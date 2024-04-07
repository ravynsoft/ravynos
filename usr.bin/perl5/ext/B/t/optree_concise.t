#!perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

# import checkOptree(), and %gOpts (containing test state)
use OptreeCheck;	# ALSO DOES @ARGV HANDLING !!!!!!

plan tests => 41;

$SIG{__WARN__} = sub {
    my $err = shift;
    $err =~ m/Subroutine re::(un)?install redefined/ and return;
};
#################################
pass("CANONICAL B::Concise EXAMPLE");

checkOptree ( name	=> 'canonical example w -basic',
	      bcopts	=> '-basic',
	      code	=>  sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 7  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->7
# 1        <;> nextstate(foo bar) v:>,<,%,{ ->2
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

checkOptree ( name	=> 'canonical example w -exec',
	      bcopts	=> '-exec',
	      code	=> sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 61 optree_concise.t:139) v:>,<,%,{
# 2  <#> gvsv[*b] s
# 3  <$> const[IV 42] s
# 4  <2> add[t3] sK/2
# 5  <#> gvsv[*a] s
# 6  <2> sassign sKS/2
# 7  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 61 optree_concise.t:139) v:>,<,%,{
# 2  <$> gvsv(*b) s
# 3  <$> const(IV 42) s
# 4  <2> add[t1] sK/2
# 5  <$> gvsv(*a) s
# 6  <2> sassign sKS/2
# 7  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

#################################
pass("B::Concise OPTION TESTS");

checkOptree ( name	=> '-base3 sticky-exec',
	      bcopts	=> '-base3',
	      code	=> sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> dbstate(main 24 optree_concise.t:132) v:>,<,%,{
2  <#> gvsv[*b] s
10 <$> const[IV 42] s
11 <2> add[t3] sK/2
12 <#> gvsv[*a] s
20 <2> sassign sKS/2
21 <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 62 optree_concise.t:161) v:>,<,%,{
# 2  <$> gvsv(*b) s
# 10 <$> const(IV 42) s
# 11 <2> add[t1] sK/2
# 12 <$> gvsv(*a) s
# 20 <2> sassign sKS/2
# 21 <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sticky-base3, -basic over sticky-exec',
	      bcopts	=> '-basic',
	      code	=> sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
21 <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->21
1        <;> nextstate(main 32 optree_concise.t:164) v:>,<,%,{ ->2
20       <2> sassign sKS/2 ->21
11          <2> add[t3] sK/2 ->12
-              <1> ex-rv2sv sK/1 ->10
2                 <#> gvsv[*b] s ->10
10             <$> const[IV 42] s ->11
-           <1> ex-rv2sv sKRM*/1 ->20
12             <#> gvsv[*a] s ->20
EOT_EOT
# 21 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->21
# 1        <;> nextstate(main 63 optree_concise.t:186) v:>,<,%,{ ->2
# 20       <2> sassign sKS/2 ->21
# 11          <2> add[t1] sK/2 ->12
# -              <1> ex-rv2sv sK/1 ->10
# 2                 <$> gvsv(*b) s ->10
# 10             <$> const(IV 42) s ->11
# -           <1> ex-rv2sv sKRM*/1 ->20
# 12             <$> gvsv(*a) s ->20
EONT_EONT

checkOptree ( name	=> '-base4',
	      bcopts	=> [qw/ -basic -base4 /],
	      code	=> sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
13 <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->13
1        <;> nextstate(main 26 optree_concise.t:145) v:>,<,%,{ ->2
12       <2> sassign sKS/2 ->13
10          <2> add[t3] sK/2 ->11
-              <1> ex-rv2sv sK/1 ->3
2                 <#> gvsv[*b] s ->3
3              <$> const[IV 42] s ->10
-           <1> ex-rv2sv sKRM*/1 ->12
11             <#> gvsv[*a] s ->12
EOT_EOT
# 13 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->13
# 1        <;> nextstate(main 64 optree_concise.t:193) v:>,<,%,{ ->2
# 12       <2> sassign sKS/2 ->13
# 10          <2> add[t1] sK/2 ->11
# -              <1> ex-rv2sv sK/1 ->3
# 2                 <$> gvsv(*b) s ->3
# 3              <$> const(IV 42) s ->10
# -           <1> ex-rv2sv sKRM*/1 ->12
# 11             <$> gvsv(*a) s ->12
EONT_EONT

checkOptree ( name	=> "restore -base36 default",
	      bcopts	=> [qw/ -basic -base36 /],
	      code	=> sub{$a},
	      crossfail	=> 1,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
3  <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->3
1        <;> nextstate(main 27 optree_concise.t:161) v:>,<,% ->2
-        <1> ex-rv2sv sK/1 ->-
2           <#> gvsv[*a] s ->3
EOT_EOT
# 3  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->3
# 1        <;> nextstate(main 65 optree_concise.t:210) v:>,<,% ->2
# -        <1> ex-rv2sv sK/1 ->-
# 2           <$> gvsv(*a) s ->3
EONT_EONT

checkOptree ( name	=> "terse basic",
	      bcopts	=> [qw/ -basic -terse /],
	      code	=> sub{$a},
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
UNOP (0x82b0918) leavesub [1] 
    LISTOP (0x82b08d8) lineseq 
        COP (0x82b0880) nextstate 
        UNOP (0x82b0860) null [15] 
            PADOP (0x82b0840) gvsv  GV (0x82a818c) *a 
EOT_EOT
# UNOP (0x8282310) leavesub [1] 
#     LISTOP (0x82822f0) lineseq 
#         COP (0x82822b8) nextstate 
#         UNOP (0x812fc20) null [15] 
#             SVOP (0x812fc00) gvsv  GV (0x814692c) *a 
EONT_EONT

checkOptree ( name	=> "sticky-terse exec",
	      bcopts	=> [qw/ -exec /],
	      code	=> sub{$a},
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
COP (0x82b0d70) nextstate 
PADOP (0x82b0d30) gvsv  GV (0x82a818c) *a 
UNOP (0x82b0e08) leavesub [1] 
EOT_EOT
# COP (0x82828e0) nextstate 
# SVOP (0x82828a0) gvsv  GV (0x814692c) *a 
# UNOP (0x8282938) leavesub [1] 
EONT_EONT

pass("OPTIONS IN CMDLINE MODE");

checkOptree ( name => 'cmdline invoke -basic works',
	      prog => 'sort @a',
	      errs => [ 'Useless use of sort in void context at -e line 1.',
			'Name "main::a" used only once: possible typo at -e line 1.',
			],
	      #bcopts	=> '-basic', # default
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 7  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 6     <@> sort vK ->7
# 3        <0> pushmark s ->4
# 5        <1> rv2av[t2] lK/1 ->6
# 4           <#> gv[*a] s ->5
EOT_EOT
# 7  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 6     <@> sort vK ->7
# 3        <0> pushmark s ->4
# 5        <1> rv2av[t1] lK/1 ->6
# 4           <$> gv(*a) s ->5
EONT_EONT

checkOptree ( name => 'cmdline invoke -exec works',
	      prog => 'sort @a',
	      errs => [ 'Useless use of sort in void context at -e line 1.',
			'Name "main::a" used only once: possible typo at -e line 1.',
			],
	      bcopts => '-exec',
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> pushmark s
4  <#> gv[*a] s
5  <1> rv2av[t2] lK/1
6  <@> sort vK
7  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <$> gv(*a) s
# 5  <1> rv2av[t1] lK/1
# 6  <@> sort vK
# 7  <@> leave[1 ref] vKP/REFC
EONT_EONT

;

checkOptree
    ( name	=> 'cmdline self-strict compile err using prog',
      prog	=> 'use strict; sort @a',
      bcopts	=> [qw/ -basic -concise -exec /],
      errs	=> 'Global symbol "@a" requires explicit package name (did you forget to declare "my @a"?) at -e line 1.',
      expect	=> 'nextstate',
      expect_nt	=> 'nextstate',
      noanchors => 1, # allow simple expectations to work
      );

checkOptree
    ( name	=> 'cmdline self-strict compile err using code',
      code	=> 'use strict; sort @a',
      bcopts	=> [qw/ -basic -concise -exec /],
      errs	=> qr/Global symbol "\@a" requires explicit package (?x:
		     )name \(did you forget to declare "my \@a"\?\) at (?x:
		     ).*? line 1\./,
      note	=> 'this test relys on a kludge which copies $@ to rendering when empty',
      expect	=> 'Global symbol',
      expect_nt	=> 'Global symbol',
      noanchors => 1, # allow simple expectations to work
      );

checkOptree
    ( name	=> 'cmdline -basic -concise -exec works',
      prog	=> 'our @a; sort @a',
      bcopts	=> [qw/ -basic -concise -exec /],
      errs	=> ['Useless use of sort in void context at -e line 1.'],
      strip_open_hints => 1,
      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <0> enter v
# 2  <;> nextstate(main 2 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <#> gv[*a] s
# 5  <1> rv2av[t5] lK/1
# 6  <@> sort vK
# 7  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 2 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <$> gv(*a) s
# 5  <1> rv2av[t3] lK/1
# 6  <@> sort vK
# 7  <@> leave[1 ref] vKP/REFC
EONT_EONT


#################################
pass("B::Concise STYLE/CALLBACK TESTS");

use B::Concise qw( walk_output add_style set_style_standard add_callback );

# new relative style, added by set_up_relative_test()
@stylespec =
    ( "#hyphseq2 (*(   (x( ;)x))*)<#classsym> "
      . "#exname#arg(?([#targarglife])?)~#flags(?(/#privateb)?)(x(;~->#next)x) "
      . "(x(;~=> #extra)x)\n" # new 'variable' used here
      
      , "  (*(    )*)     goto #seq\n"
      , "(?(<#seq>)?)#exname#arg(?([#targarglife])?)"
      #. "(x(;~=> #extra)x)\n" # new 'variable' used here
      );

sub set_up_relative_test {
    # add a new style, and a callback which adds an 'extra' property

    add_style ( "relative"	=> @stylespec );
    #set_style_standard ( "relative" );

    add_callback
	( sub {
	    my ($h, $op, $format, $level, $style) = @_;

	    # callback marks up const ops
	    $h->{arg} .= ' CALLBACK' if $h->{name} eq 'const';
	    $h->{extra} = '';

	    if ($lastnext and $$lastnext != $$op) {
		$h->{goto} = ($h->{seq} eq '-')
		    ? 'unresolved' : $h->{seq};
	    }

	    # 2 style specific behaviors
	    if ($style eq 'relative') {
		$h->{extra} = 'RELATIVE';
		$h->{arg} .= ' RELATIVE' if $h->{name} eq 'leavesub';
	    }
	    elsif ($style eq 'scope') {
		# suppress printout entirely
		$$format="" unless grep { $h->{name} eq $_ } @scopeops;
	    }
	});
}

#################################
set_up_relative_test();
pass("set_up_relative_test, new callback installed");

checkOptree ( name	=> 'callback used, independent of style',
	      bcopts	=> [qw/ -concise -exec /],
	      code	=> sub{$a=$b+42},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 76 optree_concise.t:337) v:>,<,%,{
2  <#> gvsv[*b] s
3  <$> const[IV 42] CALLBACK s
4  <2> add[t3] sK/2
5  <#> gvsv[*a] s
6  <2> sassign sKS/2
7  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 455 optree_concise.t:328) v:>,<,%,{
# 2  <$> gvsv(*b) s
# 3  <$> const(IV 42) CALLBACK s
# 4  <2> add[t1] sK/2
# 5  <$> gvsv(*a) s
# 6  <2> sassign sKS/2
# 7  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> "new 'relative' style, -exec mode",
	      bcopts	=> [qw/ -basic -relative /],
	      code	=> sub{$a=$b+42},
	      crossfail	=> 1,
	      #retry	=> 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
7  <1> leavesub RELATIVE[1 ref] K ->(end) => RELATIVE
-     <@> lineseq KP ->7 => RELATIVE
1        <;> nextstate(main 49 optree_concise.t:309) v ->2 => RELATIVE
6        <2> sassign sKS ->7 => RELATIVE
4           <2> add[t3] sK ->5 => RELATIVE
-              <1> ex-rv2sv sK ->3 => RELATIVE
2                 <#> gvsv[*b] s ->3 => RELATIVE
3              <$> const[IV 42] CALLBACK s ->4 => RELATIVE
-           <1> ex-rv2sv sKRM* ->6 => RELATIVE
5              <#> gvsv[*a] s ->6 => RELATIVE
EOT_EOT
# 7  <1> leavesub RELATIVE[1 ref] K ->(end) => RELATIVE
# -     <@> lineseq KP ->7 => RELATIVE
# 1        <;> nextstate(main 77 optree_concise.t:353) v ->2 => RELATIVE
# 6        <2> sassign sKS ->7 => RELATIVE
# 4           <2> add[t1] sK ->5 => RELATIVE
# -              <1> ex-rv2sv sK ->3 => RELATIVE
# 2                 <$> gvsv(*b) s ->3 => RELATIVE
# 3              <$> const(IV 42) CALLBACK s ->4 => RELATIVE
# -           <1> ex-rv2sv sKRM* ->6 => RELATIVE
# 5              <$> gvsv(*a) s ->6 => RELATIVE
EONT_EONT

checkOptree ( name	=> "both -exec -relative",
	      bcopts	=> [qw/ -exec -relative /],
	      code	=> sub{$a=$b+42},
	      crossfail	=> 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 50 optree_concise.t:326) v 
2  <#> gvsv[*b] s 
3  <$> const[IV 42] CALLBACK s 
4  <2> add[t3] sK 
5  <#> gvsv[*a] s 
6  <2> sassign sKS 
7  <1> leavesub RELATIVE[1 ref] K 
EOT_EOT
# 1  <;> nextstate(main 78 optree_concise.t:371) v 
# 2  <$> gvsv(*b) s 
# 3  <$> const(IV 42) CALLBACK s 
# 4  <2> add[t1] sK 
# 5  <$> gvsv(*a) s 
# 6  <2> sassign sKS 
# 7  <1> leavesub RELATIVE[1 ref] K 
EONT_EONT

#################################

@scopeops = qw( leavesub enter leave nextstate );
add_style
	( 'scope'  # concise copy
	  , "#hyphseq2 (*(   (x( ;)x))*)<#classsym> "
	  . "#exname#arg(?([#targarglife])?)~#flags(?(/#private)?)(x(;~->#next)x) "
	  , "  (*(    )*)     goto #seq\n"
	  , "(?(<#seq>)?)#exname#arg(?([#targarglife])?)"
	 );

checkOptree ( name	=> "both -exec -scope",
	      bcopts	=> [qw/ -exec -scope /],
	      code	=> sub{$a=$b+42},
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 50 optree_concise.t:337) v 
7  <1> leavesub[1 ref] K/REFC,1 
EOT_EOT
1  <;> nextstate(main 75 optree_concise.t:396) v 
7  <1> leavesub[1 ref] K/REFC,1 
EONT_EONT


checkOptree ( name	=> "both -basic -scope",
	      bcopts	=> [qw/ -basic -scope /],
	      code	=> sub{$a=$b+42},
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
7  <1> leavesub[1 ref] K/REFC,1 ->(end) 
1        <;> nextstate(main 51 optree_concise.t:347) v ->2 
EOT_EOT
7  <1> leavesub[1 ref] K/REFC,1 ->(end) 
1        <;> nextstate(main 76 optree_concise.t:407) v ->2 
EONT_EONT
