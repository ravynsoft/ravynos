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
plan tests => 38;

=head1 f_sort.t

Code test snippets here are adapted from `perldoc -f map`

=head1 Test Notes

# chunk: #!perl
#examples poached from perldoc -f sort

NOTE: name is no longer a required arg for checkOptree, as label is
synthesized out of others.  HOWEVER, if the test-code has newlines in
it, the label must be overridden by an explicit name.

This is because t/TEST is quite particular about the test output it
processes, and multi-line labels violate its 1-line-per-test
expectations.

=for gentest

# chunk: # sort lexically
@articles = sort @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 545 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t4] lK/1
# 6  <@> sort lK
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t5] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 545 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t2] lK/1
# 6  <@> sort lK
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # same thing, but with explicit sort routine
@articles = sort {$a cmp $b} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {$a cmp $b} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t7] lK/1
# 6  <@> sort lK
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t3] lK/1
# 6  <@> sort lK
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t2] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # now case-insensitively
@articles = sort {uc($a) cmp uc($b)} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {uc($a) cmp uc($b)} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t9] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t10] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t5] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t6] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # same thing in reversed order
@articles = sort {$b cmp $a} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {$b cmp $a} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t7] lK/1
# 6  <@> sort lK/DESC
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t3] lK/1
# 6  <@> sort lK/DESC
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t2] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # sort numerically ascending
@articles = sort {$a <=> $b} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {$a <=> $b} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t7] lK/1
# 6  <@> sort lK/NUM
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t3] lK/1
# 6  <@> sort lK/NUM
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t2] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # sort numerically descending
@articles = sort {$b <=> $a} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {$b <=> $a} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 587 (eval 26):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t7] lK/1
# 6  <@> sort lK/DESC,NUM
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t3] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t3] lK/1
# 6  <@> sort lK/DESC,NUM
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t2] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


=for gentest

# chunk: # this sorts the %age hash by value instead of key
# using an in-line function
@eldest = sort { $age{$b} <=> $age{$a} } keys %age;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@eldest = sort { $age{$b} <=> $age{$a} } keys %age; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 592 (eval 28):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*age] s
# 5  <1> rv2hv[t9] lKRM
# 6  <1> keys[t10] lKM/1
# 7  <@> sort lKS*
# 8  <0> pushmark s
# 9  <#> gv[*eldest] s
# a  <1> rv2av[t2] lKRM*/1
# b  <2> aassign[t11] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*age) s
# 5  <1> rv2hv[t3] lKRM
# 6  <1> keys[t4] lKM/1
# 7  <@> sort lKS*
# 8  <0> pushmark s
# 9  <$> gv(*eldest) s
# a  <1> rv2av[t1] lKRM*/1
# b  <2> aassign[t5] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # sort using explicit subroutine name
sub byage {
    $age{$a} <=> $age{$b};  # presuming numeric
}
@sortedclass = sort byage @class;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{sub byage { $age{$a} <=> $age{$b}; } @sortedclass = sort byage @class; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 597 (eval 30):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const[PV "byage"] s/BARE
# 5  <#> gv[*class] s
# 6  <1> rv2av[t4] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <#> gv[*sortedclass] s
# a  <1> rv2av[t2] lKRM*/1
# b  <2> aassign[t5] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const(PV "byage") s/BARE
# 5  <$> gv(*class) s
# 6  <1> rv2av[t2] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <$> gv(*sortedclass) s
# a  <1> rv2av[t1] lKRM*/1
# b  <2> aassign[t3] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: sub backwards { $b cmp $a }
@harry  = qw(dog cat x Cain Abel);
@george = qw(gone chased yz Punished Axed);
print sort @harry;
# prints AbelCaincatdogx
print sort backwards @harry;
# prints xdogcatCainAbel
print sort @george, 'to', @harry;
# prints AbelAxedCainPunishedcatchaseddoggonetoxyz

=cut

checkOptree(name   => q{sort USERSUB LIST },
	    bcopts => q{-exec},
	    code   => q{sub backwards { $b cmp $a }
			@harry = qw(dog cat x Cain Abel);
			@george = qw(gone chased yz Punished Axed);
			print sort @harry; print sort backwards @harry; 
			print sort @george, 'to', @harry; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 602 (eval 32):2) v
# 2  <0> pushmark s
# 3  <$> const[PV "dog"] s
# 4  <$> const[PV "cat"] s
# 5  <$> const[PV "x"] s
# 6  <$> const[PV "Cain"] s
# 7  <$> const[PV "Abel"] s
# 8  <0> pushmark s
# 9  <#> gv[*harry] s
# a  <1> rv2av[t2] lKRM*/1
# b  <2> aassign[t3] vKS
# c  <;> nextstate(main 602 (eval 32):3) v
# d  <0> pushmark s
# e  <$> const[PV "gone"] s
# f  <$> const[PV "chased"] s
# g  <$> const[PV "yz"] s
# h  <$> const[PV "Punished"] s
# i  <$> const[PV "Axed"] s
# j  <0> pushmark s
# k  <#> gv[*george] s
# l  <1> rv2av[t5] lKRM*/1
# m  <2> aassign[t6] vKS
# n  <;> nextstate(main 602 (eval 32):4) v:{
# o  <0> pushmark s
# p  <0> pushmark s
# q  <#> gv[*harry] s
# r  <1> rv2av[t8] lK/1
# s  <@> sort lK
# t  <@> print vK
# u  <;> nextstate(main 602 (eval 32):4) v:{
# v  <0> pushmark s
# w  <0> pushmark s
# x  <$> const[PV "backwards"] s/BARE
# y  <#> gv[*harry] s
# z  <1> rv2av[t10] lKM/1
# 10 <@> sort lKS
# 11 <@> print vK
# 12 <;> nextstate(main 602 (eval 32):5) v:{
# 13 <0> pushmark s
# 14 <0> pushmark s
# 15 <#> gv[*george] s
# 16 <1> rv2av[t12] lK/1
# 17 <$> const[PV "to"] s
# 18 <#> gv[*harry] s
# 19 <1> rv2av[t14] lK/1
# 1a <@> sort lK
# 1b <@> print sK
# 1c <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 602 (eval 32):2) v
# 2  <0> pushmark s
# 3  <$> const(PV "dog") s
# 4  <$> const(PV "cat") s
# 5  <$> const(PV "x") s
# 6  <$> const(PV "Cain") s
# 7  <$> const(PV "Abel") s
# 8  <0> pushmark s
# 9  <$> gv(*harry) s
# a  <1> rv2av[t1] lKRM*/1
# b  <2> aassign[t2] vKS
# c  <;> nextstate(main 602 (eval 32):3) v
# d  <0> pushmark s
# e  <$> const(PV "gone") s
# f  <$> const(PV "chased") s
# g  <$> const(PV "yz") s
# h  <$> const(PV "Punished") s
# i  <$> const(PV "Axed") s
# j  <0> pushmark s
# k  <$> gv(*george) s
# l  <1> rv2av[t3] lKRM*/1
# m  <2> aassign[t4] vKS
# n  <;> nextstate(main 602 (eval 32):4) v:{
# o  <0> pushmark s
# p  <0> pushmark s
# q  <$> gv(*harry) s
# r  <1> rv2av[t5] lK/1
# s  <@> sort lK
# t  <@> print vK
# u  <;> nextstate(main 602 (eval 32):4) v:{
# v  <0> pushmark s
# w  <0> pushmark s
# x  <$> const(PV "backwards") s/BARE
# y  <$> gv(*harry) s
# z  <1> rv2av[t6] lKM/1
# 10 <@> sort lKS
# 11 <@> print vK
# 12 <;> nextstate(main 602 (eval 32):5) v:{
# 13 <0> pushmark s
# 14 <0> pushmark s
# 15 <$> gv(*george) s
# 16 <1> rv2av[t7] lK/1
# 17 <$> const(PV "to") s
# 18 <$> gv(*harry) s
# 19 <1> rv2av[t8] lK/1
# 1a <@> sort lK
# 1b <@> print sK
# 1c <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # inefficiently sort by descending numeric compare using
# the first integer after the first = sign, or the
# whole record case-insensitively otherwise
@new = @old[ sort {
    $nums[$b] <=> $nums[$a]
	|| $caps[$a] cmp $caps[$b]
	} 0..$#old  ];

=cut
=for gentest

# chunk: # same thing, but without any temps
@new = map { $_->[0] }
sort { $b->[1] <=> $a->[1] 
	   || $a->[2] cmp $b->[2]
	   } map { [$_, /=(\d+)/, uc($_)] } @old;

=cut

checkOptree(name   => q{Compound sort/map Expression },
	    bcopts => q{-exec},
	    code   => q{ @new = map { $_->[0] }
			 sort { $b->[1] <=> $a->[1] || $a->[2] cmp $b->[2] }
			 map { [$_, /=(\d+)/, uc($_)] } @old; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 609 (eval 34):3) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <#> gv[*old] s
# 7  <1> rv2av[t19] lKM/1
# 8  <@> mapstart lK
# 9  <|> mapwhile(other->a)[t20] lKM
# a      <0> enter l
# b      <;> nextstate(main 608 (eval 34):2) v:{
# c      <0> pushmark s
# d      <#> gvsv[*_] s
# e      </> match(/"=(\\d+)"/) l
# f      <#> gvsv[*_] s
# g      <1> uc[t17] sK/1
# h      <@> anonlist sK*/1
# i      <@> leave lKP
#            goto 9
# j  <@> sort lKMS*
# k  <@> mapstart lK
# l  <|> mapwhile(other->m)[t26] lK
# m      <+> multideref($_->[0]) sK
#            goto l
# n  <0> pushmark s
# o  <#> gv[*new] s
# p  <1> rv2av[t2] lKRM*/1
# q  <2> aassign[t22] KS/COM_AGG
# r  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 609 (eval 34):3) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <0> pushmark s
# 6  <$> gv(*old) s
# 7  <1> rv2av[t10] lKM/1
# 8  <@> mapstart lK
# 9  <|> mapwhile(other->a)[t11] lKM
# a      <0> enter l
# b      <;> nextstate(main 608 (eval 34):2) v:{
# c      <0> pushmark s
# d      <$> gvsv(*_) s
# e      </> match(/"=(\\d+)"/) l
# f      <$> gvsv(*_) s
# g      <1> uc[t9] sK/1
# h      <@> anonlist sK*/1
# i      <@> leave lKP
#            goto 9
# j  <@> sort lKMS*
# k  <@> mapstart lK
# l  <|> mapwhile(other->m)[t12] lK
# m      <+> multideref($_->[0]) sK
#            goto l
# n  <0> pushmark s
# o  <$> gv(*new) s
# p  <1> rv2av[t1] lKRM*/1
# q  <2> aassign[t13] KS/COM_AGG
# r  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # using a prototype allows you to use any comparison subroutine
# as a sort subroutine (including other package's subroutines)
package other;
sub backwards ($$) { $_[1] cmp $_[0]; }     # $a and $b are not set here
package main;
@new = sort other::backwards @old;

=cut

checkOptree(name   => q{sort other::sub LIST },
	    bcopts => q{-exec},
	    code   => q{package other; sub backwards ($$) { $_[1] cmp $_[0]; }
			package main; @new = sort other::backwards @old; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 614 (eval 36):2) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const[PV "other::backwards"] s/BARE
# 5  <#> gv[*old] s
# 6  <1> rv2av[t4] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <#> gv[*new] s
# a  <1> rv2av[t2] lKRM*/1
# b  <2> aassign[t5] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 614 (eval 36):2) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const(PV "other::backwards") s/BARE
# 5  <$> gv(*old) s
# 6  <1> rv2av[t2] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <$> gv(*new) s
# a  <1> rv2av[t1] lKRM*/1
# b  <2> aassign[t3] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # repeat, condensed. $main::a and $b are unaffected
sub other::backwards ($$) { $_[1] cmp $_[0]; }
@new = sort other::backwards @old;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{sub other::backwards ($$) { $_[1] cmp $_[0]; } @new = sort other::backwards @old; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 619 (eval 38):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const[PV "other::backwards"] s/BARE
# 5  <#> gv[*old] s
# 6  <1> rv2av[t4] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <#> gv[*new] s
# a  <1> rv2av[t2] lKRM*/1
# b  <2> aassign[t5] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> const(PV "other::backwards") s/BARE
# 5  <$> gv(*old) s
# 6  <1> rv2av[t2] lKM/1
# 7  <@> sort lKS
# 8  <0> pushmark s
# 9  <$> gv(*new) s
# a  <1> rv2av[t1] lKRM*/1
# b  <2> aassign[t3] KS/COM_AGG
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # guarantee stability, regardless of algorithm
use sort 'stable';
@new = sort { substr($a, 3, 5) cmp substr($b, 3, 5) } @old;

=cut

my ($expect, $expect_nt) = (<<'EOT_EOT', <<'EONT_EONT');
# 1  <;> nextstate(main 656 (eval 40):1) v:%,{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*old] s
# 5  <1> rv2av[t9] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <#> gv[*new] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t14] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 578 (eval 15):1) v:%,{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*old) s
# 5  <1> rv2av[t5] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <$> gv(*new) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t6] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT


checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{use sort 'stable'; @new = sort { substr($a, 3, 5) cmp substr($b, 3, 5) } @old; },
	    expect => $expect, expect_nt => $expect_nt);

=for gentest

# chunk: # you should have a good reason to do this!
@articles = sort {$FooPack::b <=> $FooPack::a} @files;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@articles = sort {$FooPack::b <=> $FooPack::a} @files; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 667 (eval 44):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*files] s
# 5  <1> rv2av[t7] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <#> gv[*articles] s
# 9  <1> rv2av[t2] lKRM*/1
# a  <2> aassign[t8] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*files) s
# 5  <1> rv2av[t3] lKM/1
# 6  <@> sort lKS*
# 7  <0> pushmark s
# 8  <$> gv(*articles) s
# 9  <1> rv2av[t1] lKRM*/1
# a  <2> aassign[t4] KS/COM_AGG
# b  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # fancy
@result = sort { $a <=> $b } grep { $_ == $_ } @input;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{@result = sort { $a <=> $b } grep { $_ == $_ } @input; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 673 (eval 46):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <#> gv[*input] s
# 6  <1> rv2av[t9] lKM/1
# 7  <@> grepstart lK
# 8  <|> grepwhile(other->9)[t10] lK
# 9      <#> gvsv[*_] s
# a      <#> gvsv[*_] s
# b      <2> eq sK/2
#            goto 8
# c  <@> sort lK/NUM
# d  <0> pushmark s
# e  <#> gv[*result] s
# f  <1> rv2av[t2] lKRM*/1
# g  <2> aassign[t3] KS/COM_AGG
# h  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 547 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <0> pushmark s
# 5  <$> gv(*input) s
# 6  <1> rv2av[t3] lKM/1
# 7  <@> grepstart lK
# 8  <|> grepwhile(other->9)[t4] lK
# 9      <$> gvsv(*_) s
# a      <$> gvsv(*_) s
# b      <2> eq sK/2
#            goto 8
# c  <@> sort lK/NUM
# d  <0> pushmark s
# e  <$> gv(*result) s
# f  <1> rv2av[t1] lKRM*/1
# g  <2> aassign[t2] KS/COM_AGG
# h  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # void return context sort
sort { $a <=> $b } @input;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{sort { $a <=> $b } @input; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 678 (eval 48):1) v
# 2  <0> pushmark s
# 3  <#> gv[*input] s
# 4  <1> rv2av[t5] lK/1
# 5  <@> sort K/NUM
# 6  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v
# 2  <0> pushmark s
# 3  <$> gv(*input) s
# 4  <1> rv2av[t2] lK/1
# 5  <@> sort K/NUM
# 6  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # more void context, propagating ?
sort { $a <=> $b } grep { $_ == $_ } @input;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{sort { $a <=> $b } grep { $_ == $_ } @input; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 684 (eval 50):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*input] s
# 5  <1> rv2av[t7] lKM/1
# 6  <@> grepstart lK
# 7  <|> grepwhile(other->8)[t8] lK
# 8      <#> gvsv[*_] s
# 9      <#> gvsv[*_] s
# a      <2> eq sK/2
#            goto 7
# b  <@> sort K/NUM
# c  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 547 (eval 15):1) v
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*input) s
# 5  <1> rv2av[t2] lKM/1
# 6  <@> grepstart lK
# 7  <|> grepwhile(other->8)[t3] lK
# 8      <$> gvsv(*_) s
# 9      <$> gvsv(*_) s
# a      <2> eq sK/2
#            goto 7
# b  <@> sort K/NUM
# c  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: # scalar return context sort
$s = sort { $a <=> $b } @input;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{$s = sort { $a <=> $b } @input; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 689 (eval 52):1) v:{
# 2  <0> pushmark s
# 3  <#> gv[*input] s
# 4  <1> rv2av[t6] lK/1
# 5  <@> sort sK/NUM
# 6  <#> gvsv[*s] s
# 7  <2> sassign sKS/2
# 8  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 546 (eval 15):1) v:{
# 2  <0> pushmark s
# 3  <$> gv(*input) s
# 4  <1> rv2av[t2] lK/1
# 5  <@> sort sK/NUM
# 6  <$> gvsv(*s) s
# 7  <2> sassign sKS/2
# 8  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    

=for gentest

# chunk: $s = sort { $a <=> $b } grep { $_ == $_ } @input;

=cut

checkOptree(note   => q{},
	    bcopts => q{-exec},
	    code   => q{$s = sort { $a <=> $b } grep { $_ == $_ } @input; },
	    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT');
# 1  <;> nextstate(main 695 (eval 54):1) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <#> gv[*input] s
# 5  <1> rv2av[t8] lKM/1
# 6  <@> grepstart lK
# 7  <|> grepwhile(other->8)[t9] lK
# 8      <#> gvsv[*_] s
# 9      <#> gvsv[*_] s
# a      <2> eq sK/2
#            goto 7
# b  <@> sort sK/NUM
# c  <#> gvsv[*s] s
# d  <2> sassign sKS/2
# e  <1> leavesub[1 ref] K/REFC,1
EOT_EOT
# 1  <;> nextstate(main 547 (eval 15):1) v:{
# 2  <0> pushmark s
# 3  <0> pushmark s
# 4  <$> gv(*input) s
# 5  <1> rv2av[t2] lKM/1
# 6  <@> grepstart lK
# 7  <|> grepwhile(other->8)[t3] lK
# 8      <$> gvsv(*_) s
# 9      <$> gvsv(*_) s
# a      <2> eq sK/2
#            goto 7
# b  <@> sort sK/NUM
# c  <$> gvsv(*s) s
# d  <2> sassign sKS/2
# e  <1> leavesub[1 ref] K/REFC,1
EONT_EONT
    
