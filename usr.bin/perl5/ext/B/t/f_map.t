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


=head1 f_map.t

Code test snippets here are adapted from `perldoc -f map`

=for gentest

# chunk: #!perl
# examples shamelessly snatched from perldoc -f map

=cut

=for gentest

# chunk: # translates a list of numbers to the corresponding characters.
@chars = map(chr, @nums);

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@chars = map(chr, @nums); },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 475 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*nums] s
# 5  <1> rv2av[t7] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t8] lK
# 8      <#> gvsv[*_] s
# 9      <1> chr[t5] sK/1
#            goto 7
# a  <0> pushmark s
# b  <#> gv[*chars] s
# c  <1> rv2av[t2] lKRM*/1
# d  <2> aassign[t9] KS/COM_AGG
# e  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 559 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*nums) s
# 5  <1> rv2av[t4] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t5] lK
# 8      <$> gvsv(*_) s
# 9      <1> chr[t3] sK/1
#            goto 7
# a  <0> pushmark s
# b  <$> gv(*chars) s
# c  <1> rv2av[t1] lKRM*/1
# d  <2> aassign[t6] KS/COM_AGG
# e  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: %hash = map { getkey($_) => $_ } @array;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map { getkey($_) => $_ } @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 476 (eval 10):1) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t8] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t9] lK
# 8      <0> enter l
# 9      <;> nextstate(main 475 (eval 10):1) v:{
# a      <0> pushmark s
# b      <#> gvsv[*_] s
# c      <#> gv[*getkey] s/EARLYCV
# d      <1> entersub[t5] lKS/TARG
# e      <#> gvsv[*_] s
# f      <@> leave lKP
#            goto 7
# g  <0> pushmark s
# h  <#> gv[*hash] s
# i  <1> rv2hv[t2] lKRM*
# j  <2> aassign[t10] KS/COM_AGG
# k  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 560 (eval 15):1) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t3] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t4] lK
# 8      <0> enter l
# 9      <;> nextstate(main 559 (eval 15):1) v:{
# a      <0> pushmark s
# b      <$> gvsv(*_) s
# c      <$> gv(*getkey) s/EARLYCV
# d      <1> entersub[t2] lKS/TARG
# e      <$> gvsv(*_) s
# f      <@> leave lKP
#            goto 7
# g  <0> pushmark s
# h  <$> gv(*hash) s
# i  <1> rv2hv[t1] lKRM*
# j  <2> aassign[t5] KS/COM_AGG
# k  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: {
    %hash = ();
    foreach $_ (@array) {
	$hash{getkey($_)} = $_;
    }
}

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{{ %hash = (); foreach $_ (@array) { $hash{getkey($_)} = $_; } } },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 478 (eval 10):1) v:{
# 2  <{> enterloop(next->u last->u redo->3) 
# 3  <;> nextstate(main 475 (eval 10):1) v
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <#> gv[*hash] s
# 7  <1> rv2hv[t2] lKRM*
# 8  <2> aassign[t3] vKS
# 9  <;> nextstate(main 476 (eval 10):1) v:{
# a  <0> pushmark sM
# b  <#> gv[*array] s
# c  <1> rv2av[t6] sKRM/1
# d  <#> gv[*_] s
# e  <1> rv2gv sKRM/1
# f  <{> enteriter(next->q last->t redo->g) KS/DEF
# r  <0> iter s
# s  <|> and(other->g) K/1
# g      <;> nextstate(main 475 (eval 10):1) v:{
# h      <#> gvsv[*_] s
# i      <#> gv[*hash] s
# j      <1> rv2hv sKR
# k      <0> pushmark s
# l      <#> gvsv[*_] s
# m      <#> gv[*getkey] s/EARLYCV
# n      <1> entersub[t10] sKS/TARG
# o      <2> helem sKRM*/2
# p      <2> sassign vKS/2
# q      <0> unstack s
#            goto r
# t  <2> leaveloop KP/2
# u  <2> leaveloop K/2
# v  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 562 (eval 15):1) v:{
# 2  <{> enterloop(next->u last->u redo->3) 
# 3  <;> nextstate(main 559 (eval 15):1) v
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <$> gv(*hash) s
# 7  <1> rv2hv[t1] lKRM*
# 8  <2> aassign[t2] vKS
# 9  <;> nextstate(main 560 (eval 15):1) v:{
# a  <0> pushmark sM
# b  <$> gv(*array) s
# c  <1> rv2av[t3] sKRM/1
# d  <$> gv(*_) s
# e  <1> rv2gv sKRM/1
# f  <{> enteriter(next->q last->t redo->g) KS/DEF
# r  <0> iter s
# s  <|> and(other->g) K/1
# g      <;> nextstate(main 559 (eval 15):1) v:{
# h      <$> gvsv(*_) s
# i      <$> gv(*hash) s
# j      <1> rv2hv sKR
# k      <0> pushmark s
# l      <$> gvsv(*_) s
# m      <$> gv(*getkey) s/EARLYCV
# n      <1> entersub[t4] sKS/TARG
# o      <2> helem sKRM*/2
# p      <2> sassign vKS/2
# q      <0> unstack s
#            goto r
# t  <2> leaveloop KP/2
# u  <2> leaveloop K/2
# v  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: #%hash = map {  "\L$_", 1  } @array;  # perl guesses EXPR.  wrong
%hash = map { +"\L$_", 1  } @array;  # perl guesses BLOCK. right

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map { +"\L$_", 1 } @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 476 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t7] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t9] lK
# 8      <#> gvsv[*_] s
# 9      <1> lc[t4] sK/1
# a      <$> const[IV 1] s
#            goto 7
# b  <0> pushmark s
# c  <#> gv[*hash] s
# d  <1> rv2hv[t2] lKRM*
# e  <2> aassign[t10] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 560 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t4] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t5] lK
# 8      <$> gvsv(*_) s
# 9      <1> lc[t2] sK/1
# a      <$> const(IV 1) s
#            goto 7
# b  <0> pushmark s
# c  <$> gv(*hash) s
# d  <1> rv2hv[t1] lKRM*
# e  <2> aassign[t6] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: %hash = map { ("\L$_", 1) } @array;  # this also works

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map { ("\L$_", 1) } @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 476 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t7] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t9] lK
# 8      <#> gvsv[*_] s
# 9      <1> lc[t4] sK/1
# a      <$> const[IV 1] s
#            goto 7
# b  <0> pushmark s
# c  <#> gv[*hash] s
# d  <1> rv2hv[t2] lKRM*
# e  <2> aassign[t10] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 560 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t4] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t5] lK
# 8      <$> gvsv(*_) s
# 9      <1> lc[t2] sK/1
# a      <$> const(IV 1) s
#            goto 7
# b  <0> pushmark s
# c  <$> gv(*hash) s
# d  <1> rv2hv[t1] lKRM*
# e  <2> aassign[t6] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: %hash = map {  lc($_), 1  } @array;  # as does this.

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map { lc($_), 1 } @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 476 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t6] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t8] lK
# 8      <#> gvsv[*_] s
# 9      <1> lc[t4] sK/1
# a      <$> const[IV 1] s
#            goto 7
# b  <0> pushmark s
# c  <#> gv[*hash] s
# d  <1> rv2hv[t2] lKRM*
# e  <2> aassign[t9] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 589 (eval 26):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t3] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t4] lK
# 8      <$> gvsv(*_) s
# 9      <1> lc[t2] sK/1
# a      <$> const(IV 1) s
#            goto 7
# b  <0> pushmark s
# c  <$> gv(*hash) s
# d  <1> rv2hv[t1] lKRM*
# e  <2> aassign[t5] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: %hash = map +( lc($_), 1 ), @array;  # this is EXPR and works!

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map +( lc($_), 1 ), @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 475 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t6] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t7] lK
# 8      <#> gvsv[*_] s
# 9      <1> lc[t4] sK/1
# a      <$> const[IV 1] s
#            goto 7
# b  <0> pushmark s
# c  <#> gv[*hash] s
# d  <1> rv2hv[t2] lKRM*
# e  <2> aassign[t8] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 593 (eval 28):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t3] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t4] lK
# 8      <$> gvsv(*_) s
# 9      <1> lc[t2] sK/1
# a      <$> const(IV 1) s
#            goto 7
# b  <0> pushmark s
# c  <$> gv(*hash) s
# d  <1> rv2hv[t1] lKRM*
# e  <2> aassign[t5] KS/COM_AGG
# f  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: %hash = map  ( lc($_), 1 ), @array;  # evaluates to (1, @array)

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{%hash = map ( lc($_), 1 ), @array; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 475 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <$> const[IV 1] sM
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t5] lK
# 8      <#> gvsv[*_] s
# 9      <1> lc[t4] sK/1
#            goto 7
# a  <0> pushmark s
# b  <#> gv[*hash] s
# c  <1> rv2hv[t2] lKRM*
# d  <2> aassign[t6] KS/COM_AGG
# e  <#> gv[*array] s
# f  <1> rv2av[t8] K/1
# g  <@> list K
# h  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 597 (eval 30):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <$> const(IV 1) sM
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t3] lK
# 8      <$> gvsv(*_) s
# 9      <1> lc[t2] sK/1
#            goto 7
# a  <0> pushmark s
# b  <$> gv(*hash) s
# c  <1> rv2hv[t1] lKRM*
# d  <2> aassign[t4] KS/COM_AGG
# e  <$> gv(*array) s
# f  <1> rv2av[t5] K/1
# g  <@> list K
# h  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: @hashes = map +{ lc($_), 1 }, @array # EXPR, so needs , at end

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@hashes = map +{ lc($_), 1 }, @array },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 475 (eval 10):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*array] s
# 5  <1> rv2av[t6] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t7] lK
# 8      <0> pushmark s
# 9      <#> gvsv[*_] s
# a      <1> lc[t4] sK/1
# b      <$> const[IV 1] s
# c      <@> anonhash sK*/1
#            goto 7
# d  <0> pushmark s
# e  <#> gv[*hashes] s
# f  <1> rv2av[t2] lKRM*/1
# g  <2> aassign[t8] KS/COM_AGG
# h  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 601 (eval 32):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*array) s
# 5  <1> rv2av[t3] lKM/1
# 6  <@> mapstart lK
# 7  <|> mapwhile(other->8)[t4] lK
# 8      <0> pushmark s
# 9      <$> gvsv(*_) s
# a      <1> lc[t2] sK/1
# b      <$> const(IV 1) s
# c      <@> anonhash sK*/1
#            goto 7
# d  <0> pushmark s
# e  <$> gv(*hashes) s
# f  <1> rv2av[t1] lKRM*/1
# g  <2> aassign[t5] KS/COM_AGG
# h  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
