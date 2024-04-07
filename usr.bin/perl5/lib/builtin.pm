package builtin 0.008;

use strict;
use warnings;

# All code, including &import, is implemented by always-present functions in
# the perl interpreter itself.
# See also `builtin.c` in perl source

1;
__END__

=head1 NAME

builtin - Perl pragma to import built-in utility functions

=head1 SYNOPSIS

    use builtin qw(
        true false is_bool
        weaken unweaken is_weak
        blessed refaddr reftype
        created_as_string created_as_number
        ceil floor
        indexed
        trim
        is_tainted
        export_lexically
    );

=head1 DESCRIPTION

Perl provides several utility functions in the C<builtin> package. These are
plain functions, and look and behave just like regular user-defined functions
do. They do not provide new syntax or require special parsing. These functions
are always present in the interpreter and can be called at any time by their
fully-qualified names. By default they are not available as short names, but
can be requested for convenience.

Individual named functions can be imported by listing them as import
parameters on the C<use> statement for this pragma.

The overall C<builtin> mechanism, as well as every individual function it
provides, are currently B<experimental>.

B<Warning>:  At present, the entire C<builtin> namespace is experimental.
Calling functions in it will trigger warnings of the C<experimental::builtin>
category.

=head2 Lexical Import

This pragma module creates I<lexical> aliases in the currently-compiling scope
to these builtin functions. This is similar to the lexical effect of other
pragmas such as L<strict> and L<feature>.

    sub classify
    {
        my $val = shift;

        use builtin 'is_bool';
        return is_bool($val) ? "boolean" : "not a boolean";
    }

    # the is_bool() function is no longer visible here
    # but may still be called by builtin::is_bool()

Because these functions are imported lexically, rather than by package
symbols, the user does not need to take any special measures to ensure they
don't accidentally appear as object methods from a class.

    package An::Object::Class {
        use builtin 'true', 'false';
        ...
    }

    # does not appear as a method
    An::Object::Class->true;

    # Can't locate object method "true" via package "An::Object::Class"
    #   at ...

=head1 FUNCTIONS

=head2 true

    $val = true;

Returns the boolean truth value. While any scalar value can be tested for
truth and most defined, non-empty and non-zero values are considered "true"
by perl, this one is special in that L</is_bool> considers it to be a
distinguished boolean value.

This gives an equivalent value to expressions like C<!!1> or C<!0>.

=head2 false

    $val = false;

Returns the boolean fiction value. While any non-true scalar value is
considered "false" by perl, this one is special in that L</is_bool> considers
it to be a distinguished boolean value.

This gives an equivalent value to expressions like C<!!0> or C<!1>.

=head2 is_bool

    $bool = is_bool($val);

Returns true when given a distinguished boolean value, or false if not. A
distinguished boolean value is the result of any boolean-returning builtin
function (such as C<true> or C<is_bool> itself), boolean-returning operator
(such as the C<eq> or C<==> comparison tests or the C<!> negation operator),
or any variable containing one of these results.

This function used to be named C<isbool>. A compatibility alias is provided
currently but will be removed in a later version.

=head2 weaken

    weaken($ref);

Weakens a reference. A weakened reference does not contribute to the reference
count of its referent. If only weakened references to a referent remain, it
will be disposed of, and all remaining weak references to it will have their
value set to C<undef>.

=head2 unweaken

    unweaken($ref);

Strengthens a reference, undoing the effects of a previous call to L</weaken>.

=head2 is_weak

    $bool = is_weak($ref);

Returns true when given a weakened reference, or false if not a reference or
not weak.

This function used to be named C<isweak>. A compatibility alias is provided
currently but will be removed in a later version.

=head2 blessed

    $str = blessed($ref);

Returns the package name for an object reference, or C<undef> for a
non-reference or reference that is not an object.

=head2 refaddr

    $num = refaddr($ref);

Returns the memory address for a reference, or C<undef> for a non-reference.
This value is not likely to be very useful for pure Perl code, but is handy as
a means to test for referential identity or uniqueness.

=head2 reftype

    $str = reftype($ref);

Returns the basic container type of the referent of a reference, or C<undef>
for a non-reference. This is returned as a string in all-capitals, such as
C<ARRAY> for array references, or C<HASH> for hash references.

=head2 created_as_string

    $bool = created_as_string($val);

Returns a boolean representing if the argument value was originally created as
a string. It will return true for any scalar expression whose most recent
assignment or modification was of a string-like nature - such as assignment
from a string literal, or the result of a string operation such as
concatenation or regexp. It will return false for references (including any
object), numbers, booleans and undef.

It is unlikely that you will want to use this for regular data validation
within Perl, as it will not return true for regular numbers that are still
perfectly usable as strings, nor for any object reference - especially objects
that overload the stringification operator in an attempt to behave more like
strings. For example

    my $val = URI->new( "https://metacpan.org/" );

    if( created_as_string $val ) { ... }    # this will not execute

=head2 created_as_number

    $bool = created_as_number($val);

Returns a boolean representing if the argument value was originally created as
a number. It will return true for any scalar expression whose most recent
assignment or modification was of a numerical nature - such as assignment from
a number literal, or the result of a numerical operation such as addition. It
will return false for references (including any object), strings, booleans and
undef.

It is unlikely that you will want to use this for regular data validation
within Perl, as it will not return true for regular strings of decimal digits
that are still perfectly usable as numbers, nor for any object reference -
especially objects that overload the numification operator in an attempt to
behave more like numbers. For example

    my $val = Math::BigInt->new( 123 );

    if( created_as_number $val ) { ... }    # this will not execute

While most Perl code should operate on scalar values without needing to know
their creation history, these two functions are intended to be used by data
serialisation modules such as JSON encoders or similar situations, where
language interoperability concerns require making a distinction between values
that are fundamentally stringlike versus numberlike in nature.

=head2 ceil

    $num = ceil($num);

Returns the smallest integer value greater than or equal to the given
numerical argument.

=head2 floor

    $num = floor($num);

Returns the largest integer value less than or equal to the given numerical
argument.

=head2 indexed

    @ivpairs = indexed(@items)

Returns an even-sized list of number/value pairs, where each pair is formed
of a number giving an index in the original list followed by the value at that
position in it.  I.e. returns a list twice the size of the original, being
equal to

    (0, $items[0], 1, $items[1], 2, $items[2], ...)

Note that unlike the core C<values> function, this function returns copies of
its original arguments, not aliases to them. Any modifications of these copies
are I<not> reflected in modifications to the original.

    my @x = ...;
    $_++ for indexed @x;  # The @x array remains unaffected

This function is primarily intended to be useful combined with multi-variable
C<foreach> loop syntax; as

    foreach my ($index, $value) (indexed LIST) {
        ...
    }

In scalar context this function returns the size of the list that it would
otherwise have returned, and provokes a warning in the C<scalar> category.

=head2 trim

    $stripped = trim($string);

Returns the input string with whitespace stripped from the beginning
and end. trim() will remove these characters:

" ", an ordinary space.

"\t", a tab.

"\n", a new line (line feed).

"\r", a carriage return.

and all other Unicode characters that are flagged as whitespace.
A complete list is in L<perlrecharclass/Whitespace>.

    $var = "  Hello world   ";            # "Hello world"
    $var = "\t\t\tHello world";           # "Hello world"
    $var = "Hello world\n";               # "Hello world"
    $var = "\x{2028}Hello world\x{3000}"; # "Hello world"

C<trim> is equivalent to:

    $str =~ s/\A\s+|\s+\z//urg;

For Perl versions where this feature is not available look at the
L<String::Util> module for a comparable implementation.

=head2 is_tainted

    $bool = is_tainted($var);

Returns true when given a tainted variable.

=head2 export_lexically

    export_lexically($name1, $ref1, $name2, $ref2, ...)

Exports new lexical names into the scope currently being compiled. Names given
by the first of each pair of values will refer to the corresponding item whose
reference is given by the second. Types of item that are permitted are
subroutines, and scalar, array, and hash variables. If the item is a
subroutine, the name may optionally be prefixed with the C<&> sigil, but for
convenience it doesn't have to. For items that are variables the sigil is
required, and must match the type of the variable.

    export_lexically func    => \&func,
                     '&func' => \&func;  # same as above

    export_lexically '$scalar' => \my $var;

Z<>

    # The following are not permitted
    export_lexically '$var' => \@arr;   # sigil does not match
    export_lexically name => \$scalar;  # implied '&' sigil does not match

    export_lexically '*name' => \*globref;  # globrefs are not supported

This must be called at compile time; which typically means during a C<BEGIN>
block. Usually this would be used as part of an C<import> method of a module,
when invoked as part of a C<use ...> statement.

=head1 SEE ALSO

L<perlop>, L<perlfunc>, L<Scalar::Util>
