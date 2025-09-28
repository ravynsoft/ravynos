#!/usr/bin/perl -w
use strict;
use warnings;
use 5.010;
use File::Find;
use IO::File;
use Getopt::Long;
use Pod::Usage;

my %limits = (
  c90 => {
           'logical-source-line-length' => 509,
         },
  c99 => {
           'logical-source-line-length' => 4095,
         },
);

my %opt = (
  std => 'c99',
);

GetOptions(\%opt, qw( logical-source-line-length=i std=s ))
  && @ARGV && exists $limits{$opt{std}}
    or pod2usage(2);

for my $k (keys %{$limits{$opt{std}}}) {
  $opt{$k} //= $limits{$opt{std}}{$k};
}

{
  my $num = 1;

  sub report
  {
    my $msg = shift;
    my $info = join '', @_;

    if ($info) {
      $info =~ s/\R+$//;
      $info =~ s/^/   #|\t/mg;
      $info = "\n$info\n\n";
    }

    warn sprintf "[%d] %s(%d): %s\n%s",
         $num++, $File::Find::name, $., $msg, $info;
  }
}

find(sub {
  /\.([ch]|xs)$/ or return;

  my $fh = IO::File->new($_, 'r') or die "$_: $!\n";
  my $ll = '';

  while (defined(my $line = <$fh>)) {
    report("trailing whitespace after backslash", $line)
        if $line =~ /\\[[:blank:]]+$/;

    $ll .= $line;

    unless ($ll =~ /\\$/) {
      if (length $ll > $opt{'logical-source-line-length'}) {
        report(sprintf("logical source line too long (%d > %d)",
                       length $ll, $opt{'logical-source-line-length'}), $ll);
      }
      $ll = '';
    }
  }
}, @ARGV);

__END__

=head1 NAME

checkansi.pl - Check source code for ANSI-C violations

=head1 SYNOPSIS

checkansi.pl [B<--std>=c90|c99]
[B<--logical-source-line-length>=I<num>]
<path> ...

=head1 DESCRIPTION

B<checkansi.pl> searches 

=head1 OPTIONS

=over 4

=item B<--std>=c90|c99

Choose the ANSI/ISO standard against which shall be checked.
Defaults to C<c99>.

=item B<--logical-source-line-length>=I<number>

Maximum length of a logical source line. Overrides the default
given by the chosen standard.

=back

=head1 COPYRIGHT

Copyright 2007 by Marcus Holland-Moritz <mhx@cpan.org>.

This program is free software; you may redistribute it
and/or modify it under the same terms as Perl itself.

=cut
