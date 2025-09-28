use strict; use warnings;

package Text::Tabs;

BEGIN { require Exporter; *import = \&Exporter::import }

our @EXPORT = qw( expand unexpand $tabstop );

our $VERSION = '2021.0814';
our $SUBVERSION = 'modern'; # back-compat vestige

our $tabstop = 8;

sub expand {
	my @l;
	my $pad;
	for ( @_ ) {
		defined or do { push @l, ''; next };
		my $s = '';
		for (split(/^/m, $_, -1)) {
			my $offs;
			for (split(/\t/, $_, -1)) {
				if (defined $offs) {
					$pad = $tabstop - $offs % $tabstop;
					$s .= " " x $pad;
				}
				$s .= $_;
				$offs = () = /\PM/g;
			}
		}
		push(@l, $s);
	}
	return @l if wantarray;
	return $l[0];
}

sub unexpand
{
	my (@l) = @_;
	my @e;
	my $x;
	my $line;
	my @lines;
	my $lastbit;
	my $ts_as_space = " " x $tabstop;
	for $x (@l) {
		defined $x or next;
		@lines = split("\n", $x, -1);
		for $line (@lines) {
			$line = expand($line);
			@e = split(/((?:\PM\pM*){$tabstop})/,$line,-1);
			$lastbit = pop(@e);
			$lastbit = '' 
				unless defined $lastbit;
			$lastbit = "\t"
				if $lastbit eq $ts_as_space;
			for $_ (@e) {
				s/  +$/\t/;
			}
			$line = join('',@e, $lastbit);
		}
		$x = join("\n", @lines);
	}
	return @l if wantarray;
	return $l[0];
}

1;

__END__

=head1 NAME

Text::Tabs - expand and unexpand tabs like unix expand(1) and unexpand(1)

=head1 SYNOPSIS

  use Text::Tabs;

  $tabstop = 4;  # default = 8
  @lines_without_tabs = expand(@lines_with_tabs);
  @lines_with_tabs = unexpand(@lines_without_tabs);

=head1 DESCRIPTION

Text::Tabs does most of what the unix utilities expand(1) and unexpand(1) 
do.  Given a line with tabs in it, C<expand> replaces those tabs with
the appropriate number of spaces.  Given a line with or without tabs in
it, C<unexpand> adds tabs when it can save bytes by doing so, 
like the C<unexpand -a> command.  

Unlike the old unix utilities, this module correctly accounts for
any Unicode combining characters (such as diacriticals) that may occur
in each line for both expansion and unexpansion.  These are overstrike
characters that do not increment the logical position.  Make sure
you have the appropriate Unicode settings enabled.

=head1 EXPORTS

The following are exported:

=over 4

=item expand

=item unexpand

=item $tabstop

The C<$tabstop> variable controls how many column positions apart each
tabstop is.  The default is 8.

Please note that C<local($tabstop)> doesn't do the right thing and if you want
to use C<local> to override C<$tabstop>, you need to use
C<local($Text::Tabs::tabstop)>.

=back

=head1 EXAMPLE

  #!perl
  # unexpand -a
  use Text::Tabs;

  while (<>) {
    print unexpand $_;
  }

Instead of the shell's C<expand> command, use:

  perl -MText::Tabs -n -e 'print expand $_'

Instead of the shell's C<unexpand -a> command, use:

  perl -MText::Tabs -n -e 'print unexpand $_'

=head1 BUGS

Text::Tabs handles only tabs (C<"\t">) and combining characters (C</\pM/>).  It doesn't
count backwards for backspaces (C<"\t">), omit other non-printing control characters (C</\pC/>),
or otherwise deal with any other zero-, half-, and full-width characters.

=head1 LICENSE

Copyright (C) 1996-2002,2005,2006 David Muir Sharnoff.  
Copyright (C) 2005 Aristotle Pagaltzis 
Copyright (C) 2012-2013 Google, Inc.
This module may be modified, used, copied, and redistributed at your own risk.
Although allowed by the preceding license, please do not publicly
redistribute modified versions of this code with the name "Text::Tabs"
unless it passes the unmodified Text::Tabs test suite.
