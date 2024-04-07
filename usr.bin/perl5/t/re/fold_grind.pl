# Grind out a lot of combinatoric tests for folding.
# It uses various charset modifiers, passed in via $::TEST_CHUNK.  The caller
# will also have set the locale to use if /l is the modifier.
#   L is a pseudo-modifier that indicates to use the modifier /l instead, and
#     the locale set by the caller is known to be UTF-8,
#   T is a pseudo-modifier that indicates to use the pseudo modifier /L
#     instead, and the locale set by the caller is known to be Turkic UTF-8,

binmode STDOUT, ":utf8";

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config; import Config;
    skip_all_if_miniperl("no dynamic loading on miniperl, no Encode nor POSIX");
    if ($^O eq 'dec_osf') {
      skip_all("$^O cannot handle this test");
    }

    watchdog(5 * 60);
    require './loc_tools.pl';
}

use charnames ":full";

my $DEBUG = 0;  # Outputs extra information for debugging this .t

use strict;
use warnings;
no warnings 'locale';   # Plenty of these would otherwise get generated
use Encode;
use POSIX;

my $charset = $::TEST_CHUNK;
my $use_turkic_rules = 0;

if ($charset eq 'T') {
    $charset = 'L';
    $use_turkic_rules = 1;
}

my $has_LC_CTYPE = is_category_valid('LC_CTYPE');

# Special-cased characters in the .c's that we want to make sure get tested.
my %be_sure_to_test = (
        chr utf8::unicode_to_native(0xDF) => 1, # LATIN_SMALL_LETTER_SHARP_S

        # This is included because the uppercase occupies more bytes, but the
        # first two bytes of their representations differ only in one bit,
        # that could lead the code looking for shortcuts astray; you can't do
        # certain shortcuts if the lengths differ
        "\x{29E}" => 1, # LATIN SMALL LETTER TURNED K

        "\x{390}" => 1, # GREEK_SMALL_LETTER_IOTA_WITH_DIALYTIKA_AND_TONOS
        "\x{3B0}" => 1, # GREEK_SMALL_LETTER_UPSILON_WITH_DIALYTIKA_AND_TONOS

        # This is included because the uppercase and lowercase differ by only
        # a single bit and it is in the first of the two byte representations.
        # This showed that a previous way was erroneous of calculating if
        # initial substrings were closely-related bit-wise.
        "\x{3CC}" => 1, # GREEK SMALL LETTER OMICRON WITH TONOS

        "\x{1E9E}" => 1, # LATIN_CAPITAL_LETTER_SHARP_S
        "\x{1FD3}" => 1, # GREEK SMALL LETTER IOTA WITH DIALYTIKA AND OXIA
        "\x{1FE3}" => 1, # GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND OXIA

        # These are included because they are adjacent and fold to the same
        # result, U+01C6.  This has tripped up the code in the past that
        # wrongly thought that sequential code points must fold to sequential
        # code points
        "\x{01C4}" => 1, # LATIN CAPITAL LETTER DZ WITH CARON
        "\x{01C5}" => 1, # LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON

        "I" => 1,
);

# Tests both unicode and not, so make sure not implicitly testing unicode
no feature 'unicode_strings';

# Case-insensitive matching is a large and complicated issue.  Perl does not
# implement it fully, properly.  For example, it doesn't include normalization
# as part of the equation.  To test every conceivable combination is clearly
# impossible; these tests are mostly drawn from visual inspection of the code
# and experience, trying to exercise all areas.

# There are three basic ranges of characters that Perl may treat differently:
# 1) Invariants under utf8 which on ASCII-ish machines are ASCII, and are
#    referred to here as ASCII.  On EBCDIC machines, the non-ASCII invariants
#    are all controls that fold to themselves.
my $ASCII = 1;

# 2) Other characters that fit into a byte but are different in utf8 than not;
#    here referred to, taking some liberties, as Latin1.
my $Latin1 = 2;

# 3) Characters that won't fit in a byte; here referred to as Unicode
my $Unicode = 3;

# Within these basic groups are equivalence classes that testing any character
# in is likely to lead to the same results as any other character.  This is
# used to cut down the number of tests needed, unless PERL_RUN_SLOW_TESTS is
# set.
my $skip_apparently_redundant = ! $ENV{PERL_RUN_SLOW_TESTS};

# Additionally parts of this test run a lot of subtests, outputting the
# resulting TAP can be expensive so the tests are summarised internally. The
# PERL_DEBUG_FULL_TEST environment variable can be set to produce the full
# output for debugging purposes.

sub range_type {
    my $ord = ord shift;

    return $ASCII if utf8::native_to_unicode($ord) < 128;
    return $Latin1 if $ord < 256;
    return $Unicode;
}

sub numerically {
    return $a <=> $b
}

my $list_all_tests = $ENV{PERL_DEBUG_FULL_TEST} || $DEBUG;
$| = 1 if $list_all_tests;

# Significant time is saved by not outputting each test but grouping the
# output into subtests
my $okays;          # Number of ok's in current subtest
my $this_iteration; # Number of possible tests in current subtest
my $count = 0;      # Number of subtests = number of total tests

sub run_test($$$$) {
    my ($test, $todo, $do_we_output_locale_name, $debug) = @_;

    $debug = "" unless $DEBUG;
    my $res = eval $test;

    if ($do_we_output_locale_name) {
        $do_we_output_locale_name = 'setlocale(LC_CTYPE, "'
                         .  POSIX::setlocale(&POSIX::LC_CTYPE)
                         . '"); ';
    }
    if (!$res || $list_all_tests) {
      # Failed or debug; output the result
      $count++;
      ok($res, "$do_we_output_locale_name$test; $debug");
    } else {
      # Just count the test as passed
      $okays++;
    }
    $this_iteration++;
}

my %has_test_by_participants;   # Makes sure has tests for each range and each
                                # number of characters that fold to the same
                                # thing
my %has_test_by_byte_count; # Makes sure has tests for each combination of
                            # n bytes folds to m bytes

my %tests; # The set of tests we expect to pass.
# Each key is a code point that folds to something else.
# Each value is a list of things that the key folds to.  If the 'thing' is a
# single code point, it is that ordinal.  If it is a multi-char fold, it is an
# ordered list of the code points in that fold.  Here's an example for 'S':
#  '83' => [ 115, 383 ]
#
# And one for a multi-char fold: \xDF
#  223 => [
#            [  # 'ss'
#                83,
#                83
#            ],
#            [  # 'SS'
#                115,
#                115
#            ],
#            [  # LATIN SMALL LETTER LONG S
#                383,
#                383
#            ],
#          7838 # LATIN_CAPITAL_LETTER_SHARP_S
#        ],

my %neg_tests;  # Same format, but we expect these tests to fail

my %folds; # keys are code points that fold; values are either 0 or 1 which
           # in turn are keys with their values each a list of code points the
           # code point key folds to.  The folds under 1 are the ones that are
           # valid in this run; the ones under 0 are ones valid under other
           # circumstances.

my %inverse_folds;  # keys are strings of the folded-to; then come a layer of
                    # 0 or 1, like %folds.  The lowest values are lists of
                    # characters that fold to them

# Here's a portion of an %inverse_folds in a run where Turkic folds are not
# legal, so \x{130} doesn't fold to 'i' in this run.
#         'h' => {
#                  '1' => [
#                           'H'
#                         ]
#                },
#         "h\x{331}" => {
#                         '1' => [
#                                  "\x{1e96}"
#                                ]
#                       },
#         'i' => {
#                  '0' => [
#                           "\x{130}"
#                         ],
#                  '1' => [
#                           'I'
#                         ]
#                },
#         "i\x{307}" => {
#                         '1' => [
#                                  "\x{130}"
#                                ]
#                       },
#         'j' => {
#                  '1' => [
#                           'J'
#                         ]
#                },

sub add_test($$@) {
    my ($tests_ref, $to, @from) = @_;

    # Called to cause the input to be tested by adding to $%tests_ref.  @from
    # is the list of characters that fold to the string $to.  @from should be
    # sorted so the lowest code point is first....
    # The input is in string form; %tests uses code points, so have to
    # convert.

    my $to_chars = length $to;
    my @test_to;        # List of tests for $to

    if ($to_chars == 1) {
        @test_to = ord $to;
    }
    else {
        push @test_to, [ map { ord $_ } split "", $to ];

        # For multi-char folds, we also test that things that can fold to each
        # individual character in the fold also work.  If we were testing
        # comprehensively, we would try every combination of upper and lower
        # case in the fold, but it will have to suffice to avoid running
        # forever to make sure that each thing that folds to these is tested
        # at least once.  Because of complement matching ([^...]), we need to
        # do both the folded, and the folded-from.
        # We first look at each character in the multi-char fold, and save how
        # many characters fold to it; and also the maximum number of such
        # folds
        my @folds_to_count;     # 0th char in fold is index 0 ...
        my $max_folds_to = 0;

        for (my $i = 0; $i < $to_chars; $i++) {
            my $to_char = substr($to, $i, 1);
            if (exists $inverse_folds{$to_char}{1}) {
                $folds_to_count[$i] = scalar @{$inverse_folds{$to_char}{1}};
                $max_folds_to = $folds_to_count[$i] if $max_folds_to < $folds_to_count[$i];
            }
            else {
                $folds_to_count[$i] = 0;
            }
        }

        # We will need to generate as many tests as the maximum number of
        # folds, so that each fold will have at least one test.
        # For example, consider character X which folds to the three character
        # string 'xyz'.  If 2 things fold to x (X and x), 4 to y (Y, Y'
        # (Y-prime), Y'' (Y-prime-prime), and y), and 1 thing to z (itself), 4
        # tests will be generated:
        #   xyz
        #   XYz
        #   xY'z
        #   xY''z
        for (my $i = 0; $i < $max_folds_to; $i++) {
            my @this_test_to;   # Assemble a single test

            # For each character in the multi-char fold ...
            for (my $j = 0; $j < $to_chars; $j++) {
                my $this_char = substr($to, $j, 1);

                # Use its corresponding inverse fold, if available.
                if (   $i < $folds_to_count[$j]
                    && exists $inverse_folds{$this_char}{1})
                  {
                    push @this_test_to, ord $inverse_folds{$this_char}{1}[$i];
                }
                else {  # Or else itself.
                    push @this_test_to, ord $this_char;
                }
            }

            # Add this test to the list
            push @test_to, [ @this_test_to ];
        }

        # Here, have assembled all the tests for the multi-char fold.  Sort so
        # lowest code points are first for consistency and aesthetics in
        # output.  We know there are at least two characters in the fold, but
        # I haven't bothered to worry about sorting on an optional third
        # character if the first two are identical.
        @test_to = sort { ($a->[0] == $b->[0])
                           ? $a->[1] <=> $b->[1]
                           : $a->[0] <=> $b->[0]
                        } @test_to;
    }


    # This test is from n bytes to m bytes.  Record that so won't try to add
    # another test that does the same.
    use bytes;
    my $to_bytes = length $to;
    foreach my $from_map (@from) {
        $has_test_by_byte_count{length $from_map}{$to_bytes} = $to;
    }
    no bytes;

    my $ord_smallest_from = ord shift @from;
    if (exists $tests_ref->{$ord_smallest_from}) {
        die "There are already tests for $ord_smallest_from"
    };

    # Add in the fold tests,
    push @{$tests_ref->{$ord_smallest_from}}, @test_to;

    # Then any remaining froms in the equivalence class.
    push @{$tests_ref->{$ord_smallest_from}}, map { ord $_ } @from;
}

# Get the Unicode rules and construct inverse mappings from them

use Unicode::UCD;
my $file="../lib/unicore/CaseFolding.txt";

# Use the Unicode data file if we are on an ASCII platform (which its data is
# for), and it is in the modern format (starting in Unicode 3.1.0) and it is
# available.  This avoids being affected by potential bugs introduced by other
# layers of Perl
if ($::IS_ASCII
    && pack("C*", split /\./, Unicode::UCD::UnicodeVersion()) ge v3.1.0
    && open my $fh, "<", $file)
{
    # We process the file in reverse order because its easier to see the T
    # entry first and then know that the next line we process is the
    # corresponding one for non-T.
    my @rules = <$fh>;
    my $prev_was_turkic = 0;
    while (defined ($_ = pop @rules)) {
        chomp;

        # Lines look like (though without the initial '#')
        #0130; F; 0069 0307; # LATIN CAPITAL LETTER I WITH DOT ABOVE

        # Get rid of comments, ignore blank or comment-only lines
        my $line = $_ =~ s/ (?: \s* \# .* )? $ //rx;
        next unless length $line;
        my ($hex_from, $fold_type, @hex_folded) = split /[\s;]+/, $line;

        next if $fold_type eq 'S';  # If Unicode's tables are correct, the F
                                    # should be a superset of S
        next if $fold_type eq 'I';  # Perl doesn't do old Turkish folding

        my $test_type;
        if ($fold_type eq 'T') {
            $test_type = 0 + $use_turkic_rules;
            $prev_was_turkic = 1;
        }
        elsif ($prev_was_turkic) {
            $test_type = 0 + ! $use_turkic_rules;
            $prev_was_turkic = 0;
        }
        else {
            $test_type = 1;
            $prev_was_turkic = 0;
        }

        my $from = hex $hex_from;
        my @to = map { hex $_ } @hex_folded;
        push @{$folds{$from}{$test_type}}, @to;

        my $folded_str = pack ("U0U*", @to);
        push @{$inverse_folds{$folded_str}{$test_type}}, chr $from;
    }
}
else {  # Here, can't use the .txt file: read the Unicode rules file and
        # construct inverse mappings from it

    skip_all "Don't know how to generate turkic rules on this platform"
                                                            if $use_turkic_rules;
    my ($invlist_ref, $invmap_ref, undef, $default)
                                    = Unicode::UCD::prop_invmap('Case_Folding');
    for my $i (0 .. @$invlist_ref - 1 - 1) {
        next if $invmap_ref->[$i] == $default;

        # Make into an array if not so already, so can treat uniformly below
        $invmap_ref->[$i] = [ $invmap_ref->[$i] ] if ! ref $invmap_ref->[$i];

        # Each subsequent element of the range requires adjustment of +1 from
        # the previous element
        my $adjust = -1;
        for my $j ($invlist_ref->[$i] .. $invlist_ref->[$i+1] -1) {
            $adjust++;
            my @to = map { $_ + $adjust } @{$invmap_ref->[$i]};
            push @{$folds{$j}{1}}, @to;
            my $folded_str = join "", map { chr } @to;
            utf8::upgrade($folded_str);
            #note (sprintf "%d: %04X: %s", __LINE__, $j, join " ",
            #    map { sprintf "%04X", $_  + $adjust } @{$invmap_ref->[$i]});
            push @{$inverse_folds{$folded_str}{1}}, chr $j;
        }
    }
}

# Analyze the data and generate tests to get adequate test coverage.  We sort
# things so that smallest code points are done first.
foreach my $to (sort { $a cmp $b } keys %inverse_folds)
{
TO:
  foreach my $tests_ref (\%tests, \%neg_tests) {
    my $test_type = ($tests_ref == \%tests) ? 1 : 0;

    next unless exists $inverse_folds{$to}{$test_type};

    # Within each fold, sort so that the smallest code points are done first
    @{$inverse_folds{$to}{$test_type}} = sort { $a cmp $b } @{$inverse_folds{$to}{$test_type}};
    my @from = @{$inverse_folds{$to}{$test_type}};

    # Just add it to the tests if doing complete coverage
    if (! $skip_apparently_redundant) {
        add_test($tests_ref, $to, @from);
        next TO;
    }

    my $to_chars = length $to;
    my $to_range_type = range_type(substr($to, 0, 1));

    # If this is required to be tested, do so.  We check for these first, as
    # they will take up slots of byte-to-byte combinations that we otherwise
    # would have to have other tests to get.
    foreach my $from_map (@from) {
        if (exists $be_sure_to_test{$from_map}) {
            add_test($tests_ref, $to, @from);
            next TO;
        }
    }

    # If the fold contains heterogeneous range types, is suspect and should be
    # tested.
    if ($to_chars > 1) {
        foreach my $char (split "", $to) {
            if (range_type($char) != $to_range_type) {
                add_test($tests_ref, $to, @from);
                next TO;
            }
        }
    }

    # If the mapping crosses range types, is suspect and should be tested
    foreach my $from_map (@from) {
        if (range_type($from_map) != $to_range_type) {
            add_test($tests_ref, $to, @from);
            next TO;
        }
    }

    # Here, all components of the mapping are in the same range type.  For
    # single character folds, we test one case in each range type that has 2
    # particpants, 3 particpants, etc.
    if ($to_chars == 1) {
        if (! exists $has_test_by_participants{scalar @from}{$to_range_type}) {
            add_test($tests_ref, $to, @from);
            $has_test_by_participants{scalar @from}{$to_range_type} = $to;
            next TO;
        }
    }

    # We also test all combinations of mappings from m to n bytes.  This is
    # because the regex optimizer cares.  (Don't bother worrying about that
    # Latin1 chars will occupy a different number of bytes under utf8, as
    # there are plenty of other cases that catch these byte numbers.)
    use bytes;
    my $to_bytes = length $to;
    foreach my $from_map (@from) {
        if (! exists $has_test_by_byte_count{length $from_map}{$to_bytes}) {
            add_test($tests_ref, $to, @from);
            next TO;
        }
    }
  }
}

# For each range type, test additionally a character that folds to itself
add_test(\%tests, ":", ":");
add_test(\%tests, chr utf8::unicode_to_native(0xF7), chr utf8::unicode_to_native(0xF7));
add_test(\%tests, chr 0x2C7, chr 0x2C7);

# To cut down on the number of tests
my $has_tested_aa_above_latin1;
my $has_tested_latin1_aa;
my $has_tested_ascii_aa;
my $has_tested_l_above_latin1;
my $has_tested_above_latin1_l;
my $has_tested_ascii_l;
my $has_tested_above_latin1_d;
my $has_tested_ascii_d;
my $has_tested_non_latin1_d;
my $has_tested_above_latin1_a;
my $has_tested_ascii_a;
my $has_tested_non_latin1_a;

# For use by pairs() in generating combinations
sub prefix {
    my $p = shift;
    map [ $p, $_ ], @_
}

# Returns all ordered combinations of pairs of elements from the input array.
# It doesn't return pairs like (a, a), (b, b).  Change the slice to an array
# to do that.  This was just to have fewer tests.
sub pairs (@) {
    #print STDERR __LINE__, ": ", join(" XXX ", map { sprintf "%04X", $_ } @_), "\n";
    map { prefix $_[$_], @_[0..$_-1, $_+1..$#_] } 0..$#_
}

# Finally ready to do the tests
foreach my $tests_ref (\%neg_tests, \%tests) {
foreach my $test (sort { numerically } keys %{$tests_ref}) {

  my $previous_target;
  my $previous_pattern;
  my @pairs = pairs(sort numerically $test, @{$tests_ref->{$test}});

  # Each fold can be viewed as a closure of all the characters that
  # participate in it.  Look at each possible pairing from a closure, with the
  # first member of the pair the target string to match against, and the
  # second member forming the pattern.  Thus each fold member gets tested as
  # the string, and the pattern with every other member in the opposite role.
  while (my $pair = shift @pairs) {
    my ($target, $pattern) = @$pair;

    # When testing a char that doesn't fold, we can get the same
    # permutation twice; so skip all but the first.
    next if $previous_target
            && $previous_target == $target
            && $previous_pattern == $pattern;
    ($previous_target, $previous_pattern) = ($target, $pattern);

    # Each side may be either a single char or a string.  Extract each into an
    # array (perhaps of length 1)
    my @target, my @pattern;
    @target = (ref $target) ? @$target : $target;
    @pattern = (ref $pattern) ? @$pattern : $pattern;

    # We are testing just folds to/from a single character.  If our pairs
    # happens to generate multi/multi, skip.
    next if @target > 1 && @pattern > 1;

    # Get in hex form.
    my @x_target = map { sprintf "\\x{%04X}", $_ } @target;
    my @x_pattern = map { sprintf "\\x{%04X}", $_ } @pattern;

    my $target_above_latin1 = grep { $_ > 255 } @target;
    my $pattern_above_latin1 = grep { $_ > 255 } @pattern;
    my $target_has_ascii = grep { utf8::native_to_unicode($_) < 128 } @target;
    my $pattern_has_ascii = grep { utf8::native_to_unicode($_) < 128 } @pattern;
    my $target_only_ascii = ! grep { utf8::native_to_unicode($_) > 127 } @target;
    my $pattern_only_ascii = ! grep { utf8::native_to_unicode($_) > 127 } @pattern;
    my $target_has_latin1 = grep { $_ < 256 } @target;
    my $target_has_upper_latin1
                = grep { $_ < 256 && utf8::native_to_unicode($_) > 127 } @target;
    my $pattern_has_upper_latin1
                = grep { $_ < 256 && utf8::native_to_unicode($_) > 127 } @pattern;
    my $pattern_has_latin1 = grep { $_ < 256 } @pattern;
    my $is_self = @target == 1 && @pattern == 1 && $target[0] == $pattern[0];

    # We don't test multi-char folding into other multi-chars.  We are testing
    # a code point that folds to or from other characters.  Find the single
    # code point for diagnostic purposes.  (If both are single, choose the
    # target string)
    my $ord = @target == 1 ? $target[0] : $pattern[0];
    my $progress = sprintf "%04X: \"%s\" and /%s/",
                            $test,
                            join("", @x_target),
                            join("", @x_pattern);
    #note $progress;

    # Now grind out tests, using various combinations.
    {
      my $charset_mod = lc $charset;
      my $current_locale = ($has_LC_CTYPE)
                           ? setlocale(&POSIX::LC_CTYPE)
                           : 'C';
      $current_locale = 'C locale' if $current_locale eq 'C';
      $okays = 0;
      $this_iteration = 0;

      # To cut down somewhat on the enormous quantity of tests this currently
      # runs, skip some for some of the character sets whose results aren't
      # likely to differ from others.  But run all tests on the code points
      # that don't fold, plus one other set in each range group.
      if (! $is_self) {

        # /aa should only affect things with folds in the ASCII range.  But, try
        # it on one set in the other ranges just to make sure it doesn't break
        # them.
        if ($charset eq 'aa') {

          # It may be that this $pair of code points to test are both
          # non-ascii, but if either of them actually fold to ascii, that is
          # suspect and should be tested.  So for /aa, use whether their folds
          # are ascii or not
          my $target_has_ascii = $target_has_ascii;
          my $pattern_has_ascii = $pattern_has_ascii;
          if (! $target_has_ascii) {
            foreach my $cp (@target) {
              if (exists $folds{$cp}{1}
                  && grep { utf8::native_to_unicode($_) < 128 } @{$folds{$cp}{1}} )
              {
                  $target_has_ascii = 1;
                  last;
              }
            }
          }
          if (! $pattern_has_ascii) {
            foreach my $cp (@pattern) {
              if (exists $folds{$cp}{1}
                  && grep { utf8::native_to_unicode($_) < 128 } @{$folds{$cp}}{1} )
              {
                  $pattern_has_ascii = 1;
                  last;
              }
            }
          }

          if (! $target_has_ascii && ! $pattern_has_ascii) {
            if ($target_above_latin1 || $pattern_above_latin1) {
              next if defined $has_tested_aa_above_latin1
                      && $has_tested_aa_above_latin1 != $test;
              $has_tested_aa_above_latin1 = $test;
            }
            next if defined $has_tested_latin1_aa
                    && $has_tested_latin1_aa != $test;
            $has_tested_latin1_aa = $test;
          }
          elsif ($target_only_ascii && $pattern_only_ascii) {

              # And, except for one set just to make sure, skip tests
              # where both elements in the pair are ASCII.  If one works for
              # aa, the others are likely too.  This skips tests where the
              # fold is from non-ASCII to ASCII, but this part of the test
              # is just about the ASCII components.
              next if defined $has_tested_ascii_l
                      && $has_tested_ascii_l != $test;
              $has_tested_ascii_l = $test;
          }
        }
        elsif ($charset eq 'l') {

          # For l, don't need to test beyond one set those things that are
          # all above latin1, because unlikely to have different successes
          # than /u.  But, for the same reason as described in the /aa above,
          # it is suspect and should be tested, if either of the folds are to
          # latin1.
          my $target_has_latin1 = $target_has_latin1;
          my $pattern_has_latin1 = $pattern_has_latin1;
          if (! $target_has_latin1) {
            foreach my $cp (@target) {
              if (exists $folds{$cp}{1}
                  && grep { $_ < 256 } @{$folds{$cp}{1}} )
              {
                $target_has_latin1 = 1;
                last;
              }
            }
          }
          if (! $pattern_has_latin1) {
            foreach my $cp (@pattern) {
              if (exists $folds{$cp}{1}
                  && grep { $_ < 256 } @{$folds{$cp}{1}} )
              {
                $pattern_has_latin1 = 1;
                last;
              }
            }
          }
          if (! $target_has_latin1 && ! $pattern_has_latin1) {
            next if defined $has_tested_above_latin1_l
                    && $has_tested_above_latin1_l != $test;
            $has_tested_above_latin1_l = $test;
          }
          elsif ($target_only_ascii && $pattern_only_ascii) {

              # And, except for one set just to make sure, skip tests
              # where both elements in the pair are ASCII.  This is
              # essentially the same reasoning as above for /aa.
              next if defined $has_tested_ascii_l
                      && $has_tested_ascii_l != $test;
              $has_tested_ascii_l = $test;
          }
        }
        elsif ($charset eq 'd') {
          # Similarly for d.  Beyond one test (besides self) each, we  don't
          # test pairs that are both ascii; or both above latin1, or are
          # combinations of ascii and above latin1.
          if (! $target_has_upper_latin1 && ! $pattern_has_upper_latin1) {
            if ($target_has_ascii && $pattern_has_ascii) {
              next if defined $has_tested_ascii_d
                      && $has_tested_ascii_d != $test;
              $has_tested_ascii_d = $test
            }
            elsif (! $target_has_latin1 && ! $pattern_has_latin1) {
              next if defined $has_tested_above_latin1_d
                      && $has_tested_above_latin1_d != $test;
              $has_tested_above_latin1_d = $test;
            }
            else {
              next if defined $has_tested_non_latin1_d
                      && $has_tested_non_latin1_d != $test;
              $has_tested_non_latin1_d = $test;
            }
          }
        }
        elsif ($charset eq 'a') {
          # Similarly for a.  This should match identically to /u, so wasn't
          # tested at all until a bug was found that was thereby missed.
          # As a compromise, beyond one test (besides self) each, we  don't
          # test pairs that are both ascii; or both above latin1, or are
          # combinations of ascii and above latin1.
          if (! $target_has_upper_latin1 && ! $pattern_has_upper_latin1) {
            if ($target_has_ascii && $pattern_has_ascii) {
              next if defined $has_tested_ascii_a
                      && $has_tested_ascii_a != $test;
              $has_tested_ascii_a = $test
            }
            elsif (! $target_has_latin1 && ! $pattern_has_latin1) {
              next if defined $has_tested_above_latin1_a
                      && $has_tested_above_latin1_a != $test;
              $has_tested_above_latin1_a = $test;
            }
            else {
              next if defined $has_tested_non_latin1_a
                      && $has_tested_non_latin1_a != $test;
              $has_tested_non_latin1_a = $test;
            }
          }
        }
      }

      foreach my $utf8_target (0, 1) {    # Both utf8 and not, for
                                          # code points < 256
        my $upgrade_target = "";

        # These must already be in utf8 because the string to match has
        # something above latin1.  So impossible to test if to not to be in
        # utf8; and otherwise, no upgrade is needed.
        next if $target_above_latin1 && ! $utf8_target;
        $upgrade_target = ' utf8::upgrade($c);' if ! $target_above_latin1 && $utf8_target;

        foreach my $utf8_pattern (0, 1) {
          next if $pattern_above_latin1 && ! $utf8_pattern;

          # Our testing of 'l' uses the POSIX locale, which is ASCII-only
          my $uni_semantics = $charset ne 'l' && (    $utf8_target
                                                  ||  $charset eq 'u'
                                                  ||  $charset eq 'L'
                                                  || ($charset eq 'd' && $utf8_pattern)
                                                  ||  $charset =~ /a/);
          my $upgrade_pattern = "";
          $upgrade_pattern = ' utf8::upgrade($rhs);'
            if ! $pattern_above_latin1 && $utf8_pattern;

          my $lhs = join "", @x_target;
          my $lhs_str = eval qq{"$lhs"}; fail($@) if $@;
          my @rhs = @x_pattern;
          my $rhs = join "", @rhs;

          # Unicode created a folding rule that partially emulates what
          # happens in a Turkish locale, by using combining characters.  The
          # result is close enough to what really should happen, that it can
          # pass many of the tests, but not all.  So, if we have a rule that
          # is expecting failure, it may pass instead.  The code in the block
          # below is good enough for skipping the tests, and khw tried to make
          # it general, but should the rules be revised (unlikely at this
          # point), this might need to be tweaked.
          if ($tests_ref == \%neg_tests) {
            my ($shorter_ref, $longer_ref);

            # Convert the $rhs to a string, like we already did for the lhs
            my $rhs_str = eval qq{"$rhs"}; fail($@) if $@;

            # If the lengths of the two sides are equal, we don't want to do
            # this; this is only to bypass the combining characters affecting
            # things
            if (length $lhs_str != length $rhs_str) {

              # Find the shorter and longer of the pair
              if (length $lhs_str < length $rhs_str) {
                  $shorter_ref = \$lhs_str;
                  $longer_ref = \$rhs_str;
              }
              else {
                  $shorter_ref = \$rhs_str;
                  $longer_ref = \$lhs_str;
              }

              # If the shorter string is entirely contained in the longer, we
              # have generated a test that is likely to succeed, and the
              # reasons it would fail have nothing to do with folding.  But we
              # are expecting it to fail, and so our test is invalid.  Skip
              # it.
              next if index($$longer_ref, $$shorter_ref) >= 0;


              # The above eliminates about half the failure cases.  This gets
              # the rest.  If the shorter string is a single character and has
              # a fold legal in this run to a character that is in the longer
              # string, it is also likely to succeed under /i.  So again our
              # computed test is bogus.
              if (   length $$shorter_ref == 1
                  && exists $folds{ord $$shorter_ref}{1})
              {
                my @folded_to = @{$folds{ord $$shorter_ref}{1}};
                next if   @folded_to == 1
                       && index($$longer_ref, chr $folded_to[0]) >= 0;
              }
            }
          }

          my $should_fail = (! $uni_semantics && $ord < 256 && ! $is_self && utf8::native_to_unicode($ord) >= 128)
                            || ($charset eq 'aa' && $target_has_ascii != $pattern_has_ascii)
                            || ($charset eq 'l' && $target_has_latin1 != $pattern_has_latin1)
                            || $tests_ref == \%neg_tests;

          # Do simple tests of referencing capture buffers, named and
          # numbered.
          my $op = '=~';
          $op = '!~' if $should_fail;

          my $todo = 0;  # No longer any todo's
          my $eval = "my \$c = \"$lhs$rhs\"; my \$rhs = \"$rhs\"; "
                   . $upgrade_pattern
                   . " my \$p = qr/(?$charset_mod:^(\$rhs)\\1\$)/i;"
                   . "$upgrade_target \$c $op \$p";
          run_test($eval, $todo, ($charset_mod eq 'l'), "");

          $eval = "my \$c = \"$lhs$rhs\"; my \$rhs = \"$rhs\"; "
                . $upgrade_pattern
                . " my \$p = qr/(?$charset_mod:^(?<grind>\$rhs)\\k<grind>\$)/i;"
                . "$upgrade_target \$c $op \$p";
          run_test($eval, $todo, ($charset_mod eq 'l'), "");

          if ($lhs ne $rhs) {
            $eval = "my \$c = \"$rhs$lhs\"; my \$rhs = \"$rhs\"; "
                  . $upgrade_pattern
                  . " my \$p = qr/(?$charset_mod:^(\$rhs)\\1\$)/i;"
                  . "$upgrade_target \$c $op \$p";
            run_test($eval, "", ($charset_mod eq 'l'), "");

            $eval = "my \$c = \"$rhs$lhs\"; my \$rhs = \"$rhs\"; "
                  . $upgrade_pattern
                  . " my \$p = qr/(?$charset_mod:^(?<grind>\$rhs)\\k<grind>\$)/i;"
                  . "$upgrade_target \$c $op \$p";
            run_test($eval, "", ($charset_mod eq 'l'), "");
          }

          # See if works on what could be a simple trie.
          my $alternate;
          {
            # Keep the alternate | branch the same length as the tested one so
            # that it's length doesn't influence things
            my $evaled = eval "\"$rhs\"";   # Convert e.g. \x{foo} into its
                                            # chr equivalent
            use bytes;
            $alternate = 'q' x length $evaled;
          }
          $eval = "my \$c = \"$lhs\"; my \$rhs = \"$rhs\"; "
                . $upgrade_pattern
                . " my \$p = qr/\$rhs|$alternate/i$charset_mod;"
                . "$upgrade_target \$c $op \$p";
          run_test($eval, "", ($charset_mod eq 'l'), "");

          # Check that works when the folded character follows something that
          # is quantified.  This test knows the regex code internals to the
          # extent that it knows this is a potential problem, and that there
          # are three different types of quantifiers generated: 1) The thing
          # being quantified matches a single character; 2) it matches more
          # than one character, but is fixed width; 3) it can match a variable
          # number of characters.  (It doesn't know that case 3 shouldn't
          # matter, since it doesn't do anything special for the character
          # following the quantifier; nor that some of the different
          # quantifiers execute the same underlying code, as these tests are
          # quick, and this insulates these tests from changes in the
          # implementation.)
          for my $quantifier ('?', '??', '*', '*?', '+', '+?', '{1,2}', '{1,2}?') {
            $eval = "my \$c = \"_$lhs\"; my \$rhs = \"$rhs\"; $upgrade_pattern "
                  . "my \$p = qr/(?$charset_mod:.$quantifier\$rhs)/i;"
                  . "$upgrade_target \$c $op \$p";
            run_test($eval, "", ($charset_mod eq 'l'), "");
            $eval = "my \$c = \"__$lhs\"; my \$rhs = \"$rhs\"; $upgrade_pattern "
                  . "my \$p = qr/(?$charset_mod:(?:..)$quantifier\$rhs)/i;"
                  . "$upgrade_target \$c $op \$p";
            run_test($eval, "", ($charset_mod eq 'l'), "");
            $eval = "my \$c = \"__$lhs\"; my \$rhs = \"$rhs\"; $upgrade_pattern "
                  . "my \$p = qr/(?$charset_mod:(?:.|\\R)$quantifier\$rhs)/i;"
                  . "$upgrade_target \$c $op \$p";
            run_test($eval, "", ($charset_mod eq 'l'), "");
          }

          foreach my $bracketed (0, 1) {   # Put rhs in [...], or not
            next if $bracketed && @pattern != 1;    # bracketed makes these
                                                    # or's instead of a sequence
            foreach my $optimize_bracketed (0, 1) {
              next if $optimize_bracketed && ! $bracketed;
              foreach my $inverted (0,1) {
                  next if $inverted && ! $bracketed;  # inversion only valid
                                                      # in [^...]
                  next if $inverted && @target != 1;  # [perl #89750] multi-char
                                                      # not valid in [^...]

                # In some cases, add an extra character that doesn't fold, and
                # looks ok in the output.
                my $extra_char = "_";
                foreach my $prepend ("", $extra_char) {
                  foreach my $append ("", $extra_char) {

                    # Assemble the rhs.  Put each character in a separate
                    # bracketed if using charclasses.  This creates a stress on
                    # the code to span a match across multiple elements
                    my $rhs = "";
                    foreach my $rhs_char (@rhs) {
                        $rhs .= '[' if $bracketed;
                        $rhs .= '^' if $inverted;
                        $rhs .=  $rhs_char;

                        # Add a character to the class, so class doesn't get
                        # optimized out, unless we are testing that optimization
                        $rhs .= '_' if $optimize_bracketed;
                        $rhs .= ']' if $bracketed;
                    }

                    # Add one of: no capturing parens
                    #             a single set
                    #             a nested set
                    # Use quantifiers and extra variable width matches inside
                    # them to keep some optimizations from happening
                    foreach my $parend (0, 1, 2) {
                      my $interior = (! $parend)
                                      ? $rhs
                                      : ($parend == 1)
                                          ? "(${rhs},?)"
                                          : "((${rhs})+,?)";
                      foreach my $quantifier ("", '?', '*', '+', '{1,3}') {

                        # Perhaps should be TODOs, as are unimplemented, but
                        # maybe will never be implemented
                        next if @pattern != 1 && $quantifier;

                        # A ? or * quantifier normally causes the thing to be
                        # able to match a null string
                        my $quantifier_can_match_null = $quantifier eq '?'
                                                     || $quantifier eq '*';

                        # But since we only quantify the last character in a
                        # multiple fold, the other characters will have width,
                        # except if we are quantifying the whole rhs
                        my $can_match_null = $quantifier_can_match_null
                                             && (@rhs == 1 || $parend);

                        foreach my $l_anchor ("", '^') { # '\A' didn't change
                                                         # result)
                          foreach my $r_anchor ("", '$') { # '\Z', '\z' didn't
                                                           # change result)
                            # The folded part can match the null string if it
                            # isn't required to have width, and there's not
                            # something on one or both sides that force it to.
                            my $both_sides = ($l_anchor && $r_anchor)
                                              || ($l_anchor && $append)
                                              || ($r_anchor && $prepend)
                                              || ($prepend && $append);
                            my $must_match = ! $can_match_null || $both_sides;
                            # for performance, but doing this missed many failures
                            #next unless $must_match;
                            my $quantified = "(?$charset_mod:$l_anchor$prepend"
                                           . "$interior${quantifier}$append$r_anchor)";
                            my $op;
                            if ($must_match && $should_fail)  {
                                $op = 0;
                            } else {
                                $op = 1;
                            }
                            $op = ! $op if $must_match && $inverted;

                            if ($inverted && @target > 1) {
                              # When doing an inverted match against a
                              # multi-char target, and there is not something on
                              # the left to anchor the match, if it shouldn't
                              # succeed, skip, as what will happen (when working
                              # correctly) is that it will match the first
                              # position correctly, and then be inverted to not
                              # match; then it will go to the second position
                              # where it won't match, but get inverted to match,
                              # and hence succeeding.
                              next if ! ($l_anchor || $prepend) && ! $op;

                              # Can't ever match for latin1 code points non-uni
                              # semantics that have a inverted multi-char fold
                              # when there is something on both sides and the
                              # quantifier isn't such as to span the required
                              # width, which is 2 or 3.
                              $op = 0 if $ord < 255
                                        && ! $uni_semantics
                                        && $both_sides
                                        && ( ! $quantifier || $quantifier eq '?')
                                        && $parend < 2;

                              # Similarly can't ever match when inverting a
                              # multi-char fold for /aa and the quantifier
                              # isn't sufficient to allow it to span to both
                              # sides.
                              $op = 0 if $target_has_ascii
                                         && $charset eq 'aa'
                                         && $both_sides
                                         && ( ! $quantifier || $quantifier eq '?')
                                         && $parend < 2;

                              # Or for /l
                              $op = 0 if $target_has_latin1 && $charset eq 'l'
                                      && $both_sides
                                      && ( ! $quantifier || $quantifier eq '?')
                                      && $parend < 2;
                            }


                            my $desc = "";
                            if ($charset_mod eq 'l') {
                                $desc .= 'setlocale(LC_CTYPE, "'
                                        . POSIX::setlocale(&POSIX::LC_CTYPE)
                                        . '"); '
                            }
                            $desc .= "my \$c = \"$prepend$lhs$append\"; "
                                    . "my \$rhs = \"\"; $upgrade_pattern"
                                    . "my \$p = qr/$quantified\$rhs/i;"
                                    . "$upgrade_target "
                                    . "\$c " . ($op ? "=~" : "!~") . " \$p; ";
                            if ($DEBUG) {
                              $desc .= (
                              "; uni_semantics=$uni_semantics, "
                              . "should_fail=$should_fail, "
                              . "bracketed=$bracketed, "
                              . "prepend=$prepend, "
                              . "append=$append, "
                              . "parend=$parend, "
                              . "quantifier=$quantifier, "
                              . "l_anchor=$l_anchor, "
                              . "r_anchor=$r_anchor; "
                              . "pattern_above_latin1=$pattern_above_latin1; "
                              . "utf8_pattern=$utf8_pattern"
                              );
                            }

                            my $c = "$prepend$lhs_str$append";
                            my $p = "$quantified"; # string copy deliberate
                            utf8::upgrade($c) if length($upgrade_target);
                            utf8::upgrade($p) if length($upgrade_pattern);
                            $p = qr/$p/i;
                            my $res = $op ? ($c =~ $p): ($c !~ $p);

                            if (!$res || $list_all_tests) {
                              # Failed or debug; output the result
                              $count++;
                              ok($res, "test $count - $desc");
                            } else {
                              # Just count the test as passed
                              $okays++;
                            }
                            $this_iteration++;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      unless($list_all_tests) {
        $count++;
        is $okays, $this_iteration, "$okays subtests ok for"
          . " /$charset_mod"
          . (($charset_mod eq 'l') ? " ($current_locale)" : "")
          . ', target="' . join("", @x_target) . '",'
          . ' pat="' . join("", @x_pattern) . '"';
      }
    }
  }
}
}

plan($count);

1
