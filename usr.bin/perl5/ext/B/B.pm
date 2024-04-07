#      B.pm
#
#      Copyright (c) 1996, 1997, 1998 Malcolm Beattie
#
#      You may distribute under the terms of either the GNU General Public
#      License or the Artistic License, as specified in the README file.
#
package B;

@B::ISA = qw(Exporter);

# If B is loaded without imports, we do not want to unnecessarily pollute the stash with Exporter.
sub import {
    return unless scalar @_ > 1; # Called as a method call.
    require Exporter;
    B->export_to_level(1, @_);
}

# walkoptree_slow comes from B.pm (you are there),
# walkoptree comes from B.xs

BEGIN {
    $B::VERSION = '1.88';
    @B::EXPORT_OK = ();

    # Our BOOT code needs $VERSION set, and will append to @EXPORT_OK.
    # Want our constants loaded before the compiler meets OPf_KIDS below, as
    # the combination of having the constant stay a Proxy Constant Subroutine
    # and its value being inlined saves a little over .5K

    require XSLoader;
    XSLoader::load();
}

push @B::EXPORT_OK, (qw(minus_c ppname save_BEGINs
			class peekop cast_I32 cstring cchar hash threadsv_names
			main_root main_start main_cv svref_2object opnumber
			sub_generation amagic_generation perlstring
			walkoptree_slow walkoptree walkoptree_exec walksymtable
			parents comppadlist sv_undef compile_stats timing_info
			begin_av init_av check_av end_av regex_padav dowarn
			defstash curstash warnhook diehook inc_gv @optype
			@specialsv_name unitcheck_av safename));

@B::SV::ISA = 'B::OBJECT';
@B::NULL::ISA = 'B::SV';
@B::PV::ISA = 'B::SV';
@B::IV::ISA = 'B::SV';
@B::NV::ISA = 'B::SV';
# RV is eliminated with 5.11.0, but effectively is a specialisation of IV now.
@B::RV::ISA = 'B::IV';
@B::PVIV::ISA = qw(B::PV B::IV);
@B::PVNV::ISA = qw(B::PVIV B::NV);
@B::PVMG::ISA = 'B::PVNV';
@B::REGEXP::ISA = 'B::PVMG';
@B::INVLIST::ISA = 'B::PV';
@B::PVLV::ISA = 'B::GV';
@B::BM::ISA = 'B::GV';
@B::AV::ISA = 'B::PVMG';
@B::GV::ISA = 'B::PVMG';
@B::HV::ISA = 'B::PVMG';
@B::CV::ISA = 'B::PVMG';
@B::IO::ISA = 'B::PVMG';
@B::FM::ISA = 'B::CV';
@B::OBJ::ISA = 'B::PVMG';

@B::OP::ISA = 'B::OBJECT';
@B::UNOP::ISA = 'B::OP';
@B::UNOP_AUX::ISA = 'B::UNOP';
@B::BINOP::ISA = 'B::UNOP';
@B::LOGOP::ISA = 'B::UNOP';
@B::LISTOP::ISA = 'B::BINOP';
@B::SVOP::ISA = 'B::OP';
@B::PADOP::ISA = 'B::OP';
@B::PVOP::ISA = 'B::OP';
@B::LOOP::ISA = 'B::LISTOP';
@B::PMOP::ISA = 'B::LISTOP';
@B::COP::ISA = 'B::OP';
@B::METHOP::ISA = 'B::OP';

@B::SPECIAL::ISA = 'B::OBJECT';

our @optype = qw(OP UNOP BINOP LOGOP LISTOP PMOP SVOP PADOP PVOP LOOP COP
                METHOP UNOP_AUX);
# bytecode.pl contained the following comment:
# Nullsv *must* come first in the following so that the condition
# ($$sv == 0) can continue to be used to test (sv == Nullsv).
our @specialsv_name = qw(Nullsv &PL_sv_undef &PL_sv_yes &PL_sv_no
			(SV*)pWARN_ALL (SV*)pWARN_NONE (SV*)pWARN_STD
                        &PL_sv_zero);

# Back-compat
{
    no warnings 'once';
    *CVf_METHOD = \&CVf_NOWARN_AMBIGUOUS;
}

{
    # Stop "-w" from complaining about the lack of a real B::OBJECT class
    package B::OBJECT;
}

sub B::GV::SAFENAME {
  safename(shift()->NAME);
}

sub safename {
  my $name = shift;

  # The regex below corresponds to the isCONTROLVAR macro
  # from toke.c

  $name =~ s/^\c?/^?/
    or $name =~ s/^([\cA-\cZ\c\\c[\c]\c_\c^])/
                "^" .  chr( utf8::unicode_to_native( 64 ^ ord($1) ))/e;

  # When we say unicode_to_native we really mean ascii_to_native,
  # which matters iff this is a non-ASCII platform (EBCDIC).  '\c?' would
  # not have to be special cased, except for non-ASCII.

  return $name;
}

sub B::IV::int_value {
  my ($self) = @_;
  return (($self->FLAGS() & SVf_IVisUV()) ? $self->UVX : $self->IV);
}

sub B::NULL::as_string() {""}
*B::IV::as_string = *B::IV::as_string = \*B::IV::int_value;
*B::PV::as_string = *B::PV::as_string = \*B::PV::PV;

#  The input typemap checking makes no distinction between different SV types,
#  so the XS body will generate the same C code, despite the different XS
#  "types". So there is no change in behaviour from doing "newXS" like this,
#  compared with the old approach of having a (near) duplicate XS body.
#  We should fix the typemap checking.

#  Since perl 5.12.0
*B::IV::RV = *B::IV::RV = \*B::PV::RV;

my $debug;
my $op_count = 0;
my @parents = ();

sub debug {
    my ($class, $value) = @_;
    $debug = $value;
    walkoptree_debug($value);
}

sub class {
    my $obj = shift;
    my $name = ref $obj;
    $name =~ s/^.*:://;
    return $name;
}

sub parents { \@parents }

# For debugging
sub peekop {
    my $op = shift;
    return sprintf("%s (0x%x) %s", class($op), $$op, $op->name);
}

sub walkoptree_slow {
    my($op, $method, $level) = @_;
    $op_count++; # just for statistics
    $level ||= 0;
    warn(sprintf("walkoptree: %d. %s\n", $level, peekop($op))) if $debug;
    $op->$method($level) if $op->can($method);
    if ($$op && ($op->flags & OPf_KIDS)) {
	my $kid;
	unshift(@parents, $op);
	for ($kid = $op->first; $$kid; $kid = $kid->sibling) {
	    walkoptree_slow($kid, $method, $level + 1);
	}
	shift @parents;
    }
    if (class($op) eq 'PMOP'
	&& ref($op->pmreplroot)
	&& ${$op->pmreplroot}
	&& $op->pmreplroot->isa( 'B::OP' ))
    {
	unshift(@parents, $op);
	walkoptree_slow($op->pmreplroot, $method, $level + 1);
	shift @parents;
    }
}

sub compile_stats {
    return "Total number of OPs processed: $op_count\n";
}

sub timing_info {
    my ($sec, $min, $hr) = localtime;
    my ($user, $sys) = times;
    sprintf("%02d:%02d:%02d user=$user sys=$sys",
	    $hr, $min, $sec, $user, $sys);
}

my %symtable;

sub clearsym {
    %symtable = ();
}

sub savesym {
    my ($obj, $value) = @_;
#    warn(sprintf("savesym: sym_%x => %s\n", $$obj, $value)); # debug
    $symtable{sprintf("sym_%x", $$obj)} = $value;
}

sub objsym {
    my $obj = shift;
    return $symtable{sprintf("sym_%x", $$obj)};
}

sub walkoptree_exec {
    my ($op, $method, $level) = @_;
    $level ||= 0;
    my ($sym, $ppname);
    my $prefix = "    " x $level;
    for (; $$op; $op = $op->next) {
	$sym = objsym($op);
	if (defined($sym)) {
	    print $prefix, "goto $sym\n";
	    return;
	}
	savesym($op, sprintf("%s (0x%lx)", class($op), $$op));
	$op->$method($level);
	$ppname = $op->name;
	if ($ppname =~
	    /^(d?or(assign)?|and(assign)?|mapwhile|grepwhile|entertry|range|cond_expr)$/)
	{
	    print $prefix, uc($1), " => {\n";
	    walkoptree_exec($op->other, $method, $level + 1);
	    print $prefix, "}\n";
	} elsif ($ppname eq "match" || $ppname eq "subst") {
	    my $pmreplstart = $op->pmreplstart;
	    if ($$pmreplstart) {
		print $prefix, "PMREPLSTART => {\n";
		walkoptree_exec($pmreplstart, $method, $level + 1);
		print $prefix, "}\n";
	    }
	} elsif ($ppname eq "substcont") {
	    print $prefix, "SUBSTCONT => {\n";
	    walkoptree_exec($op->other->pmreplstart, $method, $level + 1);
	    print $prefix, "}\n";
	    $op = $op->other;
	} elsif ($ppname eq "enterloop") {
	    print $prefix, "REDO => {\n";
	    walkoptree_exec($op->redoop, $method, $level + 1);
	    print $prefix, "}\n", $prefix, "NEXT => {\n";
	    walkoptree_exec($op->nextop, $method, $level + 1);
	    print $prefix, "}\n", $prefix, "LAST => {\n";
	    walkoptree_exec($op->lastop,  $method, $level + 1);
	    print $prefix, "}\n";
	} elsif ($ppname eq "subst") {
	    my $replstart = $op->pmreplstart;
	    if ($$replstart) {
		print $prefix, "SUBST => {\n";
		walkoptree_exec($replstart, $method, $level + 1);
		print $prefix, "}\n";
	    }
	}
    }
}

sub walksymtable {
    my ($symref, $method, $recurse, $prefix) = @_;
    my $sym;
    my $fullname;
    no strict 'refs';
    $prefix = '' unless defined $prefix;
    foreach my $sym ( sort keys %$symref ) {
        my $dummy = $symref->{$sym}; # Copying the glob and incrementing
                                     # the GPs refcnt clears cached methods
        $fullname = "*main::".$prefix.$sym;
	if ($sym =~ /::$/) {
	    $sym = $prefix . $sym;
	    if (svref_2object(\*$sym)->NAME ne "main::" && $sym ne "<none>::" && &$recurse($sym)) {
               walksymtable(\%$fullname, $method, $recurse, $sym);
	    }
	} else {
           svref_2object(\*$fullname)->$method();
	}
    }
}

1;

__END__

=head1 NAME

B - The Perl Compiler Backend

=head1 SYNOPSIS

	use B;

=head1 DESCRIPTION

The C<B> module supplies classes which allow a Perl program to delve
into its own innards.  It is the module used to implement the
"backends" of the Perl compiler.  Usage of the compiler does not
require knowledge of this module: see the L<O> module for the
user-visible part.  The C<B> module is of use to those who want to
write new compiler backends.  This documentation assumes that the
reader knows a fair amount about perl's internals including such
things as SVs, OPs and the internal symbol table and syntax tree
of a program.

=head1 OVERVIEW

The C<B> module contains a set of utility functions for querying the
current state of the Perl interpreter; typically these functions
return objects from the B::SV and B::OP classes, or their derived
classes.  These classes in turn define methods for querying the
resulting objects about their own internal state.

=head1 Utility Functions

The C<B> module exports a variety of functions: some are simple
utility functions, others provide a Perl program with a way to
get an initial "handle" on an internal object.

=head2 Functions Returning C<B::SV>, C<B::AV>, C<B::HV>, and C<B::CV> objects

For descriptions of the class hierarchy of these objects and the
methods that can be called on them, see below, L<"OVERVIEW OF
CLASSES"> and L<"SV-RELATED CLASSES">.

=over 4

=item sv_undef

Returns the SV object corresponding to the C variable C<sv_undef>.

=item sv_yes

Returns the SV object corresponding to the C variable C<sv_yes>.

=item sv_no

Returns the SV object corresponding to the C variable C<sv_no>.

=item svref_2object(SVREF)

Takes a reference to any Perl value, and turns the referred-to value
into an object in the appropriate B::OP-derived or B::SV-derived
class.  Apart from functions such as C<main_root>, this is the primary
way to get an initial "handle" on an internal perl data structure
which can then be followed with the other access methods.

The returned object will only be valid as long as the underlying OPs
and SVs continue to exist.  Do not attempt to use the object after the
underlying structures are freed.

=item amagic_generation

Returns the SV object corresponding to the C variable C<amagic_generation>.
As of Perl 5.18, this is just an alias to C<PL_na>, so its value is
meaningless.

=item init_av

Returns the AV object (i.e. in class B::AV) representing INIT blocks.

=item check_av

Returns the AV object (i.e. in class B::AV) representing CHECK blocks.

=item unitcheck_av

Returns the AV object (i.e. in class B::AV) representing UNITCHECK blocks.

=item begin_av

Returns the AV object (i.e. in class B::AV) representing BEGIN blocks.

=item end_av

Returns the AV object (i.e. in class B::AV) representing END blocks.

=item comppadlist

Returns the PADLIST object (i.e. in class B::PADLIST) of the global
comppadlist.  In Perl 5.16 and earlier it returns an AV object (class
B::AV).

=item regex_padav

Only when perl was compiled with ithreads.

=item main_cv

Return the (faked) CV corresponding to the main part of the Perl
program.

=back

=head2 Functions for Examining the Symbol Table

=over 4

=item walksymtable(SYMREF, METHOD, RECURSE, PREFIX)

Walk the symbol table starting at SYMREF and call METHOD on each
symbol (a B::GV object) visited.  When the walk reaches package
symbols (such as "Foo::") it invokes RECURSE, passing in the symbol
name, and only recurses into the package if that sub returns true.

PREFIX is the name of the SYMREF you're walking.

For example:

  # Walk CGI's symbol table calling print_subs on each symbol.
  # Recurse only into CGI::Util::
  walksymtable(\%CGI::, 'print_subs',
               sub { $_[0] eq 'CGI::Util::' }, 'CGI::');

print_subs() is a B::GV method you have declared.  Also see L<"B::GV
Methods">, below.

=back

=head2 Functions Returning C<B::OP> objects or for walking op trees

For descriptions of the class hierarchy of these objects and the
methods that can be called on them, see below, L<"OVERVIEW OF
CLASSES"> and L<"OP-RELATED CLASSES">.

=over 4

=item main_root

Returns the root op (i.e. an object in the appropriate B::OP-derived
class) of the main part of the Perl program.

=item main_start

Returns the starting op of the main part of the Perl program.

=item walkoptree(OP, METHOD)

Does a tree-walk of the syntax tree based at OP and calls METHOD on
each op it visits.  Each node is visited before its children.  If
C<walkoptree_debug> (see below) has been called to turn debugging on then
the method C<walkoptree_debug> is called on each op before METHOD is
called.

=item walkoptree_debug(DEBUG)

Returns the current debugging flag for C<walkoptree>.  If the optional
DEBUG argument is non-zero, it sets the debugging flag to that.  See
the description of C<walkoptree> above for what the debugging flag
does.

=back

=head2 Miscellaneous Utility Functions

=over 4

=item ppname(OPNUM)

Return the PP function name (e.g. "pp_add") of op number OPNUM.

=item hash(STR)

Returns a string in the form "0x..." representing the value of the
internal hash function used by perl on string STR.

=item cast_I32(I)

Casts I to the internal I32 type used by that perl.

=item minus_c

Does the equivalent of the C<-c> command-line option.  Obviously, this
is only useful in a BEGIN block or else the flag is set too late.

=item cstring(STR)

Returns a double-quote-surrounded escaped version of STR which can
be used as a string in C source code.

=item perlstring(STR)

Returns a double-quote-surrounded escaped version of STR which can
be used as a string in Perl source code.

=item safename(STR)

This function returns the string with the first character modified if it
is a control character.  It converts it to ^X format first, so that "\cG"
becomes "^G".  This is used internally by L<B::GV::SAFENAME|/SAFENAME>, but
you can call it directly.

=item class(OBJ)

Returns the class of an object without the part of the classname
preceding the first C<"::">.  This is used to turn C<"B::UNOP"> into
C<"UNOP"> for example.

=item threadsv_names

This used to provide support for the old 5.005 threading module. It now
does nothing.

=back

=head2 Exported utility variables

=over 4

=item @optype

  my $op_type = $optype[$op_type_num];

A simple mapping of the op type number to its type (like 'COP' or 'BINOP').

=item @specialsv_name

  my $sv_name = $specialsv_name[$sv_index];

Certain SV types are considered 'special'.  They're represented by
B::SPECIAL and are referred to by a number from the specialsv_list.
This array maps that number back to the name of the SV (like 'Nullsv'
or '&PL_sv_undef').

=back


=head1 OVERVIEW OF CLASSES

The C structures used by Perl's internals to hold SV and OP
information (PVIV, AV, HV, ..., OP, SVOP, UNOP, ...) are modelled on a
class hierarchy and the C<B> module gives access to them via a true
object hierarchy.  Structure fields which point to other objects
(whether types of SV or types of OP) are represented by the C<B>
module as Perl objects of the appropriate class.

The bulk of the C<B> module is the methods for accessing fields of
these structures.

Note that all access is read-only.  You cannot modify the internals by
using this module.  Also, note that the B::OP and B::SV objects created
by this module are only valid for as long as the underlying objects
exist; their creation doesn't increase the reference counts of the
underlying objects.  Trying to access the fields of a freed object will
give incomprehensible results, or worse.

=head2 SV-RELATED CLASSES

B::IV, B::NV, B::PV, B::PVIV, B::PVNV, B::PVMG,
B::PVLV, B::AV, B::HV, B::CV, B::GV, B::FM, B::IO.  These classes
correspond in the obvious way to the underlying C structures of similar names.
The inheritance hierarchy mimics the underlying C "inheritance":

                           B::SV
                             |
                +------------+------------+
                |            |            |
              B::PV        B::IV        B::NV
               /  \         /           /
              /    \       /           /
        B::INVLIST  B::PVIV           /
                         \           /
                          \         /
                           \       /
                            B::PVNV
                               |
                               |
                            B::PVMG
                               |
           +-------+-------+---+---+-------+-------+
           |       |       |       |       |       |
         B::AV   B::GV   B::HV   B::CV   B::IO B::REGEXP
                   |               |
                   |               |
                B::PVLV          B::FM


Access methods correspond to the underlying C macros for field access,
usually with the leading "class indication" prefix removed (Sv, Av,
Hv, ...).  The leading prefix is only left in cases where its removal
would cause a clash in method name.  For example, C<GvREFCNT> stays
as-is since its abbreviation would clash with the "superclass" method
C<REFCNT> (corresponding to the C function C<SvREFCNT>).

=head2 B::SV Methods

=over 4

=item REFCNT

=item FLAGS

=item IsBOOL

Returns true if the SV is a boolean (true or false).
You can then use C<TRUE> to check if the value is true or false.

    my $something = ( 1 == 1 ) # boolean true
                 || ( 1 == 0 ) # boolean false
                 || 42         # IV true
                 || 0;         # IV false
    my $sv = B::svref_2object(\$something);

    say q[Not a boolean value]
        if ! $sv->IsBOOL;

    say q[This is a boolean with value: true]
        if   $sv->IsBOOL && $sv->TRUE_nomg;

    say q[This is a boolean with value: false]
        if   $sv->IsBOOL && ! $sv->TRUE_nomg;

=item object_2svref

Returns a reference to the regular scalar corresponding to this
B::SV object.  In other words, this method is the inverse operation
to the svref_2object() subroutine.  This scalar and other data it points
at should be considered read-only: modifying them is neither safe nor
guaranteed to have a sensible effect.

=item TRUE

Returns a boolean indicating hether Perl would evaluate the SV as true or
false.

B<Warning> this call performs 'get' magic. If you only want to check the
nature of this SV use C<TRUE_nomg> helper.

This is an alias for C<SvTRUE($sv)>.

=item TRUE_nomg

Check if the value is true (do not perform 'get' magic).
Returns a boolean indicating whether Perl would evaluate the SV as true or
false.

This is an alias for C<SvTRUE_nomg($sv)>.

=back

=head2 B::IV Methods

=over 4

=item IV

Returns the value of the IV, I<interpreted as
a signed integer>.  This will be misleading
if C<FLAGS & SVf_IVisUV>.  Perhaps you want the
C<int_value> method instead?

=item IVX

=item UVX

=item int_value

This method returns the value of the IV as an integer.
It differs from C<IV> in that it returns the correct
value regardless of whether it's stored signed or
unsigned.

=item needs64bits

=item packiv

=back

=head2 B::NV Methods

=over 4

=item NV

=item NVX

=item COP_SEQ_RANGE_LOW

=item COP_SEQ_RANGE_HIGH

These last two are only valid for pad name SVs.  They only existed in the
B::NV class before Perl 5.22.  In 5.22 they were moved to the B::PADNAME
class.

=back

=head2 B::RV Methods

=over 4

=item RV

=back

=head2 B::PV Methods

=over 4

=item PV

This method is the one you usually want.  It constructs a
string using the length and offset information in the struct:
for ordinary scalars it will return the string that you'd see
from Perl, even if it contains null characters.

=item RV

Same as B::RV::RV, except that it will die() if the PV isn't
a reference.

=item PVX

This method is less often useful.  It assumes that the string
stored in the struct is null-terminated, and disregards the
length information.

It is the appropriate method to use if you need to get the name
of a lexical variable from a padname array.  Lexical variable names
are always stored with a null terminator, and the length field
(CUR) is overloaded for other purposes and can't be relied on here.

=item CUR

This method returns the internal length field, which consists of the number
of internal bytes, not necessarily the number of logical characters.

=item LEN

This method returns the number of bytes allocated (via malloc) for storing
the string.  This is 0 if the scalar does not "own" the string.

=back

=head2 B::PVMG Methods

=over 4

=item MAGIC

=item SvSTASH

=back

=head2 B::MAGIC Methods

=over 4

=item MOREMAGIC

=item precomp

Only valid on r-magic, returns the string that generated the regexp.

=item PRIVATE

=item TYPE

=item FLAGS

=item OBJ

Will die() if called on r-magic.

=item PTR

=item REGEX

Only valid on r-magic, returns the integer value of the REGEX stored
in the MAGIC.

=back

=head2 B::INVLIST Methods

=over 4

=item prev_index

Returns the cache result of previous invlist_search() (internal usage)

=item is_offset

Returns a boolean value (0 or 1) to know if the invlist is using an offset.
When false the list begins with the code point U+0000.
When true the list begins with the following elements.

=item array_len

Returns an integer with the size of the array used to define the invlist.

=item get_invlist_array

This method returns a list of integers representing the array used by the
invlist.
Note: this cannot be used while in middle of iterating on an invlist and croaks.

=back

=head2 B::PVLV Methods

=over 4

=item TARGOFF

=item TARGLEN

=item TYPE

=item TARG

=back

=head2 B::BM Methods

=over 4

=item USEFUL

=item PREVIOUS

=item RARE

=item TABLE

=back

=head2 B::REGEXP Methods

=over 4

=item REGEX

=item precomp

=item qr_anoncv

=item compflags

The last two were added in Perl 5.22.

=back

=head2 B::GV Methods

=over 4

=item is_empty

This method returns TRUE if the GP field of the GV is NULL.

=item NAME

=item SAFENAME

This method returns the name of the glob, but if the first
character of the name is a control character, then it converts
it to ^X first, so that *^G would return "^G" rather than "\cG".

It's useful if you want to print out the name of a variable.
If you restrict yourself to globs which exist at compile-time
then the result ought to be unambiguous, because code like
C<${"^G"} = 1> is compiled as two ops - a constant string and
a dereference (rv2gv) - so that the glob is created at runtime.

If you're working with globs at runtime, and need to disambiguate
*^G from *{"^G"}, then you should use the raw NAME method.

=item STASH

=item SV

=item IO

=item FORM

=item AV

=item HV

=item EGV

=item CV

=item CVGEN

=item LINE

=item FILE

=item FILEGV

=item GvREFCNT

=item FLAGS

=item GPFLAGS

This last one is present only in perl 5.22.0 and higher.

=back

=head2 B::IO Methods

B::IO objects derive from IO objects and you will get more information from
the IO object itself.

For example:

  $gvio = B::svref_2object(\*main::stdin)->IO;
  $IO = $gvio->object_2svref();
  $fd = $IO->fileno();

=over 4

=item LINES

=item PAGE

=item PAGE_LEN

=item LINES_LEFT

=item TOP_NAME

=item TOP_GV

=item FMT_NAME

=item FMT_GV

=item BOTTOM_NAME

=item BOTTOM_GV

=item SUBPROCESS

=item IoTYPE

A character symbolizing the type of IO Handle.

  -     STDIN/OUT
  I     STDIN/OUT/ERR
  <     read-only
  >     write-only
  a     append
  +     read and write
  s     socket
  |     pipe
  I     IMPLICIT
  #     NUMERIC
  space closed handle
  \0    closed internal handle

=item IoFLAGS

=item IsSTD

Takes one argument ( 'stdin' | 'stdout' | 'stderr' ) and returns true
if the IoIFP of the object is equal to the handle whose name was
passed as argument; i.e., $io->IsSTD('stderr') is true if
IoIFP($io) == PerlIO_stderr().

=back

=head2 B::AV Methods

=over 4

=item FILL

=item MAX

=item ARRAY

=item ARRAYelt

Like C<ARRAY>, but takes an index as an argument to get only one element,
rather than a list of all of them.

=back

=head2 B::CV Methods

=over 4

=item STASH

=item START

=item ROOT

=item GV

=item FILE

=item DEPTH

=item PADLIST

Returns a B::PADLIST object.

=item OUTSIDE

=item OUTSIDE_SEQ

=item XSUB

=item XSUBANY

For constant subroutines, returns the constant SV returned by the subroutine.

=item CvFLAGS

=item const_sv

=item NAME_HEK

Returns the name of a lexical sub, otherwise C<undef>.

=back

=head2 B::HV Methods

=over 4

=item FILL

=item MAX

=item KEYS

=item RITER

=item NAME

=item ARRAY

=back

=head2 OP-RELATED CLASSES

C<B::OP>, C<B::UNOP>, C<B::UNOP_AUX>, C<B::BINOP>, C<B::LOGOP>,
C<B::LISTOP>, C<B::PMOP>, C<B::SVOP>, C<B::PADOP>, C<B::PVOP>, C<B::LOOP>,
C<B::COP>, C<B::METHOP>.

These classes correspond in the obvious way to the underlying C
structures of similar names.  The inheritance hierarchy mimics the
underlying C "inheritance":

                                 B::OP
                                   |
                   +----------+---------+--------+-------+---------+
                   |          |         |        |       |         |
                B::UNOP    B::SVOP  B::PADOP  B::COP  B::PVOP  B::METHOP
                   |
               +---+---+---------+
               |       |         |
           B::BINOP  B::LOGOP  B::UNOP_AUX
               |
               |
           B::LISTOP
               |
           +---+---+
           |       |
        B::LOOP   B::PMOP

Access methods correspond to the underlying C structure field names,
with the leading "class indication" prefix (C<"op_">) removed.

=head2 B::OP Methods

These methods get the values of similarly named fields within the OP
data structure.  See top of C<op.h> for more info.

=over 4

=item next

=item sibling

=item parent

Returns the OP's parent. If it has no parent, or if your perl wasn't built
with C<-DPERL_OP_PARENT>, returns NULL.

Note that the global variable C<$B::OP::does_parent> is undefined on older
perls that don't support the C<parent> method, is defined but false on
perls that support the method but were built without  C<-DPERL_OP_PARENT>,
and is true otherwise.

=item name

This returns the op name as a string (e.g. "add", "rv2av").

=item ppaddr

This returns the function name as a string (e.g. "PL_ppaddr[OP_ADD]",
"PL_ppaddr[OP_RV2AV]").

=item desc

This returns the op description from the global C PL_op_desc array
(e.g. "addition" "array deref").

=item targ

=item type

=item opt

=item flags

=item private

=item spare

=back

=head2 B::UNOP Method

=over 4

=item first

=back

=head2 B::UNOP_AUX Methods (since 5.22)

=over 4

=item aux_list(cv)

This returns a list of the elements of the op's aux data structure,
or a null list if there is no aux. What will be returned depends on the
object's type, but will typically be a collection of C<B::IV>, C<B::GV>,
etc. objects. C<cv> is the C<B::CV> object representing the sub that the
op is contained within.

=item string(cv)

This returns a textual representation of the object (likely to b useful
for deparsing and debugging), or an empty string if the op type doesn't
support this. C<cv> is the C<B::CV> object representing the sub that the
op is contained within.

=back

=head2 B::BINOP Method

=over 4

=item last

=back

=head2 B::LOGOP Method

=over 4

=item other

=back

=head2 B::LISTOP Method

=over 4

=item children

=back

=head2 B::PMOP Methods

=over 4

=item pmreplroot

=item pmreplstart

=item pmflags

=item precomp

=item pmoffset

Only when perl was compiled with ithreads.

=item code_list

Since perl 5.17.1

=item pmregexp

Added in perl 5.22, this method returns the B::REGEXP associated with the
op.  While PMOPs do not actually have C<pmregexp> fields under threaded
builds, this method returns the regexp under threads nonetheless, for
convenience.

=back

=head2 B::SVOP Methods

=over 4

=item sv

=item gv

=back

=head2 B::PADOP Method

=over 4

=item padix

=back

=head2 B::PVOP Method

=over 4

=item pv

=back

=head2 B::LOOP Methods

=over 4

=item redoop

=item nextop

=item lastop

=back

=head2 B::COP Methods

The C<B::COP> class is used for "nextstate" and "dbstate" ops.  As of Perl
5.22, it is also used for "null" ops that started out as COPs.

=over 4

=item label

=item stash

=item stashpv

=item stashoff (threaded only)

=item file

=item cop_seq

=item line

=item warnings

=item io

=item hints

=item hints_hash

=back

=head2 B::METHOP Methods (Since Perl 5.22)

=over 4

=item first

=item meth_sv

=back

=head2 PAD-RELATED CLASSES

Perl 5.18 introduced a new class, B::PADLIST, returned by B::CV's
C<PADLIST> method.

Perl 5.22 introduced the B::PADNAMELIST and B::PADNAME classes.

=head2 B::PADLIST Methods

=over 4

=item MAX

=item ARRAY

A list of pads.  The first one is a B::PADNAMELIST containing the names.
The rest are currently B::AV objects, but that could
change in future versions.

=item ARRAYelt

Like C<ARRAY>, but takes an index as an argument to get only one element,
rather than a list of all of them.

=item NAMES

This method, introduced in 5.22, returns the B::PADNAMELIST.  It is
equivalent to C<ARRAYelt> with a 0 argument.

=item REFCNT

=item id

This method, introduced in 5.22, returns an ID shared by clones of the same
padlist.

=item outid

This method, also added in 5.22, returns the ID of the outer padlist.

=back

=head2 B::PADNAMELIST Methods

=over 4

=item MAX

=item ARRAY

=item ARRAYelt

These two methods return the pad names, using B::SPECIAL objects for null
pointers and B::PADNAME objects otherwise.

=item REFCNT

=back

=head2 B::PADNAME Methods

=over 4

=item PV

=item PVX

=item LEN

=item REFCNT

=item GEN

=item FLAGS

For backward-compatibility, if the PADNAMEt_OUTER flag is set, the FLAGS
method adds the SVf_FAKE flag, too.

=item TYPE

A B::HV object representing the stash for a typed lexical.

=item SvSTASH

A backward-compatibility alias for TYPE.

=item OURSTASH

A B::HV object representing the stash for 'our' variables.

=item PROTOCV

The prototype CV for a 'my' sub.

=item COP_SEQ_RANGE_LOW

=item COP_SEQ_RANGE_HIGH

Sequence numbers representing the scope within which a lexical is visible.
Meaningless if PADNAMEt_OUTER is set.

=item PARENT_PAD_INDEX

Only meaningful if PADNAMEt_OUTER is set.

=item PARENT_FAKELEX_FLAGS

Only meaningful if PADNAMEt_OUTER is set.

=item IsUndef

Returns a boolean value to check if the padname is PL_padname_undef.

=back

=head2 $B::overlay

Although the optree is read-only, there is an overlay facility that allows
you to override what values the various B::*OP methods return for a
particular op. C<$B::overlay> should be set to reference a two-deep hash:
indexed by OP address, then method name. Whenever a an op method is
called, the value in the hash is returned if it exists. This facility is
used by B::Deparse to "undo" some optimisations. For example:


    local $B::overlay = {};
    ...
    if ($op->name eq "foo") {
        $B::overlay->{$$op} = {
                name => 'bar',
                next => $op->next->next,
        };
    }
    ...
    $op->name # returns "bar"
    $op->next # returns the next op but one


=head1 AUTHOR

Malcolm Beattie, C<mbeattie@sable.ox.ac.uk>

=cut
