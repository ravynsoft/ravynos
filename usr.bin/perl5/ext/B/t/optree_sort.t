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
plan tests => 21;

pass("SORT OPTIMIZATION");

checkOptree ( name	=> 'sub {sort @a}',
	      code	=> sub {sort @a},
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 424 optree_sort.t:14) v:>,<,%
# 2  <0> pushmark s
# 3  <#> gv[*a] s
# 4  <1> rv2av[t2] lK/1
# 5  <@> sort K
# 6  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 424 optree_sort.t:14) v:>,<,%
# 2  <0> pushmark s
# 3  <$> gv(*a) s
# 4  <1> rv2av[t1] lK/1
# 5  <@> sort K
# 6  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name => 'sort @a',
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

checkOptree ( name	=> 'sub {@a = sort @a}',
	      code	=> sub {@a = sort @a},
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -438 optree.t:244) v:>,<,%
2  <0> pushmark s
3  <0> pushmark s
4  <#> gv[*a] s
5  <1> rv2av[t4] lK/1
6  <@> sort lK
7  <0> pushmark s
8  <#> gv[*a] s
9  <1> rv2av[t2] lKRM*/1
a  <2> aassign[t5] KS/COM_AGG
b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 65 optree.t:311) v:>,<,%
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*a) s
# 5  <1> rv2av[t2] lK/1
# 6  <@> sort lK
# 7  <0> pushmark s
# 8  <$> gv(*a) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> '@a = sort @a',
	      prog	=> '@a = sort @a',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> pushmark s
4  <0> pushmark s
5  <#> gv[*a] s
6  <1> rv2av[t4] lKRM*/1
7  <@> sort lK/INPLACE
8  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <$> gv(*a) s
# 6  <1> rv2av[t2] lKRM*/1
# 7  <@> sort lK/INPLACE
# 8  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'sub {@a = sort @a; reverse @a}',
	      code	=> sub {@a = sort @a; reverse @a},
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -438 optree.t:286) v:>,<,%
2  <0> pushmark s
3  <0> pushmark s
4  <#> gv[*a] s
5  <1> rv2av[t4] lKRM*/1
6  <@> sort lK/INPLACE
7  <;> nextstate(main -438 optree.t:288) v:>,<,%
8  <0> pushmark s
9  <#> gv[*a] s
a  <1> rv2av[t7] lK/1
b  <@> reverse[t8] K/1
c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 66 optree.t:345) v:>,<,%
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*a) s
# 5  <1> rv2av[t2] lKRM*/1
# 6  <@> sort lK/INPLACE
# 7  <;> nextstate(main 66 optree.t:346) v:>,<,%
# 8  <0> pushmark s
# 9  <$> gv(*a) s
# a  <1> rv2av[t4] lK/1
# b  <@> reverse[t5] K/1
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> '@a = sort @a; reverse @a',
	      prog	=> '@a = sort @a; reverse @a',
	      errs      => ['Useless use of reverse in void context at -e line 1.'],
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> pushmark s
4  <0> pushmark s
5  <#> gv[*a] s
6  <1> rv2av[t4] lKRM*/1
7  <@> sort lK/INPLACE
8  <;> nextstate(main 1 -e:1) v:>,<,%,{
9  <0> pushmark s
a  <#> gv[*a] s
b  <1> rv2av[t7] lK/1
c  <@> reverse[t8] vK/1
d  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <$> gv(*a) s
# 6  <1> rv2av[t2] lKRM*/1
# 7  <@> sort lK/INPLACE
# 8  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 9  <0> pushmark s
# a  <$> gv(*a) s
# b  <1> rv2av[t4] lK/1
# c  <@> reverse[t5] vK/1
# d  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'sub {my @a; @a = sort @a}',
	      code	=> sub {my @a; @a = sort @a},
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -437 optree.t:254) v:>,<,%
2  <0> padav[@a:-437,-436] vM/LVINTRO
3  <;> nextstate(main -436 optree.t:256) v:>,<,%
4  <0> pushmark s
5  <0> pushmark s
6  <0> padav[@a:-437,-436] l
7  <@> sort lK
8  <0> pushmark s
9  <0> padav[@a:-437,-436] lRM*
a  <2> aassign[t2] KS/COM_AGG
b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 427 optree_sort.t:172) v:>,<,%
# 2  <0> padav[@a:427,428] vM/LVINTRO
# 3  <;> nextstate(main 428 optree_sort.t:173) v:>,<,%
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <0> padav[@a:427,428] l
# 7  <@> sort lK
# 8  <0> pushmark s
# 9  <0> padav[@a:-437,-436] lRM*
# a  <2> aassign[t2] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'my @a; @a = sort @a',
	      prog	=> 'my @a; @a = sort @a',
	      bcopts	=> '-exec',
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <0> enter v
2  <;> nextstate(main 1 -e:1) v:>,<,%,{
3  <0> padav[@a:1,2] vM/LVINTRO
4  <;> nextstate(main 2 -e:1) v:>,<,%,{
5  <0> pushmark s
6  <0> pushmark s
7  <0> padav[@a:1,2] lRM*
8  <@> sort lK/INPLACE
9  <@> leave[1 ref] vKP/REFC
EOT_EOT
# 1  <0> enter v
# 2  <;> nextstate(main 1 -e:1) v:>,<,%,{
# 3  <0> padav[@a:1,2] vM/LVINTRO
# 4  <;> nextstate(main 2 -e:1) v:>,<,%,{
# 5  <0> pushmark s
# 6  <0> pushmark s
# 7  <0> padav[@a:1,2] lRM*
# 8  <@> sort lK/INPLACE
# 9  <@> leave[1 ref] vKP/REFC
EONT_EONT

checkOptree ( name	=> 'sub {my @a; @a = sort @a; push @a, 1}',
	      code	=> sub {my @a; @a = sort @a; push @a, 1},
	      bcopts	=> '-exec',
	      debug	=> 0,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -437 optree.t:325) v:>,<,%
2  <0> padav[@a:-437,-436] vM/LVINTRO
3  <;> nextstate(main -436 optree.t:325) v:>,<,%
4  <0> pushmark s
5  <0> pushmark s
6  <0> padav[@a:-437,-436] lRM*
7  <@> sort lK/INPLACE
8  <;> nextstate(main -436 optree.t:325) v:>,<,%,{
9  <0> pushmark s
a  <0> padav[@a:-437,-436] lRM
b  <$> const[IV 1] s
c  <@> push[t3] sK/2
d  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 429 optree_sort.t:219) v:>,<,%
# 2  <0> padav[@a:429,430] vM/LVINTRO
# 3  <;> nextstate(main 430 optree_sort.t:220) v:>,<,%
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <0> padav[@a:429,430] lRM*
# 7  <@> sort lK/INPLACE
# 8  <;> nextstate(main 430 optree_sort.t:220) v:>,<,%,{
# 9  <0> pushmark s
# a  <0> padav[@a:429,430] lRM
# b  <$> const(IV 1) s
# c  <@> push[t3] sK/2
# d  <1> leavesub[1 ref] K/REFC,1
EONT_EONT

checkOptree ( name	=> 'sub {my @a; @a = sort @a; 1}',
	      code	=> sub {my @a; @a = sort @a; 1},
	      bcopts	=> '-exec',
	      debug	=> 0,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
1  <;> nextstate(main -437 optree.t:325) v:>,<,%
2  <0> padav[@a:-437,-436] vM/LVINTRO
3  <;> nextstate(main -436 optree.t:325) v:>,<,%
4  <0> pushmark s
5  <0> pushmark s
6  <0> padav[@a:-437,-436] lRM*
7  <@> sort lK/INPLACE
8  <;> nextstate(main -436 optree.t:346) v:>,<,%,{
9  <$> const[IV 1] s
a  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 431 optree_sort.t:250) v:>,<,%
# 2  <0> padav[@a:431,432] vM/LVINTRO
# 3  <;> nextstate(main 432 optree_sort.t:251) v:>,<,%
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <0> padav[@a:431,432] lRM*
# 7  <@> sort lK/INPLACE
# 8  <;> nextstate(main 432 optree_sort.t:251) v:>,<,%,{
# 9  <$> const(IV 1) s
# a  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
