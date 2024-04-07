#!/usr/bin/perl
BEGIN {
   use File::Basename;
   my $THISDIR = dirname $0;
   unshift @INC, $THISDIR;
   require "testpchk.pl";
   import TestPodChecker;
}

# this tests Pod::Checker accepts =encoding directive

my %options = map { $_ => 1 } @ARGV;  ## convert cmdline to options-hash
my $passed  = testpodchecker \%options, $0;
exit( ($passed == 1) ? 0 : -1 )  unless $ENV{HARNESS_ACTIVE};

__END__

=encoding utf8

=encode utf8

dummy error

=head1 An example.

'Twas brillig, and the slithy toves did gyre and gimble in the wabe.

=cut

