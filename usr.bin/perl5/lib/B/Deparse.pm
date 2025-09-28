# B::Deparse.pm
# Copyright (c) 1998-2000, 2002, 2003, 2004, 2005, 2006 Stephen McCamant.
# All rights reserved.
# This module is free software; you can redistribute and/or modify
# it under the same terms as Perl itself.

# This is based on the module of the same name by Malcolm Beattie,
# but essentially none of his code remains.

package B::Deparse 1.74;
use strict;
use Carp;
use B qw(class main_root main_start main_cv svref_2object opnumber perlstring
	 OPf_WANT OPf_WANT_VOID OPf_WANT_SCALAR OPf_WANT_LIST
	 OPf_KIDS OPf_REF OPf_STACKED OPf_SPECIAL OPf_MOD OPf_PARENS
	 OPpLVAL_INTRO OPpOUR_INTRO OPpENTERSUB_AMPER OPpSLICE OPpKVSLICE
         OPpCONST_BARE OPpEMPTYAVHV_IS_HV
	 OPpTRANS_SQUASH OPpTRANS_DELETE OPpTRANS_COMPLEMENT OPpTARGET_MY
	 OPpEXISTS_SUB OPpSORT_NUMERIC OPpSORT_INTEGER OPpREPEAT_DOLIST
	 OPpSORT_REVERSE OPpMULTIDEREF_EXISTS OPpMULTIDEREF_DELETE
         OPpSPLIT_ASSIGN OPpSPLIT_LEX
         OPpPADHV_ISKEYS OPpRV2HV_ISKEYS
         OPpCONCAT_NESTED
         OPpMULTICONCAT_APPEND OPpMULTICONCAT_STRINGIFY OPpMULTICONCAT_FAKE
         OPpTRUEBOOL OPpINDEX_BOOLNEG OPpDEFER_FINALLY
         OPpARG_IF_UNDEF OPpARG_IF_FALSE
	 SVf_IOK SVf_NOK SVf_ROK SVf_POK SVf_FAKE SVs_RMG SVs_SMG
	 SVs_PADTMP
         CVf_NOWARN_AMBIGUOUS CVf_LVALUE
	 PMf_KEEP PMf_GLOBAL PMf_CONTINUE PMf_EVAL PMf_ONCE
	 PMf_MULTILINE PMf_SINGLELINE PMf_FOLD PMf_EXTENDED PMf_EXTENDED_MORE
	 PADNAMEf_OUTER PADNAMEf_OUR PADNAMEf_TYPED
        MDEREF_reload
        MDEREF_AV_pop_rv2av_aelem
        MDEREF_AV_gvsv_vivify_rv2av_aelem
        MDEREF_AV_padsv_vivify_rv2av_aelem
        MDEREF_AV_vivify_rv2av_aelem
        MDEREF_AV_padav_aelem
        MDEREF_AV_gvav_aelem
        MDEREF_HV_pop_rv2hv_helem
        MDEREF_HV_gvsv_vivify_rv2hv_helem
        MDEREF_HV_padsv_vivify_rv2hv_helem
        MDEREF_HV_vivify_rv2hv_helem
        MDEREF_HV_padhv_helem
        MDEREF_HV_gvhv_helem
        MDEREF_ACTION_MASK
        MDEREF_INDEX_none
        MDEREF_INDEX_const
        MDEREF_INDEX_padsv
        MDEREF_INDEX_gvsv
        MDEREF_INDEX_MASK
        MDEREF_FLAG_last
        MDEREF_MASK
        MDEREF_SHIFT
    );

our $AUTOLOAD;
use warnings ();
require feature;

use Config;

BEGIN {
    # List version-specific constants here.
    # Easiest way to keep this code portable between version looks to
    # be to fake up a dummy constant that will never actually be true.
    foreach (qw(OPpSORT_INPLACE OPpSORT_DESCEND OPpITER_REVERSED OPpCONST_NOVER
		OPpPAD_STATE PMf_SKIPWHITE RXf_SKIPWHITE
		PMf_CHARSET PMf_KEEPCOPY PMf_NOCAPTURE CVf_ANONCONST
		CVf_LOCKED OPpREVERSE_INPLACE OPpSUBSTR_REPL_FIRST
		PMf_NONDESTRUCT OPpEVAL_BYTES
		OPpLVREF_TYPE OPpLVREF_SV OPpLVREF_AV OPpLVREF_HV
		OPpLVREF_CV OPpLVREF_ELEM SVpad_STATE)) {
	eval { B->import($_) };
	no strict 'refs';
	*{$_} = sub () {0} unless *{$_}{CODE};
    }
}

# Todo:
#  (See also BUGS section at the end of this file)
#
# - finish tr/// changes
# - add option for even more parens (generalize \&foo change)
# - left/right context
# - copy comments (look at real text with $^P?)
# - avoid semis in one-statement blocks
# - associativity of &&=, ||=, ?:
# - ',' => '=>' (auto-unquote?)
# - break long lines ("\r" as discretionary break?)
# - configurable syntax highlighting: ANSI color, HTML, TeX, etc.
# - more style options: brace style, hex vs. octal, quotes, ...
# - print big ints as hex/octal instead of decimal (heuristic?)
# - handle 'my $x if 0'?
# - version using op_next instead of op_first/sibling?
# - avoid string copies (pass arrays, one big join?)
# - here-docs?

# Current test.deparse failures
# comp/hints 6 - location of BEGIN blocks wrt. block openings
# run/switchI 1 - missing -I switches entirely
#    perl -Ifoo -e 'print @INC'
# op/caller 2 - warning mask propagates backwards before warnings::register
#    'use warnings; BEGIN {${^WARNING_BITS} eq "U"x12;} use warnings::register'
# op/getpid 2 - can't assign to shared my() declaration (threads only)
#    'my $x : shared = 5'
# op/override 7 - parens on overridden require change v-string interpretation
#    'BEGIN{*CORE::GLOBAL::require=sub {}} require v5.6'
#    c.f. 'BEGIN { *f = sub {0} }; f 2'
# op/pat 774 - losing Unicode-ness of Latin1-only strings
#    'use charnames ":short"; $x="\N{latin:a with acute}"'
# op/recurse 12 - missing parens on recursive call makes it look like method
#    'sub f { f($x) }'
# op/subst 90 - inconsistent handling of utf8 under "use utf8"
# op/taint 29 - "use re 'taint'" deparsed in the wrong place wrt. block open
# op/tiehandle compile - "use strict" deparsed in the wrong place
# uni/tr_ several
# ext/B/t/xref 11 - line numbers when we add newlines to one-line subs
# ext/Data/Dumper/t/dumper compile
# ext/DB_file/several
# ext/Encode/several
# ext/Ernno/Errno warnings
# ext/IO/lib/IO/t/io_sel 23
# ext/PerlIO/t/encoding compile
# ext/POSIX/t/posix 6
# ext/Socket/Socket 8
# ext/Storable/t/croak compile
# lib/Attribute/Handlers/t/multi compile
# lib/bignum/ several
# lib/charnames 35
# lib/constant 32
# lib/English 40
# lib/ExtUtils/t/bytes 4
# lib/File/DosGlob compile
# lib/Filter/Simple/t/data 1
# lib/Math/BigInt/t/constant 1
# lib/Net/t/config Deparse-warning
# lib/overload compile
# lib/Switch/ several
# lib/Symbol 4
# lib/Test/Simple several
# lib/Term/Complete
# lib/Tie/File/t/29_downcopy 5
# lib/vars 22

# Object fields:
#
# in_coderef2text:
# True when deparsing via $deparse->coderef2text; false when deparsing the
# main program.
#
# avoid_local:
# (local($a), local($b)) and local($a, $b) have the same internal
# representation but the short form looks better. We notice we can
# use a large-scale local when checking the list, but need to prevent
# individual locals too. This hash holds the addresses of OPs that
# have already had their local-ness accounted for. The same thing
# is done with my().
#
# curcv:
# CV for current sub (or main program) being deparsed
#
# curcvlex:
# Cached hash of lexical variables for curcv: keys are
# names prefixed with "m" or "o" (representing my/our), and
# each value is an array with two elements indicating the cop_seq
# of scopes in which a var of that name is valid and a third ele-
# ment referencing the pad name.
#
# curcop:
# COP for statement being deparsed
#
# curstash:
# name of the current package for deparsed code
#
# subs_todo:
# array of [cop_seq, CV, is_format?, name] for subs and formats we still
# want to deparse.  The fourth element is a pad name thingy for lexical
# subs or a string for special blocks.  For other subs, it is undef.  For
# lexical subs, CV may be undef, indicating a stub declaration.
#
# protos_todo:
# as above, but [name, prototype] for subs that never got a GV
#
# subs_done, forms_done:
# keys are addresses of GVs for subs and formats we've already
# deparsed (or at least put into subs_todo)
#
# subs_declared
# keys are names of subs for which we've printed declarations.
# That means we can omit parentheses from the arguments. It also means we
# need to put CORE:: on core functions of the same name.
#
# in_subst_repl
# True when deparsing the replacement part of a substitution.
#
# in_refgen
# True when deparsing the argument to \.
#
# parens: -p
# linenums: -l
# unquote: -q
# cuddle: ' ' or '\n', depending on -sC
# indent_size: -si
# use_tabs: -sT
# ex_const: -sv

# A little explanation of how precedence contexts and associativity
# work:
#
# deparse() calls each per-op subroutine with an argument $cx (short
# for context, but not the same as the cx* in the perl core), which is
# a number describing the op's parents in terms of precedence, whether
# they're inside an expression or at statement level, etc.  (see
# chart below). When ops with children call deparse on them, they pass
# along their precedence. Fractional values are used to implement
# associativity ('($x + $y) + $z' => '$x + $y + $y') and related
# parentheses hacks. The major disadvantage of this scheme is that
# it doesn't know about right sides and left sides, so say if you
# assign a listop to a variable, it can't tell it's allowed to leave
# the parens off the listop.

# Precedences:
# 26             [TODO] inside interpolation context ("")
# 25 left        terms and list operators (leftward)
# 24 left        ->
# 23 nonassoc    ++ --
# 22 right       **
# 21 right       ! ~ \ and unary + and -
# 20 left        =~ !~
# 19 left        * / % x
# 18 left        + - .
# 17 left        << >>
# 16 nonassoc    named unary operators
# 15 nonassoc    < > <= >= lt gt le ge
# 14 nonassoc    == != <=> eq ne cmp
# 13 left        &
# 12 left        | ^
# 11 left        &&
# 10 left        ||
#  9 nonassoc    ..  ...
#  8 right       ?:
#  7 right       = += -= *= etc.
#  6 left        , =>
#  5 nonassoc    list operators (rightward)
#  4 right       not
#  3 left        and
#  2 left        or xor
#  1             statement modifiers
#  0.5           statements, but still print scopes as do { ... }
#  0             statement level
# -1             format body

# Nonprinting characters with special meaning:
# \cS - steal parens (see maybe_parens_unop)
# \n - newline and indent
# \t - increase indent
# \b - decrease indent ('outdent')
# \f - flush left (no indent)
# \cK - kill following semicolon, if any

# Semicolon handling:
#  - Individual statements are not deparsed with trailing semicolons.
#    (If necessary, \cK is tacked on to the end.)
#  - Whatever code joins statements together or emits them (lineseq,
#    scopeop, deparse_root) is responsible for adding semicolons where
#    necessary.
#  - use statements are deparsed with trailing semicolons because they are
#    immediately concatenated with the following statement.
#  - indent() removes semicolons wherever it sees \cK.


BEGIN { for (qw[ const stringify rv2sv list glob pushmark null aelem
		 kvaslice kvhslice padsv argcheck
                 nextstate dbstate rv2av rv2hv helem pushdefer leavetrycatch
                 custom ]) {
    eval "sub OP_\U$_ () { " . opnumber($_) . "}"
}}

# _pessimise_walk(): recursively walk the optree of a sub,
# possibly undoing optimisations along the way.

sub DEBUG { 0 }
use if DEBUG, 'Data::Dumper';

sub _pessimise_walk {
    my ($self, $startop) = @_;

    return unless $$startop;
    my ($op, $prevop);
    for ($op = $startop; $$op; $prevop = $op, $op = $op->sibling) {
	my $ppname = $op->name;

	# pessimisations start here

	if ($ppname eq "padrange") {
	    # remove PADRANGE:
	    # the original optimisation either (1) changed this:
	    #    pushmark -> (various pad and list and null ops) -> the_rest
	    # or (2), for the = @_ case, changed this:
	    #    pushmark -> gv[_] -> rv2av -> (pad stuff)       -> the_rest
	    # into this:
	    #    padrange ----------------------------------------> the_rest
	    # so we just need to convert the padrange back into a
	    # pushmark, and in case (1), set its op_next to op_sibling,
	    # which is the head of the original chain of optimised-away
	    # pad ops, or for (2), set it to sibling->first, which is
	    # the original gv[_].

	    $B::overlay->{$$op} = {
		    type => OP_PUSHMARK,
		    name => 'pushmark',
		    private => ($op->private & OPpLVAL_INTRO),
	    };
	}

	# pessimisations end here

	if (class($op) eq 'PMOP') {
	    if (ref($op->pmreplroot)
                && ${$op->pmreplroot}
                && $op->pmreplroot->isa( 'B::OP' ))
            {
                $self-> _pessimise_walk($op->pmreplroot);
            }

            # pessimise any /(?{...})/ code blocks
            my ($re, $cv);
            my $code_list = $op->code_list;
            if ($$code_list) {
                $self->_pessimise_walk($code_list);
            }
            elsif (${$re = $op->pmregexp} && ${$cv = $re->qr_anoncv}) {
                $code_list = $cv->ROOT      # leavesub
                               ->first      #   qr
                               ->code_list; #     list
                $self->_pessimise_walk($code_list);
            }
        }

	if ($op->flags & OPf_KIDS) {
	    $self-> _pessimise_walk($op->first);
	}

    }
}


# _pessimise_walk_exe(): recursively walk the op_next chain of a sub,
# possibly undoing optimisations along the way.

sub _pessimise_walk_exe {
    my ($self, $startop, $visited) = @_;

    no warnings 'recursion';

    return unless $$startop;
    return if $visited->{$$startop};
    my ($op, $prevop);
    for ($op = $startop; $$op; $prevop = $op, $op = $op->next) {
	last if $visited->{$$op};
	$visited->{$$op} = 1;
	my $ppname = $op->name;
	if ($ppname =~
	    /^((and|d?or)(assign)?|(map|grep)while|range|cond_expr|once)$/
	    # entertry is also a logop, but its op_other invariably points
	    # into the same chain as the main execution path, so we skip it
	) {
	    $self->_pessimise_walk_exe($op->other, $visited);
	}
	elsif ($ppname eq "subst") {
	    $self->_pessimise_walk_exe($op->pmreplstart, $visited);
	}
	elsif ($ppname =~ /^(enter(loop|iter))$/) {
	    # redoop and nextop will already be covered by the main block
	    # of the loop
	    $self->_pessimise_walk_exe($op->lastop, $visited);
	}

	# pessimisations start here
    }
}

# Go through an optree and "remove" some optimisations by using an
# overlay to selectively modify or un-null some ops. Deparsing in the
# absence of those optimisations is then easier.
#
# Note that older optimisations are not removed, as Deparse was already
# written to recognise them before the pessimise/overlay system was added.

sub pessimise {
    my ($self, $root, $start) = @_;

    no warnings 'recursion';
    # walk tree in root-to-branch order
    $self->_pessimise_walk($root);

    my %visited;
    # walk tree in execution order
    $self->_pessimise_walk_exe($start, \%visited);
}


sub null {
    my $op = shift;
    return class($op) eq "NULL";
}


# Add a CV to the list of subs that still need deparsing.

sub todo {
    my $self = shift;
    my($cv, $is_form, $name) = @_;
    my $cvfile = $cv->FILE//'';
    return unless ($cvfile eq $0 || exists $self->{files}{$cvfile});
    my $seq;
    if ($cv->OUTSIDE_SEQ) {
	$seq = $cv->OUTSIDE_SEQ;
    } elsif (!null($cv->START) and is_state($cv->START)) {
	$seq = $cv->START->cop_seq;
    } else {
	$seq = 0;
    }
    my $stash = $cv->STASH;
    if (class($stash) eq 'HV') {
        $self->{packs}{$stash->NAME}++;
    }
    push @{$self->{'subs_todo'}}, [$seq, $cv, $is_form, $name];
}


# Pop the next sub from the todo list and deparse it

sub next_todo {
    my $self = shift;
    my $ent = shift @{$self->{'subs_todo'}};
    my ($seq, $cv, $is_form, $name) = @$ent;

    # any 'use strict; package foo' that should come before the sub
    # declaration to sync with the first COP of the sub
    my $pragmata = '';
    if ($cv and !null($cv->START) and is_state($cv->START))  {
        $pragmata = $self->pragmata($cv->START);
    }

    if (ref $name) { # lexical sub
	# emit the sub.
	my @text;
	my $flags = $name->FLAGS;
        my $category =
	    !$cv || $seq <= $name->COP_SEQ_RANGE_LOW
		? $self->keyword($flags & PADNAMEf_OUR
				    ? "our"
				    : $flags & SVpad_STATE
					? "state"
					: "my") . " "
		: "";

        # Skip lexical 'state' subs imported from the builtin::
        # package, since they are created automatically by
        #     use builtin "foo"
        if ($cv && $category =~  /\bstate\b/) {
            my $globname;
            my $gv = $cv->GV;
            if (
                   $gv
                && defined (($globname = $gv->object_2svref))
                && $$globname =~ /^\*builtin::/
            ) {
                return '';
            }
        }

	push @text, $category;

	# XXX We would do $self->keyword("sub"), but ‘my CORE::sub’
	#     doesn’t work and ‘my sub’ ignores a &sub in scope.  I.e.,
	#     we have a core bug here.
	push @text, "sub " . substr $name->PVX, 1;
	if ($cv) {
	    # my sub foo { }
	    push @text,  " " . $self->deparse_sub($cv);
	    $text[-1] =~ s/ ;$/;/;
	}
	else {
	    # my sub foo;
	    push @text, ";\n";
	}
	return $pragmata . join "", @text;
    }

    my $gv = $cv->GV;
    $name //= $self->gv_name($gv);
    if ($is_form) {
	return $pragmata . $self->keyword("format") . " $name =\n"
	    . $self->deparse_format($cv). "\n";
    } else {
	my $use_dec;
	if ($name eq "BEGIN") {
	    $use_dec = $self->begin_is_use($cv);
	    if (defined ($use_dec) and $self->{'expand'} < 5) {
		return $pragmata if 0 == length($use_dec);

                #  XXX bit of a hack: Test::More's use_ok() method
                #  builds a fake use statement which deparses as, e.g.
                #      use Net::Ping (@{$args[0];});
                #  As well as being superfluous (the use_ok() is deparsed
                #  too) and ugly, it fails under use strict and otherwise
                #  makes use of a lexical var that's not in scope.
                #  So strip it out.
                return $pragmata
                        if $use_dec =~
                            m/
                                \A
                                use \s \S+ \s \(\@\{
                                (
                                    \s*\#line\ \d+\ \".*"\s*
                                )?
                                \$args\[0\];\}\);
                                \n
                                \Z
                            /x;

		$use_dec =~ s/^(use|no)\b/$self->keyword($1)/e;
	    }
	}
	my $l = '';
	if ($self->{'linenums'}) {
	    my $line = $gv->LINE;
	    my $file = $gv->FILE;
	    $l = "\n\f#line $line \"$file\"\n";
	}
	my $p = '';
	my $stash;
	if (class($cv->STASH) ne "SPECIAL") {
	    $stash = $cv->STASH->NAME;
	    if ($stash ne $self->{'curstash'}) {
		$p = $self->keyword("package") . " $stash;\n";
		$name = "$self->{'curstash'}::$name" unless $name =~ /::/;
		$self->{'curstash'} = $stash;
	    }
	}
	if ($use_dec) {
	    return "$pragmata$p$l$use_dec";
	}
        if ( $name !~ /::/ and $self->lex_in_scope("&$name")
                            || $self->lex_in_scope("&$name", 1) )
        {
            $name = "$self->{'curstash'}::$name";
        } elsif (defined $stash) {
            $name =~ s/^\Q$stash\E::(?!\z|.*::)//;
        }
	my $ret = "$pragmata${p}${l}" . $self->keyword("sub") . " $name "
	      . $self->deparse_sub($cv);
	$self->{'subs_declared'}{$name} = 1;
	return $ret;
    }
}


# Return a "use" declaration for this BEGIN block, if appropriate
sub begin_is_use {
    my ($self, $cv) = @_;
    my $root = $cv->ROOT;
    local @$self{qw'curcv curcvlex'} = ($cv);
    local $B::overlay = {};
    $self->pessimise($root, $cv->START);
#require B::Debug;
#B::walkoptree($cv->ROOT, "debug");
    my $lineseq = $root->first;
    return if $lineseq->name ne "lineseq";

    my $req_op = $lineseq->first->sibling;
    return if $req_op->name ne "require";

    # maybe it's C<require expr> rather than C<require 'foo'>
    return if ($req_op->first->name ne 'const');

    my $module;
    if ($req_op->first->private & OPpCONST_BARE) {
	# Actually it should always be a bareword
	$module = $self->const_sv($req_op->first)->PV;
	$module =~ s[/][::]g;
	$module =~ s/.pm$//;
    }
    else {
	$module = $self->const($self->const_sv($req_op->first), 6);
    }

    my $version;
    my $version_op = $req_op->sibling;
    return if class($version_op) eq "NULL";
    if ($version_op->name eq "lineseq") {
	# We have a version parameter; skip nextstate & pushmark
	my $constop = $version_op->first->next->next;

	return unless $self->const_sv($constop)->PV eq $module;
	$constop = $constop->sibling;
	$version = $self->const_sv($constop);
	if (class($version) eq "IV") {
	    $version = $version->int_value;
	} elsif (class($version) eq "NV") {
	    $version = $version->NV;
	} elsif (class($version) ne "PVMG") {
	    # Includes PVIV and PVNV
	    $version = $version->PV;
	} else {
	    # version specified as a v-string
	    $version = 'v'.join '.', map ord, split //, $version->PV;
	}
	$constop = $constop->sibling;
	return if $constop->name ne "method_named";
	return if $self->meth_sv($constop)->PV ne "VERSION";
    }

    $lineseq = $version_op->sibling;
    return if $lineseq->name ne "lineseq";
    my $entersub = $lineseq->first->sibling;
    if ($entersub->name eq "stub") {
	return "use $module $version ();\n" if defined $version;
	return "use $module ();\n";
    }
    return if $entersub->name ne "entersub";

    # See if there are import arguments
    my $args = '';

    my $svop = $entersub->first->sibling; # Skip over pushmark
    return unless $self->const_sv($svop)->PV eq $module;

    # Pull out the arguments
    for ($svop=$svop->sibling; index($svop->name, "method_") != 0;
		$svop = $svop->sibling) {
	$args .= ", " if length($args);
	$args .= $self->deparse($svop, 6);
    }

    my $use = 'use';
    my $method_named = $svop;
    return if $method_named->name ne "method_named";
    my $method_name = $self->meth_sv($method_named)->PV;

    if ($method_name eq "unimport") {
	$use = 'no';
    }

    # Certain pragmas are dealt with using hint bits,
    # so we ignore them here
    if ($module eq 'strict' || $module eq 'integer'
	|| $module eq 'bytes' || $module eq 'warnings'
	|| $module eq 'feature') {
	return "";
    }

    if (defined $version && length $args) {
	return "$use $module $version ($args);\n";
    } elsif (defined $version) {
	return "$use $module $version;\n";
    } elsif (length $args) {
	return "$use $module ($args);\n";
    } else {
	return "$use $module;\n";
    }
}

sub stash_subs {
    my ($self, $pack, $seen) = @_;
    my (@ret, $stash);
    if (!defined $pack) {
	$pack = '';
	$stash = \%::;
    }
    else {
	$pack =~ s/(::)?$/::/;
	no strict 'refs';
	$stash = \%{"main::$pack"};
    }
    return
	if ($seen ||= {})->{
	    $INC{"overload.pm"} ? overload::StrVal($stash) : $stash
	   }++;
    my $stashobj = svref_2object($stash);
    my %stash = $stashobj->ARRAY;
    while (my ($key, $val) = each %stash) {
	my $flags = $val->FLAGS;
	if ($flags & SVf_ROK) {
	    # A reference.  Dump this if it is a reference to a CV.  If it
	    # is a constant acting as a proxy for a full subroutine, then
	    # we may or may not have to dump it.  If some form of perl-
	    # space visible code must have created it, be it a use
	    # statement, or some direct symbol-table manipulation code that
	    # we will deparse, then we don’t want to dump it.  If it is the
	    # result of a declaration like sub f () { 42 } then we *do*
	    # want to dump it.  The only way to distinguish these seems
	    # to be the SVs_PADTMP flag on the constant, which is admit-
	    # tedly a hack.
	    my $class = class(my $referent = $val->RV);
	    if ($class eq "CV") {
		$self->todo($referent, 0);
	    } elsif (
		$class !~ /^(AV|HV|CV|FM|IO|SPECIAL)\z/
		# A more robust way to write that would be this, but B does
		# not provide the SVt_ constants:
		# ($referent->FLAGS & B::SVTYPEMASK) < B::SVt_PVAV
		and $referent->FLAGS & SVs_PADTMP
	    ) {
		push @{$self->{'protos_todo'}}, [$pack . $key, $val];
	    }
	} elsif ($flags & (SVf_POK|SVf_IOK)) {
	    # Just a prototype. As an ugly but fairly effective way
	    # to find out if it belongs here is to see if the AUTOLOAD
	    # (if any) for the stash was defined in one of our files.
	    my $A = $stash{"AUTOLOAD"};
	    if (defined ($A) && class($A) eq "GV" && defined($A->CV)
		&& class($A->CV) eq "CV") {
		my $AF = $A->FILE;
		next unless $AF eq $0 || exists $self->{'files'}{$AF};
	    }
	    push @{$self->{'protos_todo'}},
		 [$pack . $key, $flags & SVf_POK ? $val->PV: undef];
	} elsif (class($val) eq "GV") {
	    if (class(my $cv = $val->CV) ne "SPECIAL") {
		next if $self->{'subs_done'}{$$val}++;

                # Ignore imposters (aliases etc)
                my $name = $cv->NAME_HEK;
                if(defined $name) {
                    # avoid using $cv->GV here because if the $val GV is
                    # an alias, CvGV() could upgrade the real stash entry
                    # from an RV to a GV
                    next unless $name eq $key;
                    next unless $$stashobj == ${$cv->STASH};
                }
                else {
                   next if $$val != ${$cv->GV};
                }

		$self->todo($cv, 0);
	    }
	    if (class(my $cv = $val->FORM) ne "SPECIAL") {
		next if $self->{'forms_done'}{$$val}++;
		next if $$val != ${$cv->GV};   # Ignore imposters
		$self->todo($cv, 1);
	    }
	    if (class($val->HV) ne "SPECIAL" && $key =~ /::$/) {
		$self->stash_subs($pack . $key, $seen);
	    }
	}
    }
}

sub print_protos {
    my $self = shift;
    my $ar;
    my @ret;
    foreach $ar (@{$self->{'protos_todo'}}) {
	if (ref $ar->[1]) {
	    # Only print a constant if it occurs in the same package as a
	    # dumped sub.  This is not perfect, but a heuristic that will
	    # hopefully work most of the time.  Ideally we would use
	    # CvFILE, but a constant stub has no CvFILE.
	    my $pack = ($ar->[0] =~ /(.*)::/)[0];
	    next if $pack and !$self->{packs}{$pack}
	}
	my $body = defined $ar->[1]
		? ref $ar->[1]
		    ? " () {\n    " . $self->const($ar->[1]->RV,0) . ";\n}"
		    : " (". $ar->[1] . ");"
		: ";";
	push @ret, "sub " . $ar->[0] .  "$body\n";
    }
    delete $self->{'protos_todo'};
    return @ret;
}

sub style_opts {
    my $self = shift;
    my $opts = shift;
    my $opt;
    while (length($opt = substr($opts, 0, 1))) {
	if ($opt eq "C") {
	    $self->{'cuddle'} = " ";
	    $opts = substr($opts, 1);
	} elsif ($opt eq "i") {
	    $opts =~ s/^i(\d+)//;
	    $self->{'indent_size'} = $1;
	} elsif ($opt eq "T") {
	    $self->{'use_tabs'} = 1;
	    $opts = substr($opts, 1);
	} elsif ($opt eq "v") {
	    $opts =~ s/^v([^.]*)(.|$)//;
	    $self->{'ex_const'} = $1;
	}
    }
}

sub new {
    my $class = shift;
    my $self = bless {}, $class;
    $self->{'cuddle'} = "\n";
    $self->{'curcop'} = undef;
    $self->{'curstash'} = "main";
    $self->{'ex_const'} = "'???'";
    $self->{'expand'} = 0;
    $self->{'files'} = {};
    $self->{'packs'} = {};
    $self->{'indent_size'} = 4;
    $self->{'linenums'} = 0;
    $self->{'parens'} = 0;
    $self->{'subs_todo'} = [];
    $self->{'unquote'} = 0;
    $self->{'use_dumper'} = 0;
    $self->{'use_tabs'} = 0;

    $self->{'ambient_warnings'} = undef; # Assume no lexical warnings
    $self->{'ambient_hints'} = 0;
    $self->{'ambient_hinthash'} = undef;
    $self->init();

    while (my $arg = shift @_) {
	if ($arg eq "-d") {
	    $self->{'use_dumper'} = 1;
	    require Data::Dumper;
	} elsif ($arg =~ /^-f(.*)/) {
	    $self->{'files'}{$1} = 1;
	} elsif ($arg eq "-l") {
	    $self->{'linenums'} = 1;
	} elsif ($arg eq "-p") {
	    $self->{'parens'} = 1;
	} elsif ($arg eq "-P") {
	    $self->{'noproto'} = 1;
	} elsif ($arg eq "-q") {
	    $self->{'unquote'} = 1;
	} elsif (substr($arg, 0, 2) eq "-s") {
	    $self->style_opts(substr $arg, 2);
	} elsif ($arg =~ /^-x(\d)$/) {
	    $self->{'expand'} = $1;
	}
    }
    return $self;
}


# Initialise the contextual information, either from
# defaults provided with the ambient_pragmas method,
# or from perl's own defaults otherwise.
sub init {
    my $self = shift;

    $self->{'warnings'} = $self->{'ambient_warnings'};
    $self->{'hints'}    = $self->{'ambient_hints'};
    $self->{'hinthash'} = $self->{'ambient_hinthash'};

    # also a convenient place to clear out subs_declared
    delete $self->{'subs_declared'};
}

sub compile {
    my(@args) = @_;
    return sub {
	my $self = B::Deparse->new(@args);
	# First deparse command-line args
	if (defined $^I) { # deparse -i
	    print q(BEGIN { $^I = ).perlstring($^I).qq(; }\n);
	}
	if ($^W) { # deparse -w
	    print qq(BEGIN { \$^W = $^W; }\n);
	}
	if ($/ ne "\n" or defined $O::savebackslash) { # deparse -l and -0
	    my $fs = perlstring($/) || 'undef';
	    my $bs = perlstring($O::savebackslash) || 'undef';
	    print qq(BEGIN { \$/ = $fs; \$\\ = $bs; }\n);
	}
	my @BEGINs  = B::begin_av->isa("B::AV") ? B::begin_av->ARRAY : ();
	my @UNITCHECKs = B::unitcheck_av->isa("B::AV")
	    ? B::unitcheck_av->ARRAY
	    : ();
	my @CHECKs  = B::check_av->isa("B::AV") ? B::check_av->ARRAY : ();
	my @INITs   = B::init_av->isa("B::AV") ? B::init_av->ARRAY : ();
	my @ENDs    = B::end_av->isa("B::AV") ? B::end_av->ARRAY : ();
	my @names = qw(BEGIN UNITCHECK CHECK INIT END);
	my @blocks = \(@BEGINs, @UNITCHECKs, @CHECKs, @INITs, @ENDs);
	while (@names) {
	    my ($name, $blocks) = (shift @names, shift @blocks);
	    for my $block (@$blocks) {
		$self->todo($block, 0, $name);
	    }
	}
	$self->stash_subs();
	local($SIG{"__DIE__"}) =
	  sub {
	      if ($self->{'curcop'}) {
		  my $cop = $self->{'curcop'};
		  my($line, $file) = ($cop->line, $cop->file);
		  print STDERR "While deparsing $file near line $line,\n";
	      }
	    };
	$self->{'curcv'} = main_cv;
	$self->{'curcvlex'} = undef;
	print $self->print_protos;
	@{$self->{'subs_todo'}} =
	  sort {$a->[0] <=> $b->[0]} @{$self->{'subs_todo'}};
	my $root = main_root;
	local $B::overlay = {};
	unless (null $root) {
	    $self->pad_subs($self->{'curcv'});
	    # Check for a stub-followed-by-ex-cop, resulting from a program
	    # consisting solely of sub declarations.  For backward-compati-
	    # bility (and sane output) we don’t want to emit the stub.
	    #   leave
	    #     enter
	    #     stub
	    #     ex-nextstate (or ex-dbstate)
	    my $kid;
	    if ( $root->name eq 'leave'
	     and ($kid = $root->first)->name eq 'enter'
	     and !null($kid = $kid->sibling) and $kid->name eq 'stub'
	     and !null($kid = $kid->sibling) and $kid->name eq 'null'
	     and class($kid) eq 'COP' and null $kid->sibling )
	    {
		# ignore
	    } else {
		$self->pessimise($root, main_start);
		print $self->indent($self->deparse_root($root)), "\n";
	    }
	}
	my @text;
	while (scalar(@{$self->{'subs_todo'}})) {
	    push @text, $self->next_todo;
	}
	print $self->indent(join("", @text)), "\n" if @text;

	# Print __DATA__ section, if necessary
	no strict 'refs';
	my $laststash = defined $self->{'curcop'}
	    ? $self->{'curcop'}->stash->NAME : $self->{'curstash'};
	if (defined *{$laststash."::DATA"}{IO}) {
	    print $self->keyword("package") . " $laststash;\n"
		unless $laststash eq $self->{'curstash'};
	    print $self->keyword("__DATA__") . "\n";
	    print readline(*{$laststash."::DATA"});
	}
    }
}

sub coderef2text {
    my $self = shift;
    my $sub = shift;
    croak "Usage: ->coderef2text(CODEREF)" unless UNIVERSAL::isa($sub, "CODE");

    $self->init();
    local $self->{in_coderef2text} = 1;
    return $self->indent($self->deparse_sub(svref_2object($sub)));
}

my %strict_bits = do {
    local $^H;
    map +($_ => strict::bits($_)), qw/refs subs vars/
};

sub ambient_pragmas {
    my $self = shift;
    my ($hint_bits, $warning_bits, $hinthash) = (0);

    while (@_ > 1) {
	my $name = shift();
	my $val  = shift();

	if ($name eq 'strict') {
	    require strict;

	    if ($val eq 'none') {
		$hint_bits &= $strict_bits{$_} for qw/refs subs vars/;
		next();
	    }

	    my @names;
	    if ($val eq "all") {
		@names = qw/refs subs vars/;
	    }
	    elsif (ref $val) {
		@names = @$val;
	    }
	    else {
		@names = split' ', $val;
	    }
	    $hint_bits |= $strict_bits{$_} for @names;
	}

	elsif ($name eq 'integer'
	    || $name eq 'bytes'
	    || $name eq 'utf8') {
	    require "$name.pm";
	    if ($val) {
		$hint_bits |= ${$::{"${name}::"}{"hint_bits"}};
	    }
	    else {
		$hint_bits &= ~${$::{"${name}::"}{"hint_bits"}};
	    }
	}

	elsif ($name eq 're') {
	    require re;
	    if ($val eq 'none') {
		$hint_bits &= ~re::bits(qw/taint eval/);
		next();
	    }

	    my @names;
	    if ($val eq 'all') {
		@names = qw/taint eval/;
	    }
	    elsif (ref $val) {
		@names = @$val;
	    }
	    else {
		@names = split' ',$val;
	    }
	    $hint_bits |= re::bits(@names);
	}

	elsif ($name eq 'warnings') {
	    if ($val eq 'none') {
		$warning_bits = $warnings::NONE;
		next();
	    }

	    my @names;
	    if (ref $val) {
		@names = @$val;
	    }
	    else {
		@names = split/\s+/, $val;
	    }

	    $warning_bits = $warnings::NONE if !defined ($warning_bits);
	    $warning_bits |= warnings::bits(@names);
	}

	elsif ($name eq 'warning_bits') {
	    $warning_bits = $val;
	}

	elsif ($name eq 'hint_bits') {
	    $hint_bits = $val;
	}

	elsif ($name eq '%^H') {
	    $hinthash = $val;
	}

	else {
	    croak "Unknown pragma type: $name";
	}
    }
    if (@_) {
	croak "The ambient_pragmas method expects an even number of args";
    }

    $self->{'ambient_warnings'} = $warning_bits;
    $self->{'ambient_hints'} = $hint_bits;
    $self->{'ambient_hinthash'} = $hinthash;
}

# This method is the inner loop, so try to keep it simple
sub deparse {
    my $self = shift;
    my($op, $cx) = @_;

    Carp::confess("Null op in deparse") if !defined($op)
					|| class($op) eq "NULL";
    my $meth = "pp_" . $op->name;
    return $self->$meth($op, $cx);
}

sub indent {
    my $self = shift;
    my $txt = shift;
    # \cK also swallows a preceding line break when followed by a
    # semicolon.
    $txt =~ s/\n\cK;//g;
    my @lines = split(/\n/, $txt);
    my $leader = "";
    my $level = 0;
    my $line;
    for $line (@lines) {
	my $cmd = substr($line, 0, 1);
	if ($cmd eq "\t" or $cmd eq "\b") {
	    $level += ($cmd eq "\t" ? 1 : -1) * $self->{'indent_size'};
	    if ($self->{'use_tabs'}) {
		$leader = "\t" x ($level / 8) . " " x ($level % 8);
	    } else {
		$leader = " " x $level;
	    }
	    $line = substr($line, 1);
	}
	if (index($line, "\f") > 0) {
		$line =~ s/\f/\n/;
	}
	if (substr($line, 0, 1) eq "\f") {
	    $line = substr($line, 1); # no indent
	} else {
	    $line = $leader . $line;
	}
	$line =~ s/\cK;?//g;
    }
    return join("\n", @lines);
}

sub pad_subs {
    my ($self, $cv) = @_;
    my $padlist = $cv->PADLIST;
    my @names = $padlist->ARRAYelt(0)->ARRAY;
    my @values = $padlist->ARRAYelt(1)->ARRAY;
    my @todo;
  PADENTRY:
    for my $ix (0.. $#names) { for $_ ($names[$ix]) {
	next if class($_) eq "SPECIAL";
	my $name = $_->PVX;
	if (defined $name && $name =~ /^&./) {
	    my $low = $_->COP_SEQ_RANGE_LOW;
	    my $flags = $_->FLAGS;
	    my $outer = $flags & PADNAMEf_OUTER;
	    if ($flags & PADNAMEf_OUR) {
		push @todo, [$low, undef, 0, $_]
		          # [seq, no cv, not format, padname]
		    unless $outer;
		next;
	    }
	    my $protocv = $flags & SVpad_STATE
		? $values[$ix]
		: $_->PROTOCV;
	    if (class ($protocv) ne 'CV') {
		my $flags = $flags;
		my $cv = $cv;
		my $name = $_;
		while ($flags & PADNAMEf_OUTER && class ($protocv) ne 'CV')
		{
		    $cv = $cv->OUTSIDE;
		    next PADENTRY if class($cv) eq 'SPECIAL'; # XXX freed?
		    my $padlist = $cv->PADLIST;
		    my $ix = $name->PARENT_PAD_INDEX;
		    $name = $padlist->NAMES->ARRAYelt($ix);
		    $flags = $name->FLAGS;
		    $protocv = $flags & SVpad_STATE
			? $padlist->ARRAYelt(1)->ARRAYelt($ix)
			: $name->PROTOCV;
		}
	    }
	    my $defined_in_this_sub = ${$protocv->OUTSIDE} == $$cv || do {
		my $other = $protocv->PADLIST;
		$$other && $other->outid == $padlist->id;
	    };
	    if ($flags & PADNAMEf_OUTER) {
		next unless $defined_in_this_sub;
		push @todo, [$protocv->OUTSIDE_SEQ, $protocv, 0, $_];
		next;
	    }
	    my $outseq = $protocv->OUTSIDE_SEQ;
	    if ($outseq <= $low) {
		# defined before its name is visible, so it’s gotta be
		# declared and defined at once: my sub foo { ... }
		push @todo, [$low, $protocv, 0, $_];
	    }
	    else {
		# declared and defined separately: my sub f; sub f { ... }
		push @todo, [$low, undef, 0, $_];
		push @todo, [$outseq, $protocv, 0, $_]
		    if $defined_in_this_sub;
	    }
	}
    }}
    @{$self->{'subs_todo'}} =
	sort {$a->[0] <=> $b->[0]} @{$self->{'subs_todo'}}, @todo
}


# deparse_argops(): deparse, if possible, a sequence of argcheck + argelem
# ops into a subroutine signature. If successful, return the first op
# following the signature ops plus the signature string; else return the
# empty list.
#
# Normally a bunch of argelem ops will have been generated by the
# signature parsing, but it's possible that ops have been added manually
# or altered. In this case we return "()" and fall back to general
# deparsing of the individual sigelems as 'my $x = $_[N]' etc.
#
# We're only called if the top is an ex-argcheck, which is a placeholder
# indicating a signature subtree.
#
# Return a signature string, or an empty list if no deparseable as a
# signature

sub deparse_argops {
    my ($self, $topop, $cv) = @_;

    my @sig;


    $topop = $topop->first;
    return unless $$topop and $topop->name eq 'lineseq';


    # last op should be nextstate
    my $last = $topop->last;
    return unless $$last
                    and (   _op_is_or_was($last, OP_NEXTSTATE)
                         or _op_is_or_was($last, OP_DBSTATE));

    # first OP_NEXTSTATE

    my $o = $topop->first;
    return unless $$o;
    return if $o->label;

    # OP_ARGCHECK

    $o = $o->sibling;
    return unless $$o and $o->name eq 'argcheck';

    my ($params, $opt_params, $slurpy) = $o->aux_list($cv);
    my $mandatory = $params - $opt_params;
    my $seen_slurpy = 0;
    my $last_ix = -1;

    # keep looking for valid nextstate + argelem pairs, terminated
    # by a final nextstate

    while (1) {
        $o = $o->sibling;
        return unless $$o;

        # skip trailing nextstate
        last if $$o == $$last;

        # OP_NEXTSTATE
        return unless $o->name =~ /^(next|db)state$/;
        return if $o->label;

        # OP_ARGELEM
        $o = $o->sibling;
        last unless $$o;

        if ($o->name eq 'argelem') {
            my $ix  = $o->string($cv);
            while (++$last_ix < $ix) {
                push @sig, $last_ix <  $mandatory ? '$' : '$=';
            }
            my $var = $self->padname($o->targ);
            if ($var =~ /^[@%]/) {
                return if $seen_slurpy;
                $seen_slurpy = 1;
                return if $ix != $params or !$slurpy
                            or substr($var,0,1) ne $slurpy;
            }
            else {
                return if $ix >= $params;
            }
            if ($o->flags & OPf_KIDS) {
                my $kid = $o->first;
                return unless $$kid and $kid->name eq 'argdefelem';
                my $def = $self->deparse($kid->first, 7);
                $def = "($def)" if $kid->first->flags & OPf_PARENS;
                my $assign = "=";
                $assign = "//=" if $kid->private & OPpARG_IF_UNDEF;
                $assign = "||=" if $kid->private & OPpARG_IF_FALSE;
                $var .= " $assign $def";
            }
            push @sig, $var;
        }
        elsif ($o->name eq 'null'
               and ($o->flags & OPf_KIDS)
               and $o->first->name eq 'argdefelem')
        {
            # special case - a void context default expression: $ = expr

            my $defop = $o->first;
            my $ix = $defop->targ;
            while (++$last_ix < $ix) {
                push @sig, $last_ix <  $mandatory ? '$' : '$=';
            }
            return if $last_ix >= $params
                    or $last_ix < $mandatory;
            my $def = $self->deparse($defop->first, 7);
            $def = "($def)" if $defop->first->flags & OPf_PARENS;
            push @sig, '$ = ' . $def;
        }
        else {
            return;
        }

    }

    while (++$last_ix < $params) {
        push @sig, $last_ix <  $mandatory ? '$' : '$=';
    }
    push @sig, $slurpy if $slurpy and !$seen_slurpy;

    return (join(', ', @sig));
}


# Deparse a sub. Returns everything except the 'sub foo',
# e.g.  ($$) : method { ...; }
# or    : prototype($$) lvalue ($a, $b) { ...; };

sub deparse_sub {
    my $self = shift;
    my $cv = shift;
    my @attrs;
    my $proto;
    my $sig;

Carp::confess("NULL in deparse_sub") if !defined($cv) || $cv->isa("B::NULL");
Carp::confess("SPECIAL in deparse_sub") if $cv->isa("B::SPECIAL");
    local $self->{'curcop'} = $self->{'curcop'};

    my $has_sig = $self->feature_enabled('signatures');
    if ($cv->FLAGS & SVf_POK) {
	my $myproto = $cv->PV;
	if ($has_sig) {
            push @attrs, "prototype($myproto)";
        }
        else {
            $proto = $myproto;
        }
    }
    if ($cv->CvFLAGS & (CVf_NOWARN_AMBIGUOUS|CVf_LOCKED|CVf_LVALUE|CVf_ANONCONST)) {
        push @attrs, "lvalue" if $cv->CvFLAGS & CVf_LVALUE;
        push @attrs, "method" if $cv->CvFLAGS & CVf_NOWARN_AMBIGUOUS;
        push @attrs, "const"  if $cv->CvFLAGS & CVf_ANONCONST;
    }

    local($self->{'curcv'}) = $cv;
    local($self->{'curcvlex'});
    local(@$self{qw'curstash warnings hints hinthash'})
		= @$self{qw'curstash warnings hints hinthash'};
    my $body;
    my $root = $cv->ROOT;
    local $B::overlay = {};
    if (not null $root) {
	$self->pad_subs($cv);
	$self->pessimise($root, $cv->START);
	my $lineseq = $root->first;

        # stub sub may have single op rather than list of ops
        my $is_list = ($lineseq->name eq "lineseq");
        my $firstop = $is_list ? $lineseq->first : $lineseq;

        # Try to deparse first subtree as a signature if possible.
        # Top of signature subtree has an ex-argcheck as a placeholder
        if (    $has_sig
            and $$firstop
            and $firstop->name eq 'null'
            and $firstop->targ == OP_ARGCHECK
        ) {
            my ($mysig) = $self->deparse_argops($firstop, $cv);
            if (defined $mysig) {
                $sig = $mysig;
                $firstop = $is_list ? $firstop->sibling : undef;
            }
        }

        if ($is_list && $firstop) {
            my @ops;
	    for (my $o = $firstop; $$o; $o=$o->sibling) {
		push @ops, $o;
	    }
	    $body = $self->lineseq(undef, 0, @ops).";";
            if (!$has_sig and $ops[-1]->name =~ /^(next|db)state$/) {
                # this handles void context in
                #   use feature signatures; sub ($=1) {}
                $body .= "\n()";
            }
	    my $scope_en = $self->find_scope_en($lineseq);
	    if (defined $scope_en) {
		my $subs = join"", $self->seq_subs($scope_en);
		$body .= ";\n$subs" if length($subs);
	    }
	}
	elsif ($firstop) {
	    $body = $self->deparse($root->first, 0);
	}
        else {
            $body = ';'; # stub sub
        }

        my $l = '';
        if ($self->{'linenums'}) {
            # a glob's gp_line is set from the line containing a
            # sub's closing '}' if the CV is the first use of the GV.
            # So make sure the linenum is set correctly for '}'
            my $gv = $cv->GV;
            my $line = $gv->LINE;
            my $file = $gv->FILE;
            $l = "\f#line $line \"$file\"\n";
        }
        $body = "{\n\t$body\n$l\b}";
    }
    else {
	my $sv = $cv->const_sv;
	if ($$sv) {
	    # uh-oh. inlinable sub... format it differently
	    $body = "{ " . $self->const($sv, 0) . " }\n";
	} else { # XSUB? (or just a declaration)
	    $body = ';'
	}
    }
    $proto = defined $proto ? "($proto) " : "";
    $sig   = defined $sig   ? "($sig) "   : "";
    my $attrs = '';
    $attrs = ': ' . join('', map "$_ ", @attrs) if @attrs;
    return "$proto$attrs$sig$body\n";
}

sub deparse_format {
    my $self = shift;
    my $form = shift;
    my @text;
    local($self->{'curcv'}) = $form;
    local($self->{'curcvlex'});
    local($self->{'in_format'}) = 1;
    local(@$self{qw'curstash warnings hints hinthash'})
		= @$self{qw'curstash warnings hints hinthash'};
    my $op = $form->ROOT;
    local $B::overlay = {};
    $self->pessimise($op, $form->START);
    my $kid;
    return "\f." if $op->first->name eq 'stub'
                || $op->first->name eq 'nextstate';
    $op = $op->first->first; # skip leavewrite, lineseq
    while (not null $op) {
	$op = $op->sibling; # skip nextstate
	my @exprs;
	$kid = $op->first->sibling; # skip pushmark
	push @text, "\f".$self->const_sv($kid)->PV;
	$kid = $kid->sibling;
	for (; not null $kid; $kid = $kid->sibling) {
	    push @exprs, $self->deparse($kid, -1);
	    $exprs[-1] =~ s/;\z//;
	}
	push @text, "\f".join(", ", @exprs)."\n" if @exprs;
	$op = $op->sibling;
    }
    return join("", @text) . "\f.";
}

sub is_scope {
    my $op = shift;
    return $op->name eq "leave" || $op->name eq "scope"
      || $op->name eq "lineseq"
	|| ($op->name eq "null" && class($op) eq "UNOP"
	    && (is_scope($op->first) || $op->first->name eq "enter"));
}

sub is_state {
    my $name = $_[0]->name;
    return $name eq "nextstate" || $name eq "dbstate" || $name eq "setstate";
}

sub is_miniwhile { # check for one-line loop ('foo() while $y--')
    my $op = shift;
    return (!null($op) and null($op->sibling)
	    and $op->name eq "null" and class($op) eq "UNOP"
	    and (($op->first->name =~ /^(and|or)$/
		  and $op->first->first->sibling->name eq "lineseq")
		 or ($op->first->name eq "lineseq"
		     and not null $op->first->first->sibling
		     and $op->first->first->sibling->name eq "unstack")
		 ));
}

# Check if the op and its sibling are the initialization and the rest of a
# for (..;..;..) { ... } loop
sub is_for_loop {
    my $op = shift;
    # This OP might be almost anything, though it won't be a
    # nextstate. (It's the initialization, so in the canonical case it
    # will be an sassign.) The sibling is (old style) a lineseq whose
    # first child is a nextstate and whose second is a leaveloop, or
    # (new style) an unstack whose sibling is a leaveloop.
    my $lseq = $op->sibling;
    return 0 unless !is_state($op) and !null($lseq);
    if ($lseq->name eq "lineseq") {
	if ($lseq->first && !null($lseq->first) && is_state($lseq->first)
	    && (my $sib = $lseq->first->sibling)) {
	    return (!null($sib) && $sib->name eq "leaveloop");
	}
    } elsif ($lseq->name eq "unstack" && ($lseq->flags & OPf_SPECIAL)) {
	my $sib = $lseq->sibling;
	return $sib && !null($sib) && $sib->name eq "leaveloop";
    }
    return 0;
}

sub is_scalar {
    my $op = shift;
    return ($op->name eq "rv2sv" or
	    $op->name eq "padsv" or
	    $op->name eq "gv" or # only in array/hash constructs
	    $op->flags & OPf_KIDS && !null($op->first)
	      && $op->first->name eq "gvsv");
}

sub maybe_parens {
    my $self = shift;
    my($text, $cx, $prec) = @_;
    if ($prec < $cx              # unary ops nest just fine
	or $prec == $cx and $cx != 4 and $cx != 16 and $cx != 21
	or $self->{'parens'})
    {
	$text = "($text)";
	# In a unop, let parent reuse our parens; see maybe_parens_unop
	$text = "\cS" . $text if $cx == 16;
	return $text;
    } else {
	return $text;
    }
}

# same as above, but get around the 'if it looks like a function' rule
sub maybe_parens_unop {
    my $self = shift;
    my($name, $kid, $cx) = @_;
    if ($cx > 16 or $self->{'parens'}) {
	$kid =  $self->deparse($kid, 1);
 	if ($name eq "umask" && $kid =~ /^\d+$/) {
	    $kid = sprintf("%#o", $kid);
	}
	return $self->keyword($name) . "($kid)";
    } else {
	$kid = $self->deparse($kid, 16);
 	if ($name eq "umask" && $kid =~ /^\d+$/) {
	    $kid = sprintf("%#o", $kid);
	}
	$name = $self->keyword($name);
	if (substr($kid, 0, 1) eq "\cS") {
	    # use kid's parens
	    return $name . substr($kid, 1);
	} elsif (substr($kid, 0, 1) eq "(") {
	    # avoid looks-like-a-function trap with extra parens
	    # ('+' can lead to ambiguities)
	    return "$name(" . $kid  . ")";
	} else {
	    return "$name $kid";
	}
    }
}

sub maybe_parens_func {
    my $self = shift;
    my($func, $text, $cx, $prec) = @_;
    if ($prec <= $cx or substr($text, 0, 1) eq "(" or $self->{'parens'}) {
	return "$func($text)";
    } else {
        return $func . (length($text) ? " $text" : "");
    }
}

sub find_our_type {
    my ($self, $name) = @_;
    $self->populate_curcvlex() if !defined $self->{'curcvlex'};
    my $seq = $self->{'curcop'} ? $self->{'curcop'}->cop_seq : 0;
    for my $a (@{$self->{'curcvlex'}{"o$name"}}) {
	my ($st, undef, $padname) = @$a;
	if ($st >= $seq && $padname->FLAGS & PADNAMEf_TYPED) {
	    return $padname->SvSTASH->NAME;
	}
    }
    return '';
}

sub maybe_local {
    my $self = shift;
    my($op, $cx, $text) = @_;
    my $name = $op->name;
    my $our_intro = ($name =~ /^(?:(?:gv|rv2)[ash]v|split|refassign
				  |lv(?:av)?ref)$/x)
			? OPpOUR_INTRO
			: 0;
    my $lval_intro = $name eq 'split' ? 0 : OPpLVAL_INTRO;
    # The @a in \(@a) isn't in ref context, but only when the
    # parens are there.
    my $need_parens = $self->{'in_refgen'} && $name =~ /[ah]v\z/
		   && ($op->flags & (OPf_PARENS|OPf_REF)) == OPf_PARENS;
    if ((my $priv = $op->private) & ($lval_intro|$our_intro)) {
	my @our_local;
	push @our_local, "local" if $priv & $lval_intro;
	push @our_local, "our"   if $priv & $our_intro;
	my $our_local = join " ", map $self->keyword($_), @our_local;
	if( $our_local[-1] eq 'our' ) {
	    if ( $text !~ /^\W(\w+::)*\w+\z/
	     and !utf8::decode($text) || $text !~ /^\W(\w+::)*\w+\z/
	    ) {
		die "Unexpected our($text)\n";
	    }
	    $text =~ s/(\w+::)+//;

	    if (my $type = $self->find_our_type($text)) {
		$our_local .= ' ' . $type;
	    }
	}
	return $need_parens ? "($text)" : $text
	    if $self->{'avoid_local'}{$$op};
	if ($need_parens) {
	    return "$our_local($text)";
	} elsif (want_scalar($op) || $our_local eq 'our') {
	    return "$our_local $text";
	} else {
	    return $self->maybe_parens_func("$our_local", $text, $cx, 16);
	}
    } else {
	return $need_parens ? "($text)" : $text;
    }
}

sub maybe_targmy {
    my $self = shift;
    my($op, $cx, $func, @args) = @_;
    if ($op->private & OPpTARGET_MY) {
	my $var = $self->padname($op->targ);
	my $val = $func->($self, $op, 7, @args);
	return $self->maybe_parens("$var = $val", $cx, 7);
    } else {
	return $func->($self, $op, $cx, @args);
    }
}

sub padname_sv {
    my $self = shift;
    my $targ = shift;
    return $self->{'curcv'}->PADLIST->ARRAYelt(0)->ARRAYelt($targ);
}

sub maybe_my {
    my $self = shift;
    my($op, $cx, $text, $padname, $forbid_parens) = @_;
    # The @a in \(@a) isn't in ref context, but only when the
    # parens are there.
    my $need_parens = !$forbid_parens && $self->{'in_refgen'}
		   && $op->name =~ /[ah]v\z/
		   && ($op->flags & (OPf_PARENS|OPf_REF)) == OPf_PARENS;
    # The @a in \my @a must not have parens.
    if (!$need_parens && $self->{'in_refgen'}) {
	$forbid_parens = 1;
    }
    if ($op->private & OPpLVAL_INTRO and not $self->{'avoid_local'}{$$op}) {
	# Check $padname->FLAGS for statehood, rather than $op->private,
	# because enteriter ops do not carry the flag.
	my $my =
	    $self->keyword($padname->FLAGS & SVpad_STATE ? "state" : "my");
	if ($padname->FLAGS & PADNAMEf_TYPED) {
	    $my .= ' ' . $padname->SvSTASH->NAME;
	}
	if ($need_parens) {
	    return "$my($text)";
	} elsif ($forbid_parens || want_scalar($op)) {
	    return "$my $text";
	} else {
	    return $self->maybe_parens_func($my, $text, $cx, 16);
	}
    } else {
	return $need_parens ? "($text)" : $text;
    }
}

# The following OPs don't have functions:

# pp_padany -- does not exist after parsing

sub AUTOLOAD {
    if ($AUTOLOAD =~ s/^.*::pp_//) {
	warn "unexpected OP_".
	  ($_[1]->type == OP_CUSTOM ? "CUSTOM ($AUTOLOAD)" : uc $AUTOLOAD);
	return "XXX";
    } else {
	die "Undefined subroutine $AUTOLOAD called";
    }
}

sub DESTROY {}	#	Do not AUTOLOAD

# $root should be the op which represents the root of whatever
# we're sequencing here. If it's undefined, then we don't append
# any subroutine declarations to the deparsed ops, otherwise we
# append appropriate declarations.
sub lineseq {
    my($self, $root, $cx, @ops) = @_;
    my($expr, @exprs);

    my $out_cop = $self->{'curcop'};
    my $out_seq = defined($out_cop) ? $out_cop->cop_seq : undef;
    my $limit_seq;
    if (defined $root) {
	$limit_seq = $out_seq;
	my $nseq;
	$nseq = $self->find_scope_st($root->sibling) if ${$root->sibling};
	$limit_seq = $nseq if !defined($limit_seq)
			   or defined($nseq) && $nseq < $limit_seq;
    }
    $limit_seq = $self->{'limit_seq'}
	if defined($self->{'limit_seq'})
	&& (!defined($limit_seq) || $self->{'limit_seq'} < $limit_seq);
    local $self->{'limit_seq'} = $limit_seq;

    $self->walk_lineseq($root, \@ops,
		       sub { push @exprs, $_[0]} );

    my $sep = $cx ? '; ' : ";\n";
    my $body = join($sep, grep {length} @exprs);
    my $subs = "";
    if (defined $root && defined $limit_seq && !$self->{'in_format'}) {
	$subs = join "\n", $self->seq_subs($limit_seq);
    }
    return join($sep, grep {length} $body, $subs);
}

sub scopeop {
    my($real_block, $self, $op, $cx) = @_;
    my $kid;
    my @kids;

    local(@$self{qw'curstash warnings hints hinthash'})
		= @$self{qw'curstash warnings hints hinthash'} if $real_block;
    if ($real_block) {
	$kid = $op->first->sibling; # skip enter
	if (is_miniwhile($kid)) {
	    my $top = $kid->first;
	    my $name = $top->name;
	    if ($name eq "and") {
		$name = $self->keyword("while");
	    } elsif ($name eq "or") {
		$name = $self->keyword("until");
	    } else { # no conditional -> while 1 or until 0
		return $self->deparse($top->first, 1) . " "
		     . $self->keyword("while") . " 1";
	    }
	    my $cond = $top->first;
	    my $body = $cond->sibling->first; # skip lineseq
	    $cond = $self->deparse($cond, 1);
	    $body = $self->deparse($body, 1);
	    return "$body $name $cond";
	}
        elsif($kid->type == OP_PUSHDEFER &&
            $kid->private & OPpDEFER_FINALLY &&
            $kid->sibling->type == OP_LEAVETRYCATCH &&
            null($kid->sibling->sibling)) {
            return $self->pp_leavetrycatch_with_finally($kid->sibling, $kid, $cx);
        }
    } else {
	$kid = $op->first;
    }
    for (; !null($kid); $kid = $kid->sibling) {
	push @kids, $kid;
    }
    if ($cx > 0) { # inside an expression, (a do {} while for lineseq)
	my $body = $self->lineseq($op, 0, @kids);
	return is_lexical_subs(@kids)
		? $body
		: ($self->lex_in_scope("&do") ? "CORE::do" : "do")
		 . " {\n\t$body\n\b}";
    } else {
	my $lineseq = $self->lineseq($op, $cx, @kids);
	return (length ($lineseq) ? "$lineseq;" : "");
    }
}

sub pp_scope { scopeop(0, @_); }
sub pp_lineseq { scopeop(0, @_); }
sub pp_leave { scopeop(1, @_); }

# This is a special case of scopeop and lineseq, for the case of the
# main_root. The difference is that we print the output statements as
# soon as we get them, for the sake of impatient users.
sub deparse_root {
    my $self = shift;
    my($op) = @_;
    local(@$self{qw'curstash warnings hints hinthash'})
      = @$self{qw'curstash warnings hints hinthash'};
    my @kids;
    return if null $op->first; # Can happen, e.g., for Bytecode without -k
    for (my $kid = $op->first->sibling; !null($kid); $kid = $kid->sibling) {
	push @kids, $kid;
    }
    $self->walk_lineseq($op, \@kids,
			sub { return unless length $_[0];
			      print $self->indent($_[0].';');
			      print "\n"
				unless $_[1] == $#kids;
			  });
}

sub walk_lineseq {
    my ($self, $op, $kids, $callback) = @_;
    my @kids = @$kids;
    for (my $i = 0; $i < @kids; $i++) {
	my $expr = "";
	if (is_state $kids[$i]) {
	    $expr = $self->deparse($kids[$i++], 0);
	    if ($i > $#kids) {
		$callback->($expr, $i);
		last;
	    }
	}
	if (is_for_loop($kids[$i])) {
	    $callback->($expr . $self->for_loop($kids[$i], 0),
		$i += $kids[$i]->sibling->name eq "unstack" ? 2 : 1);
	    next;
	}
	my $expr2 = $self->deparse($kids[$i], (@kids != 1)/2);
	$expr2 =~ s/^sub :(?!:)/+sub :/; # statement label otherwise
	$expr .= $expr2;
	$callback->($expr, $i);
    }
}

# The BEGIN {} is used here because otherwise this code isn't executed
# when you run B::Deparse on itself.
my %globalnames;
BEGIN { map($globalnames{$_}++, "SIG", "STDIN", "STDOUT", "STDERR", "INC",
	    "ENV", "ARGV", "ARGVOUT", "_"); }

sub gv_name {
    my $self = shift;
    my $gv = shift;
    my $raw = shift;
#Carp::confess() unless ref($gv) eq "B::GV";
    my $cv = $gv->FLAGS & SVf_ROK ? $gv->RV : 0;
    my $stash = ($cv || $gv)->STASH->NAME;
    my $name = $raw
	? $cv ? $cv->NAME_HEK || $cv->GV->NAME : $gv->NAME
	: $cv
	    ? B::safename($cv->NAME_HEK || $cv->GV->NAME)
	    : $gv->SAFENAME;
    if ($stash eq 'main' && $name =~ /^::/) {
	$stash = '::';
    }
    elsif (($stash eq 'main'
	    && ($globalnames{$name} || $name =~ /^[^A-Za-z_:]/))
	or ($stash eq $self->{'curstash'} && !$globalnames{$name}
	    && ($stash eq 'main' || $name !~ /::/))
	  )
    {
	$stash = "";
    } else {
	$stash = $stash . "::";
    }
    if (!$raw and $name =~ /^(\^..|{)/) {
        $name = "{$name}";       # ${^WARNING_BITS}, etc and ${
    }
    return $stash . $name;
}

# Return the name to use for a stash variable.
# If a lexical with the same name is in scope, or
# if strictures are enabled, it may need to be
# fully-qualified.
sub stash_variable {
    my ($self, $prefix, $name, $cx) = @_;

    return $prefix.$self->maybe_qualify($prefix, $name) if $name =~ /::/;

    unless ($prefix eq '$' || $prefix eq '@' || $prefix eq '&' || #'
	    $prefix eq '%' || $prefix eq '$#') {
	return "$prefix$name";
    }

    if ($name =~ /^[^[:alpha:]_+-]$/) {
      if (defined $cx && $cx == 26) {
	if ($prefix eq '@') {
	    return "$prefix\{$name}";
	}
	elsif ($name eq '#') { return '${#}' } #  "${#}a" vs "$#a"
      }
      if ($prefix eq '$#') {
	return "\$#{$name}";
      }
    }

    return $prefix . $self->maybe_qualify($prefix, $name);
}

my %unctrl = # portable to EBCDIC
    (
     "\c@" => '@',	# unused
     "\cA" => 'A',
     "\cB" => 'B',
     "\cC" => 'C',
     "\cD" => 'D',
     "\cE" => 'E',
     "\cF" => 'F',
     "\cG" => 'G',
     "\cH" => 'H',
     "\cI" => 'I',
     "\cJ" => 'J',
     "\cK" => 'K',
     "\cL" => 'L',
     "\cM" => 'M',
     "\cN" => 'N',
     "\cO" => 'O',
     "\cP" => 'P',
     "\cQ" => 'Q',
     "\cR" => 'R',
     "\cS" => 'S',
     "\cT" => 'T',
     "\cU" => 'U',
     "\cV" => 'V',
     "\cW" => 'W',
     "\cX" => 'X',
     "\cY" => 'Y',
     "\cZ" => 'Z',
     "\c[" => '[',	# unused
     "\c\\" => '\\',	# unused
     "\c]" => ']',	# unused
     "\c_" => '_',	# unused
    );

# Return just the name, without the prefix.  It may be returned as a quoted
# string.  The second return value is a boolean indicating that.
sub stash_variable_name {
    my($self, $prefix, $gv) = @_;
    my $name = $self->gv_name($gv, 1);
    $name = $self->maybe_qualify($prefix,$name);
    if ($name =~ /^(?:\S|(?!\d)[\ca-\cz]?(?:\w|::)*|\d+)\z/) {
	$name =~ s/^([\ca-\cz])/'^' . $unctrl{$1}/e;
	$name =~ /^(\^..|{)/ and $name = "{$name}";
	return $name, 0; # not quoted
    }
    else {
	single_delim("q", "'", $name, $self), 1;
    }
}

sub maybe_qualify {
    my ($self,$prefix,$name) = @_;
    my $v = ($prefix eq '$#' ? '@' : $prefix) . $name;
    if ($prefix eq "") {
	$name .= "::" if $name =~ /(?:\ACORE::[^:]*|::)\z/;
	return $name;
    }
    return $name if $name =~ /::/;
    return $self->{'curstash'}.'::'. $name
	if
	    $name =~ /^(?!\d)\w/         # alphabetic
	 && $v    !~ /^\$[ab]\z/	 # not $a or $b
	 && $v =~ /\A[\$\@\%\&]/         # scalar, array, hash, or sub
	 && !$globalnames{$name}         # not a global name
	 && $self->{hints} & $strict_bits{vars}  # strict vars
	 && !$self->lex_in_scope($v,1)   # no "our"
      or $self->lex_in_scope($v);        # conflicts with "my" variable
    return $name;
}

sub lex_in_scope {
    my ($self, $name, $our) = @_;
    substr $name, 0, 0, = $our ? 'o' : 'm'; # our/my
    $self->populate_curcvlex() if !defined $self->{'curcvlex'};

    return 0 if !defined($self->{'curcop'});
    my $seq = $self->{'curcop'}->cop_seq;
    return 0 if !exists $self->{'curcvlex'}{$name};
    for my $a (@{$self->{'curcvlex'}{$name}}) {
	my ($st, $en) = @$a;
	return 1 if $seq > $st && $seq <= $en;
    }
    return 0;
}

sub populate_curcvlex {
    my $self = shift;
    for (my $cv = $self->{'curcv'}; class($cv) eq "CV"; $cv = $cv->OUTSIDE) {
	my $padlist = $cv->PADLIST;
	# an undef CV still in lexical chain
	next if class($padlist) eq "SPECIAL";
	my @padlist = $padlist->ARRAY;
	my @ns = $padlist[0]->ARRAY;

	for (my $i=0; $i<@ns; ++$i) {
	    next if class($ns[$i]) eq "SPECIAL";
	    if (class($ns[$i]) eq "PV") {
		# Probably that pesky lexical @_
		next;
	    }
            my $name = $ns[$i]->PVX;
	    next unless defined $name;
	    my ($seq_st, $seq_en) =
		($ns[$i]->FLAGS & SVf_FAKE)
		    ? (0, 999999)
		    : ($ns[$i]->COP_SEQ_RANGE_LOW, $ns[$i]->COP_SEQ_RANGE_HIGH);

	    push @{$self->{'curcvlex'}{
			($ns[$i]->FLAGS & PADNAMEf_OUR ? 'o' : 'm') . $name
		  }}, [$seq_st, $seq_en, $ns[$i]];
	}
    }
}

sub find_scope_st { ((find_scope(@_))[0]); }
sub find_scope_en { ((find_scope(@_))[1]); }

# Recurses down the tree, looking for pad variable introductions and COPs
sub find_scope {
    my ($self, $op, $scope_st, $scope_en) = @_;
    carp("Undefined op in find_scope") if !defined $op;
    return ($scope_st, $scope_en) unless $op->flags & OPf_KIDS;

    my @queue = ($op);
    while(my $op = shift @queue ) {
	for (my $o=$op->first; $$o; $o=$o->sibling) {
	    if ($o->name =~ /^pad.v$/ && $o->private & OPpLVAL_INTRO) {
		my $s = int($self->padname_sv($o->targ)->COP_SEQ_RANGE_LOW);
		my $e = $self->padname_sv($o->targ)->COP_SEQ_RANGE_HIGH;
		$scope_st = $s if !defined($scope_st) || $s < $scope_st;
		$scope_en = $e if !defined($scope_en) || $e > $scope_en;
		return ($scope_st, $scope_en);
	    }
	    elsif (is_state($o)) {
		my $c = $o->cop_seq;
		$scope_st = $c if !defined($scope_st) || $c < $scope_st;
		$scope_en = $c if !defined($scope_en) || $c > $scope_en;
		return ($scope_st, $scope_en);
	    }
	    elsif ($o->flags & OPf_KIDS) {
		unshift (@queue, $o);
	    }
	}
    }

    return ($scope_st, $scope_en);
}

# Returns a list of subs which should be inserted before the COP
sub cop_subs {
    my ($self, $op, $out_seq) = @_;
    my $seq = $op->cop_seq;
    $seq = $out_seq if defined($out_seq) && $out_seq < $seq;
    return $self->seq_subs($seq);
}

sub seq_subs {
    my ($self, $seq) = @_;
    my @text;
#push @text, "# ($seq)\n";

    return "" if !defined $seq;
    my @pending;
    while (scalar(@{$self->{'subs_todo'}})
	   and $seq > $self->{'subs_todo'}[0][0]) {
	my $cv = $self->{'subs_todo'}[0][1];
	# Skip the OUTSIDE check for lexical subs.  We may be deparsing a
	# cloned anon sub with lexical subs declared in it, in which case
	# the OUTSIDE pointer points to the anon protosub.
	my $lexical = ref $self->{'subs_todo'}[0][3];
	my $outside = !$lexical && $cv && $cv->OUTSIDE;
	if (!$lexical and $cv
	 and ${$cv->OUTSIDE || \0} != ${$self->{'curcv'}})
	{
	    push @pending, shift @{$self->{'subs_todo'}};
	    next;
	}
	push @text, $self->next_todo;
    }
    unshift @{$self->{'subs_todo'}}, @pending;
    return @text;
}

sub _features_from_bundle {
    my ($hints, $hh) = @_;
    foreach (@{$feature::feature_bundle{@feature::hint_bundles[$hints >> $feature::hint_shift]}}) {
	$hh->{$feature::feature{$_}} = 1;
    }
    return $hh;
}

# generate any pragmas, 'package foo' etc needed to synchronise
# with the given cop

sub pragmata {
    my $self = shift;
    my($op) = @_;

    my @text;

    my $stash = $op->stashpv;
    if ($stash ne $self->{'curstash'}) {
	push @text, $self->keyword("package") . " $stash;\n";
	$self->{'curstash'} = $stash;
    }

    my $warnings = $op->warnings;
    my $warning_bits;
    if ($warnings->isa("B::SPECIAL") && $$warnings == 4) {
	$warning_bits = $warnings::Bits{"all"};
    }
    elsif ($warnings->isa("B::SPECIAL") && $$warnings == 5) {
        $warning_bits = $warnings::NONE;
    }
    elsif ($warnings->isa("B::SPECIAL")) {
	$warning_bits = undef;
    }
    else {
	$warning_bits = $warnings->PV;
    }

    my ($w1, $w2);
    # The number of valid bit positions may have grown (by a byte or
    # more) since the last warnings state, by custom warnings
    # categories being registered in the meantime. Normalise the
    # bitmasks first so they may be fairly compared.
    $w1 = defined($self->{warnings})
                ? warnings::_expand_bits($self->{warnings})
                : undef;
    $w2 = defined($warning_bits)
                ? warnings::_expand_bits($warning_bits)
                : undef;

    if (defined($w2) and !defined($w1) || $w1 ne $w2) {
	push @text, $self->declare_warnings($w1, $w2);
	$self->{'warnings'} = $w2;
    }

    my $hints = $op->hints;
    my $old_hints = $self->{'hints'};
    if ($self->{'hints'} != $hints) {
	push @text, $self->declare_hints($self->{'hints'}, $hints);
	$self->{'hints'} = $hints;
    }

    my $newhh;
    $newhh = $op->hints_hash->HASH;

    {
	# feature bundle hints
	my $from = $old_hints & $feature::hint_mask;
	my $to   = $    hints & $feature::hint_mask;
	if ($from != $to) {
	    if ($to == $feature::hint_mask) {
		if ($self->{'hinthash'}) {
		    delete $self->{'hinthash'}{$_}
			for grep /^feature_/, keys %{$self->{'hinthash'}};
		}
		else { $self->{'hinthash'} = {} }
		$self->{'hinthash'}
		    = _features_from_bundle($from, $self->{'hinthash'});
	    }
	    else {
		my $bundle =
		    $feature::hint_bundles[$to >> $feature::hint_shift];
		$bundle =~ s/(\d[13579])\z/$1+1/e; # 5.11 => 5.12
		push @text,
		    $self->keyword("no") . " feature ':all';\n",
		    $self->keyword("use") . " feature ':$bundle';\n";
	    }
	}
    }

    {
	push @text, $self->declare_hinthash(
	    $self->{'hinthash'}, $newhh,
	    $self->{indent_size}, $self->{hints},
	);
	$self->{'hinthash'} = $newhh;
    }

    return join("", @text);
}


# Notice how subs and formats are inserted between statements here;
# also $[ assignments and pragmas.
sub pp_nextstate {
    my $self = shift;
    my($op, $cx) = @_;
    $self->{'curcop'} = $op;

    my @text;

    my @subs = $self->cop_subs($op);
    if (@subs) {
	# Special marker to swallow up the semicolon
	push @subs, "\cK";
    }
    push @text, @subs;

    push @text, $self->pragmata($op);


    # This should go after of any branches that add statements, to
    # increase the chances that it refers to the same line it did in
    # the original program.
    if ($self->{'linenums'} && $cx != .5) { # $cx == .5 means in a format
	push @text, "\f#line " . $op->line .
	  ' "' . $op->file, qq'"\n';
    }

    push @text, $op->label . ": " if $op->label;

    return join("", @text);
}

sub declare_warnings {
    my ($self, $from, $to) = @_;
    $from //= '';
    my $all = warnings::bits("all");
    unless (($from & ~$all) =~ /[^\0]/) {
        # no FATAL bits need turning off
        if (   $to eq $all) {
            return $self->keyword("use") . " warnings;\n";
        }
        elsif ($to eq ("\0"x length($to))) {
            return $self->keyword("no") . " warnings;\n";
        }
    }

    return "BEGIN {\${^WARNING_BITS} = \""
           . join("", map { sprintf("\\x%02x", ord $_) } split "", $to)
           . "\"}\n\cK";
}

sub declare_hints {
    my ($self, $from, $to) = @_;
    my $use = $to   & ~$from;
    my $no  = $from & ~$to;
    my $decls = "";
    for my $pragma (hint_pragmas($use)) {
	$decls .= $self->keyword("use") . " $pragma;\n";
    }
    for my $pragma (hint_pragmas($no)) {
        $decls .= $self->keyword("no") . " $pragma;\n";
    }
    return $decls;
}

# Internal implementation hints that the core sets automatically, so don't need
# (or want) to be passed back to the user
my %ignored_hints = (
    'open<' => 1,
    'open>' => 1,
    ':'     => 1,
    'strict/refs' => 1,
    'strict/subs' => 1,
    'strict/vars' => 1,
    'feature/bits' => 1,
);

my %rev_feature;

sub declare_hinthash {
    my ($self, $from, $to, $indent, $hints) = @_;
    my $doing_features =
	($hints & $feature::hint_mask) == $feature::hint_mask;
    my @decls;
    my @features;
    my @unfeatures; # bugs?
    for my $key (sort keys %$to) {
	next if $ignored_hints{$key};
	my $is_feature = $key =~ /^feature_/;
	next if $is_feature and not $doing_features;
	if (!exists $from->{$key} or $from->{$key} ne $to->{$key}) {
	    push(@features, $key), next if $is_feature;
	    push @decls,
		qq(\$^H{) . single_delim("q", "'", $key, $self) . qq(} = )
	      . (
		   defined $to->{$key}
			? single_delim("q", "'", $to->{$key}, $self)
			: 'undef'
		)
	      . qq(;);
	}
    }
    for my $key (sort keys %$from) {
	next if $ignored_hints{$key};
	my $is_feature = $key =~ /^feature_/;
	next if $is_feature and not $doing_features;
	if (!exists $to->{$key}) {
	    push(@unfeatures, $key), next if $is_feature;
	    push @decls, qq(delete \$^H{'$key'};);
	}
    }
    my @ret;
    if (@features || @unfeatures) {
	if (!%rev_feature) { %rev_feature = reverse %feature::feature }
    }
    if (@features) {
	push @ret, $self->keyword("use") . " feature "
		 . join(", ", map "'$rev_feature{$_}'", @features) . ";\n";
    }
    if (@unfeatures) {
	push @ret, $self->keyword("no") . " feature "
		 . join(", ", map "'$rev_feature{$_}'", @unfeatures)
		 . ";\n";
    }
    @decls and
	push @ret,
	     join("\n" . (" " x $indent), "BEGIN {", @decls) . "\n}\n\cK";
    return @ret;
}

sub hint_pragmas {
    my ($bits) = @_;
    my (@pragmas, @strict);
    push @pragmas, "integer" if $bits & 0x1;
    for (sort keys %strict_bits) {
	push @strict, "'$_'" if $bits & $strict_bits{$_};
    }
    if (@strict == keys %strict_bits) {
	push @pragmas, "strict";
    }
    elsif (@strict) {
	push @pragmas, "strict " . join ', ', @strict;
    }
    push @pragmas, "bytes" if $bits & 0x8;
    return @pragmas;
}

sub pp_dbstate { pp_nextstate(@_) }
sub pp_setstate { pp_nextstate(@_) }

sub pp_unstack { return "" } # see also leaveloop

my %feature_keywords = (
  # keyword => 'feature',
    state   => 'state',
    say     => 'say',
    given   => 'switch',
    when    => 'switch',
    default => 'switch',
    break   => 'switch',
    evalbytes=>'evalbytes',
    __SUB__ => '__SUB__',
   fc       => 'fc',
   try      => 'try',
   catch    => 'try',
   finally  => 'try',
   defer    => 'defer',
   signatures => 'signatures',
);

# keywords that are strong and also have a prototype
#
my %strong_proto_keywords = map { $_ => 1 } qw(
    pos
    prototype
    scalar
    study
    undef
);

sub feature_enabled {
	my($self,$name) = @_;
	my $hh;
	my $hints = $self->{hints} & $feature::hint_mask;
	if ($hints && $hints != $feature::hint_mask) {
	    $hh = _features_from_bundle($hints);
	}
	elsif ($hints) { $hh = $self->{'hinthash'} }
	return $hh && $hh->{"feature_$feature_keywords{$name}"}
}

sub keyword {
    my $self = shift;
    my $name = shift;
    return $name if $name =~ /^CORE::/; # just in case
    if (exists $feature_keywords{$name}) {
	return "CORE::$name" if not $self->feature_enabled($name);
    }
    # This sub may be called for a program that has no nextstate ops.  In
    # that case we may have a lexical sub named no/use/sub in scope but
    # $self->lex_in_scope will return false because it depends on the
    # current nextstate op.  So we need this alternate method if there is
    # no current cop.
    if (!$self->{'curcop'}) {
	$self->populate_curcvlex() if !defined $self->{'curcvlex'};
	return "CORE::$name" if exists $self->{'curcvlex'}{"m&$name"}
			     || exists $self->{'curcvlex'}{"o&$name"};
    } elsif ($self->lex_in_scope("&$name")
	  || $self->lex_in_scope("&$name", 1)) {
	return "CORE::$name";
    }
    if ($strong_proto_keywords{$name}
        || ($name !~ /^(?:chom?p|do|exec|glob|s(?:elect|ystem))\z/
	    && !defined eval{prototype "CORE::$name"})
    ) { return $name }
    if (
	exists $self->{subs_declared}{$name}
	 or
	exists &{"$self->{curstash}::$name"}
    ) {
	return "CORE::$name"
    }
    return $name;
}

sub baseop {
    my $self = shift;
    my($op, $cx, $name) = @_;
    return $self->keyword($name);
}

sub pp_stub { "()" }
sub pp_wantarray { baseop(@_, "wantarray") }
sub pp_fork { baseop(@_, "fork") }
sub pp_wait { maybe_targmy(@_, \&baseop, "wait") }
sub pp_getppid { maybe_targmy(@_, \&baseop, "getppid") }
sub pp_time { maybe_targmy(@_, \&baseop, "time") }
sub pp_tms { baseop(@_, "times") }
sub pp_ghostent { baseop(@_, "gethostent") }
sub pp_gnetent { baseop(@_, "getnetent") }
sub pp_gprotoent { baseop(@_, "getprotoent") }
sub pp_gservent { baseop(@_, "getservent") }
sub pp_ehostent { baseop(@_, "endhostent") }
sub pp_enetent { baseop(@_, "endnetent") }
sub pp_eprotoent { baseop(@_, "endprotoent") }
sub pp_eservent { baseop(@_, "endservent") }
sub pp_gpwent { baseop(@_, "getpwent") }
sub pp_spwent { baseop(@_, "setpwent") }
sub pp_epwent { baseop(@_, "endpwent") }
sub pp_ggrent { baseop(@_, "getgrent") }
sub pp_sgrent { baseop(@_, "setgrent") }
sub pp_egrent { baseop(@_, "endgrent") }
sub pp_getlogin { baseop(@_, "getlogin") }

sub POSTFIX () { 1 }

# I couldn't think of a good short name, but this is the category of
# symbolic unary operators with interesting precedence

sub pfixop {
    my $self = shift;
    my($op, $cx, $name, $prec, $flags) = (@_, 0);
    my $kid = $op->first;
    $kid = $self->deparse($kid, $prec);
    return $self->maybe_parens(($flags & POSTFIX)
				 ? "$kid$name"
				   # avoid confusion with filetests
				 : $name eq '-'
				   && $kid =~ /^[a-zA-Z](?!\w)/
					? "$name($kid)"
					: "$name$kid",
			       $cx, $prec);
}

sub pp_preinc { pfixop(@_, "++", 23) }
sub pp_predec { pfixop(@_, "--", 23) }
sub pp_postinc { maybe_targmy(@_, \&pfixop, "++", 23, POSTFIX) }
sub pp_postdec { maybe_targmy(@_, \&pfixop, "--", 23, POSTFIX) }
sub pp_i_preinc { pfixop(@_, "++", 23) }
sub pp_i_predec { pfixop(@_, "--", 23) }
sub pp_i_postinc { maybe_targmy(@_, \&pfixop, "++", 23, POSTFIX) }
sub pp_i_postdec { maybe_targmy(@_, \&pfixop, "--", 23, POSTFIX) }
sub pp_complement { maybe_targmy(@_, \&pfixop, "~", 21) }
*pp_ncomplement = *pp_complement;
sub pp_scomplement { maybe_targmy(@_, \&pfixop, "~.", 21) }

sub pp_negate { maybe_targmy(@_, \&real_negate) }
sub real_negate {
    my $self = shift;
    my($op, $cx) = @_;
    if ($op->first->name =~ /^(i_)?negate$/) {
	# avoid --$x
	$self->pfixop($op, $cx, "-", 21.5);
    } else {
	$self->pfixop($op, $cx, "-", 21);	
    }
}
sub pp_i_negate { pp_negate(@_) }

sub pp_not {
    my $self = shift;
    my($op, $cx) = @_;
    if ($cx <= 4) {
	$self->listop($op, $cx, "not", $op->first);
    } else {
	$self->pfixop($op, $cx, "!", 21);	
    }
}

sub unop {
    my $self = shift;
    my($op, $cx, $name, $nollafr) = @_;
    my $kid;
    if ($op->flags & OPf_KIDS) {
	$kid = $op->first;
 	if (not $name) {
 	    # this deals with 'boolkeys' right now
 	    return $self->deparse($kid,$cx);
 	}
	my $builtinname = $name;
	$builtinname =~ /^CORE::/ or $builtinname = "CORE::$name";
	if (defined prototype($builtinname)
	   && $builtinname ne 'CORE::readline'
	   && prototype($builtinname) =~ /^;?\*/
	   && $kid->name eq "rv2gv") {
	    $kid = $kid->first;
	}

	if ($nollafr) {
	    if (($kid = $self->deparse($kid, 16)) !~ s/^\cS//) {
		# require foo() is a syntax error.
		$kid =~ /^(?!\d)\w/ and $kid = "($kid)";
	    }
	    return $self->maybe_parens(
			$self->keyword($name) . " $kid", $cx, 16
		   );
	}
	return $self->maybe_parens_unop($name, $kid, $cx);
    } else {
	return $self->maybe_parens(
	    $self->keyword($name) . ($op->flags & OPf_SPECIAL ? "()" : ""),
	    $cx, 16,
	);
    }
}

sub pp_chop { maybe_targmy(@_, \&unop, "chop") }
sub pp_chomp { maybe_targmy(@_, \&unop, "chomp") }
sub pp_schop { maybe_targmy(@_, \&unop, "chop") }
sub pp_schomp { maybe_targmy(@_, \&unop, "chomp") }
sub pp_defined { unop(@_, "defined") }
sub pp_undef {
    if ($_[1]->private & OPpTARGET_MY) {
        my $targ = $_[1]->targ;
        my $var = $_[0]->maybe_my($_[1], $_[2], $_[0]->padname($targ),
            $_[0]->padname_sv($targ),
            1);
        my $func = unop(@_, "undef");
        if ($func =~ /\s/) {
            return unop(@_, "undef").$var;
        } else {
            return "$var = undef";
        }
    }
    unop(@_, "undef") 
}
sub pp_study { unop(@_, "study") }
sub pp_ref { unop(@_, "ref") }
sub pp_pos { maybe_local(@_, unop(@_, "pos")) }

sub pp_sin { maybe_targmy(@_, \&unop, "sin") }
sub pp_cos { maybe_targmy(@_, \&unop, "cos") }
sub pp_rand { maybe_targmy(@_, \&unop, "rand") }
sub pp_srand { unop(@_, "srand") }
sub pp_exp { maybe_targmy(@_, \&unop, "exp") }
sub pp_log { maybe_targmy(@_, \&unop, "log") }
sub pp_sqrt { maybe_targmy(@_, \&unop, "sqrt") }
sub pp_int { maybe_targmy(@_, \&unop, "int") }
sub pp_hex { maybe_targmy(@_, \&unop, "hex") }
sub pp_oct { maybe_targmy(@_, \&unop, "oct") }
sub pp_abs { maybe_targmy(@_, \&unop, "abs") }

sub pp_length { maybe_targmy(@_, \&unop, "length") }
sub pp_ord { maybe_targmy(@_, \&unop, "ord") }
sub pp_chr { maybe_targmy(@_, \&unop, "chr") }

sub pp_each { unop(@_, "each") }
sub pp_values { unop(@_, "values") }
sub pp_keys { unop(@_, "keys") }
{ no strict 'refs'; *{"pp_r$_"} = *{"pp_$_"} for qw< keys each values >; }
sub pp_boolkeys {
    # no name because its an optimisation op that has no keyword
    unop(@_,"");
}
sub pp_aeach { unop(@_, "each") }
sub pp_avalues { unop(@_, "values") }
sub pp_akeys { unop(@_, "keys") }
sub pp_pop { unop(@_, "pop") }
sub pp_shift { unop(@_, "shift") }

sub pp_caller { unop(@_, "caller") }
sub pp_reset { unop(@_, "reset") }
sub pp_exit { unop(@_, "exit") }
sub pp_prototype { unop(@_, "prototype") }

sub pp_close { unop(@_, "close") }
sub pp_fileno { unop(@_, "fileno") }
sub pp_umask { unop(@_, "umask") }
sub pp_untie { unop(@_, "untie") }
sub pp_tied { unop(@_, "tied") }
sub pp_dbmclose { unop(@_, "dbmclose") }
sub pp_getc { unop(@_, "getc") }
sub pp_eof { unop(@_, "eof") }
sub pp_tell { unop(@_, "tell") }
sub pp_getsockname { unop(@_, "getsockname") }
sub pp_getpeername { unop(@_, "getpeername") }

sub pp_chdir {
    my ($self, $op, $cx) = @_;
    if (($op->flags & (OPf_SPECIAL|OPf_KIDS)) == (OPf_SPECIAL|OPf_KIDS)) {
	my $kw = $self->keyword("chdir");
	my $kid = $self->const_sv($op->first)->PV;
	my $code = $kw
		 . ($cx >= 16 || $self->{'parens'} ? "($kid)" : " $kid");
	maybe_targmy(@_, sub { $_[3] }, $code);
    } else {
	maybe_targmy(@_, \&unop, "chdir")
    }
}

sub pp_chroot { maybe_targmy(@_, \&unop, "chroot") }
sub pp_readlink { unop(@_, "readlink") }
sub pp_rmdir { maybe_targmy(@_, \&unop, "rmdir") }
sub pp_readdir { unop(@_, "readdir") }
sub pp_telldir { unop(@_, "telldir") }
sub pp_rewinddir { unop(@_, "rewinddir") }
sub pp_closedir { unop(@_, "closedir") }
sub pp_getpgrp { maybe_targmy(@_, \&unop, "getpgrp") }
sub pp_localtime { unop(@_, "localtime") }
sub pp_gmtime { unop(@_, "gmtime") }
sub pp_alarm { unop(@_, "alarm") }
sub pp_sleep { maybe_targmy(@_, \&unop, "sleep") }

sub pp_dofile {
    my $code = unop(@_, "do", 1); # llafr does not apply
    if ($code =~ s/^((?:CORE::)?do) \{/$1({/) { $code .= ')' }
    $code;
}
sub pp_entereval {
    unop(
      @_,
      $_[1]->private & OPpEVAL_BYTES ? 'evalbytes' : "eval"
    )
}

sub pp_ghbyname { unop(@_, "gethostbyname") }
sub pp_gnbyname { unop(@_, "getnetbyname") }
sub pp_gpbyname { unop(@_, "getprotobyname") }
sub pp_shostent { unop(@_, "sethostent") }
sub pp_snetent { unop(@_, "setnetent") }
sub pp_sprotoent { unop(@_, "setprotoent") }
sub pp_sservent { unop(@_, "setservent") }
sub pp_gpwnam { unop(@_, "getpwnam") }
sub pp_gpwuid { unop(@_, "getpwuid") }
sub pp_ggrnam { unop(@_, "getgrnam") }
sub pp_ggrgid { unop(@_, "getgrgid") }

sub pp_lock { unop(@_, "lock") }

sub pp_continue { unop(@_, "continue"); }
sub pp_break { unop(@_, "break"); }

sub givwhen {
    my $self = shift;
    my($op, $cx, $givwhen) = @_;

    my $enterop = $op->first;
    my ($head, $block);
    if ($enterop->flags & OPf_SPECIAL) {
	$head = $self->keyword("default");
	$block = $self->deparse($enterop->first, 0);
    }
    else {
	my $cond = $enterop->first;
	my $cond_str = $self->deparse($cond, 1);
	$head = "$givwhen ($cond_str)";
	$block = $self->deparse($cond->sibling, 0);
    }

    return "$head {\n".
	"\t$block\n".
	"\b}\cK";
}

sub pp_leavegiven { givwhen(@_, $_[0]->keyword("given")); }
sub pp_leavewhen  { givwhen(@_, $_[0]->keyword("when")); }

sub pp_exists {
    my $self = shift;
    my($op, $cx) = @_;
    my $arg;
    my $name = $self->keyword("exists");
    if ($op->private & OPpEXISTS_SUB) {
	# Checking for the existence of a subroutine
	return $self->maybe_parens_func($name,
				$self->pp_rv2cv($op->first, 16), $cx, 16);
    }
    if ($op->flags & OPf_SPECIAL) {
	# Array element, not hash element
	return $self->maybe_parens_func($name,
				$self->pp_aelem($op->first, 16), $cx, 16);
    }
    return $self->maybe_parens_func($name, $self->pp_helem($op->first, 16),
				    $cx, 16);
}

sub pp_delete {
    my $self = shift;
    my($op, $cx) = @_;
    my $arg;
    my $name = $self->keyword("delete");
    if ($op->private & (OPpSLICE|OPpKVSLICE)) {
	if ($op->flags & OPf_SPECIAL) {
	    # Deleting from an array, not a hash
	    return $self->maybe_parens_func($name,
					$self->pp_aslice($op->first, 16),
					$cx, 16);
	}
	return $self->maybe_parens_func($name,
					$self->pp_hslice($op->first, 16),
					$cx, 16);
    } else {
	if ($op->flags & OPf_SPECIAL) {
	    # Deleting from an array, not a hash
	    return $self->maybe_parens_func($name,
					$self->pp_aelem($op->first, 16),
					$cx, 16);
	}
	return $self->maybe_parens_func($name,
					$self->pp_helem($op->first, 16),
					$cx, 16);
    }
}

sub pp_require {
    my $self = shift;
    my($op, $cx) = @_;
    my $opname = $op->flags & OPf_SPECIAL ? 'CORE::require' : 'require';
    my $kid = $op->first;
    if ($kid->name eq 'const') {
	my $priv = $kid->private;
	my $sv = $self->const_sv($kid);
	my $arg;
	if ($priv & OPpCONST_BARE) {
	    $arg = $sv->PV;
	    $arg =~ s[/][::]g;
	    $arg =~ s/\.pm//g;
	} elsif ($priv & OPpCONST_NOVER) {
	    $opname = $self->keyword('no');
	    $arg = $self->const($sv, 16);
	} elsif ((my $tmp = $self->const($sv, 16)) =~ /^v/) {
	    $arg = $tmp;
	}
	if ($arg) {
	    return $self->maybe_parens("$opname $arg", $cx, 16);
	}
    }
    $self->unop(
	    $op, $cx,
	    $opname,
	    1, # llafr does not apply
    );
}

sub pp_scalar {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    if (not null $kid->sibling) {
	# XXX Was a here-doc
	return $self->dquote($op);
    }
    $self->unop(@_, "scalar");
}


sub padval {
    my $self = shift;
    my $targ = shift;
    return $self->{'curcv'}->PADLIST->ARRAYelt(1)->ARRAYelt($targ);
}

sub anon_hash_or_list {
    my $self = shift;
    my($op, $cx) = @_;

    my($pre, $post) = @{{"anonlist" => ["[","]"],
			 "anonhash" => ["{","}"]}->{$op->name}};
    my($expr, @exprs);
    $op = $op->first->sibling; # skip pushmark
    for (; !null($op); $op = $op->sibling) {
	$expr = $self->deparse($op, 6);
	push @exprs, $expr;
    }
    if ($pre eq "{" and $cx < 1) {
	# Disambiguate that it's not a block
	$pre = "+{";
    }
    return $pre . join(", ", @exprs) . $post;
}

sub pp_anonlist {
    my $self = shift;
    my ($op, $cx) = @_;
    if ($op->flags & OPf_SPECIAL) {
	return $self->anon_hash_or_list($op, $cx);
    }
    warn "Unexpected op pp_" . $op->name() . " without OPf_SPECIAL";
    return 'XXX';
}

*pp_anonhash = \&pp_anonlist;

sub pp_emptyavhv {
    my $self = shift;
    my ($op, $cx, $forbid_parens) = @_;
    my $val = ($op->private & OPpEMPTYAVHV_IS_HV) ? '{}' : '[]';
    if ($op->private & OPpTARGET_MY) {
        my $targ = $op->targ;
        my $var = $self->maybe_my($op, $cx, $self->padname($targ),
                           $self->padname_sv($targ),
                           $forbid_parens);
        return $self->maybe_parens("$var = $val", $cx, 7);
    } else {
        return $val;
    }
}

sub pp_refgen {
    my $self = shift;	
    my($op, $cx) = @_;
    my $kid = $op->first;
    if ($kid->name eq "null") {
	my $anoncode = $kid = $kid->first;

	# Perl no longer generates this, but XS modules might:
	if ($anoncode->name eq "anonconst") {
	    $anoncode = $anoncode->first->first->sibling;
	}

	# Same as with `anonconst`:
	if ($anoncode->name eq "anoncode"
	 or !null($anoncode = $kid->sibling) and
		 $anoncode->name eq "anoncode") {
            return $self->e_anoncode({ code => $self->padval($anoncode->targ) });

	# Perl still generates this:
	} elsif ($kid->name eq "pushmark") {
            my $sib_name = $kid->sibling->name;
            if ($sib_name eq 'entersub') {
                my $text = $self->deparse($kid->sibling, 1);
                # Always show parens for \(&func()), but only with -p otherwise
                $text = "($text)" if $self->{'parens'}
                                 or $kid->sibling->private & OPpENTERSUB_AMPER;
                return "\\$text";
            }
        }
    }
    local $self->{'in_refgen'} = 1;
    $self->pfixop($op, $cx, "\\", 20);
}

sub e_anoncode {
    my ($self, $info) = @_;
    my $text = $self->deparse_sub($info->{code});
    return $self->keyword("sub") . " $text";
}

sub pp_anoncode {
    my ($self, $anoncode) = @_;

    return $self->e_anoncode( { code => $self->padval($anoncode->targ) } );
}

sub pp_anonconst {
    my ($self, $anonconst) = @_;

    return $self->pp_anoncode( $anonconst->first->first->sibling );
}

sub pp_srefgen { pp_refgen(@_) }

sub pp_readline {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    if (is_scalar($kid)
        and $op->flags & OPf_SPECIAL
        and $self->deparse($kid, 1) eq 'ARGV')
    {
        return '<<>>';
    }
    return $self->unop($op, $cx, "readline");
}

sub pp_rcatline {
    my $self = shift;
    my($op) = @_;
    return "<" . $self->gv_name($self->gv_or_padgv($op)) . ">";
}

# Unary operators that can occur as pseudo-listops inside double quotes
sub dq_unop {
    my $self = shift;
    my($op, $cx, $name, $prec, $flags) = (@_, 0, 0);
    my $kid;
    if ($op->flags & OPf_KIDS) {
       $kid = $op->first;
       # If there's more than one kid, the first is an ex-pushmark.
       $kid = $kid->sibling if not null $kid->sibling;
       return $self->maybe_parens_unop($name, $kid, $cx);
    } else {
       return $name .  ($op->flags & OPf_SPECIAL ? "()" : "");
    }
}

sub pp_ucfirst { dq_unop(@_, "ucfirst") }
sub pp_lcfirst { dq_unop(@_, "lcfirst") }
sub pp_uc { dq_unop(@_, "uc") }
sub pp_lc { dq_unop(@_, "lc") }
sub pp_quotemeta { maybe_targmy(@_, \&dq_unop, "quotemeta") }
sub pp_fc { dq_unop(@_, "fc") }

sub loopex {
    my $self = shift;
    my ($op, $cx, $name) = @_;
    if (class($op) eq "PVOP") {
	$name .= " " . $op->pv;
    } elsif (class($op) eq "OP") {
	# no-op
    } elsif (class($op) eq "UNOP") {
	(my $kid = $self->deparse($op->first, 7)) =~ s/^\cS//;
	# last foo() is a syntax error.
	$kid =~ /^(?!\d)\w/ and $kid = "($kid)";
	$name .= " $kid";
    }
    return $self->maybe_parens($name, $cx, 7);
}

sub pp_last { loopex(@_, "last") }
sub pp_next { loopex(@_, "next") }
sub pp_redo { loopex(@_, "redo") }
sub pp_goto { loopex(@_, "goto") }
sub pp_dump { loopex(@_, "CORE::dump") }

sub ftst {
    my $self = shift;
    my($op, $cx, $name) = @_;
    if (class($op) eq "UNOP") {
	# Genuine '-X' filetests are exempt from the LLAFR, but not
	# l?stat()
	if ($name =~ /^-/) {
	    (my $kid = $self->deparse($op->first, 16)) =~ s/^\cS//;
	    return $self->maybe_parens("$name $kid", $cx, 16);
	}
	return $self->maybe_parens_unop($name, $op->first, $cx);
    } elsif (class($op) =~ /^(SV|PAD)OP$/) {
	return $self->maybe_parens_func($name, $self->pp_gv($op, 1), $cx, 16);
    } else { # I don't think baseop filetests ever survive ck_ftst, but...
	return $name;
    }
}

sub pp_lstat    { ftst(@_, "lstat") }
sub pp_stat     { ftst(@_, "stat") }
sub pp_ftrread  { ftst(@_, "-R") }
sub pp_ftrwrite { ftst(@_, "-W") }
sub pp_ftrexec  { ftst(@_, "-X") }
sub pp_fteread  { ftst(@_, "-r") }
sub pp_ftewrite { ftst(@_, "-w") }
sub pp_fteexec  { ftst(@_, "-x") }
sub pp_ftis     { ftst(@_, "-e") }
sub pp_fteowned { ftst(@_, "-O") }
sub pp_ftrowned { ftst(@_, "-o") }
sub pp_ftzero   { ftst(@_, "-z") }
sub pp_ftsize   { ftst(@_, "-s") }
sub pp_ftmtime  { ftst(@_, "-M") }
sub pp_ftatime  { ftst(@_, "-A") }
sub pp_ftctime  { ftst(@_, "-C") }
sub pp_ftsock   { ftst(@_, "-S") }
sub pp_ftchr    { ftst(@_, "-c") }
sub pp_ftblk    { ftst(@_, "-b") }
sub pp_ftfile   { ftst(@_, "-f") }
sub pp_ftdir    { ftst(@_, "-d") }
sub pp_ftpipe   { ftst(@_, "-p") }
sub pp_ftlink   { ftst(@_, "-l") }
sub pp_ftsuid   { ftst(@_, "-u") }
sub pp_ftsgid   { ftst(@_, "-g") }
sub pp_ftsvtx   { ftst(@_, "-k") }
sub pp_fttty    { ftst(@_, "-t") }
sub pp_fttext   { ftst(@_, "-T") }
sub pp_ftbinary { ftst(@_, "-B") }

sub SWAP_CHILDREN () { 1 }
sub ASSIGN () { 2 } # has OP= variant
sub LIST_CONTEXT () { 4 } # Assignment is in list context

my(%left, %right);

sub assoc_class {
    my $op = shift;
    my $name = $op->name;
    if ($name eq "concat" and $op->first->name eq "concat") {
	# avoid spurious '=' -- see comment in pp_concat
	return "concat";
    }
    if ($name eq "null" and class($op) eq "UNOP"
	and $op->first->name =~ /^(and|x?or)$/
	and null $op->first->sibling)
    {
	# Like all conditional constructs, OP_ANDs and OP_ORs are topped
	# with a null that's used as the common end point of the two
	# flows of control. For precedence purposes, ignore it.
	# (COND_EXPRs have these too, but we don't bother with
	# their associativity).
	return assoc_class($op->first);
    }
    return $name . ($op->flags & OPf_STACKED ? "=" : "");
}

# Left associative operators, like '+', for which
# $a + $b + $c is equivalent to ($a + $b) + $c

BEGIN {
    %left = ('multiply' => 19, 'i_multiply' => 19,
	     'divide' => 19, 'i_divide' => 19,
	     'modulo' => 19, 'i_modulo' => 19,
	     'repeat' => 19,
	     'add' => 18, 'i_add' => 18,
	     'subtract' => 18, 'i_subtract' => 18,
	     'concat' => 18,
	     'left_shift' => 17, 'right_shift' => 17,
	     'bit_and' => 13, 'nbit_and' => 13, 'sbit_and' => 13,
	     'bit_or' => 12, 'bit_xor' => 12,
	     'sbit_or' => 12, 'sbit_xor' => 12,
	     'nbit_or' => 12, 'nbit_xor' => 12,
	     'and' => 3,
	     'or' => 2, 'xor' => 2,
	    );
}

sub deparse_binop_left {
    my $self = shift;
    my($op, $left, $prec) = @_;
    if ($left{assoc_class($op)} && $left{assoc_class($left)}
	and $left{assoc_class($op)} == $left{assoc_class($left)})
    {
	return $self->deparse($left, $prec - .00001);
    } else {
	return $self->deparse($left, $prec);	
    }
}

# Right associative operators, like '=', for which
# $a = $b = $c is equivalent to $a = ($b = $c)

BEGIN {
    %right = ('pow' => 22,
	      'sassign=' => 7, 'aassign=' => 7,
	      'multiply=' => 7, 'i_multiply=' => 7,
	      'divide=' => 7, 'i_divide=' => 7,
	      'modulo=' => 7, 'i_modulo=' => 7,
	      'repeat=' => 7, 'refassign' => 7, 'refassign=' => 7,
	      'add=' => 7, 'i_add=' => 7,
	      'subtract=' => 7, 'i_subtract=' => 7,
	      'concat=' => 7,
	      'left_shift=' => 7, 'right_shift=' => 7,
	      'bit_and=' => 7, 'sbit_and=' => 7, 'nbit_and=' => 7,
	      'nbit_or=' => 7, 'nbit_xor=' => 7,
	      'sbit_or=' => 7, 'sbit_xor=' => 7,
	      'andassign' => 7,
	      'orassign' => 7,
	     );
}

sub deparse_binop_right {
    my $self = shift;
    my($op, $right, $prec) = @_;
    if ($right{assoc_class($op)} && $right{assoc_class($right)}
	and $right{assoc_class($op)} == $right{assoc_class($right)})
    {
	return $self->deparse($right, $prec - .00001);
    } else {
	return $self->deparse($right, $prec);	
    }
}

sub binop {
    my $self = shift;
    my ($op, $cx, $opname, $prec, $flags) = (@_, 0);
    my $left = $op->first;
    my $right = $op->last;
    my $eq = "";
    if ($op->flags & OPf_STACKED && $flags & ASSIGN) {
	$eq = "=";
	$prec = 7;
    }
    if ($flags & SWAP_CHILDREN) {
	($left, $right) = ($right, $left);
    }
    my $leftop = $left;
    $left = $self->deparse_binop_left($op, $left, $prec);
    $left = "($left)" if $flags & LIST_CONTEXT
		     and    $left !~ /^(my|our|local|state|)\s*[\@%\(]/
			 || do {
				# Parenthesize if the left argument is a
				# lone repeat op.
				my $left = $leftop->first->sibling;
				$left->name eq 'repeat'
				    && null($left->sibling);
			    };
    $right = $self->deparse_binop_right($op, $right, $prec);
    return $self->maybe_parens("$left $opname$eq $right", $cx, $prec);
}

sub pp_add { maybe_targmy(@_, \&binop, "+", 18, ASSIGN) }
sub pp_multiply { maybe_targmy(@_, \&binop, "*", 19, ASSIGN) }
sub pp_subtract { maybe_targmy(@_, \&binop, "-",18,  ASSIGN) }
sub pp_divide { maybe_targmy(@_, \&binop, "/", 19, ASSIGN) }
sub pp_modulo { maybe_targmy(@_, \&binop, "%", 19, ASSIGN) }
sub pp_i_add { maybe_targmy(@_, \&binop, "+", 18, ASSIGN) }
sub pp_i_multiply { maybe_targmy(@_, \&binop, "*", 19, ASSIGN) }
sub pp_i_subtract { maybe_targmy(@_, \&binop, "-", 18, ASSIGN) }
sub pp_i_divide { maybe_targmy(@_, \&binop, "/", 19, ASSIGN) }
sub pp_i_modulo { maybe_targmy(@_, \&binop, "%", 19, ASSIGN) }
sub pp_pow { maybe_targmy(@_, \&binop, "**", 22, ASSIGN) }

sub pp_left_shift { maybe_targmy(@_, \&binop, "<<", 17, ASSIGN) }
sub pp_right_shift { maybe_targmy(@_, \&binop, ">>", 17, ASSIGN) }
sub pp_bit_and { maybe_targmy(@_, \&binop, "&", 13, ASSIGN) }
sub pp_bit_or { maybe_targmy(@_, \&binop, "|", 12, ASSIGN) }
sub pp_bit_xor { maybe_targmy(@_, \&binop, "^", 12, ASSIGN) }
*pp_nbit_and = *pp_bit_and;
*pp_nbit_or  = *pp_bit_or;
*pp_nbit_xor = *pp_bit_xor;
sub pp_sbit_and { maybe_targmy(@_, \&binop, "&.", 13, ASSIGN) }
sub pp_sbit_or { maybe_targmy(@_, \&binop, "|.", 12, ASSIGN) }
sub pp_sbit_xor { maybe_targmy(@_, \&binop, "^.", 12, ASSIGN) }

sub pp_eq { binop(@_, "==", 14) }
sub pp_ne { binop(@_, "!=", 14) }
sub pp_lt { binop(@_, "<", 15) }
sub pp_gt { binop(@_, ">", 15) }
sub pp_ge { binop(@_, ">=", 15) }
sub pp_le { binop(@_, "<=", 15) }
sub pp_ncmp { binop(@_, "<=>", 14) }
sub pp_i_eq { binop(@_, "==", 14) }
sub pp_i_ne { binop(@_, "!=", 14) }
sub pp_i_lt { binop(@_, "<", 15) }
sub pp_i_gt { binop(@_, ">", 15) }
sub pp_i_ge { binop(@_, ">=", 15) }
sub pp_i_le { binop(@_, "<=", 15) }
sub pp_i_ncmp { maybe_targmy(@_, \&binop, "<=>", 14) }

sub pp_seq { binop(@_, "eq", 14) }
sub pp_sne { binop(@_, "ne", 14) }
sub pp_slt { binop(@_, "lt", 15) }
sub pp_sgt { binop(@_, "gt", 15) }
sub pp_sge { binop(@_, "ge", 15) }
sub pp_sle { binop(@_, "le", 15) }
sub pp_scmp { maybe_targmy(@_, \&binop, "cmp", 14) }

sub pp_isa { binop(@_, "isa", 15) }

sub pp_sassign { binop(@_, "=", 7, SWAP_CHILDREN) }
sub pp_aassign { binop(@_, "=", 7, SWAP_CHILDREN | LIST_CONTEXT) }

sub pp_padsv_store {
    my $self = shift;
    my ($op, $cx, $forbid_parens, @args) = @_;
    my $targ = $op->targ;
    my $var = $self->maybe_my($op, $cx, $self->padname($targ),
                           $self->padname_sv($targ),
                           $forbid_parens);

    my $val = $self->deparse($op->first, 7);
    return $self->maybe_parens("$var = $val", $cx, 7);
}

sub pp_smartmatch {
    my ($self, $op, $cx) = @_;
    if (($op->flags & OPf_SPECIAL) && $self->{expand} < 2) {
	return $self->deparse($op->last, $cx);
    }
    else {
	binop(@_, "~~", 14);
    }
}

# '.' is special because concats-of-concats are optimized to save copying
# by making all but the first concat stacked. The effect is as if the
# programmer had written '($a . $b) .= $c', except legal.
sub pp_concat { maybe_targmy(@_, \&real_concat) }
sub real_concat {
    my $self = shift;
    my($op, $cx) = @_;
    my $left = $op->first;
    my $right = $op->last;
    my $eq = "";
    my $prec = 18;
    if (($op->flags & OPf_STACKED) and !($op->private & OPpCONCAT_NESTED)) {
        # '.=' rather than optimised '.'
	$eq = "=";
	$prec = 7;
    }
    $left = $self->deparse_binop_left($op, $left, $prec);
    $right = $self->deparse_binop_right($op, $right, $prec);
    return $self->maybe_parens("$left .$eq $right", $cx, $prec);
}

sub pp_repeat { maybe_targmy(@_, \&repeat) }

# 'x' is weird when the left arg is a list
sub repeat {
    my $self = shift;
    my($op, $cx) = @_;
    my $left = $op->first;
    my $right = $op->last;
    my $eq = "";
    my $prec = 19;
    if ($op->flags & OPf_STACKED) {
	$eq = "=";
	$prec = 7;
    }
    if (null($right)) { # list repeat; count is inside left-side ex-list
			# in 5.21.5 and earlier
	my $kid = $left->first->sibling; # skip pushmark
	my @exprs;
	for (; !null($kid->sibling); $kid = $kid->sibling) {
	    push @exprs, $self->deparse($kid, 6);
	}
	$right = $kid;
	$left = "(" . join(", ", @exprs). ")";
    } else {
	my $dolist = $op->private & OPpREPEAT_DOLIST;
	$left = $self->deparse_binop_left($op, $left, $dolist ? 1 : $prec);
	if ($dolist) {
	    $left = "($left)";
	}
    }
    $right = $self->deparse_binop_right($op, $right, $prec);
    return $self->maybe_parens("$left x$eq $right", $cx, $prec);
}

sub range {
    my $self = shift;
    my ($op, $cx, $type) = @_;
    my $left = $op->first;
    my $right = $left->sibling;
    $left = $self->deparse($left, 9);
    $right = $self->deparse($right, 9);
    return $self->maybe_parens("$left $type $right", $cx, 9);
}

sub pp_flop {
    my $self = shift;
    my($op, $cx) = @_;
    my $flip = $op->first;
    my $type = ($flip->flags & OPf_SPECIAL) ? "..." : "..";
    return $self->range($flip->first, $cx, $type);
}

# one-line while/until is handled in pp_leave

sub logop {
    my $self = shift;
    my ($op, $cx, $lowop, $lowprec, $highop, $highprec, $blockname) = @_;
    my $left = $op->first;
    my $right = $op->first->sibling;
    $blockname &&= $self->keyword($blockname);
    if ($cx < 1 and is_scope($right) and $blockname
	and $self->{'expand'} < 7)
    { # if ($a) {$b}
	$left = $self->deparse($left, 1);
	$right = $self->deparse($right, 0);
	return "$blockname ($left) {\n\t$right\n\b}\cK";
    } elsif ($cx < 1 and $blockname and not $self->{'parens'}
	     and $self->{'expand'} < 7) { # $b if $a
	$right = $self->deparse($right, 1);
	$left = $self->deparse($left, 1);
	return "$right $blockname $left";
    } elsif ($cx > $lowprec and $highop) { # $a && $b
	$left = $self->deparse_binop_left($op, $left, $highprec);
	$right = $self->deparse_binop_right($op, $right, $highprec);
	return $self->maybe_parens("$left $highop $right", $cx, $highprec);
    } else { # $a and $b
	$left = $self->deparse_binop_left($op, $left, $lowprec);
	$right = $self->deparse_binop_right($op, $right, $lowprec);
	return $self->maybe_parens("$left $lowop $right", $cx, $lowprec);
    }
}

sub pp_and { logop(@_, "and", 3, "&&", 11, "if") }
sub pp_or  { logop(@_, "or",  2, "||", 10, "unless") }
sub pp_dor { logop(@_, "//", 10) }

# xor is syntactically a logop, but it's really a binop (contrary to
# old versions of opcode.pl). Syntax is what matters here.
sub pp_xor { logop(@_, "xor", 2, "",   0,  "") }

sub logassignop {
    my $self = shift;
    my ($op, $cx, $opname) = @_;
    my $left = $op->first;
    my $right = $op->first->sibling->first; # skip sassign
    $left = $self->deparse($left, 7);
    $right = $self->deparse($right, 7);
    return $self->maybe_parens("$left $opname $right", $cx, 7);
}

sub pp_andassign { logassignop(@_, "&&=") }
sub pp_orassign  { logassignop(@_, "||=") }
sub pp_dorassign { logassignop(@_, "//=") }

my %cmpchain_cmpops = (
	eq => ["==", 14],
	i_eq => ["==", 14],
	ne => ["!=", 14],
	i_ne => ["!=", 14],
	seq => ["eq", 14],
	sne => ["ne", 14],
	lt => ["<", 15],
	i_lt => ["<", 15],
	gt => [">", 15],
	i_gt => [">", 15],
	le => ["<=", 15],
	i_le => ["<=", 15],
	ge => [">=", 15],
	i_ge => [">=", 15],
	slt => ["lt", 15],
	sgt => ["gt", 15],
	sle => ["le", 15],
	sge => ["ge", 15],
);
sub pp_cmpchain_and {
    my($self, $op, $cx) = @_;
    my($prec, $dep);
    while(1) {
	my($thiscmp, $rightcond);
	if($op->name eq "cmpchain_and") {
	    $thiscmp = $op->first;
	    $rightcond = $thiscmp->sibling;
	} else {
	    $thiscmp = $op;
	}
	my $thiscmptype = $cmpchain_cmpops{$thiscmp->name} // (return "XXX");
	if(defined $prec) {
	    $thiscmptype->[1] == $prec or return "XXX";
	    $thiscmp->first->name eq "null" &&
		    !($thiscmp->first->flags & OPf_KIDS)
		or return "XXX";
	} else {
	    $prec = $thiscmptype->[1];
	    $dep = $self->deparse($thiscmp->first, $prec);
	}
	$dep .= " ".$thiscmptype->[0]." ";
	my $operand = $thiscmp->last;
	if(defined $rightcond) {
	    $operand->name eq "cmpchain_dup" or return "XXX";
	    $operand = $operand->first;
	}
	$dep .= $self->deparse($operand, $prec);
	last unless defined $rightcond;
	if($rightcond->name eq "null" && ($rightcond->flags & OPf_KIDS) &&
		$rightcond->first->name eq "cmpchain_and") {
	    $rightcond = $rightcond->first;
	}
	$op = $rightcond;
    }
    return $self->maybe_parens($dep, $cx, $prec);
}

sub rv2gv_or_string {
    my($self,$op) = @_;
    if ($op->name eq "gv") { # could be open("open") or open("###")
	my($name,$quoted) =
	    $self->stash_variable_name("", $self->gv_or_padgv($op));
	$quoted ? $name : "*$name";
    }
    else {
	$self->deparse($op, 6);
    }
}

sub listop {
    my $self = shift;
    my($op, $cx, $name, $kid, $nollafr) = @_;
    my(@exprs);
    my $parens = ($cx >= 5) || $self->{'parens'};
    $kid ||= $op->first->sibling;
    # If there are no arguments, add final parentheses (or parenthesize the
    # whole thing if the llafr does not apply) to account for cases like
    # (return)+1 or setpgrp()+1.  When the llafr does not apply, we use a
    # precedence of 6 (< comma), as "return, 1" does not need parentheses.
    if (null $kid) {
	return $nollafr
		? $self->maybe_parens($self->keyword($name), $cx, 7)
		: $self->keyword($name) . '()' x (7 < $cx);
    }
    my $first;
    my $fullname = $self->keyword($name);
    my $proto = prototype("CORE::$name");
    if (
	 (     (defined $proto && $proto =~ /^;?\*/)
	    || $name eq 'select' # select(F) doesn't have a proto
	 )
	 && $kid->name eq "rv2gv"
	 && !($kid->private & OPpLVAL_INTRO)
    ) {
	$first = $self->rv2gv_or_string($kid->first);
    }
    else {
	$first = $self->deparse($kid, 6);
    }
    if ($name eq "chmod" && $first =~ /^\d+$/) {
	$first = sprintf("%#o", $first);
    }
    $first = "+$first"
	if not $parens and not $nollafr and substr($first, 0, 1) eq "(";
    push @exprs, $first;
    $kid = $kid->sibling;
    if (defined $proto && $proto =~ /^\*\*/ && $kid->name eq "rv2gv"
	 && !($kid->private & OPpLVAL_INTRO)) {
	push @exprs, $first = $self->rv2gv_or_string($kid->first);
	$kid = $kid->sibling;
    }
    for (; !null($kid); $kid = $kid->sibling) {
	push @exprs, $self->deparse($kid, 6);
    }
    if ($name eq "reverse" && ($op->private & OPpREVERSE_INPLACE)) {
	return "$exprs[0] = $fullname"
	         . ($parens ? "($exprs[0])" : " $exprs[0]");
    }

    if ($parens && $nollafr) {
	return "($fullname " . join(", ", @exprs) . ")";
    } elsif ($parens) {
	return "$fullname(" . join(", ", @exprs) . ")";
    } else {
	return "$fullname " . join(", ", @exprs);
    }
}

sub pp_bless { listop(@_, "bless") }
sub pp_atan2 { maybe_targmy(@_, \&listop, "atan2") }
sub pp_substr {
    my ($self,$op,$cx) = @_;
    if ($op->private & OPpSUBSTR_REPL_FIRST) {
	return
	   listop($self, $op, 7, "substr", $op->first->sibling->sibling)
	 . " = "
	 . $self->deparse($op->first->sibling, 7);
    }
    maybe_local(@_, listop(@_, "substr"))
}

sub pp_index {
    # Also handles pp_rindex.
    #
    # The body of this function includes an unrolled maybe_targmy(),
    # since the two parts of that sub's actions need to have have the
    # '== -1' bit in between

    my($self, $op, $cx) = @_;

    my $lex  = ($op->private & OPpTARGET_MY);
    my $bool = ($op->private & OPpTRUEBOOL);

    my $val = $self->listop($op, ($bool ? 14 : $lex ? 7 : $cx), $op->name);

    # (index() == -1) has op_eq and op_const optimised away
    if ($bool) {
        $val .= ($op->private & OPpINDEX_BOOLNEG) ? " == -1" : " != -1";
        $val = "($val)" if ($op->flags & OPf_PARENS);
    }
    if ($lex) {
	my $var = $self->padname($op->targ);
	$val = $self->maybe_parens("$var = $val", $cx, 7);
    }
    $val;
}

sub pp_rindex { pp_index(@_); }
sub pp_vec { maybe_targmy(@_, \&maybe_local, listop(@_, "vec")) }
sub pp_sprintf { maybe_targmy(@_, \&listop, "sprintf") }
sub pp_formline { listop(@_, "formline") } # see also deparse_format
sub pp_crypt { maybe_targmy(@_, \&listop, "crypt") }
sub pp_unpack { listop(@_, "unpack") }
sub pp_pack { listop(@_, "pack") }
sub pp_join { maybe_targmy(@_, \&listop, "join") }
sub pp_splice { listop(@_, "splice") }
sub pp_push { maybe_targmy(@_, \&listop, "push") }
sub pp_unshift { maybe_targmy(@_, \&listop, "unshift") }
sub pp_reverse { listop(@_, "reverse") }
sub pp_warn { listop(@_, "warn") }
sub pp_die { listop(@_, "die") }
sub pp_return { listop(@_, "return", undef, 1) } # llafr does not apply
sub pp_open { listop(@_, "open") }
sub pp_pipe_op { listop(@_, "pipe") }
sub pp_tie { listop(@_, "tie") }
sub pp_binmode { listop(@_, "binmode") }
sub pp_dbmopen { listop(@_, "dbmopen") }
sub pp_sselect { listop(@_, "select") }
sub pp_select { listop(@_, "select") }
sub pp_read { listop(@_, "read") }
sub pp_sysopen { listop(@_, "sysopen") }
sub pp_sysseek { listop(@_, "sysseek") }
sub pp_sysread { listop(@_, "sysread") }
sub pp_syswrite { listop(@_, "syswrite") }
sub pp_send { listop(@_, "send") }
sub pp_recv { listop(@_, "recv") }
sub pp_seek { listop(@_, "seek") }
sub pp_fcntl { listop(@_, "fcntl") }
sub pp_ioctl { listop(@_, "ioctl") }
sub pp_flock { maybe_targmy(@_, \&listop, "flock") }
sub pp_socket { listop(@_, "socket") }
sub pp_sockpair { listop(@_, "socketpair") }
sub pp_bind { listop(@_, "bind") }
sub pp_connect { listop(@_, "connect") }
sub pp_listen { listop(@_, "listen") }
sub pp_accept { listop(@_, "accept") }
sub pp_shutdown { listop(@_, "shutdown") }
sub pp_gsockopt { listop(@_, "getsockopt") }
sub pp_ssockopt { listop(@_, "setsockopt") }
sub pp_chown { maybe_targmy(@_, \&listop, "chown") }
sub pp_unlink { maybe_targmy(@_, \&listop, "unlink") }
sub pp_chmod { maybe_targmy(@_, \&listop, "chmod") }
sub pp_utime { maybe_targmy(@_, \&listop, "utime") }
sub pp_rename { maybe_targmy(@_, \&listop, "rename") }
sub pp_link { maybe_targmy(@_, \&listop, "link") }
sub pp_symlink { maybe_targmy(@_, \&listop, "symlink") }
sub pp_mkdir { maybe_targmy(@_, \&listop, "mkdir") }
sub pp_open_dir { listop(@_, "opendir") }
sub pp_seekdir { listop(@_, "seekdir") }
sub pp_waitpid { maybe_targmy(@_, \&listop, "waitpid") }
sub pp_system { maybe_targmy(@_, \&indirop, "system") }
sub pp_exec { maybe_targmy(@_, \&indirop, "exec") }
sub pp_kill { maybe_targmy(@_, \&listop, "kill") }
sub pp_setpgrp { maybe_targmy(@_, \&listop, "setpgrp") }
sub pp_getpriority { maybe_targmy(@_, \&listop, "getpriority") }
sub pp_setpriority { maybe_targmy(@_, \&listop, "setpriority") }
sub pp_shmget { listop(@_, "shmget") }
sub pp_shmctl { listop(@_, "shmctl") }
sub pp_shmread { listop(@_, "shmread") }
sub pp_shmwrite { listop(@_, "shmwrite") }
sub pp_msgget { listop(@_, "msgget") }
sub pp_msgctl { listop(@_, "msgctl") }
sub pp_msgsnd { listop(@_, "msgsnd") }
sub pp_msgrcv { listop(@_, "msgrcv") }
sub pp_semget { listop(@_, "semget") }
sub pp_semctl { listop(@_, "semctl") }
sub pp_semop { listop(@_, "semop") }
sub pp_ghbyaddr { listop(@_, "gethostbyaddr") }
sub pp_gnbyaddr { listop(@_, "getnetbyaddr") }
sub pp_gpbynumber { listop(@_, "getprotobynumber") }
sub pp_gsbyname { listop(@_, "getservbyname") }
sub pp_gsbyport { listop(@_, "getservbyport") }
sub pp_syscall { listop(@_, "syscall") }

sub pp_glob {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first->sibling;  # skip pushmark
    my $keyword =
	$op->flags & OPf_SPECIAL ? 'glob' : $self->keyword('glob');
    my $text = $self->deparse($kid, $cx);
    return $cx >= 5 || $self->{'parens'}
	? "$keyword($text)"
	: "$keyword $text";
}

# Truncate is special because OPf_SPECIAL makes a bareword first arg
# be a filehandle. This could probably be better fixed in the core
# by moving the GV lookup into ck_truc.

sub pp_truncate {
    my $self = shift;
    my($op, $cx) = @_;
    my(@exprs);
    my $parens = ($cx >= 5) || $self->{'parens'};
    my $kid = $op->first->sibling;
    my $fh;
    if ($op->flags & OPf_SPECIAL) {
	# $kid is an OP_CONST
	$fh = $self->const_sv($kid)->PV;
    } else {
	$fh = $self->deparse($kid, 6);
        $fh = "+$fh" if not $parens and substr($fh, 0, 1) eq "(";
    }
    my $len = $self->deparse($kid->sibling, 6);
    my $name = $self->keyword('truncate');
    if ($parens) {
	return "$name($fh, $len)";
    } else {
	return "$name $fh, $len";
    }
}

sub indirop {
    my $self = shift;
    my($op, $cx, $name) = @_;
    my($expr, @exprs);
    my $firstkid = my $kid = $op->first->sibling;
    my $indir = "";
    if ($op->flags & OPf_STACKED) {
	$indir = $kid;
	$indir = $indir->first; # skip rv2gv
	if (is_scope($indir)) {
	    $indir = "{" . $self->deparse($indir, 0) . "}";
	    $indir = "{;}" if $indir eq "{}";
	} elsif ($indir->name eq "const" && $indir->private & OPpCONST_BARE) {
	    $indir = $self->const_sv($indir)->PV;
	} else {
	    $indir = $self->deparse($indir, 24);
	}
	$indir = $indir . " ";
	$kid = $kid->sibling;
    }
    if ($name eq "sort" && $op->private & (OPpSORT_NUMERIC | OPpSORT_INTEGER)) {
	$indir = ($op->private & OPpSORT_DESCEND) ? '{$b <=> $a} '
						  : '{$a <=> $b} ';
    }
    elsif ($name eq "sort" && $op->private & OPpSORT_DESCEND) {
	$indir = '{$b cmp $a} ';
    }
    for (; !null($kid); $kid = $kid->sibling) {
	$expr = $self->deparse($kid, !$indir && $kid == $firstkid && $name eq "sort" && $firstkid->name eq "entersub" ? 16 : 6);
	push @exprs, $expr;
    }
    my $name2;
    if ($name eq "sort" && $op->private & OPpSORT_REVERSE) {
	$name2 = $self->keyword('reverse') . ' ' . $self->keyword('sort');
    }
    else { $name2 = $self->keyword($name) }
    if ($name eq "sort" && ($op->private & OPpSORT_INPLACE)) {
	return "$exprs[0] = $name2 $indir $exprs[0]";
    }

    my $args = $indir . join(", ", @exprs);
    if ($indir ne "" && $name eq "sort") {
	# We don't want to say "sort(f 1, 2, 3)", since perl -w will
	# give bareword warnings in that case. Therefore if context
	# requires, we'll put parens around the outside "(sort f 1, 2,
	# 3)". Unfortunately, we'll currently think the parens are
	# necessary more often that they really are, because we don't
	# distinguish which side of an assignment we're on.
	if ($cx >= 5) {
	    return "($name2 $args)";
	} else {
	    return "$name2 $args";
	}
    } elsif (
	!$indir && $name eq "sort"
      && !null($op->first->sibling)
      && $op->first->sibling->name eq 'entersub'
    ) {
	# We cannot say sort foo(bar), as foo will be interpreted as a
	# comparison routine.  We have to say sort(...) in that case.
	return "$name2($args)";
    } else {
	return length $args
		? $self->maybe_parens_func($name2, $args, $cx, 5)
		: $name2 . '()' x (7 < $cx);
    }

}

sub pp_prtf { indirop(@_, "printf") }
sub pp_print { indirop(@_, "print") }
sub pp_say  { indirop(@_, "say") }
sub pp_sort { indirop(@_, "sort") }

sub mapop {
    my $self = shift;
    my($op, $cx, $name) = @_;
    my($expr, @exprs);
    my $kid = $op->first; # this is the (map|grep)start
    $kid = $kid->first->sibling; # skip a pushmark
    my $code = $kid->first; # skip a null
    if (is_scope $code) {
	$code = "{" . $self->deparse($code, 0) . "} ";
    } else {
	$code = $self->deparse($code, 24);
	$code .= ", " if !null($kid->sibling);
    }
    $kid = $kid->sibling;
    for (; !null($kid); $kid = $kid->sibling) {
	$expr = $self->deparse($kid, 6);
	push @exprs, $expr if defined $expr;
    }
    return $self->maybe_parens_func($self->keyword($name),
				    $code . join(", ", @exprs), $cx, 5);
}

sub pp_mapwhile { mapop(@_, "map") }
sub pp_grepwhile { mapop(@_, "grep") }
sub pp_mapstart { baseop(@_, "map") }
sub pp_grepstart { baseop(@_, "grep") }

my %uses_intro;
BEGIN {
    @uses_intro{
	eval { require B::Op_private }
	  ? @{$B::Op_private::ops_using{OPpLVAL_INTRO}}
	  : qw(gvsv rv2sv rv2hv rv2gv rv2av aelem helem aslice
	       hslice delete padsv padav padhv enteriter entersub padrange
	       pushmark cond_expr refassign list)
    } = ();
    delete @uses_intro{qw( lvref lvrefslice lvavref entersub )};
}


# Look for a my/state attribute declaration in a list or ex-list.
# Returns undef if not found, 'my($x, @a) :Foo(bar)' etc otherwise.
#
# There are three basic tree structs that are expected:
#
# my $x :foo;
#      <1> ex-list vK/LVINTRO ->c
#         <0> ex-pushmark v ->3
#         <1> entersub[t2] vKRS*/TARG ->b
#                ....
#         <0> padsv[$x:64,65] vM/LVINTRO ->c
#
# my @a :foo;
# my %h :foo;
#
#      <1> ex-list vK ->c
#         <0> ex-pushmark v ->3
#         <0> padav[@a:64,65] vM/LVINTRO ->4
#         <1> entersub[t2] vKRS*/TARG ->c
#            ....
#
# my ($x,@a,%h) :foo;
#
#      <;> nextstate(main 64 -e:1) v:{ ->3
#      <@> list vKP ->w
#         <0> pushmark vM/LVINTRO ->4
#         <0> padsv[$x:64,65] vM/LVINTRO ->5
#         <0> padav[@a:64,65] vM/LVINTRO ->6
#         <0> padhv[%h:64,65] vM/LVINTRO ->7
#         <1> entersub[t4] vKRS*/TARG ->f
#            ....
#         <1> entersub[t5] vKRS*/TARG ->n
#            ....
#         <1> entersub[t6] vKRS*/TARG ->v
#           ....
# where the entersub in all cases looks like
#        <1> entersub[t2] vKRS*/TARG ->c
#           <0> pushmark s ->5
#           <$> const[PV "attributes"] sM ->6
#           <$> const[PV "main"] sM ->7
#           <1> srefgen sKM/1 ->9
#              <1> ex-list lKRM ->8
#                 <0> padsv[@a:64,65] sRM ->8
#           <$> const[PV "foo"] sM ->a
#           <.> method_named[PV "import"] ->b

sub maybe_var_attr {
    my ($self, $op, $cx) = @_;

    my $kid = $op->first->sibling; # skip pushmark
    return if class($kid) eq 'NULL';

    my $lop;
    my $type;

    # Extract out all the pad ops and entersub ops into
    # @padops and @entersubops. Return if anything else seen.
    # Also determine what class (if any) all the pad vars belong to
    my $class;
    my $decl; # 'my' or 'state'
    my (@padops, @entersubops);
    for ($lop = $kid; !null($lop); $lop = $lop->sibling) {
	my $lopname = $lop->name;
	my $loppriv = $lop->private;
        if ($lopname =~ /^pad[sah]v$/) {
            return unless $loppriv & OPpLVAL_INTRO;

            my $padname = $self->padname_sv($lop->targ);
            my $thisclass = ($padname->FLAGS & PADNAMEf_TYPED)
                                ? $padname->SvSTASH->NAME : 'main';

            # all pad vars must be in the same class
            $class //= $thisclass;
            return unless $thisclass eq $class;

            # all pad vars must be the same sort of declaration
            # (all my, all state, etc)
            my $this = ($loppriv & OPpPAD_STATE) ? 'state' : 'my';
            if (defined $decl) {
                return unless $this eq $decl;
            }
            $decl = $this;

            push @padops, $lop;
        }
        elsif ($lopname eq 'entersub') {
            push @entersubops, $lop;
        }
        else {
            return;
        }
    }

    return unless @padops && @padops == @entersubops;

    # there should be a balance: each padop has a corresponding
    # 'attributes'->import() method call, in the same order.

    my @varnames;
    my $attr_text;

    for my $i (0..$#padops) {
        my $padop = $padops[$i];
        my $esop  = $entersubops[$i];

        push @varnames, $self->padname($padop->targ);

        return unless ($esop->flags & OPf_KIDS);

        my $kid = $esop->first;
        return unless $kid->type == OP_PUSHMARK;

        $kid = $kid->sibling;
        return unless $$kid && $kid->type == OP_CONST;
	return unless $self->const_sv($kid)->PV eq 'attributes';

        $kid = $kid->sibling;
        return unless $$kid && $kid->type == OP_CONST; # __PACKAGE__

        $kid = $kid->sibling;
        return unless  $$kid
                    && $kid->name eq "srefgen"
                    && ($kid->flags & OPf_KIDS)
                    && ($kid->first->flags & OPf_KIDS)
                    && $kid->first->first->name =~ /^pad[sah]v$/
                    && $kid->first->first->targ == $padop->targ;

        $kid = $kid->sibling;
        my @attr;
        while ($$kid) {
            last if ($kid->type != OP_CONST);
            push @attr, $self->const_sv($kid)->PV;
            $kid = $kid->sibling;
        }
        return unless @attr;
        my $thisattr = ":" . join(' ', @attr);
        $attr_text //= $thisattr;
        # all import calls must have the same list of attributes
        return unless $attr_text eq $thisattr;

        return unless $kid->name eq 'method_named';
	return unless $self->meth_sv($kid)->PV eq 'import';

        $kid = $kid->sibling;
        return if $$kid;
    }

    my $res = $decl;
    $res .= " $class " if $class ne 'main';
    $res .=
            (@varnames > 1)
            ? "(" . join(', ', @varnames) . ')'
            : " $varnames[0]";

    return "$res $attr_text";
}


sub pp_list {
    my $self = shift;
    my($op, $cx) = @_;

    {
        # might be my ($s,@a,%h) :Foo(bar);
        my $my_attr = maybe_var_attr($self, $op, $cx);
        return $my_attr if defined $my_attr;
    }

    my($expr, @exprs);
    my $kid = $op->first->sibling; # skip pushmark
    return '' if class($kid) eq 'NULL';
    my $lop;
    my $local = "either"; # could be local(...), my(...), state(...) or our(...)
    my $type;
    for ($lop = $kid; !null($lop); $lop = $lop->sibling) {
	my $lopname = $lop->name;
	my $loppriv = $lop->private;
	my $newtype;
	if ($lopname =~ /^pad[ash]v$/ && $loppriv & OPpLVAL_INTRO) {
	    if ($loppriv & OPpPAD_STATE) { # state()
		($local = "", last) if $local !~ /^(?:either|state)$/;
		$local = "state";
	    } else { # my()
		($local = "", last) if $local !~ /^(?:either|my)$/;
		$local = "my";
	    }
	    my $padname = $self->padname_sv($lop->targ);
	    if ($padname->FLAGS & PADNAMEf_TYPED) {
		$newtype = $padname->SvSTASH->NAME;
	    }
	} elsif ($lopname eq 'padsv_store') {
            # don't interpret as my (list) if it has an implicit assign
            $local = "";
	} elsif ($lopname =~ /^(?:gv|rv2)([ash])v$/
			&& $loppriv & OPpOUR_INTRO
		or $lopname eq "null" && class($lop) eq 'UNOP'
			&& $lop->first->name eq "gvsv"
			&& $lop->first->private & OPpOUR_INTRO) { # our()
	    my $newlocal = "local " x !!($loppriv & OPpLVAL_INTRO) . "our";
	    ($local = "", last)
		if $local ne 'either' && $local ne $newlocal;
	    $local = $newlocal;
	    my $funny = !$1 || $1 eq 's' ? '$' : $1 eq 'a' ? '@' : '%';
	    if (my $t = $self->find_our_type(
		    $funny . $self->gv_or_padgv($lop->first)->NAME
	       )) {
		$newtype = $t;
	    }
	} elsif ($lopname ne 'undef'
	   and    !($loppriv & OPpLVAL_INTRO)
	       || !exists $uses_intro{$lopname eq 'null'
					? substr B::ppname($lop->targ), 3
					: $lopname})
	{
	    $local = ""; # or not
	    last;
	} elsif ($lopname ne "undef")
	{
	    # local()
	    ($local = "", last) if $local !~ /^(?:either|local)$/;
	    $local = "local";
	}
	if (defined $type && defined $newtype && $newtype ne $type) {
	    $local = '';
	    last;
	}
	$type = $newtype;
    }
    $local = "" if $local eq "either"; # no point if it's all undefs
    $local &&= join ' ', map $self->keyword($_), split / /, $local;
    $local .= " $type " if $local && length $type;
    return $self->deparse($kid, $cx) if null $kid->sibling and not $local;
    for (; !null($kid); $kid = $kid->sibling) {
	if ($local) {
	    if (class($kid) eq "UNOP" and $kid->first->name eq "gvsv") {
		$lop = $kid->first;
	    } else {
		$lop = $kid;
	    }
	    $self->{'avoid_local'}{$$lop}++;
	    $expr = $self->deparse($kid, 6);
	    delete $self->{'avoid_local'}{$$lop};
	} else {
	    $expr = $self->deparse($kid, 6);
	}
	push @exprs, $expr;
    }
    if ($local) {
        if (@exprs == 1 && ($local eq 'state' || $local eq 'CORE::state')) {
            # 'state @a = ...' is legal, while 'state(@a) = ...' currently isn't
            return "$local $exprs[0]";
        }
	return "$local(" . join(", ", @exprs) . ")";
    } else {
	return $self->maybe_parens( join(", ", @exprs), $cx, 6);	
    }
}

sub is_ifelse_cont {
    my $op = shift;
    return ($op->name eq "null" and class($op) eq "UNOP"
	    and $op->first->name =~ /^(and|cond_expr)$/
	    and is_scope($op->first->first->sibling));
}

sub pp_cond_expr {
    my $self = shift;
    my($op, $cx) = @_;
    my $cond = $op->first;
    my $true = $cond->sibling;
    my $false = $true->sibling;
    my $cuddle = $self->{'cuddle'};
    unless ($cx < 1 and (is_scope($true) and $true->name ne "null") and
	    (is_scope($false) || is_ifelse_cont($false))
	    and $self->{'expand'} < 7) {
	$cond = $self->deparse($cond, 8);
	$true = $self->deparse($true, 6);
	$false = $self->deparse($false, 8);
	return $self->maybe_parens("$cond ? $true : $false", $cx, 8);
    }

    $cond = $self->deparse($cond, 1);
    $true = $self->deparse($true, 0);
    my $head = $self->keyword("if") . " ($cond) {\n\t$true\n\b}";
    my @elsifs;
    my $elsif;
    while (!null($false) and is_ifelse_cont($false)) {
	my $newop = $false->first;
	my $newcond = $newop->first;
	my $newtrue = $newcond->sibling;
	$false = $newtrue->sibling; # last in chain is OP_AND => no else
	if ($newcond->name eq "lineseq")
	{
	    # lineseq to ensure correct line numbers in elsif()
	    # Bug #37302 fixed by change #33710.
	    $newcond = $newcond->first->sibling;
	}
	$newcond = $self->deparse($newcond, 1);
	$newtrue = $self->deparse($newtrue, 0);
	$elsif ||= $self->keyword("elsif");
	push @elsifs, "$elsif ($newcond) {\n\t$newtrue\n\b}";
    }
    if (!null($false)) {
	$false = $cuddle . $self->keyword("else") . " {\n\t" .
	  $self->deparse($false, 0) . "\n\b}\cK";
    } else {
	$false = "\cK";
    }
    return $head . join($cuddle, "", @elsifs) . $false;
}

sub pp_once {
    my ($self, $op, $cx) = @_;
    my $cond = $op->first;
    my $true = $cond->sibling;

    my $ret = $self->deparse($true, $cx);
    $ret =~ s/^(\(?)\$/$1 . $self->keyword("state") . ' $'/e;
    $ret;
}

sub loop_common {
    my $self = shift;
    my($op, $cx, $init) = @_;
    my $enter = $op->first;
    my $kid = $enter->sibling;
    local(@$self{qw'curstash warnings hints hinthash'})
		= @$self{qw'curstash warnings hints hinthash'};
    my $head = "";
    my $bare = 0;
    my $body;
    my $cond = undef;
    my $name;
    if ($kid->name eq "lineseq") { # bare or infinite loop
	if ($kid->last->name eq "unstack") { # infinite
	    $head = "while (1) "; # Can't use for(;;) if there's a continue
	    $cond = "";
	} else {
	    $bare = 1;
	}
	$body = $kid;
    } elsif ($enter->name eq "enteriter") { # foreach
	my $ary = $enter->first->sibling; # first was pushmark
	my $var = $ary->sibling;
	if ($ary->name eq 'null' and $enter->private & OPpITER_REVERSED) {
	    # "reverse" was optimised away
	    $ary = listop($self, $ary->first->sibling, 1, 'reverse');
	} elsif ($enter->flags & OPf_STACKED
	    and not null $ary->first->sibling->sibling)
	{
	    $ary = $self->deparse($ary->first->sibling, 9) . " .. " .
	      $self->deparse($ary->first->sibling->sibling, 9);
	} else {
	    $ary = $self->deparse($ary, 1);
	}

        if ($enter->flags & OPf_PARENS) {
            # for my ($x, $y, ...) ...
            # for my ($foo, $bar) () stores the count (less 1) in the targ of
            # the ITER op. For the degenerate case of 1 var ($x), the
            # TARG is zero, so it works anyway
            my $iter_targ = $kid->first->first->targ;
            my @vars;
            my $targ = $enter->targ;
            while ($iter_targ-- >= 0) {
                push @vars, $self->padname_sv($targ)->PVX;
                ++$targ;
            }
            $var = 'my (' . join(', ', @vars) . ')';
        } elsif (null $var) {
            $var = $self->pp_padsv($enter, 1, 1);
	} elsif ($var->name eq "rv2gv") {
	    $var = $self->pp_rv2sv($var, 1);
	    if ($enter->private & OPpOUR_INTRO) {
		# our declarations don't have package names
		$var =~ s/^(.).*::/$1/;
		$var = "our $var";
	    }
	} elsif ($var->name eq "gv") {
	    $var = "\$" . $self->deparse($var, 1);
	} else {
	    $var = $self->deparse($var, 1);
	}
	$body = $kid->first->first->sibling; # skip OP_AND and OP_ITER
	if (!is_state $body->first and $body->first->name !~ /^(?:stub|leave|scope)$/) {
	    confess unless $var eq '$_';
	    $body = $body->first;
	    return $self->deparse($body, 2) . " "
		 . $self->keyword("foreach") . " ($ary)";
	}
	$head = "foreach $var ($ary) ";
    } elsif ($kid->name eq "null") { # while/until
	$kid = $kid->first;
	$name = {"and" => "while", "or" => "until"}->{$kid->name};
	$cond = $kid->first;
	$body = $kid->first->sibling;
    } elsif ($kid->name eq "stub") { # bare and empty
	return "{;}"; # {} could be a hashref
    }
    # If there isn't a continue block, then the next pointer for the loop
    # will point to the unstack, which is kid's last child, except
    # in a bare loop, when it will point to the leaveloop. When neither of
    # these conditions hold, then the second-to-last child is the continue
    # block (or the last in a bare loop).
    my $cont_start = $enter->nextop;
    my $cont;
    my $precond;
    my $postcond;
    if ($$cont_start != $$op && ${$cont_start} != ${$body->last}) {
	if ($bare) {
	    $cont = $body->last;
	} else {
	    $cont = $body->first;
	    while (!null($cont->sibling->sibling)) {
		$cont = $cont->sibling;
	    }
	}
	my $state = $body->first;
	my $cuddle = $self->{'cuddle'};
	my @states;
	for (; $$state != $$cont; $state = $state->sibling) {
	    push @states, $state;
	}
	$body = $self->lineseq(undef, 0, @states);
	if (defined $cond and not is_scope $cont and $self->{'expand'} < 3) {
	    $precond = "for ($init; ";
	    $postcond = "; " . $self->deparse($cont, 1) .") ";
	    $cont = "\cK";
	} else {
	    $cont = $cuddle . "continue {\n\t" .
	      $self->deparse($cont, 0) . "\n\b}\cK";
	}
    } else {
	return "" if !defined $body;
	if (length $init) {
	    $precond = "for ($init; ";
	    $postcond = ";) ";
	}
	$cont = "\cK";
	$body = $self->deparse($body, 0);
    }
    if ($precond) { # for(;;)
	$cond &&= $name eq 'until'
		    ? listop($self, undef, 1, "not", $cond->first)
		    : $self->deparse($cond, 1);
	$head = "$precond$cond$postcond";
    }
    if ($name && !$head) {
	ref $cond and $cond = $self->deparse($cond, 1);
	$head = "$name ($cond) ";
    }
    $head =~ s/^(for(?:each)?|while|until)/$self->keyword($1)/e;
    $body =~ s/;?$/;\n/;

    return $head . "{\n\t" . $body . "\b}" . $cont;
}

sub pp_leaveloop { shift->loop_common(@_, "") }

sub for_loop {
    my $self = shift;
    my($op, $cx) = @_;
    my $init = $self->deparse($op, 1);
    my $s = $op->sibling;
    my $ll = $s->name eq "unstack" ? $s->sibling : $s->first->sibling;
    return $self->loop_common($ll, $cx, $init);
}

sub pp_leavetry {
    my $self = shift;
    return "eval {\n\t" . $self->pp_leave(@_) . "\n\b}";
}

sub pp_leavetrycatch_with_finally {
    my $self = shift;
    my ($op, $finallyop) = @_;

    # Expect that the first three kids should be (entertrycatch, poptry, catch)
    my $entertrycatch = $op->first;
    $entertrycatch->name eq "entertrycatch" or die "Expected entertrycatch as first child of leavetrycatch";

    my $tryblock = $entertrycatch->sibling;
    $tryblock->name eq "poptry" or die "Expected poptry as second child of leavetrycatch";

    my $catch = $tryblock->sibling;
    $catch->name eq "catch" or die "Expected catch as third child of leavetrycatch";

    my $catchblock = $catch->first->sibling;
    my $name = $catchblock->name;
    unless ($name eq "scope" || $name eq "leave") {
      die "Expected scope or leave as second child of catch, got $name instead";
    }

    my $trycode = scopeop(0, $self, $tryblock);
    my $catchvar = $self->padname($catch->targ);
    my $catchcode = $name eq 'scope' ? scopeop(0, $self, $catchblock)
                                     : scopeop(1, $self, $catchblock);

    my $finallycode = "";
    if($finallyop) {
        my $body = $self->deparse($finallyop->first->first);
        $finallycode = "\nfinally {\n\t$body\n\b}";
    }

    return "try {\n\t$trycode\n\b}\n" .
           "catch($catchvar) {\n\t$catchcode\n\b}$finallycode\cK";
}

sub pp_leavetrycatch {
    my $self = shift;
    my ($op, @args) = @_;
    return $self->pp_leavetrycatch_with_finally($op, undef, @args);
}

sub _op_is_or_was {
  my ($op, $expect_type) = @_;
  my $type = $op->type;
  return($type == $expect_type
         || ($type == OP_NULL && $op->targ == $expect_type));
}

sub pp_null {
    my($self, $op, $cx) = @_;

    # might be 'my $s :Foo(bar);'
    if ($op->targ == OP_LIST) {
        my $my_attr = maybe_var_attr($self, $op, $cx);
        return $my_attr if defined $my_attr;
    }

    if (class($op) eq "OP") {
	# old value is lost
	return $self->{'ex_const'} if $op->targ == OP_CONST;
    } elsif (class ($op) eq "COP") {
	    return &pp_nextstate;
    } elsif ($op->first->name eq 'pushmark'
             or $op->first->name eq 'null'
                && $op->first->targ == OP_PUSHMARK
                && _op_is_or_was($op, OP_LIST)) {
	return $self->pp_list($op, $cx);
    } elsif ($op->first->name eq "enter") {
	return $self->pp_leave($op, $cx);
    } elsif ($op->first->name eq "leave") {
	return $self->pp_leave($op->first, $cx);
    } elsif ($op->first->name eq "scope") {
	return $self->pp_scope($op->first, $cx);
    } elsif ($op->targ == OP_STRINGIFY) {
	return $self->dquote($op, $cx);
    } elsif ($op->targ == OP_GLOB) {
	return $self->pp_glob(
	         $op->first    # entersub
	            ->first    # ex-list
	            ->first    # pushmark
	            ->sibling, # glob
	         $cx
	       );
    } elsif (!null($op->first->sibling) and
	     $op->first->sibling->name eq "readline" and
	     $op->first->sibling->flags & OPf_STACKED) {
	return $self->maybe_parens($self->deparse($op->first, 7) . " = "
				   . $self->deparse($op->first->sibling, 7),
				   $cx, 7);
    } elsif (!null($op->first->sibling) and
	     $op->first->sibling->name =~ /^transr?\z/ and
	     $op->first->sibling->flags & OPf_STACKED) {
	return $self->maybe_parens($self->deparse($op->first, 20) . " =~ "
				   . $self->deparse($op->first->sibling, 20),
				   $cx, 20);
    } elsif ($op->flags & OPf_SPECIAL && $cx < 1 && !$op->targ) {
	return ($self->lex_in_scope("&do") ? "CORE::do" : "do")
	     . " {\n\t". $self->deparse($op->first, $cx) ."\n\b};";
    } elsif (!null($op->first->sibling) and
	     $op->first->sibling->name eq "null" and
	     class($op->first->sibling) eq "UNOP" and
	     $op->first->sibling->first->flags & OPf_STACKED and
	     $op->first->sibling->first->name eq "rcatline") {
	return $self->maybe_parens($self->deparse($op->first, 18) . " .= "
				   . $self->deparse($op->first->sibling, 18),
				   $cx, 18);
    } else {
	return $self->deparse($op->first, $cx);
    }
}

sub padname {
    my $self = shift;
    my $targ = shift;
    return $self->padname_sv($targ)->PVX;
}

sub padany {
    my $self = shift;
    my $op = shift;
    return substr($self->padname($op->targ), 1); # skip $/@/%
}

sub pp_padsv {
    my $self = shift;
    my($op, $cx, $forbid_parens) = @_;
    my $targ = $op->targ;
    return $self->maybe_my($op, $cx, $self->padname($targ),
			   $self->padname_sv($targ),
			   $forbid_parens);
}

sub pp_padav { pp_padsv(@_) }

# prepend 'keys' where its been optimised away, with suitable handling
# of CORE:: and parens

sub add_keys_keyword {
    my ($self, $str, $cx) = @_;
    $str = $self->maybe_parens($str, $cx, 16);
    # 'keys %h' versus 'keys(%h)'
    $str = " $str" unless $str =~ /^\(/;
    return $self->keyword("keys") . $str;
}

sub pp_padhv {
    my ($self, $op, $cx) = @_;
    my $str =  pp_padsv(@_);
    # with OPpPADHV_ISKEYS the keys op is optimised away, except
    # in scalar context the old op is kept (but not executed) so its targ
    # can be used.
    if (     ($op->private & OPpPADHV_ISKEYS)
        && !(($op->flags & OPf_WANT) == OPf_WANT_SCALAR))
    {
        $str = $self->add_keys_keyword($str, $cx);
    }
    $str;
}

sub gv_or_padgv {
    my $self = shift;
    my $op = shift;
    if (class($op) eq "PADOP") {
	return $self->padval($op->padix);
    } else { # class($op) eq "SVOP"
	return $op->gv;
    }
}

sub pp_gvsv {
    my $self = shift;
    my($op, $cx) = @_;
    my $gv = $self->gv_or_padgv($op);
    return $self->maybe_local($op, $cx, $self->stash_variable("\$",
				 $self->gv_name($gv), $cx));
}

sub pp_gv {
    my $self = shift;
    my($op, $cx) = @_;
    my $gv = $self->gv_or_padgv($op);
    return $self->maybe_qualify("", $self->gv_name($gv));
}

sub pp_aelemfastlex_store {
    my $self = shift;
    my($op, $cx) = @_;
    my $name = $self->padname($op->targ);
    $name =~ s/^@/\$/;
    my $i = $op->private;
    $i -= 256 if $i > 127;
    my $val = $self->deparse($op->first, 7);
    return $self->maybe_parens("${name}[$i] = $val", $cx, 7);
}

sub pp_aelemfast_lex {
    my $self = shift;
    my($op, $cx) = @_;
    my $name = $self->padname($op->targ);
    $name =~ s/^@/\$/;
    my $i = $op->private;
    $i -= 256 if $i > 127;
    return $name . "[$i]";
}

sub pp_aelemfast {
    my $self = shift;
    my($op, $cx) = @_;
    # optimised PADAV, pre 5.15
    return $self->pp_aelemfast_lex(@_) if ($op->flags & OPf_SPECIAL);

    my $gv = $self->gv_or_padgv($op);
    my($name,$quoted) = $self->stash_variable_name('@',$gv);
    $name = $quoted ? "$name->" : '$' . $name;
    my $i = $op->private;
    $i -= 256 if $i > 127;
    return $name . "[$i]";
}

sub rv2x {
    my $self = shift;
    my($op, $cx, $type) = @_;

    if (class($op) eq 'NULL' || !$op->can("first")) {
	carp("Unexpected op in pp_rv2x");
	return 'XXX';
    }
    my $kid = $op->first;
    if ($kid->name eq "gv") {
	return $self->stash_variable($type,
		    $self->gv_name($self->gv_or_padgv($kid)), $cx);
    } elsif (is_scalar $kid) {
	my $str = $self->deparse($kid, 0);
	if ($str =~ /^\$([^\w\d])\z/) {
	    # "$$+" isn't a legal way to write the scalar dereference
	    # of $+, since the lexer can't tell you aren't trying to
	    # do something like "$$ + 1" to get one more than your
	    # PID. Either "${$+}" or "$${+}" are workable
	    # disambiguations, but if the programmer did the former,
	    # they'd be in the "else" clause below rather than here.
	    # It's not clear if this should somehow be unified with
	    # the code in dq and re_dq that also adds lexer
	    # disambiguation braces.
	    $str = '$' . "{$1}"; #'
	}
	return $type . $str;
    } else {
	return $type . "{" . $self->deparse($kid, 0) . "}";
    }
}

sub pp_rv2sv { maybe_local(@_, rv2x(@_, "\$")) }
sub pp_rv2gv { maybe_local(@_, rv2x(@_, "*")) }

sub pp_rv2hv {
    my ($self, $op, $cx) = @_;
    my $str = rv2x(@_, "%");
    if ($op->private & OPpRV2HV_ISKEYS) {
        $str = $self->add_keys_keyword($str, $cx);
    }
    return maybe_local(@_, $str);
}

# skip rv2av
sub pp_av2arylen {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    if ($kid->name eq "padav") {
	return $self->maybe_local($op, $cx, '$#' . $self->padany($kid));
    } else {
        my $kkid;
        if (   $kid->name eq "rv2av"
           && ($kkid = $kid->first)
           && $kkid->name !~ /^(scope|leave|gv)$/)
        {
            # handle (expr)->$#* postfix form
            my $expr;
            $expr = $self->deparse($kkid, 24); # 24 is '->'
            $expr = "$expr->\$#*";
            # XXX maybe_local is probably wrong here: local($#-expression)
            # doesn't "do" local (the is no INTRO flag set)
            return $self->maybe_local($op, $cx, $expr);
        }
        else {
            # handle $#{expr} form
            # XXX see maybe_local comment above
            return $self->maybe_local($op, $cx, $self->rv2x($kid, $cx, '$#'));
        }
    }
}

# skip down to the old, ex-rv2cv
sub pp_rv2cv {
    my ($self, $op, $cx) = @_;
    if (!null($op->first) && $op->first->name eq 'null' &&
	$op->first->targ == OP_LIST)
    {
	return $self->rv2x($op->first->first->sibling, $cx, "&")
    }
    else {
	return $self->rv2x($op, $cx, "")
    }
}

sub list_const {
    my $self = shift;
    my($cx, @list) = @_;
    my @a = map $self->const($_, 6), @list;
    if (@a == 0) {
	return "()";
    } elsif (@a == 1) {
	return $a[0];
    } elsif ( @a > 2 and !grep(!/^-?\d+$/, @a)) {
	# collapse (-1,0,1,2) into (-1..2)
	my ($s, $e) = @a[0,-1];
	my $i = $s;
	return $self->maybe_parens("$s..$e", $cx, 9)
	  unless grep $i++ != $_, @a;
    }
    return $self->maybe_parens(join(", ", @a), $cx, 6);
}

sub pp_rv2av {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    if ($kid->name eq "const") { # constant list
	my $av = $self->const_sv($kid);
	return $self->list_const($cx, $av->ARRAY);
    } else {
	return $self->maybe_local($op, $cx, $self->rv2x($op, $cx, "\@"));
    }
 }

sub is_subscriptable {
    my $op = shift;
    if ($op->name =~ /^([ahg]elem|multideref$)/) {
	return 1;
    } elsif ($op->name eq "entersub") {
	my $kid = $op->first;
	return 0 unless null $kid->sibling;
	$kid = $kid->first;
	$kid = $kid->sibling until null $kid->sibling;
	return 0 if is_scope($kid);
	$kid = $kid->first;
	return 0 if $kid->name eq "gv" || $kid->name eq "padcv";
	return 0 if is_scalar($kid);
	return is_subscriptable($kid);	
    } else {
	return 0;
    }
}

sub elem_or_slice_array_name
{
    my $self = shift;
    my ($array, $left, $padname, $allow_arrow) = @_;

    if ($array->name eq $padname) {
	return $self->padany($array);
    } elsif (is_scope($array)) { # ${expr}[0]
	return "{" . $self->deparse($array, 0) . "}";
    } elsif ($array->name eq "gv") {
	($array, my $quoted) =
	    $self->stash_variable_name(
		$left eq '[' ? '@' : '%', $self->gv_or_padgv($array)
	    );
	if (!$allow_arrow && $quoted) {
	    # This cannot happen.
	    die "Invalid variable name $array for slice";
	}
	return $quoted ? "$array->" : $array;
    } elsif (!$allow_arrow || is_scalar $array) { # $x[0], $$x[0], ...
	return $self->deparse($array, 24);
    } else {
	return undef;
    }
}

sub elem_or_slice_single_index
{
    my $self = shift;
    my ($idx) = @_;

    $idx = $self->deparse($idx, 1);

    # Outer parens in an array index will confuse perl
    # if we're interpolating in a regular expression, i.e.
    # /$x$foo[(-1)]/ is *not* the same as /$x$foo[-1]/
    #
    # If $self->{parens}, then an initial '(' will
    # definitely be paired with a final ')'. If
    # !$self->{parens}, the misleading parens won't
    # have been added in the first place.
    #
    # [You might think that we could get "(...)...(...)"
    # where the initial and final parens do not match
    # each other. But we can't, because the above would
    # only happen if there's an infix binop between the
    # two pairs of parens, and *that* means that the whole
    # expression would be parenthesized as well.]
    #
    $idx =~ s/^\((.*)\)$/$1/ if $self->{'parens'};

    # Hash-element braces will autoquote a bareword inside themselves.
    # We need to make sure that C<$hash{warn()}> doesn't come out as
    # C<$hash{warn}>, which has a quite different meaning. Currently
    # B::Deparse will always quote strings, even if the string was a
    # bareword in the original (i.e. the OPpCONST_BARE flag is ignored
    # for constant strings.) So we can cheat slightly here - if we see
    # a bareword, we know that it is supposed to be a function call.
    #
    $idx =~ s/^([A-Za-z_]\w*)$/$1()/;

    return $idx;
}

sub elem {
    my $self = shift;
    my ($op, $cx, $left, $right, $padname) = @_;
    my($array, $idx) = ($op->first, $op->first->sibling);

    $idx = $self->elem_or_slice_single_index($idx);

    unless ($array->name eq $padname) { # Maybe this has been fixed	
	$array = $array->first; # skip rv2av (or ex-rv2av in _53+)
    }
    if (my $array_name=$self->elem_or_slice_array_name
	    ($array, $left, $padname, 1)) {
	return ($array_name =~ /->\z/
		    ? $array_name
		    : $array_name eq '#' ? '${#}' : "\$" . $array_name)
	      . $left . $idx . $right;
    } else {
	# $x[20][3]{hi} or expr->[20]
	my $arrow = is_subscriptable($array) ? "" : "->";
	return $self->deparse($array, 24) . $arrow . $left . $idx . $right;
    }

}

# a simplified version of elem_or_slice_array_name()
# for the use of pp_multideref

sub multideref_var_name {
    my $self = shift;
    my ($gv, $is_hash) = @_;

    my ($name, $quoted) =
        $self->stash_variable_name( $is_hash  ? '%' : '@', $gv);
    return $quoted ? "$name->"
                   : $name eq '#'
                        ? '${#}'       # avoid ${#}[1] => $#[1]
                        : '$' . $name;
}


# deparse an OP_MULTICONCAT. If $in_dq is 1, we're within
# a double-quoted string, so for example.
#     "abc\Qdef$x\Ebar"
# might get compiled as
#    multiconcat("abc", metaquote(multiconcat("def", $x)), "bar")
# and the inner multiconcat should be deparsed as C<def$x> rather than
# the normal C<def . $x>
# Ditto if  $in_dq is 2, handle qr/...\Qdef$x\E.../.

sub do_multiconcat {
    my $self = shift;
    my($op, $cx, $in_dq) = @_;

    my $kid;
    my @kids;
    my $assign;
    my $append;
    my $lhs = "";

    for ($kid = $op->first; !null $kid; $kid = $kid->sibling) {
        # skip the consts and/or padsv we've optimised away
        push @kids, $kid
            unless $kid->type == OP_NULL
              && (   $kid->targ == OP_PADSV
                  || $kid->targ == OP_CONST
                  || $kid->targ == OP_PUSHMARK);
    }

    $append = ($op->private & OPpMULTICONCAT_APPEND);

    if ($op->private & OPpTARGET_MY) {
        # '$lex  = ...' or '$lex .= ....' or 'my $lex = '
        $lhs = $self->padname($op->targ);
        $lhs = "my $lhs" if ($op->private & OPpLVAL_INTRO);
        $assign = 1;
    }
    elsif ($op->flags & OPf_STACKED) {
        # 'expr  = ...' or 'expr .= ....'
        my $expr = $append ? shift(@kids) : pop(@kids);
        $lhs = $self->deparse($expr, 7);
        $assign = 1;
    }

    if ($assign) {
        $lhs .=  $append ? ' .= ' : ' = ';
    }

    my ($nargs, $const_str, @const_lens) = $op->aux_list($self->{curcv});

    my @consts;
    my $i = 0;
    for (@const_lens) {
        if ($_ == -1) {
            push @consts, undef;
        }
        else {
            push @consts, substr($const_str, $i, $_);
        my @args;
            $i += $_;
        }
    }

    my $rhs = "";

    if (   $in_dq
        || (($op->private & OPpMULTICONCAT_STRINGIFY) && !$self->{'unquote'}))
    {
        # "foo=$foo bar=$bar "
        my $not_first;
        while (@consts) {
            if ($not_first) {
                my $s = $self->dq(shift(@kids), 18);
                # don't deparse "a${$}b" as "a$$b"
                $s = '${$}' if $s eq '$$';
                $rhs = dq_disambiguate($rhs, $s);
            }
            $not_first = 1;
            my $c = shift @consts;
            if (defined $c) {
                if ($in_dq == 2) {
                    # in pattern: don't convert newline to '\n' etc etc
                    my $s = re_uninterp(escape_re(re_unback($c)));
                    $rhs = re_dq_disambiguate($rhs, $s)
                }
                else {
                    my $s = uninterp(escape_str(unback($c)));
                    $rhs = dq_disambiguate($rhs, $s)
                }
            }
        }
        return $rhs if $in_dq;
        $rhs = single_delim("qq", '"', $rhs, $self);
    }
    elsif ($op->private & OPpMULTICONCAT_FAKE) {
        # sprintf("foo=%s bar=%s ", $foo, $bar)

        my @all;
        @consts = map { $_ //= ''; s/%/%%/g; $_ } @consts;
        my $fmt = join '%s', @consts;
        push @all, $self->quoted_const_str($fmt);

        # the following is a stripped down copy of sub listop {}
        my $parens = $assign || ($cx >= 5) || $self->{'parens'};
        my $fullname = $self->keyword('sprintf');
        push @all, map $self->deparse($_, 6), @kids;

        $rhs = $parens
                ? "$fullname(" . join(", ", @all) . ")"
                : "$fullname " . join(", ", @all);
    }
    else {
        # "foo=" . $foo . " bar=" . $bar
        my @all;
        my $not_first;
        while (@consts) {
            push @all, $self->deparse(shift(@kids), 18) if $not_first;
            $not_first = 1;
            my $c = shift @consts;
            if (defined $c) {
                push @all, $self->quoted_const_str($c);
            }
        }
        $rhs .= join ' . ', @all;
    }

    my $text = $lhs . $rhs;

    $text = "($text)" if     ($cx >= (($assign) ? 7 : 18+1))
                          || $self->{'parens'};

    return $text;
}


sub pp_multiconcat {
    my $self = shift;
    $self->do_multiconcat(@_, 0);
}


sub pp_multideref {
    my $self = shift;
    my($op, $cx) = @_;
    my $text = "";

    if ($op->private & OPpMULTIDEREF_EXISTS) {
        $text = $self->keyword("exists"). " ";
    }
    elsif ($op->private & OPpMULTIDEREF_DELETE) {
        $text = $self->keyword("delete"). " ";
    }
    elsif ($op->private & OPpLVAL_INTRO) {
        $text = $self->keyword("local"). " ";
    }

    if ($op->first && ($op->first->flags & OPf_KIDS)) {
        # arbitrary initial expression, e.g. f(1,2,3)->[...]
        my $expr = $self->deparse($op->first, 24);
        # stop "exists (expr)->{...}" being interpreted as
        #"(exists (expr))->{...}"
        $expr = "+$expr" if $expr =~ /^\(/;
        $text .=  $expr;
    }

    my @items = $op->aux_list($self->{curcv});
    my $actions = shift @items;

    my $is_hash;
    my $derefs = 0;

    while (1) {
        if (($actions & MDEREF_ACTION_MASK) == MDEREF_reload) {
            $actions = shift @items;
            next;
        }

        $is_hash = (
           ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_pop_rv2hv_helem
        || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_gvsv_vivify_rv2hv_helem
        || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_padsv_vivify_rv2hv_helem
        || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_vivify_rv2hv_helem
        || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_padhv_helem
        || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_gvhv_helem
        );

        if (   ($actions & MDEREF_ACTION_MASK) == MDEREF_AV_padav_aelem
            || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_padhv_helem)
        {
            $derefs = 1;
            $text .= '$' . substr($self->padname(shift @items), 1);
        }
        elsif (   ($actions & MDEREF_ACTION_MASK) == MDEREF_AV_gvav_aelem
               || ($actions & MDEREF_ACTION_MASK) == MDEREF_HV_gvhv_helem)
        {
            $derefs = 1;
            $text .= $self->multideref_var_name(shift @items, $is_hash);
        }
        else {
            if (   ($actions & MDEREF_ACTION_MASK) ==
                                        MDEREF_AV_padsv_vivify_rv2av_aelem
                || ($actions & MDEREF_ACTION_MASK) ==
                                        MDEREF_HV_padsv_vivify_rv2hv_helem)
            {
                $text .= $self->padname(shift @items);
            }
            elsif (   ($actions & MDEREF_ACTION_MASK) ==
                                           MDEREF_AV_gvsv_vivify_rv2av_aelem
                   || ($actions & MDEREF_ACTION_MASK) ==
                                           MDEREF_HV_gvsv_vivify_rv2hv_helem)
            {
                $text .= $self->multideref_var_name(shift @items, $is_hash);
            }
            elsif (   ($actions & MDEREF_ACTION_MASK) ==
                                           MDEREF_AV_pop_rv2av_aelem
                   || ($actions & MDEREF_ACTION_MASK) ==
                                           MDEREF_HV_pop_rv2hv_helem)
            {
                if (   ($op->flags & OPf_KIDS)
                    && (   _op_is_or_was($op->first, OP_RV2AV)
                        || _op_is_or_was($op->first, OP_RV2HV))
                    && ($op->first->flags & OPf_KIDS)
                    && (   _op_is_or_was($op->first->first, OP_AELEM)
                        || _op_is_or_was($op->first->first, OP_HELEM))
                    )
                {
                    $derefs++;
                }
            }

            $text .= '->' if !$derefs++;
        }


        if (($actions & MDEREF_INDEX_MASK) == MDEREF_INDEX_none) {
            last;
        }

        $text .= $is_hash ? '{' : '[';

        if (($actions & MDEREF_INDEX_MASK) == MDEREF_INDEX_const) {
            my $key = shift @items;
            if ($is_hash) {
                $text .= $self->const($key, $cx);
            }
            else {
                $text .= $key;
            }
        }
        elsif (($actions & MDEREF_INDEX_MASK) == MDEREF_INDEX_padsv) {
            $text .= $self->padname(shift @items);
        }
        elsif (($actions & MDEREF_INDEX_MASK) == MDEREF_INDEX_gvsv) {
            $text .= '$' .  ($self->stash_variable_name('$', shift @items))[0];
        }

        $text .= $is_hash ? '}' : ']';

        if ($actions & MDEREF_FLAG_last) {
            last;
        }
        $actions >>= MDEREF_SHIFT;
    }

    return $text;
}


sub pp_aelem { maybe_local(@_, elem(@_, "[", "]", "padav")) }
sub pp_helem { maybe_local(@_, elem(@_, "{", "}", "padhv")) }

sub pp_gelem {
    my $self = shift;
    my($op, $cx) = @_;
    my($glob, $part) = ($op->first, $op->last);
    $glob = $glob->first; # skip rv2gv
    $glob = $glob->first if $glob->name eq "rv2gv"; # this one's a bug
    my $scope = is_scope($glob);
    $glob = $self->deparse($glob, 0);
    $part = $self->deparse($part, 1);
    $glob =~ s/::\z// unless $scope;
    return "*" . ($scope ? "{$glob}" : $glob) . "{$part}";
}

sub slice {
    my $self = shift;
    my ($op, $cx, $left, $right, $regname, $padname) = @_;
    my $last;
    my(@elems, $kid, $array, $list);
    if (class($op) eq "LISTOP") {
	$last = $op->last;
    } else { # ex-hslice inside delete()
	for ($kid = $op->first; !null $kid->sibling; $kid = $kid->sibling) {}
	$last = $kid;
    }
    $array = $last;
    $array = $array->first
	if $array->name eq $regname or $array->name eq "null";
    $array = $self->elem_or_slice_array_name($array,$left,$padname,0);
    $kid = $op->first->sibling; # skip pushmark
    if ($kid->name eq "list") {
	$kid = $kid->first->sibling; # skip list, pushmark
	for (; !null $kid; $kid = $kid->sibling) {
	    push @elems, $self->deparse($kid, 6);
	}
	$list = join(", ", @elems);
    } else {
	$list = $self->elem_or_slice_single_index($kid);
    }
    my $lead = (   _op_is_or_was($op, OP_KVHSLICE)
                || _op_is_or_was($op, OP_KVASLICE))
               ? '%' : '@';
    return $lead . $array . $left . $list . $right;
}

sub pp_aslice   { maybe_local(@_, slice(@_, "[", "]", "rv2av", "padav")) }
sub pp_kvaslice {                 slice(@_, "[", "]", "rv2av", "padav")  }
sub pp_hslice   { maybe_local(@_, slice(@_, "{", "}", "rv2hv", "padhv")) }
sub pp_kvhslice {                 slice(@_, "{", "}", "rv2hv", "padhv")  }

sub pp_lslice {
    my $self = shift;
    my($op, $cx) = @_;
    my $idx = $op->first;
    my $list = $op->last;
    my(@elems, $kid);
    $list = $self->deparse($list, 1);
    $idx = $self->deparse($idx, 1);
    return "($list)" . "[$idx]";
}

sub want_scalar {
    my $op = shift;
    return ($op->flags & OPf_WANT) == OPf_WANT_SCALAR;
}

sub want_list {
    my $op = shift;
    return ($op->flags & OPf_WANT) == OPf_WANT_LIST;
}

sub _method {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first->sibling; # skip pushmark
    my($meth, $obj, @exprs);
    if ($kid->name eq "list" and want_list $kid) {
	# When an indirect object isn't a bareword but the args are in
	# parens, the parens aren't part of the method syntax (the LLAFR
	# doesn't apply), but they make a list with OPf_PARENS set that
	# doesn't get flattened by the append_elem that adds the method,
	# making a (object, arg1, arg2, ...) list where the object
	# usually is. This can be distinguished from
	# '($obj, $arg1, $arg2)->meth()' (which is legal if $arg2 is an
	# object) because in the later the list is in scalar context
	# as the left side of -> always is, while in the former
	# the list is in list context as method arguments always are.
	# (Good thing there aren't method prototypes!)
	$meth = $kid->sibling;
	$kid = $kid->first->sibling; # skip pushmark
	$obj = $kid;
	$kid = $kid->sibling;
	for (; not null $kid; $kid = $kid->sibling) {
	    push @exprs, $kid;
	}
    } else {
	$obj = $kid;
	$kid = $kid->sibling;
	for (; !null ($kid->sibling) && $kid->name!~/^method(?:_named)?\z/;
	      $kid = $kid->sibling) {
	    push @exprs, $kid
	}
	$meth = $kid;
    }

    if ($meth->name eq "method_named") {
	$meth = $self->meth_sv($meth)->PV;
    } elsif ($meth->name eq "method_super") {
	$meth = "SUPER::".$self->meth_sv($meth)->PV;
    } elsif ($meth->name eq "method_redir") {
        $meth = $self->meth_rclass_sv($meth)->PV.'::'.$self->meth_sv($meth)->PV;
    } elsif ($meth->name eq "method_redir_super") {
        $meth = $self->meth_rclass_sv($meth)->PV.'::SUPER::'.
                $self->meth_sv($meth)->PV;
    } else {
	$meth = $meth->first;
	if ($meth->name eq "const") {
	    # As of 5.005_58, this case is probably obsoleted by the
	    # method_named case above
	    $meth = $self->const_sv($meth)->PV; # needs to be bare
	}
    }

    return { method => $meth, variable_method => ref($meth),
             object => $obj, args => \@exprs  },
	   $cx;
}

# compat function only
sub method {
    my $self = shift;
    my $info = $self->_method(@_);
    return $self->e_method( $self->_method(@_) );
}

sub e_method {
    my ($self, $info, $cx) = @_;
    my $obj = $self->deparse($info->{object}, 24);

    my $meth = $info->{method};
    $meth = $self->deparse($meth, 1) if $info->{variable_method};
    my $args = join(", ", map { $self->deparse($_, 6) } @{$info->{args}} );
    if ($info->{object}->name eq 'scope' && want_list $info->{object}) {
	# method { $object }
	# This must be deparsed this way to preserve list context
	# of $object.
	my $need_paren = $cx >= 6;
	return '(' x $need_paren
	     . $meth . substr($obj,2) # chop off the "do"
	     . " $args"
	     . ')' x $need_paren;
    }
    my $kid = $obj . "->" . $meth;
    if (length $args) {
	return $kid . "(" . $args . ")"; # parens mandatory
    } else {
	return $kid;
    }
}

# returns "&" if the prototype doesn't match the args,
# or ("", $args_after_prototype_demunging) if it does.
sub check_proto {
    my $self = shift;
    return "&" if $self->{'noproto'};
    my ($proto, @args) = @_;
    my $doneok = 0;
    my @reals;
    $proto =~ s/^\s+//;
    while (length $proto) {
        $proto =~ s/^(\\?[\$\@&%*]|\\\[[\$\@&%*]+\]|[_+;])\s*//
            or return "&";  # malformed prototype
	my $chr = $1;
        if ($chr eq ";") {
	    $doneok = 1;
        } elsif ($chr eq '@' or $chr eq '%') {
            # An unbackslashed @ or % gobbles up the rest of the args
	    push @reals, map($self->deparse($_, 6), @args);
	    @args = ();
            $proto = '';
        } elsif (!@args) {
            last if $doneok;
            return "&"; # too few args and no ';'
	} else {
            my $arg = shift @args;
            if ($chr eq '$' || $chr eq '_') {
		if (want_scalar $arg) {
		    push @reals, $self->deparse($arg, 6);
		} else {
		    return "&";
		}
	    } elsif ($chr eq "&") {
                if ($arg->name =~ /^(?:s?refgen|undef)\z/) {
		    push @reals, $self->deparse($arg, 6);
		} else {
		    return "&";
		}
	    } elsif ($chr eq "*") {
                if ($arg->name =~ /^s?refgen\z/
		    and $arg->first->first->name eq "rv2gv")
                {
                    my $real = $arg->first->first; # skip refgen, null
                    if ($real->first->name eq "gv") {
                        push @reals, $self->deparse($real, 6);
                    } else {
                        push @reals, $self->deparse($real->first, 6);
                    }
                } else {
                    return "&";
                }
            } elsif ($chr eq "+") {
                my $real;
                if ($arg->name =~ /^s?refgen\z/ and
                    !null($real = $arg->first) and
                    !null($real->first) and
                    $real->first->name =~ /^(?:rv2|pad)[ah]v\z/)
                {
                    push @reals, $self->deparse($real, 6);
                } elsif (want_scalar $arg) {
                    push @reals, $self->deparse($arg, 6);
                } else {
                    return "&";
                }
	    } elsif (substr($chr, 0, 1) eq "\\") {
		$chr =~ tr/\\[]//d;
                my $real;
                if ($arg->name =~ /^s?refgen\z/ and
		    !null($real = $arg->first) and
		    ($chr =~ /\$/ && is_scalar($real->first)
		     or ($chr =~ /@/
                         && !null($real->first)
                         && $real->first->name =~ /^(?:rv2|pad)av\z/)
		     or ($chr =~ /%/
                         && !null($real->first)
                         && $real->first->name =~ /^(?:rv2|pad)hv\z/)
		     #or ($chr =~ /&/ # This doesn't work
		     #   && $real->first->name eq "rv2cv")
		     or ($chr =~ /\*/
			 && $real->first->name eq "rv2gv")))
                {
                    push @reals, $self->deparse($real, 6);
                } else {
                    return "&";
                }
            } else {
                # should not happen
                return "&";
            }
        }
    }
    return "&" if @args; # too many args
    return ("", join ", ", @reals);
}

sub retscalar {
    my $name = $_[0]->name;
    # XXX There has to be a better way of doing this scalar-op check.
    #     Currently PL_opargs is not exposed.
    if ($name eq 'null') {
        $name = substr B::ppname($_[0]->targ), 3
    }
    $name =~ /^(?:scalar|pushmark|wantarray|const|gvsv|gv|padsv|rv2gv
                 |rv2sv|av2arylen|anoncode|prototype|srefgen|ref|bless
                 |regcmaybe|regcreset|regcomp|qr|subst|substcont|trans
                 |transr|sassign|chop|schop|chomp|schomp|defined|undef
                 |study|pos|preinc|i_preinc|predec|i_predec|postinc
                 |i_postinc|postdec|i_postdec|pow|multiply|i_multiply
                 |divide|i_divide|modulo|i_modulo|add|i_add|subtract
                 |i_subtract|concat|multiconcat|stringify|left_shift|right_shift|lt
                 |i_lt|gt|i_gt|le|i_le|ge|i_ge|eq|i_eq|ne|i_ne|ncmp|i_ncmp
                 |slt|sgt|sle|sge|seq|sne|scmp|[sn]?bit_(?:and|x?or)|negate
                 |i_negate|not|[sn]?complement|smartmatch|atan2|sin|cos
                 |rand|srand|exp|log|sqrt|int|hex|oct|abs|length|substr
                 |vec|index|rindex|sprintf|formline|ord|chr|crypt|ucfirst
                 |lcfirst|uc|lc|quotemeta|aelemfast|aelem|exists|helem
                 |pack|join|anonlist|anonhash|push|pop|shift|unshift|xor
                 |andassign|orassign|dorassign|warn|die|reset|nextstate
                 |dbstate|unstack|last|next|redo|dump|goto|exit|open|close
                 |pipe_op|fileno|umask|binmode|tie|untie|tied|dbmopen
                 |dbmclose|select|getc|read|enterwrite|prtf|print|say
                 |sysopen|sysseek|sysread|syswrite|eof|tell|seek|truncate
                 |fcntl|ioctl|flock|send|recv|socket|sockpair|bind|connect
                 |listen|accept|shutdown|gsockopt|ssockopt|getsockname
                 |getpeername|ftrread|ftrwrite|ftrexec|fteread|ftewrite
                 |fteexec|ftis|ftsize|ftmtime|ftatime|ftctime|ftrowned
                 |fteowned|ftzero|ftsock|ftchr|ftblk|ftfile|ftdir|ftpipe
                 |ftsuid|ftsgid|ftsvtx|ftlink|fttty|fttext|ftbinary|chdir
                 |chown|chroot|unlink|chmod|utime|rename|link|symlink
                 |readlink|mkdir|rmdir|open_dir|telldir|seekdir|rewinddir
                 |closedir|fork|wait|waitpid|system|exec|kill|getppid
                 |getpgrp|setpgrp|getpriority|setpriority|time|alarm|sleep
                 |shmget|shmctl|shmread|shmwrite|msgget|msgctl|msgsnd
                 |msgrcv|semop|semget|semctl|hintseval|shostent|snetent
                 |sprotoent|sservent|ehostent|enetent|eprotoent|eservent
                 |spwent|epwent|sgrent|egrent|getlogin|syscall|lock|runcv
                 |fc|padsv_store)\z/x
}

sub pp_entersub {
    my $self = shift;
    my($op, $cx) = @_;
    return $self->e_method($self->_method($op, $cx))
        unless null $op->first->sibling;
    my $prefix = "";
    my $amper = "";
    my($kid, @exprs);
    if ($op->private & OPpENTERSUB_AMPER) {
	$amper = "&";
    }
    $kid = $op->first;
    $kid = $kid->first->sibling; # skip ex-list, pushmark
    for (; not null $kid->sibling; $kid = $kid->sibling) {
	push @exprs, $kid;
    }
    my $simple = 0;
    my $proto = undef;
    my $lexical;
    if (is_scope($kid)) {
	$amper = "&";
	$kid = "{" . $self->deparse($kid, 0) . "}";
    } elsif ($kid->first->name eq "gv") {
	my $gv = $self->gv_or_padgv($kid->first);
	my $cv;
	if (class($gv) eq 'GV' && class($cv = $gv->CV) ne "SPECIAL"
	 || $gv->FLAGS & SVf_ROK && class($cv = $gv->RV) eq 'CV') {
	    $proto = $cv->PV if $cv->FLAGS & SVf_POK;
	}
	$simple = 1; # only calls of named functions can be prototyped
	$kid = $self->maybe_qualify("!", $self->gv_name($gv));
	my $fq;
	# Fully qualify any sub name that conflicts with a lexical.
	if ($self->lex_in_scope("&$kid")
	 || $self->lex_in_scope("&$kid", 1))
	{
	    $fq++;
	} elsif (!$amper) {
	    if ($kid eq 'main::') {
		$kid = '::';
	    }
	    else {
	      if ($kid !~ /::/ && $kid ne 'x') {
		# Fully qualify any sub name that is also a keyword.  While
		# we could check the import flag, we cannot guarantee that
		# the code deparsed so far would set that flag, so we qual-
		# ify the names regardless of importation.
		if (exists $feature_keywords{$kid}) {
		    $fq++ if $self->feature_enabled($kid);
		} elsif (do { local $@; local $SIG{__DIE__};
			      eval { () = prototype "CORE::$kid"; 1 } }) {
		    $fq++
		}
	      }
	      if ($kid !~ /^(?:\w|::)(?:[\w\d]|::(?!\z))*\z/) {
		$kid = single_delim("q", "'", $kid, $self) . '->';
	      }
	    }
	}
	$fq and substr $kid, 0, 0, = $self->{'curstash'}.'::';
    } elsif (is_scalar ($kid->first) && $kid->first->name ne 'rv2cv') {
	$amper = "&";
	$kid = $self->deparse($kid, 24);
    } else {
	$prefix = "";
	my $grandkid = $kid->first;
	my $arrow = ($lexical = $grandkid->name eq "padcv")
		 || is_subscriptable($grandkid)
		    ? ""
		    : "->";
	$kid = $self->deparse($kid, 24) . $arrow;
	if ($lexical) {
	    my $padlist = $self->{'curcv'}->PADLIST;
	    my $padoff = $grandkid->targ;
	    my $padname = $padlist->ARRAYelt(0)->ARRAYelt($padoff);
	    my $protocv = $padname->FLAGS & SVpad_STATE
		? $padlist->ARRAYelt(1)->ARRAYelt($padoff)
		: $padname->PROTOCV;
	    if ($protocv->FLAGS & SVf_POK) {
		$proto = $protocv->PV
	    }
	    $simple = 1;
	}
    }

    # Doesn't matter how many prototypes there are, if
    # they haven't happened yet!
    my $declared = $lexical || exists $self->{'subs_declared'}{$kid};
    if (not $declared and $self->{'in_coderef2text'}) {
	no strict 'refs';
	no warnings 'uninitialized';
	$declared =
	       (
		 defined &{ ${$self->{'curstash'}."::"}{$kid} }
		 && !exists
		     $self->{'subs_deparsed'}{$self->{'curstash'}."::".$kid}
		 && defined prototype $self->{'curstash'}."::".$kid
	       );
    }
    if (!$declared && defined($proto)) {
	# Avoid "too early to check prototype" warning
	($amper, $proto) = ('&');
    }

    my $args;
    my $listargs = 1;
    if ($declared and defined $proto and not $amper) {
	($amper, $args) = $self->check_proto($proto, @exprs);
	$listargs = $amper;
    }
    if ($listargs) {
	$args = join(", ", map(
		    ($_->flags & OPf_WANT) == OPf_WANT_SCALAR
		 && !retscalar($_)
			? $self->maybe_parens_unop('scalar', $_, 6)
			: $self->deparse($_, 6),
		    @exprs
		));
    }
    if ($prefix or $amper) {
	if ($kid eq '&') { $kid = "{$kid}" } # &{&} cannot be written as &&
	if ($op->flags & OPf_STACKED) {
	    return $prefix . $amper . $kid . "(" . $args . ")";
	} else {
	    return $prefix . $amper. $kid;
	}
    } else {
	# It's a syntax error to call CORE::GLOBAL::foo with a prefix,
	# so it must have been translated from a keyword call. Translate
	# it back.
	$kid =~ s/^CORE::GLOBAL:://;

        if (!$declared) {
	    return "$kid(" . $args . ")";
        }

        my $dproto = defined($proto) ? $proto : "undefined";
        if ($dproto =~ /^\s*\z/) {
	    return $kid;
        }

        my $scalar_proto = $dproto =~ /^ \s* (?: ;\s* )* (?: [\$*_+] |\\ \s* (?: [\$\@%&*] | \[ [^\]]+ \] ) ) \s* \z/x;
        if ($scalar_proto and !@exprs || is_scalar($exprs[0])) {
	    # is_scalar is an excessively conservative test here:
	    # really, we should be comparing to the precedence of the
	    # top operator of $exprs[0] (ala unop()), but that would
	    # take some major code restructuring to do right.
	    return $self->maybe_parens_func($kid, $args, $cx, 16);
        } elsif (not $scalar_proto and defined($proto) || $simple) {
	    return $self->maybe_parens_func($kid, $args, $cx, 5);
	} else {
	    return "$kid(" . $args . ")";
	}
    }
}

sub pp_enterwrite { unop(@_, "write") }

# escape things that cause interpolation in double quotes,
# but not character escapes
sub uninterp {
    my($str) = @_;
    $str =~ s/(^|\G|[^\\])((?:\\\\)*)([\$\@]|\\[uUlLQE])/$1$2\\$3/g;
    return $str;
}

{
my $bal;
BEGIN {
    use re "eval";
    # Matches any string which is balanced with respect to {braces}
    $bal = qr(
      (?:
	[^\\{}]
      | \\\\
      | \\[{}]
      | \{(??{$bal})\}
      )*
    )x;
}

# the same, but treat $|, $), $( and $ at the end of the string differently
# and leave comments unmangled for the sake of /x and (?x).
sub re_uninterp {
    my($str) = @_;

    $str =~ s/
	  ( ^|\G                  # $1
          | [^\\]
          )

          (                       # $2
            (?:\\\\)*
          )

          (                       # $3
            ( \(\?\??\{$bal\}\)   # $4  (skip over (?{}) and (??{}) blocks)
            | \#[^\n]*            #     (skip over comments)
            )
          | [\$\@]
            (?!\||\)|\(|$|\s)
          | \\[uUlLQE]
          )

	/defined($4) && length($4) ? "$1$2$4" : "$1$2\\$3"/xeg;

    return $str;
}
}

# character escapes, but not delimiters that might need to be escaped
sub escape_str { # ASCII, UTF8
    my($str) = @_;
    $str =~ s/(.)/ord($1) > 255 ? sprintf("\\x{%x}", ord($1)) : $1/eg;
    $str =~ s/\a/\\a/g;
#    $str =~ s/\cH/\\b/g; # \b means something different in a regex; and \cH
                          # isn't a backspace in EBCDIC
    $str =~ s/\t/\\t/g;
    $str =~ s/\n/\\n/g;
    $str =~ s/\e/\\e/g;
    $str =~ s/\f/\\f/g;
    $str =~ s/\r/\\r/g;
    $str =~ s/([\cA-\cZ])/'\\c' . $unctrl{$1}/ge;
    $str =~ s/([[:^print:]])/sprintf("\\%03o", ord($1))/age;
    return $str;
}

# For regexes.  Leave whitespace unmangled in case of /x or (?x).
sub escape_re {
    my($str) = @_;
    $str =~ s/(.)/ord($1) > 255 ? sprintf("\\x{%x}", ord($1)) : $1/eg;
    $str =~ s/([[:^print:]])/
	($1 =~ y! \t\n!!) ? $1 : sprintf("\\%03o", ord($1))/age;
    $str =~ s/\n/\n\f/g;
    return $str;
}

# Don't do this for regexen
sub unback {
    my($str) = @_;
    $str =~ s/\\/\\\\/g;
    return $str;
}

# Remove backslashes which precede literal control characters,
# to avoid creating ambiguity when we escape the latter.
#
# Don't remove a backslash from escaped whitespace: where the T represents
# a literal tab character, /T/x is not equivalent to /\T/x

sub re_unback {
    my($str) = @_;

    # the insane complexity here is due to the behaviour of "\c\"
    $str =~ s/
                # these two lines ensure that the backslash we're about to
                # remove isn't preceded by something which makes it part
                # of a \c

                (^ | [^\\] | \\c\\)             # $1
                (?<!\\c)

                # the backslash to remove
                \\

                # keep pairs of backslashes
                (\\\\)*                         # $2

                # only remove if the thing following is a control char
                (?=[[:^print:]])
                # and not whitespace
                (?=\S)
            /$1$2/xg;
    return $str;
}

sub balanced_delim {
    my($str) = @_;
    my @str = split //, $str;
    my($ar, $open, $close, $fail, $c, $cnt, $last_bs);
    for $ar (['[',']'], ['(',')'], ['<','>'], ['{','}']) {
	($open, $close) = @$ar;
	$fail = 0; $cnt = 0; $last_bs = 0;
	for $c (@str) {
	    if ($c eq $open) {
		$fail = 1 if $last_bs;
		$cnt++;
	    } elsif ($c eq $close) {
		$fail = 1 if $last_bs;
		$cnt--;
		if ($cnt < 0) {
		    # qq()() isn't ")("
		    $fail = 1;
		    last;
		}
	    }
	    $last_bs = $c eq '\\';
	}
	$fail = 1 if $cnt != 0;
	return ($open, "$open$str$close") if not $fail;
    }
    return ("", $str);
}

sub single_delim {
    my($q, $default, $str, $self) = @_;
    return "$default$str$default" if $default and index($str, $default) == -1;
    my $coreq = $self->keyword($q); # maybe CORE::q
    if ($q ne 'qr') {
	(my $succeed, $str) = balanced_delim($str);
	return "$coreq$str" if $succeed;
    }
    for my $delim ('/', '"', '#') {
	return "$coreq$delim" . $str . $delim if index($str, $delim) == -1;
    }
    if ($default) {
	$str =~ s/$default/\\$default/g;
	return "$default$str$default";
    } else {
	$str =~ s[/][\\/]g;
	return "$coreq/$str/";
    }
}

my $max_prec;
BEGIN { $max_prec = int(0.999 + 8*length(pack("F", 42))*log(2)/log(10)); }

# Split a floating point number into an integer mantissa and a binary
# exponent. Assumes you've already made sure the number isn't zero or
# some weird infinity or NaN.
sub split_float {
    my($f) = @_;
    my $exponent = 0;
    if ($f == int($f)) {
	while ($f % 2 == 0) {
	    $f /= 2;
	    $exponent++;
	}
    } else {
	while ($f != int($f)) {
	    $f *= 2;
	    $exponent--;
	}
    }
    my $mantissa = sprintf("%.0f", $f);
    return ($mantissa, $exponent);
}


# suitably single- or double-quote a literal constant string

sub quoted_const_str {
    my ($self, $str) =@_;
    if ($str =~ /[[:^print:]]/a) {
        return single_delim("qq", '"',
                             uninterp(escape_str unback $str), $self);
    } else {
        return single_delim("q", "'", unback($str), $self);
    }
}


sub const {
    my $self = shift;
    my($sv, $cx) = @_;
    if ($self->{'use_dumper'}) {
	return $self->const_dumper($sv, $cx);
    }
    if (class($sv) eq "SPECIAL") {
	# PL_sv_undef etc
        # return yes/no as boolean expressions rather than integers to
        # preserve their boolean-ness
	return
            $$sv == 1 ? 'undef'                            : # PL_sv_undef
            $$sv == 2 ? $self->maybe_parens("!0", $cx, 21) : # PL_sv_yes
            $$sv == 3 ? $self->maybe_parens("!1", $cx, 21) : # PL_sv_no
            $$sv == 7 ? '0'                                : # PL_sv_zero
                        '"???"';
    }
    if (class($sv) eq "NULL") {
       return 'undef';
    }
    # convert a version object into the "v1.2.3" string in its V magic
    if ($sv->FLAGS & SVs_RMG) {
	for (my $mg = $sv->MAGIC; $mg; $mg = $mg->MOREMAGIC) {
	    return $mg->PTR if $mg->TYPE eq 'V';
	}
    }

    if ($sv->FLAGS & SVf_IOK) {
	my $str = $sv->int_value;
	$str = $self->maybe_parens($str, $cx, 21) if $str < 0;
	return $str;
    } elsif ($sv->FLAGS & SVf_NOK) {
	my $nv = $sv->NV;
	if ($nv == 0) {
	    if (pack("F", $nv) eq pack("F", 0)) {
		# positive zero
		return "0.0";
	    } else {
		# negative zero
		return $self->maybe_parens("-0.0", $cx, 21);
	    }
	} elsif (1/$nv == 0) {
	    if ($nv > 0) {
		# positive infinity
		return $self->maybe_parens("9**9**9", $cx, 22);
	    } else {
		# negative infinity
		return $self->maybe_parens("-9**9**9", $cx, 21);
	    }
	} elsif ($nv != $nv) {
	    # NaN
	    if (pack("F", $nv) eq pack("F", sin(9**9**9))) {
		# the normal kind
		return "sin(9**9**9)";
	    } elsif (pack("F", $nv) eq pack("F", -sin(9**9**9))) {
		# the inverted kind
		return $self->maybe_parens("-sin(9**9**9)", $cx, 21);
	    } else {
		# some other kind
		my $hex = unpack("h*", pack("F", $nv));
		return qq'unpack("F", pack("h*", "$hex"))';
	    }
	}
	# first, try the default stringification
	my $str = "$nv";
	if ($str != $nv) {
	    # failing that, try using more precision
	    $str = sprintf("%.${max_prec}g", $nv);
#	    if (pack("F", $str) ne pack("F", $nv)) {
	    if ($str != $nv) {
		# not representable in decimal with whatever sprintf()
		# and atof() Perl is using here.
		my($mant, $exp) = split_float($nv);
		return $self->maybe_parens("$mant * 2**$exp", $cx, 19);
	    }
	}

        # preserve NV-ness: output as NNN.0 rather than NNN
        $str .= ".0" if $str =~ /^-?[0-9]+$/;

	$str = $self->maybe_parens($str, $cx, 21) if $nv < 0;
	return $str;
    } elsif ($sv->FLAGS & SVf_ROK && $sv->can("RV")) {
	my $ref = $sv->RV;
	my $class = class($ref);
	if ($class eq "AV") {
	    return "[" . $self->list_const(2, $ref->ARRAY) . "]";
	} elsif ($class eq "HV") {
	    my %hash = $ref->ARRAY;
	    my @elts;
	    for my $k (sort keys %hash) {
		push @elts, "$k => " . $self->const($hash{$k}, 6);
	    }
	    return "{" . join(", ", @elts) . "}";
	} elsif ($class eq "CV") {
	    no overloading;
	    if ($self->{curcv} &&
		 $self->{curcv}->object_2svref == $ref->object_2svref) {
		return $self->keyword("__SUB__");
	    }
	    return "sub " . $self->deparse_sub($ref);
	}
	if ($class ne 'SPECIAL' and $ref->FLAGS & SVs_SMG) {
	    for (my $mg = $ref->MAGIC; $mg; $mg = $mg->MOREMAGIC) {
		if ($mg->TYPE eq 'r') {
		    my $re = re_uninterp(escape_re(re_unback($mg->precomp)));
		    return single_delim("qr", "", $re, $self);
		}
	    }
	}
	
	my $const = $self->const($ref, 20);
	if ($self->{in_subst_repl} && $const =~ /^[0-9]/) {
	    $const = "($const)";
	}
	return $self->maybe_parens("\\$const", $cx, 20);
    } elsif ($sv->FLAGS & SVf_POK) {
	my $str = $sv->PV;
        return $self->quoted_const_str($str);
    } else {
	return "undef";
    }
}

sub const_dumper {
    my $self = shift;
    my($sv, $cx) = @_;
    my $ref = $sv->object_2svref();
    my $dumper = Data::Dumper->new([$$ref], ['$v']);
    $dumper->Purity(1)->Terse(1)->Deparse(1)->Indent(0)->Useqq(1)->Sortkeys(1);
    my $str = $dumper->Dump();
    if ($str =~ /^\$v/) {
	return '${my ' . $str . ' \$v}';
    } else {
	return $str;
    }
}

sub const_sv {
    my $self = shift;
    my $op = shift;
    my $sv = $op->sv;
    # the constant could be in the pad (under useithreads)
    $sv = $self->padval($op->targ) unless $$sv;
    return $sv;
}

sub meth_sv {
    my $self = shift;
    my $op = shift;
    my $sv = $op->meth_sv;
    # the constant could be in the pad (under useithreads)
    $sv = $self->padval($op->targ) unless $$sv;
    return $sv;
}

sub meth_rclass_sv {
    my $self = shift;
    my $op = shift;
    my $sv = $op->rclass;
    # the constant could be in the pad (under useithreads)
    $sv = $self->padval($sv) unless ref $sv;
    return $sv;
}

sub pp_const {
    my $self = shift;
    my($op, $cx) = @_;
#    if ($op->private & OPpCONST_BARE) { # trouble with '=>' autoquoting
#	return $self->const_sv($op)->PV;
#    }
    my $sv = $self->const_sv($op);
    return $self->const($sv, $cx);
}


# Join two components of a double-quoted string, disambiguating
# "${foo}bar", "${foo}{bar}", "${foo}[1]", "$foo\::bar"

sub dq_disambiguate {
    my ($first, $last) = @_;
    ($last =~ /^[A-Z\\\^\[\]_?]/ &&
        $first =~ s/([\$@])\^$/${1}{^}/)  # "${^}W" etc
        || ($last =~ /^[:'{\[\w_]/ && #'
            $first =~ s/([\$@])([A-Za-z_]\w*)$/${1}{$2}/);
    return $first . $last;
}


# Deparse a double-quoted optree. For example, "$a[0]\Q$b\Efo\"o" gets
# compiled to concat(concat($[0],quotemeta($b)),const("fo\"o")), and this
# sub deparses it back to $a[0]\Q$b\Efo"o
# (It does not add delimiters)

sub dq {
    my $self = shift;
    my $op = shift;
    my $type = $op->name;
    if ($type eq "const") {
	return uninterp(escape_str(unback($self->const_sv($op)->as_string)));
    } elsif ($type eq "concat") {
        return dq_disambiguate($self->dq($op->first), $self->dq($op->last));
    } elsif ($type eq "multiconcat") {
        return $self->do_multiconcat($op, 26, 1);
    } elsif ($type eq "uc") {
	return '\U' . $self->dq($op->first->sibling) . '\E';
    } elsif ($type eq "lc") {
	return '\L' . $self->dq($op->first->sibling) . '\E';
    } elsif ($type eq "ucfirst") {
	return '\u' . $self->dq($op->first->sibling);
    } elsif ($type eq "lcfirst") {
	return '\l' . $self->dq($op->first->sibling);
    } elsif ($type eq "quotemeta") {
	return '\Q' . $self->dq($op->first->sibling) . '\E';
    } elsif ($type eq "fc") {
	return '\F' . $self->dq($op->first->sibling) . '\E';
    } elsif ($type eq "join") {
	return $self->deparse($op->last, 26); # was join($", @ary)
    } else {
	return $self->deparse($op, 26);
    }
}

sub pp_backtick {
    my $self = shift;
    my($op, $cx) = @_;
    # skip pushmark if it exists (readpipe() vs ``)
    my $child = $op->first->sibling->isa('B::NULL')
	? $op->first : $op->first->sibling;
    if ($self->pure_string($child)) {
	return single_delim("qx", '`', $self->dq($child, 1), $self);
    }
    unop($self, @_, "readpipe");
}

sub dquote {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first->sibling; # skip ex-stringify, pushmark
    return $self->deparse($kid, $cx) if $self->{'unquote'};
    $self->maybe_targmy($kid, $cx,
			sub {single_delim("qq", '"', $self->dq($_[1]),
					   $self)});
}

# OP_STRINGIFY is a listop, but it only ever has one arg
sub pp_stringify {
    my ($self, $op, $cx) = @_;
    my $kid = $op->first->sibling;
    while ($kid->name eq 'null' && !null($kid->first)) {
	$kid = $kid->first;
    }
    if ($kid->name =~ /^(?:const|padsv|rv2sv|av2arylen|gvsv|multideref
			  |aelemfast(?:_lex)?|[ah]elem|join|concat)\z/x) {
	maybe_targmy(@_, \&dquote);
    }
    else {
	# Actually an optimised join.
	my $result = listop(@_,"join");
	$result =~ s/join([( ])/join$1$self->{'ex_const'}, /;
	$result;
    }
}

# tr/// and s/// (and tr[][], tr[]//, tr###, etc)
# note that tr(from)/to/ is OK, but not tr/from/(to)
sub double_delim {
    my($from, $to) = @_;
    my($succeed, $delim);
    if ($from !~ m[/] and $to !~ m[/]) {
	return "/$from/$to/";
    } elsif (($succeed, $from) = balanced_delim($from) and $succeed) {
	if (($succeed, $to) = balanced_delim($to) and $succeed) {
	    return "$from$to";
	} else {
	    for $delim ('/', '"', '#') { # note no "'" -- s''' is special
		return "$from$delim$to$delim" if index($to, $delim) == -1;
	    }
	    $to =~ s[/][\\/]g;
	    return "$from/$to/";
	}
    } else {
	for $delim ('/', '"', '#') { # note no '
	    return "$delim$from$delim$to$delim"
		if index($to . $from, $delim) == -1;
	}
	$from =~ s[/][\\/]g;
	$to =~ s[/][\\/]g;
	return "/$from/$to/";	
    }
}

# Escape a characrter.
# Only used by tr///, so backslashes hyphens

sub pchr {
    my($n) = @_;
    return sprintf("\\x{%X}", $n) if $n > 255;
    return '\\\\' if $n == ord '\\';
    return "\\-" if $n == ord "-";
    # I'm presuming a regex is not ok here, otherwise we could have used
    # /[[:print:]]/a to get here
    return chr($n) if (        utf8::native_to_unicode($n)
                            >= utf8::native_to_unicode(ord(' '))
                        and    utf8::native_to_unicode($n)
                            <= utf8::native_to_unicode(ord('~')));

    my $mnemonic_pos = index("\a\b\e\f\n\r\t", chr($n));
    return "\\" . substr("abefnrt", $mnemonic_pos, 1) if $mnemonic_pos >= 0;

    return '\\c' . $unctrl{chr $n} if $n >= ord("\cA") and $n <= ord("\cZ");
#   return '\x' . sprintf("%02x", $n);
    return '\\' . sprintf("%03o", $n);
}

# Convert a list of characters into a string suitable for tr/// search or
# replacement, with suitable escaping and collapsing of ranges

sub collapse {
    my(@chars) = @_;
    my($str, $c, $tr) = ("");
    for ($c = 0; $c < @chars; $c++) {
	$tr = $chars[$c];
	$str .= pchr($tr);
	if ($c <= $#chars - 2 and $chars[$c + 1] == $tr + 1 and
	    $chars[$c + 2] == $tr + 2)
	{
	    for (; $c <= $#chars-1 and $chars[$c + 1] == $chars[$c] + 1; $c++)
	      {}
	    $str .= "-";
	    $str .= pchr($chars[$c]);
	}
    }
    return $str;
}

sub tr_decode_byte {
    my($table, $flags) = @_;
    my $ssize_t = $Config{ptrsize} == 8 ? 'q' : 'l';
    my ($size, @table) = unpack("${ssize_t}s*", $table);
    pop @table; # remove the wildcard final entry

    my($c, $tr, @from, @to, @delfrom, $delhyphen);
    if ($table[ord "-"] != -1 and
	$table[ord("-") - 1] == -1 || $table[ord("-") + 1] == -1)
    {
	$tr = $table[ord "-"];
	$table[ord "-"] = -1;
	if ($tr >= 0) {
	    @from = ord("-");
	    @to = $tr;
	} else { # -2 ==> delete
	    $delhyphen = 1;
	}
    }
    for ($c = 0; $c < @table; $c++) {
	$tr = $table[$c];
	if ($tr >= 0) {
	    push @from, $c; push @to, $tr;
	} elsif ($tr == -2) {
	    push @delfrom, $c;
	}
    }
    @from = (@from, @delfrom);

    if ($flags & OPpTRANS_COMPLEMENT) {
        unless ($flags & OPpTRANS_DELETE) {
            @to = () if ("@from" eq "@to");
        }

	my @newfrom = ();
	my %from;
	@from{@from} = (1) x @from;
	for ($c = 0; $c < 256; $c++) {
	    push @newfrom, $c unless $from{$c};
	}
	@from = @newfrom;
    }
    unless ($flags & OPpTRANS_DELETE || !@to) {
	pop @to while $#to and $to[$#to] == $to[$#to -1];
    }
    my($from, $to);
    $from = collapse(@from);
    $to = collapse(@to);
    $from .= "-" if $delhyphen;
    return ($from, $to);
}

my $infinity = ~0 >> 1;     # IV_MAX

sub tr_append_to_invlist {
    my ($list_ref, $current, $next) = @_;

    # Appends the range $current..$next-1 to the inversion list $list_ref

    printf STDERR "%d: %d..%d %s", __LINE__, $current, $next, Dumper $list_ref if DEBUG;

    if (@$list_ref && $list_ref->[-1] == $current) {

        # The new range extends the current final one.  If it is a finite
        # rane, replace the current final by the new ending.
        if (defined $next) {
            $list_ref->[-1] = $next;
        }
        else {
            # The new range extends to infinity, which means the current end
            # of the inversion list is dangling.  Removing it causes things to
            # work.
            pop @$list_ref;
        }
    }
    else {  # The new range starts after the current final one; add it as a
            # new range
        push @$list_ref, $current;
        push @$list_ref, $next if defined $next;
    }

    print STDERR __LINE__, ": ", Dumper $list_ref if DEBUG;
}

sub tr_invlist_to_string {
    my ($list_ref, $to_complement) = @_;

    # Stringify the inversion list $list_ref, possibly complementing it first.
    # CAUTION: this can modify $list_ref.

    print STDERR __LINE__, ": ", Dumper $list_ref if DEBUG;

    if ($to_complement) {

        # Complementing an inversion list is done by prepending a 0 if it
        # doesn't have one there already; otherwise removing the leading 0.
        if ($list_ref->[0] == 0) {
            shift @$list_ref;
        }
        else {
            unshift @$list_ref, 0;
        }

        print STDERR __LINE__, ": ", Dumper $list_ref if DEBUG;
    }

    my $output = "";

    # Every other element is in the list.
    for (my $i = 0; $i < @$list_ref; $i += 2) {
        my $base = $list_ref->[$i];
        $output .= pchr($base);
        last unless defined $list_ref->[$i+1];

        # The beginning of the next element starts the range of items not in
        # the list.
        my $upper = $list_ref->[$i+1] - 1;
        my $range = $upper - $base;
        $output .= '-' if $range > 1; # Adjacent characters don't have a
                                      # minus, though it would be legal to do
                                      # so
        $output .= pchr($upper) if $range > 0;
    }

    print STDERR __LINE__, ": tr_invlist_to_string() returning '$output'\n"
                                                                       if DEBUG;
    return $output;
}

my $unmapped = ~0;
my $special_handling = ~0 - 1;

sub dump_invmap {
    my ($invlist_ref, $map_ref) = @_;

    for my $i (0 .. @$invlist_ref - 1) {
        printf STDERR "[%d]\t%x\t", $i, $invlist_ref->[$i];
        my $map = $map_ref->[$i];
        if ($map == $unmapped) {
            print STDERR "TR_UNMAPPED\n";
        }
        elsif ($map == $special_handling) {
            print STDERR "TR_SPECIAL\n";
        }
        else {
            printf STDERR "%x\n", $map;
        }
    }
}

sub tr_decode_utf8 {
    my($tr_av, $flags) = @_;

    printf STDERR "\n%s: %d: flags=0x%x\n", __FILE__, __LINE__, $flags if DEBUG;

    my $invlist = $tr_av->ARRAYelt(0);
    my @invlist = unpack("J*", $invlist->PV);
    my @map = unpack("J*", $tr_av->ARRAYelt(1)->PV);

    dump_invmap(\@invlist, \@map) if DEBUG;

    my @from;
    my @to;

    # Go through the whole map
    for (my $i = 0; $i < @invlist; $i++) {
        my $map = $map[$i];
        printf STDERR "%d: i=%d, source=%x, map=%x\n",
                      __LINE__, $i, $invlist[$i], $map if DEBUG;

        # Ignore any lines that are unmapped
        next if $map == $unmapped;

        # Calculate this component of the mapping;  First the lhs
        my $this_from = $invlist[$i];
        my $next_from = $invlist[$i+1] if $i < @invlist - 1;

        # The length of the rhs is the same as the lhs, except when special
        my $next_map = $map - $this_from + $next_from
                            if $map != $special_handling && defined $next_from;

        if (DEBUG) {
            printf STDERR "%d: i=%d, from=%x, to=%x",
                          __LINE__, $i, $this_from, $map;
            printf STDERR ", next_from=%x,", $next_from if defined $next_from;
            printf STDERR ", next_map=%x", $next_map if defined $next_map;
            print  STDERR "\n";
        }

        # Add the lhs.
        tr_append_to_invlist(\@from, $this_from, $next_from);

        # And, the rhs; special handling doesn't get output as it really is an
        # unmatched rhs
        tr_append_to_invlist(\@to, $map, $next_map) if $map != $special_handling;
    }

    # Done with the input.

    my $to;
    if (join("", @from) eq join("", @to)) {

        # the rhs is suppressed if identical to the left.  That's because
        # tr/ABC/ABC/ can be written as tr/ABC//.  (Do this comparison before
        # any complementing)
        $to = "";
    }
    else {
        $to = tr_invlist_to_string(\@to, 0);  # rhs not complemented
    }

    my $from = tr_invlist_to_string(\@from,
                                   ($flags & OPpTRANS_COMPLEMENT) != 0);

    print STDERR "Returning ", escape_str($from), "/",
                               escape_str($to), "\n" if DEBUG;
    return (escape_str($from), escape_str($to));
}

sub pp_trans {
    my $self = shift;
    my($op, $cx, $morflags) = @_;
    my($from, $to);
    my $class = class($op);
    my $priv_flags = $op->private;
    if ($class eq "PVOP") {
	($from, $to) = tr_decode_byte($op->pv, $priv_flags);
    } elsif ($class eq "PADOP") {
	($from, $to)
	  = tr_decode_utf8($self->padval($op->padix), $priv_flags);
    } else { # class($op) eq "SVOP"
	($from, $to) = tr_decode_utf8($op->sv, $priv_flags);
    }
    my $flags = "";
    $flags .= "c" if $priv_flags & OPpTRANS_COMPLEMENT;
    $flags .= "d" if $priv_flags & OPpTRANS_DELETE;
    $to = "" if $from eq $to and $flags eq "";
    $flags .= "s" if $priv_flags & OPpTRANS_SQUASH;
    $flags .= $morflags if defined $morflags;
    my $ret = $self->keyword("tr") . double_delim($from, $to) . $flags;
    if (my $targ = $op->targ) {
	return $self->maybe_parens($self->padname($targ) . " =~ $ret",
				   $cx, 20);
    }
    return $ret;
}

sub pp_transr { push @_, 'r'; goto &pp_trans }

# Join two components of a double-quoted re, disambiguating
# "${foo}bar", "${foo}{bar}", "${foo}[1]".

sub re_dq_disambiguate {
    my ($first, $last) = @_;
    ($last =~ /^[A-Z\\\^\[\]_?]/ &&
	$first =~ s/([\$@])\^$/${1}{^}/)  # "${^}W" etc
	|| ($last =~ /^[{\[\w_]/ &&
	    $first =~ s/([\$@])([A-Za-z_]\w*)$/${1}{$2}/);
    return $first . $last;
}

# Like dq(), but different
sub re_dq {
    my $self = shift;
    my ($op) = @_;

    my $type = $op->name;
    if ($type eq "const") {
	my $unbacked = re_unback($self->const_sv($op)->as_string);
	return re_uninterp(escape_re($unbacked));
    } elsif ($type eq "concat") {
	my $first = $self->re_dq($op->first);
	my $last  = $self->re_dq($op->last);
	return re_dq_disambiguate($first, $last);
    } elsif ($type eq "multiconcat") {
        return $self->do_multiconcat($op, 26, 2);
    } elsif ($type eq "uc") {
	return '\U' . $self->re_dq($op->first->sibling) . '\E';
    } elsif ($type eq "lc") {
	return '\L' . $self->re_dq($op->first->sibling) . '\E';
    } elsif ($type eq "ucfirst") {
	return '\u' . $self->re_dq($op->first->sibling);
    } elsif ($type eq "lcfirst") {
	return '\l' . $self->re_dq($op->first->sibling);
    } elsif ($type eq "quotemeta") {
	return '\Q' . $self->re_dq($op->first->sibling) . '\E';
    } elsif ($type eq "fc") {
	return '\F' . $self->re_dq($op->first->sibling) . '\E';
    } elsif ($type eq "join") {
	return $self->deparse($op->last, 26); # was join($", @ary)
    } else {
	my $ret = $self->deparse($op, 26);
	$ret =~ s/^\$([(|)])\z/\${$1}/ # $( $| $) need braces
	or $ret =~ s/^\@([-+])\z/\@{$1}/; # @- @+ need braces
	return $ret;
    }
}

sub pure_string {
    my ($self, $op) = @_;
    return 0 if null $op;
    my $type = $op->name;

    if ($type eq 'const' || $type eq 'av2arylen') {
	return 1;
    }
    elsif ($type =~ /^(?:[ul]c(first)?|fc)$/ || $type eq 'quotemeta') {
	return $self->pure_string($op->first->sibling);
    }
    elsif ($type eq 'join') {
	my $join_op = $op->first->sibling;  # Skip pushmark
	return 0 unless $join_op->name eq 'null' && $join_op->targ == OP_RV2SV;

	my $gvop = $join_op->first;
	return 0 unless $gvop->name eq 'gvsv';
        return 0 unless '"' eq $self->gv_name($self->gv_or_padgv($gvop));

	return 0 unless ${$join_op->sibling} eq ${$op->last};
	return 0 unless $op->last->name =~ /^(?:[ah]slice|(?:rv2|pad)av)$/;
    }
    elsif ($type eq 'concat') {
	return $self->pure_string($op->first)
            && $self->pure_string($op->last);
    }
    elsif ($type eq 'multiconcat') {
        my ($kid, @kids);
        for ($kid = $op->first; !null $kid; $kid = $kid->sibling) {
            # skip the consts and/or padsv we've optimised away
            push @kids, $kid
                unless $kid->type == OP_NULL
                  && (   $kid->targ == OP_PADSV
                      || $kid->targ == OP_CONST
                      || $kid->targ == OP_PUSHMARK);
        }

        if ($op->flags & OPf_STACKED) {
            # remove expr from @kids where 'expr  = ...' or 'expr .= ....'
            if ($op->private & OPpMULTICONCAT_APPEND) {
                shift(@kids);
            }
            else {
                pop(@kids);
            }
        }
        for (@kids) {
            return 0 unless $self->pure_string($_);
        }
        return 1;
    }
    elsif (is_scalar($op) || $type =~ /^[ah]elem$/) {
	return 1;
    }
    elsif ($type eq "null" and $op->can('first') and not null $op->first) {
        my $first = $op->first;

        return 1 if $first->name eq "multideref";
        return 1 if $first->name eq "aelemfast_lex";

        if (    $first->name eq "null"
            and $first->can('first')
	    and not null $first->first
            and $first->first->name eq "aelemfast"
	   )
        {
            return 1;
        }
    }

    return 0;
}

sub code_list {
    my ($self,$op,$cv) = @_;

    # localise stuff relating to the current sub
    $cv and
	local($self->{'curcv'}) = $cv,
	local($self->{'curcvlex'}),
	local(@$self{qw'curstash warnings hints hinthash curcop'})
	    = @$self{qw'curstash warnings hints hinthash curcop'};

    my $re;
    for ($op = $op->first->sibling; !null($op); $op = $op->sibling) {
	if ($op->name eq 'null' and $op->flags & OPf_SPECIAL) {
	    my $scope = $op->first;
	    # 0 context (last arg to scopeop) means statement context, so
	    # the contents of the block will not be wrapped in do{...}.
	    my $block = scopeop($scope->first->name eq "enter", $self,
				$scope, 0);
	    # next op is the source code of the block
	    $op = $op->sibling;
	    $re .= ($self->const_sv($op)->PV =~ m|^(\(\?\??\{)|)[0];
	    my $multiline = $block =~ /\n/;
	    $re .= $multiline ? "\n\t" : ' ';
	    $re .= $block;
	    $re .= $multiline ? "\n\b})" : " })";
	} else {
	    $re = re_dq_disambiguate($re, $self->re_dq($op));
	}
    }
    $re;
}

sub regcomp {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    $kid = $kid->first if $kid->name eq "regcmaybe";
    $kid = $kid->first if $kid->name eq "regcreset";
    my $kname = $kid->name;
    if ($kname eq "null" and !null($kid->first)
	and $kid->first->name eq 'pushmark')
    {
	my $str = '';
	$kid = $kid->first->sibling;
	while (!null($kid)) {
	    my $first = $str;
	    my $last = $self->re_dq($kid);
	    $str = re_dq_disambiguate($first, $last);
	    $kid = $kid->sibling;
	}
	return $str, 1;
    }

    return ($self->re_dq($kid), 1)
	if $kname =~ /^(?:rv2|pad)av/ or $self->pure_string($kid);
    return ($self->deparse($kid, $cx), 0);
}

sub pp_regcomp {
    my ($self, $op, $cx) = @_;
    return (($self->regcomp($op, $cx, 0))[0]);
}

sub re_flags {
    my ($self, $op) = @_;
    my $flags = '';
    my $pmflags = $op->pmflags;
    if (!$pmflags) {
	my $re = $op->pmregexp;
	if ($$re) {
	    $pmflags = $re->compflags;
	}
    }
    $flags .= "g" if $pmflags & PMf_GLOBAL;
    $flags .= "i" if $pmflags & PMf_FOLD;
    $flags .= "m" if $pmflags & PMf_MULTILINE;
    $flags .= "o" if $pmflags & PMf_KEEP;
    $flags .= "s" if $pmflags & PMf_SINGLELINE;
    $flags .= "x" if $pmflags & PMf_EXTENDED;
    $flags .= "x" if $pmflags & PMf_EXTENDED_MORE;
    $flags .= "p" if $pmflags & PMf_KEEPCOPY;
    $flags .= "n" if $pmflags & PMf_NOCAPTURE;
    if (my $charset = $pmflags & PMf_CHARSET) {
	# Hardcoding this is fragile, but B does not yet export the
	# constants we need.
	$flags .= qw(d l u a aa)[$charset >> 7]
    }
    # The /d flag is indicated by 0; only show it if necessary.
    elsif ($self->{hinthash} and
	     $self->{hinthash}{reflags_charset}
	    || $self->{hinthash}{feature_unicode}
	or $self->{hints} & $feature::hint_mask
	  && ($self->{hints} & $feature::hint_mask)
	       != $feature::hint_mask
	  && $self->{hints} & $feature::hint_uni8bit
    ) {
	$flags .= 'd';
    }
    $flags;
}

# osmic acid -- see osmium tetroxide

my %matchwords;
map($matchwords{join "", sort split //, $_} = $_, 'cig', 'cog', 'cos', 'cogs',
    'cox', 'go', 'is', 'ism', 'iso', 'mig', 'mix', 'osmic', 'ox', 'sic',
    'sig', 'six', 'smog', 'so', 'soc', 'sog', 'xi', 'soup', 'soupmix');

# When deparsing a regular expression with code blocks, we have to look in
# various places to find the blocks.
#
# For qr/(?{...})/ without interpolation, the CV is under $qr->qr_anoncv
# and the code list (list of blocks and constants, maybe vars) is under
# $cv->ROOT->first->code_list:
#   ./perl -Ilib -MB -e 'use O "Concise", B::svref_2object(sub {qr/(?{die})/})->ROOT->first->first->sibling->pmregexp->qr_anoncv->object_2svref'
#
# For qr/$a(?{...})/ with interpolation, the code list is more accessible,
# under $pmop->code_list, but the $cv is something you have to dig for in
# the regcomp op’s kids:
#   ./perl -Ilib -mO=Concise -e 'qr/$a(?{die})/'
#
# For m// and split //, things are much simpler.  There is no CV.  The code
# list is under $pmop->code_list.

sub matchop {
    my $self = shift;
    my($op, $cx, $name, $delim) = @_;
    my $kid = $op->first;
    my ($binop, $var, $re) = ("", "", "");
    if ($op->name ne 'split' && $op->flags & OPf_STACKED) {
	$binop = 1;
	$var = $self->deparse($kid, 20);
	$kid = $kid->sibling;
    }
           # not $name; $name will be 'm' for both match and split
    elsif ($op->name eq 'match' and my $targ = $op->targ) {
	$binop = 1;
	$var = $self->padname($targ);
    }
    my $quote = 1;
    my $pmflags = $op->pmflags;
    my $rhs_bound_to_defsv;
    my ($cv, $bregexp);
    my $have_kid = !null $kid;
    # Check for code blocks first
    if (not null my $code_list = $op->code_list) {
	$re = $self->code_list($code_list,
			       $op->name eq 'qr'
				   ? $self->padval(
				         $kid->first   # ex-list
					     ->first   #   pushmark
					     ->sibling #   entersub
					     ->first   #     ex-list
					     ->first   #       pushmark
					     ->sibling #       anoncode
					     ->targ
				     )
				   : undef);
    } elsif (${$bregexp = $op->pmregexp} && ${$cv = $bregexp->qr_anoncv}) {
	my $patop = $cv->ROOT      # leavesub
		       ->first     #   qr
		       ->code_list;#     list
	$re = $self->code_list($patop, $cv);
    } elsif (!$have_kid) {
	$re = re_uninterp(escape_re(re_unback($op->precomp)));
    } elsif ($kid->name ne 'regcomp') {
        if ($op->name eq 'split') {
            # split has other kids, not just regcomp
            $re = re_uninterp(escape_re(re_unback($op->precomp)));
        }
        else {
            carp("found ".$kid->name." where regcomp expected");
        }
    } else {
	($re, $quote) = $self->regcomp($kid, 21);
    }
    if ($have_kid and $kid->name eq 'regcomp') {
	my $matchop = $kid->first;
	if ($matchop->name eq 'regcreset') {
	    $matchop = $matchop->first;
	}
	if ($matchop->name =~ /^(?:match|transr?|subst)\z/
	   && $matchop->flags & OPf_SPECIAL) {
	    $rhs_bound_to_defsv = 1;
	}
    }
    my $flags = "";
    $flags .= "c" if $pmflags & PMf_CONTINUE;
    $flags .= $self->re_flags($op);
    $flags = join '', sort split //, $flags;
    $flags = $matchwords{$flags} if $matchwords{$flags};
    if ($pmflags & PMf_ONCE) { # only one kind of delimiter works here
	$re =~ s/\?/\\?/g;
	$re = $self->keyword("m") . "?$re?";     # explicit 'm' is required
    } elsif ($quote) {
	$re = single_delim($name, $delim, $re, $self);
    }
    $re = $re . $flags if $quote;
    if ($binop) {
	return
	 $self->maybe_parens(
	  $rhs_bound_to_defsv
	   ? "$var =~ (\$_ =~ $re)"
	   : "$var =~ $re",
	  $cx, 20
	 );
    } else {
	return $re;
    }
}

sub pp_match { matchop(@_, "m", "/") }
sub pp_qr { matchop(@_, "qr", "") }

sub pp_runcv { unop(@_, "__SUB__"); }

sub pp_split {
    my $self = shift;
    my($op, $cx) = @_;
    my($kid, @exprs, $ary, $expr);
    my $stacked = $op->flags & OPf_STACKED;

    $kid = $op->first;
    $kid = $kid->sibling if $kid->name eq 'regcomp';
    for (; !null($kid); $kid = $kid->sibling) {
	push @exprs, $self->deparse($kid, 6);
    }

    unshift @exprs, $self->matchop($op, $cx, "m", "/");

    if ($op->private & OPpSPLIT_ASSIGN) {
        # With C<@array = split(/pat/, str);>,
        #  array is stored in split's pmreplroot; either
        # as an integer index into the pad (for a lexical array)
        # or as GV for a package array (which will be a pad index
        # on threaded builds)
        # With my/our @array = split(/pat/, str), the array is instead
        # accessed via an extra padav/rv2av op at the end of the
        # split's kid ops.

        if ($stacked) {
            $ary = pop @exprs;
        }
        else {
            if ($op->private & OPpSPLIT_LEX) {
                $ary = $self->padname($op->pmreplroot);
            }
            else {
                # union with op_pmtargetoff, op_pmtargetgv
                my $gv = $op->pmreplroot;
                $gv = $self->padval($gv) if !ref($gv);
                $ary = $self->maybe_local(@_,
			      $self->stash_variable('@',
						     $self->gv_name($gv),
						     $cx))
            }
            if ($op->private & OPpLVAL_INTRO) {
                $ary = $op->private & OPpSPLIT_LEX ? "my $ary" : "local $ary";
            }
        }
    }

    # handle special case of split(), and split(' ') that compiles to /\s+/
    $exprs[0] = q{' '} if ($op->reflags // 0) & RXf_SKIPWHITE();

    $expr = "split(" . join(", ", @exprs) . ")";
    if ($ary) {
	return $self->maybe_parens("$ary = $expr", $cx, 7);
    } else {
	return $expr;
    }
}

# oxime -- any of various compounds obtained chiefly by the action of
# hydroxylamine on aldehydes and ketones and characterized by the
# bivalent grouping C=NOH [Webster's Tenth]

my %substwords;
map($substwords{join "", sort split //, $_} = $_, 'ego', 'egoism', 'em',
    'es', 'ex', 'exes', 'gee', 'go', 'goes', 'ie', 'ism', 'iso', 'me',
    'meese', 'meso', 'mig', 'mix', 'os', 'ox', 'oxime', 'see', 'seem',
    'seg', 'sex', 'sig', 'six', 'smog', 'sog', 'some', 'xi', 'rogue',
    'sir', 'rise', 'smore', 'more', 'seer', 'rome', 'gore', 'grim', 'grime',
    'or', 'rose', 'rosie');

sub pp_subst {
    my $self = shift;
    my($op, $cx) = @_;
    my $kid = $op->first;
    my($binop, $var, $re, $repl) = ("", "", "", "");
    if ($op->flags & OPf_STACKED) {
	$binop = 1;
	$var = $self->deparse($kid, 20);
	$kid = $kid->sibling;
    }
    elsif (my $targ = $op->targ) {
	$binop = 1;
	$var = $self->padname($targ);
    }
    my $flags = "";
    my $pmflags = $op->pmflags;
    if (null($op->pmreplroot)) {
	$repl = $kid;
	$kid = $kid->sibling;
    } else {
	$repl = $op->pmreplroot->first; # skip substcont
    }
    while ($repl->name eq "entereval") {
	    $repl = $repl->first;
	    $flags .= "e";
    }
    {
	local $self->{in_subst_repl} = 1;
	if ($pmflags & PMf_EVAL) {
	    $repl = $self->deparse($repl->first, 0);
	} else {
	    $repl = $self->dq($repl);	
	}
    }
    if (not null my $code_list = $op->code_list) {
	$re = $self->code_list($code_list);
    } elsif (null $kid) {
	$re = re_uninterp(escape_re(re_unback($op->precomp)));
    } else {
	($re) = $self->regcomp($kid, 1);
    }
    $flags .= "r" if $pmflags & PMf_NONDESTRUCT;
    $flags .= "e" if $pmflags & PMf_EVAL;
    $flags .= $self->re_flags($op);
    $flags = join '', sort split //, $flags;
    $flags = $substwords{$flags} if $substwords{$flags};
    my $core_s = $self->keyword("s"); # maybe CORE::s
    if ($binop) {
	return $self->maybe_parens("$var =~ $core_s"
				   . double_delim($re, $repl) . $flags,
				   $cx, 20);
    } else {
	return "$core_s". double_delim($re, $repl) . $flags;	
    }
}

sub is_lexical_subs {
    my (@ops) = shift;
    for my $op (@ops) {
        return 0 if $op->name !~ /\A(?:introcv|clonecv)\z/;
    }
    return 1;
}

# Pretend these two ops do not exist.  The perl parser adds them to the
# beginning of any block containing my-sub declarations, whereas we handle
# the subs in pad_subs and next_todo.
*pp_clonecv = *pp_introcv;
sub pp_introcv {
    my $self = shift;
    my($op, $cx) = @_;
    # For now, deparsing doesn't worry about the distinction between introcv
    # and clonecv, so pretend this op doesn't exist:
    return '';
}

sub pp_padcv {
    my $self = shift;
    my($op, $cx) = @_;
    return $self->padany($op);
}

my %lvref_funnies = (
    OPpLVREF_SV, => '$',
    OPpLVREF_AV, => '@',
    OPpLVREF_HV, => '%',
    OPpLVREF_CV, => '&',
);

sub pp_refassign {
    my ($self, $op, $cx) = @_;
    my $left;
    if ($op->private & OPpLVREF_ELEM) {
	$left = $op->first->sibling;
	$left = maybe_local(@_, elem($self, $left, undef,
				     $left->targ == OP_AELEM
					? qw([ ] padav)
					: qw({ } padhv)));
    } elsif ($op->flags & OPf_STACKED) {
	$left = maybe_local(@_,
			    $lvref_funnies{$op->private & OPpLVREF_TYPE}
			  . $self->deparse($op->first->sibling));
    } else {
	$left = &pp_padsv;
    }
    my $right = $self->deparse_binop_right($op, $op->first, 7);
    return $self->maybe_parens("\\$left = $right", $cx, 7);
}

sub pp_lvref {
    my ($self, $op, $cx) = @_;
    my $code;
    if ($op->private & OPpLVREF_ELEM) {
	$code = $op->first->name =~ /av\z/ ? &pp_aelem : &pp_helem;
    } elsif ($op->flags & OPf_STACKED) {
	$code = maybe_local(@_,
			    $lvref_funnies{$op->private & OPpLVREF_TYPE}
			  . $self->deparse($op->first));
    } else {
	$code = &pp_padsv;
    }
    "\\$code";
}

sub pp_lvrefslice {
    my ($self, $op, $cx) = @_;
    '\\' . ($op->last->name =~ /av\z/ ? &pp_aslice : &pp_hslice);
}

sub pp_lvavref {
    my ($self, $op, $cx) = @_;
    '\\(' . ($op->flags & OPf_STACKED
		? maybe_local(@_, rv2x(@_, "\@"))
		: &pp_padsv)  . ')'
}


sub pp_argcheck {
    my $self = shift;
    my($op, $cx) = @_;
    my ($params, $opt_params, $slurpy) = $op->aux_list($self->{curcv});
    my $mandatory = $params - $opt_params;
    my $check = '';

    $check .= <<EOF if !$slurpy;
die sprintf("Too many arguments for subroutine at %s line %d.\\n", (caller)[1, 2]) unless \@_ <= $params;
EOF

    $check .= <<EOF if $mandatory > 0;
die sprintf("Too few arguments for subroutine at %s line %d.\\n", (caller)[1, 2]) unless \@_ >= $mandatory;
EOF

    my $cond = ($params & 1) ? 'unless' : 'if';
    $check .= <<EOF if $slurpy eq '%';
die sprintf("Odd name/value argument for subroutine at %s line %d.\\n", (caller)[1, 2]) if \@_ > $params && ((\@_ - $params) & 1);
EOF

    $check =~ s/;\n\z//;
    return $check;
}


sub pp_argelem {
    my $self = shift;
    my($op, $cx) = @_;
    my $var = $self->padname($op->targ);
    my $ix  = $op->string($self->{curcv});
    my $expr;
    if ($op->flags & OPf_KIDS) {
        $expr = $self->deparse($op->first, 7);
    }
    elsif ($var =~ /^[@%]/) {
        $expr = $ix ? "\@_[$ix .. \$#_]" : '@_';
    }
    else {
        $expr = "\$_[$ix]";
    }
    return "my $var = $expr";
}


sub pp_argdefelem {
    my $self = shift;
    my($op, $cx) = @_;
    my $ix  = $op->targ;
    my $expr = "\@_ >= " . ($ix+1) . " ? \$_[$ix] : ";
    my $def = $self->deparse($op->first, 7);
    $def = "($def)" if $op->first->flags & OPf_PARENS;
    $expr .= $self->deparse($op->first, $cx);
    return $expr;
}


sub pp_pushdefer {
    my $self = shift;
    my($op, $cx) = @_;
    # defer block body is stored in the ->first of an OP_NULL that is
    # ->first of OP_PUSHDEFER
    my $body = $self->deparse($op->first->first);
    return "defer {\n\t$body\n\b}\cK";
}

sub builtin1 {
    my $self = shift;
    my ($op, $cx, $name) = @_;
    my $arg = $self->deparse($op->first);
    # TODO: work out if lexical alias is present somehow...
    return "builtin::$name($arg)";
}

sub pp_is_bool    { builtin1(@_, "is_bool"); }
sub pp_is_weak    { builtin1(@_, "is_weak"); }
sub pp_weaken     { builtin1(@_, "weaken"); }
sub pp_unweaken   { builtin1(@_, "unweaken"); }
sub pp_blessed    { builtin1(@_, "blessed"); }
sub pp_refaddr    { $_[0]->maybe_targmy(@_[1,2], \&builtin1, "refaddr"); }
sub pp_reftype    { $_[0]->maybe_targmy(@_[1,2], \&builtin1, "reftype"); }
sub pp_ceil       { $_[0]->maybe_targmy(@_[1,2], \&builtin1, "ceil"); }
sub pp_floor      { $_[0]->maybe_targmy(@_[1,2], \&builtin1, "floor"); }
sub pp_is_tainted { builtin1(@_, "is_tainted"); }

1;
__END__

=head1 NAME

B::Deparse - Perl compiler backend to produce perl code

=head1 SYNOPSIS

B<perl> B<-MO=Deparse>[B<,-d>][B<,-f>I<FILE>][B<,-p>][B<,-q>][B<,-l>]
        [B<,-s>I<LETTERS>][B<,-x>I<LEVEL>] I<prog.pl>

=head1 DESCRIPTION

B::Deparse is a backend module for the Perl compiler that generates
perl source code, based on the internal compiled structure that perl
itself creates after parsing a program.  The output of B::Deparse won't
be exactly the same as the original source, since perl doesn't keep
track of comments or whitespace, and there isn't a one-to-one
correspondence between perl's syntactical constructions and their
compiled form, but it will often be close.  When you use the B<-p>
option, the output also includes parentheses even when they are not
required by precedence, which can make it easy to see if perl is
parsing your expressions the way you intended.

While B::Deparse goes to some lengths to try to figure out what your
original program was doing, some parts of the language can still trip
it up; it still fails even on some parts of Perl's own test suite.  If
you encounter a failure other than the most common ones described in
the BUGS section below, you can help contribute to B::Deparse's
ongoing development by submitting a bug report with a small
example.

=head1 OPTIONS

As with all compiler backend options, these must follow directly after
the '-MO=Deparse', separated by a comma but not any white space.

=over 4

=item B<-d>

Output data values (when they appear as constants) using Data::Dumper.
Without this option, B::Deparse will use some simple routines of its
own for the same purpose.  Currently, Data::Dumper is better for some
kinds of data (such as complex structures with sharing and
self-reference) while the built-in routines are better for others
(such as odd floating-point values).

=item B<-f>I<FILE>

Normally, B::Deparse deparses the main code of a program, and all the subs
defined in the same file.  To include subs defined in
other files, pass the B<-f> option with the filename.
You can pass the B<-f> option several times, to
include more than one secondary file.  (Most of the time you don't want to
use it at all.)  You can also use this option to include subs which are
defined in the scope of a B<#line> directive with two parameters.

=item B<-l>

Add '#line' declarations to the output based on the line and file
locations of the original code.

=item B<-p>

Print extra parentheses.  Without this option, B::Deparse includes
parentheses in its output only when they are needed, based on the
structure of your program.  With B<-p>, it uses parentheses (almost)
whenever they would be legal.  This can be useful if you are used to
LISP, or if you want to see how perl parses your input.  If you say

    if ($var & 0x7f == 65) {print "Gimme an A!"}
    print ($which ? $a : $b), "\n";
    $name = $ENV{USER} or "Bob";

C<B::Deparse,-p> will print

    if (($var & 0)) {
        print('Gimme an A!')
    };
    (print(($which ? $a : $b)), '???');
    (($name = $ENV{'USER'}) or '???')

which probably isn't what you intended (the C<'???'> is a sign that
perl optimized away a constant value).

=item B<-P>

Disable prototype checking.  With this option, all function calls are
deparsed as if no prototype was defined for them.  In other words,

    perl -MO=Deparse,-P -e 'sub foo (\@) { 1 } foo @x'

will print

    sub foo (\@) {
	1;
    }
    &foo(\@x);

making clear how the parameters are actually passed to C<foo>.

=item B<-q>

Expand double-quoted strings into the corresponding combinations of
concatenation, uc, ucfirst, lc, lcfirst, quotemeta, and join.  For
instance, print

    print "Hello, $world, @ladies, \u$gentlemen\E, \u\L$me!";

as

    print 'Hello, ' . $world . ', ' . join($", @ladies) . ', '
          . ucfirst($gentlemen) . ', ' . ucfirst(lc $me . '!');

Note that the expanded form represents the way perl handles such
constructions internally -- this option actually turns off the reverse
translation that B::Deparse usually does.  On the other hand, note that
C<$x = "$y"> is not the same as C<$x = $y>: the former makes the value
of $y into a string before doing the assignment.

=item B<-s>I<LETTERS>

Tweak the style of B::Deparse's output.  The letters should follow
directly after the 's', with no space or punctuation.  The following
options are available:

=over 4

=item B<C>

Cuddle C<elsif>, C<else>, and C<continue> blocks.  For example, print

    if (...) {
         ...
    } else {
         ...
    }

instead of

    if (...) {
         ...
    }
    else {
         ...
    }

The default is not to cuddle.

=item B<i>I<NUMBER>

Indent lines by multiples of I<NUMBER> columns.  The default is 4 columns.

=item B<T>

Use tabs for each 8 columns of indent.  The default is to use only spaces.
For instance, if the style options are B<-si4T>, a line that's indented
3 times will be preceded by one tab and four spaces; if the options were
B<-si8T>, the same line would be preceded by three tabs.

=item B<v>I<STRING>B<.>

Print I<STRING> for the value of a constant that can't be determined
because it was optimized away (mnemonic: this happens when a constant
is used in B<v>oid context).  The end of the string is marked by a period.
The string should be a valid perl expression, generally a constant.
Note that unless it's a number, it probably needs to be quoted, and on
a command line quotes need to be protected from the shell.  Some
conventional values include 0, 1, 42, '', 'foo', and
'Useless use of constant omitted' (which may need to be
B<-sv"'Useless use of constant omitted'.">
or something similar depending on your shell).  The default is '???'.
If you're using B::Deparse on a module or other file that's require'd,
you shouldn't use a value that evaluates to false, since the customary
true constant at the end of a module will be in void context when the
file is compiled as a main program.

=back

=item B<-x>I<LEVEL>

Expand conventional syntax constructions into equivalent ones that expose
their internal operation.  I<LEVEL> should be a digit, with higher values
meaning more expansion.  As with B<-q>, this actually involves turning off
special cases in B::Deparse's normal operations.

If I<LEVEL> is at least 3, C<for> loops will be translated into equivalent
while loops with continue blocks; for instance

    for ($i = 0; $i < 10; ++$i) {
        print $i;
    }

turns into

    $i = 0;
    while ($i < 10) {
        print $i;
    } continue {
        ++$i
    }

Note that in a few cases this translation can't be perfectly carried back
into the source code -- if the loop's initializer declares a my variable,
for instance, it won't have the correct scope outside of the loop.

If I<LEVEL> is at least 5, C<use> declarations will be translated into
C<BEGIN> blocks containing calls to C<require> and C<import>; for
instance,

    use strict 'refs';

turns into

    sub BEGIN {
        require strict;
        do {
            'strict'->import('refs')
        };
    }

If I<LEVEL> is at least 7, C<if> statements will be translated into
equivalent expressions using C<&&>, C<?:> and C<do {}>; for instance

    print 'hi' if $nice;
    if ($nice) {
        print 'hi';
    }
    if ($nice) {
        print 'hi';
    } else {
        print 'bye';
    }

turns into

    $nice and print 'hi';
    $nice and do { print 'hi' };
    $nice ? do { print 'hi' } : do { print 'bye' };

Long sequences of elsifs will turn into nested ternary operators, which
B::Deparse doesn't know how to indent nicely.

=back

=head1 USING B::Deparse AS A MODULE

=head2 Synopsis

    use B::Deparse;
    $deparse = B::Deparse->new("-p", "-sC");
    $body = $deparse->coderef2text(\&func);
    eval "sub func $body"; # the inverse operation

=head2 Description

B::Deparse can also be used on a sub-by-sub basis from other perl
programs.

=head2 new

    $deparse = B::Deparse->new(OPTIONS)

Create an object to store the state of a deparsing operation and any
options.  The options are the same as those that can be given on the
command line (see L</OPTIONS>); options that are separated by commas
after B<-MO=Deparse> should be given as separate strings.

=head2 ambient_pragmas

    $deparse->ambient_pragmas(strict => 'all', '$[' => $[);

The compilation of a subroutine can be affected by a few compiler
directives, B<pragmas>.  These are:

=over 4

=item *

use strict;

=item *

use warnings;

=item *

Assigning to the special variable $[

=item *

use integer;

=item *

use bytes;

=item *

use utf8;

=item *

use re;

=back

Ordinarily, if you use B::Deparse on a subroutine which has
been compiled in the presence of one or more of these pragmas,
the output will include statements to turn on the appropriate
directives.  So if you then compile the code returned by coderef2text,
it will behave the same way as the subroutine which you deparsed.

However, you may know that you intend to use the results in a
particular context, where some pragmas are already in scope.  In
this case, you use the B<ambient_pragmas> method to describe the
assumptions you wish to make.

Not all of the options currently have any useful effect.  See
L</BUGS> for more details.

The parameters it accepts are:

=over 4

=item strict

Takes a string, possibly containing several values separated
by whitespace.  The special values "all" and "none" mean what you'd
expect.

    $deparse->ambient_pragmas(strict => 'subs refs');

=item $[

Takes a number, the value of the array base $[.
Obsolete: cannot be non-zero.

=item bytes

=item utf8

=item integer

If the value is true, then the appropriate pragma is assumed to
be in the ambient scope, otherwise not.

=item re

Takes a string, possibly containing a whitespace-separated list of
values.  The values "all" and "none" are special.  It's also permissible
to pass an array reference here.

    $deparser->ambient_pragmas(re => 'eval');


=item warnings

Takes a string, possibly containing a whitespace-separated list of
values.  The values "all" and "none" are special, again.  It's also
permissible to pass an array reference here.

    $deparser->ambient_pragmas(warnings => [qw[void io]]);

If one of the values is the string "FATAL", then all the warnings
in that list will be considered fatal, just as with the B<warnings>
pragma itself.  Should you need to specify that some warnings are
fatal, and others are merely enabled, you can pass the B<warnings>
parameter twice:

    $deparser->ambient_pragmas(
	warnings => 'all',
	warnings => [FATAL => qw/void io/],
    );

See L<warnings> for more information about lexical warnings.

=item hint_bits

=item warning_bits

These two parameters are used to specify the ambient pragmas in
the format used by the special variables $^H and ${^WARNING_BITS}.

They exist principally so that you can write code like:

    { my ($hint_bits, $warning_bits);
    BEGIN {($hint_bits, $warning_bits) = ($^H, ${^WARNING_BITS})}
    $deparser->ambient_pragmas (
	hint_bits    => $hint_bits,
	warning_bits => $warning_bits,
	'$['         => 0 + $[
    ); }

which specifies that the ambient pragmas are exactly those which
are in scope at the point of calling.

=item %^H

This parameter is used to specify the ambient pragmas which are
stored in the special hash %^H.

=back

=head2 coderef2text

    $body = $deparse->coderef2text(\&func)
    $body = $deparse->coderef2text(sub ($$) { ... })

Return source code for the body of a subroutine (a block, optionally
preceded by a prototype in parens), given a reference to the
sub.  Because a subroutine can have no names, or more than one name,
this method doesn't return a complete subroutine definition -- if you
want to eval the result, you should prepend "sub subname ", or "sub "
for an anonymous function constructor.  Unless the sub was defined in
the main:: package, the code will include a package declaration.

=head1 BUGS

=over 4

=item *

The only pragmas to
be completely supported are: C<use warnings>,
C<use strict>, C<use bytes>, C<use integer>
and C<use feature>.

Excepting those listed above, we're currently unable to guarantee that
B::Deparse will produce a pragma at the correct point in the program.
(Specifically, pragmas at the beginning of a block often appear right
before the start of the block instead.)
Since the effects of pragmas are often lexically scoped, this can mean
that the pragma holds sway over a different portion of the program
than in the input file.

=item *

In fact, the above is a specific instance of a more general problem:
we can't guarantee to produce BEGIN blocks or C<use> declarations in
exactly the right place.  So if you use a module which affects compilation
(such as by over-riding keywords, overloading constants or whatever)
then the output code might not work as intended.

=item *

Some constants don't print correctly either with or without B<-d>.
For instance, neither B::Deparse nor Data::Dumper know how to print
dual-valued scalars correctly, as in:

    use constant E2BIG => ($!=7); $y = E2BIG; print $y, 0+$y;

    use constant H => { "#" => 1 }; H->{"#"};

=item *

An input file that uses source filtering probably won't be deparsed into
runnable code, because it will still include the B<use> declaration
for the source filtering module, even though the code that is
produced is already ordinary Perl which shouldn't be filtered again.

=item *

Optimized-away statements are rendered as
'???'.  This includes statements that
have a compile-time side-effect, such as the obscure

    my $x if 0;

which is not, consequently, deparsed correctly.

    foreach my $i (@_) { 0 }
  =>
    foreach my $i (@_) { '???' }

=item *

Lexical (my) variables declared in scopes external to a subroutine
appear in coderef2text output text as package variables.  This is a tricky
problem, as perl has no native facility for referring to a lexical variable
defined within a different scope, although L<PadWalker> is a good start.

See also L<Data::Dump::Streamer>, which combines B::Deparse and
L<PadWalker> to serialize closures properly.

=item *

There are probably many more bugs on non-ASCII platforms (EBCDIC).

=back

=head1 AUTHOR

Stephen McCamant <smcc@CSUA.Berkeley.EDU>, based on an earlier version
by Malcolm Beattie <mbeattie@sable.ox.ac.uk>, with contributions from
Gisle Aas, James Duncan, Albert Dvornik, Robin Houston, Dave Mitchell,
Hugo van der Sanden, Gurusamy Sarathy, Nick Ing-Simmons, and Rafael
Garcia-Suarez.

=cut
