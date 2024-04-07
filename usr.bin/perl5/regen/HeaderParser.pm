package HeaderParser;
use strict;
use warnings;

# these are required below in BEGIN statements, we cant have a
# hard dependency on them as they might not be available when
# we run as part of autodoc.pl
#
# use Data::Dumper;
# use Storable qw(dclone);
#
use Carp       qw(confess);
use Text::Tabs qw(expand unexpand);
use Text::Wrap qw(wrap);

# The style of this file is determined by:
#
# perltidy -w -ple -bbb -bbc -bbs -nolq -l=80 -noll -nola -nwls='=' \
#   -isbc -nolc -otr -kis -ci=4 -se -sot -sct -nsbl -pt=2 -fs  \
#   -fsb='#start-no-tidy' -fse='#end-no-tidy' -cpb -bfvt=2

my (
    %unop,              # unary operators and their precedence
    %binop,             # binary operators and their precedence
    %is_right_assoc,    # operators which are right associative
    %precedence,        # precedence of all operators.
    %associative,       # associative operators
    %commutative,       # commutative operators
    %cmpop,             # comparison operators
    $unop_pat,          # pattern to match unary operators
    $binop_pat,         # pattern to match binary operators
    %op_names,          # map of op to description, used in error messages
    $tokenize_pat       # a pattern which can tokenize an expression
);

BEGIN {
    # this is initialization for the operator precedence expression parser
    # we use for handling preprocessor conditions.
    %op_names= (
        '==' => 'equality',
        '!=' => 'inequality',
        '<<' => 'bit-shift-left',
        '>>' => 'bit-shift-right',
        '+'  => 'addition',
        '-'  => 'subtraction',
        '*'  => 'multiplication',
        '/'  => 'division',
        '%'  => 'modulo',
        '||' => 'logical-or',       # Lowest precedence
        '&&' => 'logical-and',
        '|'  => 'binary-or',
        '^'  => 'binary-xor',
        '&'  => 'binary-and',
        '<'  => 'less-than',        # split on spaces, all with equal precedence
        '>'  => 'greater-than',
        '<=' => 'less-than-or-equal',
        '>=' => 'greater-than-or-equal',
    );
    my @cmpop= (
        '== !=',                    # listed in lowest to highest precedence
        '< > <= >=',                # split on spaces, all with equal precedence
    );
    my @binop= (
        '||',                       # Lowest precedence
        '&&',
        '|',
        '^',
        '&',
        @cmpop,    # include the numerical comparison operators.
        '<< >>',
        '+ -',
        '* / %',    # highest prcedence operators.
    );

    my @unop= qw( ! ~ + - );
    %unop= map  { $_ => 1 } @unop;
    %cmpop= map { $_ => 1 } map { split /\s+/, $_ } @cmpop;
    %binop= map { $_ => 1 } map { split /\s+/, $_ } @binop;

    my $make_pat= sub {
        my $pat= join "|", sort { length($b) <=> length($a) || $a cmp $b }
            map quotemeta($_), @_;
        return qr/$pat/;
    };
    $unop_pat= $make_pat->(@unop);
    foreach my $ix (0 .. $#binop) {
        my $sym= $binop[$ix];
        $precedence{$_}= (1 + $ix) * 10 for split /\s+/, $sym;
    }
    $is_right_assoc{"?"}= 1;
    $is_right_assoc{":"}= 1;
    $precedence{"?"}= 1;
    $precedence{":"}= 0;

    $associative{$_}++
        for qw( || && + *);    # we leave '==' out so we don't reorder terms
    $commutative{$_}++ for qw( || && + *);

    $binop_pat= $make_pat->(keys %precedence);
    $tokenize_pat= qr/
     ^(?:
        (?<comment> \/\*.*?\*\/ )
      | (?<ws>      \s+ )
      | (?<term>
            (?<literal>
                (?<define> defined\(\w+\) )
            |   (?<func>   \w+\s*\(\s*\w+(?:\s*,\s*\w+)*\s*\) )
            |   (?<const>  (?:0x[a-fA-F0-9]+|\d+[LU]*|'.') )
            |   (?<sym>    \w+ )
            )
        |   (?<op> $binop_pat | $unop_pat )
        |   (?<paren> [\(\)] )
        )
      )
    /xs;
}

# dump the arguments with dump. wraps loading Dumper
# as we are executed by miniperl where Dumper isnt available
sub dd {
    my $self= shift;
    local $self->{orig_content};
    my $ret= "(dump not available)";
    eval {
        require Data::Dumper;
        $ret= Data::Dumper->new(\@_)->Indent(1)->Sortkeys(1)->Useqq(1)->Dump();
    };
    return $ret;
}

my $has_storable;

# same story here, in miniperl we use slow perl code,
# in real perl we can use Storable and speed things up.
BEGIN { eval "use Storable; \$has_storable=1;" }

# recursively copy an AoAoA...
sub copy_aoa {
    my ($aoa)= @_;
    if ($has_storable) {
        return Storable::dclone($aoa);
    }
    else {
        return _copy_aoa($aoa);
    }
}

sub _copy_aoa {
    my ($thing)= @_;
    if (ref $thing) {
        return [ map { ref($_) ? _copy_aoa($_) : $_ } @$thing ];
    }
    else {
        return $thing;
    }
}

# return the number characters that should go in between a '#' and
# the name of a c preprocessor directive. Returns 0 spaces for level
# 0, and 2 * ($level - 1) + 1 spaces for the rest. (1,3,5, etc)
# This might sound weird, but consider these are tab *stops* and the
# '#' is included in the total. which means indents of 2, 4, 6 etc.
sub indent_chars {
    my ($self, $level)= @_;
    my $ind= "";
    $ind .= " "                 if $level;
    $ind .= "  " x ($level - 1) if $level > 1;
    return $ind;
}

# we use OO to store state, etc.
sub new {
    my ($class, %args)= @_;
    $args{add_commented_expr_after} //= 10;
    $args{max_width} //= 78;
    $args{min_break_width} //= 70;
    return bless \%args,;
}

# this parses the expression into an array of tokens
# this is somewhat crude, we could do this incrementally
# if we wanted and avoid the overhead. but it makes it
# easier to debug the tokenizer.
sub _tokenize_expr {
    my ($self, $expr)= @_;
    delete $self->{tokens};
    delete $self->{parse_tree};
    $self->{original_expr}= $expr;

    my @tokens;
    while ($expr =~ s/$tokenize_pat//xs) {
        push @tokens, {%+} if defined $+{'term'};
    }
    $self->{tokens}= \@tokens;
    warn $self->dd($self) if $self->{debug};
    if (length $expr) {
        confess "Failed to tokenize_expr: $expr\n";
    }
    return \@tokens;
}

sub _count_ops {
    my ($self, $term)= @_;
    my $count= 0;
    $count++ while $term =~ m/(?: \|\| | \&\& | \? )/gx;
    return $count;
}

# sort terms in an expression in a way that puts things
# in a sensible order. Anything starting with PERL_IN_
# should be on the left in alphabetical order. Digits
# should be on the right (eg 0), and ties are resolved
# by stripping non-alpha-numerc, thus removing underbar
# parens, spaces, logical operators, etc, and then by
# lc comparison of the result.
sub _sort_terms {
    my $self= shift;
    my (@terms)= map {
        [
            $_,                                # 0: raw
            lc($_) =~ s/[^a-zA-Z0-9]//gr,      # 1: "_" stripped and caseless
            $_     =~ m/PERL_IN_/  ? 1 : 0,    # 2: PERL_IN_ labeled define
            $_     =~ m/^\d/       ? 1 : 0,    # 3: digit
            $_     =~ m/DEBUGGING/ ? 1 : 0,    # 4: DEBUGGING?
            $self->_count_ops($_),             # 5: Number of ops (||, &&)
        ]
    } @_;
    my %seen;
    #start-no-tidy
    @terms= map { $seen{ $_->[0] }++ ? () : $_->[0] }
        sort {
            $a->[5] <=> $b->[5]         ||    # least number of ops
            $b->[2] <=> $a->[2]         ||    # PERL_IN before others
            $a->[3] <=> $b->[3]         ||    # digits after others
            $a->[4] <=> $b->[4]         ||    # DEBUGGING after all else
            $a->[1] cmp $b->[1]         ||    # stripped caseless cmp
            lc($a->[0]) cmp lc($b->[0]) ||    # caseless cmp
            $a->[0] cmp $b->[0]         ||    # exact cmp
            0
        } @terms;
    #end-no-tidy
    return @terms;
}

# normalize a condition expression by parsing it and then stringifying
# the parse tree.
sub tidy_cond {
    my ($self, $expr)= @_;
    my $ret= $self->{_tidy_cond_cache}{$expr} //= do {
        $self->parse_expr($expr) if defined $expr;
        my $text= $self->_pt_as_str();
        $text;
    };
    $self->{last_expr}= $ret;
    return $ret;
}

# convert a parse tree structure to a string recursively.
#
# Parse trees are currently made up of arrays, with the count
# of items in the object determining the type of op it represents.
# 1 argument:  literal value of some sort.
# 2 arguments: unary operator: 0 slot is the operator, 1 is a parse tree
#            : ternary: 0 slot holds '?', 1 is an array holding three
#                       parse trees: cond, true, false
# 3 arguments or more: binary operator. 0 slot is the op. 1..n are parse trees
#                    : note, this is multigate for commutative operators like
#                    : "+", "*", "&&" and "||", so an expr
#                    : like "A && B && !C" would be represented as:
#                    : [ "&&", ["A"], ["B"], [ "!",["C"] ] ]
#
sub _pt_as_str {
    my ($self, $node, $parent_op, $depth)= @_;

    $node ||= $self->{parse_tree}
        or confess "No parse tree?";
    $depth ||= 0;
    if (@$node == 1) {

        # its a literal
        return $node->[0];
    }
    elsif (@$node == 2) {

        # is this a ternary or an unop?
        if ($node->[0] eq '?') {

            # ternary, the three "parts" are tucked away in
            # an array in the payload slot
            my $expr=
                  $self->_pt_as_str($node->[1][0], "?", $depth + 1) . " ? "
                . $self->_pt_as_str($node->[1][1], "?", $depth + 1) . " : "
                . $self->_pt_as_str($node->[1][2], "?", $depth + 1);

            # stick parens on if this is a subexpression
            $expr= "( " . $expr . " )" if $depth;
            return $expr;
        }
        else {
            if (    $node->[0] eq "!"
                and @{ $node->[1] } == 2
                and $node->[1][0] eq "!")
            {
                # normalize away !! in expressions.
                return $self->_pt_as_str($node->[1][1], $parent_op, $depth);
            }

            # unop - the payload is a optree
            return $node->[0]
                . $self->_pt_as_str($node->[1], $node->[0], $depth + 1);
        }
    }

    # if we get here we are dealing with a binary operator
    # the nodes are not necessarily binary, as we "collect"
    # the terms into a list, thus: A && B && C && D -> ['&&',A,B,C,D]
    my ($op, @terms)= @$node;

    # convert the terms to strings
    @terms= map { $self->_pt_as_str($_, $op, $depth + 1) } @terms;

    # sort them to normalize the subexpression
    my $expr=
        join " $op ", $associative{$op}
        ? $self->_sort_terms(@terms)
        : @terms;

    # stick parens on if this is a subexpression
    $expr= "( " . $expr . " )" if $depth and !$cmpop{$op};

    # and we are done.
    return $expr;
}

# Returns the precedence of an operator, returns 0 if there is no token
# or the next token is not an op, or confesss if it encounters an op it does not
# know.
sub _precedence {
    my $self= shift;
    my $token= shift // return 0;

    my $op= (ref $token ? $token->{op} : $token) // return 0;

    return $precedence{$op} // confess "Unknown op '$op'";
}

# entry point into parsing the tokens, checks that we actually parsed everything
# and didnt leave anything in the token stream (possible from a malformed expression)
# Performs some minor textual cleanups using regexes, but then does a proper parse
# of the expression.
sub parse_expr {
    my ($self, $expr)= @_;
    if (defined $expr) {
        $expr =~ s/\s*\\\n\s*/ /g;
        $expr =~ s/defined\s+(\w+)/defined($1)/g;
        $self->_tokenize_expr($expr);
    }
    my $ret= $self->_parse_expr();
    if (@{ $self->{tokens} }) {

        # if all was well with parsing we should not get here.
        confess "Unparsed tokens: ", $self->dd($self->{tokens});
    }
    $self->{parse_tree}= $ret;
    return $ret;
}

# this is just a wrapper around _parse_expr_assoc() which handles
# parsing an arbitrary expression.
sub _parse_expr {
    my ($self)= @_;
    return $self->_parse_expr_assoc($self->_parse_expr_primary(), 1);
}

# This handles extracting from the token stream
#  - simple literals
#  - unops (assumed to be right associative)
#  - parens (which reset the precedence acceptable to the parser)
#
sub _parse_expr_primary {
    my ($self)= @_;
    my $tokens= $self->{tokens}
        or confess "No tokens in _parse_expr_primary?";
    my $first= $tokens->[0]
        or confess "No primary?";
    if ($first->{paren} and $first->{paren} eq "(") {
        shift @$tokens;
        my $expr= $self->_parse_expr();
        $first= $tokens->[0];
        if (!$first->{paren} or $first->{paren} ne ")") {
            confess "Expecting close paren", $self->dd($tokens);
        }
        shift @$tokens;
        return $expr;
    }
    elsif ($first->{op} and $unop{ $first->{op} }) {
        my $op_token= shift @$tokens;
        return [ $op_token->{op}, $self->_parse_expr_primary() ];
    }
    elsif (defined $first->{literal}) {
        shift @$tokens;
        return [ $first->{literal} ];
    }
    else {
        die sprintf
            "Unexpected token '%s', expecting literal, unary, or expression.\n",
            $first->{term};
    }
}

# This is the heart of the expression parser. It uses
# a pair of nested loops to avoid excessive recursion during parsing,
# which should be a bit faster than other strategies. It only should
# recurse when the precedence level changes.
sub _parse_expr_assoc {
    my ($self, $lhs, $min_precedence)= @_;
    my $tokens= $self->{tokens}
        or confess "No tokens in _parse_expr_assoc";
    my $la= $tokens->[0];                  # lookahead
    my $la_pr= $self->_precedence($la);    # lookahead precedence
    while ($la && $la_pr >= $min_precedence) {
        my $op_token= shift @$tokens;
        my $op_pr= $la_pr;                 # op precedence
        if ($op_token->{op} eq "?") {
            my $mid= $self->_parse_expr();
            if (@$tokens and $tokens->[0]{op} and $tokens->[0]{op} eq ":") {
                shift @$tokens;
                my $tail= $self->_parse_expr();
                return [ '?', [ $lhs, $mid, $tail ] ];
            }
            confess "Panic: expecting ':'", $self->dd($tokens);
        }
        my $rhs;
        eval { $rhs= $self->_parse_expr_primary(); }
            or die "Error in $op_names{$op_token->{op}} expression: $@";
        $la= $tokens->[0];
        $la_pr= $self->_precedence($la);
        while (
            $la_pr > $op_pr ||    # any and larger
            (       $is_right_assoc{ $op_token->{op} }
                and $la_pr == $op_pr)    # right and equal
        ) {
            my $new_precedence= $op_pr + ($la_pr > $op_pr ? 1 : 0);
            $rhs= $self->_parse_expr_assoc($rhs, $new_precedence);
            $la= $tokens->[0];
            $la_pr= $self->_precedence($la);
        }
        if (   @$lhs >= 3
            && $lhs->[0] eq $op_token->{op}
            && $commutative{ $op_token->{op} })
        {
            push @$lhs, $rhs;
        }
        else {
            my @lt= ($lhs);
            my @rt= ($rhs);

            # if we have '( a && b ) && ( c && d)'
            # turn it into 'a && b && c && d'
            if (@$lhs > 2 && $lhs->[0] eq $op_token->{op}) {
                (undef,@lt)= @$lhs; # throw away op.
            }
            if (@$rhs > 2 && $rhs->[0] eq $op_token->{op}) {
                (undef,@rt)= @$rhs; # throw away op.
            }
            $lhs= [ $op_token->{op}, @lt, @rt ];
        }
    }
    return $lhs;
}

#entry point for normalizing and if/elif statements
#returns the line and condition in normalized form.
sub normalize_if_elif {
    my ($self, $line, $line_info)= @_;
    if (my $dat= $self->{cache_normalize_if_elif}{$line}) {
        return $dat->{line}, $dat->{cond};
    }
    my ($cond);
    eval {
        ($line, $cond)= $self->_normalize_if_elif($line);
        1;
    } or die sprintf "Error at line %d\nLine %d: %s\n%s",
        ($line_info->start_line_num()) x 2, $line, $@;
    $self->{cache_normalize_if_elif}{$line}= { line => $line, cond => $cond };
    return ($line, $cond);
}

#guts of the normalize_if_elif() - cleans up the line, extracts
#the condition, and then tidies it with tidy_cond().
sub _normalize_if_elif {
    my ($self, $line)= @_;
    my $nl= "";
    $nl= $1 if $line =~ s/(\n+)\z//;
    $line =~ s/\s+\z//;
    my @comment;
    push @comment, $1 while $line =~ s!\s*(/\*.*?\*/)\z!!;
    $line =~ s/defined\s*\(\s*(\w+)\s*\)/defined($1)/g;
    $line =~ s/!\s+defined/!defined/g;

    if ($line =~ /^#((?:el)?if)(n?)def\s+(\w+)/) {
        my $if= $1;
        my $not= $2 ? "!" : "";
        $line= "#$if ${not}defined($3)";
    }
    $line =~ s/#((?:el)?if)\s+//
        or confess "Bad cond: $line";
    my $if= $1;
    $line =~ s/!\s+/!/g;

    my $old_cond= $line;
    my $cond= $self->tidy_cond($old_cond);

    warn "cond - $old_cond\ncond + $cond\n"
        if $old_cond ne $cond and $self->{debug};

    $line= "#$if $cond";
    $line .= "  " . join " ", reverse @comment if @comment;

    $line .= $nl;
    return ($line, $cond);
}

# parses a text buffer as though it was a file on disk
# calls parse_fh()
sub parse_text {
    my ($self, $text)= @_;
    local $self->{parse_source}= "(buffer)";
    open my $fh, "<", \$text
        or die "Failed to open buffer for read: $!";
    return $self->parse_fh($fh);
}

# takes a readable filehandle and parses whatever contents is
# returned by reading it. Returns an array of HeaderLine objects.
# this is the main routing for parsing a header file.
sub parse_fh {
    my ($self, $fh)= @_;
    my @lines;
    my @cond;
    my @cond_line;
    my $last_cond;
    local $self->{parse_source}= $self->{parse_source} || "(unknown)";
    my $cb= $self->{pre_process_content};
    $self->{orig_content}= "";
    my $line_num= 1;

    while (defined(my $line= readline($fh))) {
        my $start_line_num= $line_num++;
        $self->{orig_content} .= $line;
        while ($line =~ /\\\n\z/ or $line =~ m</\*(?:(?!\*/).)*\s*\z>s) {
            defined(my $read_line= readline($fh))
                or last;
            $self->{orig_content} .= $read_line;
            $line_num++;
            $line .= $read_line;
        }
        while ($line =~ m!/\*(.*?)(\*/|\z)!gs) {
            my ($inner, $tail)= ($1, $2);
            if ($tail ne "*/") {
                confess
                    "Unterminated comment starting at line $start_line_num\n";
            }
            elsif ($inner =~ m!/\*!) {
                confess
                    "Nested/broken comment starting at line $start_line_num\n";
            }
        }

        my $raw= $line;
        my $type= "content";
        my $sub_type= "text";
        my $level= @cond;
        my $do_pop= 0;
        my $flat= $line;
        $flat =~ s/\s*\\\n\s*/ /g;
        $flat =~ s!/\*.*?\*/! !gs;
        $flat =~ s/\s+/ /g;
        $flat =~ s/\s+\z//;
        $flat =~ s/^\s*#\s*/#/g;

        my $line_info=
            HeaderLine->new(start_line_num => $start_line_num, raw => $raw);
        my $do_cond_line;
        if ($flat =~ /^#/) {
            if ($flat =~ m/^(#(?:el)?if)(n?)def\s+(\w+)/) {
                my $if= $1;
                my $not= $2 ? "!" : "";
                my $sym= $3;
                $flat =~
                    s/^(#(?:el)?if)(n?)def\s+(\w+)/$if ${not}defined($sym)/;
            }
            my $cond;    # used in various expressions below
            if ($flat =~ /^#endif/) {
                if (!@cond) {
                    confess "Not expecting $flat";
                }
                $do_pop= 1;
                $level--;
                $type= "cond";
                $sub_type= "#endif";
            }
            elsif ($flat =~ /^#if\b/) {
                ($flat, $cond)= $self->normalize_if_elif($flat, $line_info);
                push @cond,      [$cond];
                push @cond_line, $line_info;
                $type= "cond";
                $sub_type= "#if";
            }
            elsif ($flat =~ /^#elif\b/) {
                if (!@cond) {
                    confess "No if for $flat";
                }
                $level--;
                ($flat, $cond)= $self->normalize_if_elif($flat, $line_info);
                $cond[-1][-1]= $self->tidy_cond("!($cond[-1][-1])");
                $cond_line[-1]= $line_info;
                push @{ $cond[-1] }, $cond;
                $type= "cond";
                $sub_type= "#elif";
            }
            elsif ($flat =~ /^#else\b/) {
                if (!@cond) {
                    confess "No if for $flat";
                }
                $level--;
                $cond[-1][-1]= $self->tidy_cond("!($cond[-1][-1])");
                $cond_line[-1]= $line_info;
                $type= "cond";
                $sub_type= "#else";
            }
            elsif ($flat =~ /#undef/) {
                $type= "content";
                $sub_type= "#undef";
            }
            elsif ($flat =~ /#pragma\b/) {
                $type= "content";
                $sub_type= "#pragma";
            }
            elsif ($flat =~ /#include\b/) {
                $type= "content";
                $sub_type= "#include";
            }
            elsif ($flat =~ /#define\b/) {
                $type= "content";
                $sub_type= "#define";
            }
            elsif ($flat =~ /#error\b/) {
                $type= "content";
                $sub_type= "#error";
            }
            else {
                confess "Do not know what to do with $line";
            }
            if ($type eq "cond") {

                # normalize conditional lines
                $line= $flat;
                $last_cond= $line_info;
            }
        }
        $line =~ s/\n?\z/\n/;

        %$line_info= (
            cond           => copy_aoa(\@cond),
            type           => $type,
            sub_type       => $sub_type,
            raw            => $raw,
            flat           => $flat,
            line           => $line,
            level          => $level,
            source         => $self->{parse_source},
            start_line_num => $start_line_num,
            n_lines        => $line_num - $start_line_num,
        );

        push @lines, $line_info;
        if ($do_pop) {
            $line_info->{inner_lines}=
                $line_info->start_line_num - $cond_line[-1]->start_line_num;
            pop @cond;
            pop @cond_line;
        }
        if ($type eq "content" and $cb) {
            $cb->($self, $lines[-1]);
        }
    }
    if (@cond_line) {
        my $msg= "Unterminated conditional block starting line "
            . $cond_line[-1]->start_line_num();
        $msg .=
            " with last conditional operation at line "
            . $last_cond->start_line_num()
            if $cond_line[-1] != $last_cond;
        confess $msg;
    }
    $self->{lines}= \@lines;
    return \@lines;
}

# returns the last lines we parsed.
sub lines { $_[0]->{lines} }

# assuming a line looks like an embed.fnc entry parse it
# and normalize it, and create and EmbedLine object from it.
sub tidy_embed_fnc_entry {
    my ($self, $line_data)= @_;
    my $line= $line_data->{line};
    return $line if $line =~ /^\s*:/;
    return $line unless $line_data->{type} eq "content";
    return $line unless $line =~ /\|/;

    $line =~ s/\s*\\\n/ /g;
    $line =~ s/\s+\z//;
    ($line)= expand($line);
    my ($flags, $ret, $name, @args)= split /\s*\|\s*/, $line;
    my %flag_seen;
    $flags= join "", grep !$flag_seen{$_}++, sort split //, $flags;
    if ($flags =~ s/^#//) {
        $flags .= "#";
    }
    if ($flags eq "#") {
        die "Not allowed to use only '#' for flags"
            . "in 'embed.fnc' at line $line_data->{start_line_num}";
    }
    if (!$flags) {
        die "Missing flags in function definition"
            . " in 'embed.fnc' at line $line_data->{start_line_num}\n"
            . "Did you a forget a line continuation on the previous line?\n";
    }
    for ($ret, @args) {
        s/(\w)\*/$1 */g;
        s/\*\s+(\w)/*$1/g;
        s/\*const/* const/g;
    }
    my $head= sprintf "%-8s|%-7s", $flags, $ret;
    $head .= sprintf "|%*s", -(31 - length($head)), $name;
    if (@args and length($head) > 32) {
        $head .= "\\\n";
        $head .= " " x 32;
    }
    foreach my $ix (0 .. $#args) {
        my $arg= $args[$ix];
        $head .= "|$arg";
        $head .= "\\\n" . (" " x 32) if $ix < $#args;
    }
    $line= $head . "\n";

    if ($line =~ /\\\n/) {
        my @lines= split /\s*\\\n/, $line;
        my $len= length($lines[0]);
        $len < length($_) and $len= length($_) for @lines;
        $len= int(($len + 7) / 8) * 8;
        $len= 72 if $len < 72;
        $line= join("\\\n",
            (map { sprintf "%*s", -$len, $_ } @lines[ 0 .. $#lines - 1 ]),
            $lines[-1]);
    }
    ($line)= unexpand($line);

    $line_data->{embed}= EmbedLine->new(
        flags       => $flags,
        return_type => $ret,
        name        => $name,
        args        => \@args,
    );
    $line =~ s/\s+\z/\n/;
    $line_data->{line}= $line;
    return $line;
}

# line up the text in a multiline string by a given $fragment
# of text, inserting whitespace in front or behind the $fragment
# to get the text to line up. Returns the text. This is wrapped
# by line_up() and is used to wrap long conditions and comments
# in the generated code.
sub _line_up_frag {
    my ($self, $str, $fragment)= @_;
    die "has tabs?!" if $str =~ /\t/;
    my @lines= split /\n/, $str;
    my $changed= 1;
    while ($changed) {
        $changed= 0;
        foreach my $ix (0 .. $#lines - 1) {
            my $f_index= 0;
            my $n_index= 0;
            while (1) {
                $f_index= index($lines[$ix],       $fragment, $f_index);
                $n_index= index($lines[ $ix + 1 ], $fragment, $n_index);
                if ($f_index == -1 or $n_index == -1) {
                    last;
                }
                if ($f_index < $n_index) {
                    my $f_idx= $f_index;
                    $f_idx-- while substr($lines[$ix], $f_idx, 1) ne " ";
                    substr($lines[$ix], $f_idx, 0, " " x ($n_index - $f_index));
                    $changed++;
                    last;
                }
                elsif ($n_index < $f_index) {
                    my $n_idx= $n_index;
                    $n_idx-- while substr($lines[ $ix + 1 ], $n_idx, 1) ne " ";
                    substr($lines[ $ix + 1 ],
                        $n_idx, 0, " " x ($f_index - $n_index));
                    $changed++;
                    last;
                }
                $f_index++;
                $n_index++;
            }
        }
    }
    my $ret= join "", map { "$_\n" } @lines;
    return $ret;
}

sub _fixup_indent {
    my ($self, $line)= @_;
    my @lines= split /\n/, $line;
    if ($lines[0]=~/^(#\s*\w+(?:\s*\/\*)?\s)(\s+)/) {
        my $first_left_len = length $1;

        while (1) {
            my $ok = 1;
            for (@lines) {
                /^.{$first_left_len} /
                    or do { $ok = 0; last; };
            }
            if ($ok) {
                s/^(.{$first_left_len}) /$1/ for @lines;
            } else {
                last;
            }
        }
    }

    if ($lines[0]=~/^(#\s*\w+\s+)\(/) {
        my $len = length($1);
        for my $idx (1..$#lines) {
            $lines[$idx]=~s/^([ ]{$len})(\s+)(\()/$1$3$2/;
        }
    }
    my $ret= join "", map { "$_\n" } @lines;
    return $ret;
}

# this is the workhorse for _break_line_at_op().
sub __break_line_at_op {
    my ($self, $limit, $line, $blank_prefix)= @_;
    my @lines= ("");
    while (length $line) {
        my $part;
        if ($line =~ s/^(.*?(?:\|\||&&)\s+)//) {
            $part= $1;
        }
        else {
            $part= $line;
            $line= "";
        }
        if (length($lines[-1]) + length($part) < $limit) {
            $lines[-1] .= $part;
        }
        else {
            push @lines, $blank_prefix . $part;
        }
    }
    return \@lines;
}

# Break a condition line into parts, while trying to keep the last
# token on each line being an operator like || or && or ? or : We try
# to keep each line at $limit characters, however, we also try to
# ensure that each line has the same number of operators on it such
# that across all the lines there are only two counts of operators (eg,
# we either way each line to have two operators on it, or 0, or 1 or 0,
# or 2 or 1, and so on.) If we cannot meet this requirement we reduce
# the limit by 1 and try again, until we meet the objective, or the
# limit ends up at 70 chars or less.
sub _break_line_at_op {
    my ($self, $limit, $line, $blank_prefix)= @_;
    my $lines;
    while (1) {
        $lines= $self->__break_line_at_op($limit, $line, $blank_prefix);
        my %op_counts;
        foreach my $line_idx (0 .. $#$lines) {
            my $line= $lines->[$line_idx];
            my $count= 0;
            $count++ while $line =~ /(\|\||&&|\?|:)/g;
            $op_counts{$count}++;

        }
        if ($limit <= $self->{min_break_width} || keys(%op_counts) <= 2) {
            last;
        }
        $limit--;
    }

    s/\s*\z/\n/ for @$lines;
    return join "", @$lines;
}

sub _max { # cant use Scalar::Util so we roll our own
    my $max= shift;
    $max < $_ and $max= $_ for @_;
    return $max;
}

# take a condition, split into $type and $rest
# wrap it, and try to line up operators and defined() functions
# that it contains. This is rather horrible code, but it does a
# reasonable job applying the heuristics we need to lay our the
# conditions in a reasonable way.
sub _wrap_and_line_up_cond {
    my ($self, $type, $rest)= @_;

    my $limit= $self->{max_width};

    # extract the expression part of the line, and normalize it, we do
    # this here even though it might be duplicative as it is possible
    # that the caller code has munged the expression in some way, and we
    # might want to simplify the expression first. Eg:
    # 'defined(FOO) && (defined(BAR) && defined(BAZ))' should be turned into
    # 'defined(FOO) && defined(BAR) && defined(BAZ)' if possible.
    my $rest_head= "";
    my $rest_tail= "";
    if ($rest =~ s!(if\s+)!!) {
        $rest_head= $1;
    }
    if ($rest =~ s!(\s*/\*.*?\*/)\s*\z!! || $rest =~ s!(\s*\*/\s*)\z!!) {
        $rest_tail= $1;
    }
    if ($rest) {
        $rest= $self->tidy_cond($rest);
        $rest= $rest_head . $rest . $rest_tail;
    }

    my $l= length($type);
    my $line= $type;
    $line .= $rest if length($rest);
    my $blank_prefix= " " x $l;

    # at this point we have a single line with the entire expression on it
    # if it fits on one line we are done, we can return it right away.
    if (length($line) <= $limit) {
        $line =~ s/\s*\z/\n/;
        return $line;
    }
    my $rest_copy= $rest;
    my @fragments;
    my $op_pat= qr/(?:\|\||&&|[?:])/;

    # does the $rest contain a parenthesized group? If it does then
    # there are a mixture of different ops being used, as if it was all
    # the same opcode there would not be a parenthesized group.
    # If it does then we handle it differently, and try to put the
    # different parts of the expression on their own line.
    if ($rest_copy =~ /$op_pat\s*\(/) {
        my @parts;
        while (length $rest_copy) {
            if ($rest_copy =~ s/^(.*?$op_pat)(\s*!?\()/$2/) {
                push @parts, $1;
            } else {
                #$rest_copy=~s/^\s+//;
                push @parts, $rest_copy;
                last;
            }
        }
        $parts[0]= $type . $parts[0];
        $parts[$_]= $blank_prefix . $parts[$_] for 1 .. $#parts;
        foreach my $line (@parts) {
            if (length($line) > $limit) {
                $line= $self->_break_line_at_op($limit, $line, $blank_prefix);
            }
        }
        s/\s*\z/\n/ for @parts;
        $line= join "", @parts;
        @fragments= ("defined", "||");
    }
    else {
        # the expression consists of just one opcode type, so we can use
        # simpler logic to break it apart with the objective of ensuring
        # that the lines are similarly formed with trailing operators on
        # each line but the last.
        @fragments= ("||", "defined");
        $line= $self->_break_line_at_op($limit, $type . $rest, $blank_prefix);
    }

    # try to line up the text on different lines. We stop after
    # the first $fragment that modifies the text. The order
    # of fragments we try is determined above based on the type
    # of condition this is.
    my $pre_line= $line;
    foreach my $fragment (@fragments) {
        $line= $self->_line_up_frag($line, $fragment);
        last if $line ne $pre_line;
    }

    # if we have lined up by "defined" in _line_up_frag()
    # then we may have " ||        defined(...)" type expressions
    # convert these to "        || defined(...)" as it looks better.
    $line =~ s/( )(\|\||&&|[()?:])([ ]{2,})(!?defined)/$3$2$1$4/g;
    $line =~ s/(\|\||&&|[()?:])[ ]{10,}/$1 /g;

    # add back the line continuations. this is all pretty inefficient,
    # but it works nicely.
    my @lines= split /\n/, $line;
    my $last= pop @lines;
    my $max_len= _max(map { length $_ } @lines);
    $_= sprintf "%*s \\\n", -$max_len, $_ for @lines;
    $last .= "\n";

    $line= join "", @lines, $last;

    # remove line continuations that are inside of a comment,
    # we may have a variable number of lines of the expression
    # or parts of lines of the expression in a comment, so
    # we do this last.
    $line =~ s!/\* (.*) \*/
              !"/*"._strip_line_cont("$1")."*/"!xsge;

    return $self->_fixup_indent($line);
}

#remove line continuations from the argument.
sub _strip_line_cont {
    my ($string)= @_;
    $string =~ s/\s*\\\n/\n/g;
    return $string;
}

# Takes an array of HeaderLines objects produced by parse_fh()
# or by group_content(), and turn it into a string.
sub lines_as_str {
    my ($self, $lines, $post_process_content)= @_;
    $lines ||= $self->{lines};
    my $ret;
    $post_process_content ||= $self->{post_process_content};
    my $filter= $self->{filter_content};
    my $last_line= "";

    #warn $self->dd($lines);
    foreach my $line_data (@$lines) {
        my $line= $line_data->{line};
        if ($line_data->{type} ne "content" or $line_data->{sub_type} ne "text")
        {
            my $level= $line_data->{level};
            my $ind= $self->indent_chars($level);
            $line =~ s/^#(\s*)/#$ind/;
        }
        if ($line_data->{type} eq "cond") {
            my $add_commented_expr_after= $self->{add_commented_expr_after};
            if ($line_data->{sub_type} =~ /#(?:else|endif)/) {
                my $joined= join " && ",
                    map { "($_)" } @{ $line_data->{cond}[-1] };
                my $cond_txt= $self->tidy_cond($joined);
                $cond_txt= "if $cond_txt" if $line_data->{sub_type} eq "#else";
                $line =~ s!\s*\z! /* $cond_txt */\n!
                    if $line_data->{inner_lines} >= $add_commented_expr_after;
            }
            elsif ($line_data->{sub_type} eq "#elif") {
                my $last_frame= $line_data->{cond}[-1];
                my $joined= join " && ",
                    map { "($_)" } @$last_frame[ 0 .. ($#$last_frame - 1) ];
                my $cond_txt= $self->tidy_cond($joined);
                $line =~ s!\s*\z! /* && $cond_txt */\n!
                    if $line_data->{inner_lines} >= $add_commented_expr_after;
            }
        }
        $line =~ s/\s*\z/\n/;
        if ($last_line eq "\n" and $line eq "\n") {
            next;
        }
        $last_line= $line;
        if ($line_data->{type} eq "cond") {
            $line =~ m!(^\s*#\s*\w+[ ]*)([^/].*?\s*)?(/\*.*)?\n\z!
                or die "Failed to split cond line: $line";
            my ($type, $cond, $comment)= ($1, $2, $3);
            $comment //= "";
            $cond    //= "";
            my $new_line;
            if (!length($cond) and $comment) {
                $comment =~ s!^(/\*\s+)!!
                    and $type .= $1;
            }

            $line= $self->_wrap_and_line_up_cond($type, $cond . $comment);
        }
        $line_data->{line}= $line;
        if ($post_process_content and $line_data->{type} eq "content") {
            $post_process_content->($self, $line_data);
        }
        if ($filter and $line_data->{type} eq "content") {
            $filter->($self, $line_data) or next;
        }
        $ret .= $line_data->{line};
    }
    return $ret;
}

# Text::Wrap::wrap has an odd api, so hide it behind a wrapper
# sub which sets things up properly.
sub _my_wrap {
    my ($head, $rest, $line)= @_;
    local $Text::Wrap::unexpand= 0;
    local $Text::Wrap::huge= "overflow";
    local $Text::Wrap::columns= 78;
    unless (length $line) { return $head }
    $line= wrap $head, $rest, $line;
    return $line;
}

# recursively extract the && expressions from a parse tree,
# returning the result as strings.
# if $node is not a '&&' op then it returns $node as a string,
# otherwise it returns the string form of the arguments to the
# '&&' op, recursively flattening any '&&' nodes that it might
# contain.
sub _and_clauses {
    my ($self, $node)= @_;

    my @ret;
    if (@$node < 3 or $node->[0] ne "&&") {
        return $self->_pt_as_str($node);
    }
    foreach my $idx (1 .. $#$node) {
        push @ret, $self->_and_clauses($node->[$idx]);
    }
    return @ret;
}

# recursively walk the a parse tree, and return the literal
# terms it contains, ignoring any operators in the optree.
sub _terms {
    my ($self, $node)= @_;
    if (@$node == 1) {
        return $self->_pt_as_str($node);
    }
    my @ret;
    if (@$node == 2) {
        if ($node->[0] eq "?") {
            push @ret, map { $self->_terms($_) } @{ $node->[1] };
        }
        else {
            push @ret, $self->_terms($node->[1]);
        }
    }
    else {
        foreach my $i (1 .. $#$node) {
            push @ret, $self->_terms($node->[$i]);
        }
    }
    return @ret;
}

# takes a HeaderLine "cond" AoA and flattens it into
# a single expression, and then extracts all the and clauses
# it contains. Thus [['defined(A)'],['defined(B)']] and
# [['defined(A) && defined(B)']], end up as ['defined(A)','defined(B)']
sub _flatten_cond {
    my ($self, $cond_ary)= @_;

    my $expr= join " && ", map {
        map { "($_)" }
            @$_
    } @$cond_ary;
    return [] unless $expr;
    my $tree= $self->parse_expr($expr);
    my %seen;
    my @and_clause= grep { !$seen{$_}++ } $self->_and_clauses($tree);
    return \@and_clause;
}

# Find the best path into a tree of conditions, such that
# we reuse the maximum number of existing branches. Returning
# two arrays, the first contain the parts of $cond_array that
# make up the best path, in the best path order, and a second array
# with the remaining items in the initial order they were provided.
# Thus if we have previously stored only the path "A", "B", "C"
# into the tree, and want to find the best path for
# ["E","D","C","B","A"] we should return: ["A","B","C"],["E","D"],
#
# This used to reduce the number of conditions in the grouped content,
# and is especially helpful with dealing with DEBUGGING related
# functionality. It is coupled with careful control over the order
# that we add paths and conditions to the tree.
sub _best_path {
    my ($self, $tree_node, $cond_array, @path)= @_;
    my $best= \@path;
    my $rest= $cond_array;
    foreach my $cond (@$cond_array) {
        if ($tree_node->{$cond}) {
            my ($new_best, $new_rest)=
                $self->_best_path($tree_node->{$cond},
                [ grep $_ ne $cond, @$cond_array ],
                @path, $cond);
            if (@$new_best > @$best) {
                ($best, $rest)= ($new_best, $new_rest);
            }
        }
    }
    if (@$best == @path) {
        foreach my $cond (@$cond_array) {
            my $not_cond= $self->tidy_cond("!($cond)");
            if ($tree_node->{$not_cond}) {
                $best= [ @path, $cond ];
                $rest= [ grep $_ ne $cond, @$cond_array ];
                last;
            }
        }
    }
    return ($best, $rest);
}

# This builds a group content tree from a set of lines. each content line in
# the original file is added to the file based on the conditions that apply to
# the content.
#
# The tree is made up of nested HoH's with keys in the HoH being normalized
# clauses from the {cond} data in the HeaderLine objects.
#
# Care is taken to minimize the number of pathways and to reorder clauses to
# reuse existing pathways and minimize the total number of conditions in the
# file.
#
# The '' key of a hash contains an array of the lines that are part of the
# condition that lead to that key. Thus lines with no conditions are in
# @{$tree{''}}, lines with the condition "defined(A) && defined(B)" would be
# in $tree{"defined(A)"}{"defined(B)"}{""}.
#
# The result of this sub is normally passed into __recurse_group_content_tree()
# which converts it back into a set of HeaderLine objects.
#
sub _build_group_content_tree {
    my ($self, $lines)= @_;
    $lines ||= $self->{lines};
    my $filter= $self->{filter_content};
    my %seen_normal;
    foreach my $line_data (@$lines) {
        next if $line_data->{type} ne "content";
        next if $filter and !$filter->($self, $line_data);
        my $cond_frames= $line_data->{cond};
        my $cond_frame= $self->_flatten_cond($cond_frames);
        my $flat_merged= join " && ", map "($_)", @$cond_frame;
        my $normalized;
        if (@$cond_frame) {
            $normalized= $self->tidy_cond($flat_merged);
        }
        else {
            $normalized= $flat_merged;    # empty string
        }
        push @{ $seen_normal{$normalized} }, $line_data;
    }
    my @debugging;
    my @non_debugging;
    foreach my $key (keys %seen_normal) {
        if ($key =~ /DEBUGGING/) {
            push @debugging, $key;
        }
        else {
            push @non_debugging, $key;
        }
    }
    @non_debugging=
        sort { length($a) <=> length($b) || $a cmp $b } @non_debugging;
    @debugging= sort { length($b) <=> length($a) || $a cmp $b } @debugging;
    my %tree;
    foreach my $normal_expr (@non_debugging, @debugging) {
        my $all_line_data= $seen_normal{$normal_expr};

        my $cond_frame=
            (length $normal_expr)
            ? $self->_flatten_cond([ [$normal_expr] ])
            : [];
        @$cond_frame= $self->_sort_terms(@$cond_frame);
        my $node= \%tree;
        my ($best, $rest)= $self->_best_path($node, $cond_frame);
        die sprintf "Woah: %d %d %d", 0 + @$best, 0 + @$rest, 0 + @$cond_frame
            unless @$best + @$rest == @$cond_frame;

        foreach my $cond (@$best, @$rest) {
            $node= $node->{$cond} ||= {};
        }
        push @{ $node->{''} }, @$all_line_data;
    }

    warn $self->dd(\%tree) if $self->{debug};
    $self->{tree}= \%tree;
    return \%tree;
}

sub _recurse_group_content_tree {
    my ($self, $node, @path)= @_;

    my @ret;
    local $self->{rgct_ret}= \@ret;
    local $self->{line_by_depth}= [];

    $self->__recurse_group_content_tree($node, @path);
    return \@ret;
}

# convert a tree of conditions constructed by _build_group_content_tree()
# and turn it into a set of HeaderLines that represents it. Performs the
# appropriate sets required to reconstitute an if/elif/elif/else sequence
# by calling _handle_else().
sub __recurse_group_content_tree {
    my ($self, $node, @path)= @_;
    my $depth= 0 + @path;
    my $ind= $self->indent_chars($depth);
    my $ret= $self->{rgct_ret};
    if ($node->{''}) {
        if (my $cb= $self->{post_process_grouped_content}) {
            $cb->($self, $node->{''}, \@path);
        }
        if (my $cb= $self->{post_process_content}) {
            $cb->($self, $_, \@path) for @{ $node->{''} };
        }
        push @$ret, map {
            HeaderLine->new(
                %$_,
                cond           => [@path],
                level          => $depth,
                start_line_num => 0 + @$ret
            )
        } @{ $node->{''} };
    }

    my %skip;
    foreach my $expr (
        map  { $_->[0] }
        sort { $a->[1] cmp $b->[1] || $a->[0] cmp $b->[0] }
        map  { [ $_, lc($_) =~ s/[^A-Za-z0-9]+//gr ] } keys %$node
    ) {
        next unless length $expr;    # ignore payload
        my $not= $self->tidy_cond("!($expr)");
        if ($skip{$expr} or ($not !~ /^!/ and $node->{$not})) {
            next;
        }
        my $kid= $node->{$expr};
        while (!$node->{$not} and keys(%$kid) == 1 and !$kid->{''}) {
            my ($kid_key)= keys(%$kid);
            $expr= $self->tidy_cond("($expr) && ($kid_key)");
            $kid= $kid->{$kid_key};
            my $new_not= $self->tidy_cond("!($expr)");
            if ($node->{$new_not}) {
                $not= $new_not;
                $skip{$not}++;
            }
        }
        my $raw= "#${ind}if $expr\n";
        my $hl= HeaderLine->new(
            type           => "cond",
            sub_type       => "#if",
            raw            => $raw,
            line           => $raw,
            level          => $depth,
            cond           => [ @path, [$expr] ],
            start_line_num => 0 + @$ret,
        );
        $self->{line_by_depth}[$depth]= 0 + @$ret;
        push @$ret, $hl;
        $self->__recurse_group_content_tree($kid, @path, [$expr]);
        if ($node->{$not}) {
            $skip{$not}++;
            $self->_handle_else($not, $node->{$not}, $ind, $depth, @path,
                [$not]);
        }

        # and finally the #endif

        $raw= "#${ind}endif\n";

        # we need to extract the condition information from the last line in @ret,
        # as we don't know which condition we are ending here. It could be an elsif
        # from deep in the parse tree for instance.
        # So we need to extract the last frame from the cond structure in the last
        # line-info in @ret.
        # BUT if this last line is itself an #endif, then we need to take the second
        # to last line instead, as the endif would have "popped" that frame off the
        # condition stack.
        my $last_ret= $ret->[-1];
        my $idx=
            ($last_ret->{type} eq "cond" && $last_ret->{sub_type} eq "#endif")
            ? -2
            : -1;
        my $end_line= HeaderLine->new(
            type           => "cond",
            sub_type       => "#endif",
            raw            => $raw,
            line           => $raw,
            level          => $depth,
            cond           => [ @path, $last_ret->{cond}[$idx] ],
            start_line_num => 0 + @$ret,
            inner_lines    => @$ret - $self->{line_by_depth}[$depth],
        );
        undef $self->{line_by_depth}[$depth];
        push @$ret, $end_line;
    }
    return $ret;
}

# this handles the specific case of an else clause, detecting
# when an elif can be constructed, may recursively call itself
# to deal with if/elif/elif/else chains. Calls back into
# __recurse_group_content_tree().
sub _handle_else {
    my ($self, $not, $kid, $ind, $depth, @path)= @_;

    # extract the first 3 keys - from this we can detect
    # which of the three scenarios we have to handle.
    my ($k1, $k2, $k3)=
        sort { length($a) <=> length($b) || $a cmp $b } keys %$kid;
    my $not_k1;
    if (length($k1) and defined($k2) and !defined($k3)) {

        # if we do not have a payload (length($k1)) and we have exactly
        # two keys (defined($k2) and !defined($k3)) we need to compute
        # the inverse of $k1, which we will use later.
        $not_k1= $self->tidy_cond("!($k1)");
    }
    my $ret= $self->{rgct_ret};
    if (length($k1) and !defined($k2)) {

        # only one child, no payload -> elsif $k1
        my $sub_expr;
        do {
            $sub_expr=
                 !$sub_expr
                ? $k1
                : $self->tidy_cond("($sub_expr) && ($k1)");
            $kid= $kid->{$k1};
            ($k1, $k2)=
                sort { length($a) <=> length($b) || $a cmp $b } keys %$kid;
        } while length($k1) and !defined $k2;

        my $raw= "#${ind}elif $sub_expr\n";
        push @{ $path[-1] }, $sub_expr;
        my $hl= HeaderLine->new(
            type           => "cond",
            sub_type       => "#elif",
            raw            => $raw,
            line           => $raw,
            level          => $depth,
            cond           => [ map { [@$_] } @path ],
            start_line_num => 0 + @$ret,
            inner_lines    => @$ret - $self->{line_by_depth}[$depth],
        );
        $self->{line_by_depth}[$depth]= 0 + @$ret;
        push @$ret, $hl;
        $self->__recurse_group_content_tree($kid, @path);
    }
    elsif (defined($not_k1) and $not_k1 eq $k2) {

        # two children which are complementary, no payload -> elif $k1 else..
        my $raw= "#${ind}elif $k1\n";

        push @{ $path[-1] }, $k1;
        my $hl= HeaderLine->new(
            type           => "cond",
            sub_type       => "#elif",
            raw            => $raw,
            line           => $raw,
            level          => $depth,
            cond           => [ map { [@$_] } @path ],
            start_line_num => 0 + @$ret,
            inner_lines    => @$ret - $self->{line_by_depth}[$depth],
        );
        $self->{line_by_depth}[$depth]= 0 + @$ret;
        push @$ret, $hl;
        $self->__recurse_group_content_tree($kid->{$k1}, @path);
        $path[-1][-1]= $k2;
        $self->_handle_else($k2, $kid->{$k2}, $ind, $depth, @path);
    }
    else {
        # payload, 3+ children, or 2 which are not complementary -> else
        my $raw= "#${ind}else\n";
        my $hl= HeaderLine->new(
            type           => "cond",
            sub_type       => "#else",
            raw            => $raw,
            line           => $raw,
            level          => $depth,
            cond           => [ map { [@$_] } @path ],
            start_line_num => 0 + @$ret,
            inner_lines    => @$ret - $self->{line_by_depth}[$depth],
        );
        $self->{line_by_depth}[$depth]= 0 + @$ret;
        push @$ret, $hl;
        $self->__recurse_group_content_tree($kid, @path);
    }
    return $ret;
}

# group the content in lines by the condition that apply to them
# returns a set of lines representing the new structure
sub group_content {
    my ($self, $lines, $filter)= @_;
    $lines ||= $self->{lines};
    local $self->{filter_content}= $filter || $self->{filter_content};
    my $tree= $self->_build_group_content_tree($lines);
    return $self->_recurse_group_content_tree($tree);
}

#read a file by name - opens the file and passes the fh into parse_fh().
sub read_file {
    my ($self, $file_name, $callback)= @_;
    $self= $self->new() unless ref $self;
    local $self->{parse_source}= $file_name;
    open my $fh, "<", $file_name
        or confess "Failed to open '$file_name' for read: $!";
    my $lines= $self->parse_fh($fh);
    if ($callback) {
        foreach my $line (@$lines) {
            $callback->($self, $line);
        }
    }
    return $self;
}

# These are utility methods for the HeaderLine objects.
sub HeaderLine::new {
    my ($class, %self)= @_;
    return bless \%self, $class;
}
sub HeaderLine::cond        { $_[0]->{cond} }                             # AoA
sub HeaderLine::type        { $_[0]->{type} }
sub HeaderLine::type_is     { return $_[0]->type eq $_[1] ? 1 : 0 }
sub HeaderLine::sub_type    { $_[0]->{sub_type} }
sub HeaderLine::sub_type_is { return $_[0]->sub_type eq $_[1] ? 1 : 0 }
sub HeaderLine::raw         { $_[0]->{raw} }
sub HeaderLine::flat        { $_[0]->{flat} }
sub HeaderLine::line        { $_[0]->{line} }
sub HeaderLine::level       { $_[0]->{level} }
sub HeaderLine::is_content  { return $_[0]->type_is("content") }
sub HeaderLine::is_cond     { return $_[0]->type_is("cond") }
sub HeaderLine::is_define   { return $_[0]->sub_type_is("#define") }
sub HeaderLine::line_num    { $_[0]->{start_line_num} }
sub HeaderLine::inner_lines { $_[0]->{inner_lines} }
sub HeaderLine::n_lines     { $_[0]->{n_lines} }
sub HeaderLine::embed       { $_[0]->{embed} }
*HeaderLine::start_line_num= *HeaderLine::line_num;

# these are methods for EmbedLine objects
*EmbedLine::new= *HeaderLine::new;
sub EmbedLine::flags       { $_[0]->{flags} }
sub EmbedLine::return_type { $_[0]->{return_type} }
sub EmbedLine::name        { $_[0]->{name} }
sub EmbedLine::args        { $_[0]->{args} }          # array ref

1;

__END__

=head1 NAME

HeaderParser - A minimal header file parser that can be hooked by other porting
scripts.

=head1 SYNOPSIS

    my $o= HeaderParser->new();
    my $lines= $o->parse_fh($fh);

=head1 DESCRIPTION

HeaderParser is a tool to parse C preprocessor header files. The tool
understands the syntax of preprocessor conditions, and is capable of creating
a parse tree of the expressions involved, and normalizing them as well.

C preprocessor files are a bit tricky to parse properly, especially with a
"line by line" model. There are two issues that must be dealt with:

=over 4

=item Line Continuations

Any line ending in "\\\n" (that is backslash newline) is considered to be part
of a longer string which continues on the next line. Processors should replace
the "\\\n" typically with a space when converting to a "real" line.

=item Comments Acting As A Line Continuation

The rules for header files stipulates that C style comments are stripped
before processing other content, this means that comments can serve as a form
of line continuation:

    #if defined(foo) /*
    */ && defined(bar)

is the same as

    #if defined(foo) && defined(bar)

This type of comment usage is often overlooked by people writing header file
parsers for the first time.

=item Indented pre processor directives.

It is easy to forget that there may be multiple spaces between the "#"
character and the directive. It also easy to forget that there may be spaces
in *front* of the "#" character. Both of these cases are often overlooked.

=back

The main idea of this module is to provide a single framework for correctly
parsing the content of our header files in a consistent manner. A secondary
purpose it to make various tasks we want to do easier, such as normalizing
content or preprocessor expressions, or just extracting the real "content" of
the file properly.

=head2 parse_fh

This function parses a filehandle into a set of lines.  Each line is represented by a hash
based object which contains the following fields:

    bless {
        cond     => [['defined(a)'],['defined(b)']],
        type     => "content",
        sub_type => undef,
        raw      => $raw_content_of_line,
        line     => $normalized_content_of_line,
        level    => $level,
        source         => $filename_or_string,
        start_line_num => $line_num_for_first_line,
        n_lines        => $line_num - $line_num_for_first_line,
    }, "HeaderLine"

A "line" in this context is a logical line, and because of line continuations
and comments may contain more than one physical line, and thus more than
one newline, but will always include at least one and will always end with one
(unless there is no newline at the end of the file). Thus

    before /*
     this is a comment
    */ after \
    and continues

will be treated as a single logical line even though the content is
spread over four lines.

=over 4

=item cond

An array of arrays containing the normalized expressions of any C preprocessor
conditional blocks which include the line. Each line has its own copy of the
conditions it was operated on currently, but that may change so dont alter
this data. The inner arrays may contain more than one element. If so then the
line is part of an "#else" or "#elsif" and the clauses should be considered to
be a conjuction when considering "when is this line included", however when
considered as part of an if/elsif/else, each added clause represents the most
recent condition. In the following you can see how:

    before          /* cond => [ ]                      */
    #if A           /* cond => [ ['A'] ]                */
    do-a            /* cond => [ ['A'] ]                */
    #elif B         /* cond => [ ['!A', 'B'] ]          */
    do-b            /* cond => [ ['!A', 'B'] ]          */
    #else           /* cond => [ ['!A', '!B'] ]         */
    do-c            /* cond => [ ['!A', '!B'] ]         */
    # if D          /* cond => [ ['!A', '!B'], ['D'] ]  */
    do-d            /* cond => [ ['!A', '!B'], ['D'] ]  */
    # endif         /* cond => [ ['!A', '!B'], ['D'] ]  */
    #endif          /* cond => [ ['!A', '!B'] ]         */
    after           /* cond => [ ]                      */

So in the above we can see how the three clauses of the if produce
a single "frame" in the cond array, but that frame "grows" and changes
as additional else clauses are added. When an entirely new if block
is started (D) it gets its own block. Each endif includes the clause
it terminates.

=item type

This value indicates the type of the line. This may be one of the following:
'content', 'cond', 'define', 'include' and 'error'. Several of the types
have a sub_type.

=item sub_type

This value gives more detail on the type of the line where necessary.
Not all types have a subtype.

    Type    | Sub Type
    --------+----------
    content | text
            | include
            | define
            | error
    cond    | #if
            | #elif
            | #else
            | #endif

Note that there are no '#ifdef' or '#elifndef' or similar expressions. All
expressions of that form are normalized into the '#if defined' form to
simplify processing.

=item raw

This was the raw original text before HeaderParser performed any modifications
to it.

=item line

This is the normalized and modified text after HeaderParser or any callbacks
have processed it.

=item level

This is the "indent level" of a line and corresponds to the number of blocks
that the line is within, not including any blocks that might be created by
the line itself.

    before          /* level => 0 */
    #if A           /* level => 0 */
    do-a            /* level => 1 */
    #elif B         /* level => 0 */
    do-b            /* level => 1 */
    #else           /* level => 0 */
    do-c            /* level => 1 */
    # if D          /* level => 1 */
    do-d            /* level => 2 */
    # endif         /* level => 1 */
    #endif          /* level => 0 */
    after           /* level => 0 */

=back

parse_fh() will throw an exception if it encounters a malformed expression
or input it cannot handle.

=head2 lines_as_str

This function will return a string representation of the lines it is provided.

=head2 group_content

This function will group the text in the file by the conditions which contain
it. This is only useful for files where the content is essentially a list and
where changing the order that lines are output in will not break the resulting
file.

Each content line will be grouped into a structure of nested if/else blocks
(elif will produce a new nested block) such that the content under the control
of a given set of normalized condition clauses are grouped together in the order
the occurred in the file, such that each combined conditional clause is output
only once.

This means a file like this:

    #if A
    A
    #elif K
    AK
    #else
    ZA
    #endif
    #if B && Q
    B
    #endif
    #if Q && B
    BC
    #endif
    #if A
    AD
    #endif
    #if !A
    ZZ
    #endif

Will end up looking roughly like this:

    #if A
    A
    AD
    #else
    ZZ
    # if K
    AK
    # else
    ZA
    # endif
    #endif
    #if B && Q
    B
    BC
    #endif

Content at a given block level always goes before conditional clauses
at the same nesting level.

=head2 HOOKS

There are severals hooks that are available, C<pre_process_content> and
C<post_process_content>, and C<post_process_grouped_content>. All of these
hooks  will be called with the HeaderParser object as the first argument.
The "process_content" callbacks will be called with a line hash as the second
argument, and C<post_process_grouped_content> will be called with an
array of line hashes for the content in that group, so that the array may be
modified or sorted.  Callbacks called from inside of C<group_content()>
(that is C<post_process_content> and C<post_process_grouped_content> will be
called with an additional argument containing and array specifying the actual
conditional "path" to the content  (which may differ somewhat from the data in
a lines "cond" property).

These hooks may do what they like, but generally they will modify the
"line" property of the line hash to change the final output returned
by C<lines_as_str()> or C<group_content()>.

=head2 FORMATTING AND INDENTING

Header parser tries hard to produce neat and readable output with a consistent
style and form. For example:

    #if defined(FOO)
    # define HAS_FOO
    # if defined(BAR)
    #   define HAS_FOO_AND_BAR
    # else /* !defined(BAR) */
    #   define HAS_FOO_NO_BAR
    # endif /* !defined(BAR) */
    #endif /* defined(FOO) */

HeaderParser uses two space tab stops for indenting C pre-processor
directives. It puts the spaces between the "#" and the directive. The "#" is
considered "part" of the indent, even though the space comes after it. This
means the first indent level "looks" like one space, and following indents
look like 2. This should match what a sensible editor would do with two space
tab stops. The C<indent_chars()> method can be used to convert an indent level
into a string that contains the appropriate number of spaces to go in between
the "#" and the directive.

When emitting "#endif", "#elif" and "#else" directives comments will be
emitted also to show the conditions that apply. These comments may be wrapped
to cover multiple lines. Some effort is made to get these comments to line up
visually, but it uses heuristics which may not always produce the best result.

=cut
