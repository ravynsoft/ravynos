#!/usr/bin/perl -w
#
# Regenerate (overwriting only if changed):
#
#    mg_names.inc
#    mg_raw.h
#    mg_vtable.h
#    pod/perlguts.pod
#
# from information stored in this file.  pod/perlguts.pod is not completely
# regenerated.  Only the magic table is replaced; the other parts remain
# untouched.
#
# Accepts the standard regen_lib -q and -v args.
#
# This script is normally invoked from regen.pl.
#
# Its output files contain:
#
# mg_names.inc
#    included by dump.c - the textual representation for each magic type.
#    Contains a list of
#
#         { PERL_MAGIC_foo, "foo(f)" }
#
#    pairs.
#
#
# mg_raw.h
#    processed by generate_uudmap.c into mg_data.h which eventually
#    populates PL_magic_vtables[].
#
#    Contains a list of:
#
#         { 'f', "want_vtbl_foo | FLAGS", "description for comments" }
#
#    triplets. FLAGS can be:
#         PERL_MAGIC_READONLY_ACCEPTABLE
#               ok set this type of magic on an SvREADONLY() SV
#         PERL_MAGIC_VALUE_MAGIC
#               this is value magic (pos, taint etc)
#               rather than container magic (%ENV, $1 etc)
#
#
# mg_vtable.h
#     This contains five kinds of entries:
#
#     #define PERL_MAGIC_arylen         '#'
#     ....
#
#     enum { /* pass one of these to get_vtbl */
#          want_vtbl_arylen,
#          ...
#     }
#
#     PL_magic_vtable_names[] = {
#         "arylen",
#         ...
#     }
#
#     PL_magic_vtables[] = {
#         /* per-magic sets of vtable function pointers */
#         { get, set, len, clear, free, copy, dup, local },
#     }
#
#     define PL_vtbl_arylen PL_magic_vtables[want_vtbl_arylen]
#     ....
#
#
#
# pod/perlguts.pod
#     updates the list of magic types between
#     =for mg_vtable.pl begin
#     ...
#     =for mg_vtable.pl end


use strict;
require 5.004;

BEGIN {
    # Get function prototypes
    require './regen/regen_lib.pl';
}

# =====================================================================
#
# START OF CONFIGURATION DATA


# %mg
#
# This hash is mainly concerned with populating all the other stuff
# ancillary to the vtable.
#
# The key is the name, e.g. 'regdata' for PERL_MAGIC_regdata
#
# The keys of the value hash are:
#    char
#       the magic's char identifier
#
#    desc
#       a description which appears in code comments in generated files
#
#    readonly_acceptable
#       If true, set PERL_MAGIC_READONLY_ACCEPTABLE flag;
#       SvREADONLY() svs are allowed to have this magic added to them
#
#    unknown_to_sv_magic
#       if true, this isn't one of the standard magic types which
#       Perl_sv_magic() knows how to deal with
#
#    value_magic
#       If true, set PERL_MAGIC_VALUE_MAGIC flag;
#       this kind of magic is value (pos, taint etc) rather than
#       container magic (%ENV, $1 etc)
#
#    vtable
#        name of the vtable lookup enum, e.g. 'foo' creates want_vtbl_foo

my %mg =
    (
     sv => { char => "\0", vtable => 'sv', readonly_acceptable => 1,
             desc => 'Special scalar variable' },
     # overload, or type "A" magic, used to be here.  Hence overloaded is
     # often called AMAGIC internally, even though it does not use "A"
     # magic any more.
     overload_table => { char => 'c', vtable => 'ovrld',
                         desc => 'Holds overload table (AMT) on stash' },
     bm => { char => 'B', vtable => 'regexp', value_magic => 1,
             readonly_acceptable => 1,
             desc => 'Boyer-Moore (fast string search)' },
     regdata => { char => 'D', vtable => 'regdata',
                  desc => "Regex match position data\n(\@+ and \@- vars)" },
     regdatum => { char => 'd', vtable => 'regdatum',
                   desc => 'Regex match position data element' },
     env => { char => 'E', vtable => 'env', desc => '%ENV hash' },
     envelem => { char => 'e', vtable => 'envelem',
                  desc => '%ENV hash element' },
     fm => { char => 'f', vtable => 'regexp', value_magic => 1,
             readonly_acceptable => 1, desc => "Formline ('compiled' format)" },
     regex_global => { char => 'g', vtable => 'mglob', value_magic => 1,
                       readonly_acceptable => 1, desc => 'm//g target' },
     hints => { char => 'H', vtable => 'hints', desc => '%^H hash' },
     hintselem => { char => 'h', vtable => 'hintselem',
                    desc => '%^H hash element' },
     isa => { char => 'I', vtable => 'isa', desc => '@ISA array' },
     isaelem => { char => 'i', vtable => 'isaelem',
                  desc => '@ISA array element' },
     nkeys => { char => 'k', vtable => 'nkeys', value_magic => 1,
                desc => 'scalar(keys()) lvalue' },
     dbfile => { char => 'L',
                 desc => 'Debugger %_<filename' },
     dbline => { char => 'l', vtable => 'dbline',
                 desc => 'Debugger %_<filename element' },
     shared => { char => 'N', desc => 'Shared between threads',
                 unknown_to_sv_magic => 1 },
     shared_scalar => { char => 'n', desc => 'Shared between threads',
                        unknown_to_sv_magic => 1 },
     collxfrm => { char => 'o', vtable => 'collxfrm', value_magic => 1,
                   desc => 'Locale transformation' },
     tied => { char => 'P', vtable => 'pack',
               value_magic => 1, # treat as value, so 'local @tied' isn't tied
               desc => 'Tied array or hash' },
     tiedelem => { char => 'p', vtable => 'packelem',
                   desc => 'Tied array or hash element' },
     tiedscalar => { char => 'q', vtable => 'packelem',
                     desc => 'Tied scalar or handle' },
     qr => { char => 'r', vtable => 'regexp', value_magic => 1, 
             readonly_acceptable => 1, desc => 'Precompiled qr// regex' },

     hook => { char => 'Z',
         vtable => 'hook', desc => '%{^HOOK} hash' },
     hookelem => { char => 'z',
         vtable => 'hookelem', desc => '%{^HOOK} hash element' },

     sig => { char => 'S', vtable => 'sig',
		      desc => '%SIG hash' },
     sigelem => { char => 's', vtable => 'sigelem',
                  desc => '%SIG hash element' },

     taint => { char => 't', vtable => 'taint', value_magic => 1,
                desc => 'Taintedness' },
     uvar => { char => 'U', vtable => 'uvar',
               desc => 'Available for use by extensions' },
     uvar_elem => { char => 'u', desc => 'Reserved for use by extensions',
                    unknown_to_sv_magic => 1 },
     vec => { char => 'v', vtable => 'vec', value_magic => 1,
              desc => 'vec() lvalue' },
     vstring => { char => 'V', value_magic => 1,
                  desc => 'SV was vstring literal' },
     utf8 => { char => 'w', vtable => 'utf8', value_magic => 1,
               desc => 'Cached UTF-8 information' },
     substr => { char => 'x', vtable => 'substr',  value_magic => 1,
                 desc => 'substr() lvalue' },
     defelem => { char => 'y', vtable => 'defelem', value_magic => 1,
                  desc => "Shadow \"foreach\" iterator variable /\nsmart parameter vivification" },
     nonelem => { char => 'Y', vtable => 'nonelem', value_magic => 1,
                  desc => "Array element that does not exist" },
     arylen => { char => '#', vtable => 'arylen', value_magic => 1,
                 desc => 'Array length ($#ary)' },
     pos => { char => '.', vtable => 'pos', value_magic => 1,
              desc => 'pos() lvalue' },
     backref => { char => '<', vtable => 'backref', value_magic => 1,
                  readonly_acceptable => 1, desc => 'For weak ref data' },
     symtab => { char => ':', value_magic => 1,
                 desc => 'Extra data for symbol tables' },
     rhash => { char => '%', value_magic => 1,
                desc => 'Extra data for restricted hashes' },
     arylen_p => { char => '@', value_magic => 1,
                   desc => 'To move arylen out of XPVAV' },
     ext => { char => '~', desc => 'Variable magic available for use by extensions',
              readonly_acceptable => 1 },
     extvalue => { char => '^', desc => 'Value magic available for use by extensions',
              readonly_acceptable => 1, value_magic => 1 },
     checkcall => { char => ']', value_magic => 1, vtable => 'checkcall',
                    desc => 'Inlining/mutation of call to this CV'},
     debugvar => { char => '*', desc => '$DB::single, signal, trace vars',
                   vtable => 'debugvar' },
     lvref => { char => '\\', vtable => 'lvref',
                  desc => "Lvalue reference constructor" },
     destruct => {
        char        => "X",
        vtable      => 'destruct',
        desc        => "destruct callback",
        value_magic => 1,
     },
);


# %vtable_conf
#
# This hash is mainly concerned with populating the vtable.
#
# These have a subtly different "namespace" from the magic types.
#
# The key is the name, e.g. 'regdata' for PERL_MAGIC_regdata
# The keys of the value hash are:
#
#    alias
#       for each entry in the anon array, add
#       add "#define want_vtbl_$_ want_vtbl_$name"
#
#    cond
#       prefix the vtable with the specified entry (e.g. '#ifdef FOO')
#       and suffix it with '#else { 0, 0, 0, 0, 0, 0, 0, 0 } #endif'
#
#    const
#       special-case cast a 'get' function whose signature expects
#       a pointer to constant magic, so that it can be added to a vtable
#       which expects pointers to functions without the 'const'.
#
#    get
#    set
#    len
#    clear
#    free
#    copy
#    dup
#    local
#       For each specified method, add a vtable function pointer
#       of the form "Perl_magic_$vtable_conf{foo}{get}" etc

my %vtable_conf =
    (
     'sv' => {get => 'get', set => 'set'},
     'env' => {set => 'set_all_env', clear => 'clear_all_env'},
     'envelem' => {set => 'setenv', clear => 'clearenv'},
     'sig' => { set => 'setsigall' },
     'sigelem' => {get => 'getsig', set => 'setsig', clear => 'clearsig',
                   cond => '#ifndef PERL_MICRO'},

     'hook' => { set => 'sethookall', clear => 'clearhookall' },
     'hookelem' => {set => 'sethook', clear => 'clearhook'},

     'pack' => {len => 'sizepack', clear => 'wipepack'},
     'packelem' => {get => 'getpack', set => 'setpack', clear => 'clearpack'},
     'dbline' => {set => 'setdbline'},
     'isa' => {set => 'setisa', clear => 'clearisa'},
     'isaelem' => {set => 'setisa'},
     'arylen' => {get => 'getarylen', set => 'setarylen', const => 1},
     'arylen_p' => {clear => 'cleararylen_p', free => 'freearylen_p'},
     'mglob'    => {set   => 'setmglob',
                    free  => 'freemglob' },
     'nkeys' => {get => 'getnkeys', set => 'setnkeys'},
     'taint' => {get => 'gettaint', set => 'settaint'},
     'substr' => {get => 'getsubstr', set => 'setsubstr'},
     'vec' => {get => 'getvec', set => 'setvec'},
     'pos' => {get => 'getpos', set => 'setpos'},
     'uvar' => {get => 'getuvar', set => 'setuvar'},
     'defelem' => {get => 'getdefelem', set => 'setdefelem'},
     'nonelem' => {set => 'setnonelem'},
     'regexp' => {set => 'setregexp', alias => [qw(bm fm)]},
     'regdata' => {len => 'regdata_cnt'},
     'regdatum' => {get => 'regdatum_get', set => 'regdatum_set'},
     'backref' => {free => 'killbackrefs'},
     'ovrld' => {free => 'freeovrld'},
     'utf8'     => {set   => 'setutf8',
                    free  => 'freeutf8' },
     'collxfrm' => {set => 'setcollxfrm',
                    free => 'freecollxfrm',
                    cond => '#ifdef USE_LOCALE_COLLATE'},
     'hintselem' => {set => 'sethint', clear => 'clearhint'},
     'hints' => {clear => 'clearhints'},
     'checkcall' => {copy => 'copycallchecker'},
     'debugvar' => { set => 'setdebugvar', get => 'getdebugvar' },
     'lvref' => {set => 'setlvref'},
     'destruct' => {free => 'freedestruct'},
);


# END OF CONFIGURATION DATA
#
# =====================================================================



my ($vt, $raw, $names) = map {
    open_new($_, '>',
             { by => 'regen/mg_vtable.pl', file => $_, style => '*' });
} 'mg_vtable.h', 'mg_raw.h', 'mg_names.inc';
my $guts = open_new("pod/perlguts.pod", ">");

print $vt <<'EOH';
/* These constants should be used in preference to raw characters
 * when using magic. Note that some perl guts still assume
 * certain character properties of these constants, namely that
 * isUPPER() and toLOWER() may do useful mappings.
 */

EOH

# Of course, it would be *much* easier if we could output this table directly
# here and now. However, for our sins, we try to support EBCDIC, which wouldn't
# be *so* bad, except that there are (at least) 3 EBCDIC charset variants, and
# they don't agree on the code point for '~'. Which we use. Great.
# So we have to get the local build runtime to sort our table in character order
# (And of course, just to be helpful, in POSIX BC '~' is \xFF, so we can't even
# simplify the C code by assuming that the last element of the array is
# predictable)

# Process %mg

{
    my $longest = 0;
    foreach (keys %mg) {
        $longest = length $_ if length $_ > $longest;
    }

    my $longest_p1 = $longest + 1;

    my %mg_order;
    while (my ($name, $data) = each %mg) {
        my $byte = $data->{char};
        if ($byte =~ /[[:print:]]/) {
            $data->{r_char} = $byte; # readable char
            ($data->{c_char} = $byte) =~ s/([\\"])/\\$1/g; # for C strings
        }
        else {
            $data->{c_char} = $data->{r_char} = '\\'.ord $byte;
        }
        $mg_order{(uc $byte) . $byte} = $name;
    }

    my @rows;
    my @names;
    foreach (sort keys %mg_order) {
        my $name = $mg_order{$_};
        push @names, $name;
        my $data = $mg{$name};
        my $i = ord $data->{char};

        # add entry to mg_raw.h

        unless ($data->{unknown_to_sv_magic}) {
            my $value = $data->{vtable}
                ? "want_vtbl_$data->{vtable}" : 'magic_vtable_max';
            $value .= ' | PERL_MAGIC_READONLY_ACCEPTABLE'
                if $data->{readonly_acceptable};
            $value .= ' | PERL_MAGIC_VALUE_MAGIC' if $data->{value_magic};
            my $comment = "/* $name '$data->{r_char}' $data->{desc} */";
            $comment =~ s/([\\"])/\\$1/g;
            $comment =~ tr/\n/ /;
            print $raw qq{    { '$data->{c_char}', "$value",\n      "$comment" },\n};
        }

        # add #define PERL_MAGIC_foo entry to vt_table.h

        my $comment = $data->{desc};
        my $leader = ' ' x ($longest + 27);
        $comment =~ s/\n/\n$leader/s;
        printf $vt "#define PERL_MAGIC_%-${longest}s '%s' /* %s */\n",
            $name, $data->{c_char}, $comment;

        # add entry to mg_names.inc

        my $char = $data->{r_char};
        $char =~ s/([\\"])/\\$1/g;
        printf $names qq[\t{ PERL_MAGIC_%-${longest_p1}s "%s(%s)" },\n],
            "$name,", $name, $char;

        # construct perlguts.pod entry

        push @rows, [(sprintf "%-2s PERL_MAGIC_%s", $data->{r_char},$name),
                     $data->{vtable} ? "vtbl_$data->{vtable}" : '(none)',
                     $data->{desc}];
    }

    # output @rows to perlguts.pod

    select +(select($guts), do {
        my @header = ('(old-style char and macro)', 'MGVTBL', 'Type of magic');
        my @widths = (0, 0);
        foreach my $row (@rows) {
            for (0, 1) {
                $widths[$_] = length $row->[$_]
                    if length $row->[$_] > $widths[$_];
            }
        }
        my $indent = ' ';
        my $format
            = sprintf "$indent%%-%ds%%-%ds%%s\n", $widths[0] + 1, $widths[1] + 1;
        my $desc_wrap =
            79 - 7 - (length $indent) - $widths[0] - $widths[1] - 2;

        open my $oldguts, "<", "pod/perlguts.pod"
           or die "$0 cannot open pod/perlguts.pod for reading: $!";
        while (<$oldguts>) {
            print;
            last if /^=for mg_vtable.pl begin/
        }

        print "\n", $indent . "mg_type\n";
        printf $format, @header;
        printf $format, map {'-' x length $_} @header;
        foreach (@rows) {
            my ($type, $vtbl, $desc) = @$_;
            $desc =~ tr/\n/ /;
            my @cont;
            if (length $desc > $desc_wrap) {
                # If it's too long, first split on '(', if there.
                # [Which, if there, is always short enough, currently.
                # Make this more robust if that changes]
                ($desc, @cont) = split /(?=\()/, $desc;
                if (!@cont) {
                    ($desc, @cont) = $desc =~ /(.{1,$desc_wrap})(?: |\z)/g
                }
            }
            s/\s+\z// for $desc, @cont;
            printf $format, $type, $vtbl, $desc;
            printf $format, '', '', $_ foreach @cont;
        }
        print "\n\n";

        my $first = 1;
        for my $magic (sort @names) {
            if ($first) {
                $first = 0;
                print "=for apidoc_section \$magic\n";
                print "=for apidoc AmnhU||PERL_MAGIC_$magic\n";
            }
            else {
                print "=for apidoc_item ||PERL_MAGIC_$magic\n";
            }
        }
        print "\n";

        while (<$oldguts>) {
            last if /^=for mg_vtable.pl end/;
        }
        do { print } while <$oldguts>;
    })[0];
}


# Process %vtable_conf - everything goes to mg_vtable.h

my @names = sort keys %vtable_conf;
{
    my $want = join ",\n    ", (map {"want_vtbl_$_"} @names), 'magic_vtable_max';
    my $names = join qq{",\n    "}, @names;

    print $vt <<"EOH";

enum {		/* pass one of these to get_vtbl */
    $want
};

#ifdef DOINIT
EXTCONST char * const PL_magic_vtable_names[magic_vtable_max] = {
    "$names"
};
#else
EXTCONST char * const PL_magic_vtable_names[magic_vtable_max];
#endif

EOH
}

print $vt <<'EOH';
/* These all need to be 0, not NULL, as NULL can be (void*)0, which is a
 * pointer to data, whereas we're assigning pointers to functions, which are
 * not the same beast. ANSI doesn't allow the assignment from one to the other.
 * (although most, but not all, compilers are prepared to do it)
 */

/* order is:
    get
    set
    len
    clear
    free
    copy
    dup
    local
*/

#ifdef DOINIT
EXT_MGVTBL PL_magic_vtables[magic_vtable_max] = {
EOH

my @vtable_names;
my @aliases;

while (my $name = shift @names) {
    my $data = $vtable_conf{$name};
    push @vtable_names, $name;
    my @funcs = map {
        $data->{$_} ? "Perl_magic_$data->{$_}" : 0;
    } qw(get set len clear free copy dup local);

    $funcs[0] = "(int (*)(pTHX_ SV *, MAGIC *))" . $funcs[0] if $data->{const};
    my $funcs = join ", ", @funcs;

    # Because we can't have a , after the last {...}
    my $comma = @names ? ',' : '';

    print $vt "$data->{cond}\n" if $data->{cond};
    print $vt "  { $funcs }$comma\n";
    print $vt <<"EOH" if $data->{cond};
#else
  { 0, 0, 0, 0, 0, 0, 0, 0 }$comma
#endif
EOH
    foreach(@{$data->{alias}}) {
        push @aliases, "#define want_vtbl_$_ want_vtbl_$name\n";
        push @vtable_names, $_;
    }
}

print $vt <<'EOH';
};
#else
EXT_MGVTBL PL_magic_vtables[magic_vtable_max];
#endif

EOH

print $vt (sort @aliases), "\n";

print $vt "#define PL_vtbl_$_ PL_magic_vtables[want_vtbl_$_]\n"
    foreach sort @vtable_names;

# 63, not 64, As we rely on the last possible value to mean "NULL vtable"
die "Too many vtable names" if @vtable_names > 63;

read_only_bottom_close_and_rename($_) foreach $vt, $raw, $names;
                 close_and_rename($guts);
