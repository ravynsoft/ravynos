#!perl -w
use v5.15.8;
use strict;
use warnings;
require './regen/regen_lib.pl';
require './regen/charset_translations.pl';
use Unicode::UCD 'prop_invlist';

# This program outputs l1_charclass_tab.h, which defines the guts of the
# PL_charclass table.  Each line is a bit map of properties that the Unicode
# code point at the corresponding position in the table array has.  The first
# line corresponds to code point 0x0, NULL, the last line to 0xFF.  For
# an application to see if the code point "i" has a particular property, it
# just does
#    'PL_charclass[i] & BIT'
# The bit names are of the form 'CC_property_suffix_', where 'CC' stands for
# character class, and 'property' is the corresponding property, and 'suffix'
# is one of '_A' to mean the property is true only if the corresponding code
# point is ASCII, and '_L1' means that the range includes any Latin1
# character (ISO-8859-1 including the C0 and C1 controls).  A property without
# these suffixes does not have different forms for both ranges.

# This program need be run only when adding new properties to it, or upon a
# new Unicode release, to make sure things haven't been changed by it.

# keys are the names of the bits; values are what generates the code points
# that have the bit set, or 0 if \p{key} is the generator
my %bit_names = (
            NONLATIN1_SIMPLE_FOLD   => \&Non_Latin1_Simple_Folds,
            NONLATIN1_FOLD          => \&Non_Latin1_Folds,
            ALPHANUMERIC            => 'Alnum',    # Like \w, but no underscore
            ALPHA                   => 'XPosixAlpha',
            ASCII                   => 0,
            BLANK                   => 0,
            CASED                   => 0,
            CHARNAME_CONT           => '_Perl_Charname_Continue',
            CNTRL                   => 0,
            DIGIT                   => 0,
            GRAPH                   => 0,
            IDFIRST                 => \&Id_First,
            LOWER                   => 'XPosixLower',
            NON_FINAL_FOLD          => \&Non_Final_Folds,
            PRINT                   => 0,
            PUNCT                   => \&Punct_and_Symbols,
            QUOTEMETA               => '_Perl_Quotemeta',
            SPACE                   => 'XPerlSpace',
            UPPER                   => 'XPosixUpper',
            WORDCHAR                => 'XPosixWord',
            XDIGIT                  => 0,
            VERTSPACE               => 0,
            IS_IN_SOME_FOLD         => '_Perl_Any_Folds',
            BINDIGIT                => [ ord '0', ord '1' ],
            OCTDIGIT                => [ ord '0', ord '1', ord '2', ord '3',
                                         ord '4', ord '5', ord '6', ord '7' ],

            # These are the control characters that there are mnemonics for
            MNEMONIC_CNTRL          => [ ord "\a", ord "\b", ord "\e", ord "\f",
                                         ord "\n", ord "\r", ord "\t" ],
);

sub uniques {
    # Returns non-duplicated input values.  From "Perl Best Practices:
    # Encapsulated Cleverness".  p. 455 in first edition.

    my %seen;
    return grep { ! $seen{$_}++ } @_;
}

sub expand_invlist {
    # Return the code points that are in the inversion list given by the
    # argument

    my $invlist_ref = shift;
    my $i;
    my @full_list;

    for (my $i = 0; $i < @$invlist_ref; $i += 2) {
        my $upper = ($i + 1) < @$invlist_ref
                    ? $invlist_ref->[$i+1] - 1      # In range
                    : $Unicode::UCD::MAX_CP;  # To infinity.
        for my $j ($invlist_ref->[$i] .. $upper) {
            push @full_list, $j;
        }
    }

    return @full_list;
}

# Read in the case fold mappings.
my %folded_closure;
my %simple_folded_closure;
my @non_final_folds;
my @non_latin1_simple_folds;
my @folds;
use Unicode::UCD;

# Use the Unicode data file if we are on an ASCII platform (which its data
# is for), and it is in the modern format (starting in Unicode 3.1.0) and
# it is available.  This avoids being affected by potential bugs
# introduced by other layers of Perl
my $file="lib/unicore/CaseFolding.txt";

if (ord('A') == 65
    && pack("C*", split /\./, Unicode::UCD::UnicodeVersion()) ge v3.1.0
    && open my $fh, "<", $file)
{
    @folds = <$fh>;
}
else {
    my ($invlist_ref, $invmap_ref, undef, $default)
                                = Unicode::UCD::prop_invmap('Case_Folding');
    for my $i (0 .. @$invlist_ref - 1 - 1) {
        next if $invmap_ref->[$i] == $default;
        my $adjust = -1;
        for my $j ($invlist_ref->[$i] .. $invlist_ref->[$i+1] -1) {
            $adjust++;

            # Single-code point maps go to a 'C' type
            if (! ref $invmap_ref->[$i]) {
                push @folds, sprintf("%04X; C; %04X\n",
                                    $j,
                                    $invmap_ref->[$i] + $adjust);
            }
            else {  # Multi-code point maps go to 'F'.  prop_invmap()
                    # guarantees that no adjustment is needed for these,
                    # as the range will contain just one element
                push @folds, sprintf("%04X; F; %s\n",
                                    $j,
                                    join " ", map { sprintf "%04X", $_ }
                                                    @{$invmap_ref->[$i]});
            }
        }
    }
}

for (@folds) {
    chomp;

    # Lines look like (without the initial '#'
    #0130; F; 0069 0307; # LATIN CAPITAL LETTER I WITH DOT ABOVE
    # Get rid of comments, ignore blank or comment-only lines
    my $line = $_ =~ s/ (?: \s* \# .* )? $ //rx;
    next unless length $line;
    my ($hex_from, $fold_type, @folded) = split /[\s;]+/, $line;

    my $from = hex $hex_from;

    # Perl only deals with S, C, and F folds
    next if $fold_type ne 'C' and $fold_type ne 'F' and $fold_type ne 'S';

    # Get each code point in the range that participates in this line's fold.
    # The hash has keys of each code point in the range, and values of what it
    # folds to and what folds to it
    for my $i (0 .. @folded - 1) {
        my $fold = hex $folded[$i];
        if ($fold < 256) {
            push @{$folded_closure{$fold}}, $from;
            push @{$simple_folded_closure{$fold}}, $from if $fold_type ne 'F';
        }
        if ($from < 256) {
            push @{$folded_closure{$from}}, $fold;
            push @{$simple_folded_closure{$from}}, $fold if $fold_type ne 'F';
        }

        if (($fold_type eq 'C' || $fold_type eq 'S')
            && ($fold < 256 != $from < 256))
        {
            # Fold is simple (hence can't be a non-final fold, so the 'if'
            # above is mutualy exclusive from the 'if below) and crosses
            # 255/256 boundary.  We keep track of the Latin1 code points
            # in such folds.
            push @non_latin1_simple_folds, ($fold < 256)
                                            ? $fold
                                            : $from;
        }
        elsif ($i < @folded-1
                && $fold < 256
                && ! grep { $_ == $fold } @non_final_folds)
        {
            push @non_final_folds, $fold;

            # Also add the upper case, which in the latin1 range folds to
            # $fold
            push @non_final_folds, ord uc chr $fold;
        }
    }
}

# Now having read all the lines, combine them into the full closure of each
# code point in the range by adding lists together that share a common
# element
foreach my $folded (keys %folded_closure) {
    foreach my $from (grep { $_ < 256 } @{$folded_closure{$folded}}) {
        push @{$folded_closure{$from}}, @{$folded_closure{$folded}};
    }
}
foreach my $folded (keys %simple_folded_closure) {
    foreach my $from (grep { $_ < 256 } @{$simple_folded_closure{$folded}}) {
        push @{$simple_folded_closure{$from}}, @{$simple_folded_closure{$folded}};
    }
}

# We have the single-character folds that cross the 255/256, like KELVIN
# SIGN => 'k', but we need the closure, so add like 'K' to it
foreach my $folded (@non_latin1_simple_folds) {
    foreach my $fold (@{$simple_folded_closure{$folded}}) {
        if ($fold < 256 && ! grep { $fold == $_ } @non_latin1_simple_folds) {
            push @non_latin1_simple_folds, $fold;
        }
    }
}

sub Id_First {
    my @alpha_invlist = prop_invlist("XPosixAlpha");
    my @ids = expand_invlist(\@alpha_invlist);
    push @ids, ord "_";
    return sort { $a <=> $b } uniques @ids;
}

sub Non_Latin1_Folds {
    my @return;

    foreach my $folded (keys %folded_closure) {
        push @return, $folded if grep { $_ > 255 } @{$folded_closure{$folded}};
    }
    return @return;
}

sub Non_Latin1_Simple_Folds { # Latin1 code points that are folded to by
                              # non-Latin1 code points as single character
                              # folds
    return @non_latin1_simple_folds;
}

sub Non_Final_Folds {
    return @non_final_folds;
}

sub Punct_and_Symbols {
    # Sadly, this is inconsistent: \pP and \pS for the ascii range;
    # just \pP outside it.

    my @punct_invlist = prop_invlist("Punct");
    my @return = expand_invlist(\@punct_invlist);

    my @symbols_invlist = prop_invlist("Symbol");
    my @symbols = expand_invlist(\@symbols_invlist);
    foreach my $cp (@symbols) {
        last if $cp > 0x7f;
        push @return, $cp;
    }

    return sort { $a <=> $b } uniques @return;
}

my @bits;   # Each element is a bit map for a single code point

# For each bit type, calculate which code points should have it set
foreach my $bit_name (sort keys %bit_names) {
    my @code_points;

    my $property = $bit_name;   # The bit name is the same as its property,
                                # unless overridden
    $property = $bit_names{$bit_name} if $bit_names{$bit_name};

    if (! ref $property) {
        my @invlist = prop_invlist($property, '_perl_core_internal_ok');
        @code_points = expand_invlist(\@invlist);
    }
    elsif (ref $property eq 'CODE') {
        @code_points = &$property;
    }
    elsif (ref $property eq 'ARRAY') {
        @code_points = @{$property};
    }

    foreach my $cp (@code_points) {
        last if $cp > 0xFF;
        $bits[$cp] .= '|' if $bits[$cp];
        $bits[$cp] .= "(1U<<CC_${bit_name}_)";
    }
}

my $out_fh = open_new('l1_char_class_tab.h', '>',
                      {style => '*', by => $0,
                      from => "Unicode::UCD"});

print $out_fh <<END;
/* For code points whose position is not the same as Unicode,  both are shown
 * in the comment*/
END

# Output the table using fairly short names for each char.
my $is_for_ascii = 1;   # get_supported_code_pages() returns the ASCII
                        # character set first
foreach my $charset (get_supported_code_pages()) {
    my @a2n = @{get_a2n($charset)};
    my @out;
    my @utf_to_i8;

    if ($is_for_ascii) {
        $is_for_ascii = 0;
    }
    else {  # EBCDIC.  Calculate mapping from UTF-EBCDIC bytes to I8
        my $i8_to_utf_ref = get_I8_2_utf($charset);
        for my $i (0..255) {
            $utf_to_i8[$i8_to_utf_ref->[$i]] = $i;
        }
    }

    print $out_fh "\n" . get_conditional_compile_line_start($charset);
    for my $ord (0..255) {
        my $name;
        my $char = chr $ord;
        if ($char =~ /\p{PosixGraph}/) {
            my $quote = $char eq "'" ? '"' : "'";
            $name = $quote . chr($ord) . $quote;
        }
        elsif ($char =~ /\p{XPosixGraph}/) {
            use charnames();
            $name = charnames::viacode($ord);
            $name =~ s/LATIN CAPITAL LETTER //
                    or $name =~ s/LATIN SMALL LETTER (.*)/\L$1/
                    or $name =~ s/ SIGN\b//
                    or $name =~ s/EXCLAMATION MARK/'!'/
                    or $name =~ s/QUESTION MARK/'?'/
                    or $name =~ s/QUOTATION MARK/QUOTE/
                    or $name =~ s/ INDICATOR//;
            $name =~ s/\bWITH\b/\L$&/;
            $name =~ s/\bONE\b/1/;
            $name =~ s/\b(TWO|HALF)\b/2/;
            $name =~ s/\bTHREE\b/3/;
            $name =~ s/\b QUARTER S? \b/4/x;
            $name =~ s/VULGAR FRACTION (.) (.)/$1\/$2/;
            $name =~ s/\bTILDE\b/'~'/i
                    or $name =~ s/\bCIRCUMFLEX\b/'^'/i
                    or $name =~ s/\bSTROKE\b/'\/'/i
                    or $name =~ s/ ABOVE\b//i;
        }
        else {
            use Unicode::UCD qw(prop_invmap);
            my ($list_ref, $map_ref, $format)
                   = prop_invmap("_Perl_Name_Alias", '_perl_core_internal_ok');
            if ($format !~ /^s/) {
                use Carp;
                carp "Unexpected format '$format' for '_Perl_Name_Alias";
                last;
            }
            my $which = Unicode::UCD::search_invlist($list_ref, $ord);
            if (! defined $which) {
                use Carp;
                carp "No name found for code pont $ord";
            }
            else {
                my $map = $map_ref->[$which];
                if (! ref $map) {
                    $name = $map;
                }
                else {
                    # Just pick the first abbreviation if more than one
                    my @names = grep { $_ =~ /abbreviation/ } @$map;
                    $name = $names[0];
                }
                $name =~ s/:.*//;
            }
        }

        my $index = $a2n[$ord];
        my $i8;
        $i8 = $utf_to_i8[$index] if @utf_to_i8;

        $out[$index] = "/* ";
        $out[$index] .= sprintf "0x%02X ", $index if $ord != $index;
        $out[$index] .= sprintf "U+%02X ", $ord;
        $out[$index] .= sprintf "I8=%02X ", $i8 if defined $i8 && $i8 != $ord;
        $out[$index] .= "$name */ ";
        $out[$index] .= $bits[$ord];

        $out[$index] .= ",\n";
    }
    $out[-1] =~ s/,$//;     # No trailing comma in the final entry

    print $out_fh join "", @out;
    print $out_fh "\n" . get_conditional_compile_line_end();
}

read_only_bottom_close_and_rename($out_fh)
