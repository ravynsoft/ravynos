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
plan tests => 18;

# The regression this was testing is that the first aelemfast, derived
# from a lexical array, is supposed to be a BASEOP "<0>", while the
# second, from a global, is an SVOP "<$>" or a PADOP "<#>" depending
# on threading. In buggy versions, both showed up as SVOPs/PADOPs. See
# B.xs:cc_opclass() for the relevant code.

# All this is much simpler, now that aelemfast_lex has been broken out from
# aelemfast
checkOptree ( name	=> 'OP_AELEMFAST opclass',
	      code	=> sub { my @x; our @y; $x[127] + $y[-128]},
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 7  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->7
# 1        <;> nextstate(main 634 optree_misc.t:25) v:>,<,% ->2
# 2        <0> padav[@x:634,636] vM/LVINTRO ->3
# -        <;> ex-nextstate(main 1594 optree_misc.t:27) v:>,<,% ->3
# -        <1> rv2av[t4] vK/OURINTR,1 ->3
# -           <#> gv[*y] s ->-
# 3        <;> nextstate(main 636 optree_misc.t:25) v:>,<,%,{ ->4
# 6        <2> add[t6] sK/2 ->7
# -           <1> ex-aelem sK/2 ->5
# 4              <0> aelemfast_lex[@x:634,636] sR/key=127 ->5
# -              <0> ex-const s ->-
# -           <1> ex-aelem sK/2 ->6
# -              <1> ex-rv2av sKR/1 ->-
# 5                 <#> aelemfast[*y] s/key=128 ->6
# -              <0> ex-const s/FOLD ->-
EOT_EOT
# 7  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->7
# 1        <;> nextstate(main 634 optree_misc.t:27) v:>,<,% ->2
# 2        <0> padav[@x:634,636] vM/LVINTRO ->3
# -        <;> ex-nextstate(main 1594 optree_misc.t:27) v:>,<,% ->3
# -        <1> rv2av[t3] vK/OURINTR,1 ->3
# -           <$> gv(*y) s ->-
# 3        <;> nextstate(main 636 optree_misc.t:27) v:>,<,%,{ ->4
# 6        <2> add[t4] sK/2 ->7
# -           <1> ex-aelem sK/2 ->5
# 4              <0> aelemfast_lex[@x:634,636] sR/key=127 ->5
# -              <0> ex-const s ->-
# -           <1> ex-aelem sK/2 ->6
# -              <1> ex-rv2av sKR/1 ->-
# 5                 <$> aelemfast(*y) s/key=128 ->6
# -              <0> ex-const s/FOLD ->-
EONT_EONT

checkOptree ( name	=> 'PMOP children',
	      code	=> sub { $foo =~ s/(a)/$1/ },
	      strip_open_hints => 1,
	      expect => <<'EOT_EOT',   expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 1 -e:1) v:>,<,%,{ ->2
# 4        </> subst(/"(a)"/) sKS ->5
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <#> gvsv[*foo] s ->3
# -           <1> ex-rv2sv sK/1 ->4
# 3              <#> gvsv[*1] s ->4
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 1 -e:1) v:>,<,%,{ ->2
# 4        </> subst(/"(a)"/) sKS ->5
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <$> gvsv(*foo) s ->3
# -           <1> ex-rv2sv sK/1 ->4
# 3              <$> gvsv(*1) s ->4
EONT_EONT

my $t = <<'EOT_EOT';
# 8  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 7     <2> sassign vKS/2 ->8
# 5        <@> index[t2] sK/2 ->6
# -           <0> ex-pushmark s ->3
# 3           <$> const[PV "foo"] s ->4
# 4           <$> const[PVMG "foo"] s ->5
# -        <1> ex-rv2sv sKRM*/1 ->7
# 6           <#> gvsv[*_] s ->7
EOT_EOT
my $nt = <<'EONT_EONT';
# 8  <@> leave[1 ref] vKP/REFC ->(end)
# 1     <0> enter v ->2
# 2     <;> nextstate(main 1 -e:1) v:>,<,%,{ ->3
# 7     <2> sassign vKS/2 ->8
# 5        <@> index[t1] sK/2 ->6
# -           <0> ex-pushmark s ->3
# 3           <$> const(PV "foo") s ->4
# 4           <$> const(PVMG "foo") s ->5
# -        <1> ex-rv2sv sKRM*/1 ->7
# 6           <$> gvsv(*_) s ->7
EONT_EONT

checkOptree ( name      => 'index and PVBM',
	      prog	=> '$_ = index q(foo), q(foo)',
	      strip_open_hints => 1,
	      expect	=> $t,  expect_nt => $nt);

my $tmpfile = tempfile();
open my $fh, '>', $tmpfile or die "Cannot open $tmpfile: $!";
print $fh "no warnings;format =\n@<<<\n\$a\n@>>>\n\@b\n.";
close $fh;

checkOptree ( name      => 'formats',
	      bcopts    => 'STDOUT',
	      progfile	=> $tmpfile,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# main::STDOUT (FORMAT):
# c  <1> leavewrite[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->c
# 1        <;> nextstate(main 1 -:4) v:>,<,% ->2
# 5        <@> formline vK/2 ->6
# 2           <0> pushmark s ->3
# 3           <$> const[PV "@<<<\n"] s ->4
# -           <@> lineseq lK ->5
# -              <;> ex-nextstate(main 3 tmp35894B:3) v:>,<,% ->4
# -              <1> ex-rv2sv sK/1 ->-
# 4                 <#> gvsv[*a] s ->5
# 6        <;> nextstate(main 1 -:6) v:>,<,% ->7
# b        <@> formline sK/2 ->c
# 7           <0> pushmark s ->8
# 8           <$> const[PV "@>>>\n"] s ->9
# -           <@> lineseq lK ->b
# -              <;> ex-nextstate(main 3 tmp35894B:5) v:>,<,% ->9
# a              <1> rv2av[t3] lK/1 ->b
# 9                 <#> gv[*b] s ->a
EOT_EOT
# main::STDOUT (FORMAT):
# c  <1> leavewrite[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->c
# 1        <;> nextstate(main 1 -:4) v:>,<,% ->2
# 5        <@> formline vK/2 ->6
# 2           <0> pushmark s ->3
# 3           <$> const(PV "@<<<\n") s ->4
# -           <@> lineseq lK ->5
# -              <;> ex-nextstate(main 3 tmp35894B:3) v:>,<,% ->4
# -              <1> ex-rv2sv sK/1 ->-
# 4                 <$> gvsv(*a) s ->5
# 6        <;> nextstate(main 1 -:6) v:>,<,% ->7
# b        <@> formline sK/2 ->c
# 7           <0> pushmark s ->8
# 8           <$> const(PV "@>>>\n") s ->9
# -           <@> lineseq lK ->b
# -              <;> ex-nextstate(main 3 tmp35894B:5) v:>,<,% ->9
# a              <1> rv2av[t3] lK/1 ->b
# 9                 <$> gv(*b) s ->a
EONT_EONT

checkOptree ( name      => 'padrange',
	      code	=> sub { my ($x,$y); @a = ($x,$y); ($x,$y) = @a },
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# f  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->f
# 1        <;> nextstate(main 1 -e:1) v:>,<,% ->2
# -        <@> list vKP ->3
# 2           <0> padrange[$x:1,2; $y:1,2] vM/LVINTRO,range=2 ->3
# -           <0> padsv[$x:1,2] vM/LVINTRO ->-
# -           <0> padsv[$y:1,2] vM/LVINTRO ->-
# 3        <;> nextstate(main 2 -e:1) v:>,<,% ->4
# 8        <2> aassign[t4] vKS/COM_AGG ->9
# -           <1> ex-list lKP ->5
# 4              <0> padrange[$x:1,2; $y:1,2] /range=2 ->5
# -              <0> padsv[$x:1,2] s ->-
# -              <0> padsv[$y:1,2] s ->-
# -           <1> ex-list lK ->8
# 5              <0> pushmark s ->6
# 7              <1> rv2av[t3] lKRM*/1 ->8
# 6                 <#> gv[*a] s ->7
# 9        <;> nextstate(main 2 -e:1) v:>,<,%,{ ->a
# e        <2> aassign[t6] KS/COM_RC1 ->f
# -           <1> ex-list lK ->d
# a              <0> pushmark s ->b
# c              <1> rv2av[t5] lK/1 ->d
# b                 <#> gv[*a] s ->c
# -           <1> ex-list lKPRM* ->e
# d              <0> padrange[$x:1,2; $y:1,2] RM/range=2 ->e
# -              <0> padsv[$x:1,2] sRM* ->-
# -              <0> padsv[$y:1,2] sRM* ->-
EOT_EOT
# f  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->f
# 1        <;> nextstate(main 1 -e:1) v:>,<,% ->2
# -        <@> list vKP ->3
# 2           <0> padrange[$x:1,2; $y:1,2] vM/LVINTRO,range=2 ->3
# -           <0> padsv[$x:1,2] vM/LVINTRO ->-
# -           <0> padsv[$y:1,2] vM/LVINTRO ->-
# 3        <;> nextstate(main 2 -e:1) v:>,<,% ->4
# 8        <2> aassign[t4] vKS/COM_AGG ->9
# -           <1> ex-list lKP ->5
# 4              <0> padrange[$x:1,2; $y:1,2] /range=2 ->5
# -              <0> padsv[$x:1,2] s ->-
# -              <0> padsv[$y:1,2] s ->-
# -           <1> ex-list lK ->8
# 5              <0> pushmark s ->6
# 7              <1> rv2av[t3] lKRM*/1 ->8
# 6                 <$> gv(*a) s ->7
# 9        <;> nextstate(main 2 -e:1) v:>,<,%,{ ->a
# e        <2> aassign[t6] KS/COM_RC1 ->f
# -           <1> ex-list lK ->d
# a              <0> pushmark s ->b
# c              <1> rv2av[t5] lK/1 ->d
# b                 <$> gv(*a) s ->c
# -           <1> ex-list lKPRM* ->e
# d              <0> padrange[$x:1,2; $y:1,2] RM/range=2 ->e
# -              <0> padsv[$x:1,2] sRM* ->-
# -              <0> padsv[$y:1,2] sRM* ->-
EONT_EONT

checkOptree ( name      => 'padrange and @_',
	      code	=> sub { my ($a,$b) = @_;
				 my ($c,$d) = @X::_;
				 package Y;
				 my ($e,$f) = @_;
			     },
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# d  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->d
# 1        <;> nextstate(main 1 p3:1) v:>,<,% ->2
# 3        <2> aassign[t5] vKS ->4
# -           <1> ex-list lK ->-
# 2              <0> padrange[$a:1,4; $b:1,4] */LVINTRO,range=2 ->3
# -              <1> rv2av[t4] lK/1 ->-
# -                 <#> gv[*_] s ->-
# -           <1> ex-list lKPRM* ->3
# -              <0> pushmark sRM*/LVINTRO ->-
# -              <0> padsv[$a:1,4] sRM*/LVINTRO ->-
# -              <0> padsv[$b:1,4] sRM*/LVINTRO ->-
# 4        <;> nextstate(main 2 p3:2) v:>,<,% ->5
# 9        <2> aassign[t10] vKS/COM_RC1 ->a
# -           <1> ex-list lK ->8
# 5              <0> pushmark s ->6
# 7              <1> rv2av[t9] lK/1 ->8
# 6                 <#> gv[*X::_] s ->7
# -           <1> ex-list lKPRM* ->9
# 8              <0> padrange[$c:2,4; $d:2,4] RM/LVINTRO,range=2 ->9
# -              <0> padsv[$c:2,4] sRM*/LVINTRO ->-
# -              <0> padsv[$d:2,4] sRM*/LVINTRO ->-
# a        <;> nextstate(Y 3 p3:4) v:>,<,%,{ ->b
# c        <2> aassign[t15] KS ->d
# -           <1> ex-list lK ->-
# b              <0> padrange[$e:3,4; $f:3,4] */LVINTRO,range=2 ->c
# -              <1> rv2av[t14] lK/1 ->-
# -                 <#> gv[*_] s ->-
# -           <1> ex-list lKPRM* ->c
# -              <0> pushmark sRM*/LVINTRO ->-
# -              <0> padsv[$e:3,4] sRM*/LVINTRO ->-
# -              <0> padsv[$f:3,4] sRM*/LVINTRO ->-
EOT_EOT
# d  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->d
# 1        <;> nextstate(main 1 p3:1) v:>,<,% ->2
# 3        <2> aassign[t5] vKS ->4
# -           <1> ex-list lK ->-
# 2              <0> padrange[$a:1,4; $b:1,4] */LVINTRO,range=2 ->3
# -              <1> rv2av[t4] lK/1 ->-
# -                 <$> gv(*_) s ->-
# -           <1> ex-list lKPRM* ->3
# -              <0> pushmark sRM*/LVINTRO ->-
# -              <0> padsv[$a:1,4] sRM*/LVINTRO ->-
# -              <0> padsv[$b:1,4] sRM*/LVINTRO ->-
# 4        <;> nextstate(main 2 p3:2) v:>,<,% ->5
# 9        <2> aassign[t10] vKS/COM_RC1 ->a
# -           <1> ex-list lK ->8
# 5              <0> pushmark s ->6
# 7              <1> rv2av[t9] lK/1 ->8
# 6                 <$> gv(*X::_) s ->7
# -           <1> ex-list lKPRM* ->9
# 8              <0> padrange[$c:2,4; $d:2,4] RM/LVINTRO,range=2 ->9
# -              <0> padsv[$c:2,4] sRM*/LVINTRO ->-
# -              <0> padsv[$d:2,4] sRM*/LVINTRO ->-
# a        <;> nextstate(Y 3 p3:4) v:>,<,%,{ ->b
# c        <2> aassign[t15] KS ->d
# -           <1> ex-list lK ->-
# b              <0> padrange[$e:3,4; $f:3,4] */LVINTRO,range=2 ->c
# -              <1> rv2av[t14] lK/1 ->-
# -                 <$> gv(*_) s ->-
# -           <1> ex-list lKPRM* ->c
# -              <0> pushmark sRM*/LVINTRO ->-
# -              <0> padsv[$e:3,4] sRM*/LVINTRO ->-
# -              <0> padsv[$f:3,4] sRM*/LVINTRO ->-
EONT_EONT

checkOptree ( name      => 'consolidate padranges',
	      code	=> sub { my ($a,$b); my ($c,$d); 1 },
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 900 optree_misc.t:334) v:>,<,% ->2
# -        <@> list vKP ->-
# 2           <0> padrange[$a:900,902; $b:900,902; $c:901,902; $d:901,902] vM/LVINTRO,range=4 ->3
# -           <0> padsv[$a:900,902] vM/LVINTRO ->-
# -           <0> padsv[$b:900,902] vM/LVINTRO ->-
# -        <;> nextstate(main 901 optree_misc.t:334) v:>,<,% ->-
# -        <@> list vKP ->3
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$c:901,902] vM/LVINTRO ->-
# -           <0> padsv[$d:901,902] vM/LVINTRO ->-
# 3        <;> nextstate(main 902 optree_misc.t:334) v:>,<,%,{ ->4
# 4        <$> const[IV 1] s ->5
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 900 optree_misc.t:334) v:>,<,% ->2
# -        <@> list vKP ->-
# 2           <0> padrange[$a:900,902; $b:900,902; $c:901,902; $d:901,902] vM/LVINTRO,range=4 ->3
# -           <0> padsv[$a:900,902] vM/LVINTRO ->-
# -           <0> padsv[$b:900,902] vM/LVINTRO ->-
# -        <;> nextstate(main 901 optree_misc.t:334) v:>,<,% ->-
# -        <@> list vKP ->3
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$c:901,902] vM/LVINTRO ->-
# -           <0> padsv[$d:901,902] vM/LVINTRO ->-
# 3        <;> nextstate(main 902 optree_misc.t:334) v:>,<,%,{ ->4
# 4        <$> const(IV 1) s ->5
EONT_EONT


checkOptree ( name      => 'consolidate padranges and singletons',
	      code	=> sub { my ($a,$b); my $c; my ($d,$e);
				 my @f; my $g; my ($h,$i); my %j; 1 },
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 903 optree_misc.t:371) v:>,<,% ->2
# -        <@> list vKP ->-
# 2           <0> padrange[$a:903,910; $b:903,910; $c:904,910; $d:905,910; $e:905,910; @f:906,910; $g:907,910; $h:908,910; $i:908,910; %j:909,910] vM/LVINTRO,range=10 ->3
# -           <0> padsv[$a:903,910] vM/LVINTRO ->-
# -           <0> padsv[$b:903,910] vM/LVINTRO ->-
# -        <;> nextstate(main 904 optree_misc.t:371) v:>,<,% ->-
# -        <0> padsv[$c:904,910] vM/LVINTRO ->-
# -        <;> nextstate(main 905 optree_misc.t:371) v:>,<,%,{ ->-
# -        <@> list vKP ->-
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$d:905,910] vM/LVINTRO ->-
# -           <0> padsv[$e:905,910] vM/LVINTRO ->-
# -        <;> nextstate(main 906 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padav[@f:906,910] vM/LVINTRO ->-
# -        <;> nextstate(main 907 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padsv[$g:907,910] vM/LVINTRO ->-
# -        <;> nextstate(main 908 optree_misc.t:372) v:>,<,%,{ ->-
# -        <@> list vKP ->-
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$h:908,910] vM/LVINTRO ->-
# -           <0> padsv[$i:908,910] vM/LVINTRO ->-
# -        <;> nextstate(main 909 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padhv[%j:909,910] vM/LVINTRO ->3
# 3        <;> nextstate(main 910 optree_misc.t:372) v:>,<,%,{ ->4
# 4        <$> const[IV 1] s ->5
EOT_EOT
# 5  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->5
# 1        <;> nextstate(main 903 optree_misc.t:371) v:>,<,% ->2
# -        <@> list vKP ->-
# 2           <0> padrange[$a:903,910; $b:903,910; $c:904,910; $d:905,910; $e:905,910; @f:906,910; $g:907,910; $h:908,910; $i:908,910; %j:909,910] vM/LVINTRO,range=10 ->3
# -           <0> padsv[$a:903,910] vM/LVINTRO ->-
# -           <0> padsv[$b:903,910] vM/LVINTRO ->-
# -        <;> nextstate(main 904 optree_misc.t:371) v:>,<,% ->-
# -        <0> padsv[$c:904,910] vM/LVINTRO ->-
# -        <;> nextstate(main 905 optree_misc.t:371) v:>,<,%,{ ->-
# -        <@> list vKP ->-
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$d:905,910] vM/LVINTRO ->-
# -           <0> padsv[$e:905,910] vM/LVINTRO ->-
# -        <;> nextstate(main 906 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padav[@f:906,910] vM/LVINTRO ->-
# -        <;> nextstate(main 907 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padsv[$g:907,910] vM/LVINTRO ->-
# -        <;> nextstate(main 908 optree_misc.t:372) v:>,<,%,{ ->-
# -        <@> list vKP ->-
# -           <0> pushmark vM/LVINTRO ->-
# -           <0> padsv[$h:908,910] vM/LVINTRO ->-
# -           <0> padsv[$i:908,910] vM/LVINTRO ->-
# -        <;> nextstate(main 909 optree_misc.t:372) v:>,<,%,{ ->-
# -        <0> padhv[%j:909,910] vM/LVINTRO ->3
# 3        <;> nextstate(main 910 optree_misc.t:372) v:>,<,%,{ ->4
# 4        <$> const(IV 1) s ->5
EONT_EONT


checkOptree ( name      => 'm?x?',
	      code	=> sub { m?x?; },
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 3  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->3
# 1        <;> nextstate(main 914 optree_misc.t:434) v:>,<,%,{ ->2
# 2        </> match(/"x"/) ->3
EOT_EOT
# 3  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->3
# 1        <;> nextstate(main 914 optree_misc.t:434) v:>,<,%,{ ->2
# 2        </> match(/"x"/) ->3
EONT_EONT


unlink $tmpfile;
