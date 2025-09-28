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
plan tests	=> 42;

pass("OPTIMIZER TESTS - VAR INITIALIZATION");

checkOptree ( name	=> 'sub {my $a}',
	      bcopts	=> '-exec',
	      code	=> sub {my $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 45 optree.t:23) v:>,<,%
# 2  <0> padsv[$a:45,46] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 45 optree.t:23) v:>,<,%
# 2  <0> padsv[$a:45,46] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> '-exec sub {my $a}',
	      bcopts	=> '-exec',
	      code	=> sub {my $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 49 optree.t:52) v:>,<,%
# 2  <0> padsv[$a:49,50] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 49 optree.t:45) v:>,<,%
# 2  <0> padsv[$a:49,50] sM/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sub {our $a}',
	      bcopts	=> '-exec',
	      code	=> sub {our $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 21 optree.t:47) v:>,<,%
2  <#> gvsv[*a] s/OURINTR
3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 51 optree.t:56) v:>,<,%
# 2  <$> gvsv(*a) s/OURINTR
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sub {local $a}',
	      bcopts	=> '-exec',
	      code	=> sub {local $a},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 23 optree.t:57) v:>,<,%,{
2  <#> gvsv[*a] s/LVINTRO
3  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 53 optree.t:67) v:>,<,%,{
# 2  <$> gvsv(*a) s/LVINTRO
# 3  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'my $a',
	      prog	=> 'my $a',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 4  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 3     <0> padsv[$a:1,2] vM/LVINTRO ->4
EOT_EOT
# 4  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 3     <0> padsv[$a:1,2] vM/LVINTRO ->4
EONT_EONT

checkOptree ( name	=> 'our $a',
	      prog	=> 'our $a',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
3  <@> leave[1 ref] vKP/REFC ->(end)
1     <0> enter v ->2
2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
-     <1> rv2sv vK/OURINTR,1 ->3
-        <#> gv[*a] s ->-
EOT_EOT
# 3  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# -     <1> rv2sv vK/OURINTR,1 ->3
# -        <$> gv(*a) s ->-
EONT_EONT

checkOptree ( name	=> 'local $c',
	      prog	=> 'local $c',
	      errs      => ['Name "main::c" used only once: possible typo at -e line 1.'],
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
4  <@> leave[1 ref] vKP/REFC ->(end)
1     <0> enter v ->2
2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
-     <1> ex-rv2sv vKM/LVINTRO,1 ->4
3        <#> gvsv[*c] s/LVINTRO ->4
EOT_EOT
# 4  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# -     <1> ex-rv2sv vKM/LVINTRO,1 ->4
# 3        <$> gvsv(*c) s/LVINTRO ->4
EONT_EONT

pass("MY, OUR, LOCAL, BOTH SUB AND MAIN, = undef");

checkOptree ( name	=> 'sub {my $a=undef}',
	      code	=> sub {my $a=undef},
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
3  <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->3
1        <;> nextstate(main 1517 optree_varinit.t:128) v ->2
-        <1> ex-sassign sKS/2 ->-
2           <0> undef[$a:1517,1518] s/LVINTRO,KEEP_PV,TARGMY ->3
-           <0> ex-padsv sRM*/LVINTRO ->-
EOT_EOT
# 3  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->3
# 1        <;> nextstate(main 1517 optree_varinit.t:128) v ->2
# -        <1> ex-sassign sKS/2 ->-
# 2           <0> undef[$a:1517,1518] s/LVINTRO,KEEP_PV,TARGMY ->3
# -           <0> ex-padsv sRM*/LVINTRO ->-
EONT_EONT

checkOptree ( name	=> 'sub {our $a=undef}',
	      code	=> sub {our $a=undef},
	      note	=> 'the global must be reset',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
5  <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->5
1        <;> nextstate(main 1520 optree_varinit.t:148) v:{ ->2
4        <2> sassign sKS/2 ->5
2           <0> undef s ->3
-           <1> ex-rv2sv sKRM*/OURINTR,1 ->4
3              <#> gvsv[*a] s/OURINTR ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 1520 optree_varinit.t:148) v:{ ->2
# 4        <2> sassign sKS/2 ->5
# 2           <0> undef s ->3
# -           <1> ex-rv2sv sKRM*/OURINTR,1 ->4
# 3              <$> gvsv(*a) s/OURINTR ->4
EONT_EONT

checkOptree ( name	=> 'sub {local $a=undef}',
	      code	=> sub {local $a=undef},
	      note	=> 'local not used enough to bother',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
5  <1> leavesub[1 ref] K/REFC,1 ->(end)
-     <@> lineseq KP ->5
1        <;> nextstate(main 1523 optree_varinit.t:171) v:{ ->2
4        <2> sassign sKS/2 ->5
2           <0> undef s ->3
-           <1> ex-rv2sv sKRM*/LVINTRO,1 ->4
3              <#> gvsv[*a] s/LVINTRO ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 1523 optree_varinit.t:171) v:{ ->2
# 4        <2> sassign sKS/2 ->5
# 2           <0> undef s ->3
# -           <1> ex-rv2sv sKRM*/LVINTRO,1 ->4
# 3              <$> gvsv(*a) s/LVINTRO ->4
EONT_EONT

checkOptree ( name	=> 'my $a=undef',
	      prog	=> 'my $a=undef',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
4  <@> leave[1 ref] vKP/REFC ->(end)
1     <0> enter v ->2
2     <;> nextstate(main 1 -e:1) v:{ ->3
-     <1> ex-sassign vKS/2 ->4
3        <0> undef[$a:1,2] s/LVINTRO,KEEP_PV,TARGMY ->4
-        <0> ex-padsv sRM*/LVINTRO ->-
EOT_EOT
# 4  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:{ ->3
# -     <1> ex-sassign vKS/2 ->4
# 3        <0> undef[$a:1,2] s/LVINTRO,KEEP_PV,TARGMY ->4
# -        <0> ex-padsv sRM*/LVINTRO ->-
EONT_EONT

checkOptree ( name	=> 'our $a=undef',
	      prog	=> 'our $a=undef',
	      note	=> 'global must be reassigned',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
6  <@> leave[1 ref] vKP/REFC ->(end)
1     <0> enter v ->2
2     <;> nextstate(main 1 -e:1) v:{ ->3
5     <2> sassign vKS/2 ->6
3        <0> undef s ->4
-        <1> ex-rv2sv sKRM*/OURINTR,1 ->5
4           <#> gvsv[*a] s/OURINTR ->5
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:{ ->3
# 5     <2> sassign vKS/2 ->6
# 3        <0> undef s ->4
# -        <1> ex-rv2sv sKRM*/OURINTR,1 ->5
# 4           <$> gvsv(*a) s/OURINTR ->5
EONT_EONT

checkOptree ( name	=> 'local $c=undef',
	      prog	=> 'local $c=undef',
	      errs      => ['Name "main::c" used only once: possible typo at -e line 1.'],
	      note	=> 'locals are rare, probably not worth doing',
	      bcopts	=> '-basic',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
6  <@> leave[1 ref] vKP/REFC ->(end)
1     <0> enter v ->2
2     <;> nextstate(main 1 -e:1) v:{ ->3
5     <2> sassign vKS/2 ->6
3        <0> undef s ->4
-        <1> ex-rv2sv sKRM*/LVINTRO,1 ->5
4           <#> gvsv[*c] s/LVINTRO ->5
EOT_EOT
# 6  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:{ ->3
# 5     <2> sassign vKS/2 ->6
# 3        <0> undef s ->4
# -        <1> ex-rv2sv sKRM*/LVINTRO,1 ->5
# 4           <$> gvsv(*c) s/LVINTRO ->5
EONT_EONT

checkOptree ( name	=> 'sub {my $a=()}',
	      code	=> sub {my $a=()},
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -439 optree.t:105) v:>,<,%
2  <0> stub sP
3  <1> padsv_store[$a:1516,1517] sKS/LVINTRO
4  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 438 optree_varinit.t:247) v:>,<,%
# 2  <0> stub sP
# 3  <1> padsv_store[$a:1516,1517] sKS/LVINTRO
# 4  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sub {our $a=()}',
	      code	=> sub {our $a=()},
              #todo	=> 'probly not worth doing',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 31 optree.t:177) v:>,<,%,{
2  <0> stub sP
3  <#> gvsv[*a] s/OURINTR
4  <2> sassign sKS/2
5  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 440 optree_varinit.t:262) v:>,<,%,{
# 2  <0> stub sP
# 3  <$> gvsv(*a) s/OURINTR
# 4  <2> sassign sKS/2
# 5  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sub {local $a=()}',
	      code	=> sub {local $a=()},
              #todo	=> 'probly not worth doing',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main 33 optree.t:190) v:>,<,%,{
2  <0> stub sP
3  <#> gvsv[*a] s/LVINTRO
4  <2> sassign sKS/2
5  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 63 optree.t:225) v:>,<,%,{
# 2  <0> stub sP
# 3  <$> gvsv(*a) s/LVINTRO
# 4  <2> sassign sKS/2
# 5  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'my $a=()',
	      prog	=> 'my $a=()',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> stub sP
4  <1> padsv_store[$a:1516,1517] vKS/LVINTRO
5  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> stub sP
# 4  <1> padsv_store[$a:1516,1517] vKS/LVINTRO
# 5  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'our $a=()',
	      prog	=> 'our $a=()',
              #todo	=> 'probly not worth doing',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> stub sP
4  <#> gvsv[*a] s/OURINTR
5  <2> sassign vKS/2
6  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> stub sP
# 4  <$> gvsv(*a) s/OURINTR
# 5  <2> sassign vKS/2
# 6  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'local $c=()',
	      prog	=> 'local $c=()',
	      errs      => ['Name "main::c" used only once: possible typo at -e line 1.'],
              #todo	=> 'probly not worth doing',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> stub sP
4  <#> gvsv[*c] s/LVINTRO
5  <2> sassign vKS/2
6  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> stub sP
# 4  <$> gvsv(*c) s/LVINTRO
# 5  <2> sassign vKS/2
# 6  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'my ($a,$b)=()',
	      prog	=> 'my ($a,$b)=()',
              #todo	=> 'probly not worth doing',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <0> padrange[$a:1,2; $b:1,2] RM/LVINTRO,range=2
# 5  <2> aassign[t3] vKS
# 6  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <0> padrange[$a:1,2; $b:1,2] RM/LVINTRO,range=2
# 5  <2> aassign[t3] vKS
# 6  <@> leave[1 ref] vKP/REFC
EONT_EONT
