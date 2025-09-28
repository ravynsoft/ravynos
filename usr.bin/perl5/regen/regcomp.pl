#!/usr/bin/perl -w
#
#
# Regenerate (overwriting only if changed):
#
#    pod/perldebguts.pod
#    regnodes.h
#
# from information stored in
#
#    regcomp.sym
#    op_reg_common.h
#    regexp.h
#
# pod/perldebguts.pod is not completely regenerated.  Only the table of
# regexp nodes is replaced; other parts remain unchanged.
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.
#
# F<regcomp.sym> defines the opcodes and states used in the regex
# engine, it also includes documentation on the opcodes. This script
# parses those definitions out and turns them into typedefs, defines,
# and data structures, and maybe even code which the regex engine can
# use to operate.
#
# F<regexp.h> and op_reg_common.h contain defines C<RXf_xxx> and
# C<PREGf_xxx> that are used in flags in our code. These defines are
# parsed out and data structures are created to allow the debug mode of
# the regex engine to show things such as which flags were set during
# compilation. In some cases we transform the C code in the header files
# into perl code which we execute to C<eval()> the contents. For instance
# in a situation like this:
#
#   #define RXf_X 0x1   /* the X mode */
#   #define RXf_Y 0x2   /* the Y mode */
#   #define RXf_Z (X|Y) /* the Z mode */
#
# this script might end up eval()ing something like C<0x1> and then
# C<0x2> and then C<(0x1|0x2)> the results of which it then might use in
# constructing a data structure, or pod in perldebguts, or a comment in
# C<regnodes.h>. It also would separate out the "X", "Y", and "Z" and
# use them, and would also use the data in the line comment if present.
#
# If you compile a regex under perl -Mre=Debug,ALL you can see much
# of the content that this file generates and parses out of its input
# files.

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
    require './regen/HeaderParser.pm';
}

use strict;

# NOTE I don't think anyone actually knows what all of these properties mean,
# and I suspect some of them are outright unused. This is a first attempt to
# clean up the generation so maybe one day we can move to something more self
# documenting. (One might argue that an array of hashes of properties would
# be easier to use.)
#
# Why we use the term regnode and nodes, and not say, opcodes, I am not sure.

# General thoughts:
# 1. We use a single continuum to represent both opcodes and states,
#    and in regexec.c we switch on the combined set.
# 2. Opcodes have more information associated to them, states are simpler,
#    basically just an identifier/number that can be used to switch within
#    the state machine.
# 3. Some opcode are order dependent.
# 4. Output files often use "tricks" to reduce diff effects. Some of what
#    we do below is more clumsy looking than it could be because of this.

# Op/state properties:
#
# Property      In      Descr
# ----------------------------------------------------------------------------
# name          Both    Name of op/state
# id            Both    integer value for this opcode/state
# optype        Both    Either 'op' or 'state'
# line_num      Both    line_num number of the input file for this item.
# type          Op      Type of node (aka regnode_kind)
# code          Op      Meta about the node, used to detect variable length nodes
# suffix        Op      which regnode struct this uses, so if this is '1', it
#                       uses 'struct regnode_1'
# flags         Op      S for simple; V for varies
# longj         Op      Boolean as to if this node is a longjump
# comment       Both    Comment about node, if any.  Placed in perlredebguts
#                       as its description
# pod_comment   Both    Special comments for pod output (preceding lines in def)
#                       Such lines begin with '#*'

# Global State
my @all;    # all opcodes/state
my %all;    # hash of all opcode/state names

my @ops;    # array of just opcodes
my @states; # array of just states

my $longest_name_length= 0; # track lengths of names for nicer reports
my (%type_alias);           # map the type (??)

# register a newly constructed node into our state tables.
# ensures that we have no name collisions (on name anyway),
# and issues the "id" for the node.
sub register_node {
    my ($node)= @_;

    if ( $all{ $node->{name} } ) {
        die "Duplicate item '$node->{name}' in regcomp.sym line $node->{line_num} "
            . "previously defined on line $all{ $node->{name} }{line_num}\n";
    } elsif (!$node->{optype}) {
        die "must have an optype in node ", Dumper($node);
    } elsif ($node->{optype} eq "op") {
        push @ops, $node;
    } elsif ($node->{optype} eq "state") {
        push @states, $node;
    } else {
        die "Uknown optype '$node->{optype}' in ", Dumper($node);
    }
    $node->{id}= 0 + @all;
    push @all, $node;
    $all{ $node->{name} }= $node;

    if ($node->{longj} && $node->{longj} != 1) {
        die "longj field must be in [01] if present in ", Dumper($node);
    }

}

# Parse and add an opcode definition to the global state.
# What an opcode definition looks like is given in regcomp.sym.
#
# Not every opcode definition has all of the components. We should maybe make
# this nicer/easier to read in the future. Also note that the above is tab
# sensitive.

# Special comments for an entry precede it, and begin with '#*' and are placed
# in the generated pod file just before the entry.

sub parse_opcode_def {
    my ( $text, $line_num, $pod_comment )= @_;
    my $node= {
        line_num    => $line_num,
        pod_comment => $pod_comment,
        optype      => "op",
    };

    # first split the line into three, the initial NAME, a middle part
    # that we call "desc" which contains various (not well documented) things,
    # and a comment section.
    @{$node}{qw(name desc comment)}= /^(\S+)\s+([^\t]+?)\s*;\s*(.*)/
        or die "Failed to match $_";

    # the content of the "desc" field from the first step is extracted here:
    @{$node}{qw(type code suffix flags longj)}= split /[,\s]\s*/, $node->{desc};

    defined $node->{$_} or $node->{$_} = ""
        for qw(type code suffix flags longj);

    register_node($node); # has to be before the type_alias code below

    if ( !$all{ $node->{type} } and !$type_alias{ $node->{type} } ) {

        #warn "Regop type '$node->{type}' from regcomp.sym line $line_num"
        #     ." is not an existing regop, and will be aliased to $node->{name}\n"
        #    if -t STDERR;
        $type_alias{ $node->{type} }= $node->{name};
    }

    $longest_name_length= length $node->{name}
        if length $node->{name} > $longest_name_length;
}

# parse out a state definition and add the resulting data
# into the global state. may create multiple new states from
# a single definition (this is part of the point).
# Format for states:
# REGOP \t typelist [ \t typelist]
# typelist= namelist
#         = namelist:FAIL
#         = name:count
# Eg:
# WHILEM          A_pre,A_min,A_max,B_min,B_max:FAIL
# BRANCH          next:FAIL
# CURLYM          A,B:FAIL
#
# The CURLYM definition would create the states:
# CURLYM_A, CURLYM_A_fail, CURLYM_B, CURLYM_B_fail
sub parse_state_def {
    my ( $text, $line_num, $pod_comment )= @_;
    my ( $type, @lists )= split /\s+/, $text;
    die "No list? $type" if !@lists;
    foreach my $list (@lists) {
        my ( $names, $special )= split /:/, $list, 2;
        $special ||= "";
        foreach my $name ( split /,/, $names ) {
            my $real=
                $name eq 'resume'
                ? "resume_$type"
                : "${type}_$name";
            my @suffix;
            if ( !$special ) {
                @suffix= ("");
            }
            elsif ( $special =~ /\d/ ) {
                @suffix= ( 1 .. $special );
            }
            elsif ( $special eq 'FAIL' ) {
                @suffix= ( "", "_fail" );
            }
            else {
                die "unknown :type ':$special'";
            }
            foreach my $suffix (@suffix) {
                my $node= {
                    name        => "$real$suffix",
                    optype      => "state",
                    type        => $type || "",
                    comment     => "state for $type",
                    line_num    => $line_num,
                };
                register_node($node);
            }
        }
    }
}

sub process_flags {
    my ( $flag, $varname, $comment )= @_;
    $comment= '' unless defined $comment;

    my @selected;
    my $bitmap= '';
    for my $node (@ops) {
        my $set= $node->{flags} && $node->{flags} eq $flag ? 1 : 0;

        # Whilst I could do this with vec, I'd prefer to do longhand the arithmetic
        # ops in the C code.
        my $current= do {
            no warnings;
            ord substr $bitmap, ( $node->{id} >> 3 );
        };
        substr( $bitmap, ( $node->{id} >> 3 ), 1 )=
            chr( $current | ( $set << ( $node->{id} & 7 ) ) );

        push @selected, $node->{name} if $set;
    }
    my $out_string= join ', ', @selected, 0;
    $out_string =~ s/(.{1,70},) /$1\n    /g;

    my $out_mask= join ', ', map { sprintf "0x%02X", ord $_ } split '', $bitmap;

    return $comment . <<"EOP";
#define REGNODE_\U$varname\E(node) (PL_${varname}_bitmask[(node) >> 3] & (1 << ((node) & 7)))

#ifndef DOINIT
EXTCONST U8 PL_${varname}\[] __attribute__deprecated__;
#else
EXTCONST U8 PL_${varname}\[] __attribute__deprecated__ = {
    $out_string
};
#endif /* DOINIT */

#ifndef DOINIT
EXTCONST U8 PL_${varname}_bitmask[];
#else
EXTCONST U8 PL_${varname}_bitmask[] = {
    $out_mask
};
#endif /* DOINIT */
EOP
}

sub print_process_EXACTish {
    my ($out)= @_;

    # Creates some bitmaps for EXACTish nodes.

    my @folded;
    my @req8;

    my $base;
    for my $node (@ops) {
        next unless $node->{type} eq 'EXACT';
        my $name = $node->{name};
        $base = $node->{id} if $name eq 'EXACT';

        my $index = $node->{id} - $base;

        # This depends entirely on naming conventions in regcomp.sym
        $folded[$index] = $name =~ /^EXACTF/ || 0;
        $req8[$index] = $name =~ /8/ || 0;
    }

    die "Can't cope with > 32 EXACTish nodes" if @folded > 32;

    my $exactf = sprintf "%X", oct("0b" . join "", reverse @folded);
    my $req8 =   sprintf "%X", oct("0b" . join "", reverse @req8);
    print $out <<EOP,

/* Is 'op', known to be of type EXACT, folding? */
#define isEXACTFish(op) (__ASSERT_(REGNODE_TYPE(op) == EXACT) (PL_EXACTFish_bitmask & (1U << (op - EXACT))))

/* Do only UTF-8 target strings match 'op', known to be of type EXACT? */
#define isEXACT_REQ8(op) (__ASSERT_(REGNODE_TYPE(op) == EXACT) (PL_EXACT_REQ8_bitmask & (1U << (op - EXACT))))

#ifndef DOINIT
EXTCONST U32 PL_EXACTFish_bitmask;
EXTCONST U32 PL_EXACT_REQ8_bitmask;
#else
EXTCONST U32 PL_EXACTFish_bitmask = 0x$exactf;
EXTCONST U32 PL_EXACT_REQ8_bitmask = 0x$req8;
#endif /* DOINIT */
EOP
}

sub read_definition {
    my ( $file )= @_;
    my ( $seen_sep, $pod_comment )= "";
    open my $in_fh, "<", $file
        or die "Failed to open '$file' for reading: $!";
    while (<$in_fh>) {

        # Special pod comments
        if (/^#\* ?/) { $pod_comment .= "# $'"; }

        # Truly blank lines possibly surrounding pod comments
        elsif (/^\s*$/) { $pod_comment .= "\n" }

        next if /\A\s*#/ || /\A\s*\z/;

        s/\s*\z//;
        if (/^-+\s*$/) {
            $seen_sep= 1;
            next;
        }

        if ($seen_sep) {
            parse_state_def( $_, $., $pod_comment );
        }
        else {
            parse_opcode_def( $_, $., $pod_comment );
        }
        $pod_comment= "";
    }
    close $in_fh;
    die "Too many regexp/state opcodes! Maximum is 256, but there are ", 0 + @all,
        " in file!"
        if @all > 256;
}

# use fixed width to keep the diffs between regcomp.pl recompiles
# as small as possible.
my ( $base_name_width, $rwidth, $twidth )= ( 22, 12, 9 );

sub print_state_defs {
    my ($out)= @_;
    printf $out <<EOP,
/* Regops and State definitions */

#define %*s\t%d
#define %*s\t%d

EOP
        -$base_name_width,
        REGNODE_MAX => $#ops,
        -$base_name_width, REGMATCH_STATE_MAX => $#all;

    my %rev_type_alias= reverse %type_alias;
    my $base_format = "#define %*s\t%d\t/* %#04x %s */\n";
    my @withs;
    my $in_states = 0;

    my $max_name_width = 0;
    for my $ref (\@ops, \@states) {
        for my $node ($ref->@*) {
            my $len = length $node->{name};
            $max_name_width = $len if $max_name_width < $len;
        }
    }

    die "Do a white-space only commit to increase \$base_name_width to"
     .  " $max_name_width; then re-run"  if $base_name_width < $max_name_width;

    print $out <<EOT;
/* -- For regexec.c to switch on target being utf8 (t8) or not (tb, b='byte'); */
#define with_t_UTF8ness(op, t_utf8) (((op) << 1) + (cBOOL(t_utf8)))
/* -- same, but also with pattern (p8, pb) -- */
#define with_tp_UTF8ness(op, t_utf8, p_utf8)                        \\
\t\t(((op) << 2) + (cBOOL(t_utf8) << 1) + cBOOL(p_utf8))

/* The #defines below give both the basic regnode and the expanded version for
   switching on utf8ness */
EOT

    for my $node (@ops) {
        print_state_def_line($out, $node->{name}, $node->{id}, $node->{comment});
        if ( defined( my $alias= $rev_type_alias{ $node->{name} } ) ) {
            print_state_def_line($out, $alias, $node->{id}, $node->{comment});
        }
    }

    print $out "\t/* ------------ States ------------- */\n";
    for my $node (@states) {
        print_state_def_line($out, $node->{name}, $node->{id}, $node->{comment});
    }
}

sub print_state_def_line
{
    my ($fh, $name, $id, $comment) = @_;

    # The sub-names are like '_tb' or '_tb_p8' = max 6 chars wide
    my $name_col_width = $base_name_width + 6;
    my $base_id_width = 3;  # Max is '255' or 3 cols
    my $mid_id_width  = 3;  # Max is '511' or 3 cols
    my $full_id_width = 3;  # Max is '1023' but not close to using the 4th

    my $line = "#define " . $name;
    $line .= " " x ($name_col_width - length($name));

    $line .= sprintf "%*s", $base_id_width, $id;
    $line .= " " x $mid_id_width;
    $line .= " " x ($full_id_width + 2);

    $line .= "/* ";
    my $hanging = length $line;     # Indent any subsequent line to this pos
    $line .= sprintf "0x%02x", $id;

    my $columns = 78;

    # From the documentation: 'In fact, every resulting line will have length
    # of no more than "$columns - 1"'
    $line = wrap($columns + 1, "", " " x $hanging, "$line $comment");
    chomp $line;            # wrap always adds a trailing \n
    $line =~ s/ \s+ $ //x;  # trim, just in case.

    # The comment may have wrapped.  Find the final \n and measure the length
    # to the end.  If it is short enough, just append the ' */' to the line.
    # If it is too close to the end of the space available, add an extra line
    # that consists solely of blanks and the ' */'
    my $len = length($line); my $rindex = rindex($line, "\n");
    if (length($line) - rindex($line, "\n") - 1 <= $columns - 3) {
        $line .= " */\n";
    }
    else {
        $line .= "\n" . " " x ($hanging - 3) . "*/\n";
    }

    print $fh $line;

    # And add the 2 subsidiary #defines used when switching on
    # with_t_UTF8nes()
    my $with_id_t = $id * 2;
    for my $with (qw(tb  t8)) {
        my $with_name = "${name}_$with";
        print  $fh "#define ", $with_name;
        print  $fh " " x ($name_col_width - length($with_name) + $base_id_width);
        printf $fh "%*s", $mid_id_width, $with_id_t;
        print  $fh " " x $full_id_width;
        printf $fh "  /*";
        print  $fh " " x (4 + 2);  # 4 is width of 0xHH that the base entry uses
        printf $fh "0x%03x */\n", $with_id_t;

        $with_id_t++;
    }

    # Finally add the 4 subsidiary #defines used when switching on
    # with_tp_UTF8nes()
    my $with_id_tp = $id * 4;
    for my $with (qw(tb_pb  tb_p8  t8_pb  t8_p8)) {
        my $with_name = "${name}_$with";
        print  $fh "#define ", $with_name;
        print  $fh " " x ($name_col_width - length($with_name) + $base_id_width + $mid_id_width);
        printf $fh "%*s", $full_id_width, $with_id_tp;
        printf $fh "  /*";
        print  $fh " " x (4 + 2);  # 4 is width of 0xHH that the base entry uses
        printf $fh "0x%03x */\n", $with_id_tp;

        $with_id_tp++;
    }

    print $fh "\n"; # Blank line separates groups for clarity
}

sub print_typedefs {
    my ($out)= @_;
    print $out <<EOP;

/* typedefs for regex nodes - one typedef per node type */

EOP
    my $len= 0;
    foreach my $node (@ops) {
        if ($node->{suffix} and $len < length($node->{suffix})) {
            $len= length $node->{suffix};
        }
    }
    $len += length "struct regnode_";
    $len = (int($len/5)+2)*5;
    my $prefix= "tregnode";

    foreach my $node (sort { $a->{name} cmp $b->{name} } @ops) {
        my $struct_name= "struct regnode";
        if (my $suffix= $node->{suffix}) {
            $struct_name .= "_$suffix";
        }
        $node->{typedef}= $prefix . "_" . $node->{name};
        printf $out "typedef %*s %s;\n", -$len, $struct_name, $node->{typedef};
    }
    print $out <<EOP;

/* end typedefs */

EOP

}




sub print_regnode_info {
    my ($out)= @_;
    print $out <<EOP;

/* PL_regnode_info[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST struct regnode_meta PL_regnode_info[];
#else
EXTCONST struct regnode_meta PL_regnode_info[] = {
EOP
    my @fields= qw(type arg_len arg_len_varies off_by_arg);
    foreach my $node_idx (0..$#all) {
        my $node= $all[$node_idx];
        {
            my $size= 0;
            $size= "EXTRA_SIZE($node->{typedef})" if $node->{suffix};
            $node->{arg_len}= $size;

        }
        {
            my $varies= 0;
            $varies= 1 if $node->{code} and $node->{code}=~"str";
            $node->{arg_len_varies}= $varies;
        }
        $node->{off_by_arg}= $node->{longj} || 0;
        print $out "    {\n";
        print $out "        /* #$node_idx $node->{optype} $node->{name} */\n";
        foreach my $f_idx (0..$#fields) {
            my $field= $fields[$f_idx];
            printf $out  "        .%s = %s", $field, $node->{$field} // 0;
            printf $out $f_idx == $#fields ? "\n" : ",\n";
        }
        print $out "    }";
        print $out $node_idx==$#all ? "\n" : ",\n";
    }

    print $out <<EOP;
};
#endif /* DOINIT */

EOP
}


sub print_regnode_name {
    my ($out)= @_;
    print $out <<EOP;

/* PL_regnode_name[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST char * PL_regnode_name[];
#else
EXTCONST char * const PL_regnode_name[] = {
EOP

    my $ofs= 0;
    my $sym= "";
    foreach my $node (@all) {
        printf $out "\t%*s\t/* $sym%#04x */\n",
            -3 - $base_name_width, qq("$node->{name}",), $node->{id} - $ofs;
        if ( $node->{id} == $#ops and @ops != @all ) {
            print $out "\t/* ------------ States ------------- */\n";
            $ofs= $#ops;
            $sym= 'REGNODE_MAX +';
        }
    }

    print $out <<EOP;
};
#endif /* DOINIT */

EOP
}

sub print_reg_extflags_name {
    my ($out)= @_;
    print $out <<EOP;
/* PL_reg_extflags_name[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST char * PL_reg_extflags_name[];
#else
EXTCONST char * const PL_reg_extflags_name[] = {
EOP

    my %rxfv;
    my %definitions;    # Remember what the symbol definitions are
    my $val= 0;
    my %reverse;
    my $REG_EXTFLAGS_NAME_SIZE= 0;
    my $hp= HeaderParser->new();
    foreach my $file ( "op_reg_common.h", "regexp.h" ) {
        $hp->read_file($file);
        foreach my $line_info (@{$hp->lines}) {
            next unless $line_info->{type}     eq "content"
                    and $line_info->{sub_type} eq "#define";
            my $line= $line_info->{line};
            $line=~s/\s*\\\n\s*/ /g;

            # optional leading '_'.  Return symbol in $1, and strip it from
            # comment of line.  Currently doesn't handle comments running onto
            # next line
            if ($line=~s/^ \# \s* define \s+ ( _? RXf_ \w+ ) \s+ //xi) {
                chomp($line);
                my $define= $1;
                my $orig= $_;
                $line=~s{ /\* .*? \*/ }{ }x;    # Replace comments by a blank

                # Replace any prior defined symbols by their values
                foreach my $key ( keys %definitions ) {
                    $line=~s/\b$key\b/$definitions{$key}/g;
                }

                # Remove the U suffix from unsigned int literals
                $line=~s/\b([0-9]+)U\b/$1/g;

                my $newval= eval $line;     # Get numeric definition

                $definitions{$define}= $newval;

                next unless $line =~ /<</;    # Bit defines use left shift
                if ( $val & $newval ) {
                    my @names= ( $define, $reverse{$newval} );
                    s/PMf_// for @names;
                    if ( $names[0] ne $names[1] ) {
                        die sprintf
                            "ERROR: both $define and $reverse{$newval} use 0x%08X (%s:%s)",
                            $newval, $orig, $line;
                    }
                    next;
                }
                $val |= $newval;
                $rxfv{$define}= $newval;
                $reverse{$newval}= $define;
            }
        }
    }
    my %vrxf= reverse %rxfv;
    printf $out "\t/* Bits in extflags defined: %s */\n", unpack 'B*', pack 'N',
        $val;
    my %multibits;
    for ( 0 .. 31 ) {
        my $power_of_2= 2**$_;
        my $n= $vrxf{$power_of_2};
        my $extra= "";
        if ( !$n ) {

            # Here, there was no name that matched exactly the bit.  It could be
            # either that it is unused, or the name matches multiple bits.
            if ( !( $val & $power_of_2 ) ) {
                $n= "UNUSED_BIT_$_";
            }
            else {

                # Here, must be because it matches multiple bits.  Look through
                # all possibilities until find one that matches this one.  Use
                # that name, and all the bits it matches
                foreach my $name ( keys %rxfv ) {
                    if ( $rxfv{$name} & $power_of_2 ) {
                        $n= $name . ( $multibits{$name}++ );
                        $extra= sprintf qq{ : "%s" - 0x%08x}, $name,
                            $rxfv{$name}
                            if $power_of_2 != $rxfv{$name};
                        last;
                    }
                }
            }
        }
        s/\bRXf_(PMf_)?// for $n, $extra;
        printf $out qq(\t%-20s/* 0x%08x%s */\n), qq("$n",), $power_of_2, $extra;
        $REG_EXTFLAGS_NAME_SIZE++;
    }

    print $out <<EOP;
};
#endif /* DOINIT */

#ifdef DEBUGGING
#  define REG_EXTFLAGS_NAME_SIZE $REG_EXTFLAGS_NAME_SIZE
#endif
EOP

}

sub print_reg_intflags_name {
    my ($out)= @_;
    print $out <<EOP;

/* PL_reg_intflags_name[] - Opcode/state names in string form, for debugging */

#ifndef DOINIT
EXTCONST char * PL_reg_intflags_name[];
#else
EXTCONST char * const PL_reg_intflags_name[] = {
EOP

    my %rxfv;
    my %definitions;    # Remember what the symbol definitions are
    my $val= 0;
    my %reverse;
    my $REG_INTFLAGS_NAME_SIZE= 0;
    my $hp= HeaderParser->new();
    my $last_val = 0;
    foreach my $file ("regcomp.h") {
        $hp->read_file($file);
        my @bit_tuples;
        foreach my $line_info (@{$hp->lines}) {
            next unless $line_info->{type}     eq "content"
                    and $line_info->{sub_type} eq "#define";
            my $line= $line_info->{line};
            $line=~s/\s*\\\n\s*/ /g;

            # optional leading '_'.  Return symbol in $1, and strip it from
            # comment of line
            if (
                $line =~ m/^ \# \s* define \s+ ( PREGf_ ( \w+ ) ) \s+ 0x([0-9a-f]+)(?:\s*\/\*(.*)\*\/)?/xi
            ){
                chomp $line;
                my $define= $1;
                my $abbr= $2;
                my $hex= $3;
                my $comment= $4;
                my $val= hex($hex);
                my $bin= sprintf "%b", $val;
                if ($bin=~/1.*?1/) { die "Not expecting multiple bits in PREGf" }
                my $bit= length($bin) - 1 ;
                $comment= $comment ? " - $comment" : "";
                if ($bit_tuples[$bit]) {
                    die "Duplicate PREGf bit '$bit': $define $val ($hex)";
                }
                $bit_tuples[$bit]= [ $bit, $val, $abbr, $define, $comment ];
            }
        }
        foreach my $i (0..$#bit_tuples) {
            my $bit_tuple= $bit_tuples[$i];
            if (!$bit_tuple) {
                $bit_tuple= [ $i, 1<<$i, "", "", "*UNUSED*" ];
            }
            my ($bit, $val, $abbr, $define, $comment)= @$bit_tuple;
            printf $out qq(\t%-30s/* (1<<%2d) - 0x%08x - %s%s */\n),
                qq("$abbr",), $bit, $val, $define, $comment;
        }
        $REG_INTFLAGS_NAME_SIZE=0+@bit_tuples;
    }

    print $out <<EOP;
};
#endif /* DOINIT */

EOP
    print $out <<EOQ;
#ifdef DEBUGGING
#  define REG_INTFLAGS_NAME_SIZE $REG_INTFLAGS_NAME_SIZE
#endif

EOQ
}

sub print_process_flags {
    my ($out)= @_;

    print $out process_flags( 'V', 'varies', <<'EOC');
/* The following have no fixed length. U8 so we can do strchr() on it. */
EOC

    print $out process_flags( 'S', 'simple', <<'EOC');

/* The following always have a length of 1. U8 we can do strchr() on it. */
/* (Note that length 1 means "one character" under UTF8, not "one octet".) */
EOC

}

sub do_perldebguts {
    my $guts= open_new( 'pod/perldebguts.pod', '>' );

    my $node;
    my $code;
    my $name_fmt= '<' x  ( $longest_name_length - 1 );
    my $descr_fmt= '<' x ( 58 - $longest_name_length );
    eval <<EOD or die $@;
format GuTS =
 ^*~~
 \$node->{pod_comment}
 ^$name_fmt ^<<<<<<<<< ^$descr_fmt~~
 \$node->{name}, \$code, defined \$node->{comment} ? \$node->{comment} : ''
.
1;
EOD

    my $old_fh= select($guts);
    $~= "GuTS";

    open my $oldguts, '<', 'pod/perldebguts.pod'
        or die "$0 cannot open pod/perldebguts.pod for reading: $!";
    while (<$oldguts>) {
        print;
        last if /=for regcomp.pl begin/;
    }

    print <<'END_OF_DESCR';

 # TYPE arg-description [regnode-struct-suffix] [longjump-len] DESCRIPTION
END_OF_DESCR
    for my $n (@ops) {
        $node= $n;
        $code= "$node->{code} " . ( $node->{suffix} || "" );
        $code .= " $node->{longj}" if $node->{longj};
        if ( $node->{pod_comment} ||= "" ) {

            # Trim multiple blanks
            $node->{pod_comment} =~ s/^\n\n+/\n/;
            $node->{pod_comment} =~ s/\n\n+$/\n\n/;
        }
        write;
    }
    print "\n";

    while (<$oldguts>) {
        last if /=for regcomp.pl end/;
    }
    do { print } while <$oldguts>; #win32 can't unlink an open FH
    close $oldguts or die "Error closing pod/perldebguts.pod: $!";
    select $old_fh;
    close_and_rename($guts);
}

my $confine_to_core = 'defined(PERL_CORE) || defined(PERL_EXT_RE_BUILD)';
read_definition("regcomp.sym");
if ($ENV{DUMP}) {
    require Data::Dumper;
    print Data::Dumper::Dumper(\@all);
    exit(1);
}
my $out= open_new( 'regnodes.h', '>',
    {
        by      => 'regen/regcomp.pl',
        from    => [ 'regcomp.sym', 'op_reg_common.h', 'regexp.h' ],
    },
);
print $out "#if $confine_to_core\n\n";
print_typedefs($out);
print_state_defs($out);

print_regnode_name($out);
print_regnode_info($out);


print_reg_extflags_name($out);
print_reg_intflags_name($out);
print_process_flags($out);
print_process_EXACTish($out);
print $out "\n#endif /* $confine_to_core */\n";
read_only_bottom_close_and_rename($out);

do_perldebguts();
