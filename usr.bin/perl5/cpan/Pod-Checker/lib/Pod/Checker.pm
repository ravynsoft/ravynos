#############################################################################
# Pod/Checker.pm -- check pod documents for syntax errors
#
# Copyright (C) 1994-2000 by Bradford Appleton. All rights reserved.
# This is free software; you can redistribute it and/or modify it under the
# same terms as Perl itself.
#############################################################################

package Pod::Checker;
use strict;
use warnings;

our $VERSION = '1.75';  ## Current version of this package

=head1 NAME

Pod::Checker - check pod documents for syntax errors

=head1 SYNOPSIS

  use Pod::Checker;

  $syntax_okay = podchecker($filepath, $outputpath, %options);

  my $checker = Pod::Checker->new(%options);
  $checker->parse_from_file($filepath, \*STDERR);

=head1 OPTIONS/ARGUMENTS

C<$filepath> is the input POD to read and C<$outputpath> is
where to write POD syntax error messages. Either argument may be a scalar
indicating a file-path, or else a reference to an open filehandle.
If unspecified, the input-file it defaults to C<\*STDIN>, and
the output-file defaults to C<\*STDERR>.

=head2 podchecker()

This function can take a hash of options:

=over 4

=item B<-warnings> =E<gt> I<val>

Turn warnings on/off. I<val> is usually 1 for on, but higher values
trigger additional warnings. See L<"Warnings">.

=item B<-quiet> =E<gt> I<val>

If C<val> is true, do not print any errors/warnings.

=back

=head1 DESCRIPTION

B<podchecker> will perform syntax checking of Perl5 POD format documentation.

Curious/ambitious users are welcome to propose additional features they wish
to see in B<Pod::Checker> and B<podchecker> and verify that the checks are
consistent with L<perlpod>.

The following checks are currently performed:

=over 4

=item *

Unknown '=xxxx' commands, unknown 'XE<lt>...E<gt>' interior-sequences,
and unterminated interior sequences.

=item *

Check for proper balancing of C<=begin> and C<=end>. The contents of such
a block are generally ignored, i.e. no syntax checks are performed.

=item *

Check for proper nesting and balancing of C<=over>, C<=item> and C<=back>.

=item *

Check for same nested interior-sequences (e.g.
C<LE<lt>...LE<lt>...E<gt>...E<gt>>).

=item *

Check for malformed or non-existing entities C<EE<lt>...E<gt>>.

=item *

Check for correct syntax of hyperlinks C<LE<lt>...E<gt>>. See L<perlpod>
for details.

=item *

Check for unresolved document-internal links. This check may also reveal
misspelled links that seem to be internal links but should be links
to something else.

=back

=head1 DIAGNOSTICS

=head2 Errors

=over 4

=item * empty =headn

A heading (C<=head1> or C<=head2>) without any text? That ain't no
heading!

=item * =over on line I<N> without closing =back

=item * You forgot a '=back' before '=headI<N>'

=item * =over is the last thing in the document?!

The C<=over> command does not have a corresponding C<=back> before the
next heading (C<=head1> or C<=head2>) or the end of the file.

=item * '=item' outside of any '=over'

=item * =back without =over

An C<=item> or C<=back> command has been found outside a
C<=over>/C<=back> block.

=item * Can't have a 0 in =over I<N>

You need to indent a strictly positive number of spaces, not 0.

=item * =over should be: '=over' or '=over positive_number'

Either have an argumentless =over, or have its argument a strictly positive number.

=item * =begin I<TARGET> without matching =end I<TARGET>

A C<=begin> command was found that has no matching =end command.

=item * =begin without a target?

A C<=begin> command was found that is not followed by the formatter
specification.

=item * =end I<TARGET> without matching =begin.

A standalone C<=end> command was found.

=item * '=end' without a target?

'=end' directives need to have a target, just like =begin directives.

=item * '=end I<TARGET>' is invalid.

I<TARGET> needs to be one word

=item * =end I<CONTENT> doesn't match =begin I<TARGET>

I<CONTENT> needs to match =begin's I<TARGET>.

=item * =for without a target?

There is no specification of the formatter after the C<=for> command.

=item * unresolved internal link I<NAME>

The given link to I<NAME> does not have a matching node in the current
POD. This also happened when a single word node name is not enclosed in
C<"">.

=item * Unknown directive: I<CMD>

An invalid POD command has been found. Valid are C<=head1>, C<=head2>,
C<=head3>, C<=head4>, C<=over>, C<=item>, C<=back>, C<=begin>, C<=end>,
C<=for>, C<=pod>, C<=cut>

=item * Deleting unknown formatting code I<SEQ>

An invalid markup command has been encountered. Valid are:
C<BE<lt>E<gt>>, C<CE<lt>E<gt>>, C<EE<lt>E<gt>>, C<FE<lt>E<gt>>,
C<IE<lt>E<gt>>, C<LE<lt>E<gt>>, C<SE<lt>E<gt>>, C<XE<lt>E<gt>>,
C<ZE<lt>E<gt>>

=item * Unterminated I<SEQ>E<lt>E<gt> sequence

An unclosed formatting code

=item * An EE<lt>...E<gt> surrounding strange content

The I<STRING> found cannot be interpreted as a character entity.

=item * An empty EE<lt>E<gt>

=item * An empty C<< LE<lt>E<gt> >>

=item * An empty XE<lt>E<gt>

There needs to be content inside E, L, and X formatting codes.

=item * Spurious text after =pod / =cut

The commands C<=pod> and C<=cut> do not take any arguments.

=item * =back doesn't take any parameters, but you said =back I<ARGUMENT>

The C<=back> command does not take any arguments.

=item * =pod directives shouldn't be over one line long!  Ignoring all I<N> lines of content

Self explanatory

=item * =cut found outside a pod block.

A '=cut' directive found in the middle of non-POD

=item * Invalid =encoding syntax: I<CONTENT>

Syntax error in =encoding directive

=back

=head2 Warnings

These may not necessarily cause trouble, but indicate mediocre style.

=over 4

=item * nested commands I<CMD>E<lt>...I<CMD>E<lt>...E<gt>...E<gt>

Two nested identical markup commands have been found. Generally this
does not make sense.

=item * multiple occurrences (I<N>) of link target I<name>

The POD file has some C<=item> and/or C<=head> commands that have
the same text. Potential hyperlinks to such a text cannot be unique then.
This warning is printed only with warning level greater than one.

=item * line containing nothing but whitespace in paragraph

There is some whitespace on a seemingly empty line. POD is very sensitive
to such things, so this is flagged. B<vi> users switch on the B<list>
option to avoid this problem.

=item * =item has no contents

There is a list C<=item> that has no text contents. You probably want to delete
empty items.

=item * You can't have =items (as at line I<N>) unless the first thing after the =over is an =item

A list introduced by C<=over> starts with a text or verbatim paragraph,
but continues with C<=item>s. Move the non-item paragraph out of the
C<=over>/C<=back> block.

=item * Expected '=item I<EXPECTED VALUE>'

=item * Expected '=item *'

=item * Possible =item type mismatch: 'I<x>' found leading a supposed definition =item

A list started with e.g. a bullet-like C<=item> and continued with a
numbered one. This is obviously inconsistent. For most translators the
type of the I<first> C<=item> determines the type of the list.

=item * You have '=item x' instead of the expected '=item I<N>'

Erroneous numbering of =item numbers; they need to ascend consecutively.

=item * Unknown E content in EE<lt>I<CONTENT>E<gt>

A character entity was found that does not belong to the standard
ISO set or the POD specials C<verbar> and C<sol>. I<Currently, this warning
only appears if a character entity was found that does not have a Unicode
character. This should be fixed to adhere to the original warning.>

=item * empty =over/=back block

The list opened with C<=over> does not contain anything.

=item * empty section in previous paragraph

The previous section (introduced by a C<=head> command) does not contain
any valid content. This usually indicates that something is missing. Note: A
C<=head1> followed immediately by C<=head2> does not trigger this warning.

=item * Verbatim paragraph in NAME section

The NAME section (C<=head1 NAME>) should consist of a single paragraph
with the script/module name, followed by a dash `-' and a very short
description of what the thing is good for.

=item * =headI<n> without preceding higher level

For example if there is a C<=head2> in the POD file prior to a
C<=head1>.

=item * A non-empty ZE<lt>E<gt>

The C<ZE<lt>E<gt>> sequence is supposed to be empty. Caveat: this issue is
detected in L<Pod::Simple> and will be flagged as an I<ERROR> by any client
code; any contents of C<ZE<lt>...E<gt>> will be disregarded, anyway.

=back

=head2 Hyperlinks

There are some warnings with respect to malformed hyperlinks:

=over 4

=item * ignoring leading/trailing whitespace in link

There is whitespace at the beginning or the end of the contents of
LE<lt>...E<gt>.

=item * alternative text/node '%s' contains non-escaped | or /

The characters C<|> and C</> are special in the LE<lt>...E<gt> context.
Although the hyperlink parser does its best to determine which "/" is
text and which is a delimiter in case of doubt, one ought to escape
these literal characters like this:

  /     E<sol>
  |     E<verbar>

=back

Note that the line number of the error/warning may refer to the line number of
the start of the paragraph in which the error/warning exists, not the line 
number that the error/warning is on. This bug is present in errors/warnings
related to formatting codes. I<This should be fixed.>

=head1 RETURN VALUE

B<podchecker> returns the number of POD syntax errors found or -1 if
there were no POD commands at all found in the file.

=head1 EXAMPLES

See L</SYNOPSIS>

=head1 SCRIPTS

The B<podchecker> script that comes with this distribution is a lean wrapper
around this module. See the online manual with

  podchecker -help
  podchecker -man

=head1 INTERFACE

While checking, this module collects document properties, e.g. the nodes
for hyperlinks (C<=headX>, C<=item>) and index entries (C<XE<lt>E<gt>>).
POD translators can use this feature to syntax-check and get the nodes in
a first pass before actually starting to convert. This is expensive in terms
of execution time, but allows for very robust conversions.

Since v1.24 the B<Pod::Checker> module uses only the B<poderror>
method to print errors and warnings. The summary output (e.g.
"Pod syntax OK") has been dropped from the module and has been included in
B<podchecker> (the script). This allows users of B<Pod::Checker> to
control completely the output behavior. Users of B<podchecker> (the script)
get the well-known behavior.

v1.45 inherits from L<Pod::Simple> as opposed to all previous versions
inheriting from Pod::Parser. Do B<not> use Pod::Simple's interface when
using Pod::Checker unless it is documented somewhere on this page. I
repeat, DO B<NOT> USE POD::SIMPLE'S INTERFACE.

The following list documents the overrides to Pod::Simple, primarily to
make L<Pod::Coverage> happy:

=over 4

=item end_B

=item end_C

=item end_Document

=item end_F

=item end_I

=item end_L

=item end_Para

=item end_S

=item end_X

=item end_fcode

=item end_for

=item end_head

=item end_head1

=item end_head2

=item end_head3

=item end_head4

=item end_item

=item end_item_bullet

=item end_item_number

=item end_item_text

=item handle_pod_and_cut

=item handle_text

=item handle_whiteline

=item hyperlink

=item scream

=item start_B

=item start_C

=item start_Data

=item start_F

=item start_I

=item start_L

=item start_Para

=item start_S

=item start_Verbatim

=item start_X

=item start_fcode

=item start_for

=item start_head

=item start_head1

=item start_head2

=item start_head3

=item start_head4

=item start_item_bullet

=item start_item_number

=item start_item_text

=item start_over

=item start_over_block

=item start_over_bullet

=item start_over_empty

=item start_over_number

=item start_over_text

=item whine

=back

=cut

#############################################################################

#use diagnostics;
use Carp qw(croak);
use Exporter 'import';
use base qw/Pod::Simple::Methody/;

our @EXPORT = qw(&podchecker);

##---------------------------------
## Function definitions begin here
##---------------------------------

sub podchecker {
    my ($infile, $outfile, %options) = @_;
    local $_;

    ## Set defaults
    $infile  ||= \*STDIN;
    $outfile ||= \*STDERR;

    ## Now create a pod checker
    my $checker = Pod::Checker->new(%options);

    ## Now check the pod document for errors
    $checker->parse_from_file($infile, $outfile);

    ## Return the number of errors found
    return $checker->num_errors();
}


##---------------------------------------------------------------------------

##-------------------------------
## Method definitions begin here
##-------------------------------

##################################

=over 4

=item C<Pod::Checker-E<gt>new( %options )>

Return a reference to a new Pod::Checker object that inherits from
Pod::Simple and is used for calling the required methods later. The
following options are recognized:

C<-warnings =E<gt> num>
  Print warnings if C<num> is true. The higher the value of C<num>,
the more warnings are printed. Currently there are only levels 1 and 2.

C<-quiet =E<gt> num>
  If C<num> is true, do not print any errors/warnings. This is useful
when Pod::Checker is used to munge POD code into plain text from within
POD formatters.

=cut

sub new {
    my $new = shift->SUPER::new(@_);
    $new->{'output_fh'} ||= *STDERR{IO};

    # Set options
    my %opts = @_;
    $new->{'-warnings'} = defined $opts{'-warnings'} ?
                                  $opts{'-warnings'} : 1; # default on
    $new->{'-quiet'} = $opts{'-quiet'} || 0; # default off

    # Initialize number of errors/warnings
    $new->{'_NUM_ERRORS'} = 0;
    $new->{'_NUM_WARNINGS'} = 0;

    # 'current' also means 'most recent' in the follow comments
    $new->{'_thispara'} = '';       # current POD paragraph
    $new->{'_line'} = 0;            # current line number
    $new->{'_head_num'} = 0;        # current =head level (set to 0 to make
                                    #   logic easier down the road)
    $new->{'_cmds_since_head'} = 0; # num of POD directives since prev. =headN
    $new->{'_nodes'} = [];          # stack for =head/=item nodes
    $new->{'_fcode_stack'} = [];    # stack for nested formatting codes
    $new->{'_fcode_pos'} = [];      # stack for position in paragraph of fcodes
    $new->{'_begin_stack'} = [];    # stack for =begins: [line #, target]
    $new->{'_links'} = [];          # stack for hyperlinks to external entities
    $new->{'_internal_links'} = []; # set of linked-to internal sections
    $new->{'_index'} = [];          # stack for text in X<>s

    $new->accept_targets('*'); # check all =begin/=for blocks
    $new->cut_handler( \&handle_pod_and_cut ); # warn if text after =cut
    $new->pod_handler( \&handle_pod_and_cut ); # warn if text after =pod
    $new->whiteline_handler( \&handle_whiteline ); # warn if whiteline
    $new->parse_empty_lists(1); # warn if they are empty

    return $new;
}

##################################

=item C<$checker-E<gt>poderror( @args )>

=item C<$checker-E<gt>poderror( {%opts}, @args )>

Internal method for printing errors and warnings. If no options are given,
simply prints "@_". The following options are recognized and used to form
the output:

  -msg

A message to print prior to C<@args>.

  -line

The line number the error occurred in.

  -file

The file (name) the error occurred in. Defaults to the name of the current
file being processed.

  -severity

The error level, should be 'WARNING' or 'ERROR'.

=cut

# Invoked as $self->poderror( @args ), or $self->poderror( {%opts}, @args )
sub poderror {
    my $self = shift;
    my %opts = (ref $_[0]) ? %{shift()} : ();

    ## Retrieve options
    chomp( my $msg  = ($opts{'-msg'} || '')."@_" );
    my $line = (exists $opts{'-line'}) ? " at line $opts{'-line'}" : '';
    my $file = ' in file ' . ((exists $opts{'-file'})
                              ? $opts{'-file'}
                              : ((defined $self->source_filename)
                                 ? $self->source_filename
                                 : "???"));
    unless (exists $opts{'-severity'}) {
       ## See if can find severity in message prefix
       $opts{'-severity'} = $1  if ( $msg =~ s/^\**\s*([A-Z]{3,}):\s+// );
    }
    my $severity = (exists $opts{'-severity'}) ? "*** $opts{-severity}: " : '';

    ## Increment error count and print message "
    ++($self->{'_NUM_ERRORS'})
        if(!%opts || ($opts{-severity} && $opts{'-severity'} eq 'ERROR'));
    ++($self->{'_NUM_WARNINGS'})
        if(!%opts || ($opts{-severity} && $opts{'-severity'} eq 'WARNING'));
    unless($self->{'-quiet'}) {
      my $out_fh = $self->{'output_fh'} || \*STDERR;
      print $out_fh ($severity, $msg, $line, $file, "\n")
        if($self->{'-warnings'} || !%opts || $opts{'-severity'} ne 'WARNING');
    }
}

##################################

=item C<$checker-E<gt>num_errors()>

Set (if argument specified) and retrieve the number of errors found.

=cut

sub num_errors {
   return (@_ > 1) ? ($_[0]->{'_NUM_ERRORS'} = $_[1]) : $_[0]->{'_NUM_ERRORS'};
}

##################################

=item C<$checker-E<gt>num_warnings()>

Set (if argument specified) and retrieve the number of warnings found.

=cut

sub num_warnings {
   return (@_ > 1) ? ($_[0]->{'_NUM_WARNINGS'} = $_[1]) :
                      $_[0]->{'_NUM_WARNINGS'};
}

##################################

=item C<$checker-E<gt>name()>

Set (if argument specified) and retrieve the canonical name of POD as
found in the C<=head1 NAME> section.

=cut

sub name {
    return (@_ > 1 && $_[1]) ?
        ($_[0]->{'_pod_name'} = $_[1]) : $_[0]->{'_pod_name'};
}

##################################

=item C<$checker-E<gt>node()>

Add (if argument specified) and retrieve the nodes (as defined by C<=headX>
and C<=item>) of the current POD. The nodes are returned in the order of
their occurrence. They consist of plain text, each piece of whitespace is
collapsed to a single blank.

=cut

sub node {
    my ($self,$text) = @_;
    if(defined $text) {
        $text =~ s/\s+$//s; # strip trailing whitespace
        $text =~ s/\s+/ /gs; # collapse whitespace
        # add node, order important!
        push(@{$self->{'_nodes'}}, $text);
        # keep also a uniqueness counter
        $self->{'_unique_nodes'}->{$text}++ if($text !~ /^\s*$/s);
        return $text;
    }
    @{$self->{'_nodes'}};
}

##################################

=item C<$checker-E<gt>idx()>

Add (if argument specified) and retrieve the index entries (as defined by
C<XE<lt>E<gt>>) of the current POD. They consist of plain text, each piece
of whitespace is collapsed to a single blank.

=cut

# set/return index entries of current POD
sub idx {
    my ($self,$text) = @_;
    if(defined $text) {
        $text =~ s/\s+$//s; # strip trailing whitespace
        $text =~ s/\s+/ /gs; # collapse whitespace
        # add node, order important!
        push(@{$self->{'_index'}}, $text);
        # keep also a uniqueness counter
        $self->{'_unique_nodes'}->{$text}++ if($text !~ /^\s*$/s);
        return $text;
    }
    @{$self->{'_index'}};
}

##################################

# add a hyperlink to the list of those of the current POD; returns current
# list after the addition has been done
sub hyperlink {
    my $self = shift;
    push(@{$self->{'_links'}}, $_[0]);
    return $_[0];
}

=item C<$checker-E<gt>hyperlinks()>

Retrieve an array containing the hyperlinks to things outside
the current POD (as defined by C<LE<lt>E<gt>>).

Each is an instance of a class with the following methods:

=cut

sub hyperlinks {
    @{shift->{'_links'}};
}

##################################

# override Pod::Simple's whine() and scream() to use poderror()

# Note:
# Ignore $self->{'no_whining'} b/c $self->{'quiet'} takes care of it in poderror
# Don't bother incrementing $self->{'errors_seen'} -- it's not used
# Don't bother pushing to $self->{'errata'} b/c poderror() outputs immediately
# We don't need to set $self->no_errata_section(1) b/c of these overrides


sub whine {
    my ($self, $line, $complaint) = @_;

    my $severity = 'ERROR';

    if (0) {
      # XXX: Let's standardize what's a warning and what's an error.  Let's not
      # move stuff up and down the severity tree.  -- rjbs, 2013-04-12
      # Convert errors in Pod::Simple that are warnings in Pod::Checker
      # XXX Do differently so the $complaint can be reworded without this breaking
      $severity = 'WARNING' if
          $complaint =~ /^Expected '=item .+?'$/ ||
          $complaint =~ /^You can't have =items \(as at line .+?\) unless the first thing after the =over is an =item$/ ||
          $complaint =~ /^You have '=item .+?' instead of the expected '=item .+?'$/;
    }

    # rt.cpan.org #98326 - errors about Z<> ("non-empty")
    $severity = 'WARNING' if $complaint =~ /\bZ\<\>/;

    $self->poderror({ -line => $line,
                      -severity => $severity,
                      -msg => $complaint });

    return 1; # assume everything is peachy keen
}

sub scream {
    my ($self, $line, $complaint) = @_;

    $self->poderror({ -line => $line,
                      -severity => 'ERROR', # consider making severity 'FATAL'
                      -msg => $complaint });

    return 1;
}


##################################

# Some helper subroutines

sub _init_event { # assignments done at the start of most events
    $_[0]{'_thispara'} = '';
    $_[0]{'_line'} = $_[1]{'start_line'};
    $_[0]{'_cmds_since_head'}++;
}

sub _check_fcode {
    my ($self, $inner, $outers) = @_;
    # Check for an fcode inside another of the same fcode
    # XXX line number is the line of the start of the paragraph that the warning
    # is in, not the line that the warning is on. Fix this

    # Later versions of Pod::Simple forbid nested L<>'s
    return if $inner eq 'L' && $Pod::Simple::VERSION ge '3.33';

    if (grep { $_ eq $inner } @$outers) {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'WARNING',
                          -msg => "nested commands $inner<...$inner<...>...>"});
    }
}

##################################

sub handle_text { $_[0]{'_thispara'} .= $_[1] }

# whiteline is a seemingly blank line that matches /[^\S\r\n]/
sub handle_whiteline {
    my ($line, $line_n, $self) = @_;
    $self->poderror({
        -line => $line_n,
        -severity => 'WARNING',
        -msg => 'line containing nothing but whitespace in paragraph'});
}

######## Directives
sub handle_pod_and_cut {
    my ($line, $line_n, $self) = @_;
    $self->{'_cmds_since_head'}++;
    if ($line =~ /=(pod|cut)\s+\S/) {
        $self->poderror({ -line => $line_n,
                          -severity => 'ERROR',
                          -msg => "Spurious text after =$1"});
    }
}

sub start_Para { shift->_init_event(@_); }
sub end_Para   {
    my $self = shift;
    # Get the NAME of the pod document
    if ($self->{'_head_num'} == 1 && $self->{'_head_text'} eq 'NAME') {
        if ($self->{'_thispara'} =~ /^\s*(\S+?)\s*[,-]/) {
            $self->{'_pod_name'} = $1 unless defined $self->{'_pod_name'};
        }
    }
}

sub start_Verbatim {
    my $self = shift;
    $self->_init_event(@_);

    if ($self->{'_head_num'} == 1 && $self->{'_head_text'} eq 'NAME') {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'WARNING',
                          -msg => 'Verbatim paragraph in NAME section' });
    }
}
# Don't need an end_Verbatim

# Do I need to do anything else with this?
sub start_Data { shift->_init_event() }

sub start_head1 { shift->start_head(1, @_) }
sub start_head2 { shift->start_head(2, @_) }
sub start_head3 { shift->start_head(3, @_) }
sub start_head4 { shift->start_head(4, @_) }
sub start_head  {
    my $self = shift;
    my $h = shift;
    $self->_init_event(@_);
    my $prev_h = $self->{'_head_num'};
    $self->{'_head_num'} = $h;
    $self->{"_count_head$h"}++;

    if ($h > 1 && !$self->{'_count_head'.($h-1)}) {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'WARNING',
                          -msg => "=head$h without preceding higher level"});
    }

    # If this is the first =head of the doc, $prev_h is 0, thus less than $h
    if ($self->{'_cmds_since_head'} == 1 && $prev_h >= $h) {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'WARNING',
                          -msg => 'empty section in previous paragraph'});
    }
}

sub end_head1 { shift->end_head(@_) }
sub end_head2 { shift->end_head(@_) }
sub end_head3 { shift->end_head(@_) }
sub end_head4 { shift->end_head(@_) }
sub end_head  {
    my $self = shift;
    my $arg = $self->{'_thispara'};
    $arg =~ s/\s+$//;
    $self->{'_head_text'} = $arg;
    $self->{'_cmds_since_head'} = 0;
    my $h = $self->{'_head_num'};
    $self->node($arg); # remember this node
    if ($arg eq '') {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'ERROR',
                          -msg => "empty =head$h" });
    }
}

sub start_over_bullet { shift->start_over(@_, 'bullet') }
sub start_over_number { shift->start_over(@_, 'number') }
sub start_over_text   { shift->start_over(@_, 'definition') }
sub start_over_block  { shift->start_over(@_, 'block') }
sub start_over_empty  {
    my $self = shift;
    $self->start_over(@_, 'empty');
    $self->poderror({ -line => $self->{'_line'},
                      -severity => 'WARNING',
                      -msg => 'empty =over/=back block' });
}
sub start_over {
    my $self = shift;
    my $type = pop;
    $self->_init_event(@_);
}

sub start_item_bullet { shift->_init_event(@_) }
sub start_item_number { shift->_init_event(@_) }
sub start_item_text   { shift->_init_event(@_) }
sub end_item_bullet { shift->end_item('bullet') }
sub end_item_number { shift->end_item('number') }
sub end_item_text   { shift->end_item('definition') }
sub end_item {
    my $self = shift;
    my $type = shift;
    # If there is verbatim text in this item, it will show up as part of
    # 'paras', and not part of '_thispara'.  If the first para after this is a
    # verbatim one, it actually will be (part of) the contents for this item.
    if (   $self->{'_thispara'} eq ''
        && (  ! @{$self->{'paras'}}
            ||    $self->{'paras'}[0][0] !~ /Verbatim/i))
    {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'WARNING',
                          -msg => '=item has no contents' });
    }

    $self->node($self->{'_thispara'}); # remember this node
}

sub start_for { # =for and =begin directives
    my ($self, $flags) = @_;
    $self->_init_event($flags);
    push @{$self->{'_begin_stack'}}, [$self->{'_line'}, $flags->{'target'}];
}

sub end_for {
    my ($self, $flags) = @_;
    my ($line, $target) = @{pop @{$self->{'_begin_stack'}}};
    if ($flags->{'fake-closer'}) { # meaning Pod::Simple generated this =end
        $self->poderror({ -line => $line,
                          -severity => 'ERROR',
                          -msg => "=begin $target without matching =end $target"
                        });
    }
}

sub end_Document {
    # Some final error checks
    my $self = shift;

    # no POD found here
    $self->num_errors(-1) && return unless $self->content_seen;

    my %nodes;
    for ($self->node()) {
        $nodes{$_} = 1;
        if(/^(\S+)\s+\S/) {
            # we have more than one word. Use the first as a node, too.
            # This is used heavily in perlfunc.pod
            $nodes{$1} ||= 2; # derived node
        }
    }
    for ($self->idx()) {
        $nodes{$_} = 3; # index node
    }

    # XXX update unresolved internal link POD -- single word not enclosed in ""?
    # I don't know what I was thinking when I made the above TODO, and I don't
    # know what it means...

    for my $link (@{ $self->{'_internal_links'} }) {
        my ($name, $line) = @$link;
        unless ( $nodes{$name} ) {
            $self->poderror({ -line => $line,
                              -severity => 'ERROR',
                              -msg => "unresolved internal link '$name'"});
        }
    }

    # check the internal nodes for uniqueness. This pertains to
    # =headX, =item and X<...>
    if ($self->{'-warnings'} > 1 ) {
        for my $node (sort keys %{ $self->{'_unique_nodes'} }) {
            my $count = $self->{'_unique_nodes'}{$node};
            if ($count > 1) { # not unique
                $self->poderror({
                    -line => '-',
                    -severity => 'WARNING',
                    -msg => "multiple occurrences ($count) of link target ".
                        "'$node'"});
            }
        }
    }
}

########  Formatting codes

sub start_B { shift->start_fcode('B') }
sub start_C { shift->start_fcode('C') }
sub start_F { shift->start_fcode('F') }
sub start_I { shift->start_fcode('I') }
sub start_S { shift->start_fcode('S') }
sub start_fcode {
    my ($self, $fcode) = @_;
    unshift @{$self->{'_fcode_stack'}}, $fcode;
}

sub end_B { shift->end_fcode() }
sub end_C { shift->end_fcode() }
sub end_F { shift->end_fcode() }
sub end_I { shift->end_fcode() }
sub end_S { shift->end_fcode() }
sub end_fcode {
    my $self = shift;
    $self->_check_fcode(shift @{$self->{'_fcode_stack'}}, # current fcode removed
                        $self->{'_fcode_stack'}); # previous fcodes
}

sub start_L {
    my ($self, $flags) = @_;
    $self->start_fcode('L');

    my $link = Pod::Checker::Hyperlink->new($flags, $self);
    if ($link) {
        if (   $link->type eq 'pod'
            && $link->node
                # It's an internal-to-this-page link if no page is given, or
                # if the given one is to our NAME.
            && (! $link->page || (   $self->{'_pod_name'}
                                  && $link->page eq $self->{'_pod_name'})))
        {
            push @{ $self->{'_internal_links'} }, [ $link->{'-raw_node'}, $link->line ];
        }
        else {
            $self->hyperlink($link);
        }
    }
}

sub end_L {
    my $self = shift;
    $self->end_fcode();
}

sub start_X {
    my $self = shift;
    $self->start_fcode('X');
    # keep track of where X<> starts in the paragraph
    # (this is a stack so nested X<>s are handled correctly)
    push @{$self->{'_fcode_pos'}}, length $self->{'_thispara'};
}
sub end_X {
    my $self = shift;
    # extract contents of X<> and replace with ''
    my $start = pop @{$self->{'_fcode_pos'}}; # start at the beginning of X<>
    my $end = length($self->{'_thispara'}) - $start; # end at end of X<>
    my $x = substr($self->{'_thispara'}, $start, $end, '');
    if ($x eq "") {
        $self->poderror({ -line => $self->{'_line'},
                          -severity => 'ERROR',
                          -msg => "An empty X<>" });
    }
    $self->idx($x); # remember this node
    $self->end_fcode();
}

package Pod::Checker::Hyperlink;

# This class is used to represent L<> link structures, so that the individual
# elements are easily accessible.  It is based on code in Pod::Hyperlink

sub new {
    my ($class,
        $simple_link,   # The link structure returned by Pod::Simple
        $caller         # The caller class
    ) = @_;

    my $self = +{};
    bless $self, $class;

    $self->{'-line'} ||= $caller->{'_line'};
    $self->{'-type'} ||= $simple_link->{'type'};
    # preserve raw link text for additional checks
    $self->{'-raw-link-text'} = (exists $simple_link->{'raw'})
                                ? "$simple_link->{'raw'}"
                                : "";
    # Force stringification of page and node.  (This expands any E<>.)
    $self->{'-page'} = exists $simple_link->{'to'} ? "$simple_link->{'to'}" : "";
    $self->{'-node'} = exists $simple_link->{'section'} ? "$simple_link->{'section'}" : "";

    # Save the unmodified node text, as the .t files are expecting the message
    # for internal link failures to include it (hence this preserves backward
    # compatibility).
    $self->{'-raw_node'} = $self->{'-node'};

    # Remove leading/trailing white space.  Pod::Simple already warns about
    # these, so if the only error is this, and the link is otherwise correct,
    # only the Pod::Simple warning will be output, avoiding unnecessary
    # confusion.
    $self->{'-page'} =~ s/ ^ \s+ //x;
    $self->{'-page'} =~ s/ \s+ $ //x;

    $self->{'-node'} =~ s/ ^ \s+ //x;
    $self->{'-node'} =~ s/ \s+ $ //x;

    # Pod::Simple warns about L<> and L< >, but not L</>
    if ($self->{'-page'} eq "" && $self->{'-node'} eq "") {
        $caller->poderror({ -line => $caller->{'_line'},
                          -severity => 'WARNING',
                          -msg => 'empty link'});
        return;
    }

    return $self;
}

=item line()

Returns the approximate line number in which the link was encountered

=cut

sub line {
    return $_[0]->{-line};
}

=item type()

Returns the type of the link; one of:
C<"url"> for things like
C<http://www.foo>, C<"man"> for man pages, or C<"pod">.

=cut

sub type {
    return  $_[0]->{-type};
}

=item page()

Returns the linked-to page or url.

=cut

sub page {
    return $_[0]->{-page};
}

=item node()

Returns the anchor or node within the linked-to page, or an empty string
(C<"">) if none appears in the link.

=back

=cut

sub node {
    return $_[0]->{-node};
}

=head1 AUTHOR

Please report bugs using L<http://rt.cpan.org>.

Brad Appleton E<lt>bradapp@enteract.comE<gt> (initial version),
Marek Rouchal E<lt>marekr@cpan.orgE<gt>,
Marc Green E<lt>marcgreen@cpan.orgE<gt> (port to Pod::Simple)
Ricardo Signes E<lt>rjbs@cpan.orgE<gt> (more porting to Pod::Simple)
Karl Williamson E<lt>khw@cpan.orgE<gt> (more porting to Pod::Simple)

Based on code for B<Pod::Text::pod2text()> written by
Tom Christiansen E<lt>tchrist@mox.perl.comE<gt>

=cut

1
