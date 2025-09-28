package regcharclass_multi_char_folds;
use 5.015;
use strict;
use warnings;
use Unicode::UCD "prop_invmap";

# This returns an array of strings of the form
#   "\x{foo}\x{bar}\x{baz}"
# of the sequences of code points that are multi-character folds in the
# current Unicode version.  If the parameter is 1, all such folds are
# returned.  If the parameters is 0, only the ones containing exclusively
# Latin1 characters are returned.  In the latter case all combinations of
# Latin1 characters that can fold to the base one are returned.  Thus for
# 'ss', it would return in addition, 'Ss', 'sS', and 'SS'.  This is because
# this code is designed to help regcomp.c, and EXACTFish regnodes.  For
# non-UTF-8 patterns, the strings are not necessarily folded, so we need to
# check for the upper and lower case versions.  For UTF-8 patterns, the
# strings are folded, except in EXACTFL nodes) so we only need to worry about
# the fold version.  All folded-to characters in non-UTF-8 (Latin1) are
# members of fold-pairs, at least within Latin1, 'k', and 'K', for example.
# So there aren't complications with dealing with unfolded input.  That's not
# true of UTF-8 patterns, where things can get tricky.  Thus for EXACTFL nodes
# where things aren't all folded, code has to be written specially to handle
# this, instead of the macros here being extended to try to handle it.
#
# There are no non-ASCII Latin1 multi-char folds currently, and none likely to
# be ever added.  Thus the output is the same as if it were just asking for
# ASCII characters, not full Latin1.  Hence, it is suitable for generating
# things that match EXACTFAA.  It does check for and croak if there ever were
# to be an upper Latin1 range multi-character fold.
#
# This is designed for input to regen/regcharlass.pl.

sub gen_combinations ($;) {
    # Generate all combinations for the first parameter which is an array of
    # arrays.

    my ($fold_ref, $string, $i) = @_;
    $string = "" unless $string;
    $i = 0 unless $i;

    my @ret;

    # Look at each element in this level's array.
    if (ref $fold_ref->[$i]) {
    foreach my $j (0 .. @{$fold_ref->[$i]} - 1) {

        # Append its representation to what we have currently
        my $new_string = $fold_ref->[$i][$j] =~ /[[:print:]]/
                         ? ($string . chr $fold_ref->[$i][$j])
                         : sprintf "$string\\x{%X}", $fold_ref->[$i][$j];

        if ($i >=  @$fold_ref - 1) {    # Final level: just return it
            push @ret, "\"$new_string\"";
        }
        else {  # Generate the combinations for the next level with this one's
            push @ret, &gen_combinations($fold_ref, $new_string, $i + 1);
        }
    }
    }

    return @ret;
}

sub multi_char_folds ($$) {
    my $type = shift;  # 'u' for UTF-8; 'l' for latin1
    my $range = shift;  # 'a' for all; 'h' for starting 2 bytes; 'm' for ending 2
    die "[lu] only valid values for first parameter" if $type !~ /[lu]/;
    die "[aht3] only valid values for 2nd parameter" if $range !~ /[aht3]/;

    return () if pack("C*", split /\./, Unicode::UCD::UnicodeVersion()) lt v3.0.1;

    my ($cp_ref, $folds_ref, $format) = prop_invmap("Case_Folding");
    die "Could not find inversion map for Case_Folding" unless defined $format;
    die "Incorrect format '$format' for Case_Folding inversion map"
                                                        unless $format eq 'al';

    my %inverse_latin1_folds;
    for my $i (0 .. @$cp_ref - 1) {
        next if ref $folds_ref->[$i];   # multi-char fold
        next if $folds_ref->[$i] == 0;  # Not folded
        my $cp_base = $cp_ref->[$i];

        for my $j ($cp_base .. $cp_ref->[$i+1] - 1) {
            my $folded_base = $folds_ref->[$i];
            next if $folded_base > 255;         # only interested in Latin1
            push @{$inverse_latin1_folds{$folded_base + $j - $cp_base}}, $j;
        }
    }

    my @folds;
    my %output_folds;

    for my $i (0 .. @$folds_ref - 1) {
        next unless ref $folds_ref->[$i];   # Skip single-char folds

        # The code in regcomp.c currently assumes that no multi-char fold
        # folds to the upper Latin1 range.  It's not a big deal to add; we
        # just have to forbid such a fold in EXACTFL nodes, like we do already
        # for ascii chars in EXACTFA (and EXACTFL) nodes.  But I (khw) doubt
        # that there will ever be such a fold created by Unicode, so the code
        # isn't there to occupy space and time; instead there is this check.
        die sprintf("regcomp.c can't cope with a latin1 multi-char fold (found in the fold of 0x%X", $cp_ref->[$i]) if grep { $_ < 256 && chr($_) !~ /[[:ascii:]]/ } @{$folds_ref->[$i]};

        @folds = @{$folds_ref->[$i]};
        if ($range eq '3') {
            next if @folds < 3;
        }
        elsif ($range eq 'h') {
            pop @folds;
        }
        elsif ($range eq 't') {
            next if @folds < 3;
            shift @folds;
        }

        # Create a line that looks like "\x{foo}\x{bar}\x{baz}" of the code
        # points that make up the fold (use the actual character if
        # printable).
        my $fold = join "", map { chr $_ =~ /[[:print:]]/a
                                            ? chr $_
                                            : sprintf "\\x{%X}", $_
                                } @folds;
        $fold = "\"$fold\"";

        # Skip if something else already has this fold
        next if grep { $_ eq $fold } keys %output_folds;

        my $this_fold_ref = \@folds;
        for my $j (0 .. @$this_fold_ref - 1) {
            my $this_ord = $this_fold_ref->[$j];
            undef $this_fold_ref->[$j];
            
            # If the fold is to a Latin1-range cased letter, replace the entry
            # with an array which also includes everything that folds to it.
            if (exists $inverse_latin1_folds{$this_ord}) {
                push @{$this_fold_ref->[$j]},
                      ( $this_ord, @{$inverse_latin1_folds{$this_ord}} );
            }
            else {  # Otherwise, just itself. (gen_combinations() needs a ref)
                @{$this_fold_ref->[$j]} = ( $this_ord );
            }
        }

        # Then generate all combinations of upper/lower case of the fold.
        $output_folds{$_} = $cp_ref->[$i] for gen_combinations($this_fold_ref);
    }

    # \x17F is the small LONG S, which folds to 's'.  Both Capital and small
    # LATIN SHARP S fold to 'ss'.  Therefore, they should also match two 17F's
    # in a row under regex /i matching.  But under /iaa regex matching, all
    # three folds to 's' are prohibited, but the sharp S's should still match
    # two 17F's.  This prohibition causes our regular regex algorithm that
    # would ordinarily allow this match to fail.  This is the only instance in
    # all Unicode of this kind of issue.  By adding a special case here, we
    # can use the regular algorithm (with some other changes elsewhere as
    # well).
    #
    # It would be possible to re-write the above code to automatically detect
    # and handle this case, and any others that might eventually get added to
    # the Unicode standard, but I (khw) don't think it's worth it.  I believe
    # that it's extremely unlikely that more folds to ASCII characters are
    # going to be added, and if I'm wrong, fold_grind.t has the intelligence
    # to detect them, and test that they work, at which point another special
    # case could be added here if necessary.
    #
    # No combinations of this with 's' need be added, as any of these
    # containing 's' are prohibited under /iaa.
    $output_folds{"\"\x{17F}\x{17F}\""} = 0xDF if $type eq 'u' && $range eq 'a';

    return %output_folds;
}

1
