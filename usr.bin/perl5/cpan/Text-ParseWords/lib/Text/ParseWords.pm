package Text::ParseWords;

use strict;
use warnings;
require 5.006;
our $VERSION = "3.31";


use Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(shellwords quotewords nested_quotewords parse_line);
our @EXPORT_OK = qw(old_shellwords);
our $PERL_SINGLE_QUOTE;


sub shellwords {
    my (@lines) = @_;
    my @allwords;

    foreach my $line (@lines) {
	$line =~ s/^\s+//;
	my @words = parse_line('\s+', 0, $line);
	pop @words if (@words and !defined $words[-1]);
	return() unless (@words || !length($line));
	push(@allwords, @words);
    }
    return(@allwords);
}



sub quotewords {
    my($delim, $keep, @lines) = @_;
    my($line, @words, @allwords);

    foreach $line (@lines) {
	@words = parse_line($delim, $keep, $line);
	return() unless (@words || !length($line));
	push(@allwords, @words);
    }
    return(@allwords);
}



sub nested_quotewords {
    my($delim, $keep, @lines) = @_;
    my($i, @allwords);

    for ($i = 0; $i < @lines; $i++) {
	@{$allwords[$i]} = parse_line($delim, $keep, $lines[$i]);
	return() unless (@{$allwords[$i]} || !length($lines[$i]));
    }
    return(@allwords);
}



sub parse_line {
    my($delimiter, $keep, $line) = @_;
    my($word, @pieces);

    no warnings 'uninitialized';	# we will be testing undef strings

    while (length($line)) {
        # This pattern is optimised to be stack conservative on older perls.
        # Do not refactor without being careful and testing it on very long strings.
        # See Perl bug #42980 for an example of a stack busting input.
        $line =~ s/^
                    (?: 
                        # double quoted string
                        (")                             # $quote
                        ((?>[^\\"]*(?:\\.[^\\"]*)*))"   # $quoted 
		    |	# --OR--
                        # singe quoted string
                        (')                             # $quote
                        ((?>[^\\']*(?:\\.[^\\']*)*))'   # $quoted
                    |   # --OR--
                        # unquoted string
		        (                               # $unquoted 
                            (?:\\.|[^\\"'])*?           
                        )		
                        # followed by
		        (                               # $delim
                            \Z(?!\n)                    # EOL
                        |   # --OR--
                            (?-x:$delimiter)            # delimiter
                        |   # --OR--                    
                            (?!^)(?=["'])               # a quote
                        )  
		    )//xs or return;		# extended layout                  
        my ($quote, $quoted, $unquoted, $delim) = (($1 ? ($1,$2) : ($3,$4)), $5, $6);


	return() unless( defined($quote) || length($unquoted) || length($delim));

        if ($keep) {
	    $quoted = "$quote$quoted$quote";
	}
        else {
	    $unquoted =~ s/\\(.)/$1/sg;
	    if (defined $quote) {
		$quoted =~ s/\\(.)/$1/sg if ($quote eq '"');
		$quoted =~ s/\\([\\'])/$1/g if ( $PERL_SINGLE_QUOTE && $quote eq "'");
            }
	}
        $word .= substr($line, 0, 0);	# leave results tainted
        $word .= defined $quote ? $quoted : $unquoted;
 
        if (length($delim)) {
            push(@pieces, $word);
            push(@pieces, $delim) if ($keep eq 'delimiters');
            undef $word;
        }
        if (!length($line)) {
            push(@pieces, $word);
	}
    }
    return(@pieces);
}



sub old_shellwords {

    # Usage:
    #	use ParseWords;
    #	@words = old_shellwords($line);
    #	or
    #	@words = old_shellwords(@lines);
    #	or
    #	@words = old_shellwords();	# defaults to $_ (and clobbers it)

    no warnings 'uninitialized';	# we will be testing undef strings
    local *_ = \join('', @_) if @_;
    my (@words, $snippet);

    s/\A\s+//;
    while ($_ ne '') {
	my $field = substr($_, 0, 0);	# leave results tainted
	for (;;) {
	    if (s/\A"(([^"\\]|\\.)*)"//s) {
		($snippet = $1) =~ s#\\(.)#$1#sg;
	    }
	    elsif (/\A"/) {
		require Carp;
		Carp::carp("Unmatched double quote: $_");
		return();
	    }
	    elsif (s/\A'(([^'\\]|\\.)*)'//s) {
		($snippet = $1) =~ s#\\(.)#$1#sg;
	    }
	    elsif (/\A'/) {
		require Carp;
		Carp::carp("Unmatched single quote: $_");
		return();
	    }
	    elsif (s/\A\\(.?)//s) {
		$snippet = $1;
	    }
	    elsif (s/\A([^\s\\'"]+)//) {
		$snippet = $1;
	    }
	    else {
		s/\A\s+//;
		last;
	    }
	    $field .= $snippet;
	}
	push(@words, $field);
    }
    return @words;
}

1;

__END__

=head1 NAME

Text::ParseWords - parse text into an array of tokens or array of arrays

=head1 SYNOPSIS

  use Text::ParseWords;
  @lists = nested_quotewords($delim, $keep, @lines);
  @words = quotewords($delim, $keep, @lines);
  @words = shellwords(@lines);
  @words = parse_line($delim, $keep, $line);
  @words = old_shellwords(@lines); # DEPRECATED!

=head1 DESCRIPTION

The C<nested_quotewords()> and C<quotewords()> functions accept a delimiter 
(which can be a regular expression)
and a list of lines and then breaks those lines up into a list of
words ignoring delimiters that appear inside quotes.  C<quotewords()>
returns all of the tokens in a single long list, while C<nested_quotewords()>
returns a list of token lists corresponding to the elements of C<@lines>.
C<parse_line()> does tokenizing on a single string.  The C<*quotewords()>
functions simply call C<parse_line()>, so if you're only splitting
one line you can call C<parse_line()> directly and save a function
call.

The C<$keep> controls what happens with delimters and special characters:

=over 4

=item true

If true, then the tokens are split on the specified delimiter,
but all other characters (including quotes and backslashes)
are kept in the tokens.

=item false

If $keep is false then the C<*quotewords()> functions
remove all quotes and backslashes that are
not themselves backslash-escaped or inside of single quotes (i.e.,
C<quotewords()> tries to interpret these characters just like the Bourne
shell).  NB: these semantics are significantly different from the
original version of this module shipped with Perl 5.000 through 5.004.

=item C<"delimiters">

As an additional feature, $keep may be the keyword "delimiters" which
causes the functions to preserve the delimiters in each string as
tokens in the token lists, in addition to preserving quote and
backslash characters.

=back

C<shellwords()> is written as a special case of C<quotewords()>, and it
does token parsing with whitespace as a delimiter-- similar to most
Unix shells.

=head1 EXAMPLES

The sample program:

  use Text::ParseWords;
  @words = quotewords('\s+', 0, q{this   is "a test" of\ quotewords \"for you});
  $i = 0;
  foreach (@words) {
      print "$i: <$_>\n";
      $i++;
  }

produces:

  0: <this>
  1: <is>
  2: <a test>
  3: <of quotewords>
  4: <"for>
  5: <you>

demonstrating:

=over 4

=item 0Z<>

a simple word

=item 1Z<>

multiple spaces are skipped because of our $delim

=item 2Z<>

use of quotes to include a space in a word

=item 3Z<>

use of a backslash to include a space in a word

=item 4Z<>

use of a backslash to remove the special meaning of a double-quote

=item 5Z<>

another simple word (note the lack of effect of the
backslashed double-quote)

=back

Replacing C<quotewords('\s+', 0, q{this   is...})>
with C<shellwords(q{this   is...})>
is a simpler way to accomplish the same thing.

=head1 SEE ALSO

L<Text::CSV> - for parsing CSV files

=head1 AUTHORS

The original author is unknown,
but presumably this evolved from C<shellwords.pl> in Perl 4.

Much of the code for C<parse_line()>
(including the primary regexp)
came from Joerk Behrends E<lt>jbehrends@multimediaproduzenten.deE<gt>.

Examples section and other documentation provided by
John Heidemann E<lt>johnh@ISI.EDUE<gt>.

Hal Pomeranz E<lt>pomeranz@netcom.comE<gt>
maintained this from 1994 through 1999,
and did the first CPAN release.

Alexandr Ciornii E<lt>alexchornyATgmail.comE<gt>
maintained this from 2008 to 2015.

Many other people have contributed,
with special thanks due to 
Michael Schwern E<lt>schwern@envirolink.orgE<gt>
and
Jeff Friedl E<lt>jfriedl@yahoo-inc.comE<gt>.

=head1 COPYRIGHT AND LICENSE

This library is free software; you may redistribute and/or modify it
under the same terms as Perl itself.

=cut
