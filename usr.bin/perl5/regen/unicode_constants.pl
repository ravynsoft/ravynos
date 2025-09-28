use v5.16.0;
use strict;
use warnings;
no warnings 'experimental::regex_sets';
require './regen/regen_lib.pl';
require './regen/charset_translations.pl';
use Unicode::UCD qw(prop_invlist prop_invmap search_invlist);
use charnames qw(:loose);
binmode(STDERR, ":utf8");

# Set this to 1 temporarily to get on stderr the complete list of paired
# string delimiters this generates.  This list is suitable for plugging into a
# pod.
my $output_lists = 0;

# Set this to 1 temporarily to get on stderr the complete list of punctuation
# marks and symbols that look to be directional but we didn't include for some
# reason.
my $output_omitteds = 0;

my $out_fh = open_new('unicode_constants.h', '>',
        {style => '*', by => $0,
                      from => "Unicode data"});

print $out_fh <<END;

#ifndef PERL_UNICODE_CONSTANTS_H_   /* Guard against nested #includes */
#define PERL_UNICODE_CONSTANTS_H_   1

/* This file contains #defines for the version of Unicode being used and
 * various Unicode code points.  The values the code point macros expand to
 * are the native Unicode code point, or all or portions of the UTF-8 encoding
 * for the code point.  In the former case, the macro name has the suffix
 * "_NATIVE"; otherwise, the suffix "_UTF8".
 *
 * The macros that have the suffix "_UTF8" may have further suffixes, as
 * follows:
 *  "_FIRST_BYTE" if the value is just the first byte of the UTF-8
 *                representation; the value will be a numeric constant.
 *  "_TAIL"       if instead it represents all but the first byte.  This, and
 *                with no additional suffix are both string constants */

/*
=for apidoc_section \$unicode

=for apidoc AmnU|const char *|BOM_UTF8

This is a macro that evaluates to a string constant of the  UTF-8 bytes that
define the Unicode BYTE ORDER MARK (U+FEFF) for the platform that perl
is compiled on.  This allows code to use a mnemonic for this character that
works on both ASCII and EBCDIC platforms.
S<C<sizeof(BOM_UTF8) - 1>> can be used to get its length in
bytes.

=for apidoc AmnU|const char *|REPLACEMENT_CHARACTER_UTF8

This is a macro that evaluates to a string constant of the  UTF-8 bytes that
define the Unicode REPLACEMENT CHARACTER (U+FFFD) for the platform that perl
is compiled on.  This allows code to use a mnemonic for this character that
works on both ASCII and EBCDIC platforms.
S<C<sizeof(REPLACEMENT_CHARACTER_UTF8) - 1>> can be used to get its length in
bytes.

=cut
*/

END

sub backslash_x_form($$;$) {
    # Output the code point represented by the byte string $bytes as a
    # sequence of \x{} constants.  $bytes should be the UTF-8 for the code
    # point if the final parameter is absent or empty.  Otherwise it should be
    # the Latin1 code point itself.
    #
    # The output is translated into the character set '$charset'.

    my ($bytes, $charset, $non_utf8) = @_;
    if ($non_utf8) {
        die "Must be utf8 if above 255" if $bytes > 255;
        my $a2n = get_a2n($charset);
        return sprintf "\\x%02X", $a2n->[$bytes];
    }
    else {
        return join "", map { sprintf "\\x%02X", ord $_ }
                        split //, cp_2_utfbytes($bytes, $charset);
    }
}

# The most complicated thing this program does is generate paired string
# delimiters from the Unicode database.  Some of these come from the
# Unicode Bidirectional (bidi) algorithm.

# These all visually look like left and right delimiters
my @bidi_strong_lefts = (  'LESS-THAN',
                           'ELEMENT OF',
                           'PRECEDE',
                           'PRECEDES',
                           'SMALLER THAN',
                           'SUBSET',
                        );
my @bidi_strong_rights = ( 'GREATER-THAN',
                           'CONTAINS',
                           'SUCCEED',
                           'SUCCEEDS',
                           'LARGER THAN',
                           'SUPERSET',
                         );

# Create an array of hashes for these, so as to translate between them, and
# avoid recompiling patterns in the loop.
my @bidi_strong_directionals;
for (my $i = 0; $i < @bidi_strong_lefts; $i++) {
    push @bidi_strong_directionals,
                {
                   LHS => $bidi_strong_lefts[$i],
                   RHS => $bidi_strong_rights[$i],
                   L_pattern => qr/\b$bidi_strong_lefts[$i]\b/,
                   R_pattern => qr/\b$bidi_strong_rights[$i]\b/,
                };
}

my @ok_bidi_symbols = (
                        'TACK',
                        'TURNSTILE',
                      );
my $ok_bidi_symbols_re = join '|', @ok_bidi_symbols;
$ok_bidi_symbols_re = qr/\b($ok_bidi_symbols_re)\b/n;


# Many characters have mirrors that Unicode hasn't included in their Bidi
# algorithm.  This program uses their names to find them.  The next few
# definitions are towards that end.

# Most horizontal directionality is based on LEFT vs RIGHT.  But it's
# complicated:
#   1)  a barb on one or the other side of a harpoon doesn't indicate
#       directionality of the character.  (A HARPOON is the word Unicode uses
#       to indicate an arrow with a one-sided tip.)
my $no_barb_re =  qr/(*nlb:BARB )/;

#   2)  RIGHT-SHADED doesn't signify anything about direction of the character
#   itself.  These are the suffixes Unicode uses to indicate this.  /aa is
#   needed because the wildcard names feature currently requires it for names.
my $shaded_re = qr/ [- ] (SHADED | SHADOWED) /naax;

#   3a) there are a few anomalies caught here.  'LEFT LUGGAGE' would have been
#       better named UNCLAIMED, and doesn't indicate directionality.
my $real_LEFT_re =  qr/ \b $no_barb_re LEFT  (*nla: $shaded_re)
                                             (*nla: [ ] LUGGAGE \b)
                    /nx;
#   3b) And in most cases,a RIGHT TRIANGLE also doesn't refer to
#       directionality, but indicates it contains a 90 degree angle.
my $real_RIGHT_re = qr/ \b $no_barb_re RIGHT (*nla: $shaded_re)
                                             (*nla: [ ] (TRI)? ANGLE \b)
                    /nx;
#       More items could be added to these as needed

#   4)  something that is pointing R goes on the left, so is different than
#       the character on the R.  For example,  a RIGHT BRACKET would be
#       different from a RIGHT-FACING bracket.  These patterns capture the
#       typical ways that Unicode character names indicate the latter meaning
#       as a suffix to RIGHT or LEFT
my $pointing_suffix_re = qr/ (       WARDS     # e.g., RIGHTWARDS
                              | [ ]  ARROW     # A R arrow points to the R
                              | [ -] FACING
                              | [ -] POINTING
                              | [ ]  PENCIL    # Implies a direction of its
                                               # point
                         ) \b /nx;
# And correspondingly for a prefix for LEFT RIGHT
my $pointing_prefix_re = qr/ \b (  # e.g. UP RIGHT implies a direction
                                   UP   ( [ ] AND)?
                                 | DOWN ( [ ] AND)?
                                 | CONVERGING
                                 | POINTING [ ] (DIRECTLY)?
                                 | TO [ ] THE
                               )
                             [ ]
                           /nx;

my @other_directionals =
    {
        LHS => 'LEFT',
        RHS => 'RIGHT',
        L_pattern =>
                  # Something goes on the left if it contains LEFT and doesn't
                  # point left, or it contains RIGHT and does point right.
            qr/   \b (*nlb: $pointing_prefix_re) $real_LEFT_re
                                                    (*nla: $pointing_suffix_re)
                | \b (*plb: $pointing_prefix_re)  $real_RIGHT_re \b
                | \b $real_RIGHT_re (*pla: $pointing_suffix_re)
            /nx,
        R_pattern =>
            qr/   \b (*nlb: $pointing_prefix_re) $real_RIGHT_re
                                                    (*nla: $pointing_suffix_re)
                | \b (*plb: $pointing_prefix_re)  $real_LEFT_re \b
                | \b $real_LEFT_re (*pla: $pointing_suffix_re)
            /nx,
    };

# Some horizontal directionality is based on EAST vs WEST.  These words are
# almost always used by Unicode to indicate the direction pointing to, without
# the general consistency in phrasing in L/R above.  There are a handful of
# possible exceptions, with only WEST WIND ever at all possibly an issue
push @other_directionals,
    {
        LHS => 'EAST',
        RHS => 'WEST',
        L_pattern => qr/ \b (  EAST (*nla: [ ] WIND)
                             | WEST (*pla: [ ] WIND)) \b /x,
        R_pattern => qr/ \b (  WEST (*nla: [ ] WIND)
                             | EAST (*pla: [ ] WIND)) \b /x,
    };

# The final way the Unicode signals mirroring is by using the words REVERSE or
# REVERSED;
my $reverse_re = qr/ \b REVERSE D? [- ] /x;

# Create a mapping from each direction to its opposite one
my %opposite_of;
foreach my $directional (@bidi_strong_directionals, @other_directionals) {
    $opposite_of{$directional->{LHS}} = $directional->{RHS};
    $opposite_of{$directional->{RHS}} = $directional->{LHS};
}

# Join the two types of each direction as alternatives
my $L_re = join "|", map { $_->{L_pattern} } @bidi_strong_directionals,
                                             @other_directionals;
my $R_re = join "|", map { $_->{R_pattern} } @bidi_strong_directionals,
                                             @other_directionals;
# And anything containing directionality will be either one of these two
my $directional_re = join "|", $L_re, $R_re;

# Now compile the strings that result from above
$L_re = qr/$L_re/;
$R_re = qr/$R_re/;
$directional_re = qr/($directional_re)/;    # Make sure to capture $1

my @included_symbols = (
                         0x2326,   0x232B,      # ERASE
                         0x23E9 .. 0x23EA,      # DOUBLE TRIANGLE
                         0x23ED .. 0x23EE,      # DOUBLE TRIANGLE with BAR
                         0x269E .. 0x269F,      # THREE LINES CONVERGING
                         0x1D102 .. 0x1D103,    # MUSIC STAVES
                         0x1D106 .. 0x1D107,    # MUSIC STAVES
                         0x1F57B,               # TELEPHONE RECEIVER
                         0x1F57D,               # TELEPHONE RECEIVER
                         0x1F508 .. 0x1F50A,    # LOUD SPEAKER
                         0x1F568 .. 0x1F56A,    # LOUD SPEAKER
                         0x1F5E6 .. 0x1F5E7,    # THREE RAYS
                       );
my %included_symbols;
$included_symbols{$_} = 1 for @included_symbols;

sub format_pairs_line($;$) {
    my ($from, $to) = @_;

    # Format a line containing a character singleton or pair in preparation
    # for output, suitable for pod.

    my $lhs_name = charnames::viacode($from);
    my $lhs_hex = sprintf "%04X", $from;
    my $rhs_name;
    my $rhs_hex;
    my $name = $lhs_name;

    my $hanging_indent = 26;

    # Treat a trivial pair as a singleton
    undef $to if defined $to && $to == $from;

    if (defined $to) {
        my $rhs_name = charnames::viacode($to);
        $rhs_hex = sprintf "%04X", $to;

        # Most of the names differ only in LEFT vs RIGHT; some in
        # LESS-THAN vs GREATER-THAN.  It takes less space, and is easier to
        # understand if they are displayed combined.
        if ($name =~ s/$directional_re/$opposite_of{$1}/gr eq $rhs_name) {
            $name =~ s,$directional_re,$1/$opposite_of{$1},g;
        }
        else {  # Otherwise, display them sequentially
            $name .= ",  " . $rhs_name;
        }
    }

    # Handle double-width characters, based on the East Asian Width property.
    # Add an extra space to non-wide ones so things stay vertically aligned.
    my $extra = 0;
    my $output_line = " "   # Indent in case output being used for verbatim
                            # pod
                    . chr $from;
    if (chr($from) =~ /[\p{EA=W}\p{EA=F}]/) {
        $extra++;       # The length() will be shorter than the displayed
                        # width
    }
    else {
        $output_line .= " ";
    }
    if (defined $to) {
        $output_line .= " " . chr $to;
        if (chr($to) =~ /[\p{EA=W}\p{EA=F}]/) {
            $extra++;
        }
        else {
            $output_line .= " ";
        }
    }
    else {
        $output_line .= "   ";
    }

    $output_line .= "   U+$lhs_hex";
    $output_line .= ", U+$rhs_hex" if defined $to;;
    my $cur_len = $extra + length $output_line;
    $output_line .= " " x ($hanging_indent - $cur_len);

    my $max_len = 74;   # Pod formatter will indent 4 spaces
    $cur_len = length $output_line;

    if ($cur_len + length $name <= $max_len) {
        $output_line .= $name;  # It will fit
    }
    else {  # It won't fit.  Append a segment that is unbreakable until would
            # exceed the available width; then start on a new line
            # Doesn't handle the case where the whole segment doesn't fit;
            # this just doesn't come up with the input data.
        while ($name =~ / ( .+? ) \b{lb} /xg) {
            my $segment = $1;
            my $added_length = length $segment;
            if ($cur_len + $added_length > $max_len) {
                $output_line =~ s/ +$//;
                $output_line .= "\n" . " " x $hanging_indent;
                $cur_len = $hanging_indent;
            }

            $output_line .= $segment;
            $cur_len += $added_length;
        }
    }

    return $output_line . "\n";
}

my $version = Unicode::UCD::UnicodeVersion();
my ($major, $dot, $dotdot) = $version =~ / (.*?) \. (.*?) (?: \. (.*) )? $ /x;
$dotdot = 0 unless defined $dotdot;

print $out_fh <<END;
#define UNICODE_MAJOR_VERSION   $major
#define UNICODE_DOT_VERSION     $dot
#define UNICODE_DOT_DOT_VERSION $dotdot

END

# Gather the characters in Unicode that have left/right symmetry suitable for
# paired string delimiters
my %paireds;

# So don't have to grep an array to determine if have already dealt with the
# characters that are the keys
my %inverted_paireds;

# This property is the universe of all characters in Unicode which
# are of some import to the Bidirectional Algorithm, and for which there is
# another Unicode character that is a mirror of it.
my ($bmg_invlist, $bmg_invmap, $format, $bmg_default) =
                                            prop_invmap("Bidi_Mirroring_Glyph");

# Keep track of the characters we don't use, and why not.
my %discards;
my $non_directional = 'No perceived horizontal direction';
my $not_considered_directional_because = "Not considered directional because";
my $trailing_up_down = 'Vertical direction after all L/R direction';
my $unpaired = "Didn't find a mirror";
my $illegal = "Mirror illegal";
my $no_encoded_mate = "Mirrored, but Unicode has no encoded mirror";
my $bidirectional = "Bidirectional";

my %unused_bidi_pairs;
my %inverted_unused_bidi_pairs;
my %unused_pairs;   #
my %inverted_unused_pairs;

# Could be more explicit about allowing, e.g. ARROWS, ARROWHEAD, but this
# suffices
my $arrow_like_re = qr/\b(ARROW|HARPOON)/;

# Go through the Unicode Punctuation and Symbol characters looking for ones
# that have mirrors, suitable for being string delimiters.  Some of these are
# easily derivable from Unicode properties dealing with the bidirectional
# algorithm.  But the purpose of that algorithm isn't the same as ours, and
# excludes many suitable ones.  In particular, no arrows are included in it.
# To find suitable ones, we also look at character names to see if there is a
# character with that name, but the horizontal direction reversed.  That will
# almost certainly be a mirror.
foreach my $list (qw(Punctuation Symbol)) {
    my @invlist = prop_invlist($list);
    die "Empty list $list" unless @invlist;

    my $is_Symbol = $list eq 'Symbol';

    # Convert from an inversion list to an array containing everything that
    # matches.  (This uses the recipe given in Unicode::UCD.)
    my @full_list;
    for (my $i = 0; $i < @invlist; $i += 2) {
       my $upper = ($i + 1) < @invlist
                   ? $invlist[$i+1] - 1      # In range
                   : $Unicode::UCD::MAX_CP;  # To infinity.
       for my $j ($invlist[$i] .. $upper) {
           push @full_list, $j;
       }
    }

  CODE_POINT:
    foreach my $code_point (@full_list) {
        #print STDERR __FILE__, ": ", __LINE__, ": ", sprintf("%04x ", $code_point), charnames::viacode($code_point), "\n";
        my $chr = chr $code_point;

        # Don't reexamine something we've already determined.  This happens
        # when its mate was earlier processed and found this one.
        foreach my $hash_ref (\%paireds,           \%inverted_paireds,
                              \%unused_bidi_pairs, \%inverted_unused_bidi_pairs,
                              \%unused_pairs,      \%inverted_unused_pairs)
        {
            next CODE_POINT if exists $hash_ref->{$code_point}
        }

        my $name = charnames::viacode($code_point);
        my $original_had_REVERSE;
        my $mirror;
        my $mirror_code_point;

        # If Unicode considers this to have a mirror, we don't have to go
        # looking
        if ($chr =~ /\p{Bidi_Mirrored}/) {
            my $i = search_invlist($bmg_invlist, $code_point);
            $mirror_code_point = $bmg_invmap->[$i];
            if ( $mirror_code_point eq $bmg_default) {
                $discards{$code_point} = { reason => $no_encoded_mate,
                                           mirror => undef
                                         };
                next;
            }

            # Certain Unicode properties classify some mirrored characters as
            # opening (left) vs closing (right).  Skip the closing ones this
            # iteration; they will be handled later when the opening mate
            # comes along.
            if ($chr =~ /(?[  \p{BPT=Close}
                            | \p{Gc=Close_Punctuation}
                         ])/)
            {
                next;   # Get this when its opening mirror comes up.
            }
            elsif ($chr =~ /(?[  \p{BPT=Open}
                               | \p{Gc=Open_Punctuation}
                               | \p{Gc=Initial_Punctuation}
                               | \p{Gc=Final_Punctuation}
                            ])/)
            {
                # Here, it's a left delimiter.  (The ones in Final Punctuation
                # can be opening ones in some languages.)
                $paireds{$code_point} = $mirror_code_point;
                $inverted_paireds{$mirror_code_point} = $code_point;

                # If the delimiter can be used on either side, add its
                # complement
                if ($chr =~ /(?[  \p{Gc=Initial_Punctuation}
                                | \p{Gc=Final_Punctuation}
                             ])/)
                {
                    $paireds{$mirror_code_point} = $code_point;
                    $inverted_paireds{$code_point} = $mirror_code_point;
                }

                next;
            }

            # Unicode doesn't consider '< >' to be brackets, but Perl does.  There are
            # lots of variants of these in Unicode; easiest to accept all of
            # them that aren't bidirectional (which would be visually
            # confusing).
            for (my $i = 0; $i < @bidi_strong_directionals; $i++) {
                my $hash_ref = $bidi_strong_directionals[$i];

                next if $name !~ $hash_ref->{L_pattern};

                if ($name =~ $hash_ref->{R_pattern}) {
                    $discards{$code_point} = { reason => $bidirectional,
                                               mirror => $mirror_code_point
                                             };
                    next CODE_POINT;
                }

                $paireds{$code_point} = $mirror_code_point;
                $inverted_paireds{$mirror_code_point} = $code_point;
                $original_had_REVERSE = $name =~ /$reverse_re/;
                next CODE_POINT;
            }

            # The other paired symbols are more iffy as being desirable paired
            # delimiters; we let the code below decide what to do with them.
            $mirror = charnames::viacode($mirror_code_point);
        }
        else { # Here is not involved with the bidirectional algorithm.

            # Get the mirror (if any) from reversing the directions in the
            # name, and looking that up
            $mirror = $name;
            $mirror =~ s/$directional_re/$opposite_of{$1}/g;
            $original_had_REVERSE = $mirror =~ s/$reverse_re//g;
            $mirror_code_point = charnames::vianame($mirror);
        }

        # Letter-like symbols don't really stand on their own and don't look
        # like traditional delimiters.
        if ($chr =~ /\p{Sk}/) {
            $discards{$code_point}
                = { reason => "Letter-like symbols are not eligible",
                    mirror => $mirror_code_point
                  };
            next CODE_POINT;
        }

        # Certain names are always treated as non directional.
        if ($name =~ m{ \b (  WITH [ ] (?:LEFT|RIGHT) [ ] HALF [ ] BLACK
                            | BLOCK
                            | BOX [ ] DRAWINGS
                            | CIRCLE [ ] WITH
                            | EXTENSION
                            | (?: UPPER | LOWER ) [ ] HOOK

                              # The VERTICAL marks these as not actually
                              # L/R mirrored.
                            | PRESENTATION [ ] FORM [ ] FOR [ ] VERTICAL
                            | QUADRANT
                            | SHADE
                            | SQUARE [ ] WITH
                        ) \b }x)
        {
            $discards{$code_point}
                = { reason => "$not_considered_directional_because name"
                            . " contains '$1'",
                    mirror => $mirror_code_point
                  };
            next CODE_POINT;
        }

        # If these are equal, it means the original had no horizontal
        # directioning
        if ($name eq $mirror) {
            $discards{$code_point} = { reason => $non_directional,
                                       mirror => undef
                                     };
            next CODE_POINT;
        }

        # If the name has both left and right directions, it is bidirectional,
        # so not suited to be a paired delimiter.
        if ($name =~ $L_re && $name =~ $R_re) {
            $discards{$code_point} = { reason => $bidirectional,
                                       mirror => $mirror_code_point
                                     };
            next CODE_POINT;
        }

        # If no mate was found, it could be that it's like the case of
        # SPEAKER vs RIGHT SPEAKER (which probably means the mirror was added
        # in a later version than the original.  Check by removing all
        # directionality and trying to see if there is a character with that
        # name.
        if (! defined $mirror_code_point) {
            $mirror =~ s/$directional_re //;
            $mirror_code_point = charnames::vianame($mirror);
            if (! defined $mirror_code_point) {

                # Still no mate.
                $discards{$code_point} = { reason => $unpaired,
                                           mirror => undef
                                         };
                next;
            }
        }

        if ($code_point == $mirror_code_point) {
            $discards{$code_point} =
                { reason => "$unpaired - Single character, multiple"
                          . " names; Unicode name correction",
                  mirror => $mirror_code_point
                };
            next;
        }

        if ($is_Symbol) {

            # Skip if the direction is followed by a vertical motion
            # (which defeats the left-right directionality).
            if (        $name =~ / ^ .* $no_barb_re
                                   \b (UP|DOWN|NORTH|SOUTH) /gx
                and not $name =~ /$directional_re/g)
            {
                $discards{$code_point} = { reason => $trailing_up_down,
                                           mirror => $mirror_code_point
                                         };
                next;
            }
        }

        # There are a few characters like REVERSED SEMICOLON that are mirrors,
        # but have always commonly been used unmirrored.  There is also the
        # PILCROW SIGN and its mirror which might be considered to be
        # legitimate mirrors, but maybe not.  Additionally the current
        # algorithm for finding the mirror depends on each member of a pair
        # being respresented by the same number of bytes as its mate.  By
        # skipping these, we solve both problems
        if ($code_point < 256 != $mirror_code_point < 256) {
            $discards{$code_point} = { reason => $illegal,
                                        mirror => $mirror_code_point
                                     };
            next;
        }

        # And '/' and '\' are mirrors that we don't accept
        if (   $name =~ /SOLIDUS/
            &&    $name   =~ s/REVERSE SOLIDUS/SOLIDUS/r
               eq $mirror =~ s/REVERSE SOLIDUS/SOLIDUS/r)
        {
            $discards{$code_point} = { reason => $illegal,
                                        mirror => $mirror_code_point
                                     };
            next;
        }

        # We enter the pair with the original code point on the left; if it
        # should instead be on the R, swap.  Most Symbols that contain the
        # word REVERSE go on the rhs, except those whose names explicitly
        # indicate lhs.  FINAL in the name indicates stays on the rhs.
        if ($name =~ $R_re || (   $original_had_REVERSE
                               && $is_Symbol
                               && $name !~ $L_re
                               && $name !~ /\bFINAL\b/
                               ))
        {
            my $temp = $code_point;
            $code_point = $mirror_code_point;
            $mirror_code_point = $temp;
        }

        # Only a few symbols are currently used, determined by inspection, but
        # all the (few) remaining paired punctuations.
        if ( ! $is_Symbol
            || defined $included_symbols{$code_point}
            || (    $chr =~ /\p{BidiMirrored}/
                && (   $name   =~ $ok_bidi_symbols_re
                    || $mirror =~ $ok_bidi_symbols_re))
            || $name =~ /\bINDEX\b/         # index FINGER pointing

                   # Also accept most arrows that don't have N/S in their
                   # names.  (Those are almost all currently pointing at an
                   # angle, like SW anyway.)
            || (   $name !~ /\bNORTH|SOUTH\b/
                && $name =~ $arrow_like_re

                    # Arguably bi-directional
                && $name !~ /U-SHAPED/)
        ) {
            $paireds{$code_point} = $mirror_code_point;
            $inverted_paireds{$mirror_code_point} = $code_point;

            # Again, accept either one at either end for these ambiguous
            # punctuation delimiters
            if ($chr =~ /[\p{PI}\p{PF}]/x) {
                $paireds{$mirror_code_point} = $code_point;
                $inverted_paireds{$code_point} = $mirror_code_point;
            }
        }
        elsif (     $chr =~ /\p{BidiMirrored}/
               && ! exists  $inverted_unused_bidi_pairs{$code_point}
               && ! defined $inverted_unused_bidi_pairs{$code_point})
            {
                $unused_bidi_pairs{$code_point} = $mirror_code_point;
                $inverted_unused_bidi_pairs{$mirror_code_point} = $code_point;
            }
        elsif (   ! exists  $inverted_unused_pairs{$code_point}
                && ! defined $inverted_unused_pairs{$code_point})
        {   # A pair that we don't currently accept
            $unused_pairs{$code_point} = $mirror_code_point;
            $inverted_unused_pairs{$mirror_code_point} = $code_point;
        }
    }   # End of loop through code points
}   # End of loop through properties

# The rest of the data are at __DATA__  in this file.

my @data = <DATA>;

foreach my $charset (get_supported_code_pages()) {
    print $out_fh "\n" . get_conditional_compile_line_start($charset);

    my @a2n = @{get_a2n($charset)};

    for ( @data ) {
        chomp;

        # Convert any '#' comments to /* ... */; empty lines and comments are
        # output as blank lines
        if ($_ =~ m/ ^ \s* (?: \# ( .* ) )? $ /x) {
            my $comment_body = $1 // "";
            if ($comment_body ne "") {
                print $out_fh "/* $comment_body */\n";
            }
            else {
                print $out_fh "\n";
            }
            next;
        }

        unless ($_ =~ m/ ^ ( [^\ ]* )           # Name or code point token
                        (?: [\ ]+ ( [^ ]* ) )?  # optional flag
                        (?: [\ ]+ ( .* ) )?  # name if unnamed; flag is required
                    /x)
        {
            die "Unexpected syntax at line $.: $_\n";
        }

        my $name_or_cp = $1;
        my $flag = $2;
        my $desired_name = $3;

        my $name;
        my $cp;
        my $U_cp;   # code point in Unicode (not-native) terms

        if ($name_or_cp =~ /^U\+(.*)/) {
            $U_cp = hex $1;
            $name = charnames::viacode($name_or_cp);
            if (! defined $name) {
                next if $flag =~ /skip_if_undef/;
                die "Unknown code point '$name_or_cp' at line $.: $_\n" unless $desired_name;
                $name = "";
            }
        }
        else {
            $name = $name_or_cp;
            die "Unknown name '$name' at line $.: $_\n" unless defined $name;
            $U_cp = charnames::vianame($name =~ s/_/ /gr);
        }

        $cp = ($U_cp < 256)
            ? $a2n[$U_cp]
            : $U_cp;

        $name = $desired_name if $name eq "" && $desired_name;
        $name =~ s/[- ]/_/g;   # The macro name can have no blanks nor dashes

        my $str;
        my $suffix;
        if (defined $flag && $flag eq 'native') {
            die "Are you sure you want to run this on an above-Latin1 code point?" if $cp > 0xff;
            $suffix = '_NATIVE';
            $str = sprintf "0x%02X", $cp;        # Is a numeric constant
        }
        else {
            $str = backslash_x_form($U_cp, $charset);

            $suffix = '_UTF8';
            if (! defined $flag || $flag =~ /^ string (_skip_if_undef)? $/x) {
                $str = "\"$str\"";  # Will be a string constant
            } elsif ($flag eq 'tail') {
                    $str =~ s/\\x..//;  # Remove the first byte
                    $suffix .= '_TAIL';
                    $str = "\"$str\"";  # Will be a string constant
            }
            elsif ($flag eq 'first') {
                $str =~ s/ \\x ( .. ) .* /$1/x; # Get the two nibbles of the 1st byte
                $suffix .= '_FIRST_BYTE';
                $str = "0x$str";        # Is a numeric constant
            }
            else {
                die "Unknown flag at line $.: $_\n";
            }
        }
        printf $out_fh "#   define %s%s  %s    /* U+%04X */\n", $name, $suffix, $str, $U_cp;
    }

    # Now output the strings of opening/closing delimiters.  The Unicode
    # values were earlier entered into %paireds
    my $utf8_opening = "";
    my $utf8_closing = "";
    my $non_utf8_opening = "";
    my $non_utf8_closing = "";
    my $deprecated_if_not_mirrored = "";
    my $non_utf8_deprecated_if_not_mirrored = "";

    for my $from (sort { $a <=> $b } keys %paireds) {
        my $to = $paireds{$from};
        my $utf8_from_backslashed = backslash_x_form($from, $charset);
        my $utf8_to_backslashed   = backslash_x_form($to, $charset);
        my $non_utf8_from_backslashed;
        my $non_utf8_to_backslashed;

        $utf8_opening .= $utf8_from_backslashed;
        $utf8_closing .= $utf8_to_backslashed;

        if ($from < 256) {
            $non_utf8_from_backslashed =
                                  backslash_x_form($from, $charset, 'not_utf8');
            $non_utf8_to_backslashed =
                                  backslash_x_form($to, $charset, 'not_utf8');

            $non_utf8_opening .= $non_utf8_from_backslashed;
            $non_utf8_closing .= $non_utf8_to_backslashed;
        }

        # Only the ASCII range paired delimiters have traditionally been
        # accepted.  Until the feature is considered standard, the non-ASCII
        # opening ones must be deprecated when the feature isn't in effect, so
        # as to warn about behavior that is planned to change.
        if ($from > 127) {
            $deprecated_if_not_mirrored .= $utf8_from_backslashed;
            $non_utf8_deprecated_if_not_mirrored .=
                                    $non_utf8_from_backslashed if $from < 256;

            # We deprecate using any of these strongly directional characters
            # at either end of the string, in part so we could allow them to
            # be reversed.
            $deprecated_if_not_mirrored .= $utf8_to_backslashed
                                       if index ($deprecated_if_not_mirrored,
                                                 $utf8_to_backslashed) < 0;
        }

        # The implementing code in toke.c assumes that the byte length of each
        # opening delimiter is the same as its mirrored closing one.  This
        # makes sure of that by checking upon each iteration of the loop.
        if (length $utf8_opening != length $utf8_closing) {
            die "Byte length of representation of '"
              .  charnames::viacode($from)
              . " differs from its mapping '"
              .  charnames::viacode($to)
              .  "'";
        }

        print STDERR format_pairs_line($from, $to) if $output_lists;
    }
    $output_lists = 0;  # Only output in first iteration

    print $out_fh <<~"EOT";

        #   ifdef PERL_IN_TOKE_C
               /* Paired characters for quote-like operators, in UTF-8 */
        #      define EXTRA_OPENING_UTF8_BRACKETS "$utf8_opening"
        #      define EXTRA_CLOSING_UTF8_BRACKETS "$utf8_closing"

               /* And not in UTF-8 */
        #      define EXTRA_OPENING_NON_UTF8_BRACKETS "$non_utf8_opening"
        #      define EXTRA_CLOSING_NON_UTF8_BRACKETS "$non_utf8_closing"

               /* And what's deprecated */
        #      define DEPRECATED_OPENING_UTF8_BRACKETS "$deprecated_if_not_mirrored"
        #      define DEPRECATED_OPENING_NON_UTF8_BRACKETS "$non_utf8_deprecated_if_not_mirrored"
        #   endif
        EOT

    my $max_PRINT_A = 0;
    for my $i (0x20 .. 0x7E) {
        $max_PRINT_A = $a2n[$i] if $a2n[$i] > $max_PRINT_A;
    }
    $max_PRINT_A = sprintf "0x%02X", $max_PRINT_A;
    print $out_fh <<"EOT";

#   ifdef PERL_IN_REGCOMP_ANY
#     define MAX_PRINT_A  $max_PRINT_A   /* The max code point that isPRINT_A */
#   endif
EOT

    print $out_fh get_conditional_compile_line_end();

}

if ($output_omitteds) {
    # We haven't bothered to delete things that later became used.
    foreach my $which (\%paireds,
                       \%unused_bidi_pairs,
                       \%unused_pairs)
    {
        foreach my $lhs (keys $which->%*) {
            delete $discards{$lhs};
            delete $discards{$which->{$lhs}};
        }
    }

    print STDERR "\nMirrored says Unicode, but not currently used as paired string delimiters\n";
    foreach my $from (sort { $a <=> $b } keys %unused_bidi_pairs) {
        print STDERR format_pairs_line($from, $unused_bidi_pairs{$from});
    }

    print STDERR "\nMirror found by name, but not currently used as paired string delimiters\n";
    foreach my $from (sort { $a <=> $b } keys %unused_pairs) {
        print STDERR format_pairs_line($from, $unused_pairs{$from});
    }

    # Invert %discards so that all the code points for a given reason are
    # keyed by that reason.
    my %inverted_discards;
    foreach my $code_point (sort { $a <=> $b } keys %discards) {
        my $type = $discards{$code_point}{reason};
        push $inverted_discards{$type}->@*, [ $code_point,
                                              $discards{$code_point}{mirror}
                                            ];
    }

    # Then output each list
    foreach my $type (sort keys %inverted_discards) {
        print STDERR "\n$type\n" if $type ne "";
        foreach my $ref ($inverted_discards{$type}->@*) {
            print STDERR format_pairs_line($ref->[0], $ref->[1]);
        }
    }
}

my $count = 0;
my @other_invlist = prop_invlist("Other");
for (my $i = 0; $i < @other_invlist; $i += 2) {
    $count += ((defined $other_invlist[$i+1])
              ? $other_invlist[$i+1]
              : 0x110000)
              - $other_invlist[$i];
}
$count = 0x110000 - $count;
print $out_fh <<~"EOT";

    /* The number of code points not matching \\pC */
    #ifdef PERL_IN_REGCOMP_ANY
    #  define NON_OTHER_COUNT  $count
    #endif
    EOT

# If this release has both the CWCM and CWCF properties, find the highest code
# point which changes under any case change.  We can use this to short-circuit
# code
my @cwcm = prop_invlist('CWCM');
if (@cwcm) {
    my @cwcf = prop_invlist('CWCF');
    if (@cwcf) {
        my $max = ($cwcm[-1] < $cwcf[-1])
                  ? $cwcf[-1]
                  : $cwcm[-1];
        $max = sprintf "0x%X", $max - 1;
        print $out_fh <<~"EOS";

            /* The highest code point that has any type of case change */
            #ifdef PERL_IN_UTF8_C
            #  define HIGHEST_CASE_CHANGING_CP  $max
            #endif
            EOS
    }
}

print $out_fh "\n#endif /* PERL_UNICODE_CONSTANTS_H_ */\n";

read_only_bottom_close_and_rename($out_fh);

# DATA FORMAT
#
# Note that any apidoc comments you want in the file need to be added to one
# of the prints above
#
# A blank line is output as-is.
# Comments (lines whose first non-blank is a '#') are converted to C-style,
# though empty comments are converted to blank lines.  Otherwise, each line
# represents one #define, and begins with either a Unicode character name with
# the blanks and dashes in it squeezed out or replaced by underscores; or it
# may be a hexadecimal Unicode code point of the form U+xxxx.  In the latter
# case, the name will be looked-up to use as the name of the macro.  In either
# case, the macro name will have suffixes as listed above, and all blanks and
# dashes will be replaced by underscores.
#
# Each line may optionally have one of the following flags on it, separated by
# white space from the initial token.
#   string  indicates that the output is to be of the string form
#           described in the comments above that are placed in the file.
#   string_skip_ifundef  is the same as 'string', but instead of dying if the
#           code point doesn't exist, the line is just skipped: no output is
#           generated for it
#   first   indicates that the output is to be of the FIRST_BYTE form.
#   tail    indicates that the output is of the _TAIL form.
#   native  indicates that the output is the code point, converted to the
#           platform's native character set if applicable
#
# If the code point has no official name, the desired name may be appended
# after the flag, which will be ignored if there is an official name.
#
# This program is used to make it convenient to create compile time constants
# of UTF-8, and to generate proper EBCDIC as well as ASCII without manually
# having to figure things out.

__DATA__
U+017F string

U+0300 string
U+0307 string

U+1E9E string_skip_if_undef

U+FB05 string
U+FB06 string
U+0130 string
U+0131 string

U+2010 string
BOM first
BOM tail

BOM string

U+FFFD string

U+10FFFF string MAX_UNICODE

NBSP native
NBSP string

DEL native
CR  native
LF  native
VT  native
ESC native
U+00DF native
U+00DF string
U+00E5 native
U+00C5 native
U+00FF native
U+00B5 native
U+00B5 string
