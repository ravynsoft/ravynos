#!./perl

# Use B to test that optimisations are not inadvertently removed,
# by examining particular nodes in the optree.

use warnings;
use strict;

BEGIN {
    chdir 't';
    require './test.pl';
    skip_all_if_miniperl("No B under miniperl");
    @INC = '../lib';
}

plan 2285;

use v5.10; # state
use B qw(svref_2object
         OPpASSIGN_COMMON_SCALAR
         OPpASSIGN_COMMON_RC1
         OPpASSIGN_COMMON_AGG
         OPpTRUEBOOL
         OPpMAYBE_TRUEBOOL
         OPpASSIGN_TRUEBOOL
      );

# for debugging etc. Basic dump of an optree

sub dump_optree {
    my ($o, $depth) = @_;

    return '' unless $$o;
    # use Devel::Peek; Dump $o;
    my $s = ("  " x $depth) . $o->name . "\n";
    my $n = eval { $o->first };
    while ($n && $$n) {
        $s .= dump_optree($n, $depth+1);
        $n = $n->sibling;
    }
    $s;
}



# Test that OP_AASSIGN gets the appropriate
# OPpASSIGN_COMMON* flags set.
#
# Too few flags set is likely to cause code to misbehave;
# too many flags set unnecessarily slows things down.
# See also the tests in t/op/aassign.t

for my $test (
    # Each anon array contains:
    # [
    #   expected flags:
    #      a 3 char string, each char showing whether we expect a
    #      particular flag to be set:
    #           '-' indicates any char not set, while
    #           'S':  char 0: OPpASSIGN_COMMON_SCALAR,
    #           'R':  char 1: OPpASSIGN_COMMON_RC1,
    #           'A'   char 2: OPpASSIGN_COMMON_AGG,
    #   code to eval,
    #   description,
    # ]

    [ "---", '() = (1, $x, my $y, @z, f($p))', 'no LHS' ],
    [ "---", '(undef, $x, my $y, @z, ($a ? $b : $c)) = ()', 'no RHS' ],
    [ "---", '(undef, $x, my $y, @z, ($a ? $b : $c)) = (1,2)', 'safe RHS' ],
    [ "---", 'my @a = (1,2)', 'safe RHS: my array' ],
    [ "---", 'my %h = (1,2)', 'safe RHS: my hash' ],
    [ "---", 'my ($a,$b,$c,$d) = 1..6; ($a,$b) = ($c,$d);', 'non-common lex' ],
    [ "---", '($x,$y) = (1,2)', 'pkg var LHS only' ],
    [ "---", 'my $p; my ($x,$y) = ($p, $p)', 'my; dup lex var on RHS' ],
    [ "---", 'my $p; my ($x,$y); ($x,$y) = ($p, $p)', 'dup lex var on RHS' ],
    [ "---", 'my ($self) = @_', 'LHS lex scalar only' ],
    [ "--A", 'my ($self, @rest) = @_', 'LHS lex mixed' ],
    [ "-R-", 'my ($x,$y) = ($p, $q)', 'pkg var RHS only' ],
    [ "S--", '($x,$y) = ($p, $q)', 'pkg scalar both sides' ],
    [ "--A", 'my (@a, @b); @a = @b', 'lex ary both sides' ],
    [ "-R-", 'my ($x,$y,$z,@a); ($x,$y,$z) = @a ', 'lex vars to lex ary' ],
    [ "--A", '@a = @b', 'pkg ary both sides' ],
    [ "--A", 'my (%a,%b); %a = %b', 'lex hash both sides' ],
    [ "--A", '%a = %b', 'pkg hash both sides' ],
    [ "--A", 'my $x; @a = ($a[0], $a[$x])', 'common ary' ],
    [ "--A", 'my ($x,@a); @a = ($a[0], $a[$x])', 'common lex ary' ],
    [ "S-A", 'my $x; ($a[$x], $a[0]) = ($a[0], $a[$x])', 'common ary elems' ],
    [ "S-A", 'my ($x,@a); ($a[$x], $a[0]) = ($a[0], $a[$x])',
                                                    'common lex ary elems' ],
    [ "--A", 'my $x; my @a = @$x', 'lex ary may have stuff' ],
    [ "-RA", 'my $x; my ($b, @a) = @$x', 'lex ary may have stuff' ],
    [ "--A", 'my $x; my %a = @$x', 'lex hash may have stuff' ],
    [ "-RA", 'my $x; my ($b, %a) = @$x', 'lex hash may have stuff' ],
    [ "--A", 'my (@a,@b); @a = ($b[0])', 'lex ary and elem' ],
    [ "S-A", 'my @a; ($a[1],$a[0]) = @a', 'lex ary and elem' ],
    [ "--A", 'my @x; @y = $x[0]', 'pkg ary from lex elem' ],
    [ "---", '(undef,$x) = f()', 'single scalar on LHS' ],
    [ "---", '($x,$y) = ($x)', 'single scalar on RHS, no AGG' ],
    [ "--A", '($x,@b) = ($x)', 'single scalar on RHS' ],
    [ "--A", 'my @a; @a = (@a = split())',      'split a/a'   ],
    [ "--A", 'my (@a,@b); @a = (@b = split())', 'split a/b'   ],
    [ "---", 'my @a; @a = (split(), 1)',        '(split(),1)' ],
    [ "---", '@a = (split(//, @a), 1)',         'split(@a)'   ],
    [ "--A", 'my @a; my $ar = @a; @a = (@$ar = split())', 'a/ar split'  ],
) {

    my ($exp, $code, $desc) = @$test;
    my $sub;
    {
        # package vars used in code snippets
        our (@a, %a, @b, %b, $c, $p, $q, $x, $y, @y, @z);

        $sub = eval "sub { $code }"
            or die
                "aassign eval('$code') failed: this test needs"
                . "to be rewritten:\n$@"
    }

    my $last_expr = svref_2object($sub)->ROOT->first->last;
    if ($last_expr->name ne 'aassign') {
        die "Expected aassign but found ", $last_expr->name,
            "; this test needs to be rewritten" 
    }
    my $got =
        (($last_expr->private & OPpASSIGN_COMMON_SCALAR) ? 'S' : '-')
      . (($last_expr->private & OPpASSIGN_COMMON_RC1)    ? 'R' : '-')
      . (($last_expr->private & OPpASSIGN_COMMON_AGG)    ? 'A' : '-');
    is $got, $exp,  "OPpASSIGN_COMMON: $desc: '$code'";
}    


# join -> stringify/const

for (['CONSTANT', sub {          join "foo", $_ }],
     ['$var'    , sub {          join  $_  , $_ }],
     ['$myvar'  , sub { my $var; join  $var, $_ }],
) {
    my($sep,$sub) = @$_;
    my $last_expr = svref_2object($sub)->ROOT->first->last;
    is $last_expr->name, 'stringify',
      "join($sep, \$scalar) optimised to stringify";
}

for (['CONSTANT', sub {          join "foo", "bar"    }, 0, "bar"    ],
     ['CONSTANT', sub {          join "foo", "bar", 3 }, 1, "barfoo3"],
     ['$var'    , sub {          join  $_  , "bar"    }, 0, "bar"    ],
     ['$myvar'  , sub { my $var; join  $var, "bar"    }, 0, "bar"    ],
) {
    my($sep,$sub,$is_list,$expect) = @$_;
    my $last_expr = svref_2object($sub)->ROOT->first->last;
    my $tn = "join($sep, " . ($is_list?'list of constants':'const') . ")";
    is $last_expr->name, 'const', "$tn optimised to constant";
    is $sub->(), $expect, "$tn folded correctly";
}


# list+pushmark in list context elided out of the execution chain
is svref_2object(sub { () = ($_, ($_, $_)) })
    ->START # nextstate
    ->next  # pushmark
    ->next  # gvsv
    ->next  # should be gvsv, not pushmark
  ->name, 'gvsv',
  "list+pushmark in list context where list's elder sibling is a null";


# nextstate multiple times becoming one nextstate

is svref_2object(sub { 0;0;0;0;0;0;time })->START->next->name, 'time',
  'multiple nextstates become one';


# pad[ahs]v state declarations in void context 

is svref_2object(sub{state($foo,@fit,%far);state $bar;state($a,$b); time})
    ->START->next->name, 'time',
  'pad[ahs]v state declarations in void context';


# pushmark-padsv-padav-padhv in list context --> padrange

{
    my @ops;
    my $sub = sub { \my( $f, @f, %f ) };
    my $op = svref_2object($sub)->START;
    push(@ops, $op->name), $op = $op->next while $$op;
    is "@ops", "nextstate padrange refgen leavesub", 'multi-type padrange'
}


# rv2[ahs]v in void context

is svref_2object(sub { our($foo,@fit,%far); our $bar; our($a,$b); time })
    ->START->next->name, 'time',
  'rv2[ahs]v in void context';


# split to array

for(['@pkgary'      , '@_'       ],
    ['@lexary'      , 'my @a; @a'],
    ['my(@array)'   , 'my(@a)'   ],
    ['local(@array)', 'local(@_)'],
    ['@{...}'       , '@{\@_}'   ],
){
    my($tn,$code) = @$_;
    my $sub = eval "sub { $code = split }";
    my $split = svref_2object($sub)->ROOT->first->last;
    is $split->name, 'split', "$tn = split swallows up the assignment";
}


# stringify with join kid --> join
is svref_2object(sub { "@_" })->ROOT->first->last->name, 'join',
  'qq"@_" optimised from stringify(join(...)) to join(...)';


# Check that certain ops, when in boolean context, have the
# right private "is boolean" or "maybe boolean" flags set.
#
# A maybe flag is set when the context at the end of a chain of and/or/dor
# ops isn't known till runtime, e.g.
#   sub f { ....; ((%h || $x) || $y)) }
# If f() is called in void context, then %h can return a boolean value;
# if in scalar context, %h must return a key count.

for my $ops (
    #  op          code        op_path flag               maybe_flag
    #  ---------   ----------  ------- -----------------  ----------------
    [ 'aassign',  '(@pkg = @lex)',[],  OPpASSIGN_TRUEBOOL,0,                ],
    [ 'grepwhile','grep($_,1)',   [],  OPpTRUEBOOL,       0,                ],
    [ 'length',   'length($x)',   [],  OPpTRUEBOOL,       0,                ],
    [ 'padav',    '@lex',         [],  OPpTRUEBOOL,       0,                ],
    [ 'padav',    'scalar @lex',  [0], OPpTRUEBOOL,       0,                ],
    [ 'padhv',    '%lex',         [],  OPpTRUEBOOL,       OPpMAYBE_TRUEBOOL ],
    [ 'padhv',    'scalar(%lex)', [0], OPpTRUEBOOL,       OPpMAYBE_TRUEBOOL ],
    [ 'pos',      'pos($x)',      [],  OPpTRUEBOOL,       0,                ],
    [ 'ref',      'ref($x)',      [],  OPpTRUEBOOL,       OPpMAYBE_TRUEBOOL ],
    [ 'rv2av',    '@pkg',         [],  OPpTRUEBOOL,       0,                ],
    [ 'rv2av',    'scalar(@pkg)', [0], OPpTRUEBOOL,       0,                ],
    [ 'rv2hv',    '%pkg',         [],  OPpTRUEBOOL,       OPpMAYBE_TRUEBOOL ],
    [ 'rv2hv',    'scalar(%pkg)', [0], OPpTRUEBOOL,       OPpMAYBE_TRUEBOOL ],
    [ 'subst',    's/a/b/',       [],  OPpTRUEBOOL,       0,                ],
) {
    my ($op_name, $op_code, $post_op_path, $bool_flag, $maybe_flag) = @$ops;

    for my $test (
        # 1st column: what to expect for each $context (void, scalar, unknown),
        #                0: expect no flag
        #                1: expect bool flag
        #                2: expect maybe bool flag
        #                9: skip test
        #  2nd column: path though the op subtree to the flagged op:
        #                0 is first child, 1 is second child etc.
        #                Will have @$post_op_path from above appended.
        #  3rd column: code to execute: %s holds the code for the op
        #
        # [V S U]  PATH        CODE

        # INNER PLAIN

        [ [0,0,0], [],        '%s'                               ],
        [ [1,9,1], [0,0],     'if (%s) {$x}'                     ],
        [ [1,9,1], [0,0],     'if (%s) {$x} else {$y}'           ],
        [ [1,9,2], [0,0],     'unless (%s) {$x}'                 ],

        # INNER NOT

        [ [1,1,1], [0],       '!%s'                              ],
        [ [1,9,1], [0,0,0],   'if (!%s) {$x}'                    ],
        [ [1,9,1], [0,0,0],   'if (!%s) {$x} else {$y}'          ],
        [ [1,9,1], [0,0,0],   'unless (!%s) {$x}'                ],

        # INNER COND

        [ [1,1,1], [0,0,],    '%s ? $p : $q'                     ],
        [ [1,9,1], [0,0,0,0], 'if (%s ? $p : $q) {$x}'           ],
        [ [1,9,1], [0,0,0,0], 'if (%s ? $p : $q) {$x} else {$y}' ],
        [ [1,9,1], [0,0,0,0], 'unless (%s ? $p : $q) {$x}'       ],


        # INNER OR LHS

        [ [1,0,2], [0,0],     '%s || $x'                         ],
        [ [1,1,1], [0,0,0],   '!(%s || $x)'                      ],
        [ [1,0,2], [0,1,0,0], '$y && (%s || $x)'                 ],
        [ [1,9,1], [0,0,0,0], 'if (%s || $x) {$x}'               ],
        [ [1,9,1], [0,0,0,0], 'if (%s || $x) {$x} else {$y}'     ],
        [ [1,9,2], [0,0,0,0], 'unless (%s || $x) {$x}'           ],

        # INNER OR RHS

        [ [0,0,0], [0,1],     '$x || %s'                         ],
        [ [1,1,1], [0,0,1],   '!($x || %s)'                      ],
        [ [0,0,0], [0,1,0,1], '$y && ($x || %s)'                 ],
        [ [1,9,1], [0,0,0,1], 'if ($x || %s) {$x}'               ],
        [ [1,9,1], [0,0,0,1], 'if ($x || %s) {$x} else {$y}'     ],
        [ [1,9,2], [0,0,0,1], 'unless ($x || %s) {$x}'           ],

        # INNER DOR LHS

        [ [1,0,2], [0,0],     '%s // $x'                         ],
        [ [1,1,1], [0,0,0],   '!(%s // $x)'                      ],
        [ [1,0,2], [0,1,0,0], '$y && (%s // $x)'                 ],
        [ [1,9,1], [0,0,0,0], 'if (%s // $x) {$x}'               ],
        [ [1,9,1], [0,0,0,0], 'if (%s // $x) {$x} else {$y}'     ],
        [ [1,9,2], [0,0,0,0], 'unless (%s // $x) {$x}'           ],

        # INNER DOR RHS

        [ [0,0,0], [0,1],     '$x // %s'                         ],
        [ [1,1,1], [0,0,1],   '!($x // %s)'                      ],
        [ [0,0,0], [0,1,0,1], '$y && ($x // %s)'                 ],
        [ [1,9,1], [0,0,0,1], 'if ($x // %s) {$x}'               ],
        [ [1,9,1], [0,0,0,1], 'if ($x // %s) {$x} else {$y}'     ],
        [ [1,9,2], [0,0,0,1], 'unless ($x // %s) {$x}'           ],

        # INNER AND LHS

        [ [1,1,1], [0,0],     '%s && $x'                         ],
        [ [1,1,1], [0,0,0],   '!(%s && $x)'                      ],
        [ [1,1,1], [0,1,0,0], '$y || (%s && $x)'                 ],
        [ [1,9,1], [0,0,0,0], 'if (%s && $x) {$x}'               ],
        [ [1,9,1], [0,0,0,0], 'if (%s && $x) {$x} else {$y}'     ],
        [ [1,9,1], [0,0,0,0], 'unless (%s && $x) {$x}'           ],

        # INNER AND RHS

        [ [0,0,0], [0,1],     '$x && %s'                         ],
        [ [1,1,1], [0,0,1],   '!($x && %s)'                      ],
        [ [0,0,0], [0,1,0,1], '$y || ($x && %s)'                 ],
        [ [1,9,1], [0,0,0,1], 'if ($x && %s) {$x}'               ],
        [ [1,9,1], [0,0,0,1], 'if ($x && %s) {$x} else {$y}'     ],
        [ [1,9,2], [0,0,0,1], 'unless ($x && %s) {$x}'           ],

        # INNER XOR LHS

            # LHS of XOR is currently too hard to detect as
            # being in boolean context

        # INNER XOR RHS

        [ [1,1,1], [1],       '($x xor %s)'                      ],
        [ [1,1,1], [0,1],     '!($x xor %s)'                     ],
        [ [1,1,1], [0,1,1],   '$y || ($x xor %s)'                ],
        [ [1,9,1], [0,0,1],   'if ($x xor %s) {$x}'              ],
        [ [1,9,1], [0,0,1],   'if ($x xor %s) {$x} else {$y}'    ],
        [ [1,9,1], [0,0,1],   'unless ($x xor %s) {$x}'          ],

        # GREP

        [ [1,1,1], [0,1,0],    'grep(%s,1,2)'                    ],
        [ [1,1,1], [0,1,0,0],  'grep(!%s,1,2)'                   ],
        [ [1,1,1], [0,1,0,0,1],'grep($y || %s,1,2)'              ],

        # FLIP

        [ [1,1,1], [0,0,0,0],      '%s..$x'                      ],
        [ [1,1,1], [0,0,0,0,0],    '!%s..$x'                     ],
        [ [1,1,1], [0,0,0,0,0,1],  '($y || %s)..$x'              ],

        # FLOP

        [ [1,1,1], [0,0,0,1],      '$x..%s'                      ],
        [ [1,1,1], [0,0,0,1,0],    '$x..!%s'                     ],
        [ [1,1,1], [0,0,0,1,0,1],  '$x..($y || %s)'              ],

    ) {
        my ($expects, $op_path, $code_fmt) = @$test;

        for my $context (0,1,2) {
            # 0: void
            # 1: scalar
            # 2: unknown
            # 9: skip test (principally if() can't be in scalar context)

            next if $expects->[$context] == 9;

            my $base_code = sprintf $code_fmt, $op_code;
            my $code = $base_code;
            my @op_path = @$op_path;
            push @op_path, @$post_op_path;

            # where to find the expression in the top-level lineseq
            my $seq_offset = -1;

            if ($context == 0) {
                $seq_offset -= 2;
                $code .= "; 1";
            }
            elsif ($context == 1) {
                $code = "\$pkg_result = ($code)";
                unshift @op_path, 0;
            }


            my $sub;
            {
                # don't use 'my' for $pkg_result to avoid the assignment in
                # '$result = foo()' being optimised away with OPpTARGET_MY
                our (@pkg, %pkg, $pkg_result);
                my  (@lex, %lex, $p, $q, $x, $y);

                no warnings 'void';
                $sub = eval "sub { $code }"
                    or die
                        "eval'$code' failed: this test needs to be rewritten;\n"
                        . "Errors were:\n$@";
            }

            # find the expression subtree in the main lineseq of the sub
            my $expr = svref_2object($sub)->ROOT->first;
            my $orig_expr = $expr;
            my @ops;
            my $next = $expr->first;
            while ($$next) {
                push @ops, $next;
                $next = $next->sibling;
            }
            $expr = $ops[$seq_offset];

            # search through the expr subtree looking for the named op -
            # this assumes that for all the code examples above, the
            # op is always in the LH branch
            my @orig_op_path = @op_path;
            while (defined (my $p = shift @op_path)) {
                eval {
                    $expr = $expr->first;
                    $expr = $expr->sibling while $p--;
                }
            }

            if (!$expr || !$$expr || $expr->name ne $op_name) {
                my $optree = dump_optree($orig_expr,2);
                print STDERR "Can't find $op_name op in optree for '$code'.\n";
                print STDERR "This test needs to be rewritten\n";
                print STDERR "seq_offset=$seq_offset op_path=(@orig_op_path)\n";
                print STDERR "optree=\n$optree";
                exit 1;
            }

            my $exp = $expects->[$context];
            $exp =   $exp == 0 ? 0
                   : $exp == 1 ? $bool_flag
                   :             $maybe_flag;

            my $got = ($expr->private & ($bool_flag | $maybe_flag));
            my $cxt_name = ('void   ', 'scalar ', 'unknown')[$context];
            is $got, $exp,  "boolean: $op_name $cxt_name '$base_code'";
        }
    }
}

