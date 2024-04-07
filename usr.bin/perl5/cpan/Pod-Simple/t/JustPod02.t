use strict;
use warnings;

use Test::More;

BEGIN { plan tests => 1 }

use Pod::Simple::JustPod;

my @orig = <DATA>;
my $parsed;

my $parser = Pod::Simple::JustPod->new();
$parser->output_string(\$parsed);
$parser->parse_lines(@orig, undef);

my $orig = join "", @orig;

my $msg = "Verify parsed pod sufficiently matches original";
if ($parsed eq $orig) {
    pass($msg);
}
elsif ($ENV{PERL_TEST_DIFF}) {
    fail($msg);
    require File::Temp;
    my $orig_file = File::Temp->new();
    local $/ = "\n";
    chomp $orig;
    print $orig_file $orig, "\n";
    close $orig_file || die "Can't close orig_file: $!";
  
    chomp $parsed;
    my $parsed_file = File::Temp->new();
    print $parsed_file $parsed, "\n";
    close $parsed_file || die "Can't close parsed_file";
  
    my $diff = File::Temp->new();
    system("$ENV{PERL_TEST_DIFF} $orig_file $parsed_file > $diff");
  
    open my $fh, "<", $diff || die "Can't open $diff";
    my @diffs = <$fh>;
    diag(@diffs);
}
else {
    eval { require Text::Diff; };
    if ($@) {
        is($parsed, $orig, $msg);
        diag("Set environment variable PERL_TEST_DIFF=diff_tool or install"
           . " Text::Diff to see just the differences.");
    }
    else {
        fail($msg);
        diag Text::Diff::diff(\$orig, \$parsed, { STYLE => 'Unified' });
    }
}

# The data is adapted from a test file from pod2lators.  Extra spaces are
# added in places to make sure they get retained, and some extra tests
__DATA__
=pod

=encoding  ASCII

=head1 NAME

basic.pod - Test of various basic POD features in translators.

=head1 HEADINGS

Try a few different levels of headings, with embedded formatting codes and
other interesting bits.

=head1 This C<is> a "level 1" heading

=head2  ``Level'' "2 I<heading>

=head3  Level 3 B<heading I<with C<weird F<stuff "" (double quote)>>>>

=head4  Level "4 C<heading>

=head5  Level "5 B<heading>

=head6  Level "6 I<heading>

Now try again with B<intermixed> F<text>.

=head1 This C<is> a "level 1" heading

Text.

=head2 ``Level'' 2 I<heading>

Text.

=head3 Level 3 B<heading I<with C<weird F<stuff>>>>

Text.

=head4 Level "4 C<heading>

Text.

=head5  Level "5 B<heading>

Text.

=head6  Level "6 I<heading>

Text.

=head1 LINKS

These are all taken from the Pod::Parser tests.

Try out I<LOTS> of different ways of specifying references:

Reference the L<manpage/section>

Reference the L<"manpage"/section>

Reference the L<manpage/"section">

Now try it using the new "|" stuff ...

Reference the L<thistext|manpage/section>|

Reference the L<thistext | manpage / section>|

Reference the L<thistext| manpage/ section>|

Reference the L<thistext |manpage /section>|

Reference the L<thistext|manpage/"section">|

Reference the L<thistext|
manpage/
section>|

And then throw in a few new ones of my own.

L<foo>

L<foo|bar>

L<foo/bar>

L<foo/"baz boo">

L</bar>

L</"baz boo">

L</baz boo>

L<foo bar/baz boo>

L<"boo var baz">

L<bar baz>

L</boo>, L</bar>, and L</baz>

L<fooZ<>bar>

L<Testing I<italics>|foo/bar>

L<foo/I<Italic> text>

L<fooE<verbar>barZ<>/Section C<with> I<B<other> markup>>

=head1 OVER AND ITEMS

Taken from Pod::Parser tests, this is a test to ensure that multiline
=item paragraphs get indented appropriately.

=over 4

=item This 
is
a
test.

=back

There should be whitespace now before this line.

Taken from Pod::Parser tests, this is a test to ensure the nested =item
paragraphs get indented appropriately.

=over  2

=item  1

First section.

=over 2

=item a

this is item a

=item b

this is item b

=back

=item 2

Second section.

=over 2

=item  a

this is item a

=item b

this is item b

=item c

=item d

This is item c & d.

=back

=back

Now some additional weirdness of our own.  Make sure that multiple tags
for one paragraph are properly compacted.

=over 4

=item  "foo"

=item B<bar>

=item C<baz>

There shouldn't be any spaces between any of these item tags; this idiom
is used in perlfunc.

=item   Some longer item text

Just to make sure that we test paragraphs where the item text doesn't fit
in the margin of the paragraph (and make sure that this paragraph fills a
few lines).

Let's also make it multiple paragraphs to be sure that works.

=back

Test use of =over without =item as a block "quote" or block paragraph.

=over 4

This should be indented four spaces but otherwise formatted the same as
any other regular text paragraph.  Make sure it's long enough to see the
results of the formatting.....

=back

Now try the same thing nested, and make sure that the indentation is reset
back properly.

=over 4

=over 4

This paragraph should be doubly indented.

=back

This paragraph should only be singly indented.

=over 4

=item

This is an item in the middle of a block-quote, which should be allowed.

=item

We're also testing tagless item commands.

=back

Should be back to the single level of indentation.

=back

Should be back to regular indentation.

Now also check the transformation of * into real bullets for man pages.

=over

=item *

An item.  We're also testing using =over without a number, and making sure
that item text wraps properly.

=item  *

Another item.

=back

and now test the numbering of item blocks.

=over 4

=item  1.

First item.

=item 2.

Second item.

=back

=head1   FORMATTING    CODES

Another test taken from Pod::Parser.

This is a test to see if I can do not only C<$self> and C<method()>, but
also C<< $self->method() >> and C<< $self->{FIELDNAME} >> and
C<< $Foo <=> $Bar >> without resorting to escape sequences. If 
I want to refer to the right-shift operator I can do something
like C<<< $x >> 3 >>> or even C<<<< $y >> 5 >>>>.

Now for the grand finale of C<< $self->method()->{FIELDNAME} = {FOO=>BAR} >>.
And I also want to make sure that newlines work like this
C<<<
$self->{FOOBAR} >> 3 and [$b => $a]->[$a <=> $b]
>>>

Of course I should still be able to do all this I<with> escape sequences
too: C<$self-E<gt>method()> and C<$self-E<gt>{FIELDNAME}> and
C<{FOO=E<gt>BAR}>.

Dont forget C<$self-E<gt>method()-E<gt>{FIELDNAME} = {FOO=E<gt>BAR}>.

And make sure that C<0> works too!

Now, if I use << or >> as my delimiters, then I have to use whitespace.
So things like C<<$self->method()>> and C<<$self->{FIELDNAME}>> wont end
up doing what you might expect since the first > will still terminate
the first < seen.

Lets make sure these work for empty ones too, like C<<<  >>>,
C<<<< 
>>>>, and C<< >> >> (just to be obnoxious)

The statement: C<This is dog kind's I<finest> hour!> is a parody of a
quotation from Winston Churchill.

The following tests are added to those:

Make sure that a few othZ<>er odd I<Z<>things> still work.  This should be
a vertical bar:  E<verbar>.  Here's a test of a few more special escapes
that have to be supported:

=over 3

=item  E<amp>

An ampersand.

=item E<apos>

An apostrophe.

=item E<lt>

A less-than sign.

=item E<gt>

A greater-than sign.

=item E<quot>

A double quotation mark.

=item E<sol>

A forward slash.

=back

Try to get this bit of text over towards the edge so S<|that all of this
text inside SE<lt>E<gt> won't|> be wrapped.  Also test the
|sameE<nbsp>thingE<nbsp>withE<nbsp>non-breakingS< spaces>.|

There is a soft hyE<shy>phen in hyphen at hy-phen.

This is a test of an X<index entry>index entry.

=head1 VERBATIM

Throw in a few verbatim paragraphs.

    use Term::ANSIColor;
    print color 'bold blue';
    print "This text is bold blue.\n";
    print color 'reset';
    print "This text is normal.\n";
    print colored ("Yellow on magenta.\n", 'yellow on_magenta');
    print "This text is normal.\n";
    print colored ['yellow on_magenta'], "Yellow on magenta.\n";

    use Term::ANSIColor qw(uncolor);
    print uncolor '01;31', "\n";

But this isn't verbatim (make sure it wraps properly), and the next
paragraph is again:

    use Term::ANSIColor qw(:constants);
    print BOLD, BLUE, "This text is in bold blue.\n", RESET;

    use Term::ANSIColor qw(:constants); $Term::ANSIColor::AUTORESET = 1; print BOLD BLUE "This text is in bold blue.\n"; print "This text is normal.\n";

(Ugh, that's obnoxiously long.)  Try different spacing:

	Starting with a tab.
Not
starting
with
a
tab.  But this should still be verbatim.
 As should this.

This isn't.

 This is.  And this:	is an internal tab.  It should be:
                    |--| <= lined up with that.

(Tricky, but tabs should be expanded before the translator starts in on
the text since otherwise text with mixed tabs and spaces will get messed
up.)

    And now we test verbatim paragraphs right before a heading.  Older
    versions of Pod::Man generated two spaces between paragraphs like this
    and the heading.  (In order to properly test this, one may have to
    visually inspect the nroff output when run on the generated *roff
    text, unfortunately.)

=head1 CONCLUSION

That's all, folks!

=cut
