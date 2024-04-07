use Pod::Usage;
use strict;
use warnings;

pod2usage(-verbose => 2, -exit => 17, -input => \*DATA);

__DATA__
=head1 NAME

Test

=head1 SYNOPSIS

perl podusagetest.pl

=head1 DESCRIPTION

This is a test. 

=cut

