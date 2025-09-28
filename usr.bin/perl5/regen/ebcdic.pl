use v5.16.0;
use strict;
use warnings;
use integer;

BEGIN { unshift @INC, '.' }

require './regen/regen_lib.pl';
require './regen/charset_translations.pl';

# Generates the EBCDIC translation tables that were formerly hard-coded into
# utfebcdic.h

my $out_fh = open_new('ebcdic_tables.h', '>',
        {style => '*', by => $0, });

sub get_column_headers ($$;$) {
    my ($row_hdr_len, $field_width, $dfa_columns) = @_;
    my $format;
    my $final_column_format;
    my $num_columns;

    if (defined $dfa_columns) {
        $num_columns = $dfa_columns;

        # Trailing blank to correspond with commas in the rows below
        $format = "%${field_width}d ";
    }
    else {  # Is a regular table
        $num_columns = 16;

        # Use blanks to separate the fields
        $format = " " x ( $field_width
                        - 2);               # For the '_X'
        $format .= "_%X ";  # Again, trailing blank over the commas below
    }

    my $header = "/*" . " " x ($row_hdr_len - length "/*");

    # All but the final column
    $header .= sprintf($format, $_) for 0 .. $num_columns - 2;

     # Get rid of trailing blank, so that the final column takes up one less
     # space so that the "*/" doesn't extend past the commas in the rows below
    chop $header;
    $header .= sprintf $format, $num_columns - 1;

    # Again, remove trailing blank
    chop $header;

    return $header . "*/\n";
}

sub output_table_start($$$;$) {
    my ($out_fh, $TYPE, $name, $size) = @_;

    $size = "" unless defined $size;

    # Anything locale related will be written on
    my $const = ($name !~ /locale/i) ? 'CONST' : "";

    my $declaration = "EXT$const $TYPE $name\[$size\]";
    print $out_fh <<EOF;
#  ifndef DOINIT
    $declaration;
#  else
    $declaration = {
EOF
}

sub output_table_end($) {
    print $out_fh "};\n#  endif\n\n";
}

sub output_table ($$;$) {
    my $table_ref = shift;
    my $name = shift;

    my $size = @$table_ref;

    # 0 => print in decimal
    # 1 => print in hex (translates code point to code point)
    # >= 2 => is a dfa table, like https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    #      The number is how many columns in the part after the code point
    #      portion.
    #
    # code point tables in hex areasier to debug, but don't fit into 80
    # columns
    my $type = shift // 1;

    my $print_in_hex = $type == 1;
    my $is_dfa = ($type >= 2) ? $type : 0;
    my $columns_after_256 = 16;

    die "Requres 256 entries in table $name, got @$table_ref"
                                if ! $is_dfa && @$table_ref != 256;
    if (! $is_dfa) {
        die "Requres 256 entries in table $name, got @$table_ref"
                                                        if @$table_ref != 256;
    }
    else {
        $columns_after_256 = $is_dfa;

        print $out_fh <<'EOF';

/* The table below is adapted from
 *      https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 * See copyright notice at the beginning of this file.
 */

EOF
    }

    # Highest number in the table
    my $max_entry = 0;
    $max_entry = map { $_ > $max_entry ? $_ : $max_entry } @$table_ref;

    # We assume that every table has at least one two digit entry, and none
    # are more than three digit.
    my $field_width = ($print_in_hex)
                      ? 4
                      : (($max_entry) > 99 ? 3 : 2);

    my $row_hdr_length;
    my $node_number_field_width;
    my $node_value_field_width;

    # dfa tables have a special header for the rows in the transitions part of
    # the table.  It is longer than the regular one.
    if ($is_dfa) {
        my $max_node_number = ($max_entry - 256) / $columns_after_256 - 1;
        $node_number_field_width = ($max_node_number > 9) ? 2 : 1;
        $node_value_field_width = ($max_node_number * $columns_after_256 > 99)
                                  ? 3 : 2;
        # The header starts with this template, and adds in the number of
        # digits needed to represent the maximum node number and its value
        $row_hdr_length = length("/*N=*/")
                        + $node_number_field_width
                        + $node_value_field_width;
    }
    else {
        $row_hdr_length = length "/*_X*/";  # Template for what the header
                                            # looks like
    }

    # The table may not be representable in 8 bits.
    my $TYPE = 'U8';
    $TYPE = 'U16' if grep { $_ > 255 } @$table_ref;

    output_table_start $out_fh, $TYPE, $name, $size;

    # First the headers for the columns
    print $out_fh get_column_headers($row_hdr_length, $field_width);

    # Now the table body
    my $count = @$table_ref;
    my $last_was_nl = 1;

    # Print each element individually, arranged in rows of columns
    for my $i (0 .. $count - 1) {

        # Node number for here is -1 until get into the dfa state transitions
        my $node = ($i < 256) ? -1 : ($i - 256) / $columns_after_256;

        # Print row header at beginning of each row
        if ($last_was_nl) {
            if ($node >= 0) {
                printf $out_fh "/*N%-*d=%*d*/", $node_number_field_width, $node,
                                               $node_value_field_width, $i - 256;
            }
            else {  # Otherwise is regular row; print its number
                printf $out_fh "/*%X_", $i / 16;

                # These rows in a dfa table require extra space so columns
                # will align vertically (because the Ndd=ddd requires extra
                # space)
                if ($is_dfa) {
                    print  $out_fh " " x (  $node_number_field_width
                                          + $node_value_field_width);
                }
                print  $out_fh "*/";
            }
        }

        if ($print_in_hex) {
            printf $out_fh "0x%02X", $table_ref->[$i];
        }
        else {
            printf $out_fh "%${field_width}d", $table_ref->[$i];
        }

        print $out_fh ",", if $i < $count -1;   # No comma on final entry

        # Add \n if at end of row, which is 16 columns until we get to the
        # transitions part
        if (   ($node < 0 && $i % 16 == 15)
            || ($node >= 0 && ($i -256) % $columns_after_256
                                                    == $columns_after_256 - 1))
        {
            print $out_fh "\n";
            $last_was_nl = 1;
        }
        else {
            $last_was_nl = 0;
        }
    }

    # Print column footer
    print $out_fh get_column_headers($row_hdr_length, $field_width,
                                     ($is_dfa) ? $columns_after_256 : undef);

    output_table_end($out_fh);
}

print $out_fh <<'END';

#ifndef PERL_EBCDIC_TABLES_H_   /* Guard against nested #includes */
#define PERL_EBCDIC_TABLES_H_   1

/* This file contains definitions for various tables used in EBCDIC handling.
 * More info is in utfebcdic.h
 *
 * Some of the tables are adapted from
 *      https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 * which requires this copyright notice:

Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
END

my @charsets = get_supported_code_pages();
shift @charsets;    # ASCII is the 0th, and we don't deal with that here.
foreach my $charset (@charsets) {
    # we process the whole array several times, make a copy
    my @a2e = @{get_a2n($charset)};
    my @e2a;

    print $out_fh "\n" . get_conditional_compile_line_start($charset);
    print $out_fh "\n";

    print $out_fh "/* Index is ASCII platform code point; value is $charset equivalent */\n";
    output_table(\@a2e, "PL_a2e");

    { # Construct the inverse
        for my $i (0 .. 255) {
            $e2a[$a2e[$i]] = $i;
        }
        print $out_fh "/* Index is $charset code point; value is ASCII platform equivalent */\n";
        output_table(\@e2a, "PL_e2a");
    }

    my @i82utf = @{get_I8_2_utf($charset)};
    print $out_fh <<END;
/* (Confusingly named) Index is $charset I8 byte; value is
 * $charset UTF-EBCDIC equivalent */
END
    output_table(\@i82utf, "PL_utf2e");

    { #Construct the inverse
        my @utf2i8;
        for my $i (0 .. 255) {
            $utf2i8[$i82utf[$i]] = $i;
        }
        print $out_fh <<END;
/* (Confusingly named) Index is $charset UTF-EBCDIC byte; value is
 * $charset I8 equivalent */
END
        output_table(\@utf2i8, "PL_e2utf");
    }

    {
        my @utf8skip;

        # These are invariants or continuation bytes.
        for my $i (0 .. 0xBF) {
            $utf8skip[$i82utf[$i]] = 1;
        }

        # These are start bytes;  The skip is the number of consecutive highest
        # order 1-bits (up to 7)
        for my $i (0xC0 .. 255) {
            my $count;
            if ($i == 0b11111111) {
                no warnings 'once';
                $count = $CHARSET_TRANSLATIONS::UTF_EBCDIC_MAXBYTES;
            }
            elsif (($i & 0b11111110) == 0b11111110) {
                $count= 7;
            }
            elsif (($i & 0b11111100) == 0b11111100) {
                $count= 6;
            }
            elsif (($i & 0b11111000) == 0b11111000) {
                $count= 5;
            }
            elsif (($i & 0b11110000) == 0b11110000) {
                $count= 4;
            }
            elsif (($i & 0b11100000) == 0b11100000) {
                $count= 3;
            }
            elsif (($i & 0b11000000) == 0b11000000) {
                $count= 2;
            }
            else {
                die "Something wrong for UTF8SKIP calculation for $i";
            }
            $utf8skip[$i82utf[$i]] = $count;
        }

        print $out_fh <<END;
/* Index is $charset UTF-EBCDIC byte; value is UTF8SKIP for start bytes
 * (including for overlongs); 1 for continuation.  Adapted from the shadow
 * flags table in tr16.  The entries marked 9 in tr16 are continuation bytes
 * and are marked as length 1 here so that we can recover. */
END
        output_table(\@utf8skip, "PL_utf8skip", 0);  # The 0 means don't print
                                                     # in hex
    }

    use feature 'unicode_strings';

    {
        my @lc;
        for my $i (0 .. 255) {
            $lc[$a2e[$i]] = $a2e[ord lc chr $i];
        }
        print $out_fh
        "/* Index is $charset code point; value is its lowercase equivalent */\n";
        output_table(\@lc, "PL_latin1_lc");
    }

    {
        my @uc;
        for my $i (0 .. 255) {
            my $uc = uc chr $i;
            if (length $uc > 1 || ord $uc > 255) {
                $uc = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}";
            }
            $uc[$a2e[$i]] = $a2e[ord $uc];
        }
        print $out_fh <<END;
/* Index is $charset code point; value is its uppercase equivalent.
 * The 'mod' in the name means that codepoints whose uppercase is above 255 or
 * longer than 1 character map to LATIN SMALL LETTER Y WITH DIARESIS */
END
        output_table(\@uc, "PL_mod_latin1_uc");
    }

    { # PL_fold
        my @ascii_fold;
        for my $i (0 .. 255) {  # Initialise to identity map
            $ascii_fold[$i] = $i;
        }

        # Overwrite the entries that aren't identity
        for my $chr ('A' .. 'Z') {
            $ascii_fold[$a2e[ord $chr]] = $a2e[ord lc $chr];
        }
        for my $chr ('a' .. 'z') {
            $ascii_fold[$a2e[ord $chr]] = $a2e[ord uc $chr];
        }
        print $out_fh <<END;
/* Index is $charset code point; For A-Z, value is a-z; for a-z, value
 * is A-Z; all other code points map to themselves */
END
        output_table(\@ascii_fold, "PL_fold");
    }

    {
        my @latin1_fold;
        for my $i (0 .. 255) {
            my $char = chr $i;
            my $lc = lc $char;

            # lc and uc adequately proxy for fold-case pairs in this 0-255
            # range
            my $uc = uc $char;
            $uc = $char if length $uc > 1 || ord $uc > 255;
            if ($lc ne $char) {
                $latin1_fold[$a2e[$i]] = $a2e[ord $lc];
            }
            elsif ($uc ne $char) {
                $latin1_fold[$a2e[$i]] = $a2e[ord $uc];
            }
            else {
                $latin1_fold[$a2e[$i]] = $a2e[$i];
            }
        }
        print $out_fh <<END;
/* Index is $charset code point; value is its other fold-pair equivalent
 * (A => a; a => A, etc) in the 0-255 range.  If no such equivalent, value is
 * the code point itself */
END
        output_table(\@latin1_fold, "PL_fold_latin1");
    }

    {
      # This generates the dfa table for perl extended UTF-8, which accepts
      # surrogates, non-characters, and accepts start bytes up through FE
      # (start byte FF has to be handled outside this dfa).  The class numbers
      # for start bytes are constrained so that they can be used as a shift
      # count for masking off the leading one bits
      #
      # The classes are
      #   00-9F           0
      #   A0-A1           7   Not legal immediately after start bytes F0 F8 FC
      #                       FE
      #   A2-A3           8   Not legal immediately after start bytes F0 F8 FC
      #   A4-A7           9   Not legal immediately after start bytes F0 F8
      #   A8-AF          10   Not legal immediately after start bytes F0
      #   B0-BF          11
      #   C0-C4           1
      #   C5-DF           2
      #   E0              1
      #   E1-EF           3
      #   F0             12
      #   F1-F7           4
      #   F8             13
      #   F9-FB           5
      #   FC             14
      #   FD              6
      #   FE             15
      #   FF              1
      #
      # Here's the I8 for the code points before which overlongs occur:
      # U+4000:     \xF0\xB0\xA0\xA0
      # U+40000:    \xF8\xA8\xA0\xA0\xA0
      # U+400000:   \xFC\xA4\xA0\xA0\xA0\xA0
      # U+4000000:  \xFE\xA2\xA0\xA0\xA0\xA0\xA0
      #
      # The first part of the table maps bytes to character classes to reduce
      # the size of the transition table and create bitmasks.
      #
      # The second part is a transition table that maps a combination of a
      # state of the automaton and a character class to a new state.  The
      # numbering of the original nodes is retained, but some have been split
      # so that there are new nodes.  They mean:
      # N0     The initial state, and final accepting one.
      # N1     One continuation byte (A0-BF) left.  This is transitioned to
      #        immediately when the start byte indicates a two-byte sequence
      # N2     Two continuation bytes left.
      # N3     Three continuation bytes left.
      # N4     Four continuation bytes left.
      # N5     Five continuation bytes left.
      # N6     Start byte is F0.  Continuation bytes A[0-F] are illegal
      #        (overlong); the other continuations transition to N2
      # N7     Start byte is F8.  Continuation bytes A[0-7] are illegal
      #        (overlong); the other continuations transition to N3
      # N8     Start byte is FC.  Continuation bytes A[0-3] are illegal
      #        (overlong); the other continuations transition to N4
      # N9     Start byte is FE.  Continuation bytes A[01] are illegal
      #        (overlong); the other continuations transition to N5
      # 1      Reject.  All transitions not mentioned above (except the single
      #        byte ones (as they are always legal) are to this state.

        my $NUM_CLASSES = 16;
        my $N0 = 0;
        my $N1 =  $N0 + $NUM_CLASSES;
        my $N2 =  $N1 + $NUM_CLASSES;
        my $N3 =  $N2 + $NUM_CLASSES;
        my $N4 =  $N3 + $NUM_CLASSES;
        my $N5 =  $N4 + $NUM_CLASSES;
        my $N6 =  $N5 + $NUM_CLASSES;
        my $N7 =  $N6 + $NUM_CLASSES;
        my $N8 =  $N7 + $NUM_CLASSES;
        my $N9 =  $N8 + $NUM_CLASSES;
        my $N10 = $N9 + $NUM_CLASSES;

        my @perl_extended_utf8_dfa;
        my @i8 = (
                # 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 00-0F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 10-1F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 20-2F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 30-3F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 40-4F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 50-5F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 60-6F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 70-7F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 80-8F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 90-9F
                  7, 7, 8, 8, 9, 9, 9, 9,10,10,10,10,10,10,10,10, # A0-AF
                 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, # B0-BF
                  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # C0-CF
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # D0-DF
                  1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, # E0-EF
                 12, 4, 4, 4, 4, 4, 4, 4,13, 5, 5, 5,14, 6,15, 1, # F0-FF
                );
        $perl_extended_utf8_dfa[$i82utf[$_]] = $i8[$_] for (0 .. 255);
        push @perl_extended_utf8_dfa, (
          # Class:
          # 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
            0,  1,$N1,$N2,$N3,$N4,$N5,  1,  1,  1,  1,  1,$N6,$N7,$N8,$N9, # N0
            1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  1,  1,  1,  1, # N1
            1,  1,  1,  1,  1,  1,  1,$N1,$N1,$N1,$N1,$N1,  1,  1,  1,  1, # N2
            1,  1,  1,  1,  1,  1,  1,$N2,$N2,$N2,$N2,$N2,  1,  1,  1,  1, # N3
            1,  1,  1,  1,  1,  1,  1,$N3,$N3,$N3,$N3,$N3,  1,  1,  1,  1, # N4
            1,  1,  1,  1,  1,  1,  1,$N4,$N4,$N4,$N4,$N4,  1,  1,  1,  1, # N5

            1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,$N2,  1,  1,  1,  1, # N6
            1,  1,  1,  1,  1,  1,  1,  1,  1,  1,$N3,$N3,  1,  1,  1,  1, # N7
            1,  1,  1,  1,  1,  1,  1,  1,  1,$N4,$N4,$N4,  1,  1,  1,  1, # N8
            1,  1,  1,  1,  1,  1,  1,  1,$N5,$N5,$N5,$N5,  1,  1,  1,  1, # N9
        );
        output_table(\@perl_extended_utf8_dfa, "PL_extended_utf8_dfa_tab",
                                                                   $NUM_CLASSES);
    }

    {
      # This generates the dfa table for strict UTF-8, which rejects
      # surrogates, non-characters, and above Unicode.
      #
      # The classes are
      #   00-9F           0   Always legal at start
      #   A0             10   Not legal immediately after start bytes F0 F8
      #   A1             11   Not legal immediately after start bytes F0 F8,
      #   A2-A7          12   Not legal immediately after start bytes F0 F8 F9
      #   A8,AA,AC       13   Not legal immediately after start bytes F0 F9
      #   A9,AB,AD       14   Not legal immediately after start byte F0
      #   AE             15   Not legal immediately after start byte F0
      #   AF             16   Not legal immediately after start bytes F0
      #   B[0248AC]      17   Not legal immediately after start byte F9
      #   B[1359D]       18   Not legal immediately after start byte F9
      #   B6             19   Not legal immediately after start byte F9
      #   B7             20   Not legal immediately after start byte F9
      #   BE             21   Not legal immediately after start byte F9
      #   BF             22   Not legal immediately after start byte F9
      #   C0-C4           1   (reject, all are overlong)
      #   C5-DF           2   Accepts any legal continuation
      #   E0              1   (reject, all are overlong)
      #   E1-EF           3   Accepts any legal continuation
      #   F0              8   (has overlongs)
      #   F1              6   (has surrogates, non-chars)
      #   F2,F4,F6        4   Accepts any legal continuation
      #   F3,F5,F7        5   (has non-chars)
      #   F8              9   (has overlongs, non-chars)
      #   F9              7   (has non-chars, non-Unicode)
      #   FA-FF           1   (reject, all are non-Unicode)
      #
      # Here's the I8 for enough code points so that you can figure out what's
      # going on:
      #
      # U+D800: \xF1\xB6\xA0\xA0
      # U+DFFF: \xF1\xB7\xBF\xBF
      # U+FDD0: \xF1\xBF\xAE\xB0
      # U+FDEF: \xF1\xBF\xAF\xAF
      # U+FFFE: \xF1\xBF\xBF\xBE
      # U+1FFFE: \xF3\xBF\xBF\xBE
      # U+2FFFE: \xF5\xBF\xBF\xBE
      # U+3FFFE: \xF7\xBF\xBF\xBE
      # U+4FFFE: \xF8\xA9\xBF\xBF\xBE
      # U+5FFFE: \xF8\xAB\xBF\xBF\xBE
      # U+6FFFE: \xF8\xAD\xBF\xBF\xBE
      # U+7FFFE: \xF8\xAF\xBF\xBF\xBE
      # U+8FFFE: \xF8\xB1\xBF\xBF\xBE
      # U+9FFFE: \xF8\xB3\xBF\xBF\xBE
      # U+AFFFE: \xF8\xB5\xBF\xBF\xBE
      # U+BFFFE: \xF8\xB7\xBF\xBF\xBE
      # U+CFFFE: \xF8\xB9\xBF\xBF\xBE
      # U+DFFFE: \xF8\xBB\xBF\xBF\xBE
      # U+EFFFE: \xF8\xBD\xBF\xBF\xBE
      # U+FFFFE: \xF8\xBF\xBF\xBF\xBE
      # U+10FFFE: \xF9\xA1\xBF\xBF\xBE
      #
      # The first part of the table maps bytes to character classes to reduce
      # the size of the transition table and create bitmasks.
      #
      # The second part is a transition table that maps a combination of a
      # state of the automaton and a character class to a new state.  The
      # numbering of the original nodes is retained, but some have been split
      # so that there are new nodes.  They mean:
      # N0     The initial state, and final accepting one.
      # N1     One continuation byte (A0-BF) left.  This is transitioned to
      #        immediately when the start byte indicates a two-byte sequence
      # N2     Two continuation bytes left.
      # N3     Three continuation bytes left.
      # N4     Start byte is F0.  Continuation bytes A[0-F] are illegal
      #        (overlong); the other continuations transition to N2
      # N5     Start byte is F1.  Continuation bytes B6 and B7 are illegal
      #        (surrogates); BF transitions to N9; the other continuations to
      #        N2
      # N6     Start byte is F[357].  Continuation byte BF transitions to N12;
      #        other continuations to N2
      # N7     Start byte is F8.  Continuation bytes A[0-7] are illegal
      #        (overlong); continuations A[9BDF] and B[13579BDF] transition to
      #        N14; the other continuations to N3
      # N8     Start byte is F9.  Continuation byte A0 transitions to N3; A1
      #        to N14; the other continuation bytes are illegal.
      # N9     Initial sequence is F1 BF.  Continuation byte AE transitions to
      #        state N10; AF to N11; BF to N13; the other continuations to N1.
      # N10    Initial sequence is F1 BF AE.  Continuation bytes B0-BF are
      #        illegal (non-chars); the other continuations are legal
      # N11    Initial sequence is F1 BF AF.  Continuation bytes A0-AF are
      #        illegal (non-chars); the other continuations are legal
      # N12    Initial sequence is F[357] BF.  Continuation bytes BF
      #        transitions to N13; the other continuations to N1
      # N13    Initial sequence is F[1357] BF BF or F8 x y BF (where x and y
      #        are something that can lead to a non-char.  Continuation bytes
      #        BE and BF are illegal (non-chars); the other continuations are
      #        legal
      # N14    Initial sequence is F8 A[9BDF]; or F8 B[13579BDF]; or F9 A1.
      #        Continuation byte BF transitions to N15; the other
      #        continuations to N2
      # N15    Initial sequence is F8 A[9BDF] BF; or F8 B[13579BDF] BF; or
      #        F9 A1 BF.  Continuation byte BF transitions to N16; the other
      #        continuations to N2
      # N16    Initial sequence is F8 A[9BDF] BF BF; or F8 B[13579BDF] BF BF;
      #        or F9 A1 BF BF.  Continuation bytes BE and BF are illegal
      #        (non-chars); the other continuations are legal
      # 1      Reject.  All transitions not mentioned above (except the single
      #        byte ones (as they are always legal) are to this state.

        my $NUM_CLASSES = 23;
        my $N0 = 0;
        my $N1 =  $N0 + $NUM_CLASSES;
        my $N2 =  $N1 + $NUM_CLASSES;
        my $N3 =  $N2 + $NUM_CLASSES;
        my $N4 =  $N3 + $NUM_CLASSES;
        my $N5 =  $N4 + $NUM_CLASSES;
        my $N6 =  $N5 + $NUM_CLASSES;
        my $N7 =  $N6 + $NUM_CLASSES;
        my $N8 =  $N7 + $NUM_CLASSES;
        my $N9 =  $N8 + $NUM_CLASSES;
        my $N10 = $N9 + $NUM_CLASSES;
        my $N11 = $N10 + $NUM_CLASSES;
        my $N12 = $N11 + $NUM_CLASSES;
        my $N13 = $N12 + $NUM_CLASSES;
        my $N14 = $N13 + $NUM_CLASSES;
        my $N15 = $N14 + $NUM_CLASSES;

        my @strict_utf8_dfa;
        my @i8 = (
                # 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 00-0F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 10-1F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 20-2F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 30-3F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 40-4F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 50-5F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 60-6F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 70-7F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 80-8F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 90-9F
                 10,11,12,12,12,12,12,12,13,14,13,14,13,14,15,16, # A0-AF
                 17,18,17,18,17,18,19,20,17,18,17,18,17,18,21,22, # B0-BF
                  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # C0-CF
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # D0-DF
                  1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, # E0-EF
                  8, 6, 4, 5, 4, 5, 4, 5, 9, 7, 1, 1, 1, 1, 1, 1, # F0-FF
                );
        $strict_utf8_dfa[$i82utf[$_]] = $i8[$_] for (0 .. 255);
        push @strict_utf8_dfa, (
          # Class:
          # 0 1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19   20   21   22
            0,1,$N1,$N2,$N3,$N6,$N5,$N8,$N4,$N7,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1, # N0
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, # N1
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, # N2
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, # N3

            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,   1,   1,   1,   1,   1, $N2, $N2, $N2, $N2, $N2, $N2, # N4
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2,   1,   1, $N2, $N9, # N5
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2,$N12, # N6
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,   1, $N3,$N14, $N3,$N14, $N3,$N14, $N3,$N14, $N3,$N14, # N7
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N3,$N14,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1, # N8
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N1, $N1, $N1, $N1, $N1,$N10,$N11, $N1, $N1, $N1, $N1, $N1,$N13, # N9
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1, # N10
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0, # N11
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1,$N13, # N12
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1, # N13
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2, $N2,$N15, # N14
            1,1,  1,  1,  1,  1,  1,  1,  1,  1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1, $N1,$N13, # N15
        );
        output_table(\@strict_utf8_dfa, "PL_strict_utf8_dfa_tab", $NUM_CLASSES);
    }

    {
      # This generates the dfa table for C9 strict UTF-8, which rejects
      # surrogates and above Unicode, but allows non-characters,.
      #
      # The classes are
      #   00-9F           0   Always legal at start
      #   A0-A1           9   Not legal immediately after start bytes F0 F8
      #   A2-A7          10   Not legal immediately after start bytes F0 F8 F9
      #   A8-AF          11   Not legal immediately after start bytes F0 F9
      #   B0-B5,B8-BF    12   Not legal immediately after start byte F9
      #   B6,B7          13
      #   C0-C4           1   (reject, all are overlong)
      #   C5-DF           2   Accepts any legal continuation
      #   E0              1   (reject, all are overlong)
      #   E1-EF           3   Accepts any legal continuation
      #   F0              6   (has overlongs)
      #   F1              5   (has surrogates)
      #   F2-F7           4   Accepts any legal continuation
      #   F8              8   (has overlongs)
      #   F9              7   (has non-Unicode)
      #   FA-FF           1   (reject, all are non-Unicode)
      #
      # The first part of the table maps bytes to character classes to reduce
      # the size of the transition table and create bitmasks.
      #
      # The second part is a transition table that maps a combination of a
      # state of the automaton and a character class to a new state.  The
      # numbering of the original nodes is retained, but some have been split
      # so that there are new nodes.  They mean:
      # N0     The initial state, and final accepting one.
      # N1     One continuation byte (A0-BF) left.  This is transitioned to
      #        immediately when the start byte indicates a two-byte sequence
      # N2     Two continuation bytes left.
      # N3     Three continuation bytes left.
      # N4     Start byte is F0.  Continuation bytes A[0-F] are illegal
      #        (overlong); the other continuations transition to N2
      # N5     Start byte is F1.  B6 and B7 are illegal (surrogates); the
      #        other continuations transition to N2
      # N6     Start byte is F8.  Continuation bytes A[0-7] are illegal
      #        (overlong); the other continuations transition to N3
      # N7     Start byte is F9.  Continuation bytes A0 and A1 transition to
      #        N3; the other continuation bytes are illegal (non-Unicode)
      # 1      Reject.  All transitions not mentioned above (except the single
      #        byte ones (as they are always legal) are to this state.

        my $NUM_CLASSES = 14;
        my $N0 = 0;
        my $N1 =  $N0 + $NUM_CLASSES;
        my $N2 =  $N1 + $NUM_CLASSES;
        my $N3 =  $N2 + $NUM_CLASSES;
        my $N4 =  $N3 + $NUM_CLASSES;
        my $N5 =  $N4 + $NUM_CLASSES;
        my $N6 =  $N5 + $NUM_CLASSES;
        my $N7 =  $N6 + $NUM_CLASSES;

        my @C9_utf8_dfa;
        my @i8 = (
                # 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 00-0F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 10-1F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 20-2F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 30-3F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 40-4F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 50-5F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 60-6F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 70-7F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 80-8F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, # 90-9F
                  9, 9,10,10,10,10,10,10,11,11,11,11,11,11,11,11, # A0-AF
                 12,12,12,12,12,12,13,13,12,12,12,12,12,12,12,12, # B0-BF
                  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # C0-CF
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, # D0-DF
                  1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, # E0-EF
                  6, 5, 4, 4, 4, 4, 4, 4, 8, 7, 1, 1, 1, 1, 1, 1, # F0-FF
                );
        $C9_utf8_dfa[$i82utf[$_]] = $i8[$_] for (0 .. 255);
        push @C9_utf8_dfa, (
          # Class:
          # 0 1   2   3   4   5   6   7   8   9   10   11   12   13
            0,1,$N1,$N2,$N3,$N5,$N4,$N7,$N6,  1,   1,   1,   1,   1, # N0
            1,1,  1,  1,  1,  1,  1,  1,  1,  0,   0,   0,   0,   0, # N1
            1,1,  1,  1,  1,  1,  1,  1,  1,$N1, $N1, $N1, $N1, $N1, # N2
            1,1,  1,  1,  1,  1,  1,  1,  1,$N2, $N2, $N2, $N2, $N2, # N3

            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   1,   1, $N2, $N2, # N4
            1,1,  1,  1,  1,  1,  1,  1,  1,$N2, $N2, $N2, $N2,   1, # N5
            1,1,  1,  1,  1,  1,  1,  1,  1,  1,   1, $N3, $N3, $N3, # N6
            1,1,  1,  1,  1,  1,  1,  1,  1,$N3,   1,   1,   1,   1, # N7
        );
        output_table(\@C9_utf8_dfa, "PL_c9_utf8_dfa_tab", $NUM_CLASSES);
    }

    print $out_fh get_conditional_compile_line_end();
}

print $out_fh "\n#endif /* PERL_EBCDIC_TABLES_H_ */\n";

read_only_bottom_close_and_rename($out_fh);
