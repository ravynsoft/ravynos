#!./perl
#
# opcount.t
#
# Test whether various constructs have the right numbers of particular op
# types. This is chiefly to test that various optimisations are not
# inadvertently removed.
#
# For example the array access in sub { $a[0] } should get optimised from
# aelem into aelemfast. So we want to test that there are 1 aelemfast, 0
# aelem and 1 ex-aelem ops in the optree for that sub.

BEGIN {
    chdir 't';
    require './test.pl';
    skip_all_if_miniperl("No B under miniperl");
    @INC = '../lib';
}

use warnings;
use strict;

use B ();


{
    my %counts;

    # for a given op, increment $count{opname}. Treat null ops
    # as "ex-foo" where possible

    sub B::OP::test_opcount_callback {
        my ($op) = @_;
        my $name = $op->name;
        if ($name eq 'null') {
            my $targ = $op->targ;
            if ($targ) {
                $name = "ex-" . substr(B::ppname($targ), 3);
            }
        }
        $counts{$name}++;
    }

    # Given a code ref and a hash ref of expected op counts, check that
    # for each opname => count pair, whether that op appears that many
    # times in the op tree for that sub. If $debug is 1, display all the
    # op counts for the sub.

    sub test_opcount {
        my ($debug, $desc, $coderef, $expected_counts) = @_;

        %counts = ();
        B::walkoptree(B::svref_2object($coderef)->ROOT,
                        'test_opcount_callback');

        if ($debug) {
            note(sprintf "%3d %s", $counts{$_}, $_) for sort keys %counts;
        }

        my @exp;
        for (sort keys %$expected_counts) {
            my ($c, $e) = ($counts{$_}//0, $expected_counts->{$_});
            if ($c != $e) {
                push @exp, "expected $e, got $c: $_";
            }
        }
        ok(!@exp, $desc);
        if (@exp) {
            diag($_) for @exp;
        }
    }    
}

# aelem => aelemfast: a basic test that this test file works

test_opcount(0, "basic aelemfast",
                sub { our @a; $a[0] = 1 },
                {
                    aelem      => 0,
                    aelemfast  => 1,
                    'ex-aelem' => 1,
                }
            );

# Porting/bench.pl tries to create an empty and active loop, with the
# ops executed being exactly the same apart from the additional ops
# in the active loop. Check that this remains true.

{
    test_opcount(0, "bench.pl empty loop",
                sub { for my $x (1..$ARGV[0]) { 1; } },
                {
                     aelemfast => 1,
                     and       => 1,
                     const     => 1,
                     enteriter => 1,
                     iter      => 1,
                     leaveloop => 1,
                     leavesub  => 1,
                     lineseq   => 2,
                     nextstate => 2,
                     null      => 1,
                     pushmark  => 1,
                     unstack   => 1,
                }
            );

    no warnings 'void';
    test_opcount(0, "bench.pl active loop",
                sub { for my $x (1..$ARGV[0]) { $x; } },
                {
                     aelemfast => 1,
                     and       => 1,
                     const     => 1,
                     enteriter => 1,
                     iter      => 1,
                     leaveloop => 1,
                     leavesub  => 1,
                     lineseq   => 2,
                     nextstate => 2,
                     null      => 1,
                     padsv     => 1, # this is the additional active op
                     pushmark  => 1,
                     unstack   => 1,
                }
            );
}

#
# multideref
#
# try many permutations of aggregate lookup expressions

{
    package Foo;

    my (@agg_lex, %agg_lex, $i_lex, $r_lex);
    our (@agg_pkg, %agg_pkg, $i_pkg, $r_pkg);

    my $f;
    my @bodies = ('[0]', '[128]', '[$i_lex]', '[$i_pkg]',
                   '{foo}', '{$i_lex}', '{$i_pkg}',
                  );

    for my $prefix ('$f->()->', '$agg_lex', '$agg_pkg', '$r_lex->', '$r_pkg->')
    {
        for my $mod ('', 'local', 'exists', 'delete') {
            for my $body0 (@bodies) {
                for my $body1 ('', @bodies) {
                    for my $body2 ('', '[2*$i_lex]') {
                        my $code = "$mod $prefix$body0$body1$body2";
                        my $sub = "sub { $code }";
                        my $coderef = eval $sub
                            or die "eval '$sub': $@";

                        my %c = (aelem         => 0,
                                 aelemfast     => 0,
                                 aelemfast_lex => 0,
                                 exists        => 0,
                                 delete        => 0,
                                 helem         => 0,
                                 multideref    => 0,
                        );

                        my $top = 'aelem';
                        if ($code =~ /^\s*\$agg_...\[0\]$/) {
                            # we should expect aelemfast rather than multideref
                            $top = $code =~ /lex/ ? 'aelemfast_lex'
                                                  : 'aelemfast';
                            $c{$top} = 1;
                        }
                        else {
                            $c{multideref} = 1;
                        }

                        if ($body2 ne '') {
                            # trailing index; top aelem/exists/whatever
                            # node is kept
                            $top = $mod unless $mod eq '' or $mod eq 'local';
                            $c{$top} = 1
                        }

                        ::test_opcount(0, $sub, $coderef, \%c);
                    }
                }
            }
        }
    }
}


# multideref: ensure that the prefix expression and trailing index
# expression are optimised (include aelemfast in those expressions)


test_opcount(0, 'multideref expressions',
                sub { ($_[0] // $_)->[0]{2*$_[0]} },
                {
                    aelemfast  => 2,
                    helem      => 1,
                    multideref => 1,
                },
            );

# multideref with interesting constant indices


test_opcount(0, 'multideref const index',
                sub { $_->{1}{1.1} },
                {
                    helem      => 0,
                    multideref => 1,
                },
            );

use constant my_undef => undef;
test_opcount(0, 'multideref undef const index',
                sub { $_->{+my_undef} },
                {
                    helem      => 1,
                    multideref => 0,
                },
            );

# multideref when its the first op in a subchain

test_opcount(0, 'multideref op_other etc',
                sub { $_{foo} = $_ ? $_{bar} : $_{baz} },
                {
                    helem      => 0,
                    multideref => 3,
                },
            );

# multideref without hints

{
    no strict;
    no warnings;

    test_opcount(0, 'multideref no hints',
                sub { $_{foo}[0] },
                {
                    aelem      => 0,
                    helem      => 0,
                    multideref => 1,
                },
            );
}

# exists shouldn't clash with aelemfast

test_opcount(0, 'multideref exists',
                sub { exists $_[0] },
                {
                    aelem      => 0,
                    aelemfast  => 0,
                    multideref => 1,
                },
            );

test_opcount(0, 'barewords can be constant-folded',
             sub { no strict 'subs'; FOO . BAR },
             {
                 concat => 0,
             });

{
    use feature 'signatures';

    my @a;
    test_opcount(0, 'signature default expressions get optimised',
                 sub ($s = $a[0]) {},
                 {
                     aelem         => 0,
                     aelemfast_lex => 1,
                 });
}

# in-place sorting

{
    local our @global = (3,2,1);
    my @lex = qw(a b c);

    test_opcount(0, 'in-place sort of global',
                 sub { @global = sort @global; 1 },
                 {
                     rv2av   => 1,
                     aassign => 0,
                 });

    test_opcount(0, 'in-place sort of lexical',
                 sub { @lex = sort @lex; 1 },
                 {
                     padav   => 1,
                     aassign => 0,
                 });

    test_opcount(0, 'in-place reversed sort of global',
                 sub { @global = sort { $b <=> $a } @global; 1 },
                 {
                     rv2av   => 1,
                     aassign => 0,
                 });


    test_opcount(0, 'in-place custom sort of global',
                 sub { @global = sort {  $a<$b?1:$a>$b?-1:0 } @global; 1 },
                 {
                     rv2av   => 1,
                     aassign => 0,
                 });

    sub mysort { $b cmp $a };
    test_opcount(0, 'in-place sort with function of lexical',
                 sub { @lex = sort mysort @lex; 1 },
                 {
                     padav   => 1,
                     aassign => 0,
                 });


}

# in-place assign optimisation for @a = split

{
    local our @pkg;
    my @lex;

    for (['@pkg',       0, ],
         ['local @pkg', 0, ],
         ['@lex',       0, ],
         ['my @a',      0, ],
         ['@{[]}',      1, ],
    ){
        # partial implies that the aassign has been optimised away, but
        # not the rv2av
        my ($code, $partial) = @$_;
        test_opcount(0, "in-place assignment for split: $code",
                eval qq{sub { $code = split }},
                {
                    padav   => 0,
                    rv2av   => $partial,
                    aassign => 0,
                });
    }
}

# index(...) == -1 and variants optimise away the EQ/NE/etc and CONST
# and with $lex = (index(...) == -1), the assignment is optimised away
# too

{
    local our @pkg;
    my @lex;

    my ($x, $y, $z);
    for my $assign (0, 1) {
        for my $index ('index($x,$y)', 'rindex($x,$y)') {
            for my $fmt (
                    "%s <= -1",
                    "%s == -1",
                    "%s != -1",
                    "%s >  -1",

                    "%s <  0",
                    "%s >= 0",

                    "-1 <  %s",
                    "-1 == %s",
                    "-1 != %s",
                    "-1 >= %s",

                    " 0 <= %s",
                    " 0 >  %s",

            ) {
                my $expr = sprintf $fmt, $index;
                $expr = "\$z = ($expr)" if $assign;

                test_opcount(0, "optimise away compare,const in $expr",
                        eval qq{sub { $expr }},
                        {
                            lt      => 0,
                            le      => 0,
                            eq      => 0,
                            ne      => 0,
                            ge      => 0,
                            gt      => 0,
                            const   => 0,
                            sassign => 0,
                            padsv   => 2.
                        });
            }
        }
    }
}


# a sprintf that can't be optimised shouldn't stop the .= concat being
# optimised

{
    my ($i,$j,$s);
    test_opcount(0, "sprintf pessimised",
        sub { $s .= sprintf "%d%d",$i, $j },
        {
            const       => 1,
            sprintf     => 1,
            concat      => 0,
            multiconcat => 1,
            padsv       => 2,
        });
}


# sprintf with constant args should be constant folded

test_opcount(0, "sprintf constant args",
        sub { sprintf "%s%s", "abc", "def" },
        {
            const       => 1,
            sprintf     => 0,
            multiconcat => 0.
        });

#
# concats and assigns that should be optimised into a single multiconcat
# op

{

    my %seen; # weed out duplicate combinations

    # these are the ones where using multiconcat isn't a gain, so should
    # be pessimised
    my %pessimise = map { $_ => 1 }
                        '$a1.$a2',
                        '"$a1$a2"',
                        '$pkg .= $a1',
                        '$pkg .= "$a1"',
                        '$lex  = $a1.$a2',
                        '$lex  = "$a1$a2"',
                        # these already constant folded
                        'sprintf("-")',
                        '$pkg  = sprintf("-")',
                        '$lex  = sprintf("-")',
                        'my $l = sprintf("-")',
                    ;

    for my $lhs (
        '',
        '$pkg  = ',
        '$pkg .= ',
        '$lex  = ',
        '$lex .= ',
        'my $l = ',
    ) {
        for my $nargs (0..3) {
            for my $type (0..2) {
                # 0: $a . $b
                # 1: "$a$b"
                # 2: sprintf("%s%s", $a, $b)

                for my $const (0..4) {
                    # 0: no consts:       "$a1$a2"
                    # 1: interior consts: "$a1-$a2"
                    # 2: + LH   edge:    "-$a1-$a2"
                    # 3: + RH   edge:     "$a1-$a2-"
                    # 4: + both edge:    "-$a1-$a2-"

                    my @args;
                    my @sprintf_args;
                    my $c = $type == 0 ? '"-"' : '-';
                    push @args, $c if $const == 2 || $const == 4;
                    for my $n (1..$nargs) {
                        if ($type == 2) {
                            # sprintf
                            push @sprintf_args, "\$a$n";
                            push @args, '%s';
                        }
                        else {
                            push @args, "\$a$n";
                        }
                        push @args, $c if $const;
                    }
                    pop @args if  $const == 1 || $const == 2;

                    push @args, $c if $nargs == 0 && $const == 1;


                    if ($type == 2) {
                        # sprintf
                        next unless @args;
                    }
                    else {
                        # To ensure that there's at least once concat
                        # action, if appending, need at least one RHS arg;
                        # else least 2 args:
                        #    $x = $a . $b
                        #    $x .= $a
                        next unless @args >= ($lhs =~ /\./ ? 1 : 2);
                    }

                    my $rhs;
                    if ($type == 0) {
                        $rhs = join('.', @args);
                    }
                    elsif ($type == 1) {
                        $rhs = '"' . join('',  @args) . '"'
                    }
                    else {
                        $rhs = 'sprintf("'
                               . join('',  @args)
                               . '"'
                               . join('', map ",$_",  @sprintf_args)
                               . ')';
                    }

                    my $expr = $lhs . $rhs;

                    next if exists $seen{$expr};
                    $seen{$expr} = 1;

                    my ($a1, $a2, $a3);
                    my $lex;
                    our $pkg;
                    my $sub = eval qq{sub { $expr }};
                    die "eval(sub { $expr }: $@" if $@;

                    my $pm = $pessimise{$expr};
                    test_opcount(0, ($pm ? "concat     " : "multiconcat")
                                            . ": $expr",
                            $sub,
                            $pm
                            ?   {   multiconcat => 0 }
                            :   {
                                    multiconcat => 1,
                                    padsv       => $nargs,
                                    concat      => 0,
                                    sprintf     => 0,
                                    const       => 0,
                                    sassign     => 0,
                                    stringify   => 0,
                                    gv          => 0, # optimised to gvsv
                                });
                }
            }
        }
    }
}

# $lex = "foo" should *not* get converted into a multiconcat - there's
# no actual concatenation involved, and treating it as a degnerate concat
# would forego any COW copy efficiency

test_opcount(0, '$lex = "foo"', sub { my $x; $x = "foo"; },
        {
            multiconcat => 0,
        });

# for '$lex1 = $lex2 . $lex3', multiconcat is normally slower than
# concat, except in the specific case of '$lex1 = $lex2 . $lex1'

test_opcount(0, '$lex1 = $lex2 . $lex1', sub { my ($x,$y); $x = $y . $x },
            {
                multiconcat => 1,
                padsv       => 4, # 2 are from the my()
                concat      => 0,
                sassign     => 0,
                stringify   => 0,
            });
test_opcount(0, '$lex1 = "$lex2$lex1"', sub { my ($x,$y); $x = "$y$x" },
            {
                multiconcat => 1,
                padsv       => 4, # 2 are from the my()
                concat      => 0,
                sassign     => 0,
                stringify   => 0,
            });
test_opcount(0, '$lex1 = $lex1 . $lex1', sub { my $x; $x = $x . $x },
            {
                multiconcat => 0,
            });

# 'my $x .= ...' doesn't make a lot of sense and so isn't optimised
test_opcount(0, 'my $a .= $b.$c.$d', sub { our ($b,$c,$d); my $a .= $b.$c.$d },
            {
                padsv => 1,
            });

# prefer rcatline optimisation over multiconcat

test_opcount(0, "rcatline", sub { my ($x,$y); open FOO, "xxx"; $x .= <FOO> },
        {
            rcatline    => 1,
            readline    => 0,
            multiconcat => 0,
            concat      => 0,
        });

# long chains of concats should be converted into chained multiconcats

{
    my @a;
    for my $i (60..68) { # check each side of 64 threshold
        my $c = join '.', map "\$a[$_]", 1..$i;
        my $sub = eval qq{sub { $c }} or die $@;
        test_opcount(0, "long chain $i", $sub,
            {
                multiconcat => $i > 65 ? 2 : 1,
                concat      => $i == 65 ? 1 : 0,
                aelem       => 0,
                aelemfast   => 0,
            });
    }
}

# with C<$state $s = $a . $b . ....>, the assign is optimised away,
# but the padsv isn't (it's treated like a general LHS expression rather
# than using OPpTARGET_MY).

test_opcount(0, "state works with multiconcat",
                sub { use feature 'state'; our ($a, $b, $c); state $s = $a . $b . $c },
                {
                    multiconcat => 1,
                    concat      => 0,
                    sassign     => 0,
                    once        => 1,
                    padsv       => 2, # one each for the next/once branches
                });

# multiple concats of constants preceded by at least one non-constant
# shouldn't get constant-folded so that a concat overload method is called
# for each arg. So every second constant string is left as an OP_CONST

test_opcount(0, "multiconcat: 2 adjacent consts",
                sub { my ($a, $b); $a = $b . "c" . "d" },
                {
                    const       => 1,
                    multiconcat => 1,
                    concat      => 0,
                    sassign     => 0,
                });
test_opcount(0, "multiconcat: 3 adjacent consts",
                sub { my ($a, $b); $a = $b . "c" . "d" . "e" },
                {
                    const       => 1,
                    multiconcat => 1,
                    concat      => 0,
                    sassign     => 0,
                });
test_opcount(0, "multiconcat: 4 adjacent consts",
                sub { my ($a, $b); $a = $b . "c" . "d" . "e" ."f" },
                {
                    const       => 2,
                    multiconcat => 1,
                    concat      => 0,
                    sassign     => 0,
                });

# multiconcat shouldn't include the assign if the LHS has 'local'

test_opcount(0, "multiconcat: local assign",
                sub { our $global; local $global = "$global-X" },
                {
                    const       => 0,
                    gvsv        => 2,
                    multiconcat => 1,
                    concat      => 0,
                    sassign     => 1,
                });

{
    use feature 'try';
    no warnings 'experimental::try';

    test_opcount(0, "try/catch: catch block is optimized",
                    sub { my @a; try {} catch($e) { $a[0] } },
                    {
                        aelemfast_lex => 1,
                        aelem         => 0,
                    });
}

{
    use feature 'defer';
    no warnings 'experimental::defer';

    test_opcount(0, "pushdefer: block is optimized",
                    sub { my @a; defer { $a[0] } },
                    {
                        aelemfast_lex => 1,
                        aelem         => 0,
                    });
}

# builtin:: function calls should be replaced with efficient op implementations
no warnings 'experimental::builtin';

test_opcount(0, "builtin::true/false are replaced with constants",
                sub { my $x = builtin::true(); my $y = builtin::false() },
                {
                    entersub => 0,
                    const    => 2,
                });

test_opcount(0, "builtin::is_bool is replaced with direct opcode",
                sub { my $x; my $y; $y = builtin::is_bool($x); },
                {
                    entersub => 0,
                    is_bool  => 1,
                    padsv    => 3,
                    padsv_store  => 1,
                });

test_opcount(0, "builtin::is_bool gets constant-folded",
                sub { builtin::is_bool(123); },
                {
                    entersub => 0,
                    is_bool  => 0,
                    const    => 1,
                });

test_opcount(0, "builtin::weaken is replaced with direct opcode",
                sub { my $x = []; builtin::weaken($x); },
                {
                    entersub => 0,
                    weaken   => 1,
                });

test_opcount(0, "builtin::unweaken is replaced with direct opcode",
                sub { my $x = []; builtin::unweaken($x); },
                {
                    entersub => 0,
                    unweaken => 1,
                });

test_opcount(0, "builtin::is_weak is replaced with direct opcode",
                sub { builtin::is_weak([]); },
                {
                    entersub => 0,
                    is_weak  => 1,
                });

test_opcount(0, "builtin::blessed is replaced with direct opcode",
                sub { builtin::blessed([]); },
                {
                    entersub => 0,
                    blessed  => 1,
                });

test_opcount(0, "builtin::refaddr is replaced with direct opcode",
                sub { builtin::refaddr([]); },
                {
                    entersub => 0,
                    refaddr  => 1,
                });

test_opcount(0, "builtin::reftype is replaced with direct opcode",
                sub { builtin::reftype([]); },
                {
                    entersub => 0,
                    reftype  => 1,
                });

my $one_point_five = 1.5;   # Prevent const-folding.
test_opcount(0, "builtin::ceil is replaced with direct opcode",
                sub { builtin::ceil($one_point_five); },
                {
                    entersub => 0,
                    ceil     => 1,
                });

test_opcount(0, "builtin::floor is replaced with direct opcode",
                sub { builtin::floor($one_point_five); },
                {
                    entersub => 0,
                    floor    => 1,
                });

test_opcount(0, "builtin::is_tainted is replaced with direct opcode",
                sub { builtin::is_tainted($0); },
                {
                    entersub   => 0,
                    is_tainted => 1,
                });

# sassign + padsv combinations are replaced by padsv_store
test_opcount(0, "sassign + padsv replaced by padsv_store",
                sub { my $y; my $z = $y = 3; },
                {
                    padsv        => 1,
                    padsv_store  => 2,
                });

# OPpTARGET_MY optimizations on undef
test_opcount(0, "undef + padsv (undef my \$x) is reduced to undef",
                sub { undef my $x },
                {
                    undef       => 1,
                    padsv       => 0,
                    padsv_store => 0,
                    sassign     => 0,
                });
test_opcount(0, "undef + padsv + sassign (my \$x = undef) is reduced to undef",
                sub { my $x = undef },
                {
                    undef       => 1,
                    padsv       => 0,
                    padsv_store => 0,
                    sassign     => 0,
                });
test_opcount(0, "undef + padsv (undef \$x) is reduced to undef",
                sub { my $x; undef $x },
                {
                    undef       => 1,
                    padsv       => 1,
                    padsv_store => 0,
                    sassign     => 0,
                });
test_opcount(0, "undef + padsv + sassign (\$x = undef) is reduced to undef",
                sub { my $x; $x = undef },
                {
                    undef       => 1,
                    padsv       => 1,
                    padsv_store => 0,
                    sassign     => 0,
                });
# Additional test cases requested by demerphq
test_opcount(0, 'my $y= 1; my @x= ($y= undef);',
                sub { my $y= 1; my @x= ($y= undef); },
                {
                    undef       => 1,
                    aassign     => 1,
                    padav       => 1,
                    padsv       => 0,
                    padsv_store => 1,
                    sassign     => 0,
                });

test_opcount(0, 'my $x= 1; sub f{} f($x=undef);',
                sub { my $x= 1; sub f{} f($x=undef); },
                {
                    undef       => 1,
                    gv          => 1,
                    padsv       => 0,
                    padsv_store => 1,
                    sassign     => 0,
                });

test_opcount(0, 'my ($x,$p)=(1,2); sub g{} g(($x=undef),$p);',
                sub { my ($x,$p)=(1,2); sub g{} g(($x=undef),$p); },
                {
                    undef       => 1,
                    aassign     => 1,
                    gv          => 1,
                    padrange    => 1,
                    padsv       => 3,
                    padsv_store => 0,
                    sassign     => 0,
                });

test_opcount(0, 'my $h= {}; my @k= keys %{($h=undef)||{}};',
                sub { my $h= {}; my @k= keys %{($h=undef)||{}}; },
                {
                    undef       => 1,
                    aassign     => 1,
                    emptyavhv   => 2,
                    padav       => 1,
                    padsv       => 0,
                    padsv_store => 0,
                    sassign     => 0,
                });

test_opcount(0, 'my $y= 1; my @x= \($y= undef);',
                sub { my $y= 1; my @x= \($y= undef); },
                {
                    undef       => 1,
                    aassign     => 1,
                    padav       => 1,
                    padsv       => 0,
                    padsv_store => 1,
                    sassign     => 0,
                    srefgen     => 1,
                });

# aelemfast_lex + sassign are replaced by a combined OP
test_opcount(0, "simple aelemfast_lex + sassign replacement",
                sub { my @x; $x[0] = "foo" },
                {
                    aelemfast_lex      => 0,
                    aelemfastlex_store => 1,
                    padav              => 1,
                    sassign            => 0,
                });

# aelemfast_lex + sassign are not replaced by a combined OP
# when key <0 (not handled, to keep the pp_ function simple
test_opcount(0, "aelemfast_lex + sassign replacement with neg key",
                sub { my @x = (1,2); $x[-1] = 7 },
                {
                    aelemfast_lex      => 0,
                    aelemfastlex_store => 1,
                    padav              => 1,
                    sassign            => 0,
                });

# aelemfast_lex + sassign optimization does not disrupt multideref
test_opcount(0, "no aelemfast_lex + sassign replacement with multideref",
                sub { my @x = ([1,2]); $x[0][1] = 1; },
                {
                    aelemfast_lex      => 0,
                    aelemfastlex_store => 0,
                    multideref         => 1,
                    padav              => 1,
                    sassign            => 1,
                });

# emptyavhv optimizations

test_opcount(0, "Empty anonlist",
                sub { [] },
                {
                    anonlist  => 0,
                    emptyavhv => 1,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonlist with global assignment",
                sub { our $x; $x = [] },
                {
                    anonlist  => 0,
                    emptyavhv => 1,
                    gvsv      => 1,
                    pushmark  => 0,
                    sassign   => 1,
                });
test_opcount(0, "Empty anonlist and lexical assignment",
                sub { my $x; $x = [] },
                {
                    anonlist  => 0,
                    emptyavhv => 1,
                    padsv     => 1,
                    pushmark  => 0,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonlist and direct lexical assignment",
                sub { my $x = [] },
                {
                    anonlist  => 0,
                    emptyavhv => 1,
                    padsv     => 0,
                    pushmark  => 0,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonlist ref and direct lexical assignment",
                sub { my $x = \[] },
                {
                    anonlist    => 0,
                    emptyavhv   => 1,
                    padsv       => 0,
                    padsv_store => 1,
                    pushmark    => 0,
                    sassign     => 0,
                    srefgen     => 1,
                });
test_opcount(0, "Empty anonhash",
                sub { {} },
                {
                    anonhash  => 0,
                    emptyavhv => 1,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonhash with global assignment",
                sub { our $x; $x = {} },
                {
                    anonhash  => 0,
                    emptyavhv => 1,
                    gvsv      => 1,
                    pushmark  => 0,
                    sassign   => 1,
                });
test_opcount(0, "Empty anonhash and lexical assignment",
                sub { my $x; $x = {} },
                {
                    anonhash  => 0,
                    emptyavhv => 1,
                    padsv     => 1,
                    pushmark  => 0,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonhash and direct lexical assignment",
                sub { my $x = {} },
                {
                    anonhash  => 0,
                    emptyavhv => 1,
                    padsv     => 0,
                    pushmark  => 0,
                    sassign   => 0,
                });
test_opcount(0, "Empty anonhash ref and direct lexical assignment",
                sub { my $x = \{} },
                {
                    anonhash    => 0,
                    emptyavhv   => 1,
                    padsv       => 0,
                    padsv_store => 1,
                    pushmark    => 0,
                    sassign     => 0,
                    srefgen     => 1,
                });

done_testing();
