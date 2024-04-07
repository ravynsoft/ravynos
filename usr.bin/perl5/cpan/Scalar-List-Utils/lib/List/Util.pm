# Copyright (c) 1997-2009 Graham Barr <gbarr@pobox.com>. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.
#
# Maintained since 2013 by Paul Evans <leonerd@leonerd.org.uk>

package List::Util;

use strict;
use warnings;
require Exporter;

our @ISA        = qw(Exporter);
our @EXPORT_OK  = qw(
  all any first min max minstr maxstr none notall product reduce reductions sum sum0
  sample shuffle uniq uniqint uniqnum uniqstr zip zip_longest zip_shortest mesh mesh_longest mesh_shortest
  head tail pairs unpairs pairkeys pairvalues pairmap pairgrep pairfirst
);
our $VERSION    = "1.63";
our $XS_VERSION = $VERSION;
$VERSION =~ tr/_//d;

require XSLoader;
XSLoader::load('List::Util', $XS_VERSION);

# Used by shuffle()
our $RAND;

sub import
{
  my $pkg = caller;

  # (RT88848) Touch the caller's $a and $b, to avoid the warning of
  #   Name "main::a" used only once: possible typo" warning
  no strict 'refs';
  ${"${pkg}::a"} = ${"${pkg}::a"};
  ${"${pkg}::b"} = ${"${pkg}::b"};

  goto &Exporter::import;
}

# For objects returned by pairs()
sub List::Util::_Pair::key   { shift->[0] }
sub List::Util::_Pair::value { shift->[1] }
sub List::Util::_Pair::TO_JSON { [ @{+shift} ] }

=head1 NAME

List::Util - A selection of general-utility list subroutines

=head1 SYNOPSIS

    use List::Util qw(
      reduce any all none notall first reductions

      max maxstr min minstr product sum sum0

      pairs unpairs pairkeys pairvalues pairfirst pairgrep pairmap

      shuffle uniq uniqint uniqnum uniqstr zip mesh
    );

=head1 DESCRIPTION

C<List::Util> contains a selection of subroutines that people have expressed
would be nice to have in the perl core, but the usage would not really be high
enough to warrant the use of a keyword, and the size so small such that being
individual extensions would be wasteful.

By default C<List::Util> does not export any subroutines.

=cut

=head1 LIST-REDUCTION FUNCTIONS

The following set of functions all apply a given block of code to a list of
values.

=cut

=head2 reduce

    $result = reduce { BLOCK } @list

Reduces C<@list> by calling C<BLOCK> in a scalar context multiple times,
setting C<$a> and C<$b> each time. The first call will be with C<$a> and C<$b>
set to the first two elements of the list, subsequent calls will be done by
setting C<$a> to the result of the previous call and C<$b> to the next element
in the list.

Returns the result of the last call to the C<BLOCK>. If C<@list> is empty then
C<undef> is returned. If C<@list> only contains one element then that element
is returned and C<BLOCK> is not executed.

The following examples all demonstrate how C<reduce> could be used to implement
the other list-reduction functions in this module. (They are not in fact
implemented like this, but instead in a more efficient manner in individual C
functions).

    $foo = reduce { defined($a)            ? $a :
                    $code->(local $_ = $b) ? $b :
                                             undef } undef, @list # first

    $foo = reduce { $a > $b ? $a : $b } 1..10       # max
    $foo = reduce { $a gt $b ? $a : $b } 'A'..'Z'   # maxstr
    $foo = reduce { $a < $b ? $a : $b } 1..10       # min
    $foo = reduce { $a lt $b ? $a : $b } 'aa'..'zz' # minstr
    $foo = reduce { $a + $b } 1 .. 10               # sum
    $foo = reduce { $a . $b } @bar                  # concat

    $foo = reduce { $a || $code->(local $_ = $b) } 0, @bar   # any
    $foo = reduce { $a && $code->(local $_ = $b) } 1, @bar   # all
    $foo = reduce { $a && !$code->(local $_ = $b) } 1, @bar  # none
    $foo = reduce { $a || !$code->(local $_ = $b) } 0, @bar  # notall
       # Note that these implementations do not fully short-circuit

If your algorithm requires that C<reduce> produce an identity value, then make
sure that you always pass that identity value as the first argument to prevent
C<undef> being returned

  $foo = reduce { $a + $b } 0, @values;             # sum with 0 identity value

The above example code blocks also suggest how to use C<reduce> to build a
more efficient combined version of one of these basic functions and a C<map>
block. For example, to find the total length of all the strings in a list,
we could use

    $total = sum map { length } @strings;

However, this produces a list of temporary integer values as long as the
original list of strings, only to reduce it down to a single value again. We
can compute the same result more efficiently by using C<reduce> with a code
block that accumulates lengths by writing this instead as:

    $total = reduce { $a + length $b } 0, @strings

The other scalar-returning list reduction functions are all specialisations of
this generic idea.

=head2 reductions

    @results = reductions { BLOCK } @list

I<Since version 1.54.>

Similar to C<reduce> except that it also returns the intermediate values along
with the final result. As before, C<$a> is set to the first element of the
given list, and the C<BLOCK> is then called once for remaining item in the
list set into C<$b>, with the result being captured for return as well as
becoming the new value for C<$a>.

The returned list will begin with the initial value for C<$a>, followed by
each return value from the block in order. The final value of the result will
be identical to what the C<reduce> function would have returned given the same
block and list.

    reduce     { "$a-$b" }  "a".."d"    # "a-b-c-d"
    reductions { "$a-$b" }  "a".."d"    # "a", "a-b", "a-b-c", "a-b-c-d"

=head2 any

    my $bool = any { BLOCK } @list;

I<Since version 1.33.>

Similar to C<grep> in that it evaluates C<BLOCK> setting C<$_> to each element
of C<@list> in turn. C<any> returns true if any element makes the C<BLOCK>
return a true value. If C<BLOCK> never returns true or C<@list> was empty then
it returns false.

Many cases of using C<grep> in a conditional can be written using C<any>
instead, as it can short-circuit after the first true result.

    if( any { length > 10 } @strings ) {
        # at least one string has more than 10 characters
    }

Note: Due to XS issues the block passed may be able to access the outer @_
directly. This is not intentional and will break under debugger.

=head2 all

    my $bool = all { BLOCK } @list;

I<Since version 1.33.>

Similar to L</any>, except that it requires all elements of the C<@list> to
make the C<BLOCK> return true. If any element returns false, then it returns
false. If the C<BLOCK> never returns false or the C<@list> was empty then it
returns true.

Note: Due to XS issues the block passed may be able to access the outer @_
directly. This is not intentional and will break under debugger.

=head2 none

=head2 notall

    my $bool = none { BLOCK } @list;

    my $bool = notall { BLOCK } @list;

I<Since version 1.33.>

Similar to L</any> and L</all>, but with the return sense inverted. C<none>
returns true only if no value in the C<@list> causes the C<BLOCK> to return
true, and C<notall> returns true only if not all of the values do.

Note: Due to XS issues the block passed may be able to access the outer @_
directly. This is not intentional and will break under debugger.

=head2 first

    my $val = first { BLOCK } @list;

Similar to C<grep> in that it evaluates C<BLOCK> setting C<$_> to each element
of C<@list> in turn. C<first> returns the first element where the result from
C<BLOCK> is a true value. If C<BLOCK> never returns true or C<@list> was empty
then C<undef> is returned.

    $foo = first { defined($_) } @list    # first defined value in @list
    $foo = first { $_ > $value } @list    # first value in @list which
                                          # is greater than $value

=head2 max

    my $num = max @list;

Returns the entry in the list with the highest numerical value. If the list is
empty then C<undef> is returned.

    $foo = max 1..10                # 10
    $foo = max 3,9,12               # 12
    $foo = max @bar, @baz           # whatever

=head2 maxstr

    my $str = maxstr @list;

Similar to L</max>, but treats all the entries in the list as strings and
returns the highest string as defined by the C<gt> operator. If the list is
empty then C<undef> is returned.

    $foo = maxstr 'A'..'Z'          # 'Z'
    $foo = maxstr "hello","world"   # "world"
    $foo = maxstr @bar, @baz        # whatever

=head2 min

    my $num = min @list;

Similar to L</max> but returns the entry in the list with the lowest numerical
value. If the list is empty then C<undef> is returned.

    $foo = min 1..10                # 1
    $foo = min 3,9,12               # 3
    $foo = min @bar, @baz           # whatever

=head2 minstr

    my $str = minstr @list;

Similar to L</min>, but treats all the entries in the list as strings and
returns the lowest string as defined by the C<lt> operator. If the list is
empty then C<undef> is returned.

    $foo = minstr 'A'..'Z'          # 'A'
    $foo = minstr "hello","world"   # "hello"
    $foo = minstr @bar, @baz        # whatever

=head2 product

    my $num = product @list;

I<Since version 1.35.>

Returns the numerical product of all the elements in C<@list>. If C<@list> is
empty then C<1> is returned.

    $foo = product 1..10            # 3628800
    $foo = product 3,9,12           # 324

=head2 sum

    my $num_or_undef = sum @list;

Returns the numerical sum of all the elements in C<@list>. For backwards
compatibility, if C<@list> is empty then C<undef> is returned.

    $foo = sum 1..10                # 55
    $foo = sum 3,9,12               # 24
    $foo = sum @bar, @baz           # whatever

=head2 sum0

    my $num = sum0 @list;

I<Since version 1.26.>

Similar to L</sum>, except this returns 0 when given an empty list, rather
than C<undef>.

=cut

=head1 KEY/VALUE PAIR LIST FUNCTIONS

The following set of functions, all inspired by L<List::Pairwise>, consume an
even-sized list of pairs. The pairs may be key/value associations from a hash,
or just a list of values. The functions will all preserve the original ordering
of the pairs, and will not be confused by multiple pairs having the same "key"
value - nor even do they require that the first of each pair be a plain string.

B<NOTE>: At the time of writing, the following C<pair*> functions that take a
block do not modify the value of C<$_> within the block, and instead operate
using the C<$a> and C<$b> globals instead. This has turned out to be a poor
design, as it precludes the ability to provide a C<pairsort> function. Better
would be to pass pair-like objects as 2-element array references in C<$_>, in
a style similar to the return value of the C<pairs> function. At some future
version this behaviour may be added.

Until then, users are alerted B<NOT> to rely on the value of C<$_> remaining
unmodified between the outside and the inside of the control block. In
particular, the following example is B<UNSAFE>:

 my @kvlist = ...

 foreach (qw( some keys here )) {
    my @items = pairgrep { $a eq $_ } @kvlist;
    ...
 }

Instead, write this using a lexical variable:

 foreach my $key (qw( some keys here )) {
    my @items = pairgrep { $a eq $key } @kvlist;
    ...
 }

=cut

=head2 pairs

    my @pairs = pairs @kvlist;

I<Since version 1.29.>

A convenient shortcut to operating on even-sized lists of pairs, this function
returns a list of C<ARRAY> references, each containing two items from the
given list. It is a more efficient version of

    @pairs = pairmap { [ $a, $b ] } @kvlist

It is most convenient to use in a C<foreach> loop, for example:

    foreach my $pair ( pairs @kvlist ) {
       my ( $key, $value ) = @$pair;
       ...
    }

Since version C<1.39> these C<ARRAY> references are blessed objects,
recognising the two methods C<key> and C<value>. The following code is
equivalent:

    foreach my $pair ( pairs @kvlist ) {
       my $key   = $pair->key;
       my $value = $pair->value;
       ...
    }

Since version C<1.51> they also have a C<TO_JSON> method to ease
serialisation.

=head2 unpairs

    my @kvlist = unpairs @pairs

I<Since version 1.42.>

The inverse function to C<pairs>; this function takes a list of C<ARRAY>
references containing two elements each, and returns a flattened list of the
two values from each of the pairs, in order. This is notionally equivalent to

    my @kvlist = map { @{$_}[0,1] } @pairs

except that it is implemented more efficiently internally. Specifically, for
any input item it will extract exactly two values for the output list; using
C<undef> if the input array references are short.

Between C<pairs> and C<unpairs>, a higher-order list function can be used to
operate on the pairs as single scalars; such as the following near-equivalents
of the other C<pair*> higher-order functions:

    @kvlist = unpairs grep { FUNC } pairs @kvlist
    # Like pairgrep, but takes $_ instead of $a and $b

    @kvlist = unpairs map { FUNC } pairs @kvlist
    # Like pairmap, but takes $_ instead of $a and $b

Note however that these versions will not behave as nicely in scalar context.

Finally, this technique can be used to implement a sort on a keyvalue pair
list; e.g.:

    @kvlist = unpairs sort { $a->key cmp $b->key } pairs @kvlist

=head2 pairkeys

    my @keys = pairkeys @kvlist;

I<Since version 1.29.>

A convenient shortcut to operating on even-sized lists of pairs, this function
returns a list of the the first values of each of the pairs in the given list.
It is a more efficient version of

    @keys = pairmap { $a } @kvlist

=head2 pairvalues

    my @values = pairvalues @kvlist;

I<Since version 1.29.>

A convenient shortcut to operating on even-sized lists of pairs, this function
returns a list of the the second values of each of the pairs in the given list.
It is a more efficient version of

    @values = pairmap { $b } @kvlist

=head2 pairgrep

    my @kvlist = pairgrep { BLOCK } @kvlist;

    my $count = pairgrep { BLOCK } @kvlist;

I<Since version 1.29.>

Similar to perl's C<grep> keyword, but interprets the given list as an
even-sized list of pairs. It invokes the C<BLOCK> multiple times, in scalar
context, with C<$a> and C<$b> set to successive pairs of values from the
C<@kvlist>.

Returns an even-sized list of those pairs for which the C<BLOCK> returned true
in list context, or the count of the B<number of pairs> in scalar context.
(Note, therefore, in scalar context that it returns a number half the size of
the count of items it would have returned in list context).

    @subset = pairgrep { $a =~ m/^[[:upper:]]+$/ } @kvlist

As with C<grep> aliasing C<$_> to list elements, C<pairgrep> aliases C<$a> and
C<$b> to elements of the given list. Any modifications of it by the code block
will be visible to the caller.

=head2 pairfirst

    my ( $key, $val ) = pairfirst { BLOCK } @kvlist;

    my $found = pairfirst { BLOCK } @kvlist;

I<Since version 1.30.>

Similar to the L</first> function, but interprets the given list as an
even-sized list of pairs. It invokes the C<BLOCK> multiple times, in scalar
context, with C<$a> and C<$b> set to successive pairs of values from the
C<@kvlist>.

Returns the first pair of values from the list for which the C<BLOCK> returned
true in list context, or an empty list of no such pair was found. In scalar
context it returns a simple boolean value, rather than either the key or the
value found.

    ( $key, $value ) = pairfirst { $a =~ m/^[[:upper:]]+$/ } @kvlist

As with C<grep> aliasing C<$_> to list elements, C<pairfirst> aliases C<$a> and
C<$b> to elements of the given list. Any modifications of it by the code block
will be visible to the caller.

=head2 pairmap

    my @list = pairmap { BLOCK } @kvlist;

    my $count = pairmap { BLOCK } @kvlist;

I<Since version 1.29.>

Similar to perl's C<map> keyword, but interprets the given list as an
even-sized list of pairs. It invokes the C<BLOCK> multiple times, in list
context, with C<$a> and C<$b> set to successive pairs of values from the
C<@kvlist>.

Returns the concatenation of all the values returned by the C<BLOCK> in list
context, or the count of the number of items that would have been returned in
scalar context.

    @result = pairmap { "The key $a has value $b" } @kvlist

As with C<map> aliasing C<$_> to list elements, C<pairmap> aliases C<$a> and
C<$b> to elements of the given list. Any modifications of it by the code block
will be visible to the caller.

See L</KNOWN BUGS> for a known-bug with C<pairmap>, and a workaround.

=cut

=head1 OTHER FUNCTIONS

=cut

=head2 shuffle

    my @values = shuffle @values;

Returns the values of the input in a random order

    @cards = shuffle 0..51      # 0..51 in a random order

This function is affected by the C<$RAND> variable.

=cut

=head2 sample

    my @items = sample $count, @values

I<Since version 1.54.>

Randomly select the given number of elements from the input list. Any given
position in the input list will be selected at most once.

If there are fewer than C<$count> items in the list then the function will
return once all of them have been randomly selected; effectively the function
behaves similarly to L</shuffle>.

This function is affected by the C<$RAND> variable.

=head2 uniq

    my @subset = uniq @values

I<Since version 1.45.>

Filters a list of values to remove subsequent duplicates, as judged by a
DWIM-ish string equality or C<undef> test. Preserves the order of unique
elements, and retains the first value of any duplicate set.

    my $count = uniq @values

In scalar context, returns the number of elements that would have been
returned as a list.

The C<undef> value is treated by this function as distinct from the empty
string, and no warning will be produced. It is left as-is in the returned
list. Subsequent C<undef> values are still considered identical to the first,
and will be removed.

=head2 uniqint

    my @subset = uniqint @values

I<Since version 1.55.>

Filters a list of values to remove subsequent duplicates, as judged by an
integer numerical equality test. Preserves the order of unique elements, and
retains the first value of any duplicate set. Values in the returned list will
be coerced into integers.

    my $count = uniqint @values

In scalar context, returns the number of elements that would have been
returned as a list.

Note that C<undef> is treated much as other numerical operations treat it; it
compares equal to zero but additionally produces a warning if such warnings
are enabled (C<use warnings 'uninitialized';>). In addition, an C<undef> in
the returned list is coerced into a numerical zero, so that the entire list of
values returned by C<uniqint> are well-behaved as integers.

=head2 uniqnum

    my @subset = uniqnum @values

I<Since version 1.44.>

Filters a list of values to remove subsequent duplicates, as judged by a
numerical equality test. Preserves the order of unique elements, and retains
the first value of any duplicate set.

    my $count = uniqnum @values

In scalar context, returns the number of elements that would have been
returned as a list.

Note that C<undef> is treated much as other numerical operations treat it; it
compares equal to zero but additionally produces a warning if such warnings
are enabled (C<use warnings 'uninitialized';>). In addition, an C<undef> in
the returned list is coerced into a numerical zero, so that the entire list of
values returned by C<uniqnum> are well-behaved as numbers.

Note also that multiple IEEE C<NaN> values are treated as duplicates of
each other, regardless of any differences in their payloads, and despite
the fact that C<< 0+'NaN' == 0+'NaN' >> yields false.

=head2 uniqstr

    my @subset = uniqstr @values

I<Since version 1.45.>

Filters a list of values to remove subsequent duplicates, as judged by a
string equality test. Preserves the order of unique elements, and retains the
first value of any duplicate set.

    my $count = uniqstr @values

In scalar context, returns the number of elements that would have been
returned as a list.

Note that C<undef> is treated much as other string operations treat it; it
compares equal to the empty string but additionally produces a warning if such
warnings are enabled (C<use warnings 'uninitialized';>). In addition, an
C<undef> in the returned list is coerced into an empty string, so that the
entire list of values returned by C<uniqstr> are well-behaved as strings.

=cut

=head2 head

    my @values = head $size, @list;

I<Since version 1.50.>

Returns the first C<$size> elements from C<@list>. If C<$size> is negative, returns
all but the last C<$size> elements from C<@list>.

    @result = head 2, qw( foo bar baz );
    # foo, bar

    @result = head -2, qw( foo bar baz );
    # foo

=head2 tail

    my @values = tail $size, @list;

I<Since version 1.50.>

Returns the last C<$size> elements from C<@list>. If C<$size> is negative, returns
all but the first C<$size> elements from C<@list>.

    @result = tail 2, qw( foo bar baz );
    # bar, baz

    @result = tail -2, qw( foo bar baz );
    # baz

=head2 zip

    my @result = zip [1..3], ['a'..'c'];
    # [1, 'a'], [2, 'b'], [3, 'c']

I<Since version 1.56.>

Returns a list of array references, composed of elements from the given list
of array references. Each array in the returned list is composed of elements
at that corresponding position from each of the given input arrays. If any
input arrays run out of elements before others, then C<undef> will be inserted
into the result to fill in the gaps.

The C<zip> function is particularly handy for iterating over multiple arrays
at the same time with a C<foreach> loop, taking one element from each:

    foreach ( zip \@xs, \@ys, \@zs ) {
        my ($x, $y, $z) = @$_;
        ...
    }

B<NOTE> to users of L<List::MoreUtils>: This function does not behave the same
as C<List::MoreUtils::zip>, but is actually a non-prototyped equivalent to
C<List::MoreUtils::zip_unflatten>. This function does not apply a prototype,
so make sure to invoke it with references to arrays.

For a function similar to the C<zip> function from C<List::MoreUtils>, see
L<mesh>.

    my @result = zip_shortest ...

A variation of the function that differs in how it behaves when given input
arrays of differing lengths. C<zip_shortest> will stop as soon as any one of
the input arrays run out of elements, discarding any remaining unused values
from the others.

    my @result = zip_longest ...

C<zip_longest> is an alias to the C<zip> function, provided simply to be
explicit about that behaviour as compared to C<zip_shortest>.

=head2 mesh

    my @result = mesh [1..3], ['a'..'c'];
    # (1, 'a', 2, 'b', 3, 'c')

I<Since version 1.56.>

Returns a list of items collected from elements of the given list of array
references. Each section of items in the returned list is composed of elements
at the corresponding position from each of the given input arrays. If any
input arrays run out of elements before others, then C<undef> will be inserted
into the result to fill in the gaps.

This is similar to L<zip>, except that all of the ranges in the result are
returned in one long flattened list, instead of being bundled into separate
arrays.

Because it returns a flat list of items, the C<mesh> function is particularly
useful for building a hash out of two separate arrays of keys and values:

    my %hash = mesh \@keys, \@values;

    my $href = { mesh \@keys, \@values };

B<NOTE> to users of L<List::MoreUtils>: This function is a non-prototyped
equivalent to C<List::MoreUtils::mesh> or C<List::MoreUtils::zip> (themselves
aliases of each other). This function does not apply a prototype, so make sure
to invoke it with references to arrays.

    my @result = mesh_shortest ...

    my @result = mesh_longest ...

These variations are similar to those of L<zip>, in that they differ in
behaviour when one of the input lists runs out of elements before the others.

=head1 CONFIGURATION VARIABLES

=head2 $RAND

    local $List::Util::RAND = sub { ... };

I<Since version 1.54.>

This package variable is used by code which needs to generate random numbers
(such as the L</shuffle> and L</sample> functions). If set to a CODE reference
it provides an alternative to perl's builtin C<rand()> function. When a new
random number is needed this function will be invoked with no arguments and is
expected to return a floating-point value, of which only the fractional part
will be used.

=head1 KNOWN BUGS

=head2 RT #95409

L<https://rt.cpan.org/Ticket/Display.html?id=95409>

If the block of code given to L</pairmap> contains lexical variables that are
captured by a returned closure, and the closure is executed after the block
has been re-used for the next iteration, these lexicals will not see the
correct values. For example:

 my @subs = pairmap {
    my $var = "$a is $b";
    sub { print "$var\n" };
 } one => 1, two => 2, three => 3;

 $_->() for @subs;

Will incorrectly print

 three is 3
 three is 3
 three is 3

This is due to the performance optimisation of using C<MULTICALL> for the code
block, which means that fresh SVs do not get allocated for each call to the
block. Instead, the same SV is re-assigned for each iteration, and all the
closures will share the value seen on the final iteration.

To work around this bug, surround the code with a second set of braces. This
creates an inner block that defeats the C<MULTICALL> logic, and does get fresh
SVs allocated each time:

 my @subs = pairmap {
    {
       my $var = "$a is $b";
       sub { print "$var\n"; }
    }
 } one => 1, two => 2, three => 3;

This bug only affects closures that are generated by the block but used
afterwards. Lexical variables that are only used during the lifetime of the
block's execution will take their individual values for each invocation, as
normal.

=head2 uniqnum() on oversized bignums

Due to the way that C<uniqnum()> compares numbers, it cannot distinguish
differences between bignums (especially bigints) that are too large to fit in
the native platform types. For example,

 my $x = Math::BigInt->new( "1" x 100 );
 my $y = $x + 1;

 say for uniqnum( $x, $y );

Will print just the value of C<$x>, believing that C<$y> is a numerically-
equivalent value. This bug does not affect C<uniqstr()>, which will correctly
observe that the two values stringify to different strings.

=head1 SUGGESTED ADDITIONS

The following are additions that have been requested, but I have been reluctant
to add due to them being very simple to implement in perl

  # How many elements are true

  sub true { scalar grep { $_ } @_ }

  # How many elements are false

  sub false { scalar grep { !$_ } @_ }

=head1 SEE ALSO

L<Scalar::Util>, L<List::MoreUtils>

=head1 COPYRIGHT

Copyright (c) 1997-2007 Graham Barr <gbarr@pobox.com>. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

Recent additions and current maintenance by
Paul Evans, <leonerd@leonerd.org.uk>.

=cut

1;
