#!/usr/bin/perl -w

use strict;

no locale;

my %items;
my $item_key;

$/ = '';

while (<>) {
  if (/^=item\s+(.+)/) {
    # new item

    $item_key = get_item_key($1);
    $items{$item_key} .= $_;

  } elsif (/^=back\b/) {
    # no more items in this group

    foreach my $item_key (sort keys %items) {
      print $items{$item_key};
    }

    $item_key = undef;
    %items = ();

    print;

  } elsif (defined $item_key) {
    # part of the current item

    $items{$item_key} .= $_;

  } else {
    # not part of an item

    print;

  }
}

if (keys %items) {
  warn "Missing =back after final =item.\n";

  foreach my $item_key (sort keys %items) {
    print $items{$item_key};
  }
}


# get the sortable key for an item
sub get_item_key {
  my($item) = @_;

  # remove POD formatting
  $item =~ s/[A-Z]<(.*?)>/$1/g;

  # remove printf-style escapes
  # note: be careful not to remove things like %hash
  $item =~ s/%(?:[scg]|lx|#o)//g;

  # remove all non-letter characters
  $item =~ tr/A-Za-z//cd;

  return lc $item;

}

__END__

=pod

=head1 NAME

sort_perldiag.pl - Sort warning and error messages in perldiag.pod

=head1 SYNOPSIS

B<sort_perldiag.pl> I<file>

=head1 DESCRIPTION

B<sort_perldiag.pl> is a script for sorting the warning and error
messages in F<perldiag.pod>.  POD formatting, printf-style escapes,
non-letter characters, and case are ignored, as explained in L<perldiag>.

=cut

