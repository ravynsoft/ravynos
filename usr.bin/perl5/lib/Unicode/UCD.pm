package Unicode::UCD;

use strict;
use warnings;
no warnings 'surrogate';    # surrogates can be inputs to this
use charnames ();

our $VERSION = '0.78';

sub DEBUG () { 0 }
$|=1 if DEBUG;

require Exporter;

our @ISA = qw(Exporter);

our @EXPORT_OK = qw(charinfo
		    charblock charscript
		    charblocks charscripts
		    charinrange
		    charprop
		    charprops_all
		    general_categories bidi_types
		    compexcl
		    casefold all_casefolds casespec
		    namedseq
                    num
                    prop_aliases
                    prop_value_aliases
                    prop_values
                    prop_invlist
                    prop_invmap
                    search_invlist
                    MAX_CP
                );

use Carp;

sub IS_ASCII_PLATFORM { ord("A") == 65 }

=head1 NAME

Unicode::UCD - Unicode character database

=head1 SYNOPSIS

    use Unicode::UCD 'charinfo';
    my $charinfo   = charinfo($codepoint);

    use Unicode::UCD 'charprop';
    my $value  = charprop($codepoint, $property);

    use Unicode::UCD 'charprops_all';
    my $all_values_hash_ref = charprops_all($codepoint);

    use Unicode::UCD 'casefold';
    my $casefold = casefold($codepoint);

    use Unicode::UCD 'all_casefolds';
    my $all_casefolds_ref = all_casefolds();

    use Unicode::UCD 'casespec';
    my $casespec = casespec($codepoint);

    use Unicode::UCD 'charblock';
    my $charblock  = charblock($codepoint);

    use Unicode::UCD 'charscript';
    my $charscript = charscript($codepoint);

    use Unicode::UCD 'charblocks';
    my $charblocks = charblocks();

    use Unicode::UCD 'charscripts';
    my $charscripts = charscripts();

    use Unicode::UCD qw(charscript charinrange);
    my $range = charscript($script);
    print "looks like $script\n" if charinrange($range, $codepoint);

    use Unicode::UCD qw(general_categories bidi_types);
    my $categories = general_categories();
    my $types = bidi_types();

    use Unicode::UCD 'prop_aliases';
    my @space_names = prop_aliases("space");

    use Unicode::UCD 'prop_value_aliases';
    my @gc_punct_names = prop_value_aliases("Gc", "Punct");

    use Unicode::UCD 'prop_values';
    my @all_EA_short_names = prop_values("East_Asian_Width");

    use Unicode::UCD 'prop_invlist';
    my @puncts = prop_invlist("gc=punctuation");

    use Unicode::UCD 'prop_invmap';
    my ($list_ref, $map_ref, $format, $missing)
                                      = prop_invmap("General Category");

    use Unicode::UCD 'search_invlist';
    my $index = search_invlist(\@invlist, $code_point);

    # The following function should be used only internally in
    # implementations of the Unicode Normalization Algorithm, and there
    # are better choices than it.
    use Unicode::UCD 'compexcl';
    my $compexcl = compexcl($codepoint);

    use Unicode::UCD 'namedseq';
    my $namedseq = namedseq($named_sequence_name);

    my $unicode_version = Unicode::UCD::UnicodeVersion();

    my $convert_to_numeric =
              Unicode::UCD::num("\N{RUMI DIGIT ONE}\N{RUMI DIGIT TWO}");

=head1 DESCRIPTION

The Unicode::UCD module offers a series of functions that
provide a simple interface to the Unicode
Character Database.

=head2 code point argument

Some of the functions are called with a I<code point argument>, which is either
a decimal or a hexadecimal scalar designating a code point in the platform's
native character set (extended to Unicode), or a string containing C<U+>
followed by hexadecimals
designating a Unicode code point.  A leading 0 will force a hexadecimal
interpretation, as will a hexadecimal digit that isn't a decimal digit.

Examples:

    223     # Decimal 223 in native character set
    0223    # Hexadecimal 223, native (= 547 decimal)
    0xDF    # Hexadecimal DF, native (= 223 decimal)
    '0xDF'  # String form of hexadecimal (= 223 decimal)
    'U+DF'  # Hexadecimal DF, in Unicode's character set
                              (= LATIN SMALL LETTER SHARP S)

Note that the largest code point in Unicode is U+10FFFF.

=cut

our %caseless_equivalent;
our $e_precision;
our %file_to_swash_name;
our @inline_definitions;
our %loose_property_name_of;
our %loose_property_to_file_of;
our %loose_to_file_of;
our $MAX_CP;
our %nv_floating_to_rational;
our %prop_aliases;
our %stricter_to_file_of;
our %strict_property_to_file_of;
our %SwashInfo;
our %why_deprecated;

my $v_unicode_version;  # v-string.

sub openunicode {
    my (@path) = @_;
    my $rfh;
    for my $d (@INC) {
        use File::Spec;
        my $f = File::Spec->catfile($d, "unicore", @path);
        return $rfh if open($rfh, '<', $f);
    }
    croak __PACKAGE__, ": failed to find ",
        File::Spec->catfile("unicore", @path), " in @INC";
}

sub _dclone ($) {   # Use Storable::dclone if available; otherwise emulate it.

    use if defined &DynaLoader::boot_DynaLoader, Storable => qw(dclone);

    return dclone(shift) if defined &dclone;

    my $arg = shift;
    my $type = ref $arg;
    return $arg unless $type;   # No deep cloning needed for scalars

    if ($type eq 'ARRAY') {
        my @return;
        foreach my $element (@$arg) {
            push @return, &_dclone($element);
        }
        return \@return;
    }
    elsif ($type eq 'HASH') {
        my %return;
        foreach my $key (keys %$arg) {
            $return{$key} = &_dclone($arg->{$key});
        }
        return \%return;
    }
    else {
        croak "_dclone can't handle " . $type;
    }
}

=head2 B<charinfo()>

    use Unicode::UCD 'charinfo';

    my $charinfo = charinfo(0x41);

This returns information about the input L</code point argument>
as a reference to a hash of fields as defined by the Unicode
standard.  If the L</code point argument> is not assigned in the standard
(i.e., has the general category C<Cn> meaning C<Unassigned>)
or is a non-character (meaning it is guaranteed to never be assigned in
the standard),
C<undef> is returned.

Fields that aren't applicable to the particular code point argument exist in the
returned hash, and are empty.

For results that are less "raw" than this function returns, or to get the values for
any property, not just the few covered by this function, use the
L</charprop()> function.

The keys in the hash with the meanings of their values are:

=over

=item B<code>

the input native L</code point argument> expressed in hexadecimal, with
leading zeros
added if necessary to make it contain at least four hexdigits

=item B<name>

name of I<code>, all IN UPPER CASE.
Some control-type code points do not have names.
This field will be empty for C<Surrogate> and C<Private Use> code points,
and for the others without a name,
it will contain a description enclosed in angle brackets, like
C<E<lt>controlE<gt>>.


=item B<category>

The short name of the general category of I<code>.
This will match one of the keys in the hash returned by L</general_categories()>.

The L</prop_value_aliases()> function can be used to get all the synonyms
of the category name.

=item B<combining>

the combining class number for I<code> used in the Canonical Ordering Algorithm.
For Unicode 5.1, this is described in Section 3.11 C<Canonical Ordering Behavior>
available at
L<http://www.unicode.org/versions/Unicode5.1.0/>

The L</prop_value_aliases()> function can be used to get all the synonyms
of the combining class number.

=item B<bidi>

bidirectional type of I<code>.
This will match one of the keys in the hash returned by L</bidi_types()>.

The L</prop_value_aliases()> function can be used to get all the synonyms
of the bidi type name.

=item B<decomposition>

is empty if I<code> has no decomposition; or is one or more codes
(separated by spaces) that, taken in order, represent a decomposition for
I<code>.  Each has at least four hexdigits.
The codes may be preceded by a word enclosed in angle brackets, then a space,
like C<E<lt>compatE<gt> >, giving the type of decomposition

This decomposition may be an intermediate one whose components are also
decomposable.  Use L<Unicode::Normalize> to get the final decomposition in one
step.

=item B<decimal>

if I<code> represents a decimal digit this is its integer numeric value

=item B<digit>

if I<code> represents some other digit-like number, this is its integer
numeric value

=item B<numeric>

if I<code> represents a whole or rational number, this is its numeric value.
Rational values are expressed as a string like C<1/4>.

=item B<mirrored>

C<Y> or C<N> designating if I<code> is mirrored in bidirectional text

=item B<unicode10>

name of I<code> in the Unicode 1.0 standard if one
existed for this code point and is different from the current name

=item B<comment>

As of Unicode 6.0, this is always empty.

=item B<upper>

is, if non-empty, the uppercase mapping for I<code> expressed as at least four
hexdigits.  This indicates that the full uppercase mapping is a single
character, and is identical to the simple (single-character only) mapping.
When this field is empty, it means that the simple uppercase mapping is
I<code> itself; you'll need some other means, (like L</charprop()> or
L</casespec()> to get the full mapping.

=item B<lower>

is, if non-empty, the lowercase mapping for I<code> expressed as at least four
hexdigits.  This indicates that the full lowercase mapping is a single
character, and is identical to the simple (single-character only) mapping.
When this field is empty, it means that the simple lowercase mapping is
I<code> itself; you'll need some other means, (like L</charprop()> or
L</casespec()> to get the full mapping.

=item B<title>

is, if non-empty, the titlecase mapping for I<code> expressed as at least four
hexdigits.  This indicates that the full titlecase mapping is a single
character, and is identical to the simple (single-character only) mapping.
When this field is empty, it means that the simple titlecase mapping is
I<code> itself; you'll need some other means, (like L</charprop()> or
L</casespec()> to get the full mapping.

=item B<block>

the block I<code> belongs to (used in C<\p{Blk=...}>).
The L</prop_value_aliases()> function can be used to get all the synonyms
of the block name.

See L</Blocks versus Scripts>.

=item B<script>

the script I<code> belongs to.
The L</prop_value_aliases()> function can be used to get all the synonyms
of the script name.  Note that this is the older "Script" property value, and
not the improved "Script_Extensions" value.

See L</Blocks versus Scripts>.

=back

Note that you cannot do (de)composition and casing based solely on the
I<decomposition>, I<combining>, I<lower>, I<upper>, and I<title> fields; you
will need also the L</casespec()> function and the C<Composition_Exclusion>
property.  (Or you could just use the L<lc()|perlfunc/lc>,
L<uc()|perlfunc/uc>, and L<ucfirst()|perlfunc/ucfirst> functions, and the
L<Unicode::Normalize> module.)

=cut

my %Cache;

# Digits may be separated by a single underscore
my $digits = qr/ ( [0-9] _? )+ (?!:_) /x;

# A sign can be surrounded by white space
my $sign = qr/ \s* [+-]? \s* /x;

my $f_float = qr/  $sign $digits+ \. $digits*    # e.g., 5.0, 5.
                 | $sign $digits* \. $digits+/x; # 0.7, .7

# A number may be an integer, a rational, or a float with an optional exponent
# We (shudder) accept a signed denominator
my $number = qr{  ^ $sign $digits+ $
                | ^ $sign $digits+ \/ $sign $digits+ $
                | ^ $f_float (?: [Ee] [+-]? $digits )? $}x;

sub loose_name ($) {
    # Given a lowercase property or property-value name, return its
    # standardized version that is expected for look-up in the 'loose' hashes
    # in UCD.pl (hence, this depends on what mktables does).  This squeezes
    # out blanks, underscores and dashes.  The complication stems from the
    # grandfathered-in 'L_', which retains a single trailing underscore.

# integer or float (no exponent)
my $integer_or_float_re = qr/ ^ -? \d+ (:? \. \d+ )? $ /x;

# Also includes rationals
my $numeric_re = qr! $integer_or_float_re | ^ -? \d+ / \d+ $ !x;
    return $_[0] if $_[0] =~ $numeric_re;

    (my $loose = $_[0]) =~ s/[-_ \t]//g;

    return $loose if $loose !~ / ^ (?: is | to )? l $/x;
    return 'l_' if $_[0] =~ / l .* _ /x;    # If original had a trailing '_'
    return $loose;
}

##
## "SWASH" == "SWATCH HASH". A "swatch" is a swatch of the Unicode landscape.
## It's a data structure that encodes a set of Unicode characters.
##

{
    use re "/aa";  # Nothing here uses above Latin1.

    # If a floating point number is within this distance from the value of a
    # fraction, it is considered to be that fraction, even if many more digits
    # are specified that don't exactly match.
    my $min_floating_slop;

    # To guard against this program calling something that in turn ends up
    # calling this program with the same inputs, and hence infinitely
    # recursing, we keep a stack of the properties that are currently in
    # progress, pushed upon entry, popped upon return.
    my @recursed;

    sub SWASHNEW {
        my ($class, $type, $list, $minbits) = @_;
        my $user_defined = 0;
        local $^D = 0 if $^D;

        $class = "" unless defined $class;
        print STDERR __LINE__, ": class=$class, type=$type, list=",
                                (defined $list) ? $list : ':undef:',
                                ", minbits=$minbits\n" if DEBUG;

        ##
        ## Get the list of codepoints for the type.
        ## Called from swash_init (see utf8.c) or SWASHNEW itself.
        ##
        ## Callers of swash_init:
        ##     prop_invlist
        ##     Unicode::UCD::prop_invmap
        ##
        ## Given a $type, our goal is to fill $list with the set of codepoint
        ## ranges. If $type is false, $list passed is used.
        ##
        ## $minbits:
        ##     For binary properties, $minbits must be 1.
        ##     For character mappings (case and transliteration), $minbits must
        ##     be a number except 1.
        ##
        ## $list (or that filled according to $type):
        ##     Refer to perlunicode.pod, "User-Defined Character Properties."
        ##
        ##     For binary properties, only characters with the property value
        ##     of True should be listed. The 3rd column, if any, will be ignored
        ##
        ## To make the parsing of $type clear, this code takes the a rather
        ## unorthodox approach of last'ing out of the block once we have the
        ## info we need. Were this to be a subroutine, the 'last' would just
        ## be a 'return'.
        ##
        #   If a problem is found $type is returned;
        #   Upon success, a new (or cached) blessed object is returned with
        #   keys TYPE, BITS, EXTRAS, LIST, and with values having the
        #   same meanings as the input parameters.
        #   SPECIALS contains a reference to any special-treatment hash in the
        #       property.
        #   INVERT_IT is non-zero if the result should be inverted before use
        #   USER_DEFINED is non-zero if the result came from a user-defined
        my $file; ## file to load data from, and also part of the %Cache key.

        # Change this to get a different set of Unicode tables
        my $unicore_dir = 'unicore';
        my $invert_it = 0;
        my $list_is_from_mktables = 0;  # Is $list returned from a mktables
                                        # generated file?  If so, we know it's
                                        # well behaved.

        if ($type)
        {
            # Verify that this isn't a recursive call for this property.
            # Can't use croak, as it may try to recurse to here itself.
            my $class_type = $class . "::$type";
            if (grep { $_ eq $class_type } @recursed) {
                CORE::die "panic: Infinite recursion in SWASHNEW for '$type'\n";
            }
            push @recursed, $class_type;

            $type =~ s/^\s+//;
            $type =~ s/\s+$//;

            # regcomp.c surrounds the property name with '__" and '_i' if this
            # is to be caseless matching.
            my $caseless = $type =~ s/^(.*)__(.*)_i$/$1$2/;

            print STDERR __LINE__, ": type=$type, caseless=$caseless\n" if DEBUG;

        GETFILE:
            {
                ##
                ## It could be a user-defined property.  Look in current
                ## package if no package given
                ##


                my $caller0 = caller(0);
                my $caller1 = $type =~ s/(.+):://
                              ? $1
                              : $caller0 eq 'main'
                                ? 'main'
                                : caller(1);

                if (defined $caller1 && $type =~ /^I[ns]\w+$/) {
                    my $prop = "${caller1}::$type";
                    if (exists &{$prop}) {
                        # stolen from Scalar::Util::PP::tainted()
                        my $tainted;
                        {
                            local($@, $SIG{__DIE__}, $SIG{__WARN__});
                            local $^W = 0;
                            no warnings;
                            eval { kill 0 * $prop };
                            $tainted = 1 if $@ =~ /^Insecure/;
                        }
                        die "Insecure user-defined property \\p{$prop}\n"
                            if $tainted;
                        no strict 'refs';
                        $list = &{$prop}($caseless);
                        $user_defined = 1;
                        last GETFILE;
                    }
                }

                require "$unicore_dir/UCD.pl";

                # All property names are matched caselessly
                my $property_and_table = CORE::lc $type;
                print STDERR __LINE__, ": $property_and_table\n" if DEBUG;

                # See if is of the compound form 'property=value', where the
                # value indicates the table we should use.
                my ($property, $table, @remainder) =
                                    split /\s*[:=]\s*/, $property_and_table, -1;
                if (@remainder) {
                    pop @recursed if @recursed;
                    return $type;
                }

                my $prefix;
                if (! defined $table) {

                    # Here, is the single form.  The property becomes empty, and
                    # the whole value is the table.
                    $table = $property;
                    $prefix = $property = "";
                } else {
                    print STDERR __LINE__, ": $property\n" if DEBUG;

                    # Here it is the compound property=table form.  The property
                    # name is always loosely matched, and always can have an
                    # optional 'is' prefix (which isn't true in the single
                    # form).
                    $property = loose_name($property) =~ s/^is//r;

                    # And convert to canonical form.  Quit if not valid.
                    $property = $loose_property_name_of{$property};
                    if (! defined $property) {
                        pop @recursed if @recursed;
                        return $type;
                    }

                    $prefix = "$property=";

                    # If the rhs looks like it is a number...
                    print STDERR __LINE__, ": table=$table\n" if DEBUG;

                    if ($table =~ $number) {
                        print STDERR __LINE__, ": table=$table\n" if DEBUG;

                        # Split on slash, in case it is a rational, like \p{1/5}
                        my @parts = split m{ \s* / \s* }x, $table, -1;
                        print __LINE__, ": $type\n" if @parts > 2 && DEBUG;

                        foreach my $part (@parts) {
                            print __LINE__, ": part=$part\n" if DEBUG;

                            $part =~ s/^\+\s*//;    # Remove leading plus
                            $part =~ s/^-\s*/-/;    # Remove blanks after unary
                                                    # minus

                            # Remove underscores between digits.
                            $part =~ s/(?<= [0-9] ) _ (?= [0-9] ) //xg;

                            # No leading zeros (but don't make a single '0'
                            # into a null string)
                            $part =~ s/ ^ ( -? ) 0+ /$1/x;
                            $part .= '0' if $part eq '-' || $part eq "";

                            # No trailing zeros after a decimal point
                            $part =~ s/ ( \. [0-9]*? ) 0+ $ /$1/x;

                            # Begin with a 0 if a leading decimal point
                            $part =~ s/ ^ ( -? ) \. /${1}0./x;

                            # Ensure not a trailing decimal point: turn into an
                            # integer
                            $part =~ s/ \. $ //x;

                            print STDERR __LINE__, ": part=$part\n" if DEBUG;
                            #return $type if $part eq "";
                        }

                        #  If a rational...
                        if (@parts == 2) {

                            # If denominator is negative, get rid of it, and ...
                            if ($parts[1] =~ s/^-//) {

                                # If numerator is also negative, convert the
                                # whole thing to positive, else move the minus
                                # to the numerator
                                if ($parts[0] !~ s/^-//) {
                                    $parts[0] = '-' . $parts[0];
                                }
                            }
                            $table = join '/', @parts;
                        }
                        elsif ($property ne 'nv' || $parts[0] !~ /\./) {

                            # Here is not numeric value, or doesn't have a
                            # decimal point.  No further manipulation is
                            # necessary.  (Note the hard-coded property name.
                            # This could fail if other properties eventually
                            # had fractions as well; perhaps the cjk ones
                            # could evolve to do that.  This hard-coding could
                            # be fixed by mktables generating a list of
                            # properties that could have fractions.)
                            $table = $parts[0];
                        } else {

                            # Here is a floating point numeric_value.  Convert
                            # to rational.  Get a normalized form, like
                            # 5.00E-01, and look that up in the hash

                            my $float = sprintf "%.*e",
                                                $e_precision,
                                                0 + $parts[0];

                            if (exists $nv_floating_to_rational{$float}) {
                                $table = $nv_floating_to_rational{$float};
                            } else {
                                pop @recursed if @recursed;
                                return $type;
                            }
                        }
                        print STDERR __LINE__, ": $property=$table\n" if DEBUG;
                    }
                }

                # Combine lhs (if any) and rhs to get something that matches
                # the syntax of the lookups.
                $property_and_table = "$prefix$table";
                print STDERR __LINE__, ": $property_and_table\n" if DEBUG;

                # First try stricter matching.
                $file = $stricter_to_file_of{$property_and_table};

                # If didn't find it, try again with looser matching by editing
                # out the applicable characters on the rhs and looking up
                # again.
                my $strict_property_and_table;
                if (! defined $file) {

                    # This isn't used unless the name begins with 'to'
                    $strict_property_and_table = $property_and_table =~  s/^to//r;
                    $table = loose_name($table);
                    $property_and_table = "$prefix$table";
                    print STDERR __LINE__, ": $property_and_table\n" if DEBUG;
                    $file = $loose_to_file_of{$property_and_table};
                    print STDERR __LINE__, ": $property_and_table\n" if DEBUG;
                }

                # Add the constant and go fetch it in.
                if (defined $file) {

                    # If the file name contains a !, it means to invert.  The
                    # 0+ makes sure result is numeric
                    $invert_it = 0 + $file =~ s/!//;

                    if ($caseless
                        && exists $caseless_equivalent{$property_and_table})
                    {
                        $file = $caseless_equivalent{$property_and_table};
                    }

                    # The pseudo-directory '#' means that there really isn't a
                    # file to read, the data is in-line as part of the string;
                    # we extract it below.
                    $file = "$unicore_dir/lib/$file.pl" unless $file =~ m!^#/!;
                    last GETFILE;
                }
                print STDERR __LINE__, ": didn't find $property_and_table\n" if DEBUG;

                ##
                ## Last attempt -- see if it's a standard "To" name
                ## (e.g. "ToLower")  ToTitle is used by ucfirst().
                ## The user-level way to access ToDigit() and ToFold()
                ## is to use Unicode::UCD.
                ##
                # Only check if caller wants non-binary
                if ($minbits != 1) {
                    if ($property_and_table =~ s/^to//) {
                    # Look input up in list of properties for which we have
                    # mapping files.  First do it with the strict approach
                        if (defined ($file = $strict_property_to_file_of{
                                                    $strict_property_and_table}))
                        {
                            $type = $file_to_swash_name{$file};
                            print STDERR __LINE__, ": type set to $type\n"
                                                                        if DEBUG;
                            $file = "$unicore_dir/$file.pl";
                            last GETFILE;
                        }
                        elsif (defined ($file =
                          $loose_property_to_file_of{$property_and_table}))
                        {
                            $type = $file_to_swash_name{$file};
                            print STDERR __LINE__, ": type set to $type\n"
                                                                        if DEBUG;
                            $file = "$unicore_dir/$file.pl";
                            last GETFILE;
                        }   # If that fails see if there is a corresponding binary
                            # property file
                        elsif (defined ($file =
                                    $loose_to_file_of{$property_and_table}))
                        {

                            # Here, there is no map file for the property we
                            # are trying to get the map of, but this is a
                            # binary property, and there is a file for it that
                            # can easily be translated to a mapping, so use
                            # that, treating this as a binary property.
                            # Setting 'minbits' here causes it to be stored as
                            # such in the cache, so if someone comes along
                            # later looking for just a binary, they get it.
                            $minbits = 1;

                            # The 0+ makes sure is numeric
                            $invert_it = 0 + $file =~ s/!//;
                            $file = "$unicore_dir/lib/$file.pl"
                                                         unless $file =~ m!^#/!;
                            last GETFILE;
                        }
                    }
                }

                ##
                ## If we reach this line, it's because we couldn't figure
                ## out what to do with $type. Ouch.
                ##

                pop @recursed if @recursed;
                return $type;
            } # end of GETFILE block

            if (defined $file) {
                print STDERR __LINE__, ": found it (file='$file')\n" if DEBUG;

                ##
                ## If we reach here, it was due to a 'last GETFILE' above
                ## (exception: user-defined properties and mappings), so we
                ## have a filename, so now we load it if we haven't already.

                # The pseudo-directory '#' means the result isn't really a
                # file, but is in-line, with semi-colons to be turned into
                # new-lines.  Since it is in-line there is no advantage to
                # caching the result
                if ($file =~ s!^#/!!) {
                    $list = $inline_definitions[$file];
                }
                else {
                    # Here, we have an actual file to read in and load, but it
                    # may already have been read-in and cached.  The cache key
                    # is the class and file to load, and whether the results
                    # need to be inverted.
                    my $found = $Cache{$class, $file, $invert_it};
                    if ($found and ref($found) eq $class) {
                        print STDERR __LINE__, ": Returning cached swash for '$class,$file,$invert_it' for \\p{$type}\n" if DEBUG;
                        pop @recursed if @recursed;
                        return $found;
                    }

                    local $@;
                    local $!;
                    $list = do $file; die $@ if $@;
                }

                $list_is_from_mktables = 1;
            }
        } # End of $type is non-null

        # Here, either $type was null, or we found the requested property and
        # read it into $list

        my $extras = "";

        my $bits = $minbits;

        # mktables lists don't have extras, like '&prop', so don't need
        # to separate them; also lists are already sorted, so don't need to do
        # that.
        if ($list && ! $list_is_from_mktables) {
            my $taint = substr($list,0,0); # maintain taint

            # Separate the extras from the code point list, and make sure
            # user-defined properties are well-behaved for
            # downstream code.
            if ($user_defined) {
                my @tmp = split(/^/m, $list);
                my %seen;
                no warnings;

                # The extras are anything that doesn't begin with a hex digit.
                $extras = join '', $taint, grep /^[^0-9a-fA-F]/, @tmp;

                # Remove the extras, and sort the remaining entries by the
                # numeric value of their beginning hex digits, removing any
                # duplicates.
                $list = join '', $taint,
                        map  { $_->[1] }
                        sort { $a->[0] <=> $b->[0] }
                        map  { /^([0-9a-fA-F]+)/ && !$seen{$1}++ ? [ CORE::hex($1), $_ ] : () }
                        @tmp; # XXX doesn't do ranges right
            }
            else {
                # mktables has gone to some trouble to make non-user defined
                # properties well-behaved, so we can skip the effort we do for
                # user-defined ones.  Any extras are at the very beginning of
                # the string.

                # This regex splits out the first lines of $list into $1 and
                # strips them off from $list, until we get one that begins
                # with a hex number, alone on the line, or followed by a tab.
                # Either portion may be empty.
                $list =~ s/ \A ( .*? )
                            (?: \z | (?= ^ [0-9a-fA-F]+ (?: \t | $) ) )
                          //msx;

                $extras = "$taint$1";
            }
        }

        if ($minbits != 1 && $minbits < 32) { # not binary property
            my $top = 0;
            while ($list =~ /^([0-9a-fA-F]+)(?:[\t]([0-9a-fA-F]+)?)(?:[ \t]([0-9a-fA-F]+))?/mg) {
                my $min = CORE::hex $1;
                my $max = defined $2 ? CORE::hex $2 : $min;
                my $val = defined $3 ? CORE::hex $3 : 0;
                $val += $max - $min if defined $3;
                $top = $val if $val > $top;
            }
            my $topbits =
                $top > 0xffff ? 32 :
                $top > 0xff ? 16 : 8;
            $bits = $topbits if $bits < $topbits;
        }

        my @extras;
        if ($extras) {
            for my $x ($extras) {
                my $taint = substr($x,0,0); # maintain taint
                pos $x = 0;
                while ($x =~ /^([^0-9a-fA-F\n])(.*)/mg) {
                    my $char = "$1$taint";
                    my $name = "$2$taint";
                    print STDERR __LINE__, ": char [$char] => name [$name]\n"
                        if DEBUG;
                    if ($char =~ /[-+!&]/) {
                        my ($c,$t) = split(/::/, $name, 2);	# bogus use of ::, really
                        my $subobj;
                        if ($c eq 'utf8') { # khw is unsure of this
                            $subobj = SWASHNEW($t, "", $minbits, 0);
                        }
                        elsif (exists &$name) {
                            $subobj = SWASHNEW($name, "", $minbits, 0);
                        }
                        elsif ($c =~ /^([0-9a-fA-F]+)/) {
                            $subobj = SWASHNEW("", $c, $minbits, 0);
                        }
                        print STDERR __LINE__, ": returned from getting sub object for $name\n" if DEBUG;
                        if (! ref $subobj) {
                            pop @recursed if @recursed && $type;
                            return $subobj;
                        }
                        push @extras, $name => $subobj;
                        $bits = $subobj->{BITS} if $bits < $subobj->{BITS};
                        $user_defined = $subobj->{USER_DEFINED}
                                              if $subobj->{USER_DEFINED};
                    }
                }
            }
        }

        if (DEBUG) {
            print STDERR __LINE__, ": CLASS = $class, TYPE => $type, BITS => $bits, INVERT_IT => $invert_it, USER_DEFINED => $user_defined";
            print STDERR "\nLIST =>\n$list" if defined $list;
            print STDERR "\nEXTRAS =>\n$extras" if defined $extras;
            print STDERR "\n";
        }

        my $SWASH = bless {
            TYPE => $type,
            BITS => $bits,
            EXTRAS => $extras,
            LIST => $list,
            USER_DEFINED => $user_defined,
            @extras,
        } => $class;

        if ($file) {
            $Cache{$class, $file, $invert_it} = $SWASH;
            if ($type
                && exists $SwashInfo{$type}
                && exists $SwashInfo{$type}{'specials_name'})
            {
                my $specials_name = $SwashInfo{$type}{'specials_name'};
                no strict "refs";
                print STDERR "\nspecials_name => $specials_name\n" if DEBUG;
                $SWASH->{'SPECIALS'} = \%$specials_name;
            }
            $SWASH->{'INVERT_IT'} = $invert_it;
        }

        pop @recursed if @recursed && $type;

        return $SWASH;
    }
}

# NB: This function is nearly duplicated in charnames.pm
sub _getcode {
    my $arg = shift;

    if ($arg =~ /^[1-9]\d*$/) {
	return $arg;
    }
    elsif ($arg =~ /^(?:0[xX])?([[:xdigit:]]+)$/) {
	return CORE::hex($1);
    }
    elsif ($arg =~ /^[Uu]\+([[:xdigit:]]+)$/) { # Is of form U+0000, means
                                                # wants the Unicode code
                                                # point, not the native one
        my $decimal = CORE::hex($1);
        return $decimal if IS_ASCII_PLATFORM;
        return utf8::unicode_to_native($decimal);
    }

    return;
}

# Populated by _num.  Converts real number back to input rational
my %real_to_rational;

# To store the contents of files found on disk.
my @BIDIS;
my @CATEGORIES;
my @DECOMPOSITIONS;
my @NUMERIC_TYPES;
my %SIMPLE_LOWER;
my %SIMPLE_TITLE;
my %SIMPLE_UPPER;
my %UNICODE_1_NAMES;
my %ISO_COMMENT;

# Eval'd so can run on versions earlier than the property is available in
my $Hangul_Syllables_re = eval 'qr/\p{Block=Hangul_Syllables}/';

sub charinfo {

    # This function has traditionally mimicked what is in UnicodeData.txt,
    # warts and all.  This is a re-write that avoids UnicodeData.txt so that
    # it can be removed to save disk space.  Instead, this assembles
    # information gotten by other methods that get data from various other
    # files.  It uses charnames to get the character name; and various
    # mktables tables.

    use feature 'unicode_strings';

    # Will fail if called under minitest
    use if defined &DynaLoader::boot_DynaLoader, "Unicode::Normalize" => qw(getCombinClass NFD);

    my $arg  = shift;
    my $code = _getcode($arg);
    croak __PACKAGE__, "::charinfo: unknown code '$arg'" unless defined $code;

    # Non-unicode implies undef.
    return if $code > 0x10FFFF;

    my %prop;
    my $char = chr($code);

    @CATEGORIES =_read_table("To/Gc.pl") unless @CATEGORIES;
    $prop{'category'} = _search(\@CATEGORIES, 0, $#CATEGORIES, $code)
                        // $SwashInfo{'ToGc'}{'missing'};
    # Return undef if category value is 'Unassigned' or one of its synonyms
    return if grep { lc $_ eq 'unassigned' }
                                    prop_value_aliases('Gc', $prop{'category'});

    $prop{'code'} = sprintf "%04X", $code;
    $prop{'name'} = ($char =~ /\p{Cntrl}/) ? '<control>'
                                           : (charnames::viacode($code) // "");

    $prop{'combining'} = getCombinClass($code);

    @BIDIS =_read_table("To/Bc.pl") unless @BIDIS;
    $prop{'bidi'} = _search(\@BIDIS, 0, $#BIDIS, $code)
                    // $SwashInfo{'ToBc'}{'missing'};

    # For most code points, we can just read in "unicore/Decomposition.pl", as
    # its contents are exactly what should be output.  But that file doesn't
    # contain the data for the Hangul syllable decompositions, which can be
    # algorithmically computed, and NFD() does that, so we call NFD() for
    # those.  We can't use NFD() for everything, as it does a complete
    # recursive decomposition, and what this function has always done is to
    # return what's in UnicodeData.txt which doesn't show that recursiveness.
    # Fortunately, the NFD() of the Hanguls doesn't have any recursion
    # issues.
    # Having no decomposition implies an empty field; otherwise, all but
    # "Canonical" imply a compatible decomposition, and the type is prefixed
    # to that, as it is in UnicodeData.txt
    UnicodeVersion() unless defined $v_unicode_version;
    if ($v_unicode_version ge v2.0.0 && $char =~ $Hangul_Syllables_re) {
        # The code points of the decomposition are output in standard Unicode
        # hex format, separated by blanks.
        $prop{'decomposition'} = join " ", map { sprintf("%04X", $_)}
                                           unpack "U*", NFD($char);
    }
    else {
        @DECOMPOSITIONS = _read_table("Decomposition.pl")
                          unless @DECOMPOSITIONS;
        $prop{'decomposition'} = _search(\@DECOMPOSITIONS, 0, $#DECOMPOSITIONS,
                                                                $code) // "";
    }

    # Can use num() to get the numeric values, if any.
    if (! defined (my $value = num($char))) {
        $prop{'decimal'} = $prop{'digit'} = $prop{'numeric'} = "";
    }
    else {
        if ($char =~ /\d/) {
            $prop{'decimal'} = $prop{'digit'} = $prop{'numeric'} = $value;
        }
        else {

            # For non-decimal-digits, we have to read in the Numeric type
            # to distinguish them.  It is not just a matter of integer vs.
            # rational, as some whole number values are not considered digits,
            # e.g., TAMIL NUMBER TEN.
            $prop{'decimal'} = "";

            @NUMERIC_TYPES =_read_table("To/Nt.pl") unless @NUMERIC_TYPES;
            if ((_search(\@NUMERIC_TYPES, 0, $#NUMERIC_TYPES, $code) // "")
                eq 'Digit')
            {
                $prop{'digit'} = $prop{'numeric'} = $value;
            }
            else {
                $prop{'digit'} = "";
                $prop{'numeric'} = $real_to_rational{$value} // $value;
            }
        }
    }

    $prop{'mirrored'} = ($char =~ /\p{Bidi_Mirrored}/) ? 'Y' : 'N';

    %UNICODE_1_NAMES =_read_table("To/Na1.pl", "use_hash") unless %UNICODE_1_NAMES;
    $prop{'unicode10'} = $UNICODE_1_NAMES{$code} // "";

    UnicodeVersion() unless defined $v_unicode_version;
    if ($v_unicode_version ge v6.0.0) {
        $prop{'comment'} = "";
    }
    else {
        %ISO_COMMENT = _read_table("To/Isc.pl", "use_hash") unless %ISO_COMMENT;
        $prop{'comment'} = (defined $ISO_COMMENT{$code})
                           ? $ISO_COMMENT{$code}
                           : "";
    }

    %SIMPLE_UPPER = _read_table("To/Uc.pl", "use_hash") unless %SIMPLE_UPPER;
    $prop{'upper'} = (defined $SIMPLE_UPPER{$code})
                     ? sprintf("%04X", $SIMPLE_UPPER{$code})
                     : "";

    %SIMPLE_LOWER = _read_table("To/Lc.pl", "use_hash") unless %SIMPLE_LOWER;
    $prop{'lower'} = (defined $SIMPLE_LOWER{$code})
                     ? sprintf("%04X", $SIMPLE_LOWER{$code})
                     : "";

    %SIMPLE_TITLE = _read_table("To/Tc.pl", "use_hash") unless %SIMPLE_TITLE;
    $prop{'title'} = (defined $SIMPLE_TITLE{$code})
                     ? sprintf("%04X", $SIMPLE_TITLE{$code})
                     : "";

    $prop{block}  = charblock($code);
    $prop{script} = charscript($code);
    return \%prop;
}

sub _search { # Binary search in a [[lo,hi,prop],[...],...] table.
    my ($table, $lo, $hi, $code) = @_;

    return if $lo > $hi;

    my $mid = int(($lo+$hi) / 2);

    if ($table->[$mid]->[0] < $code) {
	if ($table->[$mid]->[1] >= $code) {
	    return $table->[$mid]->[2];
	} else {
	    _search($table, $mid + 1, $hi, $code);
	}
    } elsif ($table->[$mid]->[0] > $code) {
	_search($table, $lo, $mid - 1, $code);
    } else {
	return $table->[$mid]->[2];
    }
}

sub _read_table ($;$) {

    # Returns the contents of the mktables generated table file located at $1
    # in the form of either an array of arrays or a hash, depending on if the
    # optional second parameter is true (for hash return) or not.  In the case
    # of a hash return, each key is a code point, and its corresponding value
    # is what the table gives as the code point's corresponding value.  In the
    # case of an array return, each outer array denotes a range with [0] the
    # start point of that range; [1] the end point; and [2] the value that
    # every code point in the range has.  The hash return is useful for fast
    # lookup when the table contains only single code point ranges.  The array
    # return takes much less memory when there are large ranges.
    #
    # This function has the side effect of setting
    # $SwashInfo{$property}{'format'} to be the mktables format of the
    #                                       table; and
    # $SwashInfo{$property}{'missing'} to be the value for all entries
    #                                        not listed in the table.
    # where $property is the Unicode property name, preceded by 'To' for map
    # properties., e.g., 'ToSc'.
    #
    # Table entries look like one of:
    # 0000	0040	Common	# [65]
    # 00AA		Latin

    my $table = shift;
    my $return_hash = shift;
    $return_hash = 0 unless defined $return_hash;
    my @return;
    my %return;
    local $_;
    my $list = do "unicore/$table";

    # Look up if this property requires adjustments, which we do below if it
    # does.
    require "unicore/UCD.pl";
    my $property = $table =~ s/\.pl//r;
    $property = $file_to_swash_name{$property};
    my $to_adjust = defined $property
                    && $SwashInfo{$property}{'format'} =~ / ^ a /x;

    for (split /^/m, $list) {
        my ($start, $end, $value) = / ^ (.+?) \t (.*?) \t (.+?)
                                        \s* ( \# .* )?  # Optional comment
                                        $ /x;
        my $decimal_start = hex $start;
        my $decimal_end = ($end eq "") ? $decimal_start : hex $end;
        $value = hex $value if $to_adjust
                               && $SwashInfo{$property}{'format'} eq 'ax';
        if ($return_hash) {
            foreach my $i ($decimal_start .. $decimal_end) {
                $return{$i} = ($to_adjust)
                              ? $value + $i - $decimal_start
                              : $value;
            }
        }
        elsif (! $to_adjust
               && @return
               && $return[-1][1] == $decimal_start - 1
               && $return[-1][2] eq $value)
        {
            # If this is merely extending the previous range, do just that.
            $return[-1]->[1] = $decimal_end;
        }
        else {
            push @return, [ $decimal_start, $decimal_end, $value ];
        }
    }
    return ($return_hash) ? %return : @return;
}

sub charinrange {
    my ($range, $arg) = @_;
    my $code = _getcode($arg);
    croak __PACKAGE__, "::charinrange: unknown code '$arg'"
	unless defined $code;
    _search($range, 0, $#$range, $code);
}

=head2 B<charprop()>

    use Unicode::UCD 'charprop';

    print charprop(0x41, "Gc"), "\n";
    print charprop(0x61, "General_Category"), "\n";

  prints
    Lu
    Ll

This returns the value of the Unicode property given by the second parameter
for the  L</code point argument> given by the first.

The passed-in property may be specified as any of the synonyms returned by
L</prop_aliases()>.

The return value is always a scalar, either a string or a number.  For
properties where there are synonyms for the values, the synonym returned by
this function is the longest, most descriptive form, the one returned by
L</prop_value_aliases()> when called in a scalar context.  Of course, you can
call L</prop_value_aliases()> on the result to get other synonyms.

The return values are more "cooked" than the L</charinfo()> ones.  For
example, the C<"uc"> property value is the actual string containing the full
uppercase mapping of the input code point.  You have to go to extra trouble
with C<charinfo> to get this value from its C<upper> hash element when the
full mapping differs from the simple one.

Special note should be made of the return values for a few properties:

=over

=item Block

The value returned is the new-style (see L</Old-style versus new-style block
names>).

=item Decomposition_Mapping

Like L</charinfo()>, the result may be an intermediate decomposition whose
components are also decomposable.  Use L<Unicode::Normalize> to get the final
decomposition in one step.

Unlike L</charinfo()>, this does not include the decomposition type.  Use the
C<Decomposition_Type> property to get that.

=item Name_Alias

If the input code point's name has more than one synonym, they are returned
joined into a single comma-separated string.

=item Numeric_Value

If the result is a fraction, it is converted into a floating point number to
the accuracy of your platform.

=item Script_Extensions

If the result is multiple script names, they are returned joined into a single
comma-separated string.

=back

When called with a property that is a Perl extension that isn't expressible in
a compound form, this function currently returns C<undef>, as the only two
possible values are I<true> or I<false> (1 or 0 I suppose).  This behavior may
change in the future, so don't write code that relies on it.  C<Present_In> is
a Perl extension that is expressible in a bipartite or compound form (for
example, C<\p{Present_In=4.0}>), so C<charprop> accepts it.  But C<Any> is a
Perl extension that isn't expressible that way, so C<charprop> returns
C<undef> for it.  Also C<charprop> returns C<undef> for all Perl extensions
that are internal-only.

=cut

sub charprop ($$;$) {
    my ($input_cp, $prop, $internal_ok) = @_;

    my $cp = _getcode($input_cp);
    croak __PACKAGE__, "::charprop: unknown code point '$input_cp'" unless defined $cp;

    my ($list_ref, $map_ref, $format, $default)
                                      = prop_invmap($prop, $internal_ok);
    return undef unless defined $list_ref;

    my $i = search_invlist($list_ref, $cp);
    croak __PACKAGE__, "::charprop: prop_invmap return is invalid for charprop('$input_cp', '$prop)" unless defined $i;

    # $i is the index into both the inversion list and map of $cp.
    my $map = $map_ref->[$i];

    # Convert enumeration values to their most complete form.
    if (! ref $map) {
        my $long_form = prop_value_aliases($prop, $map);
        $map = $long_form if defined $long_form;
    }

    if ($format =~ / ^ s /x) {  # Scalars
        return join ",", @$map if ref $map; # Convert to scalar with comma
                                            # separated array elements

        # Resolve ambiguity as to whether an all digit value is a code point
        # that should be converted to a character, or whether it is really
        # just a number.  To do this, look at the default.  If it is a
        # non-empty number, we can safely assume the result is also a number.
        if ($map =~ / ^ \d+ $ /ax && $default !~ / ^ \d+ $ /ax) {
            $map = chr $map;
        }
        elsif ($map =~ / ^ (?: Y | N ) $ /x) {

            # prop_invmap() returns these values for properties that are Perl
            # extensions.  But this is misleading.  For now, return undef for
            # these, as currently documented.
            undef $map unless
                exists $prop_aliases{loose_name(lc $prop)};
        }
        return $map;
    }
    elsif ($format eq 'ar') {   # numbers, including rationals
        my $offset = $cp - $list_ref->[$i];
        return $map if $map =~ /nan/i;
        return $map + $offset if $offset != 0;  # If needs adjustment
        return eval $map;   # Convert e.g., 1/2 to 0.5
    }
    elsif ($format =~ /^a/) {   # Some entries need adjusting

        # Linearize sequences into a string.
        return join "", map { chr $_ } @$map if ref $map; # XXX && $format =~ /^ a [dl] /x;

        return "" if $map eq "" && $format =~ /^a.*e/;

        # These are all character mappings.  Return the chr if no adjustment
        # is needed
        return chr $cp if $map eq "0";

        # Convert special entry.
        if ($map eq '<hangul syllable>' && $format eq 'ad') {
            use Unicode::Normalize qw(NFD);
            return NFD(chr $cp);
        }

        # The rest need adjustment from the first entry in the inversion list
        # corresponding to this map.
        my $offset = $cp - $list_ref->[$i];
        return chr($map + $cp - $list_ref->[$i]);
    }
    elsif ($format eq 'n') {    # The name property

        # There are two special cases, handled here.
        if ($map =~ / ( .+ ) <code\ point> $ /x) {
            $map = sprintf("$1%04X", $cp);
        }
        elsif ($map eq '<hangul syllable>') {
            $map = charnames::viacode($cp);
        }
        return $map;
    }
    else {
        croak __PACKAGE__, "::charprop: Internal error: unknown format '$format'.  Please perlbug this";
    }
}

=head2 B<charprops_all()>

    use Unicode::UCD 'charprops_all';

    my $%properties_of_A_hash_ref = charprops_all("U+41");

This returns a reference to a hash whose keys are all the distinct Unicode (no
Perl extension) properties, and whose values are the respective values for
those properties for the input L</code point argument>.

Each key is the property name in its longest, most descriptive form.  The
values are what L</charprop()> would return.

This function is expensive in time and memory.

=cut

sub charprops_all($) {
    my $input_cp = shift;

    my $cp = _getcode($input_cp);
    croak __PACKAGE__, "::charprops_all: unknown code point '$input_cp'" unless defined $cp;

    my %return;

    require "unicore/UCD.pl";

    foreach my $prop (keys %prop_aliases) {

        # Don't return a Perl extension.  (This is the only one that
        # %prop_aliases has in it.)
        next if $prop eq 'perldecimaldigit';

        # Use long name for $prop in the hash
        $return{scalar prop_aliases($prop)} = charprop($cp, $prop);
    }

    return \%return;
}

=head2 B<charblock()>

    use Unicode::UCD 'charblock';

    my $charblock = charblock(0x41);
    my $charblock = charblock(1234);
    my $charblock = charblock(0x263a);
    my $charblock = charblock("U+263a");

    my $range     = charblock('Armenian');

With a L</code point argument> C<charblock()> returns the I<block> the code point
belongs to, e.g.  C<Basic Latin>.  The old-style block name is returned (see
L</Old-style versus new-style block names>).
The L</prop_value_aliases()> function can be used to get all the synonyms
of the block name.

If the code point is unassigned, this returns the block it would belong to if
it were assigned.  (If the Unicode version being used is so early as to not
have blocks, all code points are considered to be in C<No_Block>.)

See also L</Blocks versus Scripts>.

If supplied with an argument that can't be a code point, C<charblock()> tries to
do the opposite and interpret the argument as an old-style block name.  On an
ASCII platform, the return value is a I<range set> with one range: an
anonymous array with a single element that consists of another anonymous array
whose first element is the first code point in the block, and whose second
element is the final code point in the block.  On an EBCDIC
platform, the first two Unicode blocks are not contiguous.  Their range sets
are lists containing I<start-of-range>, I<end-of-range> code point pairs.  You
can test whether a code point is in a range set using the L</charinrange()>
function.  (To be precise, each I<range set> contains a third array element,
after the range boundary ones: the old_style block name.)

If the argument to C<charblock()> is not a known block, C<undef> is
returned.

=cut

my @BLOCKS;
my %BLOCKS;

sub _charblocks {

    # Can't read from the mktables table because it loses the hyphens in the
    # original.
    unless (@BLOCKS) {
        UnicodeVersion() unless defined $v_unicode_version;
        if ($v_unicode_version lt v2.0.0) {
            my $subrange = [ 0, 0x10FFFF, 'No_Block' ];
            push @BLOCKS, $subrange;
            push @{$BLOCKS{'No_Block'}}, $subrange;
        }
        else {
            my $blocksfh = openunicode("Blocks.txt");
	    local $_;
	    local $/ = "\n";
	    while (<$blocksfh>) {

                # Old versions used a different syntax to mark the range.
                $_ =~ s/;\s+/../ if $v_unicode_version lt v3.1.0;

		if (/^([0-9A-F]+)\.\.([0-9A-F]+);\s+(.+)/) {
		    my ($lo, $hi) = (hex($1), hex($2));
		    my $subrange = [ $lo, $hi, $3 ];
		    push @BLOCKS, $subrange;
		    push @{$BLOCKS{$3}}, $subrange;
		}
	    }
            if (! IS_ASCII_PLATFORM) {
                # The first two blocks, through 0xFF, are wrong on EBCDIC
                # platforms.

                my @new_blocks = _read_table("To/Blk.pl");

                # Get rid of the first two ranges in the Unicode version, and
                # replace them with the ones computed by mktables.
                shift @BLOCKS;
                shift @BLOCKS;
                delete $BLOCKS{'Basic Latin'};
                delete $BLOCKS{'Latin-1 Supplement'};

                # But there are multiple entries in the computed versions, and
                # we change their names to (which we know) to be the old-style
                # ones.
                for my $i (0.. @new_blocks - 1) {
                    if ($new_blocks[$i][2] =~ s/Basic_Latin/Basic Latin/
                        or $new_blocks[$i][2] =~
                                    s/Latin_1_Supplement/Latin-1 Supplement/)
                    {
                        push @{$BLOCKS{$new_blocks[$i][2]}}, $new_blocks[$i];
                    }
                    else {
                        splice @new_blocks, $i;
                        last;
                    }
                }
                unshift @BLOCKS, @new_blocks;
            }
	}
    }
}

sub charblock {
    my $arg = shift;

    _charblocks() unless @BLOCKS;

    my $code = _getcode($arg);

    if (defined $code) {
	my $result = _search(\@BLOCKS, 0, $#BLOCKS, $code);
        return $result if defined $result;
        return 'No_Block';
    }
    elsif (exists $BLOCKS{$arg}) {
        return _dclone $BLOCKS{$arg};
    }

    carp __PACKAGE__, "::charblock: unknown code '$arg'";
    return;
}

=head2 B<charscript()>

    use Unicode::UCD 'charscript';

    my $charscript = charscript(0x41);
    my $charscript = charscript(1234);
    my $charscript = charscript("U+263a");

    my $range      = charscript('Thai');

With a L</code point argument>, C<charscript()> returns the I<script> the
code point belongs to, e.g., C<Latin>, C<Greek>, C<Han>.
If the code point is unassigned or the Unicode version being used is so early
that it doesn't have scripts, this function returns C<"Unknown">.
The L</prop_value_aliases()> function can be used to get all the synonyms
of the script name.

Note that the Script_Extensions property is an improved version of the Script
property, and you should probably be using that instead, with the
L</charprop()> function.

If supplied with an argument that can't be a code point, charscript() tries
to do the opposite and interpret the argument as a script name. The
return value is a I<range set>: an anonymous array of arrays that contain
I<start-of-range>, I<end-of-range> code point pairs. You can test whether a
code point is in a range set using the L</charinrange()> function.
(To be precise, each I<range set> contains a third array element,
after the range boundary ones: the script name.)

If the C<charscript()> argument is not a known script, C<undef> is returned.

See also L</Blocks versus Scripts>.

=cut

my @SCRIPTS;
my %SCRIPTS;

sub _charscripts {
    unless (@SCRIPTS) {
        UnicodeVersion() unless defined $v_unicode_version;
        if ($v_unicode_version lt v3.1.0) {
            push @SCRIPTS, [ 0, 0x10FFFF, 'Unknown' ];
        }
        else {
            @SCRIPTS =_read_table("To/Sc.pl");
        }
    }
    foreach my $entry (@SCRIPTS) {
        $entry->[2] =~ s/(_\w)/\L$1/g;  # Preserve old-style casing
        push @{$SCRIPTS{$entry->[2]}}, $entry;
    }
}

sub charscript {
    my $arg = shift;

    _charscripts() unless @SCRIPTS;

    my $code = _getcode($arg);

    if (defined $code) {
	my $result = _search(\@SCRIPTS, 0, $#SCRIPTS, $code);
        return $result if defined $result;
        return $SwashInfo{'ToSc'}{'missing'};
    } elsif (exists $SCRIPTS{$arg}) {
        return _dclone $SCRIPTS{$arg};
    }

    carp __PACKAGE__, "::charscript: unknown code '$arg'";
    return;
}

=head2 B<charblocks()>

    use Unicode::UCD 'charblocks';

    my $charblocks = charblocks();

C<charblocks()> returns a reference to a hash with the known block names
as the keys, and the code point ranges (see L</charblock()>) as the values.

The names are in the old-style (see L</Old-style versus new-style block
names>).

L<prop_invmap("block")|/prop_invmap()> can be used to get this same data in a
different type of data structure.

L<prop_values("Block")|/prop_values()> can be used to get all
the known new-style block names as a list, without the code point ranges.

See also L</Blocks versus Scripts>.

=cut

sub charblocks {
    _charblocks() unless %BLOCKS;
    return _dclone \%BLOCKS;
}

=head2 B<charscripts()>

    use Unicode::UCD 'charscripts';

    my $charscripts = charscripts();

C<charscripts()> returns a reference to a hash with the known script
names as the keys, and the code point ranges (see L</charscript()>) as
the values.

L<prop_invmap("script")|/prop_invmap()> can be used to get this same data in a
different type of data structure.  Since the Script_Extensions property is an
improved version of the Script property, you should instead use
L<prop_invmap("scx")|/prop_invmap()>.

L<C<prop_values("Script")>|/prop_values()> can be used to get all
the known script names as a list, without the code point ranges.

See also L</Blocks versus Scripts>.

=cut

sub charscripts {
    _charscripts() unless %SCRIPTS;
    return _dclone \%SCRIPTS;
}

=head2 B<charinrange()>

In addition to using the C<\p{Blk=...}> and C<\P{Blk=...}> constructs, you
can also test whether a code point is in the I<range> as returned by
L</charblock()> and L</charscript()> or as the values of the hash returned
by L</charblocks()> and L</charscripts()> by using C<charinrange()>:

    use Unicode::UCD qw(charscript charinrange);

    $range = charscript('Hiragana');
    print "looks like hiragana\n" if charinrange($range, $codepoint);

=cut

my %GENERAL_CATEGORIES =
 (
    'L'  =>         'Letter',
    'LC' =>         'CasedLetter',
    'Lu' =>         'UppercaseLetter',
    'Ll' =>         'LowercaseLetter',
    'Lt' =>         'TitlecaseLetter',
    'Lm' =>         'ModifierLetter',
    'Lo' =>         'OtherLetter',
    'M'  =>         'Mark',
    'Mn' =>         'NonspacingMark',
    'Mc' =>         'SpacingMark',
    'Me' =>         'EnclosingMark',
    'N'  =>         'Number',
    'Nd' =>         'DecimalNumber',
    'Nl' =>         'LetterNumber',
    'No' =>         'OtherNumber',
    'P'  =>         'Punctuation',
    'Pc' =>         'ConnectorPunctuation',
    'Pd' =>         'DashPunctuation',
    'Ps' =>         'OpenPunctuation',
    'Pe' =>         'ClosePunctuation',
    'Pi' =>         'InitialPunctuation',
    'Pf' =>         'FinalPunctuation',
    'Po' =>         'OtherPunctuation',
    'S'  =>         'Symbol',
    'Sm' =>         'MathSymbol',
    'Sc' =>         'CurrencySymbol',
    'Sk' =>         'ModifierSymbol',
    'So' =>         'OtherSymbol',
    'Z'  =>         'Separator',
    'Zs' =>         'SpaceSeparator',
    'Zl' =>         'LineSeparator',
    'Zp' =>         'ParagraphSeparator',
    'C'  =>         'Other',
    'Cc' =>         'Control',
    'Cf' =>         'Format',
    'Cs' =>         'Surrogate',
    'Co' =>         'PrivateUse',
    'Cn' =>         'Unassigned',
 );

sub general_categories {
    return _dclone \%GENERAL_CATEGORIES;
}

=head2 B<general_categories()>

    use Unicode::UCD 'general_categories';

    my $categories = general_categories();

This returns a reference to a hash which has short
general category names (such as C<Lu>, C<Nd>, C<Zs>, C<S>) as keys and long
names (such as C<UppercaseLetter>, C<DecimalNumber>, C<SpaceSeparator>,
C<Symbol>) as values.  The hash is reversible in case you need to go
from the long names to the short names.  The general category is the
one returned from
L</charinfo()> under the C<category> key.

The L</prop_values()> and L</prop_value_aliases()> functions can be used as an
alternative to this function; the first returning a simple list of the short
category names; and the second gets all the synonyms of a given category name.

=cut

my %BIDI_TYPES =
 (
   'L'   => 'Left-to-Right',
   'LRE' => 'Left-to-Right Embedding',
   'LRO' => 'Left-to-Right Override',
   'R'   => 'Right-to-Left',
   'AL'  => 'Right-to-Left Arabic',
   'RLE' => 'Right-to-Left Embedding',
   'RLO' => 'Right-to-Left Override',
   'PDF' => 'Pop Directional Format',
   'EN'  => 'European Number',
   'ES'  => 'European Number Separator',
   'ET'  => 'European Number Terminator',
   'AN'  => 'Arabic Number',
   'CS'  => 'Common Number Separator',
   'NSM' => 'Non-Spacing Mark',
   'BN'  => 'Boundary Neutral',
   'B'   => 'Paragraph Separator',
   'S'   => 'Segment Separator',
   'WS'  => 'Whitespace',
   'ON'  => 'Other Neutrals',
 );

=head2 B<bidi_types()>

    use Unicode::UCD 'bidi_types';

    my $categories = bidi_types();

This returns a reference to a hash which has the short
bidi (bidirectional) type names (such as C<L>, C<R>) as keys and long
names (such as C<Left-to-Right>, C<Right-to-Left>) as values.  The
hash is reversible in case you need to go from the long names to the
short names.  The bidi type is the one returned from
L</charinfo()>
under the C<bidi> key.  For the exact meaning of the various bidi classes
the Unicode TR9 is recommended reading:
L<http://www.unicode.org/reports/tr9/>
(as of Unicode 5.0.0)

The L</prop_values()> and L</prop_value_aliases()> functions can be used as an
alternative to this function; the first returning a simple list of the short
bidi type names; and the second gets all the synonyms of a given bidi type
name.

=cut

sub bidi_types {
    return _dclone \%BIDI_TYPES;
}

=head2 B<compexcl()>

WARNING: Unicode discourages the use of this function or any of the
alternative mechanisms listed in this section (the documentation of
C<compexcl()>), except internally in implementations of the Unicode
Normalization Algorithm.  You should be using L<Unicode::Normalize> directly
instead of these.  Using these will likely lead to half-baked results.

    use Unicode::UCD 'compexcl';

    my $compexcl = compexcl(0x09dc);

This routine returns C<undef> if the Unicode version being used is so early
that it doesn't have this property.

C<compexcl()> is included for backwards
compatibility, but as of Perl 5.12 and more modern Unicode versions, for
most purposes it is probably more convenient to use one of the following
instead:

    my $compexcl = chr(0x09dc) =~ /\p{Comp_Ex};
    my $compexcl = chr(0x09dc) =~ /\p{Full_Composition_Exclusion};

or even

    my $compexcl = chr(0x09dc) =~ /\p{CE};
    my $compexcl = chr(0x09dc) =~ /\p{Composition_Exclusion};

The first two forms return B<true> if the L</code point argument> should not
be produced by composition normalization.  For the final two forms to return
B<true>, it is additionally required that this fact not otherwise be
determinable from the Unicode data base.

This routine behaves identically to the final two forms.  That is,
it does not return B<true> if the code point has a decomposition
consisting of another single code point, nor if its decomposition starts
with a code point whose combining class is non-zero.  Code points that meet
either of these conditions should also not be produced by composition
normalization, which is probably why you should use the
C<Full_Composition_Exclusion> property instead, as shown above.

The routine returns B<false> otherwise.

=cut

# Eval'd so can run on versions earlier than the property is available in
my $Composition_Exclusion_re = eval 'qr/\p{Composition_Exclusion}/';

sub compexcl {
    my $arg  = shift;
    my $code = _getcode($arg);
    croak __PACKAGE__, "::compexcl: unknown code '$arg'"
	unless defined $code;

    UnicodeVersion() unless defined $v_unicode_version;
    return if $v_unicode_version lt v3.0.0;

    no warnings "non_unicode";     # So works on non-Unicode code points
    return chr($code) =~ $Composition_Exclusion_re
}

=head2 B<casefold()>

    use Unicode::UCD 'casefold';

    my $casefold = casefold(0xDF);
    if (defined $casefold) {
        my @full_fold_hex = split / /, $casefold->{'full'};
        my $full_fold_string =
                    join "", map {chr(hex($_))} @full_fold_hex;
        my @turkic_fold_hex =
                        split / /, ($casefold->{'turkic'} ne "")
                                        ? $casefold->{'turkic'}
                                        : $casefold->{'full'};
        my $turkic_fold_string =
                        join "", map {chr(hex($_))} @turkic_fold_hex;
    }
    if (defined $casefold && $casefold->{'simple'} ne "") {
        my $simple_fold_hex = $casefold->{'simple'};
        my $simple_fold_string = chr(hex($simple_fold_hex));
    }

This returns the (almost) locale-independent case folding of the
character specified by the L</code point argument>.  (Starting in Perl v5.16,
the core function C<fc()> returns the C<full> mapping (described below)
faster than this does, and for entire strings.)

If there is no case folding for the input code point, C<undef> is returned.

If there is a case folding for that code point, a reference to a hash
with the following fields is returned:

=over

=item B<code>

the input native L</code point argument> expressed in hexadecimal, with
leading zeros
added if necessary to make it contain at least four hexdigits

=item B<full>

one or more codes (separated by spaces) that, taken in order, give the
code points for the case folding for I<code>.
Each has at least four hexdigits.

=item B<simple>

is empty, or is exactly one code with at least four hexdigits which can be used
as an alternative case folding when the calling program cannot cope with the
fold being a sequence of multiple code points.  If I<full> is just one code
point, then I<simple> equals I<full>.  If there is no single code point folding
defined for I<code>, then I<simple> is the empty string.  Otherwise, it is an
inferior, but still better-than-nothing alternative folding to I<full>.

=item B<mapping>

is the same as I<simple> if I<simple> is not empty, and it is the same as I<full>
otherwise.  It can be considered to be the simplest possible folding for
I<code>.  It is defined primarily for backwards compatibility.

=item B<status>

is C<C> (for C<common>) if the best possible fold is a single code point
(I<simple> equals I<full> equals I<mapping>).  It is C<S> if there are distinct
folds, I<simple> and I<full> (I<mapping> equals I<simple>).  And it is C<F> if
there is only a I<full> fold (I<mapping> equals I<full>; I<simple> is empty).
Note that this
describes the contents of I<mapping>.  It is defined primarily for backwards
compatibility.

For Unicode versions between 3.1 and 3.1.1 inclusive, I<status> can also be
C<I> which is the same as C<C> but is a special case for dotted uppercase I and
dotless lowercase i:

=over

=item Z<>B<*> If you use this C<I> mapping

the result is case-insensitive,
but dotless and dotted I's are not distinguished

=item Z<>B<*> If you exclude this C<I> mapping

the result is not fully case-insensitive, but
dotless and dotted I's are distinguished

=back

=item B<turkic>

contains any special folding for Turkic languages.  For versions of Unicode
starting with 3.2, this field is empty unless I<code> has a different folding
in Turkic languages, in which case it is one or more codes (separated by
spaces) that, taken in order, give the code points for the case folding for
I<code> in those languages.
Each code has at least four hexdigits.
Note that this folding does not maintain canonical equivalence without
additional processing.

For Unicode versions between 3.1 and 3.1.1 inclusive, this field is empty unless
there is a
special folding for Turkic languages, in which case I<status> is C<I>, and
I<mapping>, I<full>, I<simple>, and I<turkic> are all equal.

=back

Programs that want complete generality and the best folding results should use
the folding contained in the I<full> field.  But note that the fold for some
code points will be a sequence of multiple code points.

Programs that can't cope with the fold mapping being multiple code points can
use the folding contained in the I<simple> field, with the loss of some
generality.  In Unicode 5.1, about 7% of the defined foldings have no single
code point folding.

The I<mapping> and I<status> fields are provided for backwards compatibility for
existing programs.  They contain the same values as in previous versions of
this function.

Locale is not completely independent.  The I<turkic> field contains results to
use when the locale is a Turkic language.

For more information about case mappings see
L<http://www.unicode.org/reports/tr21>

=cut

my %CASEFOLD;

sub _casefold {
    unless (%CASEFOLD) {   # Populate the hash
        my ($full_invlist_ref, $full_invmap_ref, undef, $default)
                                                = prop_invmap('Case_Folding');

        # Use the recipe given in the prop_invmap() pod to convert the
        # inversion map into the hash.
        for my $i (0 .. @$full_invlist_ref - 1 - 1) {
            next if $full_invmap_ref->[$i] == $default;
            my $adjust = -1;
            for my $j ($full_invlist_ref->[$i] .. $full_invlist_ref->[$i+1] -1) {
                $adjust++;
                if (! ref $full_invmap_ref->[$i]) {

                    # This is a single character mapping
                    $CASEFOLD{$j}{'status'} = 'C';
                    $CASEFOLD{$j}{'simple'}
                        = $CASEFOLD{$j}{'full'}
                        = $CASEFOLD{$j}{'mapping'}
                        = sprintf("%04X", $full_invmap_ref->[$i] + $adjust);
                    $CASEFOLD{$j}{'code'} = sprintf("%04X", $j);
                    $CASEFOLD{$j}{'turkic'} = "";
                }
                else {  # prop_invmap ensures that $adjust is 0 for a ref
                    $CASEFOLD{$j}{'status'} = 'F';
                    $CASEFOLD{$j}{'full'}
                    = $CASEFOLD{$j}{'mapping'}
                    = join " ", map { sprintf "%04X", $_ }
                                                    @{$full_invmap_ref->[$i]};
                    $CASEFOLD{$j}{'simple'} = "";
                    $CASEFOLD{$j}{'code'} = sprintf("%04X", $j);
                    $CASEFOLD{$j}{'turkic'} = "";
                }
            }
        }

        # We have filled in the full mappings above, assuming there were no
        # simple ones for the ones with multi-character maps.  Now, we find
        # and fix the cases where that assumption was false.
        (my ($simple_invlist_ref, $simple_invmap_ref, undef), $default)
                                        = prop_invmap('Simple_Case_Folding');
        for my $i (0 .. @$simple_invlist_ref - 1 - 1) {
            next if $simple_invmap_ref->[$i] == $default;
            my $adjust = -1;
            for my $j ($simple_invlist_ref->[$i]
                       .. $simple_invlist_ref->[$i+1] -1)
            {
                $adjust++;
                next if $CASEFOLD{$j}{'status'} eq 'C';
                $CASEFOLD{$j}{'status'} = 'S';
                $CASEFOLD{$j}{'simple'}
                    = $CASEFOLD{$j}{'mapping'}
                    = sprintf("%04X", $simple_invmap_ref->[$i] + $adjust);
                $CASEFOLD{$j}{'code'} = sprintf("%04X", $j);
                $CASEFOLD{$j}{'turkic'} = "";
            }
        }

        # We hard-code in the turkish rules
        UnicodeVersion() unless defined $v_unicode_version;
        if ($v_unicode_version ge v3.2.0) {

            # These two code points should already have regular entries, so
            # just fill in the turkish fields
            $CASEFOLD{ord('I')}{'turkic'} = '0131';
            $CASEFOLD{0x130}{'turkic'} = sprintf "%04X", ord('i');
        }
        elsif ($v_unicode_version ge v3.1.0) {

            # These two code points don't have entries otherwise.
            $CASEFOLD{0x130}{'code'} = '0130';
            $CASEFOLD{0x131}{'code'} = '0131';
            $CASEFOLD{0x130}{'status'} = $CASEFOLD{0x131}{'status'} = 'I';
            $CASEFOLD{0x130}{'turkic'}
                = $CASEFOLD{0x130}{'mapping'}
                = $CASEFOLD{0x130}{'full'}
                = $CASEFOLD{0x130}{'simple'}
                = $CASEFOLD{0x131}{'turkic'}
                = $CASEFOLD{0x131}{'mapping'}
                = $CASEFOLD{0x131}{'full'}
                = $CASEFOLD{0x131}{'simple'}
                = sprintf "%04X", ord('i');
        }
    }
}

sub casefold {
    my $arg  = shift;
    my $code = _getcode($arg);
    croak __PACKAGE__, "::casefold: unknown code '$arg'"
	unless defined $code;

    _casefold() unless %CASEFOLD;

    return $CASEFOLD{$code};
}

=head2 B<all_casefolds()>


    use Unicode::UCD 'all_casefolds';

    my $all_folds_ref = all_casefolds();
    foreach my $char_with_casefold (sort { $a <=> $b }
                                    keys %$all_folds_ref)
    {
        printf "%04X:", $char_with_casefold;
        my $casefold = $all_folds_ref->{$char_with_casefold};

        # Get folds for $char_with_casefold

        my @full_fold_hex = split / /, $casefold->{'full'};
        my $full_fold_string =
                    join "", map {chr(hex($_))} @full_fold_hex;
        print " full=", join " ", @full_fold_hex;
        my @turkic_fold_hex =
                        split / /, ($casefold->{'turkic'} ne "")
                                        ? $casefold->{'turkic'}
                                        : $casefold->{'full'};
        my $turkic_fold_string =
                        join "", map {chr(hex($_))} @turkic_fold_hex;
        print "; turkic=", join " ", @turkic_fold_hex;
        if (defined $casefold && $casefold->{'simple'} ne "") {
            my $simple_fold_hex = $casefold->{'simple'};
            my $simple_fold_string = chr(hex($simple_fold_hex));
            print "; simple=$simple_fold_hex";
        }
        print "\n";
    }

This returns all the case foldings in the current version of Unicode in the
form of a reference to a hash.  Each key to the hash is the decimal
representation of a Unicode character that has a casefold to other than
itself.  The casefold of a semi-colon is itself, so it isn't in the hash;
likewise for a lowercase "a", but there is an entry for a capital "A".  The
hash value for each key is another hash, identical to what is returned by
L</casefold()> if called with that code point as its argument.  So the value
C<< all_casefolds()->{ord("A")}' >> is equivalent to C<casefold(ord("A"))>;

=cut

sub all_casefolds () {
    _casefold() unless %CASEFOLD;
    return _dclone \%CASEFOLD;
}

=head2 B<casespec()>

    use Unicode::UCD 'casespec';

    my $casespec = casespec(0xFB00);

This returns the potentially locale-dependent case mappings of the L</code point
argument>.  The mappings may be longer than a single code point (which the basic
Unicode case mappings as returned by L</charinfo()> never are).

If there are no case mappings for the L</code point argument>, or if all three
possible mappings (I<lower>, I<title> and I<upper>) result in single code
points and are locale independent and unconditional, C<undef> is returned
(which means that the case mappings, if any, for the code point are those
returned by L</charinfo()>).

Otherwise, a reference to a hash giving the mappings (or a reference to a hash
of such hashes, explained below) is returned with the following keys and their
meanings:

The keys in the bottom layer hash with the meanings of their values are:

=over

=item B<code>

the input native L</code point argument> expressed in hexadecimal, with
leading zeros
added if necessary to make it contain at least four hexdigits

=item B<lower>

one or more codes (separated by spaces) that, taken in order, give the
code points for the lower case of I<code>.
Each has at least four hexdigits.

=item B<title>

one or more codes (separated by spaces) that, taken in order, give the
code points for the title case of I<code>.
Each has at least four hexdigits.

=item B<upper>

one or more codes (separated by spaces) that, taken in order, give the
code points for the upper case of I<code>.
Each has at least four hexdigits.

=item B<condition>

the conditions for the mappings to be valid.
If C<undef>, the mappings are always valid.
When defined, this field is a list of conditions,
all of which must be true for the mappings to be valid.
The list consists of one or more
I<locales> (see below)
and/or I<contexts> (explained in the next paragraph),
separated by spaces.
(Other than as used to separate elements, spaces are to be ignored.)
Case distinctions in the condition list are not significant.
Conditions preceded by "NON_" represent the negation of the condition.

A I<context> is one of those defined in the Unicode standard.
For Unicode 5.1, they are defined in Section 3.13 C<Default Case Operations>
available at
L<http://www.unicode.org/versions/Unicode5.1.0/>.
These are for context-sensitive casing.

=back

The hash described above is returned for locale-independent casing, where
at least one of the mappings has length longer than one.  If C<undef> is
returned, the code point may have mappings, but if so, all are length one,
and are returned by L</charinfo()>.
Note that when this function does return a value, it will be for the complete
set of mappings for a code point, even those whose length is one.

If there are additional casing rules that apply only in certain locales,
an additional key for each will be defined in the returned hash.  Each such key
will be its locale name, defined as a 2-letter ISO 3166 country code, possibly
followed by a "_" and a 2-letter ISO language code (possibly followed by a "_"
and a variant code).  You can find the lists of all possible locales, see
L<Locale::Country> and L<Locale::Language>.
(In Unicode 6.0, the only locales returned by this function
are C<lt>, C<tr>, and C<az>.)

Each locale key is a reference to a hash that has the form above, and gives
the casing rules for that particular locale, which take precedence over the
locale-independent ones when in that locale.

If the only casing for a code point is locale-dependent, then the returned
hash will not have any of the base keys, like C<code>, C<upper>, etc., but
will contain only locale keys.

For more information about case mappings see
L<http://www.unicode.org/reports/tr21/>

=cut

my %CASESPEC;

sub _casespec {
    unless (%CASESPEC) {
        UnicodeVersion() unless defined $v_unicode_version;
        if ($v_unicode_version ge v2.1.8) {
            my $casespecfh = openunicode("SpecialCasing.txt");
	    local $_;
	    local $/ = "\n";
	    while (<$casespecfh>) {
		if (/^([0-9A-F]+); ([0-9A-F]+(?: [0-9A-F]+)*)?; ([0-9A-F]+(?: [0-9A-F]+)*)?; ([0-9A-F]+(?: [0-9A-F]+)*)?; (\w+(?: \w+)*)?/) {

		    my ($hexcode, $lower, $title, $upper, $condition) =
			($1, $2, $3, $4, $5);
                    if (! IS_ASCII_PLATFORM) { # Remap entry to native
                        foreach my $var_ref (\$hexcode,
                                             \$lower,
                                             \$title,
                                             \$upper)
                        {
                            next unless defined $$var_ref;
                            $$var_ref = join " ",
                                        map { sprintf("%04X",
                                              utf8::unicode_to_native(hex $_)) }
                                        split " ", $$var_ref;
                        }
                    }

		    my $code = hex($hexcode);

                    # In 2.1.8, there were duplicate entries; ignore all but
                    # the first one -- there were no conditions in the file
                    # anyway.
		    if (exists $CASESPEC{$code} && $v_unicode_version ne v2.1.8)
                    {
			if (exists $CASESPEC{$code}->{code}) {
			    my ($oldlower,
				$oldtitle,
				$oldupper,
				$oldcondition) =
				    @{$CASESPEC{$code}}{qw(lower
							   title
							   upper
							   condition)};
			    if (defined $oldcondition) {
				my ($oldlocale) =
				($oldcondition =~ /^([a-z][a-z](?:_\S+)?)/);
				delete $CASESPEC{$code};
				$CASESPEC{$code}->{$oldlocale} =
				{ code      => $hexcode,
				  lower     => $oldlower,
				  title     => $oldtitle,
				  upper     => $oldupper,
				  condition => $oldcondition };
			    }
			}
			my ($locale) =
			    ($condition =~ /^([a-z][a-z](?:_\S+)?)/);
			$CASESPEC{$code}->{$locale} =
			{ code      => $hexcode,
			  lower     => $lower,
			  title     => $title,
			  upper     => $upper,
			  condition => $condition };
		    } else {
			$CASESPEC{$code} =
			{ code      => $hexcode,
			  lower     => $lower,
			  title     => $title,
			  upper     => $upper,
			  condition => $condition };
		    }
		}
	    }
	}
    }
}

sub casespec {
    my $arg  = shift;
    my $code = _getcode($arg);
    croak __PACKAGE__, "::casespec: unknown code '$arg'"
	unless defined $code;

    _casespec() unless %CASESPEC;

    return ref $CASESPEC{$code} ? _dclone $CASESPEC{$code} : $CASESPEC{$code};
}

=head2 B<namedseq()>

    use Unicode::UCD 'namedseq';

    my $namedseq = namedseq("KATAKANA LETTER AINU P");
    my @namedseq = namedseq("KATAKANA LETTER AINU P");
    my %namedseq = namedseq();

If used with a single argument in a scalar context, returns the string
consisting of the code points of the named sequence, or C<undef> if no
named sequence by that name exists.  If used with a single argument in
a list context, it returns the list of the ordinals of the code points.

If used with no
arguments in a list context, it returns a hash with the names of all the
named sequences as the keys and their sequences as strings as
the values.  Otherwise, it returns C<undef> or an empty list depending
on the context.

This function only operates on officially approved (not provisional) named
sequences.

Note that as of Perl 5.14, C<\N{KATAKANA LETTER AINU P}> will insert the named
sequence into double-quoted strings, and C<charnames::string_vianame("KATAKANA
LETTER AINU P")> will return the same string this function does, but will also
operate on character names that aren't named sequences, without you having to
know which are which.  See L<charnames>.

=cut

my %NAMEDSEQ;

sub _namedseq {
    unless (%NAMEDSEQ) {
        my @list = split "\n", do "unicore/Name.pl";
        for (my $i = 0; $i < @list; $i += 3) {
            # Each entry is currently three lines.  The first contains the code
            # points in the sequence separated by spaces.  If this entry
            # doesn't have spaces, it isn't a named sequence.
            next unless $list[$i] =~ /^ [0-9A-F]{4,5} (?: \  [0-9A-F]{4,5} )+ $ /x;

            my $sequence = $list[$i];
            chomp $sequence;

            # And the second is the name
            my $name = $list[$i+1];
            chomp $name;
            my @s = map { chr(hex($_)) } split(' ', $sequence);
            $NAMEDSEQ{$name} = join("", @s);

            # And the third is empty
        }
    }
}

sub namedseq {

    # Use charnames::string_vianame() which now returns this information,
    # unless the caller wants the hash returned, in which case we read it in,
    # and thereafter use it instead of calling charnames, as it is faster.

    my $wantarray = wantarray();
    if (defined $wantarray) {
	if ($wantarray) {
	    if (@_ == 0) {
                _namedseq() unless %NAMEDSEQ;
		return %NAMEDSEQ;
	    } elsif (@_ == 1) {
		my $s;
                if (%NAMEDSEQ) {
                    $s = $NAMEDSEQ{ $_[0] };
                }
                else {
                    $s = charnames::string_vianame($_[0]);
                }
		return defined $s ? map { ord($_) } split('', $s) : ();
	    }
	} elsif (@_ == 1) {
            return $NAMEDSEQ{ $_[0] } if %NAMEDSEQ;
            return charnames::string_vianame($_[0]);
	}
    }
    return;
}

my %NUMERIC;

sub _numeric {
    my @numbers = _read_table("To/Nv.pl");
    foreach my $entry (@numbers) {
        my ($start, $end, $value) = @$entry;

        # If value contains a slash, convert to decimal, add a reverse hash
        # used by charinfo.
        if ((my @rational = split /\//, $value) == 2) {
            my $real = $rational[0] / $rational[1];
            $real_to_rational{$real} = $value;
            $value = $real;

            # Should only be single element, but just in case...
            for my $i ($start .. $end) {
                $NUMERIC{$i} = $value;
            }
        }
        else {
            # The values require adjusting, as is in 'a' format
            for my $i ($start .. $end) {
                $NUMERIC{$i} = $value + $i - $start;
            }
        }
    }

    # Decided unsafe to use these that aren't officially part of the Unicode
    # standard.
    #use Math::Trig;
    #my $pi = acos(-1.0);
    #$NUMERIC{0x03C0} = $pi;

    # Euler's constant, not to be confused with Euler's number
    #$NUMERIC{0x2107} = 0.57721566490153286060651209008240243104215933593992;

    # Euler's number
    #$NUMERIC{0x212F} = 2.7182818284590452353602874713526624977572;

    return;
}

=pod

=head2 B<num()>

    use Unicode::UCD 'num';

    my $val = num("123");
    my $one_quarter = num("\N{VULGAR FRACTION ONE QUARTER}");
    my $val = num("12a", \$valid_length);  # $valid_length contains 2

C<num()> returns the numeric value of the input Unicode string; or C<undef> if it
doesn't think the entire string has a completely valid, safe numeric value.
If called with an optional second parameter, a reference to a scalar, C<num()>
will set the scalar to the length of any valid initial substring; or to 0 if none.

If the string is just one character in length, the Unicode numeric value
is returned if it has one, or C<undef> otherwise.  If the optional scalar ref
is passed, it would be set to 1 if the return is valid; or 0 if the return is
C<undef>.  Note that the numeric value returned need not be a whole number.
C<num("\N{TIBETAN DIGIT HALF ZERO}")>, for example returns -0.5.

=cut

#A few characters to which Unicode doesn't officially
#assign a numeric value are considered numeric by C<num>.
#These are:

# EULER CONSTANT             0.5772...  (this is NOT Euler's number)
# SCRIPT SMALL E             2.71828... (this IS Euler's number)
# GREEK SMALL LETTER PI      3.14159...

=pod

If the string is more than one character, C<undef> is returned unless
all its characters are decimal digits (that is, they would match C<\d+>),
from the same script.  For example if you have an ASCII '0' and a Bengali
'3', mixed together, they aren't considered a valid number, and C<undef>
is returned.  A further restriction is that the digits all have to be of
the same form.  A half-width digit mixed with a full-width one will
return C<undef>.  The Arabic script has two sets of digits;  C<num> will
return C<undef> unless all the digits in the string come from the same
set.  In all cases, the optional scalar ref parameter is set to how
long any valid initial substring of digits is; hence it will be set to the
entire string length if the main return value is not C<undef>.

C<num> errs on the side of safety, and there may be valid strings of
decimal digits that it doesn't recognize.  Note that Unicode defines
a number of "digit" characters that aren't "decimal digit" characters.
"Decimal digits" have the property that they have a positional value, i.e.,
there is a units position, a 10's position, a 100's, etc, AND they are
arranged in Unicode in blocks of 10 contiguous code points.  The Chinese
digits, for example, are not in such a contiguous block, and so Unicode
doesn't view them as decimal digits, but merely digits, and so C<\d> will not
match them.  A single-character string containing one of these digits will
have its decimal value returned by C<num>, but any longer string containing
only these digits will return C<undef>.

Strings of multiple sub- and superscripts are not recognized as numbers.  You
can use either of the compatibility decompositions in Unicode::Normalize to
change these into digits, and then call C<num> on the result.

=cut

# To handle sub, superscripts, this could if called in list context,
# consider those, and return the <decomposition> type in the second
# array element.

sub num ($;$) {
    my ($string, $retlen_ref) = @_;

    use feature 'unicode_strings';

    _numeric unless %NUMERIC;
    $$retlen_ref = 0 if $retlen_ref;    # Assume will fail

    my $length = length $string;
    return if $length == 0;

    my $first_ord = ord(substr($string, 0, 1));
    return if ! exists  $NUMERIC{$first_ord}
           || ! defined $NUMERIC{$first_ord};

    # Here, we know the first character is numeric
    my $value = $NUMERIC{$first_ord};
    $$retlen_ref = 1 if $retlen_ref;    # Assume only this one is numeric

    return $value if $length == 1;

    # Here, the input is longer than a single character.  To be valid, it must
    # be entirely decimal digits, which means it must start with one.
    return if $string =~ / ^ \D /x;

    # To be a valid decimal number, it should be in a block of 10 consecutive
    # characters, whose values are 0, 1, 2, ... 9.  Therefore this digit's
    # value is its offset in that block from the character that means zero.
    my $zero_ord = $first_ord - $value;

    # Unicode 6.0 instituted the rule that only digits in a consecutive
    # block of 10 would be considered decimal digits.  If this is an earlier
    # release, we verify that this first character is a member of such a
    # block.  That is, that the block of characters surrounding this one
    # consists of all \d characters whose numeric values are the expected
    # ones.  If not, then this single character is numeric, but the string as
    # a whole is not considered to be.
    UnicodeVersion() unless defined $v_unicode_version;
    if ($v_unicode_version lt v6.0.0) {
        for my $i (0 .. 9) {
            my $ord = $zero_ord + $i;
            return unless chr($ord) =~ /\d/;
            my $numeric = $NUMERIC{$ord};
            return unless defined $numeric;
            return unless $numeric == $i;
        }
    }

    for my $i (1 .. $length -1) {

        # Here we know either by verifying, or by fact of the first character
        # being a \d in Unicode 6.0 or later, that any character between the
        # character that means 0, and 9 positions above it must be \d, and
        # must have its value correspond to its offset from the zero.  Any
        # characters outside these 10 do not form a legal number for this
        # function.
        my $ord = ord(substr($string, $i, 1));
        my $digit = $ord - $zero_ord;
        if ($digit < 0 || $digit > 9) {
            $$retlen_ref = $i if $retlen_ref;
            return;
        }
        $value = $value * 10 + $digit;
    }

    $$retlen_ref = $length if $retlen_ref;
    return $value;
}

=pod

=head2 B<prop_aliases()>

    use Unicode::UCD 'prop_aliases';

    my ($short_name, $full_name, @other_names) = prop_aliases("space");
    my $same_full_name = prop_aliases("Space");     # Scalar context
    my ($same_short_name) = prop_aliases("Space");  # gets 0th element
    print "The full name is $full_name\n";
    print "The short name is $short_name\n";
    print "The other aliases are: ", join(", ", @other_names), "\n";

    prints:
    The full name is White_Space
    The short name is WSpace
    The other aliases are: Space

Most Unicode properties have several synonymous names.  Typically, there is at
least a short name, convenient to type, and a long name that more fully
describes the property, and hence is more easily understood.

If you know one name for a Unicode property, you can use C<prop_aliases> to find
either the long name (when called in scalar context), or a list of all of the
names, somewhat ordered so that the short name is in the 0th element, the long
name in the next element, and any other synonyms are in the remaining
elements, in no particular order.

The long name is returned in a form nicely capitalized, suitable for printing.

The input parameter name is loosely matched, which means that white space,
hyphens, and underscores are ignored (except for the trailing underscore in
the old_form grandfathered-in C<"L_">, which is better written as C<"LC">, and
both of which mean C<General_Category=Cased Letter>).

If the name is unknown, C<undef> is returned (or an empty list in list
context).  Note that Perl typically recognizes property names in regular
expressions with an optional C<"Is_>" (with or without the underscore)
prefixed to them, such as C<\p{isgc=punct}>.  This function does not recognize
those in the input, returning C<undef>.  Nor are they included in the output
as possible synonyms.

C<prop_aliases> does know about the Perl extensions to Unicode properties,
such as C<Any> and C<XPosixAlpha>, and the single form equivalents to Unicode
properties such as C<XDigit>, C<Greek>, C<In_Greek>, and C<Is_Greek>.  The
final example demonstrates that the C<"Is_"> prefix is recognized for these
extensions; it is needed to resolve ambiguities.  For example,
C<prop_aliases('lc')> returns the list C<(lc, Lowercase_Mapping)>, but
C<prop_aliases('islc')> returns C<(Is_LC, Cased_Letter)>.  This is
because C<islc> is a Perl extension which is short for
C<General_Category=Cased Letter>.  The lists returned for the Perl extensions
will not include the C<"Is_"> prefix (whether or not the input had it) unless
needed to resolve ambiguities, as shown in the C<"islc"> example, where the
returned list had one element containing C<"Is_">, and the other without.

It is also possible for the reverse to happen:  C<prop_aliases('isc')> returns
the list C<(isc, ISO_Comment)>; whereas C<prop_aliases('c')> returns
C<(C, Other)> (the latter being a Perl extension meaning
C<General_Category=Other>.
L<perluniprops/Properties accessible through Unicode::UCD> lists the available
forms, including which ones are discouraged from use.

Those discouraged forms are accepted as input to C<prop_aliases>, but are not
returned in the lists.  C<prop_aliases('isL&')> and C<prop_aliases('isL_')>,
which are old synonyms for C<"Is_LC"> and should not be used in new code, are
examples of this.  These both return C<(Is_LC, Cased_Letter)>.  Thus this
function allows you to take a discouraged form, and find its acceptable
alternatives.  The same goes with single-form Block property equivalences.
Only the forms that begin with C<"In_"> are not discouraged; if you pass
C<prop_aliases> a discouraged form, you will get back the equivalent ones that
begin with C<"In_">.  It will otherwise look like a new-style block name (see.
L</Old-style versus new-style block names>).

C<prop_aliases> does not know about any user-defined properties, and will
return C<undef> if called with one of those.  Likewise for Perl internal
properties, with the exception of "Perl_Decimal_Digit" which it does know
about (and which is documented below in L</prop_invmap()>).

=cut

# It may be that there are use cases where the discouraged forms should be
# returned.  If that comes up, an optional boolean second parameter to the
# function could be created, for example.

# These are created by mktables for this routine and stored in unicore/UCD.pl
# where their structures are described.
our %string_property_loose_to_name;
our %ambiguous_names;
our %loose_perlprop_to_name;

sub prop_aliases ($) {
    my $prop = $_[0];
    return unless defined $prop;

    require "unicore/UCD.pl";

    # The property name may be loosely or strictly matched; we don't know yet.
    # But both types use lower-case.
    $prop = lc $prop;

    # It is loosely matched if its lower case isn't known to be strict.
    my $list_ref;
    if (! exists $stricter_to_file_of{$prop}) {
        my $loose = loose_name($prop);

        # There is a hash that converts from any loose name to its standard
        # form, mapping all synonyms for a  name to one name that can be used
        # as a key into another hash.  The whole concept is for memory
        # savings, as the second hash doesn't have to have all the
        # combinations.  Actually, there are two hashes that do the
        # conversion.  One is stored in UCD.pl) for looking up properties
        # matchable in regexes.  This function needs to access string
        # properties, which aren't available in regexes, so a second
        # conversion hash is made for them (stored in UCD.pl).  Look in the
        # string one now, as the rest can have an optional 'is' prefix, which
        # these don't.
        if (exists $string_property_loose_to_name{$loose}) {

            # Convert to its standard loose name.
            $prop = $string_property_loose_to_name{$loose};
        }
        else {
            my $retrying = 0;   # bool.  ? Has an initial 'is' been stripped
        RETRY:
            if (exists $loose_property_name_of{$loose}
                && (! $retrying
                    || ! exists $ambiguous_names{$loose}))
            {
                # Found an entry giving the standard form.  We don't get here
                # (in the test above) when we've stripped off an
                # 'is' and the result is an ambiguous name.  That is because
                # these are official Unicode properties (though Perl can have
                # an optional 'is' prefix meaning the official property), and
                # all ambiguous cases involve a Perl single-form extension
                # for the gc, script, or block properties, and the stripped
                # 'is' means that they mean one of those, and not one of
                # these
                $prop = $loose_property_name_of{$loose};
            }
            elsif (exists $loose_perlprop_to_name{$loose}) {

                # This hash is specifically for this function to list Perl
                # extensions that aren't in the earlier hashes.  If there is
                # only one element, the short and long names are identical.
                # Otherwise the form is already in the same form as
                # %prop_aliases, which is handled at the end of the function.
                $list_ref = $loose_perlprop_to_name{$loose};
                if (@$list_ref == 1) {
                    my @list = ($list_ref->[0], $list_ref->[0]);
                    $list_ref = \@list;
                }
            }
            elsif (! exists $loose_to_file_of{$loose}) {

                # loose_to_file_of is a complete list of loose names.  If not
                # there, the input is unknown.
                return;
            }
            elsif ($loose =~ / [:=] /x) {

                # Here we found the name but not its aliases, so it has to
                # exist.  Exclude property-value combinations.  (This shows up
                # for something like ccc=vr which matches loosely, but is a
                # synonym for ccc=9 which matches only strictly.
                return;
            }
            else {

                # Here it has to exist, and isn't a property-value
                # combination.  This means it must be one of the Perl
                # single-form extensions.  First see if it is for a
                # property-value combination in one of the following
                # properties.
                my @list;
                foreach my $property ("gc", "script") {
                    @list = prop_value_aliases($property, $loose);
                    last if @list;
                }
                if (@list) {

                    # Here, it is one of those property-value combination
                    # single-form synonyms.  There are ambiguities with some
                    # of these.  Check against the list for these, and adjust
                    # if necessary.
                    for my $i (0 .. @list -1) {
                        if (exists $ambiguous_names
                                   {loose_name(lc $list[$i])})
                        {
                            # The ambiguity is resolved by toggling whether or
                            # not it has an 'is' prefix
                            $list[$i] =~ s/^Is_// or $list[$i] =~ s/^/Is_/;
                        }
                    }
                    return @list;
                }

                # Here, it wasn't one of the gc or script single-form
                # extensions.  It could be a block property single-form
                # extension.  An 'in' prefix definitely means that, and should
                # be looked up without the prefix.  However, starting in
                # Unicode 6.1, we have to special case 'indic...', as there
                # is a property that begins with that name.   We shouldn't
                # strip the 'in' from that.   I'm (khw) generalizing this to
                # 'indic' instead of the single property, because I suspect
                # that others of this class may come along in the future.
                # However, this could backfire and a block created whose name
                # begins with 'dic...', and we would want to strip the 'in'.
                # At which point this would have to be tweaked.
                my $began_with_in = $loose =~ s/^in(?!dic)//;
                @list = prop_value_aliases("block", $loose);
                if (@list) {
                    map { $_ =~ s/^/In_/ } @list;
                    return @list;
                }

                # Here still haven't found it.  The last opportunity for it
                # being valid is only if it began with 'is'.  We retry without
                # the 'is', setting a flag to that effect so that we don't
                # accept things that begin with 'isis...'
                if (! $retrying && ! $began_with_in && $loose =~ s/^is//) {
                    $retrying = 1;
                    goto RETRY;
                }

                # Here, didn't find it.  Since it was in %loose_to_file_of, we
                # should have been able to find it.
                carp __PACKAGE__, "::prop_aliases: Unexpectedly could not find '$prop'.  Send bug report to perlbug\@perl.org";
                return;
            }
        }
    }

    if (! $list_ref) {
        # Here, we have set $prop to a standard form name of the input.  Look
        # it up in the structure created by mktables for this purpose, which
        # contains both strict and loosely matched properties.  Avoid
        # autovivifying.
        $list_ref = $prop_aliases{$prop} if exists $prop_aliases{$prop};
        return unless $list_ref;
    }

    # The full name is in element 1.
    return $list_ref->[1] unless wantarray;

    return @{_dclone $list_ref};
}

=pod

=head2 B<prop_values()>

    use Unicode::UCD 'prop_values';

    print "AHex values are: ", join(", ", prop_values("AHex")),
                               "\n";
  prints:
    AHex values are: N, Y

Some Unicode properties have a restricted set of legal values.  For example,
all binary properties are restricted to just C<true> or C<false>; and there
are only a few dozen possible General Categories.  Use C<prop_values>
to find out if a given property is one such, and if so, to get a list of the
values:

    print join ", ", prop_values("NFC_Quick_Check");
  prints:
    M, N, Y

If the property doesn't have such a restricted set, C<undef> is returned.

There are usually several synonyms for each possible value.  Use
L</prop_value_aliases()> to access those.

Case, white space, hyphens, and underscores are ignored in the input property
name (except for the trailing underscore in the old-form grandfathered-in
general category property value C<"L_">, which is better written as C<"LC">).

If the property name is unknown, C<undef> is returned.  Note that Perl typically
recognizes property names in regular expressions with an optional C<"Is_>"
(with or without the underscore) prefixed to them, such as C<\p{isgc=punct}>.
This function does not recognize those in the property parameter, returning
C<undef>.

For the block property, new-style block names are returned (see
L</Old-style versus new-style block names>).

C<prop_values> does not know about any user-defined properties, and
will return C<undef> if called with one of those.

=cut

# These are created by mktables for this module and stored in unicore/UCD.pl
# where their structures are described.
our %loose_to_standard_value;
our %prop_value_aliases;

sub prop_values ($) {
    my $prop = shift;
    return undef unless defined $prop;

    require "unicore/UCD.pl";

    # Find the property name synonym that's used as the key in other hashes,
    # which is element 0 in the returned list.
    ($prop) = prop_aliases($prop);
    return undef if ! $prop;
    $prop = loose_name(lc $prop);

    # Here is a legal property.
    return undef unless exists $prop_value_aliases{$prop};
    my @return;
    foreach my $value_key (sort { lc $a cmp lc $b }
                            keys %{$prop_value_aliases{$prop}})
    {
        push @return, $prop_value_aliases{$prop}{$value_key}[0];
    }
    return @return;
}

=pod

=head2 B<prop_value_aliases()>

    use Unicode::UCD 'prop_value_aliases';

    my ($short_name, $full_name, @other_names)
                                   = prop_value_aliases("Gc", "Punct");
    my $same_full_name = prop_value_aliases("Gc", "P");   # Scalar cntxt
    my ($same_short_name) = prop_value_aliases("Gc", "P"); # gets 0th
                                                           # element
    print "The full name is $full_name\n";
    print "The short name is $short_name\n";
    print "The other aliases are: ", join(", ", @other_names), "\n";

  prints:
    The full name is Punctuation
    The short name is P
    The other aliases are: Punct

Some Unicode properties have a restricted set of legal values.  For example,
all binary properties are restricted to just C<true> or C<false>; and there
are only a few dozen possible General Categories.

You can use L</prop_values()> to find out if a given property is one which has
a restricted set of values, and if so, what those values are.  But usually
each value actually has several synonyms.  For example, in Unicode binary
properties, I<truth> can be represented by any of the strings "Y", "Yes", "T",
or "True"; and the General Category "Punctuation" by that string, or "Punct",
or simply "P".

Like property names, there is typically at least a short name for each such
property-value, and a long name.  If you know any name of the property-value
(which you can get by L</prop_values()>, you can use C<prop_value_aliases>()
to get the long name (when called in scalar context), or a list of all the
names, with the short name in the 0th element, the long name in the next
element, and any other synonyms in the remaining elements, in no particular
order, except that any all-numeric synonyms will be last.

The long name is returned in a form nicely capitalized, suitable for printing.

Case, white space, hyphens, and underscores are ignored in the input parameters
(except for the trailing underscore in the old-form grandfathered-in general
category property value C<"L_">, which is better written as C<"LC">).

If either name is unknown, C<undef> is returned.  Note that Perl typically
recognizes property names in regular expressions with an optional C<"Is_>"
(with or without the underscore) prefixed to them, such as C<\p{isgc=punct}>.
This function does not recognize those in the property parameter, returning
C<undef>.

If called with a property that doesn't have synonyms for its values, it
returns the input value, possibly normalized with capitalization and
underscores, but not necessarily checking that the input value is valid.

For the block property, new-style block names are returned (see
L</Old-style versus new-style block names>).

To find the synonyms for single-forms, such as C<\p{Any}>, use
L</prop_aliases()> instead.

C<prop_value_aliases> does not know about any user-defined properties, and
will return C<undef> if called with one of those.

=cut

sub prop_value_aliases ($$) {
    my ($prop, $value) = @_;
    return unless defined $prop && defined $value;

    require "unicore/UCD.pl";

    # Find the property name synonym that's used as the key in other hashes,
    # which is element 0 in the returned list.
    ($prop) = prop_aliases($prop);
    return if ! $prop;
    $prop = loose_name(lc $prop);

    # Here is a legal property, but the hash below (created by mktables for
    # this purpose) only knows about the properties that have a very finite
    # number of potential values, that is not ones whose value could be
    # anything, like most (if not all) string properties.  These don't have
    # synonyms anyway.  Simply return the input.  For example, there is no
    # synonym for ('Uppercase_Mapping', A').
    if (! exists $prop_value_aliases{$prop}) {

        # Here, we have a legal property, but an unknown value.  Since the
        # property is legal, if it isn't in the prop_aliases hash, it must be
        # a Perl-extension All perl extensions are binary, hence are
        # enumerateds, which means that we know that the input unknown value
        # is illegal.
        return if ! exists $prop_aliases{$prop};

        # Otherwise, we assume it's valid, as documented.
        return $value;
    }

    # The value name may be loosely or strictly matched; we don't know yet.
    # But both types use lower-case.
    $value = lc $value;

    # If the name isn't found under loose matching, it certainly won't be
    # found under strict
    my $loose_value = loose_name($value);
    return unless exists $loose_to_standard_value{"$prop=$loose_value"};

    # Similarly if the combination under loose matching doesn't exist, it
    # won't exist under strict.
    my $standard_value = $loose_to_standard_value{"$prop=$loose_value"};
    return unless exists $prop_value_aliases{$prop}{$standard_value};

    # Here we did find a combination under loose matching rules.  But it could
    # be that is a strict property match that shouldn't have matched.
    # %prop_value_aliases is set up so that the strict matches will appear as
    # if they were in loose form.  Thus, if the non-loose version is legal,
    # we're ok, can skip the further check.
    if (! exists $stricter_to_file_of{"$prop=$value"}

        # We're also ok and skip the further check if value loosely matches.
        # mktables has verified that no strict name under loose rules maps to
        # an existing loose name.  This code relies on the very limited
        # circumstances that strict names can be here.  Strict name matching
        # happens under two conditions:
        # 1) when the name begins with an underscore.  But this function
        #    doesn't accept those, and %prop_value_aliases doesn't have
        #    them.
        # 2) When the values are numeric, in which case we need to look
        #    further, but their squeezed-out loose values will be in
        #    %stricter_to_file_of
        && exists $stricter_to_file_of{"$prop=$loose_value"})
    {
        # The only thing that's legal loosely under strict is that can have an
        # underscore between digit pairs XXX
        while ($value =~ s/(\d)_(\d)/$1$2/g) {}
        return unless exists $stricter_to_file_of{"$prop=$value"};
    }

    # Here, we know that the combination exists.  Return it.
    my $list_ref = $prop_value_aliases{$prop}{$standard_value};
    if (@$list_ref > 1) {
        # The full name is in element 1.
        return $list_ref->[1] unless wantarray;

        return @{_dclone $list_ref};
    }

    return $list_ref->[0] unless wantarray;

    # Only 1 element means that it repeats
    return ( $list_ref->[0], $list_ref->[0] );
}

# All 1 bits but the top one is the largest possible IV.
$MAX_CP = (~0) >> 1;

=pod

=head2 B<prop_invlist()>

C<prop_invlist> returns an inversion list (described below) that defines all the
code points for the binary Unicode property (or "property=value" pair) given
by the input parameter string:

 use feature 'say';
 use Unicode::UCD 'prop_invlist';
 say join ", ", prop_invlist("Any");

 prints:
 0, 1114112

If the input is unknown C<undef> is returned in scalar context; an empty-list
in list context.  If the input is known, the number of elements in
the list is returned if called in scalar context.

L<perluniprops|perluniprops/Properties accessible through \p{} and \P{}> gives
the list of properties that this function accepts, as well as all the possible
forms for them (including with the optional "Is_" prefixes).  (Except this
function doesn't accept any Perl-internal properties, some of which are listed
there.) This function uses the same loose or tighter matching rules for
resolving the input property's name as is done for regular expressions.  These
are also specified in L<perluniprops|perluniprops/Properties accessible
through \p{} and \P{}>.  Examples of using the "property=value" form are:

 say join ", ", prop_invlist("Script_Extensions=Shavian");

 prints:
 66640, 66688

 say join ", ", prop_invlist("ASCII_Hex_Digit=No");

 prints:
 0, 48, 58, 65, 71, 97, 103

 say join ", ", prop_invlist("ASCII_Hex_Digit=Yes");

 prints:
 48, 58, 65, 71, 97, 103

Inversion lists are a compact way of specifying Unicode property-value
definitions.  The 0th item in the list is the lowest code point that has the
property-value.  The next item (item [1]) is the lowest code point beyond that
one that does NOT have the property-value.  And the next item beyond that
([2]) is the lowest code point beyond that one that does have the
property-value, and so on.  Put another way, each element in the list gives
the beginning of a range that has the property-value (for even numbered
elements), or doesn't have the property-value (for odd numbered elements).
The name for this data structure stems from the fact that each element in the
list toggles (or inverts) whether the corresponding range is or isn't on the
list.

In the final example above, the first ASCII Hex digit is code point 48, the
character "0", and all code points from it through 57 (a "9") are ASCII hex
digits.  Code points 58 through 64 aren't, but 65 (an "A") through 70 (an "F")
are, as are 97 ("a") through 102 ("f").  103 starts a range of code points
that aren't ASCII hex digits.  That range extends to infinity, which on your
computer can be found in the variable C<$Unicode::UCD::MAX_CP>.  (This
variable is as close to infinity as Perl can get on your platform, and may be
too high for some operations to work; you may wish to use a smaller number for
your purposes.)

Note that the inversion lists returned by this function can possibly include
non-Unicode code points, that is anything above 0x10FFFF.  Unicode properties
are not defined on such code points.  You might wish to change the output to
not include these.  Simply add 0x110000 at the end of the non-empty returned
list if it isn't already that value; and pop that value if it is; like:

 my @list = prop_invlist("foo");
 if (@list) {
     if ($list[-1] == 0x110000) {
         pop @list;  # Defeat the turning on for above Unicode
     }
     else {
         push @list, 0x110000; # Turn off for above Unicode
     }
 }

It is a simple matter to expand out an inversion list to a full list of all
code points that have the property-value:

 my @invlist = prop_invlist($property_name);
 die "empty" unless @invlist;
 my @full_list;
 for (my $i = 0; $i < @invlist; $i += 2) {
    my $upper = ($i + 1) < @invlist
                ? $invlist[$i+1] - 1      # In range
                : $Unicode::UCD::MAX_CP;  # To infinity.
    for my $j ($invlist[$i] .. $upper) {
        push @full_list, $j;
    }
 }

C<prop_invlist> does not know about any user-defined nor Perl internal-only
properties, and will return C<undef> if called with one of those.

The L</search_invlist()> function is provided for finding a code point within
an inversion list.

=cut

# User-defined properties could be handled with some changes to SWASHNEW;
# and implementing here of dealing with EXTRAS.  If done, consideration should
# be given to the fact that the user subroutine could return different results
# with each call; security issues need to be thought about.

# These are created by mktables for this routine and stored in unicore/UCD.pl
# where their structures are described.
our %loose_defaults;
our $MAX_UNICODE_CODEPOINT;

sub prop_invlist ($;$) {
    my $prop = $_[0];

    # Undocumented way to get at Perl internal properties; it may be changed
    # or removed without notice at any time.
    my $internal_ok = defined $_[1] && $_[1] eq '_perl_core_internal_ok';

    return if ! defined $prop;

    # Warnings for these are only for regexes, so not applicable to us
    no warnings 'deprecated';

    # Get the swash definition of the property-value.
    my $swash = SWASHNEW(__PACKAGE__, $prop, undef, 1, 0);

    # Fail if not found, or isn't a boolean property-value, or is a
    # user-defined property, or is internal-only.
    return if ! $swash
              || ref $swash eq ""
              || $swash->{'BITS'} != 1
              || $swash->{'USER_DEFINED'}
              || (! $internal_ok && $prop =~ /^\s*_/);

    if ($swash->{'EXTRAS'}) {
        carp __PACKAGE__, "::prop_invlist: swash returned for $prop unexpectedly has EXTRAS magic";
        return;
    }
    if ($swash->{'SPECIALS'}) {
        carp __PACKAGE__, "::prop_invlist: swash returned for $prop unexpectedly has SPECIALS magic";
        return;
    }

    my @invlist;

    if ($swash->{'LIST'} =~ /^V/) {

        # A 'V' as the first character marks the input as already an inversion
        # list, in which case, all we need to do is put the remaining lines
        # into our array.
        @invlist = split "\n", $swash->{'LIST'} =~ s/ \s* (?: \# .* )? $ //xmgr;
        shift @invlist;
    }
    else {
        # The input lines look like:
        # 0041\t005A   # [26]
        # 005F

        # Split into lines, stripped of trailing comments
        foreach my $range (split "\n",
                              $swash->{'LIST'} =~ s/ \s* (?: \# .* )? $ //xmgr)
        {
            # And find the beginning and end of the range on the line
            my ($hex_begin, $hex_end) = split "\t", $range;
            my $begin = hex $hex_begin;

            # If the new range merely extends the old, we remove the marker
            # created the last time through the loop for the old's end, which
            # causes the new one's end to be used instead.
            if (@invlist && $begin == $invlist[-1]) {
                pop @invlist;
            }
            else {
                # Add the beginning of the range
                push @invlist, $begin;
            }

            if (defined $hex_end) { # The next item starts with the code point 1
                                    # beyond the end of the range.
                no warnings 'portable';
                my $end = hex $hex_end;
                last if $end == $MAX_CP;
                push @invlist, $end + 1;
            }
            else {  # No end of range, is a single code point.
                push @invlist, $begin + 1;
            }
        }
    }

    # Could need to be inverted: add or subtract a 0 at the beginning of the
    # list.
    if ($swash->{'INVERT_IT'}) {
        if (@invlist && $invlist[0] == 0) {
            shift @invlist;
        }
        else {
            unshift @invlist, 0;
        }
    }

    return @invlist;
}

=pod

=head2 B<prop_invmap()>

 use Unicode::UCD 'prop_invmap';
 my ($list_ref, $map_ref, $format, $default)
                                      = prop_invmap("General Category");

C<prop_invmap> is used to get the complete mapping definition for a property,
in the form of an inversion map.  An inversion map consists of two parallel
arrays.  One is an ordered list of code points that mark range beginnings, and
the other gives the value (or mapping) that all code points in the
corresponding range have.

C<prop_invmap> is called with the name of the desired property.  The name is
loosely matched, meaning that differences in case, white-space, hyphens, and
underscores are not meaningful (except for the trailing underscore in the
old-form grandfathered-in property C<"L_">, which is better written as C<"LC">,
or even better, C<"Gc=LC">).

Many Unicode properties have more than one name (or alias).  C<prop_invmap>
understands all of these, including Perl extensions to them.  Ambiguities are
resolved as described above for L</prop_aliases()> (except if a property has
both a complete mapping, and a binary C<Y>/C<N> mapping, then specifying the
property name prefixed by C<"is"> causes the binary one to be returned).  The
Perl internal property "Perl_Decimal_Digit, described below, is also accepted.
An empty list is returned if the property name is unknown.
See L<perluniprops/Properties accessible through Unicode::UCD> for the
properties acceptable as inputs to this function.

It is a fatal error to call this function except in list context.

In addition to the two arrays that form the inversion map, C<prop_invmap>
returns two other values; one is a scalar that gives some details as to the
format of the entries of the map array; the other is a default value, useful
in maps whose format name begins with the letter C<"a">, as described
L<below in its subsection|/a>; and for specialized purposes, such as
converting to another data structure, described at the end of this main
section.

This means that C<prop_invmap> returns a 4 element list.  For example,

 my ($blocks_ranges_ref, $blocks_maps_ref, $format, $default)
                                                 = prop_invmap("Block");

In this call, the two arrays will be populated as shown below (for Unicode
6.0):

 Index  @blocks_ranges  @blocks_maps
   0        0x0000      Basic Latin
   1        0x0080      Latin-1 Supplement
   2        0x0100      Latin Extended-A
   3        0x0180      Latin Extended-B
   4        0x0250      IPA Extensions
   5        0x02B0      Spacing Modifier Letters
   6        0x0300      Combining Diacritical Marks
   7        0x0370      Greek and Coptic
   8        0x0400      Cyrillic
  ...
 233        0x2B820     No_Block
 234        0x2F800     CJK Compatibility Ideographs Supplement
 235        0x2FA20     No_Block
 236        0xE0000     Tags
 237        0xE0080     No_Block
 238        0xE0100     Variation Selectors Supplement
 239        0xE01F0     No_Block
 240        0xF0000     Supplementary Private Use Area-A
 241        0x100000    Supplementary Private Use Area-B
 242        0x110000    No_Block

The first line (with Index [0]) means that the value for code point 0 is "Basic
Latin".  The entry "0x0080" in the @blocks_ranges column in the second line
means that the value from the first line, "Basic Latin", extends to all code
points in the range from 0 up to but not including 0x0080, that is, through
127.  In other words, the code points from 0 to 127 are all in the "Basic
Latin" block.  Similarly, all code points in the range from 0x0080 up to (but
not including) 0x0100 are in the block named "Latin-1 Supplement", etc.
(Notice that the return is the old-style block names; see L</Old-style versus
new-style block names>).

The final line (with Index [242]) means that the value for all code points above
the legal Unicode maximum code point have the value "No_Block", which is the
term Unicode uses for a non-existing block.

The arrays completely specify the mappings for all possible code points.
The final element in an inversion map returned by this function will always be
for the range that consists of all the code points that aren't legal Unicode,
but that are expressible on the platform.  (That is, it starts with code point
0x110000, the first code point above the legal Unicode maximum, and extends to
infinity.) The value for that range will be the same that any typical
unassigned code point has for the specified property.  (Certain unassigned
code points are not "typical"; for example the non-character code points, or
those in blocks that are to be written right-to-left.  The above-Unicode
range's value is not based on these atypical code points.)  It could be argued
that, instead of treating these as unassigned Unicode code points, the value
for this range should be C<undef>.  If you wish, you can change the returned
arrays accordingly.

The maps for almost all properties are simple scalars that should be
interpreted as-is.
These values are those given in the Unicode-supplied data files, which may be
inconsistent as to capitalization and as to which synonym for a property-value
is given.  The results may be normalized by using the L</prop_value_aliases()>
function.

There are exceptions to the simple scalar maps.  Some properties have some
elements in their map list that are themselves lists of scalars; and some
special strings are returned that are not to be interpreted as-is.  Element
[2] (placed into C<$format> in the example above) of the returned four element
list tells you if the map has any of these special elements or not, as follows:

=over

=item B<C<s>>

means all the elements of the map array are simple scalars, with no special
elements.  Almost all properties are like this, like the C<block> example
above.

=item B<C<sl>>

means that some of the map array elements have the form given by C<"s">, and
the rest are lists of scalars.  For example, here is a portion of the output
of calling C<prop_invmap>() with the "Script Extensions" property:

 @scripts_ranges  @scripts_maps
      ...
      0x0953      Devanagari
      0x0964      [ Bengali, Devanagari, Gurumukhi, Oriya ]
      0x0966      Devanagari
      0x0970      Common

Here, the code points 0x964 and 0x965 are both used in Bengali,
Devanagari, Gurmukhi, and Oriya, but no other scripts.

The Name_Alias property is also of this form.  But each scalar consists of two
components:  1) the name, and 2) the type of alias this is.  They are
separated by a colon and a space.  In Unicode 6.1, there are several alias types:

=over

=item C<correction>

indicates that the name is a corrected form for the
original name (which remains valid) for the same code point.

=item C<control>

adds a new name for a control character.

=item C<alternate>

is an alternate name for a character

=item C<figment>

is a name for a character that has been documented but was never in any
actual standard.

=item C<abbreviation>

is a common abbreviation for a character

=back

The lists are ordered (roughly) so the most preferred names come before less
preferred ones.

For example,

 @aliases_ranges        @alias_maps
    ...
    0x009E        [ 'PRIVACY MESSAGE: control', 'PM: abbreviation' ]
    0x009F        [ 'APPLICATION PROGRAM COMMAND: control',
                    'APC: abbreviation'
                  ]
    0x00A0        'NBSP: abbreviation'
    0x00A1        ""
    0x00AD        'SHY: abbreviation'
    0x00AE        ""
    0x01A2        'LATIN CAPITAL LETTER GHA: correction'
    0x01A3        'LATIN SMALL LETTER GHA: correction'
    0x01A4        ""
    ...

A map to the empty string means that there is no alias defined for the code
point.

=item B<C<a>>

is like C<"s"> in that all the map array elements are scalars, but here they are
restricted to all being integers, and some have to be adjusted (hence the name
C<"a">) to get the correct result.  For example, in:

 my ($uppers_ranges_ref, $uppers_maps_ref, $format, $default)
                          = prop_invmap("Simple_Uppercase_Mapping");

the returned arrays look like this:

 @$uppers_ranges_ref    @$uppers_maps_ref   Note
       0                      0
      97                     65          'a' maps to 'A', b => B ...
     123                      0
     181                    924          MICRO SIGN => Greek Cap MU
     182                      0
     ...

and C<$default> is 0.

Let's start with the second line.  It says that the uppercase of code point 97
is 65; or C<uc("a")> == "A".  But the line is for the entire range of code
points 97 through 122.  To get the mapping for any code point in this range,
you take the offset it has from the beginning code point of the range, and add
that to the mapping for that first code point.  So, the mapping for 122 ("z")
is derived by taking the offset of 122 from 97 (=25) and adding that to 65,
yielding 90 ("Z").  Likewise for everything in between.

Requiring this simple adjustment allows the returned arrays to be
significantly smaller than otherwise, up to a factor of 10, speeding up
searching through them.

Ranges that map to C<$default>, C<"0">, behave somewhat differently.  For
these, each code point maps to itself.  So, in the first line in the example,
S<C<ord(uc(chr(0)))>> is 0, S<C<ord(uc(chr(1)))>> is 1, ..
S<C<ord(uc(chr(96)))>> is 96.

=item B<C<al>>

means that some of the map array elements have the form given by C<"a">, and
the rest are ordered lists of code points.
For example, in:

 my ($uppers_ranges_ref, $uppers_maps_ref, $format, $default)
                                 = prop_invmap("Uppercase_Mapping");

the returned arrays look like this:

 @$uppers_ranges_ref    @$uppers_maps_ref
       0                      0
      97                     65
     123                      0
     181                    924
     182                      0
     ...
    0x0149              [ 0x02BC 0x004E ]
    0x014A                    0
    0x014B                  330
     ...

This is the full Uppercase_Mapping property (as opposed to the
Simple_Uppercase_Mapping given in the example for format C<"a">).  The only
difference between the two in the ranges shown is that the code point at
0x0149 (LATIN SMALL LETTER N PRECEDED BY APOSTROPHE) maps to a string of two
characters, 0x02BC (MODIFIER LETTER APOSTROPHE) followed by 0x004E (LATIN
CAPITAL LETTER N).

No adjustments are needed to entries that are references to arrays; each such
entry will have exactly one element in its range, so the offset is always 0.

The fourth (index [3]) element (C<$default>) in the list returned for this
format is 0.

=item B<C<ae>>

This is like C<"a">, but some elements are the empty string, and should not be
adjusted.
The one internal Perl property accessible by C<prop_invmap> is of this type:
"Perl_Decimal_Digit" returns an inversion map which gives the numeric values
that are represented by the Unicode decimal digit characters.  Characters that
don't represent decimal digits map to the empty string, like so:

 @digits    @values
 0x0000       ""
 0x0030        0
 0x003A:      ""
 0x0660:       0
 0x066A:      ""
 0x06F0:       0
 0x06FA:      ""
 0x07C0:       0
 0x07CA:      ""
 0x0966:       0
 ...

This means that the code points from 0 to 0x2F do not represent decimal digits;
the code point 0x30 (DIGIT ZERO) represents 0;  code point 0x31, (DIGIT ONE),
represents 0+1-0 = 1; ... code point 0x39, (DIGIT NINE), represents 0+9-0 = 9;
... code points 0x3A through 0x65F do not represent decimal digits; 0x660
(ARABIC-INDIC DIGIT ZERO), represents 0; ... 0x07C1 (NKO DIGIT ONE),
represents 0+1-0 = 1 ...

The fourth (index [3]) element (C<$default>) in the list returned for this
format is the empty string.

=item B<C<ale>>

is a combination of the C<"al"> type and the C<"ae"> type.  Some of
the map array elements have the forms given by C<"al">, and
the rest are the empty string.  The property C<NFKC_Casefold> has this form.
An example slice is:

 @$ranges_ref  @$maps_ref         Note
    ...
   0x00AA       97                FEMININE ORDINAL INDICATOR => 'a'
   0x00AB        0
   0x00AD                         SOFT HYPHEN => ""
   0x00AE        0
   0x00AF     [ 0x0020, 0x0304 ]  MACRON => SPACE . COMBINING MACRON
   0x00B0        0
   ...

The fourth (index [3]) element (C<$default>) in the list returned for this
format is 0.

=item B<C<ar>>

means that all the elements of the map array are either rational numbers or
the string C<"NaN">, meaning "Not a Number".  A rational number is either an
integer, or two integers separated by a solidus (C<"/">).  The second integer
represents the denominator of the division implied by the solidus, and is
actually always positive, so it is guaranteed not to be 0 and to not be
signed.  When the element is a plain integer (without the
solidus), it may need to be adjusted to get the correct value by adding the
offset, just as other C<"a"> properties.  No adjustment is needed for
fractions, as the range is guaranteed to have just a single element, and so
the offset is always 0.

If you want to convert the returned map to entirely scalar numbers, you
can use something like this:

 my ($invlist_ref, $invmap_ref, $format) = prop_invmap($property);
 if ($format && $format eq "ar") {
     map { $_ = eval $_ if $_ ne 'NaN' } @$map_ref;
 }

Here's some entries from the output of the property "Nv", which has format
C<"ar">.

 @numerics_ranges  @numerics_maps       Note
        0x00           "NaN"
        0x30             0           DIGIT 0 .. DIGIT 9
        0x3A           "NaN"
        0xB2             2           SUPERSCRIPTs 2 and 3
        0xB4           "NaN"
        0xB9             1           SUPERSCRIPT 1
        0xBA           "NaN"
        0xBC            1/4          VULGAR FRACTION 1/4
        0xBD            1/2          VULGAR FRACTION 1/2
        0xBE            3/4          VULGAR FRACTION 3/4
        0xBF           "NaN"
        0x660            0           ARABIC-INDIC DIGIT ZERO .. NINE
        0x66A          "NaN"

The fourth (index [3]) element (C<$default>) in the list returned for this
format is C<"NaN">.

=item B<C<n>>

means the Name property.  All the elements of the map array are simple
scalars, but some of them contain special strings that require more work to
get the actual name.

Entries such as:

 CJK UNIFIED IDEOGRAPH-<code point>

mean that the name for the code point is "CJK UNIFIED IDEOGRAPH-"
with the code point (expressed in hexadecimal) appended to it, like "CJK
UNIFIED IDEOGRAPH-3403" (similarly for S<C<CJK COMPATIBILITY IDEOGRAPH-E<lt>code
pointE<gt>>>).

Also, entries like

 <hangul syllable>

means that the name is algorithmically calculated.  This is easily done by
the function L<charnames/charnames::viacode(code)>.

Note that for control characters (C<Gc=cc>), Unicode's data files have the
string "C<E<lt>controlE<gt>>", but the real name of each of these characters is the empty
string.  This function returns that real name, the empty string.  (There are
names for these characters, but they are considered aliases, not the Name
property name, and are contained in the C<Name_Alias> property.)

=item B<C<ad>>

means the Decomposition_Mapping property.  This property is like C<"al">
properties, except that one of the scalar elements is of the form:

 <hangul syllable>

This signifies that this entry should be replaced by the decompositions for
all the code points whose decomposition is algorithmically calculated.  (All
of them are currently in one range and no others outside the range are likely
to ever be added to Unicode; the C<"n"> format
has this same entry.)  These can be generated via the function
L<Unicode::Normalize::NFD()|Unicode::Normalize>.

Note that the mapping is the one that is specified in the Unicode data files,
and to get the final decomposition, it may need to be applied recursively.
Unicode in fact discourages use of this property except internally in
implementations of the Unicode Normalization Algorithm.

The fourth (index [3]) element (C<$default>) in the list returned for this
format is 0.

=back

Note that a format begins with the letter "a" if and only the property it is
for requires adjustments by adding the offsets in multi-element ranges.  For
all these properties, an entry should be adjusted only if the map is a scalar
which is an integer.  That is, it must match the regular expression:

    / ^ -? \d+ $ /xa

Further, the first element in a range never needs adjustment, as the
adjustment would be just adding 0.

A binary search such as that provided by L</search_invlist()>, can be used to
quickly find a code point in the inversion list, and hence its corresponding
mapping.

The final, fourth element (index [3], assigned to C<$default> in the "block"
example) in the four element list returned by this function is used with the
C<"a"> format types; it may also be useful for applications
that wish to convert the returned inversion map data structure into some
other, such as a hash.  It gives the mapping that most code points map to
under the property.  If you establish the convention that any code point not
explicitly listed in your data structure maps to this value, you can
potentially make your data structure much smaller.  As you construct your data
structure from the one returned by this function, simply ignore those ranges
that map to this value.  For example, to
convert to the data structure searchable by L</charinrange()>, you can follow
this recipe for properties that don't require adjustments:

 my ($list_ref, $map_ref, $format, $default) = prop_invmap($property);
 my @range_list;

 # Look at each element in the list, but the -2 is needed because we
 # look at $i+1 in the loop, and the final element is guaranteed to map
 # to $default by prop_invmap(), so we would skip it anyway.
 for my $i (0 .. @$list_ref - 2) {
    next if $map_ref->[$i] eq $default;
    push @range_list, [ $list_ref->[$i],
                        $list_ref->[$i+1],
                        $map_ref->[$i]
                      ];
 }

 print charinrange(\@range_list, $code_point), "\n";

With this, C<charinrange()> will return C<undef> if its input code point maps
to C<$default>.  You can avoid this by omitting the C<next> statement, and adding
a line after the loop to handle the final element of the inversion map.

Similarly, this recipe can be used for properties that do require adjustments:

 for my $i (0 .. @$list_ref - 2) {
    next if $map_ref->[$i] eq $default;

    # prop_invmap() guarantees that if the mapping is to an array, the
    # range has just one element, so no need to worry about adjustments.
    if (ref $map_ref->[$i]) {
        push @range_list,
                   [ $list_ref->[$i], $list_ref->[$i], $map_ref->[$i] ];
    }
    else {  # Otherwise each element is actually mapped to a separate
            # value, so the range has to be split into single code point
            # ranges.

        my $adjustment = 0;

        # For each code point that gets mapped to something...
        for my $j ($list_ref->[$i] .. $list_ref->[$i+1] -1 ) {

            # ... add a range consisting of just it mapping to the
            # original plus the adjustment, which is incremented for the
            # next time through the loop, as the offset increases by 1
            # for each element in the range
            push @range_list,
                             [ $j, $j, $map_ref->[$i] + $adjustment++ ];
        }
    }
 }

Note that the inversion maps returned for the C<Case_Folding> and
C<Simple_Case_Folding> properties do not include the Turkic-locale mappings.
Use L</casefold()> for these.

C<prop_invmap> does not know about any user-defined properties, and will
return C<undef> if called with one of those.

The returned values for the Perl extension properties, such as C<Any> and
C<Greek> are somewhat misleading.  The values are either C<"Y"> or C<"N>".
All Unicode properties are bipartite, so you can actually use the C<"Y"> or
C<"N>" in a Perl regular expression for these, like C<qr/\p{ID_Start=Y/}> or
C<qr/\p{Upper=N/}>.  But the Perl extensions aren't specified this way, only
like C</qr/\p{Any}>, I<etc>.  You can't actually use the C<"Y"> and C<"N>" in
them.

=head3 Getting every available name

Instead of reading the Unicode Database directly from files, as you were able
to do for a long time, you are encouraged to use the supplied functions. So,
instead of reading C<Name.pl> directly, which changed formats in 5.32, and may
do so again without notice in the future or even disappear, you ought to use
L</prop_invmap()> like this:

  my (%name, %cp, %cps, $n);
  # All codepoints
  foreach my $cat (qw( Name Name_Alias )) {
      my ($codepoints, $names, $format, $default) = prop_invmap($cat);
      # $format => "n", $default => ""
      foreach my $i (0 .. @$codepoints - 2) {
          my ($cp, $n) = ($codepoints->[$i], $names->[$i]);
          # If $n is a ref, the same codepoint has multiple names
          foreach my $name (ref $n ? @$n : $n) {
              $name{$cp} //= $name;
              $cp{$name} //= $cp;
          }
      }
  }
  # Named sequences
  {   my %ns = namedseq();
      foreach my $name (sort { $ns{$a} cmp $ns{$b} } keys %ns) {
          $cp{$name} //= [ map { ord } split "" => $ns{$name} ];
      }
  }

=cut

# User-defined properties could be handled with some changes to SWASHNEW;
# if done, consideration should be given to the fact that the user subroutine
# could return different results with each call, which could lead to some
# security issues.

# One could store things in memory so they don't have to be recalculated, but
# it is unlikely this will be called often, and some properties would take up
# significant memory.

# These are created by mktables for this routine and stored in unicore/UCD.pl
# where their structures are described.
our @algorithmic_named_code_points;
our $HANGUL_BEGIN;
our $HANGUL_COUNT;

sub prop_invmap ($;$) {

    croak __PACKAGE__, "::prop_invmap: must be called in list context" unless wantarray;

    my $prop = $_[0];
    return unless defined $prop;

    # Undocumented way to get at Perl internal properties; it may be changed
    # or removed without notice at any time.  It currently also changes the
    # output to use the format specified in the file rather than the one we
    # normally compute and return
    my $internal_ok = defined $_[1] && $_[1] eq '_perl_core_internal_ok';

    # Fail internal properties
    return if $prop =~ /^_/ && ! $internal_ok;

    # The values returned by this function.
    my (@invlist, @invmap, $format, $missing);

    # The swash has two components we look at, the base list, and a hash,
    # named 'SPECIALS', containing any additional members whose mappings don't
    # fit into the base list scheme of things.  These generally 'override'
    # any value in the base list for the same code point.
    my $overrides;

    require "unicore/UCD.pl";

RETRY:

    # If there are multiple entries for a single code point
    my $has_multiples = 0;

    # Try to get the map swash for the property.  They have 'To' prepended to
    # the property name, and 32 means we will accept 32 bit return values.
    # The 0 means we aren't calling this from tr///.
    my $swash = SWASHNEW(__PACKAGE__, "To$prop", undef, 32, 0);

    # If didn't find it, could be because needs a proxy.  And if was the
    # 'Block' or 'Name' property, use a proxy even if did find it.  Finding it
    # in these cases would be the result of the installation changing mktables
    # to output the Block or Name tables.  The Block table gives block names
    # in the new-style, and this routine is supposed to return old-style block
    # names.  The Name table is valid, but we need to execute the special code
    # below to add in the algorithmic-defined name entries.
    # And NFKCCF needs conversion, so handle that here too.
    if (ref $swash eq ""
        || $swash->{'TYPE'} =~ / ^ To (?: Blk | Na | NFKCCF ) $ /x)
    {

        # Get the short name of the input property, in standard form
        my ($second_try) = prop_aliases($prop);
        return unless $second_try;
        $second_try = loose_name(lc $second_try);

        if ($second_try eq "in") {

            # This property is identical to age for inversion map purposes
            $prop = "age";
            goto RETRY;
        }
        elsif ($second_try =~ / ^ s ( cf | fc | [ltu] c ) $ /x) {

            # These properties use just the LIST part of the full mapping,
            # which includes the simple maps that are otherwise overridden by
            # the SPECIALS.  So all we need do is to not look at the SPECIALS;
            # set $overrides to indicate that
            $overrides = -1;

            # The full name is the simple name stripped of its initial 's'
            $prop = $1;

            # .. except for this case
            $prop = 'cf' if $prop eq 'fc';

            goto RETRY;
        }
        elsif ($second_try eq "blk") {

            # We use the old block names.  Just create a fake swash from its
            # data.
            _charblocks();
            my %blocks;
            $blocks{'LIST'} = "";
            $blocks{'TYPE'} = "ToBlk";
            $SwashInfo{ToBlk}{'missing'} = "No_Block";
            $SwashInfo{ToBlk}{'format'} = "s";

            foreach my $block (@BLOCKS) {
                $blocks{'LIST'} .= sprintf "%x\t%x\t%s\n",
                                           $block->[0],
                                           $block->[1],
                                           $block->[2];
            }
            $swash = \%blocks;
        }
        elsif ($second_try eq "na") {

            # Use the combo file that has all the Name-type properties in it,
            # extracting just the ones that are for the actual 'Name'
            # property.  And create a fake swash from it.
            my %names;
            $names{'LIST'} = "";
            my $original = do "unicore/Name.pl";

            # Change the double \n format of the file back to single lines
            # with a tab
            $original =~ s/\n\n/\e/g;   # Use a control that shouldn't occur
                                        #in the file
            $original =~ s/\n/\t/g;
            $original =~ s/\e/\n/g;

            my $algorithm_names = \@algorithmic_named_code_points;

            # We need to remove the names from it that are aliases.  For that
            # we need to also read in that table.  Create a hash with the keys
            # being the code points, and the values being a list of the
            # aliases for the code point key.
            my ($aliases_code_points, $aliases_maps, undef, undef)
                  = &prop_invmap("_Perl_Name_Alias", '_perl_core_internal_ok');
            my %aliases;
            for (my $i = 0; $i < @$aliases_code_points; $i++) {
                my $code_point = $aliases_code_points->[$i];
                $aliases{$code_point} = $aliases_maps->[$i];

                # If not already a list, make it into one, so that later we
                # can treat things uniformly
                if (! ref $aliases{$code_point}) {
                    $aliases{$code_point} = [ $aliases{$code_point} ];
                }

                # Remove the alias type from the entry, retaining just the
                # name.
                map { s/:.*// } @{$aliases{$code_point}};
            }

            my $i = 0;
            foreach my $line (split "\n", $original) {
                my ($hex_code_point, $name) = split "\t", $line;

                # Weeds out any comments, blank lines, and named sequences
                next if $hex_code_point =~ /[^[:xdigit:]]/a;

                my $code_point = hex $hex_code_point;

                # The name of all controls is the default: the empty string.
                # The set of controls is immutable
                next if chr($code_point) =~ /[[:cntrl:]]/u;

                # If this is a name_alias, it isn't a name
                next if grep { $_ eq $name } @{$aliases{$code_point}};

                # If we are beyond where one of the special lines needs to
                # be inserted ...
                while ($i < @$algorithm_names
                    && $code_point > $algorithm_names->[$i]->{'low'})
                {

                    # ... then insert it, ahead of what we were about to
                    # output
                    $names{'LIST'} .= sprintf "%x\t%x\t%s\n",
                                            $algorithm_names->[$i]->{'low'},
                                            $algorithm_names->[$i]->{'high'},
                                            $algorithm_names->[$i]->{'name'};

                    # Done with this range.
                    $i++;

                    # We loop until all special lines that precede the next
                    # regular one are output.
                }

                # Here, is a normal name.
                $names{'LIST'} .= sprintf "%x\t\t%s\n", $code_point, $name;
            } # End of loop through all the names

            $names{'TYPE'} = "ToNa";
            $SwashInfo{ToNa}{'missing'} = "";
            $SwashInfo{ToNa}{'format'} = "n";
            $swash = \%names;
        }
        elsif ($second_try =~ / ^ ( d [mt] ) $ /x) {

            # The file is a combination of dt and dm properties.  Create a
            # fake swash from the portion that we want.
            my $original = do "unicore/Decomposition.pl";
            my %decomps;

            if ($second_try eq 'dt') {
                $decomps{'TYPE'} = "ToDt";
                $SwashInfo{'ToDt'}{'missing'} = "None";
                $SwashInfo{'ToDt'}{'format'} = "s";
            }   # 'dm' is handled below, with 'nfkccf'

            $decomps{'LIST'} = "";

            # This property has one special range not in the file: for the
            # hangul syllables.  But not in Unicode version 1.
            UnicodeVersion() unless defined $v_unicode_version;
            my $done_hangul = ($v_unicode_version lt v2.0.0)
                              ? 1
                              : 0;    # Have we done the hangul range ?
            foreach my $line (split "\n", $original) {
                my ($hex_lower, $hex_upper, $type_and_map) = split "\t", $line;
                my $code_point = hex $hex_lower;
                my $value;
                my $redo = 0;

                # The type, enclosed in <...>, precedes the mapping separated
                # by blanks
                if ($type_and_map =~ / ^ < ( .* ) > \s+ (.*) $ /x) {
                    $value = ($second_try eq 'dt') ? $1 : $2
                }
                else {  # If there is no type specified, it's canonical
                    $value = ($second_try eq 'dt')
                             ? "Canonical" :
                             $type_and_map;
                }

                # Insert the hangul range at the appropriate spot.
                if (! $done_hangul && $code_point > $HANGUL_BEGIN) {
                    $done_hangul = 1;
                    $decomps{'LIST'} .=
                                sprintf "%x\t%x\t%s\n",
                                        $HANGUL_BEGIN,
                                        $HANGUL_BEGIN + $HANGUL_COUNT - 1,
                                        ($second_try eq 'dt')
                                        ? "Canonical"
                                        : "<hangul syllable>";
                }

                if ($value =~ / / && $hex_upper ne "" && $hex_upper ne $hex_lower) {
                    $line = sprintf("%04X\t%s\t%s", hex($hex_lower) + 1, $hex_upper, $value);
                    $hex_upper = "";
                    $redo = 1;
                }

                # And append this to our constructed LIST.
                $decomps{'LIST'} .= "$hex_lower\t$hex_upper\t$value\n";

                redo if $redo;
            }
            $swash = \%decomps;
        }
        elsif ($second_try ne 'nfkccf') { # Don't know this property. Fail.
            return;
        }

        if ($second_try eq 'nfkccf' || $second_try eq 'dm') {

            # The 'nfkccf' property is stored in the old format for backwards
            # compatibility for any applications that has read its file
            # directly before prop_invmap() existed.
            # And the code above has extracted the 'dm' property from its file
            # yielding the same format.  So here we convert them to adjusted
            # format for compatibility with the other properties similar to
            # them.
            my %revised_swash;

            # We construct a new converted list.
            my $list = "";

            my @ranges = split "\n", $swash->{'LIST'};
            for (my $i = 0; $i < @ranges; $i++) {
                my ($hex_begin, $hex_end, $map) = split "\t", $ranges[$i];

                # The dm property has maps that are space separated sequences
                # of code points, as well as the special entry "<hangul
                # syllable>, which also contains a blank.
                my @map = split " ", $map;
                if (@map > 1) {

                    # If it's just the special entry, append as-is.
                    if ($map eq '<hangul syllable>') {
                        $list .= "$ranges[$i]\n";
                    }
                    else {

                        # These should all be single-element ranges.
                        croak __PACKAGE__, "::prop_invmap: Not expecting a mapping with multiple code points in a multi-element range, $ranges[$i]" if $hex_end ne "" && $hex_end ne $hex_begin;

                        # Convert them to decimal, as that's what's expected.
                        $list .= "$hex_begin\t\t"
                            . join(" ", map { hex } @map)
                            . "\n";
                    }
                    next;
                }

                # Here, the mapping doesn't have a blank, is for a single code
                # point.
                my $begin = hex $hex_begin;
                my $end = (defined $hex_end && $hex_end ne "")
                        ? hex $hex_end
                        : $begin;

                # Again, the output is to be in decimal.
                my $decimal_map = hex $map;

                # We know that multi-element ranges with the same mapping
                # should not be adjusted, as after the adjustment
                # multi-element ranges are for consecutive increasing code
                # points.  Further, the final element in the list won't be
                # adjusted, as there is nothing after it to include in the
                # adjustment
                if ($begin != $end || $i == @ranges -1) {

                    # So just convert these to single-element ranges
                    foreach my $code_point ($begin .. $end) {
                        $list .= sprintf("%04X\t\t%d\n",
                                        $code_point, $decimal_map);
                    }
                }
                else {

                    # Here, we have a candidate for adjusting.  What we do is
                    # look through the subsequent adjacent elements in the
                    # input.  If the map to the next one differs by 1 from the
                    # one before, then we combine into a larger range with the
                    # initial map.  Loop doing this until we find one that
                    # can't be combined.

                    my $offset = 0;     # How far away are we from the initial
                                        # map
                    my $squished = 0;   # ? Did we squish at least two
                                        # elements together into one range
                    for ( ; $i < @ranges; $i++) {
                        my ($next_hex_begin, $next_hex_end, $next_map)
                                                = split "\t", $ranges[$i+1];

                        # In the case of 'dm', the map may be a sequence of
                        # multiple code points, which are never combined with
                        # another range
                        last if $next_map =~ / /;

                        $offset++;
                        my $next_decimal_map = hex $next_map;

                        # If the next map is not next in sequence, it
                        # shouldn't be combined.
                        last if $next_decimal_map != $decimal_map + $offset;

                        my $next_begin = hex $next_hex_begin;

                        # Likewise, if the next element isn't adjacent to the
                        # previous one, it shouldn't be combined.
                        last if $next_begin != $begin + $offset;

                        my $next_end = (defined $next_hex_end
                                        && $next_hex_end ne "")
                                            ? hex $next_hex_end
                                            : $next_begin;

                        # And finally, if the next element is a multi-element
                        # range, it shouldn't be combined.
                        last if $next_end != $next_begin;

                        # Here, we will combine.  Loop to see if we should
                        # combine the next element too.
                        $squished = 1;
                    }

                    if ($squished) {

                        # Here, 'i' is the element number of the last element to
                        # be combined, and the range is single-element, or we
                        # wouldn't be combining.  Get it's code point.
                        my ($hex_end, undef, undef) = split "\t", $ranges[$i];
                        $list .= "$hex_begin\t$hex_end\t$decimal_map\n";
                    } else {

                        # Here, no combining done.  Just append the initial
                        # (and current) values.
                        $list .= "$hex_begin\t\t$decimal_map\n";
                    }
                }
            } # End of loop constructing the converted list

            # Finish up the data structure for our converted swash
            my $type = ($second_try eq 'nfkccf') ? 'ToNFKCCF' : 'ToDm';
            $revised_swash{'LIST'} = $list;
            $revised_swash{'TYPE'} = $type;
            $revised_swash{'SPECIALS'} = $swash->{'SPECIALS'};
            $swash = \%revised_swash;

            $SwashInfo{$type}{'missing'} = 0;
            $SwashInfo{$type}{'format'} = 'a';
        }
    }

    if ($swash->{'EXTRAS'}) {
        carp __PACKAGE__, "::prop_invmap: swash returned for $prop unexpectedly has EXTRAS magic";
        return;
    }

    # Here, have a valid swash return.  Examine it.
    my $returned_prop = $swash->{'TYPE'};

    # All properties but binary ones should have 'missing' and 'format'
    # entries
    $missing = $SwashInfo{$returned_prop}{'missing'};
    $missing = 'N' unless defined $missing;

    $format = $SwashInfo{$returned_prop}{'format'};
    $format = 'b' unless defined $format;

    my $requires_adjustment = $format =~ /^a/;

    if ($swash->{'LIST'} =~ /^V/) {
        @invlist = split "\n", $swash->{'LIST'} =~ s/ \s* (?: \# .* )? $ //xmgr;

        shift @invlist;     # Get rid of 'V';

        # Could need to be inverted: add or subtract a 0 at the beginning of
        # the list.
        if ($swash->{'INVERT_IT'}) {
            if (@invlist && $invlist[0] == 0) {
                shift @invlist;
            }
            else {
                unshift @invlist, 0;
            }
        }

        if (@invlist) {
            foreach my $i (0 .. @invlist - 1) {
                $invmap[$i] = ($i % 2 == 0) ? 'Y' : 'N'
            }

            # The map includes lines for all code points; add one for the range
            # from 0 to the first Y.
            if ($invlist[0] != 0) {
                unshift @invlist, 0;
                unshift @invmap, 'N';
            }
        }
    }
    else {
        if ($swash->{'INVERT_IT'}) {
            croak __PACKAGE__, ":prop_invmap: Don't know how to deal with inverted";
        }

        # The LIST input lines look like:
        # ...
        # 0374\t\tCommon
        # 0375\t0377\tGreek   # [3]
        # 037A\t037D\tGreek   # [4]
        # 037E\t\tCommon
        # 0384\t\tGreek
        # ...
        #
        # Convert them to like
        # 0374 => Common
        # 0375 => Greek
        # 0378 => $missing
        # 037A => Greek
        # 037E => Common
        # 037F => $missing
        # 0384 => Greek
        #
        # For binary properties, the final non-comment column is absent, and
        # assumed to be 'Y'.

        foreach my $range (split "\n", $swash->{'LIST'}) {
            $range =~ s/ \s* (?: \# .* )? $ //xg; # rmv trailing space, comments

            # Find the beginning and end of the range on the line
            my ($hex_begin, $hex_end, $map) = split "\t", $range;
            my $begin = hex $hex_begin;
            no warnings 'portable';
            my $end = (defined $hex_end && $hex_end ne "")
                    ? hex $hex_end
                    : $begin;

            # Each time through the loop (after the first):
            # $invlist[-2] contains the beginning of the previous range processed
            # $invlist[-1] contains the end+1 of the previous range processed
            # $invmap[-2] contains the value of the previous range processed
            # $invmap[-1] contains the default value for missing ranges
            #                                                       ($missing)
            #
            # Thus, things are set up for the typical case of a new
            # non-adjacent range of non-missings to be added.  But, if the new
            # range is adjacent, it needs to replace the [-1] element; and if
            # the new range is a multiple value of the previous one, it needs
            # to be added to the [-2] map element.

            # The first time through, everything will be empty.  If the
            # property doesn't have a range that begins at 0, add one that
            # maps to $missing
            if (! @invlist) {
                if ($begin != 0) {
                    push @invlist, 0;
                    push @invmap, $missing;
                }
            }
            elsif (@invlist > 1 && $invlist[-2] == $begin) {

                # Here we handle the case where the input has multiple entries
                # for each code point.  mktables should have made sure that
                # each such range contains only one code point.  At this
                # point, $invlist[-1] is the $missing that was added at the
                # end of the last loop iteration, and [-2] is the last real
                # input code point, and that code point is the same as the one
                # we are adding now, making the new one a multiple entry.  Add
                # it to the existing entry, either by pushing it to the
                # existing list of multiple entries, or converting the single
                # current entry into a list with both on it.  This is all we
                # need do for this iteration.

                if ($end != $begin) {
                    croak __PACKAGE__, ":prop_invmap: Multiple maps per code point in '$prop' require single-element ranges: begin=$begin, end=$end, map=$map";
                }
                if (! ref $invmap[-2]) {
                    $invmap[-2] = [ $invmap[-2], $map ];
                }
                else {
                    push @{$invmap[-2]}, $map;
                }
                $has_multiples = 1;
                next;
            }
            elsif ($invlist[-1] == $begin) {

                # If the input isn't in the most compact form, so that there
                # are two adjacent ranges that map to the same thing, they
                # should be combined (EXCEPT where the arrays require
                # adjustments, in which case everything is already set up
                # correctly).  This happens in our constructed dt mapping, as
                # Element [-2] is the map for the latest range so far
                # processed.  Just set the beginning point of the map to
                # $missing (in invlist[-1]) to 1 beyond where this range ends.
                # For example, in
                # 12\t13\tXYZ
                # 14\t17\tXYZ
                # we have set it up so that it looks like
                # 12 => XYZ
                # 14 => $missing
                #
                # We now see that it should be
                # 12 => XYZ
                # 18 => $missing
                if (! $requires_adjustment && @invlist > 1 && ( (defined $map)
                                    ? $invmap[-2] eq $map
                                    : $invmap[-2] eq 'Y'))
                {
                    $invlist[-1] = $end + 1;
                    next;
                }

                # Here, the range started in the previous iteration that maps
                # to $missing starts at the same code point as this range.
                # That means there is no gap to fill that that range was
                # intended for, so we just pop it off the parallel arrays.
                pop @invlist;
                pop @invmap;
            }

            # Add the range beginning, and the range's map.
            push @invlist, $begin;
            if ($returned_prop eq 'ToDm') {

                # The decomposition maps are either a line like <hangul
                # syllable> which are to be taken as is; or a sequence of code
                # points in hex and separated by blanks.  Convert them to
                # decimal, and if there is more than one, use an anonymous
                # array as the map.
                if ($map =~ /^ < /x) {
                    push @invmap, $map;
                }
                else {
                    my @map = split " ", $map;
                    if (@map == 1) {
                        push @invmap, $map[0];
                    }
                    else {
                        push @invmap, \@map;
                    }
                }
            }
            else {

                # Otherwise, convert hex formatted list entries to decimal;
                # add a 'Y' map for the missing value in binary properties, or
                # otherwise, use the input map unchanged.
                $map = ($format eq 'x' || $format eq 'ax')
                    ? hex $map
                    : $format eq 'b'
                    ? 'Y'
                    : $map;
                push @invmap, $map;
            }

            # We just started a range.  It ends with $end.  The gap between it
            # and the next element in the list must be filled with a range
            # that maps to the default value.  If there is no gap, the next
            # iteration will pop this, unless there is no next iteration, and
            # we have filled all of the Unicode code space, so check for that
            # and skip.
            if ($end < $MAX_CP) {
                push @invlist, $end + 1;
                push @invmap, $missing;
            }
        }
    }

    # If the property is empty, make all code points use the value for missing
    # ones.
    if (! @invlist) {
        push @invlist, 0;
        push @invmap, $missing;
    }

    # The final element is always for just the above-Unicode code points.  If
    # not already there, add it.  It merely splits the current final range
    # that extends to infinity into two elements, each with the same map.
    # (This is to conform with the API that says the final element is for
    # $MAX_UNICODE_CODEPOINT + 1 .. INFINITY.)
    if ($invlist[-1] != $MAX_UNICODE_CODEPOINT + 1) {
        push @invmap, $invmap[-1];
        push @invlist, $MAX_UNICODE_CODEPOINT + 1;
    }

    # The second component of the map are those values that require
    # non-standard specification, stored in SPECIALS.  These override any
    # duplicate code points in LIST.  If we are using a proxy, we may have
    # already set $overrides based on the proxy.
    $overrides = $swash->{'SPECIALS'} unless defined $overrides;
    if ($overrides) {

        # A negative $overrides implies that the SPECIALS should be ignored,
        # and a simple 'a' list is the value.
        if ($overrides < 0) {
            $format = 'a';
        }
        else {

            # Currently, all overrides are for properties that normally map to
            # single code points, but now some will map to lists of code
            # points (but there is an exception case handled below).
            $format = 'al';

            # Look through the overrides.
            foreach my $cp_maybe_utf8 (keys %$overrides) {
                my $cp;
                my @map;

                # If the overrides came from SPECIALS, the code point keys are
                # packed UTF-8.
                if ($overrides == $swash->{'SPECIALS'}) {
                    $cp = $cp_maybe_utf8;
                    if (! utf8::decode($cp)) {
                        croak __PACKAGE__, "::prop_invmap: Malformed UTF-8: ",
                              map { sprintf("\\x{%02X}", unpack("C", $_)) }
                                                                split "", $cp;
                    }

                    $cp = unpack("W", $cp);
                    @map = unpack "W*", $swash->{'SPECIALS'}{$cp_maybe_utf8};

                    # The empty string will show up unpacked as an empty
                    # array.
                    $format = 'ale' if @map == 0;
                }
                else {

                    # But if we generated the overrides, we didn't bother to
                    # pack them, and we, so far, do this only for properties
                    # that are 'a' ones.
                    $cp = $cp_maybe_utf8;
                    @map = hex $overrides->{$cp};
                    $format = 'a';
                }

                # Find the range that the override applies to.
                my $i = search_invlist(\@invlist, $cp);
                if ($cp < $invlist[$i] || $cp >= $invlist[$i + 1]) {
                    croak __PACKAGE__, "::prop_invmap: wrong_range, cp=$cp; i=$i, current=$invlist[$i]; next=$invlist[$i + 1]"
                }

                # And what that range currently maps to
                my $cur_map = $invmap[$i];

                # If there is a gap between the next range and the code point
                # we are overriding, we have to add elements to both arrays to
                # fill that gap, using the map that applies to it, which is
                # $cur_map, since it is part of the current range.
                if ($invlist[$i + 1] > $cp + 1) {
                    #use feature 'say';
                    #say "Before splice:";
                    #say 'i-2=[', $i-2, ']', sprintf("%04X maps to %s", $invlist[$i-2], $invmap[$i-2]) if $i >= 2;
                    #say 'i-1=[', $i-1, ']', sprintf("%04X maps to %s", $invlist[$i-1], $invmap[$i-1]) if $i >= 1;
                    #say 'i  =[', $i, ']', sprintf("%04X maps to %s", $invlist[$i], $invmap[$i]);
                    #say 'i+1=[', $i+1, ']', sprintf("%04X maps to %s", $invlist[$i+1], $invmap[$i+1]) if $i < @invlist + 1;
                    #say 'i+2=[', $i+2, ']', sprintf("%04X maps to %s", $invlist[$i+2], $invmap[$i+2]) if $i < @invlist + 2;

                    splice @invlist, $i + 1, 0, $cp + 1;
                    splice @invmap, $i + 1, 0, $cur_map;

                    #say "After splice:";
                    #say 'i-2=[', $i-2, ']', sprintf("%04X maps to %s", $invlist[$i-2], $invmap[$i-2]) if $i >= 2;
                    #say 'i-1=[', $i-1, ']', sprintf("%04X maps to %s", $invlist[$i-1], $invmap[$i-1]) if $i >= 1;
                    #say 'i  =[', $i, ']', sprintf("%04X maps to %s", $invlist[$i], $invmap[$i]);
                    #say 'i+1=[', $i+1, ']', sprintf("%04X maps to %s", $invlist[$i+1], $invmap[$i+1]) if $i < @invlist + 1;
                    #say 'i+2=[', $i+2, ']', sprintf("%04X maps to %s", $invlist[$i+2], $invmap[$i+2]) if $i < @invlist + 2;
                }

                # If the remaining portion of the range is multiple code
                # points (ending with the one we are replacing, guaranteed by
                # the earlier splice).  We must split it into two
                if ($invlist[$i] < $cp) {
                    $i++;   # Compensate for the new element

                    #use feature 'say';
                    #say "Before splice:";
                    #say 'i-2=[', $i-2, ']', sprintf("%04X maps to %s", $invlist[$i-2], $invmap[$i-2]) if $i >= 2;
                    #say 'i-1=[', $i-1, ']', sprintf("%04X maps to %s", $invlist[$i-1], $invmap[$i-1]) if $i >= 1;
                    #say 'i  =[', $i, ']', sprintf("%04X maps to %s", $invlist[$i], $invmap[$i]);
                    #say 'i+1=[', $i+1, ']', sprintf("%04X maps to %s", $invlist[$i+1], $invmap[$i+1]) if $i < @invlist + 1;
                    #say 'i+2=[', $i+2, ']', sprintf("%04X maps to %s", $invlist[$i+2], $invmap[$i+2]) if $i < @invlist + 2;

                    splice @invlist, $i, 0, $cp;
                    splice @invmap, $i, 0, 'dummy';

                    #say "After splice:";
                    #say 'i-2=[', $i-2, ']', sprintf("%04X maps to %s", $invlist[$i-2], $invmap[$i-2]) if $i >= 2;
                    #say 'i-1=[', $i-1, ']', sprintf("%04X maps to %s", $invlist[$i-1], $invmap[$i-1]) if $i >= 1;
                    #say 'i  =[', $i, ']', sprintf("%04X maps to %s", $invlist[$i], $invmap[$i]);
                    #say 'i+1=[', $i+1, ']', sprintf("%04X maps to %s", $invlist[$i+1], $invmap[$i+1]) if $i < @invlist + 1;
                    #say 'i+2=[', $i+2, ']', sprintf("%04X maps to %s", $invlist[$i+2], $invmap[$i+2]) if $i < @invlist + 2;
                }

                # Here, the range we are overriding contains a single code
                # point.  The result could be the empty string, a single
                # value, or a list.  If the last case, we use an anonymous
                # array.
                $invmap[$i] = (scalar @map == 0)
                               ? ""
                               : (scalar @map > 1)
                                  ? \@map
                                  : $map[0];
            }
        }
    }
    elsif ($format eq 'x') {

        # All hex-valued properties are really to code points, and have been
        # converted to decimal.
        $format = 's';
    }
    elsif ($returned_prop eq 'ToDm') {
        $format = 'ad';
    }
    elsif ($format eq 'sw') { # blank-separated elements to form a list.
        map { $_ = [ split " ", $_  ] if $_ =~ / / } @invmap;
        $format = 'sl';
    }
    elsif ($returned_prop =~ / To ( _Perl )? NameAlias/x) {

        # This property currently doesn't have any lists, but theoretically
        # could
        $format = 'sl';
    }
    elsif ($returned_prop eq 'ToPerlDecimalDigit') {
        $format = 'ae';
    }
    elsif ($returned_prop eq 'ToNv') {

        # The one property that has this format is stored as a delta, so needs
        # to indicate that need to add code point to it.
        $format = 'ar';
    }
    elsif ($format eq 'ax') {

        # Normally 'ax' properties have overrides, and will have been handled
        # above, but if not, they still need adjustment, and the hex values
        # have already been converted to decimal
        $format = 'a';
    }
    elsif ($format ne 'n' && $format !~ / ^ a /x) {

        # All others are simple scalars
        $format = 's';
    }
    if ($has_multiples &&  $format !~ /l/) {
	croak __PACKAGE__, "::prop_invmap: Wrong format '$format' for prop_invmap('$prop'); should indicate has lists";
    }

    return (\@invlist, \@invmap, $format, $missing);
}

sub search_invlist {

=pod

=head2 B<search_invlist()>

 use Unicode::UCD qw(prop_invmap prop_invlist);
 use Unicode::UCD 'search_invlist';

 my @invlist = prop_invlist($property_name);
 print $code_point, ((search_invlist(\@invlist, $code_point) // -1) % 2)
                     ? " isn't"
                     : " is",
     " in $property_name\n";

 my ($blocks_ranges_ref, $blocks_map_ref) = prop_invmap("Block");
 my $index = search_invlist($blocks_ranges_ref, $code_point);
 print "$code_point is in block ", $blocks_map_ref->[$index], "\n";

C<search_invlist> is used to search an inversion list returned by
C<prop_invlist> or C<prop_invmap> for a particular L</code point argument>.
C<undef> is returned if the code point is not found in the inversion list
(this happens only when it is not a legal L</code point argument>, or is less
than the list's first element).  A warning is raised in the first instance.

Otherwise, it returns the index into the list of the range that contains the
code point.; that is, find C<i> such that

    list[i]<= code_point < list[i+1].

As explained in L</prop_invlist()>, whether a code point is in the list or not
depends on if the index is even (in) or odd (not in).  And as explained in
L</prop_invmap()>, the index is used with the returned parallel array to find
the mapping.

=cut


    my $list_ref = shift;
    my $input_code_point = shift;
    my $code_point = _getcode($input_code_point);

    if (! defined $code_point) {
        carp __PACKAGE__, "::search_invlist: unknown code '$input_code_point'";
        return;
    }

    my $max_element = @$list_ref - 1;

    # Return undef if list is empty or requested item is before the first element.
    return if $max_element < 0;
    return if $code_point < $list_ref->[0];

    # Short cut something at the far-end of the table.  This also allows us to
    # refer to element [$i+1] without fear of being out-of-bounds in the loop
    # below.
    return $max_element if $code_point >= $list_ref->[$max_element];

    use integer;        # want integer division

    my $i = $max_element / 2;

    my $lower = 0;
    my $upper = $max_element;
    while (1) {

        if ($code_point >= $list_ref->[$i]) {

            # Here we have met the lower constraint.  We can quit if we
            # also meet the upper one.
            last if $code_point < $list_ref->[$i+1];

            $lower = $i;        # Still too low.

        }
        else {

            # Here, $code_point < $list_ref[$i], so look lower down.
            $upper = $i;
        }

        # Split search domain in half to try again.
        my $temp = ($upper + $lower) / 2;

        # No point in continuing unless $i changes for next time
        # in the loop.
        return $i if $temp == $i;
        $i = $temp;
    } # End of while loop

    # Here we have found the offset
    return $i;
}

=head2 Unicode::UCD::UnicodeVersion

This returns the version of the Unicode Character Database, in other words, the
version of the Unicode standard the database implements.  The version is a
string of numbers delimited by dots (C<'.'>).

=cut

my $UNICODEVERSION;

sub UnicodeVersion {
    unless (defined $UNICODEVERSION) {
	my $versionfh = openunicode("version");
	local $/ = "\n";
	chomp($UNICODEVERSION = <$versionfh>);
	croak __PACKAGE__, "::VERSION: strange version '$UNICODEVERSION'"
	    unless $UNICODEVERSION =~ /^\d+(?:\.\d+)+$/;
    }
    $v_unicode_version = pack "C*", split /\./, $UNICODEVERSION;
    return $UNICODEVERSION;
}

=head2 B<Blocks versus Scripts>

The difference between a block and a script is that scripts are closer
to the linguistic notion of a set of code points required to represent
languages, while block is more of an artifact of the Unicode code point
numbering and separation into blocks of consecutive code points (so far the
size of a block is some multiple of 16, like 128 or 256).

For example the Latin B<script> is spread over several B<blocks>, such
as C<Basic Latin>, C<Latin 1 Supplement>, C<Latin Extended-A>, and
C<Latin Extended-B>.  On the other hand, the Latin script does not
contain all the characters of the C<Basic Latin> block (also known as
ASCII): it includes only the letters, and not, for example, the digits
nor the punctuation.

For blocks see L<http://www.unicode.org/Public/UNIDATA/Blocks.txt>

For scripts see UTR #24: L<http://www.unicode.org/reports/tr24/>

=head2 B<Matching Scripts and Blocks>

Scripts are matched with the regular-expression construct
C<\p{...}> (e.g. C<\p{Tibetan}> matches characters of the Tibetan script),
while C<\p{Blk=...}> is used for blocks (e.g. C<\p{Blk=Tibetan}> matches
any of the 256 code points in the Tibetan block).

=head2 Old-style versus new-style block names

Unicode publishes the names of blocks in two different styles, though the two
are equivalent under Unicode's loose matching rules.

The original style uses blanks and hyphens in the block names (except for
C<No_Block>), like so:

 Miscellaneous Mathematical Symbols-B

The newer style replaces these with underscores, like this:

 Miscellaneous_Mathematical_Symbols_B

This newer style is consistent with the values of other Unicode properties.
To preserve backward compatibility, all the functions in Unicode::UCD that
return block names (except as noted) return the old-style ones.
L</prop_value_aliases()> returns the new-style and can be used to convert from
old-style to new-style:

 my $new_style = prop_values_aliases("block", $old_style);

Perl also has single-form extensions that refer to blocks, C<In_Cyrillic>,
meaning C<Block=Cyrillic>.  These have always been written in the new style.

To convert from new-style to old-style, follow this recipe:

 $old_style = charblock((prop_invlist("block=$new_style"))[0]);

(which finds the range of code points in the block using C<prop_invlist>,
gets the lower end of the range (0th element) and then looks up the old name
for its block using C<charblock>).

Note that starting in Unicode 6.1, many of the block names have shorter
synonyms.  These are always given in the new style.

=head2 Use with older Unicode versions

The functions in this module work as well as can be expected when
used on earlier Unicode versions.  But, obviously, they use the available data
from that Unicode version.  For example, if the Unicode version predates the
definition of the script property (Unicode 3.1), then any function that deals
with scripts is going to return C<undef> for the script portion of the return
value.

=head1 AUTHOR

Jarkko Hietaniemi.  Now maintained by perl5 porters.

=cut

1;
