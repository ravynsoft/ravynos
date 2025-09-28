package Pod::Simple::BlackBox;
#
# "What's in the box?"  "Pain."
#
###########################################################################
#
# This is where all the scary things happen: parsing lines into
#  paragraphs; and then into directives, verbatims, and then also
#  turning formatting sequences into treelets.
#
# Are you really sure you want to read this code?
#
#-----------------------------------------------------------------------------
#
# The basic work of this module Pod::Simple::BlackBox is doing the dirty work
# of parsing Pod into treelets (generally one per non-verbatim paragraph), and
# to call the proper callbacks on the treelets.
#
# Every node in a treelet is a ['name', {attrhash}, ...children...]

use integer; # vroom!
use strict;
use Carp ();
use vars qw($VERSION );
$VERSION = '3.43';
#use constant DEBUG => 7;

sub my_qr ($$) {

    # $1 is a pattern to compile and return.  Older perls compile any
    # syntactically valid property, even if it isn't legal.  To cope with
    # this, return an empty string unless the compiled pattern also
    # successfully matches $2, which the caller furnishes.

    my ($input_re, $should_match) = @_;
    # XXX could have a third parameter $shouldnt_match for extra safety

    my $use_utf8 = ($] le 5.006002) ? 'use utf8;' : "";

    my $re = eval "no warnings; $use_utf8 qr/$input_re/";
    #print STDERR  __LINE__, ": $input_re: $@\n" if $@;
    return "" if $@;

    my $matches = eval "no warnings; $use_utf8 '$should_match' =~ /$re/";
    #print STDERR  __LINE__, ": $input_re: $@\n" if $@;
    return "" if $@;

    #print STDERR  __LINE__, ": SUCCESS: $re\n" if $matches;
    return $re if $matches;

    #print STDERR  __LINE__, ": $re: didn't match\n";
    return "";
}

BEGIN {
  require Pod::Simple;
  *DEBUG = \&Pod::Simple::DEBUG unless defined &DEBUG
}

# Matches a character iff the character will have a different meaning
# if we choose CP1252 vs UTF-8 if there is no =encoding line.
# This is broken for early Perls on non-ASCII platforms.
my $non_ascii_re = my_qr('[[:^ascii:]]', "\xB6");
$non_ascii_re = qr/[\x80-\xFF]/ unless $non_ascii_re;

# Use patterns understandable by Perl 5.6, if possible
my $cs_re = do { no warnings; my_qr('\p{IsCs}', "\x{D800}") };
my $cn_re = my_qr('\p{IsCn}', "\x{09E4}");  # <reserved> code point unlikely
                                            # to get assigned
my $rare_blocks_re = my_qr('[\p{InIPAExtensions}\p{InSpacingModifierLetters}]',
                           "\x{250}");
$rare_blocks_re = my_qr('[\x{0250}-\x{02FF}]', "\x{250}") unless $rare_blocks_re;

my $script_run_re = eval 'no warnings "experimental::script_run";
                          qr/(*script_run: ^ .* $ )/x';
my $latin_re = my_qr('[\p{IsLatin}\p{IsInherited}\p{IsCommon}]', "\x{100}");
unless ($latin_re) {
    # This was machine generated to be the ranges of the union of the above
    # three properties, with things that were undefined by Unicode 4.1 filling
    # gaps.  That is the version in use when Perl advanced enough to
    # successfully compile and execute the above pattern.
    $latin_re = my_qr('[\x00-\x{02E9}\x{02EC}-\x{0374}\x{037E}\x{0385}\x{0387}\x{0485}\x{0486}\x{0589}\x{060C}\x{061B}\x{061F}\x{0640}\x{064B}-\x{0655}\x{0670}\x{06DD}\x{0951}-\x{0954}\x{0964}\x{0965}\x{0E3F}\x{10FB}\x{16EB}-\x{16ED}\x{1735}\x{1736}\x{1802}\x{1803}\x{1805}\x{1D00}-\x{1D25}\x{1D2C}-\x{1D5C}\x{1D62}-\x{1D65}\x{1D6B}-\x{1D77}\x{1D79}-\x{1DBE}\x{1DC0}-\x{1EF9}\x{2000}-\x{2125}\x{2127}-\x{27FF}\x{2900}-\x{2B13}\x{2E00}-\x{2E1D}\x{2FF0}-\x{3004}\x{3006}\x{3008}-\x{3020}\x{302A}-\x{302D}\x{3030}-\x{3037}\x{303C}-\x{303F}\x{3099}-\x{309C}\x{30A0}\x{30FB}\x{30FC}\x{3190}-\x{319F}\x{31C0}-\x{31CF}\x{3220}-\x{325F}\x{327F}-\x{32CF}\x{3358}-\x{33FF}\x{4DC0}-\x{4DFF}\x{A700}-\x{A716}\x{FB00}-\x{FB06}\x{FD3E}\x{FD3F}\x{FE00}-\x{FE6B}\x{FEFF}-\x{FF65}\x{FF70}\x{FF9E}\x{FF9F}\x{FFE0}-\x{FFFD}\x{10100}-\x{1013F}\x{1D000}-\x{1D1DD}\x{1D300}-\x{1D7FF}]', "\x{100}");
}

my $every_char_is_latin_re = my_qr("^(?:$latin_re)*\\z", "A");

# Latin script code points not in the first release of Unicode
my $later_latin_re = my_qr('[^\P{IsLatin}\p{IsAge=1.1}]', "\x{1F6}");

# If this perl doesn't have the Deprecated property, there's only one code
# point in it that we need be concerned with.
my $deprecated_re = my_qr('\p{IsDeprecated}', "\x{149}");
$deprecated_re = qr/\x{149}/ unless $deprecated_re;

my $utf8_bom;
if (($] ge 5.007_003)) {
  $utf8_bom = "\x{FEFF}";
  utf8::encode($utf8_bom);
} else {
  $utf8_bom = "\xEF\xBB\xBF";   # No EBCDIC BOM detection for early Perls.
}

# This is used so that the 'content_seen' method doesn't return true on a
# file that just happens to have a line that matches /^=[a-zA-z]/.  Only if
# there is a valid =foo line will we return that content was seen.
my $seen_legal_directive = 0;

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub parse_line { shift->parse_lines(@_) } # alias

# - - -  Turn back now!  Run away!  - - -

sub parse_lines {             # Usage: $parser->parse_lines(@lines)
  # an undef means end-of-stream
  my $self = shift;

  my $code_handler = $self->{'code_handler'};
  my $cut_handler  = $self->{'cut_handler'};
  my $wl_handler   = $self->{'whiteline_handler'};
  $self->{'line_count'} ||= 0;

  my $scratch;

  DEBUG > 4 and
   print STDERR "# Parsing starting at line ", $self->{'line_count'}, ".\n";

  DEBUG > 5 and
   print STDERR "#  About to parse lines: ",
     join(' ', map defined($_) ? "[$_]" : "EOF", @_), "\n";

  my $paras = ($self->{'paras'} ||= []);
   # paragraph buffer.  Because we need to defer processing of =over
   # directives and verbatim paragraphs.  We call _ponder_paragraph_buffer
   # to process this.

  $self->{'pod_para_count'} ||= 0;

  # An attempt to match the pod portions of a line.  This is not fool proof,
  # but is good enough to serve as part of the heuristic for guessing the pod
  # encoding if not specified.
  my $codes = join '', grep { / ^ [A-Za-z] $/x } sort keys %{$self->{accept_codes}};
  my $pod_chars_re = qr/ ^ = [A-Za-z]+ | [\Q$codes\E] < /x;

  my $line;
  foreach my $source_line (@_) {
    if( $self->{'source_dead'} ) {
      DEBUG > 4 and print STDERR "# Source is dead.\n";
      last;
    }

    unless( defined $source_line ) {
      DEBUG > 4 and print STDERR "# Undef-line seen.\n";

      push @$paras, ['~end', {'start_line' => $self->{'line_count'}}];
      push @$paras, $paras->[-1], $paras->[-1];
       # So that it definitely fills the buffer.
      $self->{'source_dead'} = 1;
      $self->_ponder_paragraph_buffer;
      next;
    }


    if( $self->{'line_count'}++ ) {
      ($line = $source_line) =~ tr/\n\r//d;
       # If we don't have two vars, we'll end up with that there
       # tr/// modding the (potentially read-only) original source line!

    } else {
      DEBUG > 2 and print STDERR "First line: [$source_line]\n";

      if( ($line = $source_line) =~ s/^$utf8_bom//s ) {
        DEBUG and print STDERR "UTF-8 BOM seen.  Faking a '=encoding utf8'.\n";
        $self->_handle_encoding_line( "=encoding utf8" );
        delete $self->{'_processed_encoding'};
        $line =~ tr/\n\r//d;

      } elsif( $line =~ s/^\xFE\xFF//s ) {
        DEBUG and print STDERR "Big-endian UTF-16 BOM seen.  Aborting parsing.\n";
        $self->scream(
          $self->{'line_count'},
          "UTF16-BE Byte Encoding Mark found; but Pod::Simple v$Pod::Simple::VERSION doesn't implement UTF16 yet."
        );
        splice @_;
        push @_, undef;
        next;

        # TODO: implement somehow?

      } elsif( $line =~ s/^\xFF\xFE//s ) {
        DEBUG and print STDERR "Little-endian UTF-16 BOM seen.  Aborting parsing.\n";
        $self->scream(
          $self->{'line_count'},
          "UTF16-LE Byte Encoding Mark found; but Pod::Simple v$Pod::Simple::VERSION doesn't implement UTF16 yet."
        );
        splice @_;
        push @_, undef;
        next;

        # TODO: implement somehow?

      } else {
        DEBUG > 2 and print STDERR "First line is BOM-less.\n";
        ($line = $source_line) =~ tr/\n\r//d;
      }
    }

    if(!$self->{'parse_characters'} && !$self->{'encoding'}
      && ($self->{'in_pod'} || $line =~ /^=/s)
      && $line =~ /$non_ascii_re/
    ) {

      my $encoding;

      # No =encoding line, and we are at the first pod line in the input that
      # contains a non-ascii byte, that is, one whose meaning varies depending
      # on whether the file is encoded in UTF-8 or CP1252, which are the two
      # possibilities permitted by the pod spec.  (ASCII is assumed if the
      # file only contains ASCII bytes.)  In order to process this line, we
      # need to figure out what encoding we will use for the file.
      #
      # Strictly speaking ISO 8859-1 (Latin 1) refers to the code points
      # 160-255, but it is used here, as it often colloquially is, to refer to
      # the complete set of code points 0-255, including ASCII (0-127), the C1
      # controls (128-159), and strict Latin 1 (160-255).
      #
      # CP1252 is effectively a superset of Latin 1, because it differs only
      # from colloquial 8859-1 in the C1 controls, which are very unlikely to
      # actually be present in 8859-1 files, so can be used for other purposes
      # without conflict.  CP 1252 uses most of them for graphic characters.
      #
      # Note that all ASCII-range bytes represent their corresponding code
      # points in both CP1252 and UTF-8.  In ASCII platform UTF-8, all other
      # code points require multiple (non-ASCII) bytes to represent.  (A
      # separate paragraph for EBCDIC is below.)  The multi-byte
      # representation is quite structured.  If we find an isolated byte that
      # would require multiple bytes to represent in UTF-8, we know that the
      # encoding is not UTF-8.  If we find a sequence of bytes that violates
      # the UTF-8 structure, we also can presume the encoding isn't UTF-8, and
      # hence must be 1252.
      #
      # But there are ambiguous cases where we could guess wrong.  If so, the
      # user will end up having to supply an =encoding line.  We use all
      # readily available information to improve our chances of guessing
      # right.  The odds of something not being UTF-8, but still passing a
      # UTF-8 validity test go down very rapidly with increasing length of the
      # sequence.  Therefore we look at all non-ascii sequences on the line.
      # If any of the sequences can't be UTF-8, we quit there and choose
      # CP1252.  If all could be UTF-8, we see if any of the code points
      # represented are unlikely to be in pod.  If so, we guess CP1252.  If
      # not, we check if the line is all in the same script; if not guess
      # CP1252; otherwise UTF-8.  For perls that don't have convenient script
      # run testing, see if there is both Latin and non-Latin.  If so, CP1252,
      # otherwise UTF-8.
      #
      # On EBCDIC platforms, the situation is somewhat different.  In
      # UTF-EBCDIC, not only do ASCII-range bytes represent their code points,
      # but so do the bytes that are for the C1 controls.  Recall that these
      # correspond to the unused portion of 8859-1 that 1252 mostly takes
      # over.  That means that there are fewer code points that are
      # represented by multi-bytes.  But, note that the these controls are
      # very unlikely to be in pod text.  So if we encounter one of them, it
      # means that it is quite likely CP1252 and not UTF-8.  The net result is
      # the same code below is used for both platforms.
      #
      # XXX probably if the line has E<foo> that evaluates to illegal CP1252,
      # then it is UTF-8.  But we haven't processed E<> yet.

      goto set_1252 if $] lt 5.006_000;    # No UTF-8 on very early perls

      my $copy;

      no warnings 'utf8';

      if ($] ge 5.007_003) {
        $copy = $line;

        # On perls that have this function, we can use it to easily see if the
        # sequence is valid UTF-8 or not; if valid it turns on the UTF-8 flag
        # needed below for script run detection
        goto set_1252 if ! utf8::decode($copy);
      }
      elsif (ord("A") != 65) {  # Early EBCDIC, assume UTF-8.  What's a windows
                                # code page doing here anyway?
        goto set_utf8;
      }
      else { # ASCII, no decode(): do it ourselves using the fundamental
             # characteristics of UTF-8
        use if $] le 5.006002, 'utf8';

        my $char_ord;
        my $needed;         # How many continuation bytes to gobble up

        # Initialize the translated line with a dummy character that will be
        # deleted after everything else is done.  This dummy makes sure that
        # $copy will be in UTF-8.  Doing it now avoids the bugs in early perls
        # with upgrading in the middle
        $copy = chr(0x100);

        # Parse through the line
        for (my $i = 0; $i < length $line; $i++) {
          my $byte = substr($line, $i, 1);

          # ASCII bytes are trivially dealt with
          if ($byte !~ $non_ascii_re) {
            $copy .= $byte;
            next;
          }

          my $b_ord = ord $byte;

          # Now figure out what this code point would be if the input is
          # actually in UTF-8.  If, in the process, we discover that it isn't
          # well-formed UTF-8, we guess CP1252.
          #
          # Start the process.  If it is UTF-8, we are at the first, start
          # byte, of a multi-byte sequence.  We look at this byte to figure
          # out how many continuation bytes are needed, and to initialize the
          # code point accumulator with the data from this byte.
          #
          # Normally the minimum continuation byte is 0x80, but in certain
          # instances the minimum is a higher number.  So the code below
          # overrides this for those instances.
          my $min_cont = 0x80;

          if ($b_ord < 0xC2) { #  A start byte < C2 is malformed
            goto set_1252;
          }
          elsif ($b_ord <= 0xDF) {
            $needed = 1;
            $char_ord = $b_ord & 0x1F;
          }
          elsif ($b_ord <= 0xEF) {
            $min_cont = 0xA0 if $b_ord == 0xE0;
            $needed = 2;
            $char_ord = $b_ord & (0x1F >> 1);
          }
          elsif ($b_ord <= 0xF4) {
            $min_cont = 0x90 if $b_ord == 0xF0;
            $needed = 3;
            $char_ord = $b_ord & (0x1F >> 2);
          }
          else { # F4 is the highest start byte for legal Unicode; higher is
                 # unlikely to be in pod.
            goto set_1252;
          }

          # ? not enough continuation bytes available
          goto set_1252 if $i + $needed >= length $line;

          # Accumulate the ordinal of the character from the remaining
          # (continuation) bytes.
          while ($needed-- > 0) {
            my $cont = substr($line, ++$i, 1);
            $b_ord = ord $cont;
            goto set_1252 if $b_ord < $min_cont || $b_ord > 0xBF;

            # In all cases, any next continuation bytes all have the same
            # minimum legal value
            $min_cont = 0x80;

            # Accumulate this byte's contribution to the code point
            $char_ord <<= 6;
            $char_ord |= ($b_ord & 0x3F);
          }

          # Here, the sequence that formed this code point was valid UTF-8,
          # so add the completed character to the output
          $copy .= chr $char_ord;
        } # End of loop through line

        # Delete the dummy first character
        $copy = substr($copy, 1);
      }

      # Here, $copy is legal UTF-8.

      # If it can't be legal CP1252, no need to look further.  (These bytes
      # aren't valid in CP1252.)  This test could have been placed higher in
      # the code, but it seemed wrong to set the encoding to UTF-8 without
      # making sure that the very first instance is well-formed.  But what if
      # it isn't legal CP1252 either?  We have to choose one or the other, and
      # It seems safer to favor the single-byte encoding over the multi-byte.
      goto set_utf8 if ord("A") == 65 && $line =~ /[\x81\x8D\x8F\x90\x9D]/;

      # The C1 controls are not likely to appear in pod
      goto set_1252 if ord("A") == 65 && $copy =~ /[\x80-\x9F]/;

      # Nor are surrogates nor unassigned, nor deprecated.
      DEBUG > 8 and print STDERR __LINE__, ": $copy: surrogate\n" if $copy =~ $cs_re;
      goto set_1252 if $cs_re && $copy =~ $cs_re;
      DEBUG > 8 and print STDERR __LINE__, ": $copy: unassigned\n" if $cn_re && $copy =~ $cn_re;
      goto set_1252 if $cn_re && $copy =~ $cn_re;
      DEBUG > 8 and print STDERR __LINE__, ": $copy: deprecated\n" if $copy =~ $deprecated_re;
      goto set_1252 if $copy =~ $deprecated_re;

      # Nor are rare code points.  But this is hard to determine.  khw
      # believes that IPA characters and the modifier letters are unlikely to
      # be in pod (and certainly very unlikely to be the in the first line in
      # the pod containing non-ASCII)
      DEBUG > 8 and print STDERR __LINE__, ": $copy: rare\n" if $copy =~ $rare_blocks_re;
      goto set_1252 if $rare_blocks_re && $copy =~ $rare_blocks_re;

      # The first Unicode version included essentially every Latin character
      # in modern usage.  So, a Latin character not in the first release will
      # unlikely be in pod.
      DEBUG > 8 and print STDERR __LINE__, ": $copy: later_latin\n" if $later_latin_re && $copy =~ $later_latin_re;
      goto set_1252 if $later_latin_re && $copy =~ $later_latin_re;

      # On perls that handle script runs, if the UTF-8 interpretation yields
      # a single script, we guess UTF-8, otherwise just having a mixture of
      # scripts is suspicious, so guess CP1252.  We first strip off, as best
      # we can, the ASCII characters that look like they are pod directives,
      # as these would always show as mixed with non-Latin text.
      $copy =~ s/$pod_chars_re//g;

      if ($script_run_re) {
        goto set_utf8 if $copy =~ $script_run_re;
        DEBUG > 8 and print STDERR __LINE__, ":  not script run\n";
        goto set_1252;
      }

      # Even without script runs, but on recent enough perls and Unicodes, we
      # can check if there is a mixture of both Latin and non-Latin.  Again,
      # having a mixture of scripts is suspicious, so assume CP1252

      # If it's all non-Latin, there is no CP1252, as that is Latin
      # characters and punct, etc.
      DEBUG > 8 and print STDERR __LINE__, ": $copy: not latin\n" if $copy !~ $latin_re;
      goto set_utf8 if $copy !~ $latin_re;

      DEBUG > 8 and print STDERR __LINE__, ": $copy: all latin\n" if $copy =~ $every_char_is_latin_re;
      goto set_utf8 if $copy =~ $every_char_is_latin_re;

      DEBUG > 8 and print STDERR __LINE__, ": $copy: mixed\n";

     set_1252:
      DEBUG > 9 and print STDERR __LINE__, ": $copy: is 1252\n";
      $encoding = 'CP1252';
      goto done_set;

     set_utf8:
      DEBUG > 9 and print STDERR __LINE__, ": $copy: is UTF-8\n";
      $encoding = 'UTF-8';

     done_set:
      $self->_handle_encoding_line( "=encoding $encoding" );
      delete $self->{'_processed_encoding'};
      $self->{'_transcoder'} && $self->{'_transcoder'}->($line);

      my ($word) = $line =~ /(\S*$non_ascii_re\S*)/;

      $self->whine(
        $self->{'line_count'},
        "Non-ASCII character seen before =encoding in '$word'. Assuming $encoding"
      );
    }

    DEBUG > 5 and print STDERR "# Parsing line: [$line]\n";

    if(!$self->{'in_pod'}) {
      if($line =~ m/^=([a-zA-Z][a-zA-Z0-9]*)(?:\s|$)/s) {
        if($1 eq 'cut') {
          $self->scream(
            $self->{'line_count'},
            "=cut found outside a pod block.  Skipping to next block."
          );

          ## Before there were errata sections in the world, it was
          ## least-pessimal to abort processing the file.  But now we can
          ## just barrel on thru (but still not start a pod block).
          #splice @_;
          #push @_, undef;

          next;
        } else {
          $self->{'in_pod'} = $self->{'start_of_pod_block'}
                            = $self->{'last_was_blank'}     = 1;
          # And fall thru to the pod-mode block further down
        }
      } else {
        DEBUG > 5 and print STDERR "# It's a code-line.\n";
        $code_handler->(map $_, $line, $self->{'line_count'}, $self)
         if $code_handler;
        # Note: this may cause code to be processed out of order relative
        #  to pods, but in order relative to cuts.

        # Note also that we haven't yet applied the transcoding to $line
        #  by time we call $code_handler!

        if( $line =~ m/^#\s*line\s+(\d+)\s*(?:\s"([^"]+)")?\s*$/ ) {
          # That RE is from perlsyn, section "Plain Old Comments (Not!)",
          #$fname = $2 if defined $2;
          #DEBUG > 1 and defined $2 and print STDERR "# Setting fname to \"$fname\"\n";
          DEBUG > 1 and print STDERR "# Setting nextline to $1\n";
          $self->{'line_count'} = $1 - 1;
        }

        next;
      }
    }

    # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
    # Else we're in pod mode:

    # Apply any necessary transcoding:
    $self->{'_transcoder'} && $self->{'_transcoder'}->($line);

    # HERE WE CATCH =encoding EARLY!
    if( $line =~ m/^=encoding\s+\S+\s*$/s ) {
      next if $self->parse_characters;   # Ignore this line
      $line = $self->_handle_encoding_line( $line );
    }

    if($line =~ m/^=cut/s) {
      # here ends the pod block, and therefore the previous pod para
      DEBUG > 1 and print STDERR "Noting =cut at line ${$self}{'line_count'}\n";
      $self->{'in_pod'} = 0;
      # ++$self->{'pod_para_count'};
      $self->_ponder_paragraph_buffer();
       # by now it's safe to consider the previous paragraph as done.
      DEBUG > 6 and print STDERR "Processing any cut handler, line ${$self}{'line_count'}\n";
      $cut_handler->(map $_, $line, $self->{'line_count'}, $self)
       if $cut_handler;

      # TODO: add to docs: Note: this may cause cuts to be processed out
      #  of order relative to pods, but in order relative to code.

    } elsif($line =~ m/^(\s*)$/s) {  # it's a blank line
      if (defined $1 and $1 =~ /[^\S\r\n]/) { # it's a white line
        $wl_handler->(map $_, $line, $self->{'line_count'}, $self)
          if $wl_handler;
      }

      if(!$self->{'start_of_pod_block'} and @$paras and $paras->[-1][0] eq '~Verbatim') {
        DEBUG > 1 and print STDERR "Saving blank line at line ${$self}{'line_count'}\n";
        push @{$paras->[-1]}, $line;
      }  # otherwise it's not interesting

      if(!$self->{'start_of_pod_block'} and !$self->{'last_was_blank'}) {
        DEBUG > 1 and print STDERR "Noting para ends with blank line at ${$self}{'line_count'}\n";
      }

      $self->{'last_was_blank'} = 1;

    } elsif($self->{'last_was_blank'}) {  # A non-blank line starting a new para...

      if($line =~ m/^(=[a-zA-Z][a-zA-Z0-9]*)(\s+|$)(.*)/s) {
        # THIS IS THE ONE PLACE WHERE WE CONSTRUCT NEW DIRECTIVE OBJECTS
        my $new = [$1, {'start_line' => $self->{'line_count'}}, $3];
        $new->[1]{'~orig_spacer'} = $2 if $2 && $2 ne " ";
         # Note that in "=head1 foo", the WS is lost.
         # Example: ['=head1', {'start_line' => 123}, ' foo']

        ++$self->{'pod_para_count'};

        $self->_ponder_paragraph_buffer();
         # by now it's safe to consider the previous paragraph as done.

        push @$paras, $new; # the new incipient paragraph
        DEBUG > 1 and print STDERR "Starting new ${$paras}[-1][0] para at line ${$self}{'line_count'}\n";

      } elsif($line =~ m/^\s/s) {

        if(!$self->{'start_of_pod_block'} and @$paras and $paras->[-1][0] eq '~Verbatim') {
          DEBUG > 1 and print STDERR "Resuming verbatim para at line ${$self}{'line_count'}\n";
          push @{$paras->[-1]}, $line;
        } else {
          ++$self->{'pod_para_count'};
          $self->_ponder_paragraph_buffer();
           # by now it's safe to consider the previous paragraph as done.
          DEBUG > 1 and print STDERR "Starting verbatim para at line ${$self}{'line_count'}\n";
          push @$paras, ['~Verbatim', {'start_line' => $self->{'line_count'}}, $line];
        }
      } else {
        ++$self->{'pod_para_count'};
        $self->_ponder_paragraph_buffer();
         # by now it's safe to consider the previous paragraph as done.
        push @$paras, ['~Para',  {'start_line' => $self->{'line_count'}}, $line];
        DEBUG > 1 and print STDERR "Starting plain para at line ${$self}{'line_count'}\n";
      }
      $self->{'last_was_blank'} = $self->{'start_of_pod_block'} = 0;

    } else {
      # It's a non-blank line /continuing/ the current para
      if(@$paras) {
        DEBUG > 2 and print STDERR "Line ${$self}{'line_count'} continues current paragraph\n";
        push @{$paras->[-1]}, $line;
      } else {
        # Unexpected case!
        die "Continuing a paragraph but \@\$paras is empty?";
      }
      $self->{'last_was_blank'} = $self->{'start_of_pod_block'} = 0;
    }

  } # ends the big while loop

  DEBUG > 1 and print STDERR (pretty(@$paras), "\n");
  return $self;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub _handle_encoding_line {
  my($self, $line) = @_;

  return if $self->parse_characters;

  # The point of this routine is to set $self->{'_transcoder'} as indicated.

  return $line unless $line =~ m/^=encoding\s+(\S+)\s*$/s;
  DEBUG > 1 and print STDERR "Found an encoding line \"=encoding $1\"\n";

  my $e    = $1;
  my $orig = $e;
  push @{ $self->{'encoding_command_reqs'} }, "=encoding $orig";

  my $enc_error;

  # Cf.   perldoc Encode   and   perldoc Encode::Supported

  require Pod::Simple::Transcode;

  if( $self->{'encoding'} ) {
    my $norm_current = $self->{'encoding'};
    my $norm_e = $e;
    foreach my $that ($norm_current, $norm_e) {
      $that =  lc($that);
      $that =~ s/[-_]//g;
    }
    if($norm_current eq $norm_e) {
      DEBUG > 1 and print STDERR "The '=encoding $orig' line is ",
       "redundant.  ($norm_current eq $norm_e).  Ignoring.\n";
      $enc_error = '';
       # But that doesn't necessarily mean that the earlier one went okay
    } else {
      $enc_error = "Encoding is already set to " . $self->{'encoding'};
      DEBUG > 1 and print STDERR $enc_error;
    }
  } elsif (
    # OK, let's turn on the encoding
    do {
      DEBUG > 1 and print STDERR " Setting encoding to $e\n";
      $self->{'encoding'} = $e;
      1;
    }
    and $e eq 'HACKRAW'
  ) {
    DEBUG and print STDERR " Putting in HACKRAW (no-op) encoding mode.\n";

  } elsif( Pod::Simple::Transcode::->encoding_is_available($e) ) {

    die($enc_error = "WHAT? _transcoder is already set?!")
     if $self->{'_transcoder'};   # should never happen
    require Pod::Simple::Transcode;
    $self->{'_transcoder'} = Pod::Simple::Transcode::->make_transcoder($e);
    eval {
      my @x = ('', "abc", "123");
      $self->{'_transcoder'}->(@x);
    };
    $@ && die( $enc_error =
      "Really unexpected error setting up encoding $e: $@\nAborting"
    );
    $self->{'detected_encoding'} = $e;

  } else {
    my @supported = Pod::Simple::Transcode::->all_encodings;

    # Note unsupported, and complain
    DEBUG and print STDERR " Encoding [$e] is unsupported.",
      "\nSupporteds: @supported\n";
    my $suggestion = '';

    # Look for a near match:
    my $norm = lc($e);
    $norm =~ tr[-_][]d;
    my $n;
    foreach my $enc (@supported) {
      $n = lc($enc);
      $n =~ tr[-_][]d;
      next unless $n eq $norm;
      $suggestion = "  (Maybe \"$e\" should be \"$enc\"?)";
      last;
    }
    my $encmodver = Pod::Simple::Transcode::->encmodver;
    $enc_error = join '' =>
      "This document probably does not appear as it should, because its ",
      "\"=encoding $e\" line calls for an unsupported encoding.",
      $suggestion, "  [$encmodver\'s supported encodings are: @supported]"
    ;

    $self->scream( $self->{'line_count'}, $enc_error );
  }
  push @{ $self->{'encoding_command_statuses'} }, $enc_error;
  if (defined($self->{'_processed_encoding'})) {
    # Double declaration.
    $self->scream( $self->{'line_count'}, 'Cannot have multiple =encoding directives');
  }
  $self->{'_processed_encoding'} = $orig;

  return $line;
}

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

sub _handle_encoding_second_level {
  # By time this is called, the encoding (if well formed) will already
  #  have been acted on.
  my($self, $para) = @_;
  my @x = @$para;
  my $content = join ' ', splice @x, 2;
  $content =~ s/^\s+//s;
  $content =~ s/\s+$//s;

  DEBUG > 2 and print STDERR "Ogling encoding directive: =encoding $content\n";

  if (defined($self->{'_processed_encoding'})) {
    #if($content ne $self->{'_processed_encoding'}) {
    #  Could it happen?
    #}
    delete $self->{'_processed_encoding'};
    # It's already been handled.  Check for errors.
    if(! $self->{'encoding_command_statuses'} ) {
      DEBUG > 2 and print STDERR " CRAZY ERROR: It wasn't really handled?!\n";
    } elsif( $self->{'encoding_command_statuses'}[-1] ) {
      $self->whine( $para->[1]{'start_line'},
        sprintf "Couldn't do %s: %s",
          $self->{'encoding_command_reqs'  }[-1],
          $self->{'encoding_command_statuses'}[-1],
      );
    } else {
      DEBUG > 2 and print STDERR " (Yup, it was successfully handled already.)\n";
    }

  } else {
    # Otherwise it's a syntax error
    $self->whine( $para->[1]{'start_line'},
      "Invalid =encoding syntax: $content"
    );
  }

  return;
}

#~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`~`

{
my $m = -321;   # magic line number

sub _gen_errata {
  my $self = $_[0];
  # Return 0 or more fake-o paragraphs explaining the accumulated
  #  errors on this document.

  return() unless $self->{'errata'} and keys %{$self->{'errata'}};

  my @out;

  foreach my $line (sort {$a <=> $b} keys %{$self->{'errata'}}) {
    push @out,
      ['=item', {'start_line' => $m}, "Around line $line:"],
      map( ['~Para', {'start_line' => $m, '~cooked' => 1},
        #['~Top', {'start_line' => $m},
        $_
        #]
        ],
        @{$self->{'errata'}{$line}}
      )
    ;
  }

  # TODO: report of unknown entities? unrenderable characters?

  unshift @out,
    ['=head1', {'start_line' => $m, 'errata' => 1}, 'POD ERRORS'],
    ['~Para', {'start_line' => $m, '~cooked' => 1, 'errata' => 1},
     "Hey! ",
     ['B', {},
      'The above document had some coding errors, which are explained below:'
     ]
    ],
    ['=over',  {'start_line' => $m, 'errata' => 1}, ''],
  ;

  push @out,
    ['=back',  {'start_line' => $m, 'errata' => 1}, ''],
  ;

  DEBUG and print STDERR "\n<<\n", pretty(\@out), "\n>>\n\n";

  return @out;
}

}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

##############################################################################
##
##  stop reading now stop reading now stop reading now stop reading now stop
##
##                         HERE IT BECOMES REALLY SCARY
##
##  stop reading now stop reading now stop reading now stop reading now stop
##
##############################################################################

sub _ponder_paragraph_buffer {

  # Para-token types as found in the buffer.
  #   ~Verbatim, ~Para, ~end, =head1..4, =for, =begin, =end,
  #   =over, =back, =item
  #   and the null =pod (to be complained about if over one line)
  #
  # "~data" paragraphs are something we generate at this level, depending on
  # a currently open =over region

  # Events fired:  Begin and end for:
  #                   directivename (like head1 .. head4), item, extend,
  #                   for (from =begin...=end, =for),
  #                   over-bullet, over-number, over-text, over-block,
  #                   item-bullet, item-number, item-text,
  #                   Document,
  #                   Data, Para, Verbatim
  #                   B, C, longdirname (TODO -- wha?), etc. for all directives
  #

  my $self = $_[0];
  my $paras;
  return unless @{$paras = $self->{'paras'}};
  my $curr_open = ($self->{'curr_open'} ||= []);

  my $scratch;

  DEBUG > 10 and print STDERR "# Paragraph buffer: <<", pretty($paras), ">>\n";

  # We have something in our buffer.  So apparently the document has started.
  unless($self->{'doc_has_started'}) {
    $self->{'doc_has_started'} = 1;

    my $starting_contentless;
    $starting_contentless =
     (
       !@$curr_open
       and @$paras and ! grep $_->[0] ne '~end', @$paras
        # i.e., if the paras is all ~ends
     )
    ;
    DEBUG and print STDERR "# Starting ",
      $starting_contentless ? 'contentless' : 'contentful',
      " document\n"
    ;

    $self->_handle_element_start(
      ($scratch = 'Document'),
      {
        'start_line' => $paras->[0][1]{'start_line'},
        $starting_contentless ? ( 'contentless' => 1 ) : (),
      },
    );
  }

  my($para, $para_type);
  while(@$paras) {

    # If a directive, assume it's legal; subtract below if found not to be
    $seen_legal_directive++ if $paras->[0][0] =~ /^=/;

    last if      @$paras == 1
            and (    $paras->[0][0] eq '=over'
                 or  $paras->[0][0] eq '=item'
                 or ($paras->[0][0] eq '~Verbatim' and $self->{'in_pod'}));
    # Those're the three kinds of paragraphs that require lookahead.
    #   Actually, an "=item Foo" inside an <over type=text> region
    #   and any =item inside an <over type=block> region (rare)
    #   don't require any lookahead, but all others (bullets
    #   and numbers) do.
    # The verbatim is different from the other two, because those might be
    # like:
    #
    #   =item
    #   ...
    #   =cut
    #   ...
    #   =item
    #
    # The =cut here finishes the paragraph but doesn't terminate the =over
    # they should be in. (khw apologizes that he didn't comment at the time
    # why the 'in_pod' works, and no longer remembers why, and doesn't think
    # it is currently worth the effort to re-figure it out.)

# TODO: whinge about many kinds of directives in non-resolving =for regions?
# TODO: many?  like what?  =head1 etc?

    $para = shift @$paras;
    $para_type = $para->[0];

    DEBUG > 1 and print STDERR "Pondering a $para_type paragraph, given the stack: (",
      $self->_dump_curr_open(), ")\n";

    if($para_type eq '=for') {
      next if $self->_ponder_for($para,$curr_open,$paras);

    } elsif($para_type eq '=begin') {
      next if $self->_ponder_begin($para,$curr_open,$paras);

    } elsif($para_type eq '=end') {
      next if $self->_ponder_end($para,$curr_open,$paras);

    } elsif($para_type eq '~end') { # The virtual end-document signal
      next if $self->_ponder_doc_end($para,$curr_open,$paras);
    }


    # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    #~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    if(grep $_->[1]{'~ignore'}, @$curr_open) {
      DEBUG > 1 and
       print STDERR "Skipping $para_type paragraph because in ignore mode.\n";
      next;
    }
    #~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
    # ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

    if($para_type eq '=pod') {
      $self->_ponder_pod($para,$curr_open,$paras);

    } elsif($para_type eq '=over') {
      next if $self->_ponder_over($para,$curr_open,$paras);

    } elsif($para_type eq '=back') {
      next if $self->_ponder_back($para,$curr_open,$paras);

    } else {

      # All non-magical codes!!!

      # Here we start using $para_type for our own twisted purposes, to
      #  mean how it should get treated, not as what the element name
      #  should be.

      DEBUG > 1 and print STDERR "Pondering non-magical $para_type\n";

      my $i;

      # Enforce some =headN discipline
      if($para_type =~ m/^=head\d$/s
         and ! $self->{'accept_heads_anywhere'}
         and @$curr_open
         and $curr_open->[-1][0] eq '=over'
      ) {
        DEBUG > 2 and print STDERR "'=$para_type' inside an '=over'!\n";
        $self->whine(
          $para->[1]{'start_line'},
          "You forgot a '=back' before '$para_type'"
        );
        unshift @$paras, ['=back', {}, ''], $para;   # close the =over
        next;
      }


      if($para_type eq '=item') {

        my $over;
        unless(@$curr_open and
               $over = (grep { $_->[0] eq '=over' } @$curr_open)[-1]) {
          $self->whine(
            $para->[1]{'start_line'},
            "'=item' outside of any '=over'"
          );
          unshift @$paras,
            ['=over', {'start_line' => $para->[1]{'start_line'}}, ''],
            $para
          ;
          next;
        }


        my $over_type = $over->[1]{'~type'};

        if(!$over_type) {
          # Shouldn't happen1
          die "Typeless over in stack, starting at line "
           . $over->[1]{'start_line'};

        } elsif($over_type eq 'block') {
          unless($curr_open->[-1][1]{'~bitched_about'}) {
            $curr_open->[-1][1]{'~bitched_about'} = 1;
            $self->whine(
              $curr_open->[-1][1]{'start_line'},
              "You can't have =items (as at line "
              . $para->[1]{'start_line'}
              . ") unless the first thing after the =over is an =item"
            );
          }
          # Just turn it into a paragraph and reconsider it
          $para->[0] = '~Para';
          unshift @$paras, $para;
          next;

        } elsif($over_type eq 'text') {
          my $item_type = $self->_get_item_type($para);
            # That kills the content of the item if it's a number or bullet.
          DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

          if($item_type eq 'text') {
            # Nothing special needs doing for 'text'
          } elsif($item_type eq 'number' or $item_type eq 'bullet') {
            $self->whine(
              $para->[1]{'start_line'},
              "Expected text after =item, not a $item_type"
            );
            # Undo our clobbering:
            push @$para, $para->[1]{'~orig_content'};
            delete $para->[1]{'number'};
             # Only a PROPER item-number element is allowed
             #  to have a number attribute.
          } else {
            die "Unhandled item type $item_type"; # should never happen
          }

          # =item-text thingies don't need any assimilation, it seems.

        } elsif($over_type eq 'number') {
          my $item_type = $self->_get_item_type($para);
            # That kills the content of the item if it's a number or bullet.
          DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

          my $expected_value = ++ $curr_open->[-1][1]{'~counter'};

          if($item_type eq 'bullet') {
            # Hm, it's not numeric.  Correct for this.
            $para->[1]{'number'} = $expected_value;
            $self->whine(
              $para->[1]{'start_line'},
              "Expected '=item $expected_value'"
            );
            push @$para, $para->[1]{'~orig_content'};
              # restore the bullet, blocking the assimilation of next para

          } elsif($item_type eq 'text') {
            # Hm, it's not numeric.  Correct for this.
            $para->[1]{'number'} = $expected_value;
            $self->whine(
              $para->[1]{'start_line'},
              "Expected '=item $expected_value'"
            );
            # Text content will still be there and will block next ~Para

          } elsif($item_type ne 'number') {
            die "Unknown item type $item_type"; # should never happen

          } elsif($expected_value == $para->[1]{'number'}) {
            DEBUG > 1 and print STDERR " Numeric item has the expected value of $expected_value\n";

          } else {
            DEBUG > 1 and print STDERR " Numeric item has ", $para->[1]{'number'},
             " instead of the expected value of $expected_value\n";
            $self->whine(
              $para->[1]{'start_line'},
              "You have '=item " . $para->[1]{'number'} .
              "' instead of the expected '=item $expected_value'"
            );
            $para->[1]{'number'} = $expected_value;  # correcting!!
          }

          if(@$para == 2) {
            # For the cases where we /didn't/ push to @$para
            if($paras->[0][0] eq '~Para') {
              DEBUG and print STDERR "Assimilating following ~Para content into $over_type item\n";
              push @$para, splice @{shift @$paras},2;
            } else {
              DEBUG and print STDERR "Can't assimilate following ", $paras->[0][0], "\n";
              push @$para, '';  # Just so it's not contentless
            }
          }


        } elsif($over_type eq 'bullet') {
          my $item_type = $self->_get_item_type($para);
            # That kills the content of the item if it's a number or bullet.
          DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

          if($item_type eq 'bullet') {
            # as expected!

            if( $para->[1]{'~_freaky_para_hack'} ) {
              DEBUG and print STDERR "Accomodating '=item * Foo' tolerance hack.\n";
              push @$para, $para->[1]{'~_freaky_para_hack'};
            }

          } elsif($item_type eq 'number') {
            $self->whine(
              $para->[1]{'start_line'},
              "Expected '=item *'"
            );
            push @$para, $para->[1]{'~orig_content'};
             # and block assimilation of the next paragraph
            delete $para->[1]{'number'};
             # Only a PROPER item-number element is allowed
             #  to have a number attribute.
          } elsif($item_type eq 'text') {
            $self->whine(
              $para->[1]{'start_line'},
              "Expected '=item *'"
            );
             # But doesn't need processing.  But it'll block assimilation
             #  of the next para.
          } else {
            die "Unhandled item type $item_type"; # should never happen
          }

          if(@$para == 2) {
            # For the cases where we /didn't/ push to @$para
            if($paras->[0][0] eq '~Para') {
              DEBUG and print STDERR "Assimilating following ~Para content into $over_type item\n";
              push @$para, splice @{shift @$paras},2;
            } else {
              DEBUG and print STDERR "Can't assimilate following ", $paras->[0][0], "\n";
              push @$para, '';  # Just so it's not contentless
            }
          }

        } else {
          die "Unhandled =over type \"$over_type\"?";
          # Shouldn't happen!
        }

        $para_type = 'Plain';
        $para->[0] .= '-' . $over_type;
        # Whew.  Now fall thru and process it.


      } elsif($para_type eq '=extend') {
        # Well, might as well implement it here.
        $self->_ponder_extend($para);
        next;  # and skip
      } elsif($para_type eq '=encoding') {
        # Not actually acted on here, but we catch errors here.
        $self->_handle_encoding_second_level($para);
        next unless $self->keep_encoding_directive;
        $para_type = 'Plain';
      } elsif($para_type eq '~Verbatim') {
        $para->[0] = 'Verbatim';
        $para_type = '?Verbatim';
      } elsif($para_type eq '~Para') {
        $para->[0] = 'Para';
        $para_type = '?Plain';
      } elsif($para_type eq 'Data') {
        $para->[0] = 'Data';
        $para_type = '?Data';
      } elsif( $para_type =~ s/^=//s
        and defined( $para_type = $self->{'accept_directives'}{$para_type} )
      ) {
        DEBUG > 1 and print STDERR " Pondering known directive ${$para}[0] as $para_type\n";
      } else {
        # An unknown directive!
        $seen_legal_directive--;
        DEBUG > 1 and printf STDERR "Unhandled directive %s (Handled: %s)\n",
         $para->[0], join(' ', sort keys %{$self->{'accept_directives'}} )
        ;
        $self->whine(
          $para->[1]{'start_line'},
          "Unknown directive: $para->[0]"
        );

        # And maybe treat it as text instead of just letting it go?
        next;
      }

      if($para_type =~ s/^\?//s) {
        if(! @$curr_open) {  # usual case
          DEBUG and print STDERR "Treating $para_type paragraph as such because stack is empty.\n";
        } else {
          my @fors = grep $_->[0] eq '=for', @$curr_open;
          DEBUG > 1 and print STDERR "Containing fors: ",
            join(',', map $_->[1]{'target'}, @fors), "\n";

          if(! @fors) {
            DEBUG and print STDERR "Treating $para_type paragraph as such because stack has no =for's\n";

          #} elsif(grep $_->[1]{'~resolve'}, @fors) {
          #} elsif(not grep !$_->[1]{'~resolve'}, @fors) {
          } elsif( $fors[-1][1]{'~resolve'} ) {
            # Look to the immediately containing for

            if($para_type eq 'Data') {
              DEBUG and print STDERR "Treating Data paragraph as Plain/Verbatim because the containing =for ($fors[-1][1]{'target'}) is a resolver\n";
              $para->[0] = 'Para';
              $para_type = 'Plain';
            } else {
              DEBUG and print STDERR "Treating $para_type paragraph as such because the containing =for ($fors[-1][1]{'target'}) is a resolver\n";
            }
          } else {
            DEBUG and print STDERR "Treating $para_type paragraph as Data because the containing =for ($fors[-1][1]{'target'}) is a non-resolver\n";
            $para->[0] = $para_type = 'Data';
          }
        }
      }

      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if($para_type eq 'Plain') {
        $self->_ponder_Plain($para);
      } elsif($para_type eq 'Verbatim') {
        $self->_ponder_Verbatim($para);
      } elsif($para_type eq 'Data') {
        $self->_ponder_Data($para);
      } else {
        die "\$para type is $para_type -- how did that happen?";
        # Shouldn't happen.
      }

      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      $para->[0] =~ s/^[~=]//s;

      DEBUG and print STDERR "\n", pretty($para), "\n";

      # traverse the treelet (which might well be just one string scalar)
      $self->{'content_seen'} ||= 1 if   $seen_legal_directive
                                    && ! $self->{'~tried_gen_errata'};
      $self->_traverse_treelet_bit(@$para);
    }
  }

  return;
}

###########################################################################
# The sub-ponderers...



sub _ponder_for {
  my ($self,$para,$curr_open,$paras) = @_;

  # Fake it out as a begin/end
  my $target;

  if(grep $_->[1]{'~ignore'}, @$curr_open) {
    DEBUG > 1 and print STDERR "Ignoring ignorable =for\n";
    return 1;
  }

  for(my $i = 2; $i < @$para; ++$i) {
    if($para->[$i] =~ s/^\s*(\S+)\s*//s) {
      $target = $1;
      last;
    }
  }
  unless(defined $target) {
    $self->whine(
      $para->[1]{'start_line'},
      "=for without a target?"
    );
    return 1;
  }
  DEBUG > 1 and
   print STDERR "Faking out a =for $target as a =begin $target / =end $target\n";

  $para->[0] = 'Data';

  unshift @$paras,
    ['=begin',
      {'start_line' => $para->[1]{'start_line'}, '~really' => '=for'},
      $target,
    ],
    $para,
    ['=end',
      {'start_line' => $para->[1]{'start_line'}, '~really' => '=for'},
      $target,
    ],
  ;

  return 1;
}

sub _ponder_begin {
  my ($self,$para,$curr_open,$paras) = @_;
  my $content = join ' ', splice @$para, 2;
  $content =~ s/^\s+//s;
  $content =~ s/\s+$//s;
  unless(length($content)) {
    $self->whine(
      $para->[1]{'start_line'},
      "=begin without a target?"
    );
    DEBUG and print STDERR "Ignoring targetless =begin\n";
    return 1;
  }

  my ($target, $title) = $content =~ m/^(\S+)\s*(.*)$/;
  $para->[1]{'title'} = $title if ($title);
  $para->[1]{'target'} = $target;  # without any ':'
  $content = $target; # strip off the title

  $content =~ s/^:!/!:/s;
  my $neg;  # whether this is a negation-match
  $neg = 1        if $content =~ s/^!//s;
  my $to_resolve;  # whether to process formatting codes
  $to_resolve = 1 if $content =~ s/^://s;

  my $dont_ignore; # whether this target matches us

  foreach my $target_name (
    split(',', $content, -1),
    $neg ? () : '*'
  ) {
    DEBUG > 2 and
     print STDERR " Considering whether =begin $content matches $target_name\n";
    next unless $self->{'accept_targets'}{$target_name};

    DEBUG > 2 and
     print STDERR "  It DOES match the acceptable target $target_name!\n";
    $to_resolve = 1
      if $self->{'accept_targets'}{$target_name} eq 'force_resolve';
    $dont_ignore = 1;
    $para->[1]{'target_matching'} = $target_name;
    last; # stop looking at other target names
  }

  if($neg) {
    if( $dont_ignore ) {
      $dont_ignore = '';
      delete $para->[1]{'target_matching'};
      DEBUG > 2 and print STDERR " But the leading ! means that this is a NON-match!\n";
    } else {
      $dont_ignore = 1;
      $para->[1]{'target_matching'} = '!';
      DEBUG > 2 and print STDERR " But the leading ! means that this IS a match!\n";
    }
  }

  $para->[0] = '=for';  # Just what we happen to call these, internally
  $para->[1]{'~really'} ||= '=begin';
  $para->[1]{'~ignore'}   = (! $dont_ignore) || 0;
  $para->[1]{'~resolve'}  = $to_resolve || 0;

  DEBUG > 1 and print STDERR " Making note to ", $dont_ignore ? 'not ' : '',
    "ignore contents of this region\n";
  DEBUG > 1 and $dont_ignore and print STDERR " Making note to treat contents as ",
    ($to_resolve ? 'verbatim/plain' : 'data'), " paragraphs\n";
  DEBUG > 1 and print STDERR " (Stack now: ", $self->_dump_curr_open(), ")\n";

  push @$curr_open, $para;
  if(!$dont_ignore or scalar grep $_->[1]{'~ignore'}, @$curr_open) {
    DEBUG > 1 and print STDERR "Ignoring ignorable =begin\n";
  } else {
    $self->{'content_seen'} ||= 1 unless $self->{'~tried_gen_errata'};
    $self->_handle_element_start((my $scratch='for'), $para->[1]);
  }

  return 1;
}

sub _ponder_end {
  my ($self,$para,$curr_open,$paras) = @_;
  my $content = join ' ', splice @$para, 2;
  $content =~ s/^\s+//s;
  $content =~ s/\s+$//s;
  DEBUG and print STDERR "Ogling '=end $content' directive\n";

  unless(length($content)) {
    $self->whine(
      $para->[1]{'start_line'},
      "'=end' without a target?" . (
        ( @$curr_open and $curr_open->[-1][0] eq '=for' )
        ? ( " (Should be \"=end " . $curr_open->[-1][1]{'target'} . '")' )
        : ''
      )
    );
    DEBUG and print STDERR "Ignoring targetless =end\n";
    return 1;
  }

  unless($content =~ m/^\S+$/) {  # i.e., unless it's one word
    $self->whine(
      $para->[1]{'start_line'},
      "'=end $content' is invalid.  (Stack: "
      . $self->_dump_curr_open() . ')'
    );
    DEBUG and print STDERR "Ignoring mistargetted =end $content\n";
    return 1;
  }

  unless(@$curr_open and $curr_open->[-1][0] eq '=for') {
    $self->whine(
      $para->[1]{'start_line'},
      "=end $content without matching =begin.  (Stack: "
      . $self->_dump_curr_open() . ')'
    );
    DEBUG and print STDERR "Ignoring mistargetted =end $content\n";
    return 1;
  }

  unless($content eq $curr_open->[-1][1]{'target'}) {
    $self->whine(
      $para->[1]{'start_line'},
      "=end $content doesn't match =begin "
      . $curr_open->[-1][1]{'target'}
      . ".  (Stack: "
      . $self->_dump_curr_open() . ')'
    );
    DEBUG and print STDERR "Ignoring mistargetted =end $content at line $para->[1]{'start_line'}\n";
    return 1;
  }

  # Else it's okay to close...
  if(grep $_->[1]{'~ignore'}, @$curr_open) {
    DEBUG > 1 and print STDERR "Not firing any event for this =end $content because in an ignored region\n";
    # And that may be because of this to-be-closed =for region, or some
    #  other one, but it doesn't matter.
  } else {
    $curr_open->[-1][1]{'start_line'} = $para->[1]{'start_line'};
      # what's that for?

    $self->{'content_seen'} ||= 1 unless $self->{'~tried_gen_errata'};
    $self->_handle_element_end( my $scratch = 'for', $para->[1]);
  }
  DEBUG > 1 and print STDERR "Popping $curr_open->[-1][0] $curr_open->[-1][1]{'target'} because of =end $content\n";
  pop @$curr_open;

  return 1;
}

sub _ponder_doc_end {
  my ($self,$para,$curr_open,$paras) = @_;
  if(@$curr_open) { # Deal with things left open
    DEBUG and print STDERR "Stack is nonempty at end-document: (",
      $self->_dump_curr_open(), ")\n";

    DEBUG > 9 and print STDERR "Stack: ", pretty($curr_open), "\n";
    unshift @$paras, $self->_closers_for_all_curr_open;
    # Make sure there is exactly one ~end in the parastack, at the end:
    @$paras = grep $_->[0] ne '~end', @$paras;
    push @$paras, $para, $para;
     # We need two -- once for the next cycle where we
     #  generate errata, and then another to be at the end
     #  when that loop back around to process the errata.
    return 1;

  } else {
    DEBUG and print STDERR "Okay, stack is empty now.\n";
  }

  # Try generating errata section, if applicable
  unless($self->{'~tried_gen_errata'}) {
    $self->{'~tried_gen_errata'} = 1;
    my @extras = $self->_gen_errata();
    if(@extras) {
      unshift @$paras, @extras;
      DEBUG and print STDERR "Generated errata... relooping...\n";
      return 1;  # I.e., loop around again to process these fake-o paragraphs
    }
  }

  splice @$paras; # Well, that's that for this paragraph buffer.
  DEBUG and print STDERR "Throwing end-document event.\n";

  $self->_handle_element_end( my $scratch = 'Document' );
  return 1; # Hasta la byebye
}

sub _ponder_pod {
  my ($self,$para,$curr_open,$paras) = @_;
  $self->whine(
    $para->[1]{'start_line'},
    "=pod directives shouldn't be over one line long!  Ignoring all "
     . (@$para - 2) . " lines of content"
  ) if @$para > 3;

  # Content ignored unless 'pod_handler' is set
  if (my $pod_handler = $self->{'pod_handler'}) {
      my ($line_num, $line) = map $_, $para->[1]{'start_line'}, $para->[2];
      $line = $line eq '' ? "=pod" : "=pod $line"; # imitate cut_handler output
      $pod_handler->($line, $line_num, $self);
  }

  # The surrounding methods set content_seen, so let us remain consistent.
  # I do not know why it was not here before -- should it not be here?
  # $self->{'content_seen'} ||= 1 unless $self->{'~tried_gen_errata'};

  return;
}

sub _ponder_over {
  my ($self,$para,$curr_open,$paras) = @_;
  return 1 unless @$paras;
  my $list_type;

  if($paras->[0][0] eq '=item') { # most common case
    $list_type = $self->_get_initial_item_type($paras->[0]);

  } elsif($paras->[0][0] eq '=back') {
    # Ignore empty lists by default
    if ($self->{'parse_empty_lists'}) {
      $list_type = 'empty';
    } else {
      shift @$paras;
      return 1;
    }
  } elsif($paras->[0][0] eq '~end') {
    $self->whine(
      $para->[1]{'start_line'},
      "=over is the last thing in the document?!"
    );
    return 1; # But feh, ignore it.
  } else {
    $list_type = 'block';
  }
  $para->[1]{'~type'} = $list_type;
  push @$curr_open, $para;
   # yes, we reuse the paragraph as a stack item

  my $content = join ' ', splice @$para, 2;
  $para->[1]{'~orig_content'} = $content;
  my $overness;
  if($content =~ m/^\s*$/s) {
    $para->[1]{'indent'} = 4;
  } elsif($content =~ m/^\s*((?:\d*\.)?\d+)\s*$/s) {
    no integer;
    $para->[1]{'indent'} = $1;
    if($1 == 0) {
      $self->whine(
        $para->[1]{'start_line'},
        "Can't have a 0 in =over $content"
      );
      $para->[1]{'indent'} = 4;
    }
  } else {
    $self->whine(
      $para->[1]{'start_line'},
      "=over should be: '=over' or '=over positive_number'"
    );
    $para->[1]{'indent'} = 4;
  }
  DEBUG > 1 and print STDERR "=over found of type $list_type\n";

  $self->{'content_seen'} ||= 1 unless $self->{'~tried_gen_errata'};
  $self->_handle_element_start((my $scratch = 'over-' . $list_type), $para->[1]);

  return;
}

sub _ponder_back {
  my ($self,$para,$curr_open,$paras) = @_;
  # TODO: fire off </item-number> or </item-bullet> or </item-text> ??

  my $content = join ' ', splice @$para, 2;
  if($content =~ m/\S/) {
    $self->whine(
      $para->[1]{'start_line'},
      "=back doesn't take any parameters, but you said =back $content"
    );
  }

  if(@$curr_open and $curr_open->[-1][0] eq '=over') {
    DEBUG > 1 and print STDERR "=back happily closes matching =over\n";
    # Expected case: we're closing the most recently opened thing
    #my $over = pop @$curr_open;
    $self->{'content_seen'} ||= 1 unless $self->{'~tried_gen_errata'};
    $self->_handle_element_end( my $scratch =
      'over-' . ( (pop @$curr_open)->[1]{'~type'} ), $para->[1]
    );
  } else {
    DEBUG > 1 and print STDERR "=back found without a matching =over.  Stack: (",
        join(', ', map $_->[0], @$curr_open), ").\n";
    $self->whine(
      $para->[1]{'start_line'},
      '=back without =over'
    );
    return 1; # and ignore it
  }
}

sub _ponder_item {
  my ($self,$para,$curr_open,$paras) = @_;
  my $over;
  unless(@$curr_open and
         $over = (grep { $_->[0] eq '=over' } @$curr_open)[-1]) {
    $self->whine(
      $para->[1]{'start_line'},
      "'=item' outside of any '=over'"
    );
    unshift @$paras,
      ['=over', {'start_line' => $para->[1]{'start_line'}}, ''],
      $para
    ;
    return 1;
  }


  my $over_type = $over->[1]{'~type'};

  if(!$over_type) {
    # Shouldn't happen1
    die "Typeless over in stack, starting at line "
     . $over->[1]{'start_line'};

  } elsif($over_type eq 'block') {
    unless($curr_open->[-1][1]{'~bitched_about'}) {
      $curr_open->[-1][1]{'~bitched_about'} = 1;
      $self->whine(
        $curr_open->[-1][1]{'start_line'},
        "You can't have =items (as at line "
        . $para->[1]{'start_line'}
        . ") unless the first thing after the =over is an =item"
      );
    }
    # Just turn it into a paragraph and reconsider it
    $para->[0] = '~Para';
    unshift @$paras, $para;
    return 1;

  } elsif($over_type eq 'text') {
    my $item_type = $self->_get_item_type($para);
      # That kills the content of the item if it's a number or bullet.
    DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

    if($item_type eq 'text') {
      # Nothing special needs doing for 'text'
    } elsif($item_type eq 'number' or $item_type eq 'bullet') {
      $self->whine(
          $para->[1]{'start_line'},
          "Expected text after =item, not a $item_type"
      );
      # Undo our clobbering:
      push @$para, $para->[1]{'~orig_content'};
      delete $para->[1]{'number'};
       # Only a PROPER item-number element is allowed
       #  to have a number attribute.
    } else {
      die "Unhandled item type $item_type"; # should never happen
    }

    # =item-text thingies don't need any assimilation, it seems.

  } elsif($over_type eq 'number') {
    my $item_type = $self->_get_item_type($para);
      # That kills the content of the item if it's a number or bullet.
    DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

    my $expected_value = ++ $curr_open->[-1][1]{'~counter'};

    if($item_type eq 'bullet') {
      # Hm, it's not numeric.  Correct for this.
      $para->[1]{'number'} = $expected_value;
      $self->whine(
        $para->[1]{'start_line'},
        "Expected '=item $expected_value'"
      );
      push @$para, $para->[1]{'~orig_content'};
        # restore the bullet, blocking the assimilation of next para

    } elsif($item_type eq 'text') {
      # Hm, it's not numeric.  Correct for this.
      $para->[1]{'number'} = $expected_value;
      $self->whine(
        $para->[1]{'start_line'},
        "Expected '=item $expected_value'"
      );
      # Text content will still be there and will block next ~Para

    } elsif($item_type ne 'number') {
      die "Unknown item type $item_type"; # should never happen

    } elsif($expected_value == $para->[1]{'number'}) {
      DEBUG > 1 and print STDERR " Numeric item has the expected value of $expected_value\n";

    } else {
      DEBUG > 1 and print STDERR " Numeric item has ", $para->[1]{'number'},
       " instead of the expected value of $expected_value\n";
      $self->whine(
        $para->[1]{'start_line'},
        "You have '=item " . $para->[1]{'number'} .
        "' instead of the expected '=item $expected_value'"
      );
      $para->[1]{'number'} = $expected_value;  # correcting!!
    }

    if(@$para == 2) {
      # For the cases where we /didn't/ push to @$para
      if($paras->[0][0] eq '~Para') {
        DEBUG and print STDERR "Assimilating following ~Para content into $over_type item\n";
        push @$para, splice @{shift @$paras},2;
      } else {
        DEBUG and print STDERR "Can't assimilate following ", $paras->[0][0], "\n";
        push @$para, '';  # Just so it's not contentless
      }
    }


  } elsif($over_type eq 'bullet') {
    my $item_type = $self->_get_item_type($para);
      # That kills the content of the item if it's a number or bullet.
    DEBUG and print STDERR " Item is of type ", $para->[0], " under $over_type\n";

    if($item_type eq 'bullet') {
      # as expected!

      if( $para->[1]{'~_freaky_para_hack'} ) {
        DEBUG and print STDERR "Accomodating '=item * Foo' tolerance hack.\n";
        push @$para, $para->[1]{'~_freaky_para_hack'};
      }

    } elsif($item_type eq 'number') {
      $self->whine(
        $para->[1]{'start_line'},
        "Expected '=item *'"
      );
      push @$para, $para->[1]{'~orig_content'};
       # and block assimilation of the next paragraph
      delete $para->[1]{'number'};
       # Only a PROPER item-number element is allowed
       #  to have a number attribute.
    } elsif($item_type eq 'text') {
      $self->whine(
        $para->[1]{'start_line'},
        "Expected '=item *'"
      );
       # But doesn't need processing.  But it'll block assimilation
       #  of the next para.
    } else {
      die "Unhandled item type $item_type"; # should never happen
    }

    if(@$para == 2) {
      # For the cases where we /didn't/ push to @$para
      if($paras->[0][0] eq '~Para') {
        DEBUG and print STDERR "Assimilating following ~Para content into $over_type item\n";
        push @$para, splice @{shift @$paras},2;
      } else {
        DEBUG and print STDERR "Can't assimilate following ", $paras->[0][0], "\n";
        push @$para, '';  # Just so it's not contentless
      }
    }

  } else {
    die "Unhandled =over type \"$over_type\"?";
    # Shouldn't happen!
  }
  $para->[0] .= '-' . $over_type;

  return;
}

sub _ponder_Plain {
  my ($self,$para) = @_;
  DEBUG and print STDERR " giving plain treatment...\n";
  unless( @$para == 2 or ( @$para == 3 and $para->[2] eq '' )
    or $para->[1]{'~cooked'}
  ) {
    push @$para,
    @{$self->_make_treelet(
      join("\n", splice(@$para, 2)),
      $para->[1]{'start_line'}
    )};
  }
  # Empty paragraphs don't need a treelet for any reason I can see.
  # And precooked paragraphs already have a treelet.
  return;
}

sub _ponder_Verbatim {
  my ($self,$para) = @_;
  DEBUG and print STDERR " giving verbatim treatment...\n";

  $para->[1]{'xml:space'} = 'preserve';

  unless ($self->{'_output_is_for_JustPod'}) {
    # Fix illegal settings for expand_verbatim_tabs()
    # This is because this module doesn't do input error checking, but khw
    # doesn't want to add yet another instance of that.
    $self->expand_verbatim_tabs(8)
                            if ! defined $self->expand_verbatim_tabs()
                            ||   $self->expand_verbatim_tabs() =~ /\D/;

    my $indent = $self->strip_verbatim_indent;
    if ($indent && ref $indent eq 'CODE') {
        my @shifted = (shift @{$para}, shift @{$para});
        $indent = $indent->($para);
        unshift @{$para}, @shifted;
    }

    for(my $i = 2; $i < @$para; $i++) {
      foreach my $line ($para->[$i]) { # just for aliasing
        # Strip indentation.
        $line =~ s/^\Q$indent// if $indent;
        next unless $self->expand_verbatim_tabs;

            # This is commented out because of github issue #85, and the
            # current maintainers don't know why it was there in the first
            # place.
            #&& !($self->{accept_codes} && $self->{accept_codes}{VerbatimFormatted});
        while( $line =~
          # Sort of adapted from Text::Tabs.
          s/^([^\t]*)(\t+)/$1.(" " x ((length($2)
                                       * $self->expand_verbatim_tabs)
                                       -(length($1)&7)))/e
        ) {}

        # TODO: whinge about (or otherwise treat) unindented or overlong lines

      }
    }
  }

  # Now the VerbatimFormatted hoodoo...
  if( $self->{'accept_codes'} and
      $self->{'accept_codes'}{'VerbatimFormatted'}
  ) {
    while(@$para > 3 and $para->[-1] !~ m/\S/) { pop @$para }
     # Kill any number of terminal newlines
    $self->_verbatim_format($para);
  } elsif ($self->{'codes_in_verbatim'}) {
    push @$para,
    @{$self->_make_treelet(
      join("\n", splice(@$para, 2)),
      $para->[1]{'start_line'}, $para->[1]{'xml:space'}
    )};
    $para->[-1] =~ s/\n+$//s; # Kill any number of terminal newlines
  } else {
    push @$para, join "\n", splice(@$para, 2) if @$para > 3;
    $para->[-1] =~ s/\n+$//s; # Kill any number of terminal newlines
  }
  return;
}

sub _ponder_Data {
  my ($self,$para) = @_;
  DEBUG and print STDERR " giving data treatment...\n";
  $para->[1]{'xml:space'} = 'preserve';
  push @$para, join "\n", splice(@$para, 2) if @$para > 3;
  return;
}




###########################################################################

sub _traverse_treelet_bit {  # for use only by the routine above
  my($self, $name) = splice @_,0,2;

  my $scratch;
  $self->_handle_element_start(($scratch=$name), shift @_);

  while (@_) {
    my $x = shift;
    if (ref($x)) {
      &_traverse_treelet_bit($self, @$x);
    } else {
      $x .= shift while @_ && !ref($_[0]);
      $self->_handle_text($x);
    }
  }

  $self->_handle_element_end($scratch=$name);
  return;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub _closers_for_all_curr_open {
  my $self = $_[0];
  my @closers;
  foreach my $still_open (@{  $self->{'curr_open'} || return  }) {
    my @copy = @$still_open;
    $copy[1] = {%{ $copy[1] }};
    #$copy[1]{'start_line'} = -1;
    if($copy[0] eq '=for') {
      $copy[0] = '=end';
    } elsif($copy[0] eq '=over') {
      $self->whine(
        $still_open->[1]{start_line} ,
        "=over without closing =back"
      );

      $copy[0] = '=back';
    } else {
      die "I don't know how to auto-close an open $copy[0] region";
    }

    unless( @copy > 2 ) {
      push @copy, $copy[1]{'target'};
      $copy[-1] = '' unless defined $copy[-1];
       # since =over's don't have targets
    }

    $copy[1]{'fake-closer'} = 1;

    DEBUG and print STDERR "Queuing up fake-o event: ", pretty(\@copy), "\n";
    unshift @closers, \@copy;
  }
  return @closers;
}

#--------------------------------------------------------------------------

sub _verbatim_format {
  my($it, $p) = @_;

  my $formatting;

  for(my $i = 2; $i < @$p; $i++) { # work backwards over the lines
    DEBUG and print STDERR "_verbatim_format appends a newline to $i: $p->[$i]\n";
    $p->[$i] .= "\n";
     # Unlike with simple Verbatim blocks, we don't end up just doing
     # a join("\n", ...) on the contents, so we have to append a
     # newline to every line, and then nix the last one later.
  }

  if( DEBUG > 4 ) {
    print STDERR "<<\n";
    for(my $i = $#$p; $i >= 2; $i--) { # work backwards over the lines
      print STDERR "_verbatim_format $i: $p->[$i]";
    }
    print STDERR ">>\n";
  }

  for(my $i = $#$p; $i > 2; $i--) {
    # work backwards over the lines, except the first (#2)

    #next unless $p->[$i]   =~ m{^#:([ \^\/\%]*)\n?$}s
    #        and $p->[$i-1] !~ m{^#:[ \^\/\%]*\n?$}s;
     # look at a formatty line preceding a nonformatty one
    DEBUG > 5 and print STDERR "Scrutinizing line $i: $$p[$i]\n";
    if($p->[$i]   =~ m{^#:([ \^\/\%]*)\n?$}s) {
      DEBUG > 5 and print STDERR "  It's a formatty line.  ",
       "Peeking at previous line ", $i-1, ": $$p[$i-1]: \n";

      if( $p->[$i-1] =~ m{^#:[ \^\/\%]*\n?$}s ) {
        DEBUG > 5 and print STDERR "  Previous line is formatty!  Skipping this one.\n";
        next;
      } else {
        DEBUG > 5 and print STDERR "  Previous line is non-formatty!  Yay!\n";
      }
    } else {
      DEBUG > 5 and print STDERR "  It's not a formatty line.  Ignoring\n";
      next;
    }

    # A formatty line has to have #: in the first two columns, and uses
    # "^" to mean bold, "/" to mean underline, and "%" to mean bold italic.
    # Example:
    #   What do you want?  i like pie. [or whatever]
    # #:^^^^^^^^^^^^^^^^^              /////////////


    DEBUG > 4 and print STDERR "_verbatim_format considers:\n<$p->[$i-1]>\n<$p->[$i]>\n";

    $formatting = '  ' . $1;
    $formatting =~ s/\s+$//s; # nix trailing whitespace
    unless(length $formatting and $p->[$i-1] =~ m/\S/) { # no-op
      splice @$p,$i,1; # remove this line
      $i--; # don't consider next line
      next;
    }

    if( length($formatting) >= length($p->[$i-1]) ) {
      $formatting = substr($formatting, 0, length($p->[$i-1]) - 1) . ' ';
    } else {
      $formatting .= ' ' x (length($p->[$i-1]) - length($formatting));
    }
    # Make $formatting and the previous line be exactly the same length,
    # with $formatting having a " " as the last character.

    DEBUG > 4 and print STDERR "Formatting <$formatting>    on <", $p->[$i-1], ">\n";


    my @new_line;
    while( $formatting =~ m{\G(( +)|(\^+)|(\/+)|(\%+))}g ) {
      #print STDERR "Format matches $1\n";

      if($2) {
        #print STDERR "SKIPPING <$2>\n";
        push @new_line,
          substr($p->[$i-1], pos($formatting)-length($1), length($1));
      } else {
        #print STDERR "SNARING $+\n";
        push @new_line, [
          (
            $3 ? 'VerbatimB'  :
            $4 ? 'VerbatimI'  :
            $5 ? 'VerbatimBI' : die("Should never get called")
          ), {},
          substr($p->[$i-1], pos($formatting)-length($1), length($1))
        ];
        #print STDERR "Formatting <$new_line[-1][-1]> as $new_line[-1][0]\n";
      }
    }
    my @nixed =
      splice @$p, $i-1, 2, @new_line; # replace myself and the next line
    DEBUG > 10 and print STDERR "Nixed count: ", scalar(@nixed), "\n";

    DEBUG > 6 and print STDERR "New version of the above line is these tokens (",
      scalar(@new_line), "):",
      map( ref($_)?"<@$_> ":"<$_>", @new_line ), "\n";
    $i--; # So the next line we scrutinize is the line before the one
          #  that we just went and formatted
  }

  $p->[0] = 'VerbatimFormatted';

  # Collapse adjacent text nodes, just for kicks.
  for( my $i = 2; $i > $#$p; $i++ ) { # work forwards over the tokens except for the last
    if( !ref($p->[$i]) and !ref($p->[$i + 1]) ) {
      DEBUG > 5 and print STDERR "_verbatim_format merges {$p->[$i]} and {$p->[$i+1]}\n";
      $p->[$i] .= splice @$p, $i+1, 1; # merge
      --$i;  # and back up
    }
  }

  # Now look for the last text token, and remove the terminal newline
  for( my $i = $#$p; $i >= 2; $i-- ) {
    # work backwards over the tokens, even the first
    if( !ref($p->[$i]) ) {
      if($p->[$i] =~ s/\n$//s) {
        DEBUG > 5 and print STDERR "_verbatim_format killed the terminal newline on #$i: {$p->[$i]}, after {$p->[$i-1]}\n";
      } else {
        DEBUG > 5 and print STDERR
         "No terminal newline on #$i: {$p->[$i]}, after {$p->[$i-1]} !?\n";
      }
      last; # we only want the next one
    }
  }

  return;
}


#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


sub _treelet_from_formatting_codes {
  # Given a paragraph, returns a treelet.  Full of scary tokenizing code.
  #  Like [ '~Top', {'start_line' => $start_line},
  #            "I like ",
  #            [ 'B', {}, "pie" ],
  #            "!"
  #       ]
  # This illustrates the general format of a treelet.  It is an array:
  #     [0]       is a scalar indicating its type.  In the example above, the
  #               types are '~Top' and 'B'
  #     [1]       is a hash of various flags about it, possibly empty
  #     [2] - [N] are an ordered list of the subcomponents of the treelet.
  #               Scalars are literal text, refs are sub-treelets, to
  #               arbitrary levels.  Stringifying a treelet will recursively
  #               stringify the sub-treelets, concatentating everything
  #               together to form the exact text of the treelet.

  my($self, $para, $start_line, $preserve_space) = @_;

  my $treelet = ['~Top', {'start_line' => $start_line},];

  unless ($preserve_space || $self->{'preserve_whitespace'}) {
    $para =~ s/\s+/ /g; # collapse and trim all whitespace first.
    $para =~ s/ $//;
    $para =~ s/^ //;
  }

  # Only apparent problem the above code is that N<<  >> turns into
  # N<< >>.  But then, word wrapping does that too!  So don't do that!


  # As a Start-code is encountered, the number of opening bracket '<'
  # characters minus 1 is pushed onto @stack (so 0 means a single bracket,
  # etc).  When closing brackets are found in the text, at least this number
  # (plus the 1) will be required to mean the Start-code is terminated.  When
  # those are found, @stack is popped.
  my @stack;

  my @lineage = ($treelet);
  my $raw = ''; # raw content of L<> fcode before splitting/processing
    # XXX 'raw' is not 100% accurate: all surrounding whitespace is condensed
    # into just 1 ' '. Is this the regex's doing or 'raw's?  Answer is it's
    # the 'collapse and trim all whitespace first' lines just above.
  my $inL = 0;

  DEBUG > 4 and print STDERR "Paragraph:\n$para\n\n";

  # Here begins our frightening tokenizer RE.  The following regex matches
  # text in four main parts:
  #
  #  * Start-codes.  The first alternative matches C< or C<<, the latter
  #    followed by some whitespace.  $1 will hold the entire start code
  #    (including any space following a multiple-angle-bracket delimiter),
  #    and $2 will hold only the additional brackets past the first in a
  #    multiple-bracket delimiter.  length($2) + 1 will be the number of
  #    closing brackets we have to find.
  #
  #  * Closing brackets.  Match some amount of whitespace followed by
  #    multiple close brackets.  The logic to see if this closes anything
  #    is down below.  Note that in order to parse C<<  >> correctly, we
  #    have to use look-behind (?<=\s\s), since the match of the starting
  #    code will have consumed the whitespace.
  #
  #  * A single closing bracket, to close a simple code like C<>.
  #
  #  * Something that isn't a start or end code.  We have to be careful
  #    about accepting whitespace, since perlpodspec says that any whitespace
  #    before a multiple-bracket closing delimiter should be ignored.
  #
  while($para =~
    m/\G
      (?:
        # Match starting codes, including the whitespace following a
        # multiple-delimiter start code.  $1 gets the whole start code and
        # $2 gets all but one of the <s in the multiple-bracket case.
        ([A-Z]<(?:(<+)\s+)?)
        |
        # Match multiple-bracket end codes.  $3 gets the whitespace that
        # should be discarded before an end bracket but kept in other cases
        # and $4 gets the end brackets themselves.  ($3 can be empty if the
        # construct is empty, like C<<  >>, and all the white-space has been
        # gobbled up already, considered to be space after the opening
        # bracket.  In this case we use look-behind to verify that there are
        # at least 2 spaces in a row before the ">".)
        (\s+|(?<=\s\s))(>{2,})
        |
        (\s?>)          # $5: simple end-codes
        |
        (               # $6: stuff containing no start-codes or end-codes
          (?:
            [^A-Z\s>]
            |
            (?:
              [A-Z](?!<)
            )
            |
            # whitespace is ok, but we don't want to eat the whitespace before
            # a multiple-bracket end code.
            # NOTE: we may still have problems with e.g. S<<    >>
            (?:
              \s(?!\s*>{2,})
            )
          )+
        )
      )
    /xgo
  ) {
    DEBUG > 4 and print STDERR "\nParagraphic tokenstack = (@stack)\n";
    if(defined $1) {
      my $bracket_count;    # How many '<<<' in a row this has.  Needed for
                            # Pod::Simple::JustPod
      if(defined $2) {
        DEBUG > 3 and print STDERR "Found complex start-text code \"$1\"\n";
        $bracket_count = length($2) + 1;
        push @stack, $bracket_count; # length of the necessary complex
                                     # end-code string
      } else {
        DEBUG > 3 and print STDERR "Found simple start-text code \"$1\"\n";
        push @stack, 0;  # signal that we're looking for simple
        $bracket_count = 1;
      }
      my $code = substr($1,0,1);
      if ('L' eq $code) {
        if ($inL) {
            $raw .= $1;
            $self->scream( $start_line,
                           'Nested L<> are illegal.  Pretending inner one is '
                         . 'X<...> so can continue looking for other errors.');
            $code = "X";
        }
        else {
            $raw = ""; # reset raw content accumulator
            $inL = @stack;
        }
      } else {
        $raw .= $1 if $inL;
      }
      push @lineage, [ $code, {}, ];  # new node object

      # Tell Pod::Simple::JustPod how many brackets there were, but to save
      # space, not in the most usual case of there was just 1.  It can be
      # inferred by the absence of this element.  Similarly, if there is more
      # than one bracket, extract the white space between the final bracket
      # and the real beginning of the interior.  Save that if it isn't just a
      # single space
      if ($self->{'_output_is_for_JustPod'} && $bracket_count > 1) {
        $lineage[-1][1]{'~bracket_count'} = $bracket_count;
        my $lspacer = substr($1, 1 + $bracket_count);
        $lineage[-1][1]{'~lspacer'} = $lspacer if $lspacer ne " ";
      }
      push @{ $lineage[-2] }, $lineage[-1];
    } elsif(defined $4) {
      DEBUG > 3 and print STDERR "Found apparent complex end-text code \"$3$4\"\n";
      # This is where it gets messy...
      if(! @stack) {
        # We saw " >>>>" but needed nothing.  This is ALL just stuff then.
        DEBUG > 4 and print STDERR " But it's really just stuff.\n";
        push @{ $lineage[-1] }, $3, $4;
        next;
      } elsif(!$stack[-1]) {
        # We saw " >>>>" but needed only ">".  Back pos up.
        DEBUG > 4 and print STDERR " And that's more than we needed to close simple.\n";
        push @{ $lineage[-1] }, $3; # That was a for-real space, too.
        pos($para) = pos($para) - length($4) + 1;
      } elsif($stack[-1] == length($4)) {
        # We found " >>>>", and it was exactly what we needed.  Commonest case.
        DEBUG > 4 and print STDERR " And that's exactly what we needed to close complex.\n";
      } elsif($stack[-1] < length($4)) {
        # We saw " >>>>" but needed only " >>".  Back pos up.
        DEBUG > 4 and print STDERR " And that's more than we needed to close complex.\n";
        pos($para) = pos($para) - length($4) + $stack[-1];
      } else {
        # We saw " >>>>" but needed " >>>>>>".  So this is all just stuff!
        DEBUG > 4 and print STDERR " But it's really just stuff, because we needed more.\n";
        push @{ $lineage[-1] }, $3, $4;
        next;
      }
      #print STDERR "\nHOOBOY ", scalar(@{$lineage[-1]}), "!!!\n";

      if ($3 ne " " && $self->{'_output_is_for_JustPod'}) {
        if ($3 ne "") {
          $lineage[-1][1]{'~rspacer'} = $3;
        }
        elsif ($lineage[-1][1]{'~lspacer'} eq "  ") {

          # Here we had something like C<<  >> which was a false positive
          delete $lineage[-1][1]{'~lspacer'};
        }
        else {
          $lineage[-1][1]{'~rspacer'}
                                = substr($lineage[-1][1]{'~lspacer'}, -1, 1);
          chop $lineage[-1][1]{'~lspacer'};
        }
      }

      push @{ $lineage[-1] }, '' if 2 == @{ $lineage[-1] };
      # Keep the element from being childless

      if ($inL == @stack) {
        $lineage[-1][1]{'raw'} = $raw;
        $inL = 0;
      }

      pop @stack;
      pop @lineage;

      $raw .= $3.$4 if $inL;

    } elsif(defined $5) {
      DEBUG > 3 and print STDERR "Found apparent simple end-text code \"$5\"\n";

      if(@stack and ! $stack[-1]) {
        # We're indeed expecting a simple end-code
        DEBUG > 4 and print STDERR " It's indeed an end-code.\n";

        if(length($5) == 2) { # There was a space there: " >"
          push @{ $lineage[-1] }, ' ';
        } elsif( 2 == @{ $lineage[-1] } ) { # Closing a childless element
          push @{ $lineage[-1] }, ''; # keep it from being really childless
        }

        if ($inL == @stack) {
          $lineage[-1][1]{'raw'} = $raw;
          $inL = 0;
        }

        pop @stack;
        pop @lineage;
      } else {
        DEBUG > 4 and print STDERR " It's just stuff.\n";
        push @{ $lineage[-1] }, $5;
      }

      $raw .= $5 if $inL;

    } elsif(defined $6) {
      DEBUG > 3 and print STDERR "Found stuff \"$6\"\n";
      push @{ $lineage[-1] }, $6;
      $raw .= $6 if $inL;
        # XXX does not capture multiplace whitespaces -- 'raw' ends up with
        #     at most 1 leading/trailing whitespace, why not all of it?
        #     Answer, because we deliberately trimmed it above

    } else {
      # should never ever ever ever happen
      DEBUG and print STDERR "AYYAYAAAAA at line ", __LINE__, "\n";
      die "SPORK 512512!";
    }
  }

  if(@stack) { # Uhoh, some sequences weren't closed.
    my $x= "...";
    while(@stack) {
      push @{ $lineage[-1] }, '' if 2 == @{ $lineage[-1] };
      # Hmmmmm!

      my $code         = (pop @lineage)->[0];
      my $ender_length =  pop @stack;
      if($ender_length) {
        --$ender_length;
        $x = $code . ("<" x $ender_length) . " $x " . (">" x $ender_length);
      } else {
        $x = $code . "<$x>";
      }
    }
    DEBUG > 1 and print STDERR "Unterminated $x sequence\n";
    $self->whine($start_line,
      "Unterminated $x sequence",
    );
  }

  return $treelet;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub text_content_of_treelet {  # method: $parser->text_content_of_treelet($lol)
  return stringify_lol($_[1]);
}

sub stringify_lol {  # function: stringify_lol($lol)
  my $string_form = '';
  _stringify_lol( $_[0] => \$string_form );
  return $string_form;
}

sub _stringify_lol {  # the real recursor
  my($lol, $to) = @_;
  for(my $i = 2; $i < @$lol; ++$i) {
    if( ref($lol->[$i] || '') and UNIVERSAL::isa($lol->[$i], 'ARRAY') ) {
      _stringify_lol( $lol->[$i], $to);  # recurse!
    } else {
      $$to .= $lol->[$i];
    }
  }
  return;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub _dump_curr_open { # return a string representation of the stack
  my $curr_open = $_[0]{'curr_open'};

  return '[empty]' unless @$curr_open;
  return join '; ',
    map {;
           ($_->[0] eq '=for')
             ? ( ($_->[1]{'~really'} || '=over')
               . ' ' . $_->[1]{'target'})
             : $_->[0]
        }
    @$curr_open
  ;
}

###########################################################################
my %pretty_form = (
  "\a" => '\a', # ding!
  "\b" => '\b', # BS
  "\e" => '\e', # ESC
  "\f" => '\f', # FF
  "\t" => '\t', # tab
  "\cm" => '\cm',
  "\cj" => '\cj',
  "\n" => '\n', # probably overrides one of either \cm or \cj
  '"' => '\"',
  '\\' => '\\\\',
  '$' => '\\$',
  '@' => '\\@',
  '%' => '\\%',
  '#' => '\\#',
);

sub pretty { # adopted from Class::Classless
  # Not the most brilliant routine, but passable.
  # Don't give it a cyclic data structure!
  my @stuff = @_; # copy
  my $x;
  my $out =
    # join ",\n" .
    join ", ",
    map {;
    if(!defined($_)) {
      "undef";
    } elsif(ref($_) eq 'ARRAY' or ref($_) eq 'Pod::Simple::LinkSection') {
      $x = "[ " . pretty(@$_) . " ]" ;
      $x;
    } elsif(ref($_) eq 'SCALAR') {
      $x = "\\" . pretty($$_) ;
      $x;
    } elsif(ref($_) eq 'HASH') {
      my $hr = $_;
      $x = "{" . join(", ",
        map(pretty($_) . '=>' . pretty($hr->{$_}),
            sort keys %$hr ) ) . "}" ;
      $x;
    } elsif(!length($_)) { q{''} # empty string
    } elsif(
      $_ eq '0' # very common case
      or(
         m/^-?(?:[123456789]\d*|0)(?:\.\d+)?$/s
         and $_ ne '-0' # the strange case that RE lets thru
      )
    ) { $_;
    } else {
        # Yes, explicitly name every character desired. There are shorcuts one
        # could make, but I (Karl Williamson) was afraid that some Perl
        # releases would have bugs in some of them. For example [A-Z] works
        # even on EBCDIC platforms to match exactly the 26 uppercase English
        # letters, but I don't know if it has always worked without bugs. It
        # seemed safest just to list the characters.
        # s<([^\x20\x21\x23\x27-\x3F\x41-\x5B\x5D-\x7E])>
        s<([^ !"#'()*+,\-./0123456789:;\<=\>?ABCDEFGHIJKLMNOPQRSTUVWXYZ\[\]^_`abcdefghijklmnopqrstuvwxyz{|}~])>
         <$pretty_form{$1} || '\\x{'.sprintf("%x", ord($1)).'}'>eg;
         #<$pretty_form{$1} || '\\x'.(unpack("H2",$1))>eg;
      qq{"$_"};
    }
  } @stuff;
  # $out =~ s/\n */ /g if length($out) < 75;
  return $out;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

# A rather unsubtle method of blowing away all the state information
# from a parser object so it can be reused. Provided as a utility for
# backward compatibility in Pod::Man, etc. but not recommended for
# general use.

sub reinit {
  my $self = shift;
  foreach (qw(source_dead source_filename doc_has_started
start_of_pod_block content_seen last_was_blank paras curr_open
line_count pod_para_count in_pod ~tried_gen_errata all_errata errata errors_seen
Title)) {

    delete $self->{$_};
  }
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
1;

