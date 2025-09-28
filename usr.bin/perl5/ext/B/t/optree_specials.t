#!./perl

# This tests the B:: module(s) with CHECK, BEGIN, END and INIT blocks. The
# text excerpts below marked with "# " in front are the expected output. They
# are there twice, EOT for threading, and EONT for a non-threading Perl. The
# output is matched losely. If the match fails even though the "got" and
# "expected" output look exactly the same, then watch for trailing, invisible
# spaces.
#
# Note that if this test is mysteriously failing smokes and is hard to
# reproduce, try running with LC_ALL=en_US.UTF-8 PERL_UNICODE="".
# This causes nextstate ops to have a bunch of extra hint info, which
# needs adding to the expected output (for both thraded and non-threaded
# versions)

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

plan tests => 15;

require_ok("B::Concise");

my $out = runperl(
    switches => ["-MO=Concise,BEGIN,CHECK,INIT,END,-exec"],
    prog => q{$a=$b && print q/foo/},
    stderr => 1 );

#print "out:$out\n";

my $src = q[our ($beg, $chk, $init, $end, $uc) = qq{'foo'}; BEGIN { $beg++ } CHECK { $chk++ } INIT { $init++ } END { $end++ } UNITCHECK {$uc++}];


checkOptree ( name	=> 'BEGIN',
	      bcopts	=> 'BEGIN',
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# BEGIN 1:
# a  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->a
# 1        <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$ ->2
# 3        <1> require sK/1 ->4
# 2           <$> const[PV "strict.pm"] s/BARE ->3
# -        <;> ex-nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$ ->4
# -        <@> lineseq K ->-
# 4           <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$ ->5
# 9           <1> entersub[t1] KRS/TARG,STRICT ->a
# 5              <0> pushmark s ->6
# 6              <$> const[PV "strict"] sM ->7
# 7              <$> const[PV "refs"] sM ->8
# 8              <.> method_named[PV "unimport"] ->9
# BEGIN 2:
# k  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->k
# b        <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$ ->c
# d        <1> require sK/1 ->e
# c           <$> const[PV "warnings.pm"] s/BARE ->d
# -        <;> ex-nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$ ->e
# -        <@> lineseq K ->-
# e           <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$ ->f
# j           <1> entersub[t1] KRS/TARG ->k
# f              <0> pushmark s ->g
# g              <$> const[PV "warnings"] sM ->h
# h              <$> const[PV "once"] sM ->i
# i              <.> method_named[PV "unimport"] ->j
# BEGIN 3:
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# l        <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$ ->m
# q        <2> sassign sKS/2 ->r
# o           <1> srefgen sK/1 ->p
# -              <1> ex-list lKRM ->o
# n                 <1> rv2gv sKRM/STRICT,1 ->o
# m                    <#> gv[*STDOUT] s ->n
# -           <1> ex-rv2sv sKRM*/STRICT,1 ->q
# p              <#> gvsv[*B::Concise::walkHandle] s ->q
# BEGIN 4:
# 11 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->11
# s        <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$ ->t
# u        <1> require sK/1 ->v
# t           <$> const[PV "strict.pm"] s/BARE ->u
# -        <;> ex-nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$ ->v
# -        <@> lineseq K ->-
# v           <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$ ->w
# 10          <1> entersub[t1] KRS/TARG,STRICT ->11
# w              <0> pushmark s ->x
# x              <$> const[PV "strict"] sM ->y
# y              <$> const[PV "refs"] sM ->z
# z              <.> method_named[PV "unimport"] ->10
# BEGIN 5:
# 1b <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->1b
# 12       <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$ ->13
# 14       <1> require sK/1 ->15
# 13          <$> const[PV "strict.pm"] s/BARE ->14
# -        <;> ex-nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$ ->15
# -        <@> lineseq K ->-
# 15          <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$ ->16
# 1a          <1> entersub[t1] KRS/TARG,STRICT ->1b
# 16             <0> pushmark s ->17
# 17             <$> const[PV "strict"] sM ->18
# 18             <$> const[PV "refs"] sM ->19
# 19             <.> method_named[PV "unimport"] ->1a
# BEGIN 6:
# 1l <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->1l
# 1c       <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$ ->1d
# 1e       <1> require sK/1 ->1f
# 1d          <$> const[PV "strict.pm"] s/BARE ->1e
# -        <;> ex-nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$ ->1f
# -        <@> lineseq K ->-
# 1f          <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$ ->1g
# 1k          <1> entersub[t1] KRS/TARG,STRICT ->1l
# 1g             <0> pushmark s ->1h
# 1h             <$> const[PV "strict"] sM ->1i
# 1i             <$> const[PV "refs"] sM ->1j
# 1j             <.> method_named[PV "unimport"] ->1k
# BEGIN 7:
# 1v <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->1v
# 1m       <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$ ->1n
# 1o       <1> require sK/1 ->1p
# 1n          <$> const[PV "strict.pm"] s/BARE ->1o
# -        <;> ex-nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$ ->1p
# -        <@> lineseq K ->-
# 1p          <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$ ->1q
# 1u          <1> entersub[t1] KRS/TARG,STRICT ->1v
# 1q             <0> pushmark s ->1r
# 1r             <$> const[PV "strict"] sM ->1s
# 1s             <$> const[PV "refs"] sM ->1t
# 1t             <.> method_named[PV "unimport"] ->1u
# BEGIN 8:
# 25 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->25
# 1w       <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$ ->1x
# 1y       <1> require sK/1 ->1z
# 1x          <$> const[PV "warnings.pm"] s/BARE ->1y
# -        <;> ex-nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$ ->1z
# -        <@> lineseq K ->-
# 1z          <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$ ->20
# 24          <1> entersub[t1] KRS/TARG,STRICT ->25
# 20             <0> pushmark s ->21
# 21             <$> const[PV "warnings"] sM ->22
# 22             <$> const[PV "qw"] sM ->23
# 23             <.> method_named[PV "unimport"] ->24
# BEGIN 9:
# 29 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->29
# 26       <;> nextstate(main 3 -e:1) v:{ ->27
# 28       <1> postinc[t3] sK/1 ->29
# -           <1> ex-rv2sv sKRM/1 ->28
# 27             <#> gvsv[*beg] s ->28
EOT_EOT
# BEGIN 1:
# a  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->a
# 1        <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$ ->2
# 3        <1> require sK/1 ->4
# 2           <$> const(PV "strict.pm") s/BARE ->3
# -        <;> ex-nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$ ->4
# -        <@> lineseq K ->-
# 4           <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$ ->5
# 9           <1> entersub[t1] KRS/TARG,STRICT ->a
# 5              <0> pushmark s ->6
# 6              <$> const(PV "strict") sM ->7
# 7              <$> const(PV "refs") sM ->8
# 8              <.> method_named(PV "unimport") ->9
# BEGIN 2:
# k  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->k
# b        <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$ ->c
# d        <1> require sK/1 ->e
# c           <$> const(PV "warnings.pm") s/BARE ->d
# -        <;> ex-nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$ ->e
# -        <@> lineseq K ->-
# e           <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$ ->f
# j           <1> entersub[t1] KRS/TARG ->k
# f              <0> pushmark s ->g
# g              <$> const(PV "warnings") sM ->h
# h              <$> const(PV "once") sM ->i
# i              <.> method_named(PV "unimport") ->j
# BEGIN 3:
# r  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->r
# l        <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$ ->m
# q        <2> sassign sKS/2 ->r
# o           <1> srefgen sK/1 ->p
# -              <1> ex-list lKRM ->o
# n                 <1> rv2gv sKRM/STRICT,1 ->o
# m                    <$> gv(*STDOUT) s ->n
# -           <1> ex-rv2sv sKRM*/STRICT,1 ->q
# p              <$> gvsv(*B::Concise::walkHandle) s ->q
# BEGIN 4:
# 11 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->11
# s        <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$ ->t
# u        <1> require sK/1 ->v
# t           <$> const(PV "strict.pm") s/BARE ->u
# -        <;> ex-nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$ ->v
# -        <@> lineseq K ->-
# v           <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$ ->w
# 10          <1> entersub[t1] KRS/TARG,STRICT ->11
# w              <0> pushmark s ->x
# x              <$> const(PV "strict") sM ->y
# y              <$> const(PV "refs") sM ->z
# z              <.> method_named(PV "unimport") ->10
# BEGIN 5:
# 1b <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->1b
# 12       <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$ ->13
# 14       <1> require sK/1 ->15
# 13          <$> const(PV "strict.pm") s/BARE ->14
# -        <;> ex-nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$ ->15
# -        <@> lineseq K ->-
# 15          <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$ ->16
# 1a          <1> entersub[t1] KRS/TARG,STRICT ->1b
# 16             <0> pushmark s ->17
# 17             <$> const(PV "strict") sM ->18
# 18             <$> const(PV "refs") sM ->19
# 19             <.> method_named(PV "unimport") ->1a
# BEGIN 6:
# 1l <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->1l
# 1c       <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$ ->1d
# 1e       <1> require sK/1 ->1f
# 1d          <$> const(PV "strict.pm") s/BARE ->1e
# -        <;> ex-nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$ ->1f
# -        <@> lineseq K ->-
# 1f          <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$ ->1g
# 1k          <1> entersub[t1] KRS/TARG,STRICT ->1l
# 1g             <0> pushmark s ->1h
# 1h             <$> const(PV "strict") sM ->1i
# 1i             <$> const(PV "refs") sM ->1j
# 1j             <.> method_named(PV "unimport") ->1k
# BEGIN 7:
# 1v <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq K ->1v
# 1m       <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$ ->1n
# 1o       <1> require sK/1 ->1p
# 1n          <$> const(PV "strict.pm") s/BARE ->1o
# -        <;> ex-nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$ ->1p
# -        <@> lineseq K ->-
# 1p          <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$ ->1q
# 1u          <1> entersub[t1] KRS/TARG,STRICT ->1v
# 1q             <0> pushmark s ->1r
# 1r             <$> const(PV "strict") sM ->1s
# 1s             <$> const(PV "refs") sM ->1t
# 1t             <.> method_named(PV "unimport") ->1u
# BEGIN 8:
# 25 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->25
# 1w       <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$ ->1x
# 1y       <1> require sK/1 ->1z
# 1x          <$> const(PV "warnings.pm") s/BARE ->1y
# -        <;> ex-nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$ ->1z
# -        <@> lineseq K ->-
# 1z          <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$ ->20
# 24          <1> entersub[t1] KRS/TARG,STRICT ->25
# 20             <0> pushmark s ->21
# 21             <$> const(PV "warnings") sM ->22
# 22             <$> const(PV "qw") sM ->23
# 23             <.> method_named(PV "unimport") ->24
# BEGIN 9:
# 29 <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->29
# 26       <;> nextstate(main 3 -e:1) v:{ ->27
# 28       <1> postinc[t2] sK/1 ->29
# -           <1> ex-rv2sv sKRM/1 ->28
# 27             <$> gvsv(*beg) s ->28
EONT_EONT

checkOptree ( name	=> 'END',
	      bcopts	=> 'END',
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# END 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 5 -e:6) v:>,<,%,{ ->2
# 3        <1> postinc[t3] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <#> gvsv[*end] s ->3
EOT_EOT
# END 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 5 -e:6) v:>,<,%,{ ->2
# 3        <1> postinc[t2] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <$> gvsv(*end) s ->3
EONT_EONT

checkOptree ( name	=> 'CHECK',
	      bcopts	=> 'CHECK',
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# CHECK 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 3 -e:4) v:>,<,%,{ ->2
# 3        <1> postinc[t3] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <#> gvsv[*chk] s ->3
EOT_EOT
# CHECK 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 3 -e:4) v:>,<,%,{ ->2
# 3        <1> postinc[t2] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <$> gvsv(*chk) s ->3
EONT_EONT

checkOptree ( name	=> 'UNITCHECK',
	      bcopts=> 'UNITCHECK',
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# UNITCHECK 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 3 -e:4) v:>,<,%,{ ->2
# 3        <1> postinc[t3] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <#> gvsv[*uc] s ->3
EOT_EOT
# UNITCHECK 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 3 -e:4) v:>,<,%,{ ->2
# 3        <1> postinc[t2] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <$> gvsv(*uc) s ->3
EONT_EONT

checkOptree ( name	=> 'INIT',
	      bcopts	=> 'INIT',
	      #todo	=> 'get working',
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# INIT 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 4 -e:5) v:>,<,%,{ ->2
# 3        <1> postinc[t3] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <#> gvsv[*init] s ->3
EOT_EOT
# INIT 1:
# 4  <1> leavesub[1 ref] K/REFC,1 ->(end)
# -     <@> lineseq KP ->4
# 1        <;> nextstate(main 4 -e:5) v:>,<,%,{ ->2
# 3        <1> postinc[t2] sK/1 ->4
# -           <1> ex-rv2sv sKRM/1 ->3
# 2              <$> gvsv(*init) s ->3
EONT_EONT

checkOptree ( name	=> 'all of BEGIN END INIT CHECK UNITCHECK -exec',
	      bcopts	=> [qw/ BEGIN END INIT CHECK UNITCHECK -exec /],
	      prog	=> $src,
	      strip_open_hints => 1,
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# BEGIN 1:
# 1  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$
# 2  <$> const[PV "strict.pm"] s/BARE
# 3  <1> require sK/1
# 4  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$
# 5  <0> pushmark s
# 6  <$> const[PV "strict"] sM
# 7  <$> const[PV "refs"] sM
# 8  <.> method_named[PV "unimport"] 
# 9  <1> entersub[t1] KRS/TARG,STRICT
# a  <1> leavesub[1 ref] K/REFC,1
# BEGIN 2:
# b  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$
# c  <$> const[PV "warnings.pm"] s/BARE
# d  <1> require sK/1
# e  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$
# f  <0> pushmark s
# g  <$> const[PV "warnings"] sM
# h  <$> const[PV "once"] sM
# i  <.> method_named[PV "unimport"] 
# j  <1> entersub[t1] KRS/TARG
# k  <1> leavesub[1 ref] K/REFC,1
# BEGIN 3:
# l  <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$
# m  <#> gv[*STDOUT] s
# n  <1> rv2gv sKRM/STRICT,1
# o  <1> srefgen sK/1
# p  <#> gvsv[*B::Concise::walkHandle] s
# q  <2> sassign sKS/2
# r  <1> leavesub[1 ref] K/REFC,1
# BEGIN 4:
# s  <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$
# t  <$> const[PV "strict.pm"] s/BARE
# u  <1> require sK/1
# v  <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$
# w  <0> pushmark s
# x  <$> const[PV "strict"] sM
# y  <$> const[PV "refs"] sM
# z  <.> method_named[PV "unimport"] 
# 10 <1> entersub[t1] KRS/TARG,STRICT
# 11 <1> leavesub[1 ref] K/REFC,1
# BEGIN 5:
# 12 <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$
# 13 <$> const[PV "strict.pm"] s/BARE
# 14 <1> require sK/1
# 15 <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$
# 16 <0> pushmark s
# 17 <$> const[PV "strict"] sM
# 18 <$> const[PV "refs"] sM
# 19 <.> method_named[PV "unimport"] 
# 1a <1> entersub[t1] KRS/TARG,STRICT
# 1b <1> leavesub[1 ref] K/REFC,1
# BEGIN 6:
# 1c <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$
# 1d <$> const[PV "strict.pm"] s/BARE
# 1e <1> require sK/1
# 1f <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$
# 1g <0> pushmark s
# 1h <$> const[PV "strict"] sM
# 1i <$> const[PV "refs"] sM
# 1j <.> method_named[PV "unimport"] 
# 1k <1> entersub[t1] KRS/TARG,STRICT
# 1l <1> leavesub[1 ref] K/REFC,1
# BEGIN 7:
# 1m <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$
# 1n <$> const[PV "strict.pm"] s/BARE
# 1o <1> require sK/1
# 1p <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$
# 1q <0> pushmark s
# 1r <$> const[PV "strict"] sM
# 1s <$> const[PV "refs"] sM
# 1t <.> method_named[PV "unimport"] 
# 1u <1> entersub[t1] KRS/TARG,STRICT
# 1v <1> leavesub[1 ref] K/REFC,1
# BEGIN 8:
# 1w <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$
# 1x <$> const[PV "warnings.pm"] s/BARE
# 1y <1> require sK/1
# 1z <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$
# 20 <0> pushmark s
# 21 <$> const[PV "warnings"] sM
# 22 <$> const[PV "qw"] sM
# 23 <.> method_named[PV "unimport"] 
# 24 <1> entersub[t1] KRS/TARG,STRICT
# 25 <1> leavesub[1 ref] K/REFC,1
# BEGIN 9:
# 26 <;> nextstate(main 3 -e:1) v:{
# 27 <#> gvsv[*beg] s
# 28 <1> postinc[t3] sK/1
# 29 <1> leavesub[1 ref] K/REFC,1
# END 1:
# 2a <;> nextstate(main 9 -e:1) v:{
# 2b <#> gvsv[*end] s
# 2c <1> postinc[t3] sK/1
# 2d <1> leavesub[1 ref] K/REFC,1
# INIT 1:
# 2e <;> nextstate(main 7 -e:1) v:{
# 2f <#> gvsv[*init] s
# 2g <1> postinc[t3] sK/1
# 2h <1> leavesub[1 ref] K/REFC,1
# CHECK 1:
# 2i <;> nextstate(main 5 -e:1) v:{
# 2j <#> gvsv[*chk] s
# 2k <1> postinc[t3] sK/1
# 2l <1> leavesub[1 ref] K/REFC,1
# UNITCHECK 1:
# 2m <;> nextstate(main 11 -e:1) v:{
# 2n <#> gvsv[*uc] s
# 2o <1> postinc[t3] sK/1
# 2p <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# BEGIN 1:
# 1  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$
# 2  <$> const(PV "strict.pm") s/BARE
# 3  <1> require sK/1
# 4  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$
# 5  <0> pushmark s
# 6  <$> const(PV "strict") sM
# 7  <$> const(PV "refs") sM
# 8  <.> method_named(PV "unimport") 
# 9  <1> entersub[t1] KRS/TARG,STRICT
# a  <1> leavesub[1 ref] K/REFC,1
# BEGIN 2:
# b  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$
# c  <$> const(PV "warnings.pm") s/BARE
# d  <1> require sK/1
# e  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$
# f  <0> pushmark s
# g  <$> const(PV "warnings") sM
# h  <$> const(PV "once") sM
# i  <.> method_named(PV "unimport") 
# j  <1> entersub[t1] KRS/TARG
# k  <1> leavesub[1 ref] K/REFC,1
# BEGIN 3:
# l  <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$
# m  <$> gv(*STDOUT) s
# n  <1> rv2gv sKRM/STRICT,1
# o  <1> srefgen sK/1
# p  <$> gvsv(*B::Concise::walkHandle) s
# q  <2> sassign sKS/2
# r  <1> leavesub[1 ref] K/REFC,1
# BEGIN 4:
# s  <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$
# t  <$> const(PV "strict.pm") s/BARE
# u  <1> require sK/1
# v  <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$
# w  <0> pushmark s
# x  <$> const(PV "strict") sM
# y  <$> const(PV "refs") sM
# z  <.> method_named(PV "unimport") 
# 10 <1> entersub[t1] KRS/TARG,STRICT
# 11 <1> leavesub[1 ref] K/REFC,1
# BEGIN 5:
# 12 <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$
# 13 <$> const(PV "strict.pm") s/BARE
# 14 <1> require sK/1
# 15 <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$
# 16 <0> pushmark s
# 17 <$> const(PV "strict") sM
# 18 <$> const(PV "refs") sM
# 19 <.> method_named(PV "unimport") 
# 1a <1> entersub[t1] KRS/TARG,STRICT
# 1b <1> leavesub[1 ref] K/REFC,1
# BEGIN 6:
# 1c <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$
# 1d <$> const(PV "strict.pm") s/BARE
# 1e <1> require sK/1
# 1f <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$
# 1g <0> pushmark s
# 1h <$> const(PV "strict") sM
# 1i <$> const(PV "refs") sM
# 1j <.> method_named(PV "unimport") 
# 1k <1> entersub[t1] KRS/TARG,STRICT
# 1l <1> leavesub[1 ref] K/REFC,1
# BEGIN 7:
# 1m <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$
# 1n <$> const(PV "strict.pm") s/BARE
# 1o <1> require sK/1
# 1p <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$
# 1q <0> pushmark s
# 1r <$> const(PV "strict") sM
# 1s <$> const(PV "refs") sM
# 1t <.> method_named(PV "unimport") 
# 1u <1> entersub[t1] KRS/TARG,STRICT
# 1v <1> leavesub[1 ref] K/REFC,1
# BEGIN 8:
# 1w <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$
# 1x <$> const(PV "warnings.pm") s/BARE
# 1y <1> require sK/1
# 1z <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$
# 20 <0> pushmark s
# 21 <$> const(PV "warnings") sM
# 22 <$> const(PV "qw") sM
# 23 <.> method_named(PV "unimport") 
# 24 <1> entersub[t1] KRS/TARG,STRICT
# 25 <1> leavesub[1 ref] K/REFC,1
# BEGIN 9:
# 26 <;> nextstate(main 3 -e:1) v:{
# 27 <$> gvsv(*beg) s
# 28 <1> postinc[t2] sK/1
# 29 <1> leavesub[1 ref] K/REFC,1
# END 1:
# 2a <;> nextstate(main 9 -e:1) v:{
# 2b <$> gvsv(*end) s
# 2c <1> postinc[t2] sK/1
# 2d <1> leavesub[1 ref] K/REFC,1
# INIT 1:
# 2e <;> nextstate(main 7 -e:1) v:{
# 2f <$> gvsv(*init) s
# 2g <1> postinc[t2] sK/1
# 2h <1> leavesub[1 ref] K/REFC,1
# CHECK 1:
# 2i <;> nextstate(main 5 -e:1) v:{
# 2j <$> gvsv(*chk) s
# 2k <1> postinc[t2] sK/1
# 2l <1> leavesub[1 ref] K/REFC,1
# UNITCHECK 1:
# 2m <;> nextstate(main 11 -e:1) v:{
# 2n <$> gvsv(*uc) s
# 2o <1> postinc[t2] sK/1
# 2p <1> leavesub[1 ref] K/REFC,1
EONT_EONT

# perl "-I../lib" -MO=Concise,BEGIN,CHECK,INIT,END,-exec -e '$a=$b && print q/foo/'

checkOptree ( name	=> 'regression test for patch 25352',
	      bcopts	=> [qw/ BEGIN END INIT CHECK -exec /],
	      prog	=> 'print q/foo/',
	      expect	=> <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# BEGIN 1:
# 1  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$
# 2  <$> const[PV "strict.pm"] s/BARE
# 3  <1> require sK/1
# 4  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$
# 5  <0> pushmark s
# 6  <$> const[PV "strict"] sM
# 7  <$> const[PV "refs"] sM
# 8  <.> method_named[PV "unimport"] 
# 9  <1> entersub[t1] KRS/TARG,STRICT
# a  <1> leavesub[1 ref] K/REFC,1
# BEGIN 2:
# b  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$
# c  <$> const[PV "warnings.pm"] s/BARE
# d  <1> require sK/1
# e  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$
# f  <0> pushmark s
# g  <$> const[PV "warnings"] sM
# h  <$> const[PV "once"] sM
# i  <.> method_named[PV "unimport"] 
# j  <1> entersub[t1] KRS/TARG
# k  <1> leavesub[1 ref] K/REFC,1
# BEGIN 3:
# l  <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$
# m  <#> gv[*STDOUT] s
# n  <1> rv2gv sKRM/STRICT,1
# o  <1> srefgen sK/1
# p  <#> gvsv[*B::Concise::walkHandle] s
# q  <2> sassign sKS/2
# r  <1> leavesub[1 ref] K/REFC,1
# BEGIN 4:
# s  <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$
# t  <$> const[PV "strict.pm"] s/BARE
# u  <1> require sK/1
# v  <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$
# w  <0> pushmark s
# x  <$> const[PV "strict"] sM
# y  <$> const[PV "refs"] sM
# z  <.> method_named[PV "unimport"] 
# 10 <1> entersub[t1] KRS/TARG,STRICT
# 11 <1> leavesub[1 ref] K/REFC,1
# BEGIN 5:
# 12 <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$
# 13 <$> const[PV "strict.pm"] s/BARE
# 14 <1> require sK/1
# 15 <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$
# 16 <0> pushmark s
# 17 <$> const[PV "strict"] sM
# 18 <$> const[PV "refs"] sM
# 19 <.> method_named[PV "unimport"] 
# 1a <1> entersub[t1] KRS/TARG,STRICT
# 1b <1> leavesub[1 ref] K/REFC,1
# BEGIN 6:
# 1c <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$
# 1d <$> const[PV "strict.pm"] s/BARE
# 1e <1> require sK/1
# 1f <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$
# 1g <0> pushmark s
# 1h <$> const[PV "strict"] sM
# 1i <$> const[PV "refs"] sM
# 1j <.> method_named[PV "unimport"] 
# 1k <1> entersub[t1] KRS/TARG,STRICT
# 1l <1> leavesub[1 ref] K/REFC,1
# BEGIN 7:
# 1m <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$
# 1n <$> const[PV "strict.pm"] s/BARE
# 1o <1> require sK/1
# 1p <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$
# 1q <0> pushmark s
# 1r <$> const[PV "strict"] sM
# 1s <$> const[PV "refs"] sM
# 1t <.> method_named[PV "unimport"] 
# 1u <1> entersub[t1] KRS/TARG,STRICT
# 1v <1> leavesub[1 ref] K/REFC,1
# BEGIN 8:
# 1w <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$
# 1x <$> const[PV "warnings.pm"] s/BARE
# 1y <1> require sK/1
# 1z <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$
# 20 <0> pushmark s
# 21 <$> const[PV "warnings"] sM
# 22 <$> const[PV "qw"] sM
# 23 <.> method_named[PV "unimport"] 
# 24 <1> entersub[t1] KRS/TARG,STRICT
# 25 <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# BEGIN 1:
# 1  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) v:*,&,{,x*,x&,x$,$
# 2  <$> const(PV "strict.pm") s/BARE
# 3  <1> require sK/1
# 4  <;> nextstate(Exporter::Heavy -1410 Heavy.pm:4) :*,&,{,x*,x&,x$,$
# 5  <0> pushmark s
# 6  <$> const(PV "strict") sM
# 7  <$> const(PV "refs") sM
# 8  <.> method_named(PV "unimport") 
# 9  <1> entersub[t1] KRS/TARG,STRICT
# a  <1> leavesub[1 ref] K/REFC,1
# BEGIN 2:
# b  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) v:*,&,{,x*,x&,x$
# c  <$> const(PV "warnings.pm") s/BARE
# d  <1> require sK/1
# e  <;> nextstate(Exporter::Heavy -1251 Heavy.pm:202) :*,&,{,x*,x&,x$
# f  <0> pushmark s
# g  <$> const(PV "warnings") sM
# h  <$> const(PV "once") sM
# i  <.> method_named(PV "unimport") 
# j  <1> entersub[t1] KRS/TARG
# k  <1> leavesub[1 ref] K/REFC,1
# BEGIN 3:
# l  <;> nextstate(B::Concise -1175 Concise.pm:117) v:*,&,{,x*,x&,x$,$
# m  <$> gv(*STDOUT) s
# n  <1> rv2gv sKRM/STRICT,1
# o  <1> srefgen sK/1
# p  <$> gvsv(*B::Concise::walkHandle) s
# q  <2> sassign sKS/2
# r  <1> leavesub[1 ref] K/REFC,1
# BEGIN 4:
# s  <;> nextstate(B::Concise -1134 Concise.pm:183) v:*,&,x*,x&,x$,$
# t  <$> const(PV "strict.pm") s/BARE
# u  <1> require sK/1
# v  <;> nextstate(B::Concise -1134 Concise.pm:183) :*,&,x*,x&,x$,$
# w  <0> pushmark s
# x  <$> const(PV "strict") sM
# y  <$> const(PV "refs") sM
# z  <.> method_named(PV "unimport") 
# 10 <1> entersub[t1] KRS/TARG,STRICT
# 11 <1> leavesub[1 ref] K/REFC,1
# BEGIN 5:
# 12 <;> nextstate(B::Concise -1031 Concise.pm:305) v:*,&,x*,x&,x$,$
# 13 <$> const(PV "strict.pm") s/BARE
# 14 <1> require sK/1
# 15 <;> nextstate(B::Concise -1031 Concise.pm:305) :*,&,x*,x&,x$,$
# 16 <0> pushmark s
# 17 <$> const(PV "strict") sM
# 18 <$> const(PV "refs") sM
# 19 <.> method_named(PV "unimport") 
# 1a <1> entersub[t1] KRS/TARG,STRICT
# 1b <1> leavesub[1 ref] K/REFC,1
# BEGIN 6:
# 1c <;> nextstate(B::Concise -984 Concise.pm:370) v:*,&,{,x*,x&,x$,$
# 1d <$> const(PV "strict.pm") s/BARE
# 1e <1> require sK/1
# 1f <;> nextstate(B::Concise -984 Concise.pm:370) :*,&,{,x*,x&,x$,$
# 1g <0> pushmark s
# 1h <$> const(PV "strict") sM
# 1i <$> const(PV "refs") sM
# 1j <.> method_named(PV "unimport") 
# 1k <1> entersub[t1] KRS/TARG,STRICT
# 1l <1> leavesub[1 ref] K/REFC,1
# BEGIN 7:
# 1m <;> nextstate(B::Concise -959 Concise.pm:390) v:*,&,x*,x&,x$,$
# 1n <$> const(PV "strict.pm") s/BARE
# 1o <1> require sK/1
# 1p <;> nextstate(B::Concise -959 Concise.pm:390) :*,&,x*,x&,x$,$
# 1q <0> pushmark s
# 1r <$> const(PV "strict") sM
# 1s <$> const(PV "refs") sM
# 1t <.> method_named(PV "unimport") 
# 1u <1> entersub[t1] KRS/TARG,STRICT
# 1v <1> leavesub[1 ref] K/REFC,1
# BEGIN 8:
# 1w <;> nextstate(B::Concise -945 Concise.pm:410) v:*,&,{,x*,x&,x$,$
# 1x <$> const(PV "warnings.pm") s/BARE
# 1y <1> require sK/1
# 1z <;> nextstate(B::Concise -945 Concise.pm:410) :*,&,{,x*,x&,x$,$
# 20 <0> pushmark s
# 21 <$> const(PV "warnings") sM
# 22 <$> const(PV "qw") sM
# 23 <.> method_named(PV "unimport") 
# 24 <1> entersub[t1] KRS/TARG,STRICT
# 25 <1> leavesub[1 ref] K/REFC,1
EONT_EONT
